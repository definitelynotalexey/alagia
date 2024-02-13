#include "App.h"

C_Client::C_Client() : clientSocket(INVALID_SOCKET) {
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed." << std::endl;
    return;
  }
  std::cout << "C_ClientApplication initialized" << std::endl;
}

C_Client::~C_Client() {}

bool C_Client::Connect(const char* serverAddress, int port) {
  // Create a socket for the client
  clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (clientSocket == INVALID_SOCKET) {
    std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
    return false;
  }

  // Resolve server address
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  inet_pton(AF_INET, serverAddress, &serverAddr.sin_addr);

  // Connect to the server
  if (connect(clientSocket, reinterpret_cast<SOCKADDR*>(&serverAddr),
              sizeof(serverAddr)) == SOCKET_ERROR) {
    std::cerr << "Failed to connect to server: " << WSAGetLastError()
              << std::endl;
    closesocket(clientSocket);
    return false;
  }

  std::cout << "Connected to server at " << serverAddress << ":" << port
            << std::endl;

  return true;
}

void C_Client::Disconnect() {
  // Close the socket
  if (clientSocket != INVALID_SOCKET) {
    closesocket(clientSocket);
    clientSocket = INVALID_SOCKET;
    std::cout << "Disconnected from server." << std::endl;
  }

  // Cleanup Winsock
  WSACleanup();
}

bool C_Client::SendPacket(const PacketHeader_t& header, const void* packetData,
                          uint64_t packetSize) {
  // Send the packet header to the server
  if (send(clientSocket, reinterpret_cast<const char*>(&header),
           sizeof(PacketHeader_t), 0) == SOCKET_ERROR) {
    std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
    return false;
  }

  // XOR encryption key, perfect, absolutely up with modern day standard
  const uint8_t xorKey = 0xAB;

  // Encrypt packet data using XOR encryption
  std::vector<uint8_t> encryptedPacket(packetSize);
  const uint8_t* data = reinterpret_cast<const uint8_t*>(packetData);
  for (uint64_t i = 0; i < packetSize; ++i) {
    encryptedPacket[i] = data[i] ^ (xorKey + i);
  }

  // Send the encrypted packet data to the server
  if (send(clientSocket, reinterpret_cast<const char*>(encryptedPacket.data()),
           packetSize, 0) == SOCKET_ERROR) {
    std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
    return false;
  }

  std::cout << std::format("Sent encrypted packet [{} bytes] to server.",
                           (int)packetSize)
            << std::endl;
  return true;
}

// RECEIVE UNTIL YOU CAN!!!
bool C_Client::ReceiveAll(SOCKET socket, char* buffer, int size) {
  int bytesReceived = 0;
  while (bytesReceived < size) {
    int result = recv(socket, buffer + bytesReceived, size - bytesReceived, 0);
    if (result == SOCKET_ERROR || result == 0) {
      return false;
    }
    bytesReceived += result;
  }
  return true;
}
