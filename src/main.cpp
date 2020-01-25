#include <memory>
#include <vector>
#include <iostream>

#include "benchmark.hpp"
#include "record.hpp"
#include "flow.hpp"
#include "shared_object.hpp"
#include "provider.hpp"

struct Options {
  std::string provider;
  std::string provider_arg;
};

int main(int argc, char** argv) {
  auto st = ScopedTimer{__func__};
  if (argc != 2) return 1;

  auto file_provider_so = SharedObject("./shared/file_provider.so");
  // auto file_provider_so = SharedObject("./shared/interface_provider.so");
  auto file_provider_factory =
    file_provider_so.factory<PacketProvider, const char*>("create_provider");
  auto file_provider = file_provider_factory(argv[1]);

  auto cache = FlowNode<IPv4Record>{};

  while (true) {
    auto packet = std::unique_ptr<RecordPacket>{
        reinterpret_cast<RecordPacket*>(file_provider->getPacket())};
    if (!packet) break;

    cache.insert(*packet);
  }

  for (const auto& ip : cache) {
    std::cout << ip.first.src << " " << ip.first.dst << std::endl;
    for (const auto& tcp : ip.second.tcp_next) {
      std::cout << tcp.first.src << " " << tcp.first.dst << std::endl;
    }
  }

  return 0;
}
