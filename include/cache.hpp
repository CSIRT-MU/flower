#pragma once

#include <unordered_map>
#include <vector>
#include <ipfix.hpp>

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
  using Values = std::vector<std::byte>;
  using Record = std::tuple<IPFIX::Properties, Type, Values>;
  using RecordsType = std::unordered_map<Digest, Record>;

  RecordsType _records;

public:
  using Entry = RecordsType::iterator;

  Entry insert(Digest, Type, Values, Timestamp);
  std::size_t size() const;
  void erase(Digest);
  Entry find(Digest);

  Entry begin();
  Entry end();
};

} // namespace Flow
