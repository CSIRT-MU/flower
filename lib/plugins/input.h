#pragma once

#include <plugin.h>

#define INPUT_PLUGIN 1

/**
 * Structure representing raw packet
 */
struct Packet {
  /**
   * Pointer to raw packet bytes.
   */
  const unsigned char* data;

  /**
   * Total length of packet.
   */
  unsigned int len;

  /**
   * Length of captured portion of packet.
   */
  unsigned int caplen;

  /**
   * Timestamp of packet capture.
   */
  unsigned int timestamp;
};
