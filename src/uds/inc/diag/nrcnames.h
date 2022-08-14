#pragma once

typedef enum
{
  // positive response PR
  NRC_PR = 0,

  // generalReject GR
  NRC_GR = 0x10,

  // serviceNotSupported SNS
  NRC_SNS = 0x11,

  // sub-functionNotSupported NRC_SFNS
  NRC_SFNS = 0x12,

  // ncorrectMessageLengthOrInvalidFormat NRC_IMLOIF
  NRC_IMLOIF = 0x13,

  // responseTooLong NRC_RTL
  NRC_RTL = 0x14,

  // busyRepeatRequest NRC_BRR
  NRC_BRR = 0x21,

  // conditionsNotCorrect NRC_CNC
  NRC_CNC = 0x22,

  // requestSequenceError NRC_RSE
  NRC_RSE = 0x24,

  // noResponseFromSubnetComponent NRFSC
  NRC_NRFSC = 0x25,

  // FailurePreventsExecutionOfRequestedAction FPEORA
  NRC_FPEORA = 0x26,

  // requestOutOfRange ROOR
  NRC_ROOR = 0x31,

  // securityAccessDenied SAD
  NRC_SAD = 0x33,

  // invalidKey IK
  NRC_IK = 0x35,

  // exceedNumberOfAttempts ENOA
  NRC_ENOA = 0x36,

  // requiredTimeDelayNotExpired RTDNE
  NRC_RTDNE = 0x37,

  // uploadDownloadNotAccepted UDNA
  NRC_UDNA = 0x70,

  // transferDataSuspended TDS
  NRC_TDS = 0x71,

  // generalProgrammingFailure GPF
  NRC_GPF = 0x72,

  // wrongBlockSequenceCounter WBSC
  NRC_WBSC = 0x73,

  // requestCorrectlyReceived-ResponsePending RCRRP
  NRC_RCRRP = 0x78,

  // sub-functionNotSupportedInActiveSession SFNSIAS
  NRC_SFNSIAS = 0x7E,

  // serviceNotSupportedInActiveSession SNSIAS
  NRC_SNSIAS = 0x7F,

  // rpmTooHigh RPMTH
  NRC_RPMTH = 0x81,

  // rpmTooLow RPMTL
  NRC_RPMTL = 0x82,

  // engineIsRunning EIR
  NRC_EIR = 0x83,

  // engineIsNotRunning EINR
  NRC_EINR = 0x84,

  // engineRunTimeTooLow ERTTL
  NRC_ERTTL = 0x85,

  // temperatureTooHigh TEMPTH
  NRC_TEMPTH = 0x86,

  // temperatureTooLow TEMPTL
  NRC_TEMPTL = 0x87,

  // vehicleSpeedTooHigh VSTH
  NRC_VSTH = 0x88,

  // vehicleSpeedTooLow VSTL
  NRC_VSTL = 0x89,

  // throttle/PedalTooHigh TPTH
  NRC_TPTH = 0x8A,

  // throttle/PedalTooLow TPTL
  NRC_TPTL = 0x8B,

  // transmissionRangeNotInNeutral TRNIN
  NRC_TRNIN = 0x8C,

  // transmissionRangeNotInGear TRNIG
  NRC_TRNIG = 0x8D,

  // brakeSwitch(es)NotClosed (Brake Pedal not pressed or not applied) BSNC
  NRC_BSNC = 0x8F,

  // shifterLeverNotInPark SLNIP
  NRC_SLNIP = 0x90,

  // torqueConverterClutchLocked TCCL
  NRC_TCCL = 0x91,

  // voltageTooHigh VTH
  NRC_VTH = 0x92,

  // voltageTooLow VTL
  NRC_VTL = 0x93,

  // 0x94 – 0xEF reservedForSpecificConditionsNotCorrect RFSCNC
  // This range of values is reserved by this document for future definition.

  // 0xF0 – 0xFE vehicleManufacturerSpecificConditionsNotCorrect VMSCNC
  // This range of values is reserved for vehicle manufacturer specific condition not
  // correct scenarios.

} NRCs_t;

