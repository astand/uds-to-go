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

enum class DiagSession
{
  Default = 1,
  Extended = 3,
  Programming = 2
};

// result of servicediag process
typedef enum { kSI_NotHandled, kSI_HandledResponseOk, kSI_HandledNoResponse } ProcessResult_t;

//------------------------------------------------------------------------------
// Security access levels
// kSA_Locked - equivalent for (sa_unlocked == false)
//------------------------------------------------------------------------------
typedef enum { kSA_Locked = 0, kSA_Level_0 = 1, kSA_Level_1 = 2, kSA_Level_2 = 3, kSA_Level_3 = 4} SA_Level_t;

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
