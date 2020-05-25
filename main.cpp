#include <iostream>

#include <tins/tins.h>

#include <arguments.hpp>
#include <flow.hpp>
#include <manager.hpp>

static void start(Plugins::Input input) {
  auto cache = Flow::Cache{};

  for (;;) {
    auto raw = input.get_packet();
    if (raw.data == nullptr) {
      break;
    }

    auto record = Flow::Record{};
    auto packet = Tins::EthernetII{raw.data, raw.caplen};
    for (const auto& pdu: Tins::iterate_pdus(packet)) {
      std::cout << Tins::Utils::to_string(pdu.pdu_type()) << " -> ";

      if (pdu.pdu_type() == Tins::PDU::PDUType::IP) {
        const auto& ip = dynamic_cast<const Tins::IP&>(pdu);
        record.emplace_back(Flow::IP{ip.src_addr(), ip.dst_addr()});
      } else if (pdu.pdu_type() == Tins::PDU::PDUType::TCP) {
        const auto& tcp = dynamic_cast<const Tins::TCP&>(pdu);
        record.emplace_back(Flow::TCP{tcp.sport(), tcp.dport()});
      }
    }
    std::cout << "END" << std::endl;

    if (!record.empty()) {
      cache.insert(record);
    }
  }

  std::cout << "CACHE:" << std::endl;
  for (const auto& record: cache.records()) {
    std::cout << record.first << " ";
    for (const auto& entry: record.second) {
      std::visit([](const auto& e){
          using T = std::decay_t<decltype(e)>;
          if constexpr (std::is_same_v<T, Flow::IP>) {
            std::cout << "IP " << e.src << " " << e.dst << " -> ";
          } else if constexpr (std::is_same_v<T, Flow::TCP>) {
            std::cout << "TCP " << e.src << " " << e.dst << " -> ";
          }
          }, entry);
    }
    std::cout << "END" << std::endl;
  }
}

int main(int argc, char** argv) {
  auto options = Arguments::Options{};
  try {
    options = Arguments::parse(argc, argv);
  } catch (std::runtime_error& e) {
    std::cerr << e.what() << std::endl;
    return 2;
  }

  auto plugin_manager = Plugins::Manager{"plugins"};

  switch (options.activity) {
    case Arguments::Activity::LIST_PLUGINS:
      std::cout << "Input plugins:" << std::endl;
      for (const auto& p: plugin_manager.inputs()) {
        std::cout << p.info().name << std::endl;
      }
      break;
    case Arguments::Activity::SHOW_USAGE:
      std::cout << "./flower [options]..." << std::endl;
      break;
    case Arguments::Activity::MAIN_ACTIVITY:
      try {
        auto input = plugin_manager.create_input(options.input_name,
            options.input_argument.c_str());
        start(std::move(input));
      } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 3;
      }
      break;
  }

  return 0;
}
