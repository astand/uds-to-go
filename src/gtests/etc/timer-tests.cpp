#include <etc/timers/d-timer.h>

#include <gtest/gtest.h>

using namespace DTimers;

void RollRootCounter(uint32_t ticks) {

  while (ticks--) {
    Timers::TickerCounter::ProcessTick();
  }
}

// Demonstrate some basic assertions.
TEST(DTimerTests, GeneralTests) {

  // no explicit construction
  // Timer d = {1};
  Timer t0{};

  EXPECT_EQ(t0.Elapsed(), false);
  t0.Start(10);
  EXPECT_EQ(t0.Elapsed(), false);

  RollRootCounter(9);
  EXPECT_EQ(t0.Elapsed(), false);

  // 10th tick should cause elapsed event
  RollRootCounter(1);
  EXPECT_EQ(t0.Elapsed(), true);
  // next elapsed after true must be false
  EXPECT_EQ(t0.Elapsed(), false);

  // not repeatable timer, no elapsed event
  RollRootCounter(10);
  EXPECT_EQ(t0.Elapsed(), false);

  t0.Start(10, true);
  RollRootCounter(10);
  EXPECT_EQ(t0.Elapsed(), true);

  RollRootCounter(9);
  EXPECT_EQ(t0.Elapsed(), false);

  RollRootCounter(1);
  EXPECT_EQ(t0.Elapsed(), true);

  RollRootCounter(9);
  t0.Stop();
  EXPECT_EQ(t0.Elapsed(), false);
  RollRootCounter(1);
  EXPECT_EQ(t0.Elapsed(), false);
  RollRootCounter(5);
  EXPECT_EQ(t0.Elapsed(), false);

}


TEST(DTimerTests, ZeroInterval) {

  Timer t1;

  t1.Start(0);

  EXPECT_EQ(t1.IsActive(), true);
  EXPECT_EQ(t1.Elapsed(), true);
  // not repeatable
  EXPECT_EQ(t1.Elapsed(), false);
  EXPECT_EQ(t1.IsActive(), false);

  EXPECT_EQ(t1.Ticks(), 0);

  t1.Stop();
  EXPECT_EQ(t1.IsActive(), false);
  EXPECT_EQ(t1.Elapsed(), false);

  t1.Start(0, true);

  EXPECT_EQ(t1.Elapsed(), true);
  EXPECT_EQ(t1.Elapsed(), true);
  EXPECT_EQ(t1.Elapsed(), true);
}

TEST(DTimersTests, Restart) {

  Timer t2{10};

  EXPECT_EQ(t2.IsActive(), true);

  RollRootCounter(9);
  EXPECT_EQ(t2.Elapsed(), false);
  EXPECT_EQ(t2.Ticks(), 1);

  t2.Restart();

  EXPECT_EQ(t2.Ticks(), 10);

  RollRootCounter(2);
  EXPECT_FALSE(t2.Elapsed());

  RollRootCounter(8);
  EXPECT_TRUE(t2.Elapsed());
}
