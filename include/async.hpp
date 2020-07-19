#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Async {

/**
 * Timer class used for calling specific function
 * in intervals.
 */
template<typename Rep, typename Period,
  typename Clock = std::chrono::high_resolution_clock>
class Timer {
  using Interval = std::chrono::duration<Rep, Period>;

  Interval _interval;
  std::thread _executor;
  std::atomic<bool> _running = false;
  std::mutex _mutex = {};
  std::condition_variable _cv = {};

public:

  explicit Timer(Interval interval): _interval(interval) {}

  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

  Timer(Timer&&) = delete;
  Timer& operator=(Timer&&) = delete;

  /**
   * Function that takes a callable object/function and calls it
   * in set intervals.
   * @param fun callable
   */
  template<typename Fun>
  void start(Fun&& fun) {
    _running = true;
    _executor = std::thread{[&, fun](){
      for (;;) {
        auto lock = std::unique_lock{_mutex};
        auto before = Clock::now();
        fun();
        if (_running) {
          _cv.wait_until(lock, before + _interval);
        } else {
          break;
        }
      }
    }};
  }

  /**
   * Function that stops execution of timer function and also
   * cleans the executor thread.
   */
  void stop() {
    // TODO(dudoslav): If thread is in valid state
    _running = false;
    _cv.notify_all();
    _executor.join();
  }

  ~Timer() {
    stop();
  }
};

/**
 * Thread safe implementation of queue
 */
template<typename T>
class Queue {
  std::queue<T> _queue;
  mutable std::mutex _mutex;

public:

  /**
   * Pushes new item into queue. This method uses
   * emplace.
   * @param args arguments for constructing item
   */
  template<typename... Args>
  void push(Args&&... args) {
    const auto lock = std::lock_guard{_mutex};
    _queue.emplace(std::forward<Args>(args)...);
  }

  /**
   * Pops an item from queue and returns it. This
   * method moves item out from container.
   * @return T front item from queue
   */
  T pop() {
    const auto lock = std::lock_guard{_mutex};
    auto result = std::move(_queue.front());
    _queue.pop();
    return result;
  }

  /**
   * Function for checking if queue is empty.
   * @return bool true if queue is empty
   */
  bool empty() const {
    const auto lock = std::lock_guard{_mutex};
    return _queue.empty();
  }

  /**
   * Getter for size of queue.
   * @return std::size_t number of items in queue
   */
  std::size_t size() const {
    const auto lock = std::lock_guard{_mutex};
    return _queue.size();
  }
};

/**
 * Worker pool implementation using basic thread safe queue.
 * This pool uses queue to store jobs and vector of workers
 * to take these jobs and execute them. A job is a tuple of
 * arguments that is used to invoke function given in constructor.
 */
template<typename... Args>
class Pool {
  std::vector<std::thread> _workers;
  Queue<std::tuple<Args...>> _jobs;
  std::mutex _mutex = {};
  std::condition_variable _cv = {};
  std::atomic<bool> _finished = false;

public:

  /**
   * Constructs Pool object that executes given function across
   * all worker threads.
   * @param fun function executed by workers
   * @param threads number of threads, defaults to number of system cores
   */
  template<typename Fun>
  explicit Pool(Fun&& fun, std::size_t threads = std::thread::hardware_concurrency()):
    _workers(threads) {
    auto worker = [&](){
      do {
        auto ul = std::unique_lock{_mutex};
        _cv.wait(ul, [&](){ return !_jobs.empty() || _finished; });
        if (_jobs.empty()) { continue; }

        auto job = _jobs.pop();
        ul.unlock();
        std::apply(fun, std::move(job));
      } while (!_finished);
    };

    for (auto& w: _workers) {
      w = std::thread{worker};
    }
  }

  Pool(const Pool&) = delete;
  Pool& operator=(const Pool&) = delete;

  Pool(Pool&&) = delete;
  Pool& operator=(Pool&&) = delete;

  ~Pool() {
    _finished = true;
    _cv.notify_all();
    for (auto& worker: _workers) {
      worker.join();
    }
  }

  /**
   * Adds another job into job queue. The worker function will
   * be called with given arguments.
   * @param args arguments for worker thread
   */
  template<typename... A>
  void run(A&&... args) {
    _jobs.push(std::forward<A>(args)...);
    _cv.notify_one();
  }
};

} // namespace Async
