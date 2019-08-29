//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the NXP PN512 RFID reader
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRVRFIDREADERNXPPN512_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef DRVRFIDREADERNXPPN512_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               DRVRFIDREADERNXPPN512_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef NXP_PN512_COUNT
	#define NXP_PN512_COUNT             1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section

//STD lib include section

//COM lib include section

//APP include section
#include "DrvRfidTransceiverNxpPn512.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
/** \name Command Register Contents (0x01)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_RCVOFF       0x20U   /**< Switches the receiver on/off. */
#define PHHAL_HW_RC523_BIT_POWERDOWN    0x10U   /**< Switches Ic to Power Down mode. */
#define PHHAL_HW_RC523_CMD_IDLE         0x00U   /**< No action; cancels current command execution. */
#define PHHAL_HW_RC523_CMD_MEM          0x01U   /**< Copy configuration data from Fifo to internal Buffer or if Fifo empty vice versa. */
#define PHHAL_HW_RC523_CMD_RANDOMIDS    0x02U   /**< Replaces stored IDs with random ones. */
/** \brief Activate the CRC-Coprocessor.
*
* <b>Remark:</b> The result of the CRC calculation can be read from e.g. #PHHAL_HW_RC523_REG_CRCRESULT1.
*/
#define PHHAL_HW_RC523_CMD_CALCCRC      0x03U
#define PHHAL_HW_RC523_CMD_TRANSMIT     0x04U   /**< Transmits data from the FIFO buffer to Card. */
#define PHHAL_HW_RC523_CMD_NOCMDCHANGE  0x07U   /**< No change; Use together with e.g. #PHHAL_HW_RC523_BIT_POWERDOWN to keep current command-state. */
#define PHHAL_HW_RC523_CMD_RECEIVE      0x08U   /**< Activates the receiver circuitry. */
#define PHHAL_HW_RC523_CMD_TRANSCEIVE   0x0CU   /**< Like #PHHAL_HW_RC523_CMD_TRANSMIT but automatically activates the receiver after transmission is finished. */
#define PHHAL_HW_RC523_CMD_AUTOCOLL     0x0DU   /**< Handles Felica polling or MIFARE anticollision in Target mode. */
#define PHHAL_HW_RC523_CMD_AUTHENT      0x0EU   /**< Performs the Mifare Classic authentication (in Mifare Reader/Writer mode only). */
#define PHHAL_HW_RC523_CMD_SOFTRESET    0x0FU   /**< Resets the IC. */
#define PHHAL_HW_RC523_MASK_COMMAND     0x0FU   /**< Mask for Command-bits. */
/*@}*/

/** \name CommIEn/CommIrq Register Contents (0x02/0x04)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_SET          0x80U   /**< Bit position to set/clear dedicated IRQ bits. */
#define PHHAL_HW_RC523_BIT_IRQINV       0x80U   /**< Inverts the output of IRQ Pin. */
#define PHHAL_HW_RC523_BIT_TXI          0x40U   /**< Bit position for Transmit Interrupt Enable/Request. */
#define PHHAL_HW_RC523_BIT_RXI          0x20U   /**< Bit position for Receive Interrupt Enable/Request. */
#define PHHAL_HW_RC523_BIT_IDLEI        0x10U   /**< Bit position for Idle Interrupt Enable/Request. */
#define PHHAL_HW_RC523_BIT_HIALERTI     0x08U   /**< Bit position for HiAlert Interrupt Enable/Request. */
#define PHHAL_HW_RC523_BIT_LOALERTI     0x04U   /**< Bit position for LoAlert Interrupt Enable/Request. */
#define PHHAL_HW_RC523_BIT_ERRI         0x02U   /**< Bit position for Error Interrupt Enable/Request. */
#define PHHAL_HW_RC523_BIT_TIMERI       0x01U   /**< Bit position for Timer Interrupt Enable/Request. */
/*@}*/

/** \name DivIEn/DivIrq Register Contents (0x03/0x05)
*/
/*@{*/
/* #define PHHAL_HW_RC523_BIT_SET          0x80U */
#define PHHAL_HW_RC523_BIT_IRQPUSHPULL  0x80U   /**< Sets the IRQ pin to Push Pull mode. */
#define PHHAL_HW_RC523_BIT_MFINACT      0x10U   /**< Bit position for MfInAct Interrupt Enable/Request. */
#define PHHAL_HW_RC523_BIT_CRCI         0x04U   /**< Bit position for CRC Interrupt Enable/Request. */
#define PHHAL_HW_RC523_BIT_EXT_RF_ON    0x02U   /**< Bit position for External RF ON */
#define PHHAL_HW_RC523_BIT_EXT_RF_OFF   0x01U   /**< Bit position for External RF OFF */
/*@}*/

/** \name Error Register Contents (0x06)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_WRERR        0x80U   /**< Bit position for Write Access Error. */
#define PHHAL_HW_RC523_BIT_TEMPERR      0x40U   /**< Bit position for Temerature Error. */
#define PHHAL_HW_RC523_BIT_BUFFEROVFL   0x10U   /**< Bit position for Buffer Overflow Error. */
#define PHHAL_HW_RC523_BIT_COLLERR      0x08U   /**< Bit position for Collision Error. */
#define PHHAL_HW_RC523_BIT_CRCERR       0x04U   /**< Bit position for CRC Error. */
#define PHHAL_HW_RC523_BIT_PARITYERR    0x02U   /**< Bit position for Parity Error. */
#define PHHAL_HW_RC523_BIT_PROTERR      0x01U   /**< Bit position for Protocol Error. */
/*@}*/

/** \name Status1 Register Contents (0x07)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_CRCOK        0x40U   /**< Bit position for status CRC OK. */
#define PHHAL_HW_RC523_BIT_CRCREADY     0x20U   /**< Bit position for status CRC Ready. */
#define PHHAL_HW_RC523_BIT_IRQ          0x10U   /**< Bit position for status IRQ is active. */
#define PHHAL_HW_RC523_BIT_TRUNNUNG     0x08U   /**< Bit position for status Timer is running. */
#define PHHAL_HW_RC523_BIT_HIALERT      0x02U   /**< Bit position for status HiAlert. */
#define PHHAL_HW_RC523_BIT_LOALERT      0x01U   /**< Bit position for status LoAlert. */
/*@}*/

/** \name Status2 Register Contents (0x08)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_TEMPSENSOFF  	0x80U   /**< Bit position to switch Temperture sensors on/off. */
#define PHHAL_HW_RC523_BIT_I2CFORCEHS   	0x40U   /**< Bit position to forece High speed mode for I2C Interface. */
#define PHHAL_HW_RC523_BIT_TARGET_ACTIVATED	0x10U   /**< Bit position for reader status Target Activated. */
#define PHHAL_HW_RC523_BIT_CRYPTO1ON    	0x08U   /**< Bit position for reader status Crypto is on. */

/*@}*/

/** \name FifoLevel Register Contents (0x0A)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_FLUSHBUFFER  0x80U   /**< Clears Fifo buffer if set to 1. */
/*@}*/

/** \name WaterLevel Register Contents (0x0B)
*/
/*@{*/
#define PHHAL_HW_RC523_MASK_WATERLEVEL  0x3FU   /**< Bitmask for Waterlevel bits. */
/*@}*/

/** \name Control Register Contents (0x0C)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_TSTOPNOW     0x80U   /**< Stops timer if set to 1. */
#define PHHAL_HW_RC523_BIT_TSTARTNOW    0x40U   /**< Starts timer if set to 1. */
#define PHHAL_HW_RC523_BIT_INITIATOR    0x10U   /**< If set to 1, the IC acts as initiator, otherwise as target. */
#define PHHAL_HW_RC523_MASK_RXBITS      0x07U   /**< Bitmask for RxLast bits. */
/*@}*/

/** \name BitFraming Register Contents (0x0D)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_STARTSEND    0x80U   /**< Starts transmission in transceive command if set to 1. */
#define PHHAL_HW_RC523_MASK_TXBITS      0x07U   /**< Bitmask for TxLast bits. */
#define PHHAL_HW_RC523_MASK_RXALIGN     0x70U   /**< Bitmask for RxAlign bits. */
/*@}*/

/** \name Collision Register Contents (0x0E)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_VALUESAFTERCOLL  0x80U   /**< Activates mode to keep data after collision. */
#define PHHAL_HW_RC523_BIT_COLLPOSNOTVALID  0x20U   /**< Whether collision position is valid or not. */
#define PHHAL_HW_RC523_MASK_COLLPOS         0x1FU   /**< Bitmask for CollPos bits. */
/*@}*/

/** \name Mode Register Contents (0x11)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_MSBFIRST     0x80U   /**< Sets CRC coprocessor with MSB first. */
#define PHHAL_HW_RC523_BIT_TXWAITRF     0x20U   /**< Waits until RF is enabled until transmission is startet, else starts immideately. */
#define PHHAL_HW_RC523_BIT_POLMFIN      0x08U   /**< Inverts polarity of MfinActIrq. If bit is set to 1 IRQ occures when Sigin line is 0. */
#define PHHAL_HW_RC523_BIT_MODEDETOFF   0x04U   /**< Set to logic 1, the internal mode detector is switched off during #PHHAL_HW_RC523_CMD_AUTOCOLL. */
#define PHHAL_HW_RC523_MASK_CRCPRESET   0x03U   /**< Bitmask for CRCPreset bits. */
/*@}*/

/** \name TxMode / RxMode Register Contents (0x12/0x13)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_CRCEN        0x80U   /**< Activates transmit or receive CRC. */
#define PHHAL_HW_RC523_BIT_848KBPS      0x30U   /**< Activates speed of 848kbps. */
#define PHHAL_HW_RC523_BIT_424KBPS      0x20U   /**< Activates speed of 424kbps. */
#define PHHAL_HW_RC523_BIT_212KBPS      0x10U   /**< Activates speed of 212kbps. */
#define PHHAL_HW_RC523_BIT_106KBPS      0x00U   /**< Activates speed of 106kbps. */
#define PHHAL_HW_RC523_BIT_INVMOD       0x08U   /**< Activates inverted transmission mode. */
#define PHHAL_HW_RC523_BIT_RXNOERR      0x08U   /**< If set, receiver ignores less than 4 bits. */
#define PHHAL_HW_RC523_BIT_RXMULTIPLE   0x04U   /**< Activates reception mode for multiple responses. */
#define PHHAL_HW_RC523_BIT_TYPEB        0x03U   /**< Activates Type B communication mode. */
#define PHHAL_HW_RC523_BIT_FELICA       0x02U   /**< Activates Felica communication mode. */
#define PHHAL_HW_RC523_BIT_ACTIVE       0x01U   /**< Activates Active communication mode. */
#define PHHAL_HW_RC523_BIT_MIFARE       0x00U   /**< Activates Mifare communication mode. */
#define PHHAL_HW_RC523_BIT_MIFARE_CE    0x80U   /**< Activates Mifare communication mode. */
#define PHHAL_HW_RC523_MASK_SPEED       0x70U   /**< Bitmask for Tx/RxSpeed bits. */
#define PHHAL_HW_RC523_MASK_FRAMING     0x03U   /**< Bitmask for Tx/RxFraming bits. */
#define PHHAL_HW_RC523_BIT_AUTORF_OFF   0x80U   /**< Bitmask for Auto RF Off. */
#define PHHAL_HW_RC523_BIT_CA_ON        0x08U   /**< Bitmask for CA. */
#define PHHAL_HW_RC523_BIT_DETECT_SYNC  0x40U   /**< Bitmask for Detect Sync. */
#define PHHAL_HW_RC523_BIT_INITIAL_RF_ON 0x04U  /**< Bitmask for Initial RF on. */
#define PHHAL_HW_RC523_BIT_TX2RFAutoEN   0x02U  /**< Bitmask for Auto RF ON. */
#define PHHAL_HW_RC523_BIT_TX1RFAutoEN   0x1U   /**< Bitmask for Auto RF ON. */
/*@}*/

/** \name TxControl Register Contents (0x14)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_INVTX2ON     0x80U   /**< Inverts the Tx2 output if drivers are switched on. */
#define PHHAL_HW_RC523_BIT_INVTX1ON     0x40U   /**< Inverts the Tx1 output if drivers are switched on. */
#define PHHAL_HW_RC523_BIT_INVTX2OFF    0x20U   /**< Inverts the Tx2 output if drivers are switched off. */
#define PHHAL_HW_RC523_BIT_INVTX1OFF    0x10U   /**< Inverts the Tx1 output if drivers are switched off. */
#define PHHAL_HW_RC523_BIT_TX2CW        0x08U   /**< Does not modulate the Tx2 output, only constant wave. */
#define PHHAL_HW_RC523_BIT_CHECKRF      0x04U   /**< Set to logic 1, RF field cannot be activated if external RF field is detected. */
#define PHHAL_HW_RC523_BIT_TX2RFEN      0x02U   /**< Switches the driver for Tx2 pin on. */
#define PHHAL_HW_RC523_BIT_TX1RFEN      0x01U   /**< Switches the driver for Tx1 pin on. */
/*@}*/

/** \name TxASK Register Contents (0x15)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_FORCE100ASK  0x40U   /**< Activates 100%ASK mode independent of driver settings. */
/*@}*/

/** \name TxSel Register Contents (0x16)
*/
/*@{*/
#define PHHAL_HW_RC523_MASK_DRIVERSEL   0x30U   /**< Bitmask for DriverSel bits. */
#define PHHAL_HW_RC523_MASK_MFOUTSEL    0x0FU   /**< Bitmask for MfOutSel bits. */
/*@}*/

/** \name RxSel Register Contents (0x17)
*/
/*@{*/
#define PHHAL_HW_RC523_MASK_UARTSEL     0xC0U   /**< Bitmask for UartSel bits. */
#define PHHAL_HW_RC523_MASK_RXWAIT      0x3FU   /**< Bitmask for RxWait bits. */
/*@}*/

/** \name RxThreshold Register Contents (0x18)
*/
/*@{*/
#define PHHAL_HW_RC523_MASK_MINLEVEL    0xF0U   /**< Bitmask for MinLevel bits. */
#define PHHAL_HW_RC523_MASK_COLLEVEL    0x07U   /**< Bitmask for CollLevel bits. */
/*@}*/

/** \name Demod Register Contents (0x19)
*/
/*@{*/
/**
* If set and the lower bit of AddIQ is cleared, the receiving is fixed to I channel.\n
* If set and the lower bit of AddIQ is set, the receiving is fixed to Q channel.
*/
#define PHHAL_HW_RC523_BIT_FIXIQ        0x20U
#define PHHAL_HW_RC523_MASK_ADDIQ       0xC0U   /**< Bitmask for ADDIQ bits. */
#define PHHAL_HW_RC523_MASK_TAURCV      0x0CU   /**< Bitmask for TauRcv bits. */
#define PHHAL_HW_RC523_MASK_TAUSYNC     0x03U   /**< Bitmask for TauSync bits. */
/*@}*/

/** \name MfTx Register Contents (0x1C)
*/
/*@{*/
#define PHHAL_HW_RC523_MASK_TXWAIT      0x03U   /**< Bitmask for TxWait bits. */
#define PHHAL_HW_RC523_BIT_MFHALTED     0x04U   /**< Set to logic 1, Responds only for ALL_REQ ("52"). */
/*@}*/

/** \name MfRx Register Contents (0x1D)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_PARITYDISABLE    0x10U   /**< Disables the parity generation and sending independent from the mode. */
/*@}*/

/** \name RFCfg Register Contents (0x26)
*/
/*@{*/
#define PHHAL_HW_RC523_MASK_RXGAIN      0x70U   /**< Bitmask for RxGain bits. */
/*@}*/

/** \name GsN Register Contents (0x27)
*/
/*@{*/
#define PHHAL_HW_RC523_MASK_CWGSN       0xF0U   /**< Bitmask for CWGsN bits. */
#define PHHAL_HW_RC523_MASK_MODGSN      0x0FU   /**< Bitmask for ModGsN bits. */
/*@}*/

/** \name CWGsP Register Contents (0x28)
*/
/*@{*/
#define PHHAL_HW_RC523_MASK_CWGSP       0x3FU   /**< Bitmask for CWGsP bits. */
/*@}*/

/** \name ModGsP Register Contents (0x29)
*/
/*@{*/
#define PHHAL_HW_RC523_MASK_MODGSP      0x3FU   /**< Bitmask for ModGsP bits. */
/*@}*/

/** \name TMode Register Contents (0x2A)
*/
/*@{*/
#define PHHAL_HW_RC523_BIT_TAUTO            0x80U   /**< Sets the Timer start/stop conditions to Auto mode. */
#define PHHAL_HW_RC523_BIT_TAUTORESTART     0x10U   /**< Restarts the timer automatically after counting down to 0. */
#define PHHAL_HW_RC523_MASK_TGATED          0x60U   /**< Bitmask for TGated bits. */
#define PHHAL_HW_RC523_MASK_TPRESCALER_HI   0x0FU   /**< Bitmask for TPrescalerHi bits. */
/*@}*/
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    PN512_REG_PAGE                          = 0x00,
    PN512_REG_COMMAND                       = 0x01,
    PN512_REG_COMMIEN                       = 0x02,
    PN512_REG_DIVIEN                        = 0x03,
    PN512_REG_COMMIRQ                       = 0x04,
    PN512_REG_DIVIRQ                        = 0x05,
    PN512_REG_ERROR                         = 0x06,
    PN512_REG_STATUS1                       = 0x07,
    PN512_REG_STATUS2                       = 0x08,
    PN512_REG_FIFODATA                      = 0x09,
    PN512_REG_FIFOLEVEL                     = 0x0A,
    PN512_REG_WATERLEVEL                    = 0x0B,
    PN512_REG_CONTROL                       = 0x0C,
    PN512_REG_BITFRAMING                    = 0x0D,
    PN512_REG_COLL                          = 0x0E,
    PN512_REG_MODE                          = 0x11,
    PN512_REG_TXMODE                        = 0x12,
    PN512_REG_RXMODE                        = 0x13,
    PN512_REG_TXCONTROL                     = 0x14,
    PN512_REG_TXASK                         = 0x15,
    PN512_REG_TXSEL                         = 0x16,
    PN512_REG_RXSEL                         = 0x17,
    PN512_REG_RXTHRESHOLD                   = 0x18,
    PN512_REG_DEMOD                         = 0x19,
    PN512_REG_MFTX                          = 0x1C,
    PN512_REG_MFRX                          = 0x1D,
    PN512_REG_TYPEB                         = 0x1E,
    PN512_REG_SERIALSPEED                   = 0x1F,
    PN512_REG_CRCRESULT1                    = 0x21,
    PN512_REG_CRCRESULT2                    = 0x22,
    PN512_REG_GSNOFF                        = 0x23,
    PN512_REG_MODWIDTH                      = 0x24,
    PN512_REG_TXBITPHASE                    = 0x25,
    PN512_REG_RFCFG                         = 0x26,
    PN512_REG_GSN                           = 0x27,
    PN512_REG_CWGSP                         = 0x28,
    PN512_REG_MODGSP                        = 0x29,
    PN512_REG_TMODE                         = 0x2A,
    PN512_REG_TPRESCALER                    = 0x2B,
    PN512_REG_TRELOADHI                     = 0x2C,
    PN512_REG_TRELOADLO                     = 0x2D,
    PN512_REG_TCOUNTERVALHI                 = 0x2E,
    PN512_REG_TCOUNTERVALLO                 = 0x2F,
    PN512_REG_TESTSEL1                      = 0x31,
    PN512_REG_TESTSEL2                      = 0x32,
    PN512_REG_TESTPINEN                     = 0x33,
    PN512_REG_TESTPINVALUE                  = 0x34,
    PN512_REG_TESTBUS                       = 0x35,
    PN512_REG_AUTOTEST                      = 0x36,
    PN512_REG_VERSION                       = 0x37,
    PN512_REG_ANALOGTEST                    = 0x38,
    PN512_REG_TESTDAC1                      = 0x39,
    PN512_REG_TESTDAC2                      = 0x3A,
    PN512_REG_TESTADC                       = 0x3B
}
NXP_PN512_REGISTER;

typedef struct
{
    SPI_DEVICE_ID               spi_id;
    DRVGPIO_PIN_HNDL            not_reset_pin;
    DRVGPIO_PIN_HNDL            irq_pin;
}
NXP_PN512_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void InitChip(RFID_TRANSCEIVER_ID transceiver_id);
static void InitExtraRegisters(RFID_TRANSCEIVER_ID transceiver_id);
static void ProtocolApplyIso14443a(RFID_TRANSCEIVER_ID transceiver_id);
static void NxpPn512ResetChip(RFID_TRANSCEIVER_ID transceiver_id);
static void NxpPn512ClearAllIrqFields(RFID_TRANSCEIVER_ID transceiver_id);
static U8   NxpPn512SpiReadRegister(RFID_TRANSCEIVER_ID transceiver_id, NXP_PN512_REGISTER address);
static void NxpPn512SpiReadFifoBuffer(RFID_TRANSCEIVER_ID transceiver_id, U8* buffer_ptr, U8 length);
static void NxpPn512SpiWriteRegister(RFID_TRANSCEIVER_ID transceiver_id, NXP_PN512_REGISTER address, U8 byte);
static void NxpPn512SpiWriteToFifoBuffer(RFID_TRANSCEIVER_ID transceiver_id, U8* buffer_ptr, U8 length);
static U8   NxpPn512Transceive(RFID_TRANSCEIVER_ID transceiver_id, U8* transmit_buffer, U8 transmit_buffer_length, U8* receive_buffer, U8* receive_buffer_length);
static BOOL DrvRfidTransceiverNxpPn512_SetRfField(RFID_TRANSCEIVER_ID transceiver_id, BOOL on_off);
static RFID_ERROR DrvRfidTransceiverNxpPn512_Transceive(RFID_TRANSCEIVER_ID transceiver_id, U8* tx_buffer_ptr, U8 tx_buffer_length, U8* rx_buffer_ptr, U8* rx_buffer_length);
static BOOL DrvRfidTransceiverNxpPn512_Iso14443aPrepareReqA(RFID_TRANSCEIVER_ID transceiver_id);
static BOOL DrvRfidTransceiverNxpPn512_Iso14443aPrepareSelect(RFID_TRANSCEIVER_ID transceiver_id);
static BOOL DrvRfidTransceiverNxpPn512_Iso14443aPrepareAnticollision(RFID_TRANSCEIVER_ID transceiver_id);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_NxpPn512ReadRegisters(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static RFID_TRANSCEIVER_HOOK_LIST	        transceiver_hook_list;
static RFID_TRANSCEIVER_STRUCT              transceiver_struct[NXP_PN512_COUNT];
static NXP_PN512_CTRL_STRUCT                transceiver_ctrl_struct[NXP_PN512_COUNT];
static U8                                   transceiver_count;

static U8                       rx_buffer[30];
static U8                       tx_buffer[30];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void InitExtraRegisters(RFID_TRANSCEIVER_ID transceiver_id)
{
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TXASK, 0x40);
    
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_RFCFG, 0x59);
        
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_MODGSP, 0x3f);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TMODE, 0x80);
    
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TRELOADHI, 0xa);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TRELOADLO, 0x59);
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512ResetChip(RFID_TRANSCEIVER_ID transceiver_id)
{
    NXP_PN512_CTRL_STRUCT*  transceiver_ctrl_struct_ptr = &transceiver_ctrl_struct[transceiver_id];
    U16 i;
    
    DrvGpio_SetPin(transceiver_ctrl_struct_ptr->not_reset_pin, TRUE);
    // insert delay
    i = 0;
    while(i < 0xEFFF){i++;}
    DrvGpio_SetPin(transceiver_ctrl_struct_ptr->not_reset_pin, FALSE);
    // insert delay
    i = 0;
    while(i < 0xEFFF){i++;}
    DrvGpio_SetPin(transceiver_ctrl_struct_ptr->not_reset_pin, TRUE);
}
//------------------------------------------------------------------------------------------------//
static U8 NxpPn512SpiReadRegister(RFID_TRANSCEIVER_ID transceiver_id, NXP_PN512_REGISTER address)
{
    NXP_PN512_CTRL_STRUCT*  transceiver_ctrl_struct_ptr = &transceiver_ctrl_struct[transceiver_id];
    
    // bit 7        | bit 6 | bit 5 | bit 4 | bit 3 | bit 2 | bit 1 | bit 0
    // w = 0; r = 1 |                   A D D R E S S               |   0
    tx_buffer[0] = (0x80 | (address << 1)) & 0xFE;
    
    DrvSpiMasterDevice_Select(transceiver_ctrl_struct_ptr->spi_id);
    DrvSpiMasterDevice_WriteData(transceiver_ctrl_struct_ptr->spi_id, &tx_buffer[0], 1);
    DrvSpiMasterDevice_ReadData(transceiver_ctrl_struct_ptr->spi_id, &rx_buffer[0], 1);
    DrvSpiMasterDevice_Deselect(transceiver_ctrl_struct_ptr->spi_id);
    
    return rx_buffer[0];
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512SpiReadFifoBuffer(RFID_TRANSCEIVER_ID transceiver_id, U8* buffer_ptr, U8 length)
{
    NXP_PN512_CTRL_STRUCT*  transceiver_ctrl_struct_ptr = &transceiver_ctrl_struct[transceiver_id];
    
    tx_buffer[0] = (0x80 | (PN512_REG_FIFODATA << 1)) & 0xFE;
    
    DrvSpiMasterDevice_Select(transceiver_ctrl_struct_ptr->spi_id);
    DrvSpiMasterDevice_WriteData(transceiver_ctrl_struct_ptr->spi_id, &tx_buffer[0], 1);
    DrvSpiMasterDevice_ReadData(transceiver_ctrl_struct_ptr->spi_id, &buffer_ptr[0], length);
    DrvSpiMasterDevice_Deselect(transceiver_ctrl_struct_ptr->spi_id);
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512SpiWriteToFifoBuffer(RFID_TRANSCEIVER_ID transceiver_id, U8* buffer_ptr, U8 length)
{
    NXP_PN512_CTRL_STRUCT*  transceiver_ctrl_struct_ptr = &transceiver_ctrl_struct[transceiver_id];
    
    if(length > 30)
    {
        return;
    }
    
    tx_buffer[0] = (PN512_REG_FIFODATA << 1) & 0x7E;
    MEMCPY(&tx_buffer[1], buffer_ptr, length);
    
    DrvSpiMasterDevice_Select(transceiver_ctrl_struct_ptr->spi_id);
    DrvSpiMasterDevice_WriteData(transceiver_ctrl_struct_ptr->spi_id, &tx_buffer[0], length + 1);
    DrvSpiMasterDevice_Deselect(transceiver_ctrl_struct_ptr->spi_id);
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512SpiWriteRegister(RFID_TRANSCEIVER_ID transceiver_id, NXP_PN512_REGISTER address, U8 byte)
{
    NXP_PN512_CTRL_STRUCT*  transceiver_ctrl_struct_ptr = &transceiver_ctrl_struct[transceiver_id];
    
    tx_buffer[0] = (address << 1) & 0x7E;
    tx_buffer[1] = byte;
    
    DrvSpiMasterDevice_Select(transceiver_ctrl_struct_ptr->spi_id);
    DrvSpiMasterDevice_WriteData(transceiver_ctrl_struct_ptr->spi_id, &tx_buffer[0], 2);
    DrvSpiMasterDevice_Deselect(transceiver_ctrl_struct_ptr->spi_id);
}
//------------------------------------------------------------------------------------------------//
static U8 NxpPn512Transceive(RFID_TRANSCEIVER_ID transceiver_id, U8* transmit_buffer, U8 transmit_buffer_length, U8* receive_buffer, U8* receive_buffer_length)
{
    U8 temp_reg;
    U8 reg_commien;
    U8 reg_divien;
    U8 status = RFID_NO_ERROR;
    
    *receive_buffer_length = 0;
    
    // Terminate a possibly running command (PHHAL_HW_RC523_CMD_IDLE = 0x00)
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_COMMAND, 0);
    // Flush Fifo (PHHAL_HW_RC523_BIT_FLUSHBUFFER = 0x80)
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_FIFOLEVEL, 0x80);
    // Clear all IRQ fields
    NxpPn512ClearAllIrqFields(transceiver_id);
    // Fill Fifo with the transmit buffer
    NxpPn512SpiWriteToFifoBuffer(transceiver_id, transmit_buffer, transmit_buffer_length);
    // Write the transceive command
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_COMMAND, PHHAL_HW_RC523_CMD_TRANSCEIVE);
    // Initiate the transceive operation
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_BITFRAMING);
    temp_reg |= PHHAL_HW_RC523_BIT_STARTSEND;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_BITFRAMING, temp_reg);
    
    // Enable IRQ1 and IRQ1 interrupt sources
    reg_commien = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_COMMIEN);
    reg_commien &= PHHAL_HW_RC523_BIT_IRQINV;
    reg_commien |= PHHAL_HW_RC523_BIT_TXI | PHHAL_HW_RC523_BIT_IDLEI | PHHAL_HW_RC523_BIT_ERRI;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_COMMIEN, reg_commien);
    reg_divien = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_DIVIEN);
    reg_divien &= PHHAL_HW_RC523_BIT_IRQPUSHPULL;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_DIVIEN, reg_divien);
    
    // wait until an IRQ occurs
    do
    {
        temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_STATUS1);
    }
    while ((temp_reg & PHHAL_HW_RC523_BIT_IRQ) != PHHAL_HW_RC523_BIT_IRQ);
    
    // Clear IRQ0 & IRQ1 interrupt sources
    reg_commien &= PHHAL_HW_RC523_BIT_IRQINV;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_COMMIEN, reg_commien);
    reg_divien &= PHHAL_HW_RC523_BIT_IRQPUSHPULL;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_DIVIEN, reg_divien);
    
    // Read IRQ0 register
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_COMMIRQ);
    if((temp_reg & 0x20) != 0x20)
    {
        return RFID_UNKNOWN_ERROR;
    }
    else if((temp_reg & 0x02) == 0x02)
    {
        // A bit in the ERROR register is set
        temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_ERROR);
        LOG_DBG("ERROR reg: 0x%02h", PU8(temp_reg));
        if((temp_reg & 0x08) == 0x08)
        {
            return RFID_COLLISION_ERROR;
        }
        return RFID_UNKNOWN_ERROR;
    }
    
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_FIFOLEVEL);
    *receive_buffer_length = temp_reg;
    
    LOG_DBG("FIFO buffer: %d bytes stored", PU8(*receive_buffer_length));
    
    while(temp_reg > 0)
    {
        receive_buffer[*receive_buffer_length - temp_reg] = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_FIFODATA);
        LOG_DBG("FIFO byte: 0x%02h", PU8(temp_reg));
        temp_reg--;
    }
    
    return status;
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512ClearAllIrqFields(RFID_TRANSCEIVER_ID transceiver_id)
{
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_COMMIRQ, (U8)~(U8)PHHAL_HW_RC523_BIT_SET);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_DIVIRQ, (U8)~(U8)PHHAL_HW_RC523_BIT_SET);
}
//------------------------------------------------------------------------------------------------//
static void InitChip(RFID_TRANSCEIVER_ID transceiver_id)
{
    U8 temp_reg;
    
    // Read PN512 version
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_VERSION);
    LOG_DBG("NXP PN512 version: 0x%02h", PU8(temp_reg));
    
    // Switch on the field
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_TXCONTROL);
    temp_reg |= PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TXCONTROL, temp_reg);
    
    // Switch off the field
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_TXCONTROL);
    temp_reg &= ~(PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TXCONTROL, temp_reg);
    
    InitExtraRegisters(transceiver_id);
    
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_COMMIEN, 0x00);
}
//------------------------------------------------------------------------------------------------//
static void ProtocolApplyIso14443a(RFID_TRANSCEIVER_ID transceiver_id)
{
    U8 temp_reg;
    
    // Tx speed: 106kbit, passive communication 14443a/mifare mode
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TXMODE, PHHAL_HW_RC523_BIT_MIFARE);
    // Same for RX
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_RXMODE, PHHAL_HW_RC523_BIT_MIFARE);
    // These bits define the width of the Miller modulation
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_MODWIDTH, 0x26);
    // Signal strength config (PHHAL_HW_RC523_RXTHRESHOLD_I14443A = 0x55)
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_RXTHRESHOLD, 0x55);
    // Set initiator mode
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_CONTROL, PHHAL_HW_RC523_BIT_INITIATOR);
    // configure the gain factor to 23dB for Target and 38dB for Initiator
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_RFCFG, 0x59);
    // configure TX2 inverted of but do not change the field status
    temp_reg  = PHHAL_HW_RC523_BIT_TX2RFEN | PHHAL_HW_RC523_BIT_TX1RFEN;
    temp_reg |= PHHAL_HW_RC523_BIT_INVTX2ON | PHHAL_HW_RC523_BIT_CHECKRF;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_RFCFG, 0x59);
    // configure the RxSel Register
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_RXSEL, 0x80);
    // configure general settings
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_MODE, 0x00);
    // configure the conductance if no modulation is active
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_GSN, 0xFF);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_CWGSP, 0x3F);
    // configure the conductance for LoadModulation (PHHAL_HW_RC523_MODINDEX_TARGET = 0xF2)
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_GSNOFF, 0xF2);
    // reset bitframing register
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_BITFRAMING, 0x00);
    // configure the water level (PHHAL_HW_RC523_FIFOSIZE = 64)
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_WATERLEVEL, 64 - 8);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRfidTransceiverNxpPn512_SetProtocol(RFID_TRANSCEIVER_ID transceiver_id, RFID_PROTOCOL protocol)
{
    switch(protocol)
    {
    case RFID_PROTOCOL_ISO_14443A:
        // clear all IRQ0 and IRQ1 flags
        NxpPn512ClearAllIrqFields(transceiver_id);
        
        // set protocol
        ProtocolApplyIso14443a(transceiver_id);
        
        NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_COMMIEN, 0x00);
        
        // clear all IRQ0 and IRQ1 flags
        NxpPn512ClearAllIrqFields(transceiver_id);
        break;
    default:
        LOG_WRN("RFID protocol not supported");
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRfidTransceiverNxpPn512_SetCrc(RFID_TRANSCEIVER_ID transceiver_id, BOOL on_off)
{
    U8 reg_tx_mode;
    U8 reg_rx_mode;
    
    reg_tx_mode = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_TXMODE);
    reg_rx_mode = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_RXMODE);
    
    if(on_off)
    {
        reg_tx_mode |= (U8)(PHHAL_HW_RC523_BIT_CRCEN);
        reg_rx_mode |= (U8)(PHHAL_HW_RC523_BIT_CRCEN);
    }
    else
    {
        reg_tx_mode &= ~(U8)(PHHAL_HW_RC523_BIT_CRCEN);
        reg_rx_mode &= ~(U8)(PHHAL_HW_RC523_BIT_CRCEN);
    }
    
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TXMODE, reg_tx_mode);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_RXMODE, reg_rx_mode);
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRfidTransceiverNxpPn512_SetRfField(RFID_TRANSCEIVER_ID transceiver_id, BOOL on_off)
{
    U8 temp_reg;
    
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_TXCONTROL);
    if(on_off)
    {
        temp_reg |= PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN;
    }
    else
    {
        temp_reg &= ~(U8)(PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN);
    }
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TXCONTROL, temp_reg);
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static RFID_ERROR DrvRfidTransceiverNxpPn512_Transceive(RFID_TRANSCEIVER_ID transceiver_id, U8* tx_buffer_ptr, U8 tx_buffer_length, U8* rx_buffer_ptr, U8* rx_buffer_length)
{
    U8 status = RFID_NO_ERROR;
    
    status = NxpPn512Transceive(transceiver_id, tx_buffer_ptr, tx_buffer_length, rx_buffer_ptr, rx_buffer_length);
    
    return (RFID_ERROR)status;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRfidTransceiverNxpPn512_Iso14443aPrepareReqA(RFID_TRANSCEIVER_ID transceiver_id)
{
    U8 temp_reg;
    
    // Disable MIFARE(R) Crypto1
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_STATUS2);
    if(temp_reg != 0)
    {
        temp_reg &= ~(U8)(PHHAL_HW_RC523_BIT_CRYPTO1ON);
        NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_STATUS2, temp_reg);
    }
    
    // Reset default data rates
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_TXMODE);
    if(temp_reg != 0)
    {
        temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_SPEED);
        temp_reg |= PHHAL_HW_RC523_BIT_106KBPS;
        NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_TXMODE, temp_reg);
    }
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_MODWIDTH, 0x26);
    
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_RXMODE);
    if(temp_reg != 0)
    {
        temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_SPEED);
        temp_reg |= PHHAL_HW_RC523_BIT_106KBPS;
        NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_RXMODE, temp_reg);
    }
    
    // Set selection timeout
//    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
//        pDataParams->pHalDataParams,
//        PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,
//        PHPAL_I14443P3A_SELECTION_TIME_US + PHPAL_I14443P3A_EXT_TIME_US));

    // Set RxDeafTime to 8 Bits
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_RXSEL);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_RXWAIT);
    temp_reg |= 8;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_RXSEL, temp_reg);
    
    // Only 7 bits are valid
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_TXBITS);
    temp_reg |= 7;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_BITFRAMING, temp_reg);
    
    // enable analog part of driver
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_COMMAND);
    temp_reg &= ~0x20;
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_COMMAND, temp_reg);
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRfidTransceiverNxpPn512_Iso14443aPrepareSelect(RFID_TRANSCEIVER_ID transceiver_id)
{
    U8 temp_reg;
    U8 uid_number_of_valid_bits = 0x70;
    
    // Adjust Rx-Align
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_RXALIGN);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_BITFRAMING, (temp_reg | ((uid_number_of_valid_bits & 0x07) << 4)));
    
    // Adjust TxBits
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_TXBITS);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_BITFRAMING, (temp_reg | (uid_number_of_valid_bits & 0x07)));
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRfidTransceiverNxpPn512_Iso14443aExitSelect(RFID_TRANSCEIVER_ID transceiver_id)
{
    U8 temp_reg;
    
    // end select sequence
    // Reset RxAlignment
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_RXALIGN);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_BITFRAMING, temp_reg);
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRfidTransceiverNxpPn512_Iso14443aPrepareAnticollision(RFID_TRANSCEIVER_ID transceiver_id)
{ 
    U8 temp_reg;
    U8 uid_number_of_valid_bits = 0x20;
    
    // Adjust Rx-Align
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_RXALIGN);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_BITFRAMING, (temp_reg | ((uid_number_of_valid_bits & 0x07) << 4)));
    
    // Adjust TxBits
    temp_reg = NxpPn512SpiReadRegister(transceiver_id, PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_TXBITS);
    NxpPn512SpiWriteRegister(transceiver_id, PN512_REG_BITFRAMING, (temp_reg | (uid_number_of_valid_bits & 0x07)));
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_NxpPn512ReadRegisters(void)
{
    RFID_TRANSCEIVER_ID transceiver_id = (U8)CoreTerm_GetArgumentAsU32(0);
    U8 reg;
    U8 temp_reg;
    
    LOG_TRM("NXP PN512");
    
    for(reg = 0; reg <= 0x3F; reg++)
    {
        if((reg & 0xF) == 0x00)
        {
            LOG_TRM("Page %d", PU8(reg >> 4));
        }
        temp_reg = NxpPn512SpiReadRegister(transceiver_id, (NXP_PN512_REGISTER)reg);
        LOG_TRM("Reg 0x%02h - 0x%02h", PU8(reg), PU8(temp_reg));
        
        CoreLog_Flush();
    }
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvRfidTransceiverNxpPn512_Init(void)
{
    transceiver_hook_list.set_protocol_hook                     = DrvRfidTransceiverNxpPn512_SetProtocol;
    transceiver_hook_list.set_crc_hook                          = DrvRfidTransceiverNxpPn512_SetCrc;             
    transceiver_hook_list.set_rf_field_hook                     = DrvRfidTransceiverNxpPn512_SetRfField;
    transceiver_hook_list.transceive_hook                       = DrvRfidTransceiverNxpPn512_Transceive;
    transceiver_hook_list.iso14443a_prepare_reqa_hook           = DrvRfidTransceiverNxpPn512_Iso14443aPrepareReqA;
    transceiver_hook_list.iso14443a_prepare_anticollision_hook  = DrvRfidTransceiverNxpPn512_Iso14443aPrepareAnticollision;
    transceiver_hook_list.iso14443a_prepare_select_hook         = DrvRfidTransceiverNxpPn512_Iso14443aPrepareSelect;
    transceiver_hook_list.iso14443a_exit_select_hook            = DrvRfidTransceiverNxpPn512_Iso14443aExitSelect;
    
    transceiver_count = 0;
    
    MEMSET((VPTR)transceiver_struct, 0, SIZEOF(transceiver_struct));
    MEMSET((VPTR)transceiver_ctrl_struct, 0, SIZEOF(transceiver_ctrl_struct));
    
    MEMSET((VPTR)rx_buffer, 0, SIZEOF(rx_buffer));
    MEMSET((VPTR)tx_buffer, 0, SIZEOF(tx_buffer));
    
#if (TERM_LEVEL > TERM_LEVEL_NONE)
    CoreTerm_RegisterCommand("NxpPn512ReadRegisters" , "Read out registers of chip x", 1, Command_NxpPn512ReadRegisters, TRUE);
#endif
}
//------------------------------------------------------------------------------------------------//
RFID_TRANSCEIVER_HNDL DrvRfidTransceiverNxpPn512_Register(SPI_DEVICE_ID spi_device_id, DRVGPIO_PIN_HNDL not_reset_pin_hndl, DRVGPIO_PIN_HNDL irq_pin_hndl)
{
    RFID_TRANSCEIVER_HNDL   transceiver_hndl = &transceiver_struct[transceiver_count];
    NXP_PN512_CTRL_STRUCT*  transceiver_ctrl_struct_ptr = &transceiver_ctrl_struct[transceiver_count];
    SPI_CONFIG_STRUCT       spi_config_struct = {500000, MODE_0, 8, FALSE};
    
    if(transceiver_count < NXP_PN512_COUNT)
    {
        transceiver_ctrl_struct_ptr->spi_id        = spi_device_id;
        transceiver_ctrl_struct_ptr->not_reset_pin = not_reset_pin_hndl;
        transceiver_ctrl_struct_ptr->irq_pin       = irq_pin_hndl;
        
        // Start driver with a hard reset
        DrvGpio_SetPin(transceiver_ctrl_struct_ptr->not_reset_pin, TRUE);
        DrvGpio_SetPin(transceiver_ctrl_struct_ptr->not_reset_pin, FALSE);
        DrvGpio_SetPin(transceiver_ctrl_struct_ptr->not_reset_pin, TRUE);
        DrvSpiMasterDevice_Config(transceiver_ctrl_struct_ptr->spi_id, &spi_config_struct);
        
        transceiver_hndl->hook_list_ptr  = &transceiver_hook_list;
        transceiver_hndl->transceiver_id = transceiver_count;
        transceiver_count++;
        
        // Start with resetting the chip
        NxpPn512ResetChip(transceiver_hndl->transceiver_id);
        InitChip(transceiver_hndl->transceiver_id);
        
        return transceiver_hndl;
    }
    LOG_WRN("NXP_PN512_COUNT overrun");
    return NULL;
}
//================================================================================================//