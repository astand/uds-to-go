#pragma once

#include <uds/session/uds-app-client.h>

class DSCClient : public UdsAppClient {

 public:
  DSCClient(UdsAppManager& router_, SessionInfo& sessInfo) :
    UdsAppClient(router_), sessionInfoContext(sessInfo) {

    sessionInfoContext.currSession = 1;
    sessionInfoContext.secLevel = 0u;
  }

  virtual bool IsServiceSupported(sid_t sid, size_t& minlength, bool& subfunc) override;

  virtual ProcessResult OnAppIndication(const IndicationInfo& inf) override;

  virtual void OnAppConfirmation(S_Result) override {}

  virtual void OnSessionChange(bool isdefault) override;

 private:

  ProcessResult Handle_SA_request(const IndicationInfo& rcont);
  ProcessResult Handle_SA_response(const IndicationInfo& rcont);

  SessionInfo& sessionInfoContext;
  uint16_t seedsent{0};
};
