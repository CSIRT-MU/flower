#include <processor.hpp>

#include <unordered_map>
#include <memory>
#include <atomic>
#include <csignal>
#include <chrono>

#include <tins/tins.h>

#include <log.hpp>
#include <common.hpp>
#include <options.hpp>
#include <cache.hpp>
#include <exporter.hpp>

#include <protocols/vxlan.hpp>

#include <flows/ip.hpp>
#include <flows/ipv6.hpp>
#include <flows/tcp.hpp>
#include <flows/udp.hpp>
#include <flows/vlan.hpp>
#include <flows/vxlan.hpp>

namespace Flow {

static std::unordered_map<Tins::PDU::PDUType, std::unique_ptr<Flow>> reducers;
static std::atomic<bool> running = true;
static Cache cache;

static void on_signal(int) {
  if (!running) {
    Log::info("Exiting...\n");
    std::exit(10);
  }

  Log::info("Shuting down. Please wait for cleanup...\n");
  running = false;
}

template<typename Reducer>
static void register_reducer(Tins::PDU::PDUType pdu_type) {
  reducers.emplace(pdu_type, std::make_unique<Reducer>(Options::get_toml()));
}

static std::vector<std::byte>
prepare_fields() {
  static const auto t = std::array{
    htons(IPFIX::FIELD_PACKET_DELTA_COUNT),
    htons(IPFIX::TYPE_64),
    htons(IPFIX::FIELD_FLOW_START_SECONDS),
    htons(IPFIX::TYPE_SECONDS),
    htons(IPFIX::FIELD_FLOW_END_SECONDS),
    htons(IPFIX::TYPE_SECONDS),
    htons(IPFIX::FIELD_FLOW_START_MILLISECONDS),
    htons(IPFIX::TYPE_MILLISECONDS),
    htons(IPFIX::FIELD_FLOW_END_MILLISECONDS),
    htons(IPFIX::TYPE_MILLISECONDS)
  };
  auto tp = reinterpret_cast<const std::byte*>(t.data());

  return {tp, tp + t.size() * IPFIX::TYPE_16};
}

/**
 * Performs active timout check on record in timestamp given in arguments.
 * Active timeout is set in global options. If record timed out, it is exported
 * and then its counter is reset.
 */
static void active_timeout_check(Exporter &exporter, Cache::Entry &entry,
                                 timeval ts) {
  const auto active_timeout = Options::active_timeout;

  auto& [props, type, values] = entry->second;
  auto diff = ts.tv_sec - props.flow_start.tv_sec;

  if (diff >= active_timeout) {
    Log::debug("Active timeout with error %lus\n", diff - active_timeout);

    exporter.insert_record(type, props, values);
    props = {0, ts, ts};
  }
}

static void idle_timeout_check(Exporter &exporter, timeval ts,
                               std::size_t peek_interval) {
  static std::size_t peek_digest = 0;

  const auto idle_timeout = Options::idle_timeout;

  auto to_remove = std::vector<std::size_t>{};
  auto it = cache.find(peek_digest);

  Log::debug("Checking %lu records for idle timeout\n", peek_interval);
  for (std::size_t i = 0; i < peek_interval; ++i, ++it) {
    /* If cache is empty */
    if (cache.begin() == cache.end())
      break;

    if (it == cache.end())
      it = cache.begin();

    auto& [props, type, values] = it->second;

    auto diff = ts.tv_sec - props.flow_end.tv_sec;
    if (diff >= idle_timeout) {
      Log::debug("Idle timeout with error %lus\n", diff - idle_timeout);

      exporter.insert_record(type, props, values);
      to_remove.emplace_back(it->first);
    }
  }

  /* Check if next exists and set it as peek digest */
  if (it != cache.end()) {
    auto next = std::next(it);
    if (next != cache.end()) {
      peek_digest = next->first;
    }
  }

  /* Remove all idle timouts */
  for (const auto &d : to_remove) {
    cache.erase(d);
  }
}

static void process_packet(Tins::PDU& pdu, timeval ts, Exporter& exporter) {
  auto flow_type = std::size_t{0};
  auto flow_digest = std::size_t{0};
  auto flow_values = std::vector<std::byte>{};
  auto flow_fields = prepare_fields();

  for (auto* p = &pdu; p != nullptr; p = p->inner_pdu()) {
    const auto pdu_type = p->pdu_type();
    const auto& reducer = reducers[pdu_type];
    if (!reducer)
      continue;

    if (!reducer->should_process())
      continue;

    /* Reduce PDU into type, digest and values */
    flow_type = combine(flow_type, reducer->type());
    flow_digest = combine(flow_digest, reducer->digest(pdu));
    auto values = reducer->values(*p);
    flow_values.insert(flow_values.end(), values.begin(), values.end());
    auto fields = reducer->fields();
    flow_fields.insert(flow_fields.end(), fields.begin(), fields.end());

    if (pdu_type == Tins::PDU::PDUType::UDP) {
      auto* udp = static_cast<const Tins::UDP*>(p);
      const auto& raw = p->rfind_pdu<Tins::RawPDU>();
      if (udp->dport() == Protocols::VXLAN::VXLAN_PORT) {
        p->inner_pdu(new Protocols::VXLAN(raw.payload().data(), raw.payload_size()));
      }
    }
  }

  if (flow_values.empty())
    return;

  // TODO(dudoslav): Move generation of fields here
  /* If template does not exist generate it */
  if (!exporter.has_template(flow_type)) {
    exporter.insert_template(flow_type, flow_fields);
  }

  auto e = cache.insert(flow_digest, flow_type, std::move(flow_values), ts);
  active_timeout_check(exporter, e, ts);
}

static void reducers_init() {
  register_reducer<IP>(Tins::PDU::PDUType::IP);
  register_reducer<IPV6>(Tins::PDU::PDUType::IPv6);
  register_reducer<TCP>(Tins::PDU::PDUType::TCP);
  register_reducer<UDP>(Tins::PDU::PDUType::UDP);
  register_reducer<VLAN>(Tins::PDU::PDUType::DOT1Q);
  register_reducer<VXLAN>(Protocols::VXLAN_PDU);
}

static void processor_loop(Plugins::Input& input) {
  auto old = std::chrono::high_resolution_clock::now();
  auto exporter = Exporter{Options::ip_address, Options::port};

  std::signal(SIGINT, on_signal);
  while (running) {
    auto res = input.get_packet();
    if (res.type != PACKET)
      break;

    try {
      auto pdu = Tins::EthernetII{res.packet.data, res.packet.caplen};
      auto ts = timeval{res.packet.sec, res.packet.usec};

      process_packet(pdu, ts, exporter);

      /* Time delta calculation */
      auto now = std::chrono::high_resolution_clock::now();
      auto delta = now - old;
      old = now;

      std::size_t peek_interval = std::chrono::duration_cast
        <std::chrono::milliseconds>(delta).count();
      peek_interval = cache.size() / 1000.f * peek_interval;
      peek_interval = std::clamp(peek_interval, 1lu, cache.size());
      idle_timeout_check(exporter, ts, peek_interval);

      exporter.export_all();
    } catch (std::runtime_error& e){
      Log::error("%s\n", e.what());
      return;
    } catch (...) {
      /* This might even catch network exceptions */
      continue;
    }
  }

  /* Flush flow cache */
  for (auto& [_, r] : cache) {
    auto& [props, type, values] = r;

    exporter.insert_record(type, props, values);
  }

  exporter.export_all();
}

/**
 * Starts the packet processing loop. The packets to process
 * are capture by using input plugin.
 * @param input Input plugin to use to get packets to process
 */
void start_processor(Plugins::Input input) {
  reducers_init();
  processor_loop(input);
}

} // namespace Flow
