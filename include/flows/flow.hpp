#pragma once

#include <tins/tins.h>

namespace Flow {

class Flow {
public:
  virtual bool should_process() = 0;
  virtual std::size_t type() = 0;
  virtual std::size_t digest(const Tins::PDU& pdu) = 0;
  virtual std::vector<std::byte> fields() = 0;
  virtual std::vector<std::byte> values(const Tins::PDU& pdu) = 0;
  virtual ~Flow() = default;
};

} // namespace Flow
