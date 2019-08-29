//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// brief explanation
//
// Copyright (c), PsiControl NV, All rights reserved.
//================================================================================================//
#define RTC__DRVRTCPCF85263A_C
//================================================================================================//
 
 
 
//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef RTC__DRVRTCPCF85263A_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               RTC__DRVRTCPCF85263A_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef RTC_COUNT
    #define RTC_COUNT                   1
#endif
//================================================================================================//
 
 
 
//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
 
// DRV
 
// STD
 
// COM
 
// DRV
#include "DrvRtcPcf85263a.h"
//================================================================================================//
 
 
 
//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define STOP_ENABLE_REGISTER            (0x2E)
#define RESET_REGISTER                  (0x2F)
#define RESET_REGISTER_CLEAR_PRESCALER  (0xA4)
//================================================================================================//
 
 
 
//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
 
 
 
//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U8 ConvertHexToDec(U8 hex_value);
static U8 ConvertDecToHex(U8 dec_value);
static BOOL WriteData(RTC_ID rtc_id, U8 address, U8* data, U8 data_length);
static BOOL ReadData(RTC_ID rtc_id, U8 address, U8* data, U8 data_length);
static BOOL SetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr); 
static BOOL GetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//
 
 
 
//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
 
static I2C_DEVICE_ID        rtc_i2c_hndl[RTC_COUNT];
static const RTC_HOOK_LIST  rtc_hook_list = {SetTime, GetTime};
static RTC_STRUCT           rtc_struct[RTC_COUNT];
static U8                   rtc_count;
//================================================================================================//
 
 
 
//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
 
 
 
//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static U8 ConvertHexToDec(U8 hex_value)
{
    return (U8)(((hex_value >> 4) * 10) + (hex_value & 0xF));
}
//------------------------------------------------------------------------------------------------//
static U8 ConvertDecToHex(U8 dec_value)
{
    U8  decimal = dec_value / 10;
    return (U8)((decimal << 4) + (dec_value - (decimal * 10)));
}
//------------------------------------------------------------------------------------------------//
static BOOL WriteData(RTC_ID rtc_id, U8 address, U8* data, U8 data_length)
{
    if(rtc_id >= rtc_count)
    {
        return FALSE;
    }

    U8 rtc_i2c_buffer[9];

    if(data_length > (sizeof(rtc_i2c_buffer) - 1))
    {
        return FALSE; // data_length to large
    }

    I2C_DEVICE_ID i2c_hndl = rtc_i2c_hndl[rtc_id];
    rtc_i2c_buffer[0] = address;
    MEMCPY(&rtc_i2c_buffer[1], data, data_length);

    if(DrvI2cMasterDevice_WriteData(i2c_hndl, rtc_i2c_buffer, data_length + 1, TRUE))
    {
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL ReadData(RTC_ID rtc_id, U8 address, U8* data, U8 data_length)
{
    if(rtc_id >= rtc_count)
    {
        return FALSE;
    }

    I2C_DEVICE_ID i2c_hndl = rtc_i2c_hndl[rtc_id];

    if(DrvI2cMasterDevice_WriteData(i2c_hndl, &address, 1, TRUE) == FALSE)
    {
        return FALSE;
    }

    if(DrvI2cMasterDevice_ReadData(i2c_hndl, data, data_length, TRUE))
    {
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL SetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr)
{
    U8 rtc_i2c_buffer[8];

    // Stop RTC clock
    rtc_i2c_buffer[0] = 1;
    if(WriteData(rtc_id, STOP_ENABLE_REGISTER, rtc_i2c_buffer, 1) == FALSE)
    {
        return FALSE;
    }

    // Clear prescaler
    rtc_i2c_buffer[0] = RESET_REGISTER_CLEAR_PRESCALER;
    if(WriteData(rtc_id, RESET_REGISTER, rtc_i2c_buffer, 1) == FALSE)
    {
        return FALSE;
    }

    // Set time
    rtc_i2c_buffer[0] = 0;
    rtc_i2c_buffer[1] = ConvertDecToHex(rtc_time_ptr->second);
    rtc_i2c_buffer[2] = ConvertDecToHex(rtc_time_ptr->minute);
    rtc_i2c_buffer[3] = ConvertDecToHex(rtc_time_ptr->hour);
    rtc_i2c_buffer[4] = ConvertDecToHex(rtc_time_ptr->day_of_month);
    rtc_i2c_buffer[5] = rtc_time_ptr->day_of_week;
    rtc_i2c_buffer[6] = ConvertDecToHex(rtc_time_ptr->month);
    rtc_i2c_buffer[7] = ConvertDecToHex(rtc_time_ptr->year % 100);
    if(WriteData(rtc_id, 0x00, rtc_i2c_buffer, 8) == FALSE)
    {
        return FALSE;
    }
    
    // Start RTC clock
    rtc_i2c_buffer[0] = 0;
    if(WriteData(rtc_id, STOP_ENABLE_REGISTER, rtc_i2c_buffer, 1) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL GetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr)
{
    U8 rtc_i2c_buffer[8];
    if(ReadData(rtc_id, 0, rtc_i2c_buffer, 8) == FALSE)
    {
        return FALSE;
    }

    rtc_time_ptr->second       = ConvertHexToDec(rtc_i2c_buffer[1] & 0x7F);
    rtc_time_ptr->minute       = ConvertHexToDec(rtc_i2c_buffer[2] & 0x7F);
    rtc_time_ptr->hour         = ConvertHexToDec(rtc_i2c_buffer[3] & 0x3F);
    rtc_time_ptr->day_of_month = ConvertHexToDec(rtc_i2c_buffer[4] & 0x3F);
    rtc_time_ptr->day_of_week  = (DAY_OF_WEEK)(rtc_i2c_buffer[5] & 0x07);
    rtc_time_ptr->month        = (MONTH)ConvertHexToDec(rtc_i2c_buffer[6] & 0x1F);
    rtc_time_ptr->year         = 2000 + ConvertHexToDec(rtc_i2c_buffer[7]);
    return TRUE;
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
void DrvRtcPcf85263a_Init(void)
{
    MODULE_INIT_ONCE();
    //place init code which must only be executed once here
    
    
    MODULE_INIT_DONE();
    //place init code which must executed on every re-init here
}
//------------------------------------------------------------------------------------------------//
RTC_HNDL DrvRtcPcf85263a_Register(I2C_DEVICE_ID i2c_device_id)
{
    U8          i;
    RTC_HNDL    rtc_hndl = &rtc_struct[rtc_count];    
    
    MODULE_CHECK();
    
    for(i = 0; i < rtc_count; i++)
    {
        if(rtc_i2c_hndl[i] == i2c_device_id)
        {
            return &rtc_struct[i];
        }
    }
    
    if(rtc_count < RTC_COUNT)
    {
        rtc_i2c_hndl[i]         = i2c_device_id;
        rtc_hndl->hook_list_ptr = (RTC_HOOK_LIST*)&rtc_hook_list;
        rtc_hndl->rtc_id        = rtc_count;
        rtc_count++;
        
        return rtc_hndl;
    }
    return NULL;
}
//================================================================================================//
