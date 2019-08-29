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
// @brief  Defines the outbox queue size
#ifndef STDDLLPSIRF_QUEUE_SIZE
    #error "STDDLLPSIRF_QUEUE_SIZE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the outbox queue size
#ifndef STDDLLPSIRF_INBOX_QUEUE_SIZE
    #define STDDLLPSIRF_INBOX_QUEUE_SIZE            STDDLLPSIRF_QUEUE_SIZE
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the timeout in milliseconds the state machine can stay in non-stable IDLE and TX states
#ifndef STDDLLPSIRF_STATE_TIMEOUT
    #define STDDLLPSIRF_STATE_TIMEOUT               1000
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the timeout in ms of the task that calls the CommDllPsiRf_Handler() from the timer tick interrupt.
// This is a backup solution in case the CommDllPsiRf_Handler() cannot be called fast enough from background. In that
// case the handler will be called from the timer tick interrupt.
// @remark if set to 0, the handler will never be called from interrupt
// @remark special attention must be made to the SPI in case multiple devices are on the same SPI bus
#ifndef STDDLLPSIRF_HANDLER_FROM_INTERRUPT_TIMEOUT
    #error "STDDLLPSIRF_HANDLER_FROM_INTERRUPT_TIMEOUT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the timeout in seconds between each calibraion.
// @remark if set to 0, the will be no regular calibration
#ifndef STDDLLPSIRF_CALIBRATION_TIMEOUT
    #error "STDDLLPSIRF_CALIBRATION_TIMEOUT not defined in AppConfig.h"
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

#define RX_MESSAGE_AVAILABLE            DrvGpio_GetPin(dllpsirf_gdo0_pin_hndl)
#define CHANNEL_IS_CLEAR                DrvGpio_GetPin(dllpsirf_gdo2_pin_hndl)
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

typedef struct
{
    U8      buffer[STDDLLPSIRF_FRAME_LENGTH];
    U8      rssi;
}
DLL_PSIRF_MSG;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DllPsiRfOnEntry(DLL_PSIRF_STATE state);
static BOOL DllPsiRfInitTransceiver(void);
static BOOL DllPsiRfResetTransceiver(void);
static BOOL DllPsiRfCalibrate(void);
static U8 DllPsiRfSendChipCmd(U8 cmd);
static U8 DllPsiRfReadChipStatus(U8 cmd);
static void DllPsiRfSetConfigBytes(U8 addr, U8* data_ptr, U8 data_len);
static void DllPsiRfGetConfigByte(U8 addr, U8* data_ptr);
static void DllPsiRfReadRxMessage(void);
static void DllPsiRfHandleRxMessage(DLL_PSIRF_MSG* inbox_msg_ptr);
static void DllPsiRfLoadTxMessage(void);
static void DllPsiRfHandleStateMachine(void);
static void DllPsiRfOnStateTimeOut(VPTR data_ptr);
#if STDDLLPSIRF_HANDLER_FROM_INTERRUPT_TIMEOUT
static void DllPsiRfOnHandlerTimeOut(VPTR data_ptr);
#endif
#if STDDLLPSIRF_CALIBRATION_TIMEOUT
static void DllPsiRfOnCalibrationTimeOut(VPTR data_ptr);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static DLLPSIRFHOOK_FRAME_RECEIVED      dllpsirf_frame_hook;

static SPI_DEVICE_ID                    dllpsirf_spi_dev_hndl;
static DRVGPIO_PIN_HNDL                 dllpsirf_cs_pin_hndl;
static DRVGPIO_PIN_HNDL                 dllpsirf_gdo0_pin_hndl;
static DRVGPIO_PIN_HNDL                 dllpsirf_gdo2_pin_hndl;

static DLL_PSIRF_STATE                  dllpsirf_state;
static U8                               dllpsirf_registers[REG_COUNT] = 
{
    0x09,  // IOCFG2              GDO2 Output Pin Configuration
    0x2E,  // IOCFG1              GDO1 Output Pin Configuration
    0x07,  // IOCFG0              GDO0 Output Pin Configuration
    0x47,  // FIFOTHR             RX FIFO and TX FIFO Thresholds
    0xD3,  // SYNC1               Sync Word, High Byte
    0x91,  // SYNC0               Sync Word, Low Byte
    0xFF,  // PKTLEN              Packet Length
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
    0x0F,  // MCSM1               Main Radio Control State Machine Configuration
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
};
static U8                               dllpsirf_pa_table[8] =
{
	0xC5,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00,  0x00
};

static U8                               dllpsirf_wait_for_tx;
static DLL_PSIRF_MSG                    dllpsirf_inbox_msg;
static DLL_PSIRF_MSG                    dllpsirf_outbox_msg;
static BOOL                             dllpsirf_communication_enabled;

static Q_HNDL                           dllpsirf_outbox_q_hndl;
static Q_HNDL                           dllpsirf_inbox_q_hndl;

static TASK_HNDL                        stddlpsirf_state_timeout_task;

#if STDDLLPSIRF_HANDLER_FROM_INTERRUPT_TIMEOUT
static TASK_HNDL                        stddlpsirf_timeout_handler_task;
#endif

#if STDDLLPSIRF_CALIBRATION_TIMEOUT
static U16                              stddlpsirf_timeout_counter;
#endif

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static const STRING     dll_states[] = {"INIT", "IDLE", "RX_ACTIVE", "TX_LOADED", "TX_ACTIVE", "DISABLED"};
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DllPsiRfOnEntry(DLL_PSIRF_STATE state)
{
    dllpsirf_state = state;
    
    LOG_DEV("[DLLSTATE] %s", PCSTR(dll_states[dllpsirf_state]));
    
    if((state == DLL_PSIRF_IDLE) || (state == DLL_PSIRF_TX_ACTIVE))
    {
        CoreTask_Start(stddlpsirf_state_timeout_task);
    }
    else
    {
        CoreTask_Stop(stddlpsirf_state_timeout_task);
    }
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
    
    DllPsiRfSetConfigBytes(CC1101_CNF_START, &dllpsirf_registers[0], sizeof(dllpsirf_registers));
    DllPsiRfSetConfigBytes(CC1101_PA_TABLE, &dllpsirf_pa_table[0], sizeof(dllpsirf_pa_table));
    
    for(i=0;i<sizeof(dllpsirf_registers);i++)
    {
        DllPsiRfGetConfigByte(CC1101_CNF_START+i, &data);
        if(data != dllpsirf_registers[i])
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
static void DllPsiRfReadRxMessage(void)
{
    U8  status[2];
    BOOL     read_msg;
    U8  cmd = CC1101_RX_FIFO|BURST_FLAG;
    
    do
    {
        DrvSpiMasterDevice_Select(dllpsirf_spi_dev_hndl);
        DrvSpiMasterDevice_ExchangeData(dllpsirf_spi_dev_hndl, cmd, &status[0]);
        DrvSpiMasterDevice_ReadData(dllpsirf_spi_dev_hndl, &dllpsirf_inbox_msg.buffer[0], 1);

        if(dllpsirf_inbox_msg.buffer[0] >= STDDLLPSIRF_FRAME_LENGTH)
        {
            LOG_WRN("LEN!! %d", PU8(dllpsirf_inbox_msg.buffer[0]));
            
            DrvSpiMasterDevice_Deselect(dllpsirf_spi_dev_hndl);
            
            DllPsiRfSendChipCmd(CC1101_CMD_SIDLE);
            DllPsiRfSendChipCmd(CC1101_CMD_SFRX);
            DllPsiRfSendChipCmd(CC1101_CMD_SRX);
            DllPsiRfOnEntry(DLL_PSIRF_IDLE);
            return;
        }
        else
        {
            DrvSpiMasterDevice_ReadData(dllpsirf_spi_dev_hndl, &dllpsirf_inbox_msg.buffer[1], dllpsirf_inbox_msg.buffer[0]);
            DrvSpiMasterDevice_ReadData(dllpsirf_spi_dev_hndl, &status[0], 2);
            
            DrvSpiMasterDevice_Deselect(dllpsirf_spi_dev_hndl);
            
            dllpsirf_inbox_msg.rssi = status[0] ^ 0x80;
            dllpsirf_wait_for_tx ^= dllpsirf_inbox_msg.rssi;
            
            LOG_DBG("RSSI : %d - LQI : %d", PU8(dllpsirf_inbox_msg.rssi), PU8(status[1] & 0x7F));
            
            if(CoreQ_Write(dllpsirf_inbox_q_hndl, (VPTR)&dllpsirf_inbox_msg, 1) == FALSE)
            {
                LOG_WRN("IQ1");
            }

            read_msg = (BOOL)((RX_MESSAGE_AVAILABLE == TRUE) ||
                                 ((CHANNEL_IS_CLEAR == TRUE) && (DllPsiRfReadChipStatus(CC1101_STATRXBYTES) > 0)));
            if(read_msg)
            {
                LOG_WRN("RX++");
            }
        }
    }
    while(read_msg);
}
//------------------------------------------------------------------------------------------------//
static void DllPsiRfHandleRxMessage(DLL_PSIRF_MSG* inbox_msg_ptr)
{
    if(dllpsirf_frame_hook != NULL)
    {
        if(dllpsirf_frame_hook(&inbox_msg_ptr->buffer[0], inbox_msg_ptr->rssi))
        {
            if(CoreQ_Write(dllpsirf_outbox_q_hndl, (VPTR)inbox_msg_ptr, 1) == FALSE)
            {
                LOG_WRN("OQ1");
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void DllPsiRfLoadTxMessage(void)
{
    U8  status;
    U8  cmd = CC1101_TX_FIFO|BURST_FLAG;
    
    if(CoreQ_Read(dllpsirf_outbox_q_hndl, (VPTR)&dllpsirf_outbox_msg, 1))
    {
        DrvSpiMasterDevice_Select(dllpsirf_spi_dev_hndl);
        
        DrvSpiMasterDevice_ExchangeData(dllpsirf_spi_dev_hndl, cmd, &status);
        DrvSpiMasterDevice_WriteData(dllpsirf_spi_dev_hndl, &dllpsirf_outbox_msg.buffer[0], dllpsirf_outbox_msg.buffer[0]+1);
        
        DrvSpiMasterDevice_Deselect(dllpsirf_spi_dev_hndl);
        
        DllPsiRfOnEntry(DLL_PSIRF_TX_LOADED);
    }
}
//------------------------------------------------------------------------------------------------//
static void DllPsiRfHandleStateMachine(void)
{
    U16 i = 0;
    U8  status;
    
    if(dllpsirf_communication_enabled)
    {
        if(dllpsirf_state == DLL_PSIRF_DISABLED)
        {
            DllPsiRfOnEntry(DLL_PSIRF_INIT);
        }
    }
    else if((dllpsirf_state < DLL_PSIRF_TX_LOADED) && (CoreQ_GetCount(dllpsirf_outbox_q_hndl) == 0))
    {
        DllPsiRfResetTransceiver();
        DllPsiRfOnEntry(DLL_PSIRF_DISABLED);
    }
    
    if(dllpsirf_state == DLL_PSIRF_DISABLED)
    {
        return;
    }
    else if(dllpsirf_state == DLL_PSIRF_INIT)
    {
        if(DllPsiRfInitTransceiver() && DllPsiRfCalibrate())
        {
            DllPsiRfOnEntry(DLL_PSIRF_IDLE);
        }
    }
    else
    {
        if((dllpsirf_state == DLL_PSIRF_IDLE) || (dllpsirf_state == DLL_PSIRF_TX_ACTIVE))
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
                if((((dllpsirf_state == DLL_PSIRF_IDLE) && (status != CC1101_STATE_IDLE)) ||
                    ((dllpsirf_state == DLL_PSIRF_TX_ACTIVE) && (status != CC1101_STATE_TX))))
                {
                    LOG_WRN("E1 %02h", PU8(status));
                    DllPsiRfOnEntry(DLL_PSIRF_INIT);
                }
                return;
            }
        }

        if(RX_MESSAGE_AVAILABLE == TRUE)
        {
            DllPsiRfReadRxMessage();
        }
        
        if(dllpsirf_state == DLL_PSIRF_RX_ACTIVE)
        {
#if STDDLLPSIRF_CALIBRATION_TIMEOUT
            if((stddlpsirf_timeout_counter >= STDDLLPSIRF_CALIBRATION_TIMEOUT) && (CHANNEL_IS_CLEAR == TRUE))
            {
                stddlpsirf_timeout_counter = 0;
                if(DllPsiRfCalibrate())
                {
                    DllPsiRfOnEntry(DLL_PSIRF_IDLE);
                }
                else
                {
                    DllPsiRfOnEntry(DLL_PSIRF_INIT);
                }
                return;
            }
#endif
            if(CoreQ_GetCount(dllpsirf_outbox_q_hndl) > 0)
            {
                DllPsiRfLoadTxMessage();
            }
        }
        if(dllpsirf_state == DLL_PSIRF_TX_LOADED)
        {
            if(CHANNEL_IS_CLEAR == TRUE)
            {
                i = (U16)(dllpsirf_wait_for_tx);
                while(i)
                {
                    i--;
                }
                
                if(CHANNEL_IS_CLEAR == FALSE)
                {
                    return;
                }
                
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
            else
            {
                do
                {
                    status = DllPsiRfSendChipCmd(CC1101_CMD_SNOP);
                }
                while(status == CC1101_STATE_SPLL);
                
                if(status != CC1101_STATE_RX)
                {
                    LOG_WRN("E3 %02h", PU8(status));
                    DllPsiRfOnEntry(DLL_PSIRF_INIT);
                }
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void DllPsiRfOnStateTimeOut(VPTR data_ptr)
{
    LOG_WRN("E4 %02h", PU8(dllpsirf_state));
    DllPsiRfOnEntry(DLL_PSIRF_INIT);
}
//------------------------------------------------------------------------------------------------//
#if STDDLLPSIRF_HANDLER_FROM_INTERRUPT_TIMEOUT
static void DllPsiRfOnHandlerTimeOut(VPTR data_ptr)
{
    if(((dllpsirf_state == DLL_PSIRF_RX_ACTIVE) || (dllpsirf_state == DLL_PSIRF_TX_LOADED)) &&
       (RX_MESSAGE_AVAILABLE == TRUE))
    {
        DllPsiRfReadRxMessage();
    }
}
#endif
//------------------------------------------------------------------------------------------------//
#if STDDLLPSIRF_CALIBRATION_TIMEOUT
static void DllPsiRfOnCalibrationTimeOut(VPTR data_ptr)
{
    stddlpsirf_timeout_counter++;
}
#endif
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
    dllpsirf_frame_hook = NULL;
    
    dllpsirf_outbox_q_hndl = CoreQ_Register(STDDLLPSIRF_QUEUE_SIZE, SIZEOF(DLL_PSIRF_MSG), "CommDllPsiRf_OutboxQ");
    dllpsirf_inbox_q_hndl = CoreQ_Register(STDDLLPSIRF_INBOX_QUEUE_SIZE, SIZEOF(DLL_PSIRF_MSG), "CommDllPsiRf_InboxQ");
    
    dllpsirf_cs_pin_hndl = cs_pin_hndl;
    dllpsirf_gdo0_pin_hndl = gdo0_pin_hndl;
    dllpsirf_gdo2_pin_hndl = gdo2_pin_hndl;
    
    dllpsirf_registers[CC1101_IOCFG2] = 0x09;
    dllpsirf_registers[CC1101_IOCFG0] = 0x07;
    dllpsirf_registers[CC1101_PKTLEN] = STDDLLPSIRF_FRAME_LENGTH;    // max length of received msg
    dllpsirf_registers[CC1101_PKTCTRL1] = 0x0C;
    dllpsirf_registers[CC1101_PKTCTRL0] = 0x05;
    dllpsirf_registers[CC1101_MCSM2] = 0x07;
    dllpsirf_registers[CC1101_MCSM1] = 0x2F;
    dllpsirf_registers[CC1101_MCSM0] = 0x08;
    
    dllpsirf_wait_for_tx = 0x55;
    dllpsirf_communication_enabled = TRUE;
    
    stddlpsirf_state_timeout_task = CoreTask_RegisterTask((U32)STDDLLPSIRF_STATE_TIMEOUT * 1e3, DllPsiRfOnStateTimeOut, NULL, 128, "CommDllPsiRfStateTO");
    
#if STDDLLPSIRF_HANDLER_FROM_INTERRUPT_TIMEOUT
    stddlpsirf_timeout_handler_task = CoreTask_RegisterTask((U32)STDDLLPSIRF_HANDLER_FROM_INTERRUPT_TIMEOUT * 1e3, DllPsiRfOnHandlerTimeOut, NULL, 64, "CommDllPsiRfHandlerTO");
#endif
#if STDDLLPSIRF_CALIBRATION_TIMEOUT
    stddlpsirf_timeout_counter = 0;
    CoreTask_Start(CoreTask_RegisterTask((U32)1e6, DllPsiRfOnCalibrationTimeOut, NULL, 128, "CommDllPsiRfCalib"));
#endif
    
    DllPsiRfOnEntry(DLL_PSIRF_INIT);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_RegisterFrameHook(DLLPSIRFHOOK_FRAME_RECEIVED frame_hook)
{
    dllpsirf_frame_hook = frame_hook;
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_Handler(void)
{
    DLL_PSIRF_MSG   inbx_msg;
    
    MODULE_CHECK();
    
#if STDDLLPSIRF_HANDLER_FROM_INTERRUPT_TIMEOUT
    CoreTask_Stop(stddlpsirf_timeout_handler_task);
#endif
    
    DllPsiRfHandleStateMachine();
    
#if STDDLLPSIRF_HANDLER_FROM_INTERRUPT_TIMEOUT
    CoreTask_Start(stddlpsirf_timeout_handler_task);
#endif
    
    if(CoreQ_Read(dllpsirf_inbox_q_hndl, (VPTR)&inbx_msg, 1))
    {
        DllPsiRfHandleRxMessage(&inbx_msg);
    }
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_UpdateRegister(U8 reg_number, U8 value)
{
    if(reg_number < REG_COUNT)
    {
        dllpsirf_registers[reg_number] = value;
    }
    DllPsiRfOnEntry(DLL_PSIRF_INIT);
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_UpdatePaTable(U8 index, U8 value)
{
    if(index < 8)
    {
        dllpsirf_pa_table[index] = value;
    }
    DllPsiRfOnEntry(DLL_PSIRF_INIT);
}
//------------------------------------------------------------------------------------------------//
void CommDllPsiRf_SendFrame(U8* data_ptr)
{
    if(data_ptr[0] < STDDLLPSIRF_FRAME_LENGTH)
    {
        if(CoreQ_Write(dllpsirf_outbox_q_hndl, (VPTR)data_ptr, 1) == FALSE)
        {
            LOG_WRN("OQ2");
        }
    }
    else
    {
        LOG_DBG("[DLLTXBUF] length send frame too long");
    }
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
    LOG_TRM("DLL CCxx Regs   : %02h", PU8A(dllpsirf_registers, 16));
    LOG_TRM("                  %02h", PU8A(&dllpsirf_registers[16], 16));
    LOG_TRM("                  %02h", PU8A(&dllpsirf_registers[32], 15));
    LOG_TRM("DLL CCxx PA Tbl : %02h", PU8A(dllpsirf_pa_table, 8));
    LOG_TRM("DLL TX Delay    : %d", PU8(dllpsirf_wait_for_tx));
    LOG_TRM("DLL RX Buffer   : %02x", PU8A(dllpsirf_inbox_msg.buffer, dllpsirf_inbox_msg.buffer[0]+1));
    LOG_TRM("DLL TX Buffer   : %02x", PU8A(dllpsirf_outbox_msg.buffer, dllpsirf_outbox_msg.buffer[0]+1));
}
//================================================================================================//

