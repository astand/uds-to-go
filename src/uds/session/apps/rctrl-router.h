#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <helpers/IKeeper.h>

#include <uds/session/uds-service-handler.h>

#ifndef HWREGH
#define HWREGH(x) (*(uint16_t*)(x))
#endif

#ifndef V16_TO_BE
#define V16_TO_BE(x)  (( ((x) >> 8) & 0xffu ) | ( ((x) & 0xffu) << 8u ))
#endif

#ifndef V16_FROM_BE
#define V16_FROM_BE(x) (V16_TO_BE(x))
#endif

// SID + SF + RoutineID + routineInfo
static constexpr size_t RoutineHeadSize = 5;

typedef uint16_t routine_id_t;

typedef struct
{
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
class RoutineRouter : public UdsServiceHandler {
 public:
  RoutineRouter(UdsServerBase& bserver, RoutineHandler& r) : UdsServiceHandler(bserver), rhandler(r) {}

  /**
   * @brief Main response function for clients
   *
   * @param rid code of routine
   * @param rtype one of 3 possible values (1 - start, 2 - stop, 3 - get result)
   * @param data pointer to response routine result related data
   * @param size length of data
   * @return SendResult
   */
  SendResult SendRoutineResponse(routine_id_t rid, uint8_t rtype, uint8_t rinfo, const uint8_t* data, size_t size) {
    if (size + RoutineHeadSize >= rtr1.TX_SIZE) {
      return SendResult::OVRF;
    }

    // Setup head of routine response
    rtr1.pubBuff[0] = SID_response(SIDs::RC);
    rtr1.pubBuff[1] = rtype;
    HWREGH(rtr1.pubBuff + 2) = V16_TO_BE(rid);
    rtr1.pubBuff[4] = rinfo;

    // Copy response data
    memcpy(rtr1.pubBuff + RoutineHeadSize, data, size);

    // Sending response
    rtr1.pubSize = RoutineHeadSize + size;
    rtr1.SendResponse(rtr1.pubBuff, rtr1.pubSize);

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
      rtr1.SendNegResponse(SIDs::RC, n);
    }

    return SendResult::OK;
  }

  ProcessResult OnIndication(const IndicationInfo& inf) {
    // Minimal service length (4) is tested on the initial service check
    if (inf.head.SI != SIDs::RC) {
      return ProcessResult::NOT_HANDLED;
    }
    else if (inf.head.SF == 0 || inf.head.SF > 3) {
      rtr1.SendNegResponse(NRCs::SFNS);
      return ProcessResult::HANDLED_RESP_NO;
    }
    // else if (!rtr1.GetSession().sec_level) {
    //   rtr1.SendNegResponse(NRCs::SAD);
    //   return ProcessResult::HANDLED_RESP_NO;
    // }

    ProcessResult pres = ProcessResult::NOT_HANDLED;
    routine_id_t routine_id = V16_FROM_BE(HWREGH(inf.data + 2));

    pres = rhandler.OnRoutine(routine_id, inf.head.SF, inf.data + 4, inf.size - 4);

    if (pres == ProcessResult::NOT_HANDLED) {
      // send ROOR to client, no one routine handler has been found
      rtr1.SendNegResponse(NRCs::ROOR);
    }

    return pres;
  }

  /* SiClient */
  ProcessResult OnConfirmation(S_Result res) {
    (void)res;
    return ProcessResult::NOT_HANDLED;
  }

 protected:

  RoutineHandler& rhandler;

};

template<size_t N>
class MultiRoutineHandler : public AsKeeper<RoutineHandler> {
 public:
  MultiRoutineHandler() : AsKeeper<RoutineHandler>(rarray, N) {}
  virtual ProcessResult OnRoutine(routine_id_t rid, uint8_t rtype, const uint8_t* data, size_t size) {
    auto ret = ProcessResult::NOT_HANDLED;
    RoutineHandler* handler {nullptr};

    for (size_t i = 0; i < Count(); i++) {
      Item(i, handler);
      ret = handler->OnRoutine(rid, rtype, data, size);

      if (ret != ProcessResult::NOT_HANDLED) {
        break;
      }
    }

    return ret;
  }

 private:

  RoutineHandler* rarray[N] {nullptr};
};
