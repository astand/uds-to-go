// The SiClient is the base class for common UDS service client.
// It also provides SiRouter registration and base dummy Process()
// implementation
#pragma once

#include <stdint.h>
#include "si-router.h"

class SiClient : public IProcessable {
 public:
  SiClient(SiRouter& router_) : rtr1(router_) {
    rtr1.RegisterClient(this);
  }


  /**
    * @brief Method which notifies current client about new UDS pdu packet
    * @param inf PDU packet descriptor
    * @return Status of handling
    * */
  virtual ProcessResult_t OnIndication(const IndicationInfo& inf) = 0;


  /**
   * @brief Callback method by which client is notified about
   * successful response sending
   * NOTE: NOT IMPLEMENTED ON LOW LEVEL
   *
   * @return Status of confirmation handling
   * */
  virtual ProcessResult_t OnConfirmation(S_Result res) = 0;


  /**
   * @brief Callback by which session leyer notifies client
   * about session change
   * @param s3timer_event Indicates that session was changed by
   * S3 time elapsed
   *
   * @return void
   * */
  virtual void DSCSessionEvent(bool s3timer_event) {
    (void) s3timer_event;
    /* do nothing by default */
  }


  virtual void Process() {}

 protected:
  SiRouter& rtr1;
};

