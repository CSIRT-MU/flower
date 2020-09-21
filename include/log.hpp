#pragma once

#include <utility>

namespace Log {

enum class Level : unsigned int {
  DEBUG,
  INFO,
  WARN,
  ERROR
};

void set_level(Level);
void log(Level, const char*, ...);

template<typename... Args>
void debug(const char* fmt, Args... args) {
  log(Level::DEBUG, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void info(const char* fmt, Args... args) {
  log(Level::INFO, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void warn(const char* fmt, Args... args) {
  log(Level::WARN, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
void error(const char* fmt, Args... args) {
  log(Level::ERROR, fmt, std::forward<Args>(args)...);
}

} // namespace Log
