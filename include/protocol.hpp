#pragma once

#include <cstdint>
#include <ctime>
#include <variant>
#include <vector>
#include <array>

#include <common.hpp>

namespace Flow {

static constexpr auto IPFIX_SHORT = 2;
static constexpr auto IPFIX_LONG = 4;

// https://www.iana.org/assignments/ipfix/ipfix.xhtml
static constexpr auto IPFIX_PACKET_DELTA_COUNT = 2;
static constexpr auto IPFIX_PROTOCOL_IDENTIFIER = 4;
static constexpr auto IPFIX_SRC_IP4_ADDR = 8;
static constexpr auto IPFIX_DST_IP4_ADDR = 12;
static constexpr auto IPFIX_SRC_IP6_ADDR = 27;
static constexpr auto IPFIX_DST_IP6_ADDR = 28;
static constexpr auto IPFIX_SRC_PORT = 7;
static constexpr auto IPFIX_DST_PORT = 11;
static constexpr auto IPFIX_VLAN_ID = 58;
static constexpr auto IPFIX_FLOW_START_SECONDS = 150;
static constexpr auto IPFIX_FLOW_END_SECONDS = 151;
static constexpr auto IPFIX_FLOW_START_MICROSECONDS = 154;
static constexpr auto IPFIX_FLOW_END_MICROSECONDS = 155;

// TODO(dudoslav): implement these ipfix fiels
static constexpr auto IPFIX_FLOW_END_REASON = 136;

static constexpr auto IPFIX_PROTOCOL_IP = 4;
static constexpr auto IPFIX_PROTOCOL_IPV6 = 41;
static constexpr auto IPFIX_PROTOCOL_TCP = 6;
static constexpr auto IPFIX_PROTOCOL_UDP = 17;
static constexpr auto IPFIX_PROTOCOL_DOT1Q = 144; // TODO(dudoslav): Change

// https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
enum class Type: short {
  IP = IPFIX_PROTOCOL_IP,
  IPV6 = IPFIX_PROTOCOL_IPV6,
  TCP = IPFIX_PROTOCOL_TCP,
  UDP = IPFIX_PROTOCOL_UDP,
  DOT1Q = IPFIX_PROTOCOL_DOT1Q
};

[[nodiscard]] inline std::size_t ttou(const Type& type) {
  return static_cast<std::underlying_type_t<Type>>(type);
}

/**
 * Metadata used in stroring flow records.
 */
struct Properties {
  std::size_t count;
  timeval first_timestamp;
  timeval last_timestamp;
};

struct IP {
  uint32_t src;
  uint32_t dst;

  [[nodiscard]] static Type type() {
    return Type::IP;
  }
};

struct IPv6 {
  std::array<std::byte, 16> src;
  std::array<std::byte, 16> dst;

  [[nodiscard]] static Type type() {
    return Type::IPV6;
  }
};

struct TCP {
  uint16_t src;
  uint16_t dst;

  [[nodiscard]] static Type type() {
    return Type::TCP;
  }
};

struct UDP {
  uint16_t src;
  uint16_t dst;

  [[nodiscard]] static Type type() {
    return Type::UDP;
  }
};

struct DOT1Q {
  uint16_t id;

  [[nodiscard]] static Type type() {
    return Type::DOT1Q;
  }
};

using Protocol = std::variant<IP, IPv6, TCP, UDP, DOT1Q>;
using Chain = std::vector<Protocol>;
using Record = std::pair<Properties, Chain>;

[[nodiscard]] inline bool tsgeq(timeval f, timeval s) {
  return f.tv_sec == s.tv_sec ? f.tv_usec > s.tv_usec : f.tv_sec > s.tv_sec;
}

[[nodiscard]] inline std::size_t type(const Chain& chain) {
  auto result = 0ul;

  for (const auto& protocol: chain) {
    auto type = std::visit([](const auto& p){
        return p.type(); }, protocol);

    result = combine(result, type);
  }

  return result;
}

} // namespace Flow
