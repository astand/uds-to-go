#pragma once

#include <stdint.h>
#include <timers/d-timer.h>
#include <uds/inc/iso-tp-if.h>
#include <uds/inc/uds-session.h>

class SessionControl : public IsoTpClient {
 public:
  SessionControl();

  void SetIsoTp(IsoTpImpl* sender) {
    host = sender;
  }

  typedef struct
  {
    SessionState session;
    UdsAddress atype;
  } S_PDU_t;

 public:
  // Session state processing API
  virtual void Process();
  // IsoTpClient API implementation
  virtual void OnIsoEvent(N_Type t, N_Result res, const IsoTpInfo& info) override;

  // For heritance classes to react on session events
  virtual void NotifyInd(const uint8_t* data, uint32_t length, UdsAddress addr) {
    (void)data;
    (void)length;
    (void)addr;
  }

  virtual void NotifyConf(S_Result res) {
    (void)res;
  }

  // one of the upper inheritor must consume this event
  virtual void NotifyS3Timeout() = 0;
  virtual uint8_t GetNRC() = 0;
  void SendRequest(const uint8_t* data, uint32_t len);

 protected:
  void SetSessionMode(bool is_default);
  void ProcessSessionMode();
  IsoTpImpl* host;

 private:
  void SetSessionMode(SessionState state) {
    SetSessionMode((state == SessionState::DEFAULT) ? true : false);
  }
  using Timer = DTimers::Timer;

  Timer p2;
  Timer S3;

 private:

  S_PDU_t sState;

  typedef struct
  {
    uint32_t S3_max;
    uint32_t p2_max;
    uint32_t p2_enhanced;
  } SessionTimings_t;

  SessionTimings_t tims;

};
