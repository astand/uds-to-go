#include <iostream>
#include <cstdlib>
#include <iostream>
#include "session-client.h"


bool DSCClient::IsServiceSupported(sid_t sid, size_t& minlength, bool& subfunc) {

  (void) subfunc;

  if (sid == sidhelper::DSC) {
    minlength = 2u;
    return true;
  } else if (sid == sidhelper::SA) {
    minlength = 2u;
    return true;
  }

  return false;
}


ProcessResult DSCClient::OnAppIndication(const IndicationInfo& inf) {

  std::cout << "DSC handler : ";

  if (inf.head.SI == sidhelper::DSC) {
    std::cout << "SI ok, ";

    if (inf.head.SF > 0 && inf.head.SF <= 3) {
      if (sessionInfoContext.currSession == 1u && inf.head.SF == 2u) {
        // direct transition from default to programmaing session is not allowed
        udsRouter.SendNegResponse(NRCs::SFNSIAS);
      } else {
        std::cout << "Session ok (" << (int) inf.head.SF << ")" << std::endl;
        // session change request ok
        udsRouter.pubBuff[2] = ((250 >> 8) & 0xFF);
        udsRouter.pubBuff[3] = (250 & 0xFF);
        udsRouter.pubBuff[4] = (((5000 / 10) >> 8) & 0xFF);
        udsRouter.pubBuff[5] = ((5000 / 10) & 0xFF);
        sessionInfoContext.currSession = inf.head.SF;
        udsRouter.SetServiceSession(sessionInfoContext.currSession);
        udsRouter.SendResponse(udsRouter.pubBuff, 6);
      }
    } else {
      std::cout << "SF bad (" << (int) inf.head.SF << ")" << std::endl;
      udsRouter.SendNegResponse(NRCs::SFNS);
    }

    return ProcessResult::HANDLED_RESP_OK;
  } else if (inf.head.SI == sidhelper::SA) {
    return Handle_SA_request(inf);
  } else if (inf.head.SI == sidhelper::to_response(sidhelper::SA)) {
    return Handle_SA_response(inf);
  }

  return ProcessResult::NOT_HANDLED;
}


void DSCClient::OnSessionChange(bool isdefault) {

  if (isdefault) {
    sessionInfoContext.currSession = 1u;
    sessionInfoContext.secLevel = 0u;
  }
}

ProcessResult DSCClient::Handle_SA_request(const IndicationInfo& reqcontext) {

  uint16_t keytocheck = 0u;

  // if request check if seed can be sent
  if ((reqcontext.data[1] & 0x01u) == 0u) {
    // key must be checked
    switch (reqcontext.data[1] >> 1u) {
      case (1):
        keytocheck = seedsent + 4u;
        break;

      case (2):
        keytocheck = seedsent + 8u;
        break;

      default:
        keytocheck = 0xabab;
        break;
    }

    if (ophelper::from_be_u16(HWREGH(reqcontext.data + 2)) == keytocheck) {
      udsRouter.pubRespLength = 2u;
    } else {
      udsRouter.SendNegResponse(NRCs::SAD);
      return ProcessResult::HANDLED_RESP_NO;
    }
  } else {
    seedsent = rand() & 0xffffu;
    HWREGH(udsRouter.pubBuff + 2) = ophelper::to_be_u16(seedsent);
    udsRouter.pubRespLength = 4u;
  }

  return ProcessResult::HANDLED_RESP_OK;
}

ProcessResult DSCClient::Handle_SA_response(const IndicationInfo& reqcontext) {

  // if response the key must be calculated and send back
  uint16_t seedvalue = ophelper::from_be_u16(HWREGH(reqcontext.data + 2));
  uint16_t keyvalue = 0u;
  bool isready = true;

  if ((reqcontext.data[1] & 0x01u) == 0x01u) {
    switch (reqcontext.data[1] >> 1u) {
      case (0):
        keyvalue = seedvalue + 4;
        break;

      case (1):
        keyvalue = seedvalue + 8;
        break;

      default:
        keyvalue = 0x0000;
        break;
    }
  } else {
    isready = false;
  }

  std::cout << "is ready : " << isready << std::endl;

  // key must be send to server as the second step
  if (isready) {
    udsRouter.pubBuff[0] = sidhelper::SA;
    udsRouter.pubBuff[1] = reqcontext.data[1] + 1;
    HWREGH(udsRouter.pubBuff + 2) = ophelper::to_be_u16(keyvalue);
    udsRouter.SendResponse(udsRouter.pubBuff, 4);
  }

  return ProcessResult::NOT_HANDLED;
}
