#include <cache.hpp>

namespace Flow {

Cache::RangeType Cache::insert(Type type, Digest digest, Chain chain, Timestamp timestamp) {
  auto& type_records = _records[type];

  auto search = type_records.find(digest);
  if (search == type_records.end()) {
    auto [it, _] = type_records.emplace(digest,
        Record{Properties{1, timestamp, timestamp}, std::move(chain)});

    return {type_records.begin(), it, type_records.end()};
  } else {
    auto& props = search->second.first;

    props.count += 1;
    if (props.first_timestamp > timestamp) {
      props.first_timestamp = timestamp;
    }
    if (props.last_timestamp < timestamp) {
      props.last_timestamp = timestamp;
    }

    return {type_records.begin(), search, type_records.end()};
  }
}

void Cache::erase(Type type, RecordsType::iterator position) {
  _records[type].erase(position);
}

std::size_t Cache::records_size(Type type) const {
  return _records.find(type)->second.size();
}

} // namespace Flow
