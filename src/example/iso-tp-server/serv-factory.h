#pragma once

#include <uds/session/uds-app-manager.h>
#include <uds/isotp/docan-tp.h>
#include <example/can-bridge.h>

SocketCanSender& GetCanSender();
DoCAN_TP& GetDoCAN();
IProcessable& GetMainProcHandler();

void BuildApp();
