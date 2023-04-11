#pragma once

#include <stdint.h>
#include <stddef.h>

#include "../inc/typedefs.h"

/// @brief DoCAN frame type
enum class DC_FrameType {

  SF = 0, FF = 0x10, FC = 0x30, CF = 0x20, ERROR = 0xffu
};

/// @brief Segmented transaction state
enum class DC_FlowState {

  CTS = 0, WAIT = 1, OVERFLOW = 2
};

/// @brief Helper class for DoCAN protocol information handling
class PciHelper {

 public:
  typedef struct {
    datasize_t pcilen;
    // SF, FF
    DC_FrameType type;
    datasize_t dlen;
    // FC
    DC_FlowState flowstate;
    uint8_t bs;
    uint8_t stmin;
    // CF
    uint8_t sn;
  } PciMeta;

 public:

  /// @brief Extracts protocol control info from raw message payload
  /// @param data pointer to message payload
  /// @param candl payload size
  /// @param pci out reference to pci variable
  /// @return pci info size
  datasize_t UnpackPciInfo(const uint8_t* data, datasize_t candl, PciMeta& pci);

  /// @brief Packs protocol control information for general data packet
  /// @param data pointer to message payload (minimal length is 8)
  /// @param length data size
  /// @param candl CAN message data length
  /// @param reftype out reference to pci packet type
  /// @return pci info size
  datasize_t PackPciForData(uint8_t* data, datasize_t length, datasize_t candl, DC_FrameType& reftype);

  /// @brief Packs protocol control information for flow control packet
  /// @param data pointer to message payload (minimal length is 3)
  /// @param state state of flow
  /// @param bs block size
  /// @param stmin st min
  /// @return pci info size
  datasize_t PackFlowControl(uint8_t* data, DC_FlowState state, uint8_t bs, uint8_t stmin);

  /// @brief Packs protocol control information consequitive frame
  /// @param data pointer to message payload (minimal length is 1)
  /// @param sn current serial number
  /// @return pci info size
  datasize_t PackConseqFrame(uint8_t* data, uint8_t sn);
};
