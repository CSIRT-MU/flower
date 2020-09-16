#include <cstdlib>
#include <iostream>

#include <tins/tins.h>

#include <async.hpp>
#include <flow.hpp>
#include <network.hpp>
#include <ipfix.hpp>
#include <manager.hpp>
#include <options.hpp>
#include <serializer.hpp>

static Flow::Record reduce_packet(const Tins::EthernetII& packet) {
  auto record = Flow::Record{};
  auto pdu_range = Tins::iterate_pdus(packet);
  record.reserve(std::distance(pdu_range.begin(), pdu_range.end()));

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

static void start(Plugins::Input input) {
  auto export_interval = std::chrono::seconds{
    Options::export_interval
  };

  std::cout << "Connecting to: " << Options::ip_address << '\n';
  std::cout << "with port: " << Options::port << '\n';

  auto serializer = Flow::Serializer{Options::definition};
  auto conn = Net::Connection::tcp(Options::ip_address,
      Options::port);
  auto exporter = IPFIX::Exporter{};
  auto cache = Flow::Cache{};

  auto timer = Async::Timer{export_interval};
  timer.start([&](){
      cache.erase_if([&](const auto& p){
          auto type = Flow::type(p.second.second);
          if (!exporter.has_template(type)) {
            exporter.insert_template(type, serializer.fields(p.second.second,
                  p.second.first));
          }
          exporter.insert_record(type, serializer.values(p.second.second,
                p.second.first));
          return true;
          });

      exporter.for_each_buffer([&conn](const auto& buffer){
          std::printf("Buffer size: %lu\n", buffer.size());
          conn.write(buffer.data(), buffer.size());
          });

      exporter.clear();
      });

  for (;;) {
    auto raw = input.get_packet();
    if (raw.data == nullptr) {
      break;
    }

    auto packet = Tins::EthernetII{raw.data, static_cast<unsigned int>(raw.caplen)};
    // TODO(dudoslav): Add concurrency after packet was parsed!
    auto record = reduce_packet(packet);

    if (!record.empty()) {
      cache.insert(serializer.digest(record), std::move(record), raw.timestamp);
    }
  }
}

int main(int argc, char** argv) {
  const auto* home = std::getenv("HOME");
  try {
    auto config_path = std::string{home} + "/.flower.conf";
    if (std::filesystem::is_regular_file(config_path)) {
      Options::load_file(std::string{home} + "/.flower.conf");
    }

    if (std::filesystem::is_regular_file("./flower.conf")) {
      Options::load_file("./flower.conf");
    }
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Error loading options file\n%s\n", e.what());
    return 1;
  }

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
