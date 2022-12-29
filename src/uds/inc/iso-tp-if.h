#pragma once

#include "iso-tp-types.h"

/// @brief ISO TP client interface. ISO TP handler calls this interface
/// on each ISO TP event (confirmation, indication etc)
class IsoTpClient {
 public:
  /// @brief ISO TP event context information descriptor
  typedef struct
  {
    const uint8_t* data;
    size_t length;
    N_TarAddress address;
  } IsoTpInfo;

  /// @brief ISO TP event notification
  /// @param ev event type
  /// @param res network event result
  /// @param info context information
  virtual void OnIsoEvent(N_Event ev, N_Result res, const IsoTpInfo& info) = 0;
};

/// @brief ISO TP implementation main interface. Clients use this
/// interface to invoke payload transmittion
class IsoTpImpl {
 public:

  /// @brief ISO TP request
  /// @param data pointer to ISO TP payload
  /// @param length size of ISO TP payload
  virtual IsoTpResult Request(const uint8_t* data, size_t length) = 0;
};

/// @brief CAN message receiver interface
class ICAN_Listener {
 public:

  /// @brief Callback on CAN message receiving
  /// @param data pointer to CAN message data
  /// @param length CAN message payload size
  /// @param msgid CAN message ID
  virtual void ReadFrame(const uint8_t* data, size_t length, uint32_t msgid) = 0;
};

/// @brief CAN message sender interface
class ICAN_Sender {
 public:

  /// @brief Sends CAN message to CAN line
  /// @param data pointer to CAN message payload
  /// @param length size of CAN message payload
  /// @param msgid CAN message ID
  /// @return length if OK, 0 if failed
  virtual size_t SendFrame(const uint8_t* data, size_t length, uint32_t msgid) = 0;
};
