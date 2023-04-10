#pragma once

#include <uds/session/uds-app-client.h>

class ProxyUdsAppClient : public UdsAppClient {

 public:
  ProxyUdsAppClient(UdsAppManager& router, UdsAppClient& handler) :
    UdsAppClient(router), realHandler(handler) {}

  virtual bool IsServiceSupported(sid_t sid, size_t& minlength, bool& subfunc) {

    return realHandler.IsServiceSupported(sid, minlength, subfunc);
  }

  virtual ProcessResult OnAppIndication(const IndicationInfo& inf) override {

    std::cout << "SC : On Ind  -> " << "Addr: " << ((inf.addr == TargetAddressType::PHYS) ? ("PHYS ") : ("FUNC "));
    std::cout << " -> SI [" << (int) inf.head.SI << "] SF [" << (int) inf.head.SF << "]";
    std::cout << "Data size = " << inf.size << " b." << std::endl;
    std::cout << std::endl;

    return realHandler.OnAppIndication(inf);
  }


  virtual void OnAppConfirmation(S_Result res) override {

    std::cout << "SC : On Conf -> [" << ((res == S_Result::OK) ? (" OK ") : (" ! NOK ")) << "]";
    std::cout << std::endl;

    return realHandler.OnAppConfirmation(res);
  }


  virtual void OnSessionChange(bool s3timer_event) override {

    std::cout << ((s3timer_event) ? (" s3 ") : (" -- "));
    realHandler.OnSessionChange(s3timer_event);
    std::cout << "Session update :" << (int) udsRouter.GetSession().currSession;
    std::cout << std::endl;
  }

 private:
  UdsAppClient& realHandler;

};
