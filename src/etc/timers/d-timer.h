#pragma once

#include "tickerup.h"

typedef Timers::TickerCounter::systick_t interval_t;

namespace DTimers
{

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
  /// @return is timer started
  bool Start(interval_t interval);

  /// @brief Start timer with new interval and repeat setup
  /// @param interval new timer's interval
  /// @param repeat repeat setup
  /// @return is timer started
  bool Start(interval_t interval, bool repeat);

  /// @brief Restart timer with current configuration
  /// @return is timer started
  bool Restart();

  /// @brief Checks if the timer elapsed
  /// @return is timer elapsed
  bool Elapsed();

  /// @brief Checks if timer is run (active)
  /// @return is timer active
  bool IsActive() const {
    return (is_active_);
  }

  /// @brief Number of ticks to next elapsing
  /// @return 0 if timer is stopped or active elapse event
  uint32_t Ticks() const;

  /// @brief Set elapsed forcibly
  void ForceElapse() {
    AdjustInterval(now() - interval_);
  }

  /// @brief Stop timer
  void Stop() {
    is_active_ = false;
  }

 private:
  void FixCurrentTicks() {
    AdjustInterval(now());
  }

  bool SetIntv(interval_t interval) {
    if (interval > 0) {
      interval_ = interval;
      return true;
    }

    return false;
  }

  interval_t GetTicksToNextElapse(systick_t liveticks) const {
    // check if current tick (@liveticks) is not further than inteval_
    systick_t passed_ticks = liveticks - freeze_ticks_;
    return (interval_ > passed_ticks) ? (interval_ - passed_ticks) : (0);
  }

  void AdjustInterval(Timers::TickerCounter::systick_t now_ticks) {
    freeze_ticks_ = now_ticks;
  }

  interval_t interval_ = 0;
  systick_t freeze_ticks_ = 0;

  bool is_active_ = false;
  bool repeat_ = false;

 public:
  /// @brief Non copyable
  Timer(const Timer&) = delete;
  Timer& operator=(const Timer&) = delete;

};

} // namespace DTimers
