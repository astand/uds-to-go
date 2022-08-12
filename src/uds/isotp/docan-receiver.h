#pragma once

#include <timers/d-timer.h>
#include "../inc/iso-tp-if.h"
#include "../inc/iso-tp-types.h"

class IsoTp;

class DoCAN_Receiver {
 public:
  DoCAN_Receiver(uint8_t* mem, const size_t length, IsoTp& isotp) : rxbuff(mem), RXLEN(length), itp(isotp) {}
  void ProcessRx();
  void Receive(IsoSender& sender, const uint8_t* data, size_t candl);
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
    uint32_t respid{0};

    uint32_t rxsize{0};
    uint32_t passed{0};

    uint8_t expectsn{0};
    uint8_t currblkn{0};

  } RxDescriptor;

  RxDescriptor rxds{};

  uint8_t can_message[64u];

  IsoTp& itp;

  DTimers::Timer Cr_tim{1000, false, false};

};

