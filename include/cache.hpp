#pragma once

#include <unordered_map>
#include <mutex>

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

  /**
   * Type used to store record with some metadata.
   */
  using Record = std::pair<Properties, Chain>;
  using Type = std::size_t;
  using Digest = std::size_t;
  using Timestamp = unsigned int;
  using RecordsType = std::unordered_map<Digest, Record>;
  using CacheType = std::unordered_map<Type, RecordsType>;
  using RangeType = std::pair<RecordsType::iterator, RecordsType::iterator>;

  CacheType _records;
  mutable std::mutex _mutex;

public:

  RangeType insert(Type, Digest, Chain, Timestamp);
  void erase(Type, RecordsType::iterator);

  template<typename Fun>
  void for_each(Fun&& f) {
    const auto lock = std::lock_guard{_mutex};
    for (auto& [type, cache]: _records) {
      for (auto& [_, entry]: cache) {
        auto& [props, chain] = entry;
        f(type, props, chain);
      }
    }
  }
};

} // namespace Flow
