//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Application main source file
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define APPTOUCH_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
#include <stdlib.h>
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef APPTOUCH_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               APPTOUCH_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// SYS
#include "gpio\SysPin.h"

// DRV
#include "gpio\DrvGpio.h"
#include "gpio\DrvGpioSys.h"
// APP
#include "AppTouch.h"
#include "IQS5xx.h"
#include "IQS5xx_Init.h"

//other...
#include "i2c\SysI2cMasterInt.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void AppTouch_settings(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static I2C_DEVICE_ID	i2c_device_id;
static U8	        data_buffer[60];
static U8               evenOrNot = 0;
static U8               timer = 200;
static DRVGPIO_PIN_HNDL RDYpin;
static DRVGPIO_PIN_HNDL NRSTpin;
static BOOL             RDY_wait = FALSE;
// Used to end communication window
U8 end_communication_window[4] = {0xEE,0xEE,0x00,0x01};  
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//

// @brief: Configuration of parameters on the IQS525
// refer to datasheet of IQS525 B000 for explanation of every register, pages are mentioned below
static void AppTouch_settings(void)
{
    // ====================================================================================================================//
    // set all configuration values
    // ====================================================================================================================//
  
  
     
    // Register 0x0431 = SystemControl0 ; 0x0432 = SystemControl1 ; p. 40 datasheet
    U8 system_control_0and1[4] = {0x04,0x31,0xB8,0x00};  // bit 5 of high byte is 'auto ATI bit' --> important that it is set so that auto ATI is performed at start - the bit is automatically cleared afterwards         
    
    // Register 0x058E = SystemConfig0 ; 0x058F = SystemConfig1 ; p. 42 datasheet
    U8 system_config_0and1[4] = {0x05, 0x8E, 0x6C, 0x24};
    
    // thresholds for touch, snap and proximity, starting at register 0x0592
    U8 thresholds[8];
    thresholds[0] = 0x05;  // address high byte
    thresholds[1] = 0x92;  // address low byte
    thresholds[2] = 0x00;  // Snap threshold high byte  -- not used in this implementation so value doesn't really matter
    thresholds[3] = 0x64;  // Snap threshold low byte   -- not used in this implementation so value doesn't really matter
    thresholds[4] = 0x17;  // Prox threshold - trackpad
    thresholds[5] = 0x08;  // Prox threshold - ALP channel
    thresholds[6] = 0x12;  // Global touch multiplier - set
    thresholds[7] = 0x0C;  // Global touch multiplier - clear
    
    U8 indiviual_multiplier_adjustments[150];  // Individual touch multiplier adjustments, for finetuning every channel separately - TODO if necessary
    
    // ATI = auto tuning, to balance out small variations between trackpad hardware and IQS525, to give similar performance accross devices
    // ATI targets = target values for the ATI tuning, the ATI compensation values will automatically be chosen such that each count value is close to the ATI target value
    U8 ATI_settings[13];
    ATI_settings[0] = 0x05;  // address high byte 
    ATI_settings[1] = 0x6D;  // address low byte
    ATI_settings[2] = 0x01;  // ATI target value (for normal mode) high byte
    ATI_settings[3] = 0xF4;  // ATI target value (for normal mode) low byte
    ATI_settings[4] = 0x01;  // ATI target value (for ALP channel) high byte
    ATI_settings[5] = 0xF4;  // ATI target value (for ALP channel) high byte
    ATI_settings[6] = 0x4B;  // Reference drift limit - p.17 datasheet - condition for Re-ATI to activate
    ATI_settings[7] = 0x32;  // ALP LTA drift limit - p.17  datasheet - condition for Re-ATI to activate
    ATI_settings[6] = 0x00;  // Re-ATI lower compensation limit - used to check if an ATI error hasn't occured - p. 18 datasheet
    ATI_settings[9] = 0xFF;  // Re-ATI upper compensation limit - used to check if an ATI error hasn't occured - p. 18 datasheet
    ATI_settings[10] = 0x07; // Max count limit high byte - reATI is triggered if max count limit is exceeded for 15 consevutive cycles --> indicates something is wrong - p.18 datasheet
    ATI_settings[11] = 0xD0; // Max count limit low byte - reATI is triggered if max count limit is exceeded for 15 consevutive cycles --> indicates something is wrong - p.18 datasheet
    ATI_settings[12] = 0x05; // reATI retry time - to prevent reAti repeating indefinetely in certain circumstances
    
    // Report rates ; faster report rate will have higher current consumption, but will give faster response to user interacton
    // Higher than the rates defined below doesn't work, the report rate is not achieved (too fast)
    // Lower speeds used for lower power modes, active mode has fastest report rate (32ms)
    // Note that lower value means faster report rate! (value stored here is actually the period)
    U8 report_rates[12]; 
    report_rates[0] = 0x05;  // address high byte
    report_rates[1] = 0x7A;  // address low byte
    report_rates[2] = 0x00;  // report rate in active mode, high byte
    report_rates[3] = 0x0D;  // report rate in active mode, low byte
    report_rates[4] = 0x00;  // report rate in idle touch mode, high byte
    report_rates[5] = 0x32;  // report rate in idle touch mode, low byte
    report_rates[6] = 0x00;  // report rate in idle mode, high byte
    report_rates[7] = 0x4B;  // report rate in idle mode, low byte
    report_rates[8] = 0x00;  // report rate in LP1 mode, high byte
    report_rates[9] = 0xC8;  // report rate in LP1 mode, low byte
    report_rates[10] = 0x01; // report rate in LP2 mode, high byte
    report_rates[10] = 0x90; // report rate in LP2 mode, low byte
    
    // Timeout times ; once these times have elapsed, the system will change to the next state according to the state diagram
    U8 timeout_times[9]; 
    timeout_times[0] = 0x05;  // address high byte
    timeout_times[1] = 0x84;  // address low byte
    timeout_times[2] = 0x05;  // timeout time in active mode
    timeout_times[3] = 0x3C;  // timeout time in idle touch mode
    timeout_times[4] = 0x14;  // timeout time in idle mode
    timeout_times[5] = 0x1E;  // timeout time in LP1 mode, in multiples of 20s!
    timeout_times[6] = 0x08;  // reference update time - updates the reference values (count values are compared against reference values to detect user interaction) - only happens in LP-modes
    timeout_times[7] = 0x0A;  // snap timeout -- not used here so doesn't matter
    timeout_times[8] = 0x64;  // I2C timeout - if communicaiton window is not serviced within this time, the session is ended and processing continues as normal
    
    // filter_settings ; enable MAV filter, IIR filter with dinamically adjusted damping factor (relative to XY movement), ALP count filter
    U8 filter_settings[11]; 
    filter_settings[0] = 0x06;  // address high byte
    filter_settings[1] = 0x32;  // address low byte
    filter_settings[2] = 0x0B;  // enable all filters and set IIR filtering method to dynamic
    filter_settings[3] = 0x80;  // XY static beta
    filter_settings[4] = 0x32;  // ALP count beta
    filter_settings[5] = 0x08;  // ALP1 LTA beta
    filter_settings[6] = 0x06;  // ALP2 LTA beta
    filter_settings[7] = 0x07;  // XY dynamic filter – bottom beta 
    filter_settings[8] = 0x06;  // XY dynamic filter – lower speed
    filter_settings[9] = 0x00;  // XY dynamic filter – upper speed, high byte
    filter_settings[10] = 0x7C; // XY dynamic filter – upper speed, high byte
    
    
    // =====================================================================================================================//
    // write configuration to IQS525
    // =====================================================================================================================//
    
       
    while(!RDY_wait)
    {
      RDY_wait = DrvGpio_GetPin(RDYpin);
    }
    //DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 1, TRUE, 0xEEEE);
    //DrvI2cMasterDevice_WriteData(i2c_device_id, end_communication_window, sizeof(end_communication_window)/sizeof(U8), TRUE);
    DrvI2cMasterDevice_WriteData(i2c_device_id, system_control_0and1, sizeof(system_control_0and1)/sizeof(U8), TRUE);
    DrvI2cMasterDevice_WriteData(i2c_device_id, system_config_0and1, sizeof(system_config_0and1)/sizeof(U8), TRUE);
    
    
    // End communication window by sending some data to 0xEEEE. This will allow the ATI procedure to happen. I2C communication will reume again once the ATI routine has completed.
    //DrvI2cMasterDevice_WriteData(i2c_device_id, end_communication_window, sizeof(end_communication_window)/sizeof(U8), TRUE);
    // ATI happening now...
    
//    DrvI2cMasterDevice_WriteData(i2c_device_id, thresholds, sizeof(thresholds)/sizeof(U8), TRUE);
//    DrvI2cMasterDevice_WriteData(i2c_device_id, indiviual_multiplier_adjustments, sizeof(indiviual_multiplier_adjustments)/sizeof(U8), TRUE);
//    DrvI2cMasterDevice_WriteData(i2c_device_id, ATI_settings, sizeof(ATI_settings)/sizeof(U8), TRUE);
//    DrvI2cMasterDevice_WriteData(i2c_device_id, report_rates, sizeof(report_rates)/sizeof(U8), TRUE);
//    DrvI2cMasterDevice_WriteData(i2c_device_id, timeout_times, sizeof(timeout_times)/sizeof(U8), TRUE);    
//    DrvI2cMasterDevice_WriteData(i2c_device_id, filter_settings, sizeof(filter_settings)/sizeof(U8), TRUE);
    
    // Wake i2C and then wait at least 150us (if we don't wait 150us the device doesn't wake)
    
}    
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
    
void AppTouch_Init(I2C_CHANNEL_HNDL i2c_channel, U8 address)
{
  
    MODULE_INIT_ONCE();
    U8 data[2] = {0,0};
    RDYpin = DrvGpioSys_RegisterPin(GPIO_PORT_A, 3, PIN_INPUT);
    i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, address, 100000);
    //NRSTpin = DrvGpioSys_RegisterPin(GPIO_PORT_A, 2, PIN_OUTPUT);
    //DrvGpio_SetPin(NRSTpin,0);   //appears to break something
    AppTouch_settings();  // configure all parameters related to the touchpad
    
    MODULE_INIT_DONE();
}

//------------------------------------------------------------------------------------------------//

void AppTouch_Handler(void)
{
    MODULE_CHECK();
}

//------------------------------------------------------------------------------------------------//

U8 AppTouch_GetTouch(U16* x, U16* y)
{
    while(!RDY_wait)
    {
      RDY_wait = DrvGpio_GetPin(RDYpin);
    }
    //read  x,y data from the touchpad
    //DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 1, TRUE, 0x0431);

    if(DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 10, TRUE, 0x0011) )//DrvI2cMasterDevice_ReadData(i2c_device_id, &data_buffer[0], 8, TRUE))
    {
        *x = ((data_buffer[5] << 8) + data_buffer[6]);
        *y = ((data_buffer[7] << 8) + data_buffer[8]); 
        //DrvI2cMasterDevice_WriteData(i2c_device_id, end_communication_window, sizeof(end_communication_window)/sizeof(U8), TRUE);
        //DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 1, TRUE, 0x00EE);
        return (data_buffer[0]);
    }
    
   //DrvI2cMasterDevice_WriteData(i2c_device_id, end_communication_window, sizeof(end_communication_window)/sizeof(U8), TRUE);
   //DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 1, TRUE, 0xEEEE);
    return 0;
}

//================================================================================================//
// @ brief : used to write and read to registers - only for testing purposes
U8 AppTouch_GetData(U16* x, U16* y)
{
  
  U8 testdata[5] = {0xAA, 0xBB, 0xCC};
  U8 defaultReg[4] = {0x00, 0x11, 0x099, 0x55};
  while(!RDY_wait)
  {
    RDY_wait = DrvGpio_GetPin(RDYpin);
  }
  if (evenOrNot % 2 == 0)
    
  {
    if (evenOrNot == 0)
    {
      DrvI2cMasterDevice_WriteData_specificSlaveRegister(i2c_device_id, testdata, 2, TRUE,0x0673); // write 2 bytes to 0x0673
      
    }
    else
    {
      DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, testdata, 2, TRUE,0x0673);  // read 2 bytes from 0x0673
    }
  }
  else if (evenOrNot > 1 && evenOrNot < 4)
  {
    //DrvI2cMasterDevice_ReadData_repStart(i2c_device_id, data_buffer, 8, TRUE, 0x01);   
    //DrvI2cMasterDevice_ReadData_specificSlaveRegister(i2c_device_id, data_buffer, 6, TRUE, 0x0000);
  }
  else
  {
  }
  
  evenOrNot = evenOrNot % 254;
  
  evenOrNot++;
  
  return 0;
}
//================================================================================================//
