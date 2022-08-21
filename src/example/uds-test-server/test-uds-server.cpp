#include "test-uds-server.h"

/* ------------------- SID_Phyaddr   SID_Support   SID_NoInDef   SID_NoSubFu   SID_HasMinL */
#define SI_Flags_DSC   SID_Support | SID________ | SID________ | SID________ | SID_MinLen(2)

TestUdsServer::TestUdsServer(IKeeper<UdsServiceHandler>& vec, uint8_t* txptr, datasize_t txsize) :
  UdsServerBase(vec, txptr, txsize)
{
  SID_Flag[PUDS_SI_DiagnosticSessionControl] = SI_Flags_DSC;
  SetSessionParam(SessParam::S3_TIM, 5000);
  SetSessionParam(SessParam::P2_TIM, 250);
  SetSessionParam(SessParam::P2_ENHC, 5000);
}

