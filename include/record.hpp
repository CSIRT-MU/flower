#pragma once

#include <vector>
#include <memory>

enum class FlowType { IPv4, TCP, OTHER };

struct FlowRecord {
  const FlowType type;

  FlowRecord(FlowType type): type(type) {}
};

struct IPv4Record: FlowRecord {
  const int src;
  const int dst;

  IPv4Record(int src, int dst):
    FlowRecord(FlowType::IPv4), src(src), dst(dst) {}

  friend bool operator<(const IPv4Record& f, const IPv4Record& s) {
    return f.src < s.src && f.dst < s.dst;
  }
};

struct TCPRecord: FlowRecord {
  const short src;
  const short dst;

  TCPRecord(short src, short dst):
    FlowRecord(FlowType::TCP), src(src), dst(dst) {}

  friend bool operator<(const TCPRecord& f, const TCPRecord& s) {
    return f.src < s.src && f.dst < s.dst;
  }
};

using RecordPacket = std::vector<std::unique_ptr<FlowRecord>>;
