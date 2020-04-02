#pragma once

#include "plugin.hpp"

struct Packet {
  const uint8_t* data;
  uint32_t len;
  uint32_t caplen;
  uint32_t timestamp;
};

class PacketProvider {
  using GetPacketFun = Packet(*)();
  using InitFun = void(*)(const char*);
  using FinalizeFun = void(*)();
  Plugin plugin;

  GetPacketFun get_packet_;
  InitFun init_;
  FinalizeFun finalize_;
  public:
  PacketProvider(Plugin&& plugin, const char* arg):
    plugin{std::move(plugin)},
    get_packet_{this->plugin.function<Packet()>("get_packet")},
    init_{this->plugin.function<void(const char*)>("init")},
    finalize_{this->plugin.function<void()>("finalize")} {
      init_(arg);
    }

  ~PacketProvider() {
    finalize_();
  }

  Packet get_packet() {
    return get_packet_();
  }
};
