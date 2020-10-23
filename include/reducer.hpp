#pragma once

#include <memory>

#include <tins/tins.h>
#include <toml.hpp>

#include <flows/flow.hpp>

/**
 * Reducer namespace handles reducing PDUs into Flow specific
 * data, such as values, fields, and digest.
 */
namespace Reducer {

void insert_reducer(Tins::PDU::PDUType, std::unique_ptr<Flow::Flow>);

template<typename T>
void register_reducer(Tins::PDU::PDUType pdu_type, const toml::table& config) {
  insert_reducer(pdu_type, std::make_unique<T>(config));
}

const Flow::Flow* reducer(Tins::PDU::PDUType);

} // namespace Reducer
