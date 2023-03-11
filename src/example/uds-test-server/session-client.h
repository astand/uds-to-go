#pragma once

#include <uds/session/uds-service-handler.h>

class DSCClient : public UdsServiceHandler {
 public:
  DSCClient(UdsServerBase& router_) : UdsServiceHandler(router_) {}

  virtual ProcessResult OnIndication(const IndicationInfo& inf) override;

  virtual ProcessResult OnConfirmation(S_Result) override {
    return ProcessResult::NOT_HANDLED;
  }
};
