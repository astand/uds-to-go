
#include <string.h>
#include <assert.h>
#include "docan-sender.h"
#include "pci-helper.h"
#include "docan-tp.h"


IsoTpResult DoCAN_Sender::Send(const uint8_t* data, datasize_t length)
{
  assert(data != nullptr);

  auto ret = CheckTxValid(length);
  const datasize_t candl = itp.Config().candl;

  if (ret == IsoTpResult::OK)
  {
    PciHelper helper;
    PciType pci{};
    auto pci_len = helper.PackPciForData(can_message, length, candl, pci);

    assert(pci_len < candl);

    if (pci != PciType::ERROR)
    {
      txds.size = length;
      txds.passed = ((pci_len + length) > candl) ? (candl - pci_len) : length;

      memcpy(can_message + pci_len, data, txds.passed);

      if (IsBusy())
      {
        // Active transmittion is in progress. Stop it and process current request
        txds.state = DtState::IDLE;
      }

      if (itp.PduToCan(can_message, pci_len + txds.passed))
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
          // TODO: this is simplification. when it is necessary
          // to get true HW CAN node confirmation of sending
          // this call should be removed and CAN HW sending provider
          // must call it by itself after successful frame acceptance by BUS
          // that means that CAN HW sender has to have reference to DoCAN_TP
          itp.OnIsoTxEvent(N_Type::Conf, N_Result::OK_s);
        }
      }
    }
  }

  return ret;
}

IsoTpResult DoCAN_Sender::CheckTxValid(datasize_t length)
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

void DoCAN_Sender::ProcessTx()
{
  PciHelper pchelper;

  switch (txds.state)
  {
    case (DtState::WAIT):
      if (N_Bs_tim.Elapsed())
      {
        // check BS timer while wait next FC, timeout leads to stop transmittion
        txds.state = DtState::IDLE;
        itp.OnIsoTxEvent(N_Type::Conf, N_Result::TIMEOUT_Bs);
      }

      break;

    case (DtState::MF_DT):
      while (true)
      {
        const datasize_t pci_len = pchelper.PackConseqFrame(can_message, ++txds.sn);
        // get the data len
        const datasize_t candlen = itp.Config().candl - pci_len;
        // get the length of data for putting in frame payload
        const datasize_t cpylen = ((txds.passed + candlen) < txds.size) ? candlen : (txds.size - txds.passed);

        if (txds.passed == 0 || last_send_ok)
        {
          // set pci part of data
          memcpy(can_message + pci_len, txbuff + txds.passed, cpylen);
        }

        last_send_ok = false;

        // Start As timer just before attempt to send CAN frame
        if (!N_As_tim.IsActive())
        {
          N_As_tim.Restart();
        }

        // if (sender.SendFrame(can_message, candl, itp.Config().resp_id) != 0)
        if (itp.PduToCan(can_message, cpylen + pci_len))
        {
          // CAN sending result it OK, stop As timer
          N_As_tim.Stop();
          last_send_ok = true;

          txds.passed += cpylen;

          if (txds.passed >= txds.size)
          {
            // done, transfer stopped
            txds.state = DtState::IDLE;

            // TODO: this is simplification. when it is necessary
            // to get true HW CAN node confirmation of sending
            // this call should be removed and CAN HW sending provider
            // must call it by itself after successful frame acceptance by BUS
            // that means that CAN HW sender has to have reference to DoCAN_TP
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
            break;
          }
        }
        else
        {
          // impossible to send, quit and try next time
          break;
        }
      }

      if (N_As_tim.Elapsed())
      {
        txds.state = DtState::IDLE;
        itp.OnIsoTxEvent(N_Type::Conf, N_Result::TIMEOUT_As);
      }

      break;

    case (DtState::PAUSE):
      if (STmin_tim.Elapsed())
      {
        txds.state = DtState::MF_DT;
      }

      break;

    default:
      break;
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

  switch (name)
  {
    case (ParName::As_TIM_ms):
      N_As_tim.Start(v);
      N_As_tim.Stop();
      break;

    case (ParName::Bs_TIM_ms):
      N_Bs_tim.Start(v);
      N_Bs_tim.Stop();
      break;

    default:
      ret = ParChangeResult::WRONG_PARAMETER;
      break;
  }

  return ret;
}
