#pragma once

#include <unordered_map>

#include <buffer.hpp>
#include <ipfix.hpp>
#include <network.hpp>

namespace Flow {

class Exporter {
  static constexpr std::size_t BUFFER_SIZE = 1024;
  static constexpr std::uint16_t FLOW_TEMPLATE = IPFIX::SET_USER_TEMPLATE;

  Net::Connection _conn;
  std::unordered_map<std::size_t, std::uint16_t> _templates;
  Buffer _buffer;
  std::uint16_t _last_template = IPFIX::SET_USER_TEMPLATE + 1;
  std::uint32_t _sequence_num = 0;

  void copy_template(std::uint16_t, Buffer);

public:

  Exporter(const std::string&, std::uint16_t);

  /* Getters */
  std::uint16_t get_template_id(std::size_t) const;

  /* Modifiers */
  std::uint16_t insert_template(std::size_t, Buffer);
  void insert_record(const IPFIX::Properties&, std::uint8_t, Buffer);
  void flush();
};

} // namespace Flow
