#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <etc/helpers/ophelper.h>

#include <helpers/IKeeper.h>

#include <uds/session/uds-app-client.h>

// SID + SF + RoutineID + routineInfo
static constexpr size_t RoutineHeadSize = 5;

typedef uint16_t routine_id_t;

typedef struct {
  uint16_t id;
  uint8_t type;
  uint8_t info;
} RoutineContext_t;

class RoutineRouter;

/**
 * @brief Interface class for Routine Control clients
 *
 */
class RoutineHandler {
 public:

  /**
   * @brief Main client callback function. Routine manager will call client
   * via this method. Client must check @rid handle request if rid is its responsibility
   *
   * @param rid code of routine
   * @param rtype rtype one of 3 possible values (1 - start, 2 - stop, 3 - get result)
   * @param data pointer to data with params for routine execution
   * @param size size of data
   * @return ProcessResult
   */
  virtual ProcessResult OnRoutine(routine_id_t rid, uint8_t rtype, const uint8_t* data, size_t size) = 0;

 protected:
  void LoadRoutineContext(routine_id_t rid, uint8_t reqtype) {
    rcontext.id = rid;
    rcontext.type = reqtype;
    rcontext.info = 0;
  }

  /**
   * @brief client routine info
   */
  RoutineContext_t rcontext;
};


/**
 * @brief Interface class for providing to clients API
 * to response on routine requests and SiClient for routine
 * control UDS service
 */
class RoutineRouter : public UdsAppClient {
 public:
  RoutineRouter(UdsAppManager& bserver, RoutineHandler& r) : UdsAppClient(bserver), rhandler(r) {}

  /**
   * @brief Main response function for clients
   *
   * @param rid code of routine
   * @param rtype one of 3 possible values (1 - start, 2 - stop, 3 - get result)
   * @param data pointer to response routine result related data
   * @param size length of data
   * @return SendResult
   */
  SendResult SendRoutineResponse(routine_id_t rid, uint8_t rtype, uint8_t rinfo,
      const uint8_t* data, size_t size) {

    if (size + RoutineHeadSize >= udsRouter.PubBuffCapacity) {
      return SendResult::OVRF;
    }

    // Setup head of routine response
    udsRouter.pubBuff[0] = sidhelper::to_response(sidhelper::RC);
    udsRouter.pubBuff[1] = rtype;
    HWREGH(udsRouter.pubBuff + 2) = ophelper::to_be_u16(rid);
    udsRouter.pubBuff[4] = rinfo;

    // Copy response data
    memcpy(udsRouter.pubBuff + RoutineHeadSize, data, size);

    // Sending response
    udsRouter.pubRespLength = RoutineHeadSize + size;
    udsRouter.SendResponse(udsRouter.pubBuff, udsRouter.pubRespLength);

    return SendResult::OK;
  }

  SendResult SendRoutineResponse(routine_id_t rid, uint8_t rtype, uint8_t rinfo = 0) {

    return SendRoutineResponse(rid, rtype, rinfo, nullptr, 0);
  }

  SendResult SendRoutineResponse(RoutineContext_t& rcon, const uint8_t* data, size_t size) {

    return SendRoutineResponse(rcon.id, rcon.type, rcon.type, data, size);
  }

  SendResult SendRoutineResponse(RoutineContext_t& rcon) {

    return SendRoutineResponse(rcon.id, rcon.type, rcon.type, nullptr, 0);
  }

  /**
   * @brief Negative response function for clients
   *
   * @param n Negative response code value, cannot be PositiveResponse
   * @return SendResult
   */
  SendResult SendRoutineNegResponse(NRCs n) {

    if (n != NRCs::PR) {
      udsRouter.SendNegResponse(sidhelper::RC, n);
    }

    return SendResult::OK;
  }

  virtual bool IsServiceSupported(sid_t sid, size_t& minlength, bool& subfunc) override {

    (void) subfunc;

    if (sid == sidhelper::RC) {
      minlength = 3u;
      return true;
    }

    return false;
  }

  ProcessResult OnAppIndication(const IndicationInfo& inf) {

    // Minimal service length (4) is tested on the initial service check
    if (inf.head.SI != sidhelper::RC) {
      return ProcessResult::NOT_HANDLED;
    } else if (inf.head.SF == 0 || inf.head.SF > 3) {
      udsRouter.SendNegResponse(NRCs::SFNS);
      return ProcessResult::HANDLED_RESP_NO;
    }

    ProcessResult pres = ProcessResult::NOT_HANDLED;
    routine_id_t routine_id = ophelper::from_be_u16(HWREGH(inf.data + 2));

    pres = rhandler.OnRoutine(routine_id, inf.head.SF, inf.data + 4, inf.size - 4);

    if (pres == ProcessResult::NOT_HANDLED) {
      // send ROOR to client, no one routine handler has been found
      udsRouter.SendNegResponse(NRCs::ROOR);
    } else if (pres == ProcessResult::HANDLED_PENDING) {
      udsRouter.StartPending(60000u, 1000u);
    }

    return pres;
  }

  void OnAppConfirmation(S_Result res) {

    (void) res;
  }

 protected:

  RoutineHandler& rhandler;

};

template<size_t N>
class MultiRoutineHandler : public MemAsKeeper<RoutineHandler, N> {
 public:
  MultiRoutineHandler() : MemAsKeeper<RoutineHandler, N>() {}

  virtual ProcessResult OnRoutine(routine_id_t rid, uint8_t rtype, const uint8_t* data, size_t size) {
    auto ret = ProcessResult::NOT_HANDLED;
    RoutineHandler* handler {nullptr};

    for (size_t i = 0; i < this->Count(); i++) {
      if (this->TryReadElem(i, handler)) {
        ret = handler->OnRoutine(rid, rtype, data, size);

        if (ret != ProcessResult::NOT_HANDLED) {
          break;
        }
      }
    }

    return ret;
  }
};
