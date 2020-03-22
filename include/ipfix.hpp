#pragma once

#include <unordered_map>

#include <tins/tins.h>

#include "network.hpp"
#include "flow.hpp"

namespace IPFIX {

static constexpr auto VERSION = uint16_t{0x000a};
static constexpr auto BUFFER_SIZE = std::size_t{1024};

#pragma pack(push)
#pragma pack(1)
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
#pragma pack(pop)

class Connection {
  std::unordered_map<std::size_t, std::size_t> templates;
  uint16_t template_id{256};
  std::size_t sequence_num{0};

  uint8_t buffer[BUFFER_SIZE];
  MessageHeader& msg_header = reinterpret_cast<MessageHeader&>(buffer);
  uint8_t* pos = buffer + sizeof(MessageHeader);

  TCPSocket socket;

  public:

  template<typename... Args>
  Connection(Args... args) {
    socket.connect(args...);
    msg_header.version = htons(VERSION);
  }

  std::size_t free() const {
    return buffer + BUFFER_SIZE - pos;
  }

  std::size_t length() const {
    return pos - buffer;
  }

  void to_export(const Flow::Record& record) {
    auto type = record.type_hash();
    auto id = std::size_t{};

    // Find if template was created
    auto search = templates.find(type);
    if (search == templates.end()) {
      templates.emplace(type, template_id);
      id = template_id++;

      // Send template
      auto& template_header = reinterpret_cast<TemplateHeader&>(*pos);
      pos += sizeof(TemplateHeader);
      template_header.id = htons(2);
      template_header.length = htons(record.template_length() + 8);
      template_header.template_id = htons(id);
      template_header.field_count = htons(record.template_fields());
      pos = record.export_template(pos);
    } else {
      id = search->second;
    }
    // TODO: Buffer overflow

    // Send record
    auto& record_header = reinterpret_cast<RecordHeader&>(*pos);
    pos += sizeof(RecordHeader);
    record_header.template_id = htons(id);
    record_header.length = htons(record.record_length() + 4);
    pos = record.export_record(pos);

    // Send message
    msg_header.length = htons(length());
    msg_header.timestamp = 0;
    msg_header.sequence_num = htonl(sequence_num);
    msg_header.domain_num = 0;
    socket.send(buffer, length());
    pos = buffer + sizeof(MessageHeader);
  }
};

} // namespace IPFIX
