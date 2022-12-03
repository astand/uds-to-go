#include "uds-server-base.h"
#include "uds-service-handler.h"

constexpr uint8_t DSC_SF_DS = 0x01u;

#define IS_NRC_PASSED(x) ((x == NRC_SNS) ||\
  (x == NRC_SNSIAS) ||\
  (x == NRC_SFNS) ||\
  (x == NRC_SFNSIAS) ||\
  (x == NRC_ROOR))

#define IS_NRC_PHYS_NOPOS(x) ((x) == NRC_ROOR || (x) == NRC_SNS || (x) == NRC_SFNS)

/* ------------------- SID_Phyaddr   SID_Support   SID_NoInDef   SID_NoSubFu   SID_HasMinL */
#define SI_Flags_TP    SID_Support | SID________ | SID________ | SID________ | SID_MinLen(2)

template<uint8_t low, uint8_t high>
bool out_of_range(uint8_t v)
{
  assert(low < high);
  return (v < low || v > high);
}

UdsServerBase::UdsServerBase(IKeeper<UdsServiceHandler>& vec, uint8_t* d, datasize_t s) : cls(vec), pubBuff(d),
  TX_SIZE(s)
{
  assert(pubBuff != nullptr);
  assert(TX_SIZE != 0);

  SID_Flag[PUDS_SI_TesterPresent] = SI_Flags_TP;

  sess_info.sec_level = 0;
  sess_info.sess = DSC_SF_DS;

  router_is_disabled = false;
}


void UdsServerBase::SendResponse(const uint8_t* data, uint32_t len, bool enhanced)
{
  if (len == 0)
  {
    return;
  }

  // check address and NRC code
  if (ResponseAllowed())
  {
    SendRequest(data, len, enhanced);
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
  pubBuff[0] = PUDS_NR_SI;
  pubBuff[1] = sid;
  pubBuff[2] = static_cast<uint8_t>(nrc);
  nrc_bad_param = (nrc == NRC_IMLOIF);
  nrc_code = nrc;

  SendResponse(pubBuff, 3, nrc == NRCs_t::NRC_RCRRP);
}

void UdsServerBase::SendNegResponse(NRCs_t nrc)
{
  SendNegResponse(data_info.head.SI, nrc);
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

void UdsServerBase::NotifyInd(const uint8_t* data, uint32_t length, TargetAddressType addr)
{
  nrc_code = NRC_PR;
  nrc_bad_param = false;

  req_addr = addr;

  data_info.data = data;
  data_info.size = length;
  data_info.head.SI = data_info.data[0];

  // ISO 14229-1 7.3.2 table 2 (p. 25)
  bool bad_sid = out_of_range<0x10, 0x3e>(data_info.head.SI) && out_of_range<0x83, 0x88>(data_info.head.SI);

  data_info.head.SF = (data_info.data[1] & 0x7FU);
  data_info.head.respSI = RESPONSE_ON_SID(data_info.head.SI);
  // services without subfunctions must set NoResponse bit to 0 by themself!!!
  data_info.head.NoResponse = data_info.data[1] & 0x80U ? 1 : 0;
  // set most frequent case
  pubBuff[0] = data_info.head.respSI;
  pubBuff[1] = data_info.data[1];
  pubSize = 0;

  if (router_is_disabled || bad_sid)
  {
    return;
  }

  // Handle base service functions
  if (SelfIndHandler())
  {
    SendResponse(pubBuff, pubSize);
    return;
  }

  UdsServiceHandler* client = nullptr;
  uint32_t i = 0u;

  while (cls.Item(i++, client))
  {
    clientHandRes = client->OnIndication(data_info);

    if (clientHandRes != ProcessResult::NOT_HANDLED)
    {
      break;
    }
  }

  if (clientHandRes == ProcessResult::NOT_HANDLED)
  {
    // there was no service to answer. so if the address is physycal
    // NRC NRC_SNS must be sent (ISO 14229-1 table 4 (i))
    if (req_addr == TargetAddressType::PHYS)
    {
      SendNegResponse(NRC_SNS);
    }
  }
  else if (clientHandRes == ProcessResult::HANDLED_RESP_OK)
  {
    SendResponse(pubBuff, pubSize);
  }
  else if (clientHandRes == ProcessResult::HANDLED_RESP_NO)
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
  data_info.addr = TargetAddressType::UNKNOWN;
  data_info.head = data_info.head;

  UdsServiceHandler* client = nullptr;
  uint32_t i = 0u;

  while (cls.Item(i++, client))
  {
    clientHandRes = client->OnIndication(data_info);

    if (clientHandRes == ProcessResult::NOT_HANDLED)
    {
      break;
    }
  }
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
  if (req_addr == TargetAddressType::FUNC)
  {
    // ISO 14229-1 Table 5
    return (nrc_bad_param || (data_info.head.NoResponse == false && nrc_code == NRC_PR));
  }
  else if (req_addr == TargetAddressType::PHYS)
  {
    if (nrc_code != NRC_RCRRP)
    {
      /* ISO 14229-1 7.5.3.2 tab 4 (p 30)                               h, i, j                  */
      return (data_info.head.NoResponse == false || (IS_NRC_PHYS_NOPOS(nrc_code) ||  nrc_bad_param));
    }
    else
    {
      /* ISO 14229-1 7.5.3.2 tab 4 (p 30) - *for the case with RCRRP negative response */
      data_info.head.NoResponse = false;
      return true;
    }
  }

  return false;
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

  if (data_info.head.SI == PUDS_SI_TesterPresent)
  {
    SID_TesterPresent();
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
  if (data_info.head.SF != 0)
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
    pubSize = 2;
  }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool UdsServerBase::MakeBaseSIDChecks()
{
  //service is available
  bool ret = true;
  uint8_t flags = SID_Flag[data_info.head.SI];

  // negResponse handling
  if (flags == 0)
  {
    // service not supported
    SendNegResponse(NRC_SNS);
  }
  else if (((flags & SID_Phyaddr) != 0) && (req_addr != TargetAddressType::PHYS))
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
    data_info.head.NoResponse = 0;
  }

  return ret;
}

