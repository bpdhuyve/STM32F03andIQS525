//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the M41T82Z Rtc driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define RTC__DRVRTCM41T82Z_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef RTC__DRVRTCM41T82Z_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               RTC__DRVRTCM41T82Z_LOG_LEVEL
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
#include "rtc\DrvRtcM41t82z.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define ADDRESS_POINTER_SIZE            1
#define CLOCK_REGISTER_COUNT            8
#define CONTROL_REGISTER_COUNT          24
#define HT_MASK                         0x40
#define OF_MASK                         0x04
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
static BOOL DrvRtcM41t82z_ReadHaltBit(RTC_HNDL rtc_hndl);
static BOOL DrvRtcM41t82z_SetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr);
static BOOL DrvRtcM41t82z_GetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static const RTC_HOOK_LIST                  rtc_hook_list = {DrvRtcM41t82z_SetTime, DrvRtcM41t82z_GetTime};
static RTC_STRUCT                           rtc_struct[RTC_COUNT];
static U8                                   rtc_count;
static BOOL                                 first_read[RTC_COUNT];    

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
BOOL DrvRtcM41t82z_ReadHaltBit(RTC_HNDL rtc_hndl)
{
    I2C_DEVICE_ID i2c_device_id = rtc_i2c_hndl[rtc_hndl->rtc_id];
    U8  rtc_i2c_buffer[2];    
    U8* address_buf_ptr = &rtc_i2c_buffer[0];
    U8* registers_buf_ptr = &rtc_i2c_buffer[1];

    // set register address 0x0C
    *address_buf_ptr = 0x0C;
    if(DrvI2cMasterDevice_WriteData(rtc_i2c_hndl[i2c_device_id], address_buf_ptr, ADDRESS_POINTER_SIZE, TRUE) == FALSE)
    {
        return FALSE;
    }
    
    // read register
    if(DrvI2cMasterDevice_ReadData(rtc_i2c_hndl[i2c_device_id], registers_buf_ptr, 1, TRUE))
    {
        return (BOOL) ((*registers_buf_ptr & HT_MASK) > 0); // clear halt bit       
    }    
    
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRtcM41t82z_SetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr)
{
    I2C_DEVICE_ID   i2c_hndl = rtc_i2c_hndl[rtc_id];
    U8              rtc_i2c_buffer[ADDRESS_POINTER_SIZE + CLOCK_REGISTER_COUNT];
    U8*             address_buf_ptr = &rtc_i2c_buffer[0];
    U8*             registers_buf_ptr = &rtc_i2c_buffer[1];
    
    if(rtc_id >= rtc_count)
    {
        return FALSE;
    }
    
    // set register address 0
    *address_buf_ptr = 0;
    if(DrvI2cMasterDevice_WriteData(i2c_hndl, address_buf_ptr, ADDRESS_POINTER_SIZE, TRUE) == FALSE)
    {
        return FALSE;
    }
    
    // read registers
    if(DrvI2cMasterDevice_ReadData(i2c_hndl, registers_buf_ptr, CLOCK_REGISTER_COUNT, TRUE))
    {
        *address_buf_ptr = 0; // on-chip address pointer
        registers_buf_ptr[1]   = Rtc_ConvertDecToHex(rtc_time_ptr->second);
        registers_buf_ptr[2]   = Rtc_ConvertDecToHex(rtc_time_ptr->minute);
        registers_buf_ptr[3]   = Rtc_ConvertDecToHex(rtc_time_ptr->hour) + (((rtc_time_ptr->year / 100) & 0x01) << 6) + 0x80;
        registers_buf_ptr[4]   = rtc_time_ptr->day_of_week + 1;
        registers_buf_ptr[5]   = Rtc_ConvertDecToHex(rtc_time_ptr->day_of_month);
        registers_buf_ptr[6]   = Rtc_ConvertDecToHex(rtc_time_ptr->month);
        registers_buf_ptr[7]   = Rtc_ConvertDecToHex(rtc_time_ptr->year % 100);
        
        // write back updated registers
        U8 count = ADDRESS_POINTER_SIZE + CLOCK_REGISTER_COUNT;
        return DrvI2cMasterDevice_WriteData(i2c_hndl, rtc_i2c_buffer, count, TRUE);
    }
    
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvRtcM41t82z_GetTime(RTC_ID rtc_id, RTC_TIME* rtc_time_ptr)
{
    RTC_HNDL        rtc_hndl;
    I2C_DEVICE_ID   i2c_hndl;// = rtc_i2c_hndl[rtc_id];
    U16             i;    
    U8              rtc_i2c_buffer[ADDRESS_POINTER_SIZE + CLOCK_REGISTER_COUNT];
    U8*             address_buf_ptr = &rtc_i2c_buffer[0];
    U8*             registers_buf_ptr = &rtc_i2c_buffer[1];
    
    MODULE_CHECK();
    
    if(rtc_id >= rtc_count)
    {
        return FALSE;
    }
    
    rtc_hndl = &rtc_struct[rtc_id];
    i2c_hndl = rtc_i2c_hndl[rtc_id];       
    
    if(first_read[rtc_id] == TRUE)
    {                    
        LOG_TRM("check halt bit");
        //Read halt bit, clear if high
        if(DrvRtcM41t82z_ReadHaltBit(rtc_hndl))
        {
            //clear halt bit
            LOG_TRM("clear halt bit");
            DrvRtcM41t82z_ClearHaltBit(rtc_hndl);
        }    
        
        first_read[rtc_id] = FALSE;
    }

    // set register address 0
    *address_buf_ptr = 0;
    if(DrvI2cMasterDevice_WriteData(i2c_hndl, rtc_i2c_buffer, ADDRESS_POINTER_SIZE, TRUE) == FALSE)
    {
        return FALSE;
    }
    
    // read time
    if(DrvI2cMasterDevice_ReadData(i2c_hndl, registers_buf_ptr, CLOCK_REGISTER_COUNT, TRUE))
    {
        rtc_time_ptr->second       = Rtc_ConvertHexToDec(registers_buf_ptr[1] & 0x7F);
        rtc_time_ptr->minute       = Rtc_ConvertHexToDec(registers_buf_ptr[2] & 0x7F);
        rtc_time_ptr->hour         = Rtc_ConvertHexToDec(registers_buf_ptr[3] & 0x3F);
        rtc_time_ptr->day_of_week  = (DAY_OF_WEEK)((registers_buf_ptr[4] & 0x07) - 1);
        rtc_time_ptr->day_of_month = Rtc_ConvertHexToDec(registers_buf_ptr[5] & 0x3F);
        rtc_time_ptr->month        = (MONTH)Rtc_ConvertHexToDec(registers_buf_ptr[6] & 0x1F);
        rtc_time_ptr->year         = 2000 + 100*((registers_buf_ptr[3] >> 6) & 0x01) + Rtc_ConvertHexToDec(registers_buf_ptr[7] & 0xFF);
        
        // check for oscillator failure
        // set register address 15
        *address_buf_ptr = 15;
        if(DrvI2cMasterDevice_WriteData(i2c_hndl, address_buf_ptr, ADDRESS_POINTER_SIZE, TRUE) == FALSE)
        {
            return FALSE;
        }
              
        if(DrvI2cMasterDevice_ReadData(i2c_hndl, registers_buf_ptr, 1, TRUE))
        {
            if(*registers_buf_ptr & OF_MASK) // OF bit is 1
            {
                // give some time
                for(i=0; i < 1000; i++)
                {
                    NOP;
                }
                
                // set ST
                *address_buf_ptr = 1; // on-chip address pointer
                registers_buf_ptr[0] = 0x80 + Rtc_ConvertDecToHex(rtc_time_ptr->second); // set ST bit => KICK OSCILLATOR
                
                if(DrvI2cMasterDevice_WriteData(i2c_hndl, rtc_i2c_buffer, 2, TRUE) == FALSE)
                {
                    return FALSE;
                }
                
                // give some time
                for(i=0; i < 1000; i++)
                {
                    NOP;
                }
                
                // clear ST
                *address_buf_ptr = 1; // on-chip address pointer
                registers_buf_ptr[0] = Rtc_ConvertDecToHex(rtc_time_ptr->second);
                
                if(DrvI2cMasterDevice_WriteData(i2c_hndl, rtc_i2c_buffer, 2, TRUE) == FALSE)
                {
                    return FALSE;
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvRtcM41t82z_Init(void)
{
    MODULE_INIT_ONCE();
    
    MEMSET((VPTR)rtc_struct, 0, SIZEOF(rtc_struct));
    rtc_count = 0;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
RTC_HNDL DrvRtcM41t82z_Register(I2C_DEVICE_ID i2c_device_id)
{
    U8          i;
    RTC_HNDL    rtc_hndl = &rtc_struct[rtc_count];    
    first_read[rtc_count] = TRUE;
    
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
//------------------------------------------------------------------------------------------------//
BOOL DrvRtcM41t82z_ClearHaltBit(RTC_HNDL rtc_hndl)
{
    I2C_DEVICE_ID i2c_device_id = rtc_i2c_hndl[rtc_hndl->rtc_id];
    U8  rtc_i2c_buffer[2];    
    U8* address_buf_ptr = &rtc_i2c_buffer[0];
    U8* registers_buf_ptr = &rtc_i2c_buffer[1];

    // set register address 0x0C
    *address_buf_ptr = 0x0C;
    if(DrvI2cMasterDevice_WriteData(rtc_i2c_hndl[i2c_device_id], address_buf_ptr, ADDRESS_POINTER_SIZE, TRUE) == FALSE)
    {
        return FALSE;
    }
    
    // read register
    if(DrvI2cMasterDevice_ReadData(rtc_i2c_hndl[i2c_device_id], registers_buf_ptr, 1, TRUE))
    {
        *registers_buf_ptr &= ~HT_MASK; // clear halt bit
        
        if(DrvI2cMasterDevice_WriteData(rtc_i2c_hndl[i2c_device_id], rtc_i2c_buffer, 2, TRUE) == FALSE)
        {
            return FALSE;
        }
    }    
    
    return TRUE;
}
//================================================================================================//
