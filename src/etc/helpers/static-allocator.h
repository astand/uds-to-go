#pragma once

/* ---------------------------------------------------------------------------- */
template<typename T, size_t N>
class StaticMemAllocator {
 public:
  uint8_t* ptr() {
    return static_cast<uint8_t*>(__raw__);
  }

  StaticMemAllocator() = default;
  StaticMemAllocator(const StaticMemAllocator&) = delete;
  StaticMemAllocator& operator=(const StaticMemAllocator&) = delete;

 private:
  uint8_t __raw__[N * sizeof(T)] {0};
};
