#pragma once

#include <initializer_list>
#include <numeric>
#include <variant>
#include <vector>

namespace Flow {

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

  return std::accumulate(std::next(args.begin()), args.end(), *(args.begin()),
      [](std::size_t acc, std::size_t a) constexpr {
      return a ^ ((acc<<SHIFT_LEFT) + (acc>>SHIFT_RIGHT) + SEED);
      });
}

enum class Type: unsigned long {
  IP,
  TCP,
  UDP,
  DOT1Q
};

struct IP {
  uint32_t src;
  uint32_t dst;

  [[nodiscard]] std::size_t digest() const {
    return combine(src, dst);
  }

  [[nodiscard]] static Type type() {
    return Type::IP;
  }
};

struct TCP {
  uint16_t src;
  uint16_t dst;

  [[nodiscard]] std::size_t digest() const {
    return combine(src, dst);
  }

  [[nodiscard]] static Type type() {
    return Type::TCP;
  }
};

struct UDP {
  uint16_t src;
  uint16_t dst;

  [[nodiscard]] std::size_t digest() const {
    return combine(src, dst);
  }

  [[nodiscard]] static Type type() {
    return Type::UDP;
  }
};

struct DOT1Q {
  uint16_t id;

  [[nodiscard]] std::size_t digest() const {
    return combine(id);
  }

  [[nodiscard]] static Type type() {
    return Type::DOT1Q;
  }
};

using Protocol = std::variant<IP, TCP, UDP, DOT1Q>;
using Record = std::vector<Protocol>;

inline std::size_t digest(const Protocol& protocol) {
  return std::visit([](const auto& v){ return v.digest(); }, protocol);
}

inline std::size_t type(const Protocol& protocol) {
  return static_cast<std::size_t>(
      std::visit([](const auto& v){ return v.type(); }, protocol));
}

// TODO(dudoslav): Handle if record is empty
inline std::size_t digest(const Record& record) {
  return std::accumulate(std::next(record.cbegin()), record.cend(),
      digest(record.front()), [](std::size_t acc, const Protocol& p){
      return combine(acc, digest(p));
      });
}

// TODO(dudoslav): Handle if record is empty
inline std::size_t type(const Record& record) {
  return std::accumulate(std::next(record.cbegin()), record.cend(),
      type(record.front()), [](std::size_t acc, const Protocol& p){
      return combine(acc, type(p));
      });
}

} // namespace Flow
