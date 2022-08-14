#pragma once

#include "iso-tp-types.h"

class IsoTpClient {
 public:
  typedef struct
  {
    const uint8_t* data;
    size_t length;
    N_TarAddress address;
  } IsoTpInfo;

  virtual void OnIsoEvent(N_Type t, N_Result res, const IsoTpInfo& info) = 0;
};

/**
 * @brief Interface for ISO TP users
 *
 */
class IsoTpImpl {
 public:
  /**
   * @brief Send ISO TP send request
   *
   * @param data - pointer to date to be send to network
   * @param length - data length
   * @return IsoTpResult
   */
  virtual IsoTpResult Request(const uint8_t* data, size_t length) = 0;
};

/**
 * @brief Interface for CAN router for passing CAN frames to client
 *
 */
class ICAN_Listener {
 public:
  /**
   * @brief Process CAN frame from the bus
   *
   * @param data - pointer to data in CAN frame
   * @param length - length of CAN frame payload
   * @param msgid - CAN frame message ID
   */
  virtual void ReadFrame(const uint8_t* data, size_t length, uint32_t msgid) = 0;
};

/**
 * @brief Interface for sending CAN frame to outside world
 *
 */
class ICAN_Sender {
 public:
  /**
   * @brief Sends CAN frame to the BUS
   *
   * @param data - pointer to CAN frame payload
   * @param length - DLC or data length
   * @param msgid - CAN frame message ID
   * @return size_t - length if OK, 0 if failed
   */
  virtual size_t SendFrame(const uint8_t* data, size_t length, uint32_t msgid) = 0;
};
