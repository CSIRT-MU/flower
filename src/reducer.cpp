#include <reducer.hpp>

#include <unordered_map>

namespace Reducer {

static std::unordered_map<Tins::PDU::PDUType, std::unique_ptr<Flow::Flow>> reducers;

void
insert_reducer(Tins::PDU::PDUType pdu_type, std::unique_ptr<Flow::Flow> reducer)
{
  reducers.emplace(pdu_type, std::move(reducer));
}

const Flow::Flow*
reducer(Tins::PDU::PDUType pdu_type)
{
  auto search = reducers.find(pdu_type);

  if (search == reducers.end())
    return nullptr;

  return search->second.get();
}

} // namespace Reducer
