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

  bool should_process() const override {
    return _def.process;
  }

  std::size_t type() const override {
    return ttou(IPFIX::Type::UDP);
  }

  std::size_t digest(const Tins::PDU& pdu) const override {
    auto digest = std::size_t{type()};

    const auto& udp = static_cast<const Tins::UDP&>(pdu);

    if (_def.src)
      digest = combine(digest, udp.sport());

    if (_def.dst)
      digest = combine(digest, udp.dport());

    return digest;
  }

  Buffer fields() const override {
    auto fields = Buffer{};

    if (_def.src) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_SRC_PORT));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_16));
    }

    if (_def.dst) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_DST_PORT));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_16));
    }

    return fields;
  }

  Buffer values(const Tins::PDU& pdu) const override {
    const auto& udp = static_cast<const Tins::UDP&>(pdu);
    auto values = Buffer{};

    if (_def.src) {
      values.push_back_any<std::uint16_t>(htons(udp.sport()));
    }

    if (_def.dst) {
      values.push_back_any<std::uint16_t>(htons(udp.dport()));
    }

    return values;
  }
};

} // namespace Flow
