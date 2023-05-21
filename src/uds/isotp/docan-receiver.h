#pragma once

#include <timers/d-timer.h>
#include "../inc/iso-tp-types.h"
#include "../inc/iso-tp-const.h"

class DoCAN_TP;

/// @brief DoCAN receiver class
class DoCAN_Receiver {

 public:
  /// @brief DoCAN receiver constructor
  /// @param mem pointer to buffer for income paylaod
  /// @param bufcap buffer maximum
  /// @param isotp reference to iso-tp host object
  DoCAN_Receiver(uint8_t* mem, const size_t bufcap, DoCAN_TP& isotp) : rxbuff(mem), RXLEN(bufcap), itp(isotp) {}

  /// @brief General receiver handler
  void ProcessRx();

  /// @brief DoCAN single payload receiver
  /// @param data pointer to data from CAN
  /// @param candl length of CAN data
  void Receive(const uint8_t* data, datasize_t candl);

  /// @brief Sets receiver params
  /// @param name name of parameter to set
  /// @param v parameter value
  /// @return status of setting parameter
  SetParamResult SetParameter(ParName name, uint32_t v);

  /// @brief Checks if receiver is busy
  /// @return busy status
  bool IsBusy() const {
    return rxds.state != RxState::IDLE;
  }

 private:
  /// @brief internal pointer to rx buffer
  uint8_t* const rxbuff;
  /// @brief rx buffer capacity
  const size_t RXLEN;

  /// @brief DoCAN host reference
  DoCAN_TP& itp;

  enum class RxState {
    IDLE, ACTIVE
  };

  struct RxDescriptor {
    RxState state{RxState::IDLE};

    datasize_t rxsize{0};
    datasize_t passed{0};

    uint8_t expectsn{0};
    uint8_t currblkn{0};
    uint8_t blksize{16};
    uint8_t stmin{0};
  };

  /// @brief internal state descriptor
  RxDescriptor rxds{};

  /// @brief can message buffer
  uint8_t can_message[MAX_CANDL];

  /// @brief Cr timer
  DTimers::Timer Cr_tim{1000, false, false};

};

