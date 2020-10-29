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

enum ResultType {
  RESULT_OK,
  RESULT_ERROR
};

struct InitResult {
  enum ResultType type;
  const char* error_msg;
};

enum GetPacketResultType {
  CAPTURE_PACKET = 0,
  CAPTURE_TIMEOUT,
  CAPTURE_END_OF_INPUT,
  CAPTURE_INPUT_ERROR
};

struct GetPacketResult {
  enum GetPacketResultType type;
  struct Packet packet;
};

typedef struct GetPacketResult GetPacketRT;
typedef struct InitResult InitRT;
typedef void FinalizeRT;
