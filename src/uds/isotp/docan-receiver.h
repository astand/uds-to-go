#pragma once

#include <timers/d-timer.h>
#include "../inc/iso-tp-types.h"
#include "../inc/iso-tp-const.h"

class DoCAN_TP;

class DoCAN_Receiver {
 public:
  DoCAN_Receiver(uint8_t* mem, const size_t length, DoCAN_TP& isotp) : rxbuff(mem), RXLEN(length), itp(isotp) {}
  void ProcessRx();
  void Receive(const uint8_t* data, datasize_t candl);
  ParChangeResult SetParameter(ParName name, uint32_t v);

  bool IsBusy() const {
    return rxds.state != RxState::IDLE;
  }

 private:
  uint8_t* const rxbuff;
  const size_t RXLEN;

  enum class RxState
  {
    IDLE, ACTIVE
  };

  typedef struct
  {
    RxState state{RxState::IDLE};
    uint8_t blksize{16};
    uint8_t stmin{0};

    datasize_t rxsize{0};
    datasize_t passed{0};

    uint8_t expectsn{0};
    uint8_t currblkn{0};
  } RxDescriptor;

  RxDescriptor rxds{};

  uint8_t can_message[MAX_CANDL];

  DoCAN_TP& itp;

  DTimers::Timer Cr_tim{1000, false, false};

};

