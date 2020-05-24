#include <pcap.h>

#include <input.h>

pcap_t* handle;
char errbuf[PCAP_ERRBUF_SIZE];

struct PluginInfo info() {
  struct PluginInfo result;
  result.type = INPUT_PLUGIN;
  result.name = "InterfaceInput";
  return result;
}

void init(const char* arg) {
  handle = pcap_open_live(arg, BUFSIZ, 1, 1000, errbuf);
  // TODO(dudoslav): Handle error
}

void finalize() {
  pcap_close(handle);
}

struct Packet get_packet() {
  struct pcap_pkthdr header;
  const u_char* data = pcap_next(handle, &header);
  struct Packet result;
  result.data = data;

  if (!data) return result;
  result.len = header.len;
  result.caplen = header.caplen;
  result.timestamp = header.ts.tv_sec;

  return result;
}
