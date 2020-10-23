#include <parser.hpp>

#include <unordered_map>

namespace Parser {

static std::unordered_map<std::uint16_t, ParserFun> udp_parsers;

void
insert_udp_parser(std::uint16_t port, ParserFun f)
{
  udp_parsers.emplace(port, f);
}

static Tins::PDU*
parse_udp(const Tins::UDP* udp) {
  auto search = udp_parsers.find(udp->dport());

  if (search == udp_parsers.end())
    return nullptr;

  auto raw = udp->find_pdu<Tins::RawPDU>();
  return search->second(raw->payload().data(), raw->payload_size());
}


std::unique_ptr<Tins::PDU>
parse(const std::uint8_t* data, std::uint32_t size)
{
  Tins::PDU* pdu = new Tins::EthernetII{data, size};

  /* Parse TCP and UDP inner protocols */
  for (Tins::PDU* p = pdu; p != nullptr; p = p->inner_pdu()) {
    Tins::PDU* inner;
    switch (p->pdu_type()) {
      case Tins::PDU::PDUType::UDP:
        inner = parse_udp(dynamic_cast<Tins::UDP*>(p));
        if (inner)
          p->inner_pdu(inner);
        break;
      case Tins::PDU::PDUType::TCP:
        /* TCP inner protocol parsing is not implemented yet */
        break;
      default:
        continue;
    }
  }

  return std::unique_ptr<Tins::PDU>{pdu};
}

} // namespace Parser
