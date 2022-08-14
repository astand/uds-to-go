#pragma once

#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include "../inc/iso-tp-if.h"
#include "../inc/app-types.h"

class DiagRouter : public IsoTpClient {
 public:
  DiagRouter(uint8_t* mem, size_t size) : sbuff(mem), SBUFF_LEN(size) {
    assert(sbuff != nullptr);
    assert(size > 8);
  }

  // this function exports for clients
  void SendResponse(const uint8_t* data, size_t len);
  void SendNegResponse(NRCs_t nrc);
  void SendNegResponse(uint8_t sid, NRCs_t nrc);

  // API for ISO-TP to notify session/application layer about any kind of events
  virtual void OnIsoEvent(N_Type t, N_Result res, const IsoTpInfo& info) override;

  DSC_Session GetSession() const {
    return dscsess;
  }

  void SetIsoTp(IsoTpImpl* h) {
    assert(h != nullptr);
    host = h;
  }

 private:
  void Handle_Conf();
  void Handle_DataFF();
  void Handle_Data();

  N_Type ntype;
  N_Result nres;
  IsoTpInfo ninf;

  IsoTpImpl* host{nullptr};

  uint8_t* const sbuff;
  const size_t SBUFF_LEN;

  DSC_Session dscsess{DSC_Session::DEFAULT};
};
