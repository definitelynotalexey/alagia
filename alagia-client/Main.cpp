#include "App.h"
__forceinline void login(C_Client* clientApp, uint64_t keyInput) {
  LoginPacket_t loginPacket;
  loginPacket.keyHash = keyInput ^ 0x1337;  // PERFECT XOR!!!

  std::cout << std::dec << keyInput << std::endl;
  std::cout << std::hex << keyInput << std::endl;

  std::cout << std::dec << loginPacket.keyHash << std::endl;
  std::cout << std::hex << loginPacket.keyHash << std::endl;

  // Calculate the payload size dynamically based on the actual size of
  // loginPacket data, this is gay and needs rework
  uint64_t payloadSize = sizeof(loginPacket);

  auto header = PacketHeader_t::Create(payloadSize, PacketType_t::LOGIN);

  // Send the packet with the calculated payload size
  clientApp->SendPacket(header, &loginPacket, payloadSize);
}

int main() {
  SetConsoleTitleA("Client");

  // Initialize C_ClientApplication using unique pointer
  std::unique_ptr<C_Client> clientApp = std::make_unique<C_Client>();

  if (!clientApp) {
    std::cerr << "Failed to initialize client application." << std::endl;
    return 1;
  }

  // Connect to the server
  const char* serverAddress = "127.0.0.1";
  if (!clientApp->Connect(serverAddress, PORT)) {
    std::cerr << "Failed to connect to server." << std::endl;
    return 1;
  }

  std::cout << "Enter license: " << std::endl;

  uint64_t keyInput;
  std::cin >> keyInput;

  login(clientApp.get(), keyInput);

  LoginResp_t loginResponse = clientApp->Receive<LoginResp_t>();
  std::cout << loginResponse.authCode << std::endl;

  if (loginResponse.authCode == 0)
    std::cout << "Invalid key :(" << std::endl;
  else
    std::cout << "Key very nice!" << std::endl;

  clientApp->Disconnect();

  Sleep(5000);

  return 0;
}