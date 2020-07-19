#pragma once

#include <cstring>
#include <initializer_list>
#include <numeric>
#include <span>
#include <variant>
#include <vector>

#include <arpa/inet.h>

namespace Flow {

static constexpr auto IPFIX_SHORT = 2;
static constexpr auto IPFIX_LONG = 4;

// https://www.iana.org/assignments/ipfix/ipfix.xhtml
static constexpr auto IPFIX_PACKET_DELTA_COUNT = 2;
static constexpr auto IPFIX_PROTOCOL_IDENTIFIER = 4;
static constexpr auto IPFIX_SRC_IP4_ADDR = 8;
static constexpr auto IPFIX_DST_IP4_ADDR = 12;
static constexpr auto IPFIX_SRC_PORT = 7;
static constexpr auto IPFIX_DST_PORT = 11;
static constexpr auto IPFIX_VLAN_ID = 58;

static constexpr auto IPFIX_PROTOCOL_IP = 4;
static constexpr auto IPFIX_PROTOCOL_TCP = 6;
static constexpr auto IPFIX_PROTOCOL_UDP = 17;
static constexpr auto IPFIX_PROTOCOL_DOT1Q = 144; // TODO(dudoslav): Change

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

// https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
enum class Type: unsigned long {
  IP = IPFIX_PROTOCOL_IP,
  TCP = IPFIX_PROTOCOL_TCP,
  UDP = IPFIX_PROTOCOL_UDP,
  DOT1Q = IPFIX_PROTOCOL_DOT1Q
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

  [[nodiscard]] std::vector<std::byte> values() const {
    struct [[gnu::packed]] {
      uint32_t src;
      uint32_t dst;
      uint16_t protocol;
    } v = {};
    v.src = src; // TODO(dudoslav): Doesn't need htonl, why?
    v.dst = dst;
    v.protocol = htons(IPFIX_PROTOCOL_IP);

    auto bv = std::as_bytes(std::span{&v, 1});

    return {bv.begin(), bv.end()};
  }

  [[nodiscard]] static std::vector<std::byte> fields() {
    static const auto t = std::array{
      htons(IPFIX_SRC_IP4_ADDR),
      htons(IPFIX_LONG),
      htons(IPFIX_DST_IP4_ADDR),
      htons(IPFIX_LONG),
      htons(IPFIX_PROTOCOL_IDENTIFIER),
      htons(IPFIX_SHORT)
    };
    auto s = std::as_bytes(std::span{t});

    return {s.begin(), s.end()};
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

  [[nodiscard]] std::vector<std::byte> values() const {
    struct [[gnu::packed]] {
      uint16_t src;
      uint16_t dst;
      uint16_t protocol;
    } v = {};
    v.src = htons(src);
    v.dst = htons(dst);
    v.protocol = htons(IPFIX_PROTOCOL_TCP);

    auto bv = std::as_bytes(std::span{&v, 1});

    return {bv.begin(), bv.end()};
  }

  [[nodiscard]] static std::vector<std::byte> fields() {
    static const auto t = std::array{
      htons(IPFIX_SRC_PORT),
      htons(IPFIX_SHORT),
      htons(IPFIX_DST_PORT),
      htons(IPFIX_SHORT),
      htons(IPFIX_PROTOCOL_IDENTIFIER),
      htons(IPFIX_SHORT)
    };
    auto s = std::as_bytes(std::span{t});

    return {s.begin(), s.end()};
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

  [[nodiscard]] std::vector<std::byte> values() const {
    struct [[gnu::packed]] {
      uint16_t src;
      uint16_t dst;
      uint16_t protocol;
    } v = {};
    v.src = htons(src);
    v.dst = htons(dst);
    v.protocol = htons(IPFIX_PROTOCOL_UDP);

    auto bv = std::as_bytes(std::span{&v, 1});

    return {bv.begin(), bv.end()};
  }

  [[nodiscard]] static std::vector<std::byte> fields() {
    static const auto t = std::array{
      htons(IPFIX_SRC_PORT),
      htons(IPFIX_SHORT),
      htons(IPFIX_DST_PORT),
      htons(IPFIX_SHORT),
      htons(IPFIX_PROTOCOL_IDENTIFIER),
      htons(IPFIX_SHORT)
    };
    auto s = std::as_bytes(std::span{t});

    return {s.begin(), s.end()};
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

  [[nodiscard]] std::vector<std::byte> values() const {
    struct [[gnu::packed]] {
      uint16_t id;
      uint16_t protocol;
    } v = {};
    v.id = htons(id);
    v.protocol = htons(IPFIX_PROTOCOL_DOT1Q);

    auto bv = std::as_bytes(std::span{&v, 1});

    return {bv.begin(), bv.end()};
  }

  [[nodiscard]] static std::vector<std::byte> fields() {
    static const auto t = std::array{
      htons(IPFIX_VLAN_ID),
      htons(IPFIX_SHORT),
      htons(IPFIX_PROTOCOL_IDENTIFIER),
      htons(IPFIX_SHORT)
    };
    auto s = std::as_bytes(std::span{t});

    return {s.begin(), s.end()};
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

inline std::vector<std::byte> values(const Protocol& protocol) {
  return std::visit([](const auto& v){ return v.values(); }, protocol);
}

inline std::vector<std::byte> fields(const Protocol& protocol) {
  return std::visit([](const auto& v){ return v.fields(); }, protocol);
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

inline std::vector<std::byte> values(const Record& record) {
  auto result = std::vector<std::byte>{};

  for (const auto& p: record) {
    auto v = values(p);
    result.insert(result.end(), v.cbegin(), v.cend());
  }

  return result;
}

inline std::vector<std::byte> fields(const Record& record) {
  auto result = std::vector<std::byte>{};

  for (const auto& p: record) {
    auto f = fields(p);
    result.insert(result.end(), f.cbegin(), f.cend());
  }

  return result;
}

} // namespace Flow
