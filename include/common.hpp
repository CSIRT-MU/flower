#pragma once

#include <numeric>

/**
 * Hash function of arbitrary arity. This function MUST BE non commutative.
 * @param args numbers to reduce to one hash
 * @return hash reduced from all provided numbers
 */
template<typename... N>
constexpr std::size_t combine(std::size_t f, N... n) {
  constexpr std::size_t SHIFT_LEFT = 6;
  constexpr std::size_t SHIFT_RIGHT = 2;
  constexpr std::size_t SEED = 0x9e3779b9;

  auto args = { f, static_cast<std::size_t>(n)... };

  return std::accumulate(std::next(args.begin()), args.end(), *(args.begin()),
      [](std::size_t acc, std::size_t a) constexpr {
      return a ^ ((acc<<SHIFT_LEFT) + (acc>>SHIFT_RIGHT) + SEED);
      });
}
