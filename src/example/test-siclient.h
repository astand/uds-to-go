#pragma once

#include <uds/session/uds-service-handler.h>

class TestUdsServiceHandler : public UdsServiceHandler {

 public:
  TestUdsServiceHandler(UdsServerBase& router) : UdsServiceHandler(router) {}

  virtual ProcessResult OnIndication(const IndicationInfo& inf) override {
    std::cout << "SC : " << "On Ind -> " << "Addr: " << ((inf.addr == UdsAddress::PHYS) ? ("PHYS ") : ("FUNC "));
    std::cout << " -> SI [" << (int)inf.head.SI << "] SF [" << (int)inf.head.SF << "]";
    std::cout << "Data size = " << inf.size << " b." << std::endl;
    std::cout << std::endl;

    return ProcessResult::NOT_HANDLED;
  }


  virtual ProcessResult OnConfirmation(S_Result res) override {
    std::cout << "SC : On Confirmation [" << ((res == S_Result::OK) ? (" OK ") : (" ! NOK ")) << "]";
    std::cout << std::endl;

    return ProcessResult::NOT_HANDLED;
  }


  virtual void DSCSessionEvent(bool s3timer_event) override {
    std::cout << "DSC session change event. S3:" << ((s3timer_event) ? ("YES") : ("NO"));
    std::cout << ". Session= " << (int)rtr1.GetSession().sess;
    std::cout << std::endl;
  }

};
