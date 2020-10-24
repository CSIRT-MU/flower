#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

namespace Flow {

class IP : public Flow {
  struct {
    bool process;
    bool src;
    bool dst;
  } _def;

public:

  IP(const toml::value& config) {
    if (config.contains("ip")) {
      const auto& ip = toml::find(config, "ip");
      _def.process = true;
      _def.src = toml::find_or(ip, "src", false);
      _def.dst = toml::find_or(ip, "dst", false);
    } else {
      _def.process = false;
    }
  }

  bool should_process() const override {
    return _def.process;
  }

  std::size_t type() const override {
    return ttou(IPFIX::Type::IP);
  }

  std::size_t digest(const Tins::PDU& pdu) const override {
    auto digest = std::size_t{type()};

    const auto& ip = static_cast<const Tins::IP&>(pdu);

    if (_def.src)
      digest = combine(digest, ip.src_addr());

    if (_def.dst)
      digest = combine(digest, ip.dst_addr());

    return digest;
  }

  Buffer fields() const override {
    auto fields = Buffer{};

    if (_def.src) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_SRC_IP4_ADDR));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_IPV4));
    }

    if (_def.dst) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_DST_IP4_ADDR));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_IPV4));
    }

    fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_IP_VERSION));
    fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_8));

    fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_PROTOCOL_IDENTIFIER));
    fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_8));

    return fields;
  }

  Buffer values(const Tins::PDU& pdu) const override {
    const auto& ip = static_cast<const Tins::IP&>(pdu);
    auto values = Buffer{};

    if (_def.src) {
      values.push_back_any<std::uint32_t>(ip.src_addr());
    }

    if (_def.dst) {
      values.push_back_any<std::uint32_t>(ip.dst_addr());
    }

    values.push_back_any<std::uint8_t>(ip.version());
    values.push_back_any<std::uint8_t>(ip.protocol());

    return values;
  }
};

} // namespace Flow
