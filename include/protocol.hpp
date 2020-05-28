#pragma once

#include <initializer_list>
#include <numeric>
#include <variant>
#include <vector>

namespace Flow {

// TODO(dudoslav): Rework digest as class method

struct IP {
  uint32_t src;
  uint32_t dst;
};

struct TCP {
  uint16_t src;
  uint16_t dst;
};

struct UDP {
  uint16_t src;
  uint16_t dst;
};

struct DOT1Q {
  uint16_t id;
};

using Protocol = std::variant<IP, TCP, UDP, DOT1Q>;
using Record = std::vector<Protocol>;

/**
 * Hash function of arbitrary arity. This function MUST BE non commutative.
 * @param args numbers to reduce to one hash
 * @return hash reduced from all provided numbers
 */
template<typename... N>
constexpr std::size_t combine(std::size_t f, N... n) {
  constexpr std::size_t SHIFT_LEFT = 6;
  constexpr std::size_t SHIFT_RIGHT = 2;
  constexpr std::size_t SEED = 0x9e3779b9;

  auto args = { f, static_cast<std::size_t>(n)... };

  return std::reduce(std::next(args.begin()), args.end(), *(args.begin()),
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

inline std::size_t digest(const UDP& udp) {
  return combine(udp.src, udp.dst);
}

inline std::size_t digest(const DOT1Q& dot1q) {
  return combine(dot1q.id);
}

inline std::size_t digest(const Protocol& protocol) {
  return std::visit([](const auto& v){ return digest(v); }, protocol);
}

inline std::size_t digest(const Record& record) {
  auto result = digest(record.front());

  for (auto it = ++(record.cbegin()); it != record.cend(); ++it) {
    result = combine(result, digest(*it));
  }

  return result;
}

} // namespace Flow
