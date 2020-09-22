#include <serializer.hpp>

#include <algorithm>
#include <array>

#include <arpa/inet.h>

#include <common.hpp>

namespace Flow {

Serializer::Serializer(Flow::Definition def) : _def(def) {}

// BEGIN DIGEST
[[nodiscard]] std::size_t Serializer::digest(const IP& ip) const {
  auto result = 0ul;

  if (_def.ip.src)
    result = combine(result, ip.src);

  if (_def.ip.dst)
    result = combine(result, ip.dst);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const TCP& tcp) const {
  auto result = 0ul;

  if (_def.tcp.src)
    result = combine(result, tcp.src);

  if (_def.tcp.dst)
    result = combine(result, tcp.dst);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const UDP& udp) const {
  auto result = 0ul;

  if (_def.udp.src)
    result = combine(result, udp.src);

  if (_def.udp.dst)
    result = combine(result, udp.dst);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const DOT1Q& dot1q) const {
  auto result = 0ul;

  if (_def.dot1q.id)
    result = combine(result, dot1q.id);

  return result;
}

[[nodiscard]] std::size_t Serializer::digest(const Chain& chain) const {
  auto result = 0ul;
  
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
      htons(IPFIX_SRC_IP4_ADDR),
      htons(IPFIX_LONG)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  if (_def.ip.dst) {
    auto f = Field{
      htons(IPFIX_DST_IP4_ADDR),
      htons(IPFIX_LONG)
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
      htons(IPFIX_SRC_PORT),
      htons(IPFIX_SHORT)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  if (_def.tcp.dst) {
    auto f = Field{
      htons(IPFIX_DST_PORT),
      htons(IPFIX_SHORT)
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
      htons(IPFIX_SRC_PORT),
      htons(IPFIX_SHORT)
    };
    auto fp = reinterpret_cast<std::byte*>(&f);
    std::copy_n(fp, sizeof(f), bkit);
  }

  if (_def.udp.dst) {
    auto f = Field{
      htons(IPFIX_DST_PORT),
      htons(IPFIX_SHORT)
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
      htons(IPFIX_VLAN_ID),
      htons(IPFIX_SHORT)
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
      htons(IPFIX_PROTOCOL_IDENTIFIER),
      htons(IPFIX_SHORT)
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
    htons(IPFIX_PACKET_DELTA_COUNT),
    htons(IPFIX_LONG),
    htons(IPFIX_START_TIME),
    htons(IPFIX_LONG),
    htons(IPFIX_END_TIME),
    htons(IPFIX_LONG)
  };
  auto tp = reinterpret_cast<const std::byte*>(t.data());

  return {tp, tp + t.size() * IPFIX_SHORT};
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
    std::copy_n(vp, sizeof(ip.src), bkit);
  }

  if (_def.ip.dst) {
    auto vp = reinterpret_cast<const std::byte*>(&ip.dst);
    std::copy_n(vp, sizeof(ip.dst), bkit);
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
    std::copy_n(vp, sizeof(src), bkit);
  }

  if (_def.tcp.dst) {
    auto dst = htons(tcp.dst);
    auto vp = reinterpret_cast<const std::byte*>(&dst);
    std::copy_n(vp, sizeof(dst), bkit);
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
    std::copy_n(vp, sizeof(src), bkit);
  }

  if (_def.udp.dst) {
    auto dst = htons(udp.dst);
    auto vp = reinterpret_cast<const std::byte*>(&dst);
    std::copy_n(vp, sizeof(dst), bkit);
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
    std::copy_n(vp, sizeof(id), bkit);
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
    auto ntype = htons(static_cast<std::underlying_type_t<Flow::Type>>(type));
    auto tp = reinterpret_cast<std::byte*>(&ntype);
    std::copy_n(tp, sizeof(ntype), std::back_inserter(vs));

    std::copy(vs.begin(), vs.end(), bkit);
  }

  return result;
}

[[nodiscard]]
Serializer::BufferType Serializer::values(const Properties& properties) const {
  auto result = Serializer::BufferType{};
  auto bkit = std::back_inserter(result);

  auto count = htonl(properties.count);
  auto p = reinterpret_cast<const std::byte*>(&count);
  std::copy_n(p, sizeof(count), bkit);

  auto first_timestamp = htonl(properties.first_timestamp * 1000);
  p = reinterpret_cast<const std::byte*>(&first_timestamp);
  std::copy_n(p, sizeof(first_timestamp), bkit);

  auto last_timestamp = htonl(properties.last_timestamp * 1000);
  p = reinterpret_cast<const std::byte*>(&last_timestamp);
  std::copy_n(p, sizeof(last_timestamp), bkit);

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
