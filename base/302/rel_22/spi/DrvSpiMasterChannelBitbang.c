//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the blocking SPI Master bitbang driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SPI__DRVSPIMASTERCHANNELBITBANG_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SPI__DRVSPIMASTERCHANNELBITBANG_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               SPI__DRVSPIMASTERCHANNELBITBANG_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking SPI Master channels
#ifndef DRVSPIMASTERCHANNELBITBANG_COUNT
	#define DRVSPIMASTERCHANNELBITBANG_COUNT			    SPI_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "DrvSpiMasterChannelBitbang.h"
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
    DRVGPIO_PIN_HNDL            clock_pin_hndl;
    DRVGPIO_PIN_HNDL            mosi_pin_hndl;
    DRVGPIO_PIN_HNDL            miso_pin_hndl;
    SPI_CONFIG_STRUCT           config_struct;
}
SPI_CHANNEL_BITBANG_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U16  ExchangeData(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, U16 write_data);
static BOOL ExchangeBit(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit);
static BOOL ExchangeBitInMode0(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit);
static BOOL ExchangeBitInMode1(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit);
static BOOL ExchangeBitInMode2(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit);
static BOOL ExchangeBitInMode3(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit);
static BOOL DrvSpiMasterChannelBitbang_ExchangeData(SPI_CHANNEL_ID channel_id, U8* write_data_ptr, U8* read_data_ptr, U16 count);
static BOOL DrvSpiMasterChannelBitbang_Config(SPI_CHANNEL_ID channel_id, SPI_CONFIG_STRUCT* config_struct_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static const SPI_CHANNEL_HOOK_LIST          spi_channel_hook_list = {DrvSpiMasterChannelBitbang_ExchangeData, DrvSpiMasterChannelBitbang_Config};
static SPI_CHANNEL_BITBANG_STRUCT           spi_channel_bitbang_struct[DRVSPIMASTERCHANNELBITBANG_COUNT];
static SPI_CHANNEL_STRUCT                   spi_channel_struct[DRVSPIMASTERCHANNELBITBANG_COUNT];
static U8                                   spi_channel_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static U16 ExchangeData(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, U16 write_data)
{
    U8  count = bitbang_struct_ptr->config_struct.bitcount;
    U16 read_data = 0;
    
    if(bitbang_struct_ptr->config_struct.lsb_first)
    {
        while(count > 0)
        {
            if(ExchangeBit(bitbang_struct_ptr, (BOOL)((write_data >> (bitbang_struct_ptr->config_struct.bitcount - count)) & 0x1)))
            {
                read_data |= (1 << (bitbang_struct_ptr->config_struct.bitcount - count));
            }
            count--;
        }
    }
    else
    {
        while(count > 0)
        {
            if(ExchangeBit(bitbang_struct_ptr, (BOOL)((write_data >> (count - 1)) & 0x1)))
            {
                read_data |= (1 << (count - 1));
            }
            count--;
        }
    }
    return read_data;
}
//------------------------------------------------------------------------------------------------//
static BOOL ExchangeBit(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit)
{
    switch(bitbang_struct_ptr->config_struct.mode)
    {
    case MODE_0:
        return ExchangeBitInMode0(bitbang_struct_ptr, write_bit);
        
    case MODE_1:
        return ExchangeBitInMode1(bitbang_struct_ptr, write_bit);
        
    case MODE_2:
        return ExchangeBitInMode2(bitbang_struct_ptr, write_bit);
        
    case MODE_3:
        return ExchangeBitInMode3(bitbang_struct_ptr, write_bit);
        
    default:
        LOG_WRN("Invalid mode");
        return FALSE;
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL ExchangeBitInMode0(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit)
{
    BOOL bit;
    
    DrvGpio_SetPin(bitbang_struct_ptr->mosi_pin_hndl, write_bit);
    DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, TRUE);
    bit = DrvGpio_GetPin(bitbang_struct_ptr->miso_pin_hndl);
    DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, FALSE);
    return bit;
}
//------------------------------------------------------------------------------------------------//
static BOOL ExchangeBitInMode1(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit)
{
    DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, TRUE);
    DrvGpio_SetPin(bitbang_struct_ptr->mosi_pin_hndl, write_bit);
    DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, FALSE);
    return DrvGpio_GetPin(bitbang_struct_ptr->miso_pin_hndl);
}
//------------------------------------------------------------------------------------------------//
static BOOL ExchangeBitInMode2(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit)
{
    BOOL bit;
    
    DrvGpio_SetPin(bitbang_struct_ptr->mosi_pin_hndl, write_bit);
    DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, FALSE);
    bit = DrvGpio_GetPin(bitbang_struct_ptr->miso_pin_hndl);
    DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, TRUE);
    return bit;
}
//------------------------------------------------------------------------------------------------//
static BOOL ExchangeBitInMode3(SPI_CHANNEL_BITBANG_STRUCT* bitbang_struct_ptr, BOOL write_bit)
{
    DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, FALSE);
    DrvGpio_SetPin(bitbang_struct_ptr->mosi_pin_hndl, write_bit);
    DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, TRUE);
    return DrvGpio_GetPin(bitbang_struct_ptr->miso_pin_hndl);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSpiMasterChannelBitbang_ExchangeData(SPI_CHANNEL_ID channel_id, U8* write_data_ptr, U8* read_data_ptr, U16 count)
{
    SPI_CHANNEL_BITBANG_STRUCT*     bitbang_struct_ptr = spi_channel_bitbang_struct;
    SPI_CHANNEL_HNDL                channel_hndl;
    U16                             write_data;
    U16                             read_data;
    
    for(channel_hndl = spi_channel_struct; channel_hndl < &spi_channel_struct[spi_channel_count]; channel_hndl++, bitbang_struct_ptr++)
    {
        if(channel_hndl->channel_id == channel_id)
        {
            break;
        }
    }
    
    if(channel_hndl >= &spi_channel_struct[spi_channel_count])
    {
        return FALSE;
    }
    
    if(bitbang_struct_ptr->config_struct.bitcount <= 8)
    {
        while(count > 0)
        {
            if(write_data_ptr != NULL)
            {
                write_data = (U16)*write_data_ptr;
                write_data_ptr++;
            }
            read_data = ExchangeData(bitbang_struct_ptr, write_data);
            if(read_data_ptr != NULL)
            {
                *read_data_ptr = (U8)read_data;
                read_data_ptr++;
            }
            count--;
        }
    }
    else if(bitbang_struct_ptr->config_struct.bitcount <= 16)
    {
        count &= 0xFFFE;
        while(count > 0)
        {
            if(write_data_ptr != NULL)
            {
                write_data = *((U16*)write_data_ptr);
                write_data_ptr += 2;
            }
            read_data = ExchangeData(bitbang_struct_ptr, write_data);
            if(read_data_ptr != NULL)
            {
                *((U16*)read_data_ptr) = read_data;
                read_data_ptr += 2;
            }
            count -= 2;
        }
    }
    
    drvspimasterchannel_msg_complete_hook(channel_hndl);
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSpiMasterChannelBitbang_Config(SPI_CHANNEL_ID channel_id, SPI_CONFIG_STRUCT* config_struct_ptr)
{
    SPI_CHANNEL_BITBANG_STRUCT*     bitbang_struct_ptr = spi_channel_bitbang_struct;
    SPI_CHANNEL_HNDL                channel_hndl;
    
    for(channel_hndl = spi_channel_struct; channel_hndl < &spi_channel_struct[spi_channel_count]; channel_hndl++, bitbang_struct_ptr++)
    {
        if(channel_hndl->channel_id == channel_id)
        {
            bitbang_struct_ptr->config_struct = *config_struct_ptr;
            
            // Set initial clock state
            switch(bitbang_struct_ptr->config_struct.mode)
            {
            case MODE_0:
            case MODE_1:
                DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, FALSE);
                break;
                
            case MODE_2:
            case MODE_3:
                DrvGpio_SetPin(bitbang_struct_ptr->clock_pin_hndl, TRUE);
                break;
                
            default:
                LOG_WRN("Invalid mode");
                return FALSE;
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
void DrvSpiMasterChannelBitbang_Init(void)
{
    MODULE_INIT_ONCE();
    
    LOG_DEV("DrvSpiMasterChannelBitbang_Init");
    
    MEMSET((VPTR)spi_channel_struct, 0, SIZEOF(spi_channel_struct));
    spi_channel_count = 0;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
SPI_CHANNEL_HNDL DrvSpiMasterChannelBitbang_Register(DRVGPIO_PIN_HNDL clock_pin,
                                                     DRVGPIO_PIN_HNDL mosi_pin,
                                                     DRVGPIO_PIN_HNDL miso_pin)
{
    MODULE_CHECK();
    
    SPI_CHANNEL_BITBANG_STRUCT*     bitbang_struct_ptr;
    SPI_CHANNEL_HNDL                channel_hndl = spi_channel_struct;
    
    LOG_DEV("DrvSpiMasterChannelBitbang_Register");
    
    for(bitbang_struct_ptr = spi_channel_bitbang_struct; bitbang_struct_ptr < &spi_channel_bitbang_struct[spi_channel_count]; bitbang_struct_ptr++, channel_hndl++)
    {
        if((bitbang_struct_ptr->clock_pin_hndl->pin_id == clock_pin->pin_id) &&
           (bitbang_struct_ptr->mosi_pin_hndl->pin_id  == mosi_pin->pin_id) &&
           (bitbang_struct_ptr->miso_pin_hndl->pin_id  == miso_pin->pin_id))
        {
            return channel_hndl;
        }
    }
    if(spi_channel_count < DRVSPIMASTERCHANNELBITBANG_COUNT)
    {
        bitbang_struct_ptr->clock_pin_hndl = clock_pin;
        bitbang_struct_ptr->mosi_pin_hndl  = mosi_pin; 
        bitbang_struct_ptr->miso_pin_hndl  = miso_pin;
        
        channel_hndl->hook_list_ptr = (SPI_CHANNEL_HOOK_LIST*)&spi_channel_hook_list;
        channel_hndl->channel_id    = spi_channel_count;
        spi_channel_count++;
        return channel_hndl;
    }
    LOG_ERR("Illegal count");
    return NULL;
}
//================================================================================================//