#pragma once

#include <uds/session/uds-server-base.h>
#include <uds/isotp/docan-tp.h>
#include <example/can-bridge.h>
#include <example/uds-test-server/cli-client.h>

SocketCanSender& GetCanSender();
UdsServerBase& GetBaseUdsServer();
DoCAN_TP& GetDoCAN();
IProcessable& GetMainProcHandler();
CliMen& GetClientUds();

void BuildApp();
