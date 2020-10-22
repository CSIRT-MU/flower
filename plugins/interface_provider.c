#include <pcap.h>

#include <input.h>

static pcap_t *handle;
static char errbuf[PCAP_ERRBUF_SIZE];

InfoRT info() {
  struct PluginInfo result;
  result.type = INPUT_PLUGIN;
  result.name = "InterfaceInput";
  result.description =
      "Input from network interface\n"
      "The argument is a name of the interface used to capture packets\n";
  return result;
}

InitRT init(const char *arg) {
  struct InitResult result = {OK, NULL};
  handle = pcap_open_live(arg, BUFSIZ, 1, 1000, errbuf);

  if (handle) {
    return result;
  } else {
    result.type = ERROR;
    result.error_msg = errbuf;
    return result;
  }
}

FinalizeRT finalize() { pcap_close(handle); }

GetPacketRT get_packet() {
  struct pcap_pkthdr *header;
  const u_char *data;
  int status = pcap_next_ex(handle, &header, &data);

  struct GetPacketResult result = {END_OF_INPUT, {}};

  if (status == 1) {
    result.type = PACKET;
    result.packet.data = data;
    result.packet.len = header->len;
    result.packet.caplen = header->caplen;
    result.packet.sec = header->ts.tv_sec;
    result.packet.usec = header->ts.tv_usec;
    return result;
  }

  if (status == 0) {
    result.type = TIMEOUT;
    return result;
  }

  if (status == PCAP_ERROR) {
    result.type = INPUT_ERROR;
    return result;
  }

  return result;
}
