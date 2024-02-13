#include <iostream>
#include <memory>

#include "App.h"

int main() {
  SetConsoleTitleA("Server");

  std::unique_ptr<C_Application> app = std::make_unique<C_Application>();

  if (!app) {
    std::cerr << "Failed to initialize application." << std::endl;
    return 1;
  }

  // Start the server on port 6969
  if (!app->Start(PORT)) {
    std::cerr << "Failed to start the server." << std::endl;
    return 1;
  }

  return 0;
}