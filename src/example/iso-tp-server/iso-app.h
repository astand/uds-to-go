#pragma once

#include <assert.h>
#include <stdlib.h>
#include <uds/isotp/docan-tp.h>

/* ---------------------------------------------------------------------------- */
class IsoApp : public IsoTpClient {
 public:
  void OnIsoEvent(N_Event t, N_Result res, const IsoTpInfo& inf) {
    assert((uint8_t)t < 3 && (uint8_t)res < 13);

    std::cout << "Event [" << type_names[(uint8_t)t] << "] Status [" << res_names[(uint8_t)res] << "]";

    if (t == N_Event::Data && res == N_Result::OK_r) {
      // print data
      std::cout << std::endl << "------------------------------------------------" << std::endl;
      //                        "00 11 22 33 44 55 66 77 88 99 aa bb cc dd ee ff ";
      assert(inf.data != nullptr && inf.length != 0u);

      std::cout << std::hex;

      for (size_t i = 0; i < inf.length; i++) {
        if (i != 0 && (i & 0x0f) == 0u) {
          std::cout << std::endl;
        }

        if (inf.data[i] < 0x10u) {
          std::cout << "0";
        }

        std::cout << (int)inf.data[i] << " ";
      }

      std::cout << std::endl << "------------------------------------------------" << std::dec;
      std::cout << std::endl << " <--- RECV OK: " << inf.length << " bytes.";
    }
    else if (t == N_Event::DataFF && res == N_Result::OK_r) {
      std::cout << " Expected size = " << inf.length << " bytes.";
    }

    std::cout << std::endl;
  }

 private:
  const char* type_names[3] =
  {
    "Conf  ",
    "Data  ",
    "DataFF"
  };

  const char* res_names[13] =
  {
    "OK_s",
    "OK_r",
    "TIMEOUT_As",
    "TIMEOUT_Ar",
    "TIMEOUT_Bs",
    "TIMEOUT_Cr",
    "WRONG_SN",
    "INVALID_FS",
    "UNEXP_PDU",
    "WFT_OVRN",
    "BUFFER_OVFLW",
    "ERROR_s",
    "ERROR_r"
  };
};

