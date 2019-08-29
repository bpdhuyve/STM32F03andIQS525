//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// DMX Protocol module
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef COMM_DLL_DMX_H
#define COMM_DLL_DMX_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// @brief  Defines the total data slots, 512 is max, but sometimes you will want less because otherwise it takes to much ram
#ifndef COMMDLLDMX_MAX_DATA_SLOTS
    #define COMMDLLDMX_MAX_DATA_SLOTS 512
#endif
//------------------------------------------------------------------------------------------------//
typedef enum
{
	TX_PIN_MODE_GPIO_OUT_LOW   ,
	TX_PIN_MODE_UART_TX        ,             //pin needs to be high in idle state! (when no uart communication is being made)
}
TX_PIN_MODE;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
/// @remark prototype of the event that will be called if the tx pin needs to change role
typedef void (*EVENT_CHANGE_TX_PIN_MODE)(TX_PIN_MODE mode);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
/// @brief  Init of the module, must be done before use
/// @param  "event_change_tx_pin_mode" here you MUST register an event handler that changes the IO mode of the uart tx pin between gpio low output and uart tx output
void CommDllDmx_Init(SCI_CHANNEL_HNDL rs485_channel,
                     DRVGPIO_PIN_HNDL rs485_dirpin,
                     EVENT_CHANGE_TX_PIN_MODE event_change_tx_pin_mode);
//------------------------------------------------------------------------------------------------//
/// @brief  calling this funcion will immediately starts sending dmx data
/// @param  "data_ptr" Data will be copied from this pointer to internal buffer inside this module, the fist dmx start code byte MUST NOT be included
/// @param  "length"   Length cannot be longer then the defined COMMDLLDMX_MAX_DATA_SLOTS
/// @return	returns TRUE is sending is started, returns FALSE if something was wrong
/// @remark function is non-blocking, so the sending will still be busy after this call, if you call this function again while a previous transmission is stil active, the call will be ignored
BOOL CommDllDmx_SendDMXdata(U8* data_ptr,
                            U8 length);
//================================================================================================//



#endif /* COMM_DLL_DMX_H */
