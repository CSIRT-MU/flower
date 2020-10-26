#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

namespace Flow {

class ETHERNET : public Flow {
  struct {
    bool process;
    bool src;
    bool dst;
  } _def;

public:

  ETHERNET(const toml::value& config) {
    if (config.contains("ethernet")) {
      const auto& ethernet = toml::find(config, "ethernet");
      _def.process = true;
      _def.src = toml::find_or(ethernet, "src", false);
      _def.dst = toml::find_or(ethernet, "dst", false);
    } else {
      _def.process = false;
    }
  }

  bool should_process() const override {
    return _def.process;
  }

  std::size_t type() const override {
    return ttou(IPFIX::Type::ETHERNET);
  }

  std::size_t digest(const Tins::PDU& pdu) const override {
    auto digest = std::size_t{type()};

    const auto& ethernet = static_cast<const Tins::EthernetII&>(pdu);

    if (_def.src) {
      for (const auto& b : ethernet.src_addr()) {
        digest = combine(digest, b);
      }
    }

    if (_def.dst) {
      for (const auto& b : ethernet.dst_addr()) {
        digest = combine(digest, b);
      }
    }

    digest = combine(digest, ethernet.payload_type());

    return digest;
  }

  Buffer fields() const override {
    auto fields = Buffer{};

    if (_def.src) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_SRC_MAC_ADDR));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_MAC));
    }

    if (_def.dst) {
      fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_DST_MAC_ADDR));
      fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_MAC));
    }

    fields.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_ETHERNET_TYPE));
    fields.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_16));

    return fields;
  }

  Buffer values(const Tins::PDU& pdu) const override {
    const auto& ethernet = static_cast<const Tins::EthernetII&>(pdu);
    auto values = Buffer{};
    auto bkit = std::back_inserter(values);

    if (_def.src) {
      std::copy_n(reinterpret_cast<std::byte*>(ethernet.src_addr().begin()),
          IPFIX::TYPE_MAC, bkit);
    }

    if (_def.dst) {
      std::copy_n(reinterpret_cast<std::byte*>(ethernet.dst_addr().begin()),
          IPFIX::TYPE_MAC, bkit);
    }

    values.push_back_any<std::uint16_t>(htons(ethernet.payload_type()));

    return values;
  }
};
} // namespace Flow
