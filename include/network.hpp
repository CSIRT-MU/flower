#pragma once

#include <stdexcept>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace net {

class Socket {
  int handle;

  friend class Connection;

  // Construtor from linux handle
  Socket(int handle): handle(handle) {
    if (handle == -1)
      throw std::system_error{errno, std::system_category()};
  }

  operator int() { return handle; }

  public:

  // Construtor from linux socket function
  Socket(int domain, int type, int protocol):
    Socket(socket(domain, type, protocol)) {}

  ~Socket() { if (handle != -1) close(handle); }

  // Copy
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  // Move
  Socket(Socket&& o) noexcept : handle(o.handle) { o.handle = -1; };
  Socket& operator=(Socket&& o) {
    handle = o.handle;
    o.handle = -1;
    return *this;
  }
};

class Connection {
  Socket socket;

  // Construct from connected socket
  Connection(Socket&& sock):
    socket(std::move(sock)) {}

  public:

  // Connect and construct
  template<typename T>
  Connection(Socket&& sock, T addr):
    Connection(std::move(sock)) {
    if (connect(socket, addr, addr.size()) == -1)
      throw std::system_error{errno, std::system_category()};
  }

  ~Connection() { shutdown(socket, SHUT_RDWR); }

  // Copy
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  // Move
  Connection(Connection&& o) noexcept : socket(std::move(o.socket)) {}
  Connection& operator=(Connection&& o) {
    socket = std::move(o.socket);
    return *this;
  }

  // Send data with given size over socket
  Connection& write(const unsigned char* data, std::size_t size) {
    // TODO: On EAGAIN
    if (::write(socket, data, size) == -1)
      throw std::system_error{errno, std::system_category()};
    return *this;
  }
};

class SocketAddressIPv4 {
  struct sockaddr_in saddr;

  public:

  SocketAddressIPv4(int addr, short port) {
    saddr.sin_addr.s_addr = addr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
  }

  SocketAddressIPv4(const char* saddr, short port):
    SocketAddressIPv4(inet_addr(saddr), port) {}

  socklen_t size() const { return sizeof(saddr); }

  operator struct sockaddr*()
    { return reinterpret_cast<struct sockaddr*>(&saddr); }
  operator const struct sockaddr*() const
    { return reinterpret_cast<const struct sockaddr*>(&saddr); }
};

// Network helper functions
Socket make_tcp_socket() { return Socket{AF_INET, SOCK_STREAM, 0}; }

Connection make_tcp_connection(const char* saddr, short port) {
  return Connection(make_tcp_socket(), SocketAddressIPv4{saddr, port});
}

};
