#pragma once

#include <chrono>

#include <cache.hpp>
#include <exporter.hpp>
#include <input.hpp>

namespace Tins {
  class PDU;
} // namespace Tins

namespace Flow {

class Processor {

  Cache _cache;
  Exporter _exporter;
  Plugins::Input _input;
  Cache::iterator _peek_iterator;
  std::chrono::time_point<std::chrono::high_resolution_clock> _time_point;

  void process(Tins::PDU*, timeval);
  void check_idle_timeout(timeval, std::size_t);
  void check_active_timeout(timeval, CacheEntry&);

public:

  Processor();
  void start();
};

} // namespace Flow
