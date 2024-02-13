#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")
#include <cstdint>

#include "Utils.h"

constexpr int PORT = 6969;
constexpr uint64_t MAX_TIMESTAMP_TOLERANCE_MS = 1000;

struct Session_t {
  uint64_t ID;       // ID of the session
  uint64_t keyHash;  // Hash of the user's key
  bool isLoggedIn;   // Flag indicating if the client is logged in, this will
                     // help us prevent auth-skipping

  Session_t(uint64_t id) : ID(id), keyHash(0x0), isLoggedIn(false) {}
};

struct LoginPacket_t {
  uint64_t keyHash;  // user key that will be authenticated
};

struct LoginResp_t {
  short authCode;
};

struct HeartbeatPacket {
  uint64_t violationCode;  // idk this can be whatever
};

enum class PacketType_t {
  HANDSHAKE,
  LOGIN,
  HEARTBEAT,
  FETCH_MODULE,
  INJECT_FINISHED
};

struct PacketHeader_t {
  uint64_t payloadSize;
  PacketType_t packetType;
  uint64_t timestamp;  // To prevent replay attacks, not the best, but suits the
                       // job for now

  PacketHeader_t() {}

  PacketHeader_t(uint64_t payloadSize_, PacketType_t packetType_,
                 uint64_t timestamp_)
      : payloadSize(payloadSize_),
        packetType(packetType_),
        timestamp(timestamp_) {}

  static PacketHeader_t Create(uint64_t payloadSize, PacketType_t packetType) {
    return PacketHeader_t(payloadSize, packetType, Utils::getTimestamp());
  }
};