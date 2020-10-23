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

  std::vector<std::byte> fields() const override {
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

  std::vector<std::byte> values(const Tins::PDU& pdu) const override {
    const auto& tcp = static_cast<const Tins::TCP&>(pdu);
    auto values = std::vector<std::byte>{};
    auto bkit = std::back_inserter(values);

    if (_def.src) {
      uint16_t src = htons(tcp.sport());
      const auto* vp = reinterpret_cast<std::byte*>(&src);
      std::copy_n(vp, IPFIX::TYPE_16, bkit);
    }

    if (_def.dst) {
      uint16_t dst = htons(tcp.dport());
      const auto* vp = reinterpret_cast<std::byte*>(&dst);
      std::copy_n(vp, IPFIX::TYPE_16, bkit);
    }

    uint8_t protocol = IPFIX::PROTOCOL_TCP;
    auto vp = reinterpret_cast<const std::byte*>(&protocol);
    std::copy_n(vp, IPFIX::TYPE_8, bkit);

    return values;
  }
};

} // namespace Flow
