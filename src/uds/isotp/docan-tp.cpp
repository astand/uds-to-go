#include <string.h>
#include "pci-helper.h"
#include "docan-tp.h"


void DoCAN_TP::Process()
{
  iso_sender.ProcessTx();
  iso_receiver.ProcessRx();
}

void DoCAN_TP::ReadFrame(const uint8_t* data, size_t length, uint32_t msgid)
{
  bool is_phys = (msgid == docan_config.phys_id);
  bool is_func = (msgid == docan_config.func_id);

  PciType ptype = static_cast<PciType>(data[0] & 0xf0u);

  // request with functional address can be only SF
  if ((is_func) && (ptype == PciType::SF))
  {
    // request with functional address can be only SF
    paydsc.address = N_TarAddress::TAtype_2_Functional;

    if ((data[0] & 0x0fu) == 0)
    {
      paydsc.data = data + 2;
      paydsc.length = data[1];
    }
    else
    {
      paydsc.data = data + 1;
      paydsc.length = data[0] & 0x0fu;
    }

    // notify upper layer
    iso_client.OnIsoEvent(N_Type::Data, N_Result::OK_r, paydsc);
  }
  else if (is_phys)
  {
    switch (ptype)
    {
      case (PciType::FC):
        // flow control message for isosender
        iso_sender.OnFlowControl(data[0] & 0x0fu, data[1], data[2]);
        break;

      default:
        iso_receiver.Receive(data, length);
        break;
    }
  }
}

IsoTpResult DoCAN_TP::Request(const uint8_t* data, size_t length)
{
  return iso_sender.Send(data, length);
}

ParChangeResult DoCAN_TP::SetParameter(ParName name, uint32_t v)
{
  auto ret = ParChangeResult::OK;

  if (iso_sender.IsBusy() || iso_receiver.IsBusy())
  {
    // TODO: make this return more clear (for the case when Tx is busy)
    return ParChangeResult::RX_ON;
  }

  switch (name)
  {
    case (ParName::BLKSZ):
    case (ParName::ST_MIN):
    case (ParName::Br_TIM_ms):
    case (ParName::Cr_TIM_ms):
      ret = iso_receiver.SetParameter(name, v);
      break;

    case (ParName::As_TIM_ms):
    case (ParName::Bs_TIM_ms):
    case (ParName::Cs_TIM_ms):
      ret = iso_sender.SetParameter(name, v);
      break;

    case (ParName::CANDL):
      docan_config.candl = (v <= MAX_CANDL) ? v : MAX_CANDL;
      break;

    case (ParName::PADD_BYTE):
      docan_config.padding = static_cast<uint8_t>(v);
      break;

    case (ParName::PHYS_ADDR):
      docan_config.phys_id = v;
      break;;

    case (ParName::FUNC_ADDR):
      docan_config.func_id = v;
      break;

    case (ParName::RESP_ADDR):
      docan_config.resp_id = v;
      break;

    default:
      break;
  }

  return ret;
}

void DoCAN_TP::OnIsoRxEvent(N_Type event, N_Result result, const uint8_t* data, size_t length)
{
  paydsc.data = data;
  paydsc.length = length;
  paydsc.address = N_TarAddress::TAtype_1_Physical;

  iso_client.OnIsoEvent(event, result, paydsc);
}

void DoCAN_TP::OnIsoTxEvent(N_Type event, N_Result result)
{
  iso_client.OnIsoEvent(event, result, paydsc);
}
