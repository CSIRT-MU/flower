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
#include <buffer.hpp>
#include <parser.hpp>
#include <reducer.hpp>

#include <protocols/vxlan.hpp>

#include <flows/ip.hpp>

namespace Flow {

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

static Buffer
prepare_values() {
  auto result = Buffer{};

  result.push_back_any<std::uint8_t>(255);
  result.push_back_any<std::uint16_t>(0);
  result.push_back_any<std::uint8_t>(IPFIX::SEMANTIC_ORDERED);

  return result;
}

/**
 * Performs active timout check on record in timestamp given in arguments.
 * Active timeout is set in global options. If record timed out, it is exported
 * and then its counter is reset.
 */
// static void active_timeout_check(Exporter &exporter, Cache::Entry &entry,
//                                  timeval ts) {
//   const auto active_timeout = Options::active_timeout;
// 
//   auto& [props, type, values] = entry->second;
//   auto diff = ts.tv_sec - props.flow_start.tv_sec;
// 
//   if (diff >= active_timeout) {
//     Log::debug("Active timeout with error %lus\n", diff - active_timeout);
// 
//     exporter.insert_record(type, props, values);
//     props = {0, ts, ts};
//   }
// }
// 
// static void idle_timeout_check(Exporter &exporter, timeval ts,
//                                std::size_t peek_interval) {
//   static std::size_t peek_digest = 0;
// 
//   const auto idle_timeout = Options::idle_timeout;
// 
//   auto to_remove = std::vector<std::size_t>{};
//   auto it = cache.find(peek_digest);
// 
//   Log::debug("Checking %lu records for idle timeout\n", peek_interval);
//   for (std::size_t i = 0; i < peek_interval; ++i, ++it) {
//     /* If cache is empty */
//     if (cache.begin() == cache.end())
//       break;
// 
//     if (it == cache.end())
//       it = cache.begin();
// 
//     auto& [props, type, values] = it->second;
// 
//     auto diff = ts.tv_sec - props.flow_end.tv_sec;
//     if (diff >= idle_timeout) {
//       Log::debug("Idle timeout with error %lus\n", diff - idle_timeout);
// 
//       exporter.insert_record(type, props, values);
//       to_remove.emplace_back(it->first);
//     }
//   }
// 
//   /* Check if next exists and set it as peek digest */
//   if (it != cache.end()) {
//     auto next = std::next(it);
//     if (next != cache.end()) {
//       peek_digest = next->first;
//     }
//   }
// 
//   /* Remove all idle timouts */
//   for (const auto &d : to_remove) {
//     cache.erase(d);
//   }
// }

static void process_packet(Tins::PDU& pdu, timeval ts, Exporter& exporter) {
  auto flow_digest = std::size_t{0};
  auto flow_values = prepare_values();

  for (auto* p = &pdu; p != nullptr; p = p->inner_pdu()) {
    const auto pdu_type = p->pdu_type();
    const auto& reducer = Reducer::reducer(pdu_type);
    if (!reducer)
      continue;

    if (!reducer->should_process())
      continue;

    auto flow_type = reducer->type();
    flow_digest = combine(flow_digest, reducer->digest(pdu));

    /* Check if template exists */
    auto template_id = exporter.get_template_id(flow_type);
    if (template_id == 0) {
      template_id = exporter.insert_template(flow_type, reducer->fields());
    }

    auto values = reducer->values(*p);
    flow_values.push_back_any<std::uint16_t>(htons(template_id));
    flow_values.push_back_any<std::uint16_t>(htons(values.size() + 4));
    flow_values.insert(flow_values.end(), values.begin(), values.end());
  }

  flow_values.set_any_at<std::uint16_t>(1, htons(flow_values.size() - 3));

  if (flow_values.size() <= 4)
    return;

  exporter.insert_record({1, ts, ts}, IPFIX::REASON_FORCED, flow_values);
  exporter.flush();
}

static void reducers_init() {
  Reducer::register_reducer<IP>(Tins::PDU::PDUType::IP, Options::config());
  // Reducer::register_reducer<IPV6>(Tins::PDU::PDUType::IPv6, Options::config());
  // Reducer::register_reducer<TCP>(Tins::PDU::PDUType::TCP, Options::config());
  // Reducer::register_reducer<UDP>(Tins::PDU::PDUType::UDP, Options::config());
  // Reducer::register_reducer<VLAN>(Tins::PDU::PDUType::DOT1Q, Options::config());
  // Reducer::register_reducer<VXLAN>(Protocols::VXLAN_PDU, Options::config());

  Parser::register_udp_parser<Protocols::VXLAN>(Protocols::VXLAN::VXLAN_PORT);
}

static void processor_loop(Plugins::Input& input) {
  const auto& options = Options::options();

  auto old = std::chrono::high_resolution_clock::now();
  auto exporter = Exporter{options.ip_address, options.port};

  std::signal(SIGINT, on_signal);
  while (running) {
    auto res = input.get_packet();
    if (res.type != PACKET)
      break;

    try {
      auto pdu = Parser::parse(res.packet.data, res.packet.caplen);
      auto ts = timeval{res.packet.sec, res.packet.usec};

      process_packet(*pdu, ts, exporter);

      /* Time delta calculation */
      auto now = std::chrono::high_resolution_clock::now();
      auto delta = now - old;
      old = now;

      std::size_t peek_interval = std::chrono::duration_cast
        <std::chrono::milliseconds>(delta).count();
      peek_interval = cache.size() / 1000.f * peek_interval;
      peek_interval = std::clamp(peek_interval, 1lu, cache.size());
      // idle_timeout_check(exporter, ts, peek_interval);

      // exporter.export_all();
    } catch (std::runtime_error& e){
      Log::error("%s\n", e.what());
      return;
    } catch (...) {
      /* This might even catch network exceptions */
      continue;
    }
  }

  /* Flush flow cache */
  // for (auto& [_, r] : cache) {
  //   auto& [props, type, values] = r;

  //   exporter.insert_record(type, props, values);
  // }

  // exporter.export_all();
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
