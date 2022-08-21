#include <assert.h>
#include "session-control.h"
#include <uds/inc/diag/nrcnames.h>

SessionControl::SessionControl()
{
  sState.session = SessionState::DEFAULT;
  tims.S3_max = tims.S3_max;
  tims.p2_max = tims.p2_max;
  tims.p2_enhanced = tims.p2_enhanced;
}

void SessionControl::SendRequest(const uint8_t* data, uint32_t len)
{
  // resetart timer and may be some session state here
  // check availability / restart timers
  // any response should stop p2 timer
  if (host != nullptr)
  {
    p2.Stop();
    host->Request(data, len);
  }
}

void SessionControl::Process()
{
  // process timers here and notify Diag in case of timeout
  ProcessSessionMode();
}

void SessionControl::OnIsoEvent(N_Type event, N_Result res, const IsoTpInfo& info)
{
  sState.atype = (info.address == N_TarAddress::TAtype_1_Physical) ? (UdsAddress::PHYS) : (UdsAddress::FUNC);

  switch (event)
  {
    case (N_Type::Data):
      if (res == N_Result::OK_r)
      {
        // data indication from Transport layer
        p2.Start(tims.p2_max);

        if (sState.session == SessionState::NONDEFAULT)
        {
          // when non default session is active S3 timer must be stopped to
          // complete req/repsond transfer
          S3.Stop();
        }

        NotifyInd(info.data, info.length, sState.atype);
      }
      else
      {
        // the data was corrupted of something else happened
        NotifyConf(S_Result::NOK);
      }

      break;

    case (N_Type::DataFF):
      if (res == N_Result::OK_r)
      {
        // SOM indication is not sending to Diag app.
        // TODO: restart timings?
        if (sState.session == SessionState::NONDEFAULT)
        {
          // when non default session is active S3 timer must be stopped to
          // complete req/repsond transfer
          S3.Stop();
        }
      }
      else
      {
        // what happend?
        SetSessionMode(sState.session);
        assert(false);
      }

      break;

    case (N_Type::Conf):

      // confirmation about timeout error and data transferring ending like this
      if (res == N_Result::OK_s)
      {
        // response send OK, check what was that
        if ((GetNRC() == NRC_RCRRP) && (sState.session == SessionState::NONDEFAULT))
        {
          // response was NRC_RCRRP - set p2 enhanced timeout
          p2.Start(tims.p2_enhanced);
        }
        else
        {
          SetSessionMode(sState.session);
        }
      }
      else if (res == N_Result::TIMEOUT_Cr)
      {
        // segmented receptions is broken
        SetSessionMode(sState.session);
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
    sState.session = SessionState::DEFAULT;
  }
  else
  {
    if (sState.session == SessionState::DEFAULT)
    {
      // initial start of S3 timer
      S3.Start(tims.S3_max);
      sState.session = SessionState::NONDEFAULT;
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
  if (sState.session != SessionState::DEFAULT)
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