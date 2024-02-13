#pragma once
#include <WS2tcpip.h>

#include <iostream>

#include "../shared/Packet.h"
#include "../shared/Utils.h"

/*
 * NOTE: The server is not async, meaning you cannot simply send packets however
 * and whenever you'd like.
 */

constexpr const char* SERVER_ADDRESS = "127.0.0.1";

class C_Client {
 public:
  C_Client();
  ~C_Client();

  bool Connect(const char* serverAddress, int port);
  void Disconnect();
  bool SendPacket(const PacketHeader_t& header, const void* packetData,
                  uint64_t packetSize);
  bool ReceiveAll(SOCKET socket, char* buffer, int size);

  /* We need to do this here,
   * otherwise, the compiler won't generate the function code
   * for all possible types unless we explicitly instruct it to do so.
   */
  template <typename T>
  T Receive() {
    T receivedPacket;

    // Receive the packet header
    PacketHeader_t header;
    if (!ReceiveAll(clientSocket, reinterpret_cast<char*>(&header),
                    sizeof(PacketHeader_t))) {
      std::cerr << "Failed to receive packet header." << std::endl;
    }

    // Decrypt packet data using XOR decryption
    const uint8_t xorKey = 0xAB;
    std::vector<uint8_t> decryptedPacket(header.payloadSize);
    if (!ReceiveAll(clientSocket,
                    reinterpret_cast<char*>(decryptedPacket.data()),
                    header.payloadSize)) {
      std::cerr << "Failed to receive payload." << std::endl;
    }

    for (uint64_t i = 0; i < header.payloadSize; ++i) {
      decryptedPacket[i] ^= (xorKey + i);
    }

    // Copy decrypted packet data to the received packet object
    std::memcpy(&receivedPacket, decryptedPacket.data(), sizeof(T));

    return receivedPacket;
  }

 private:
  WSADATA wsaData;      // Winsock data structure
  SOCKET clientSocket;  // Socket for the client
};