#include <pcap.h>

#include <input.h>

pcap_t* handle;
char errbuf[PCAP_ERRBUF_SIZE];

struct PluginInfo info() {
  struct PluginInfo result;
  result.type = INPUT_PLUGIN;
  result.name = "FileInput";
  return result;
}

void init(const char* arg) {
  handle = pcap_open_offline(arg, errbuf);
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
