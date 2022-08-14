// This driver is the first APPLICATION layer handler
// It performs basic service handling :
// TesterPresent
// Diagnostic Session Control
// It is the top level of the UDS DataLink stack (layer 7)
#pragma once

#include "session-control.h"
#include <uds/inc/diag/diag.h>

#define SID________       (0)
#define SID_Phyaddr       (1u << 3u)
#define SID_Support       (1u << 4u)
#define SID_NoInDef       (1u << 5u)
#define SID_NoSubFu       (1u << 6u)
#define SID_HasMinL       (1u << 7u)

#define SID_MinLen(x)     (SID_HasMinL | ((uint8_t)(x) > 7u) ? (7u) : ((uint8_t)(x) & 0x07U))

class SiClient;

class SiRouter : public SessionControl {
 public:
  SiRouter();
  // this function exports for clients
  void SendResponse(const uint8_t* data, int32_t len);

  /**
   * @brief Sending negative response on the request in current UDS context
   *
   * @param nrc Negative response code
   */
  void SendNegResponse(NRCs_t nrc);

  /**
   * @brief Sending independent negative response
   *
   * @param sid Servide ID
   * @param nrc Negative response code
   */
  void SendNegResponse(uint8_t sid, NRCs_t nrc);

  // this function registers new si-client and it will
  // !!! be blocked if there is no slot for client in stack
  void RegisterClient(SiClient* client);
  // disable any service processing (to be used before reset or power-off)
  void RouterDisable();
  // Changed the session mode by user
  void SetServiceSession(uint8_t s);
  // Set current security access level by user
  void SetSecurityLevel(uint8_t level);

 public:

  // this readonly descriptor for clients to be informed about UDS curr state
  static constexpr size_t TX_DATA_MAX_LEN = 4096;
  uint8_t tData[TX_DATA_MAX_LEN];
  int32_t tLength;

  SI_Head_t sihead;

  const SessionInfo& GetSession() const {
    return sess_info;
  }

 protected:
  IndicationInfo data_info{0};
  SessionInfo sess_info{};

  virtual void NotifyInd(const uint8_t* data, uint32_t length, UdsAddress addr) override;
  virtual void NotifyConf(S_Result res) override;

  // This method is used by session layer to get information about negative response at the end of transmitting
  virtual uint8_t GetNRC() override;
  // Handlers for base common services,
  // it not necessary to implement them in inheritors
  virtual bool SelfIndHandler();
  virtual bool SelfConfHandler();

  // This method must be called when any session related
  // activity is invoked
  void SessionChangeEvent(uint8_t sessnum);

  // this method perform calling the notify event func of each registered client
  void NotifyDSCSessionChanged(bool s3timer);

  uint8_t SID_Flag[0xff] = { 0 };

 private:
  // Session layer calls this method when S3 timer is out
  // and session goes to the kSSL_Default
  virtual void NotifyS3Timeout();
  // Handlers for UDS lower datalink layer events
  void IndRouterEvent();
  void ConfRouterEvent();
  // Handlers for basic services handling
  // checking table 4,5 (ISO-14229-1) conditions
  bool ResponseAllowed();

 private:
  bool router_is_disabled;
  static constexpr size_t SI_CLIENT_MAX = 8;
  SiClient* cls[SI_CLIENT_MAX];
  uint16_t now_clients_cnt;
  ProcessResult_t clientHandRes;
  uint8_t router_tx_buff[8];
  NRCs_t nrc_code;
  UdsAddress req_addr{UdsAddress::UNKNOWN};

  /**
   * @brief this value indicates response module if
   * the response is related to cases in ISO 14229-1 table 4 (b, h)
   */
  bool nrc_bad_param;


 private:
  bool MakeBaseSIDChecks();
  void SID_TesterPresent();
  void SID_DiagServiceControl();
};

