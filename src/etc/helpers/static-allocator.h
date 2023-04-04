#pragma once

#include <stdint.h>

/// @brief Wrapper for statically allocated memory buffer
/// @tparam T Type of the elements for storing over allocated memory buffer
/// @tparam N Elements capacity
template<typename T, size_t N>
class StaticMemAllocator {
 public:
  /// @brief Memory begin address getter
  /// @return Pointer to allocated memory
  uint8_t* ptr() {
    return static_cast<uint8_t*>(__raw__);
  }

  /// @brief Buffer size in bytes
  constexpr size_t Size() {
    return N * sizeof(T);
  }

  /// @brief Constructor
  /// @tparam T Type of the elements for storing over allocated memory buffer
  /// @tparam N Elements capacity
  constexpr StaticMemAllocator() = default;

  /// @brief Copy constructor is deleted
  StaticMemAllocator(const StaticMemAllocator&) = delete;

  /// @brief Assignment operator is deleted
  StaticMemAllocator& operator=(const StaticMemAllocator&) = delete;

 private:

  /// @brief Raw memory array
  uint8_t __raw__[N * sizeof(T)] = {0};
};
