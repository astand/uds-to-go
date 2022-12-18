#include "test-uds-server.h"

/* ----------------------- SID_Phyaddr   SID_Support   SID_NoInDef   SID_NoSubFu   SID_HasMinL */
#define SI_Flags_DSC       SID_Support | SID________ | SID________ | SID________ | SID_MinLen(2)
#define SI_Flags_RDBI      SID_Support | SID_NoSubFu | SID________ | SID________ | SID_MinLen(3)
#define SI_Flags_RC        SID_Support | SID________ | SID________ | SID________ | SID_MinLen(3)


TestUdsServer::TestUdsServer(IKeeper<UdsServiceHandler>& vec, uint8_t* txptr, datasize_t txsize) :
  UdsServerBase(vec, txptr, txsize)
{
  SID_Flag[SID_to_byte(SIDs::PUDS_SI_DiagnosticSessionControl)] = SI_Flags_DSC;
  SID_Flag[SID_to_byte(SIDs::PUDS_SI_ReadDataByIdentifier)] = SI_Flags_RDBI;
  SID_Flag[SID_to_byte(SIDs::PUDS_SI_RoutineControl)] = SI_Flags_RC;

  SetSessionParam(SessParamType::S3_TIM, 5000);
  SetSessionParam(SessParamType::P2_TIM, 250);
  SetSessionParam(SessParamType::P2_ENHC, 5000);
}

