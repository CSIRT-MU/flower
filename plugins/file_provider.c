#include <pcap.h>

#include <input.h>

static pcap_t *handle;
static char errbuf[PCAP_ERRBUF_SIZE];

InfoRT
info()
{
  return (InfoRT){
    "FileInput",
    INPUT_PLUGIN,
    "Input from file in cap/pcap format\n"
    "The argument is a path to the cap/pcap file\n"
  };
}

InitRT
init(const char *arg)
{
  handle = pcap_open_offline(arg, errbuf);

  if (!handle) {
    return (InitRT){RESULT_ERROR, errbuf};
  }

  return (InitRT){RESULT_OK, ""};
}

FinalizeRT
finalize()
{
  pcap_close(handle);
}

GetPacketRT
get_packet()
{
  struct pcap_pkthdr header;
  const u_char *data = pcap_next(handle, &header);

  if (data) {
    return (GetPacketRT){CAPTURE_PACKET, {
      data,
      header.len,
      header.caplen,
      header.ts.tv_sec,
      header.ts.tv_usec
    }};
  }

  return (GetPacketRT){CAPTURE_END_OF_INPUT, {}};
}
