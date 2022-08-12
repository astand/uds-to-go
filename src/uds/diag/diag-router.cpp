#include "diag-router.h"

void DiagRouter::SendResponse(const uint8_t* data, size_t len)
{
  assert(host != nullptr);
  host->Request(data, len);
}

void DiagRouter::SendNegResponse(NRCs_t nrc)
{
  SendNegResponse(/* sihead.SI */ 0, nrc);
}

void DiagRouter::SendNegResponse(uint8_t sid, NRCs_t nrc)
{
  sbuff[0] = sid;
  sbuff[1] = nrc;

  SendResponse(sbuff, 2);
}


void DiagRouter::OnIsoEvent(N_Type t, N_Result res, const IsoTpInfo& info)
{
  switch (t)
  {
    case (N_Type::DataFF):
      Handle_DataFF();
      break;

    case (N_Type::Data):
      Handle_Data();
      break;

    case (N_Type::Conf):
      Handle_Conf();
      break;

    default:
      break;
  }// event from network layer
}

void DiagRouter::Handle_Conf()
{
}

void DiagRouter::Handle_DataFF()
{
}

void DiagRouter::Handle_Data()
{
}
