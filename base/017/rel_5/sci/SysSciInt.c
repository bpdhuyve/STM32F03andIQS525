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
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
/* STM32F09 has a different name for USART IRQ 29 than STM32F03 */
#ifndef STM32F030xC
#define USART3_6_IRQn USART3_8_IRQn
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    SCI_RX_NEW_BYTE_HOOK        rx_new_byte_hook;
    SCI_TX_GET_NEXT_BYTE_HOOK   tx_get_new_byte_hook;
    
#ifdef INCLUDE_MPCM
    BOOL                     mpcm_mode;
    BOOL                     mpcm_allow_rx;
    BOOL                     mpcm_address_indication;
#endif

    U16                 overrun_err_cnt;
    U16                 parity_err_cnt;
    U16                 framing_err_cnt;
    U16                 noise_err_cnt;
    
    BOOL                     init_done;
    BOOL                     tx_busy;
}
SCI_CTRL_STRUCT;

typedef USART_TypeDef*          SCI_REG_HNDL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL SysSciIntInitPheripheral(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr);
__irq static void SysSciIntUsart1Isr(void);
__irq static void SysSciIntUsart2Isr(void);
__irq static void SysSciIntUsart3Till6Isr(void);
static void SysSciIntUsartIsr(SCI_CHANNEL channel);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const SCI_REG_HNDL           syssci_registers[SCI_CHANNEL_COUNT] = {(SCI_REG_HNDL)USART1_BASE,
                                                                           (SCI_REG_HNDL)USART2_BASE,
                                                                           (SCI_REG_HNDL)USART3_BASE,
                                                                           (SCI_REG_HNDL)USART4_BASE,
                                                                           (SCI_REG_HNDL)USART5_BASE,
                                                                           (SCI_REG_HNDL)USART6_BASE};
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
    SCI_REG_HNDL        reg_ptr = (SCI_REG_HNDL)syssci_registers[channel];
    U32                 apbclock;

    reg_ptr->CR3 = USART_CR3_EIE;
    reg_ptr->CR2 = 0x0000;
    reg_ptr->CR1 &= ~USART_CR1_UE; //disable USART;
    
    switch(config_struct_ptr->stopbit)
    {
    case SCI_STOPBIT_1:
        break;
    case SCI_STOPBIT_15:
        reg_ptr->CR2 |= USART_CR2_STOP_1 | USART_CR2_STOP_0;
        break;
    case SCI_STOPBIT_2:
        reg_ptr->CR2 |= USART_CR2_STOP_1;
        break;
    default:
        LOG_ERR("unsupported stopbit config - %d", PU8(config_struct_ptr->stopbit));
        return FALSE;
    }
    
    reg_ptr->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE | USART_CR1_PEIE;
    
    reg_ptr->CR1 &= ~USART_CR1_M;
    switch(config_struct_ptr->data_length)
    {
    case SCI_DATA_LENGTH_8_BITS:
        break;
    case SCI_DATA_LENGTH_9_BITS:
        reg_ptr->CR1 |= USART_CR1_M;
        break;
    case SCI_DATA_LENGTH_7_BITS:
    default:
        LOG_ERR("unsupported data length - %d", PU8(config_struct_ptr->data_length));
        return FALSE;
    }
    
    reg_ptr->CR1 &= ~(USART_CR1_PCE | USART_CR1_PS);
    switch(config_struct_ptr->parity)
    {
    case SCI_PARITY_NONE:
        break;
    case SCI_PARITY_EVEN:
        reg_ptr->CR1 |= USART_CR1_PCE;
        break;
    case SCI_PARITY_ODD:
        reg_ptr->CR1 |= USART_CR1_PCE | USART_CR1_PS;
        break;
    case SCI_PARITY_MARK:
    case SCI_PARITY_SPACE:
    default:
        LOG_ERR("unsupported parity - %d", PU8(config_struct_ptr->parity));
        return FALSE;
    }
    
    apbclock = SysGetAPBClk();
    
    // baudrate = clock / (16*uartdiv) --> 16*uartdiv = clock / speed
    // 16*uartdiv has to be filled in BRR register
    // half of the speed is added for rounding
    reg_ptr->BRR = (apbclock + (config_struct_ptr->speed >> 1)) / config_struct_ptr->speed;
    
    reg_ptr->ISR = 0; //clear all flags
    
    reg_ptr->CR1 |= USART_CR1_UE; //enable USART;
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
__irq static void SysSciIntUsart1Isr(void)
{
    SysSciIntUsartIsr(SCI_CHANNEL_USART1);
}
//------------------------------------------------------------------------------------------------//
__irq static void SysSciIntUsart2Isr(void)
{
    SysSciIntUsartIsr(SCI_CHANNEL_USART2);
}
//------------------------------------------------------------------------------------------------//
__irq static void SysSciIntUsart3Till6Isr(void)
{
    SysSciIntUsartIsr(SCI_CHANNEL_USART3);
    SysSciIntUsartIsr(SCI_CHANNEL_USART4);
    SysSciIntUsartIsr(SCI_CHANNEL_USART5);
    SysSciIntUsartIsr(SCI_CHANNEL_USART6);
}
//------------------------------------------------------------------------------------------------//
static void SysSciIntUsartIsr(SCI_CHANNEL channel)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];
    SCI_REG_HNDL        reg_ptr = (SCI_REG_HNDL)syssci_registers[channel];
    U32         status = reg_ptr->ISR;
    U8          data_byte = (reg_ptr->RDR & 0x00FF);
    
    if(status & 0x000F) // ERROR
    {
        if(status & USART_ISR_ORE) /*** OVERRUN ERROR ***/
        {
            LOG_DEV("ERR OVR");
            ctrl_struct_ptr->overrun_err_cnt++;
        }
        if(status & USART_ISR_NE) /*** NOISE ERROR ***/
        {
            LOG_DEV("ERR NOZ");
            ctrl_struct_ptr->noise_err_cnt++;
        }
        if(status & USART_ISR_FE) /*** FRAMING ERROR ***/
        {
            LOG_DEV("ERR FRM");
            ctrl_struct_ptr->framing_err_cnt++;
        }
        if(status & USART_ISR_PE) /*** PARITY ERROR ***/
        {
            LOG_DEV("ERR PAR");
            ctrl_struct_ptr->parity_err_cnt++;
        }
        
        // acknowledge interrupt
        reg_ptr->ICR |= status;
    }
    else if((status & reg_ptr->CR1) & USART_CR1_RXNEIE)  // RXNE
    {
#ifdef INCLUDE_MPCM
        if(ctrl_struct_ptr->mpcm_mode) // mpcm mode?
        {
            LOG_DEV("RX %04h", PU16(reg_ptr->RDR));
            if(((reg_ptr->RDR & 0x0100) == 0x0000) // parity bit is '0'
               && (ctrl_struct_ptr->mpcm_allow_rx == FALSE))  // and RX not allowed
            {
                return;
            }
        }
#endif
        if(ctrl_struct_ptr->rx_new_byte_hook != NULL)
        {
            ctrl_struct_ptr->rx_new_byte_hook(&data_byte, 1);
        }
    }
    else if((status & reg_ptr->CR1) & USART_CR1_IDLEIE) //IDLE LINE detection
    {
        reg_ptr->CR1 &= ~USART_CR1_IDLEIE;
    }
    else if((status & reg_ptr->CR1) & USART_CR1_TCIE) //TX comp detection
    {
        reg_ptr->CR1 &= ~USART_CR1_TCIE;
        reg_ptr->ISR &= ~0x0040; // write 0 to clear
        if(syssci_msg_complete_hook != NULL)
        {
            syssci_msg_complete_hook(channel);
        }
    }
    else if((status & reg_ptr->CR1) & USART_CR1_TXEIE) //TXE --> it is important that this is the last check because in a lot of the cases the Tx buffer is always empty.
    {
        reg_ptr->CR1 &= ~(USART_CR1_TXEIE);
        if(ctrl_struct_ptr->tx_get_new_byte_hook != NULL)
        {
            if(ctrl_struct_ptr->tx_get_new_byte_hook(&data_byte, 1))
            {
#ifdef INCLUDE_MPCM
                if(ctrl_struct_ptr->mpcm_mode && ctrl_struct_ptr->mpcm_address_indication) // mpcm mode?
                {
                    reg_ptr->TDR = 0x0100 | data_byte;
                    ctrl_struct_ptr->mpcm_address_indication = FALSE;
                }
                else
#endif
                {
                    reg_ptr->TDR = data_byte;
                }
                reg_ptr->CR1 |= USART_CR1_TXEIE;
            }
            else
            {
                ctrl_struct_ptr->tx_busy = FALSE;
                if(syssci_msg_complete_hook != NULL)
                {
                    reg_ptr->CR1 |= USART_CR1_TCIE;
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
            SysInt_RegisterIsr(USART1_IRQn, (EVENT_CALLBACK)SysSciIntUsart1Isr);
            SysInt_EnableIsr(USART1_IRQn);
            break;
        case SCI_CHANNEL_USART2:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
            SysInt_RegisterIsr(USART2_IRQn, (EVENT_CALLBACK)SysSciIntUsart2Isr);
            SysInt_EnableIsr(USART2_IRQn);
            break;
        case SCI_CHANNEL_USART3:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
            SysInt_RegisterIsr(USART3_6_IRQn, (EVENT_CALLBACK)SysSciIntUsart3Till6Isr);
            SysInt_EnableIsr(USART3_6_IRQn);
            break;
        case SCI_CHANNEL_USART4:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4, ENABLE);
            SysInt_RegisterIsr(USART3_6_IRQn, (EVENT_CALLBACK)SysSciIntUsart3Till6Isr);
            SysInt_EnableIsr(USART3_6_IRQn);
            break;
        case SCI_CHANNEL_USART5:
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART5, ENABLE);
            SysInt_RegisterIsr(USART3_6_IRQn, (EVENT_CALLBACK)SysSciIntUsart3Till6Isr);
            SysInt_EnableIsr(USART3_6_IRQn);
            break;
        case SCI_CHANNEL_USART6:
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
            SysInt_RegisterIsr(USART3_6_IRQn, (EVENT_CALLBACK)SysSciIntUsart3Till6Isr);
            SysInt_EnableIsr(USART3_6_IRQn);
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
    SCI_REG_HNDL        reg_ptr = (SCI_REG_HNDL)syssci_registers[channel];;

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
                reg_ptr->CR1 &= ~USART_CR1_TCIE;
                reg_ptr->CR1 |= USART_CR1_TXEIE;
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
