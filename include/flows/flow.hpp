#pragma once

#include <tins/tins.h>

#include <buffer.hpp>

namespace Flow {

class Flow {
public:
  virtual bool should_process() const = 0;
  virtual std::size_t type() const = 0;
  virtual std::size_t digest(const Tins::PDU& pdu) const = 0;
  virtual Buffer fields() const = 0;
  virtual Buffer values(const Tins::PDU& pdu) const = 0;
  virtual ~Flow() = default;
};

} // namespace Flow
