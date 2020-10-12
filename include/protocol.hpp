#pragma once

#include <cstdint>
#include <ctime>
#include <variant>
#include <vector>
#include <array>

#include <common.hpp>
#include <ipfix.hpp>

namespace Flow {

/**
 * Metadata used in stroring flow records.
 */
struct Properties {
  std::size_t count;
  timeval first_timestamp;
  timeval last_timestamp;
};

struct IP {
  uint32_t src;
  uint32_t dst;

  [[nodiscard]] static IPFIX::Type type() {
    return IPFIX::Type::IP;
  }
};

struct IPv6 {
  std::array<std::byte, 16> src;
  std::array<std::byte, 16> dst;

  [[nodiscard]] static IPFIX::Type type() {
    return IPFIX::Type::IPV6;
  }
};

struct TCP {
  uint16_t src;
  uint16_t dst;

  [[nodiscard]] static IPFIX::Type type() {
    return IPFIX::Type::TCP;
  }
};

struct UDP {
  uint16_t src;
  uint16_t dst;

  [[nodiscard]] static IPFIX::Type type() {
    return IPFIX::Type::UDP;
  }
};

struct DOT1Q {
  uint16_t id;

  [[nodiscard]] static IPFIX::Type type() {
    return IPFIX::Type::DOT1Q;
  }
};

struct MPLS {
  uint32_t label;

  [[nodiscard]] static IPFIX::Type type() {
    return IPFIX::Type::MPLS;
  }
};

using Protocol = std::variant<IP, IPv6, TCP, UDP, DOT1Q, MPLS>;
using Chain = std::vector<Protocol>;
using Record = std::pair<Properties, Chain>;

[[nodiscard]] inline bool tsgeq(timeval f, timeval s) {
  return f.tv_sec == s.tv_sec ? f.tv_usec > s.tv_usec : f.tv_sec > s.tv_sec;
}

[[nodiscard]] inline std::size_t type(const Chain& chain) {
  auto result = 0ul;

  for (const auto& protocol: chain) {
    auto type = std::visit([](const auto& p){
        return p.type(); }, protocol);

    result = combine(result, type);
  }

  return result;
}

} // namespace Flow
