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

  std::vector<std::byte> fields() const override {
    auto fields = std::vector<std::byte>{};
    auto bkit = std::back_inserter(fields);

    if (_def.vni) {
      auto f = std::array{
        htons(IPFIX::FIELD_LAYER2_SEGEMENT_ID),
        htons(IPFIX::TYPE_64)
      };
      auto fp = reinterpret_cast<std::byte*>(&f);
      std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);
    }

    return fields;
  }

  std::vector<std::byte> values(const Tins::PDU& pdu) const override {
    const auto& vxlan = static_cast<const Protocols::VXLAN&>(pdu);
    auto values = std::vector<std::byte>{};
    auto bkit = std::back_inserter(values);

    if (_def.vni) {
      uint64_t segment_id = (uint64_t{0x01} << 56) + vxlan.vni();
      segment_id = htonT(segment_id);
      auto vp = reinterpret_cast<const std::byte*>(&segment_id);
      std::copy_n(vp, IPFIX::TYPE_64, bkit);
    }

    return values;
  }
};

} // namespace Flow
