#pragma once

namespace IPFIX {

/* Type sizes */
static constexpr std::uint16_t TYPE_8 = 1;
static constexpr std::uint16_t TYPE_16 = 2;
static constexpr std::uint16_t TYPE_32 = 4;
static constexpr std::uint16_t TYPE_64 = 8;
static constexpr std::uint16_t TYPE_IPV4 = TYPE_32;
static constexpr std::uint16_t TYPE_IPV6 = 16;
static constexpr std::uint16_t TYPE_SECONDS = TYPE_32;
static constexpr std::uint16_t TYPE_MILLISECONDS = TYPE_64;
static constexpr std::uint16_t TYPE_MICROSECONDS = TYPE_64;

/* Template/Record fields */
/* https://www.iana.org/assignments/ipfix/ipfix.xhtml */
static constexpr std::uint16_t FIELD_PACKET_DELTA_COUNT = 2;
static constexpr std::uint16_t FIELD_PROTOCOL_IDENTIFIER = 4;
static constexpr std::uint16_t FIELD_SRC_IP4_ADDR = 8;
static constexpr std::uint16_t FIELD_DST_IP4_ADDR = 12;
static constexpr std::uint16_t FIELD_SRC_IP6_ADDR = 27;
static constexpr std::uint16_t FIELD_DST_IP6_ADDR = 28;
static constexpr std::uint16_t FIELD_SRC_PORT = 7;
static constexpr std::uint16_t FIELD_DST_PORT = 11;
static constexpr std::uint16_t FIELD_VLAN_ID = 58;
static constexpr std::uint16_t FIELD_FLOW_START_SECONDS = 150;
static constexpr std::uint16_t FIELD_FLOW_END_SECONDS = 151;
static constexpr std::uint16_t FIELD_FLOW_START_MILLISECONDS = 152;
static constexpr std::uint16_t FIELD_FLOW_END_MILLISECONDS = 153;
static constexpr std::uint16_t FIELD_FLOW_START_MICROSECONDS = 154;
static constexpr std::uint16_t FIELD_FLOW_END_MICROSECONDS = 155;
static constexpr std::uint16_t FIELD_LAYER2_SEGEMENT_ID = 351;

/* Protocol identifiers */
/* https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml */
static constexpr std::uint8_t PROTOCOL_IP = 4;
static constexpr std::uint8_t PROTOCOL_IPV6 = 41;
static constexpr std::uint8_t PROTOCOL_TCP = 6;
static constexpr std::uint8_t PROTOCOL_UDP = 17;
static constexpr std::uint8_t PROTOCOL_MPLS = 137;
static constexpr std::uint8_t PROTOCOL_DOT1Q = 144; // TODO(dudoslav): Change
static constexpr std::uint8_t PROTOCOL_VXLAN = 145; // TODO(dudoslav): Change

/* IPFIX type enum */
enum class Type : std::uint8_t {
  IP = PROTOCOL_IP,
  IPV6 = PROTOCOL_IPV6,
  TCP = PROTOCOL_TCP,
  UDP = PROTOCOL_UDP,
  DOT1Q = PROTOCOL_DOT1Q,
  MPLS = PROTOCOL_MPLS,
  VXLAN = PROTOCOL_VXLAN
};

/* Cast from Type to uint8_t */
[[nodiscard]] inline std::uint8_t ttou(const Type &type) {
  return static_cast<std::underlying_type_t<Type>>(type);
}

} // namespace IPFIX
