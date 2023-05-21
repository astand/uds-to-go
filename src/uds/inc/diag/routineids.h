#pragma once

//**************************************************************************************************
// This file contains definitions for common routine IDS
//**************************************************************************************************

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

