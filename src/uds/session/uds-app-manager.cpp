#include "etc/helpers/ophelper.h"
#include "uds-app-manager.h"
#include "uds-app-client.h"

using namespace sidhelper;

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

  nrcPending = false;
  // reset session layer anyway
  SetSessionMode(is_dsc_def_session(sessionState.currSession));
}


void UdsAppManager::SendNegResponse(const sid_t sid, const NRCs nrc) {

  pubBuff[0] = NR_SI;
  pubBuff[1] = sid;
  pubBuff[2] = NRC_to_byte(nrc);
  nrcOutCode = nrc;

  SendResponse(pubBuff, 3);
}


void UdsAppManager::SendNegResponse(NRCs nrc) {

  SendNegResponse(reqContext.head.SI, nrc);
}


void UdsAppManager::SetServiceSession(uint8_t sessionValue) {

  // reset session layer anyway
  SetSessionMode(is_dsc_def_session(sessionValue));

  // all the clients must be informed
  assert(clientHandler != nullptr);
  clientHandler->OnSessionChange(is_dsc_def_session(sessionValue));
}


void UdsAppManager::OnSessIndication(const uint8_t* payload, uint32_t length, TargetAddressType addr) {

  nrcOutCode = NRCs::PR;

  assert(length != 0u);

  if (!isManagerActive || !is_sid_processable(payload[0])) {
    return;
  }

  reqContext.addr = addr;
  reqContext.data = payload;
  reqContext.size = length;

  reqContext.head.SI = reqContext.data[0];
  reqContext.head.SF = get_subfunc(reqContext.data[1]);
  // by default request is handled as with SubFunction
  reqContext.head.suppressPosResponse = is_pos_response_suppress(reqContext.data[1]);
  // set most frequent case
  pubBuff[0] = to_response(reqContext.head.SI);
  pubBuff[1] = reqContext.data[1];
  pubRespLength = 0;

  bool isrequest = !is_response(reqContext.data[0]);

  if (isrequest) {
    // Handle base service functions
    if (HandleReqInternally()) {
      SendResponse(pubBuff, pubRespLength);
      return;
    }
  }

  assert(clientHandler != nullptr);
  ProcessResult clientHandRes = clientHandler->OnAppIndication(reqContext);

  if (clientHandRes == ProcessResult::NOT_HANDLED && isrequest) {
    // there was no service to answer. so if the address is physycal
    // NRC NRCs::SNS must be sent (ISO 14229-1 table 4 (i))
    if (reqContext.addr == TargetAddressType::PHYS) {
      SendNegResponse(NRCs::SNS);
    }
  } else if (clientHandRes == ProcessResult::HANDLED_RESP_OK) {
    SendResponse(pubBuff, pubRespLength);
  } else if (clientHandRes == ProcessResult::HANDLED_RESP_NO) {
    // reset session layer anyway
    SetSessionMode(is_dsc_def_session(sessionState.currSession));
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
      && (nrcPending == false)
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

  nrcPending = true;
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

  if (reqContext.head.SI == TP) {
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

