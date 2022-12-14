#pragma

#include <uds/session/uds-server-base.h>
#include <uds/isotp/docan-tp.h>
#include <example/can-bridge.h>

SocketCanSender& GetCanSender();
DoCAN_TP& GetDoCAN();
IProcessable& GetMainProcHandler();

void BuildApp();
