#pragma once
#include "../shared/Packet.h"
#include "../shared/Utils.h"
#include <map>


class C_Application {
 public:
  C_Application();
  ~C_Application();

  // Starts up our server
  bool Start(int port);
  // This runs per-client, handles all parsing and all work for each client
  void HandleClient(SOCKET clientSocket, std::map<SOCKET, Session_t>& sessions);
  bool SendPacket(SOCKET socket, const PacketHeader_t& header,
                  const void* packetData, uint64_t packetSize);
  // Self-explanatory
  bool ReceiveAll(SOCKET socket, char* buffer, int size);

 private:
  WSADATA wsaData;                       // Winsock data structure
  SOCKET listenSocket;                   // Socket for listening
  std::map<SOCKET, Session_t> sessions;  // Map of client sockets to sessions
};