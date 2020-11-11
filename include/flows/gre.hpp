#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

namespace Flow {

class GRE : public Flow {
  struct {
    bool process;
  } _def;

public:

  GRE(const toml::value& config) {
    if (config.contains("gre")) {
      // const auto& gre = toml::find(config, "gre");
      _def.process = true;
    } else {
      _def.process = false;
    }
  }

  bool should_process() const override {
    return _def.process;
  }

  std::size_t type() const override {
    return ttou(IPFIX::Type::GRE);
  }

  std::size_t digest(const Tins::PDU& pdu) const override {
    auto digest = std::size_t{type()};

    const auto& gre = static_cast<const Protocols::GREPDU&>(pdu);

    digest = combine(digest, gre.protocol());

    return digest;
  }

  Buffer fields() const override {
    auto fields = Buffer{};

    fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_ETHERNET_TYPE));
    fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_16));

    return fields;
  }

  Buffer values(const Tins::PDU& pdu) const override {
    const auto& gre = static_cast<const Protocols::GREPDU&>(pdu);
    auto values = Buffer{};

    values.push_back_any<std::uint16_t>(htons(gre.protocol()));

    return values;
  }
};

} // namespace Flow
