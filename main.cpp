#include <csignal>
#include <cstdlib>
#include <iostream>

#include <tins/tins.h>

#include <async.hpp>
#include <exporter.hpp>
#include <cache.hpp>
#include <manager.hpp>
#include <options.hpp>
#include <serializer.hpp>

constexpr static auto CONFIG_NAME = "flower.conf";
constexpr static auto CACHE_INTERVAL = 20;

static volatile auto should_close = std::atomic{false};

static Flow::Chain reduce_packet(const Tins::EthernetII& packet) {
  auto record = Flow::Chain{};
  auto pdu_range = Tins::iterate_pdus(packet);
  // Small optimization if needed
  // record.reserve(std::distance(pdu_range.begin(), pdu_range.end()));

  try {
    for (const auto& pdu: pdu_range) {
      if (pdu.pdu_type() == Tins::PDU::PDUType::IP) {
        if (!Options::definition.ip.process)
          continue;
        const auto& ip = dynamic_cast<const Tins::IP&>(pdu);
        record.emplace_back(Flow::IP{ip.src_addr(), ip.dst_addr()});
      } else if (pdu.pdu_type() == Tins::PDU::PDUType::TCP) {
        if (!Options::definition.tcp.process)
          continue;
        const auto& tcp = dynamic_cast<const Tins::TCP&>(pdu);
        record.emplace_back(Flow::TCP{tcp.sport(), tcp.dport()});
      } else if (pdu.pdu_type() == Tins::PDU::PDUType::UDP) {
        if (!Options::definition.udp.process)
          continue;
        const auto& udp = dynamic_cast<const Tins::UDP&>(pdu);
        record.emplace_back(Flow::UDP{udp.sport(), udp.dport()});
      } else if (pdu.pdu_type() == Tins::PDU::PDUType::DOT1Q) {
        if (!Options::definition.dot1q.process)
          continue;
        const auto& dot1q = dynamic_cast<const Tins::Dot1Q&>(pdu);
        record.emplace_back(Flow::DOT1Q{dot1q.id()});
      }
    }
  } catch (const std::bad_cast& e) {
    std::cerr << e.what() << std::endl;
  }

  return record;
}

static void on_signal([[maybe_unused]] int signal) {
  if (should_close) {
    std::printf("Exiting...\n");
    std::exit(1);
  }
  std::printf("Shuting down...\n");
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

  auto timer = Async::Timer{export_interval};
  timer.start([&](){
      // cache.for_each([&](auto type, auto& props, auto& record){
      //     auto now = static_cast<unsigned int>(std::time(nullptr));
      //     auto active = now - props.first_timestamp;
      //     if (active > Options::export_interval) {
      //     std::printf("Active timeout with error: %u\n", active - Options::export_interval);
      //     if (!exporter.has_template(type)) {
      //     exporter.insert_template(type, serializer.fields(record, props));
      //     }
      //     exporter.insert_record(type, serializer.values(record, props));
      //     props = {0, now, now};
      //     }
      //     });
      });

  while (not should_close) {
    auto raw = input.get_packet();
    if (raw.data == nullptr) {
      break;
    }

    auto packet = Tins::EthernetII{raw.data,
      static_cast<unsigned int>(raw.caplen)};
    auto record = reduce_packet(packet);

    if (!record.empty()) {
      auto type = Flow::type(record);
      auto [b, e] = cache.insert(
          type,
          serializer.digest(record),
          std::move(record),
          raw.timestamp);

      for (auto [it, c] = std::make_pair(b, 0);
          it != e && c != CACHE_INTERVAL;
          ++it, ++c) {
        auto& [props, record] = it->second;
        if (props.count == 0) {
          continue;
        }

        auto now = static_cast<unsigned int>(std::time(nullptr));
        auto active = now - props.first_timestamp;
        auto idle = now - props.last_timestamp;
        if (active > Options::export_interval) {
          std::printf("Active timeout with error: %u\n", active - Options::export_interval);
          if (!exporter.has_template(type)) {
            exporter.insert_template(type, serializer.fields(record, props));
          }
          exporter.insert_record(type, serializer.values(record, props));
          props = {0, now, now};
        }
        if (idle > Options::export_interval) {
          std::printf("Idle timeout with error: %u\n", idle - Options::export_interval);
          if (!exporter.has_template(type)) {
            exporter.insert_template(type, serializer.fields(record, props));
          }
          exporter.insert_record(type, serializer.values(record, props));
          props = {0, now, now};
        }
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
        std::fprintf(stderr, "Error: %s\n", e.what());
        return 2;
      }
      break;
    default:
      return 0;
  }

  return 0;
}
