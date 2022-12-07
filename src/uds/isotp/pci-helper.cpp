#include "pci-helper.h"
#include "../inc/iso-tp-const.h"

datasize_t PciHelper::UnpackPciInfo(const uint8_t* data, datasize_t candl, PciMeta& pci)
{
  datasize_t ret = 0;
  pci.type = DC_FrameType::ERROR;
  pci.pcilen = ret;

  if (candl == 0)
  {
    return ret;
  }

  pci.type = from_byte<DC_FrameType>(data[0] & 0xf0u);

  switch (pci.type)
  {
    case (DC_FrameType::SF):
      if (candl <= CLASSICCAN_DL_MAX)
      {
        // classic SF
        ret = 1;
        pci.dlen = data[0] & 0x0fu;

        if ((pci.dlen + ret) > candl)
        {
          ret = 0;
          pci.type = DC_FrameType::ERROR;
        }
      }
      else if (candl <= CANFD_DL_MAX)
      {
        // CAN FD
        ret = 2;
        pci.dlen = data[1];

        if (pci.dlen + ret > candl)
        {
          ret = 0;
          pci.type = DC_FrameType::ERROR;
        }
      }
      else
      {
        ret = 0;
        pci.type = DC_FrameType::ERROR;
      }

      break;

    case (DC_FrameType::FF):
      if (candl < MIN_FF_CANDL)
      {
        ret = 0;
        pci.type = DC_FrameType::ERROR;
      }
      else
      {
        ret = 2;
        pci.dlen = (((data[0] & 0x0fu) << 8) | (data[1]));

        if (pci.dlen == 0)
        {
          // > 4095
          ret = 6;
          pci.dlen = (((data[2] & 0xFF) << 24)  |
              ((data[3] & 0xFF) << 16)  |
              ((data[4] & 0xFF)  << 8)  |
              ((data[5] & 0xFF)  << 0));

          if (pci.dlen <= 0xfffu)
          {
            // error dlen value, reset it zero to ignore handling
            pci.type = DC_FrameType::ERROR;
            pci.dlen = 0;
            ret = 0;
          }
        }
        else if (pci.dlen < MIN_FF_PAYLOAD_SIZE)
        {
          // error, FF min possible length is 8
          pci.type = DC_FrameType::ERROR;
          pci.dlen = 0;
          ret = 0;
        }
      }

      break;

    case (DC_FrameType::FC):
    {
      uint8_t fs = (data[0] & 0x0fu);

      if (fs > 2 || candl < 3)
      {
        ret = 0;
        pci.type = DC_FrameType::ERROR;
      }
      else
      {
        ret = 3;
        pci.flowstate = from_byte<DC_FlowState>(fs);
        pci.bs = data[1];
        pci.stmin = data[2];

        if (((pci.stmin > 0x7f) && (pci.stmin <= 0xf0)) || pci.stmin > 0xf9)
        {
          ret = 0;
          pci.type = DC_FrameType::ERROR;
        }
        else if (pci.stmin > 0x7f)
        {
          // 1 ms for range 100..900 us
          pci.stmin = 1;
        }
      }
    }

    break;

    case (DC_FrameType::CF):
      ret = 1;
      pci.sn = data[0] & 0x0f;

      break;

    case (DC_FrameType::ERROR):
    default:
      ret = 0;
      pci.type = DC_FrameType::ERROR;
      break;
  }

  pci.pcilen = ret;
  return ret;
}

datasize_t PciHelper::PackPciForData(uint8_t* data, datasize_t length, datasize_t candl, DC_FrameType& reftype)
{
  datasize_t pci_len = 0;

  if (length > 0)
  {
    bool is_can_fd = (candl > CLASSICCAN_DL_MAX);
    pci_len = (is_can_fd) ? (2) : (1);

    if ((pci_len + length) <= candl)
    {
      // SF: full length is less or equal to candl
      reftype = DC_FrameType::SF;
      data[0] = to_byte(reftype);

      if (is_can_fd)
      {
        // CAN FD SF
        data[1] = length & 0xFFU;
        pci_len = 2;
      }
      else
      {
        // Classic SF
        data[0] |= (length & 0x0fu);
        pci_len = 1;
      }
    }
    else
    {
      // FF
      reftype = DC_FrameType::FF;
      data[0] = to_byte(reftype);

      if (length <= 0xfff)
      {
        data[0] |= ((length >> 8u) & 0x0fu);
        data[1] = length & 0xff;
        pci_len = 2;
      }
      else
      {
        // FF > 4095 (0xfff)
        data[1] = 0;
        data[2] = ((length >> 24) & 0xFF);
        data[3] = ((length >> 16) & 0xFF);
        data[4] = ((length >> 8) & 0xFF);
        data[5] = ((length >> 0) & 0xFF);
        pci_len = 6;
      }
    }
  }

  return pci_len;
}

datasize_t PciHelper::PackFlowControl(uint8_t* data, DC_FlowState state, uint8_t bs, uint8_t stmin)
{
  data[0] = to_byte(DC_FrameType::FC) | to_byte(state);
  data[1] = bs;
  data[2] = stmin;
  return 3;
}

datasize_t PciHelper::PackConseqFrame(uint8_t* data, uint8_t sn)
{
  data[0] = static_cast<uint8_t>(DC_FrameType::CF) | (sn & 0x0fu);
  return 1;
}
