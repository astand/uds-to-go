#pragma once

#include <stdint.h>

namespace Timers
{
/// @brief Base static class to process base timers counter
class TickerCounter {
 public:
  /// @brief tick counter type
  using systick_t = volatile uint32_t;

  /// @brief Tick counter increment call
  static void ProcessTick() {
    ++root_tick_counter;
  }

  static systick_t RootTickNow() {
    return root_tick_counter;
  }

 private:
  /// @brief counter
  static systick_t root_tick_counter;
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

    return RootTickNow();
  }

  TickerUp() = default;
};

} // namespace Timers
