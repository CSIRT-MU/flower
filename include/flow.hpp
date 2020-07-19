#pragma once

#include <algorithm>
#include <mutex>
#include <unordered_map>

#include <protocol.hpp>

namespace Flow {

/**
 * Metadata used in stroring flow records.
 */
struct Properties {
  std::size_t count;
  unsigned int first_timestamp;
  unsigned int last_timestamp;
};

/**
 * Type used to store record with some metadata.
 */
using Entry = std::pair<Properties, Record>;

/**
 * Flow cache is used to store flow data during program
 * execution. It needs to be fast and it holds relatively
 * large amount of data. Also, it needs to be thread safe,
 * since one thread is inserting data and another is
 * exporting data.
 */
class Cache {
  std::unordered_map<std::size_t, Entry> _records;
  mutable std::mutex _mutex;

public:

  /**
   * Method that inserts record into flow cache
   * @param record record to insert into flow cache
   * @param timestamp timestamp of captured record
   */
  template<typename T>
  void insert(T&& record, unsigned int timestamp) {
    auto key = digest(record);

    const auto lock = std::lock_guard{_mutex};
    auto search = _records.find(key);
    if (search != _records.end()) {
      auto& props = search->second.first;

      props.count += 1;
      if (props.first_timestamp > timestamp) {
        props.first_timestamp = timestamp;
      }
      if (props.last_timestamp < timestamp) {
        props.last_timestamp = timestamp;
      }
    } else {
      _records.emplace(key, Entry{Properties{1, timestamp, timestamp},
          std::forward<T>(record)});
    }
  }

  /**
   * Higher order method used to iterate over entries in no
   * particular order.
   * @param fun callable that will be called for each entry
   */
  template<typename Fun>
  void for_each(Fun&& fun) const {
    const auto lock = std::lock_guard{_mutex};

    std::for_each(_records.cbegin(), _records.cend(),
        std::forward<Fun>(fun));
  }

  /**
   * Higher order method used to iterate and delete elements in
   * flow cache.
   * @param fun callable that must return true/false if element
   *   should be deleted
   */
  template<typename Fun>
  void erase_if(Fun&& fun) {
    const auto lock = std::lock_guard{_mutex};

    std::erase_if(_records, std::forward<Fun>(fun));
  }

  /**
   * Simple getter for all records.
   * @return map of all record entries
   */
  const auto& records() const {
    return _records;
  }
};

} // namespace Flow
