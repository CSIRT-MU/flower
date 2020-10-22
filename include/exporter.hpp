#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>

#include <network.hpp>
#include <ipfix.hpp>

namespace Flow {

static constexpr auto IPFIX_VERSION = uint16_t{0x000a};
static constexpr auto IPFIX_USER_TEMPLATES = 256;
static constexpr auto IPFIX_TEMPLATE_ID = 2;

static constexpr auto MAX_BUFFER_SIZE = 1024;

class Exporter {

  using TemplateEntry = std::pair<uint16_t, std::vector<std::byte>>;

  std::size_t _last_template{IPFIX_USER_TEMPLATES};
  std::size_t _sequence_num{0};

  std::unordered_map<std::size_t, TemplateEntry> _templates;
  std::unordered_map<std::size_t, std::vector<std::byte>> _records;

  Net::Connection _conn;

  void export_flow(std::size_t);

public:

  Exporter(const std::string&, short);

  bool has_template(std::size_t) const;
  void insert_template(std::size_t, std::vector<std::byte>);
  void insert_record(std::size_t, IPFIX::Properties, std::vector<std::byte>);
  void clear();
  void export_all();

};

} // namespace Flow
