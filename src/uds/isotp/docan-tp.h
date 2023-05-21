#pragma once

#include <timers/d-timer.h>
#include <helpers/IProcessable.h>
#include "../inc/iso-tp-if.h"
#include "docan-sender.h"
#include "docan-receiver.h"

class DoCAN_TP : public ICAN_Listener, public IsoTpImpl, public IProcessable {

 public:
  DoCAN_TP(uint8_t* memrx, size_t lengthrx, uint8_t* memtx, size_t lengthtx, ICAN_Sender& sender, IsoTpClient& client) :
    iso_sender(memtx, lengthtx, *this),
    iso_receiver(memrx, lengthrx, *this),
    can_sender(sender),
    iso_client(client)
  {}

  /// @brief Periodic processing
  virtual void Process() override;

  /// @brief Function is called on each new CAN frame
  /// @param data Pointer to CAN frame data
  /// @param length CAN frame payload length
  /// @param msgid CAN message ID
  virtual void ReadFrame(const uint8_t* data, size_t length, uint32_t msgid) override;

  /// @brief Sencs CAN frame payload on bus
  /// @param data Pointer to payload data
  /// @param length Payload length
  /// @return Sending result
  virtual IsoTpResult Request(const uint8_t* data, size_t length) override;

  /// @brief Puts padding byte and sends CAN payload
  /// @param data Pointer to payload to be sent
  /// @param len Usefull part of payload length
  /// @return CAN frame payload length
  size_t PduToCan(uint8_t* data, datasize_t len) {
    // WARNING: data has to have enough space for padding bytes,
    while (len < configDoCAN.candl) {
      data[len] = configDoCAN.padding;
      ++len;
    }

    return can_sender.SendFrame(data, configDoCAN.candl, configDoCAN.resp_id);
  }

  /// @brief ISO TP receive event handler
  /// @param event Type of receiving event
  /// @param result Event result (status)
  /// @param data Pointer to the income payload
  /// @param length Length of the income payload
  void OnIsoRxEvent(N_Event event, N_Result result, const uint8_t* data = nullptr, size_t length = 0);

  /// @brief ISO TP transmitt event handler
  /// @param event Type of transmitting event
  /// @param result Event result (status)
  void OnIsoTxEvent(N_Event event, N_Result result);

  /// @brief Sets ISO TP parameter
  /// @param name Parameter name
  /// @param value Parameter value
  /// @return Setting result
  SetParamResult SetParameter(ParName name, uint32_t value);

  /// @brief DoCAN instance static configuration
  typedef struct {
    /// @brief CAN message Id for the physically addressed CAN requests (income)
    uint32_t phys_id;
    /// @brief CAN message Id for the functionally addressed CAN requests (income)
    uint32_t func_id;
    /// @brief CAN message Id for the response messages (outcome)
    uint32_t resp_id;
    /// @brief CAN message payload capacity
    datasize_t candl;
    /// @brief CAN message outcome payload padding byte
    uint8_t padding;
  } configDoCAN_t;

  /// @brief DoCAN static configuration reading export
  /// @return Reference to the DoCAN configuration
  const configDoCAN_t& Config() const {
    return configDoCAN;
  }

 private:
  /// @brief Instance of ISO TP DoCAN sender
  DoCAN_Sender iso_sender;
  /// @brief Instance of ISO TP DoCAN receiver
  DoCAN_Receiver iso_receiver;

  /// @brief Reference to the physical CAN message sender
  ICAN_Sender& can_sender;
  /// @brief Reference to the ISO TP client (application level instance)
  IsoTpClient& iso_client;

  /// @brief PDU context info (data, length etc)
  IsoTpClient::IsoTpInfo pduContext{};

  /// @brief ISO TP DoCAN configuration
  configDoCAN_t configDoCAN{};
};

template<size_t Rx, size_t Tx, template<typename, size_t> class Memgiver>
class DoCAN_TP_Mem : public DoCAN_TP {

 public:
  DoCAN_TP_Mem(ICAN_Sender& s, IsoTpClient& c) : DoCAN_TP(rxalloc.ptr(), Rx, rxalloc.ptr(), Tx, s, c) {}

 private:
  Memgiver<uint8_t, Rx> rxalloc;
  Memgiver<uint8_t, Tx> txalloc;

};
