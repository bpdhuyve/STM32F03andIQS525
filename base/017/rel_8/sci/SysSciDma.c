//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the interrupt driven SCI system.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SCI__SYSSCIDMA_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SCI__SYSSCIDMA_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SCI__SYSSCIDMA_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of dma driven SCI channels
#ifndef SYSSCIDMA_COUNT
	#define SYSSCIDMA_COUNT			    SCI_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYS include section
#include "sci\SysSciDma.h"

#include "core\stm32f0xx_dma.h"
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
typedef USART_TypeDef*          SCI_REG_HNDL;
typedef DMA_Channel_TypeDef*    DMA_REG_HNDL;

typedef struct
{
    U8* q_buffer_base_ptr;
    U16 q_size;
    U16 q_write_index;
    U16 q_read_index;
}
Q_CTRL_STRUCT;

typedef struct
{
    SCI_RX_NEW_BYTE_HOOK        rx_new_byte_hook;
    SCI_TX_GET_NEXT_BYTE_HOOK   tx_get_new_byte_hook;

    U16                 overrun_err_cnt;
    U16                 parity_err_cnt;
    U16                 framing_err_cnt;
    U16                 noise_err_cnt;
    U16                 dma_err_cnt;

    BOOL                init_done;
    BOOL                rx_busy;

    DMA_CHANNEL        dma_tx_channel;
    DMA_CHANNEL        dma_rx_channel;

    Q_CTRL_STRUCT       rx_q;
    Q_CTRL_STRUCT       tx_q;

}
SCI_CTRL_STRUCT;

typedef enum
{
    DIR_TX = 0,
    DIR_RX = 1
} DMA_DIRECTION;

typedef struct
{
    BOOL                dma_active;
    DMA_DIRECTION       direction;
    SCI_CHANNEL         sci_channel;
}
DMA_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static __irq void SysSciDma1Channel1Isr(void);
static __irq void SysSciDma1Channel2Till3Dma2Channel1Till2Isr(void);
static __irq void SysSciDma1Channel4Till7Dma2Channel3Till5Isr(void);
static void SysSciDmaChannelIsr(DMA_CHANNEL channel);
static BOOL SysSciDmaInitPheripheral(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr);
static __irq void SysSciDmaUsart1Isr(void);
static __irq void SysSciDmaUsart2Isr(void);
static __irq void SysSciDmaUsart3Till6Isr(void);
static void SysSciDmaUsartIsr(SCI_CHANNEL channel);
static void SysSciDmaRxNewData(SCI_CHANNEL channel);
static BOOL SysSciDmaStartTxNewData(SCI_CHANNEL channel);
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
static const DMA_REG_HNDL           sysscidma_registers[DMA_CHANNEL_COUNT] = {(DMA_REG_HNDL)DMA1_Channel1_BASE,
                                                                              (DMA_REG_HNDL)DMA1_Channel2_BASE,
                                                                              (DMA_REG_HNDL)DMA1_Channel3_BASE,
                                                                              (DMA_REG_HNDL)DMA1_Channel4_BASE,
                                                                              (DMA_REG_HNDL)DMA1_Channel5_BASE,
                                                                              (DMA_REG_HNDL)DMA1_Channel6_BASE,
                                                                              (DMA_REG_HNDL)DMA1_Channel7_BASE,
                                                                              (DMA_REG_HNDL)DMA2_Channel1_BASE,
                                                                              (DMA_REG_HNDL)DMA2_Channel2_BASE,
                                                                              (DMA_REG_HNDL)DMA2_Channel3_BASE,
                                                                              (DMA_REG_HNDL)DMA2_Channel4_BASE,
                                                                              (DMA_REG_HNDL)DMA2_Channel5_BASE};
static SCI_CTRL_STRUCT              syssci_ctrl_struct[SYSSCIDMA_COUNT];
static DMA_CTRL_STRUCT              syssci_dma_ctrl_struct[DMA_CHANNEL_COUNT];
static SCI_MSG_COMPLETE             syssci_msg_complete_hook;
#if (CORELOG_LEVEL & LOG_LEVEL_ERROR) != 0
static const STRING                 syssci_illegal_sci_channel_string = "illegal sci channel - ";
static const STRING                 syssci_illegal_dma_channel_string = "illegal dma channel - ";
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void SysSciDmaRegisterCircBuffer(Q_CTRL_STRUCT* q_ctrl_ptr, U16 q_size)
{
    q_ctrl_ptr->q_buffer_base_ptr = CoreBuffer_CreateStaticU8(q_size, "SysSciDma");
    q_ctrl_ptr->q_size = q_size;
    q_ctrl_ptr->q_write_index = 0;
    q_ctrl_ptr->q_read_index = 0;
}
//------------------------------------------------------------------------------------------------//
static void SysSciDmaSetupIrq(DMA_CHANNEL channel)
{
    switch (channel)
    {
    case DMA1_CHANNEL_1:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
        SysInt_RegisterIsr(DMA1_Ch1_IRQn, SysSciDma1Channel1Isr);
        SysInt_EnableIsr(DMA1_Ch1_IRQn);
        break;
    case DMA1_CHANNEL_2:
    case DMA1_CHANNEL_3:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
        SysInt_RegisterIsr(DMA1_Ch2_3_DMA2_Ch1_2_IRQn, SysSciDma1Channel2Till3Dma2Channel1Till2Isr);
        SysInt_EnableIsr(DMA1_Ch2_3_DMA2_Ch1_2_IRQn);
        break;
    case DMA1_CHANNEL_4:
    case DMA1_CHANNEL_5:
    case DMA1_CHANNEL_6:
    case DMA1_CHANNEL_7:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
        SysInt_RegisterIsr(DMA1_Ch4_7_DMA2_Ch3_5_IRQn, SysSciDma1Channel4Till7Dma2Channel3Till5Isr);
        SysInt_EnableIsr(DMA1_Ch4_7_DMA2_Ch3_5_IRQn);
        break;
#ifdef STM32F091
    // other STM32F0X don't have DMA2
    case DMA2_CHANNEL_1:
    case DMA2_CHANNEL_2:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
        SysInt_RegisterIsr(DMA1_Ch2_3_DMA2_Ch1_2_IRQn, SysSciDma1Channel2Till3Dma2Channel1Till2Isr);
        SysInt_EnableIsr(DMA1_Ch2_3_DMA2_Ch1_2_IRQn);
        break;
    case DMA2_CHANNEL_3:
    case DMA2_CHANNEL_4:
    case DMA2_CHANNEL_5:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
        SysInt_RegisterIsr(DMA1_Ch4_7_DMA2_Ch3_5_IRQn, SysSciDma1Channel4Till7Dma2Channel3Till5Isr);
        SysInt_EnableIsr(DMA1_Ch4_7_DMA2_Ch3_5_IRQn);
        break;
#endif
    }
}
//------------------------------------------------------------------------------------------------//
static __irq void SysSciDma1Channel1Isr(void)
{
    SysSciDmaChannelIsr(DMA1_CHANNEL_1);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysSciDma1Channel2Till3Dma2Channel1Till2Isr(void)
{
    if (DMA_GetFlagStatus(DMA1_FLAG_GL2) == SET)
    {
        SysSciDmaChannelIsr(DMA1_CHANNEL_2);
    }

    if (DMA_GetFlagStatus(DMA1_FLAG_GL3) == SET)
    {
        SysSciDmaChannelIsr(DMA1_CHANNEL_3);
    }

#ifdef STM32F091
    // other STM32F0X don't have DMA2
    if (DMA_GetFlagStatus(DMA2_FLAG_GL1) == SET)
    {
        SysSciDmaChannelIsr(DMA2_CHANNEL_1);
    }

    if (DMA_GetFlagStatus(DMA2_FLAG_GL2) == SET)
    {
        SysSciDmaChannelIsr(DMA2_CHANNEL_2);
    }
#endif
}
//------------------------------------------------------------------------------------------------//
static __irq void SysSciDma1Channel4Till7Dma2Channel3Till5Isr(void)
{
    if (DMA_GetFlagStatus(DMA1_FLAG_GL4) == SET)
    {
        SysSciDmaChannelIsr(DMA1_CHANNEL_4);
    }

    if (DMA_GetFlagStatus(DMA1_FLAG_GL5) == SET)
    {
        SysSciDmaChannelIsr(DMA1_CHANNEL_5);
    }

    if (DMA_GetFlagStatus(DMA1_FLAG_GL6) == SET)
    {
        SysSciDmaChannelIsr(DMA1_CHANNEL_6);
    }

    if (DMA_GetFlagStatus(DMA1_FLAG_GL7) == SET)
    {
        SysSciDmaChannelIsr(DMA1_CHANNEL_7);
    }

#ifdef STM32F091
    // other STM32F0X don't have DMA2
    if (DMA_GetFlagStatus(DMA2_FLAG_GL3) == SET)
    {
        SysSciDmaChannelIsr(DMA2_CHANNEL_3);
    }

    if (DMA_GetFlagStatus(DMA2_FLAG_GL4) == SET)
    {
        SysSciDmaChannelIsr(DMA2_CHANNEL_4);
    }

    if (DMA_GetFlagStatus(DMA2_FLAG_GL5) == SET)
    {
        SysSciDmaChannelIsr(DMA2_CHANNEL_5);
    }
#endif
}
//------------------------------------------------------------------------------------------------//
static void SysSciDmaChannelIsr(DMA_CHANNEL channel)
{
    SCI_CHANNEL sci_channel = syssci_dma_ctrl_struct[channel].sci_channel;
    static U32 dma_tx_accidently_halftransfer_isr_count = 0;

    U32 tmpreg;
    U8 bit_offset;

    if (channel < DMA2_CHANNEL_1)
    {
        tmpreg = DMA1->ISR;
        bit_offset = channel * 4;
        DMA1->IFCR = (tmpreg & (0xE << bit_offset)); // only clear set bits
    }
    else
    {
        tmpreg = DMA2->ISR ;
        bit_offset = (channel - DMA2_CHANNEL_1) * 4;
        DMA2->IFCR = (tmpreg & (0xE << bit_offset)); // only clear set bits
    }

    tmpreg >>= bit_offset;
    tmpreg &= 0xF;

    if (syssci_dma_ctrl_struct[channel].direction == DIR_RX)
    {
        if (tmpreg & 0x2) // Transfer complete ISR flag
        {
            SysSciDmaRxNewData(sci_channel);
        }

        if (tmpreg & 0x4) // Half transfer ISR flag
        {
            SysSciDmaRxNewData(sci_channel);
        }

        if (tmpreg & 0x8) // DMA error ISR flag
        {
            syssci_ctrl_struct[sci_channel].dma_err_cnt++;
        }
    }
    else // TX
    {
        if ((tmpreg & 0xA) != 0)
        {
            // done or error, clear busy flag
            syssci_dma_ctrl_struct[channel].dma_active = FALSE;

            if (tmpreg & 0x2) // Transfer complete ISR flag
            {
                SysSciDmaStartTxNewData(sci_channel);
            }

            if (tmpreg & 0x8) // DMA error ISR flag
            {
                syssci_ctrl_struct[sci_channel].dma_err_cnt++;
            }
        }
        else
        {
            //oops, HT isr flags is set, but it was the isr of the RX channel that also called this
            dma_tx_accidently_halftransfer_isr_count++;
        }
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL SysSciDmaInitPheripheral(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    SCI_REG_HNDL        reg_ptr = (SCI_REG_HNDL)syssci_registers[channel];
    U32                 apbclock;

    reg_ptr->CR3 = USART_CR3_EIE | USART_CR3_DMAT | USART_CR3_DMAR; // enable Error interrupt, DMA receiver & DMA transmitter
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

    // enable receiver, transmitter, parity error interrupt & idle interrupt
    reg_ptr->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_PEIE | USART_CR1_IDLEIE;

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
static __irq void SysSciDmaUsart1Isr(void)
{
    SysSciDmaUsartIsr(SCI_CHANNEL_USART1);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysSciDmaUsart2Isr(void)
{
    SysSciDmaUsartIsr(SCI_CHANNEL_USART2);
}
//------------------------------------------------------------------------------------------------//
static __irq void SysSciDmaUsart3Till6Isr(void)
{
    if (syssci_ctrl_struct[SCI_CHANNEL_USART3].init_done)
    {
      SysSciDmaUsartIsr(SCI_CHANNEL_USART3);
    }
    if (syssci_ctrl_struct[SCI_CHANNEL_USART4].init_done)
    {
      SysSciDmaUsartIsr(SCI_CHANNEL_USART4);
    }
    if (syssci_ctrl_struct[SCI_CHANNEL_USART5].init_done)
    {
      SysSciDmaUsartIsr(SCI_CHANNEL_USART5);
    }
    if (syssci_ctrl_struct[SCI_CHANNEL_USART6].init_done)
    {
      SysSciDmaUsartIsr(SCI_CHANNEL_USART6);
    }
}
//------------------------------------------------------------------------------------------------//
static void SysSciDmaUsartIsr(SCI_CHANNEL channel)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];
    SCI_REG_HNDL        reg_ptr = (SCI_REG_HNDL)syssci_registers[channel];
    U32                 status = reg_ptr->ISR;

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
        reg_ptr->ICR |= (status & 0x000F);
    }

    if((status & reg_ptr->CR1) & USART_CR1_TCIE) //TX complete detection
    {
        reg_ptr->CR1 &= ~USART_CR1_TCIE;
        reg_ptr->ISR &= ~0x0040; // write 0 to clear
        if(syssci_msg_complete_hook != NULL)
        {
            syssci_msg_complete_hook(channel);
        }
    }

    if((status & reg_ptr->CR1) & USART_CR1_IDLEIE) //IDLE LINE detection
    {
        SysSciDmaRxNewData(channel);
        reg_ptr->ICR |= USART_ISR_IDLE;
    }

}
//------------------------------------------------------------------------------------------------//
static void SysSciDmaRxNewData(SCI_CHANNEL channel)
{
    SCI_CTRL_STRUCT*    ctrl = &syssci_ctrl_struct[channel];
    Q_CTRL_STRUCT*      rx_q = &ctrl->rx_q;
    DMA_REG_HNDL        rx_reg_hndl = sysscidma_registers[ctrl->dma_rx_channel];

    // protect this routine from being called twice in a row
    Core_CriticalEnter();
    if (ctrl->rx_busy == TRUE)
    {
        Core_CriticalExit();
        return;
    }

    ctrl->rx_busy = TRUE;
    Core_CriticalExit();


    if (ctrl->rx_new_byte_hook != NULL)
    {
        rx_q->q_write_index = rx_q->q_size - rx_reg_hndl->CNDTR;

        // loop till there is no more data to fetch, to prevent missing a byte when 2 interrupts come in a row
        do
        {
            if (rx_q->q_write_index > rx_q->q_read_index)
            {
                ctrl->rx_new_byte_hook(rx_q->q_buffer_base_ptr + rx_q->q_read_index,
                                       rx_q->q_write_index - rx_q->q_read_index);
            }
            else if (rx_q->q_write_index < rx_q->q_read_index)
            {
                ctrl->rx_new_byte_hook(rx_q->q_buffer_base_ptr + rx_q->q_read_index,
                                       rx_q->q_size - rx_q->q_read_index);
                ctrl->rx_new_byte_hook(rx_q->q_buffer_base_ptr, rx_q->q_write_index);
            }
            // update read index
            rx_q->q_read_index = rx_q->q_write_index;
            // update write index again, there might be a new byte already
            rx_q->q_write_index = rx_q->q_size - rx_reg_hndl->CNDTR;

        } while (rx_q->q_read_index != rx_q->q_write_index);
    }

    ctrl->rx_busy = FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL SysSciDmaStartTxNewData(SCI_CHANNEL sci_channel)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[sci_channel];
    Q_CTRL_STRUCT* tx_q = &ctrl_struct_ptr->tx_q;
    U16 nb_bytes_to_send;
    DMA_InitTypeDef     dma_tx_initstruct;

    Core_CriticalEnter();
    if (syssci_dma_ctrl_struct[ctrl_struct_ptr->dma_tx_channel].dma_active == TRUE)
    {
        // already busy
        Core_CriticalExit();
        return FALSE;
    }

    syssci_dma_ctrl_struct[ctrl_struct_ptr->dma_tx_channel].dma_active = TRUE;

    Core_CriticalExit();

    if (ctrl_struct_ptr->tx_get_new_byte_hook != NULL)
    {
        DMA_DeInit(sysscidma_registers[ctrl_struct_ptr->dma_tx_channel]);

        nb_bytes_to_send = ctrl_struct_ptr->tx_get_new_byte_hook(tx_q->q_buffer_base_ptr, tx_q->q_size);

        if (nb_bytes_to_send > 0)
        {
            // Initialize UART TX DMA Channel:
            dma_tx_initstruct.DMA_PeripheralBaseAddr = (uint32_t)&syssci_registers[sci_channel]->TDR; // UART1 TX Data Register.
            dma_tx_initstruct.DMA_MemoryBaseAddr = (uint32_t)tx_q->q_buffer_base_ptr; // Copy data from TxBuffer.
            dma_tx_initstruct.DMA_DIR = DMA_DIR_PeripheralDST; // Peripheral as destination, memory as source.
            dma_tx_initstruct.DMA_BufferSize = nb_bytes_to_send; // Defined above.
            dma_tx_initstruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // No increment on TDR address.
            dma_tx_initstruct.DMA_MemoryInc = DMA_MemoryInc_Enable; // Increment memory address.
            dma_tx_initstruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // Byte-wise copy.
            dma_tx_initstruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; // Byte-wise copy.
            dma_tx_initstruct.DMA_Mode = DMA_Mode_Normal; // Single transfer
            dma_tx_initstruct.DMA_Priority = DMA_Priority_High; // High priority.
            dma_tx_initstruct.DMA_M2M = DMA_M2M_Disable; // Peripheral to memory, not M2M.
            // Initialize UART TX DMA Channel.
            DMA_Init(sysscidma_registers[ctrl_struct_ptr->dma_tx_channel], &dma_tx_initstruct);
            // Enable Transfer Complete and Transfer Error interrupts.
            DMA_ITConfig(sysscidma_registers[ctrl_struct_ptr->dma_tx_channel], DMA_IT_TC | DMA_IT_TE, ENABLE);
            // Enable UART TX DMA Channel.
            DMA_Cmd(sysscidma_registers[ctrl_struct_ptr->dma_tx_channel], ENABLE);

            return TRUE;

        }
        else
        {
            if(syssci_msg_complete_hook != NULL)
            {
                syssci_registers[sci_channel]->CR1 |= USART_CR1_TCIE;
            }
        }

    }

    syssci_dma_ctrl_struct[ctrl_struct_ptr->dma_tx_channel].dma_active = FALSE;
    return FALSE;
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void SysSciDma_Init(void)
{
    syssci_msg_complete_hook = NULL;
    MEMSET((VPTR)syssci_ctrl_struct, 0, SIZEOF(syssci_ctrl_struct));
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciDma_RegisterMsgComplete(SCI_MSG_COMPLETE msg_complete_hook)
{
    syssci_msg_complete_hook = msg_complete_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciDma_Channel_Init(SCI_CHANNEL sci_channel, DMA_CHANNEL dma_rx_channel, U16 rx_buffer_size, DMA_CHANNEL dma_tx_channel, U16 tx_buffer_size)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[sci_channel];
    DMA_InitTypeDef     dma_rx_initstruct;

    if((U8)sci_channel >= SYSSCIDMA_COUNT)
    {
        LOG_ERR("%s%d", PCSTR(syssci_illegal_sci_channel_string), PU8(channel));
        return FALSE;
    }

    if (dma_rx_channel >= DMA_CHANNEL_COUNT)
    {
        LOG_ERR("%s%d", PCSTR(syssci_illegal_dma_channel_string), PU8(dma_rx_channel));
        return FALSE;
    }

    if (dma_tx_channel >= DMA_CHANNEL_COUNT)
    {
        LOG_ERR("%s%d", PCSTR(syssci_illegal_dma_channel_string), PU8(dma_tx_channel));
        return FALSE;
    }

    SysSciDmaRegisterCircBuffer(&ctrl_struct_ptr->rx_q, rx_buffer_size);
    SysSciDmaRegisterCircBuffer(&ctrl_struct_ptr->tx_q, tx_buffer_size);


    SysSciDmaSetupIrq(dma_rx_channel);
    SysSciDmaSetupIrq(dma_tx_channel);

    DMA_DeInit(sysscidma_registers[dma_rx_channel]);

    // Initialize USART1 RX DMA Channel:
    dma_rx_initstruct.DMA_PeripheralBaseAddr = (uint32_t)&syssci_registers[sci_channel]->RDR; // USART1 RX Data Register.
    dma_rx_initstruct.DMA_MemoryBaseAddr = (uint32_t)ctrl_struct_ptr->rx_q.q_buffer_base_ptr; // Copy data to RxBuffer.
    dma_rx_initstruct.DMA_DIR = DMA_DIR_PeripheralSRC; // Peripheral as source, memory as destination.
    dma_rx_initstruct.DMA_BufferSize = rx_buffer_size; // Defined above.
    dma_rx_initstruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // No increment on RDR address.
    dma_rx_initstruct.DMA_MemoryInc = DMA_MemoryInc_Enable; // Increment memory address.
    dma_rx_initstruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // Byte-wise copy.
    dma_rx_initstruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; // Byte-wise copy.
    dma_rx_initstruct.DMA_Mode = DMA_Mode_Circular; // Ring buffer - don't interrupt when at end of memory region.
    dma_rx_initstruct.DMA_Priority = DMA_Priority_High; // High priority.
    dma_rx_initstruct.DMA_M2M = DMA_M2M_Disable; // Peripheral to memory, not M2M.
    // Initialize USART1 RX DMA Channel.
    DMA_Init(sysscidma_registers[dma_rx_channel], &dma_rx_initstruct);
    // Enable Transfer Complete, Half Transfer and Transfer Error interrupts.
    DMA_ITConfig(sysscidma_registers[dma_rx_channel], DMA_IT_TC | DMA_IT_HT | DMA_IT_TE, ENABLE);
    // Enable USART1 RX DMA Channel.
    DMA_Cmd(sysscidma_registers[dma_rx_channel], ENABLE);


    syssci_dma_ctrl_struct[dma_rx_channel].sci_channel = sci_channel;
    syssci_dma_ctrl_struct[dma_rx_channel].direction = DIR_RX;
    syssci_dma_ctrl_struct[dma_rx_channel].dma_active = TRUE;

    syssci_dma_ctrl_struct[dma_tx_channel].sci_channel = sci_channel;
    syssci_dma_ctrl_struct[dma_tx_channel].direction = DIR_TX;
    syssci_dma_ctrl_struct[dma_tx_channel].dma_active = FALSE; // will be active when TX is busy

    switch(sci_channel)
    {
    case SCI_CHANNEL_USART1:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        SysInt_RegisterIsr(USART1_IRQn, SysSciDmaUsart1Isr);
        SysInt_EnableIsr(USART1_IRQn);
        break;
    case SCI_CHANNEL_USART2:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        SysInt_RegisterIsr(USART2_IRQn, SysSciDmaUsart2Isr);
        SysInt_EnableIsr(USART2_IRQn);
        break;
    case SCI_CHANNEL_USART3:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
        SysInt_RegisterIsr(USART3_6_IRQn, SysSciDmaUsart3Till6Isr);
        SysInt_EnableIsr(USART3_6_IRQn);
        break;
    case SCI_CHANNEL_USART4:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4, ENABLE);
        SysInt_RegisterIsr(USART3_6_IRQn, SysSciDmaUsart3Till6Isr);
        SysInt_EnableIsr(USART3_6_IRQn);
        break;
    case SCI_CHANNEL_USART5:
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART5, ENABLE);
        SysInt_RegisterIsr(USART3_6_IRQn, SysSciDmaUsart3Till6Isr);
        SysInt_EnableIsr(USART3_6_IRQn);
        break;
    case SCI_CHANNEL_USART6:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
        SysInt_RegisterIsr(USART3_6_IRQn, SysSciDmaUsart3Till6Isr);
        SysInt_EnableIsr(USART3_6_IRQn);
        break;
    default:
        LOG_ERR("%s%d", PCSTR(syssci_illegal_sci_channel_string), PU8(channel));
        return FALSE;
    }


    ctrl_struct_ptr->dma_rx_channel = dma_rx_channel;
    ctrl_struct_ptr->dma_tx_channel = dma_tx_channel;
    ctrl_struct_ptr->init_done = TRUE;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciDma_Channel_Config(SCI_CHANNEL channel, SCI_CONFIG_STRUCT* config_struct_ptr)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];

    if((U8)channel < SYSSCIDMA_COUNT)
    {
        if(ctrl_struct_ptr->init_done == TRUE)
        {
            return SysSciDmaInitPheripheral(channel, config_struct_ptr);
        }
        LOG_WRN("channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_sci_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciDma_Channel_RegisterRxHook(SCI_CHANNEL channel, SCI_RX_NEW_BYTE_HOOK rx_new_byte_hook)
{
    if((U8)channel < SYSSCIDMA_COUNT)
    {
        syssci_ctrl_struct[channel].rx_new_byte_hook = rx_new_byte_hook;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_sci_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciDma_Channel_RegisterTxHook(SCI_CHANNEL channel, SCI_TX_GET_NEXT_BYTE_HOOK tx_get_next_byte_hook)
{
    if((U8)channel < SYSSCIDMA_COUNT)
    {
        syssci_ctrl_struct[channel].tx_get_new_byte_hook = tx_get_next_byte_hook;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_sci_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysSciDma_Channel_NotityTxDataReady(SCI_CHANNEL channel)
{
    SCI_CTRL_STRUCT*    ctrl_struct_ptr = &syssci_ctrl_struct[channel];

    if((U8)channel < SYSSCIDMA_COUNT)
    {
        if(ctrl_struct_ptr->tx_get_new_byte_hook != NULL)
        {
            return SysSciDmaStartTxNewData(channel);
        }
        LOG_WRN("no TX hook - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("%s%d", PCSTR(syssci_illegal_sci_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
U16 SysSciDmaGetOverrunErrorCount(SCI_CHANNEL channel)
{
    if((U8)channel < SYSSCIDMA_COUNT)
    {
        return syssci_ctrl_struct[channel].overrun_err_cnt;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 SysSciDmaGetFramingErrorCount(SCI_CHANNEL channel)
{
    if((U8)channel < SYSSCIDMA_COUNT)
    {
        return syssci_ctrl_struct[channel].framing_err_cnt;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 SysSciDmaGetParityErrorCount(SCI_CHANNEL channel)
{
    if((U8)channel < SYSSCIDMA_COUNT)
    {
        return syssci_ctrl_struct[channel].parity_err_cnt;
    }
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 SysSciDmaGetNoiseErrorCount(SCI_CHANNEL channel)
{
    if((U8)channel < SYSSCIDMA_COUNT)
    {
        return syssci_ctrl_struct[channel].noise_err_cnt;
    }
    return 0;
}
//================================================================================================//
