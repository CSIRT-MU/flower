#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

namespace Flow {

class IPV6 : public Flow {
  struct {
    bool process;
    bool src;
    bool dst;
  } _def;

public:

  IPV6(const toml::value& config) {
    if (config.contains("ipv6")) {
      const auto& ipv6 = toml::find(config, "ipv6");
      _def.process = true;
      _def.src = toml::find_or(ipv6, "src", false);
      _def.dst = toml::find_or(ipv6, "dst", false);
    } else {
      _def.process = false;
    }
  }

  bool should_process() const override {
    return _def.process;
  }

  std::size_t type() const override {
    return ttou(IPFIX::Type::IPV6);
  }

  std::size_t digest(const Tins::PDU& pdu) const override {
    auto digest = std::size_t{type()};

    const auto& ipv6 = static_cast<const Tins::IPv6&>(pdu);

    if (_def.src) {
      for (const auto& b : ipv6.src_addr()) {
        digest = combine(digest, b);
      }
    }

    if (_def.dst) {
      for (const auto& b : ipv6.dst_addr()) {
        digest = combine(digest, b);
      }
    }

    digest = combine(digest, ipv6.next_header());

    return digest;
  }

  Buffer fields() const override {
    auto fields = Buffer{};

    if (_def.src) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_SRC_IP6_ADDR));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_IPV6));
    }

    if (_def.dst) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_DST_IP6_ADDR));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_IPV6));
    }

    fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_IP_VERSION));
    fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_8));

    fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_PROTOCOL_IDENTIFIER));
    fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_8));

    return fields;
  }

  Buffer values(const Tins::PDU& pdu) const override {
    const auto& ipv6 = static_cast<const Tins::IPv6&>(pdu);
    auto values = Buffer{};
    auto bkit = std::back_inserter(values);

    if (_def.src) {
      std::copy_n(reinterpret_cast<std::byte*>(ipv6.src_addr().begin()),
          IPFIX::TYPE_IPV6, bkit);
    }

    if (_def.dst) {
      std::copy_n(reinterpret_cast<std::byte*>(ipv6.dst_addr().begin()),
          IPFIX::TYPE_IPV6, bkit);
    }

    values.push_back_any<std::uint8_t>(ipv6.version());
    values.push_back_any<std::uint8_t>(ipv6.next_header());

    return values;
  }
};

};
