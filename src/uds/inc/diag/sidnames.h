#pragma once

#include <stdint.h>

enum class SIDs : uint8_t
{
  PUDS_SI_DiagnosticSessionControl = 0x10,
  PUDS_SI_ECUReset = 0x11,
  PUDS_SI_SecurityAccess = 0x27,
  PUDS_SI_CommunicationControl = 0x28,
  PUDS_SI_TesterPresent = 0x3E,
  PUDS_SI_AccessTimingParameter = 0x83,
  PUDS_SI_SecuredDataTransmission = 0x84,
  PUDS_SI_ControlDTCSetting = 0x85,
  PUDS_SI_ResponseOnEvent = 0x86,
  PUDS_SI_LinkControl = 0x87,
  PUDS_SI_ReadDataByIdentifier = 0x22,
  PUDS_SI_ReadMemoryByAddress = 0x23,
  PUDS_SI_ReadScalingDataByIdentifier = 0x24,
  PUDS_SI_ReadDataByPeriodicIdentifier = 0x2A,
  PUDS_SI_DynamicallyDefineDataIdentifier = 0x2C,
  PUDS_SI_WriteDataByIdentifier = 0x2E,
  PUDS_SI_WriteMemoryByAddress = 0x3D,
  PUDS_SI_ClearDiagnosticInformation = 0x14,
  PUDS_SI_ReadDTCInformation = 0x19,
  PUDS_SI_InputOutputControlByIdentifier = 0x2F,
  PUDS_SI_RoutineControl = 0x31,
  PUDS_SI_RequestDownload = 0x34,
  PUDS_SI_RequestUpload = 0x35,
  PUDS_SI_TransferData = 0x36,
  PUDS_SI_RequestTransferExit = 0x37,

  PUDS_NR_SI = 0x7f,
};

constexpr uint8_t SID_to_byte(SIDs sid)
{
  return static_cast<uint8_t>(sid);
}

constexpr uint8_t SID_response(SIDs sid)
{
  return SID_to_byte(sid) | (1u << 6u);
}

