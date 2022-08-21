#include <iostream>
#include "session-client.h"


ProcessResult_t DSCClient::OnIndication(const IndicationInfo& inf)
{
  std::cout << "DSC handler : ";

  if (inf.head.SI == PUDS_SI_DiagnosticSessionControl)
  {
    std::cout << "SI ok, ";

    if (inf.head.SF > 0 && inf.head.SF <= 3)
    {
      std::cout << "Session ok (" << (int)inf.head.SF << ")" << std::endl;
      // session change request ok
      rtr1.SetServiceSession((uint8_t)inf.head.SF);
      rtr1.tData[2] = ((250 >> 8) & 0xFF);
      rtr1.tData[3] = (250 & 0xFF);
      rtr1.tData[4] = (((5000 / 10) >> 8) & 0xFF);
      rtr1.tData[5] = ((5000 / 10) & 0xFF);
      rtr1.SendResponse(rtr1.tData, 6);
    }
    else
    {
      std::cout << "SF bad (" << (int)inf.head.SF << ")" << std::endl;
      rtr1.SendNegResponse(NRC_SFNS);
    }

    return kSI_HandledResponseOk;
  }
  else
  {
    std::cout << std::endl;
  }

  return kSI_NotHandled;

}
