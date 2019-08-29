//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the Data Link Layer of the PsiRf-protocol.
// The main function of this layer is to transmit frames of characters between Master and slave equipment. The
// layer serves as a communication medium to the network layer.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define PSIRF__COMMDLLPSIRF_C
#warning "Module in development"
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef PSIRF__COMMDLLPSIRF_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               PSIRF__COMMDLLPSIRF_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the timeout in seconds between each calibraion.
// @remark if set to 0, the will be no regular calibration
#ifndef INCLUDE_RSSI
    #define INCLUDE_RSSI                            1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//COMM lib include section
#include "psirf\CommDllPsiRf.h"

//DRV lib include section
#include "spi\DrvSpiMasterDevice.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define REG_COUNT               0x2F

#define BURST_FLAG              0x40
#define READ_FLAG               0x80

#define	CC1101_CNF_START				0x00									// First config register
#define CC1101_PA_TABLE					0x3E									// Control table for power amplifier
#define CC1101_TX_FIFO                  0x3F
#define CC1101_RX_FIFO                  0x3F|READ_FLAG

// CC1101 command strobes
#define CC1101_CMD_SRES					0x30									// Reset chip.
#define CC1101_CMD_SFSTXON				0x31									// Enable and calibrate frequency synthesizer
#define CC1101_CMD_SXOFF				0x32									// Turn off crystal oscillator.
#define CC1101_CMD_SCAL					0x33									// Calibrate frequency synthesizer and turn it off
#define CC1101_CMD_SRX					0x34									// Enable RX
#define CC1101_CMD_STX					0x35									// In IDLE state: Enable TX
#define CC1101_CMD_SIDLE				0x36									// Exit RX / TX, turn off frequency synthesizer
#define CC1101_CMD_SWOR					0x38									// Start automatic RX polling sequence (WOR)
#define CC1101_CMD_SPWD					0x39									// Enter power down mode when CSn goes high.
#define CC1101_CMD_SFRX					0x3A									// Flush the RX FIFO buffer.
#define CC1101_CMD_SFTX					0x3B									// Flush the TX FIFO buffer.
#define CC1101_CMD_SWORRST				0x3C									// Reset real time clock.
#define CC1101_CMD_SNOP					0x3D									// No operation.

// CC1101 internal states
#define CC1101_STATE_MASK				0xF0									// Mask to get the chip state
#define CC1101_STATE_IDLE				0x00									// IDLE state
#define CC1101_STATE_RX					0x10									// Receive mode
#define CC1101_STATE_TX					0x20									// Transmit mode
#define CC1101_STATE_FSTXON				0x30									// Fast TX ready
#define CC1101_STATE_CAL				0x40									// Frequency synthesizer calibration is running
#define CC1101_STATE_SPLL				0x50									// PLL is settling
#define CC1101_STATE_RXOV				0x60									// RX FIFO has overflowed
#define CC1101_STATE_TXOV				0x70									// TX FIFO has underflowed
#define CC1101_STATE_ERROR				0x80									// Chip not ready

// CC1101 status registers
#define CC1101_STATPARTNUM				0x30									// Part number for CC1101 81
#define CC1101_STATVERSION				0x31									// Current version number 81
#define CC1101_STATFREQEST				0x32									// Frequency Offset Estimate 81
#define CC1101_STATLQI					0x33									// Demodulator estimate for Link Quality 81
#define CC1101_STATRSSI					0x34									// Received signal strength indication 81
#define CC1101_STATMARCSTATE			0x35									// Control state machine state 82
#define CC1101_STATWORTIME1				0x36									// High byte of WOR timer 82
#define CC1101_STATWORTIME0				0x37									// Low byte of WOR timer 82
#define CC1101_STATPKTSTATUS			0x38									// Current GDOx status and packet status 83
#define CC1101_STATVGO_VC_DAC			0x39									// Current setting from PLL calibration module 83
#define CC1101_STATTXBYTES				0x3A									// Underflow and number of bytes in the TX FIFO 83
#define CC1101_STATRXBYTES				0x3B									// Overflow and number of bytes in the RX FIFO 83

#define TX_BUFFER_ABOVE_THR             DrvGpio_GetPin(dllpsirf_gdo0_pin_hndl)
#define NOT_USED                        DrvGpio_GetPin(dllpsirf_gdo2_pin_hndl)
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   enumeration typedef for the DLE data link layer <em>state</em>
typedef enum
{
    DLL_PSIRF_INIT,
    DLL_PSIRF_IDLE,
    DLL_PSIRF_RX_ACTIVE,
    DLL_PSIRF_TX_LOADED,
    DLL_PSIRF_TX_ACTIVE,
    DLL_PSIRF_DISABLED,
}
DLL_PSIRF_STATE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE) && (INCLUDE_RSSI == 1)
static void Command_RssiStart(void);
static void Command_RssiStop(void);
static void RssiTask(VPTR data_ptr);
static void Command_CC11SetReg(void);
static void Command_CC11SetPA(void);
#endif
static void DllPsiRfOnEntry(DLL_PSIRF_STATE state);
static BOOL DllPsiRfInitTransceiver(void);
static BOOL DllPsiRfResetTransceiver(void);
static BOOL DllPsiRfCalibrate(void);
static U8 DllPsiRfSendChipCmd(U8 cmd);
static U8 DllPsiRfReadChipStatus(U8 cmd);
static void DllPsiRfSetConfigBytes(U8 addr, U8* data_ptr, U8 data_len);
static void DllPsiRfGetConfigByte(U8 addr, U8* data_ptr);
static void DllPsiRfLoadTxMessage(void);
static void DllPsiRfHandleStateMachine(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SPI_DEVICE_ID                    dllpsirf_spi_dev_hndl;
static DRVGPIO_PIN_HNDL                 dllpsirf_cs_pin_hndl;
static DRVGPIO_PIN_HNDL                 dllpsirf_gdo0_pin_hndl;
static DRVGPIO_PIN_HNDL                 dllpsirf_gdo2_pin_hndl;

static DLL_PSIRF_STATE                  dllpsirf_state;
static BOOL                             dllpsirf_communication_enabled;

static U8                               dllpsirf_data[32];

#if (TERM_LEVEL > TERM_LEVEL_NONE) && (INCLUDE_RSSI == 1)
static TASK_HNDL                        stddlpsirf_rssi_task;
#endif

static     const STRING    dll_states[] = {"INIT", "IDLE", "RX_ACTIVE", "TX_LOADED", "TX_ACTIVE", "DISABLED"};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
COMMDLLPSIRF_DATA        commdllpsirf_data = 
{
    .registers =
    {
        0x09,  // IOCFG2              GDO2 Output Pin Configuration
        0x2E,  // IOCFG1              GDO1 Output Pin Configuration
        0x07,  // IOCFG0              GDO0 Output Pin Configuration
        0x47,  // FIFOTHR             RX FIFO and TX FIFO Thresholds
        0xD3,  // SYNC1               Sync Word, High Byte
        0x91,  // SYNC0               Sync Word, Low Byte
        STDDLLPSIRF_FRAME_LENGTH,  // PKTLEN              Packet Length
        0x0C,  // PKTCTRL1            Packet Automation Control
        0x05,  // PKTCTRL0            Packet Automation Control
        0x00,  // ADDR                Device Address
        0x00,  // CHANNR              Channel Number
        0x06,  // FSCTRL1             Frequency Synthesizer Control
        0x00,  // FSCTRL0             Frequency Synthesizer Control
        0x21,  // FREQ2               Frequency Control Word, High Byte
        0x65,  // FREQ1               Frequency Control Word, Middle Byte
        0x62,  // FREQ0               Frequency Control Word, Low Byte
        0xCA,  // MDMCFG4             Modem Configuration
        0x83,  // MDMCFG3             Modem Configuration
        0x13,  // MDMCFG2             Modem Configuration
        0x22,  // MDMCFG1             Modem Configuration
        0xF8,  // MDMCFG0             Modem Configuration
        0x35,  // DEVIATN             Modem Deviation Setting
        0x07,  // MCSM2               Main Radio Control State Machine Configuration
        0x2F,  // MCSM1               Main Radio Control State Machine Configuration
        0x08,  // MCSM0               Main Radio Control State Machine Configuration
        0x16,  // FOCCFG              Frequency Offset Compensation Configuration
        0x6C,  // BSCFG               Bit Synchronization Configuration
        0x43,  // AGCCTRL2            AGC Control
        0x40,  // AGCCTRL1            AGC Control
        0x91,  // AGCCTRL0            AGC Control
        0x87,  // WOREVT1             High Byte Event0 Timeout
        0x6B,  // WOREVT0             Low Byte Event0 Timeout
        0xFB,  // WORCTRL             Wake On Radio Control
        0x56,  // FREND1              Front End RX Configuration
        0x10,  // FREND0              Front End TX Configuration
        0xE9,  // FSCAL3              Frequency Synthesizer Calibration
        0x2A,  // FSCAL2              Frequency Synthesizer Calibration
        0x00,  // FSCAL1              Frequency Synthesizer Calibration
        0x1F,  // FSCAL0              Frequency Synthesizer Calibration
        0x41,  // RCCTRL1             RC Oscillator Configuration
        0x00,  // RCCTRL0             RC Oscillator Configuration
        0x59,  // FSTEST              Frequency Synthesizer Calibration Control
        0x7F,  // PTEST               Production Test
        0x3F,  // AGCTEST             AGC Test
        0x81,  // TEST2               Various Test Settings
        0x35,  // TEST1               Various Test Settings
        0x09,  // TEST0               Various Test Settings
    },
    .pa_table = {0xC5,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00}
};
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE) && (INCLUDE_RSSI == 1)
static void Command_RssiStart(void)
{
    CoreTask_Start(stddlpsirf_rssi_task);
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_RssiStop(void)
{
    CoreTask_Stop(stddlpsirf_rssi_task);
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void RssiTask(VPTR data_ptr)
{
    U8 rssi = DllPsiRfReadChipStatus(CC1101_STATRSSI) ^ 0x80;
    LOG_TRM("%d", PS16(((S16)rssi >> 1) - 138));
}
//------------------------------------------------------------------------------------------------//
static void Command_CC11SetReg(void)
{
    CoreTerm_PrintFeedback(CommDllPsiRf_UpdateRegister((U8)CoreTerm_GetArgumentAsU32(0), (U8)CoreTerm_GetArgumentAsU32(1)));
}
//------------------------------------------------------------------------------------------------//
static void Command_CC11SetPA(void)
{
    CoreTerm_PrintFeedback(CommDllPsiRf_UpdatePaTable((U8)CoreTerm_GetArgumentAsU32(0), (U8)CoreTerm_GetArgumentAsU32(1)));
}
#endif
//------------------------------------------------------------------------------------------------//
static void DllPsiRfOnEntry(DLL_PSIRF_STATE state)
{
    dllpsirf_state = state;
    
    LOG_DEV("[DLLSTATE] %s", PCSTR(dll_states[dllpsirf_state]));
}
//------------------------------------------------------------------------------------------------//
static BOOL DllPsiRfInitTransceiver(void)
{
    U8  i;
    U8 data;
    
    if(DllPsiRfResetTransceiver() == FALSE)
    {
        LOG_DBG("DLLPSIRF - RESET FAILED");
        return FALSE;
    }
    
    DllPsiRfSetConfigBytes(CC1101_CNF_START, &commdllpsirf_data.registers[0], sizeof(commdllpsirf_data.registers));
    DllPsiRfSetConfigBytes(CC1101_PA_TABLE, &commdllpsirf_data.pa_table[0], sizeof(commdllpsirf_data.pa_table));
    
    for(i=0;i<sizeof(commdllpsirf_data.registers);i++)
    {
        DllPsiRfGetConfigByte(CC1101_CNF_START+i, &data);
        if(data != commdllpsirf_data.registers[i])
        {
            LOG_DBG("DLLPSIRF - REG NOK");
            return FALSE;
        }
    }
    
    DllPsiRfSendChipCmd(CC1101_CMD_SIDLE);
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DllPsiRfResetTransceiver(void)
{
    U8 i;
    U8 status;
    
    // Do a strobe on CS
    DrvGpio_SetPin(dllpsirf_cs_pin_hndl, FALSE);
    DrvGpio_SetPin(dllpsirf_cs_pin_hndl, FALSE);
    DrvGpio_SetPin(dllpsirf_cs_pin_hndl, TRUE);
    
    // Wait for ~62.5us
    i=200;
    while(i--)
    {
        asm("nop");
    }
    
    DllPsiRfSendChipCmd(CC1101_CMD_SRES);
    
    // Wait for ~62.5us
    i=200;
    while(i--)
    {
        asm("nop");
    }
    
    status = DllPsiRfSendChipCmd(CC1101_CMD_SNOP);
    if(status != CC1101_STATE_IDLE)
    {
        LOG_DBG("DLLPSIRF - RESET state not IDLE - %02x", PU8(status));
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DllPsiRfCalibrate(void)
{
    U16 i = 0;
    
    DllPsiRfSendChipCmd(CC1101_CMD_SIDLE);
    DllPsiRfSendChipCmd(CC1101_CMD_SCAL);
    
    while(DllPsiRfSendChipCmd(CC1101_CMD_SNOP) == CC1101_STATE_CAL)
    {
        if(++i > 100)
        {
            LOG_DBG("DLLPSIRF - CALIB not DONE");
            return FALSE;
        }
    }
    LOG_DEV("[DLLCALIB] DONE (i = %d)", PU16(i));
    
    DllPsiRfSendChipCmd(CC1101_CMD_SRX);
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static U8 DllPsiRfSendChipCmd(U8 cmd)
{
    U8  status;
    
    DrvSpiMasterDevice_SelectExchangeData(dllpsirf_spi_dev_hndl, cmd, &status);
    
    return (status & CC1101_STATE_MASK);
}
//------------------------------------------------------------------------------------------------//
static U8 DllPsiRfReadChipStatus(U8 cmd)
{
    U8  status;
    
    DrvSpiMasterDevice_Select(dllpsirf_spi_dev_hndl);
    
    cmd |= (READ_FLAG|BURST_FLAG);
    DrvSpiMasterDevice_ExchangeData(dllpsirf_spi_dev_hndl, cmd, &status);
    DrvSpiMasterDevice_ReadData(dllpsirf_spi_dev_hndl, &status, 1);
    
    DrvSpiMasterDevice_Deselect(dllpsirf_spi_dev_hndl);
    
    return status;
}
//------------------------------------------------------------------------------------------------//
static void DllPsiRfSetConfigBytes(U8 addr, U8* data_ptr, U8 data_len)
{
    U8  status;
    
    DrvSpiMasterDevice_Select(dllpsirf_spi_dev_hndl);
    
    addr |= BURST_FLAG;
    DrvSpiMasterDevice_ExchangeData(dllpsirf_spi_dev_hndl, addr, &status);
    DrvSpiMasterDevice_WriteData(dllpsirf_spi_dev_hndl, data_ptr, data_len);
    
    DrvSpiMasterDevice_Deselect(dllpsirf_spi_dev_hndl);
}
//------------------------------------------------------------------------------------------------//
static void DllPsiRfGetConfigByte(U8 addr, U8* data_ptr)
{
    U8  status;
    
    DrvSpiMasterDevice_Select(dllpsirf_spi_dev_hndl);
    
    addr |= READ_FLAG;
    DrvSpiMasterDevice_ExchangeData(dllpsirf_spi_dev_hndl, addr, &status);
    DrvSpiMasterDevice_ReadData(dllpsirf_spi_dev_hndl, data_ptr, 1);
    
    DrvSpiMasterDevice_Deselect(dllpsirf_spi_dev_hndl);
}
//------------------------------------------------------------------------------------------------//
static void DllPsiRfLoadTxMessage(void)
{
    U8  status;
    U8  cmd = CC1101_TX_FIFO|BURST_FLAG;
    
    DrvSpiMasterDevice_Select(dllpsirf_spi_dev_hndl);
    
    DrvSpiMasterDevice_ExchangeData(dllpsirf_spi_dev_hndl, cmd, &status);
    DrvSpiMasterDevice_WriteData(dllpsirf_spi_dev_hndl, &dllpsirf_data[0], 32);
    
    DrvSpiMasterDevice_Deselect(dllpsirf_spi_dev_hndl);
}
//------------------------------------------------------------------------------------------------//
static void DllPsiRfHandleStateMachine(void)
{
    U8  status;
    
    if(dllpsirf_communication_enabled == FALSE)
    {
        if(dllpsirf_state == DLL_PSIRF_TX_ACTIVE)
        {
            DllPsiRfOnEntry(DLL_PSIRF_INIT);
        }
    }
//    else if(dllpsirf_state != DLL_PSIRF_DISABLED)
//    {
//        DllPsiRfResetTransceiver();
//        DllPsiRfSendChipCmd(CC1101_CMD_SPWD);
//        DllPsiRfOnEntry(DLL_PSIRF_DISABLED);
//    }
    
    if(dllpsirf_state == DLL_PSIRF_INIT)
    {
        if(DllPsiRfInitTransceiver() && DllPsiRfCalibrate())
        {
            DllPsiRfOnEntry(DLL_PSIRF_IDLE);
        }
    }
    else
    {
        if(dllpsirf_state == DLL_PSIRF_IDLE)
        {
            do
            {
                status = DllPsiRfSendChipCmd(CC1101_CMD_SNOP);
            }
            while(status == CC1101_STATE_SPLL);
            
            if(status == CC1101_STATE_RX)
            {
                DllPsiRfOnEntry(DLL_PSIRF_RX_ACTIVE);
            }
            else
            {
                if((dllpsirf_state == DLL_PSIRF_IDLE) && (status != CC1101_STATE_IDLE))
                {
                    LOG_WRN("E1 %02h", PU8(status));
                    DllPsiRfOnEntry(DLL_PSIRF_INIT);
                }
                return;
            }
        }
        
        if((dllpsirf_communication_enabled == TRUE) && (dllpsirf_state == DLL_PSIRF_RX_ACTIVE))
        {
            DllPsiRfLoadTxMessage();
            DllPsiRfOnEntry(DLL_PSIRF_TX_LOADED);
        }
        if(dllpsirf_state == DLL_PSIRF_TX_LOADED)
        {
            DllPsiRfSendChipCmd(CC1101_CMD_STX);
            
            do
            {
                status = DllPsiRfSendChipCmd(CC1101_CMD_SNOP);
            }
            while(status == CC1101_STATE_SPLL);
            
            if(status == CC1101_STATE_TX)
            {
                DllPsiRfOnEntry(DLL_PSIRF_TX_ACTIVE);
            }
            else if(status != CC1101_STATE_RX)
            {
                LOG_WRN("E2 %02h", PU8(status));
                DllPsiRfOnEntry(DLL_PSIRF_INIT);
            }
        }
        if(dllpsirf_state == DLL_PSIRF_TX_ACTIVE)
        {
            status = DllPsiRfSendChipCmd(CC1101_CMD_SNOP);
            if(TX_BUFFER_ABOVE_THR == FALSE)
            {
                DllPsiRfLoadTxMessage();
            }
            else if(status != CC1101_STATE_TX)
            {
                LOG_WRN("E3 %02h", PU8(status));
                DllPsiRfOnEntry(DLL_PSIRF_INIT);
            }
        }
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_Init(SPI_CHANNEL_HNDL channel_hndl,
                       DRVGPIO_PIN_HNDL cs_pin_hndl,
                       DRVGPIO_PIN_HNDL gdo0_pin_hndl,
                       DRVGPIO_PIN_HNDL gdo2_pin_hndl)
{
    MODULE_INIT_ONCE();
    
    SPI_CONFIG_STRUCT   spi_cfg_struct = {1000000, MODE_0, 8, FALSE};
    
    dllpsirf_spi_dev_hndl = DrvSpiMasterDevice_Register(channel_hndl, cs_pin_hndl, &spi_cfg_struct, NULL);
    
    dllpsirf_cs_pin_hndl = cs_pin_hndl;
    dllpsirf_gdo0_pin_hndl = gdo0_pin_hndl;
    dllpsirf_gdo2_pin_hndl = gdo2_pin_hndl;
    
    DllPsiRfOnEntry(DLL_PSIRF_INIT);
    
    commdllpsirf_data.registers[CC1101_IOCFG2] = 0x02;
    commdllpsirf_data.registers[CC1101_IOCFG0] = 0x02;        // TX THR indication
    commdllpsirf_data.registers[CC1101_FIFOTHR] = 0x4D;       // TX THR @ 9 bytes
    commdllpsirf_data.registers[CC1101_PKTLEN] = STDDLLPSIRF_FRAME_LENGTH;    // max length of received msg
    commdllpsirf_data.registers[CC1101_PKTCTRL1] = 0x00;      // no crc auto flash, no status append
    commdllpsirf_data.registers[CC1101_PKTCTRL0] = 0x02;      // infinite packet length
    commdllpsirf_data.registers[CC1101_MCSM2] = 0x07;
    commdllpsirf_data.registers[CC1101_MCSM1] = 0x0F;         // no CCA
    commdllpsirf_data.registers[CC1101_MCSM0] = 0x08;
    
    MEMSET(dllpsirf_data, 0xFF, 32);
    
#if (TERM_LEVEL > TERM_LEVEL_NONE) && (INCLUDE_RSSI == 1)
    stddlpsirf_rssi_task = CoreTask_RegisterTask(2000, RssiTask, NULL, 128, "RSSI");
    CoreTerm_RegisterCommand("RssiStart","RSSI Start logging", 0, Command_RssiStart, TRUE);
    CoreTerm_RegisterCommand("RssiStop","RSSI Stop logging", 0, Command_RssiStop, TRUE);
    CoreTerm_RegisterCommand("CC11SetReg","CC11 set reg a to b", 2, Command_CC11SetReg, TRUE);
    CoreTerm_RegisterCommand("CC11SetPA","CC11 set PA reg a to b", 2, Command_CC11SetPA, TRUE);
#endif
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_RegisterFrameHook(DLLPSIRFHOOK_FRAME_RECEIVED frame_hook)
{
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_Handler(void)
{
    MODULE_CHECK();
    DllPsiRfHandleStateMachine();
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllPsiRf_UpdateRegister(U8 reg_number, U8 value)
{
    if(reg_number < REG_COUNT)
    {
        commdllpsirf_data.registers[reg_number] = value;
        DllPsiRfOnEntry(DLL_PSIRF_INIT);
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllPsiRf_UpdatePaTable(U8 index, U8 value)
{
    if(index < PA_COUNT)
    {
        commdllpsirf_data.pa_table[index] = value;
        DllPsiRfOnEntry(DLL_PSIRF_INIT);
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_SendFrame(U8* data_ptr)
{
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_EnableCommunication(BOOL enable_communication)
{
    dllpsirf_communication_enabled = enable_communication;
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_PrintStatus(void)
{
    LOG_TRM("[DLL PSIRF] INFO");
    
    LOG_TRM("DLL State       : %s", PCSTR(dll_states[dllpsirf_state]));
    LOG_TRM("DLL CCxx Regs   : %02h", PU8A(commdllpsirf_data.registers, 16));
    LOG_TRM("                  %02h", PU8A(&commdllpsirf_data.registers[16], 16));
    LOG_TRM("                  %02h", PU8A(&commdllpsirf_data.registers[32], 15));
    LOG_TRM("DLL CCxx PA Tbl : %02h", PU8A(commdllpsirf_data.pa_table, 8));
}
//================================================================================================//

