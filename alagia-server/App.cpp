#include "App.h"

#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

C_Application::C_Application() {
  // Initialize Winsock
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "WSAStartup failed." << std::endl;
    return;
  }
  std::cout << "C_Application initialized" << std::endl;
}

C_Application::~C_Application() {
  // Cleanup Winsock
  WSACleanup();
}

bool C_Application::Start(int port) {
  // Create a socket for listening
  listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listenSocket == INVALID_SOCKET) {
    std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
    return false;
  }

  // Bind the socket to the local address and port
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serverAddr.sin_port = htons(port);
  if (bind(listenSocket, reinterpret_cast<SOCKADDR*>(&serverAddr),
           sizeof(serverAddr)) == SOCKET_ERROR) {
    std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
    closesocket(listenSocket);
    return false;
  }

  // Start listening on the socket
  if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
    std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
    closesocket(listenSocket);
    return false;
  }

  std::cout << "Server started on port " << port << std::endl;

  // Accept incoming connections and handle them
  while (true) {
    // Accept a new client connection
    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET) {
      std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
      continue;
    }

    // Add new session for the client
    sessions.emplace(clientSocket, Session_t(sessions.size() + 1));

    // Create a new thread to handle communication with the client
    std::thread clientThread(&C_Application::HandleClient, this, clientSocket,
                             std::ref(sessions));
    clientThread.detach();  // Detach the thread to let it run independently
  }

  return true;
}

void C_Application::HandleClient(SOCKET clientSocket,
                                 std::map<SOCKET, Session_t>& sessions) {
  if (clientSocket == INVALID_SOCKET) {  // safety measure
    sessions.erase(clientSocket);
    closesocket(clientSocket);
  }

  // Get the session for this client
  Session_t& session = sessions.at(clientSocket);

  PacketHeader_t header;

  while (true) {
    // Receive the packet header
    if (!ReceiveAll(clientSocket, reinterpret_cast<char*>(&header),
                    sizeof(PacketHeader_t))) {
      std::cerr << "Failed to receive packet header." << std::endl;
      sessions.erase(clientSocket);
      closesocket(clientSocket);
      return;
    }

    // Allocate buffer for packet payload
    char* payloadBuffer = new char[header.payloadSize];

    // Receive the packet payload based on packet type
    if (!ReceiveAll(clientSocket, payloadBuffer, header.payloadSize)) {
      std::cerr << "Failed to receive payload." << std::endl;
      delete[] payloadBuffer;
      sessions.erase(clientSocket);
      closesocket(clientSocket);
      return;
    }

    // De-XOR the received payload
    const uint8_t xorKey = 0xAB;
    for (uint64_t i = 0; i < header.payloadSize; ++i) {
      payloadBuffer[i] ^= (xorKey + i);
    }

    // Parse the packet based on packet type
    switch (header.packetType) {
      case PacketType_t::LOGIN: {
        // Parse LoginPacket_t
        if (header.payloadSize != sizeof(LoginPacket_t)) {
          std::cerr << "Invalid payload size for LOGIN packet." << std::endl;
          delete[] payloadBuffer;
          sessions.erase(clientSocket);
          closesocket(clientSocket);
          return;
        }

        LoginPacket_t* loginPacket =
            reinterpret_cast<LoginPacket_t*>(payloadBuffer);

        std::cout << "keyHash hex => " << std::hex << loginPacket->keyHash
                  << std::endl;
        std::cout << "keyHash dec => " << std::dec << loginPacket->keyHash
                  << std::endl;

        bool has_permissions = (loginPacket->keyHash == 0x4c02138087b57);

        LoginResp_t loginResp;
        loginResp.authCode = has_permissions;
        uint64_t payloadSize = sizeof(loginResp);

        auto header = PacketHeader_t::Create(payloadSize, PacketType_t::LOGIN);

        // Send the packet with the calculated payload size
        this->SendPacket(clientSocket, header, &loginResp, payloadSize);

        break;
      }
      case PacketType_t::HEARTBEAT: {
        // Parse HeartbeatPacket
        if (header.payloadSize != sizeof(HeartbeatPacket)) {
          std::cerr << "Invalid payload size for HEARTBEAT packet."
                    << std::endl;
          delete[] payloadBuffer;
          sessions.erase(clientSocket);
          closesocket(clientSocket);
          return;
        }
        HeartbeatPacket* heartbeatPacket =
            reinterpret_cast<HeartbeatPacket*>(payloadBuffer);

        std::cout << "Violation => " << heartbeatPacket->violationCode
                  << std::endl;
        break;
      }
      // Add cases for other packet types as needed
      default:
        std::cerr << "Unknown packet type." << std::endl;
        delete[] payloadBuffer;
        sessions.erase(clientSocket);
        closesocket(clientSocket);
        return;
    }

    // Clean up payload buffer
    delete[] payloadBuffer;
  }
}

bool C_Application::SendPacket(SOCKET socket, const PacketHeader_t& header,
                               const void* packetData, uint64_t packetSize) {
  // Send the packet header to the server
  if (send(socket, reinterpret_cast<const char*>(&header),
           sizeof(PacketHeader_t), 0) == SOCKET_ERROR) {
    std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
    return false;
  }

  // XOR encryption key
  const uint8_t xorKey = 0xAB;

  // Encrypt packet data using XOR encryption
  std::vector<uint8_t> encryptedPacket(packetSize);
  const uint8_t* data = reinterpret_cast<const uint8_t*>(packetData);
  for (uint64_t i = 0; i < packetSize; ++i) {
    encryptedPacket[i] = data[i] ^ (xorKey + i);
  }

  // Send the encrypted packet data to the server
  if (send(socket, reinterpret_cast<const char*>(encryptedPacket.data()),
           packetSize, 0) == SOCKET_ERROR) {
    std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
    return false;
  }

  std::cout << std::format("Sent encrypted packet [{} bytes] to client.",
                           (int)packetSize)
            << std::endl;
  return true;
}

// We want the server to "receive while there is something to receive"
bool C_Application::ReceiveAll(SOCKET socket, char* buffer, int size) {
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