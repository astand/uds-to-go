#pragma once

#include <uds/inc/diag/nrcnames.h>


//------------------------------------------------------------------------------
// Result of handling
//------------------------------------------------------------------------------
enum class DidResult {

  // DID request is not processed and another attempt can be performed using another DIDHandler class
  Ignored,

  // DID request is already handled with positive response
  Positive,

  // DID request is already handled with negative response
  Negative,
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class DidHandler {

 public:
  // DID not supported: returned DidResult::Ignored
  // DID pos handling:  returned DidResult::Positive, len_out is filled and > 0
  // DID neg handling:  returned DidResult::Negative, nrc_out is filled
  virtual DidResult ReadDID(uint32_t did, uint8_t* data, size_t capacity, size_t& len_out, NRCs& nrc_out) = 0;

  // DID not supported: returned DidResult::Ignored
  // DID pos handling:  returned DidResult::Positive
  // DID neg handling:  returned DidResult::Negative, nrc_out is filled
  virtual DidResult WriteDID(uint32_t did, const uint8_t* data, size_t len, NRCs& nrc_out) = 0;
};
