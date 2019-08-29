//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// CAN peripheral interface
//
// Processor       : specific
// Implementation  : specific
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CAN__SYSCANINT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CAN__SYSCANINT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               CAN__SYSCANINT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the maximum number of interrupt driven CAN channels
#ifndef SYSCANINT_COUNT
	#define SYSCANINT_COUNT			    CAN_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//SYS include section
#include "can\SysCanInt.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define STDPCMS_BROADCAST_BIT           0x02000000
#define STDPCMS_SUBNODE_MASK            0x00010700
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    BOOL                       init_done;
    CAN_RX_NEW_MSSG_HOOK       rx_new_message_hook;
    CAN_TX_GET_NEXT_MSSG_HOOK  tx_get_next_message_hook;
    CAN_ERROR_HOOK             error_hook;
}
CAN_CTRL_STRUCT;


typedef CAN_TypeDef*           CAN_REG_HNDL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL SysCanIntInitPheripheral(CAN_REG_HNDL reg_ptr, CAN_CONFIG_STRUCT* config_struct_ptr);
static BOOL SysCanIntInitPeripheralSpeed(CAN_REG_HNDL reg_ptr, CAN_SPEED speed);
static BOOL SysCanIntRecoverFromBusOff(CAN_REG_HNDL reg_ptr);
static BOOL SysCanIntSetAcceptanceFilter(CAN_REG_HNDL      reg_ptr,
                                         ACCEPTANCE_FILTER filter,
                                         U32               mask,            // which bits are important
                                         U32               match,           // specifies wether these these important bits must be 0 or 1
                                         IDENTIFIER_TYPE   identifier_type, // what's the expected identifier type
                                         FRAME_TYPE        frame_type);     // data or remote frame
static BOOL SysCanIntAddFilterToRxMailbox(CAN_REG_HNDL      reg_ptr,
                                          RX_MAILBOX        mailbox,
                                          ACCEPTANCE_FILTER filter);
static BOOL SysCanIntConfigMailboxes(CAN_REG_HNDL reg_ptr, CONFIG_SCHEME cfg, U32 node_info);
__irq static void SysCanIntGlobalIsr(void);
__irq static void SysCanIntReceiveIsr(void);
__irq static void SysCanIntTransmitIsr(void);
__irq static void SysCanIntErrorIsr(void);
static void SysCanIntNotifyError(CAN_CHANNEL channel, CAN_ERROR error);
static void SysCanIntReceiveMssg(CAN_CHANNEL channel, U8 mailbox_nr);
static void SysCanIntGetNextTransmitMssgAndSendViaMailbox(CAN_CHANNEL channel, U8 mailbox_nr);
static void SysCanIntTransmitMssg(CAN_CHANNEL channel, U8 mailbox_nr, CAN_MSSG_STRUCT* tx_mssg_ptr);
static BOOL SysCanIntGetFirstFreeMailboxNr(CAN_CHANNEL channel, U8* mailbox_nr_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const CAN_REG_HNDL           syscan_registers[CAN_CHANNEL_COUNT] = {(CAN_REG_HNDL)CAN_BASE};
static CAN_CTRL_STRUCT              syscan_ctrl_struct[SYSCANINT_COUNT];
#if (CORELOG_LEVEL & LOG_LEVEL_ERROR) != 0
static const STRING                 syscan_illegal_channel_string = "CAN illegal channel - ";
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL SysCanIntInitPheripheral(CAN_REG_HNDL reg_ptr, CAN_CONFIG_STRUCT* config_struct_ptr)
{
    reg_ptr->TSR |= 0x00808080;
    reg_ptr->MCR |= 0x00000001;     //Enter Initialisation mode
    reg_ptr->MCR &= ~0x00000002;    //Exit Sleep mode
    
    while(!(reg_ptr->MSR & 0x00000001))
    {
    }
    if(!SysCanIntInitPeripheralSpeed(reg_ptr, config_struct_ptr->speed))
    {
        return FALSE;
    }

    //Automatic Bus-Off Management
    reg_ptr->MCR |= 0x00000040;
    //Transmit FIFO Priority: Chronologically
    reg_ptr->MCR |= 0x00000004;

    //Enable FIFO mssg pending & overrun interrupt
    reg_ptr->IER |= 0x0000000A;     //FIFO 0
    //Enable FIFO mssg pending & overrun interrupt
    reg_ptr->IER |= 0x00000050;     //FIFO 1
    //Enable status & error interrupts
    reg_ptr->IER |= 0x00008F00;     //ERRIE, LECIE, BOFIE, EPVIE, EWGIE
    //Enable transmit mailbox empty interrupt
    reg_ptr->IER |= 0x00000001;     //TMEIE

    reg_ptr->MCR &= ~0x00000003;    //Exit Sleep mode & Enter Normal mode
    while(reg_ptr->MSR & 0x00000001)
    {
    }

    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL SysCanIntInitPeripheralSpeed(CAN_REG_HNDL reg_ptr, CAN_SPEED speed)
{
    U32 spd;
    U32 request_spd;
    U16 brp;
    U32 limit_high;
    U32 limit_low;
    U32 tseg;
    U32 tseg1;
    U32 tseg2;
    U32 temp;

    switch(speed)
    {
    case CAN_SPEED_1_Mb:
        request_spd = 1000000;
        break;
    case CAN_SPEED_500_Kb:
        request_spd = 500000;
        break;
    case CAN_SPEED_250_Kb:
        request_spd = 250000;
        break;
    case CAN_SPEED_125_Kb:
        request_spd = 125000;
        break;
    default:
        LOG_ERR("CAN invalid speed : %d", PU8(speed));
        return FALSE;
    }

    spd = SysGetAPBClk();
    brp = 0;
    while(brp < 1024)
    {
        for (tseg = 3; tseg < 20; tseg++)
        {
            temp = spd / ((brp + 1) * (3 + tseg));
            if((spd % ((brp + 1) * (3 + tseg)) == 0) && (temp == request_spd))// found an exact divider
            {
                //BRP
                reg_ptr->BTR &= ~0x000003FF;
                reg_ptr->BTR |= brp;
                tseg1 = (tseg * 85) / 100;    //picanol specifies 85%
                if(((tseg * 85) % 100) > 50)
                {
					tseg1++; //round up
				}
                tseg2 = tseg - tseg1;
                //TS1
                reg_ptr->BTR &= ~0x000F0000;
                reg_ptr->BTR |= (tseg1 << 16);
                //TS2
                reg_ptr->BTR &= ~0x00700000;
                reg_ptr->BTR |= (tseg2 << 20);
                //SJW
                reg_ptr->BTR &= ~0x01000000;
                reg_ptr->BTR |= 0x01000000;
                goto LABEL_SPEED_CONFIG_OK;
            }
            else
            {
                if(temp < request_spd)
                {
                    break;
                }
            }

        }
        brp++;
    }

    //did not find an exact divider, so find closest thing
    brp = 0;
    limit_high = request_spd + 10000;
    limit_low = request_spd - 10000;
    while(brp < 1024)
    {
        for (tseg = 3; tseg < 20; tseg++)
        {
            temp = spd / ((brp + 1)* (3 + tseg));

            if((temp < limit_high) && (temp > limit_low))// found an exact divider
            {
                //BRP
                reg_ptr->BTR &= ~0x000003FF;
                reg_ptr->BTR |= brp;
				tseg1 = (tseg * 85) / 100;    //picanol specifies 85%
				if(((tseg * 85) % 100) > 50)
				{
					tseg1++; //round up
				}
                tseg2 = tseg - tseg1;
                //TS1
                reg_ptr->BTR &= ~0x000F0000;
                reg_ptr->BTR |= (tseg1 << 16);
                //TS2
                reg_ptr->BTR &= ~0x00700000;
                reg_ptr->BTR |= (tseg2 << 20);
                //SJW
                reg_ptr->BTR &= ~0x03000000;
                reg_ptr->BTR |= 0x03000000;
                goto LABEL_SPEED_CONFIG_OK;
            }
            else
            {
                if(temp < request_spd)
                {
                    break;
                }
            }

        }
        brp++;
    }

    LOG_ERR("CAN failed to setup speed = %d bps", PU32(request_spd));
    return FALSE;

LABEL_SPEED_CONFIG_OK:
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL SysCanIntRecoverFromBusOff(CAN_REG_HNDL reg_ptr)
{
    reg_ptr->MCR |= 0x00000001;     //Enter Initialisation mode
    while(!(reg_ptr->MSR & 0x00000001))
    {
    }
    reg_ptr->MCR &= ~0x00000001;    //Exit Initialisation mode & Enter Normal mode
    while(reg_ptr->MSR & 0x00000001)
    {
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL SysCanIntSetAcceptanceFilter(CAN_REG_HNDL      reg_ptr,
                                         ACCEPTANCE_FILTER filter,
                                         U32               mask,            // which bits are important
                                         U32               match,           // specifies wether these these important bits must be 0 or 1
                                         IDENTIFIER_TYPE   identifier_type, // what's the expected identifier type
                                         FRAME_TYPE        frame_type)      // data or remote frame
{
    volatile U32* match_ptr = (U32*)&(reg_ptr->sFilterRegister[filter].FR1);
    volatile U32* mask_ptr  = (U32*)&(reg_ptr->sFilterRegister[filter].FR2);
    
    reg_ptr->FMR |= 0x00000001;  //enter initialisation mode for filters
    reg_ptr->FM1R = 0x00000000;  //CAN filter mode register. We will use ALWAYS identifier MASK mode for all filters

    if(((U8)filter) > 13)  //only 0...13 filters
    {
        LOG_WRN("Invalid acceptance  filter (%d)", PU8(filter));
        return FALSE;
    }
    
    reg_ptr->FS1R |= (0x00000001 << filter); //single 32-bit scale configuration
    
    if(identifier_type == STANDARD)
    {
        *match_ptr = (match & 0x000007FF) << 21;
        *mask_ptr = (mask & 0x000007FF) << 21;
    }
    else
    {
        *match_ptr = 0x04 /* EXTENDED */ | ((match & 0x1FFFFFFF) << 3);
        *mask_ptr = (mask & 0x1FFFFFFF) << 3;
    }
    if(frame_type == REMOTE_REQUEST_FRAME)
    {
        *match_ptr |= 0x02; /* REMOTE REQ */
    }
    
    *mask_ptr |= 0x06; //ALWAYS check frame & identifier type

    reg_ptr->FMR &= ~0x00000001; //exit initialisation mode for filters

    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL SysCanIntAddFilterToRxMailbox(CAN_REG_HNDL      reg_ptr,
                                          RX_MAILBOX        mailbox,
                                          ACCEPTANCE_FILTER filter)
{
    if(((U8)filter) > 13)  //only 0...13 filters
    {
        LOG_WRN("Invalid acceptance  filter (%d)", PU8(filter));
        return FALSE;
    }
    
    reg_ptr->FMR |= 0x00000001;  //Enter Initialisation mode for filters

    if(RX_MAILBOX_0)
    {
        reg_ptr->FFA1R &= ~(0x00000001 << (U8)filter);    //RX FIFO 0
    }        
    else
    {
        reg_ptr->FFA1R |= 0x00000001 << (U8)filter;       //RX FIFO 1
    }
    reg_ptr->FA1R |= 0x00000001 << (U8)filter;            //enable filter

    reg_ptr->FMR &= ~0x00000001;  //Exit Initialisation mode for filters        
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL SysCanIntConfigMailboxes(CAN_REG_HNDL reg_ptr, CONFIG_SCHEME cfg, U32 node_info)
{
    U32 mask;
    U32 match;

    if(cfg == PCMS_TX_RX_BALANCED) //node_info must contain only it's node-bit and subnode bits
    {
        //broadcast messages : 
        // mask needs to check 
        //       + the devices NODE bit
        //       + the broadcast bit
        //     subnode bits are treated as don't care
        // match
        //       + the devices NODE bit must be 1
        //       + broadcast bit must be 1
        //       == equal to the mask
        mask = STDPCMS_BROADCAST_BIT;
        mask |= (node_info & ~STDPCMS_SUBNODE_MASK); //filter out the subnode bits
        match = mask; //       == equal to the mask
        SysCanIntSetAcceptanceFilter(reg_ptr, ACCEPTANCE_FILTER_0, mask, match, EXTENDED, DATA_FRAME);
        SysCanIntAddFilterToRxMailbox(reg_ptr, RX_MAILBOX_0, ACCEPTANCE_FILTER_0);
    
        //non broadcast messages : 
        // mask needs to check 
        //       + the devices NODE bit
        //       + all SUBNODE bits
        //       + the broadcast bit
        // match
        //       + the devices NODE bit must be 1
        //       + the SUBNODE bits must match with the devices subnode bits (0/1's)
        //       + broadcast bit must be 0
        mask = node_info | STDPCMS_SUBNODE_MASK | STDPCMS_BROADCAST_BIT;
        match = node_info & ~STDPCMS_BROADCAST_BIT; //be sure the broadcast is 0
        SysCanIntSetAcceptanceFilter(reg_ptr, ACCEPTANCE_FILTER_1, mask, match, EXTENDED, DATA_FRAME);
        SysCanIntAddFilterToRxMailbox(reg_ptr, RX_MAILBOX_1, ACCEPTANCE_FILTER_1);
        
        return TRUE;
    }
    else if (cfg == RECEIVE_ALL)
    {
        // Mailbox 0: allow all standard messages
        mask = 0x0;
        match = 0x0;
        SysCanIntSetAcceptanceFilter(reg_ptr, ACCEPTANCE_FILTER_0, mask, match, STANDARD, DATA_FRAME);
        SysCanIntAddFilterToRxMailbox(reg_ptr, RX_MAILBOX_0, ACCEPTANCE_FILTER_0);


        // Mailbox 1: allow all extended messages
        mask = 0x0;
        match = 0x0;
        SysCanIntSetAcceptanceFilter(reg_ptr, ACCEPTANCE_FILTER_1, mask, match, EXTENDED, DATA_FRAME);
        SysCanIntAddFilterToRxMailbox(reg_ptr, RX_MAILBOX_1, ACCEPTANCE_FILTER_1);
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
__irq static void SysCanIntGlobalIsr(void)
{
  SysCanIntReceiveIsr();
  SysCanIntTransmitIsr();
  SysCanIntErrorIsr();
}
//------------------------------------------------------------------------------------------------//
__irq static void SysCanIntReceiveIsr(void)
{
    CAN_REG_HNDL reg_ptr = syscan_registers[CAN_CHANNEL_1];
    
    if(reg_ptr->RF0R & 0x00000010)        /*** overrun error for mailbox 0 ***/
    {
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_RX_MSSG_LOST);
        reg_ptr->RF0R |= 0x00000010;
    }
    if(reg_ptr->RF0R & 0x00000003)        /*** something arrived for mailbox 0 ***/
    {
        SysCanIntReceiveMssg(CAN_CHANNEL_1, 0);
        reg_ptr->RF0R |= 0x00000020;
    }
    if(reg_ptr->RF1R & 0x00000010)        /*** overrun error for mailbox 1 ***/
    {
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_RX_MSSG_LOST);
        reg_ptr->RF1R |= 0x00000010;
    }
    if(reg_ptr->RF1R & 0x00000003)        /*** something arrived for mailbox 1 ***/
    {
        SysCanIntReceiveMssg(CAN_CHANNEL_1, 1);
        reg_ptr->RF1R |= 0x00000020;
    }
}
//------------------------------------------------------------------------------------------------//
__irq static void SysCanIntTransmitIsr(void)
{
    //no checks on channel: is ISR
    CAN_REG_HNDL     reg_ptr = syscan_registers[CAN_CHANNEL_1];
    
    if(reg_ptr->TSR & 0x00010000) //mailbox 2
    {
        SysCanIntGetNextTransmitMssgAndSendViaMailbox(CAN_CHANNEL_1, 2);
        reg_ptr->TSR |= 0x00010000; //clear
    }
    if(reg_ptr->TSR & 0x00000100) //mailbox 1
    {
        SysCanIntGetNextTransmitMssgAndSendViaMailbox(CAN_CHANNEL_1, 1);
        reg_ptr->TSR |= 0x00000100; //clear
    }
    if(reg_ptr->TSR & 0x00000001) //mailbox 0
    {
        SysCanIntGetNextTransmitMssgAndSendViaMailbox(CAN_CHANNEL_1, 0);
        reg_ptr->TSR |= 0x00000001; //clear
    }
}
//------------------------------------------------------------------------------------------------//
__irq static void SysCanIntErrorIsr(void)
{
    CAN_REG_HNDL reg_ptr = syscan_registers[CAN_CHANNEL_1];
    U16 error_flags;
    
    error_flags = (U16)reg_ptr->ESR;
    
    switch((error_flags & 0x00000070) >> 4)
    {
    case 1:
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_STUFF_BIT_ERROR);
        break;
    case 2:
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_FORM_ERROR);
        break;
    case 3:
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_ACKNOWLEDGE_ERROR);
        break;
    case 4:
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_BIT_ERROR);
        break;
    case 5:
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_BIT_ERROR);
        break;
    case 6:
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_CRC_ERROR);
        break;
    }

    if(error_flags & 0x00000004)  /*** bus off ***/
    {
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_BUS_OFF);
    }
    if(error_flags & 0x00000002)  /*** error passive ***/
    {
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_PASSIVE_STATE);
    }
    if(error_flags & 0x00000001)  /*** warning level ***/
    {
        SysCanIntNotifyError(CAN_CHANNEL_1, CAN_ERR_WARNING_STATE);
    }
    reg_ptr->MSR |= 0x04; /* ERRI : clear interrupt*/
}
//------------------------------------------------------------------------------------------------//
static void SysCanIntNotifyError(CAN_CHANNEL channel, CAN_ERROR error)
{
    //no checks on channel: gets called from ISR
    CAN_CTRL_STRUCT* ctrl_struct_ptr = &syscan_ctrl_struct[channel];
    
    SysCan_LogError(channel, error);
    if(ctrl_struct_ptr->error_hook != NULL) 
    {
        syscan_ctrl_struct[channel].error_hook(error);
    }
}
//------------------------------------------------------------------------------------------------//
static void SysCanIntReceiveMssg(CAN_CHANNEL channel, U8 mailbox_nr)
{
    //no checks on channel: gets called from ISR
    CAN_MSSG_STRUCT           rx_mssg;
    CAN_FIFOMailBox_TypeDef*  mailbox_ptr;
    CAN_CTRL_STRUCT*          ctrl_struct_ptr = &syscan_ctrl_struct[channel];
    CAN_REG_HNDL              reg_ptr         = syscan_registers[channel];

    if(ctrl_struct_ptr->rx_new_message_hook != NULL) 
    {
        if(mailbox_nr == 0)
        {
            mailbox_ptr = &reg_ptr->sFIFOMailBox[0];
        }
        else
        {
            mailbox_ptr = &reg_ptr->sFIFOMailBox[1];
        }
    
        if(mailbox_ptr->RIR & 0x0002)
        {
            rx_mssg.frame_type = REMOTE_REQUEST_FRAME;
        }
        else
        {
            rx_mssg.frame_type = DATA_FRAME;
        }
        
        rx_mssg.identifier  = mailbox_ptr->RIR >> 3;
        rx_mssg.dlc         = mailbox_ptr->RDTR & 0x0F;
        
        rx_mssg.data[0]     = mailbox_ptr->RDLR & 0x000000FF;
        rx_mssg.data[1]     = (mailbox_ptr->RDLR & 0x0000FF00) >> 8;
        rx_mssg.data[2]     = (mailbox_ptr->RDLR & 0x00FF0000) >> 16;
        rx_mssg.data[3]     = (mailbox_ptr->RDLR & 0xFF000000) >> 24;
        rx_mssg.data[4]     = mailbox_ptr->RDHR & 0x000000FF;
        rx_mssg.data[5]     = (mailbox_ptr->RDHR & 0x0000FF00) >> 8;
        rx_mssg.data[6]     = (mailbox_ptr->RDHR & 0x00FF0000) >> 16;
        rx_mssg.data[7]     = (mailbox_ptr->RDHR & 0xFF000000) >> 24;
    
        if(mailbox_ptr->RIR & 0x0004)
        {
            rx_mssg.identifier_type = EXTENDED;
        }
        else
        {
            rx_mssg.identifier_type = STANDARD;
            rx_mssg.identifier >>= 18;
        }
        
        ctrl_struct_ptr->rx_new_message_hook(&rx_mssg);
    }
    else
    {
        LOG_WRN("CAN mssg received, but no rx_new_message_hook for channel %d", PU8(channel));
    }
}
//------------------------------------------------------------------------------------------------//
static void SysCanIntGetNextTransmitMssgAndSendViaMailbox(CAN_CHANNEL channel, U8 mailbox_nr)
{
    //no checks on channel: gets called from ISR
    CAN_CTRL_STRUCT* ctrl_struct_ptr = &syscan_ctrl_struct[channel];
    CAN_MSSG_STRUCT  mssg;
    
    if(ctrl_struct_ptr->tx_get_next_message_hook != NULL)
    {
        if(ctrl_struct_ptr->tx_get_next_message_hook(&mssg))
        {
            SysCanIntTransmitMssg(channel, mailbox_nr, &mssg);
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void SysCanIntTransmitMssg(CAN_CHANNEL channel, U8 mailbox_nr, CAN_MSSG_STRUCT* tx_mssg_ptr)
{
    //no checks on channel: gets called from module internal code
    CAN_TxMailBox_TypeDef* mailbox_ptr = &(syscan_registers[channel]->sTxMailBox[mailbox_nr]);
    
    if(tx_mssg_ptr->identifier_type == EXTENDED)
    {
        mailbox_ptr->TIR = (tx_mssg_ptr->identifier) << 3;
        mailbox_ptr->TIR |= 0x00000004; //IDE
    }
    else
    {
        mailbox_ptr->TIR = (tx_mssg_ptr->identifier) << 21;
        //mailbox_ptr->TIR &= ~0x00000004;
    }
    
    if(tx_mssg_ptr->frame_type == REMOTE_REQUEST_FRAME)
    {
        mailbox_ptr->TIR |= 0x00000002; //RTR
    }
    else
    {
        //mailbox_ptr->TIR &= ~0x00000002;
        mailbox_ptr->TDTR = tx_mssg_ptr->dlc & 0xF;
    }

    mailbox_ptr->TDLR  = tx_mssg_ptr->data[0]
                      | (tx_mssg_ptr->data[1] << 8)
                      | (tx_mssg_ptr->data[2] << 16)
                      | (tx_mssg_ptr->data[3] << 24);
    mailbox_ptr->TDHR  = tx_mssg_ptr->data[4]
                      | (tx_mssg_ptr->data[5] << 8)
                      | (tx_mssg_ptr->data[6] << 16) 
                      | (tx_mssg_ptr->data[7] << 24);

    mailbox_ptr->TIR |= 0x00000001;
}
//------------------------------------------------------------------------------------------------//
static BOOL SysCanIntGetFirstFreeMailboxNr(CAN_CHANNEL channel, U8* mailbox_nr_ptr)
{
    //no checks on channel: gets called from module internal code
    CAN_REG_HNDL reg_ptr = syscan_registers[channel];
    
    if(reg_ptr->TSR & 0x04000000) /*** mailbox 0 empty ***/
    {
        *mailbox_nr_ptr = 0;
        return TRUE;
    }
    if(reg_ptr->TSR & 0x08000000) /*** mailbox 1 empty ***/
    {
        *mailbox_nr_ptr = 1;
        return TRUE;
    }
    if(reg_ptr->TSR & 0x10000000) /*** mailbox 2 empty ***/
    {
        *mailbox_nr_ptr = 2;
        return TRUE;
    }
    return FALSE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void SysCanInt_Init(void)
{
    MEMSET((VPTR)syscan_ctrl_struct, 0, SIZEOF(syscan_ctrl_struct));
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_Init(CAN_CHANNEL channel)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        //Enable CAN reset state
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN, ENABLE);
        //Release CAN from reset state
        RCC_APB1PeriphResetCmd(RCC_APB1Periph_CAN, DISABLE);
        //Clock
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN, ENABLE);
        //register interrupts
        SysInt_RegisterIsr(CEC_CAN_IRQn, (EVENT_CALLBACK)SysCanIntGlobalIsr); //Global ISR
        SysInt_EnableIsr(CEC_CAN_IRQn);

        syscan_ctrl_struct[channel].init_done = TRUE;
        return TRUE;
    }
    
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_Config(CAN_CHANNEL channel, CAN_CONFIG_STRUCT* config_struct_ptr)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        if(syscan_ctrl_struct[channel].init_done == TRUE)
        {
            return SysCanIntInitPheripheral(syscan_registers[channel], config_struct_ptr);
        }
        LOG_WRN("CAN channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_ConfigMailboxes(CAN_CHANNEL channel, CONFIG_SCHEME cfg, U32 node_info)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        if(syscan_ctrl_struct[channel].init_done == TRUE)
        {
            return SysCanIntConfigMailboxes(syscan_registers[channel], cfg, node_info);
        }
        LOG_WRN("CAN channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_RegisterRxHook(CAN_CHANNEL channel, CAN_RX_NEW_MSSG_HOOK hook)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        syscan_ctrl_struct[channel].rx_new_message_hook = hook;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_RegisterTxHook(CAN_CHANNEL channel, CAN_TX_GET_NEXT_MSSG_HOOK hook)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        syscan_ctrl_struct[channel].tx_get_next_message_hook = hook;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_RegisterErrorHook(CAN_CHANNEL channel, CAN_ERROR_HOOK hook)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        syscan_ctrl_struct[channel].error_hook = hook;
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_NotifyTxMessageReady(CAN_CHANNEL channel)
{
    U8 mailbox_nr = 255;
    
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        if(SysCanIntGetFirstFreeMailboxNr(channel, &mailbox_nr))
        {
            //free mailbox found: put it in there
            SysCanIntGetNextTransmitMssgAndSendViaMailbox(channel, mailbox_nr);
        }
        //else : no free mailbox found, the ISR will fetch it from the notifier
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_RecoverFromBusOff(CAN_CHANNEL channel)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        if(syscan_ctrl_struct[channel].init_done == TRUE)
        {
            return SysCanIntRecoverFromBusOff(syscan_registers[channel]);
        }
        LOG_WRN("CAN channel not initialised - %d", PU8(channel));
        return FALSE;
    }
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_SetAcceptanceFilter(CAN_CHANNEL       channel,
                                   ACCEPTANCE_FILTER filter,
                                   U32               mask,            // which bits are important
                                   U32               match,           // specifies wether these these important bits must be 0 or 1
                                   IDENTIFIER_TYPE   identifier_type, // what's the expected identifier type
                                   FRAME_TYPE        frame_type)      // data or remote frame
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        return SysCanIntSetAcceptanceFilter(syscan_registers[CAN_CHANNEL_1],
                                            filter,
                                            mask,
                                            match,
                                            identifier_type,
                                            frame_type);
    }
    
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_AddFilterToRxMailbox(CAN_CHANNEL       channel,
                                    RX_MAILBOX        mailbox,
                                    ACCEPTANCE_FILTER filter)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        return SysCanIntAddFilterToRxMailbox(syscan_registers[CAN_CHANNEL_1], mailbox, filter);
    }
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_RegisterBitTimeAnalyseHook(CAN_CHANNEL channel, CAN_BIT_TIME_ANALYSE_HOOK hook)
{
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL SysCanInt_Channel_ConfigBitTimeAnalyser(CAN_CHANNEL channel, CAN_TIME_ANALYSE_MODE mode, BOOL capture_all)
{
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
U8 SysCanInt_GetRec(CAN_CHANNEL channel)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        return (syscan_registers[CAN_CHANNEL_1]->ESR >> 24) & 0xFF;
    }
    
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return 0;
}
//------------------------------------------------------------------------------------------------//
U8 SysCanInt_GetTec(CAN_CHANNEL channel)
{
    if(channel == CAN_CHANNEL_1) //only one channel
    {
        return (syscan_registers[CAN_CHANNEL_1]->ESR >> 16) & 0xFF;
    }
    
    LOG_ERR("%s%d", PCSTR(syscan_illegal_channel_string), PU8(channel));
    return 0;
}
//================================================================================================//
