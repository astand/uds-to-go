#pragma once

typedef enum
{
  NRC_PR = 0, // positive response PR
  NRC_GR = 0x10, // generalReject GR
  NRC_SNS = 0x11, // serviceNotSupported SNS
  NRC_SFNS = 0x12, // sub-functionNotSupported NRC_SFNS
  NRC_IMLOIF = 0x13, // ncorrectMessageLengthOrInvalidFormat NRC_IMLOIF
  NRC_RTL = 0x14, // responseTooLong NRC_RTL
  NRC_BRR = 0x21, // busyRepeatRequest NRC_BRR
  NRC_CNC = 0x22, // conditionsNotCorrect NRC_CNC
  NRC_RSE = 0x24, // requestSequenceError NRC_RSE
  NRC_NRFSC = 0x25, // noResponseFromSubnetComponent NRFSC
  NRC_FPEORA = 0x26, // FailurePreventsExecutionOfRequestedAction FPEORA
  NRC_ROOR = 0x31, // requestOutOfRange ROOR
  NRC_SAD = 0x33, // securityAccessDenied SAD
  NRC_IK = 0x35, // invalidKey IK
  NRC_ENOA = 0x36, // exceedNumberOfAttempts ENOA
  NRC_RTDNE = 0x37, // requiredTimeDelayNotExpired RTDNE
  NRC_UDNA = 0x70, // uploadDownloadNotAccepted UDNA
  NRC_TDS = 0x71, // transferDataSuspended TDS
  NRC_GPF = 0x72, // generalProgrammingFailure GPF
  NRC_WBSC = 0x73, // wrongBlockSequenceCounter WBSC
  NRC_RCRRP = 0x78, // requestCorrectlyReceived-ResponsePending RCRRP
  NRC_SFNSIAS = 0x7E, // sub-functionNotSupportedInActiveSession SFNSIAS
  NRC_SNSIAS = 0x7F, // serviceNotSupportedInActiveSession SNSIAS
  NRC_RPMTH = 0x81, // rpmTooHigh RPMTH
  NRC_RPMTL = 0x82, // rpmTooLow RPMTL
  NRC_EIR = 0x83, // engineIsRunning EIR
  NRC_EINR = 0x84, // engineIsNotRunning EINR
  NRC_ERTTL = 0x85, // engineRunTimeTooLow ERTTL
  NRC_TEMPTH = 0x86, // temperatureTooHigh TEMPTH
  NRC_TEMPTL = 0x87, // temperatureTooLow TEMPTL
  NRC_VSTH = 0x88, // vehicleSpeedTooHigh VSTH
  NRC_VSTL = 0x89, // vehicleSpeedTooLow VSTL
  NRC_TPTH = 0x8A, // throttle/PedalTooHigh TPTH
  NRC_TPTL = 0x8B, // throttle/PedalTooLow TPTL
  NRC_TRNIN = 0x8C, // transmissionRangeNotInNeutral TRNIN
  NRC_TRNIG = 0x8D, // transmissionRangeNotInGear TRNIG
  NRC_BSNC = 0x8F, // brakeSwitch(es)NotClosed (Brake Pedal not pressed or not applied) BSNC
  NRC_SLNIP = 0x90, // shifterLeverNotInPark SLNIP
  NRC_TCCL = 0x91, // torqueConverterClutchLocked TCCL
  NRC_VTH = 0x92, // voltageTooHigh VTH
  NRC_VTL = 0x93, // voltageTooLow VTL

  // This range of values is reserved by this document for future definition.
  // 0x94 – 0xEF reservedForSpecificConditionsNotCorrect RFSCNC

  // This range of values is reserved for vehicle manufacturer specific condition not
  // 0xF0 – 0xFE vehicleManufacturerSpecificConditionsNotCorrect VMSCNC

} NRCs_t;

