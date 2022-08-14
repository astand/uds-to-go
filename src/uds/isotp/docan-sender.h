#pragma once

#include <timers/d-timer.h>
#include "../inc/iso-tp-types.h"
#include "../inc/iso-tp-const.h"

class DoCAN_TP;

class DoCAN_Sender {
 public:
  DoCAN_Sender(uint8_t* mem, const size_t length, DoCAN_TP& isotp) : txbuff(mem), TXLEN(length), itp(isotp) {}

  void ProcessTx();
  void OnFlowControl(uint8_t flow_statue, uint8_t blks, uint8_t stm);
  IsoTpResult Send(const uint8_t* data, datasize_t len);
  ParChangeResult SetParameter(ParName name, uint32_t v);

  bool IsBusy() const {
    return (txds.state != DtState::IDLE);
  }

 private:
  IsoTpResult CheckTxValid(datasize_t l);

  DTimers::Timer N_As_tim{1000, false, false};
  DTimers::Timer N_Bs_tim{1000, false, false};
  DTimers::Timer N_Cs_tim{1000, false, false};
  DTimers::Timer STmin_tim{0, false, false};

  uint8_t can_message[MAX_CANDL] {0};
  uint8_t* const txbuff;
  const size_t TXLEN;

  enum class DtState
  {
    IDLE,
    SF_DT,
    MF_DT,
    PAUSE,
    WAIT,
  };

  typedef struct
  {
    DtState state {DtState::IDLE};
    // control block
    datasize_t passed{0};
    datasize_t size{0};
    uint8_t sn{0};
    uint8_t currblksize{0};
    // flow control config from this (sender) side
    uint8_t blksize{25};
    uint8_t stmin{0};
  } TxDescriptor;

  TxDescriptor txds{};

  bool last_send_ok{true};

  DoCAN_TP& itp;
};

