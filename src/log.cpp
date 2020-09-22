#include <log.hpp>

#include <cstdio>
#include <cstdarg>

namespace Log {

static Level g_level = Level::INFO;

void set_level(Level level) {
  g_level = level;
}

void log(Level level, const char* fmt, ...) {
  if (g_level > level) return;
  auto out = level == Level::ERROR ? stderr : stdout;

  switch (level) {
    case Level::DEBUG:
      std::fputs("[DEBUG] ", out);
      break;
    case Level::INFO:
      std::fputs("[INFO ] ", out);
      break;
    case Level::WARN:
      std::fputs("[WARN ] ", out);
      break;
    case Level::ERROR:
      std::fputs("[ERROR] ", out);
      break;
  }

  va_list ap;
  va_start(ap, fmt);
  std::vfprintf(out, fmt, ap);
  va_end(ap);
}

} // namespace Log
