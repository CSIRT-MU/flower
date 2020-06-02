#pragma once

#include <stdexcept>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace Net {

/**
 * RAII wrapper class for LINUX socket.
 */
class Socket {
  int _handle = -1;

  /**
   * Connection class must be able to create socket from
   * file descriptor, since it is returned by connect(2).
   */
  friend class Connection;

  /**
   * Create socket from file descriptor, used by Connection
   * class.
   * @param handle LINUX file descriptor
   */
  explicit Socket(int handle): _handle(handle) {}

  Socket() = default;

  [[nodiscard]] int descriptor() const { return _handle; }

  public:

  /**
   * Create socket from LINUX command socket(2).
   * @param domain
   * @param type
   * @param protocol
   * @throw std::system_error if failed to create socket
   */
  Socket(int domain, int type, int protocol):
    Socket(socket(domain, type, protocol)) {
      if (_handle == -1) {
        throw std::system_error{errno, std::system_category()};
      }
    }

  /**
   * Create default streaming socket. Most probably TCP. 
   */
  static Socket tcp() {
    return Socket{AF_INET, SOCK_STREAM, 0};
  }

  ~Socket() { if (_handle != -1) { close(_handle); } }

  // Copy
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  // Move
  Socket(Socket&& other) noexcept {
    *this = std::move(other);
  }

  Socket& operator=(Socket&& other) noexcept {
    std::swap(_handle, other._handle);

    return *this;
  }
};

/**
 * Wrapper class for all sockaddr_* structs.
 */
class SocketAddress {
  using SockAddrStorage = struct sockaddr_storage;
  using SockAddr = struct sockaddr;
  using SockAddrIn = struct sockaddr_in;

  SockAddrStorage _saddr = {};

  public:

  explicit SocketAddress(SockAddrIn saddr) {
    memcpy(&_saddr, &saddr, sizeof(saddr));
  }

  static SocketAddress ipv4(const std::string& addr, uint16_t port) {
    auto saddr = SockAddrIn{};
    saddr.sin_addr.s_addr = inet_addr(addr.c_str());
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);

    return SocketAddress{saddr};
  }

  [[nodiscard]] socklen_t size() const { return sizeof(_saddr); }

  [[nodiscard]] SockAddr* sockaddr() {
    return reinterpret_cast<SockAddr*>(&_saddr); // NOLINT
  }

  [[nodiscard]] const SockAddr* sockaddr() const {
    return reinterpret_cast<const SockAddr*>(&_saddr); // NOLINT
  }
};

/**
 * RAII connection wrapper class. This class is used for connecting to
 * remote server and send data. For this project writing to socket is
 * enough.
 */
class Connection {
  Socket _socket;

  public:

  /**
   * Connect provided socket to given address.
   * @param sock socket as rvalue
   * @param saddr socket address as destination address
   * @throw std::system_error if failed to connect socket
   */
  Connection(Socket&& sock, SocketAddress saddr):
    _socket(std::move(sock)) {
    if (connect(_socket.descriptor(), saddr.sockaddr(), saddr.size()) == -1) {
      throw std::system_error{errno, std::system_category()};
    }
  }

  ~Connection() { shutdown(_socket.descriptor(), SHUT_RDWR); }

  /**
   * Helper function for creating stream connection (most probably TCP)
   * @see Connection::Connection();
   * @param addr address of destination
   * @param port port of destination
   * @throw std::system_error if failed to connect socket
   */
  static Connection tcp(const std::string& addr, uint16_t port) {
    return Connection{Socket::tcp(), SocketAddress::ipv4(addr, port)};
  }

  // Copy
  Connection(const Connection&) = delete;
  Connection& operator=(const Connection&) = delete;

  // Move
  Connection(Connection&& other) noexcept {
    *this = std::move(other);
  }

  Connection& operator=(Connection&& other) noexcept {
    std::swap(_socket, other._socket);

    return *this;
  }

  /**
   * Write into connection, since this is stream connection this should
   * be guaranteed to arrive.
   * @param data pointer to data block to send
   * @param size size of data block to send
   * @throw std::system_error if failed to send
   */
  Connection& write(const unsigned char* data, std::size_t size) {
    auto sent = std::size_t{0};

    for (int n = 0;
        sent < size;
        n = send(_socket.descriptor(),
          data + sent, // NOLINT: Pointer arithmetics are required
          size - sent, 0)) {
      if (n == -1) {
        if (errno == EAGAIN) {
          continue;
        }
        throw std::system_error{errno, std::system_category()};
      }
      sent += n;
    }

    return *this;
  }
};

}; // namespace Net
