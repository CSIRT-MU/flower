#pragma once

#include "record.hpp"

struct PacketProvider {
  virtual void* getPacket() = 0;
  virtual ~PacketProvider() = default;
};
