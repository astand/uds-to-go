#include <assert.h>
#include "session-control.h"
#include <uds/inc/diag/nrcnames.h>

void SessionControl::SendRequest(const uint8_t* data, uint32_t len)
{
  // stop p2 timer on response sending
  p2.Stop();

  // send response to iso-tp
  host->Request(data, len);
}

void SessionControl::Process()
{
  // process timers here and notify Diag in case of timeout
  ProcessSessionMode();
}

void SessionControl::OnIsoEvent(N_Event event, N_Result res, const IsoTpInfo& info)
{
  ta_addr = (info.address == N_TarAddress::TAtype_1_Physical) ? (TargetAddressType::PHYS) : (TargetAddressType::FUNC);

  switch (event)
  {
    case (N_Event::Data):
      if (res == N_Result::OK_r)
      {
        // data indication from Transport layer
        p2.Start(tims.p2_max);

        if (ss_state == SessionType::NONDEFAULT)
        {
          // stop s3 here (to be restart on response conf)
          S3.Stop();
        }

        NotifyInd(info.data, info.length, ta_addr);
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
        if (ss_state == SessionType::NONDEFAULT)
        {
          // stop s3 here to be aviod session stop during long receiving
          S3.Stop();
        }
      }
      else
      {
        // should never get here
        SetSessionMode(ss_state);
        assert(false);
      }

      break;

    case (N_Event::Conf):
      if (res == N_Result::OK_s)
      {
        // response send OK, check what was that
        if ((GetNRC() == NRC_RCRRP) && (ss_state == SessionType::NONDEFAULT))
        {
          // p2 enhanced here because of RCRRP
          p2.Start(tims.p2_enhanced);
        }
        else
        {
          // update current session type (restart s3)
          SetSessionMode(ss_state);
        }
      }
      else if (res == N_Result::TIMEOUT_Cr)
      {
        // segmented receptions is broken
        SetSessionMode(ss_state);
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
    ss_state = SessionType::DEFAULT;
  }
  else
  {
    if (ss_state == SessionType::DEFAULT)
    {
      // initial start of S3 timer
      S3.Start(tims.S3_max);
      ss_state = SessionType::NONDEFAULT;
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
  if (ss_state != SessionType::DEFAULT)
  {
    if (S3.Elapsed())
    {
      SetSessionMode(true);
      On_s3_Timeout();
    }

    if (p2.Elapsed())
    {
      // Restart S3 timer!
      S3.Restart();
    }
  }
}

SessResult SessionControl::SetSessionParam(SessParam par, uint32_t v)
{
  auto ret = SessResult::OK;

  switch (par)
  {
    case (SessParam::S3_TIM):
      tims.S3_max = v;
      break;

    case (SessParam::P2_TIM):
      tims.p2_max = v;
      break;

    case (SessParam::P2_ENHC):
      tims.p2_enhanced = v;
      break;

    default:
      ret = SessResult::ERR;
      break;
  }

  return ret;
}