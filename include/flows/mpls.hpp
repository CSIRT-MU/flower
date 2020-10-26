#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

namespace Flow {

class MPLS : public Flow {
  struct {
    bool process;
  } _def;

public:

  MPLS(const toml::value& config) {
    if (config.contains("mpls")) {
      // const auto& mpls = toml::find(config, "mpls");
      _def.process = true;
    } else {
      _def.process = false;
    }
  }

  bool should_process() const override {
    return _def.process;
  }

  std::size_t type() const override {
    return ttou(IPFIX::Type::MPLS);
  }

  std::size_t digest(const Tins::PDU& pdu) const override {
    auto digest = std::size_t{type()};

    const auto& mpls = static_cast<const Tins::MPLS&>(pdu);

    digest = combine(digest, mpls.label());

    return digest;
  }

  Buffer fields() const override {
    auto fields = Buffer{};

    fields.push_back_any<std::uint16_t>(
        htons(IPFIX::FIELD_MPLS_LABEL_STACK_SECTION));
    fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_32));

    return fields;
  }

  Buffer values(const Tins::PDU& pdu) const override {
    const auto& mpls = static_cast<const Tins::MPLS&>(pdu);
    auto values = Buffer{};

    values.push_back_any<std::uint32_t>(htonl(mpls.label()));

    return values;
  }
};

} // namespace Flow
