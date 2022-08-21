#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <helpers/IKeeper.h>

#include <uds/session/uds-service-handler.h>

#define HWREGH(x) (*(uint16_t*)(x))
#define V16_TO_BE(x)  (( ((x) >> 8) & 0xffu ) | ( ((x) & 0xffu) << 8u ))
#define V16_FROM_BE(x) (V16_TO_BE(x))

// SID + SF + RoutineID + routineInfo
static constexpr size_t RoutineHeadSize = 5;

typedef uint16_t routine_id_t;

typedef struct
{
  uint16_t id;
  uint8_t type;
  uint8_t info;
} RoutineContext_t;

/**
 * @brief Interface class for providing to clients API
 * to response on routine requests and SiClient for routine
 * control UDS service
 */
class RoutinerBase : public UdsServiceHandler {
 public:

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
    if (size + RoutineHeadSize >= MAX_RESP_SIZE) {
      return SendResult::OVRF;
    }

    // Setup head of routine response
    rtr1.tData[0] = RESPONSE_ON_SID(PUDS_SI_RoutineControl);
    rtr1.tData[1] = rtype;
    HWREGH(rtr1.tData + 2) = V16_TO_BE(rid);
    rtr1.tData[4] = rinfo;

    // Copy response data
    memcpy(rtr1.tData + RoutineHeadSize, data, size);

    // Sending response
    rtr1.tLength = RoutineHeadSize + size;
    rtr1.SendResponse(rtr1.tData, rtr1.tLength);

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
  SendResult SendRoutineNegResponse(NRCs_t n) {
    if (n != NRC_PR) {
      rtr1.SendNegResponse(PUDS_SI_RoutineControl, n);
    }

    return SendResult::OK;
  }

 protected:
  // protected constructor to prevent instance
  RoutinerBase(UdsServerBase& r, size_t maxlen) : UdsServiceHandler(r), MAX_RESP_SIZE(maxlen) {}

  const size_t MAX_RESP_SIZE;
};

/**
 * @brief Interface class for Routine Control clients
 *
 */
class RoutineClient {
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
  RoutineClient(RoutinerBase& routiner) : routman(routiner) {}

  void LoadRoutineContext(routine_id_t rid, uint8_t reqtype) {
    rcontext.id = rid;
    rcontext.type = reqtype;
    rcontext.info = 0;
  }

  /**
   * @brief Routine control manager. Provide API to client for sending responses
   */
  RoutinerBase& routman;

  /**
   * @brief client routine info
   */
  RoutineContext_t rcontext;
};

/**
 * @brief Final class for RoutinerBase, can keep up to T clients (based on IKeeper)
 * Must be instantiate single per programm
 *
 * @tparam T number of clients available for keeping in IKeeper
 * @tparam M maximum UDS transmit packet size
 */
template<int32_t T, size_t M>
class Routiner : public IKeeper<RoutineClient>, public RoutinerBase {
 public:
  Routiner(UdsServerBase& router_) : IKeeper<RoutineClient>(list, T), RoutinerBase(router_, M)  {}

  /* SiClient */
  ProcessResult OnIndication(const IndicationInfo& inf) {
    // Minimal service length (4) is tested on the initial service check
    if (rtr1.data_info.head.SI != PUDS_SI_RoutineControl) {
      return ProcessResult::NOT_HANDLED;
    }
    else if (rtr1.data_info.head.SF == 0 || rtr1.data_info.head.SF > 3) {
      rtr1.SendNegResponse(NRC_SFNS);
      return ProcessResult::HANDLED_RESP_NO;
    }
    else if (!rtr1.GetSession().sec_level) {
      rtr1.SendNegResponse(NRC_SAD);
      return ProcessResult::HANDLED_RESP_NO;
    }

    ProcessResult pres = ProcessResult::NOT_HANDLED;
    routine_id_t routine_id = V16_FROM_BE(HWREGH(inf.data + 2));

    for (size_t i = 0; i < Count(); i++) {
      pres = list[i]->OnRoutine(routine_id, rtr1.data_info.head.SF, inf.data + 4, inf.size - 4);

      if (pres != ProcessResult::NOT_HANDLED) {
        break;
      }
    }

    if (pres == ProcessResult::NOT_HANDLED) {
      // send ROOR to client, no one routine handler has been found
      rtr1.SendNegResponse(NRC_ROOR);
    }

    // Routiner will never return state for positive response by
    // UdsServerBase to avoid response duplications. Clients of routiner
    // have to only use API of RoutinerBase for sending responses,
    // which uses direct UdsServerBase reponse sending
    return ProcessResult::HANDLED_RESP_NO;
  }

  /* SiClient */
  ProcessResult OnConfirmation(S_Result res) {
    (void)res;
    return ProcessResult::NOT_HANDLED;
  }

 private:

  RoutineClient* list[T];
};
