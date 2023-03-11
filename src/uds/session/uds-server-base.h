// This driver is the first APPLICATION layer handler
// It performs basic service handling :
// TesterPresent
// Diagnostic Session Control
// It is the top level of the UDS DataLink stack (layer 7)
#pragma once

#include "session-control.h"
#include <helpers/IKeeper.h>
#include <uds/inc/diag/diag.h>

class UdsServiceHandler;

class UdsServerBase : public SessionControl {
 public:
  UdsServerBase(IKeeper<UdsServiceHandler>& vec, uint8_t* txptr, datasize_t txsize);
  // this function exports for clients
  void SendResponse(const uint8_t* data, uint32_t len, bool enhanced = false);
  void SendNegResponse(NRCs nrc);
  void SendNegResponse(SIDs sid, NRCs nrc);

  void RegisterClient(UdsServiceHandler* client);
  void RouterDisable();
  void SetServiceSession(uint8_t s);
  void SetSecurityLevel(uint8_t level);

  using flag_t = uint8_t;
 public:

  const datasize_t TX_SIZE;
  uint8_t* const pubBuff;
  datasize_t pubSize;

  const SessionInfo& GetSession() const {
    return sess_info;
  }

 protected:
  IndicationInfo data_info{};
  SessionInfo sess_info;

  /// @brief Sets UDS service for handling by uds server base router
  /// @param sid service ID
  /// @param noindef is service not supported in defualt session
  /// @param nosubf service doesn't support subfunctions
  /// @param onlyphys service supports only physical addressing
  /// @param minlen service minimal payload length. 0 for
  /// @return true if service has been added
  bool EnableSID(SIDs sid, bool noindef, bool nosubf, bool onlyphys, uint8_t minlen = 0);

  virtual void NotifyInd(const uint8_t* data, uint32_t length, TargetAddressType addr) override;
  virtual void NotifyConf(S_Result res) override;

  // Handlers for base common services,
  // it not necessary to implement them in inheritors
  virtual bool SelfIndHandler();
  virtual bool SelfConfHandler();

  // This method must be called when any session related
  // activity is invoked
  void SessionChangeEvent(uint8_t sessnum);

  // this method perform calling the notify event func of each registered client
  void NotifyDSCSessionChanged(bool s3timer);


 private:
  // Session layer calls this method when S3 timer is out
  // and session goes to the kSSL_Default
  virtual void On_s3_Timeout();
  // Handlers for basic services handling
  // checking table 4,5 (ISO-14229-1) conditions
  bool ResponseAllowed();

 private:
  IKeeper<UdsServiceHandler>& cls;
  bool router_is_disabled;
  ProcessResult clientHandRes;
  NRCs nrc_code;
  TargetAddressType req_addr{TargetAddressType::UNKNOWN};

  /**
   * @brief this value indicates response module if
   * the response is related to cases in ISO 14229-1 table 4 (b, h)
   */
  bool nrc_bad_param;

  uint8_t SID_Flag[0xff] = { 0 };

 private:
  bool MakeBaseSIDChecks();
  void SID_TesterPresent();
};

