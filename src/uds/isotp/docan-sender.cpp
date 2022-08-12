
#include <string.h>
#include <assert.h>
#include "docan-sender.h"
#include "pci-helper.h"
#include "iso-tp.h"


IsoTpResult DoCAN_Sender::Send(IsoSender& sender, const uint8_t* data, size_t length)
{
  assert(data != nullptr);

  auto ret = CheckTxValid(length);
  const BaseEnum candl = static_cast<BaseEnum>(itp_conf.can_dl);

  if (ret == IsoTpResult::OK)
  {
    PciHelper helper;
    PciType pci{};
    auto pci_len = helper.PackData(can_message, length, candl, pci);

    assert(pci_len < candl);

    if (pci != PciType::ERROR)
    {
      txds.size = length;
      txds.passed = ((pci_len + length) > candl) ? (candl - pci_len) : length;

      memcpy(can_message + pci_len, data, txds.passed);

      for (size_t i = pci_len + txds.passed; i < candl; can_message[i++] = itp_conf.padding_byte);

      if (sender.SendFrame(can_message, candl, itp_conf.response_id))
      {
        if (pci == PciType::FF)
        {
          // FF transmition begins, wait for FC
          N_Bs_tim.Restart();
          txds.state = DtState::WAIT;
          txds.sn = 0u;
          memcpy(txbuff, data, length);
        }
        else if (pci == PciType::SF)
        {
          itp.OnIsoTxEvent(N_Type::Conf, N_Result::OK_s);
        }
      }
    }
  }

  return ret;
}

IsoTpResult DoCAN_Sender::CheckTxValid(size_t length)
{
  if (length == 0)
  {
    return IsoTpResult::WRONG_STATE;
  }
  else if (length > TXLEN)
  {
    return IsoTpResult::OVERFLOW;
  }
  else
  {
    return IsoTpResult::OK;
  }
}

void DoCAN_Sender::ProcessTx(IsoSender& sender)
{
  PciHelper pchelper;
  const size_t candl = static_cast<BaseEnum>(itp_conf.can_dl);

  if (txds.state == DtState::WAIT)
  {
    // segmented transmition is in progress wait for FC
    if (N_Bs_tim.Elapsed())
    {
      // wNo FC in specified time
      txds.state = DtState::IDLE;
      itp.OnIsoTxEvent(N_Type::Conf, N_Result::TIMEOUT_Bs);
    }
  }
  else if (txds.state == DtState::MF_DT)
  {
    while (true)
    {
      const size_t pci_len = pchelper.PackConseqFrame(can_message, ++txds.sn);
      // get the data len
      const size_t candlen = candl - pci_len;

      // get the length of data for putting in frame payload
      const size_t cpylen = ((txds.passed + candlen) < txds.size) ? candlen : (txds.size - txds.passed);

      if (txds.passed == 0 || last_send_ok)
      {
        // set pci part of data
        memcpy(can_message + pci_len, txbuff + txds.passed, cpylen);

        for (size_t i = cpylen + pci_len; i < candl; can_message[i++] = itp_conf.padding_byte);
      }

      last_send_ok = false;

      if (sender.SendFrame(can_message, candl, itp_conf.response_id) != 0)
      {
        last_send_ok = true;
        // Restart As timer every successful sending
        N_As_tim.Restart();
        txds.passed += cpylen;

        if (txds.passed >= txds.size)
        {
          // done, transfer stopped
          txds.state = DtState::IDLE;
          itp.OnIsoTxEvent(N_Type::Conf, N_Result::OK_s);
          break;
        }
        else
        {
          if (++txds.currblksize >= txds.blksize)
          {
            //
            txds.state = DtState::WAIT;
            break;
          }
        }

        if (txds.stmin != 0)
        {
          // STmin time have to be waited
          txds.state = DtState::PAUSE;
          N_As_tim.Stop();
          // break and wait in pause state
          break;
        }
      }
    }

    if (N_As_tim.Elapsed())
    {
      txds.state = DtState::IDLE;
      itp.OnIsoTxEvent(N_Type::Conf, N_Result::TIMEOUT_As);
    }
  }
  else if (txds.state == DtState::PAUSE)
  {
    // wait STmin
    if (STmin_tim.Elapsed())
    {
      txds.state = DtState::MF_DT;
    }
  }
}

void DoCAN_Sender::OnFlowControl(uint8_t flow_status, uint8_t blks, uint8_t stm)
{
  if (txds.state != DtState::WAIT)
  {
    // Unexpected PDU has came
    txds.state = DtState::IDLE;
    itp.OnIsoTxEvent(N_Type::Conf, N_Result::UNEXP_PDU);
    return;
  }

  switch (flow_status)
  {
    // CTS - client ready to receive BS messages
    case (0):
      txds.currblksize = 0;
      txds.blksize = blks;

      if (stm < 0x80)
      {
        txds.stmin = stm;
      }
      else if (stm > 0xf0u && stm < 0xfau)
      {
        txds.stmin = 1;
      }
      else
      {
        // if STmin is in reserved value - apply maximum valid value
        txds.stmin = 0x7fu;
      }

      // leave WAIT state here, stop Bs and restart As
      txds.state = DtState::MF_DT;
      // restart N_As before CF sending
      N_As_tim.Restart();
      N_Bs_tim.Stop();
      STmin_tim.Start(txds.stmin, true);
      break;

    // wait state, reset Bs timer
    case (1):
      N_Bs_tim.Restart();
      break;

    // Receiver side buffer overflow
    case (2):
      txds.state = DtState::IDLE;
      itp.OnIsoTxEvent(N_Type::Conf, N_Result::BUFFER_OVFLW);
      break;

    default:
      txds.state = DtState::IDLE;
      itp.OnIsoTxEvent(N_Type::Conf, N_Result::INVALID_FS);
      break;
  }
}

ParChangeResult DoCAN_Sender::SetParameter(ParName name, uint32_t v)
{
  auto ret = ParChangeResult::OK;

  if (txds.state != DtState::IDLE)
  {
    ret = ParChangeResult::WRONG_PARAMETER;
  }
  else
  {
    switch (name)
    {
      case (ParName::RESP_ADDR):
        if (v > 0)
        {
          itp_conf.response_id = v;
        }

        break;

      case (ParName::As_TIM_ms):
        itp_conf.As_tim = v;

        if (itp_conf.As_tim > 0)
        {
          N_As_tim.Start(itp_conf.As_tim);
          N_As_tim.Stop();
        }

        break;

      case (ParName::Bs_TIM_ms):
        itp_conf.Bs_tim = v;

        if (itp_conf.Bs_tim > 0)
        {
          N_Bs_tim.Start(itp_conf.Bs_tim);
          N_Bs_tim.Stop();
        }

        break;

      case (ParName::Cs_TIM_ms):
        itp_conf.Cs_tim = v;

        if (itp_conf.Cs_tim > 0)
        {
          N_Cs_tim.Start(itp_conf.Cs_tim);
          N_Cs_tim.Stop();
        }

        break;

      case (ParName::CANDL):
        itp_conf.can_dl = v > 64u ? 64u : v < 8 ? 8 : v;
        break;

      case (ParName::PADD_BYTE):
        itp_conf.padding_byte = v & 0xffu;
        break;

      default:
        ret = ParChangeResult::WRONG_PARAMETER;
        break;
    }
  }

  return ret;
}
