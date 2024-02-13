#pragma once
#include <chrono>

namespace Utils {
__forceinline uint64_t getTimestamp() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

 
}  // namespace Utils