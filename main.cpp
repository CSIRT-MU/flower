#include <iostream>

#include <tins/tins.h>

#include <arguments.hpp>
#include <flow.hpp>
#include <manager.hpp>

static void start(Plugins::Input input) {
  for (;;) {
    auto raw = input.get_packet();
    if (raw.data == nullptr) {
      return;
    }

    auto packet = Tins::EthernetII{raw.data, raw.caplen};
    for (const auto& pdu: Tins::iterate_pdus(packet)) {
      std::cout << Tins::Utils::to_string(pdu.pdu_type()) << " -> ";
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
