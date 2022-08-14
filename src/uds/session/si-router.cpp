#include "si-router.h"
#include "si-client.h"

#define IS_NRC_PASSED(x) ((x == NRC_SNS) ||\
  (x == NRC_SNSIAS) ||\
  (x == NRC_SFNS) ||\
  (x == NRC_SFNSIAS) ||\
  (x == NRC_ROOR))

#define SI_Flags_DSC    SID_Support | SID________ | SID________ | SID________ | SID_MinLen(2)
#define SI_Flags_TP     SID_Support | SID________ | SID________ | SID________ | SID_MinLen(2)

SiRouter::SiRouter()
{
  SID_Flag[PUDS_SI_DiagnosticSessionControl] = SI_Flags_DSC;
  SID_Flag[PUDS_SI_TesterPresent] = SI_Flags_TP;

  // clear all the clients
  for (int32_t i = 0; i < SI_CLIENT_MAX; i++)
  {
    cls[i] = nullptr;
  }

  now_clients_cnt = 0;

  sess_info.sec_level = 0;
  sess_info.sess = DSC_SF_DS;

  router_is_disabled = false;
}


void SiRouter::SendResponse(const uint8_t* data, int32_t len)
{
  if (len <= 0)
  {
    // the service handled the request but the response will be
    // returned in the next loop cycle.
    return;
  }

  // check address and NRC code
  if (len > 0 && ResponseAllowed())
  {
    SendRequest(data, len);
  }
  else
  {
    // the response is not one for response, the session timer must be
    // reset here (ISO14229-2 Table 6)
    SessionChangeEvent(sess_info.sess);
  }
}

void SiRouter::SendNegResponse(uint8_t sid, NRCs_t nrc)
{
  router_tx_buff[0] = PUDS_NR_SI;
  router_tx_buff[1] = sid;
  router_tx_buff[2] = static_cast<uint8_t>(nrc);
  nrc_bad_param = (nrc == NRC_IMLOIF);
  nrc_code = nrc;

  SendResponse(router_tx_buff, 3);
}

void SiRouter::SendNegResponse(NRCs_t nrc)
{
  SendNegResponse(sihead.SI, nrc);
}


void SiRouter::SetServiceSession(uint8_t s)
{
  // make all the control inside the session layer
  SessionChangeEvent(s);

  // all the clients must be informed
  NotifyDSCSessionChanged(false);
}


void SiRouter::SetSecurityLevel(uint8_t sa_level)
{
  sess_info.sec_level = sa_level;
}


void SiRouter::SessionChangeEvent(uint8_t s)
{
  SetSessionMode(s == DSC_SF_DS);

  if (sess_info.sess != s)
  {
    // current session will be changed. notify every client about it
    sess_info.sess = s;
  }
}

void SiRouter::NotifyInd(const uint8_t* data, uint32_t length, UdsAddress addr)
{
  nrc_code = NRC_PR;
  nrc_bad_param = false;

  req_addr = addr;

  data_info.data = data;
  data_info.size = length;

  sihead.SI = data_info.data[0];
  sihead.SF = (data_info.data[1] & 0x7FU);
  sihead.respSI = RESPONSE_ON_SID(sihead.SI);
  // services without subfunctions must set NoResponse bit to 0 by themself!!!
  sihead.NoResponse = data_info.data[1] & 0x80U ? 1 : 0;
  // set most frequent case
  tData[0] = sihead.respSI;
  tData[1] = data_info.data[1];
  tLength = 0;

  if (router_is_disabled)
  {
    return;
  }

  // Handle base service functions
  if (SelfIndHandler())
  {
    SendResponse(tData, tLength);
    return;
  }

  // event is passing to client
  for (int32_t i = 0; i < now_clients_cnt && i < SI_CLIENT_MAX; i++)
  {
    clientHandRes = cls[i]->OnIndication(data_info);

    if (clientHandRes != kSI_NotHandled)
    {
      // One of the clients accepted request
      break;
    }
  }

  if (clientHandRes == kSI_NotHandled)
  {
    // there was no service to answer. so if the address is physycal
    // NRC NRC_SNS must be sent (ISO 14229-1 table 4 (i))
    if (req_addr == UdsAddress::PHYS)
    {
      SendNegResponse(NRC_SNS);
    }
  }
  else if (clientHandRes == kSI_HandledResponseOk)
  {
    SendResponse(tData, tLength);
  }
  else if (clientHandRes == kSI_HandledNoResponse)
  {
    // the service handled the request but decided that
    // there is no necessity to send response, the current
    // session must be re-init in this case
    SessionChangeEvent(sess_info.sess);
  }
}

void SiRouter::NotifyConf(S_Result res)
{
  if (SelfConfHandler())
  {
    // router handler handled event
    return;
  }

  data_info.data = nullptr;
  data_info.size = 0;
  data_info.addr = UdsAddress::UNKNOWN;
  data_info.head = sihead;

  // one of the client must handle request
  for (int32_t i = 0; i < now_clients_cnt && i < SI_CLIENT_MAX; i++)
  {
    clientHandRes = cls[i]->OnConfirmation(res);
    {
      if (clientHandRes != kSI_NotHandled)
      {
        // one of the clients accepted the request
        break;
      }
    }
  }
}


uint8_t SiRouter::GetNRC()
{
  return nrc_code;
}


void SiRouter::NotifyS3Timeout()
{
  // the session layer says that S3 timer is out
  // the current SSL session must be kSSL_Default

  // make self proccessing and notify all the clients
  if (sess_info.sess != DSC_SF_DS)
  {
    // appPci.dsc_state.prev = sess_info.sess;
    sess_info.sess = DSC_SF_DS;
  }

  NotifyDSCSessionChanged(true);
}


void SiRouter::NotifyDSCSessionChanged(bool s3timer)
{
  // event is passing to client
  for (int32_t i = 0; i < now_clients_cnt && i < SI_CLIENT_MAX; i++)
  {
    cls[i]->DSCSessionEvent(s3timer);
  }
}


bool SiRouter::ResponseAllowed()
{
  if (req_addr == UdsAddress::FUNC)
  {
    // ISO 14229-1 Table 5
    if (sihead.NoResponse == false)
    {
      // Only POS answers or reading error related NRC is allowed
      if ((nrc_code == NRC_PR) || (nrc_bad_param))
      {
        return true;
      }
    }
    else
    {
      // Only reading error related NRC is allowed (g)
      if (nrc_bad_param)
      {
        return true;
      }
    }

    // answer doesn't allowed
    return false;
  }
  else if (req_addr == UdsAddress::PHYS)
  {
    if (sihead.NoResponse == true)
    {
      /* Table 4 (h, i, j) */
      /* Table 4 (g) - client sets this state */
      if (IS_NRC_PASSED(nrc_code) ||  nrc_bad_param)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
  }

  return true;
}



void SiRouter::RegisterClient(SiClient* client)
{
  if (now_clients_cnt < SI_CLIENT_MAX)
  {
    cls[now_clients_cnt] = client;
    now_clients_cnt++;
  }
  else
  {
    // deadloop here because it is a static allocation (will happen in the start code)
    for (;;);
  }
}

void SiRouter::RouterDisable()
{
  router_is_disabled = true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool SiRouter::SelfIndHandler()
{
  bool ret = true;

  // the function will return true only when it detected SNS, SNSIAS state, and
  // it will set NoResponse bit based on the values in the SID_Flag table
  if (MakeBaseSIDChecks())
  {
    return ret;
  }

  if (sihead.SI == PUDS_SI_TesterPresent)
  {
    SID_TesterPresent();
  }
  else if (sihead.SI == PUDS_SI_DiagnosticSessionControl)
  {
    SID_DiagServiceControl();
  }
  else
  {
    ret = false;
  }

  return ret;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool SiRouter::SelfConfHandler()
{
  return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SiRouter::SID_TesterPresent()
{
  if (sihead.SF != 0)
  {
    // invalid SF
    SendNegResponse(NRC_SFNS);
  }
  else if (data_info.size != 2)
  {
    // TesterPresent : total length check
    SendNegResponse(NRC_IMLOIF);
  }
  else
  {
    tLength = 2;
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SiRouter::SID_DiagServiceControl()
{
  // minimal length check already done
  if (sihead.SF == 0 || sihead.SF > 3)
  {
    SendNegResponse(NRC_SFNS);
  }
  else  if (data_info.size != 2)
  {
    // DSC : total length check
    SendNegResponse(NRC_IMLOIF);
  }
  else
  {
    tData[2] = ((tims.p2_max >> 8) & 0xFF);
    tData[3] = (tims.p2_max & 0xFF);
    tData[4] = (((tims.p2_enhanced / 10) >> 8) & 0xFF);
    tData[5] = ((tims.p2_enhanced / 10) & 0xFF);
    tLength = 6;

    // everything is ok. restart session
    if (sihead.SF == DSC_SF_PRGS)
    {
      SetServiceSession(DSC_SF_PRGS);
    }
    else if (sihead.SF == DSC_SF_EXTDS)
    {
      // reset programming session here but NOT extended
      SetServiceSession(DSC_SF_EXTDS);
    }
    else if (sihead.SF == DSC_SF_DS)
    {
      // TODO: Reset (if possible) or what?
      SetServiceSession(DSC_SF_DS);
    }
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool SiRouter::MakeBaseSIDChecks()
{
  //service is available
  bool ret = true;
  uint8_t flags = SID_Flag[sihead.SI];

  // negResponse handling
  if (flags == 0)
  {
    // service not supported
    SendNegResponse(NRC_SNS);
  }
  else if (((flags & SID_Phyaddr) != 0) && (req_addr != UdsAddress::PHYS))
  {
    // do nothing here,the true will be returned and no further actions will be made
  }
  else if (((flags & SID_NoInDef) != 0) && (sess_info.sess == DSC_SF_DS))
  {
    SendNegResponse(NRC_SNSIAS);
  }
  else if ((flags & SID_HasMinL) && (data_info.size  < (flags & 0x0FU)))
  {
    SendNegResponse(NRC_IMLOIF);
  }
  else
  {
    ret = false;
  }

  // noResponse bit handling
  if (flags & SID_NoSubFu)
  {
    sihead.NoResponse = 0;
  }

  return ret;
}

