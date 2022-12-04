#pragma once

#include <stdint.h>
#include <timers/d-timer.h>
#include <helpers/IProcessable.h>
#include <uds/inc/iso-tp-if.h>
#include <uds/inc/diag/uds-session.h>

/// @brief UDS session layer abstraction
/// Abstract class, must be expand but actual implementation
class SessionControl : public IsoTpClient, public IProcessable {
 public:
  /// @brief sets new TP sender
  /// @param sender pointer to TP sender instance
  void SetIsoTp(IsoTpImpl* sender) {
    if (sender != nullptr) {
      host = sender;
    }
  }

  /// @brief General processing handler
  virtual void Process();

  /// @brief ISO-TP event callback
  /// @param event iso-tp event type
  /// @param res iso-tp processing result
  virtual void OnIsoEvent(N_Event event, N_Result res, const IsoTpInfo& info) override;

 protected:
  /// @brief Payload sender
  /// @param data data pointer
  /// @param size data size
  /// @param enhanced enhanced session requested (p2 enhanced)
  void SendRequest(const uint8_t* data, uint32_t size, bool enhanced = false);

  /// @brief Sets session enhanced timing mode. During this mode NRC 0x78
  /// will be periodically sent with interval (ms) during duration (ms)
  /// While ETM is active all Data OK_r events will be ignored
  /// @param duration duration of etm
  /// @param interval interval to send keep-alive
  /// @param si service ID
  void SetPending(uint32_t duration, uint32_t interval, uint8_t si);

  /// @brief Indication callback for implementaion
  /// @param data received data pointer
  /// @param size received data size
  /// @param addr target address
  virtual void NotifyInd(const uint8_t* data, uint32_t size, TargetAddressType addr) = 0;

  /// @brief Confirmation callback for implementation
  /// @param res result
  virtual void NotifyConf(S_Result res) = 0;

  /// @brief S3 timeout callback for implemention
  virtual void On_s3_Timeout() = 0;

  /// @brief Updates session type
  /// @param is_default true if default session is requested
  void SetSessionMode(bool is_default);

  /// @brief Session state processing
  void ProcessSessionMode();

  /// @brief Session parameters setter
  /// @param par parameter type
  /// @param val parameter value
  SessParamResult SetSessionParam(SessParamType par, uint32_t val);

  /// @brief Timeout values descriptor
  typedef struct
  {
    uint32_t S3_max{5000};
    uint32_t p2_max{250};
    uint32_t p2_enhanced{5000};
  } SessionTimings_t;

  /// @brief Timeouts
  SessionTimings_t tims;

 private:
  /// @brief Updates session state
  /// @param state session type to be set
  void SetSessionMode(SessionType state) {
    SetSessionMode((state == SessionType::DEFAULT) ? true : false);
  }

  /// @brief Stub class to avoid host nullptr value
  class EmptyTpSender : public IsoTpImpl {
   public:
    virtual IsoTpResult Request(const uint8_t* data, size_t length) override {
      return IsoTpResult::BUSY;
    }
  };

  /// @brief Stub tp sender. does nothing
  EmptyTpSender plushtp;

  /// @brief Pointer to tp implementation
  IsoTpImpl* host{&plushtp};

  DTimers::Timer p2;
  DTimers::Timer S3;

  /// @brief Current session state
  SessionType ss_state{SessionType::DEFAULT};
  /// @brief Target address type of the current req/resp session
  TargetAddressType ta_addr{TargetAddressType::UNKNOWN};

  /// @brief Enhanced timing mode active status
  bool etm_active{false};

  /// @brief ETM pending message
  uint8_t etm_buff[3] = { 0x7fu, 0u, 0x78u };

  /// @brief ETM left duration (ms)
  uint32_t left_duration{0u};

  /// @brief ETM pending message sending interval (ms)
  uint32_t etm_interval;

  /// @brief ETM interval timer
  DTimers::Timer etm_timer{0u};
};
