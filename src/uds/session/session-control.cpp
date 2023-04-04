#include <assert.h>
#include "session-control.h"
#include <uds/inc/diag/nrcnames.h>

void SessionControl::SendRequest(const uint8_t* data, uint32_t size, bool enhanced)
{
  isEtmActive = enhanced;

  if (isEtmActive)
  {
    p2.Start(tims.p2_enhanced);
  }
  else
  {
    // stop p2 timer on response sending
    p2.Stop();
  }

  // send response to iso-tp
  host->Request(data, size);
}

void SessionControl::SetPending(uint32_t duration, uint32_t interval, SIDs si)
{
  if ((duration >= interval) && (interval > 999u))
  {
    etmWaitRespBuff[1] = SID_to_byte(si);
    SendRequest(etmWaitRespBuff, 3u, true);
    etmTim.Start(interval);
    etmKeepAliveInterval = interval;
    etmDuration = duration - interval;
  }
}

void SessionControl::Process()
{
  // process timers here and notify Diag in case of timeout
  ProcessSessionMode();

  if (isEtmActive)
  {
    // left duration not zero, continue periodic send
    if ((etmDuration != 0u) && etmTim.Elapsed())
    {
      // etm pending message has to be sent
      etmDuration = (etmDuration > etmKeepAliveInterval) ? (etmDuration - etmKeepAliveInterval) : 0u;
      SendRequest(etmWaitRespBuff, 3, true);

      if (etmDuration == 0)
      {
        // after this p2 timer will handled by general session processing
        etmTim.Stop();
      }
    }
  }
}

void SessionControl::OnIsoEvent(N_Event event, N_Result res, const IsoTpInfo& info)
{
  targetAddress = (info.address == N_TarAddress::TAtype_1_Physical) ? (TargetAddressType::PHYS) : (TargetAddressType::FUNC);

  switch (event)
  {
    case (N_Event::Data):
      if (res == N_Result::OK_r)
      {
        if (!isEtmActive)
        {
          // data indication from Transport layer
          p2.Start(tims.p2_max);

          if (sessType == SessionType::NONDEFAULT)
          {
            // stop s3 here (to be restart on response conf)
            S3.Stop();
          }

          NotifyInd(info.data, info.length, targetAddress);
        }
        else
        {
          // do not process any request while etm is active
        }
      }
      else
      {
        // unexpected result
        NotifyConf(S_Result::NOK);
      }

      break;

    case (N_Event::DataFF):
      if (res == N_Result::OK_r)
      {
        if (sessType == SessionType::NONDEFAULT)
        {
          // stop s3 here to be aviod session stop during long receiving
          S3.Stop();
        }
      }
      else
      {
        // should never get here
        SetSessionMode(sessType);
        assert(false);
      }

      break;

    case (N_Event::Conf):
      if (res == N_Result::OK_s)
      {
        if (!isEtmActive)
        {
          // update current session state
          SetSessionMode(sessType);
        }
      }
      else if (res == N_Result::TIMEOUT_Cr)
      {
        // segmented receptions is broken
        SetSessionMode(sessType);
      }
      else
      {
        // error occured
      }
  }

  return;
}


void SessionControl::SetSessionMode(bool is_default)
{
  if (is_default)
  {
    // default session doesn't use s3 timer, so stop it
    S3.Stop();
    sessType = SessionType::DEFAULT;
  }
  else
  {
    if (sessType == SessionType::DEFAULT)
    {
      // initial start of S3 timer
      S3.Start(tims.S3_max);
      sessType = SessionType::NONDEFAULT;
    }
    else
    {
      // already not default -> subsequent start
      S3.Restart();
    }
  }
}


void SessionControl::ProcessSessionMode()
{
  if (sessType != SessionType::DEFAULT)
  {
    if (S3.Elapsed())
    {
      SetSessionMode(true);
      On_s3_Timeout();
    }
  }

  if (p2.Elapsed())
  {
    if (sessType != SessionType::DEFAULT)
    {
      // Restart S3 timer!
      S3.Restart();
    }

    if (isEtmActive)
    {
      // no response from handler was sent
      isEtmActive = false;
      // send common NRC (protocol requirement)
      etmWaitRespBuff[2] = NRC_to_byte(NRCs::ENOA);
      SendRequest(etmWaitRespBuff, 3u);
      // restore original NRC value in etm buffer
      etmWaitRespBuff[2] = NRC_to_byte(NRCs::RCRRP);
    }
  }
}

SessParamResult SessionControl::SetSessionParam(SessParamType par, uint32_t v)
{
  auto ret = SessParamResult::OK;

  switch (par)
  {
    case (SessParamType::S3_TIM):
      tims.S3_max = v;
      break;

    case (SessParamType::P2_TIM):
      tims.p2_max = v;
      break;

    case (SessParamType::P2_ENHC):
      tims.p2_enhanced = v;
      break;

    default:
      ret = SessParamResult::ERR;
      break;
  }

  return ret;
}

