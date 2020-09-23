#pragma once

#include <cstdint>
#include <variant>
#include <vector>

#include <common.hpp>

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
static constexpr auto IPFIX_FLOW_START_SECONDS = 150;
static constexpr auto IPFIX_FLOW_END_SECONDS = 151;

// TODO(dudoslav): implement these ipfix fiels
static constexpr auto IPFIX_FLOW_START_MILLISECONDS = 152;
static constexpr auto IPFIX_FLOW_END_MILLISECONDS = 153;
static constexpr auto IPFIX_FLOW_END_REASON = 136;

static constexpr auto IPFIX_PROTOCOL_IP = 4;
static constexpr auto IPFIX_PROTOCOL_TCP = 6;
static constexpr auto IPFIX_PROTOCOL_UDP = 17;
static constexpr auto IPFIX_PROTOCOL_DOT1Q = 144; // TODO(dudoslav): Change

// https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml
enum class Type: short {
  IP = IPFIX_PROTOCOL_IP,
  TCP = IPFIX_PROTOCOL_TCP,
  UDP = IPFIX_PROTOCOL_UDP,
  DOT1Q = IPFIX_PROTOCOL_DOT1Q
};

/**
 * Metadata used in stroring flow records.
 */
struct Properties {
  std::size_t count;
  unsigned int first_timestamp;
  unsigned int last_timestamp;
};

struct IP {
  uint32_t src;
  uint32_t dst;

  [[nodiscard]] static Type type() {
    return Type::IP;
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

using Protocol = std::variant<IP, TCP, UDP, DOT1Q>;
using Chain = std::vector<Protocol>;

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
