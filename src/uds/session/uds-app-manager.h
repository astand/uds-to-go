#pragma once

#include "session-control.h"
#include <helpers/IKeeper.h>
#include <uds/inc/diag/diag.h>

/// @brief Forward declarion for uds request handler
class UdsAppClient;

class UdsAppManager : public SessionControl {

 public:
  /// @brief Constructor
  /// @param membuff Pointer to memory for using as response buffer
  /// @param capacity Transmit buffer capacity
  /// @param sessinstance Reference to the session information instance
  UdsAppManager(uint8_t* membuff, datasize_t capacity, const SessionInfo& sessinstance);

  /// @brief Send application response
  /// @param data Pointer to payload
  /// @param length Payload length
  void SendResponse(const uint8_t* data, uint32_t length);

  /// @brief Send negative response for the last requested service
  /// @param nrc Negative repsponse code
  void SendNegResponse(NRCs nrc);

  /// @brief Send negative response for specific service
  /// @param sid Service identificator
  /// @param nrc Negative response code
  void SendNegResponse(const sid_t sid, const NRCs nrc);

  /// @brief Send negative response code RCRRP
  /// @param maxduration Global enhanced mode timeout
  /// @param resendperiod Enhanced mode keeping alive message send period
  void StartPending(const size_t maxduration, const size_t resendperiod = 2000u);

  /// @brief Set client. Can be called once
  /// @param client Pointer to the uds application client
  void SetClient(UdsAppClient* client);

  /// @brief Set app manager mode
  /// @param isactive when true manager is enabled, otherwise is disabled
  void SetActiveMode(bool isactive);


  /// @brief Notify app manager about session change
  /// @param sessionValue New (old) session value
  void SetServiceSession(uint8_t sessionValue);

  /// @brief Session state getter
  /// @return Reference on app manager session state
  const SessionInfo& GetSession() const {
    return sessionState;
  }

  /// @brief Response buffer for loading response by client
  uint8_t* const pubBuff;
  /// @brief Response buffer capacity
  const datasize_t PubBuffCapacity;
  /// @brief Response payload length
  datasize_t pubRespLength;

 private:
  /// @brief Request context information
  IndicationInfo reqContext{};
  /// @brief Reference to session state instance
  const SessionInfo& sessionState;

  /// @brief Session layer indication callback
  /// @param data Pointer to indication data
  /// @param length Length of indication data
  /// @param addr Request address
  virtual void OnSessIndication(const uint8_t* data, uint32_t length, TargetAddressType addr) override;

  /// @brief Session layer confirmation callback
  /// @param res Confirmation result
  virtual void OnSessConfirmation(S_Result res) override;

  /// @brief Session layer s3 timer timeout callback
  virtual void On_s3_Timeout();

  /// @brief Internal request handler
  /// @return true if request has been handled internally, otherwise false
  bool HandleReqInternally();

 private:

  /// @brief Checks if the response attributes allow to send response on CAN
  /// @return false when response must not be sent, otherwise true
  bool ResponseAllowed();

  /// @brief Checks if requested SID is supported by client
  /// @return true when client is able to handle service request, otherwise false
  bool CheckIfSidSupported();

  /// @brief General tester present request handler
  void SID_TesterPresent();

  /// @brief Pointer to uds app client
  UdsAppClient* clientHandler;
  /// @brief Negative output code to be sent as response
  NRCs nrcOutCode;
  /// @brief Pending mode activation indicator
  bool nrcPending;
  /// @brief Manager activity flag, manager blocks requests when false
  bool isManagerActive;

};

