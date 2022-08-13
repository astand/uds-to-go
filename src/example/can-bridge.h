#pragma once

#include <assert.h>
#include <stdlib.h>
#include <uds/isotp/iso-tp.h>
#include <linux/can/raw.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/if.h>

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
