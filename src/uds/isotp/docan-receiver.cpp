#include <assert.h>
#include <string.h>
#include "docan-receiver.h"
#include "docan-tp.h"
#include "pci-helper.h"

void DoCAN_Receiver::ProcessRx()
{
  if (rxds.state == RxState::ACTIVE)
  {
    if (Cr_tim.Elapsed())
    {
      // no consequitive frame in time
      rxds.state = RxState::IDLE;
      rxds.passed = 0;
      rxds.rxsize = 0;
      itp.OnIsoRxEvent(N_Event::Conf, N_Result::TIMEOUT_Cr);
    }
  }
}


void DoCAN_Receiver::Receive(const uint8_t* data, datasize_t candl)
{
  PciHelper helper;
  PciHelper::PciMeta inf;
  datasize_t pcioffset = helper.UnpackPciInfo(data, candl, inf);

  assert(pcioffset <= candl && candl < CANFD_DL_MAX);

  if (pcioffset > 0)
  {
    switch (inf.type)
    {
      case (DC_FrameType::FF):
        if (rxds.state == RxState::ACTIVE)
        {
          // reception is in progress (ISO 15765-2 tab 23 (p 38))
          itp.OnIsoRxEvent(N_Event::Data, N_Result::UNEXP_PDU);
        }

        // start new multi reception (cannot be less than 7 bytes)
        if (inf.dlen < MIN_FF_CANDL)
        {
          // Ignore case (ISO 15765-2 9.6.3.2 (p 29))
        }
        else if (inf.dlen > RXLEN)
        {
          rxds.state = RxState::IDLE;
          // send FC with overflow status
          auto ret = helper.PackFlowControl(can_message, DC_FlowState::OVERFLOW, 0, 0);
          itp.PduToCan(can_message, ret);
        }
        else
        {
          // fine, reception can be processed
          auto ret = helper.PackFlowControl(can_message, DC_FlowState::CTS, rxds.blksize, rxds.stmin);
          itp.PduToCan(can_message, ret);

          rxds.currblkn = 0;
          rxds.rxsize = inf.dlen;
          datasize_t cpylen = candl - pcioffset;
          memcpy(rxbuff + rxds.passed, data + pcioffset, cpylen);
          rxds.passed = cpylen;
          rxds.expectsn = 1;
          itp.OnIsoRxEvent(N_Event::DataFF, N_Result::OK_r, nullptr, rxds.rxsize);
          // start timer
          rxds.state = RxState::ACTIVE;
          Cr_tim.Restart();
        }

        break;

      case (DC_FrameType::CF):
        if (rxds.state != RxState::ACTIVE || pcioffset != 1)
        {
          rxds.state = RxState::IDLE;
          itp.OnIsoRxEvent(N_Event::Data, N_Result::UNEXP_PDU);
        }
        else
        {
          if (inf.sn != (rxds.expectsn & 0xfu))
          {
            // Abort (ISO 15765 9.6.4.4 (p.30))
            rxds.state = RxState::IDLE;
            itp.OnIsoRxEvent(N_Event::Data, N_Result::UNEXP_PDU);
          }
          else
          {
            Cr_tim.Restart();
            datasize_t cpylen = candl - pcioffset;

            if ((rxds.passed + cpylen) > rxds.rxsize)
            {
              cpylen = rxds.rxsize - rxds.passed;
            }

            assert(cpylen <= candl);

            memcpy(rxbuff + rxds.passed, data + pcioffset, cpylen);
            rxds.passed += cpylen;
            ++rxds.expectsn;
            ++rxds.currblkn;

            if (rxds.passed == rxds.rxsize)
            {
              // reception ended, notify client
              rxds.state = RxState::IDLE;
              itp.OnIsoRxEvent(N_Event::Data, N_Result::OK_r, rxbuff, rxds.rxsize);
            }
            else if (rxds.currblkn >= rxds.blksize)
            {
              // block ended, send FC
              auto ret = helper.PackFlowControl(can_message, DC_FlowState::CTS, rxds.blksize, rxds.stmin);
              itp.PduToCan(can_message, ret);
              rxds.currblkn = 0;
            }
          }
        }

        break;

      case (DC_FrameType::SF):
        if (rxds.state == RxState::ACTIVE)
        {
          // ISO 15765-2 tab 23 (p 38) Notify unexpected pdu, abort multi reception
          // and process SF payload
          rxds.state = RxState::IDLE;
          itp.OnIsoRxEvent(N_Event::Data, N_Result::UNEXP_PDU);
        }

        itp.OnIsoRxEvent(N_Event::Data, N_Result::OK_r, data + pcioffset, inf.dlen);

        break;

      default:
        if (rxds.state == RxState::ACTIVE)
        {
          // unexpected PDU during active reception
          rxds.state = RxState::IDLE;
          itp.OnIsoRxEvent(N_Event::Conf, N_Result::UNEXP_PDU);
        }

        break;
    }
  }
}

SetParamResult DoCAN_Receiver::SetParameter(ParName name, uint32_t v)
{
  auto ret = SetParamResult::OK;

  switch (name)
  {
    case (ParName::BLKSZ):
      rxds.blksize = v;
      break;

    case (ParName::ST_MIN):
      rxds.stmin = v;
      break;

    case (ParName::Cr_TIM_ms):
      Cr_tim.Start(v);
      Cr_tim.Stop();
      break;

    default:
      ret = SetParamResult::WRONG_PARAMETER;
      break;
  }

  return ret;
}
