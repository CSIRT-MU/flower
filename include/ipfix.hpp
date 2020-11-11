#pragma once

#include <ctime>
#include <cstdint>

namespace IPFIX {

/* Set IDs */
static constexpr std::uint16_t SET_TEMPLATE = 2;
static constexpr std::uint16_t SET_USER_TEMPLATE = 256;

/* Version */
static constexpr std::uint16_t VERSION = 0x000A;

/* Type sizes */
static constexpr std::uint16_t TYPE_8 = 1;
static constexpr std::uint16_t TYPE_16 = 2;
static constexpr std::uint16_t TYPE_32 = 4;
static constexpr std::uint16_t TYPE_64 = 8;
static constexpr std::uint16_t TYPE_IPV4 = TYPE_32;
static constexpr std::uint16_t TYPE_IPV6 = 16;
static constexpr std::uint16_t TYPE_MAC = 6;
static constexpr std::uint16_t TYPE_SECONDS = TYPE_32;
static constexpr std::uint16_t TYPE_MILLISECONDS = TYPE_64;
static constexpr std::uint16_t TYPE_MICROSECONDS = TYPE_64;
static constexpr std::uint16_t TYPE_LIST = 0xFFFF;

/* Semantics */
static constexpr std::uint8_t SEMANTIC_ORDERED = 0x04;

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
static constexpr std::uint16_t FIELD_SRC_MAC_ADDR = 56;
static constexpr std::uint16_t FIELD_IP_VERSION = 60;
static constexpr std::uint16_t FIELD_DST_MAC_ADDR = 80;
static constexpr std::uint16_t FIELD_FLOW_END_REASON = 136;
static constexpr std::uint16_t FIELD_FLOW_START_SECONDS = 150;
static constexpr std::uint16_t FIELD_FLOW_END_SECONDS = 151;
static constexpr std::uint16_t FIELD_FLOW_START_MILLISECONDS = 152;
static constexpr std::uint16_t FIELD_FLOW_END_MILLISECONDS = 153;
static constexpr std::uint16_t FIELD_FLOW_START_MICROSECONDS = 154;
static constexpr std::uint16_t FIELD_FLOW_END_MICROSECONDS = 155;
static constexpr std::uint16_t FIELD_ETHERNET_TYPE = 256;
static constexpr std::uint16_t FIELD_SUB_TEMPLATE_MULTI_LIST = 293;
static constexpr std::uint16_t FIELD_MPLS_LABEL_STACK_SECTION = 316;
static constexpr std::uint16_t FIELD_LAYER2_SEGEMENT_ID = 351;

/* Protocol identifiers */
/* https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml */
static constexpr std::uint8_t PROTOCOL_TCP = 6;
static constexpr std::uint8_t PROTOCOL_UDP = 17;
static constexpr std::uint8_t PROTOCOL_GRE = 47;
static constexpr std::uint8_t PROTOCOL_MPLS = 137;

/* Flow end reasons */
static constexpr std::uint8_t REASON_IDLE = 0x01;
static constexpr std::uint8_t REASON_ACTIVE = 0x02;
static constexpr std::uint8_t REASON_FORCED = 0x04;

/* IPFIX type enum */
enum class Type {
  IP,
  IPV6,
  TCP = PROTOCOL_TCP,
  UDP = PROTOCOL_UDP,
  DOT1Q,
  MPLS = PROTOCOL_MPLS,
  VXLAN,
  GRE = PROTOCOL_GRE,
  ETHERNET
};

struct Properties {
  std::size_t count;
  timeval flow_start;
  timeval flow_end;
};

/* Cast from Type to uint8_t */
[[nodiscard]]
constexpr std::underlying_type_t<Type>
ttou(const Type &type)
{
  return static_cast<std::underlying_type_t<Type>>(type);
}

} // namespace IPFIX
