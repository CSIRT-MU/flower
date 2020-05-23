#include <tins/tins.h>

#include <provider.hpp>
#include <flow.hpp>
#include <ipfix.hpp>

int main(int argc, char** argv) {
  if (argc != 3) return 1;

  // auto plugins = Plugins::Manager{"/var/lib/flower/"};
  // auto plugin = Plugin{""};
  // auto provider = PacketProvider{};
  auto connection = IPFIX::Connection{"127.0.0.1", 20'000};
  auto cache = Flow::Cache{};

  // for (int i = 0;; ++i) {
  //   auto raw = provider.get_packet();
  //   if (!raw.data) break;
  //   auto parsed = Tins::EthernetII{raw.data, static_cast<uint32_t>(raw.caplen)};
  //   cache.insert(parsed);

  //   if (i >= 200) {
  //     for (auto& record: cache.records()) {
  //       connection.to_export(record);
  //     }
  //     i = 0;
  //   }
  // }

  /* EXPORT */
  for (auto& record: cache.records()) {
    connection.to_export(record);
  }

  return 0;
}
