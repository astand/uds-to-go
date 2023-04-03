#include "uds-server-base.h"
#include "uds-service-handler.h"

static constexpr uint8_t DSC_SF_DS = 0x01u;

static constexpr UdsServerBase::flag_t BIT_PHYSADDR = (1u << 4u);
static constexpr UdsServerBase::flag_t BIT_SUPPORT = (1u << 5u);
static constexpr UdsServerBase::flag_t BIT_NOINDEF = (1u << 6u);
static constexpr UdsServerBase::flag_t BIT_NOSUBF = (1u << 7u);

constexpr bool IsActive(const UdsServerBase::flag_t flag, const UdsServerBase::flag_t bits)
{
  return ((flag & bits) != 0u);
}

constexpr uint8_t MinLength(const UdsServerBase::flag_t flag)
{
  return (flag & 7u);
}

#define IS_NRC_PHYS_NOPOS(x) ((x) == NRCs::ROOR || (x) == NRCs::SNS || (x) == NRCs::SFNS)

template<uint8_t low, uint8_t high>
bool out_of_range(uint8_t v)
{
  assert(low < high);
  return ((v < low) || (v > high));
}

UdsServerBase::UdsServerBase(IKeeper<UdsServiceHandler>& vec, uint8_t* d, datasize_t s) : TX_SIZE(s), pubBuff(d), cls(vec)
{
  assert(pubBuff != nullptr);
  assert(TX_SIZE != 0);

  EnableSID(SIDs::TP, false, true, false, 2);

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

void UdsServerBase::SendNegResponse(SIDs sid, NRCs nrc)
{
  pubBuff[0] = SID_to_byte(SIDs::NR_SI);
  pubBuff[1] = SID_to_byte(sid);
  pubBuff[2] = NRC_to_byte(nrc);
  nrc_bad_param = (nrc == NRCs::IMLOIF);
  nrc_code = nrc;

  SendResponse(pubBuff, 3, nrc == NRCs::RCRRP);
}

void UdsServerBase::SendNegResponse(NRCs nrc)
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

bool UdsServerBase::EnableSID(SIDs sid, bool noindef, bool nosubf, bool onlyphys, uint8_t minlen)
{
  SID_Flag[SID_to_byte(sid)] = BIT_SUPPORT |
    (noindef ? BIT_NOINDEF : 0u) |
    (nosubf ? BIT_NOSUBF : 0u) |
    (onlyphys ? BIT_PHYSADDR : 0u) |
    (minlen > 0xFu ? 0xFu : minlen);

  return true;
}

void UdsServerBase::NotifyInd(const uint8_t* data, uint32_t length, TargetAddressType addr)
{
  nrc_code = NRCs::PR;
  nrc_bad_param = false;

  req_addr = addr;

  data_info.data = data;
  data_info.size = length;

  // ISO 14229-1 7.3.2 table 2 (p. 25)
  bool bad_sid = out_of_range<0x10, 0x3e>(data_info.data[0]) &&
    out_of_range<0x83, 0x88>(data_info.data[0]);

  if (router_is_disabled || bad_sid)
  {
    return;
  }

  data_info.head.SI = static_cast<SIDs>(data_info.data[0]);

  data_info.head.SF = (data_info.data[1] & 0x7FU);
  data_info.head.respSI = SID_response(data_info.head.SI);
  // services without subfunctions must set NoResponse bit to 0 by themself!!!
  data_info.head.NoResponse = data_info.data[1] & 0x80U ? 1 : 0;
  // set most frequent case
  pubBuff[0] = data_info.head.respSI;
  pubBuff[1] = data_info.data[1];
  pubSize = 0;

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
    // NRC NRCs::SNS must be sent (ISO 14229-1 table 4 (i))
    if (req_addr == TargetAddressType::PHYS)
    {
      SendNegResponse(NRCs::SNS);
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
  else if (clientHandRes == ProcessResult::HANDLED_PENDING)
  {
    // handler in pending state, wait for reposne
    SetPending(60000u, 2000u, data_info.head.SI);
  }
}

void UdsServerBase::NotifyConf(S_Result)
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
    return (nrc_bad_param || (data_info.head.NoResponse == false && nrc_code == NRCs::PR));
  }
  else if (req_addr == TargetAddressType::PHYS)
  {
    if (nrc_code != NRCs::RCRRP)
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

  if (data_info.head.SI == SIDs::TP)
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
    SendNegResponse(NRCs::SFNS);
  }
  else if (data_info.size != 2)
  {
    // TesterPresent : total length check
    SendNegResponse(NRCs::IMLOIF);
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
  uint8_t flags = SID_Flag[SID_to_byte(data_info.head.SI)];

  // negResponse handling
  if (IsActive(flags, BIT_SUPPORT) == false)
  {
    // service not supported
    SendNegResponse(NRCs::SNS);
  }
  else if (IsActive(flags, BIT_PHYSADDR) && (req_addr != TargetAddressType::PHYS))
  {
    // do nothing here,the true will be returned and no further actions will be made
  }
  else if (IsActive(flags, BIT_NOINDEF) && (sess_info.sess == DSC_SF_DS))
  {
    SendNegResponse(NRCs::SNSIAS);
  }
  else if ((MinLength(flags) != 0u) && (data_info.size < MinLength(flags)))
  {
    SendNegResponse(NRCs::IMLOIF);
  }
  else
  {
    ret = false;
  }

  // noResponse bit handling
  if (IsActive(flags, BIT_NOSUBF))
  {
    data_info.head.NoResponse = 0;
  }

  return ret;
}

