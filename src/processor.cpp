#include <processor.hpp>

#include <atomic>
#include <chrono>
#include <csignal>

#include <tins/tins.h>

#include <cache.hpp>
#include <exporter.hpp>
#include <log.hpp>
#include <options.hpp>
#include <serializer.hpp>

namespace Flow {

static Cache cache;
static Serializer serializer;

static std::atomic<bool> running = true;

static void on_signal(int) {
  if (!running) {
    Log::info("Exiting...\n");
    std::exit(10);
  }

  Log::info("Shuting down. Please wait for cleanup...\n");
  running = false;
}

static Flow::Chain reduce_packet(const Tins::EthernetII &packet) {
  auto chain = Flow::Chain{};
  auto pdu_range = Tins::iterate_pdus(packet);
  // Small optimization if needed
  chain.reserve(std::distance(pdu_range.begin(), pdu_range.end()));

  for (const auto &pdu : pdu_range) {
    switch (pdu.pdu_type()) {
    /* IPv4 */
    case Tins::PDU::PDUType::IP: {
      if (!Options::definition.ip.process)
        continue;
      const auto &ip = static_cast<const Tins::IP &>(pdu);
      chain.emplace_back(Flow::IP{ip.src_addr(), ip.dst_addr()});
    } break;

    /* IPv6 */
    case Tins::PDU::PDUType::IPv6: {
      if (!Options::definition.ipv6.process)
        continue;
      const auto &ipv6 = static_cast<const Tins::IPv6 &>(pdu);
      auto protocol = Flow::IPv6{};
      ipv6.src_addr().copy(reinterpret_cast<std::array<unsigned char, 16>::iterator>(protocol.src.begin()));
      ipv6.dst_addr().copy(reinterpret_cast<std::array<unsigned char, 16>::iterator>(protocol.dst.begin()));
      chain.emplace_back(std::move(protocol));
    } break;

    /* TCP */
    case Tins::PDU::PDUType::TCP: {
      if (!Options::definition.tcp.process)
        continue;
      const auto &tcp = static_cast<const Tins::TCP &>(pdu);
      chain.emplace_back(Flow::TCP{tcp.sport(), tcp.dport()});
    } break;

    /* UDP */
    case Tins::PDU::PDUType::UDP: {
      if (!Options::definition.udp.process)
        continue;
      const auto &udp = static_cast<const Tins::UDP &>(pdu);
      chain.emplace_back(Flow::UDP{udp.sport(), udp.dport()});
    } break;

    /* DOT1Q / VLAN */
    case Tins::PDU::PDUType::DOT1Q: {
      if (!Options::definition.dot1q.process)
        continue;
      const auto &dot1q = static_cast<const Tins::Dot1Q &>(pdu);
      chain.emplace_back(Flow::DOT1Q{dot1q.id()});
    } break;

    /* MPLS */
    case Tins::PDU::PDUType::MPLS: {
      if (!Options::definition.mpls.process)
        continue;
      const auto &mpls = static_cast<const Tins::MPLS &>(pdu);
      chain.emplace_back(Flow::MPLS{mpls.label()});
    } break;
    }
  }

  return chain;
}

/**
 * Performs active timout check on record in timestamp given in arguments.
 * Active timeout is set in global options. If record timed out, it is exported
 * and then its counter is reset.
 */
static void active_timeout_check(Exporter &exporter, Record &record,
                                 timeval ts) {
  const auto active_timeout = Options::active_timeout;

  auto &[props, chain] = record;
  auto diff = ts.tv_sec - props.first_timestamp.tv_sec;
  auto type = Flow::type(chain);

  if (diff >= active_timeout) {
    Log::debug("Active timeout with error %lus\n", diff - active_timeout);

    if (!exporter.has_template(type)) {
      exporter.insert_template(type, serializer.fields(chain, props));
    }
    exporter.insert_record(type, serializer.values(chain, props));
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

    auto &[props, chain] = it->second;
    auto type = Flow::type(chain);

    auto diff = ts.tv_sec - props.last_timestamp.tv_sec;
    if (diff >= idle_timeout) {
      Log::debug("Idle timeout with error %lus\n", diff - idle_timeout);

      if (!exporter.has_template(type)) {
        exporter.insert_template(type, serializer.fields(chain, props));
      }
      exporter.insert_record(type, serializer.values(chain, props));
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

static void processor_loop(Plugins::Input input, Exporter &exporter) {
  auto old = std::chrono::high_resolution_clock::now();

  while (running) {
    auto input_result = input.get_packet();

    switch (input_result.type) {
    case END_OF_INPUT:
      Log::debug("End of input\n");
      return;
    case TIMEOUT:
      Log::debug("Packet capture timeout\n");
      idle_timeout_check(exporter, {std::time(nullptr), 0}, cache.size());
      continue;
    case PACKET:
      Log::debug("Processing packet, cache size %lu\n", cache.size());
    }

    /* Calculate time difference between last packet and now */
    auto now = std::chrono::high_resolution_clock::now();
    auto delta = now - old;
    old = now;

    auto &raw = input_result.packet;
    auto packet = Tins::EthernetII{};
    try {
      packet = Tins::EthernetII{raw.data, raw.caplen};
    } catch (const std::exception& e) {
      Log::error("Failed to parse packet: %s\n", e.what());
      continue;
    }
    auto chain = reduce_packet(packet);

    auto raw_ts = timeval{raw.sec, raw.usec};

    if (!chain.empty()) {
      auto &[_, record] =
          *cache.insert(serializer.digest(chain), std::move(chain), raw_ts);

      active_timeout_check(exporter, record, raw_ts);
    }

    std::size_t peek_interval =
      std::chrono::duration_cast<std::chrono::milliseconds>(delta).count();
    peek_interval = cache.size() / 1000.f * peek_interval;
    peek_interval = std::clamp(peek_interval, 1lu, cache.size());
    idle_timeout_check(exporter, raw_ts, peek_interval);
  }
}

/**
 * Starts the packet processing loop. The packets to process
 * are capture by using input plugin.
 * @param input Input plugin to use to get packets to process
 */
void start_processor(Plugins::Input input) {
  auto ip_address = Options::ip_address;
  auto port = Options::port;
  auto definition = Options::definition;

  auto exporter = Exporter{ip_address, port};
  serializer.set_definition(definition);

  std::signal(SIGINT, on_signal);

  /* Start main metering process */
  processor_loop(std::move(input), exporter);

  /* Flush cache */
  for (const auto &[_, r] : cache) {
    auto &[props, chain] = r;
    auto type = Flow::type(chain);

    if (!exporter.has_template(type)) {
      exporter.insert_template(type, serializer.fields(chain, props));
    }
    exporter.insert_record(type, serializer.values(chain, props));
  }

  /* Flush exporter */
  exporter.export_all();
}

} // namespace Flow
