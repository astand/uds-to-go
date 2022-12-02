#pragma once

#include <stdint.h>

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

typedef uint8_t ServiceID_t;
typedef uint8_t SubFunctionID_t;

typedef struct
{
  uint8_t SI;
  uint8_t SF;
  uint8_t NoResponse;
  uint8_t respSI;
} SI_Head_t;

enum class ProcessResult
{
  NOT_HANDLED, HANDLED_RESP_OK, HANDLED_RESP_NO
};


/**
 * @brief High level based sending operation result types
 *
 */
enum class SendResult
{
  OK,
  OVRF,
  ERROR
};

typedef struct
{
  const uint8_t* data;
  uint32_t size;
  TargetAddressType addr;
  SI_Head_t head;
} IndicationInfo;

typedef struct
{
  uint8_t sess;
  uint8_t sec_level;
} SessionInfo;

enum class A_Result
{
  OK, ERROR
};

enum class SessResult
{
  OK, ERR
};

enum class SessParam
{
  S3_TIM, P2_TIM, P2_ENHC
};
