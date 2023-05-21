#pragma once

#include <stdint.h>
#include <helpers/IKeeper.h>
#include "did-handler.h"

template<size_t N>
class DidKeeper : public MemAsKeeper<DidHandler, N> {

 public:
  DidKeeper() : MemAsKeeper<DidHandler, N>() {}

  virtual DidResult ReadDID(uint32_t did, uint8_t* data, size_t capacity, size_t& len_out, NRCs& nrc_out) override {
    DidResult ret = DidResult::Ignored;

    uint32_t i = 0u;
    DidHandler* refdid {nullptr};

    while (this->TryReadElem(i, refdid)) {
      ret = refdid->ReadDID(did, data, capacity, len_out, nrc_out);

      if (ret != DidResult::Ignored) {
        break;
      }
    }

    return ret;
  }

  virtual DidResult WriteDID(uint32_t did, const uint8_t* data, size_t len, NRCs& nrc_out) override {
    DidResult ret = DidResult::Ignored;
    uint32_t i = 0u;
    DidHandler* refdid {nullptr};

    while (this->TryReadElem(i, refdid)) {
      ret = refdid->WriteDID(did, data, len, nrc_out);

      if (ret != DidResult::Ignored) {
        break;
      }
    }

    return ret;
  }
};
