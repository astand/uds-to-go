#include "serv-factory.h"
#include "iso-app.h"
#include <etc/helpers/static-allocator.h>
#include <example/proc-runner.h>
#include <example/ticker-wrapper.h>
#include <example/can-bridge.h>
#include <example/uds-test-server/session-client.h>
#include <example/uds-test-server/test-did-reader.h>
#include <example/test-siclient.h>
#include <uds/session/apps/rctrl-router.h>
#include <uds/session/apps/did-router.h>
#include <uds/session/apps/did-keeper.h>

constexpr size_t RxBufferSize = 8192;
constexpr size_t TxBufferSize = 8192;

SocketCanSender& GetCanSender() {

  static SocketCanSender sender;
  return sender;
}

DoCAN_TP& GetDoCAN() {

  static IsoApp isoapp;
  static DoCAN_TP_Mem<RxBufferSize, TxBufferSize, StaticMemAllocator> isotpsource(GetCanSender(), isoapp);

  return isotpsource;
}

static ProcRunner<4>& GetProcRunner() {

  static ProcRunner<4> procrunner;
  return procrunner;
}

IProcessable& GetMainProcHandler() {

  return GetProcRunner();
}

void BuildApp() {

  static TickerWrapper ticker;

  GetProcRunner().Add(&ticker);
  GetProcRunner().Add(&GetDoCAN());
}

