#pragma once

#include <chrono>

#include <cache.hpp>
#include <exporter.hpp>

namespace Tins {
  class PDU;
} // namespace Tins

namespace Flow {

class Processor {

  Cache _cache;
  Exporter _exporter;
  Cache::iterator _peek_iterator;
  std::chrono::time_point<std::chrono::high_resolution_clock> _time_point;

  std::uint32_t _active_timeout;
  std::uint32_t _idle_timeout;

  void process(Tins::PDU*, timeval);
  void check_idle_timeout(std::uint32_t, std::size_t);
  void check_active_timeout(std::uint32_t, CacheEntry&);

public:

  Processor();
  void start();
};

} // namespace Flow
