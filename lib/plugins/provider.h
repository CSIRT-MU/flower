#pragma once

struct Packet {
  const uint8_t* data;
  uint32_t len;
  uint32_t caplen;
  uint32_t timestamp;
};

