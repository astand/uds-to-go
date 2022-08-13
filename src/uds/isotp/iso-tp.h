#pragma once

#include "timers/d-timer.h"
#include "../inc/iso-tp-if.h"
#include "docan-sender.h"
#include "docan-receiver.h"

class IsoTp : public IsoListener, public IsoTpHost {
 public:
  IsoTp(uint8_t* memrx, size_t lengthrx, uint8_t* memtx, size_t lengthtx, IsoSender& sender, IsoTpClient& client) :
    iso_sender(memtx, lengthtx, *this),
    iso_receiver(memrx, lengthrx, *this),
    can_sender(sender),
    iso_client(client)
  {}

  virtual void Process();
  virtual void ReadFrame(const uint8_t* data, size_t length, uint32_t msgid) override;
  virtual IsoTpResult Request(const uint8_t* data, size_t length) override;


  void OnIsoRxEvent(N_Type event, N_Result result, const uint8_t* data = nullptr, size_t length = 0);
  void OnIsoTxEvent(N_Type event, N_Result result);
  ParChangeResult SetParameter(ParName name, uint32_t value);

  typedef struct
  {
    uint32_t phys_id;
    uint32_t func_id;
    uint32_t resp_id;
    uint32_t candl;
    uint8_t padding;
  } DoCAN_Config_t;

  const DoCAN_Config_t& Config() const {
    return docan_config;
  }

 private:
  DoCAN_Sender iso_sender;
  DoCAN_Receiver iso_receiver;

  IsoSender& can_sender;
  IsoTpClient& iso_client;

  IsoTpClient::IsoTpInfo paydsc{};

  DoCAN_Config_t docan_config{0};

  IsoTpResult state {IsoTpResult::WRONG_STATE};
};

template<size_t Rx, size_t Tx, template<typename, size_t> class Memgiver>
class IsoTpMem : public IsoTp {
 public:
  IsoTpMem(IsoSender& s, IsoTpClient& c) : IsoTp(rxalloc.ptr(), Rx, rxalloc.ptr(), Tx, s, c) {}

 private:
  Memgiver<uint8_t, Rx> rxalloc;
  Memgiver<uint8_t, Tx> txalloc;

};
