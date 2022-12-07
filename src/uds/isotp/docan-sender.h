#pragma once

#include <timers/d-timer.h>
#include "../inc/iso-tp-types.h"
#include "../inc/iso-tp-const.h"

class DoCAN_TP;

class DoCAN_Sender {
 public:
  /// @brief DoCAN sender constructor
  /// @param mem pointer to buffer for transmit message
  /// @param bufcap transmit buffer length
  /// @param isotp reference to DoCAN iso tp object
  DoCAN_Sender(uint8_t* mem, const size_t bufcap, DoCAN_TP& isotp) : txbuff(mem), TXLEN(bufcap), itp(isotp) {}

  /// @brief General transmitter handler
  void ProcessTx();

  /// @brief FlowControl frame handler
  /// @param flow_status flow status from FC payload
  /// @param blks block size from FC payload
  /// @param stm STmin from FC payload
  void OnFlowControl(uint8_t flow_status, uint8_t blks, uint8_t stm);

  /// @brief Payload sender
  /// @param data pointer to ISO TP packet
  /// @param len length of ISO TP packet
  /// @return call result
  IsoTpResult Send(const uint8_t* data, datasize_t len);

  /// @brief DoCAN sender parameter setter
  /// @param name parameter name
  /// @param v parameter value
  /// @return parameter set result
  SetParamResult SetParameter(ParName name, uint32_t v);

  /// @brief Is DoCAN sender is busy
  /// @return true - if busy
  bool IsBusy() const {
    return (txds.state != DtState::IDLE);
  }

 private:
  /// @brief Checks if datasize fits to transmit buffer
  /// @param datasize size of data
  /// @return result of data size check
  IsoTpResult CheckTxValid(datasize_t datasize);

  /// @brief pointer to general transmit buffer (from outside)
  uint8_t* const txbuff;

  /// @brief length of general transmit buffer
  const size_t TXLEN;

  /// @brief host DoCAN object (DoCAN_TP)
  DoCAN_TP& itp;

  /// @brief DoCAN sender states
  enum class DtState
  {
    IDLE, SF_DT, MF_DT, PAUSE, WAIT,
  };

  struct TxDescriptor
  {
    /// @brief general sender state
    DtState state {DtState::IDLE};
    /// @brief control block: previously passed size
    datasize_t passed{0};
    /// @brief control block: the whole transmitted size
    datasize_t size{0};

    /// @brief flow control: serial number
    uint8_t sn{0};
    // @currblknum - current number of sent CF

    /// @brief flow control: number of correctly received blocks
    uint8_t currblknum{0};

    /// @brief flow control: whole number of blocks for current tx segment
    uint8_t segblkcount{25};

    /// @brief flow control: STmin setting for current tx segment
    uint8_t stmin{0};
  };

  /// @brief general flow control descriptor
  TxDescriptor txds{};

  /// @brief can message buffer
  uint8_t can_message[MAX_CANDL] {0};

  DTimers::Timer N_As_tim{1000, false, false};
  DTimers::Timer N_Bs_tim{1000, false, false};
  // STmin is Cs timer
  DTimers::Timer STmin_tim{0, false, false};

  /// @brief indicates if last part of payload was sent by previous attempt
  bool prev_data_sent{true};

};

