#include "did-router.h"
#include "did-keeper.h"

constexpr uint32_t SI_SIZE_BYTES = (1u);
constexpr uint32_t DID_SIZE_BYTES = (2u);

ProcessResult DidRouter::OnIndication(const IndicationInfo& inf)
{
  auto pres = ProcessResult::HANDLED_RESP_OK;
  len_ = inf.size;
  data_ = inf.data;

  switch (inf.head.SI)
  {
    case PUDS_SI_ReadDataByIdentifier:
      ReadDataByIdentifierHandler();
      break;

    case PUDS_SI_WriteDataByIdentifier:
      WriteDataByIdentifierHandler();
      break;

    default:
      pres = ProcessResult::NOT_HANDLED;
      break;
  }

  return pres;
}

ProcessResult DidRouter::OnConfirmation(S_Result res)
{
  return ProcessResult::NOT_HANDLED;
}


void DidRouter::ReadDataByIdentifierHandler()
{
  if (len_ != (SI_SIZE_BYTES + DID_SIZE_BYTES))
  {
    rtr1.SendNegResponse(NRCs::IMLOIF);
    return;
  }

  // Process all DIDs
  uint32_t idx = SI_SIZE_BYTES; // index in output buffer

  // Read DID from request (big-endian)
  uint16_t dataid = data_[SI_SIZE_BYTES] << 8 | data_[SI_SIZE_BYTES + 1];

  // Place the next DID into res
  rtr1.pubBuff[idx++] = dataid >> 8;
  rtr1.pubBuff[idx++] = dataid & 0xFF;

  size_t len_out = 0;
  NRCs nrc_out = NRCs::ROOR;
  DidResult ret = dider.ReadDID(dataid, rtr1.pubBuff + idx, 64u, len_out, nrc_out);

  // In case of positive handling and zero len_out - response with ROOR
  // In case of ignored request - response with ROOR
  if (((ret == DidResult::Positive) && (len_out == 0)) || (ret == DidResult::Ignored))
  {
    ret = DidResult::Negative;
    nrc_out = NRCs::ROOR;
  }

  // Send positive response
  rtr1.pubSize = len_out + SI_SIZE_BYTES + DID_SIZE_BYTES;
}

void DidRouter::WriteDataByIdentifierHandler()
{
  // Only one DID can be written at once
  uint16_t dataid = data_[1] << 8 | data_[2];
  rtr1.pubBuff[1] = data_[1];
  rtr1.pubBuff[2] = data_[2];

  // Every WDBI must have len >= 3 bytes
  if (len_ <= SI_SIZE_BYTES + DID_SIZE_BYTES)
  {
    rtr1.SendNegResponse(NRCs::IMLOIF);
  }
  else
  {
    NRCs nrc_out = NRCs::ROOR;
    DidResult ret = dider.WriteDID(dataid, data_ + SI_SIZE_BYTES + DID_SIZE_BYTES, len_ - SI_SIZE_BYTES - DID_SIZE_BYTES,
        nrc_out);

    // In case of ignored request - response with ROOR
    if (ret == DidResult::Ignored)
    {
      ret = DidResult::Negative;
      nrc_out = NRCs::ROOR;
    }

    // Prepare answer
    if (ret == DidResult::Positive)
    {
      // Success - DID is handled and data is copied into buffer
      rtr1.pubSize = 3;
    }
    else
    {
      // Error - DID is handled with error
      rtr1.SendNegResponse(nrc_out);
    }
  }
}
