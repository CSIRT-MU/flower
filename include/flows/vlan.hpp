#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

namespace Flow {

class VLAN : public Flow {
  struct {
    bool process;
    bool id;
  } _def;

public:

  VLAN(const toml::value& config) {
    if (config.contains("vlan")) {
      const auto& vlan = toml::find(config, "vlan");
      _def.process = true;
      _def.id = toml::find_or(vlan, "id", false);
    } else {
      _def.process = false;
    }
  }

  bool should_process() const override {
    return _def.process;
  }

  std::size_t type() const override {
    return ttou(IPFIX::Type::DOT1Q);
  }

  std::size_t digest(const Tins::PDU& pdu) const override {
    auto digest = std::size_t{type()};

    const auto& vlan = static_cast<const Tins::Dot1Q&>(pdu);

    if (_def.id)
      digest = combine(digest, vlan.id());

    return digest;
  }

  Buffer fields() const override {
    auto fields = Buffer{};

    if (_def.id) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_VLAN_ID));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_16));
    }

    return fields;
  }

  Buffer values(const Tins::PDU& pdu) const override {
    const auto& dot1q = static_cast<const Tins::Dot1Q&>(pdu);
    auto values = Buffer{};

    if (_def.id) {
      values.push_back_any<std::uint16_t>(htons(dot1q.id()));
    }

    return values;
  }
};

} // namespace Flow
