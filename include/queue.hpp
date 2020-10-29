#include <chrono>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace Async {

template<typename T>
class Queue {
  std::queue<T> _queue;
  mutable std::mutex _mutex;
  mutable std::condition_variable _cv;

public:

  template<typename... Args>
  void
  push(Args&&... args)
  {
    const auto lock = std::lock_guard{_mutex};
    _queue.emplace(std::forward<Args>(args)...);
    _cv.notify_one();
  }

  T
  pop()
  {
    const auto lock = std::lock_guard{_mutex};
    auto result = std::move(_queue.front());
    _queue.pop();
    return result;
  }

  template<typename Rep, typename Period>
  bool
  wait_for(const std::chrono::duration<Rep, Period> rel_time) const
  {
    auto lock = std::unique_lock{_mutex};
    return _cv.wait_for(lock, rel_time, [&](){ return !_queue.empty(); });
  }
};

} // namespace Async
