#pragma once

#include <list>
#include <variant>

namespace Flow {

struct IP {
  unsigned src;
  unsigned dst;
};

struct TCP {
  unsigned src;
  unsigned dst;
};

using Entry = std::variant<IP, TCP>;
using Record = std::list<Entry>;

/**
 * Hash function of arbitrary arity. This function MUST BE commutative.
 * @param args numbers to reduce to one hash
 * @return hash reduced from all provided numbers
 */
template<typename F, typename... N>
constexpr std::size_t combine(F f, N... n) {
  constexpr std::size_t SHIFT_LEFT = 6;
  constexpr std::size_t SHIFT_RIGHT = 2;
  constexpr std::size_t SEED = 0x9e3779b9;

  auto args = { f, n... };

  return std::reduce(args.begin(), args.end(), 0,
      [](std::size_t acc, std::size_t a) constexpr {
      return a ^ ((acc<<SHIFT_LEFT) + (acc>>SHIFT_RIGHT) + SEED);
      });
}

inline std::size_t digest(const IP& ip) {
  return combine(ip.src, ip.dst);
}

inline std::size_t digest(const TCP& tcp) {
  return combine(tcp.src, tcp.dst);
}

inline std::size_t digest(const Entry& entry) {
  return std::visit([](const auto& v){ return digest(v); }, entry);
}

inline std::size_t digest(const Record& record) {
  auto result = digest(record.front());

  for (auto it = ++(record.cbegin()); it != record.cend(); ++it) {
    result = combine(result, digest(*it));
  }

  return result;
}

} // namespace Flow
