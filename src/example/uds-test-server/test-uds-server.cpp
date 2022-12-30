#include "test-uds-server.h"

TestUdsServer::TestUdsServer(IKeeper<UdsServiceHandler>& vec, uint8_t* txptr, datasize_t txsize) :
  UdsServerBase(vec, txptr, txsize)
{
  EnableSID(SIDs::DSC, false, false, false, 2);
  EnableSID(SIDs::RDBI, false, true, false, 3);
  EnableSID(SIDs::RC, false, false, false, 3);

  SetSessionParam(SessParamType::S3_TIM, 5000);
  SetSessionParam(SessParamType::P2_TIM, 250);
  SetSessionParam(SessParamType::P2_ENHC, 5000);
}

