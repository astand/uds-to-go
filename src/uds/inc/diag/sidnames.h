#pragma once

#include <stdint.h>

/// @brief All UDS services
enum class SIDs : uint8_t {
  /// @brief Diagnostic session control
  DSC = 0x10,

  /// @brief ECU reset
  ERES = 0x11,

  /// @brief Security Access
  SA = 0x27,

  /// @brief Communication control
  COMC = 0x28,

  /// @brief Tester present
  TP = 0x3E,

  /// @brief Access timing parameter
  ATP = 0x83,

  /// @brief Secured data transmission
  SDT = 0x84,

  /// @brief Control DTC settings
  CDTC = 0x85,

  /// @brief Response on event
  ROE = 0x86,

  /// @brief Link control
  LC = 0x87,

  /// @brief Read data by identifier
  RDBI = 0x22,

  /// @brief Read memory by address
  RMBA = 0x23,

  /// @brief Read scaling data by identifier
  RSDBI = 0x24,

  /// @brief Read data by periodic identifier
  RDBPI = 0x2A,

  /// @brief Dynamically define data identifier
  DDDI = 0x2C,

  /// @brief Write data by identifier
  WDBI = 0x2E,

  /// @brief Write memory by address
  WMBA = 0x3D,

  /// @brief Clear diagnostic information
  CDI = 0x14,

  /// @brief Read DTC information
  RDTC = 0x19,

  /// @brief Input output control by identifier
  IOCBI = 0x2F,

  /// @brief Routine control
  RC = 0x31,

  /// @brief Request download
  RD = 0x34,

  /// @brief Request upload
  RU = 0x35,

  /// @brief Transfer data
  TD = 0x36,

  /// @brief Request transfer exit
  RTE = 0x37,

  /// @brief Negative response
  NR_SI = 0x7f,
};

// Diagnostic session levels (4) defined by standard (ISO 14299-1:2020)

/// @brief Default diagnostic session level
constexpr uint8_t DSC_LEV_DEFT = 1u;
/// @brief Programming diagnostic session level
constexpr uint8_t DSC_LEV_PRGM = 2u;
/// @brief Extended diagnostic session level
constexpr uint8_t DSC_LEV_EXTD = 3u;
/// @brief Safety system diagnostic session level
constexpr uint8_t DSC_LEV_SSYS = 4u;

constexpr uint8_t SID_to_byte(SIDs sid) {

  return static_cast<uint8_t>(sid);
}

constexpr SIDs SID_from_byte(const uint8_t byte) {

  return static_cast<SIDs>(byte);
}

constexpr uint8_t SID_response(SIDs sid) {

  return SID_to_byte(sid) | (1u << 6u);
}

constexpr uint8_t SID_get_subfunc(const uint8_t byte) {

  return (byte & 0x7fu);
}

constexpr bool SID_is_pos_response_suppress(const uint8_t secondbyte) {

  return (secondbyte & 0x80u) == 0x80u;
}

/// @brief Checks if current session value is default session
/// @param sessvalue current session value
/// @return true if the value is equal to DSC_LEV_DEFT, otherwise false
constexpr bool is_dsc_def_session(uint8_t sessvalue) {

  return sessvalue == DSC_LEV_DEFT;
}

