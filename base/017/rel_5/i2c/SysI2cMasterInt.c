//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the interrupt driven SPI Master system.
// Number of bytes in one transfer shouldn't exceed 255
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define I2C__SYSI2CMASTERINT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef I2C__SYSI2CMASTERINT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               I2C__SYSI2CMASTERINT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of blocking I2C Master channels
#ifndef SYSI2CMASTERINT_COUNT
	#define SYSI2CMASTERINT_COUNT		I2C_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYS include section
#include "i2c\SysI2cMasterInt.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
	WRITE   = 0,                            //transmit
	READ    = 1, 	                        //receive
}
I2C_TRANSFER_MODE;
//------------------------------------------------------------------------------------------------//
typedef enum
{
    I2C_ACTION_NONE = 0,
    I2C_ACTION_TX   = 1,
    I2C_ACTION_RX   = 2
}
I2C_ACTION;
//------------------------------------------------------------------------------------------------//
typedef struct
{
    BOOL                init_done;
    I2C_ACTION          i2c_action;
    U8*                 data_ptr;
}
I2C_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL SysI2cMasterIntTransferData(I2C_CHANNEL channel, U8 address, U8* data_ptr, U32 data_length, I2C_TRANSFER_MODE transfer_mode);
static void SysI2cMasterIntEndTransfer(I2C_CHANNEL channel, BOOL generate_stop, BOOL success);
__irq static void SysI2cMasterIntI2c1Isr(void);
__irq static void SysI2cMasterIntI2c2Isr(void);
__irq static void SysI2cMasterIntI2cIsr(I2C_CHANNEL channel);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const I2C_TypeDef*           sysi2cmasterint_reg_ptr[I2C_CHANNEL_COUNT] = {(I2C_TypeDef*)I2C1_BASE, (I2C_TypeDef*)I2C2_BASE};
static I2C_CTRL_STRUCT              sysi2cmasterint_ctrl_struct[SYSI2CMASTERINT_COUNT];
static I2C_MSG_COMPLETE             sysi2cmasterint_msg_complete_hook;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL SysI2cMasterIntTransferData(I2C_CHANNEL channel, U8 address, U8* data_ptr, U32 data_length, I2C_TRANSFER_MODE transfer_mode)
{
    I2C_CTRL_STRUCT*            ctrl_struct_ptr = &sysi2cmasterint_ctrl_struct[channel];
    I2C_TypeDef*                reg_ptr = (I2C_TypeDef*)sysi2cmasterint_reg_ptr[channel];
    
    if (ctrl_struct_ptr->i2c_action != I2C_ACTION_NONE)
    {
        return FALSE;   //i2c still busy
    }
    
    ctrl_struct_ptr->data_ptr = data_ptr;
    reg_ptr->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_RD_WRN | I2C_CR2_NBYTES);
    reg_ptr->CR2 |= (address << 1) & 0xFE;
    reg_ptr->CR2 |= (transfer_mode << 10);
    if(data_length > 255)
    {
        reg_ptr->CR2 |= (255 << 16);
    }
    else
    {
        reg_ptr->CR2 |= (data_length << 16);
    }
    
    ctrl_struct_ptr->i2c_action = (I2C_ACTION)(transfer_mode + 1);
    
    // enable interrupts
    reg_ptr->CR1 |= (I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_TCIE | I2C_CR1_NACKIE);
    // start condition and slave address are sent automatically
    reg_ptr->CR2 |= I2C_CR2_START;
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static void SysI2cMasterIntEndTransfer(I2C_CHANNEL channel, BOOL generate_stop, BOOL success)
{
    I2C_CTRL_STRUCT*            ctrl_struct_ptr = &sysi2cmasterint_ctrl_struct[channel];
    I2C_TypeDef*                reg_ptr = (I2C_TypeDef*)sysi2cmasterint_reg_ptr[channel];
    
    if((U8)channel < SYSI2CMASTERINT_COUNT)
    {
        if(generate_stop == TRUE)
        {
            reg_ptr->CR2 |= I2C_CR2_STOP;
        }
        
        ctrl_struct_ptr->i2c_action = I2C_ACTION_NONE;
        
        if(sysi2cmasterint_msg_complete_hook != NULL)
        {
            sysi2cmasterint_msg_complete_hook(channel, success);
        }
    }
}
//------------------------------------------------------------------------------------------------//
__irq static void SysI2cMasterIntI2c1Isr(void)
{
    SysI2cMasterIntI2cIsr(I2C_CHANNEL_1);
}
//------------------------------------------------------------------------------------------------//
__irq static void SysI2cMasterIntI2c2Isr(void)
{
    SysI2cMasterIntI2cIsr(I2C_CHANNEL_2);
}
//------------------------------------------------------------------------------------------------//
__irq static void SysI2cMasterIntI2cIsr(I2C_CHANNEL channel)
{
    I2C_CTRL_STRUCT*    ctrl_struct_ptr = &sysi2cmasterint_ctrl_struct[channel];
    I2C_TypeDef*        reg_ptr = (I2C_TypeDef*)sysi2cmasterint_reg_ptr[channel];
    U32                 status = reg_ptr->ISR;
    
    if((U8)channel < SYSI2CMASTERINT_COUNT)
    {
        switch(ctrl_struct_ptr->i2c_action)
        {
        case I2C_ACTION_TX:
            if(status & I2C_ISR_TXIS)
            {
                if(((reg_ptr->CR2 & I2C_CR2_NBYTES) >> 16) > 0)
                {
                    reg_ptr->TXDR = *ctrl_struct_ptr->data_ptr;
                    ctrl_struct_ptr->data_ptr++;
                }
            }
            if(status & I2C_ISR_TC)
            {
                SysI2cMasterIntEndTransfer(channel, TRUE, TRUE);
            }
            if(status & I2C_ISR_NACKF)
            {
                // stop condition is automatically sent
                reg_ptr->ICR |= I2C_ICR_STOPCF;
                reg_ptr->ICR |= I2C_ICR_NACKCF; // clear flag
                SysI2cMasterIntEndTransfer(channel, FALSE, FALSE);
            }
            break;
        case I2C_ACTION_RX:
            if(status & I2C_ISR_RXNE)
            {
                if(((reg_ptr->CR2 & I2C_CR2_NBYTES) >> 16) > 0)
                {
                    *ctrl_struct_ptr->data_ptr = reg_ptr->RXDR;
                    ctrl_struct_ptr->data_ptr++;
                }
            }
            if(status & I2C_ISR_TC)
            {
                SysI2cMasterIntEndTransfer(channel, TRUE, TRUE);
            }
            if(status & I2C_ISR_NACKF)
            {
                // stop condition is automatically sent
                reg_ptr->ICR |= I2C_ICR_STOPCF;
                reg_ptr->ICR |= I2C_ICR_NACKCF; // clear flag
                SysI2cMasterIntEndTransfer(channel, FALSE, FALSE);
            }
            break;
        default:
            SysI2cMasterIntEndTransfer(channel, TRUE, FALSE);
            break;
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void SysI2cMasterInt_Init(void)
{
    sysi2cmasterint_msg_complete_hook = NULL;
    MEMSET((VPTR)sysi2cmasterint_ctrl_struct, 0, SIZEOF(sysi2cmasterint_ctrl_struct));
}
//------------------------------------------------------------------------------------------------//
BOOL SysI2cMasterInt_RegisterMsgComplete(I2C_MSG_COMPLETE msg_complete_hook)
{
    sysi2cmasterint_msg_complete_hook = msg_complete_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysI2cMasterInt_Channel_Init(I2C_CHANNEL channel)
{
    I2C_TypeDef*        reg_ptr;
    
    if((U8)channel < SYSI2CMASTERINT_COUNT)
    {
        switch(channel)
        {
        case I2C_CHANNEL_1:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,ENABLE);
            SysInt_RegisterIsr(I2C1_IRQn, (EVENT_CALLBACK)SysI2cMasterIntI2c1Isr);
            SysInt_EnableIsr(I2C1_IRQn);
            break;
        case I2C_CHANNEL_2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2,ENABLE);
            SysInt_RegisterIsr(I2C2_IRQn, (EVENT_CALLBACK)SysI2cMasterIntI2c2Isr);
            SysInt_EnableIsr(I2C2_IRQn);
            break;
        default:
            LOG_ERR("illegal channel - %d", PU8(channel));
            return FALSE;
        }
        
        reg_ptr = (I2C_TypeDef*)sysi2cmasterint_reg_ptr[channel];
        
        reg_ptr->CR1  &= ~I2C_CR1_PE; // disable peripheral
        reg_ptr->OAR1 &= ~I2C_OAR1_OA1EN; // disable own address 1
        reg_ptr->OAR2  = 0x0000;
        reg_ptr->CR1  |= I2C_CR1_PE; // enable peripheral
        
        sysi2cmasterint_ctrl_struct[channel].init_done = TRUE;
        return TRUE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysI2cMasterInt_Channel_Config(I2C_CHANNEL channel, I2C_CONFIG_STRUCT* config_struct_ptr)
{
    I2C_CTRL_STRUCT*            ctrl_struct_ptr = &sysi2cmasterint_ctrl_struct[channel];
    I2C_TypeDef*                reg_ptr = (I2C_TypeDef*)sysi2cmasterint_reg_ptr[channel];
    U8                          prescaler;
    U16                         temp1 = (U16)(SysGetAPBClk() / config_struct_ptr->speed);
    U16                         temp2;
    
    if((U8)channel < SYSI2CMASTERINT_COUNT)
    {
        ctrl_struct_ptr = &sysi2cmasterint_ctrl_struct[channel];
        if(ctrl_struct_ptr->init_done == TRUE)
        {
            reg_ptr->CR1 &= ~I2C_CR1_PE; // disable peripheral
            
            reg_ptr->TIMINGR = 0;
            for(prescaler = 0; prescaler < 16; prescaler++)
            {
                // reference manual page 529 t_scl
                temp2 = temp1;
                temp2 /= (1 + prescaler);
                temp2 = (temp2 / 2) - 1;
                if(temp2 > 0xFF)
                {
                    prescaler++;
                }
                else
                {
                    // compensate t_sync1 and t_sync2
                    temp2 -= 6;
                    reg_ptr->TIMINGR |= ((prescaler << 28) & I2C_TIMINGR_PRESC);
                    reg_ptr->TIMINGR |= (temp2 & I2C_TIMINGR_SCLL);
                    reg_ptr->TIMINGR |= ((temp2 << 8) & I2C_TIMINGR_SCLH);
                    reg_ptr->TIMINGR |= (3 << 16);
                    reg_ptr->TIMINGR |= (3 << 20);
                    break;
                }
            }
            
            reg_ptr->CR1 |= I2C_CR1_PE; // enable peripheral
            return TRUE;
        }
        LOG_WRN("channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("illegal channel - %d", PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysI2cMasterInt_Channel_WriteData(I2C_CHANNEL channel, U8 address, U8* data_ptr, U16 count)
{
    I2C_CTRL_STRUCT*            ctrl_struct_ptr = &sysi2cmasterint_ctrl_struct[channel];
    
    if((U8)channel >= SYSI2CMASTERINT_COUNT)
    {
        return FALSE;
    }
    if(ctrl_struct_ptr->init_done == FALSE)
    {
        return FALSE;
    }
    return SysI2cMasterIntTransferData(channel, address, data_ptr, count, WRITE);
}
//------------------------------------------------------------------------------------------------//
BOOL SysI2cMasterInt_Channel_ReadData(I2C_CHANNEL channel, U8 address, U8* data_ptr, U16 count)
{
    I2C_CTRL_STRUCT*            ctrl_struct_ptr = &sysi2cmasterint_ctrl_struct[channel];
    
    if((U8)channel >= SYSI2CMASTERINT_COUNT)
    {
        return FALSE;
    }
    if(ctrl_struct_ptr->init_done == FALSE)
    {
        return FALSE;
    }
    return SysI2cMasterIntTransferData(channel, address, data_ptr, count, READ);
}
//================================================================================================//