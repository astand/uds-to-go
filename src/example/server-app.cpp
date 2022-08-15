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
#include "can-bridge.h"
#include "iso-app.h"
#include "argcollector.h"
#include "proc-runner.h"
#include "ticker-wrapper.h"
#include "test-siclient.h"
#include <uds/session/uds-server-base.h>
#include <uds/session/uds-service-handler.h>

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

static MemKeeper<UdsServiceHandler, 4> sicl_keeper;
static UdsServerBase sirout(sicl_keeper);
static DoCAN_TP_Mem<RxBufferSize, TxBufferSize, StaticMemAllocator> isotpsource(sender, sirout);
static TestUdsServiceHandler testclient(sirout);

static DoCAN_TP& iso_tp = isotpsource;
static CanListener listener(iso_tp);
static ProcRunner<4> procrunner;

bool set_byte(char c, uint8_t& byte, bool is_high)
{
  uint8_t value = 0u;

  if (c >= '0' && c <= '9')
  {
    value = c - '0';
  }
  else if (c >= 'a' && c <= 'f')
  {
    value = (c - 'a') + 10;
  }
  else if (c >= 'A' && c <= 'F')
  {
    value = (c - 'A') + 10;
  }
  else
  {
    return false;
  }

  if (is_high)
  {
    byte = value << 4;
  }
  else
  {
    byte |= value & 0x0fu;
  }

  return true;
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

static void set_do_can_parameters(DoCAN_TP& isotp, argsret& params)
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
  set_do_can_parameters(iso_tp, params);
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

  static TickerWrapper ticker;

  sirout.SetIsoTp(&iso_tp);

  procrunner.Add(&ticker);
  procrunner.Add(&iso_tp);
  procrunner.Add(&listener);
  procrunner.Add(&sirout);

  std::string readcmd;

  while (true)
  {
    procrunner.RunAllProcess();

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
        std::vector<uint8_t> payload;
        uint8_t byte;


        for (size_t i = 0; i < readcmd.size(); i++)
        {
          if (set_byte(readcmd.at(i), byte, i % 2 == 0) == false)
          {
            payload.clear();
            break;
          }

          if (i % 2 == 1)
          {
            payload.push_back(byte);
          }
        }

        iso_tp.Request(payload.data(), payload.size());
      }

      readcmd.clear();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  th1.join();
  std::cout << "Exit ... " << std::endl;
}
