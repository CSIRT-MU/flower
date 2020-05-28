#pragma once

#include <algorithm>
#include <mutex>
#include <unordered_map>

#include <protocol.hpp>

namespace Flow {
/**
 * Flow cache is used to store flow data during program
 * execution. It needs to be fast and it host relatively
 * large amount of data.
 */
class Cache {
  struct Properties {
    std::size_t count;
    unsigned int first_timestamp;
    unsigned int last_timestamp;
  };

  using Entry = std::pair<Properties, Record>;

  std::unordered_map<std::size_t, Entry> _records;
  mutable std::mutex _mutex;

public:

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

  template<typename Fun>
  void for_each(Fun&& fun) const {
    const auto lock = std::lock_guard{_mutex};

    std::for_each(_records.cbegin(), _records.cend(),
        std::forward<Fun>(fun));
  }

  template<typename Fun>
  void erase_if(Fun&& fun) {
    const auto lock = std::lock_guard{_mutex};

    std::erase_if(_records, std::forward<Fun>(fun));
  }

  const auto& records() const {
    return _records;
  }
};

} // namespace Flow
