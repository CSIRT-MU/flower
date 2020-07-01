#pragma once

#include <cstddef>
#include <unordered_map>

#include <arpa/inet.h>

#include <network.hpp>
#include <protocol.hpp>

namespace IPFIX {

static constexpr auto VERSION = uint16_t{0x000a};

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

class Exporter {
  using TemplateEntry = std::pair<uint16_t, std::vector<std::byte>>;

  // TODO(dudoslav): Remove magic number
  std::size_t _last_template{256};

  std::unordered_map<std::size_t, TemplateEntry> _templates;
  std::unordered_map<std::size_t, std::vector<std::byte>> _records;

  public:

  template<typename T>
  void insert(T&& record) {
    const auto type = Flow::type(record);

    auto search = _templates.find(type);
    if (search == _templates.end()) {
      // TODO(dudoslav): Create template
      auto t = fields(record);
      _templates.emplace(type,
          std::make_pair(_last_template, std::move(t)));
      ++_last_template;
    }

    // TODO(dudoslav): Buffer cannot exceed MTL
    auto& buffer = _records[type];
    auto v = values(record);
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
      th.id = htons(2); // TODO(dudoslav): Remove magic number
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
      mh.timestamp = 0; // TODO(dudoslav): Fill and htonl
      mh.sequence_num = 0;
      mh.domain_num = 0;

      auto bmh = std::as_bytes(std::span{&mh, 1});
      std::copy(bmh.begin(), bmh.end(), buffer.begin());

      f(buffer);
    }
  }
};

} // namespace IPFIX
