#pragma once

#include "iso-tp-types.h"

class IsoTpClient {
 public:
  typedef struct
  {
    const uint8_t* data;
    size_t length;
    N_TarAddress address;
  } IsoTpInfo;

  virtual void OnIsoEvent(N_Type t, N_Result res, const IsoTpInfo& info) = 0;
};

class IsoTpHost {
 public:
  virtual IsoTpResult Request(const uint8_t* data, size_t length) = 0;
};

class IsoListener {
 public:
  virtual void ReadFrame(const uint8_t* data, size_t length, uint32_t msgid) = 0;
};

class IsoSender {
 public:
  virtual size_t SendFrame(const uint8_t* data, size_t length, uint32_t msgid) = 0;
};
