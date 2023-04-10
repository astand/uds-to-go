#pragma once

#include <stdint.h>
#include <etc/helpers/ophelper.h>

using sid_t = uint8_t;

namespace sidhelper {

/// @brief All UDS services
/// @brief Diagnostic session control
constexpr sid_t DSC = 0x10;

/// @brief ECU reset
constexpr sid_t ERES = 0x11;

/// @brief Security Access
constexpr sid_t SA = 0x27;

/// @brief Communication control
constexpr sid_t COMC = 0x28;

/// @brief Tester present
constexpr sid_t TP = 0x3E;

/// @brief Access timing parameter
constexpr sid_t ATP = 0x83;

/// @brief Secured data transmission
constexpr sid_t SDT = 0x84;

/// @brief Control DTC settings
constexpr sid_t CDTC = 0x85;

/// @brief Response on event
constexpr sid_t ROE = 0x86;

/// @brief Link control
constexpr sid_t LC = 0x87;

/// @brief Read data by identifier
constexpr sid_t RDBI = 0x22;

/// @brief Read memory by address
constexpr sid_t RMBA = 0x23;

/// @brief Read scaling data by identifier
constexpr sid_t RSDBI = 0x24;

/// @brief Read data by periodic identifier
constexpr sid_t RDBPI = 0x2A;

/// @brief Dynamically define data identifier
constexpr sid_t DDDI = 0x2C;

/// @brief Write data by identifier
constexpr sid_t WDBI = 0x2E;

/// @brief Write memory by address
constexpr sid_t WMBA = 0x3D;

/// @brief Clear diagnostic information
constexpr sid_t CDI = 0x14;

/// @brief Read DTC information
constexpr sid_t RDTC = 0x19;

/// @brief Input output control by identifier
constexpr sid_t IOCBI = 0x2F;

/// @brief Routine control
constexpr sid_t RC = 0x31;

/// @brief Request download
constexpr sid_t RD = 0x34;

/// @brief Request upload
constexpr sid_t RU = 0x35;

/// @brief Transfer data
constexpr sid_t TD = 0x36;

/// @brief Request transfer exit
constexpr sid_t RTE = 0x37;

/// @brief Negative response
constexpr sid_t NR_SI = 0x7f;


/// @brief Service response bit indicator
constexpr sid_t SID_RESPONSE_BIT = (1u << 6u);

/// @brief Mask for getting service from response value
constexpr sid_t SID_RESPONSE_MASK = ~SID_RESPONSE_BIT;

// Diagnostic session levels (4) defined by standard (ISO 14299-1:2020)

/// @brief Default diagnostic session level
constexpr uint8_t DSC_LEV_DEFT = 1u;
/// @brief Programming diagnostic session level
constexpr uint8_t DSC_LEV_PRGM = 2u;
/// @brief Extended diagnostic session level
constexpr uint8_t DSC_LEV_EXTD = 3u;
/// @brief Safety system diagnostic session level
constexpr uint8_t DSC_LEV_SSYS = 4u;

constexpr sid_t to_response(const sid_t sid) {

  return sid | SID_RESPONSE_BIT;
}

constexpr sid_t remove_response(const sid_t sid) {

  return (sid & SID_RESPONSE_MASK);
}

constexpr bool is_response(sid_t sid) {

  return ((sid & SID_RESPONSE_BIT) == SID_RESPONSE_BIT);
}

constexpr sid_t get_subfunc(const sid_t byte) {

  return (byte & 0x7fu);
}

constexpr bool is_pos_response_suppress(const sid_t secondbyte) {

  return (secondbyte & 0x80u) == 0x80u;
}

/// @brief Checks if current session value is default session
/// @param sessvalue current session value
/// @return true if the value is equal to DSC_LEV_DEFT, otherwise false
constexpr bool is_dsc_def_session(uint8_t sessvalue) {

  return sessvalue == DSC_LEV_DEFT;
}

/// @brief Check if the service identificator can be handled
/// @param sid Service value
/// @return true if requested service can be handled, otherwise false
constexpr bool is_sid_processable(const sid_t sid) {

  return ophelper::is_in_range<0x01, 0x3e>(remove_response(sid))
      || ophelper::is_in_range<0xba, 0xbe>(remove_response(sid))
      || ophelper::is_in_range<0x83, 0x88>(remove_response(sid))
      || (sid == NR_SI);
}

} // namespace sidhelper
