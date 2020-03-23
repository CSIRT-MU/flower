#include <pcap.h>

#include "provider.hpp"

extern "C" {

pcap_t* handle;
char errbuf[PCAP_ERRBUF_SIZE];

PluginInfo info() {
  PluginInfo result;
  result.type = PluginType::PacketProvider;
  result.name = "InterfacePacketProvider";
  return result;
}

void init(const char* arg) {
  handle = pcap_open_live(arg, BUFSIZ, 1, 1000, errbuf);
  // TODO: Handle error
}

void finalize() {
  pcap_close(handle);
}

Packet get_packet() {
  struct pcap_pkthdr header;
  const u_char* data = pcap_next(handle, &header);
  Packet result;
  result.data = data;

  if (!data) return result;
  result.len = header.len;
  result.caplen = header.caplen;
  result.timestamp = header.ts.tv_sec;

  return result;
}

}

