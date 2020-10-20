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

  bool should_process() override {
    return _def.process;
  }

  std::size_t type() override {
    return ttou(IPFIX::Type::IPV6);
  }

  std::size_t digest(const Tins::PDU& pdu) override {
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

    return digest;
  }

  std::vector<std::byte> fields() override {
    auto fields = std::vector<std::byte>{};
    auto bkit = std::back_inserter(fields);

    if (_def.src) {
      auto f = std::array{
        htons(IPFIX::FIELD_SRC_IP6_ADDR),
        htons(IPFIX::TYPE_IPV6)
      };
      const auto* fp = reinterpret_cast<std::byte*>(f.data());
      std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);
    }

    if (_def.dst) {
      auto f = std::array{
        htons(IPFIX::FIELD_DST_IP6_ADDR),
        htons(IPFIX::TYPE_IPV6)
      };
      const auto* fp = reinterpret_cast<std::byte*>(f.data());
      std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);
    }

    auto f = std::array{
      htons(IPFIX::FIELD_IP_VERSION),
      htons(IPFIX::TYPE_8)
    };
    const auto* fp = reinterpret_cast<std::byte*>(f.data());
    std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);

    return fields;
  }

  std::vector<std::byte> values(const Tins::PDU& pdu) override {
    const auto& ipv6 = static_cast<const Tins::IPv6&>(pdu);
    auto values = std::vector<std::byte>{};
    auto bkit = std::back_inserter(values);

    if (_def.src) {
      std::copy_n(reinterpret_cast<std::byte*>(ipv6.src_addr().begin()),
          IPFIX::TYPE_IPV6, bkit);
    }

    if (_def.dst) {
      std::copy_n(reinterpret_cast<std::byte*>(ipv6.dst_addr().begin()),
          IPFIX::TYPE_IPV6, bkit);
    }

    uint8_t version = htonT(ipv6.version());
    const auto* vp = reinterpret_cast<std::byte*>(&version);
    std::copy_n(vp, IPFIX::TYPE_8, bkit);

    return values;
  }
};

};
