#include "pci-helper.h"


size_t PciHelper::UnpackPciInfo(const uint8_t* data, size_t candl, PciMeta& pci)
{
  size_t ret = 0;
  pci.type = PciType::ERROR;
  pci.pcilen = ret;

  if (candl == 0)
  {
    return ret;
  }

  pci.type = static_cast<PciType>(data[0] & 0xf0u);

  switch (pci.type)
  {
    case (PciType::SF):
      if (candl <= 8)
      {
        // classic SF
        ret = 1;
        pci.dlen = data[0] & 0x0fu;

        if (pci.dlen + ret > candl)
        {
          ret = 0;
          pci.type = PciType::ERROR;
        }
      }
      else if (candl <= 64u)
      {
        // CAN FD
        ret = 2;
        pci.dlen = data[1];

        if (pci.dlen + ret > candl)
        {
          ret = 0;
          pci.type = PciType::ERROR;
        }
      }
      else
      {
        ret = 0;
        pci.type = PciType::ERROR;
      }

      break;

    case (PciType::FF):
      if (candl < 8)
      {
        ret = 0;
        pci.type = PciType::ERROR;
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
            pci.type = PciType::ERROR;
            pci.dlen = 0;
            ret = 0;
          }
        }
        else if (pci.dlen < 8)
        {
          // error, FF min possible length is 8
          pci.type = PciType::ERROR;
          pci.dlen = 0;
          ret = 0;
        }
      }

      break;

    case (PciType::FC):
    {
      uint8_t fs = (data[0] & 0x0fu);

      if (fs > 2 || candl < 3)
      {
        ret = 0;
        pci.type = PciType::ERROR;
      }
      else
      {
        ret = 3;
        pci.flowstate = static_cast<FlowState>(fs);
        pci.bs = data[1];
        pci.stmin = data[2];

        if (((pci.stmin > 0x7f) && (pci.stmin <= 0xf0)) || pci.stmin > 0xf9)
        {
          ret = 0;
          pci.type = PciType::ERROR;
        }
        else if (pci.stmin > 0x7f)
        {
          // 1 ms for range 100..900 us
          pci.stmin = 1;
        }
      }
    }

    break;

    case (PciType::CF):
      ret = 1;
      pci.sn = data[0] & 0x0f;

      break;

    case (PciType::ERROR):
      ret = 0;
      pci.type = PciType::ERROR;

    default:
      ret = 0;
      pci.type = PciType::ERROR;
      break;
  }

  pci.pcilen = ret;
  return ret;
}

size_t PciHelper::PackData(uint8_t* data, size_t length, size_t candl, PciType& type)
{
  size_t pci_len = 0;
  type = PciType::ERROR;

  if (length > 0 && candl >= 8 && candl <= 64)
  {
    bool is_can_fd = (candl > 8);
    pci_len = (is_can_fd) ? (2) : (1);

    if ((pci_len + length) > candl)
    {
      // FF
      type = PciType::FF;
      data[0] = static_cast<uint8_t>(type);

      if (length > 4095)
      {
        // FF > 4095
        data[1] = 0;
        data[2] = ((length >> 24) & 0xFF);
        data[3] = ((length >> 16) & 0xFF);
        data[4] = ((length >> 8) & 0xFF);
        data[5] = ((length >> 0) & 0xFF);
        pci_len = 6;
      }
      else
      {
        // FF <= 4096
        data[0] |= ((length >> 8) & 0x0fu);
        data[1] = length & 0xff;
        pci_len = 2;
      }
    }
    else
    {
      type = PciType::SF;
      data[0] = static_cast<uint8_t>(type);

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
  }

  return pci_len;
}

size_t PciHelper::PackFlowControl(uint8_t* data, FlowState state, uint8_t bs, uint8_t stmin)
{
  data[0] = static_cast<uint8_t>(PciType::FC);
  data[0] |= ((static_cast<uint8_t>(state) >> 4) & 0x0f);
  data[1] = bs;
  data[2] = stmin;
  return 3;
}

size_t PciHelper::PackConseqFrame(uint8_t* data, uint8_t sn)
{
  data[0] = static_cast<uint8_t>(PciType::CF);
  data[0] |= (sn & 0x0fu);
  return 1;
}
