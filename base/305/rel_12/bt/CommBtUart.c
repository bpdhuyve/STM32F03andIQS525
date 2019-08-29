//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// The purpose of the module is the communication between the PCM (power control module) and any
// version the DM (display module): LED, LED+ and LCD. The communication is performed over a single
// wire, where the DM acts as master and the PCM as slave.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COMM_BT_UARTBT_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COMM_BT_UARTBT_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COMM_BT_UARTBT_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  defines the maximum data length that can be handled.
#ifndef MAX_FRAME_LENGTH
    #define MAX_FRAME_LENGTH            50
#endif
//------------------------------------------------------------------------------------------------//
// @brief  defines the output q size
#ifndef OUT_Q_SIZE
    #define OUT_Q_SIZE                  200
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
#include "gpio\DrvGpio.h"
#include "gpio\DrvGpioSys.h"
#include "sci\DrvSciChannel.h"
#include "sci\DrvSciChannelSysInt.h"

//Comm include section
#include "bt\CommBtUart.h"

#define Q_PREFIX(postfix)                   BT_TX_Q_##postfix
#define Q_SIZE                              2
#include "sci\DrvSciQTxTpl.h"

#define Q_PREFIX(postfix)                   BT_RX_Q_##postfix
#define Q_SIZE                              100
#include "sci\DrvSciQRxTpl.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DEVICE_RX_STRING_LENGTH     MAX_FRAME_LENGTH
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void ProcessRxData(void);
static BOOL SendCommand(STRING command);
static BOOL SendCommand_U8Argument(STRING command, U8 arg1);
static BOOL SendCommand_StringArgument(STRING command, STRING arg1);
static void TermCommand_BtDeleteBondInfo(void);
static void BtUart_OutTask(VPTR data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static STRING                               device_tx_string;
static STRING                               convert_string;
static STRING                               device_rx_string;
static STRING                               device_rx_char;
static STRING                               bt_rx_string;

static SCI_CHANNEL_HNDL                     uart_hndl;
static SCI_CONFIG_STRUCT                    uart_config;

static EVENT_PASSKEY_RECEIVED               event_passkey_received = NULL;
static EVENT_CALLBACK                       event_device_state_changed = NULL;
static EVENT_CALLBACK                       event_bt_data_received = NULL;
static EVENT_SWVERSION_RECEIVED             event_swversion_received = NULL;
static EVENT_CHIPID_RECEIVED                event_chipid_received = NULL;

static DEVICE_STATE                         bt_device_state = STATE_UNKOWN;

static Q_HNDL                               btuart_out_q;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void ProcessRxData(void)
{
    //process tags ([....]}
    if (CoreString_Contains(device_rx_string,"]") && CoreString_Contains(device_rx_string,"["))
    {
        U8 tag_data_index;
        tag_data_index = CoreString_Search(device_rx_string,"]") + 2;

        if (CoreString_Contains(device_rx_string,"[BT KEY]"))
        {
            CoreString_SubString(device_rx_string,convert_string,tag_data_index,6);

            U32 key;
            key = CoreConvert_StringToU32(convert_string);

            CALL_HOOK(event_passkey_received)(key);
        }
        else if (CoreString_Contains(device_rx_string,"[BT STATE]"))
        {
            STRING state_string = &device_rx_string[tag_data_index];
            DEVICE_STATE received_device_state ;

            if (CoreString_Contains(state_string,"STANDBY"))
            {
                received_device_state = STATE_STANDBY;
            }
            else if (CoreString_Contains(state_string,"ADV_CON_IND"))
            {
                received_device_state = STATE_ADVERTISING_TO_ALL;
            }
            else if (CoreString_Contains(state_string,"ADV_WHITELIST") || CoreString_Contains(state_string,"ADV_NONCON")) //ADV_NONCON is a state that the device comes in when it wants to go to state adv_whitelist but has no devices in its whitelist, in the swipestats these states are treated the same because there is no difference for the swipestat app
            {
                received_device_state = STATE_ADVERTISING_TO_WHITELIST;
            }
            else if (CoreString_Contains(state_string,"CONNECTED_NO_ENCRYPTION"))
            {
                received_device_state = STATE_CONNECTED_NO_ENCRYPTION;
            }
            else if (CoreString_Contains(state_string,"CONNECTED_ENCRYPTION"))
            {
                received_device_state = STATE_CONNECTED_ENCRYPTION;
            }
            else
            {
                received_device_state = STATE_UNKOWN;
            }

            //call changed hook if state is changed
            if (received_device_state != bt_device_state)
            {
                bt_device_state = received_device_state; //before you call the changed hook change the state incase in this hook they ask the state using CommBtUart_GetDeviceState
                CALL_HOOK(event_device_state_changed)();
            }
        }
        else if (CoreString_Contains(device_rx_string,"[BT RX]"))
        {
            STRING partial_bt_rx_string = &device_rx_string[tag_data_index];
            U8 bt_sting_terminator[2] = {4,0}; //EOT char

            CoreString_Concat(bt_rx_string,partial_bt_rx_string);   //add data to rbt rx buffer

            //check if this is the end of the data
            if (CoreString_Contains(partial_bt_rx_string,(STRING)bt_sting_terminator)||(CoreString_GetLength(partial_bt_rx_string) < 20))      //<20 is not really needed but this allows me to simulate the app with the nordic uart sending app (unable to send this char)
            {
                CoreString_TrimEnd(bt_rx_string);   //remove EOT char
                CALL_HOOK(event_bt_data_received)();
                CoreString_Clear(bt_rx_string,1);   //clear buffer , dont know if this is te correct place to do this but normally it will be processed in the hook that i just called
            }
        }
    }
    else if (CoreString_StartsWith(device_rx_string,"id: ")) //unique bt chip id
    {
        if (!CoreString_Contains(device_rx_string,"0x"))
        {
            CoreString_Insert(device_rx_string,"0x",4); //inset hex indicator to make response the same as lpc chip id
        }

        CALL_HOOK(event_chipid_received)(device_rx_string);

    }
    else if (CoreString_StartsWith(device_rx_string,"1")) //sw version start altijd met "1" bv "15017.0.0.1"
    {
        CALL_HOOK(event_swversion_received)(device_rx_string);
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL SendCommand(STRING command)
{
    CoreString_Clear(device_tx_string,1);

    CoreString_Concat(device_tx_string,command);
    CoreString_Concat(device_tx_string,"\r");

    return CoreQ_Write(btuart_out_q, (U8*)device_tx_string,CoreString_GetLength(device_tx_string));
}
//------------------------------------------------------------------------------------------------//
static BOOL SendCommand_U8Argument(STRING command, U8 arg1)
{
    CoreString_Clear(device_tx_string,1);
    CoreString_Clear(convert_string,1);

    CoreString_Concat(device_tx_string,command);
    CoreString_Concat(device_tx_string," ");

    CoreConvert_U8ToDecimalString((U8)arg1,convert_string,FALSE);
    CoreString_Concat(device_tx_string,convert_string);

    CoreString_Concat(device_tx_string,"\r");

    return CoreQ_Write(btuart_out_q, (U8*)device_tx_string,CoreString_GetLength(device_tx_string));
}
//------------------------------------------------------------------------------------------------//
static BOOL SendCommand_StringArgument(STRING command, STRING arg1)
{
    CoreString_Clear(device_tx_string,1);
    CoreString_Clear(convert_string,1);

    CoreString_Concat(device_tx_string,command);
    CoreString_Concat(device_tx_string," ");
    CoreString_Concat(device_tx_string,arg1);

    CoreString_Concat(device_tx_string,"\r");

    return CoreQ_Write(btuart_out_q, (U8*)device_tx_string,CoreString_GetLength(device_tx_string));
}
//------------------------------------------------------------------------------------------------//
static void TermCommand_BtDeleteBondInfo(void)
{
    SendCommand("BtDeleteBondInfo");
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void BtUart_OutTask(VPTR data_ptr)
{
    U8  byte;
    
    if((BT_TX_Q_SciQTx_GetSpace() > 1) && (CoreQ_Read(btuart_out_q, &byte, 1) == TRUE))
    {
        BT_TX_Q_SciQTx_Write(&byte, 1);
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommBtUart_Init(SCI_CHANNEL_HNDL uart_hndl)
{
    MODULE_INIT_ONCE();

    //allocate static strings
    device_tx_string = CoreBuffer_NewString(200,"CommBtUart device_tx_string");
    device_rx_string = CoreBuffer_NewString(DEVICE_RX_STRING_LENGTH,"CommBtUart device_rx_string"); //bt rx string 1 packet can max be 20 bytes + de bt commands, het passkey cmd is langer
    device_rx_char = CoreBuffer_NewString(1,"CommBtUart device_rx_char");
    convert_string = CoreBuffer_NewString(10,"CommBtUart convert_string");
    bt_rx_string = CoreBuffer_NewString(100,"CommBtUart bt_rx_string"); //maximum bt rx data string (defined by the maximum data send from over the bt to the swipestat)

    uart_config.data_length = SCI_DATA_LENGTH_8_BITS;
    uart_config.parity = SCI_PARITY_NONE;
    uart_config.speed = SCI_SPEED_19200_BPS;
    uart_config.stopbit = SCI_STOPBIT_1;

    DrvSciChannel_Config(uart_hndl,&uart_config);

    btuart_out_q = CoreQ_Register(OUT_Q_SIZE, SIZEOF(U8), "BtUart Out");
    BT_TX_Q_SciQTx_Create(uart_hndl);//CREATE THE BT TX Q
    CoreTask_Start(CoreTask_RegisterTask(1000, BtUart_OutTask, NULL, 127, "BtUart Out"));
    BT_RX_Q_SciQRx_Create(uart_hndl);//CREATE THE BT RX Q

    //SendCommand("Reset"); //reset the bt processor before starting this caused problems in the tll niveau and shit!

    //debug commands
    CoreTerm_RegisterCommand("BtDeleteBondInfo", "clears the whitelist of the bt device, this forces devices to re-enter the passkey", 0, TermCommand_BtDeleteBondInfo, TRUE);

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommBtUart_Handler(void)
{
    while(BT_RX_Q_SciQRx_Read((U8*)device_rx_char, 1))
    {
        if (CoreString_GetLength(device_rx_string) >= DEVICE_RX_STRING_LENGTH)
        {
            LOG_WRN("Receiving to much data in a burst from Bt device, clearing buffer");
            CoreString_Clear(device_rx_string,1);
            return;
        }

        CoreString_Concat(device_rx_string,device_rx_char);//add all chars received from rx que to one long string and check if it contains a \r

        if (CoreString_Contains(device_rx_string,"\r"))
        {
            //trim enkel spaces en /r (we need the EOT char later)
            CoreString_TrimChar(device_rx_string,'\r');    //remove newline
            CoreString_TrimChar(device_rx_string,' ');     //leading and trailing spaces

            if (CoreString_GetLength(device_rx_string) != 0)
            {
                LOG_DBG("rx from bt chip: \"%c\"", PDSTR(device_rx_string));

                ProcessRxData();
            }

            //line is processed - clear rx buffer
            CoreString_Clear(device_rx_string,1);
        }
     }
}
//------------------------------------------------------------------------------------------------//
BOOL CommBtUart_SetDeviceState(DEVICE_STATE state)
{
    if ((state != STATE_STANDBY) && (state != STATE_ADVERTISING_TO_ALL) && (state != STATE_ADVERTISING_TO_WHITELIST)) //only this states can be
    {
        return FALSE;
    }
    SendCommand_U8Argument("BtSetDeviceState",(U8)state);
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
DEVICE_STATE CommBtUart_GetDeviceState(void)
{
    return bt_device_state;
}
//------------------------------------------------------------------------------------------------//
void CommBtUart_RequestDevicePassKey(void)
{
    SendCommand("BtGetPassKey");
}
//------------------------------------------------------------------------------------------------//
void CommBtUart_RegisterEvent_PassKeyReceived(EVENT_PASSKEY_RECEIVED event)
{
    event_passkey_received = event;
}
//------------------------------------------------------------------------------------------------//
void CommBtUart_SetDeviceName(STRING name)
{
    SendCommand_StringArgument("BtSetDeviceName",name);
}
//------------------------------------------------------------------------------------------------//
void CommBtUart_RegisterEvent_DeviceStateChanged(EVENT_CALLBACK event)
{
    event_device_state_changed = event;
}
//------------------------------------------------------------------------------------------------//
BOOL CommBtUart_SendData(STRING data)
{
    //if (bt_device_state == STATE_CONNECTED_ENCRYPTION)
    {
        return SendCommand_StringArgument("BtTx",data);
    }
    //return FALSE;
}
//------------------------------------------------------------------------------------------------//
void CommBtUart_RegisterEvent_DataReceived(EVENT_CALLBACK event)
{
    event_bt_data_received = event;
}
//------------------------------------------------------------------------------------------------//
STRING CommBtUart_GetReceivedData(void)
{
    return bt_rx_string;    //maybe this string needs to be copyed ?
}
//------------------------------------------------------------------------------------------------//
//async request the sw version, the answer will be given trough the EVENT_SWVERSION_RECEIVED event
void CommBtUart_RequestSwVersion(void)
{
    SendCommand("SwVersion");
}
//------------------------------------------------------------------------------------------------//
//register event to notify if the sw version is received from the bt chip
void CommBtUart_RegisterEvent_SwVersionReceived(EVENT_SWVERSION_RECEIVED event)
{
    event_swversion_received = event;
}
//------------------------------------------------------------------------------------------------//
//async request the unique processor id, the answer will be given trough the EVENT_CHIPID_RECEIVED event
void CommBtUart_RequestChipId(void)
{
    SendCommand("ChipId");
}
//------------------------------------------------------------------------------------------------//
//register event to notify if the sw version is received from the bt chip
void CommBtUart_RegisterEvent_ChipIdReceived(EVENT_CHIPID_RECEIVED event)
{
    event_chipid_received = event;
}
//------------------------------------------------------------------------------------------------//
BOOL CommBtUart_SetDeviceMITM(BOOL mitm_on)
{
    if(mitm_on)
    {
        SendCommand("BtSetMitmProtection 1");
    }
    else
    {
        SendCommand("BtSetMitmProtection 0");      
    }
    return TRUE;
}
//================================================================================================//
