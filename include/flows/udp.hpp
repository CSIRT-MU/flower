#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

namespace Flow {

class UDP : public Flow {
  struct {
    bool process;
    bool src;
    bool dst;
  } _def;

public:

  UDP(const toml::value& config) {
    if (config.contains("udp")) {
      const auto& udp = toml::find(config, "udp");
      _def.process = true;
      _def.src = toml::find_or(udp, "src", false);
      _def.dst = toml::find_or(udp, "dst", false);
    } else {
      _def.process = false;
    }
  }

  bool should_process() override {
    return _def.process;
  }

  std::size_t type() override {
    return ttou(IPFIX::Type::UDP);
  }

  std::size_t digest(const Tins::PDU& pdu) override {
    auto digest = std::size_t{type()};

    const auto& udp = static_cast<const Tins::UDP&>(pdu);

    if (_def.src)
      digest = combine(digest, udp.sport());

    if (_def.dst)
      digest = combine(digest, udp.dport());

    return digest;
  }

  std::vector<std::byte> fields() override {
    auto fields = std::vector<std::byte>{};
    auto bkit = std::back_inserter(fields);

    if (_def.src) {
      auto f = std::array{
        htons(IPFIX::FIELD_SRC_PORT),
        htons(IPFIX::TYPE_16)
      };
      const auto* fp = reinterpret_cast<std::byte*>(f.data());
      std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);
    }

    if (_def.dst) {
      auto f = std::array{
        htons(IPFIX::FIELD_DST_PORT),
        htons(IPFIX::TYPE_16)
      };
      const auto* fp = reinterpret_cast<std::byte*>(f.data());
      std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);
    }

    auto f = std::array{
      htons(IPFIX::FIELD_PROTOCOL_IDENTIFIER),
      htons(IPFIX::TYPE_8)
    };
    const auto* fp = reinterpret_cast<std::byte*>(f.data());
    std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);

    return fields;
  }

  std::vector<std::byte> values(const Tins::PDU& pdu) override {
    const auto& udp = static_cast<const Tins::UDP&>(pdu);
    auto values = std::vector<std::byte>{};
    auto bkit = std::back_inserter(values);

    if (_def.src) {
      uint16_t src = htons(udp.sport());
      const auto* vp = reinterpret_cast<std::byte*>(&src);
      std::copy_n(vp, IPFIX::TYPE_16, bkit);
    }

    if (_def.dst) {
      uint16_t dst = htons(udp.dport());
      const auto* vp = reinterpret_cast<std::byte*>(&dst);
      std::copy_n(vp, IPFIX::TYPE_16, bkit);
    }

    uint8_t protocol = IPFIX::PROTOCOL_UDP;
    auto vp = reinterpret_cast<const std::byte*>(&protocol);
    std::copy_n(vp, IPFIX::TYPE_8, bkit);

    return values;
  }
};

} // namespace Flow