//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the blocking I2C Master Channel driver.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define I2C__DRVI2CMASTERCHANNELBITBANG_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef I2C__DRVI2CMASTERCHANNELBITBANG_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               I2C__DRVI2CMASTERCHANNELBITBANG_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking I2C Master channels
#ifndef DRVI2CMASTERCHANNELBITBANG_COUNT
	#define DRVI2CMASTERCHANNELBITBANG_COUNT			1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "i2c\DrvI2cMasterChannelBitBang.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    DRVGPIO_PIN_HNDL    scl_pin_hndl;
    DRVGPIO_PIN_HNDL    sda_pin_hndl;
    U32                 speed;
    BOOL                started;
}
I2C_BB_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void I2cBitBang_StartCond(I2C_BB_STRUCT* i2c_bb_struct_ptr);
static void I2cBitBang_StopCond(I2C_BB_STRUCT* i2c_bb_struct_ptr);
static void I2cBitBang_WriteBit(I2C_BB_STRUCT* i2c_bb_struct_ptr, BOOL bit);
static BOOL I2cBitBang_ReadBit(I2C_BB_STRUCT* i2c_bb_struct_ptr);
static BOOL I2cBitBang_WriteByte(I2C_BB_STRUCT* i2c_bb_struct_ptr, U8 byte);
static U8 I2cBitBang_ReadByte(I2C_BB_STRUCT* i2c_bb_struct_ptr, BOOL nack);
static BOOL DrvI2cMasterChannelBitBang_WriteData(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count);
static BOOL DrvI2cMasterChannelBitBang_ReadData(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count);
static BOOL DrvI2cMasterChannelBitBang_Config(I2C_CHANNEL_ID channel_id, I2C_CONFIG_STRUCT* config_struct_ptr);
static void DrvI2cMasterChannelBitBang_MsgComplete(I2C_CHANNEL_ID channel_id, BOOL success);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static I2C_CHANNEL_HOOK_LIST                i2c_bb_channel_hook_list;
static I2C_CHANNEL_STRUCT                   i2c_bb_channel_struct[DRVI2CMASTERCHANNELBITBANG_COUNT];
static U8                                   i2c_bb_channel_count;
static I2C_BB_STRUCT                        i2c_bb_struct[DRVI2CMASTERCHANNELBITBANG_COUNT];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void I2cBitBang_StartCond(I2C_BB_STRUCT* i2c_bb_struct_ptr)
{
    LOG_DEV("S");
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->sda_pin_hndl, GPIO_PIN_INPUT);     // SDA HIGH
    // I2cBitBang_Delay();
    if(i2c_bb_struct_ptr->started == TRUE)
    {
        DrvGpio_ReInitPin(i2c_bb_struct_ptr->scl_pin_hndl, GPIO_PIN_INPUT);     // SCL HIGH
        while(DrvGpio_GetPin(i2c_bb_struct_ptr->scl_pin_hndl) == FALSE){}       // clock stretching - should timeout...
        // I2cBitBang_Delay();
    }
    if(DrvGpio_GetPin(i2c_bb_struct_ptr->sda_pin_hndl) == FALSE)
    {
        //lost arbritation
    }
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->sda_pin_hndl, GPIO_PIN_OUTPUT);    // SDA LOW
    // I2cBitBang_Delay();
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->scl_pin_hndl, GPIO_PIN_OUTPUT);    // SCL LOW
    i2c_bb_struct_ptr->started = TRUE;
}
//------------------------------------------------------------------------------------------------//
static void I2cBitBang_StopCond(I2C_BB_STRUCT* i2c_bb_struct_ptr)
{
    LOG_DEV("P");
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->sda_pin_hndl, GPIO_PIN_OUTPUT);    // SDA LOW
    // I2cBitBang_Delay();
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->scl_pin_hndl, GPIO_PIN_INPUT);     // SCL HIGH
    while(DrvGpio_GetPin(i2c_bb_struct_ptr->scl_pin_hndl) == FALSE){}       // clock stretching - should timeout...
    // I2cBitBang_Delay();
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->sda_pin_hndl, GPIO_PIN_INPUT);     // SDA HIGH
    if(DrvGpio_GetPin(i2c_bb_struct_ptr->sda_pin_hndl) == FALSE)
    {
        //lost arbritation
    }
    // I2cBitBang_Delay();
    i2c_bb_struct_ptr->started = FALSE;
}
//------------------------------------------------------------------------------------------------//
static void I2cBitBang_WriteBit(I2C_BB_STRUCT* i2c_bb_struct_ptr, BOOL bit)
{
    if(bit == TRUE)
    {
        DrvGpio_ReInitPin(i2c_bb_struct_ptr->sda_pin_hndl, GPIO_PIN_INPUT);     // SDA HIGH
    }
    else
    {
        DrvGpio_ReInitPin(i2c_bb_struct_ptr->sda_pin_hndl, GPIO_PIN_OUTPUT);    // SDA LOW
    }
    // I2cBitBang_Delay();
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->scl_pin_hndl, GPIO_PIN_INPUT);     // SCL HIGH
    while(DrvGpio_GetPin(i2c_bb_struct_ptr->scl_pin_hndl) == FALSE){}       // clock stretching - should timeout...
    if(bit == TRUE)
    {
        if(DrvGpio_GetPin(i2c_bb_struct_ptr->sda_pin_hndl) == FALSE)
        {
            //lost arbritation
        }
    }
    // I2cBitBang_Delay();
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->scl_pin_hndl, GPIO_PIN_OUTPUT);    // SCL LOW
}
//------------------------------------------------------------------------------------------------//
static BOOL I2cBitBang_ReadBit(I2C_BB_STRUCT* i2c_bb_struct_ptr)
{
    BOOL    bit;
    
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->sda_pin_hndl, GPIO_PIN_INPUT);     // SDA HIGH
    // I2cBitBang_Delay();
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->scl_pin_hndl, GPIO_PIN_INPUT);     // SCL HIGH
    while(DrvGpio_GetPin(i2c_bb_struct_ptr->scl_pin_hndl) == FALSE){}       // clock stretching - should timeout...
    bit = DrvGpio_GetPin(i2c_bb_struct_ptr->sda_pin_hndl);
    // I2cBitBang_Delay();
    DrvGpio_ReInitPin(i2c_bb_struct_ptr->scl_pin_hndl, GPIO_PIN_OUTPUT);    // SCL LOW
    
    return bit;
}
//------------------------------------------------------------------------------------------------//
static BOOL I2cBitBang_WriteByte(I2C_BB_STRUCT* i2c_bb_struct_ptr, U8 byte)
{
    U8      bit;
    BOOL    nack;
    
    for(bit = 0; bit < 8; bit++, byte <<= 1)
    {
        I2cBitBang_WriteBit(i2c_bb_struct_ptr, (BOOL)((byte & 0x80) > 0));
    }
    nack = I2cBitBang_ReadBit(i2c_bb_struct_ptr);
    
    return nack;
}
//------------------------------------------------------------------------------------------------//
static U8 I2cBitBang_ReadByte(I2C_BB_STRUCT* i2c_bb_struct_ptr, BOOL nack)
{
    U8  bit;
    U8  byte = 0x00;
    
    for(bit = 0; bit < 8; bit++)
    {
        byte <<= 1;
        byte |= (U8)I2cBitBang_ReadBit(i2c_bb_struct_ptr);
    }
    I2cBitBang_WriteBit(i2c_bb_struct_ptr, nack);
    
    return byte;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvI2cMasterChannelBitBang_WriteData(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count)
{
    I2C_BB_STRUCT*      i2c_bb_struct_ptr = &i2c_bb_struct[channel_id];
    
    LOG_DEV("W %d %02h", PU8(address), PU8A(data_ptr, count));
    
    if(channel_id < i2c_bb_channel_count)
    {
        I2cBitBang_StartCond(i2c_bb_struct_ptr);
        if(I2cBitBang_WriteByte(i2c_bb_struct_ptr, (U8)((address << 1) & 0xFE)) == TRUE)
        {
            DrvI2cMasterChannelBitBang_MsgComplete(channel_id, FALSE);      // NACK
            return FALSE;
        }
        while(count > 0)
        {
            count--;
            if(I2cBitBang_WriteByte(i2c_bb_struct_ptr, *data_ptr) == TRUE)
            {
                DrvI2cMasterChannelBitBang_MsgComplete(channel_id, FALSE);      // NACK
                return FALSE;
            }
            data_ptr++;
        }
        DrvI2cMasterChannelBitBang_MsgComplete(channel_id, TRUE);
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvI2cMasterChannelBitBang_ReadData(I2C_CHANNEL_ID channel_id, U8 address, U8* data_ptr, U16 count)
{
    I2C_BB_STRUCT*      i2c_bb_struct_ptr = &i2c_bb_struct[channel_id];
    
    LOG_DEV("R %d", PU8(address));
    
    if(channel_id < i2c_bb_channel_count)
    {
        I2cBitBang_StartCond(i2c_bb_struct_ptr);
        if(I2cBitBang_WriteByte(i2c_bb_struct_ptr, (U8)((address << 1) & 0xFE) + 1) == TRUE)
        {
            DrvI2cMasterChannelBitBang_MsgComplete(channel_id, FALSE);      // NACK
            return FALSE;
        }
        while(count > 0)
        {
            count--;
            *data_ptr = I2cBitBang_ReadByte(i2c_bb_struct_ptr, (BOOL)(count == 0));
            data_ptr++;
        }
        DrvI2cMasterChannelBitBang_MsgComplete(channel_id, TRUE);
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvI2cMasterChannelBitBang_Config(I2C_CHANNEL_ID channel_id, I2C_CONFIG_STRUCT* config_struct_ptr)
{
    if(channel_id < i2c_bb_channel_count)
    {
        i2c_bb_struct[channel_id].speed = config_struct_ptr->speed;
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvI2cMasterChannelBitBang_MsgComplete(I2C_CHANNEL_ID channel_id, BOOL success)
{
    I2C_CHANNEL_HNDL    channel_hndl = &i2c_bb_channel_struct[channel_id];
    I2C_BB_STRUCT*      i2c_bb_struct_ptr = &i2c_bb_struct[channel_id];
    
    LOG_DEV("DrvI2cMasterChannelBitBang_MsgComplete");
    if(channel_id < i2c_bb_channel_count)
    {
        I2cBitBang_StopCond(i2c_bb_struct_ptr);
        if(drvi2cmasterchannel_msg_complete_hook != NULL)
        {
            drvi2cmasterchannel_msg_complete_hook(channel_hndl, success);
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvI2cMasterChannelBitBang_Init(void)
{
    LOG_DEV("DrvI2cMasterChannelBitBang_Init");
    i2c_bb_channel_hook_list.write_hook = DrvI2cMasterChannelBitBang_WriteData;
    i2c_bb_channel_hook_list.read_hook = DrvI2cMasterChannelBitBang_ReadData;
    i2c_bb_channel_hook_list.config_hook = DrvI2cMasterChannelBitBang_Config;
    
    MEMSET((VPTR)i2c_bb_channel_struct, 0, SIZEOF(i2c_bb_channel_struct));
    MEMSET((VPTR)i2c_bb_struct, 0, SIZEOF(i2c_bb_struct));
    i2c_bb_channel_count = 0;
}
//------------------------------------------------------------------------------------------------//
I2C_CHANNEL_HNDL DrvI2cMasterChannelBitBang_Register(DRVGPIO_PIN_HNDL scl_pin_hndl, DRVGPIO_PIN_HNDL sda_pin_hndl)
{
    I2C_CHANNEL_HNDL    channel_hndl;
    U8                  channel;
    I2C_BB_STRUCT*      i2c_bb_struct_ptr;
    
    LOG_DEV("DrvI2cMasterChannelBitBang_Register");
    for(channel = 0, channel_hndl = i2c_bb_channel_struct; channel < i2c_bb_channel_count; channel++, channel_hndl++)
    {
        if((i2c_bb_struct[channel].scl_pin_hndl == scl_pin_hndl) && (i2c_bb_struct[channel].sda_pin_hndl == sda_pin_hndl))
        {
            return channel_hndl;
        }
    }
    if(i2c_bb_channel_count < DRVI2CMASTERCHANNELBITBANG_COUNT)
    {
        i2c_bb_struct_ptr = &i2c_bb_struct[channel];
        i2c_bb_struct_ptr->scl_pin_hndl = scl_pin_hndl;
        i2c_bb_struct_ptr->sda_pin_hndl = sda_pin_hndl;
        i2c_bb_struct_ptr->speed = 10000;
        i2c_bb_struct_ptr->started = FALSE;
        
        channel_hndl->channel_id = channel;
        channel_hndl->hook_list_ptr = &i2c_bb_channel_hook_list;
        i2c_bb_channel_count++;
        
        DrvGpio_ReInitPin(scl_pin_hndl, GPIO_PIN_INPUT);
        DrvGpio_ReInitPin(sda_pin_hndl, GPIO_PIN_INPUT);
        return channel_hndl;
    }
    LOG_ERR("Illegal I2C channel - %d", PU8(channel));
    return NULL;
}
//================================================================================================//
