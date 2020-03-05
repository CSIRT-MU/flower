#pragma once

#include <unordered_map>
#include <vector>
#include <numeric>

#include <tins/tins.h>

#include "entry.hpp"

namespace Flow {

template<typename T>
std::size_t combine(T h) {
  return h;
}

template<typename T, typename... Args>
std::size_t combine(T h, Args... args) {
  return ((h<<6) + (h>>2) + 0x9e3779b9) ^ combine(args...);
}

class Iterator {
  const Entry* entry;

  public:

  Iterator(const Entry* entry): entry{entry} {}
  Iterator(): Iterator{nullptr} {}

  Iterator& operator++() {
    entry = entry->prev;
    return *this;
  }

  bool operator==(const Iterator& o) const {
    return entry == o.entry;
  }

  bool operator!=(const Iterator& o) const {
    return !operator==(o);
  }

  const Entry& operator*() const { return *entry; }
  const Entry& operator->() const { return *entry; }
};

class Record {
  const Iterator start;

  public:

  Record(const Entry* entry): start{entry} {}

  Iterator begin() const { return start; }
  Iterator end() const { return nullptr; }

  std::size_t type_hash() const {
    return std::accumulate(begin(), end(), 0, [](auto a, const auto& x){
        return combine(a, x.type);
        });
  }

  Payload template_body() const {
    return std::accumulate(begin(), end(), Payload{}, [](auto a, const auto& x){
        return a += x.template_body();
        });
  }

  Payload record_body() const {
    return std::accumulate(begin(), end(), Payload{}, [](auto a, const auto& x){
        return a += x.record_body();
        });
  }
};

class Cache {
  std::unordered_map<std::size_t, IP> ip_cache;
  std::unordered_map<std::size_t, TCP> tcp_cache;
  std::vector<Record> last;

  public:

  void insert(const Tins::PDU& packet) {
    auto hash = std::size_t{0};
    Entry* prev = nullptr;
    for (const auto& pdu: Tins::iterate_pdus(packet)) {
      // IP
      if (pdu.pdu_type() == Tins::PDU::PDUType::IP) {
        auto ip = static_cast<const Tins::IP&>(pdu);
        hash = combine(hash, ip.src_addr(), ip.dst_addr());
        auto search = ip_cache.find(hash);
        if (search == ip_cache.end()) {
          auto entry = IP{ip.src_addr(), ip.dst_addr(), prev};
          auto [placed, _] = ip_cache.emplace(hash, entry);
          prev = &(placed->second);
        } else {
          search->second.count++;
        }
      }
      // TCP
      if (pdu.pdu_type() == Tins::PDU::PDUType::TCP) {
        auto tcp = static_cast<const Tins::TCP&>(pdu);
        hash = combine(hash, tcp.sport(), tcp.dport());
        auto search = tcp_cache.find(hash);
        if (search == tcp_cache.end()) {
          auto entry = TCP{tcp.sport(), tcp.dport(), prev};
          auto [placed, _] = tcp_cache.emplace(hash, entry);
          prev = &(placed->second);
        } else {
          search->second.count++;
        }
      }
    }

    last.emplace_back(prev); // TODO: Only emplace one last
  }

  auto records() const {
    return last;
  }
};

} // namespace Flow
