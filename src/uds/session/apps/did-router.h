#pragma once

#include <stdint.h>

#include "../uds-app-manager.h"
#include "../uds-app-client.h"

class DidHandler;

class DidRouter : public UdsAppClient {

 public:
  DidRouter(UdsAppManager& r, DidHandler& didhandler) : UdsAppClient(r), dider(didhandler) {}
  virtual bool IsServiceSupported(sid_t sid, size_t& minlength, bool& subfunc) override;
  virtual ProcessResult OnAppIndication(const IndicationInfo& inf) override;
  virtual void OnAppConfirmation(S_Result res) override;

 protected:
  const uint8_t* data_;
  uint32_t len_;

 private:
  void ReadDataByIdentifierHandler();
  void WriteDataByIdentifierHandler();
  DidHandler& dider;
};
