#pragma once

#include <unordered_map>
#include <ctime>

#include <buffer.hpp>
#include <ipfix.hpp>

namespace Flow {

struct CacheEntry {
  IPFIX::Properties props;
  Buffer values;
};

class Cache : public std::unordered_map<std::size_t, CacheEntry> {
public:
  iterator insert_record(std::size_t, timeval, Buffer);
};

} // namespace Flow
