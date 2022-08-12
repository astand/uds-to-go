#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <array>
#include <thread>
#include <string.h>
#include <unistd.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <mutex>
#include <uds/isotp/iso-tp.h>

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

class IsoClient : public IsoTpClient {
 public:
  void OnIsoEvent(N_Type t, N_Result res, const IsoTpInfo& inf) {
    assert((uint8_t)t < 3 && (uint8_t)res < 13);

    std::cout << "Event [" << type_names[(uint8_t)t] << "] Status [" << res_names[(uint8_t)res] << "]";

    if (t == N_Type::Data && res == N_Result::OK_r) {
      // print data
      std::cout << std::endl << "------------------------------------------------" << std::endl;
      //           "00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff ";
      assert(inf.data != nullptr && inf.length != 0u);

      std::cout << std::hex;

      for (size_t i = 0; i < inf.length; i++) {
        if (i != 0 && (i & 0x0f) == 0u) {
          std::cout << std::endl;
        }

        if (inf.data[i] < 0x10u) {
          std::cout << "0";
        }

        std::cout << (int)inf.data[i] << " ";
      }

      std::cout << std::endl << "------------------------------------------------";
      std::cout << std::dec;
    }

    std::cout << std::endl;
  }

 private:
  const char* type_names[3] =
  {
    "Conf  ",
    "Data  ",
    "DataFF"
  };

  const char* res_names[13] =
  {
    "OK_s",
    "OK_r",
    "TIMEOUT_As",
    "TIMEOUT_Ar",
    "TIMEOUT_Bs",
    "TIMEOUT_Cr",
    "WRONG_SN",
    "INVALID_FS",
    "UNEXP_PDU",
    "WFT_OVRN",
    "BUFFER_OVFLW",
    "ERROR_s",
    "ERROR_r"
  };
};

class CanSender : public IsoSender {
 public:
  size_t SendFrame(const uint8_t* data, size_t length, uint32_t msgid) {
    assert(length <= 8);
    assert(txsocket != 0);

    struct can_frame frame;
    frame.can_id = msgid;
    frame.can_dlc = 8;
    memcpy(frame.data, data, length);
    assert(write(txsocket, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame) > 0);
    return length;
  }

  void SetSocket(int s) {
    assert(s != 0);
    txsocket = s;
  }

 private:
  int txsocket{0};
};

class CanListener {
 public:
  CanListener(IsoListener& receiver) : isoreceiver(receiver) {
    select_to.tv_sec = 0u;
    select_to.tv_usec = 0u;
  }

  void SetSocket(int s) {
    assert(s != 0);
    rxsock = s;
  }

  void ProcessRx() {
    assert(rxsock != 0);
    struct canfd_frame read_frame;

    /* Prepare readfds */
    FD_ZERO(&readfds);
    FD_SET(rxsock, &readfds);
    // read all fds with no blocking timeout
    int ret = select(rxsock + 1, &readfds, NULL, NULL, &select_to);
    assert(ret >= 0);

    if (FD_ISSET(rxsock, &readfds)) {
      // read data from vcans
      do {
        auto recv_bytes = recv(rxsock, &read_frame, sizeof(struct canfd_frame), 0);

        if (recv_bytes < 0) {
          if (errno != EWOULDBLOCK) {
            std::cout << "CAN read error: " << recv_bytes << ". errno = " << errno << std::endl;
          }

          break;
        }
        else if (recv_bytes == CAN_MTU) {
          isoreceiver.ReadFrame(read_frame.data, 8, read_frame.can_id);
        }
        else if (recv_bytes == 0) {
          break;
        }
        else {
          std::cout << "Unexpected length: " << recv_bytes << std::endl;
        }
      }
      while (true);
    }
  }

 private:
  IsoListener& isoreceiver;
  int rxsock{0};
  timeval select_to;
  fd_set readfds;
};

constexpr size_t RxBufferSize = 4096;
constexpr size_t TxBufferSize = 4096;
std::string cmd;
std::mutex mtx;

/* ---------------------------------------------------------------------------- */
static CanSender sender;
static IsoClient isoclient;
static IsoTpMem<RxBufferSize, TxBufferSize, StaticMemAllocator> isotpsource(sender, isoclient);
static IsoTp& iso_tp = isotpsource;
static CanListener listener(iso_tp);

static uint32_t phys_id = 0x700u;
static uint32_t resp_id = 0x701u;

int main()
{
  iso_tp.SetParameter(ParName::BLKSZ, 8);
  iso_tp.SetParameter(ParName::CANDL, 8);
  iso_tp.SetParameter(ParName::ST_MIN, 50);
  iso_tp.SetParameter(ParName::PHYS_ADDR, phys_id);
  iso_tp.SetParameter(ParName::FUNC_ADDR, 0x7df);
  iso_tp.SetParameter(ParName::RESP_ADDR, resp_id);

  std::cout << "Hello uds!" << std::endl;

  auto sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  assert(sockfd > 0);

  struct ifreq can_iface;
  strcpy(can_iface.ifr_name, "vcan0");

  assert(ioctl(sockfd, SIOCGIFINDEX, &can_iface) >= 0);

  struct sockaddr_can loc_addr;
  bzero(&loc_addr, sizeof(loc_addr));
  loc_addr.can_ifindex = can_iface.ifr_ifindex;
  loc_addr.can_family = AF_CAN;

  /* Check MTU of interface */
  assert(ioctl(sockfd, SIOCGIFMTU, &can_iface) >= 0);
  assert(fcntl(sockfd, F_SETFL, (fcntl(sockfd, F_GETFL) | O_NONBLOCK)) >= 0);
  assert(bind(sockfd, (struct sockaddr*)&loc_addr, sizeof(loc_addr)) >= 0);

  listener.SetSocket(sockfd);
  sender.SetSocket(sockfd);

  std::thread th1([&]()
  {
    std::string readcmd;
    std::cout << "Read command thread started..." << std::endl;
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

  std::array<uint8_t, 256> buffer {0};

  for (size_t i = 0; i < buffer.size(); buffer[i] = (i + 1), i++);

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
      else if (readcmd.at(0) == 'd')
      {
        iso_tp.Request(buffer.data(), 100);
      }
      else
      {
        iso_tp.Request(buffer.data(), 7);
      }

      readcmd.clear();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    Timers::TickerCounter::ProcessTick();
  }

  th1.join();
  std::cout << "Exit ... " << std::endl;
}
