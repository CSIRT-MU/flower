#include <exporter.hpp>

#include <common.hpp>

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
prepare_flow_template()
{
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
  result.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_FLOW_END_REASON));
  result.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_8));
  result.push_back_any<std::uint16_t>(htons(IPFIX::FIELD_SUB_TEMPLATE_MULTI_LIST));
  result.push_back_any<std::uint16_t>(htons(IPFIX::TYPE_LIST));

  return result;
}

Exporter::Exporter(const std::string& address, std::uint16_t port)
  : _conn(Net::Connection::tcp(address, port))
{
  _buffer.reserve(BUFFER_SIZE);
  _buffer.push_back_any<MessageHeader>({});

  copy_template(FLOW_TEMPLATE, prepare_flow_template());
}

std::uint16_t
Exporter::get_template_id(std::size_t type) const
{
  auto search = _templates.find(type);

  if (search == _templates.end())
    return 0;

  return search->second;
}

void 
Exporter::copy_template(std::uint16_t tid, Buffer fields)
{
  _buffer.push_back_any<TemplateHeader>({
      htons(IPFIX::SET_TEMPLATE),
      htons(sizeof(TemplateHeader) + fields.size()),
      htons(tid),
      htons(fields.size() / 4)
      });

  _buffer.insert(_buffer.end(), fields.begin(), fields.end());
}

std::uint16_t
Exporter::insert_template(std::size_t type, Buffer fields)
{
  if (_buffer.capacity() < fields.size() + sizeof(TemplateHeader))
    flush();

  copy_template(_last_template, fields);

  _templates.emplace(type, _last_template);
  ++_last_template;

  return _last_template - 1;
}

void
Exporter::insert_record(
    const IPFIX::Properties& props, std::uint8_t reason, Buffer values)
{
  if (_buffer.capacity() < values.size() + sizeof(RecordHeader))
    flush();

  _buffer.push_back_any<RecordHeader>({
      htons(FLOW_TEMPLATE),
      htons(sizeof(RecordHeader) + 33 + values.size())
      });

  _buffer.push_back_any<std::uint64_t>(htonT(props.count));
  _buffer.push_back_any<std::uint32_t>(htonl(props.flow_start.tv_sec));
  _buffer.push_back_any<std::uint32_t>(htonl(props.flow_end.tv_sec));
  _buffer.push_back_any<std::uint64_t>(
      htonT(props.flow_start.tv_sec * 1000 + props.flow_start.tv_usec / 1000));
  _buffer.push_back_any<std::uint64_t>(
      htonT(props.flow_end.tv_sec * 1000 + props.flow_end.tv_usec / 1000));
  _buffer.push_back_any<std::uint8_t>(reason);

  _buffer.insert(_buffer.end(), values.begin(), values.end());

  ++_sequence_num;
}

void
Exporter::flush()
{
  _buffer.set_any_at<MessageHeader>(0, {
      htons(IPFIX::VERSION),
      htons(_buffer.size()),
      htonl(std::time(nullptr)),
      htonl(_sequence_num),
      htonl(0)
      });

  _conn.write(_buffer.data(), _buffer.size());
  _buffer.clear();
  _buffer.push_back_any<MessageHeader>({});
}

} // namespace Flow
