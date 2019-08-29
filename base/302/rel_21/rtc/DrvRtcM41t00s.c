//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the M41T00S Rtc driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define RTC__DRVRTCM41T00S_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef RTC__DRVRTCM41T00S_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               RTC__DRVRTCM41T00S_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef RTC_COUNT
    #define RTC_COUNT                   1
#endif
//------------------------------------------------------------------------------------------------//
//mvbl2: if Isys (lib0) is not updated where ISysCompiler is not known
//then this code is needed:
#ifndef NOP
    #ifdef  __ICCARM__ 
        //IAR
        #define NOP                     asm("nop");
    #elif defined(__ARMCC_VERSION)
        //KEIL
        #define NOP                     __nop();
    #else
        #define NOP   
    #endif
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "rtc\DrvRtcM41t00s.h"
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
static U8 Rtc_ConvertHexToDec(U8 hex_value);
static U8 Rtc_ConvertDecToHex(U8 dec_value);
static BOOL DrvRtcM41t00s_SetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr);
static BOOL DrvRtcM41t00s_GetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static const RTC_HOOK_LIST                  rtc_hook_list = {DrvRtcM41t00s_SetTime, DrvRtcM41t00s_GetTime};
static RTC_STRUCT                           rtc_struct[RTC_COUNT];
static U8                                   rtc_count;

static U8                                   rtc_i2c_buffer[10];
static I2C_DEVICE_ID                        rtc_i2c_hndl[RTC_COUNT];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static U8 Rtc_ConvertHexToDec(U8 hex_value)
{
    return (U8)(((hex_value >> 4) * 10) + (hex_value & 0xF));
}
//------------------------------------------------------------------------------------------------//
static U8 Rtc_ConvertDecToHex(U8 dec_value)
{
    U8  decimal = dec_value / 10;
    return (U8)((decimal << 4) + (dec_value - (decimal * 10)));
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRtcM41t00s_SetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr)
{
    I2C_DEVICE_ID   i2c_hndl = rtc_i2c_hndl[rtc_id];
    
    if(rtc_id >= rtc_count)
    {
        return FALSE;
    }
    
    // set register address 0
    rtc_i2c_buffer[0] = 0;
    
    rtc_i2c_buffer[1] = Rtc_ConvertDecToHex(rtc_time_ptr->second);
    rtc_i2c_buffer[2] = Rtc_ConvertDecToHex(rtc_time_ptr->minute);
    rtc_i2c_buffer[3] = Rtc_ConvertDecToHex(rtc_time_ptr->hour) + (((rtc_time_ptr->year / 100) & 0x01) << 6) + 0x80;
    rtc_i2c_buffer[4] = rtc_time_ptr->day_of_week + 1;
    rtc_i2c_buffer[5] = Rtc_ConvertDecToHex(rtc_time_ptr->day_of_month);
    rtc_i2c_buffer[6] = Rtc_ConvertDecToHex(rtc_time_ptr->month);
    rtc_i2c_buffer[7] = Rtc_ConvertDecToHex(rtc_time_ptr->year % 100);
    
    return DrvI2cMasterDevice_WriteData(i2c_hndl, rtc_i2c_buffer, 8, TRUE);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRtcM41t00s_GetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr)
{
    I2C_DEVICE_ID   i2c_hndl = rtc_i2c_hndl[rtc_id];
    U16             i;
    
    if(rtc_id >= rtc_count)
    {
        return FALSE;
    }
    
    // set register address 0
    rtc_i2c_buffer[0] = 0;
    if(DrvI2cMasterDevice_WriteData(i2c_hndl, &rtc_i2c_buffer[0], 1, TRUE) == FALSE)
    {
        return FALSE;
    }
    
    // read time
    if(DrvI2cMasterDevice_ReadData(i2c_hndl, &rtc_i2c_buffer[1], 8, TRUE))
    {
        rtc_time_ptr->second        = Rtc_ConvertHexToDec(rtc_i2c_buffer[1] & 0x7F);
        rtc_time_ptr->minute        = Rtc_ConvertHexToDec(rtc_i2c_buffer[2] & 0x7F);
        rtc_time_ptr->hour          = Rtc_ConvertHexToDec(rtc_i2c_buffer[3] & 0x3F);
        rtc_time_ptr->day_of_week   = (DAY_OF_WEEK)((rtc_i2c_buffer[4] & 0x07) - 1);
        rtc_time_ptr->day_of_month  = Rtc_ConvertHexToDec(rtc_i2c_buffer[5] & 0x3F);
        rtc_time_ptr->month         = (MONTH)Rtc_ConvertHexToDec(rtc_i2c_buffer[6] & 0x1F);
        rtc_time_ptr->year          = 2000 + 100*((rtc_i2c_buffer[3] >> 6) & 0x01) + Rtc_ConvertHexToDec(rtc_i2c_buffer[7] & 0xFF);
        
        // check for oscillator failure
        if(rtc_i2c_buffer[2] & 0x80)
        {
            LOG_DBG("kick");
            // give some time
            for(i=0; i < 1000; i++)
            {
                NOP;
            }
            
            // kick oscillator
            rtc_i2c_buffer[1] |= 0x80;
            if(DrvI2cMasterDevice_WriteData(i2c_hndl, rtc_i2c_buffer, 9, TRUE) == FALSE)
            {
                return FALSE;
            }
            
            // give some time
            for(i=0; i < 1000; i++)
            {
                NOP;
            }
            
            rtc_i2c_buffer[1] &= ~0x80;
            if(DrvI2cMasterDevice_WriteData(i2c_hndl, rtc_i2c_buffer, 9, TRUE) == FALSE)
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvRtcM41t00s_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)rtc_struct, 0, SIZEOF(rtc_struct));
    rtc_count = 0;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
RTC_HNDL DrvRtcM41t00s_Register(I2C_DEVICE_ID i2c_device_id)
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
        rtc_i2c_hndl[i]             = i2c_device_id;
        
        rtc_hndl->hook_list_ptr     = (RTC_HOOK_LIST*)&rtc_hook_list;
        rtc_hndl->rtc_id            = rtc_count;
        rtc_count++;
        return rtc_hndl;
    }
    return NULL;
}
//================================================================================================//
