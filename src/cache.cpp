#include <cache.hpp>

namespace Flow {

[[nodiscard]] static bool tsgeq(timeval f, timeval s) {
  return f.tv_sec == s.tv_sec ? f.tv_usec > s.tv_usec : f.tv_sec > s.tv_sec;
}

Cache::Entry
Cache::insert(Digest digest, Type type, Values values, Timestamp ts)
{
  auto search = _records.find(digest);
  if (search == _records.end()) {
    auto [it, _] = 
      _records.emplace(digest, Record{{1, ts, ts}, type, std::move(values)});
    return it;
  } else {
    auto& [props, _, __] = search->second;

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

std::size_t
Cache::size() const
{
  return _records.size();
}

void
Cache::erase(Digest digest)
{
  _records.erase(digest);
}

Cache::Entry
Cache::find(Digest digest)
{
  return _records.find(digest);
}

Cache::Entry
Cache::begin()
{
  return _records.begin();
}

Cache::Entry 
Cache::end()
{
  return _records.end();
}

} // namespace Flow
