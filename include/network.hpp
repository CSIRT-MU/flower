#pragma once

#include <sys/socket.h>
#include <unistd.h>

struct Payload {
  std::vector<uint8_t> data;

  Payload& operator<<(uint32_t d) {
    auto l = htonl(d);
    auto ps = reinterpret_cast<uint8_t*>(&l);
    std::copy(ps, ps + sizeof(l), std::back_inserter(data));
    return *this;
  }

  Payload& operator<<(uint16_t d) {
    auto s = htons(d);
    auto ps = reinterpret_cast<uint8_t*>(&s);
    std::copy(ps, ps + sizeof(s), std::back_inserter(data));
    return *this;
  }

  Payload& operator+=(const Payload& o) {
    // TODO: Optimize with reserve
    std::copy(o.data.begin(), o.data.end(), std::back_inserter(data));
    return *this;
  }

  static Payload from_shorts(std::initializer_list<uint16_t> shorts) {
    auto payload = Payload{};

    for (auto s: shorts) {
      payload << s;
    }

    return payload;
  }

  static Payload from_longs(std::initializer_list<uint32_t> longs) {
    auto payload = Payload{};

    for (auto l: longs) {
      payload << l;
    }

    return payload;
  }
};

class TCPSocket {
  int handle;
  public:
  TCPSocket(): handle{socket(AF_INET, SOCK_STREAM, 0)} {
    // TODO: raise if handle == -1
  }

  ~TCPSocket() {
    close(handle);
  }

  TCPSocket(const TCPSocket&) = delete;
  TCPSocket& operator=(const TCPSocket&) = delete;

  void connect(const char* addr, unsigned short port) {
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(addr);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    ::connect(handle, (struct sockaddr*) &server, sizeof(server));
  }

  TCPSocket& operator<<(const std::vector<uint8_t>& data) {
    send(handle, data.data(), data.size(), 0);
    return *this;
  }

  TCPSocket& operator<<(const std::string& data) {
    send(handle, data.c_str(), data.size(), 0);
    return *this;
  }

  TCPSocket& operator<<(int data) {
    auto payload = htonl(data);
    send(handle, &payload, sizeof(data), 0);
    return *this;
  }

  TCPSocket& operator<<(short data) {
    auto payload = htons(data);
    send(handle, &payload, sizeof(data), 0);
    return *this;
  }
};
