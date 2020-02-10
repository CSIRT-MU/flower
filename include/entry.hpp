#pragma once

#include <array>

#include <tins/tins.h>

#include "network.hpp"

struct Entry {
  Tins::PDU::PDUType type;
  Entry* prev;
  std::size_t count{1};

  Entry(Tins::PDU::PDUType type, Entry* prev): type{type}, prev{prev} {}
  virtual ~Entry() = default;

  virtual Payload template_body() const = 0;
  virtual Payload record_body() const = 0;
};

struct IP: public Entry {
  uint32_t src;
  uint32_t dst;

  template<typename... Args>
  IP(uint32_t src, uint32_t dst, Args... args):
    Entry{Tins::PDU::PDUType::IP, args...},
    src{src},
    dst{dst} {}

  Payload template_body() const override {
    return Payload::from_shorts({8, 4, 12, 4});
  };

  Payload record_body() const override {
    return Payload::from_longs({src, dst});
  };
};

struct TCP: public Entry {
  uint16_t src;
  uint16_t dst;

  template<typename... Args>
  TCP(uint16_t src, uint16_t dst, Args... args):
    Entry{Tins::PDU::PDUType::TCP, args...},
    src{src},
    dst{dst} {}

  Payload template_body() const override {
    return Payload::from_shorts({7, 2, 11, 2});
  };

  Payload record_body() const override {
    return Payload::from_shorts({src, dst});
  };
};
