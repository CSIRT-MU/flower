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

  std::vector<std::byte> fields() const override {
    auto fields = std::vector<std::byte>{};
    auto bkit = std::back_inserter(fields);

    if (_def.id) {
      auto f = std::array{
        htons(IPFIX::FIELD_VLAN_ID),
        htons(IPFIX::TYPE_16)
      };
      auto fp = reinterpret_cast<std::byte*>(&f);
      std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);
    }

    return fields;
  }

  std::vector<std::byte> values(const Tins::PDU& pdu) const override {
    const auto& dot1q = static_cast<const Tins::Dot1Q&>(pdu);
    auto values = std::vector<std::byte>{};
    auto bkit = std::back_inserter(values);

    if (_def.id) {
      auto id = htons(dot1q.id());
      auto vp = reinterpret_cast<const std::byte*>(&id);
      std::copy_n(vp, IPFIX::TYPE_16, bkit);
    }

    return values;
  }
};

} // namespace Flow
