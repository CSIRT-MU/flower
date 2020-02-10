#pragma once

#include <unordered_map>

#include <tins/tins.h>

#include "network.hpp"
#include "flow.hpp"

namespace IPFIX {

static constexpr auto VERSION = uint16_t{0x000a};

class Connection {
  std::unordered_map<std::size_t, std::size_t> templates;
  uint16_t template_id{256};
  std::size_t sequence_num{0};

  TCPSocket socket;

  Payload create_template(const Flow::Record& record) {
    auto result = Payload{};
    auto t = record.template_body();

    result << uint16_t{2};
    result << static_cast<uint16_t>(t.data.size() + 8);
    result << uint16_t{template_id};
    result << static_cast<uint16_t>(t.data.size() / 4);
    result += t;

    ++template_id;
    return result;
  }

  Payload create_record(const Flow::Record& record, uint16_t id) {
    auto result = Payload{};
    auto r = record.record_body();

    result << id;
    result << static_cast<uint16_t>(r.data.size() + 4);
    result += r;

    return result;
  }

  Payload create_message(const Payload& msg_body) {
    auto result = Payload{};

    result << VERSION;
    result << static_cast<uint16_t>(msg_body.data.size() + 16);
    result << uint32_t{0}; // TODO: Timestamp
    result << static_cast<uint32_t>(sequence_num++);
    result << uint32_t{0}; // TODO: Domain
    result += msg_body;

    return result;
  }

  public:

  template<typename... Args>
  Connection(Args... args) {
    socket.connect(args...);
  }

  void send_record(const Flow::Record& record) {
    auto hash = record.type_hash();
    auto id = uint16_t{0};
    auto msg_body = Payload{};

    auto search = templates.find(hash);
    if (search == templates.end()) {
      id = template_id;
      msg_body += create_template(record);
      templates[hash] = id;
    } else {
      id = search->second;
    }

    msg_body += create_record(record, id);
    socket << create_message(msg_body).data;
  }
};

} // namespace IPFIX
