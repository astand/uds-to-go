#pragma once

// DiagnosticSessionControl Request SID
#define DSC_SF_DS         (0x01U)
#define DSC_SF_PRGS       (0x02U)
#define DSC_SF_EXTDS      (0x03U)

// SecurityAcces Request SID
#define SA_SF_RSD_LEV0    (0x01U)
#define SA_SF_SK_LEV0     (0x02U)
#define SA_SF_RSD_LEV1    (0x03U)
#define SA_SF_SK_LEV1     (0x04U)
#define SA_SF_RSD_LEV2    (0x05U)
#define SA_SF_SK_LEV2     (0x06U)
#define SA_SF_RSD_LEV3    (0x07U)
#define SA_SF_SK_LEV3     (0x08U)
#define SA_SF_TOTAL       (8)


// ReadDTCInformation Request SID
// reportNumberOfDTCByStatusMask
#define RDTCI_SF_RNODTCBSM      (0x01U)
// reportDTCByStatusMask
#define RDTCI_SF_RDTCBSM        (0x02U)
// reportMirrorMemoryDTCByStatusMask
#define RDTCI_SF_RMMDTCBSM      (0x0FU)
// reportNumberOfMirrorMemoryDTCByStatusMask
#define RDTCI_SF_RNOMMDTCBSM    (0x11U)
// reportNumberOfEmissionsOBDDTCByStatusMask
#define RDTCI_SF_RNOOEBDDTCBSM  (0x12U)
// reportEmissionsOBDDTCByStatusMask
#define RDTCI_SF_ROBDDTCBSM     (0x13U)
//  reportSupportedDTC
#define RDTCI_SF_RSUPDTC        (0x0AU)
// reportDTCExtDataRecordByDTCNumber
#define RDTCI_SF_EXTDATA_BY_DTCCODE   (0x06U)

// reportDTCExtDataRecordByRecordNumber
#define RDTCI_SF_EXTDATA_BY_RECNUM    (0x16U)

// reportDTCSnapshotIdentification
// See 11.3.5.5 Example #4 - ReadDTCInformation in ISO 14229-3 (page 228)
#define RDTCI_SF_ALL_SNSHT_IDS        (0x03U)

// reportDTCSnapshotRecordByDTCNumber
// See 11.3.5.6 Example #5 - ReadDTCInformation in ISO 14229-3 (page 229)
#define RDTCI_SF_DTC_SNSHT_NUM        (0x04U)

//  ControlDTCSetting Request SID
#define CDTCS_SF_ON             (0x01U)
#define CDTCS_SF_OFF            (0x02U)

// RoutineControl (0x31) service
// start routine
#define RC_SF_STR               (0x01U)
// stop routine
#define RC_SF_STPR              (0x02U)
// request routine results
#define RC_SF_RRR               (0x03U)

// enableRxAndTx
#define CC_SF_ERXTX             (0x00U)
// disableRxAndEnableTx
#define CC_SF_ERXDTX            (0x01U)
// disableRxAndTx
#define CC_SF_DRXTX             (0x03U)


//  inputOutputControlParameter definitions ISO14229-1 E.1 p 374
// returnControlToECU U
#define IOCP_RCTECU   0x00
// resetToDefault U
#define IOCP_RTD      0x01
// freezeCurrentState U
#define IOCP_FCS      0x02
// shortTermAdjustment U
#define IOCP_STA      0x03
