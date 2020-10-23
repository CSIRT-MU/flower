#pragma once

#include <vector>
#include <algorithm>

class Buffer: public std::vector<std::byte> {
public:
  template<typename T>
  void push_back_any(T value) {
    auto* value_p = reinterpret_cast<std::byte*>(&value);
    insert(end(), value_p, value_p + sizeof(value));
  }
  
  template<typename T>
  void set_any_at(size_type index, T value) {
    auto* value_p = reinterpret_cast<std::byte*>(&value);
    std::copy(value_p, value_p + sizeof(value), begin() + index);
  }
};
