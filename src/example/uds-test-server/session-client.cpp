#include <iostream>
#include "session-client.h"


ProcessResult DSCClient::OnIndication(const IndicationInfo& inf)
{
  std::cout << "DSC handler : ";

  if (inf.head.SI == SIDs::PUDS_SI_DiagnosticSessionControl)
  {
    std::cout << "SI ok, ";

    if (inf.head.SF > 0 && inf.head.SF <= 3)
    {
      std::cout << "Session ok (" << (int)inf.head.SF << ")" << std::endl;
      // session change request ok
      rtr1.SetServiceSession((uint8_t)inf.head.SF);
      rtr1.pubBuff[2] = ((250 >> 8) & 0xFF);
      rtr1.pubBuff[3] = (250 & 0xFF);
      rtr1.pubBuff[4] = (((5000 / 10) >> 8) & 0xFF);
      rtr1.pubBuff[5] = ((5000 / 10) & 0xFF);
      rtr1.SendResponse(rtr1.pubBuff, 6);
    }
    else
    {
      std::cout << "SF bad (" << (int)inf.head.SF << ")" << std::endl;
      rtr1.SendNegResponse(NRCs::SFNS);
    }

    return ProcessResult::HANDLED_RESP_OK;
  }
  else
  {
    std::cout << std::endl;
  }

  return ProcessResult::NOT_HANDLED;

}
