//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Data Link Layer of the OpenTherm communication
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef OPENTHERM__COMMDLLOPENTHERM_H
#define OPENTHERM__COMMDLLOPENTHERM_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "gpio/DrvGpio.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INVALID_OPENTHERM_CHANNEL           255

typedef enum
{
    MSG_TYPE_M2S_READ_DATA          = 0,
    MSG_TYPE_M2S_WRITE_DATA         = 1,
    MSG_TYPE_M2S_INVALID_DATA       = 2,
    //MSG_TYPE_M2S_RESERVED         = 3,
    
    MSG_TYPE_S2M_READ_ACK           = 4,
    MSG_TYPE_S2M_WRITE_ACK          = 5,
    MSG_TYPE_S2M_DATA_INVALID       = 6,
    MSG_TYPE_S2M_UNKNOWN_DATAID     = 7,
}
MSG_TYPE;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef U8              OPENTHERM_CHANNEL;

typedef struct
{
    MSG_TYPE    msg_type;
    U8          data_id;
    U16         data_value;
}
OPENTHERM_MSG;

// @brief   Prototype of the frame handling function
typedef void (*DLLOPENTHERM_RECV_FRAME_HOOK)(OPENTHERM_MSG opentherm_msg);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommDllOpenTherm_Init(void);

// @remark  Function to register an OpenTherm channel
OPENTHERM_CHANNEL CommDllOpenTherm_RegisterChannel(DRVGPIO_PIN_HNDL rx_pin_hndl, DRVGPIO_PIN_HNDL tx_pin_hndl);

// @remark  Function to register the function to be called upon reception of a frame on an OpenTherm channel
BOOL CommDllOpenTherm_RegisterFrameHook(OPENTHERM_CHANNEL opentherm_channel, DLLOPENTHERM_RECV_FRAME_HOOK frame_hook);

// @remark  Function to send a frame on an OpenTherm channel
BOOL CommDllOpenTherm_SendFrame(OPENTHERM_CHANNEL opentherm_channel, OPENTHERM_MSG opentherm_msg);

// @remark  Function to set the level when idle (to allow higher power modes)
BOOL CommDllOpenTherm_SetIdleLevel(OPENTHERM_CHANNEL opentherm_channel, BOOL set_idle_level_high);
//================================================================================================//



#endif /* OPENTHERM__COMMDLLOPENTHERM_H */

