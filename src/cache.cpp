#include <cache.hpp>

namespace Flow {

[[nodiscard]]
static bool
tsgeq(timeval f, timeval s)
{
  return f.tv_sec == s.tv_sec ? f.tv_usec > s.tv_usec : f.tv_sec > s.tv_sec;
}

Cache::iterator
Cache::insert_record(std::size_t digest, timeval ts, Buffer values)
{
  auto search = find(digest);
  if (search == end()) {
    /* If this record is new add it to cache */
    auto [it, _] = emplace(digest, CacheEntry{{1, ts, ts}, std::move(values)});

    return it;
  } else {
    /* If this record already exists update counter */

    auto& [props, _] = search->second;
    props.count += 1;

    if (tsgeq(props.flow_start, ts)) {
      props.flow_start = ts;
    }
    if (tsgeq(ts, props.flow_end)) {
      props.flow_end = ts;
    }

    return search;
  }
}

} // namespace Flow
