#include <iostream>
#include <iomanip>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include <pcapplusplus/IPv4Layer.h>
#include <pcapplusplus/VlanLayer.h>
#include <pcapplusplus/TcpLayer.h>
#include <pcapplusplus/Packet.h>
#include <pcapplusplus/PcapFileDevice.h>

using FlowKey = std::size_t;

struct IPv4Record {
  int src, dst;
  std::size_t count = 0;

  IPv4Record(int src, int dst): src(src), dst(dst) {}
  FlowKey hash() const {
    return src ^ (dst << 1);
  }
};

struct VlanRecord {
  int id;
  std::size_t count = 0;

  VlanRecord(int id): id(id) {}
  FlowKey hash() const {
    return id;
  }
};

struct TcpRecord {
  unsigned src, dst;
  std::size_t count = 0;

  TcpRecord(unsigned src, unsigned dst): src(src), dst(dst) {}
  FlowKey hash() const {
    return src ^ (dst << 1);
  }
};

template<typename T>
class FlowCache {
  using FlowRecord = T;
  using FlowRecordCPtr = const T*;

  std::unordered_map<FlowKey, FlowRecord> records;
  std::unordered_multimap<FlowKey, FlowRecordCPtr> parented;

  public:

  void handle(FlowRecord record, FlowKey phash) {
    FlowKey hash = record.hash();
    auto it = records.find(hash);
    if (it == records.end()) {
      auto rit = records.emplace(std::make_pair(hash, record));
      parented.emplace(std::make_pair(phash, &(rit.first->second)));

      rit.first->second.count++;
    } else {
      it->second.count++;
    }
  }

  decltype(auto) for_parent(FlowKey phash) const {
    return parented.equal_range(phash);
  }
};

enum class FlowProcessorType { IPv4, Vlan, Tcp };

class FlowCollector {
  using FlowProcessor = FlowKey (FlowCollector::*)(const pcpp::Layer*, FlowKey);
  using FlowPrinter = void (FlowCollector::*)(FlowKey, unsigned) const;

  std::vector<FlowProcessor> processor_chain;
  std::vector<FlowPrinter> printer_chain;

  FlowCache<IPv4Record> ipv4_cache;
  FlowCache<VlanRecord> vlan_cache;
  FlowCache<TcpRecord> tcp_cache;

  public:

  FlowCollector(const std::vector<FlowProcessorType>& processors) {
    for (const auto& fpt : processors) {
      switch (fpt) {
        case FlowProcessorType::Vlan:
          processor_chain.push_back(&FlowCollector::process_vlan);
          printer_chain.push_back(&FlowCollector::print_vlan);
          break;
        case FlowProcessorType::IPv4:
          processor_chain.push_back(&FlowCollector::process_ipv4);
          printer_chain.push_back(&FlowCollector::print_ipv4);
          break;
        case FlowProcessorType::Tcp:
          processor_chain.push_back(&FlowCollector::process_tcp);
          printer_chain.push_back(&FlowCollector::print_tcp);
          break;
        default:
          break;
      }
    }
  }

  void print_vlan(FlowKey parent, unsigned level) const {
    auto range = vlan_cache.for_parent(parent);
    for (auto it = range.first; it != range.second; ++it) {
      for (unsigned i = 0; i < level; ++i) std::cout << '\t';
      std::cout << std::left << std::setw(8) << "VLAN";
      std::cout << std::left << std::setw(8) << it->second->count;
      std::cout << std::left << std::setw(16) << it->second->id << std::endl;

      if (level + 1 >= printer_chain.size()) continue;
      (this->*printer_chain[level + 1])(it->second->hash(), level + 1);
    }
  }

  FlowKey process_vlan(const pcpp::Layer* layer, FlowKey parent) {
    if (layer->getProtocol() != pcpp::VLAN) return 0;

    auto vlan = static_cast<const pcpp::VlanLayer*>(layer);

    int id = vlan->getVlanID();
    VlanRecord record{id};
    vlan_cache.handle(record, parent);

    return record.hash();
  }

  void print_ipv4(FlowKey parent, unsigned level) const {
    auto range = ipv4_cache.for_parent(parent);
    for (auto it = range.first; it != range.second; ++it) {
      for (unsigned i = 0; i < level; ++i) std::cout << '\t';
      std::cout << std::left << std::setw(8) << "IPv4";
      std::cout << std::left << std::setw(8) << it->second->count;
      std::cout << std::left << std::setw(16) << pcpp::IPv4Address(it->second->src).toString();
      std::cout << std::left << std::setw(16) << pcpp::IPv4Address(it->second->dst).toString();
      std::cout << std::endl;

      if (level + 1 >= printer_chain.size()) continue;
      (this->*printer_chain[level + 1])(it->second->hash(), level + 1);
    }
  }

  FlowKey process_ipv4(const pcpp::Layer* layer, FlowKey parent) {
    if (layer->getProtocol() != pcpp::IPv4) return 0;

    auto ipv4 = static_cast<const pcpp::IPv4Layer*>(layer);

    int src = ipv4->getSrcIpAddress().toInt();
    int dst = ipv4->getDstIpAddress().toInt();
    IPv4Record record{src, dst};
    ipv4_cache.handle(record, parent);

    return record.hash();
  }

  void print_tcp(FlowKey parent, unsigned level) const {
    auto range = tcp_cache.for_parent(parent);
    for (auto it = range.first; it != range.second; ++it) {
      for (unsigned i = 0; i < level; ++i) std::cout << '\t';
      std::cout << std::left << std::setw(8) << "TCP";
      std::cout << std::left << std::setw(8) << it->second->count;
      std::cout << std::left << std::setw(8) << it->second->src;
      std::cout << std::left << std::setw(8) << it->second->dst;
      std::cout << std::endl;

      if (level + 1 >= printer_chain.size()) continue;
      (this->*printer_chain[level + 1])(it->second->hash(), level + 1);
    }
  }

  FlowKey process_tcp(const pcpp::Layer* layer, FlowKey parent) {
    if (layer->getProtocol() != pcpp::TCP) return 0;

    auto tcp = static_cast<const pcpp::TcpLayer*>(layer);

    unsigned src = tcp->getTcpHeader()->portSrc;
    unsigned dst = tcp->getTcpHeader()->portDst;
    TcpRecord record{src, dst};
    tcp_cache.handle(record, parent);

    return record.hash();
  }

  void print() const {
    if (!printer_chain.size()) return;
    (this->*printer_chain[0])(0, 0);
  }

  void collect(const pcpp::Layer* layer) {
    unsigned index = 0;
    FlowKey parent = 0;
    for (auto l = layer; l != nullptr; l = l->getNextLayer()) {
      if (processor_chain.size() <= index) return;

      FlowKey matched = (this->*processor_chain[index])(l, parent);
      if (matched) {
        parent = matched;
        index++;
      }
    }
  }
};

/* Function used to create PCAP reader */
decltype(auto) open_reader(const char* path) {
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
  if (argc != 2) return 1;

  auto reader = open_reader(argv[1]);
  FlowCollector flow_collector({FlowProcessorType::IPv4, FlowProcessorType::Tcp});

  pcpp::RawPacket raw_packet;
  while (reader->getNextPacket(raw_packet)) {
    pcpp::Packet packet(&raw_packet);
    flow_collector.collect(packet.getFirstLayer());
  }

  flow_collector.print();

  return 0;
}
