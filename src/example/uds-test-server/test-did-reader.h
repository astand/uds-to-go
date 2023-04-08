#pragma once

#include <uds/session/apps/did-handler.h>

class TestDidReader : public DidHandler {

 public:
  TestDidReader(const SessionInfo& infoInstance) : sessInfoInstance(infoInstance) {}

  virtual DidResult ReadDID(uint32_t did, uint8_t* data, size_t, size_t& len_out, NRCs&) override {

    DidResult handleResult = DidResult::Positive;

    switch (did) {
      case (0x2200u):
        data[0] = 1;
        data[1] = 2;
        data[2] = 3;
        data[3] = 4;
        data[4] = 5;
        data[5] = 6;
        data[6] = 7;
        len_out = 7;
        break;

      case (0x00A0u):
        data[0] = sessInfoInstance.currSession;
        len_out = 1u;
        break;

      case (0x00A1u):
        data[0] = sessInfoInstance.secLevel;
        len_out = 1u;
        break;

      default:
        handleResult = DidResult::Ignored;
        break;
    }

    return handleResult;
  }

  virtual DidResult WriteDID(uint32_t, const uint8_t*, size_t, NRCs&) override {

    return DidResult::Ignored;
  }

 private:
  const SessionInfo& sessInfoInstance;

};
