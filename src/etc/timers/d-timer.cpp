#include "d-timer.h"

namespace DTimers
{

Timer::Timer(interval_t intv, bool start, bool repeat)
  : tick_period(intv), tick_start(0), is_active(start), is_endless(repeat)
{
  if (is_active)
  {
    // if is active try to Restart immediately
    Restart();
  }
}

void Timer::Start(interval_t interval)
{
  tick_period = interval;
  SetStartTick(now());
  is_active = true;
}

void Timer::Start(interval_t interval, bool repeat)
{
  Start(interval);
  is_endless = repeat;
}

void Timer::Restart()
{
  Start(tick_period);
}

bool Timer::Elapsed()
{
  if (IsActive())
  {
    if (tick_period == 0)
    {
      is_active = is_endless;
      return true;
    }
    else
    {
      systick_t liveticks = now();

      if (GetTicksToNextElapse(liveticks) == 0)
      {
        is_active = is_endless;
        SetStartTick(liveticks);
        return true;
      }
    }
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
