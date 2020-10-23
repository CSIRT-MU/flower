#pragma once

#include <memory>

#include <tins/tins.h>

/**
 * Parser namespace wraps libtins' parsing functionality
 * and adds additional parsing capabilities such as parsing
 * protocols based on UDP/TCP ports.
 */
namespace Parser {

using ParserFun = Tins::PDU* (*)(const std::uint8_t*, std::uint32_t);

void insert_udp_parser(std::uint16_t port, ParserFun f);

template<typename T>
Tins::PDU*
default_parser(const std::uint8_t* data, std::uint32_t size)
{
  return new T{data, size};
}

template<typename T>
void
register_udp_parser(std::uint16_t port)
{
  insert_udp_parser(port, &default_parser<T>);
}

/**
 * Parse raw packet buffer into Tins::PDU.
 */
std::unique_ptr<Tins::PDU> parse(const std::uint8_t*, std::uint32_t);

} // namespace Parser
