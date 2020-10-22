#pragma once

#include <tins/tins.h>
#include <tins/constants.h>
#include <tins/detail/pdu_helpers.h>

#include <log.hpp>

namespace Protocols {

/*
 * A PDU for GRE protocol
 */
class GRE : public Tins::PDU {
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
    /* 
     * Unique protocol identifier. For user-defined PDUs, you **must**
     * use values greater or equal to PDU::USER_DEFINED_PDU;
     */
    static const PDU::PDUType pdu_flag;

    static constexpr auto GRE_HEADER_BASE_SIZE = 4;

    /*
     * Constructor from buffer. This constructor will be called while
     * sniffing packets, whenever a PDU of this type is found. 
     * 
     * The "data" parameter points to a buffer of length "sz". 
     */
    GRE(const uint8_t* data, uint32_t sz) {
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
    GRE* clone() const {
        return new GRE(*this);
    }
    
    /*
     * Retrieves the size of this PDU. 
     */
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

    uint32_t checksum() const {
      return _header.checksum;
    }

    uint32_t key() const {
      return _header.key;
    }
    
    uint32_t seq() const {
      return _header.seq;
    }

    /*
     * This method must return pdu_flag.
     */
    PDUType pdu_type() const {
      return pdu_flag;
    }
    
    /*
     * Serializes the PDU. The serialization output should be written
     * to the buffer pointed to by "data", which is of size "sz". The
     * "sz" parameter will be equal to the value returned by 
     * DummyPDU::header_size. 
     *
     * Note that before libtins 4.0, there would be an extra
     * const PDU* parameter after "sz" which would contain the parent
     * PDU. On libtins 4.0 this parameter was removed as you can get
     * the parent PDU by calling PDU::parent_pdu()
     */
    void write_serialization(uint8_t *data, uint32_t sz) {
      // TODO(dudoslav): Serialize
      // std::memcpy(data, buffer_.data(), sz);
    }
private:
    std::vector<uint8_t> buffer_;
};

const Tins::PDU::PDUType GREPDU::pdu_flag = PDU::USER_DEFINED_PDU;

}
