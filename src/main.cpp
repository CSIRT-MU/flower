#include <memory>
#include <stdexcept>
#include <vector>
#include <iostream>

#include <pcapplusplus/PcapFileDevice.h>
#include <pcapplusplus/Packet.h>
#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/TcpLayer.h>

#include "benchmark.hpp"
#include "flow.hpp"

/* Function used to create PCAP reader */
auto open_reader(const char* path) {
  auto deleter = [](pcpp::IFileReaderDevice* frd){
    if (frd && frd->isOpened()) frd->close();
    if (frd) delete frd;
  };
  auto reader = std::unique_ptr<pcpp::IFileReaderDevice, decltype(deleter)>
    (pcpp::IFileReaderDevice::getReader(path), deleter);

  if (!reader)
    throw std::runtime_error("Cannot create reader");
  if (!reader->open())
    throw std::runtime_error("Cannot open reader");

  return reader;
}

int main(int argc, char** argv) {
  auto st = ScopedTimer{__func__};
  if (argc != 2) return 1;

  auto cache = FlowNode<IPv4Record>{};
  
  auto reader = open_reader(argv[1]);
  pcpp::RawPacket raw_packet;
  while (reader->getNextPacket(raw_packet)) {
    pcpp::Packet packet(&raw_packet);
    auto p = std::vector<std::unique_ptr<FlowRecord>>{};
    for (auto l = packet.getFirstLayer(); l != nullptr; l = l->getNextLayer()) {
      switch(l->getProtocol()) {
        case pcpp::IPv4: {
          auto ipv4 = static_cast<const pcpp::IPv4Layer*>(l);
          p.push_back(std::make_unique<IPv4Record>
              (ipv4->getSrcIpAddress().toInt(), ipv4->getDstIpAddress().toInt()));
                         }
          break;
        case pcpp::TCP: {
          auto tcp = static_cast<const pcpp::TcpLayer*>(l);
          p.push_back(std::make_unique<TCPRecord>
              (tcp->getTcpHeader()->portSrc, tcp->getTcpHeader()->portDst));
                        }
          break;
      }
    }
    cache.insert(p.begin(), p.end());
  }


  for (const auto& ip : cache) {
    std::cout << ip.first.src << " " << ip.first.dst << std::endl;
    for (const auto& tcp : ip.second.tcp_next) {
      std::cout << tcp.first.src << " " << tcp.first.dst << std::endl;
    }
  }

  return 0;
}
