#pragma once

#include <stdint.h>
#include <stddef.h>

enum class PciType
{
  SF = 0, FF = 0x10, FC = 0x30, CF = 0x20, ERROR = 0xffu
};

enum class FlowState
{
  CTS = 0, WAIT = 1, OVERFLOW = 2, ERROR
};

class PciHelper {
 public:
  typedef struct
  {
    size_t pcilen;
    // SF, FF
    PciType type;
    size_t dlen;
    // FC
    FlowState flowstate;
    uint8_t bs;
    uint8_t stmin;
    // CF
    uint8_t sn;
  } PciMeta;

 public:
  size_t UnpackPciInfo(const uint8_t* data, size_t length, PciMeta& pci);

  size_t PackData(uint8_t* data, size_t length, size_t candl, PciType& type);

  size_t PackFlowControl(uint8_t* data, FlowState state, uint8_t bs, uint8_t stmin);

  size_t PackConseqFrame(uint8_t* data, uint8_t sn);
};
