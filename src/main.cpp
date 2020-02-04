#include <memory>
#include <vector>
#include <iostream>

#include "benchmark.hpp"
#include "record.hpp"
#include "flow.hpp"
#include "shared_object.hpp"
#include "provider.hpp"

// Usage: ./flower -P FileProvider

int main(int argc, char** argv) {
  auto st = ScopedTimer{__func__};
  if (argc != 2) return 1;
  
  try {
    auto cache = FlowNode<IPv4Record>{};
    auto plugin = Plugin{"shared/file_provider.so"};
    auto provider = PacketProvider{std::move(plugin), argv[1]};

    auto packet = provider.get_packet();

    for (const auto& ip : cache) {
      std::cout << ip.first.src << " " << ip.first.dst << std::endl;
      for (const auto& tcp : ip.second.tcp_next) {
        std::cout << tcp.first.src << " " << tcp.first.dst << std::endl;
      }
    }
  } catch (std::string msg) {
    std::cout << msg << std::endl;
  }

  return 0;
}
