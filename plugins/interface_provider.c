#include <pcap.h>

#include <input.h>

static pcap_t *handle;
static char errbuf[PCAP_ERRBUF_SIZE];

/**
 * Function info must return PluginInfo struct.
 */
InfoRT
info()
{
  return (InfoRT){
    "InterfaceInput",
    INPUT_PLUGIN,
    "Input from network interface\n"
    "The argument is a name of the interface used to capture packets\n"
  };
}

/**
 * Function init initializes plugin. Plugin should open capture device
 * in this function.
 */
InitRT
init(const char *arg)
{
  handle = pcap_open_live(arg, BUFSIZ, 1, 1000, errbuf);

  if (!handle) {
    return (InitRT){RESULT_ERROR, errbuf};
  }

  return (InitRT){RESULT_OK, ""};
}

/**
 * Function finalize is used when application is being closed. It should
 * free all allocated resources.
 */
FinalizeRT
finalize()
{
  pcap_close(handle);
}

/**
 * Function get packet should return GetPacketResult structure. Since failure
 * can occur, the return type must contain information about it.
 */
GetPacketRT
get_packet()
{
  struct pcap_pkthdr *header;
  const u_char *data;
  int status = pcap_next_ex(handle, &header, &data);

  /* Check if packet was captured */
  if (status == 1) {
    return (GetPacketRT){CAPTURE_PACKET, {
      data,
      header->len,
      header->caplen,
      header->ts.tv_sec,
      header->ts.tv_usec
    }};
  }

  /* Check if buffer timeout happened */
  if (status == 0) {
    return (GetPacketRT){CAPTURE_TIMEOUT, {}};
  }

  /* Check if error happened */
  if (status == PCAP_ERROR) {
    return (GetPacketRT){CAPTURE_INPUT_ERROR, {}};
  }

  /* Else return end of input, in interface mode this should not happen */
  return (GetPacketRT){CAPTURE_END_OF_INPUT, {}};
}
