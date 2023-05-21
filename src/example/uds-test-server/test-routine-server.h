#pragma once

#include <uds/session/apps/rctrl-router.h>

class ClientRoutineBase : public RoutineHandler {

 public:
  ClientRoutineBase(RoutineRouter& b) : routman(b) {}

 protected:
  RoutineRouter& routman;
};

class RotineServ1 : public ClientRoutineBase, public IProcessable {

 public:
  RotineServ1(RoutineRouter& routiner) : ClientRoutineBase(routiner) {}

  ProcessResult OnRoutine(routine_id_t rid, uint8_t, const uint8_t*, size_t) {

    auto ret = ProcessResult::NOT_HANDLED;

    if (rid == 0x0102) {
      if (true) {
        pending = true;
        // restart routing execution timer
        timeout.Restart();
        // return HANDLED_PENDING status to inform host that final response will be sent later
        ret = ProcessResult::HANDLED_PENDING;
      } else {
        routman.SendRoutineNegResponse(NRCs::IMLOIF);
      }
    }

    return ret;
  }

  virtual void Process() override {

    if (pending && timeout.Elapsed()) {
      pending = false;
      timeout.Stop();

      rcontext.id = 0x0111;
      rcontext.info = 1;
      rcontext.type = 2;
      routman.SendRoutineResponse(rcontext);
    }
  }

 private:

  /// @brief timer for simulating 15s routine execution
  DTimers::Timer timeout{15000u};
  bool pending{false};
};

class RotineServ2 : public ClientRoutineBase {

 public:
  RotineServ2(RoutineRouter& routiner) : ClientRoutineBase(routiner) {}

  ProcessResult OnRoutine(routine_id_t rid, uint8_t, const uint8_t*, size_t) {

    if (rid == 0xff00) {
      if (true) {
        // Ok
        rcontext.id = 0xff00;
        rcontext.info = 99;
        rcontext.type = 64;
        routman.SendRoutineResponse(rcontext);
        return ProcessResult::HANDLED_RESP_NO;
      } else {
        routman.SendRoutineNegResponse(NRCs::IMLOIF);
        return ProcessResult::HANDLED_RESP_NO;
      }
    }

    return ProcessResult::NOT_HANDLED;
  }
};
