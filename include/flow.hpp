#pragma once

#include <set>
#include <map>

#include "record.hpp"

class FlowMatcher {
  const std::set<FlowType> types;
  public:
  FlowMatcher(std::initializer_list<FlowType> types): types(types) {}

  FlowType find_type(const FlowRecord& record) const {
    auto res = types.find(record.type);
    if (res == types.end())
      return FlowType::OTHER;
    
    return *res;
  }
};

struct FlowNodeEntry;

template<typename T>
class FlowNode {
  using FlowNodeRecord = T;

  std::map<FlowNodeRecord, FlowNodeEntry> entries;
  public:
  template<typename Iterator>
  void insert(Iterator begin, Iterator end);

  template<typename Container>
  void insert(Container&& c) { insert(std::begin(c), std::end(c)); }

  auto begin() const { return entries.begin(); }
  auto end() const { return entries.end(); }
  auto cbegin() const { return entries.cbegin(); }
  auto cend() const { return entries.cend(); }
};

struct FlowNodeEntry {
  std::size_t count{1};

  FlowNode<IPv4Record> ipv4_next{};
  FlowNode<TCPRecord> tcp_next{};
};

/* This method expects that all FlowRecords will be of captured types */
template<typename T>
template<typename Iterator>
void FlowNode<T>::insert(Iterator begin, Iterator end) {
  if (begin == end) return;

  // TODO: Refactor
  auto record = static_cast<FlowNodeRecord*>(begin->get());
  auto entry = entries.find(*record);
  if (entry != entries.end()) {
    ++(entry->second.count);
  } else {
    entry = entries.emplace(*record, FlowNodeEntry{}).first;
  }

  for (auto it = ++begin; begin != end; ++begin) {
    auto type = (*it)->type;
    // TODO: Try to remove switch
    switch(type) {
      case FlowType::IPv4:
        entry->second.ipv4_next.insert(it, end);
        return;
      case FlowType::TCP:
        entry->second.tcp_next.insert(it, end);
        return;
      default:
        continue;
    }
  }
}
