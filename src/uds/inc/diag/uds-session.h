#pragma once

#include <stdint.h>

enum class UdsAddress
{
  PHYS, FUNC, UNKNOWN
};

enum class SessionState
{
  DEFAULT, NONDEFAULT
};

enum class S_Result
{
  OK, NOK
};

typedef uint8_t ServiceID_t;
typedef uint8_t SubFunctionID_t;

// result of servicediag process
typedef enum { kSI_NotHandled, kSI_HandledResponseOk, kSI_HandledNoResponse } ProcessResult_t;

typedef struct
{
  uint8_t SI;
  uint8_t SF;
  uint8_t NoResponse;
  uint8_t respSI;
} SI_Head_t;

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
  UdsAddress addr;
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