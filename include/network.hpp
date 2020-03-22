#pragma once

#include <sys/socket.h>
#include <unistd.h>

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

  void send(const uint8_t* data, std::size_t size) {
    // TODO: Exception
    ::send(handle, data, size, 0);
  }
};
