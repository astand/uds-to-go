#pragma once

#include <uds/session/si-client.h>

class TestSiClient : public SiClient {

 public:
  TestSiClient(SiRouter& router) : SiClient(router) {}

  virtual ProcessResult_t OnIndication(const IndicationInfo& inf) override {
    std::cout << "SC : " << "On Ind -> " << "Addr: " << ((inf.addr == UdsAddress::PHYS) ? ("PHYS ") : ("FUNC "));
    std::cout << "SI " << inf.head.SI << "] SF [" << inf.head.SF << "]";
    std::cout << "Data size = " << inf.size << " b." << std::endl;
    std::cout << std::endl;

    return ProcessResult_t::kSI_HandledResponseOk;
  }


  virtual ProcessResult_t OnConfirmation(S_Result res) override {
    std::cout << "SC : On Confirmation [" << ((res == S_Result::OK) ? (" OK ") : (" ! NOK ")) << "]";
    std::cout << std::endl;

    return ProcessResult_t::kSI_HandledResponseOk;
  }


  virtual void DSCSessionEvent(bool s3timer_event) override {
    std::cout << "DSC session change event. S3:" << ((s3timer_event) ? ("YES") : ("NO"));
    std::cout << ". Session= " << (int)rtr1.GetSession().sess;
    std::cout << std::endl;
  }

};
