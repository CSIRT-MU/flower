#pragma once

#include <toml.hpp>

#include <flows/flow.hpp>
#include <ipfix.hpp>
#include <common.hpp>

namespace Flow {

class IP : public Flow {
  struct {
    bool process;
    bool src;
    bool dst;
  } _def;

public:

  IP(const toml::value& config) {
    if (config.contains("ip")) {
      const auto& ip = toml::find(config, "ip");
      _def.process = true;
      _def.src = toml::find_or(ip, "src", false);
      _def.dst = toml::find_or(ip, "dst", false);
    } else {
      _def.process = false;
    }
  }

  bool should_process() override {
    return _def.process;
  }

  std::size_t type() override {
    return ttou(IPFIX::Type::IP);
  }

  std::size_t digest(const Tins::PDU& pdu) override {
    auto digest = std::size_t{type()};

    const auto& ip = static_cast<const Tins::IP&>(pdu);

    if (_def.src)
      digest = combine(digest, ip.src_addr());

    if (_def.dst)
      digest = combine(digest, ip.dst_addr());

    return digest;
  }

  std::vector<std::byte> fields() override {
    auto fields = std::vector<std::byte>{};
    auto bkit = std::back_inserter(fields);

    if (_def.src) {
      auto f = std::array{
        htons(IPFIX::FIELD_SRC_IP4_ADDR),
        htons(IPFIX::TYPE_IPV4)
      };
      const auto* fp = reinterpret_cast<std::byte*>(f.data());
      std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);
    }

    if (_def.dst) {
      auto f = std::array{
        htons(IPFIX::FIELD_DST_IP4_ADDR),
        htons(IPFIX::TYPE_IPV4)
      };
      const auto* fp = reinterpret_cast<std::byte*>(f.data());
      std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);
    }

    auto f = std::array{
      htons(IPFIX::FIELD_IP_VERSION),
      htons(IPFIX::TYPE_8)
    };
    const auto* fp = reinterpret_cast<std::byte*>(f.data());
    std::copy_n(fp, IPFIX::TYPE_16 * 2, bkit);

    return fields;
  }

  std::vector<std::byte> values(const Tins::PDU& pdu) override {
    const auto& ip = static_cast<const Tins::IP&>(pdu);
    auto values = std::vector<std::byte>{};
    auto bkit = std::back_inserter(values);

    if (_def.src) {
      uint32_t src = ip.src_addr();
      const auto* vp = reinterpret_cast<std::byte*>(&src);
      std::copy_n(vp, IPFIX::TYPE_IPV4, bkit);
    }

    if (_def.dst) {
      uint32_t dst = ip.dst_addr();
      const auto* vp = reinterpret_cast<std::byte*>(&dst);
      std::copy_n(vp, IPFIX::TYPE_IPV4, bkit);
    }

    uint8_t version = htonT(ip.version());
    const auto* vp = reinterpret_cast<std::byte*>(&version);
    std::copy_n(vp, IPFIX::TYPE_8, bkit);

    return values;
  }
};

} // namespace Flow
