#pragma once

#include <protocol.hpp>
#include <definition.hpp>

namespace Flow {

class Serializer {

  Flow::Definition _def;

  using BufferType = std::vector<std::byte>;

  // DIGEST
  [[nodiscard]] std::size_t digest(const IP&) const;
  [[nodiscard]] std::size_t digest(const IPv6&) const;
  [[nodiscard]] std::size_t digest(const TCP&) const;
  [[nodiscard]] std::size_t digest(const UDP&) const;
  [[nodiscard]] std::size_t digest(const DOT1Q&) const;
  [[nodiscard]] std::size_t digest(const MPLS&) const;
  [[nodiscard]] std::size_t digest(const VXLAN&) const;

  // FIELDS
  [[nodiscard]] BufferType fields([[maybe_unused]] const IP&) const;
  [[nodiscard]] BufferType fields([[maybe_unused]] const IPv6&) const;
  [[nodiscard]] BufferType fields([[maybe_unused]] const TCP&) const;
  [[nodiscard]] BufferType fields([[maybe_unused]] const UDP&) const;
  [[nodiscard]] BufferType fields([[maybe_unused]] const DOT1Q&) const;
  [[nodiscard]] BufferType fields([[maybe_unused]] const MPLS&) const;
  [[nodiscard]] BufferType fields([[maybe_unused]] const VXLAN&) const;

  // VALUES
  [[nodiscard]] BufferType values(const IP&) const;
  [[nodiscard]] BufferType values(const IPv6&) const;
  [[nodiscard]] BufferType values(const TCP&) const;
  [[nodiscard]] BufferType values(const UDP&) const;
  [[nodiscard]] BufferType values(const DOT1Q&) const;
  [[nodiscard]] BufferType values(const MPLS&) const;
  [[nodiscard]] BufferType values(const VXLAN&) const;

  [[nodiscard]] BufferType fields([[maybe_unused]] const Properties&) const;
  [[nodiscard]] BufferType values(const Properties&) const;

  [[nodiscard]] BufferType fields(const Chain&) const;
  [[nodiscard]] BufferType values(const Chain&) const;

public:

  [[nodiscard]] std::size_t digest(const Chain&) const;
  [[nodiscard]] BufferType fields(const Chain&, const Properties&) const;
  [[nodiscard]] BufferType values(const Chain&, const Properties&) const;

  void set_definition(Flow::Definition);

};

} // namespace Flow
