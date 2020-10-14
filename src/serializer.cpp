#include <serializer.hpp>

#include <cstdio>

#include <algorithm>
#include <array>

#include <endian.h>
#include <arpa/inet.h>

#include <common.hpp>
#include <ipfix.hpp>

namespace Flow {
  
template <typename T>
static constexpr T htonT (T value) noexcept
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
  char* ptr = reinterpret_cast<char*>(&value);
  std::reverse(ptr, ptr + sizeof(T));
#endif
  return value;
}

void Serializer::set_definition(Flow::Definition def) {
  _def = def;
}

// BEGIN DIGEST
[[nodiscard]] std::size_t Serializer::digest(const IP& ip) const {
  std::size_t result = IPFIX::ttou(ip.type());

  if (_def.ip.src)
    result = combine(result, ip.src);

  if (_def.ip.dst)
    result = combine(result, ip.dst);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const IPv6& ipv6) const {
  std::size_t result = IPFIX::ttou(ipv6.type());

  if (_def.ipv6.src) {
    for (const auto& b: ipv6.src) {
      result = combine(result, b);
    }
  }

  if (_def.ipv6.dst) {
    for (const auto& b: ipv6.dst) {
      result = combine(result, b);
    }
  }

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const TCP& tcp) const {
  std::size_t result = IPFIX::ttou(tcp.type());

  if (_def.tcp.src)
    result = combine(result, tcp.src);

  if (_def.tcp.dst)
    result = combine(result, tcp.dst);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const UDP& udp) const {
  std::size_t result = IPFIX::ttou(udp.type());

  if (_def.udp.src)
    result = combine(result, udp.src);

  if (_def.udp.dst)
    result = combine(result, udp.dst);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const DOT1Q& dot1q) const {
  std::size_t result = IPFIX::ttou(dot1q.type());

  if (_def.dot1q.id)
    result = combine(result, dot1q.id);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const MPLS& mpls) const {
  std::size_t result = IPFIX::ttou(mpls.type());

  if (_def.mpls.label)
    result = combine(result, mpls.label);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const VXLAN& vxlan) const {
  std::size_t result = IPFIX::ttou(vxlan.type());

  if (_def.vxlan.vni)
    result = combine(result, vxlan.vni);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const Chain& chain) const {
  std::size_t result = 0ul;
  
  for (const auto& protocol: chain) {
    auto hash = std::visit([&](const auto& p){
        return digest(p); }, protocol);

    result = combine(result, hash);
  }

  return result;
}
// END DIGEST

// BEGIN FIELDS
struct [[gnu::packed]] Field {
  uint16_t type;
  uint16_t size;
};

[[nodiscard]]
Serializer::BufferType Serializer::fields([[maybe_unused]] const IP& ip) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.ip.src) {
    auto f = Field{
      htons(IPFIX::FIELD_SRC_IP4_ADDR),
      htons(IPFIX::TYPE_IPV4)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  if (_def.ip.dst) {
    auto f = Field{
      htons(IPFIX::FIELD_DST_IP4_ADDR),
      htons(IPFIX::TYPE_IPV4)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::fields([[maybe_unused]] const IPv6& ipv6) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.ipv6.src) {
    auto f = Field{
      htons(IPFIX::FIELD_SRC_IP6_ADDR),
      htons(IPFIX::TYPE_IPV6)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  if (_def.ipv6.dst) {
    auto f = Field{
      htons(IPFIX::FIELD_DST_IP6_ADDR),
      htons(IPFIX::TYPE_IPV6)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::fields([[maybe_unused]] const TCP& tcp) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.tcp.src) {
    auto f = Field{
      htons(IPFIX::FIELD_SRC_PORT),
      htons(IPFIX::TYPE_16)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  if (_def.tcp.dst) {
    auto f = Field{
      htons(IPFIX::FIELD_DST_PORT),
      htons(IPFIX::TYPE_16)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::fields([[maybe_unused]] const UDP& udp) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.udp.src) {
    auto f = Field{
      htons(IPFIX::FIELD_SRC_PORT),
      htons(IPFIX::TYPE_16)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  if (_def.udp.dst) {
    auto f = Field{
      htons(IPFIX::FIELD_DST_PORT),
      htons(IPFIX::TYPE_16)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::fields([[maybe_unused]] const DOT1Q& dot1q) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.dot1q.id) {
    auto f = Field{
      htons(IPFIX::FIELD_VLAN_ID),
      htons(IPFIX::TYPE_16)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  return result;
}

[[nodiscard]] Serializer::BufferType Serializer::fields([[maybe_unused]] const MPLS& mpls) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  // TODO(dudoslav): finish
  if (_def.mpls.label) {
    auto f = Field{
      htons(IPFIX::FIELD_VLAN_ID),
      htons(IPFIX::TYPE_16)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  return result;
}

[[nodiscard]] Serializer::BufferType Serializer::fields([[maybe_unused]] const VXLAN& vxlan) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.vxlan.vni) {
    auto f = Field{
      htons(IPFIX::FIELD_LAYER2_SEGEMENT_ID),
      htons(IPFIX::TYPE_64)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::fields(const Chain& chain) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  for (const auto& protocol: chain) {
    auto fs = std::visit([&](const auto& p){
        return fields(p); }, protocol);

    auto f = Field{
      htons(IPFIX::FIELD_PROTOCOL_IDENTIFIER),
      htons(IPFIX::TYPE_8)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), std::back_inserter(fs));

    std::copy(fs.begin(), fs.end(), bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::fields([[maybe_unused]] const Properties& properties) const {
  static const auto t = std::array{
    htons(IPFIX::FIELD_PACKET_DELTA_COUNT),
    htons(IPFIX::TYPE_64),
    htons(IPFIX::FIELD_FLOW_START_SECONDS),
    htons(IPFIX::TYPE_SECONDS),
    htons(IPFIX::FIELD_FLOW_END_SECONDS),
    htons(IPFIX::TYPE_SECONDS),
    htons(IPFIX::FIELD_FLOW_START_MILLISECONDS),
    htons(IPFIX::TYPE_MILLISECONDS),
    htons(IPFIX::FIELD_FLOW_END_MILLISECONDS),
    htons(IPFIX::TYPE_MILLISECONDS)
  };
  auto tp = reinterpret_cast<const std::byte*>(t.data());

  return {tp, tp + t.size() * IPFIX::TYPE_16};
}

[[nodiscard]]
Serializer::BufferType Serializer::fields(const Chain& chain, const Properties& properties) const {
  auto result = fields(properties);
  auto rf = fields(chain);
  std::copy(rf.begin(), rf.end(), std::back_inserter(result));
  return result;
}
// END FIELDS

// BEGIN VALUES
[[nodiscard]]
Serializer::BufferType Serializer::values(const IP& ip) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.ip.src) {
    auto vp = reinterpret_cast<const std::byte*>(&ip.src);
    std::copy_n(vp, IPFIX::TYPE_IPV4, bkit);
  }

  if (_def.ip.dst) {
    auto vp = reinterpret_cast<const std::byte*>(&ip.dst);
    std::copy_n(vp, IPFIX::TYPE_IPV4, bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const IPv6& ipv6) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.ipv6.src) {
    auto vp = reinterpret_cast<const std::byte*>(ipv6.src.data());
    std::copy_n(vp, IPFIX::TYPE_IPV6, bkit);
  }

  if (_def.ipv6.dst) {
    auto vp = reinterpret_cast<const std::byte*>(ipv6.dst.data());
    std::copy_n(vp, IPFIX::TYPE_IPV6, bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const TCP& tcp) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.tcp.src) {
    auto src = htons(tcp.src);
    auto vp = reinterpret_cast<const std::byte*>(&src);
    std::copy_n(vp, IPFIX::TYPE_16, bkit);
  }

  if (_def.tcp.dst) {
    auto dst = htons(tcp.dst);
    auto vp = reinterpret_cast<const std::byte*>(&dst);
    std::copy_n(vp, IPFIX::TYPE_16, bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const UDP& udp) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.udp.src) {
    auto src = htons(udp.src);
    auto vp = reinterpret_cast<const std::byte*>(&src);
    std::copy_n(vp, IPFIX::TYPE_16, bkit);
  }

  if (_def.udp.dst) {
    auto dst = htons(udp.dst);
    auto vp = reinterpret_cast<const std::byte*>(&dst);
    std::copy_n(vp, IPFIX::TYPE_16, bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const DOT1Q& dot1q) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  if (_def.dot1q.id) {
    auto id = htons(dot1q.id);
    auto vp = reinterpret_cast<const std::byte*>(&id);
    std::copy_n(vp, IPFIX::TYPE_16, bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const MPLS& mpls) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  // TODO(dudoslav): finish
  if (_def.mpls.label) {
    auto id = htons(mpls.label);
    auto vp = reinterpret_cast<const std::byte*>(&id);
    std::copy_n(vp, IPFIX::TYPE_16, bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const VXLAN& vxlan) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  // TODO(dudoslav): finish
  if (_def.vxlan.vni) {
    uint64_t segment_id = (uint64_t{0x01} << 56) + vxlan.vni;
    segment_id = htonT(segment_id);
    auto vp = reinterpret_cast<const std::byte*>(&segment_id);
    std::copy_n(vp, IPFIX::TYPE_64, bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const Chain& chain) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  for (const auto& protocol: chain) {
    auto vs = std::visit([&](const auto& p){
        return values(p); }, protocol);

    auto type = std::visit([](const auto&p){
        return p.type(); }, protocol);
    auto ntype = IPFIX::ttou(type);
    auto tp = reinterpret_cast<std::byte*>(&ntype);
    std::copy_n(tp, IPFIX::TYPE_8, std::back_inserter(vs));

    std::copy(vs.begin(), vs.end(), bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const Properties& properties) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  auto count = htonT(properties.count);
  auto p = reinterpret_cast<const std::byte*>(&count);
  std::copy_n(p, IPFIX::TYPE_64, bkit);

  std::uint32_t first_timestamp_sec = htonl(properties.first_timestamp.tv_sec);
  p = reinterpret_cast<const std::byte*>(&first_timestamp_sec);
  std::copy_n(p, IPFIX::TYPE_SECONDS, bkit);

  std::uint32_t last_timestamp_sec = htonl(properties.last_timestamp.tv_sec);
  p = reinterpret_cast<const std::byte*>(&last_timestamp_sec);
  std::copy_n(p, IPFIX::TYPE_SECONDS, bkit);

  std::uint64_t first_timestamp_msec = htonT(
      properties.first_timestamp.tv_sec * 1000
      + properties.first_timestamp.tv_usec / 1000);
  p = reinterpret_cast<const std::byte*>(&first_timestamp_msec);
  std::copy_n(p, IPFIX::TYPE_MILLISECONDS, bkit);

  std::uint64_t last_timestamp_msec = htonT(
      properties.last_timestamp.tv_sec * 1000
      + properties.last_timestamp.tv_usec / 1000);
  p = reinterpret_cast<const std::byte*>(&last_timestamp_msec);
  std::copy_n(p, IPFIX::TYPE_MILLISECONDS, bkit);

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const Chain& chain, const Properties& properties) const {
  auto result = values(properties);
  auto rv = values(chain);
  std::copy(rv.begin(), rv.end(), std::back_inserter(result));
  return result;
}
// END VALUES

} // namespace Flow
