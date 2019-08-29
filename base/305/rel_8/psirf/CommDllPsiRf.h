//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Header file for the Data Link Layer for the PsiRf-protocol.
// The main function of this layer is to transmit frames of characters between master and salve equipment. The
// layer serves as a communication medium to the network layer.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef PSIRF__COMMDLLPSIRF_H
#define PSIRF__COMMDLLPSIRF_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max length of one PSI RF message
#ifndef STDDLLPSIRF_FRAME_LENGTH
    #error "STDDLLPSIRF_FRAME_LENGTH not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//DRIVER lib include section
#include "spi\DrvSpiMasterChannel.h"
#include "gpio\DrvGpio.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define CC1101_IOCFG2       0x00        // GDO2 output pin configuration
#define CC1101_IOCFG1       0x01        // GDO1 output pin configuration
#define CC1101_IOCFG0       0x02        // GDO0 output pin configuration
#define CC1101_FIFOTHR      0x03        // RX FIFO and TX FIFO thresholds
#define CC1101_SYNC1        0x04        // Sync word, high byte
#define CC1101_SYNC0        0x05        // Sync word, low byte
#define CC1101_PKTLEN       0x06        // Packet length
#define CC1101_PKTCTRL1     0x07        // Packet automation control
#define CC1101_PKTCTRL0     0x08        // Packet automation control
#define CC1101_ADDR         0x09        // Device address
#define CC1101_CHANNR       0x0A        // Channel number
#define CC1101_FSCTRL1      0x0B        // Frequency synthesizer control
#define CC1101_FSCTRL0      0x0C        // Frequency synthesizer control
#define CC1101_FREQ2        0x0D        // Frequency control word, high byte
#define CC1101_FREQ1        0x0E        // Frequency control word, middle byte
#define CC1101_FREQ0        0x0F        // Frequency control word, low byte
#define CC1101_MDMCFG4      0x10        // Modem configuration
#define CC1101_MDMCFG3      0x11        // Modem configuration
#define CC1101_MDMCFG2      0x12        // Modem configuration
#define CC1101_MDMCFG1      0x13        // Modem configuration
#define CC1101_MDMCFG0      0x14        // Modem configuration
#define CC1101_DEVIATN      0x15        // Modem deviation setting
#define CC1101_MCSM2        0x16        // Main Radio Control State Machine configuration
#define CC1101_MCSM1        0x17        // Main Radio Control State Machine configuration
#define CC1101_MCSM0        0x18        // Main Radio Control State Machine configuration
#define CC1101_FOCCFG       0x19        // Frequency Offset Compensation configuration
#define CC1101_BSCFG        0x1A        // Bit Synchronization configuration
#define CC1101_AGCCTRL2     0x1B        // AGC control
#define CC1101_AGCCTRL1     0x1C        // AGC control
#define CC1101_AGCCTRL0     0x1D        // AGC control
#define CC1101_WOREVT1      0x1E        // High byte Event 0 timeout
#define CC1101_WOREVT0      0x1F        // Low byte Event 0 timeout
#define CC1101_WORCTRL      0x20        // Wake On Radio control
#define CC1101_FREND1       0x21        // Front end RX configuration
#define CC1101_FREND0       0x22        // Front end TX configuration
#define CC1101_FSCAL3       0x23        // Frequency synthesizer calibration
#define CC1101_FSCAL2       0x24        // Frequency synthesizer calibration
#define CC1101_FSCAL1       0x25        // Frequency synthesizer calibration
#define CC1101_FSCAL0       0x26        // Frequency synthesizer calibration
#define CC1101_RCCTRL1      0x27        // RC oscillator configuration
#define CC1101_RCCTRL0      0x28        // RC oscillator configuration
#define CC1101_FSTEST       0x29        // Frequency synthesizer calibration control
#define CC1101_PTEST        0x2A        // Production test
#define CC1101_AGCTEST      0x2B        // AGC test
#define CC1101_TEST2        0x2C        // Various test settings
#define CC1101_TEST1        0x2D        // Various test settings
#define CC1101_TEST0        0x2E        // Various test settings

#define REG_COUNT           0x2F
#define PA_COUNT            8
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Prototype of the frame handling function
typedef BOOL (*DLLPSIRFHOOK_FRAME_RECEIVED)(U8* frame_ptr, U8 rssi);

// @brief   Structure holding the register information
typedef struct
{
    U8  registers[REG_COUNT];
    U8  pa_table[PA_COUNT];
}
COMMDLLPSIRF_DATA;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern COMMDLLPSIRF_DATA        commdllpsirf_data;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialiser for the DataLink Layer entity
// Initialises the DataLink Layer and registers this entity to the Module Manager.\n
// @param   spi_channel :            The SPI channel
// @param   frame_hook :             Pointer to the frame handling function
void CommDllPsiRf_Init(SPI_CHANNEL_HNDL channel_hndl,
                       DRVGPIO_PIN_HNDL cs_pin_hndl,
                       DRVGPIO_PIN_HNDL gdo0_pin_hndl,
                       DRVGPIO_PIN_HNDL gdo2_pin_hndl);

void CommDllPsiRf_RegisterFrameHook(DLLPSIRFHOOK_FRAME_RECEIVED frame_hook);

void CommDllPsiRf_RegisterLogHook(DLLPSIRFHOOK_FRAME_RECEIVED frame_hook);

// @brief   Function to be called in the background loop
// This function will be called in background. It will call be executed when a complete new message is received.
// The frame handling function will be called and the message will be interpreted.
void CommDllPsiRf_Handler(void);

BOOL CommDllPsiRf_UpdateRegister(U8 reg_number, U8 value);
BOOL CommDllPsiRf_UpdatePaTable(U8 index, U8 value);

// @brief   Function to send a frame.\n
// @param   ptr_ptr :        Pointer to the data to be transmitted.
void CommDllPsiRf_SendFrame(U8* data_ptr);

// @brief   Function to enable/disable the RF communication
void CommDllPsiRf_EnableCommunication(BOOL enable_communication);

BOOL CommDllPsiRf_IsCommunicationEnabled(void);

// @brief   Print all local variables of this DLL layer at current time.
void CommDllPsiRf_PrintStatus(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// C L E A R / U N D E F    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* PSIRF__COMMDLLPSIRF_H */

