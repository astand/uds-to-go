#pragma once

#include <uds/session/uds-service-handler.h>

class DSCClient : public UdsServiceHandler {
 public:
  DSCClient(UdsServerBase& router_) : UdsServiceHandler(router_) {}

  virtual ProcessResult_t OnIndication(const IndicationInfo& inf);

  virtual ProcessResult_t OnConfirmation(S_Result res) {
    return kSI_NotHandled;
  }
};
