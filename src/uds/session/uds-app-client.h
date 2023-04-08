#pragma once

#include <stdint.h>
#include "uds-app-manager.h"

/// @brief Uds app client interface class
class UdsAppClient {
 public:
  /// @brief Constructor
  /// @param udsapp Reference to application uds manager
  UdsAppClient(UdsAppManager& udsapp) : udsRouter(udsapp) {}

  /// @brief Checks if service can be handled by current instance
  /// @param sid Service Id to be handled
  /// @param minlenght Minimal payload for requested service
  /// @param subfunc Does the service have sub function
  /// @return true when instance can handle requested service, otherwise false
  virtual bool IsServiceSupported(const SIDs sid, size_t& minlenght, bool& subfunc) = 0;

  /// @brief Callback from application uds manager on session indication event
  /// @param inf Indication event descriptor
  /// @return Processing result
  virtual ProcessResult OnAppIndication(const IndicationInfo& inf) = 0;

  /// @brief Callback from application uds manager on session confirmation event
  /// @param res
  virtual void OnAppConfirmation(S_Result res) = 0;

  /// @brief Callback from application uds manager on session change event
  /// @param isdefault
  virtual void OnSessionChange(bool isdefault) {

    (void) isdefault;
  }

 protected:
  UdsAppManager& udsRouter;
};


/// @brief Proxy uds application handler class, aggregates more than 1 client
/// @tparam N Number of possible uds application clients
template<size_t N>
class MultiServiceManager : public MemKeeper<UdsAppClient, N>, public UdsAppClient {
 public:

  /// @brief Constructor
  /// @param base Reference on uds application manager
  MultiServiceManager(UdsAppManager& base) : UdsAppClient(base) {}

  /// @brief Checks if service can be handled by current instance
  /// @param sid Service Id to be handled
  /// @param minlenght Minimal payload for requested service
  /// @param subfunc Does the service have sub function
  /// @return true when instance can handle requested service, otherwise false
  virtual bool IsServiceSupported(const SIDs sid, size_t& minlenght, bool& subfunc) {

    bool ret = false;
    UdsAppClient* service = nullptr;
    size_t i = 0u;

    while (this->TryReadElem(i++, service)) {
      if (service->IsServiceSupported(sid, minlenght, subfunc)) {
        ret = true;
        break;
      }
    }

    return ret;
  }

  /// @brief Callback from application uds manager on session indication event
  /// @param inf Indication event descriptor
  /// @return Processing result
  virtual ProcessResult OnAppIndication(const IndicationInfo& inf) {

    ProcessResult procResult;
    UdsAppClient* service = nullptr;
    size_t i = 0u;

    while (this->TryReadElem(i++, service)) {
      procResult = service->OnAppIndication(inf);

      if (procResult != ProcessResult::NOT_HANDLED) {
        break;
      }
    }

    return procResult;
  }

  /// @brief Callback from application uds manager on session confirmation event
  /// @param res Confirmation result (status)
  virtual void OnAppConfirmation(S_Result res) {

    this->StartIteration();

    while (!this->IsLastIteration()) {
      this->IterNextElem()->OnAppConfirmation(res);
    }
  }

  /// @brief Callback from uds session layer on session change event
  /// @param isdefault true when current session is default
  virtual void OnSessionChange(bool isdefault) {

    this->StartIteration();

    while (!this->IsLastIteration()) {
      this->IterNextElem()->OnSessionChange(isdefault);
    }
  }
};

