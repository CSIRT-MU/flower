#pragma once

#include <tins/tins.h>
#include <tins/constants.h>
#include <tins/detail/pdu_helpers.h>

#include <log.hpp>

namespace Protocols {

/*
 * A PDU for GRE protocol
 */
class GREPDU : public Tins::PDU {
  struct {
    uint8_t checksum_present;
    uint8_t key_present;
    uint8_t seq_present;
    uint16_t protocol;
    uint16_t checksum;
    uint32_t key;
    uint32_t seq;
  } _header;

  uint32_t _size;

public:

    static const PDU::PDUType pdu_flag;

    static constexpr auto GRE_HEADER_BASE_SIZE = 4;

    /* https://tools.ietf.org/html/rfc2890 */
    GREPDU(const uint8_t* data, uint32_t sz) {
      uint8_t bits = *data;
      _header.checksum_present = bits & 0b10000000;
      _header.key_present = bits & 0b00100000;
      _header.seq_present = bits & 0b00010000;

      std::memcpy(&_header.protocol, data + 2, 2);
      _header.protocol = htons(_header.protocol);

      const uint8_t* pos = data + GRE_HEADER_BASE_SIZE;
      _size = GRE_HEADER_BASE_SIZE;
      if (_header.checksum_present) {
        std::memcpy(&_header.checksum, pos, 2);
        _header.checksum = ntohs(_header.checksum);
        pos += 4;
        _size += 4;
      }
      if (_header.key_present) {
        std::memcpy(&_header.key, pos, 4);
        _header.key = ntohl(_header.key);
        pos += 4;
        _size += 4;
      }
      if (_header.seq_present) {
        std::memcpy(&_header.seq, pos, 4);
        _header.seq = ntohl(_header.seq);
        pos += 4;
        _size += 4;
      }

      buffer_.insert(buffer_.end(), data, pos);

      inner_pdu(
          Tins::Internals::pdu_from_flag(
            static_cast<Tins::Constants::Ethernet::e>(_header.protocol),
            data + _size,
            sz - _size,
            true
            )
          );
    }
    
    /*
     * Clones the PDU. This method is used when copying PDUs.
     */
    GREPDU* clone() const {
        return new GREPDU(*this);
    }
    
    uint32_t header_size() const {
      return _size;
    }

    uint8_t checksum_present() const {
      return _header.checksum_present;
    }

    uint8_t key_present() const {
      return _header.key_present;
    }

    uint8_t seq_present() const {
      return _header.seq_present;
    }

    uint16_t protocol() const {
      return _header.protocol;
    }

    uint16_t checksum() const {
      return _header.checksum;
    }

    uint32_t key() const {
      return _header.key;
    }
    
    uint32_t seq() const {
      return _header.seq;
    }

    PDUType pdu_type() const {
      return pdu_flag;
    }
    
    void write_serialization(uint8_t *data, uint32_t sz) {
      std::memcpy(data, buffer_.data(), sz);
    }
private:
    std::vector<uint8_t> buffer_;
};

static constexpr auto GREPDU_TYPE = static_cast<Tins::PDU::PDUType>(Tins::PDU::USER_DEFINED_PDU + 0);
const Tins::PDU::PDUType GREPDU::pdu_flag = GREPDU_TYPE;

}
