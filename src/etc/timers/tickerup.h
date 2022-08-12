#pragma once

#include <stdint.h>

namespace Timers
{

/**
 * @brief Base static class for keeping and processing main system
 * tick
 *
 */
class TickerCounter {
 public:
  typedef volatile uint32_t systick_t;

  static void ProcessTick() {
    ++systick__;
  }

 protected:
  static systick_t systick__;
};

/**
 * @brief Intermediate wrapper for all users of TickerCounter with
 * ability to make Atomic operation, if Atomic argument will
 * make atomic state enabled in its constructor and disable it
 * back in desctructor
 *
 * @tparam Atomic
 */
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

} // namespace
