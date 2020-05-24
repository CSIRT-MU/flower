#pragma once

#include <initializer_list>
#include <numeric>
#include <unordered_map>

namespace Flow {

/**
 * Hash function of arbitrary arity.
 * @param args numbers to reduce to one hash
 * @return hash reduced from all provided numbers
 */
template<typename... N>
constexpr std::size_t combine(const N... n) {
  constexpr std::size_t SHIFT_LEFT = 6;
  constexpr std::size_t SHIFT_RIGHT = 2;
  constexpr std::size_t SEED = 0x9e3779b9;

  auto args = { n... };

  return std::reduce(args.begin(), args.end(), 0,
      [](std::size_t acc, std::size_t a) constexpr {
      return ((a<<SHIFT_LEFT) + (a>>SHIFT_RIGHT) + SEED) ^ acc;
      });
}

namespace Entry {

struct Header {
  std::size_t prev;
  std::size_t type;
};

template<typename T>
struct Entry: public Header {
  T value;
};

} // namespace Entry

struct Record {
  std::size_t count;
  std::size_t timestamp;
};

struct IP: public Record {
  unsigned src;
  unsigned dst;
};

struct TCP: public Record {
  unsigned src;
  unsigned dst;
};

// TODO(dudoslav): Implement
class Cache {
  std::unordered_map<std::size_t, Entry::Entry<IP>> _ip_cache;

public:

  template<typename InputIt>
  void insert(InputIt first, InputIt last) {
    for (auto it = first; it != last; ++it) {
    }
  }
};

} // namespace Flow
