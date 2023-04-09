#include "etc/helpers/ophelper.h"
#include "uds-app-manager.h"
#include "uds-app-client.h"

// ISO 14229-1 7.3.2 table 2 (p. 25)
constexpr uint8_t range1begin = 0x10u;
constexpr uint8_t range1end = 0x3f;
constexpr uint8_t range2begin = 0x83u;
constexpr uint8_t range2end = 0x88u;

/// @brief Checks if NRC code is
/// @param nrc Negative response code in response
/// @return true when NRC can be sent when positive response is suppressed
constexpr bool nrc_to_send_when_pos_suppressed(const NRCs nrc) {

  return ((nrc == NRCs::ROOR)
      || (nrc == NRCs::SNS)
      || (nrc == NRCs::SFNS));
}

/// @brief Check if the service identificator can be handled
/// @param sid Service value
/// @return true if requested service can be handled, otherwise false
constexpr bool is_sid_processable(SIDs sid) {

  return ophelper::is_in_range<range1begin, range1end>(SID_to_byte(sid)) ||
      ophelper::is_in_range<range2begin, range2end>(SID_to_byte(sid));
}


UdsAppManager::UdsAppManager(uint8_t* membuff, datasize_t capacity, const SessionInfo& sessinstance) :
  pubBuff(membuff),
  PubBuffCapacity(capacity),
  sessionState(sessinstance) {

  assert(pubBuff != nullptr);
  assert(PubBuffCapacity != 0);

  SetActiveMode(true);
}


void UdsAppManager::SendResponse(const uint8_t* data, uint32_t length) {

  if (length == 0) {
    return;
  }

  // check address and NRC code
  if (ResponseAllowed()) {
    SendRequest(data, length, false);
  }

  // reset session layer anyway
  SetSessionMode(sessionState.currSession == 1u);
}


void UdsAppManager::SendNegResponse(SIDs sid, NRCs nrc) {

  pubBuff[0] = SID_to_byte(SIDs::NR_SI);
  pubBuff[1] = SID_to_byte(sid);
  pubBuff[2] = NRC_to_byte(nrc);
  nrcBadParam = (nrc == NRCs::IMLOIF);
  nrcOutCode = nrc;

  SendResponse(pubBuff, 3);
}


void UdsAppManager::SendNegResponse(NRCs nrc) {

  SendNegResponse(reqContext.head.SI, nrc);
}


void UdsAppManager::SetServiceSession(uint8_t sessionValue) {

  // reset session layer anyway
  SetSessionMode(sessionValue == 1u);

  // all the clients must be informed
  assert(clientHandler != nullptr);
  clientHandler->OnSessionChange(sessionValue == 1u);
}


void UdsAppManager::OnSessIndication(const uint8_t* data, uint32_t length, TargetAddressType addr) {

  nrcOutCode = NRCs::PR;
  nrcBadParam = false;

  reqContext.addr = addr;
  reqContext.data = data;
  reqContext.size = length;
  reqContext.head.SI = SID_from_byte(reqContext.data[0]);

  if (!isManagerActive || !is_sid_processable(reqContext.head.SI)) {
    return;
  }

  reqContext.head.SF = SID_get_subfunc(reqContext.data[1]);
  reqContext.head.respSI = SID_response(reqContext.head.SI);
  // by default request is handled as with SubFunction
  reqContext.head.suppressPosResponse = SID_is_pos_response_suppress(reqContext.data[1]);
  // set most frequent case
  pubBuff[0] = reqContext.head.respSI;
  pubBuff[1] = reqContext.data[1];
  pubRespLength = 0;

  // Handle base service functions
  if (HandleReqInternally()) {
    SendResponse(pubBuff, pubRespLength);
    return;
  }

  assert(clientHandler != nullptr);
  ProcessResult clientHandRes = clientHandler->OnAppIndication(reqContext);

  if (clientHandRes == ProcessResult::NOT_HANDLED) {
    // there was no service to answer. so if the address is physycal
    // NRC NRCs::SNS must be sent (ISO 14229-1 table 4 (i))
    if (reqContext.addr == TargetAddressType::PHYS) {
      SendNegResponse(NRCs::SNS);
    }
  } else if (clientHandRes == ProcessResult::HANDLED_RESP_OK) {
    SendResponse(pubBuff, pubRespLength);
  } else if (clientHandRes == ProcessResult::HANDLED_RESP_NO) {
    // reset session layer anyway
    SetSessionMode(sessionState.currSession == 1u);
  }
}


void UdsAppManager::OnSessConfirmation(S_Result event) {

  assert(clientHandler != nullptr);
  clientHandler->OnAppConfirmation(event);
}


void UdsAppManager::On_s3_Timeout() {

  assert(clientHandler != nullptr);
  clientHandler->OnSessionChange(true);
}


bool UdsAppManager::ResponseAllowed() {

  if ((reqContext.addr == TargetAddressType::FUNC)
      && ((nrcOutCode == NRCs::SNSIAS)
          || (nrcOutCode == NRCs::SNS)
          || (nrcOutCode == NRCs::SFNS)
          || (nrcOutCode == NRCs::SFNSIAS)
          || (nrcOutCode == NRCs::ROOR))) {
    return false;
  } else if (nrcOutCode == NRCs::PR && reqContext.head.suppressPosResponse == true) {
    return false;
  } else if (nrcOutCode == NRCs::RCRRP) {
    reqContext.head.suppressPosResponse = false;
  }

  return true;
}


void UdsAppManager::StartPending(const size_t maxduration, const size_t resendperiod) {

  SetPending(maxduration, resendperiod, reqContext.head.SI);
}


void UdsAppManager::SetClient(UdsAppClient* client) {

  assert(clientHandler == nullptr);
  assert(client != nullptr);
  clientHandler = client;
}


void UdsAppManager::SetActiveMode(bool isactive) {

  isManagerActive = isactive;
}


bool UdsAppManager::HandleReqInternally() {

  if (reqContext.head.SI == SIDs::TP) {
    SID_TesterPresent();
  } else if (CheckIfSidSupported()) {
    return false;
  }

  return true;
}


void UdsAppManager::SID_TesterPresent() {

  if (reqContext.head.SF != 0) {
    // invalid SF
    SendNegResponse(NRCs::SFNS);
  } else if (reqContext.size != 2) {
    // TesterPresent : total length check
    SendNegResponse(NRCs::IMLOIF);
  } else {
    pubRespLength = 2;
  }
}


bool UdsAppManager::CheckIfSidSupported() {

  bool ret = false;
  bool subfunc = false;
  size_t minlength = 0u;

  assert(clientHandler != nullptr);
  ret = clientHandler->IsServiceSupported(reqContext.head.SI, minlength, subfunc);

  if (ret) {
    if (reqContext.size < minlength) {
      SendNegResponse(NRCs::IMLOIF);
      ret = false;
    } else {
      if (!subfunc) {
        // no subfunc means - no suppressPosResponse at all
        reqContext.head.suppressPosResponse = false;
      }
    }
  } else {
    SendNegResponse(NRCs::SNS);
  }

  return ret;
}

