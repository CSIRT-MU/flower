#include <memory>
#include <vector>
#include <iostream>

#include <tins/tins.h>

#include "benchmark.hpp"
#include "record.hpp"
#include "flow.hpp"
#include "shared_object.hpp"
#include "provider.hpp"
#include "network.hpp"

// Usage: ./flower -P FileProvider

int main(int argc, char** argv) {
  auto st = ScopedTimer{__func__};
  if (argc != 2) return 1;
  
  auto cache = FlowNode<IPv4Record>{};
  auto plugin = Plugin{"shared/file_provider.so"};
  auto provider = PacketProvider{std::move(plugin), argv[1]};
  auto socket = TCPSocket{};
  socket.connect("127.0.0.1", 20'000);

  for (;;) {
    auto packet = provider.get_packet();
    if (!(packet.data)) break;
    auto record = RecordPacket{};
    auto pdu = Tins::EthernetII{packet.data, static_cast<uint32_t>(packet.caplen)};

    auto ipv4 = pdu.find_pdu<Tins::IP>();
    if (ipv4)
      record.push_back(std::make_unique<IPv4Record>(ipv4->src_addr(), ipv4->dst_addr()));
    auto tcp = pdu.find_pdu<Tins::TCP>();
    if (tcp)
      record.push_back(std::make_unique<TCPRecord>(tcp->sport(), tcp->dport()));

    cache.insert(record);
  }

  /*
  auto msg = ipfix::Message{};
  auto template = ipfix::Template{256};
  template.add_field(8, 4);
  template.add_field(12, 4);
  
  auto records = ipfix::Records{256};
  records << ip.src;
  records << ip.dst;

  msg << template << records;
  socket << msg;
  */

  // IPFIX Message header
  socket << short{0x000a}; // Version
  socket << short{44}; // Length
  socket << 0; // Timestamp
  socket << 0; // Sequence nymber
  socket << 0; // Domain observation

  // IPFIX Template
  socket << short{2}; // Template Set
  socket << short{28}; // Length
  socket << short{256}; // Template ID
  socket << short{5}; // Field count
  socket << short{8}; // IPSource Field
  socket << short{4}; // Field length
  socket << short{12}; // IPDst Field
  socket << short{4}; // Field length
  socket << short{7}; // SRCPort
  socket << short{2}; // Field length
  socket << short{11}; // DSTPort
  socket << short{2}; // Field length
  socket << short{2}; // PacketDelta
  socket << short{4}; // Field length

  for (const auto& ip : cache) {
    std::cout << ip.first.src << " " << ip.first.dst << std::endl;
    for (const auto& tcp : ip.second.tcp_next) {
      std::cout << "\t" << tcp.first.src << " " << tcp.first.dst << std::endl;

      socket << short{0x000a}; // Version
      socket << short{36}; // Length
      socket << 0; // Timestamp
      socket << 0; // Sequence nymber
      socket << 0; // Domain observation

      socket << short{256}; // Template ID
      socket << short{20}; // Length
      socket << ip.first.src; // SRCIP
      socket << ip.first.dst; // DSTIP
      socket << short{tcp.first.src}; // SRCPORT
      socket << short{tcp.first.dst}; // DSTPORT
      socket << static_cast<int>(tcp.second.count); // Count
    }
  }

  return 0;
}
