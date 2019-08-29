//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Driver for the LCD display 64128M
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRVLCDDISPLAY64128M_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef DRVLCDDISPLAY64128M_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               DRVLCDDISPLAY64128M_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// SYS
#include "gpio\SysPin.h"
#include "gpio\SysGpio.h"

// DRV
#include "gpio\DrvGpio.h"
#include "gpio\DrvGpioSys.h"
#include "spi\DrvSpiMasterChannel.h"
#include "spi\DrvSpiMasterChannelSysBlock.h"
#include "spi\DrvSpiMasterDevice.h"
#include "timer\DrvPwmSys.h"
#include "display\DrvLcdDisplay64128M.h"

//EXT
#include "cmsis_os.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define CMD_DISPLAY_OFF                     0xAE
#define CMD_DISPLAY_ON                      0xAF

#define CMD_SET_DISP_START_LINE             0x40
#define CMD_SET_PAGE                        0xB0

#define CMD_SET_COLUMN_UPPER                0x10
#define CMD_SET_COLUMN_LOWER                0x00

#define CMD_SET_ADC_NORMAL                  0xA0
#define CMD_SET_ADC_REVERSE                 0xA1

#define CMD_SET_DISP_NORMAL                 0xA6
#define CMD_SET_DISP_REVERSE                0xA7

#define CMD_SET_ALLPTS_NORMAL               0xA4
#define CMD_SET_ALLPTS_ON                   0xA5
#define CMD_SET_BIAS_9                      0xA2 
#define CMD_SET_BIAS_7                      0xA3

#define CMD_RMW                             0xE0
#define CMD_RMW_CLEAR                       0xEE
#define CMD_INTERNAL_RESET                  0xE2
#define CMD_SET_COM_NORMAL                  0xC0
#define CMD_SET_COM_REVERSE                 0xC8
#define CMD_SET_POWER_CONTROL               0x28
#define CMD_SET_RESISTOR_RATIO              0x20
#define CMD_SET_VOLUME_FIRST                0x81
#define CMD_SET_VOLUME_SECOND               0
#define CMD_SET_STATIC_OFF                  0xAC
#define CMD_SET_STATIC_ON                   0xAD
#define CMD_SET_STATIC_REG                  0x0
#define CMD_SET_BOOSTER_FIRST               0xF8
#define CMD_SET_BOOSTER_234                 0
#define CMD_SET_BOOSTER_5                   1
#define CMD_SET_BOOSTER_6                   3
#define CMD_NOP                             0xE3
#define CMD_TEST                            0xF0
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void ExchangeData(U8 *write_data_ptr, U16 count);
static void SetDisplayCommand(U8 cmd);
static void InitDisplay(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SPI_DEVICE_ID                            spi_device_id;

static PWM_CHANNEL_HNDL                         bl_pwm_handle;
static DRVGPIO_PIN_HNDL                         rs_pin_handle;
static DRVGPIO_PIN_HNDL                         a0_pin_handle;

static const U8                                 (**frame_ptr_ptr)[PAGES][COLUMNS];

static U8                                       lcd_brightness;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void ExchangeData(U8 *write_data_ptr, U16 count)
{
    DrvSpiMasterDevice_SelectExchange(spi_device_id, (U8*)write_data_ptr, (U8*) 0, count);
}
//------------------------------------------------------------------------------------------------//
static void SetDisplayCommand(U8 cmd)
{    
    DrvGpio_SetPin(a0_pin_handle, FALSE);        
    DrvSpiMasterDevice_SelectExchange(spi_device_id, &cmd, (U8*) 0, (U16) 1);  
}
//------------------------------------------------------------------------------------------------//
//see page 51 of the Sitronix-ST7565R reference manual for the initialization routine
//see page 50 for a table of the commands
static void InitDisplay(void)
{   
    U32 wait_count;
    
    // Turn On the power and keep /RES pin low
    DrvGpio_SetPin(rs_pin_handle, FALSE);
    DrvPwm_SetState(bl_pwm_handle, TRUE);
    
    // Wait for reset circuit finished (<1ms)
    for(wait_count = 0; wait_count < 216000; wait_count++){};
    
    // Power should have been stabilized by now
    // Release reset state (/RES = "H")
    DrvGpio_SetPin(rs_pin_handle, TRUE);
    
    // Wait for reset circuit finished (<1ms)
    for(wait_count = 0; wait_count < 216000; wait_count++){};
    
    // Initialized state (Default)
    
    // Function setup by command input
    // LCD bias setting
    SetDisplayCommand(CMD_SET_BIAS_9);                  // 1/65 duty; 1/9 bias
    // ADC selection
    SetDisplayCommand(CMD_SET_ADC_REVERSE);
    // Common output mode selection
    SetDisplayCommand(CMD_SET_COM_NORMAL);
    // Initial display line
    SetDisplayCommand(CMD_SET_DISP_START_LINE);
    
    // Set internal V0 regulator resistor ratio    
    SetDisplayCommand(CMD_SET_RESISTOR_RATIO | 0x04);
    
    DrvLcdDisplay64128M_SetBrightness(lcd_brightness);  // max value = 63
    
    // Power control set
    SetDisplayCommand(CMD_SET_POWER_CONTROL | 0x07);
    
    // Booster ratio set
    SetDisplayCommand(CMD_SET_BOOSTER_FIRST);
    SetDisplayCommand(CMD_SET_BOOSTER_234);             // 4x boost
    
    // Display ON
    SetDisplayCommand(CMD_DISPLAY_ON);

    // Display All Points Normal
    SetDisplayCommand(CMD_SET_ALLPTS_NORMAL);
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvLcdDisplay64128M_Init(SPI_CHANNEL_HNDL spi_channel_hndl,
                              U32              spi_speed,
                              PWM_CHANNEL_HNDL backlight_pwm_hndl,
                              DRVGPIO_PIN_HNDL chip_select_pin_hndl,
                              DRVGPIO_PIN_HNDL reset_pin_hndl,
                              DRVGPIO_PIN_HNDL a0_pin_hndl,
                              const U8         (**frame_data_ptr_ptr)[PAGES][COLUMNS])
{
    MODULE_INIT_ONCE();
    
    SPI_CONFIG_STRUCT config_struct;      
    
    rs_pin_handle = reset_pin_hndl;
    bl_pwm_handle = backlight_pwm_hndl;
    a0_pin_handle = a0_pin_hndl;
    frame_ptr_ptr = frame_data_ptr_ptr;
    
    DrvGpio_SetPin(rs_pin_handle, TRUE);    // reset is active low
    
    config_struct.speed     = spi_speed;
    config_struct.mode      = MODE_3;
    config_struct.bitcount  = 8;
    config_struct.lsb_first = FALSE;

    spi_device_id = DrvSpiMasterDevice_Register(spi_channel_hndl, chip_select_pin_hndl, &config_struct, NULL);
    DrvSpiMasterDevice_Config(spi_device_id, &config_struct);
    
    lcd_brightness = 35;
    
    InitDisplay();
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void DrvLcdDisplay64128M_Refresh(void)
{
    const U8 (*frame_data_ptr)[PAGES][COLUMNS];
    U8   page;
    U8  *page_data_ptr;
    
    frame_data_ptr = *frame_ptr_ptr;

	for(page = 0; page < PAGES; page++)
	{
        page_data_ptr = (U8*)&(*frame_data_ptr)[page][0];
        
        // A0 FALSE marks a command data exchange
        DrvGpio_SetPin(a0_pin_handle, FALSE);        
        // Display start line set
        SetDisplayCommand(CMD_SET_DISP_START_LINE);        
        // Set page
        SetDisplayCommand(CMD_SET_PAGE | page);
        // Set column start address
        SetDisplayCommand(CMD_SET_COLUMN_LOWER | 4);
        SetDisplayCommand(CMD_SET_COLUMN_UPPER);
        // Column address increment at write +1
        SetDisplayCommand(CMD_RMW);
        
        // A0 TRUE marks a display data exchange
        DrvGpio_SetPin(a0_pin_handle, TRUE);        
        ExchangeData(page_data_ptr, COLUMNS);
        
        // A0 FALSE marks a command data exchange
        DrvGpio_SetPin(a0_pin_handle, FALSE);
        //End of read modify write
        SetDisplayCommand(CMD_RMW_CLEAR);
    }
}
//------------------------------------------------------------------------------------------------//
void DrvLcdDisplay64128M_SetBrightness(U8 brightness)
{
    lcd_brightness = brightness;
    
    // Electronic volume control
    SetDisplayCommand(CMD_SET_VOLUME_FIRST);
    SetDisplayCommand(lcd_brightness & 0x3F);
}
//------------------------------------------------------------------------------------------------//
void DrvLcdDisplay64128M_SetBacklight(BOOL state)
{
    if(state)
    {
        DrvLcdDisplay64128M_SetBrightness(lcd_brightness);  // max value = 63
    }
    else
    {
        DrvLcdDisplay64128M_SetBrightness(0);
    }
    DrvPwm_SetState(bl_pwm_handle, state);
}
//================================================================================================//
