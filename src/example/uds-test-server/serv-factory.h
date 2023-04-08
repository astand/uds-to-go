#pragma once

#include <uds/session/uds-app-manager.h>
#include <uds/isotp/docan-tp.h>
#include <example/can-bridge.h>
#include <example/uds-test-server/cli-client.h>

SocketCanSender& GetCanSender();
UdsAppManager& GetBaseUdsServer();
DoCAN_TP& GetDoCAN();
IProcessable& GetMainProcHandler();
CliMen& GetClientUds();

void BuildApp();
