#include "d-timer.h"

namespace DTimers
{

Timer::Timer(interval_t intv, bool start, bool repeat)
  : freeze_ticks_(0), is_active_(start), repeat_(repeat)
{
  // First: set interval (with check on 0 value)
  SetIntv(intv);

  if (is_active_)
  {
    // if is active try to Restart immediately
    Restart();
  }
}

bool Timer::Start(interval_t interval)
{
  is_active_ = false;

  if (SetIntv(interval))
  {
    FixCurrentTicks();
    is_active_ = true;
  }

  return is_active_;
}

bool Timer::Start(interval_t interval, bool repeat)
{
  if (Start(interval))
  {
    repeat_ = repeat;
    return true;
  }

  return false;
}

bool Timer::Restart()
{
  return Start(interval_);
}

bool Timer::Elapsed()
{
  systick_t liveticks = now();

  if (IsActive() && (GetTicksToNextElapse(liveticks) == 0))
  {
    is_active_ = repeat_;
    AdjustInterval(liveticks);
    return true;

  }

  return false;
}


uint32_t Timer::Ticks() const
{
  if (IsActive())
  {
    return static_cast<uint32_t>(GetTicksToNextElapse(now()));
  }
  else
  {
    return 0;
  }
}

} // namespace
