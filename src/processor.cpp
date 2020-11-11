#include <processor.hpp>

#include <atomic>
#include <csignal>
#include <thread>

#include <tins/tins.h>

#include <options.hpp>
#include <parser.hpp>
#include <reducer.hpp>
#include <manager.hpp>
#include <buffer.hpp>
#include <ipfix.hpp>
#include <log.hpp>
#include <queue.hpp>

/* Parsers */
#include <protocols/gre.hpp>
#include <protocols/vxlan.hpp>

/* Reducers */
#include <flows/ip.hpp>
#include <flows/ipv6.hpp>
#include <flows/tcp.hpp>
#include <flows/udp.hpp>
#include <flows/vlan.hpp>
#include <flows/vxlan.hpp>
#include <flows/gre.hpp>
#include <flows/mpls.hpp>
#include <flows/ethernet.hpp>

namespace Flow {

static std::atomic<bool> running = true;

static void
on_signal(int)
{
  if (!running) {
    Log::info("Exiting...\n");
    std::exit(10);
  }

  Log::info("Shuting down. Please wait for cleanup...\n");
  running = false;
}

static Buffer
sub_template_multi_list()
{
  auto result = Buffer{};

  result.push_back_any<std::uint8_t>(0);
  result.push_back_any<std::uint8_t>(IPFIX::SEMANTIC_ORDERED);

  return result;
}

/* Processor */
Processor::Processor()
  : _exporter(Options::options().ip_address, Options::options().port)
{
  const auto& config = Options::config();

  /* Register additional parsers */
  Parser::register_tins_parser<Tins::IP, Protocols::GREPDU>(
      IPFIX::PROTOCOL_GRE);
  Parser::register_udp_parser<Protocols::VXLANPDU>(
      Protocols::VXLANPDU::VXLAN_PORT);

  /* Register reducers */
  Reducer::register_reducer<IP>(Tins::PDU::PDUType::IP, config);
  Reducer::register_reducer<IPV6>(Tins::PDU::PDUType::IPv6, config);
  Reducer::register_reducer<TCP>(Tins::PDU::PDUType::TCP, config);
  Reducer::register_reducer<UDP>(Tins::PDU::PDUType::UDP, config);
  Reducer::register_reducer<VLAN>(Tins::PDU::PDUType::DOT1Q, config);
  Reducer::register_reducer<MPLS>(Tins::PDU::PDUType::MPLS, config);
  Reducer::register_reducer<ETHERNET>(Tins::PDU::PDUType::ETHERNET_II, config);
  Reducer::register_reducer<GRE>(Protocols::GREPDU_TYPE, config);
  Reducer::register_reducer<VXLAN>(Protocols::VXLANPDU_TYPE, config);

  std::signal(SIGINT, on_signal);
}

void
Processor::process(Tins::PDU* pdu, timeval timestamp)
{
  auto flow_digest = std::size_t{0};

  /* Generate digest */
  for (auto* p = pdu; p != nullptr; p = p->inner_pdu()) {
    const auto* reducer = Reducer::reducer(p->pdu_type());
    if (reducer == nullptr)
      continue;

    if (!reducer->should_process())
      continue;

    flow_digest = combine(flow_digest, reducer->digest(*p));
  }

  /* If the digest is already in cache */
  auto search = _cache.find(flow_digest);
  if (search != _cache.end()) {
    _cache.update_record(search, timestamp);
    return;
  }

  auto flow_values = sub_template_multi_list();

  /* Generate values */
  for (auto* p = pdu; p != nullptr; p = p->inner_pdu()) {
    const auto* reducer = Reducer::reducer(p->pdu_type());
    if (reducer == nullptr)
      continue;

    if (!reducer->should_process())
      continue;

    auto type = reducer->type();
    auto tid = _exporter.get_template_id(type);
    if (tid == 0) {
      tid = _exporter.insert_template(type, reducer->fields());
    }

    auto values = reducer->values(*p);
    flow_values.push_back_any<std::uint16_t>(htons(tid));
    flow_values.push_back_any<std::uint16_t>(htons(values.size() + 4));
    flow_values.insert(flow_values.end(), values.begin(), values.end());
  }

  /* Check if record isn't empty */
  if (flow_values.size() <= 2)
    return;

  flow_values.set_any_at<std::uint8_t>(0, flow_values.size() - 1);

  auto it = _cache.insert_record(flow_digest, timestamp, flow_values);
  check_active_timeout(timestamp, it->second);
}

void
Processor::check_active_timeout(timeval now, CacheEntry& entry)
{
  const auto active_timeout = Options::options().active_timeout;
  const auto diff = now.tv_sec - entry.props.flow_start.tv_sec;

  if (diff < active_timeout)
    return;

  Log::debug("Active timeout with error %u\n", diff - active_timeout);

  // TODO(dudoslav): Should we reset counter to 0 or 1?
  _exporter.insert_record(entry.props, IPFIX::REASON_ACTIVE, entry.values);
  entry.props = {0, now, now};
}

void
Processor::check_idle_timeout(timeval now, std::size_t peek_size)
{
  const auto idle_timeout = Options::options().idle_timeout;

  if (_cache.empty())
    return;

  for (std::size_t i = 0; i < peek_size; ++i) {
    if (_peek_iterator == _cache.end())
      _peek_iterator = _cache.begin();

    const auto& entry = _peek_iterator->second;
    auto diff = now.tv_sec - entry.props.flow_end.tv_sec;

    if (diff < idle_timeout) {
      ++_peek_iterator;
      continue;
    }

    Log::debug("Idle timeout %u with error %u\n",
        _peek_iterator->first,
        diff - idle_timeout);

    _exporter.insert_record(entry.props, IPFIX::REASON_IDLE, entry.values);
    _peek_iterator = _cache.erase(_peek_iterator);
  }
}

static void
capture_worker(Async::Queue<Tins::Packet>& queue)
{
  auto input = Plugins::create_input(Options::options().input_plugin,
      Options::options().argument.c_str());

  while (running) {
    auto result = input.get_packet();

    if (result.type != CAPTURE_PACKET) {
      running = false;
      break;
    }

    auto pdu = Parser::parse(result.packet.data, result.packet.caplen);
    auto timestamp = timeval{result.packet.sec, result.packet.usec};

    if (pdu == nullptr)
      continue;

    queue.push(Tins::Packet{std::move(*pdu), timestamp});
  }
}

static timeval
timestamp_to_timeval(const Tins::Timestamp& timestamp)
{
  return {
    timestamp.seconds(), timestamp.microseconds()
  };
}

void
Processor::start()
{
  using namespace std::chrono;
  
  _time_point = high_resolution_clock::now();
  auto queue = Async::Queue<Tins::Packet>{};

  running = true;

  /* Start packet capture and parsing thread */
  auto capture_thread = std::thread(capture_worker, std::ref(queue));

  /* Start packet reducing loop */
  try {
    while (running) {
      if (!queue.wait_for(seconds(1))) {
        check_idle_timeout(
            timestamp_to_timeval(Tins::Timestamp::current_time()), _cache.size());
        _time_point = high_resolution_clock::now();
        continue;
      }

      /* Get packet from parsed packets queue */
      auto packet = queue.pop();

      process(packet.pdu(), timestamp_to_timeval(packet.timestamp()));

      /* Calculate time delta */
      auto now = high_resolution_clock::now();
      auto delta = duration<double, std::milli>(now - _time_point).count();
      _time_point = now;

      /* Perform idle check. Check the whole cache each second */
      check_idle_timeout(
          timestamp_to_timeval(packet.timestamp()),
          (delta / 1000.f) * _cache.size());
    }

    /* Flush cache */
    for (auto& [_, entry] : _cache) {
      _exporter.insert_record(entry.props, IPFIX::REASON_FORCED, entry.values);
    }

    /* Flush exporter */
    _exporter.flush();
  } catch (const std::exception& e) {
    /* On exception, the connection may have closed */
    Log::error("%s\n", e.what());
    running = false;
  }

  /* Join the thread that handles packet capture */
  capture_thread.join();
}

} // namespace Flow
