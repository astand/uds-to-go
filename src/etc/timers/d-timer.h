#pragma once

#include "tickerup.h"

typedef Timers::TickerCounter::systick_t interval_t;

namespace DTimers
{

using namespace Timers;

class Timer : private TickerUp<void*> {
 public:
  /**
   * @brief Construct a new Timer object
   * Sets followed timer parameters:
   * - stopped.
   * - interval = 0, can be started only by explicit call 'Start(n)'
   * - not repeatable.
   */
  Timer() = default;

  /**
   * @brief Construct a new Timer object with parameters
   *
   * @param intv interval
   * @param start is start at the instantiation moment
   * @param repeat is it repeatable timer
   */
  explicit Timer(interval_t intv, bool start = true, bool repeat = true);

  bool Start(interval_t interval);
  bool Start(interval_t interval, bool repeat);

  bool Restart();

  /**
   * @brief Checks if previously started or repeatedely run
   * timer is elapsed
   *
   * @return true if elapse conditions are fullfilled
   */
  bool Elapsed();

  /**
   * @brief Checks if timer is running (active) and elapse event
   * is expected
   *
   * @return true if elapse event is expected, always true for repeatable timers
   */
  bool IsActive() const {
    return (is_active_);
  }

  /**
   * @brief Returns number of ticks to the next elapsing event
   *
   * @return uint32_t n: number of ticks to elapse event.
   *                  0: either timer is stopped or elapse event has already been set
   */
  uint32_t Ticks() const;

  /**
   * @brief Sets internal state to Elapsed at the moment
   *
   */
  void ForceElapse() {
    AdjustInterval(now() - interval_);
  }

  void Stop() {
    is_active_ = false;
  }

 private:
  /**
   * @brief Fixes current system ticks counter
   */
  void FixCurrentTicks() {
    AdjustInterval(now());
  }

  /**
   * @brief Set the Intv object sets new interval_ value with test on 0
   *
   * @param interval new interval value
   * @return was interval_ updated or not
   */
  bool SetIntv(interval_t interval) {
    if (interval > 0) {
      interval_ = interval;
      return true;
    }

    return false;
  }

  /**
   * @brief Gets the gap to next 'elapse' event
   *
   * @param liveticks
   * @return interval_t gap in ticks for active and not overruned timer (otherwise 0)
   */
  interval_t GetTicksToNextElapse(systick_t liveticks) const {
    // check if current tick (@liveticks) is not further than inteval_
    systick_t passed_ticks = liveticks - freeze_ticks_;
    return (interval_ > passed_ticks) ? (interval_ - passed_ticks) : (0);
  }

  /**
   * @brief Set freeze_ticks to new value.
   *
   * @param now_ticks new freeze_tick value
   */
  void AdjustInterval(Timers::TickerCounter::systick_t now_ticks) {
    freeze_ticks_ = now_ticks;
  }

  interval_t interval_ = 0;
  systick_t freeze_ticks_ = 0;

  bool is_active_ = false;
  bool repeat_ = false;

 public:

  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

};

} // namespace
