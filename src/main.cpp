#include <tins/tins.h>

#include "benchmark.hpp"
#include "flow.hpp"
#include "provider.hpp"
#include "ipfix.hpp"

int main(int argc, char** argv) {
  if (argc != 2) return 1;

  auto provider = PacketProvider{Plugin{"shared/file_provider.so"}, argv[1]};
  auto connection = IPFIX::Connection{"127.0.0.1", 20'000};
  auto cache = Flow::Cache{};

  auto st = ScopedTimer{__func__};

  for (int i = 0;;++i) {
    auto raw = provider.get_packet();
    if (!raw.data) break;
    auto parsed = Tins::EthernetII{raw.data, static_cast<uint32_t>(raw.caplen)};
    cache.insert(parsed);

    if (i >= 10) {
      for (auto& record: cache.records()) {
        connection.to_export(record);
      }
      i = 0;
    }
  }

  /* EXPORT */
  for (auto& record: cache.records()) {
    connection.to_export(record);
  }

  return 0;
}
