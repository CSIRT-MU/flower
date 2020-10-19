#include <exporter.hpp>

#include <cstdint>
#include <ctime>
#include <algorithm>
#include <common.hpp>

namespace Flow {

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

Exporter::Exporter(const std::string& address, short port):
  _conn(Net::Connection::tcp(address, port)) {}

void Exporter::export_flow(std::size_t type) {
  auto buffer = std::vector<std::byte>{sizeof(MessageHeader)};
  auto bkit = std::back_inserter(buffer);

  auto& [templ_id, templ] = _templates.find(type)->second;
  auto& [_, record] = *_records.find(type);

  auto templ_header = TemplateHeader{
    htons(IPFIX_TEMPLATE_ID),
    htons(sizeof(TemplateHeader) + templ.size()),
    htons(templ_id),
    htons(templ.size() / 4)
  };

  auto* templ_header_p = reinterpret_cast<std::byte*>(&templ_header);
  std::copy(templ_header_p, templ_header_p + sizeof(TemplateHeader), bkit);
  std::copy(templ.begin(), templ.end(), bkit);

  auto record_header = RecordHeader{
    htons(templ_id),
    htons(sizeof(RecordHeader) + record.size())
  };

  auto* record_header_p = reinterpret_cast<std::byte*>(&record_header);
  std::copy(record_header_p, record_header_p + sizeof(RecordHeader), bkit);
  std::copy(record.begin(), record.end(), bkit);

  auto& message_header = reinterpret_cast<MessageHeader&>(*buffer.data());
  message_header.version = htons(IPFIX_VERSION);
  message_header.length = htons(buffer.size());
  message_header.timestamp = htonl(std::time(nullptr));
  message_header.sequence_num = htonl(++_sequence_num);
  message_header.domain_num = 0;

  _conn.write(buffer.data(), buffer.size());

  record.clear();
}

bool Exporter::has_template(std::size_t type) const {
  return _templates.find(type) != _templates.end();
}

void Exporter::insert_template(std::size_t type, std::vector<std::byte> fields) {
  _templates.emplace(type, std::make_pair(_last_template, std::move(fields)));
  ++_last_template;
}

void Exporter::insert_record(std::size_t type,
    IPFIX::Properties props, std::vector<std::byte> values) {
  auto& buffer = _records[type];

  if (buffer.size() + values.size() + 32 > MAX_BUFFER_SIZE) {
    export_flow(type);
  }

  auto bkit = std::back_inserter(buffer);

  std::uint64_t count = htonT(props.count);
  auto* p = reinterpret_cast<std::byte*>(&count);
  std::copy_n(p, IPFIX::TYPE_64, bkit);

  std::uint32_t flow_start_sec =
    htonl(props.flow_start.tv_sec);
  p = reinterpret_cast<std::byte*>(&flow_start_sec);
  std::copy_n(p, IPFIX::TYPE_SECONDS, bkit);

  std::uint32_t flow_end_sec =
    htonl(props.flow_end.tv_sec);
  p = reinterpret_cast<std::byte*>(&flow_end_sec);
  std::copy_n(p, IPFIX::TYPE_SECONDS, bkit);

  std::uint64_t flow_start_msec = htonT(
      props.flow_start.tv_sec * 1000
      + props.flow_start.tv_usec / 1000);
  p = reinterpret_cast<std::byte*>(&flow_start_msec);
  std::copy_n(p, IPFIX::TYPE_MILLISECONDS, bkit);

  std::uint64_t flow_end_msec = htonT(
      props.flow_end.tv_sec * 1000
      + props.flow_end.tv_usec / 1000);
  p = reinterpret_cast<std::byte*>(&flow_end_msec);
  std::copy_n(p, IPFIX::TYPE_MILLISECONDS, bkit);

  std::move(values.begin(), values.end(), bkit);
}

void Exporter::export_all() {
  for (const auto& [type, _]: _records) {
    export_flow(type);
  }
}

void Exporter::clear() {
  _records.clear();
}

} // namespace Flow
