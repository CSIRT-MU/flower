#include <tins/tins.h>

#include "record.hpp"
#include "provider.hpp"

class FilePacketProvider: public PacketProvider {
  Tins::FileSniffer handle;
  public:
  FilePacketProvider(const char* file): handle(file) {}

  void* getPacket() override {
    const Tins::PDU* packet = handle.next_packet();
    if (!packet) return nullptr;

    auto result = new RecordPacket{};
    auto ipv4 = packet->find_pdu<Tins::IP>();
    if (ipv4)
      result->push_back(std::make_unique<IPv4Record>(ipv4->src_addr(), ipv4->dst_addr()));
    auto tcp = packet->find_pdu<Tins::TCP>();
    if (tcp)
      result->push_back(std::make_unique<TCPRecord>(tcp->sport(), tcp->dport()));

    return result;
  }
};

extern "C" void* create_provider(const char* arg) {
  return new FilePacketProvider{arg};
}
