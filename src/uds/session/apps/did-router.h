#pragma once

#include <stdint.h>

#include "../uds-server-base.h"
#include "../uds-service-handler.h"

class DidHandler;

class DidRouter : public UdsServiceHandler {
 public:
  DidRouter(UdsServerBase& r, DidHandler& didhandler) : UdsServiceHandler(r), dider(didhandler) {}

  virtual ProcessResult OnIndication(const IndicationInfo& inf) override;
  virtual ProcessResult OnConfirmation(S_Result res) override;

 protected:
  const uint8_t* data_;
  uint32_t len_;

 private:
  void ReadDataByIdentifierHandler();
  void WriteDataByIdentifierHandler();
  DidHandler& dider;
};
