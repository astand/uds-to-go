#include "d-timer.h"

namespace DTimers
{

Timer::Timer(interval_t intv, bool start, bool repeat)
  : tick_start(0), is_active(start), is_endless(repeat)
{
  // First: set interval (with check on 0 value)
  SetIntv(intv);

  if (is_active)
  {
    // if is active try to Restart immediately
    Restart();
  }
}

bool Timer::Start(interval_t interval)
{
  is_active = false;

  if (SetIntv(interval))
  {
    FixCurrentTicks();
    is_active = true;
  }

  return is_active;
}

bool Timer::Start(interval_t interval, bool repeat)
{
  if (Start(interval))
  {
    is_endless = repeat;
    return true;
  }

  return false;
}

bool Timer::Restart()
{
  return Start(tick_period);
}

bool Timer::Elapsed()
{
  systick_t liveticks = now();

  if (IsActive() && (GetTicksToNextElapse(liveticks) == 0))
  {
    is_active = is_endless;
    SetStartTick(liveticks);
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
