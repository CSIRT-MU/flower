#pragma once

#include <unordered_map>

#include <network.hpp>
#include <protocol.hpp>

namespace IPFIX {

static constexpr auto VERSION = uint16_t{0x000a};

struct MessageHeader {
  uint16_t version;
  uint16_t length;
  uint32_t timestamp;
  uint32_t sequence_num;
  uint32_t domain_num;
};

struct RecordHeader {
  uint16_t template_id;
  uint16_t length;
};

struct TemplateHeader {
  uint16_t id;
  uint16_t length;
  uint16_t template_id;
  uint16_t field_count;
};

class Template {
  static constexpr auto id = 2;
  uint16_t _length;
  uint16_t _template_id;
  uint16_t _field_count;

  std::vector<std::pair<uint16_t, uint16_t>> _fields;

  public:
};

class Record {
  uint16_t _template_id;
  uint16_t _length;

  public:
};

class Buffer {
  std::vector<std::byte> _data;

  public:

  template<typename T>
  void push_back(T&& value) {
    auto* b = reinterpret_cast<std::byte*>(&value); // NOLINT
    auto* e = b + sizeof(T); // NOLINT
    std::copy(b, e, std::back_inserter(_data));
  }

  template<typename T>
  [[nodiscard]] T& at(std::size_t index) {
    return reinterpret_cast<T&>(_data[index]); // NOLINT
  }

  std::vector<std::byte>&& data() {
    return std::move(_data);
  }

};

class Exporter {
  std::unordered_multimap<std::size_t, Flow::Record> _records;
  std::unordered_map<std::size_t, Template> _templates;

  public:

  template<typename T>
  void insert(T&& record) {
    const auto key = Flow::digest(record);
    const auto type = Flow::type(record);

    auto search = _templates.find(type);
    if (search == _templates.end()) {
      // TODO(dudoslav): Create template
    } else {
      // TODO(dudoslav): Get record set id
    }

    _records.emplace(type, std::forward<T>(record));
  }

  std::vector<std::byte> data() {
    auto buffer = Buffer{};

    buffer.push_back(MessageHeader{VERSION, 0, 0, 0, 0});

    for (const auto& p: _templates) {
      buffer.push_back(TemplateHeader{});
    }

    return buffer.data();
  }
};

} // namespace IPFIX
