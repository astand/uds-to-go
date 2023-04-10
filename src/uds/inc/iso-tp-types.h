#pragma once

#include <stdint.h>
#include <stddef.h>

#include "typedefs.h"

/// @brief Network event result
enum class N_Result : enumbase_t {
  // This value means that the service execution has been completed successfully; it can be issued to a
  // service user on both the sender and receiver sides.
  OK_s,
  OK_r,
  // This value is issued to the protocol user when the timer N_Ar / N_As has passed its time - out value
  // N_Asmax / N_Armax; it can be issued to service users on both the sender and receiver sides.
  TIMEOUT_As,
  TIMEOUT_Ar,
  // This value is issued to the service user when the timer N_Bs has passed its time - out value N_Bsmax;
  // it can be issued to the service user on the sender side only.
  TIMEOUT_Bs,
  // This value is issued to the service user when the timer N_Cr has passed its time - out value N_Crmax;
  // it can be issued to the service user on the receiver side only.
  TIMEOUT_Cr,
  // This value is issued to the service user upon receipt of an unexpected SequenceNumber (PCI.SN)
  // value; it can be issued to the service user on the receiver side only.
  WRONG_SN,
  // This value is issued to the service user when an invalid or unknown FlowStatus value has been
  // received in a FlowControl (FC) N_PDU; it can be issued to the service user on the sender side only.
  INVALID_FS,
  // This value is issued to the service user upon receipt of an unexpected protocol data unit; it can be
  // issued to the service user on the receiver side only.
  UNEXP_PDU,
  // This value is issued to the service user when the receiver has transmitted N_WFTmax FlowControl
  // N_PDUs with FlowStatus = WAIT in a row and following this, it cannot meet the performance
  // requirement for the transmission of a FlowControl N_PDU with FlowStatus = ClearToSend. It can be
  // issued to the service user on the receiver side only.
  WFT_OVRN,
  // This value is issued to the service user upon receipt of a FlowControl (FC) N_PDU with
  // FlowStatus = OVFLW. It indicates that the buffer on the receiver side of a segmented message
  // transmission cannot store the number of bytes specified by the FirstFrame DataLength (FF_DL)
  // parameter in the FirstFrame and therefore the transmission of the segmented message was
  // aborted. It can be issued to the service user on the sender side only.
  BUFFER_OVFLW,
  // This is the general error value. It shall be issued to the service user when an error has been detected
  // by the network layer and no other parameter value can be used to better describe the error. It can
  // be issued to the service user on both the sender and receiver sides.
  ERROR_s,
  ERROR_r
};

/// @brief Result of attempt to change parameter
enum class SetParamResult : enumbase_t {
  // This value means that the service execution has been completed successfully; it can be issued to a
  // service user on both the sender and receiver sides.
  OK,
  // This value is issued to the service user to indicate that the service did not execute since reception
  // of the message identified by <N_AI> was taking place; it can be issued to the service user on the
  // receiver side only.
  RX_ON,
  // This value is issued to the service user to indicate that the service did not execute due to an
  // undefined <Parameter>; it can be issued to a service user on both the receiver and sender sides.
  WRONG_PARAMETER,
  // This value is issued to the service user to indicate that the service did not execute due to an out - ofrange
  // <Parameter_Value>; it can be issued to a service user on both the receiver and sender sides.
  WRONG_VALUE
};

/// @brief List of parameters
enum class ParName : enumbase_t {
  ST_MIN,
  BLKSZ,
  PHYS_ADDR,
  FUNC_ADDR,
  RESP_ADDR,
  CANDL,
  As_TIM_ms,
  Bs_TIM_ms,
  Br_TIM_ms,
  Cr_TIM_ms,
  PADD_BYTE,
};


/// @brief Network event type
enum class N_Event : enumbase_t {
  Conf,
  Data,
  DataFF,
};

/// @brief Result of request ot iso-tp
enum class IsoTpResult : enumbase_t {
  OK,
  BUSY,
  WRONG_STATE,
  OVERFLOW
};

/// @brief List of possible CAN datalength
enum class CanDl : enumbase_t {
  CANDL_8 = 8,
  CANDL_12 = 12,
  CANDL_16 = 16,
  CANDL_20 = 20,
  CANDL_24 = 24,
  CANDL_32 = 32,
  CANDL_48 = 48,
  CANDL_64 = 64
};

/// @brief ISO-TP target address
enum class N_TarAddress {
  TAtype_Invalid = 0,
  // CAN base format (CLASSICAL CAN, 11-bit)
  TAtype_1_Physical,
  TAtype_2_Functional,
  // CAN FD base format (CAN FD, 11-bit)
  TAtype_3_Physical,
  TAtype_4_Functional,
  // CAN extended format (CLASSICAL CAN, 29-bit)
  TAtype_5_Physical,
  TAtype_6_Functional,
  // CAN FD extended format (CAN FD, 29-bit)
  TAtype_7_Physical,
  TAtype_8_Functional
};
