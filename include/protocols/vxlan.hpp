#pragma once

#include <tins/tins.h>
#include <tins/constants.h>
#include <tins/detail/pdu_helpers.h>

#include <log.hpp>

namespace Protocols {

/*
 * A PDU for VXLAN protocol
 */
class VXLAN : public Tins::PDU {
  std::vector<std::uint8_t> _buffer;
  std::uint32_t _vni;
public:
  static constexpr auto VXLAN_PORT = 4789;

  static const PDU::PDUType pdu_flag;

  VXLAN(const uint8_t* data, uint32_t sz) : _buffer(data, data + sz){
    std::memcpy(&_vni, data + 4, 4);
    _vni = ntohl(_vni) >> 8;

    inner_pdu(new Tins::EthernetII{data + 8, sz - 8});
  }

  VXLAN* clone() const {
    return new VXLAN(*this);
  }

  uint32_t header_size() const {
    return 8;
  }

  uint32_t vni() const {
    return _vni;
  }

  PDUType pdu_type() const {
    return pdu_flag;
  }

  void write_serialization(uint8_t *data, uint32_t sz) {
    std::memcpy(data, _buffer.data(), sz);
  }
};

static constexpr auto VXLAN_PDU = static_cast<Tins::PDU::PDUType>(Tins::PDU::USER_DEFINED_PDU + 1);
const Tins::PDU::PDUType VXLAN::pdu_flag = VXLAN_PDU;

}
