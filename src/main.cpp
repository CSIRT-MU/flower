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

  for (;;) {
    auto raw = provider.get_packet();
    if (!raw.data) break;
    auto parsed = Tins::EthernetII{raw.data, static_cast<uint32_t>(raw.caplen)};
    cache.insert(parsed);
  }

  /* EXPORT */
  for (auto& record: cache.records()) {
    connection.send_record(record);
  }

  return 0;
}
