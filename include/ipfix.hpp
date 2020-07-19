#pragma once

#include <cstddef>
#include <unordered_map>

#include <arpa/inet.h>

#include <flow.hpp>
#include <network.hpp>
#include <protocol.hpp>

namespace IPFIX {

static constexpr auto VERSION = uint16_t{0x000a};
static constexpr auto USER_TEMPLATES = 256;
static constexpr auto TEMPLATE_ID = 2;

struct [[gnu::packed]] MessageHeader {
  uint16_t version;
  uint16_t length;
  uint32_t timestamp;
  uint32_t sequence_num;
  uint32_t domain_num;
};

struct [[gnu::packed]] RecordHeader {
  uint16_t template_id;
  uint16_t length;
};

struct [[gnu::packed]] TemplateHeader {
  uint16_t id;
  uint16_t length;
  uint16_t template_id;
  uint16_t field_count;
};

inline std::vector<std::byte> values(const Flow::Properties& props) {
  auto ncount = htonl(props.count);
  auto result = std::vector<std::byte>(sizeof(ncount));
  std::memcpy(result.data(), &ncount, sizeof(ncount));

  return result;
}

inline std::vector<std::byte> fields([[maybe_unused]] const Flow::Properties& props) {
  static const auto t = std::array{
    htons(Flow::IPFIX_PACKET_DELTA_COUNT),
    htons(Flow::IPFIX_LONG)
  };
  auto s = std::as_bytes(std::span{t});

  return {s.begin(), s.end()};
}

inline std::vector<std::byte> values(const Flow::Entry& entry) {
  auto result = values(entry.first);
  auto v = values(entry.second);
  result.insert(result.end(), v.cbegin(), v.cend());

  return result;
}

inline std::vector<std::byte> fields(const Flow::Entry& entry) {
  auto result = fields(entry.first);
  auto f = fields(entry.second);
  result.insert(result.end(), f.cbegin(), f.cend());

  return result;
}

class Exporter {
  using TemplateEntry = std::pair<uint16_t, std::vector<std::byte>>;

  std::size_t _last_template{USER_TEMPLATES};
  std::size_t _sequence_num{0};

  std::unordered_map<std::size_t, TemplateEntry> _templates;
  std::unordered_map<std::size_t, std::vector<std::byte>> _records;

  public:

  template<typename T>
  void insert(T&& cache_entry) {
    const auto type = Flow::type(cache_entry.second);

    auto search = _templates.find(type);
    if (search == _templates.end()) {
      auto t = fields(cache_entry);
      _templates.emplace(type,
          std::make_pair(_last_template, std::move(t)));
      ++_last_template;
    }

    // TODO(dudoslav): Buffer cannot exceed MTL
    auto& buffer = _records[type];
    auto v = values(cache_entry);
    buffer.insert(buffer.end(), v.begin(), v.end());
  }

  void clear() {
    _records.clear();
  }

  // TODO(dudoslav): Make const
  template<typename Fun>
  void for_each_buffer(Fun&& f) {
    for (const auto& r: _records) {
      auto buffer = std::vector<std::byte>{sizeof(MessageHeader)};

      auto t = _templates[r.first]; // TODO(dudoslav): If exists

      auto th = TemplateHeader{};
      th.id = htons(TEMPLATE_ID);
      th.length = htons(sizeof(th) + t.second.size());
      th.template_id = htons(t.first);
      th.field_count = htons(t.second.size() / 4);
      
      auto bth = std::as_bytes(std::span{&th, 1});

      std::copy(bth.begin(), bth.end(), std::back_inserter(buffer));
      std::copy(t.second.begin(), t.second.end(), std::back_inserter(buffer));

      auto rh = RecordHeader{};
      rh.template_id = htons(t.first);
      rh.length = htons(sizeof(rh) + r.second.size());

      auto brh = std::as_bytes(std::span{&rh, 1});

      std::copy(brh.begin(), brh.end(), std::back_inserter(buffer));
      std::copy(r.second.begin(), r.second.end(), std::back_inserter(buffer));

      auto mh = MessageHeader{};
      mh.version = htons(VERSION);
      mh.length = htons(buffer.size());
      mh.timestamp = htonl(std::time(nullptr));
      mh.sequence_num = htonl(++_sequence_num);
      mh.domain_num = 0;

      auto bmh = std::as_bytes(std::span{&mh, 1});
      std::copy(bmh.begin(), bmh.end(), buffer.begin());

      f(buffer);
    }
  }
};

} // namespace IPFIX
