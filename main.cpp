#include <iostream>

#include <tins/tins.h>

#include <async.hpp>
#include <flow.hpp>
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
  auto cache = Flow::Cache{};
  auto timer = Async::Timer{Options::instance().export_interval};
  timer.start([&cache](){
      std::cout << "EXPORTING:" << std::endl;
      cache.erase_if([](const auto& p){
          auto& props = p.second.first;
          std::cout << "COUNT: " << props.count;
          std::cout << " FIRST: " << props.first_timestamp;
          std::cout << " LAST: " << props.last_timestamp << std::endl;
          print_record(p.second.second);
          return true;
          });
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
  try {
    Options::load("/home/dudoslav/.flower.conf");
    Options::parse(argc, argv);
  } catch (const std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  const auto& options = Options::instance();

  auto plugin_manager = Plugins::Manager{"plugins"};

  switch (options.activity) {
    case Options::Activity::LIST_PLUGINS:
      std::cout << "Input plugins:" << std::endl;
      for (const auto& p: plugin_manager.inputs()) {
        std::cout << p.info().name << std::endl;
      }
      break;
    case Options::Activity::SHOW_USAGE:
      std::cout << "./flower argument [options] ..." << std::endl;
      break;
    case Options::Activity::MAIN_ACTIVITY:
      try {
        auto input = plugin_manager.create_input(options.input_name,
            options.input_argument.c_str());
        start(std::move(input));
      } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 2;
      }
      break;
  }

  return 0;
}
