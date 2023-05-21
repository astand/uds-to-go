#pragma once

#include "tickerup.h"

using interval_t = Timers::TickerCounter::systick_t;

namespace DTimers {

using namespace Timers;

class Timer : private TickerUp<void*> {

 public:
  /// @brief Default empty constuctor (stopped, interval 0, non repeat)
  Timer() = default;

  /// @brief Detailed timer constructor
  /// @param intv interval to repeat
  /// @param start start immediately
  /// @param repeat repeatable or not
  explicit Timer(interval_t intv, bool start = true, bool repeat = true);

  /// @brief Start timer with new interval
  /// @param interval new timer's interval
  void Start(interval_t interval);

  /// @brief Start timer with new interval and repeat setup
  /// @param interval new timer's interval
  /// @param repeat repeat setup
  void Start(interval_t interval, bool repeat);

  /// @brief Restart timer with current configuration
  void Restart();

  /// @brief Checks if the timer elapsed
  /// @return is timer elapsed
  bool Elapsed();

  /// @brief Checks if timer is run (active)
  /// @return is timer active
  bool IsActive() const {
    return (is_active);
  }

  /// @brief Number of ticks to next elapsing
  /// @return 0 if timer is stopped or active elapse event
  uint32_t Ticks() const;

  /// @brief Set elapsed forcibly
  void ForceElapse() {
    SetStartTick(now() - tick_period);
  }

  /// @brief Deactivates timer (elapse is not possible)
  void Stop() {
    is_active = false;
  }

 private:

  /// @brief Returns number of ticks to the next elapse event
  /// @param liveticks current sys tick
  /// @return number of ticks
  interval_t GetTicksToNextElapse(systick_t liveticks) const {
    // check if current tick (@liveticks) is not further than inteval_
    systick_t passed_ticks = liveticks - tick_start;
    return (tick_period > passed_ticks) ? (tick_period - passed_ticks) : (0);
  }

  /// @brief Sets start tick point to new value
  /// @param ticks ticks to set
  void SetStartTick(Timers::TickerCounter::systick_t ticks) {
    tick_start = ticks;
  }

  /// @brief period value
  interval_t tick_period = 0;
  /// @brief current period start tick
  systick_t tick_start = 0;

  /// @brief is timer active
  bool is_active = false;

  /// @brief is timer is_endless
  bool is_endless = false;

 public:
  /// @brief Non copyable
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

};

} // namespace DTimers
