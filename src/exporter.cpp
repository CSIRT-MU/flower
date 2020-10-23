#include <exporter.hpp>

#include <cstdint>
#include <ctime>
#include <algorithm>

#include <common.hpp>
#include <buffer.hpp>

namespace Flow {

struct [[gnu::packed]] MessageHeader {
  std::uint16_t version;
  std::uint16_t length;
  std::uint32_t timestamp;
  std::uint32_t sequence_num;
  std::uint32_t domain_num;
};

struct [[gnu::packed]] RecordHeader {
  std::uint16_t template_id;
  std::uint16_t length;
};

struct [[gnu::packed]] TemplateHeader {
  std::uint16_t id;
  std::uint16_t length;
  std::uint16_t template_id;
  std::uint16_t field_count;
};

static Buffer
prepare_fields() {
  auto result = Buffer{};

  result.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_PACKET_DELTA_COUNT));
  result.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_64));
  result.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_FLOW_START_SECONDS));
  result.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_SECONDS));
  result.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_FLOW_END_SECONDS));
  result.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_SECONDS));
  result.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_FLOW_START_MILLISECONDS));
  result.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_MILLISECONDS));
  result.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_FLOW_END_MILLISECONDS));
  result.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_MILLISECONDS));
  result.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_SUB_TEMPLATE_MULTI_LIST));
  result.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_LIST));

  return result;
}

Exporter::Exporter(const std::string& address, short port)
  : _conn(Net::Connection::tcp(address, port)) {
  insert_template(69, prepare_fields());
}


bool
Exporter::has_template(std::size_t type) const {
  return _templates.find(type) != _templates.end();
}

std::uint16_t
Exporter::get_template_id(std::size_t type) const {
  return _templates.find(type)->second.first;
}

std::uint16_t
Exporter::insert_template(std::size_t type, std::vector<std::byte> fields) {
  auto message = Buffer{};
  message.push_back_any<MessageHeader>({});
  message.push_back_any<TemplateHeader>({
      htons(IPFIX_TEMPLATE_ID),
      htons(sizeof(TemplateHeader) + fields.size()),
      htons(_last_template),
      htons(fields.size() / 4)
      });
  message.insert(message.end(), fields.begin(), fields.end());
  message.set_any_at<MessageHeader>(0, {
      htons(IPFIX::VERSION),
      htons(message.size()),
      htonl(std::time(nullptr)),
      htonl(++_sequence_num),
      htonl(0),
      });
  _conn.write(message.data(), message.size());
  
  _templates.emplace(type, std::make_pair(_last_template, std::move(fields)));
  return ++_last_template - 1;
}

void
Exporter::insert_record(IPFIX::Properties props, std::vector<std::byte> values) {
  auto message = Buffer{};

  message.push_back_any<MessageHeader>({});
  message.push_back_any<RecordHeader>({
      htons(IPFIX_USER_TEMPLATES),
      htons(sizeof(RecordHeader) + values.size() + 32)
      });
  message.push_back_any<std::uint64_t>(htonT(props.count));
  message.push_back_any<std::uint32_t>(htonl(props.flow_start.tv_sec));
  message.push_back_any<std::uint32_t>(htonl(props.flow_end.tv_sec));
  // TODO: finish
  message.push_back_any<std::uint64_t>(htonT(props.flow_start.tv_sec * 1000));
  message.push_back_any<std::uint64_t>(htonT(props.flow_end.tv_sec * 1000));

  message.insert(message.end(), values.begin(), values.end());

  message.set_any_at<MessageHeader>(0, {
      htons(IPFIX::VERSION),
      htons(message.size()),
      htonl(std::time(nullptr)),
      htonl(++_sequence_num),
      htonl(0),
      });

  _conn.write(message.data(), message.size());
}

} // namespace Flow
