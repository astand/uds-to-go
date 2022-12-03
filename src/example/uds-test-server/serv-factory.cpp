#include "serv-factory.h"
#include "../uds-test-server/test-uds-server.h"
#include <etc/helpers/static-allocator.h>
#include <example/proc-runner.h>
#include <example/ticker-wrapper.h>
#include <example/can-bridge.h>
#include <example/uds-test-server/test-uds-server.h>
#include <example/uds-test-server/session-client.h>
#include <example/uds-test-server/test-did-reader.h>
#include <example/uds-test-server/test-routine-server.h>
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
  static uint32_t count = 0u;
  static TickerWrapper ticker;
  static TestUdsServiceHandler testclient(GetBaseUdsServer());
  static DSCClient dschandler(GetBaseUdsServer());
  static DidKeeper<4> didkeeper;
  static TestDidReader didreader;
  static DidRouter didrouter(GetBaseUdsServer(), didkeeper);

  static MultiRoutineHandler<4> rkeeper;
  static RoutineRouter baser(GetBaseUdsServer(), rkeeper);
  static RotineServ1 r1(baser);
  static RotineServ2 r2(baser);

  assert(count++ == 0);

  rkeeper.Add(&r1);
  rkeeper.Add(&r2);

  GetProcRunner().Add(&ticker);
  GetProcRunner().Add(&GetDoCAN());
  GetProcRunner().Add(&GetBaseUdsServer());
  GetProcRunner().Add(&r1);
  GetBaseUdsServer().SetIsoTp(&GetDoCAN());

  didkeeper.Add(&didreader);
}

CliMen& GetClientUds()
{
  static Menu sessctrl = Menu("Session Control");
  static Menu readdid = Menu("Read DID");
  static Menu reqroutine = Menu("Routine request");
  static Menu defsess = Menu("Default");
  static Menu extsess = Menu("Extended");
  static Menu prgsess = Menu("Programming");
  static Menu read22 = Menu("Read 22");
  static Menu tester = Menu("Tester Present");

  static Menu routine1 = Menu("Routine #1 (0x0102)");
  static Menu routine2 = Menu("Routine #2 (0xff00)");

  defsess.cmd = { 0x10, 0x01 };
  extsess.cmd = { 0x10, 0x03 };
  prgsess.cmd = { 0x10, 0x02 };

  read22.cmd = { 0x22, 0x22, 0x22 };
  tester.cmd = { 0x3e, 0x00};

  routine1.cmd = { 0x31, 0x01, 0x01, 0x02, 1 };
  routine2.cmd = { 0x31, 0x01, 0xff, 0x00, 1 };

  sessctrl.SetNext(&readdid)->SetNext(&tester)->SetNext(&reqroutine);
  sessctrl.SetDown(&defsess);

  defsess.SetNext(&extsess);
  extsess.SetNext(&prgsess);

  readdid.SetDown(&read22);
  readdid.SetNext(&tester);

  reqroutine.SetDown(&routine1);
  routine1.SetNext(&routine2);

  static CliMen client(&sessctrl);
  return client;
}
