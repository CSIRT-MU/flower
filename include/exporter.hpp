#pragma once

#include <vector>
#include <unordered_map>
#include <cstdint>

#include <network.hpp>
#include <ipfix.hpp>

namespace Flow {

static constexpr auto IPFIX_USER_TEMPLATES = 256;
static constexpr auto IPFIX_TEMPLATE_ID = 2;

class Exporter {

  using TemplateEntry = std::pair<uint16_t, std::vector<std::byte>>;

  std::size_t _last_template{IPFIX_USER_TEMPLATES};
  std::size_t _sequence_num{0};

  std::unordered_map<std::size_t, TemplateEntry> _templates;

  Net::Connection _conn;

public:

  Exporter(const std::string&, short);

  bool has_template(std::size_t) const;
  std::uint16_t get_template_id(std::size_t) const;
  std::uint16_t insert_template(std::size_t, std::vector<std::byte>);
  void insert_record(IPFIX::Properties, std::vector<std::byte>);

};

} // namespace Flow
