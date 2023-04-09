#include <gtest/gtest.h>
#include <stdint.h>
#include <memory>
#include <uds/inc/iso-tp-if.h>
#include <uds/session/uds-app-manager.h>
#include <uds/session/uds-app-client.h>

static uint32_t responseCounter = 0u;

constexpr uint8_t SID_1 = 0x22u;
constexpr uint8_t SID1_DATA_PR = 0x33u;
constexpr uint8_t SID1_DATA_VTH = 0x34u;
constexpr uint8_t SID1_DATA_ROOR = 0xFFu;

constexpr uint8_t SID_2_SNSIAS = 0x32u;
constexpr uint8_t SID_SNS = 0x23u;

// The frist class must be mock for IsoTpImpl
class MockIsoTp : public IsoTpImpl {

 public:
  virtual IsoTpResult Request(const uint8_t* data, size_t length) override {
    ++responseCounter;
    return IsoTpResult::OK;
  }
};

class MockUdsAppClient : public UdsAppClient {

 public:
  MockUdsAppClient(UdsAppManager& appManager) : UdsAppClient(appManager) {}

  virtual bool IsServiceSupported(const SIDs sid, size_t& minlenght, bool& subfunc) {

    uint8_t sidbyte = SID_to_byte(sid);

    if (sidbyte == SID_1 || sidbyte == SID_2_SNSIAS) {
      minlenght = 3u;
      subfunc = false;
    } else if (sidbyte == 0x42) {
      minlenght = 4u;
      subfunc = true;
    } else {
      return false;
    }

    return true;
  }

  virtual ProcessResult OnAppIndication(const IndicationInfo& inf) {

    uint8_t sidbyte = SID_to_byte(inf.head.SI);
    size_t retLength = 0u;

    if (sidbyte == SID_1) {
      if (inf.data[2] == SID1_DATA_PR) {
        retLength = 2u;
      } else if (inf.data[2] == SID1_DATA_VTH) {
        udsRouter.SendNegResponse(NRCs::VTH);
      } else {
        udsRouter.SendNegResponse(NRCs::ROOR);
      }
    } else if (sidbyte == SID_2_SNSIAS) {
      udsRouter.SendNegResponse(NRCs::SNSIAS);
    } else {
      return ProcessResult::NOT_HANDLED;
    }

    if (retLength == 0u) {
      return ProcessResult::HANDLED_RESP_NO;
    } else {
      udsRouter.pubRespLength = retLength;
      return ProcessResult::HANDLED_RESP_OK;
    }
  }

  virtual void OnAppConfirmation(S_Result) {

  }
};

static MockIsoTp testIsoTp;

static uint8_t appBuff[32u] = { 0 };
static SessionInfo testSessionInfo;

static UdsAppManager testAppManger(appBuff, 32u, testSessionInfo);
static MockUdsAppClient testAppClient(testAppManger);


static uint8_t rxArray[8] = { 0 };

static IsoTpClient::IsoTpInfo isoContext = {
  rxArray,
  0,
  N_TarAddress::TAtype_1_Physical
};


void set_payload(uint8_t sid, std::array<uint8_t, 8> data, size_t len) {
  assert(len <= 7);
  memset(rxArray, 0, 8);
  rxArray[0] = sid;
  memcpy(rxArray + 1, data.data(), len);
}

void set_phys_payload(uint8_t sid, std::array<uint8_t, 8> data, size_t len) {
  isoContext.address = N_TarAddress::TAtype_1_Physical;
  set_payload(sid, data, len);
  isoContext.length = 1 + len;
}

void set_func_payload(uint8_t sid, std::array<uint8_t, 8> data, size_t len) {
  set_phys_payload(sid, data, len);
  isoContext.address = N_TarAddress::TAtype_2_Functional;
}


static uint32_t response_expected = 0u;

uint32_t response_was_not_sent() {
  return response_expected;
}

uint32_t response_was_sent() {
  ++response_expected;
  return response_was_not_sent();
}

// Demonstrate some basic assertions.
TEST(ResponseUdsAppTest, GeneralTests) {

  EXPECT_EQ(1, 1);
  testAppManger.SetClient(&testAppClient);
  testAppManger.SetIsoTp(&testIsoTp);


  // Table 6 shows 6 — Physically possible communication addressed request schemes message with physical without addressing.

  // a) - Server sends a positive response message because the service identifier and all data-parameters
  // are supported of the client's request message.

  // b) -  Server sends a positive response message because the service identifier and at least one data-
  // parameter is supported of the client's request message.

  // expected positive response
  set_phys_payload(SID_1, {0x00, SID1_DATA_PR}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_sent());

  // c) - Server sends a negative response message (e.g. IMLOIF:
  // incorrectMessageLengthOrIncorrectFormat) because the service identifier is supported and at least
  // one data-parameter is supported of the client's request message, but some other error occurred
  // (e.g. wrong length of the request message) during processing of the service.

  // expected VTH
  set_phys_payload(SID_1, {0x00, SID1_DATA_VTH}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_sent());

  // d) - Server sends a negative response message with the negative response code ROOR
  // (requestOutOfRange) because the service identifier is supported and none of the requested data-
  // parameters are supported of the client's request message.

  // expected ROOR
  set_phys_payload(SID_1, {0x00, SID1_DATA_ROOR}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_sent());


  // e) - Server sends a negative response message with the negative response code SNS
  // (serviceNotSupported) or SNSIAS (serviceNotSupportedInActiveSession) because the service
  // identifier  is not supported  of the  client's  request  message.

  // e.1 - expected SNS
  set_phys_payload(SID_SNS, {0x00, SID1_DATA_ROOR}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_sent());

  // e.2 - expected SNSIAS
  set_phys_payload(SID_2_SNSIAS, {0x00, SID1_DATA_ROOR}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_sent());

  //
  // Table 7 — Functionally addressed request message without SubFunction parameter and server response behaviour
  //

  // a) - Server sends a positive response message because the service identifier and all data-parameters
  // are supported of the client's request message.
  // b) - Server sends a positive response message because the service identifier and at least one data-
  // parameter is supported of the client's request message.

  // expected positive response
  set_func_payload(SID_1, {0x00, SID1_DATA_PR}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_sent());

  // c) - Server sends a negative response message (e.g. IMLOIF: incorrectMessageLengthOrIncorrectFormat)
  // because the service identifier is supported and at least one data-parameter is supported
  // of the client's request message, but some other error occurred (e.g. wrong length of the
  // request message) during processing of the service.

  // exptected VTH
  set_func_payload(SID_1, {0x00, SID1_DATA_VTH}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_sent());

  // d) - Server sends no response message because the negative response code ROOR (requestOutOfRange;
  // which would occur because the service identifier is supported, but none of the requested data-
  // parameters is supported of the client's request) is always suppressed in case of a functionally
  // addressed request.

  // expected ROOR
  set_func_payload(SID_1, {0x00, SID1_DATA_ROOR}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_not_sent());


  // e) - Server sends no response message because the negative response codes SNS
  // (serviceNotSupported) and SNSIAS (serviceNotSupportedInActiveSession), which are identified by
  // the server because the service identifier is not supported of the client's request, are always
  // suppressed  in case  of a functionally  addressed  request.

  // expected SNS
  set_func_payload(SID_SNS, {0x00, SID1_DATA_ROOR}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_not_sent());

  // expected SNSIAS
  set_func_payload(SID_2_SNSIAS, {0x00, SID1_DATA_ROOR}, 2);
  testAppManger.OnIsoEvent(N_Event::Data, N_Result::OK_r, isoContext);
  EXPECT_EQ(responseCounter, response_was_not_sent());

}