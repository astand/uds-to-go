#pragma once

#include <uds/session/apps/did-handler.h>

class TestDidReader : public DidHandler {
  virtual DidResult ReadDID(uint32_t, uint8_t* data, size_t, size_t& len_out, NRCs&) override {
    data[0] = 1;
    data[1] = 2;
    data[2] = 3;
    data[3] = 4;
    data[4] = 5;
    data[5] = 6;
    data[6] = 7;
    len_out = 7;
    return DidResult::Positive;
  }

  virtual DidResult WriteDID(uint32_t, const uint8_t*, size_t, NRCs&) override {
    return DidResult::Ignored;
  }

};
