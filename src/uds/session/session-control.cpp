#include <assert.h>
#include "session-control.h"
#include <uds/inc/diag/nrcnames.h>

void SessionControl::SendRequest(const uint8_t* data, uint32_t size, bool enhanced)
{
  etm_active = enhanced;

  if (etm_active)
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

void SessionControl::SetPending(uint32_t duration, uint32_t interval, uint8_t si)
{
  if ((duration >= interval) && (interval > 999u))
  {
    etm_buff[1] = si;
    SendRequest(etm_buff, 3u, true);
    etm_timer.Start(interval);
    etm_interval = interval;
    left_duration = duration - interval;
  }
}

void SessionControl::Process()
{
  // process timers here and notify Diag in case of timeout
  ProcessSessionMode();

  if (etm_active)
  {
    // left duration not zero, continue periodic send
    if ((left_duration != 0u) && etm_timer.Elapsed())
    {
      // etm pending message has to be sent
      left_duration = (left_duration > etm_interval) ? (left_duration - etm_interval) : 0u;
      SendRequest(etm_buff, 3, true);

      if (left_duration == 0)
      {
        // after this p2 timer will handled by general session processing
        etm_timer.Stop();
      }
    }
  }
}

void SessionControl::OnIsoEvent(N_Event event, N_Result res, const IsoTpInfo& info)
{
  ta_addr = (info.address == N_TarAddress::TAtype_1_Physical) ? (TargetAddressType::PHYS) : (TargetAddressType::FUNC);

  switch (event)
  {
    case (N_Event::Data):
      if (res == N_Result::OK_r)
      {
        if (!etm_active)
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
        if (!etm_active)
        {
          // update current session state
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
  }

  if (p2.Elapsed())
  {
    if (ss_state != SessionType::DEFAULT)
    {
      // Restart S3 timer!
      S3.Restart();
    }

    if (etm_active)
    {
      // no response from handler was sent
      etm_active = false;
      // send common NRC (protocol requirement)
      etm_buff[2] = NRCs_t::NRC_ENOA;
      SendRequest(etm_buff, 3u);
      // restore original NRC value in etm buffer
      etm_buff[2] = NRCs_t::NRC_RCRRP;
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

