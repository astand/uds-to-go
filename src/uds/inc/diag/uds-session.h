#pragma once

#include <stdint.h>
#include "sidnames.h"

/// @brief Target address type (physical, functional)
enum class TargetAddressType
{
  PHYS, FUNC, UNKNOWN
};

/// @brief Session type (default, non-default)
enum class SessionType
{
  DEFAULT, NONDEFAULT
};

/// @brief Session processing result (OK, notOK)
enum class S_Result
{
  OK, NOK
};

/// @brief Session parameter set result
enum class SessParamResult
{
  OK, ERR
};

/// @brief Session parameter collection
enum class SessParamType
{
  S3_TIM, P2_TIM, P2_ENHC
};

/// @brief Service processing result
enum class ProcessResult
{
  /// @brief Request not handled
  NOT_HANDLED,

  /// @brief Request handled, response must be sent
  HANDLED_RESP_OK,

  /// @brief Request handled, response must not be sent
  HANDLED_RESP_NO,

  /// @brief Request handled, session pending mode must be invoked
  HANDLED_PENDING
};

/// @brief Send request result
enum class SendResult
{
  OK, OVRF, ERROR
};

typedef struct
{
  /// @brief Service identifie6r6
  SIDs SI;
  /// @brief Sub-function code
  uint8_t SF;
  /// @brief No response
  uint8_t NoResponse;
  /// @brief Response `
  uint8_t respSI;
} RequestContext;


typedef struct
{
  /// @brief Pointer to payload data
  const uint8_t* data;
  /// @brief Data size
  uint32_t size;
  /// @brief Requested target address
  TargetAddressType addr;
  /// @brief Request service information
  RequestContext head;
} IndicationInfo;

/// @brief Session state descriptor
typedef struct
{
  /// @brief Current session number
  uint8_t sess;
  /// @brief Current session security levle
  uint8_t sec_level;
} SessionInfo;
