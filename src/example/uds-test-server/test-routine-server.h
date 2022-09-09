#pragma once

#include <uds/session/apps/rctrl-router.h>

class ClientRoutineBase : public RoutineHandler {
 public:
  ClientRoutineBase(RoutineRouter& b) : routman(b) {}

 protected:
  RoutineRouter& routman;
};

class RotineServ1 : public ClientRoutineBase {
 public:
  RotineServ1(RoutineRouter& routiner) : ClientRoutineBase(routiner) {}
  ProcessResult OnRoutine(routine_id_t rid, uint8_t rtype, const uint8_t* data, size_t size) {
    if (rid == 0x0102) {
      if (true) {
        // Ok
        rcontext.id = 0x0102;
        rcontext.info = 1;
        rcontext.type = 2;
        routman.SendRoutineResponse(rcontext);
        return ProcessResult::HANDLED_RESP_OK;
      }
      else {
        routman.SendRoutineNegResponse(NRC_IMLOIF);
      }
    }

    return ProcessResult::NOT_HANDLED;
  }

};

class RotineServ2 : public ClientRoutineBase {
 public:
  RotineServ2(RoutineRouter& routiner) : ClientRoutineBase(routiner) {}
  ProcessResult OnRoutine(routine_id_t rid, uint8_t rtype, const uint8_t* data, size_t size) {
    if (rid == 0xff00) {
      if (true) {
        // Ok
        rcontext.id = 0xff00;
        rcontext.info = 99;
        rcontext.type = 64;
        routman.SendRoutineResponse(rcontext);
        return ProcessResult::HANDLED_RESP_OK;
      }
      else {
        routman.SendRoutineNegResponse(NRC_IMLOIF);
        return ProcessResult::HANDLED_RESP_OK;
      }
    }

    return ProcessResult::NOT_HANDLED;
  }
};
