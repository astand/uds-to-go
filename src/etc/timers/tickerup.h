#pragma once

#include <stdint.h>

namespace Timers
{
/// @brief Base static class to process base timers counter
class TickerCounter {
 public:
  /// @brief tick counter type
  typedef volatile uint32_t systick_t;

  /// @brief Tick counter increment call
  static void ProcessTick() {
    ++systick__;
  }

 protected:
  /// @brief counter
  static systick_t systick__;
};

/// @brief Intermediate wrapper for all users of TickerCounter with
/// ability to make Atomic operation, if Atomic argument will
/// make atomic state enabled in its constructor and disable it
/// back in desctructor
/// @tparam Atomic RAII type to fulfill atomicy
template<class Atomic>
class TickerUp : public TickerCounter {
 protected:
  static systick_t now() {
    Atomic guard;
    (void)guard;

    return systick__;
  }

  TickerUp() = default;
};

} // namespace Timers
