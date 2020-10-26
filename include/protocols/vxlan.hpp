#pragma once

#include <tins/tins.h>
#include <tins/constants.h>
#include <tins/detail/pdu_helpers.h>

#include <log.hpp>

namespace Protocols {

/*
 * A PDU for VXLAN protocol
 */
class VXLANPDU : public Tins::PDU {
  std::vector<std::uint8_t> _buffer;
  std::uint32_t _vni;
public:
  static constexpr auto VXLAN_PORT = 4789;

  static const PDU::PDUType pdu_flag;

  VXLANPDU(const uint8_t* data, uint32_t sz) : _buffer(data, data + 8){
    std::memcpy(&_vni, data + 4, 4);
    _vni = ntohl(_vni) >> 8;

    inner_pdu(new Tins::EthernetII{data + 8, sz - 8});
  }

  VXLANPDU* clone() const {
    return new VXLANPDU(*this);
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

static constexpr auto VXLANPDU_TYPE = static_cast<Tins::PDU::PDUType>(Tins::PDU::USER_DEFINED_PDU + 1);
const Tins::PDU::PDUType VXLANPDU::pdu_flag = VXLANPDU_TYPE;

}
