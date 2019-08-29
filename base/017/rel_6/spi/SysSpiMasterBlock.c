//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the blocking SPI Master system.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SPI__SYSSPIMASTERBLOCK_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SPI__SYSSPIMASTERBLOCK_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SPI__SYSSPIMASTERBLOCK_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking SPI Master channels
#ifndef SYSSPIMASTERBLOCK_COUNT
	#define SYSSPIMASTERBLOCK_COUNT		SPI_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYS include section
#include "spi\SysSpiMasterBlock.h"
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
    BOOL            init_done;
    U8              bitcount;
}
SPI_CTRL_STRUCT;

typedef SPI_TypeDef*            SPI_REG_HNDL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static U16 ExchangeData(SPI_REG_HNDL spi_reg_hndl, U16 write_byte, U8 ctrl_struct_bitcount);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const SPI_REG_HNDL           sysspi_registers[SPI_CHANNEL_COUNT] = {(SPI_REG_HNDL)SPI1_BASE, (SPI_REG_HNDL)SPI2_BASE};
static SPI_CTRL_STRUCT              sysspimasterblock_ctrl_struct[SYSSPIMASTERBLOCK_COUNT];
static SPI_MSG_COMPLETE             sysspimasterblock_msg_complete_hook;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static U16 ExchangeData(SPI_REG_HNDL spi_reg_hndl, U16 write_byte, U8 ctrl_struct_bitcount)
{
    U32         DRL;
    
    switch(ctrl_struct_bitcount)
    {
    case 8:
        DRL = 0x00;
        DRL = (U32)spi_reg_hndl + 0x0C;
        *(__IO U8 *) DRL = (U8)write_byte;
        
        while(!(spi_reg_hndl->SR & 0x02))   //wait until data is send
        {}
        while(!(spi_reg_hndl->SR & 0x01))   //wait until data is received
        {}
        
        return (U16)(*(__IO U8 *) DRL);
    case 16:
        spi_reg_hndl->DR = write_byte;      //write outgoing byte
        
        while(!(spi_reg_hndl->SR & 0x02))   //wait until data is send
        {}
        while(!(spi_reg_hndl->SR & 0x01))   //wait until data is received
        {}
        
        return (spi_reg_hndl->DR);          //return received byte
    default:
        LOG_ERR("unsupported bitcount");
        return 0;
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void SysSpiMasterBlock_Init(void)
{
    sysspimasterblock_msg_complete_hook = NULL;
    MEMSET((VPTR)sysspimasterblock_ctrl_struct, 0, SIZEOF(sysspimasterblock_ctrl_struct));
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiMasterBlock_RegisterMsgComplete(SPI_MSG_COMPLETE msg_complete_hook)
{
    sysspimasterblock_msg_complete_hook = msg_complete_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiMasterBlock_Channel_Init(SPI_CHANNEL channel)
{
    SPI_CTRL_STRUCT*            ctrl_struct_ptr = &sysspimasterblock_ctrl_struct[channel];
    SPI_REG_HNDL                reg_ptr = (SPI_REG_HNDL)sysspi_registers[channel];
    
    if((U8)channel < SYSSPIMASTERBLOCK_COUNT)
    {
        switch(channel)
        {
        case SPI_CHANNEL_1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
            break;
        case SPI_CHANNEL_2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
            break;
        default:
            LOG_ERR("illegal channel - %d", PU8(channel));
            return FALSE;
        }
        
        reg_ptr->CR1 =  0x0000; //disable the spi channel and reset register
        reg_ptr->CR2 =  0x0000; //reset register
        reg_ptr->CR1 |= 0x0004; //master mode, msb transmitted first, crc disabled, unidirectional data mode
        reg_ptr->CR2 |= 0x0004; //SSOE on
        
        ctrl_struct_ptr->init_done = TRUE;
        return TRUE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiMasterBlock_Channel_Config(SPI_CHANNEL channel, SPI_CONFIG_STRUCT* config_struct_ptr)
{
    SPI_CTRL_STRUCT*            ctrl_struct_ptr = &sysspimasterblock_ctrl_struct[channel];
    SPI_REG_HNDL                reg_ptr = (SPI_REG_HNDL)sysspi_registers[channel];
    U8                  speed_br = 0;
    U32                 speed;
    U32                 clock_speed;
    
    if((U8)channel < SYSSPIMASTERBLOCK_COUNT)
    {
        if(ctrl_struct_ptr->init_done == TRUE)
        {
            // Disable the SPI channel
            reg_ptr->CR1 &= ~0x0040;
            
            // set bitcount
            switch(config_struct_ptr->bitcount)
            {
            case 8:
                reg_ptr->CR2 |= 0x1700;
                break;
            case 16:
                reg_ptr->CR2 |= 0x0F00;
                break;
            default:
                LOG_ERR("unsupported bitcount - %d", PU8(config_struct_ptr->bitcount));
                return FALSE;
            }
            ctrl_struct_ptr->bitcount = config_struct_ptr->bitcount;
            
            // set speed
            reg_ptr->CR1 &= ~0x0038; //clear baudrate bits first
            clock_speed = SysGetAPBClk() >> 1;
            speed = config_struct_ptr->speed;
            while(speed < clock_speed)
            {
                speed <<= 1;
                speed_br++;
            }
            if(speed_br > 7)
            {
                LOG_ERR("illegal speed - %d", PU32(config_struct_ptr->speed));
                return FALSE;
            }
            reg_ptr->CR1 |= (speed_br << 3);
            
            // set mode
            switch(config_struct_ptr->mode)
            {
            case MODE_0:
                reg_ptr->CR1 &= ~0x0002; //CPOL = 0
                reg_ptr->CR1 &= ~0x0001; //CPHA = 0
                break;
            case MODE_1:
                reg_ptr->CR1 &= ~0x0002; //CPOL = 0
                reg_ptr->CR1 |= 0x0001;  //CPHA = 1
                break;
            case MODE_2:
                reg_ptr->CR1 |= 0x0002;  //CPOL = 1
                reg_ptr->CR1 &= ~0x0001; //CPHA = 0
                break;
            case MODE_3:
                reg_ptr->CR1 |= 0x0002;  //CPOL = 1
                reg_ptr->CR1 |= 0x0001;  //CPHA = 1
                break;
            default:
                LOG_ERR("illegal mode - %d", PU8(config_struct_ptr->mode));
                return FALSE;
            }
            
            // set lsb/msb first
            if(config_struct_ptr->lsb_first)
            {
                reg_ptr->CR1 |= 0x0080;  //LSB first
            }
            else
            {
                reg_ptr->CR1 &= ~0x0080; //MSB first
            }
            
            // enable spi channel
            reg_ptr->CR1 |=  0x0040;
            return TRUE;
        }
        LOG_ERR("channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiMasterBlock_Channel_ExchangeData(SPI_CHANNEL channel, U8* write_data_ptr, U8* read_data_ptr, U16 count)
{
    SPI_CTRL_STRUCT*            ctrl_struct_ptr = &sysspimasterblock_ctrl_struct[channel];
    SPI_REG_HNDL                reg_ptr = (SPI_REG_HNDL)sysspi_registers[channel];
    #ifdef  SPI_INIT_HIGH
    U16                 write_data = 0xFF;
    #else
    U16                 write_data = 0x00;
    #endif
    U16                 read_data = 0;
    
    LOG_DBG("OUT %02h", PU8A(write_data_ptr, count));
    
    if((U8)channel < SYSSPIMASTERBLOCK_COUNT)
    {
        if(ctrl_struct_ptr->init_done == TRUE)
        {
            if(ctrl_struct_ptr->bitcount == 8)
            {
                while(count > 0)
                {
                    if(write_data_ptr != NULL)
                    {
                        write_data = (U16)*write_data_ptr;
                        write_data_ptr++;
                    }
                    read_data = ExchangeData(reg_ptr, write_data, 8);
                    if(read_data_ptr != NULL)
                    {
                        *read_data_ptr = (U8)read_data;
                        read_data_ptr++;
                    }
                    count--;
                }
            }
            else if(ctrl_struct_ptr->bitcount == 16)
            {
                count &= 0xFFFE;
                while(count > 0)
                {
                    if(write_data_ptr != NULL)
                    {
                        write_data = *((U16*)write_data_ptr);
                        write_data_ptr += 2;
                    }
                    read_data = ExchangeData(reg_ptr, write_data, 16);
                    if(read_data_ptr != NULL)
                    {
                        *((U16*)read_data_ptr) = read_data;
                        read_data_ptr += 2;
                    }
                    count -= 2;
                }
            }
            
            if(sysspimasterblock_msg_complete_hook != NULL)
            {
                sysspimasterblock_msg_complete_hook(channel);
            }
            return TRUE;
        }
        LOG_ERR("channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//================================================================================================//
