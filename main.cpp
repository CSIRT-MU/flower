#include <cstdlib>
#include <iostream>

#include <tins/tins.h>

#include <async.hpp>
#include <flow.hpp>
#include <ipfix.hpp>
#include <manager.hpp>
#include <options.hpp>

static void print_record(const Flow::Record& record) {
  for (const auto& protocol: record) {
    std::visit([](const auto& e){
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, Flow::IP>) {
        std::cout << "IP " << e.src << " " << e.dst << " -> ";
        } else if constexpr (std::is_same_v<T, Flow::TCP>) {
        std::cout << "TCP " << e.src << " " << e.dst << " -> ";
        } else if constexpr (std::is_same_v<T, Flow::UDP>) {
        std::cout << "UDP " << e.src << " " << e.dst << " -> ";
        } else if constexpr (std::is_same_v<T, Flow::DOT1Q>) {
        std::cout << "DOT1Q " << e.id << " -> ";
        }
        }, protocol);
  }
  std::cout << "END" << std::endl;
}

static Flow::Record reduce_packet(const Tins::EthernetII& packet) {
  auto record = Flow::Record{};
  auto pdu_range = Tins::iterate_pdus(packet);
  record.reserve(std::distance(pdu_range.begin(), pdu_range.end()));

  try {
    for (const auto& pdu: pdu_range) {
      if (pdu.pdu_type() == Tins::PDU::PDUType::IP) {
        const auto& ip = dynamic_cast<const Tins::IP&>(pdu);
        record.emplace_back(Flow::IP{ip.src_addr(), ip.dst_addr()});
      } else if (pdu.pdu_type() == Tins::PDU::PDUType::TCP) {
        const auto& tcp = dynamic_cast<const Tins::TCP&>(pdu);
        record.emplace_back(Flow::TCP{tcp.sport(), tcp.dport()});
      } else if (pdu.pdu_type() == Tins::PDU::PDUType::UDP) {
        const auto& udp = dynamic_cast<const Tins::UDP&>(pdu);
        record.emplace_back(Flow::UDP{udp.sport(), udp.dport()});
      } else if (pdu.pdu_type() == Tins::PDU::PDUType::DOT1Q) {
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

  std::cout << "Connecting to: " << Options::output_ip_address << '\n';
  std::cout << "with port: " << Options::output_port << '\n';

  auto conn = Net::Connection::tcp(Options::output_ip_address,
      Options::output_port);
  auto exporter = IPFIX::Exporter{};
  auto cache = Flow::Cache{};

  auto timer = Async::Timer{export_interval};
  timer.start([&cache, &exporter, &conn](){
      std::cout << "EXPORTING..." << std::endl;
      cache.erase_if([&exporter](const auto& p){
          auto& props = p.second.first;
          std::cout << "COUNT: " << props.count;
          std::cout << " FIRST: " << props.first_timestamp;
          std::cout << " LAST: " << props.last_timestamp << std::endl;
          print_record(p.second.second);
          exporter.insert(p.second);
          return true;
          });

      exporter.for_each_buffer([&conn](const auto& buffer){
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
      cache.insert(std::move(record), raw.timestamp);
    }
  }
}

int main(int argc, char** argv) {
  const auto* home = std::getenv("HOME");
  try {
    if (home != nullptr) {
      Options::load_file(std::string{home} + "/.flower.conf");
    }
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Error loading options file\n%s\n", e.what());
    return 1;
  }

  Options::parse(argc, argv);

  auto plugin_manager = Plugins::Manager{"plugins"};

  switch (Options::mode) {
    case Options::Mode::PRINT_PLUGINS:
      std::printf("Input plugins:\n");
      for (const auto& p: plugin_manager.inputs()) {
        std::printf("\t%s\n", p.info().name);
      }
      break;
    case Options::Mode::PRINT_CONFIG:
      std::printf("input_plugin: %s\n", Options::input_plugin.c_str());
      std::printf("export_interval: %u\n", Options::export_interval);
      std::printf("ip_address: %s\n", Options::output_ip_address.c_str());
      std::printf("port: %u\n", Options::output_port);

      std::printf("IP defintion:\n");
      std::printf("\tsrc: %d\n", Options::Definitions::ip.src);
      std::printf("\tdst: %d\n", Options::Definitions::ip.dst);
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
