#include "serv-factory.h"
#include "../uds-test-server/test-uds-server.h"
#include <etc/helpers/static-allocator.h>
#include <example/proc-runner.h>
#include <example/ticker-wrapper.h>
#include <example/can-bridge.h>
#include <example/uds-test-server/test-uds-server.h>
#include <example/uds-test-server/session-client.h>
#include <example/uds-test-server/test-did-reader.h>
#include <example/test-siclient.h>
#include <uds/session/apps/rctrl-router.h>
#include <uds/session/apps/did-router.h>
#include <uds/session/apps/did-keeper.h>

constexpr size_t RxBufferSize = 8192;
constexpr size_t TxBufferSize = 8192;

SocketCanSender& GetCanSender()
{
  static SocketCanSender sender;
  return sender;
}

UdsServerBase& GetBaseUdsServer()
{
  constexpr size_t MAX_ARRAY = 1024u;
  static uint8_t serv_array[MAX_ARRAY] {0};
  static MemKeeper<UdsServiceHandler, 4> si_client_keeper;
  static TestUdsServer si_router(si_client_keeper, serv_array, MAX_ARRAY);

  return si_router;
}

DoCAN_TP& GetDoCAN()
{
  static DoCAN_TP_Mem<RxBufferSize, TxBufferSize, StaticMemAllocator> isotpsource(GetCanSender(), GetBaseUdsServer());

  return isotpsource;
}

static ProcRunner<4>& GetProcRunner()
{
  static ProcRunner<4> procrunner;
  return procrunner;
}

IProcessable& GetMainProcHandler()
{
  return GetProcRunner();
}

void BuildApp()
{
  static TickerWrapper ticker;
  static TestUdsServiceHandler testclient(GetBaseUdsServer());
  static DSCClient dschandler(GetBaseUdsServer());
  static DidKeeper<4> didkeeper;
  static TestDidReader didreader;
  static DidRouter didrouter(GetBaseUdsServer(), didkeeper);

  GetProcRunner().Add(&ticker);
  GetProcRunner().Add(&GetDoCAN());
  GetProcRunner().Add(&GetBaseUdsServer());
  GetBaseUdsServer().SetIsoTp(&GetDoCAN());

  didkeeper.Add(&didreader);
}
