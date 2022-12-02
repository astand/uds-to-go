#pragma once

#include <stdint.h>
#include <timers/d-timer.h>
#include <helpers/IProcessable.h>
#include <uds/inc/iso-tp-if.h>
#include <uds/inc/diag/uds-session.h>

class SessionControl : public IsoTpClient, public IProcessable {
 public:
  void SetIsoTp(IsoTpImpl* sender) {
    host = sender;
  }

 public:
  // Session state processing API
  virtual void Process();
  // IsoTpClient API implementation
  virtual void OnIsoEvent(N_Type t, N_Result res, const IsoTpInfo& info) override;

 protected:
  void SendRequest(const uint8_t* data, uint32_t len);

  // For heritance classes to react on session events
  virtual void NotifyInd(const uint8_t* data, uint32_t length, TargetAddressType addr) = 0;
  virtual void NotifyConf(S_Result res) = 0;
  // one of the upper inheritor must consume this event
  virtual void On_s3_Timeout() = 0;
  virtual uint8_t GetNRC() = 0;
  void SetSessionMode(bool is_default);
  void ProcessSessionMode();
  SessResult SetSessionParam(SessParam par, uint32_t v);

  typedef struct
  {
    uint32_t S3_max{5000};
    uint32_t p2_max{250};
    uint32_t p2_enhanced{5000};
  } SessionTimings_t;

  SessionTimings_t tims;

 private:
  void SetSessionMode(SessionType state) {
    SetSessionMode((state == SessionType::DEFAULT) ? true : false);
  }

  IsoTpImpl* host;

  DTimers::Timer p2;
  DTimers::Timer S3;

  /// @brief current session state
  SessionType ss_state{SessionType::DEFAULT};
  /// @brief target address type
  TargetAddressType ta_addr{TargetAddressType::UNKNOWN};
};
