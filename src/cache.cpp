#include <cache.hpp>

namespace Flow {

Cache::RecordsType::iterator
Cache::insert(Digest digest, Chain chain, Timestamp timestamp) {
  auto search = _records.find(digest);
  if (search == _records.end()) {
    auto [it, _] = _records.emplace(digest,
        Record{Properties{1, timestamp, timestamp}, std::move(chain)});

    return it;
  } else {
    auto& props = search->second.first;

    props.count += 1;
    if (tsgeq(props.first_timestamp, timestamp)) {
      props.first_timestamp = timestamp;
    }
    if (tsgeq(timestamp, props.last_timestamp)) {
      props.last_timestamp = timestamp;
    }

    return search;
  }
}

void Cache::erase(Digest digest) {
  _records.erase(digest);
}

std::size_t Cache::size() const {
  return _records.size();
}

Cache::RecordsType::iterator Cache::find(Digest digest) {
  return _records.find(digest);
}

Cache::RecordsType::iterator Cache::begin() {
  return _records.begin();
}

Cache::RecordsType::iterator Cache::end() {
  return _records.end();
}

} // namespace Flow
