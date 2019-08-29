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
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section

//STD lib include section

//COM lib include section

//APP include section
#include "DrvRfidReaderNxpPn512.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
/** \name Register definitions of Page 0
*  Following all register defintions of Page 0.
*/
/*@{*/
/** \brief Page register.
*/
#define PHHAL_HW_RC523_REG_PAGE         0x00U

/** \brief Command register.
*
* Contains Command bits, PowerDown bit and bit to switch receiver off.
*/
#define PHHAL_HW_RC523_REG_COMMAND      0x01U

/** \brief CommIEn register.
*
* Contains Communication interrupt enable bits and bit for Interrupt inversion.
*/
#define PHHAL_HW_RC523_REG_COMMIEN      0x02U

/** \brief DivIEn register.
*
* Contains RfOn, RfOff, CRC and Mode Interrupt enable and bit to switch Interrupt pin to PushPull mode.
*/
#define PHHAL_HW_RC523_REG_DIVIEN       0x03U

/** \brief CommIrq register.
*
* Contains Communication interrupt request bits.
*/
#define PHHAL_HW_RC523_REG_COMMIRQ      0x04U

/** \brief DivIrq register.
*
* Contains RfOn, RfOff, CRC and Mode Interrupt request.
*/
#define PHHAL_HW_RC523_REG_DIVIRQ       0x05U

/** \brief Error register.
*
* Contains Protocol, Parity, CRC, Collision, Buffer overflow, Temperature and RF error flags.
*/
#define PHHAL_HW_RC523_REG_ERROR        0x06U

/** \brief Status1 register.
*
* Contains status information about Lo- and HiAlert, RF-field on, Timer, Interrupt request and CRC status.
*/
#define PHHAL_HW_RC523_REG_STATUS1      0x07U

/** \brief Status2 register.
*
* Contains information about internal states and Temperature sensor switch.
*/
#define PHHAL_HW_RC523_REG_STATUS2      0x08U

/** \brief Fifo register.
*
* Read/Write Fifo. Writing to register increments the Fifo level, reading decrements it.
*/
#define PHHAL_HW_RC523_REG_FIFODATA     0x09U

/** \brief FifoLevel register.
*
* Contains the actual level (number of bytes) of the Fifo.
*/
#define PHHAL_HW_RC523_REG_FIFOLEVEL    0x0AU

/** \brief WaterLevel register.
*
* Contains the Waterlevel value for the Fifo.
*/
#define PHHAL_HW_RC523_REG_WATERLEVEL   0x0BU

/** \brief Control register.
*
* Contains information about last received bits, and to Start and stop the Timer unit.
*/
#define PHHAL_HW_RC523_REG_CONTROL      0x0CU

/** \brief Bitframing register.
*
* Contains information of last bits to send, to align received bits in Fifo and activate sending.
*/
#define PHHAL_HW_RC523_REG_BITFRAMING   0x0DU

/** \brief Collision register.
*
* Contains all necessary bits for Collision handling.
*/
#define PHHAL_HW_RC523_REG_COLL         0x0EU
/*@}*/

/** \name Register definitions of Page 1
*  Following all register defintions of Page 1.
*/
/*@{*/
/** \brief Mode register.
*
* Contains bits for auto wait on Rf and MSB first for CRC calculation.
*/
#define PHHAL_HW_RC523_REG_MODE         0x11U

/** \brief TxMode register.
*
* Contains Transmit Framing, Speed, CRC enable, bit for inverse mode and TXMix bit.
*/
#define PHHAL_HW_RC523_REG_TXMODE       0x12U

/** \brief RxMode register.
*
* Contains Transmit Framing, Speed, CRC enable, bit for multiple receive and to filter errors.
*/
#define PHHAL_HW_RC523_REG_RXMODE       0x13U

/** \brief TxControl register.
*
* Contains bits to activate and configure Tx1 and Tx2 and bit to activate 100% modulation.
*/
#define PHHAL_HW_RC523_REG_TXCONTROL    0x14U

/** \brief TxAsk register.
*
* Contains Tx ASK settings.
*/
#define PHHAL_HW_RC523_REG_TXASK        0x15U

/** \brief TxSel register.
*
* Contains SigoutSel, DriverSel and LoadModSel bits.
*/
#define PHHAL_HW_RC523_REG_TXSEL        0x16U

/** \brief RxSel register.
*
* Contains UartSel and RxWait bits.
*/
#define PHHAL_HW_RC523_REG_RXSEL        0x17U

/** \brief RxThreshold register.
*
* Contains MinLevel and CollLevel for detection.
*/
#define PHHAL_HW_RC523_REG_RXTHRESHOLD  0x18U

/** \brief Demod register.
*
* Contains bits for time constants, hysteresis and IQ demodulator settings.
*/
#define PHHAL_HW_RC523_REG_DEMOD        0x19U

/** \brief MfTx register.
*
* Mifare transmission settings.
*/
#define PHHAL_HW_RC523_REG_MFTX         0x1CU

/** \brief MfRx register.
*
* Mifare reception settings.
*/
#define PHHAL_HW_RC523_REG_MFRX         0x1DU

/** \brief TypeB register.
*
* Used for ISO/IEC 14443 Type B configuration.
*/
#define PHHAL_HW_RC523_REG_TYPEB        0x1EU

/** \brief SerialSpeed register.
*
* Contains speed settings for RS232 interface.
*/
#define PHHAL_HW_RC523_REG_SERIALSPEED  0x1FU
/*@}*/

/** \name Register definitions of Page 2
*  Following all register defintions of Page 2.
*/
/*@{*/
/** \brief CRCResult1 register.
*
* Contains MSByte of CRC Result.
*/
#define PHHAL_HW_RC523_REG_CRCRESULT1   0x21U

/** \brief CRCResult2 register.
*
* Contains LSByte of CRC Result.
*/
#define PHHAL_HW_RC523_REG_CRCRESULT2   0x22U

/** \brief GSNOff register.
*
* Contains the conductance and the modulation settings for the N-MOS transistor for LoadModulation.
*/
#define PHHAL_HW_RC523_REG_GSNOFF       0x23U

/** \brief ModWidth register.
*
* Contains modulation width setting.
*/
#define PHHAL_HW_RC523_REG_MODWIDTH     0x24U
#define PHHAL_HW_RC523_REG_TXBITPHASE     0x25U

/** \brief RfCfg register.
*
* Contains sensitivity of Rf Level detector, the receiver gain factor and the RfLevelAmp.
*/
#define PHHAL_HW_RC523_REG_RFCFG        0x26U

/** \brief GSN register.
*
* Contains the conductance and the modulation settings for the N-MOS transistor during active modulation.
*/
#define PHHAL_HW_RC523_REG_GSN          0x27U

/** \brief CWGSP register.
*
* Contains the conductance for the P-Mos transistor.
*/
#define PHHAL_HW_RC523_REG_CWGSP        0x28U

/** \brief MODGSP register.
*
* Contains the modulation index for the PMos transistor.
*/
#define PHHAL_HW_RC523_REG_MODGSP       0x29U

/** \brief TMode register.
*
* Contains all settings for the timer and the highest 4 bits of the prescaler.
*/
#define PHHAL_HW_RC523_REG_TMODE        0x2AU

/** \brief TPrescaler register.
*
* Contais the lowest byte of the prescaler.
*/
#define PHHAL_HW_RC523_REG_TPRESCALER   0x2BU

/** \brief TReloadHi register.
*
* Contains the high byte of the reload value.
*/
#define PHHAL_HW_RC523_REG_TRELOADHI    0x2CU

/** \brief TReloadLo register.
*
* Contains the low byte of the reload value.
*/
#define PHHAL_HW_RC523_REG_TRELOADLO    0x2DU

/** \brief TCounterValHi register.
*
* Contains the high byte of the counter value.
*/
#define PHHAL_HW_RC523_REG_TCOUNTERVALHI    0x2EU

/** \brief TCounterValLo register.
*
* Contains the low byte of the counter value.
*/
#define PHHAL_HW_RC523_REG_TCOUNTERVALLO    0x2FU
/*@}*/

/** \name Register definitions of Page 3
*  Following all register defintions of Page 3.
*/
/*@{*/
#define PHHAL_HW_RC523_REG_TESTSEL1         0x31U   /**< Test register. */
#define PHHAL_HW_RC523_REG_TESTSEL2         0x32U   /**< Test register. */
#define PHHAL_HW_RC523_REG_TESTPINEN        0x33U   /**< Test register. */
#define PHHAL_HW_RC523_REG_TESTPINVALUE     0x34U   /**< Test register. */
#define PHHAL_HW_RC523_REG_TESTBUS          0x35U   /**< Test register .*/
#define PHHAL_HW_RC523_REG_AUTOTEST         0x36U   /**< Test register. */
#define PHHAL_HW_RC523_REG_VERSION          0x37U   /**< Contains the product number and the version. */
#define PHHAL_HW_RC523_REG_ANALOGTEST       0x38U   /**< Test register. */
#define PHHAL_HW_RC523_REG_TESTDAC1         0x39U   /**< Test register. */
#define PHHAL_HW_RC523_REG_TESTDAC2         0x3AU   /**< Test register. */
#define PHHAL_HW_RC523_REG_TESTADC          0x3BU   /**< Test register. */
/*@}*/

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
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void NxpPn512ResetChip(void);
static U8   NxpPn512SpiReadRegister(NXP_PN512_REGISTER address);
static void NxpPn512SpiReadFifoBuffer(U8* buffer_ptr, U8 length);
static void NxpPn512SpiWriteRegister(NXP_PN512_REGISTER address, U8 byte);
static void NxpPn512SpiWriteCommandToFifoBuffer(U8 command, U8 length);
static void NxpPn512SpiWriteToFifoBuffer(U8* buffer_ptr, U8 length);
static void NxpPn512Transceive(U8* transmit_buffer, U8 transmit_buffer_length, U8* receive_buffer, U8* receive_buffer_length);
static void NxpPn512ClearAllIrqFields(void);
static void NxpPn512SetCrc(BOOL enable);
static void InitChip(void);
static void ProtocolApplyIso14443a(void);
static BOOL Iso14443AAntiCollisionLoop(U8* user_id, U8* user_id_length);
static BOOL Iso14443AntiCollision(U8 cascade_level,
                                  U8 uid_number_of_valid_bits,
                                  U8* uid_number_of_valid_out_bits,
                                  U8* uid_in_ptr,
                                  U8* uid_out_ptr);
static BOOL Iso14443RequestA(U8* atqa, U8* atqa_length);
static void Iso14443Select(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SPI_DEVICE_ID            spi_id;
static DRVGPIO_PIN_HNDL         not_reset_pin;
static DRVGPIO_PIN_HNDL         irq_pin;

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
static void InitExtraRegisters(void)
{
    NxpPn512SpiWriteRegister(PN512_REG_TXASK, 0x40);
    
    NxpPn512SpiWriteRegister(PN512_REG_RFCFG, 0x59);
        
    NxpPn512SpiWriteRegister(PN512_REG_MODGSP, 0x3f);
    NxpPn512SpiWriteRegister(PN512_REG_TMODE, 0x80);
    
    NxpPn512SpiWriteRegister(PN512_REG_TRELOADHI, 0xa);
    NxpPn512SpiWriteRegister(PN512_REG_TRELOADLO, 0x59);
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512ResetChip(void)
{
    U16 i;
    
    DrvGpio_SetPin(not_reset_pin, TRUE);
    // insert delay
    i = 0;
    while(i < 0xEFFF){i++;}
    DrvGpio_SetPin(not_reset_pin, FALSE);
    // insert delay
    i = 0;
    while(i < 0xEFFF){i++;}
    DrvGpio_SetPin(not_reset_pin, TRUE);
}
//------------------------------------------------------------------------------------------------//
static U8 NxpPn512SpiReadRegister(NXP_PN512_REGISTER address)
{
    // bit 7        | bit 6 | bit 5 | bit 4 | bit 3 | bit 2 | bit 1 | bit 0
    // w = 0; r = 1 |                   A D D R E S S               |   0
    tx_buffer[0] = (0x80 | (address << 1)) & 0xFE;
    
    DrvSpiMasterDevice_Select(spi_id);
    DrvSpiMasterDevice_WriteData(spi_id, &tx_buffer[0], 1);
    DrvSpiMasterDevice_ReadData(spi_id, &rx_buffer[0], 1);
    DrvSpiMasterDevice_Deselect(spi_id);
    
    return rx_buffer[0];
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512SpiReadFifoBuffer(U8* buffer_ptr, U8 length)
{
    tx_buffer[0] = (0x80 | (PN512_REG_FIFODATA << 1)) & 0xFE;
    
    DrvSpiMasterDevice_Select(spi_id);
    DrvSpiMasterDevice_WriteData(spi_id, &tx_buffer[0], 1);
    DrvSpiMasterDevice_ReadData(spi_id, &buffer_ptr[0], length);
    DrvSpiMasterDevice_Deselect(spi_id);
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512SpiWriteToFifoBuffer(U8* buffer_ptr, U8 length)
{
    tx_buffer[0] = (PN512_REG_FIFODATA << 1) & 0x7E;
    MEMCPY(&tx_buffer[1], buffer_ptr, length);
    
    DrvSpiMasterDevice_Select(spi_id);
    DrvSpiMasterDevice_WriteData(spi_id, &tx_buffer[0], length + 1);
    DrvSpiMasterDevice_Deselect(spi_id);
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512SpiWriteRegister(NXP_PN512_REGISTER address, U8 byte)
{
    tx_buffer[0] = (address << 1) & 0x7E;
    tx_buffer[1] = byte;
    
    DrvSpiMasterDevice_Select(spi_id);
    DrvSpiMasterDevice_WriteData(spi_id, &tx_buffer[0], 2);
    DrvSpiMasterDevice_Deselect(spi_id);
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512Transceive(U8* transmit_buffer, U8 transmit_buffer_length, U8* receive_buffer, U8* receive_buffer_length)
{
    U8 temp_reg;
    U8 reg_commien;
    U8 reg_divien;
    
    // Terminate a possibly running command (PHHAL_HW_RC523_CMD_IDLE = 0x00)
    NxpPn512SpiWriteRegister(PN512_REG_COMMAND, 0);
    // Flush Fifo (PHHAL_HW_RC523_BIT_FLUSHBUFFER = 0x80)
    NxpPn512SpiWriteRegister(PN512_REG_FIFOLEVEL, 0x80);
    // Clear all IRQ fields
    NxpPn512ClearAllIrqFields();
    // Fill Fifo with the transmit buffer
    NxpPn512SpiWriteToFifoBuffer(transmit_buffer, transmit_buffer_length);
    // Write the transceive command
    NxpPn512SpiWriteRegister(PN512_REG_COMMAND, PHHAL_HW_RC523_CMD_TRANSCEIVE);
    // Initiate the transceive operation
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_BITFRAMING);
    temp_reg |= PHHAL_HW_RC523_BIT_STARTSEND;
    NxpPn512SpiWriteRegister(PN512_REG_BITFRAMING, temp_reg);
    
    // Enable IRQ1 and IRQ1 interrupt sources
    reg_commien = NxpPn512SpiReadRegister(PN512_REG_COMMIEN);
    reg_commien &= PHHAL_HW_RC523_BIT_IRQINV;
    reg_commien |= PHHAL_HW_RC523_BIT_TXI | PHHAL_HW_RC523_BIT_IDLEI | PHHAL_HW_RC523_BIT_ERRI;
    NxpPn512SpiWriteRegister(PN512_REG_COMMIEN, reg_commien);
    reg_divien = NxpPn512SpiReadRegister(PN512_REG_DIVIEN);
    reg_divien &= PHHAL_HW_RC523_BIT_IRQPUSHPULL;
    NxpPn512SpiWriteRegister(PN512_REG_DIVIEN, reg_divien);
    
    // wait until an IRQ occurs
    do
    {
        temp_reg = NxpPn512SpiReadRegister(PN512_REG_STATUS1);
    }
    while ((temp_reg & PHHAL_HW_RC523_BIT_IRQ) != PHHAL_HW_RC523_BIT_IRQ);
    
    // Clear IRQ0 & IRQ1 interrupt sources
    reg_commien &= PHHAL_HW_RC523_BIT_IRQINV;
    NxpPn512SpiWriteRegister(PN512_REG_COMMIEN, reg_commien);
    reg_divien &= PHHAL_HW_RC523_BIT_IRQPUSHPULL;
    NxpPn512SpiWriteRegister(PN512_REG_DIVIEN, reg_divien);
    
    // Read IRQ0 register
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_COMMIRQ);
    if((temp_reg & 0x20) != 0x20)
    {
        *receive_buffer_length = 0;
        return;
    }
    
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_FIFOLEVEL);
    *receive_buffer_length = temp_reg;
    
    LOG_DBG("FIFO buffer: %d bytes stored", PU8(*receive_buffer_length));
    
    while(temp_reg > 0)
    {
        receive_buffer[*receive_buffer_length - temp_reg] = NxpPn512SpiReadRegister(PN512_REG_FIFODATA);
        LOG_DBG("FIFO byte: 0x%02h", PU8(temp_reg));
        temp_reg--;
    }
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512ClearAllIrqFields(void)
{
    NxpPn512SpiWriteRegister(PN512_REG_COMMIRQ, (U8)~(U8)PHHAL_HW_RC523_BIT_SET);
    NxpPn512SpiWriteRegister(PN512_REG_DIVIRQ, (U8)~(U8)PHHAL_HW_RC523_BIT_SET);
}
//------------------------------------------------------------------------------------------------//
static void NxpPn512SetCrc(BOOL enable)
{
    U8 reg_tx_mode;
    U8 reg_rx_mode;
    
    reg_tx_mode = NxpPn512SpiReadRegister(PN512_REG_TXMODE);
    reg_rx_mode = NxpPn512SpiReadRegister(PN512_REG_RXMODE);
    
    if(enable)
    {
        reg_tx_mode |= (U8)(PHHAL_HW_RC523_BIT_CRCEN);
        reg_rx_mode |= (U8)(PHHAL_HW_RC523_BIT_CRCEN);
    }
    else
    {
        reg_tx_mode &= ~(U8)(PHHAL_HW_RC523_BIT_CRCEN);
        reg_rx_mode &= ~(U8)(PHHAL_HW_RC523_BIT_CRCEN);
    }
    
    NxpPn512SpiWriteRegister(PN512_REG_TXMODE, reg_tx_mode);
    NxpPn512SpiWriteRegister(PN512_REG_RXMODE, reg_rx_mode);
}
//------------------------------------------------------------------------------------------------//
static void InitChip(void)
{
    U8 temp_reg;
    
    // Read PN512 version
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_VERSION);
    LOG_DBG("NXP PN512 version: 0x%02h", PU8(temp_reg));
    
    // Switch on the field
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_TXCONTROL);
    temp_reg |= PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN;
    NxpPn512SpiWriteRegister(PN512_REG_TXCONTROL, temp_reg);
    
    // TEST
    
    // Switch off the field
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_TXCONTROL);
    temp_reg &= ~(PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN);
    NxpPn512SpiWriteRegister(PN512_REG_TXCONTROL, temp_reg);
    
    InitExtraRegisters();
    
    // apply ISO14443a
    //ProtocolApplyIso14443a();
    
    NxpPn512SpiWriteRegister(PN512_REG_COMMIEN, 0x00);
}
//------------------------------------------------------------------------------------------------//
static void ProtocolApplyIso14443a(void)
{
    U8 temp_reg;
    
    // Tx speed: 106kbit, passive communication 14443a/mifare mode
    NxpPn512SpiWriteRegister(PN512_REG_TXMODE, PHHAL_HW_RC523_BIT_MIFARE);
    // Same for RX
    NxpPn512SpiWriteRegister(PN512_REG_RXMODE, PHHAL_HW_RC523_BIT_MIFARE);
    // These bits define the width of the Miller modulation
    NxpPn512SpiWriteRegister(PN512_REG_MODWIDTH, 0x26);
    // Signal strength config (PHHAL_HW_RC523_RXTHRESHOLD_I14443A = 0x55)
    NxpPn512SpiWriteRegister(PN512_REG_RXTHRESHOLD, 0x55);
    // Set initiator mode
    NxpPn512SpiWriteRegister(PN512_REG_CONTROL, PHHAL_HW_RC523_BIT_INITIATOR);
    // configure the gain factor to 23dB for Target and 38dB for Initiator
    NxpPn512SpiWriteRegister(PN512_REG_RFCFG, 0x59);
    // configure TX2 inverted of but do not change the field status
#warning hier zit bug in, nog eens bekijken. resultaat wordt gelezen en niet direct ge-ord
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_TXCONTROL);
    temp_reg  = PHHAL_HW_RC523_BIT_TX2RFEN | PHHAL_HW_RC523_BIT_TX1RFEN;
    temp_reg |= PHHAL_HW_RC523_BIT_INVTX2ON | PHHAL_HW_RC523_BIT_CHECKRF;
    NxpPn512SpiWriteRegister(PN512_REG_RFCFG, 0x87);
    // configure the RxSel Register
    NxpPn512SpiWriteRegister(PN512_REG_RXSEL, 0x80);
    // configure general settings
    NxpPn512SpiWriteRegister(PN512_REG_MODE, 0x00);
    // configure the conductance if no modulation is active
    NxpPn512SpiWriteRegister(PN512_REG_GSN, 0xFF);
    NxpPn512SpiWriteRegister(PN512_REG_CWGSP, 0x3F);
    // configure the conductance for LoadModulation (PHHAL_HW_RC523_MODINDEX_TARGET = 0xF2)
    NxpPn512SpiWriteRegister(PN512_REG_GSNOFF, 0xF2);
    // reset bitframing register
    NxpPn512SpiWriteRegister(PN512_REG_BITFRAMING, 0x00);
    // configure the water level (PHHAL_HW_RC523_FIFOSIZE = 64)
    NxpPn512SpiWriteRegister(PN512_REG_WATERLEVEL, 64 - 8);
}
//------------------------------------------------------------------------------------------------//
static BOOL Iso14443RequestA(U8* atqa, U8* atqa_length)
{
    U8 transmit_buffer = 0x26; // ReqA command (PHPAL_I14443P3A_REQUEST_CMD = 0x26)
    U8 temp_reg;
    
    // Disable MIFARE(R) Crypto1
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_STATUS2);
    if(temp_reg != 0)
    {
        temp_reg &= ~(U8)(PHHAL_HW_RC523_BIT_CRYPTO1ON);
        NxpPn512SpiWriteRegister(PN512_REG_STATUS2, temp_reg);
    }
    
    // Reset default data rates
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_TXMODE);
    if(temp_reg != 0)
    {
        temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_SPEED);
        temp_reg |= PHHAL_HW_RC523_BIT_106KBPS;
        NxpPn512SpiWriteRegister(PN512_REG_TXMODE, temp_reg);
    }
    NxpPn512SpiWriteRegister(PN512_REG_MODWIDTH, 0x26);
    
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_RXMODE);
    if(temp_reg != 0)
    {
        temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_SPEED);
        temp_reg |= PHHAL_HW_RC523_BIT_106KBPS;
        NxpPn512SpiWriteRegister(PN512_REG_RXMODE, temp_reg);
    }
    
    // Set selection timeout
//    PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_SetConfig(
//        pDataParams->pHalDataParams,
//        PHHAL_HW_CONFIG_TIMEOUT_VALUE_US,
//        PHPAL_I14443P3A_SELECTION_TIME_US + PHPAL_I14443P3A_EXT_TIME_US));

    // Set RxDeafTime to 8 Bits
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_RXSEL);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_RXWAIT);
    temp_reg |= 8;
    NxpPn512SpiWriteRegister(PN512_REG_RXSEL, temp_reg);
    
    // Switch off CRC
    NxpPn512SetCrc(FALSE);
    
    // Only 7 bits are valid
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_TXBITS);
    temp_reg |= 7;
    NxpPn512SpiWriteRegister(PN512_REG_BITFRAMING, temp_reg);
    
    // enable analog part of driver
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_COMMAND);
    temp_reg &= ~0x20;
    NxpPn512SpiWriteRegister(PN512_REG_COMMAND, temp_reg);
    
    NxpPn512Transceive(&transmit_buffer, 1, atqa, atqa_length);
    
    if(*atqa_length != 2)
    {
        // answer to request should be 2 bytes long
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL Iso14443AntiCollision(U8 cascade_level,
                                  U8 uid_number_of_valid_bits,
                                  U8* uid_number_of_valid_out_bits,
                                  U8* uid_in_ptr,
                                  U8* uid_out_ptr)
{
    U8 command_buffer[10];
    U8 receive_buffer[10];
    U8 cascade_level_codes[] = {0x93, 0x95, 0x97};
    U16 wSndBytes;
    U16 wRcvBits;
    U8 i;
    U8 temp_reg;
    U8 length;
    U8 bUidStartIndex;
    U8 bBitCount;
    
    if(uid_number_of_valid_bits >= 0x40)
    {
        LOG_DBG("ERROR: invalid number of valid bits");
        return FALSE;
    }
    
    // Disable CRC
    NxpPn512SetCrc(FALSE);
    
    // command_buffer[0] should be the cascade level code
    command_buffer[0] = cascade_level_codes[cascade_level];
    MEMSET(&command_buffer[2], 0x00, 5);
    // Copy valid uid bits
    wSndBytes = (U16) (((uid_number_of_valid_bits & 0xF0) >> 4)
			+ ((uid_number_of_valid_bits & 0x0F) ? 1 : 0));
	MEMCPY(&command_buffer[2], uid_in_ptr, wSndBytes); /* PRQA S 3200 */
	wSndBytes += 2;
    
	// Encode NVB
	command_buffer[1] = uid_number_of_valid_bits + 0x20;
    
    // Adjust Rx-Align
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_RXALIGN);
    NxpPn512SpiWriteRegister(PN512_REG_BITFRAMING, (temp_reg | ((uid_number_of_valid_bits & 0x07) << 4)));
    
    // Adjust TxBits
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_TXBITS);
    NxpPn512SpiWriteRegister(PN512_REG_BITFRAMING, (temp_reg | (uid_number_of_valid_bits & 0x07)));
    
    NxpPn512Transceive(command_buffer, wSndBytes, receive_buffer, &length);
    
    // Reset RxAlignment
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_RXALIGN);
    NxpPn512SpiWriteRegister(PN512_REG_BITFRAMING, temp_reg);
    
    // Check status, Collision is allowed for anticollision command
    if (FALSE)
    {
        /* Retrieve number of valid bits of last byte */
//        PH_CHECK_SUCCESS_FCT(statusTmp, phhalHw_GetConfig(pDataParams->pHalDataParams, PHHAL_HW_CONFIG_RXLASTBITS, &wRcvBits));
#warning hier nog errors toevoegen, hieronder ook in else
        LOG_DBG("Scenario where collision occured");
    }
    else
    {
        // whole byte valid
        wRcvBits = 0;
    }
    
    // Add received data to UID
    // Retrieve byte-starting-index of received Uid
    bUidStartIndex = (U8)((uid_number_of_valid_bits & 0xF0) >> 4);
    
    // Add new bitcount
    bBitCount = (U8)(((uid_number_of_valid_bits >> 4) << 3) + (length << 3) + wRcvBits);
    
    // Last incomplete byte is added to length, so remove that again
    if (wRcvBits > 0)
    {
        bBitCount -= 8;
    }
    
    /* Convert bitcount to NVB format */
    *uid_number_of_valid_out_bits = (uint8_t)(((bBitCount >> 3) << 4) + (bBitCount & 0x07));
    
    /* We do not tolerate more than (5 * 8 =)40 bits because it would lead to buffer overflows */
    if (*uid_number_of_valid_out_bits != 0x50)
    {
        LOG_DBG("PROTOCOL error");
        return FALSE;
    }
    
    /* Copy received bytes to uid */
    if (length > 0)
    {
        /* Incomplete byte sent: Merge Rx-Aligned first byte */
        if (uid_number_of_valid_bits & 0x07)
        {
            command_buffer[2 + bUidStartIndex] |= receive_buffer[0];
        }
        /* Else just copy the first byte */
        else
        {
            command_buffer[2 + bUidStartIndex] = receive_buffer[0];
        }
        
        /* Add the rest of the uid bytes */
        memcpy(&command_buffer[2 + bUidStartIndex + 1], &receive_buffer[1], (length - 1));  /* PRQA S 3200 */
    }
    
    /* Anticollision finished */
    if (*uid_number_of_valid_out_bits > 0x40)
    {
        /* Collision in BCC byte can never happen */
        if (*uid_number_of_valid_out_bits < 0x50)
        {
            LOG_DBG("PROTOCOL error");
            return FALSE;
        }

        /* Remove BCC from NvbUidOut */
        *uid_number_of_valid_out_bits = 0x40;
        --length;
        
        /* BCC Check */
        if ((command_buffer[2] ^ command_buffer[3] ^ command_buffer[4] ^ command_buffer[5]) != command_buffer[6])
        {
            LOG_DBG("PROTOCOL error");
            return FALSE;
        }
    }
    
    /* Copy UID */
    memcpy(uid_out_ptr, &command_buffer[2], bUidStartIndex + length);  /* PRQA S 3200 */
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL Iso14443ASelect(U8 cascade_level,
                            U8 uid_number_of_valid_bits,
                            U8* uid_number_of_valid_out_bits,
                            U8* uid_in_ptr,
                            U8* uid_out_ptr)
{
    U8 command_buffer[10];
    U8 receive_buffer[10];
    U8 cascade_level_codes[] = {0x93, 0x95, 0x97};
    U16 wSndBytes;
    U16 wRcvBits;
    U8 i;
    U8 temp_reg;
    U8 length;
    
    // Enable CRC
    NxpPn512SetCrc(TRUE);
    
    // command_buffer[0] should be the cascade level code
    command_buffer[0] = cascade_level_codes[cascade_level];
    MEMSET(&command_buffer[2], 0x00, 5);
    // Copy valid uid bits
    wSndBytes = (U16) (((uid_number_of_valid_bits & 0xF0) >> 4)
			+ ((uid_number_of_valid_bits & 0x0F) ? 1 : 0));
	MEMCPY(&command_buffer[2], uid_in_ptr, wSndBytes); /* PRQA S 3200 */
	wSndBytes += 2;
    
	// SELECT: Add BCC
    uid_number_of_valid_bits = 0x50;
    command_buffer[6] = uid_in_ptr[0] ^ uid_in_ptr[1] ^ uid_in_ptr[2] ^ uid_in_ptr[3];
    ++wSndBytes;
    
	// Encode NVB
	command_buffer[1] = uid_number_of_valid_bits + 0x20;
    
    LOG_DBG("wSndBytes: %d", PU8(wSndBytes));
    for(i = 0; i < wSndBytes; i++)
    {
        LOG_DBG("command_buffer[%d]: 0x%02h", PU8(i), PU8(command_buffer[i]));
    }
    
    // Adjust Rx-Align
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_RXALIGN);
    NxpPn512SpiWriteRegister(PN512_REG_BITFRAMING, (temp_reg | ((uid_number_of_valid_bits & 0x07) << 4)));
    
    // Adjust TxBits
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_TXBITS);
    NxpPn512SpiWriteRegister(PN512_REG_BITFRAMING, (temp_reg | (uid_number_of_valid_bits & 0x07)));
    
    NxpPn512Transceive(command_buffer, wSndBytes, receive_buffer, &length);
    
    // Reset RxAlignment
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_BITFRAMING);
    temp_reg &= ~(U8)(PHHAL_HW_RC523_MASK_RXALIGN);
    NxpPn512SpiWriteRegister(PN512_REG_BITFRAMING, temp_reg);
    
    wRcvBits = 0;
    
    /* Return SAK instead of the UID */
    /* only one byte allowed */
    if (length != 1)
    {
        LOG_DBG("PROTOCOL error");
        return FALSE;
    }
    
    /* Cascade Bit is set */
    if (receive_buffer[0] & 0x04)
    {
        /* If additional cascade levels are impossible -> protocol error */
        if (cascade_level_codes[cascade_level] == 0x97)
        {
            LOG_DBG("PROTOCOL error");
            return FALSE;
        }

        /* Cascade tag does not match -> protocol error */
        if (uid_in_ptr[0] != 0x88)
        {
            LOG_DBG("PROTOCOL error");
            return FALSE;
        }

//            /* Ignore Cascade Tag */
//            memcpy(&pDataParams->abUid[pDataParams->bUidLength], &pUidIn[1], 3);  /* PRQA S 3200 */
//
//            /* Increment Uid length */
//            pDataParams->bUidLength += 3;
    }
    /* Cascade Bit is cleared -> no further cascade levels */
    else
    {
        /* Cascade tag does not match -> protocol error */
        if (uid_in_ptr[0] == 0x88)
        {
            LOG_DBG("PROTOCOL error");
            return FALSE;
        }

//            /* Copy all uid bytes except BCC */
//            memcpy(&pDataParams->abUid[pDataParams->bUidLength], &pUidIn[0], 4);  /* PRQA S 3200 */
//
//            /* Increment Uid length */
//            pDataParams->bUidLength += 4;
//
//            /* Set UID complete flag */
//            pDataParams->bUidComplete = 1;

        /* set default card timeout */
        U16 i;
        while(i++ < 60000){};
        
    }

    /* Copy SAK */
    uid_out_ptr[0] = receive_buffer[0];
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL Iso14443AAntiCollisionLoop(U8* user_id, U8* user_id_length)
{
    U8 cascade_level;
    // uid_number_of_valid_bits : Number of valid bits in the UID of the card currently processed by the anticollision procedure.
    // This variable consists of two parts: the four MSB (Most Significat bit) keep the information of the number of complete valid bytes,
    // and the four LSB (Least Signicant bit) keep the number of remaining valid bits.
    U8 uid_number_of_valid_bits;
    U8 uid_out_number_of_valid_bits;
    U8 uid[10];
    U8 uid_out[20];
    U8 complete_uid[10];
    U8 complete_uid_length = 0;
    U8 sak[2];
    
    // MIFARE PLUS 2K "S"
    // UID:  04 81 6C 92 09 2B 80 (=CASCADE LEVEL 2)
    // Atqa: 44 00
    // Sak: 20
    LOG_DBG("Start ISO14443A Anticollision/Select Sequence");
    
    // We gaan ervan uit dat er maar 1 kaart in het veld aanwezig is
    // Go through all cascade levels
    for(cascade_level = 0; cascade_level < 3; cascade_level++)
    {
        uid_number_of_valid_bits = 0; //number of valid bits
        {
        /* Anticollision loop */
        while (uid_number_of_valid_bits != 0x40)
        {
            if(Iso14443AntiCollision(cascade_level, 
                                     uid_number_of_valid_bits,
                                     &uid_out_number_of_valid_bits,
                                     uid, 
                                     uid_out) == FALSE)
            {
                // Error occured
#warning unhandled collision
                LOG_DBG("Error occured!!!!!!!!!!!!!!!!!!");
                return FALSE;
            }
            uid_number_of_valid_bits = uid_out_number_of_valid_bits;
            if (cascade_level == 0)
            {
                if (uid_out_number_of_valid_bits != 0x40)
                {
                    /* cascade level 1, last 3 bytes are valid */
                    MEMCPY(complete_uid, &uid_out[1], 3);  /* PRQA S 3200 */   
                    complete_uid_length += 3;
                }
                else
                {
                    MEMCPY(&complete_uid[complete_uid_length], uid_out,4);  /* PRQA S 3200 */
                    complete_uid_length += 4;
                }
                
            }
            else
            {
                MEMCPY(&complete_uid[complete_uid_length], uid_out, 4);  /* PRQA S 3200 */
                complete_uid_length += 4;
            }
            continue;
        }
        
        U8 dummy;
        
        if(Iso14443ASelect(cascade_level, 
                           0x40,
                           &dummy,
                           uid_out, 
                           sak) == FALSE)
        {
            LOG_DBG("Select niet goed");
            return FALSE;
        }
        
            if (!(sak[0] & 0x04))
            {
                /* A card UID has been resolved */
                if ((sak[0] & (U8) ~0xFB) == 0)
                {
                    /* Bit b3 is set to zero, [Digital] 4.8.2 */
                    /* Mask out all other bits except for b7 and b6 */
                    U8 bTagPlat;
                    
                    bTagPlat = (sak[0] & 0x60);
                    bTagPlat = bTagPlat >> 5;

                    switch(bTagPlat)
                    {
                    case 1:
                        LOG_DBG("tags found");
                        break;
                    default:
                        break;
                    }
                }
                
                // LOG de UID
                U8 i;
                
                LOG_DBG("USER ID:");
                LOG_DBG("0x%02h", PU8A(complete_uid, complete_uid_length));
                
                break;
            }
        }
    }
    
    if(cascade_level == 0)
    {
        MEMCPY(user_id, complete_uid, complete_uid_length);
        *user_id_length = complete_uid_length;
    }
    else
    {
        MEMCPY(user_id, &complete_uid[1], complete_uid_length);
        *user_id_length = complete_uid_length - 1;
    }
    return TRUE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvRfidReaderNxpPn512_Init(SPI_DEVICE_ID spi_device_id, DRVGPIO_PIN_HNDL not_reset_pin_hndl, DRVGPIO_PIN_HNDL irq_pin_hndl)
{
    SPI_CONFIG_STRUCT spi_config_struct = {500000, MODE_0, 8, FALSE};
    
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)rx_buffer, 0, SIZEOF(rx_buffer));
    MEMSET((VPTR)tx_buffer, 0, SIZEOF(tx_buffer));
    
    spi_id          = spi_device_id;
    not_reset_pin   = not_reset_pin_hndl;
    irq_pin         = irq_pin_hndl;
    
    // Start driver with a hard reset
    DrvGpio_SetPin(not_reset_pin, FALSE);
    DrvGpio_SetPin(not_reset_pin, TRUE);
    
    DrvSpiMasterDevice_Config(spi_device_id, &spi_config_struct);
    
    // Start with resetting the chip
    NxpPn512ResetChip();
    
    InitChip();
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void DrvRfidReaderNxpPn512_SetProtocol(RFID_PROTOCOL protocol)
{
    // clear all IRQ0 and IRQ1 flags
    NxpPn512ClearAllIrqFields();
    
    // set protocol
    ProtocolApplyIso14443a();
    
    NxpPn512SpiWriteRegister(PN512_REG_COMMIEN, 0x00);
    
    // clear all IRQ0 and IRQ1 flags
    NxpPn512ClearAllIrqFields();
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRfidReaderNxpPn512_Iso14443aRequestAnswer(U8* atqa, U8* atqa_length)
{
    return Iso14443RequestA(atqa, atqa_length);
}
//------------------------------------------------------------------------------------------------//
BOOL DrvRfidReaderNxpPn512_Iso14443aAntiCollision(U8* user_id, U8* user_id_length)
{
    return Iso14443AAntiCollisionLoop(user_id, user_id_length);
}
//------------------------------------------------------------------------------------------------//
void DrvRfidReaderNxpPn512_SetRfField(BOOL on_off)
{
    U8 temp_reg;
    
    temp_reg = NxpPn512SpiReadRegister(PN512_REG_TXCONTROL);
    if(on_off)
    {
        temp_reg |= PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN;
    }
    else
    {
        temp_reg &= ~(U8)(PHHAL_HW_RC523_BIT_TX1RFEN | PHHAL_HW_RC523_BIT_TX2RFEN);
    }
    NxpPn512SpiWriteRegister(PN512_REG_TXCONTROL, temp_reg);
}
//================================================================================================//