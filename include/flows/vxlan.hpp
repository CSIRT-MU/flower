#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

#include <protocols/vxlan.hpp>

namespace Flow {

class VXLAN : public Flow {
  struct {
    bool process;
    bool vni;
  } _def;

public:

  VXLAN(const toml::value& config) {
    if (config.contains("vxlan")) {
      const auto& vxlan = toml::find(config, "vxlan");
      _def.process = true;
      _def.vni = toml::find_or(vxlan, "vni", false);
    } else {
      _def.process = false;
    }
  }

  bool should_process() const override {
    return _def.process;
  }

  std::size_t type() const override {
    return ttou(IPFIX::Type::VXLAN);
  }

  std::size_t digest(const Tins::PDU& pdu) const override {
    auto digest = std::size_t{type()};

    const auto& vxlan = static_cast<const Protocols::VXLAN&>(pdu);

    if (_def.vni)
      digest = combine(digest, vxlan.vni());

    return digest;
  }

  Buffer fields() const override {
    auto fields = Buffer{};

    if (_def.vni) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_LAYER2_SEGEMENT_ID));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_64));
    }

    return fields;
  }

  Buffer values(const Tins::PDU& pdu) const override {
    const auto& vxlan = static_cast<const Protocols::VXLAN&>(pdu);
    auto values = Buffer{};

    if (_def.vni) {
      values.push_back_any<std::uint64_t>(
          htonT((uint64_t{0x01} << 56) + vxlan.vni()));
    }

    return values;
  }
};

} // namespace Flow
