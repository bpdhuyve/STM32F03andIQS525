//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Data Link Layer of the OpenTherm communication
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define OPENTHERM__COMMDLLOPENTHERM_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef OPENTHERM__COMMDLLOPENTHERM_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               OPENTHERM__COMMDLLOPENTHERM_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef OPENTHERM_CHANNEL_COUNT
    #define OPENTHERM_CHANNEL_COUNT                 1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef OPENTHERM_TIMER
    #error "OPENTHERM_TIMER not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SIGNAL_LOW_LEVEL
    #define SIGNAL_LOW_LEVEL                        TRUE
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// SYS
#include "timer/SysTimer.h"

// DRV

// STD

// COM
#include "opentherm/CommDllOpenTherm.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef union
{
    struct
    {
        U16         data_value;
        U8          data_id;
        U8          reserved:4;
        MSG_TYPE    msg_type:3;
        BOOL        parity:1;
    }
    msg;
    U32     data;
}
MSG_STRUCT;

typedef enum
{
    TX_STATE_IDLE           = 0,
    TX_STATE_START_1        = 1,
    TX_STATE_START_2        = 2,
    TX_STATE_DATA_1         = 3,
    TX_STATE_DATA_2         = 4,
    TX_STATE_STOP_1         = 5,
    TX_STATE_STOP_2         = 6,
}
TX_STATE;

typedef enum
{
    RX_STATE_IDLE           = 0,
    RX_STATE_DATA           = 1,
    RX_STATE_STOP           = 2,
}
RX_STATE;

typedef struct
{
    DRVGPIO_PIN_HNDL                rx_pin_hndl;
    DRVGPIO_PIN_HNDL                tx_pin_hndl;
    DLLOPENTHERM_RECV_FRAME_HOOK    recv_frame_hook;
    BOOL                            idle_level;
    
    TX_STATE                        tx_state;
    U8                              tx_bit;
    MSG_STRUCT                      tx_msg;
    
    RX_STATE                        rx_state;
    U8                              rx_bit;
    BOOL                            rx_prev_level;
    U8                              rx_timer_count;
    MSG_STRUCT                      rx_msg;
}
OPENTHERM_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DllOpenTherm_SetParity(MSG_STRUCT* msg_ptr);
static BOOL DllOpenTherm_CheckParity(MSG_STRUCT* msg_ptr);

static void DllOpenTherm_HandleRx(OPENTHERM_STRUCT* ot_channel_ptr);
static void DllOpenTherm_HandleTx(OPENTHERM_STRUCT* ot_channel_ptr);
static void DllOpenTherm_RxIsr(TIMER t);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_SendFrame(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static U8                           dllopentherm_channel_count;
static OPENTHERM_STRUCT             dllopentherm_struct[OPENTHERM_CHANNEL_COUNT];

static U8                           dllopentherm_isr_count;

const STRING msg_type_names[] = {"M2S_READ_DATA", "M2S_WRITE_DATA","M2S_INVALID_DATA", "",
                                 "S2M_READ_ACK", "S2M_WRITE_ACK", "S2M_DATA_INVALID", "S2M_UNKNOWN_DATAID"};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DllOpenTherm_SetParity(MSG_STRUCT* msg_ptr)
{
    U32 data        = msg_ptr->data;
    U8  bitcount    = 0;
    
    while(data)
    {
        bitcount += (data & 0x0001);
        data >>= 1;
    }
    
    msg_ptr->msg.parity = (BOOL)(bitcount & 0x01);
}
//------------------------------------------------------------------------------------------------//
static BOOL DllOpenTherm_CheckParity(MSG_STRUCT* msg_ptr)
{
    U32 data        = msg_ptr->data;
    U8  bitcount    = 0;
    
    while(data)
    {
        bitcount += (data & 0x0001);
        data >>= 1;
    }
    
    return (BOOL)((bitcount & 0x01) == 0);
}
//------------------------------------------------------------------------------------------------//
static void DllOpenTherm_HandleRx(OPENTHERM_STRUCT* ot_channel_ptr)
{
    BOOL                rx_level;
    S8                  rx_timer_delta;
    OPENTHERM_MSG       rx_msg;
    
    // read RX pin
    rx_level = DrvGpio_GetPin(ot_channel_ptr->rx_pin_hndl);
    
    // RX timer delta
    rx_timer_delta = (S8)(dllopentherm_isr_count - ot_channel_ptr->rx_timer_count);
    
    // check for level change
    if(rx_level != ot_channel_ptr->rx_prev_level)
    {
        switch(ot_channel_ptr->rx_state)
        {
        case RX_STATE_IDLE:
            // check for start condition
            if(rx_level == SIGNAL_LOW_LEVEL)
            {
                ot_channel_ptr->rx_timer_count = dllopentherm_isr_count;
                ot_channel_ptr->rx_msg.data = 0;
                ot_channel_ptr->rx_bit = 0;
                ot_channel_ptr->rx_state = RX_STATE_DATA;
            }
            break;
            
        case RX_STATE_DATA:
            // ignore level changes that come too soon
            if(rx_timer_delta > 6)
            {
                ot_channel_ptr->rx_timer_count = dllopentherm_isr_count;
                if(rx_level == SIGNAL_LOW_LEVEL)
                {
                    ot_channel_ptr->rx_msg.data |= 1 << (31 - ot_channel_ptr->rx_bit);
                }
                if(++(ot_channel_ptr->rx_bit) >= 32)
                {
                    ot_channel_ptr->rx_state = RX_STATE_STOP;
                }
            }
            break;
            
        case RX_STATE_STOP:
            // ignore level changes that come too soon or have wrong polarity
            if((rx_timer_delta > 6) && (rx_level == SIGNAL_LOW_LEVEL))
            {
                ot_channel_ptr->rx_state = RX_STATE_IDLE;
                
                if(DllOpenTherm_CheckParity(&ot_channel_ptr->rx_msg))
                {
                    rx_msg.msg_type     = ot_channel_ptr->rx_msg.msg.msg_type;
                    rx_msg.data_id      = ot_channel_ptr->rx_msg.msg.data_id;
                    rx_msg.data_value   = ot_channel_ptr->rx_msg.msg.data_value;
                    
                    LOG_DBG("OT RCV on %d : %-18s id %3d val 0x%04x", PU8(ot_channel_ptr - dllopentherm_struct), PCSTR(msg_type_names[rx_msg.msg_type]), PU8(rx_msg.data_id), PU16(rx_msg.data_value));
                    
                    if(ot_channel_ptr->recv_frame_hook != NULL)
                    {
                        ot_channel_ptr->recv_frame_hook(rx_msg);
                    }
                }
                else
                {
                    LOG_DBG("failed 0x%04x", PU32(ot_channel_ptr->rx_msg.data));
                }
            }
            break;
        }
    }
    // otherwise check for timeout
    else if((ot_channel_ptr->rx_state != RX_STATE_IDLE) && (rx_timer_delta >= 14))
    {
        LOG_DBG("timeout (recv %d bits)", PU8(ot_channel_ptr->rx_bit));
        ot_channel_ptr->rx_state = RX_STATE_IDLE;
    }
            
    ot_channel_ptr->rx_prev_level = rx_level;
}
//------------------------------------------------------------------------------------------------//
static void DllOpenTherm_HandleTx(OPENTHERM_STRUCT* ot_channel_ptr)
{
    // set TX output
    switch(ot_channel_ptr->tx_state)
    {
    case TX_STATE_START_1:
    case TX_STATE_STOP_1:
        DrvGpio_SetPin(ot_channel_ptr->tx_pin_hndl, (BOOL)(FALSE == SIGNAL_LOW_LEVEL));
        break;
    case TX_STATE_DATA_1:
        DrvGpio_SetPin(ot_channel_ptr->tx_pin_hndl, (BOOL)((BOOL)((ot_channel_ptr->tx_msg.data & (0x80000000 >> ot_channel_ptr->tx_bit)) == 0) == SIGNAL_LOW_LEVEL));
        break;
    case TX_STATE_START_2:
    case TX_STATE_DATA_2:
    case TX_STATE_STOP_2:
        DrvGpio_TogglePin(ot_channel_ptr->tx_pin_hndl);
        break;
    case TX_STATE_IDLE:
    default:
        DrvGpio_SetPin(ot_channel_ptr->tx_pin_hndl, ot_channel_ptr->idle_level);
        break;
    }
    
    // change state
    switch(ot_channel_ptr->tx_state)
    {
    case TX_STATE_IDLE:
        // do nothing
        break;
    case TX_STATE_STOP_2:
        ot_channel_ptr->tx_state = TX_STATE_IDLE;
        break;
    case TX_STATE_DATA_2:
        if(++(ot_channel_ptr->tx_bit) < 32)
        {
            ot_channel_ptr->tx_state = TX_STATE_DATA_1;
            break;
        }
        // no break
    default:
        ot_channel_ptr->tx_state++;
        break;
    }
}
//------------------------------------------------------------------------------------------------//
// @remark  Background handler
static void DllOpenTherm_RxIsr(TIMER t)
{
    static U8           tx_count = 0;
    OPENTHERM_STRUCT*   ot_channel_ptr;
    
    MODULE_CHECK();
    
    // update counters
    dllopentherm_isr_count++;
    if(++tx_count >= 5)
    {
        tx_count = 0;
    }
    
    // loop all OpenTherm channels
    for(ot_channel_ptr = dllopentherm_struct; ot_channel_ptr < &dllopentherm_struct[dllopentherm_channel_count]; ot_channel_ptr++)
    {
        DllOpenTherm_HandleRx(ot_channel_ptr);
        
        if(tx_count == 0)
        {
            DllOpenTherm_HandleTx(ot_channel_ptr);
        }
    }
    
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_SendFrame(void)
{
    OPENTHERM_MSG opentherm_msg;
    
    opentherm_msg.msg_type      = (MSG_TYPE)CoreTerm_GetArgumentAsU32(1);
    opentherm_msg.data_id       = (U8)CoreTerm_GetArgumentAsU32(2);
    opentherm_msg.data_value    = (U16)CoreTerm_GetArgumentAsU32(3);
    
    CoreTerm_PrintFeedback(CommDllOpenTherm_SendFrame((OPENTHERM_CHANNEL)CoreTerm_GetArgumentAsU32(0), opentherm_msg));
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommDllOpenTherm_Init(void)
{
    MODULE_INIT_ONCE();
    
    dllopentherm_channel_count = 0;
    MEMSET((VPTR)dllopentherm_struct, 0, SIZEOF(dllopentherm_struct));
    
    dllopentherm_isr_count = 0;
    
    SysTimer_Init();
    SysTimer_Timer_Init(OPENTHERM_TIMER, 1000000);
    SysTimer_Timer_SetPeriod(OPENTHERM_TIMER, 100);     // 100µs
    SysTimer_Timer_RegisterPeriodInterrupt(OPENTHERM_TIMER, DllOpenTherm_RxIsr);
    SysTimer_Timer_EnablePeriodicInterrupt(OPENTHERM_TIMER);
    SysTimer_Timer_Start(OPENTHERM_TIMER);
    
    CoreTerm_RegisterCommand("OTSend", "OPENTHERM send frame", 4, Command_SendFrame, TRUE);
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to register an OpenTherm channel
OPENTHERM_CHANNEL CommDllOpenTherm_RegisterChannel(DRVGPIO_PIN_HNDL rx_pin_hndl, DRVGPIO_PIN_HNDL tx_pin_hndl)
{
    OPENTHERM_STRUCT*   ot_channel_ptr = &dllopentherm_struct[dllopentherm_channel_count];
    
    MODULE_CHECK();
    
    if(dllopentherm_channel_count < OPENTHERM_CHANNEL_COUNT)
    {
        ot_channel_ptr->rx_pin_hndl = rx_pin_hndl;
        ot_channel_ptr->tx_pin_hndl = tx_pin_hndl;
        ot_channel_ptr->idle_level  = SIGNAL_LOW_LEVEL;
        
        dllopentherm_channel_count++;
        return (OPENTHERM_CHANNEL)(dllopentherm_channel_count - 1);
    }
    
    return INVALID_OPENTHERM_CHANNEL;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to register the function to be called upon reception of a frame on an OpenTherm channel
BOOL CommDllOpenTherm_RegisterFrameHook(OPENTHERM_CHANNEL opentherm_channel, DLLOPENTHERM_RECV_FRAME_HOOK frame_hook)
{
    OPENTHERM_STRUCT*   ot_channel_ptr = &dllopentherm_struct[opentherm_channel];
    
    if(opentherm_channel >= dllopentherm_channel_count)
    {
        return FALSE;
    }
    
    ot_channel_ptr->recv_frame_hook = frame_hook;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to send a frame on an OpenTherm channel
BOOL CommDllOpenTherm_SendFrame(OPENTHERM_CHANNEL opentherm_channel, OPENTHERM_MSG opentherm_msg)
{
    OPENTHERM_STRUCT*   ot_channel_ptr = &dllopentherm_struct[opentherm_channel];
    
    if(opentherm_channel >= dllopentherm_channel_count)
    {
        return FALSE;
    }
    
    if(ot_channel_ptr->tx_state != TX_STATE_IDLE)
    {
        return FALSE;
    }
    
    ot_channel_ptr->tx_msg.data = 0;
    ot_channel_ptr->tx_msg.msg.msg_type     = opentherm_msg.msg_type;
    ot_channel_ptr->tx_msg.msg.data_id      = opentherm_msg.data_id;
    ot_channel_ptr->tx_msg.msg.data_value   = opentherm_msg.data_value;
    DllOpenTherm_SetParity(&ot_channel_ptr->tx_msg);
    
    ot_channel_ptr->tx_bit   = 0;
    ot_channel_ptr->tx_state = TX_STATE_START_1;
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @remark  Function to set the level when idle (to allow higher power modes)
BOOL CommDllOpenTherm_SetIdleLevel(OPENTHERM_CHANNEL opentherm_channel, BOOL set_idle_level_high)
{
    OPENTHERM_STRUCT*   ot_channel_ptr = &dllopentherm_struct[opentherm_channel];
    
    if(opentherm_channel >= dllopentherm_channel_count)
    {
        return FALSE;
    }
    
    ot_channel_ptr->idle_level = (BOOL)(set_idle_level_high != SIGNAL_LOW_LEVEL);
    return TRUE;
}    
//================================================================================================//
