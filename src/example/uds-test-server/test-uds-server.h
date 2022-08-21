#pragma once

#include <uds/session/uds-server-base.h>

class TestUdsServer : public UdsServerBase {
 public:
  TestUdsServer(IKeeper<UdsServiceHandler>& vec, uint8_t* txptr, datasize_t txsize);

};
