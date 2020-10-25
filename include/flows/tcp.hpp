#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

namespace Flow {

class TCP : public Flow {
  struct {
    bool process;
    bool src;
    bool dst;
  } _def;

public:

  TCP(const toml::value& config) {
    if (config.contains("tcp")) {
      const auto& tcp = toml::find(config, "tcp");
      _def.process = true;
      _def.src = toml::find_or(tcp, "src", false);
      _def.dst = toml::find_or(tcp, "dst", false);
    } else {
      _def.process = false;
    }
  }

  bool should_process() const override {
    return _def.process;
  }

  std::size_t type() const override {
    return ttou(IPFIX::Type::TCP);
  }

  std::size_t digest(const Tins::PDU& pdu) const override {
    auto digest = std::size_t{type()};

    const auto& tcp = static_cast<const Tins::TCP&>(pdu);

    if (_def.src)
      digest = combine(digest, tcp.sport());

    if (_def.dst)
      digest = combine(digest, tcp.dport());

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
    const auto& tcp = static_cast<const Tins::TCP&>(pdu);
    auto values = Buffer{};

    if (_def.src) {
      values.push_back_any<std::uint16_t>(htons(tcp.sport()));
    }

    if (_def.dst) {
      values.push_back_any<std::uint16_t>(htons(tcp.dport()));
    }

    return values;
  }
};

} // namespace Flow
