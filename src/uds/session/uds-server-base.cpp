#include "uds-server-base.h"
#include "uds-service-handler.h"

#define IS_NRC_PASSED(x) ((x == NRC_SNS) ||\
  (x == NRC_SNSIAS) ||\
  (x == NRC_SFNS) ||\
  (x == NRC_SFNSIAS) ||\
  (x == NRC_ROOR))
/* ------------------- SID_Phyaddr   SID_Support   SID_NoInDef   SID_NoSubFu   SID_HasMinL */
#define SI_Flags_DSC   SID_Support | SID________ | SID________ | SID________ | SID_MinLen(2)
#define SI_Flags_TP    SID_Support | SID________ | SID________ | SID________ | SID_MinLen(2)

template<uint8_t low, uint8_t high>
bool out_of_range(uint8_t v)
{
  assert(low < high);
  return (v < low || v > high);
}

UdsServerBase::UdsServerBase(IKeeper<UdsServiceHandler>& vec) : cls(vec)
{
  SID_Flag[PUDS_SI_DiagnosticSessionControl] = SI_Flags_DSC;
  SID_Flag[PUDS_SI_TesterPresent] = SI_Flags_TP;

  sess_info.sec_level = 0;
  sess_info.sess = DSC_SF_DS;

  router_is_disabled = false;
}


void UdsServerBase::SendResponse(const uint8_t* data, int32_t len)
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

void UdsServerBase::SendNegResponse(uint8_t sid, NRCs_t nrc)
{
  router_tx_buff[0] = PUDS_NR_SI;
  router_tx_buff[1] = sid;
  router_tx_buff[2] = static_cast<uint8_t>(nrc);
  nrc_bad_param = (nrc == NRC_IMLOIF);
  nrc_code = nrc;

  SendResponse(router_tx_buff, 3);
}

void UdsServerBase::SendNegResponse(NRCs_t nrc)
{
  SendNegResponse(sihead.SI, nrc);
}


void UdsServerBase::SetServiceSession(uint8_t s)
{
  // make all the control inside the session layer
  SessionChangeEvent(s);

  // all the clients must be informed
  NotifyDSCSessionChanged(false);
}


void UdsServerBase::SetSecurityLevel(uint8_t sa_level)
{
  sess_info.sec_level = sa_level;
}


void UdsServerBase::SessionChangeEvent(uint8_t s)
{
  SetSessionMode(s == DSC_SF_DS);

  if (sess_info.sess != s)
  {
    // current session will be changed. notify every client about it
    sess_info.sess = s;
  }
}

void UdsServerBase::NotifyInd(const uint8_t* data, uint32_t length, UdsAddress addr)
{
  nrc_code = NRC_PR;
  nrc_bad_param = false;

  req_addr = addr;

  data_info.data = data;
  data_info.size = length;
  sihead.SI = data_info.data[0];

  // ISO 14229-1 7.3.2 table 2 (p. 25)
  bool bad_sid = out_of_range<0x10, 0x3e>(sihead.SI) && out_of_range<0x83, 0x88>(sihead.SI);

  sihead.SF = (data_info.data[1] & 0x7FU);
  sihead.respSI = RESPONSE_ON_SID(sihead.SI);
  // services without subfunctions must set NoResponse bit to 0 by themself!!!
  sihead.NoResponse = data_info.data[1] & 0x80U ? 1 : 0;
  // set most frequent case
  tData[0] = sihead.respSI;
  tData[1] = data_info.data[1];
  tLength = 0;

  if (router_is_disabled || bad_sid)
  {
    return;
  }

  // Handle base service functions
  if (SelfIndHandler())
  {
    SendResponse(tData, tLength);
    return;
  }

  UdsServiceHandler* client = nullptr;
  uint32_t i = 0u;

  while (cls.Item(i++, client))
  {
    clientHandRes = client->OnIndication(data_info);

    if (clientHandRes == kSI_NotHandled)
    {
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

void UdsServerBase::NotifyConf(S_Result res)
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

  UdsServiceHandler* client = nullptr;
  uint32_t i = 0u;

  while (cls.Item(i++, client))
  {
    clientHandRes = client->OnIndication(data_info);

    if (clientHandRes == kSI_NotHandled)
    {
      break;
    }
  }
}


uint8_t UdsServerBase::GetNRC()
{
  return nrc_code;
}


void UdsServerBase::On_s3_Timeout()
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


void UdsServerBase::NotifyDSCSessionChanged(bool s3timer)
{
  UdsServiceHandler* client = nullptr;
  uint32_t i = 0u;

  while (cls.Item(i++, client))
  {
    client->DSCSessionEvent(s3timer);
  }
}


bool UdsServerBase::ResponseAllowed()
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

void UdsServerBase::RegisterClient(UdsServiceHandler* client)
{
  cls.Add(client);
}

void UdsServerBase::RouterDisable()
{
  router_is_disabled = true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool UdsServerBase::SelfIndHandler()
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
bool UdsServerBase::SelfConfHandler()
{
  return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void UdsServerBase::SID_TesterPresent()
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
void UdsServerBase::SID_DiagServiceControl()
{
  // minimal length check already done
  if (sihead.SF == 0 || sihead.SF > 3)
  {
    SendNegResponse(NRC_SFNS);
  }
  else if (data_info.size != 2)
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
    SetServiceSession(sihead.SF);
  }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool UdsServerBase::MakeBaseSIDChecks()
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

