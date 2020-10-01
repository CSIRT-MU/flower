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
   * Timestamp seconds of packet capture.
   */
  unsigned int sec;

  /**
   * Timestamp nanoseconds of packet capture.
   */
  unsigned int usec;
};

enum InitResultType {
  OK,
  ERROR
};

struct InitResult {
  enum InitResultType type;
  const char* error_msg;
};

enum GetPacketResultType {
  TIMEOUT,
  END_OF_INPUT,
  PACKET
};

struct GetPacketResult {
  enum GetPacketResultType type;
  struct Packet packet;
};

typedef struct GetPacketResult GetPacketRT;
typedef struct InitResult InitRT;
typedef void FinalizeRT;
