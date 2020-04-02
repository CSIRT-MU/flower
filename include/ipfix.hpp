#pragma once

#include <unordered_map>
#include <ctime>

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

  void export_template(const Flow::Record& record, std::size_t id) {
    // Check if buffer is full and send if true
    auto total_length = record.template_length() + sizeof(TemplateHeader);
    if (total_length > free()) send();

    auto& template_header = reinterpret_cast<TemplateHeader&>(*pos);
    pos += sizeof(TemplateHeader);
    template_header.id = htons(2);
    template_header.length = htons(total_length);
    template_header.template_id = htons(id);
    template_header.field_count = htons(record.template_fields());
    pos = record.export_template(pos);
  }

  void export_record(const Flow::Record& record, std::size_t id) {
    // Check if buffer is full and send if true
    auto total_length = record.record_length() + sizeof(RecordHeader);
    if (total_length > free()) send();
    
    auto& record_header = reinterpret_cast<RecordHeader&>(*pos);
    pos += sizeof(RecordHeader);
    record_header.template_id = htons(id);
    record_header.length = htons(total_length);
    pos = record.export_record(pos);
  }

  public:

  template<typename... Args>
  Connection(Args... args) {
    socket.connect(args...);
    msg_header.version = htons(VERSION);
  }


  ~Connection() {
    if (!empty())
      send();
  }

  bool empty() const {
    return pos == buffer + sizeof(MessageHeader);
  }

  std::size_t free() const {
    return buffer + BUFFER_SIZE - pos;
  }

  std::size_t length() const {
    return pos - buffer;
  }

  void to_export(Flow::Record& record) {
    auto type = record.type_hash();
    auto id = std::size_t{};

    // Find if template was created
    auto search = templates.find(type);
    if (search == templates.end()) {
      templates.emplace(type, template_id);
      id = template_id++;
      // TODO: Check if template_id id not overflow
      export_template(record, id);
    } else {
      id = search->second;
    }

    // Send record
    // Do not export record with count 0
    if (record.empty()) return;
    export_record(record, id);
    record.clean();
  }

  void send() {
    msg_header.length = htons(length());
    msg_header.timestamp = htonl(std::time(nullptr));
    msg_header.sequence_num = htonl(sequence_num);
    msg_header.domain_num = 0;
    socket.send(buffer, length());
    pos = buffer + sizeof(MessageHeader);
  }
};

} // namespace IPFIX
