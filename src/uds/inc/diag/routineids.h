//**************************************************************************************************
// This file contains definitions for common routine IDS
//**************************************************************************************************
#pragma once

//------------------------------------------------------------------------------
// ISOSAEReserved range
//------------------------------------------------------------------------------
#define RC_ID_ISOSAERESRVD1_MIN (0x0000U)
#define RC_ID_ISOSAERESRVD1_MAX (0x00FFU)

//------------------------------------------------------------------------------
// TachographTestIds range
//------------------------------------------------------------------------------
#define RC_ID_TACHORI_MIN       (0x0100U)
#define RC_ID_TACHORI_MAX       (0x01FFU)

//------------------------------------------------------------------------------
// vehicleManufacturerSpecific range
//------------------------------------------------------------------------------
#define RC_ID_VMS_MIN           (0x0200U)
#define RC_ID_VMS_MAX           (0xDFFFU)

// preProgrammingCheck
#define RC_ID_SSS_PREPROGCHK    (0x0201U)

// postProgrammingCheck
#define RC_ID_SSS_POSTPROGCHK   (0x0202U)

// eraseSection
#define RC_ID_SSS_ERASESECTION  (0x0203U)

// encryptionInitVector
#define RC_ID_SSS_ENCRINITVECT  (0x0204U)


// setEngineTime
#define RC_ID_SSS__SET_ENGINETIME   (0x0300U)

// setMaintenance
#define RC_ID_SSS__SET_MAINTENANCE  (0x0301U)

// performMaintenance
#define RC_ID_SSS__DO_MAINTENANCE   (0x0302U)

// setSelfdrivingTime
#define RC_ID_SSS__SET_SDTIME       (0x0303U)


// [satbrake] reset calibration to factory default
#define RC_ID_SSS__CAL_RESET        (0x0400U)

// [satbrake] pipes bleeding
#define RC_ID_SSS__BLEEDING         (0x0401U)

// [satbrake] all valves and pump test to check electrical connection
// must be verified manually by sound
#define RC_ID_SSS__SOUNDTEST         (0x0402U)

// [satbrake] high power test to check MCU reboot
#define RC_ID_SSS__POWERTEST         (0x0403U)


// mcuProtection
#define RC_LOCK_FLASH           (0xDEADU)

//------------------------------------------------------------------------------
// OBDTestIds range
//------------------------------------------------------------------------------
#define RC_ID_OBDRI_MIN         (0xE000U)
#define RC_ID_OBDRI_MAX         (0xE1FFU)

// DeployLoopRoutineID
#define RC_ID_DLRI              (0xE200U)

//------------------------------------------------------------------------------
// SafetySystemRoutineIDs range
//------------------------------------------------------------------------------
#define RC_ID_SASRI_MIN         (0xE201U)
#define RC_ID_SASRI_MAX         (0xE2FFU)

//------------------------------------------------------------------------------
// ISOSAEReserved range
//------------------------------------------------------------------------------
#define RC_ID_ISOSAERESRVD2_MIN (0xE300U)
#define RC_ID_ISOSAERESRVD2_MAX (0xEFFFU)

//------------------------------------------------------------------------------
// systemSupplierSpecific range
//------------------------------------------------------------------------------
#define RC_ID_SSS_MIN           (0xF000U)
#define RC_ID_SSS_MAX           (0xFEFFU)

// eraseMemory
#define RC_ID_EM                (0xFF00U)

// checkProgrammingDependencies
#define RC_ID_CPD               (0xFF01U)

// eraseMirrorMemoryDTCs
#define RC_ID_EMMDTC            (0xFF02U)

//------------------------------------------------------------------------------
// ISOSAEReserved range
//------------------------------------------------------------------------------
#define RC_ID_ISOSAERESRVD3_MIN (0xFF03U)
#define RC_ID_ISOSAERESRVD3_MAX (0xFFFFU)

/**
 * @brief @RC_LOCK_FLASH routine parameters
 * this value defines desired MCU protection level
 */
#define LOCK_FLASH_VAL_OFF      (0x1Cu)
#define LOCK_FLASH_VAL_LEV1     (0x24u)
#define LOCK_FLASH_VAL_LEV2     (0x36u)
#define LOCK_FLASH_VAL_LEV3     (0x5Fu)
#define LOCK_FLASH_VAL_FULL     (0xDFu)
