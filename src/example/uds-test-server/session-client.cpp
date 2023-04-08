#include <iostream>
#include "session-client.h"


bool DSCClient::IsServiceSupported(SIDs sid, size_t& minlength, bool& subfunc) {

  (void) subfunc;

  if (sid == SIDs::DSC) {
    minlength = 2u;
    return true;
  } else if (sid == SIDs::SA) {
    minlength = 5u;
    return true;
  }

  return false;
}


ProcessResult DSCClient::OnAppIndication(const IndicationInfo& inf) {

  std::cout << "DSC handler : ";

  if (inf.head.SI == SIDs::DSC) {
    std::cout << "SI ok, ";

    if (inf.head.SF > 0 && inf.head.SF <= 3) {
      std::cout << "Session ok (" << (int) inf.head.SF << ")" << std::endl;
      // session change request ok
      udsRouter.pubBuff[2] = ((250 >> 8) & 0xFF);
      udsRouter.pubBuff[3] = (250 & 0xFF);
      udsRouter.pubBuff[4] = (((5000 / 10) >> 8) & 0xFF);
      udsRouter.pubBuff[5] = ((5000 / 10) & 0xFF);
      sessionInfoContext.currSession = inf.head.SF;
      udsRouter.SetServiceSession(sessionInfoContext.currSession);
      udsRouter.SendResponse(udsRouter.pubBuff, 6);
    } else {
      std::cout << "SF bad (" << (int) inf.head.SF << ")" << std::endl;
      udsRouter.SendNegResponse(NRCs::SFNS);
    }

    return ProcessResult::HANDLED_RESP_OK;
  } else {
    std::cout << std::endl;
  }

  return ProcessResult::NOT_HANDLED;

}


void DSCClient::OnSessionChange(bool isdefault) {

  if (isdefault) {
    sessionInfoContext.currSession = 1u;
    sessionInfoContext.secLevel = 0u;
  }
}
