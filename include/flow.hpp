#pragma once

#include <initializer_list>
#include <numeric>
#include <unordered_map>

#include <entry.hpp>

namespace Flow {
/**
 * Flow cache is used to store flow data during program
 * execution. It needs to be fast and it host relatively
 * large amount of data.
 */
class Cache {
  std::unordered_map<std::size_t, Record> _records;

public:

  template<typename T>
  void insert(T&& record) {
    auto key = digest(record);
    _records.emplace(key, std::forward<T>(record));
  }

  const auto& records() const {
    return _records;
  }
};

} // namespace Flow
