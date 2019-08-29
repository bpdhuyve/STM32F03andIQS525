//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor dependent prototypes and definitions for the interrupt based SPI slave system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SYSSPISLAVEINT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SYSSPISLAVEINT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               SYSSPISLAVEINT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking SPI Master channels
#ifndef SYSSPISLAVEINT_COUNT
	#define SYSSPISLAVEINT_COUNT		SPI_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section

//STD lib include section

//COM lib include section

//APP include section
#include "SysSpiSlaveInt.h"
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
    union
    {
        SPI_RX_NEW_U8_DATA_HOOK         u8;
        SPI_RX_NEW_U16_DATA_HOOK        u16;
    }
    rx_new_data_hook;
    
    union
    {
        SPI_TX_GET_NEXT_U8_DATA_HOOK    u8;
        SPI_TX_GET_NEXT_U16_DATA_HOOK   u16;
    }
    tx_get_new_data_hook;
    
    U16                                 crc_err_cnt;
    U16                                 overrun_err_cnt;
    U16                                 frame_format_err_cnt;
    U16                                 underrun_err_cnt;
            
    BOOL                                init_done;
    U8                                  bitcount;
}
SPI_CTRL_STRUCT;

typedef SPI_TypeDef*                    SPI_REG_HNDL;

typedef union
{
    U8  u8[4];
    U16 u16[2];    
}
TX_DATA_ARRAY;

typedef union
{
    U8  u8[4];
    U16 u16[2];
}
RX_DATA_ARRAY;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static __irq void               SysSpiSlaveIntSpi1Isr(void);
static __irq void               SysSpiSlaveIntSpi2Isr(void);
static void                     SysSpiSlaveIntSpiIsr(SPI_CHANNEL channel);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const SPI_REG_HNDL       sysspi_registers[SPI_CHANNEL_COUNT] = {(SPI_REG_HNDL)SPI1_BASE, (SPI_REG_HNDL)SPI2_BASE};
static SPI_CTRL_STRUCT          sysspislaveint_ctrl_struct[SYSSPISLAVEINT_COUNT];
static TX_DATA_ARRAY            tx_data_array;
static RX_DATA_ARRAY            rx_data_array;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static __irq void SysSpiSlaveIntSpi1Isr(void)
{
    SysSpiSlaveIntSpiIsr(SPI_CHANNEL_1);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysSpiSlaveIntSpi2Isr(void)
{
    SysSpiSlaveIntSpiIsr(SPI_CHANNEL_2);
}
//------------------------------------------------------------------------------------------------//
static void SysSpiSlaveIntSpiIsr(SPI_CHANNEL channel)
{
    SPI_CTRL_STRUCT*    ctrl_struct_ptr = &sysspislaveint_ctrl_struct[channel];
    SPI_REG_HNDL        reg_ptr = (SPI_REG_HNDL)sysspi_registers[channel];
    U32                 status = reg_ptr->SR;
    
    if(status & SPI_SR_RXNE) // Rx FIFO not empty
    {
        U8 fifo_filled_count    = 0;
        U8 fifo_reception_level = 0;
        
        fifo_reception_level = (status & SPI_SR_FRLVL_1) ? (fifo_reception_level | 0x02) : fifo_reception_level;
        fifo_reception_level = (status & SPI_SR_FRLVL_0) ? (fifo_reception_level | 0x01) : fifo_reception_level;
        
        // put RxFIFO data in rx_data_array, untill FIFO is empty
        switch(ctrl_struct_ptr->bitcount)
        {
        case 8 :
            fifo_filled_count = fifo_reception_level;
            if(ctrl_struct_ptr->rx_new_data_hook.u8 != NULL)
            {
                for(U8 i = 0; i < fifo_filled_count; i++)
                {
                    rx_data_array.u8[i] = *(__IO U8 *)&reg_ptr->DR;
                }
            }
            ctrl_struct_ptr->rx_new_data_hook.u8(&rx_data_array.u8[0], fifo_filled_count);
            break;
            
        case 16 :
            if(fifo_reception_level <= 1)
                fifo_filled_count = 0;
            else
                fifo_filled_count = fifo_reception_level - 1;
            if(ctrl_struct_ptr->rx_new_data_hook.u16 != NULL)
            {
                for(U8 i = 0; i < fifo_filled_count; i++)
                {
                    rx_data_array.u16[i] = *(__IO U16 *)&reg_ptr->DR;
                }
            }
            ctrl_struct_ptr->rx_new_data_hook.u16(&rx_data_array.u16[0], fifo_filled_count);
            break;
            
        default:
            LOG_ERR("unsupported bitcount");
        }
    }
    if(status & SPI_SR_TXE)  // Tx FIFO empty
    {
        U8 fifo_free_space         = 0;        
        U8 fifo_transmission_level = 0;
        
        fifo_transmission_level = (status & SPI_SR_FTLVL_1) ? (fifo_transmission_level | 0x02) : fifo_transmission_level;
        fifo_transmission_level = (status & SPI_SR_FTLVL_0) ? (fifo_transmission_level | 0x01) : fifo_transmission_level;
        
        // put RxFIFO data in rx_data_array, untill FIFO is empty
        switch(ctrl_struct_ptr->bitcount)
        {
        case 8 :
            fifo_free_space = 3 - fifo_transmission_level;            
            if(ctrl_struct_ptr->tx_get_new_data_hook.u8 != NULL)
            {
                U8 number_of_data_to_send = ctrl_struct_ptr->tx_get_new_data_hook.u8(&tx_data_array.u8[0], fifo_free_space);
                if(number_of_data_to_send > 0)
                {
                    for(U8 i = 0; (i < number_of_data_to_send) && (i < fifo_free_space); i++)
                    {
                        *(__IO U8 *)&reg_ptr->DR = tx_data_array.u8[i];
                    }
                }
                else
                {
                    *(__IO U8 *)&reg_ptr->DR = 0;   // if no data received from tx_get_new_data_hook, send 0
                }
            }
            break;
            
        case 16 :
            if(fifo_transmission_level <= 1)
                fifo_free_space = 0;
            else                
                fifo_free_space = 3 - fifo_transmission_level;            
            if(ctrl_struct_ptr->tx_get_new_data_hook.u16 != NULL)
            {
                U8 number_of_data_to_send = ctrl_struct_ptr->tx_get_new_data_hook.u16(&tx_data_array.u16[0], fifo_free_space);
                if(number_of_data_to_send > 0)
                {
                    for(U8 i = 0; (i < number_of_data_to_send) && (i < fifo_free_space); i++)
                    {
                        *(__IO U16 *)&reg_ptr->DR = tx_data_array.u16[i];
                    }
                }
                else
                {
                    *(__IO U16 *)&reg_ptr->DR = 0;   // if no data received from tx_get_new_data_hook, send 0
                }
            }
            break;
            
        default:
            LOG_ERR("unsupported bitcount");
        }
    }
    if(status & (SPI_SR_CRCERR | SPI_SR_OVR | SPI_SR_FRE | SPI_SR_UDR)) // ERROR
    {
        if(status & SPI_SR_CRCERR) /*** CRC ERROR ***/
        {
            LOG_DEV("SPI ERR CRC");
            ctrl_struct_ptr->crc_err_cnt++;
            // flag is cleared by software
            reg_ptr->SR &= ~SPI_SR_CRCERR;
        }
        if(status & SPI_SR_MODF) /*** MODE FAULT ERROR ***/
        {
            LOG_DEV("SPI ERR MODF");
            // SPI is not configured correctly! => disable SPI
            reg_ptr->CR1 &= ~SPI_CR1_SPE;
        }
        if(status & SPI_SR_FRE) /*** FRAME FORMAT ERROR ***/
        {
            LOG_DEV("SPI ERR FRE");
            ctrl_struct_ptr->frame_format_err_cnt++;
        }
        if(status & SPI_SR_UDR) /*** UNDERRUN ERROR ***/
        {
            LOG_DEV("SPI ERR UDR");
            ctrl_struct_ptr->underrun_err_cnt++;
        }
        if(status & SPI_SR_OVR) /*** OVERRUN ERROR ***/
        {            
            LOG_DEV("SPI ERR OVR");
            ctrl_struct_ptr->overrun_err_cnt++;
            // the status register needs to be read after the data register
            // has been read to clear the OVR flag
            U32 dummy = reg_ptr->SR;
        }
    }
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
void SysSpiSlaveInt_Init(void)
{
    MEMSET((VPTR)sysspislaveint_ctrl_struct, 0, SIZEOF(sysspislaveint_ctrl_struct));
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiSlaveInt_Channel_Init(SPI_CHANNEL channel)
{
    SPI_CTRL_STRUCT*    ctrl_struct_ptr = &sysspislaveint_ctrl_struct[channel];
    SPI_REG_HNDL        reg_ptr = (SPI_REG_HNDL)sysspi_registers[channel];
    
    if(channel < SYSSPISLAVEINT_COUNT)
    {
        reg_ptr->CR1 = 0x0000; // reset register
        reg_ptr->CR2 = 0x0000; // reset register
        
        switch(channel)
        {
        case SPI_CHANNEL_1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
            SysInt_RegisterIsr(SPI1_IRQn, SysSpiSlaveIntSpi1Isr);
            SysInt_EnableIsr(SPI1_IRQn);
            break;
            
        case SPI_CHANNEL_2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
            SysInt_RegisterIsr(SPI2_IRQn, SysSpiSlaveIntSpi2Isr);
            SysInt_EnableIsr(SPI2_IRQn);
            break;
            
        default:
            LOG_ERR("illegal channel - %d", PU8(channel));
            return FALSE;
        }        
        ctrl_struct_ptr->init_done = TRUE;
        return TRUE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiSlaveInt_Channel_Config(SPI_CHANNEL channel, SPI_CONFIG_STRUCT* config_struct_ptr)
{
    SPI_CTRL_STRUCT*    ctrl_struct_ptr = &sysspislaveint_ctrl_struct[channel];
    SPI_REG_HNDL        reg_ptr = (SPI_REG_HNDL)sysspi_registers[channel];
    
    if(channel < SYSSPISLAVEINT_COUNT)
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
            
            reg_ptr->CR2 |= SPI_CR2_RXNEIE; // Rx buffer Not Empty Interrupt Enable
//            reg_ptr->CR2 |= SPI_CR2_TXEIE;  // Tx buffer Empty Interrupt Enable
            reg_ptr->CR2 |= SPI_CR2_ERRIE;  // error interrupt enable
            reg_ptr->CR1 |= SPI_CR1_SPE;    // enable spi channel
            return TRUE;
        }
        LOG_ERR("channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiSlaveInt_Channel_RegisterRxU8DataHook(SPI_CHANNEL channel, SPI_RX_NEW_U8_DATA_HOOK rx_new_data_hook)
{
    if(channel < SYSSPISLAVEINT_COUNT)
    {
        sysspislaveint_ctrl_struct[channel].rx_new_data_hook.u8 = rx_new_data_hook;
        return TRUE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiSlaveInt_Channel_RegisterRxU16DataHook(SPI_CHANNEL channel, SPI_RX_NEW_U16_DATA_HOOK rx_new_data_hook)
{
    if(channel < SYSSPISLAVEINT_COUNT)
    {
        sysspislaveint_ctrl_struct[channel].rx_new_data_hook.u16 = rx_new_data_hook;
        return TRUE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiSlaveInt_Channel_RegisterTxU8DataHook(SPI_CHANNEL channel, SPI_TX_GET_NEXT_U8_DATA_HOOK tx_get_next_data_hook)
{
    if(channel < SYSSPISLAVEINT_COUNT)
    {
        sysspislaveint_ctrl_struct[channel].tx_get_new_data_hook.u8 = tx_get_next_data_hook;
        return TRUE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSpiSlaveInt_Channel_RegisterTxU16DataHook(SPI_CHANNEL channel, SPI_TX_GET_NEXT_U16_DATA_HOOK tx_get_next_data_hook)
{
    if(channel < SYSSPISLAVEINT_COUNT)
    {
        sysspislaveint_ctrl_struct[channel].tx_get_new_data_hook.u16 = tx_get_next_data_hook;
        return TRUE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
U16 SysSpiSlaveInt_GetOverrunErrorCount(SPI_CHANNEL channel)
{
    if(channel < SYSSPISLAVEINT_COUNT)
    {
        return sysspislaveint_ctrl_struct[channel].overrun_err_cnt;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 SysSpiSlaveInt_GetFrameFormatErrorCount(SPI_CHANNEL channel)
{
    if(channel < SYSSPISLAVEINT_COUNT)
    {
        return sysspislaveint_ctrl_struct[channel].frame_format_err_cnt;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 SysSpiSlaveInt_GetCrcErrorCount(SPI_CHANNEL channel)
{
    if(channel < SYSSPISLAVEINT_COUNT)
    {
        return sysspislaveint_ctrl_struct[channel].crc_err_cnt;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 SysSpiSlaveInt_GetUnderrunErrorCount(SPI_CHANNEL channel)
{
    if(channel < SYSSPISLAVEINT_COUNT)
    {
        return sysspislaveint_ctrl_struct[channel].underrun_err_cnt;
    }
    return 0;
}
//================================================================================================//
