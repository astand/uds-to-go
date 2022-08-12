#pragma once

#include <timers/d-timer.h>
#include "../inc/iso-tp-if.h"
#include "../inc/iso-tp-types.h"

class IsoTp;

typedef struct
{
  uint32_t response_id;

  uint32_t As_tim;
  uint32_t Bs_tim;
  uint32_t Cs_tim;

  uint8_t padding_byte;
  uint32_t can_dl;
} ITP_SendConf_t;

class DoCAN_Sender {
 public:
  DoCAN_Sender(uint8_t* mem, const size_t length, IsoTp& isotp) : txbuff(mem), TXLEN(length), itp(isotp) {}

  void ProcessTx(IsoSender& sender);
  void OnFlowControl(uint8_t flow_statue, uint8_t blks, uint8_t stm);
  IsoTpResult Send(IsoSender& sender, const uint8_t* data, size_t len);
  ParChangeResult SetParameter(ParName name, uint32_t v);

  bool IsBusy() const {
    return (txds.state != DtState::IDLE);
  }

 private:

  IsoTpResult CheckTxValid(size_t l);

  DTimers::Timer N_As_tim{1000, false, false};
  DTimers::Timer N_Bs_tim{1000, false, false};
  DTimers::Timer N_Cs_tim{1000, false, false};
  DTimers::Timer STmin_tim{0, false, false};

  uint8_t can_message[(uint8_t)CanDl::CANDL_64] {0};
  uint8_t* const txbuff;
  const size_t TXLEN;
  ITP_SendConf_t itp_conf{};

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
    bool is_sf{false};
    DtState state {DtState::IDLE};
    size_t passed{0};
    size_t size{0};
    uint8_t sn{0};
    uint8_t blksize{0};
    uint8_t currblksize{0};
    uint8_t stmin{0};
  } TxDescriptor;

  TxDescriptor txds{};

  bool last_send_ok{true};

  IsoTp& itp;

  ITP_SendConf_t isotp_config{};
};

