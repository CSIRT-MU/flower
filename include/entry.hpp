#pragma once

#include <tins/tins.h>

struct Entry {
  Tins::PDU::PDUType type;
  Entry* prev;
  std::size_t count{1};

  Entry(Tins::PDU::PDUType type, Entry* prev): type{type}, prev{prev} {}
  virtual ~Entry() = default;

  virtual std::size_t template_length() const = 0;
  virtual std::size_t record_length() const = 0;
  virtual uint8_t* export_template(uint8_t* buffer) const = 0;
  virtual uint8_t* export_record(uint8_t* buffer) const = 0;
};

struct IP: public Entry {
  uint32_t src;
  uint32_t dst;

  template<typename... Args>
  IP(uint32_t src, uint32_t dst, Args... args):
    Entry{Tins::PDU::PDUType::IP, args...},
    src{src},
    dst{dst} {}

  std::size_t template_length() const override { return 8; }
  std::size_t record_length() const override { return 8; }

  uint8_t* export_template(uint8_t* buffer) const override {
    static const uint16_t t[4] = { htons(8), htons(4), htons(12), htons(4) };
    memcpy(buffer, t, sizeof(t));
    buffer += sizeof(t);
    return buffer;
  }

  uint8_t* export_record(uint8_t* buffer) const override {
    auto nsrc = src;
    auto ndst = dst;
    memcpy(buffer, &nsrc, sizeof(nsrc));
    buffer = buffer + sizeof(nsrc);
    memcpy(buffer, &ndst, sizeof(ndst));
    return buffer + sizeof(ndst);
  }
};

struct TCP: public Entry {
  uint16_t src;
  uint16_t dst;

  template<typename... Args>
  TCP(uint16_t src, uint16_t dst, Args... args):
    Entry{Tins::PDU::PDUType::TCP, args...},
    src{src},
    dst{dst} {}

  std::size_t template_length() const override { return 8; }
  std::size_t record_length() const override { return 4; }

  uint8_t* export_template(uint8_t* buffer) const override {
    static const uint16_t t[4] = { htons(7), htons(2), htons(11), htons(2) };
    memcpy(buffer, t, sizeof(t));
    buffer += sizeof(t);
    return buffer;
  }

  uint8_t* export_record(uint8_t* buffer) const override {
    auto nsrc = htons(src);
    auto ndst = htons(dst);
    memcpy(buffer, &nsrc, sizeof(nsrc));
    buffer = buffer + sizeof(nsrc);
    memcpy(buffer, &ndst, sizeof(ndst));
    return buffer + sizeof(ndst);
  }
};
