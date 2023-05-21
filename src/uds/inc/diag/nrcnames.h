#pragma once

#include <stdint.h>

enum class NRCs : uint8_t {
  PR = 0, // positive response PR
  GR = 0x10, // generalReject GR
  SNS = 0x11, // serviceNotSupported SNS
  SFNS = 0x12, // sub-functionNotSupportedSFNS
  IMLOIF = 0x13, // ncorrectMessageLengthOrInvalidFormatIMLOIF
  RTL = 0x14, // responseTooLongRTL
  BRR = 0x21, // busyRepeatRequestBRR
  CNC = 0x22, // conditionsNotCorrectCNC
  RSE = 0x24, // requestSequenceErrorRSE
  NRFSC = 0x25, // noResponseFromSubnetComponent NRFSC
  FPEORA = 0x26, // FailurePreventsExecutionOfRequestedAction FPEORA
  ROOR = 0x31, // requestOutOfRange ROOR
  SAD = 0x33, // securityAccessDenied SAD
  IK = 0x35, // invalidKey IK
  ENOA = 0x36, // exceedNumberOfAttempts ENOA
  RTDNE = 0x37, // requiredTimeDelayNotExpired RTDNE
  UDNA = 0x70, // uploadDownloadNotAccepted UDNA
  TDS = 0x71, // transferDataSuspended TDS
  GPF = 0x72, // generalProgrammingFailure GPF
  WBSC = 0x73, // wrongBlockSequenceCounter WBSC
  RCRRP = 0x78, // requestCorrectlyReceived-ResponsePending RCRRP
  SFNSIAS = 0x7E, // sub-functionNotSupportedInActiveSession SFNSIAS
  SNSIAS = 0x7F, // serviceNotSupportedInActiveSession SNSIAS
  RPMTH = 0x81, // rpmTooHigh RPMTH
  RPMTL = 0x82, // rpmTooLow RPMTL
  EIR = 0x83, // engineIsRunning EIR
  EINR = 0x84, // engineIsNotRunning EINR
  ERTTL = 0x85, // engineRunTimeTooLow ERTTL
  TEMPTH = 0x86, // temperatureTooHigh TEMPTH
  TEMPTL = 0x87, // temperatureTooLow TEMPTL
  VSTH = 0x88, // vehicleSpeedTooHigh VSTH
  VSTL = 0x89, // vehicleSpeedTooLow VSTL
  TPTH = 0x8A, // throttle/PedalTooHigh TPTH
  TPTL = 0x8B, // throttle/PedalTooLow TPTL
  TRNIN = 0x8C, // transmissionRangeNotInNeutral TRNIN
  TRNIG = 0x8D, // transmissionRangeNotInGear TRNIG
  BSNC = 0x8F, // brakeSwitch(es)NotClosed (Brake Pedal not pressed or not applied) BSNC
  SLNIP = 0x90, // shifterLeverNotInPark SLNIP
  TCCL = 0x91, // torqueConverterClutchLocked TCCL
  VTH = 0x92, // voltageTooHigh VTH
  VTL = 0x93, // voltageTooLow VTL

  // This range of values is reserved by this document for future definition.
  // 0x94 – 0xEF reservedForSpecificConditionsNotCorrect RFSCNC

  // This range of values is reserved for vehicle manufacturer specific condition not
  // 0xF0 – 0xFE vehicleManufacturerSpecificConditionsNotCorrect VMSCNC

};

constexpr uint8_t NRC_to_byte(NRCs nrc) {

  return static_cast<uint8_t>(nrc);
}
