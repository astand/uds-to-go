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
#include <uds/isotp/iso-tp.h>
#include "can-bridge.h"
#include "iso-app.h"
#include "argcollector.h"

/* ---------------------------------------------------------------------------- */
template<typename T, size_t N>
class StaticMemAllocator {
 public:
  uint8_t* ptr() {
    return static_cast<uint8_t*>(__raw__);
  }

  StaticMemAllocator() = default;
  StaticMemAllocator(const StaticMemAllocator&) = delete;
  StaticMemAllocator& operator=(const StaticMemAllocator&) = delete;

 private:
  uint8_t __raw__[N * sizeof(T)] {0};
};

/* ---------------------------------------------------------------------------- */
constexpr size_t RxBufferSize = 8192;
constexpr size_t TxBufferSize = 8192;
std::string cmd;
std::mutex mtx;

// name of socketcan interface for ISO-TP communication test
static std::string ifname = "vcan0";

/* ---------------------------------------------------------------------------- */
static CanSender sender;
static IsoApp isoapp;
static IsoTpMem<RxBufferSize, TxBufferSize, StaticMemAllocator> isotpsource(sender, isoapp);
static IsoTp& iso_tp = isotpsource;
static CanListener listener(iso_tp);

static void simple_timer_process()
{
#if 0
  static auto first_stamp = std::chrono::steady_clock::now();
  static uint64_t ticked_us = 0u;

  auto now_stamp = std::chrono::steady_clock::now();
  auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(now_stamp - first_stamp).count();

  while (elapsed_us > 1000)
  {
    elapsed_us -= 1000;
    Timers::TickerCounter::ProcessTick();
    first_stamp += std::chrono::microseconds(1000);
  }

#else
  Timers::TickerCounter::ProcessTick();
#endif
}

static void try_to_set_param(const onepair& pair, uint32_t& vset)
{
  uint32_t scaned = 0u;
  std::string frmt = "%u";

  if (pair.second.size() > 2 && pair.second.at(1) == 'x')
  {
    frmt = "%x";
  }

  if (sscanf(pair.second.c_str(), frmt.c_str(), &scaned) != 1)
  {
    std::cout << "Wrong value (" << pair.second << ") for parameter '" << pair.first << "'";
  }
  else
  {
    vset = scaned;
  }
}

static void set_iso_tp(IsoTp& isotp, argsret& params)
{
  int scaned = 0;
  uint32_t phys_id = 0x700u;
  uint32_t resp_id = 0x701u;
  uint32_t func_id = 0x7dfu;
  uint32_t stmin = 0u;
  uint32_t blksize = 8u;

  for (size_t i = 0; i < params.size(); i++)
  {
    if (params[i].first.compare("-blksize") == 0)
    {
      try_to_set_param(params[i], blksize);
    }
    else if (params[i].first.compare("-phys") == 0)
    {
      try_to_set_param(params[i], phys_id);
    }
    else if (params[i].first.compare("-resp") == 0)
    {
      try_to_set_param(params[i], resp_id);
    }
    else if (params[i].first.compare("-func") == 0)
    {
      try_to_set_param(params[i], func_id);
    }
    else if (params[i].first.compare("-stmin") == 0)
    {
      try_to_set_param(params[i], stmin);
    }
    else if (params[i].first.compare("-iface") == 0)
    {
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

int main(int argc, char** argv)
{
  auto params = collectargs(argc, argv);

  iso_tp.SetParameter(ParName::CANDL, 8);
  iso_tp.SetParameter(ParName::PADD_BYTE, 0xcc);

  std::cout << "ISO Tp starting. Binding to '" << ifname << "'" << std::endl;

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
  std::cout << " ----------------------------------------- " << std::endl;
  set_iso_tp(iso_tp, params);
  std::cout << " ----------------------------------------- " << std::endl << std::endl;

  listener.SetSocket(sockfd);
  sender.SetSocket(sockfd);

  std::array<uint8_t, TxBufferSize> buffer;

  for (size_t i = 0; i < buffer.size(); buffer[i] = static_cast<uint8_t>(i), i++);

  std::thread th1([&]()
  {
    std::string readcmd;
    std::cout << "Command read thread started... OK" << std::endl;
    std::cout << "To send ISO TP packet input number (payload size) and press Enter" << std::endl;
    std::cout << "If the target instance is available you will see the result in its output..." << std::endl;
    bool run = true;

    while (run)
    {
      std::cin >> readcmd;

      if (readcmd.size() > 0)
      {
        if (readcmd.at(0) == 'e')
        {
          // return from thread (exit)
          run = false;
        }

        std::lock_guard<std::mutex> guard(mtx);
        cmd = readcmd;
      }
    }
  });

  std::string readcmd;

  while (true)
  {
    listener.ProcessRx();
    iso_tp.Process();

    {
      std::lock_guard<std::mutex> guard(mtx);

      if (cmd.size() > 0)
      {
        readcmd = cmd;
        cmd.clear();
      }
    }

    if (readcmd.size() > 0)
    {
      if (readcmd.at(0) == 'e')
      {
        // exit
        break;
      }
      else
      {
        uint32_t sendsize = 0u;

        if (sscanf(readcmd.c_str(), "%u", &sendsize) == 1)
        {
          std::cout << " ---> SEND " << sendsize << " bytes payload" << std::endl;
          iso_tp.Request(buffer.data(), sendsize);
        }
      }

      readcmd.clear();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    simple_timer_process();
  }

  th1.join();
  std::cout << "Exit ... " << std::endl;
}
