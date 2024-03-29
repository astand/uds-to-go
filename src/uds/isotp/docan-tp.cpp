#include <string.h>
#include "pci-helper.h"
#include "docan-tp.h"


void DoCAN_TP::Process() {

  iso_sender.ProcessTx();
  iso_receiver.ProcessRx();
}

void DoCAN_TP::ReadFrame(const uint8_t* data, size_t length, uint32_t msgid) {

  bool is_phys = (msgid == configDoCAN.phys_id);
  bool is_func = (msgid == configDoCAN.func_id);

  DC_FrameType ptype = static_cast<DC_FrameType>(data[0] & 0xf0u);

  // request with functional address can be only SF
  if ((is_func) && (ptype == DC_FrameType::SF)) {
    // request with functional address can be only SF
    pduContext.address = N_TarAddress::TAtype_2_Functional;

    if ((data[0] & 0x0fu) == 0) {
      pduContext.data = data + 2;
      pduContext.length = data[1];
    } else {
      pduContext.data = data + 1;
      pduContext.length = data[0] & 0x0fu;
    }

    // notify upper layer
    iso_client.OnIsoEvent(N_Event::Data, N_Result::OK_r, pduContext);
  } else if (is_phys) {
    switch (ptype) {
      case (DC_FrameType::FC):
        // flow control message for ICAN_Sender
        iso_sender.OnFlowControl(data[0] & 0x0fu, data[1], data[2]);
        break;

      default:
        iso_receiver.Receive(data, length);
        break;
    }
  }
}

IsoTpResult DoCAN_TP::Request(const uint8_t* data, size_t length) {

  return iso_sender.Send(data, length);
}

SetParamResult DoCAN_TP::SetParameter(ParName name, uint32_t v) {

  auto ret = SetParamResult::OK;

  if (iso_sender.IsBusy() || iso_receiver.IsBusy()) {
    // TODO: make this return more clear (for the case when Tx is busy)
    return SetParamResult::RX_ON;
  }

  switch (name) {
    case (ParName::BLKSZ):
    case (ParName::ST_MIN):
    case (ParName::Br_TIM_ms):
    case (ParName::Cr_TIM_ms):
      ret = iso_receiver.SetParameter(name, v);
      break;

    case (ParName::As_TIM_ms):
    case (ParName::Bs_TIM_ms):
      ret = iso_sender.SetParameter(name, v);
      break;

    case (ParName::CANDL):
      configDoCAN.candl = (v <= MAX_CANDL) ? v : MAX_CANDL;
      break;

    case (ParName::PADD_BYTE):
      configDoCAN.padding = static_cast<uint8_t>(v);
      break;

    case (ParName::PHYS_ADDR):
      configDoCAN.phys_id = v;
      break;;

    case (ParName::FUNC_ADDR):
      configDoCAN.func_id = v;
      break;

    case (ParName::RESP_ADDR):
      configDoCAN.resp_id = v;
      break;

    default:
      break;
  }

  return ret;
}

void DoCAN_TP::OnIsoRxEvent(N_Event event, N_Result result, const uint8_t* data, size_t length) {

  pduContext.data = data;
  pduContext.length = length;
  pduContext.address = N_TarAddress::TAtype_1_Physical;

  iso_client.OnIsoEvent(event, result, pduContext);
}

void DoCAN_TP::OnIsoTxEvent(N_Event event, N_Result result) {

  iso_client.OnIsoEvent(event, result, pduContext);
}
