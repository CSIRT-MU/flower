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

template<std::size_t N>
class Buffer {
  static constexpr auto CAPACITY = std::size_t{N};

  uint8_t buffer[CAPACITY];
  uint8_t* const begin = buffer + sizeof(MessageHeader);
  uint8_t* pos = begin;

  public:

  MessageHeader* const msg_header = reinterpret_cast<MessageHeader*>(buffer);

  Buffer() { reset(); }

  template<typename T>
  T* push(std::size_t size) {
    T* result = reinterpret_cast<T*>(pos);
    pos += sizeof(T) + size;
    return result;
  }

  bool empty() const { return pos == begin; }
  std::size_t free() const { return buffer + CAPACITY - pos; }
  std::size_t size() const { return pos - buffer; }
  void reset() { pos = begin; }
  uint8_t* data() { return buffer; }

};

class Connection {
  std::unordered_map<std::size_t, std::size_t> templates;
  uint16_t template_id{256};
  std::size_t sequence_num{0};

  Buffer<BUFFER_SIZE> buffer;
  net::Connection conn;

  template<typename T>
  static uint8_t* after(T* header) {
    return reinterpret_cast<uint8_t*>(header) + sizeof(T);
  }

  void export_template(const Flow::Record& record, std::size_t id) {
    // Check if buffer is full and send if true
    auto total_length = record.template_length() + sizeof(TemplateHeader);
    if (total_length > buffer.free()) send();

    auto* template_header = buffer.push<TemplateHeader>(record.template_length());
    template_header->id = htons(2);
    template_header->length = htons(total_length);
    template_header->template_id = htons(id);
    template_header->field_count = htons(record.template_fields());
    record.export_template(after(template_header));
  }

  void export_record(const Flow::Record& record, std::size_t id) {
    // Check if buffer is full and send if true
    auto total_length = record.record_length() + sizeof(RecordHeader);
    if (total_length > buffer.free()) send();
    
    auto* record_header = buffer.push<RecordHeader>(record.record_length());
    record_header->template_id = htons(id);
    record_header->length = htons(total_length);
    record.export_record(after(record_header));
  }

  public:

  template<typename... Args>
  Connection(Args... args):
    conn(net::make_tcp_connection(args...)){
    buffer.msg_header->version = htons(VERSION);
  }


  ~Connection() {
    if (!buffer.empty())
      send();
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
    buffer.msg_header->length = htons(buffer.size());
    buffer.msg_header->timestamp = htonl(std::time(nullptr));
    buffer.msg_header->sequence_num = htonl(sequence_num);
    buffer.msg_header->domain_num = 0;
    conn.write(buffer.data(), buffer.size());
    buffer.reset();
  }
};

} // namespace IPFIX
