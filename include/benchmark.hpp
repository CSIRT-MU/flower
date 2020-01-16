#pragma once

#include <iostream>
#include <chrono>

struct ScopedTimer {
  using ClockType = std::chrono::steady_clock;

  template<typename T>
  ScopedTimer(T&& msg):
    msg(std::forward<T>(msg)), start(ClockType::now()) {}

  ScopedTimer(const ScopedTimer&) = delete;
  ScopedTimer(ScopedTimer&&) = delete;
  ScopedTimer& operator=(const ScopedTimer&) = delete;
  ScopedTimer& operator=(ScopedTimer&&) = delete;

  ~ScopedTimer() {
    using namespace std::chrono;
    auto stop = ClockType::now();
    auto duration = stop - start;
    auto ms = duration_cast<milliseconds>(duration).count();
    std::cout << ms << " ms " << msg << std::endl;
  }
  private:
  const std::string msg;
  const ClockType::time_point start;
};
