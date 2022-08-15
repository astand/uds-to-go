#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../inc/typedefs.h"

enum class PciType
{
  SF = 0, FF = 0x10, FC = 0x30, CF = 0x20, ERROR = 0xffu
};

enum class FlowState
{
  CTS = 0, WAIT = 1, OVERFLOW = 2
};

class PciHelper {
 public:
  typedef struct
  {
    datasize_t pcilen;
    // SF, FF
    PciType type;
    datasize_t dlen;
    // FC
    FlowState flowstate;
    uint8_t bs;
    uint8_t stmin;
    // CF
    uint8_t sn;
  } PciMeta;

 public:
  datasize_t UnpackPciInfo(const uint8_t* data, datasize_t length, PciMeta& pci);

  datasize_t PackPciForData(uint8_t* data, datasize_t length, datasize_t candl, PciType& reftype);

  datasize_t PackFlowControl(uint8_t* data, FlowState state, uint8_t bs, uint8_t stmin);

  datasize_t PackConseqFrame(uint8_t* data, uint8_t sn);
};
