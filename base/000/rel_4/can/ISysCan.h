//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the CAN system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CAN__ISYSCAN_H
#define CAN__ISYSCAN_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    CAN_SPEED_125_Kb,
    CAN_SPEED_250_Kb,
    CAN_SPEED_500_Kb,
    CAN_SPEED_1_Mb
}
CAN_SPEED;

typedef enum
{
    STANDARD   = 0,
    EXTENDED   = 1
}
IDENTIFIER_TYPE;

typedef enum
{
    DATA_FRAME   = 0,
    REMOTE_REQUEST_FRAME   = 1
}
FRAME_TYPE;

// @brief   CAN mssg object structure
typedef struct
{
    // 29-bit or 11-bit identifier
    U32 identifier;

    // data length : DLC = number of valid bytes in the next "data[8]" array
    U8 dlc;

    // up to 8 data bytes
    U8 data[8];

    // identifier type
	IDENTIFIER_TYPE identifier_type;
	
    // frame type
    FRAME_TYPE frame_type;
}
CAN_MSSG_STRUCT;

// @brief   enumeration with CAN error codes
typedef enum
{
    CAN_ERR_CRC_ERROR,
    CAN_ERR_ACKNOWLEDGE_ERROR,
    CAN_ERR_FORM_ERROR,
    CAN_ERR_BIT_ERROR,
    CAN_ERR_STUFF_BIT_ERROR,

    CAN_ERR_BUS_OFF,
    CAN_ERR_PASSIVE_STATE,
    CAN_ERR_WARNING_STATE,

    CAN_ERR_RX_MSSG_LOST,
    
    CAN_ERR_LIST_LENGTH_ERROR,
    CAN_ERR_LIST_OBJECT_ERROR
}
CAN_ERROR;

// @remark   CAN channel config struct
typedef struct CHANNEL_CONFIG
{
	CAN_SPEED speed;
}
CAN_CONFIG_STRUCT;

typedef enum
{
    CAN_ANALYSE_MODE_0,
    CAN_ANALYSE_MODE_1,
    CAN_ANALYSE_MODE_2,
    CAN_ANALYSE_MODE_3,
    CAN_ANALYSE_MODE_4,
    CAN_ANALYSE_MODE_5,
    
    CAN_ANALYSE_MODE_OFF
}
CAN_TIME_ANALYSE_MODE;

typedef enum
{
    PCMS_TX_RX_BALANCED
}
CONFIG_SCHEME;

// Hook which must handle the given RX mssg
typedef void (*CAN_RX_NEW_MSSG_HOOK)(CAN_MSSG_STRUCT* mssg_ptr);

// Hook which can pass a mssg to be transmitted
typedef BOOL (*CAN_TX_GET_NEXT_MSSG_HOOK)(CAN_MSSG_STRUCT* mssg_ptr);

// Hook which must handle a CAN error
// Note: if bus-off : use 
typedef void (*CAN_ERROR_HOOK)(CAN_ERROR error);

// Hook which handles Bit Time Analyser
typedef void (*CAN_BIT_TIME_ANALYSE_HOOK)(CAN_TIME_ANALYSE_MODE mode, U32 identifier, U16 value, BOOL message_done);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* CAN__ISYSCAN_H */
