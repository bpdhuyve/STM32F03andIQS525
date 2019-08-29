//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the interrupt driven SCI system.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SCI__SYSSCIINT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SCI__SYSSCIINT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SCI__SYSSCIINT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of interrupt driven SCI channels
#ifndef SYSSCIINT_COUNT
	#define SYSSCIINT_COUNT			    SCI_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYS include section
#include "sci\SysSciInt.h"
#include "core\stm32f0xx_usart.h"

#include "gpio\DrvGPIOSys.h"
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
    SCI_RX_NEW_BYTE_HOOK        rx_new_byte_hook;
    SCI_TX_GET_NEXT_BYTE_HOOK   tx_get_new_byte_hook;
    
#ifdef INCLUDE_MPCM
    BOOL                        mpcm_mode;
    BOOL                        mpcm_allow_rx;
    BOOL                        mpcm_address_indication;
#endif
    
#ifdef INCLUDE_LIN
    EVENT_CALLBACK              lin_break_detected_hook;
#endif

    U16                         overrun_err_cnt;
    U16                         parity_err_cnt;
    U16                         framing_err_cnt;
    U16                         noise_err_cnt;
    
    BOOL                            init_done;
    BOOL                        tx_busy;
}
SCI_CTRL_STRUCT;

typedef USART_TypeDef*          SCI_REG_HNDL_PTR;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL SysSciIntInitPheripheral(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr);
static __irq void SysSciIntUsart1Isr(void);
static __irq void SysSciIntUsart2Isr(void);
static __irq void SysSciIntUsart3Till8Isr(void);
static void SysSciIntUsartIsr(SCI_CHANNEL channel, SCI_REG_HNDL_PTR peripheral_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const SCI_REG_HNDL_PTR       syssci_registers[SCI_CHANNEL_COUNT] = {USART1, USART2, USART3, USART4, USART5, USART6};
static SCI_CTRL_STRUCT              syssci_ctrl_struct[SYSSCIINT_COUNT];
static SCI_MSG_COMPLETE             syssci_msg_complete_hook;
#if (CORELOG_LEVEL & LOG_LEVEL_ERROR) != 0
static const STRING                 syssci_illegal_channel_string = "illegal channel - ";
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL SysSciIntInitPheripheral(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    BOOL result = FALSE ;

    USART_InitTypeDef usart_init_structure ;
    SCI_REG_HNDL_PTR      usart_ptr = NULL ;
    
    if (config_struct_ptr)
    {
        result = TRUE ;
        
        switch (config_struct_ptr->data_length)
        {
        
        case SCI_DATA_LENGTH_7_BITS:
            usart_init_structure.USART_WordLength = USART_WordLength_7b;
            break ;
            
        case SCI_DATA_LENGTH_8_BITS:
            usart_init_structure.USART_WordLength = USART_WordLength_8b;
            break ;
        
        case SCI_DATA_LENGTH_9_BITS:
            usart_init_structure.USART_WordLength = USART_WordLength_9b;
            break ;
        
        default:
        case SCI_DATA_LENGTH_5_BITS:
        case SCI_DATA_LENGTH_6_BITS:
            goto error ;
            break ;
            
        }
        
        switch (config_struct_ptr->speed)
        {       
        case SCI_SPEED_4800_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_4800_BPS ;
            break ;
        case SCI_SPEED_9600_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_9600_BPS ;
            break ;
        case SCI_SPEED_19200_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_19200_BPS ;
            break ;
        case SCI_SPEED_38400_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_38400_BPS ;
            break ;
        case SCI_SPEED_57600_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_57600_BPS ;
            break ;
        case SCI_SPEED_115200_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_115200_BPS ;
            break ;
        case SCI_SPEED_128000_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_128000_BPS ;
            break ;
        case SCI_SPEED_230400_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_230400_BPS ;
            break ;
        case SCI_SPEED_250000_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_250000_BPS ;
            break ;
        case SCI_SPEED_460800_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_460800_BPS ;
            break ;
        case SCI_SPEED_921600_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_921600_BPS ;
            break ;
        case SCI_SPEED_1497600_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_1497600_BPS ;
            break ;
        case SCI_SPEED_1843200_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_1843200_BPS ;
            break ;
        case SCI_SPEED_3686400_BPS:
            usart_init_structure.USART_BaudRate = SCI_SPEED_3686400_BPS ;
            break ;
            
        default:
            goto error ;
            break ;
        }
        
        switch (config_struct_ptr->parity)
        {
        case SCI_PARITY_NONE:
            usart_init_structure.USART_Parity = USART_Parity_No ;
            break ;
        case SCI_PARITY_EVEN:
            usart_init_structure.USART_Parity = USART_Parity_Even ;
            break ;
        case SCI_PARITY_ODD:
            usart_init_structure.USART_Parity = USART_Parity_Odd ;
            break ;
        case SCI_PARITY_MARK:
            goto error ;
            break ;
        case SCI_PARITY_SPACE:
            goto error ;
            break ;
        default:
            goto error ;
            break ;
        }
        
        switch (config_struct_ptr->stopbit)
        {
        case SCI_STOPBIT_05:
            goto error ;
            break ;
        case SCI_STOPBIT_1:
            usart_init_structure.USART_StopBits = USART_StopBits_1 ;
            break ;
        case SCI_STOPBIT_15:
            usart_init_structure.USART_StopBits = USART_StopBits_1_5 ;
            break ;
        case SCI_STOPBIT_2:
            usart_init_structure.USART_StopBits = USART_StopBits_2 ;
            break ;
        default:
            goto error ;
            break ;
        }
        
        switch (channel)
        {
        case SCI_CHANNEL_USART1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE) ;
            usart_ptr = USART1 ;
            break ;
        case SCI_CHANNEL_USART2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE) ;
            usart_ptr = USART2 ;
            break ;
        case SCI_CHANNEL_USART3:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE) ;
            usart_ptr = USART3 ;
            break ;
        case SCI_CHANNEL_USART4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4, ENABLE) ;
            usart_ptr = USART4 ;
            break ;
        case SCI_CHANNEL_USART5:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART5, ENABLE) ;
            usart_ptr = USART5 ;
            break ;
        case SCI_CHANNEL_USART6:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE) ;
            usart_ptr = USART6 ;
            break ;
        default:    
            goto error ;
            break ;
        }
    }
    
    if (result)
    {
        usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_None ;
        usart_init_structure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx ;
        
        USART_Init(usart_ptr, &usart_init_structure) ;
        USART_Cmd(usart_ptr, ENABLE) ;
        USART_ITConfig(usart_ptr, USART_IT_RXNE, ENABLE);
        USART_ITConfig(usart_ptr, USART_IT_PE,   ENABLE);
        USART_ITConfig(usart_ptr, USART_IT_ERR,  ENABLE);
        USART_ITConfig(usart_ptr, USART_IT_ORE,  ENABLE);
        USART_ITConfig(usart_ptr, USART_IT_FE,   ENABLE);
        USART_ITConfig(usart_ptr, USART_IT_NE,   ENABLE);
    }
    
    return result ;
    
error:
    return FALSE ;
}

//------------------------------------------------------------------------------------------------//
static __irq void SysSciIntUsart1Isr(void)
{
    SysSciIntUsartIsr(SCI_CHANNEL_USART1, syssci_registers[SCI_CHANNEL_USART1] );
}
//------------------------------------------------------------------------------------------------//
static __irq void SysSciIntUsart2Isr(void)
{
    SysSciIntUsartIsr(SCI_CHANNEL_USART2, syssci_registers[SCI_CHANNEL_USART2]);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysSciIntUsart3Till8Isr(void)
{
    if (syssci_ctrl_struct[SCI_CHANNEL_USART3].init_done)
    {
        SysSciIntUsartIsr(SCI_CHANNEL_USART3, syssci_registers[SCI_CHANNEL_USART3]); 
    }
    if (syssci_ctrl_struct[SCI_CHANNEL_USART4].init_done)
    {
        SysSciIntUsartIsr(SCI_CHANNEL_USART4, syssci_registers[SCI_CHANNEL_USART4]);
    }
    if (syssci_ctrl_struct[SCI_CHANNEL_USART5].init_done)
    {

      SysSciIntUsartIsr(SCI_CHANNEL_USART5, syssci_registers[SCI_CHANNEL_USART5]);
             
    }
    if (syssci_ctrl_struct[SCI_CHANNEL_USART6].init_done)
    {  
      SysSciIntUsartIsr(SCI_CHANNEL_USART6, syssci_registers[SCI_CHANNEL_USART6]);
    }
}
//------------------------------------------------------------------------------------------------//
static void SysSciIntUsartIsr(SCI_CHANNEL channel, SCI_REG_HNDL_PTR channel_ptr)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];
    BOOL                error           = FALSE ;
    U8                  frame ;
    
    // first check for errors
    if (USART_GetITStatus(channel_ptr, USART_IT_ORE) == SET)
    {
        LOG_DEV("ERR OVR");
        ctrl_struct_ptr->overrun_err_cnt++;
        error = TRUE ;
        USART_ClearFlag(channel_ptr, USART_FLAG_ORE) ;
    }
    if (USART_GetITStatus(channel_ptr, USART_IT_NE) == SET)
    {
        LOG_DEV("ERR NOZ");
        ctrl_struct_ptr->noise_err_cnt++;
        error = TRUE ;
        USART_ClearFlag(channel_ptr, USART_FLAG_NE) ;
    }
    if (USART_GetITStatus(channel_ptr, USART_IT_FE) == SET)
    {
        LOG_DEV("ERR FRM");
        ctrl_struct_ptr->framing_err_cnt++;
        error = TRUE ;
        USART_ClearFlag(channel_ptr, USART_FLAG_FE) ;
    }
    if (USART_GetITStatus(channel_ptr, USART_IT_PE) == SET)
    {
        LOG_DEV("ERR PAR");
        ctrl_struct_ptr->parity_err_cnt++;
        error = TRUE ;
        USART_ClearFlag(channel_ptr, USART_FLAG_PE) ;
    }

    if (!error)
    {
        if (USART_GetITStatus(channel_ptr, USART_IT_LBD) == SET)
        {
#ifdef INCLUDE_LIN
            if(ctrl_struct_ptr->lin_break_detected_hook != NULL)
            {
                ctrl_struct_ptr->lin_break_detected_hook();
            }
#else
            USART_ITConfig(channel_ptr, USART_IT_LBD, DISABLE) ;
#endif
            USART_ClearFlag(channel_ptr, USART_FLAG_LBD) ;
        }
        else if (USART_GetITStatus(channel_ptr, USART_IT_RXNE) == SET)
        {
            USART_ClearFlag(channel_ptr, USART_FLAG_RXNE) ;
            
            frame = USART_ReceiveData(channel_ptr) ;
#ifdef INCLUDE_MPCM
            if(ctrl_struct_ptr->mpcm_mode) // mpcm mode?
            {
                LOG_DEV("RX %04h", PU16(frame));
                if(((frame & 0x0100) == 0x0000) // parity bit is '0'
                   && (ctrl_struct_ptr->mpcm_allow_rx == FALSE))  // and RX not allowed
                {
                    return;
                }
            }
#endif
            if(ctrl_struct_ptr->rx_new_byte_hook != NULL)
            {
                ctrl_struct_ptr->rx_new_byte_hook(&frame, 1);
            }
        }
        else if (USART_GetITStatus(channel_ptr, USART_IT_TXE) == SET)//TXE --> it is important that this is the last check because in a lot of the cases the Tx buffer is always empty.
        {   
            USART_ClearFlag(channel_ptr, USART_FLAG_TXE) ;
            
            if(ctrl_struct_ptr->tx_get_new_byte_hook != NULL)
            {
                if(ctrl_struct_ptr->tx_get_new_byte_hook(&frame, 1))
                {
#ifdef INCLUDE_MPCM
                    if(ctrl_struct_ptr->mpcm_mode && ctrl_struct_ptr->mpcm_address_indication) // mpcm mode?
                    {
                        USART_SendData(channel_ptr, 0x0100 | data_byte) ;
                        ctrl_struct_ptr->mpcm_address_indication = FALSE;
                    }
                    else
#endif
                    {
                        USART_SendData(channel_ptr, frame) ;
                    }
                    USART_ITConfig(channel_ptr, USART_IT_TXE, ENABLE) ;
                }
                else
                {
                    // done transmitting, call hook
                    if (syssci_msg_complete_hook != NULL)
                    {
                        while (USART_GetFlagStatus(channel_ptr, USART_FLAG_TC) == RESET) System_KickDog() ;
                        syssci_msg_complete_hook(channel);
                    }
                    
                    ctrl_struct_ptr->tx_busy = FALSE;
                    USART_ITConfig(channel_ptr, USART_IT_TXE, DISABLE) ;
                }
            }
        }
    }
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void SysSciInt_Init(void)
{
    syssci_msg_complete_hook = NULL;
    MEMSET((VPTR)syssci_ctrl_struct, 0, SIZEOF(syssci_ctrl_struct));
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciInt_RegisterMsgComplete(SCI_MSG_COMPLETE msg_complete_hook)
{
    syssci_msg_complete_hook = msg_complete_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciInt_Channel_Init(SCI_CHANNEL channel)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];
    
    if((U8)channel < SYSSCIINT_COUNT)
    {
        switch(channel)
        {
        case SCI_CHANNEL_USART1:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
            SysInt_RegisterIsr(USART1_IRQn, SysSciIntUsart1Isr);
            SysInt_EnableIsr(USART1_IRQn);
            break;
        case SCI_CHANNEL_USART2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
            SysInt_RegisterIsr(USART2_IRQn, SysSciIntUsart2Isr);
            SysInt_EnableIsr(USART2_IRQn);
            break;
        case SCI_CHANNEL_USART3:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
            SysInt_RegisterIsr(USART3_8_IRQn, SysSciIntUsart3Till8Isr);
            SysInt_EnableIsr(USART3_8_IRQn);
            break;
        case SCI_CHANNEL_USART4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4, ENABLE);
            SysInt_RegisterIsr(USART3_8_IRQn, SysSciIntUsart3Till8Isr);
            SysInt_EnableIsr(USART3_8_IRQn);
            break;
        case SCI_CHANNEL_USART5:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART5, ENABLE);
            SysInt_RegisterIsr(USART3_8_IRQn, SysSciIntUsart3Till8Isr);
            SysInt_EnableIsr(USART3_8_IRQn);
            break;
        case SCI_CHANNEL_USART6:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
            SysInt_RegisterIsr(USART3_8_IRQn, SysSciIntUsart3Till8Isr);
            SysInt_EnableIsr(USART3_8_IRQn);
            break;
        default:
            LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
            return FALSE;
        }
        
        ctrl_struct_ptr->init_done = TRUE;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciInt_Channel_Config(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];

    if((U8)channel < SYSSCIINT_COUNT)
    {
        if(ctrl_struct_ptr->init_done == TRUE)
        {
#ifdef INCLUDE_MPCM
            ctrl_struct_ptr->mpcm_mode = FALSE;
#endif
            ctrl_struct_ptr->tx_busy = FALSE;
            
            return SysSciIntInitPheripheral(channel, config_struct_ptr);
        }
        LOG_WRN("channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciInt_Channel_RegisterRxHook(SCI_CHANNEL channel, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook)
{
    if((U8)channel < SYSSCIINT_COUNT)
    {
        syssci_ctrl_struct[channel].rx_new_byte_hook = rx_new_byte_hook;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciInt_Channel_RegisterTxHook(SCI_CHANNEL channel, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook)
{
    if((U8)channel < SYSSCIINT_COUNT)
    {
        syssci_ctrl_struct[channel].tx_get_new_byte_hook = tx_get_next_byte_hook;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciInt_Channel_NotityTxDataReady(SCI_CHANNEL channel)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];
    
    if((U8)channel < SYSSCIINT_COUNT)
    {
        if(ctrl_struct_ptr->tx_get_new_byte_hook != NULL)
        {
            if(!ctrl_struct_ptr->tx_busy)
            {
                ctrl_struct_ptr->tx_busy = TRUE;
#ifdef INCLUDE_MPCM
                if(ctrl_struct_ptr->mpcm_mode)
                {
                    ctrl_struct_ptr->mpcm_address_indication = TRUE;
                }
#endif
                
                SCI_REG_HNDL_PTR channel_ptr = syssci_registers[channel] ;
                
                USART_ITConfig(channel_ptr, USART_IT_TXE, ENABLE) ;
                
                return TRUE;
            }
            return FALSE;
        }
        LOG_WRN("no TX hook - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
#ifdef INCLUDE_MPCM
BOOL SysSciInt_Channel_ConfigAsMpcm(SCI_CHANNEL channel, SCI_SPEED speed, BOOL allow_rx)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];
    SCI_CONFIG_STRUCT   config_struct;

    if((U8)channel < SYSSCIINT_COUNT)
    {
        if(ctrl_struct_ptr->init_done == TRUE)
        {
            config_struct.speed = speed;
            config_struct.parity = SCI_PARITY_NONE;
            config_struct.stopbit = SCI_STOPBIT_1;
            config_struct.data_length = SCI_DATA_LENGTH_9_BITS;
            ctrl_struct_ptr->mpcm_mode = TRUE;
            ctrl_struct_ptr->mpcm_allow_rx = allow_rx;
            ctrl_struct_ptr->mpcm_address_indication = FALSE;
            ctrl_struct_ptr->tx_busy = FALSE;
            
            return SysSciIntInitPheripheral(channel, &config_struct);
        }
        LOG_WRN("channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciInt_Channel_SetMpcmFilter(SCI_CHANNEL channel, BOOL enable)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];
    
    if((U8)channel < SYSSCIINT_COUNT)
    {
        ctrl_struct_ptr->mpcm_allow_rx = (BOOL)(enable == FALSE);
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
#endif
//------------------------------------------------------------------------------------------------//
#ifdef INCLUDE_LIN
BOOL SysSciInt_Channel_ConfigAsLin(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    SCI_REG_HNDL_PTR        reg_ptr = (SCI_REG_HNDL_PTR)syssci_registers[channel];
    
    if(SysSciInt_Channel_Config(channel, config_struct_ptr))
    {
        // ADD LIN SETTINGS
        SCI_REG_HNDL_PTR channel_ptr = syssci_registers[channel] ;
        
        // disable USART
        USART_Cmd(channel_ptr, DISABLE) ;
        USART_LINCmd(channel_ptr, ENABLE) ;
        USART_LINBreakDetectLengthConfig(channel_ptr, USART_LINBreakDetectLength_11b) ;
        USART_ITConfig(channel_ptr, USART_IT_LBD, ENABLE) ;
        // re-enable USART;
        USART_Cmd(channel_ptr, ENABLE) ;
        
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciInt_Channel_RegisterLinBreakDetectHook(SCI_CHANNEL channel, EVENT_CALLBACK lin_break_detected_hook)
{
    if((U8)channel < SYSSCIINT_COUNT)
    {
        syssci_ctrl_struct[channel].lin_break_detected_hook = lin_break_detected_hook;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciInt_Channel_SendLinBreak(SCI_CHANNEL channel)
{
    SCI_REG_HNDL_PTR        reg_ptr = (SCI_REG_HNDL_PTR)syssci_registers[channel];
    
    if((U8)channel < SYSSCIINT_COUNT)
    {
        reg_ptr->RQR |= USART_RQR_SBKRQ;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_channel_string), PU8(channel));
    return FALSE;
}
#endif
//------------------------------------------------------------------------------------------------//
U16 SysSciIntGetOverrunErrorCount(SCI_CHANNEL channel)
{
    if((U8)channel < SYSSCIINT_COUNT)
    {
        return syssci_ctrl_struct[channel].overrun_err_cnt;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 SysSciIntGetFramingErrorCount(SCI_CHANNEL channel)
{
    if((U8)channel < SYSSCIINT_COUNT)
    {
        return syssci_ctrl_struct[channel].framing_err_cnt;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 SysSciIntGetParityErrorCount(SCI_CHANNEL channel)
{
    if((U8)channel < SYSSCIINT_COUNT)
    {
        return syssci_ctrl_struct[channel].parity_err_cnt;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 SysSciIntGetNoiseErrorCount(SCI_CHANNEL channel)
{
    if((U8)channel < SYSSCIINT_COUNT)
    {
        return syssci_ctrl_struct[channel].noise_err_cnt;
    }
    return 0;
}
//================================================================================================//
