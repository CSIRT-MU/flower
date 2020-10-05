#pragma once

#include <unordered_map>
#include <ctime>

#include <protocol.hpp>

namespace Flow {

/**
 * Flow cache is used to store flow data during program
 * execution. It needs to be fast and it holds relatively
 * large amount of data. Also, it needs to be thread safe,
 * since one thread is inserting data and another is
 * exporting data.
 */
class Cache {

  using Type = std::size_t;
  using Digest = std::size_t;
  using Timestamp = timeval;
  using RecordsType = std::unordered_map<Digest, Record>;

  RecordsType _records;

public:

  RecordsType::iterator insert(Digest, Chain, Timestamp);
  void erase(Digest);
  std::size_t size() const;
  RecordsType::iterator find(Digest);
  RecordsType::iterator begin();
  RecordsType::iterator end();
};

} // namespace Flow
