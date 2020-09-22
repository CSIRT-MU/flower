#include <csignal>
#include <cstdlib>
#include <atomic>

#include <tins/tins.h>

#include <exporter.hpp>
#include <cache.hpp>
#include <manager.hpp>
#include <options.hpp>
#include <serializer.hpp>
#include <log.hpp>

constexpr static auto CONFIG_NAME = "flower.conf";
constexpr static auto CACHE_INTERVAL_MIN = 5ul;
constexpr static auto CACHE_INTERVAL_COEF = 20;

static volatile auto should_close = std::atomic{false};

static Flow::Chain reduce_packet(const Tins::EthernetII& packet) {
  auto chain = Flow::Chain{};
  auto pdu_range = Tins::iterate_pdus(packet);
  // Small optimization if needed
  // chain.reserve(std::distance(pdu_range.begin(), pdu_range.end()));

  for (const auto& pdu: pdu_range) {
    if (pdu.pdu_type() == Tins::PDU::PDUType::IP) {
      if (!Options::definition.ip.process)
        continue;
      const auto& ip = static_cast<const Tins::IP&>(pdu);
      chain.emplace_back(Flow::IP{ip.src_addr(), ip.dst_addr()});
    } else if (pdu.pdu_type() == Tins::PDU::PDUType::TCP) {
      if (!Options::definition.tcp.process)
        continue;
      const auto& tcp = static_cast<const Tins::TCP&>(pdu);
      chain.emplace_back(Flow::TCP{tcp.sport(), tcp.dport()});
    } else if (pdu.pdu_type() == Tins::PDU::PDUType::UDP) {
      if (!Options::definition.udp.process)
        continue;
      const auto& udp = static_cast<const Tins::UDP&>(pdu);
      chain.emplace_back(Flow::UDP{udp.sport(), udp.dport()});
    } else if (pdu.pdu_type() == Tins::PDU::PDUType::DOT1Q) {
      if (!Options::definition.dot1q.process)
        continue;
      const auto& dot1q = static_cast<const Tins::Dot1Q&>(pdu);
      chain.emplace_back(Flow::DOT1Q{dot1q.id()});
    }
  }

  return chain;
}

static void export_if_timeout(
    std::size_t type,
    Flow::Properties& props,
    Flow::Chain& chain,
    Flow::Exporter& exporter,
    Flow::Serializer& serializer) {
  if (props.count == 0) {
    return;
  }

  auto now = static_cast<unsigned int>(std::time(nullptr));
  auto active = now - props.first_timestamp;
  auto idle = now - props.last_timestamp;

  if (active > Options::active_timeout) {
    Log::debug("Active timeout with error: %us\n",
        active - Options::active_timeout);
    if (!exporter.has_template(type)) {
      exporter.insert_template(type, serializer.fields(chain, props));
    }
    exporter.insert_record(type, serializer.values(chain, props));
    props = {0, now, now};
  } else if (idle > Options::idle_timeout) {
    Log::debug("Idle timeout with error: %us\n",
        idle - Options::idle_timeout);
    if (!exporter.has_template(type)) {
      exporter.insert_template(type, serializer.fields(chain, props));
    }
    exporter.insert_record(type, serializer.values(chain, props));
    props = {0, now, now};
  }
}

static void on_signal([[maybe_unused]] int signal) {
  if (should_close) {
    Log::info("Exiting...\n");
    std::exit(1);
  }
  Log::info("Shuting down...\n");
  should_close = true;
}

static void start(Plugins::Input input) {
  std::signal(SIGINT, on_signal);

  auto export_interval = std::chrono::seconds{
    Options::export_interval
  };

  auto serializer = Flow::Serializer{Options::definition};
  auto cache = Flow::Cache{};
  auto exporter = Flow::Exporter{Options::ip_address, Options::port};

  Log::info("Starting packet processing loop...\n");
  while (not should_close) {
    auto raw = input.get_packet();
    if (raw.data == nullptr) {
      break;
    }

    auto packet = Tins::EthernetII{raw.data,
      static_cast<unsigned int>(raw.caplen)};
    auto chain = reduce_packet(packet);

    if (!chain.empty()) {
      auto type = Flow::type(chain);
      auto [b, r, e] = cache.insert(
          type,
          serializer.digest(chain),
          std::move(chain),
          raw.timestamp);

      export_if_timeout(type, r->second.first, r->second.second, exporter, serializer);

      auto s = cache.records_size(type);
      auto interval = std::max(s / CACHE_INTERVAL_COEF, CACHE_INTERVAL_MIN);
      Log::debug("Peaking into %lu records\n", interval);
      for (auto [it, c] = std::make_pair(std::next(r), 0ul);
          it != r && c != interval;
          ++it, ++c) {

        if (it == e) {
          it = b;
        }

        auto& [props, chain] = it->second;
        export_if_timeout(type, props, chain, exporter, serializer);
      }
    }
  }

  exporter.export_all();
}

void load_config() {
  namespace fs = std::filesystem;

  const auto* home = std::getenv("HOME");
  auto home_config = std::string{home} + "/." + CONFIG_NAME;
  try {
    if (fs::is_regular_file(home_config)) {
      Options::load_file(home_config);
    }
  } catch (const std::exception&) {}

  try {
    if (fs::is_regular_file(CONFIG_NAME)) {
      Options::load_file(CONFIG_NAME);
    }
  } catch (const std::exception&) {}
}

int main(int argc, char** argv) {
  load_config();
  Options::parse_args(argc, argv);

  auto plugin_manager = Plugins::Manager{"plugins"};

  switch (Options::mode) {
    case Options::Mode::PRINT_PLUGINS:
      std::printf("Input plugins:\n");
      for (const auto& p: plugin_manager.inputs()) {
        std::printf("\t%s\n", p.info().name);
      }
      break;
    case Options::Mode::PROCESS:
      try {
        auto input = plugin_manager.create_input(
            Options::input_plugin, Options::argument.c_str());
        start(std::move(input));
      } catch (const std::exception& e) {
        Log::error("Error: %s\n", e.what());
        return 2;
      }
      break;
    default:
      return 0;
  }

  return 0;
}
