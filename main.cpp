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

static volatile auto should_close = std::atomic{false};

static Flow::Chain reduce_packet(const Tins::EthernetII& packet) {
  auto chain = Flow::Chain{};
  auto pdu_range = Tins::iterate_pdus(packet);
  // Small optimization if needed
  chain.reserve(std::distance(pdu_range.begin(), pdu_range.end()));

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

static void on_signal([[maybe_unused]] int signal) {
  if (should_close) {
    Log::info("Exiting...\n");
    std::exit(1);
  }
  Log::info("Shuting down. Please wait for cleanup...\n");
  should_close = true;
}

static void start(Plugins::Input input) {
  /* Register interrupt signal to wait for cleanup */
  std::signal(SIGINT, on_signal);

  auto export_interval = std::chrono::seconds{
    Options::export_interval
  };

  /* Initialize all required structures */
  auto serializer = Flow::Serializer{Options::definition};
  auto cache = Flow::Cache{};
  auto exporter = Flow::Exporter{Options::ip_address, Options::port};

  auto peek_digest = 0ul;
  /* Start main application loop */
  Log::info("Starting packet processing loop...\n");
  while (not should_close) {
    /* Get packet from input plugin */
    auto raw = input.get_packet();
    if (raw.type == END_OF_INPUT) {
      break;
    }

    /* Parse packet */
    auto packet = Tins::EthernetII{raw.packet.data,
      static_cast<unsigned int>(raw.packet.caplen)};
    auto chain = reduce_packet(packet);

    if (not chain.empty()) {
      auto& [curr_digest, record] = *cache.insert(
          serializer.digest(chain),
          std::move(chain),
          raw.packet.sec);

      auto& chain = record.second;

      /* Idle timeout check */
      auto it = cache.find(peek_digest);
      auto to_remove = std::vector<std::size_t>{};
      for (std::size_t i = 0; i < 5; ++i, ++it) {
        if (cache.begin() == cache.end()) {
          break;
        }

        if (it == cache.end()) {
          it = cache.begin();
        }

        auto& [i_props, i_chain] = it->second;
        auto i_type = Flow::type(i_chain);
        /* Check timestamps */
        auto time_diff = raw.packet.sec - i_props.last_timestamp;
        if (time_diff > Options::idle_timeout) {
          Log::debug("Idle timeout with error %lus\n", time_diff - Options::idle_timeout);
          if (not exporter.has_template(i_type)) {
            exporter.insert_template(i_type,
                serializer.fields(i_chain, i_props));
          }
          exporter.insert_record(i_type, serializer.values(i_chain, i_props));
          to_remove.emplace_back(it->first);
        }
      }

      if (it != cache.end()) {
        auto next = std::next(it);
        if (next != cache.end()) {
          peek_digest = next->first;
        }
      }

      for (const auto& d: to_remove) {
        cache.erase(d);
      }

      auto type = Flow::type(chain);
      auto& props = record.first;
      auto time_diff = raw.packet.sec - props.first_timestamp;
      /* Active timeout check */
      if (time_diff > Options::active_timeout) {
        Log::debug("Active timeout with error %lus\n", time_diff - Options::active_timeout);
        if (not exporter.has_template(type)) {
          exporter.insert_template(type,
              serializer.fields(chain, props));
        }
        exporter.insert_record(type, serializer.values(chain, props));
        props = {0, raw.packet.sec, raw.packet.sec};
      }
    }
  }

  /* When exiting the application export all cached records */
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
