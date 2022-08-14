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

  uint8_t* const txbuff;
  const size_t TXLEN;

  // reference to HOST DoCAN (DoCAN_TP)
  DoCAN_TP& itp;

  enum class DtState
  {
    IDLE, SF_DT, MF_DT, PAUSE, WAIT,
  };

  typedef struct
  {
    DtState state {DtState::IDLE};
    // control block
    datasize_t passed{0};
    datasize_t size{0};
    // flow control information block
    // @sn - serial number for the next CF
    uint8_t sn{0};
    // @currblksize - current number of sent CF
    uint8_t currblksize{0};
    // @blksize - number of CF for the next FC confirmation
    // from receiver side
    uint8_t blksize{25};
    // @stmin - timeout between two CF which is required by
    // receiver side
    uint8_t stmin{0};
  } TxDescriptor;

  TxDescriptor txds{};

  // DoCAN will put padding byte, so the size
  // of this array must be minimum 64 bytes (MAX_CANDL)
  uint8_t can_message[MAX_CANDL] {0};

  DTimers::Timer N_As_tim{1000, false, false};
  DTimers::Timer N_Bs_tim{1000, false, false};
  DTimers::Timer N_Cs_tim{1000, false, false};
  DTimers::Timer STmin_tim{0, false, false};

  bool last_send_ok{true};

};

