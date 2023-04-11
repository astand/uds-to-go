#pragma once

#include <timers/tickerup.h>
#include <helpers/IProcessable.h>
#include <chrono>


class TickerWrapper : public IProcessable {

  virtual void Process() override {
    static auto first_stamp = std::chrono::steady_clock::now();

    auto now_stamp = std::chrono::steady_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(now_stamp - first_stamp).count();

    while (elapsed_us > 1000) {
      elapsed_us -= 1000;
      first_stamp += std::chrono::microseconds(1000);

      Timers::TickerCounter::ProcessTick();
    }
  }
};
