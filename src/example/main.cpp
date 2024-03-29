#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <array>
#include <thread>
#include <string.h>
#include <chrono>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <mutex>
#include <uds/isotp/docan-tp.h>
#include <etc/helpers/static-allocator.h>
#include "can-bridge.h"
#include "argcollector.h"
#include "iso-tp-server/serv-factory.h"
#include "app-helper.h"

/* ---------------------------------------------------------------------------- */
constexpr size_t RxBufferSize = 8192;
constexpr size_t TxBufferSize = 8192;
std::string cmd;
std::mutex mtx;

// name of socketcan interface for ISO-TP communication test
static std::string ifname = "vcan0";

/* ---------------------------------------------------------------------------- */
static DoCAN_TP& iso_tp = GetDoCAN();
static SocketCanReader listener(iso_tp);

static void set_do_can_parameters(DoCAN_TP&, argsret& params) {

  uint32_t phys_id = 0x700u;
  uint32_t resp_id = 0x701u;
  uint32_t func_id = 0x7dfu;
  uint32_t stmin = 0u;
  uint32_t blksize = 8u;

  for (size_t i = 0; i < params.size(); i++) {
    if (params[i].first.compare("-blksize") == 0) {
      try_to_set_param(params[i], blksize);
    } else if (params[i].first.compare("-phys") == 0) {
      try_to_set_param(params[i], phys_id);
    } else if (params[i].first.compare("-resp") == 0) {
      try_to_set_param(params[i], resp_id);
    } else if (params[i].first.compare("-func") == 0) {
      try_to_set_param(params[i], func_id);
    } else if (params[i].first.compare("-stmin") == 0) {
      try_to_set_param(params[i], stmin);
    } else if (params[i].first.compare("-iface") == 0) {
      ifname = params[i].second;
    }
  }

  std::cout << "Init iso tp parameters:" << std::endl;

  std::cout << "BLKSIZE = " << (int)blksize << std::endl;
  iso_tp.SetParameter(ParName::BLKSZ, blksize);

  std::cout << "STMIN   = " << (int)stmin << std::endl;
  iso_tp.SetParameter(ParName::ST_MIN, stmin);

  std::cout << std::hex;
  std::cout << "PHYS    = " << (int)phys_id << std::endl;
  iso_tp.SetParameter(ParName::PHYS_ADDR, phys_id);

  std::cout << "RESP    = " << (int)resp_id << std::endl;
  iso_tp.SetParameter(ParName::RESP_ADDR, resp_id);

  std::cout << "FUNC    = " << (int)func_id << std::endl;
  std::cout << std::dec;

  iso_tp.SetParameter(ParName::FUNC_ADDR, func_id);
}

int main(int argc, char** argv) {

  auto params = collectargs(argc, argv);

  iso_tp.SetParameter(ParName::CANDL, 8);
  iso_tp.SetParameter(ParName::PADD_BYTE, 0xcc);

  std::cout << "ISO Tp starting. Binding to '" << ifname << "'" << std::endl;
  std::cout << " ----------------------------------------- " << std::endl;
  set_do_can_parameters(iso_tp, params);
  std::cout << " ----------------------------------------- " << std::endl << std::endl;
  auto sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  assert(sockfd > 0);

  struct ifreq can_iface;
  strcpy(can_iface.ifr_name, ifname.c_str());

  assert(ioctl(sockfd, SIOCGIFINDEX, &can_iface) >= 0);

  struct sockaddr_can loc_addr;
  bzero(&loc_addr, sizeof(loc_addr));
  loc_addr.can_ifindex = can_iface.ifr_ifindex;
  loc_addr.can_family = AF_CAN;

  /* Check MTU of interface */
  assert(ioctl(sockfd, SIOCGIFMTU, &can_iface) >= 0);
  assert(fcntl(sockfd, F_SETFL, (fcntl(sockfd, F_GETFL) | O_NONBLOCK)) >= 0);
  assert(bind(sockfd, (struct sockaddr*)&loc_addr, sizeof(loc_addr)) >= 0);

  std::cout << "Started succesfully." << std::endl;
  std::cout << " ----------------------------------------- " << std::endl << std::endl;

  listener.SetSocket(sockfd);
  GetCanSender().SetSocket(sockfd);

  std::array<uint8_t, TxBufferSize> buffer;

  for (size_t i = 0; i < buffer.size(); buffer[i] = static_cast<uint8_t>(i), i++);

  std::thread th1([&]() {
    std::string readcmd;
    std::cout << "Command read thread started... OK" << std::endl;
    std::cout << "To send ISO TP packet input number (payload size) and press Enter" << std::endl;
    std::cout << "If the target instance is available you will see the result in its output..." << std::endl;
    bool run = true;

    while (run) {
      std::cin >> readcmd;

      if (readcmd.size() > 0) {
        if (readcmd.at(0) == 'e') {
          // return from thread (exit)
          run = false;
        }

        std::lock_guard<std::mutex> guard(mtx);
        cmd = readcmd;
      }
    }
  });

  BuildApp();

  std::string readcmd;

  while (true) {
    GetMainProcHandler().Process();
    listener.Process();

    {
      std::lock_guard<std::mutex> guard(mtx);

      if (cmd.size() > 0) {
        readcmd = cmd;
        cmd.clear();
      }
    }

    if (readcmd.size() > 0) {
      if (readcmd.at(0) == 'e') {
        // exit
        break;
      } else {
        uint32_t sendsize = 0u;

        if (sscanf(readcmd.c_str(), "%u", &sendsize) == 1) {
          std::cout << " ---> SEND " << sendsize << " bytes payload" << std::endl;
          iso_tp.Request(buffer.data(), sendsize);
        }
      }

      readcmd.clear();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  th1.join();
  std::cout << "Exit ... " << std::endl;
}
