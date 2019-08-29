//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Dmx Protocoll driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COMM_DLL_DMX_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"

#ifndef DMX_LOG_LEVEL
	#define CORELOG_LEVEL                                   LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL                                   SWIPECORE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
//queue
#define Q_PREFIX(postfix)                                   Dmx_##postfix
#ifndef Q_SIZE
    #define Q_SIZE                                          10
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "dmx\CommDllDmx.h"

//SYSTEM include section
#include "gpio\SysPin.h"

//DRIVER include section
#include "sci\DrvSciQTxTpl.h"

//STANDARD include section
//APPLICATION include section
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
    DMX_LINE_STATE_IDLE,
	DMX_LINE_STATE_BREAK,
	DMX_LINE_STATE_MAB  ,
    DMX_LINE_STATE_DATA ,
}
DMX_LINE_STATE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void task_lineStateTimer(VPTR data_ptr);
static U8 EventHandler_GetNextTxBytes(U8* data_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SCI_CHANNEL_HNDL                 channel_hndl;
static TASK_HNDL                        task_hndl_linestatetimer;
static SCI_CONFIG_STRUCT                testconfigrs;
static U8                               txdata[COMMDLLDMX_MAX_DATA_SLOTS+1];
static U16                              txdata_counter;
static DMX_LINE_STATE                   line_state = DMX_LINE_STATE_IDLE;
static EVENT_CHANGE_TX_PIN_MODE         eventhook_changeTxPinMode = NULL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//task routine
static void task_lineStateTimer(VPTR data_ptr)
{
    switch (line_state)
    {
        case DMX_LINE_STATE_BREAK://vorige state
            line_state = DMX_LINE_STATE_MAB;//next send 12ms MAB (line high)
            eventhook_changeTxPinMode(TX_PIN_MODE_UART_TX);//config pin back as uart (line is in idle state HIGH)
            CoreTask_SetPeriod(task_hndl_linestatetimer,20);//min 12 dus 20µs is goe
            CoreTask_ReStart(task_hndl_linestatetimer);
            break;

        case DMX_LINE_STATE_MAB:
            line_state = DMX_LINE_STATE_DATA;//next, send data,

            txdata_counter=0; //init counter to zero, data bytes will be transfered in EventHandler_GetNextTxBytes
            DrvSciChannel_NotifyTxDataReady(channel_hndl);
            CoreTask_Stop(task_hndl_linestatetimer); //stop yourself
            break;

        default:
            LOG_WRN("should never happen");
    }
}
//------------------------------------------------------------------------------------------------//
static U8 EventHandler_GetNextTxBytes(U8* data_ptr, U8 length)
{
    U8 datalength_to_send = length;
    if (txdata_counter + length > COMMDLLDMX_MAX_DATA_SLOTS+1)
    {
        datalength_to_send = (COMMDLLDMX_MAX_DATA_SLOTS+1) - txdata_counter;   //cap length
    }
    MEMCPY(data_ptr,&(txdata[txdata_counter]),datalength_to_send);

    txdata_counter+= datalength_to_send;

    if (datalength_to_send == 0)
    {
        line_state = DMX_LINE_STATE_IDLE;   //sending is complete
    }
    return datalength_to_send;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommDllDmx_Init(SCI_CHANNEL_HNDL rs485_channel,
                     DRVGPIO_PIN_HNDL rs485_dirpin,
                     EVENT_CHANGE_TX_PIN_MODE event_change_tx_pin_mode)
{
    eventhook_changeTxPinMode = event_change_tx_pin_mode;

    MEMSET(txdata,0,COMMDLLDMX_MAX_DATA_SLOTS+1);  //firstye is 0 = start code for dmx512

    channel_hndl = rs485_channel;

    DrvGpio_SetPin(rs485_dirpin, TRUE);//dmx master is always sending

    testconfigrs.data_length = SCI_DATA_LENGTH_8_BITS;
    testconfigrs.speed       = SCI_SPEED_250000_BPS;
    testconfigrs.parity      = SCI_PARITY_NONE;
    testconfigrs.stopbit     = SCI_STOPBIT_2;

    DrvSciChannel_Config(channel_hndl,&testconfigrs);
    DrvSciChannel_RegisterTxHook(channel_hndl, EventHandler_GetNextTxBytes);

    task_hndl_linestatetimer = CoreTask_RegisterTask(1e6, task_lineStateTimer, NULL, 1, "task_lineStateTimer");
}
//------------------------------------------------------------------------------------------------//
BOOL CommDllDmx_SendDMXdata(U8 *data_ptr, U8 length)
{
    if (line_state != DMX_LINE_STATE_IDLE)
    {
        LOG_WRN("trying to send dmx data while previous dmx transfer is not completed");
        return FALSE;
    }

    if (length > COMMDLLDMX_MAX_DATA_SLOTS)
    {
        LOG_WRN("to much data, can not fit in allocated dmx data slots");
        return FALSE;
    }

    MEMCPY(&(txdata[1]),data_ptr,length);

    //start break, config uart tx pin as gpio low to send break
    line_state = DMX_LINE_STATE_BREAK;
    eventhook_changeTxPinMode(TX_PIN_MODE_GPIO_OUT_LOW);

    CoreTask_SetPeriod(task_hndl_linestatetimer,100);//min 92µs dus 100 is goe
    CoreTask_Start(task_hndl_linestatetimer);  //timer task will do the rest
    return TRUE;
}
//================================================================================================//
