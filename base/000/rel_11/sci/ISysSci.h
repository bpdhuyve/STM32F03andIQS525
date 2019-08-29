//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Processor and implementation independent prototypes and definitions for the SCI system interface.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SCI__ISYSSCI_H
#define SCI__ISYSSCI_H
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
    SCI_SPEED_4800_BPS          = 4800,
    SCI_SPEED_9600_BPS          = 9600,
    SCI_SPEED_19200_BPS         = 19200,
    SCI_SPEED_38400_BPS         = 38400,
    SCI_SPEED_57600_BPS         = 57600,
    SCI_SPEED_115200_BPS        = 115200,
    SCI_SPEED_128000_BPS        = 128000,
    SCI_SPEED_230400_BPS        = 230400,
    SCI_SPEED_250000_BPS        = 250000,
    SCI_SPEED_460800_BPS        = 460800,
    SCI_SPEED_921600_BPS        = 921600,
    SCI_SPEED_1497600_BPS       = 1497600,
    SCI_SPEED_1843200_BPS       = 1843200,
    SCI_SPEED_3686400_BPS       = 3686400
}
SCI_SPEED;

typedef enum
{
    SCI_PARITY_NONE,
	SCI_PARITY_EVEN,
	SCI_PARITY_ODD,
	SCI_PARITY_MARK,
	SCI_PARITY_SPACE
}
SCI_PARITY;

typedef enum
{
    SCI_STOPBIT_05,
    SCI_STOPBIT_1,
    SCI_STOPBIT_15,
    SCI_STOPBIT_2
}
SCI_STOPBIT;

typedef enum
{
    SCI_DATA_LENGTH_5_BITS,
    SCI_DATA_LENGTH_6_BITS,
    SCI_DATA_LENGTH_7_BITS,
    SCI_DATA_LENGTH_8_BITS,
    SCI_DATA_LENGTH_9_BITS
}
SCI_DATA_LENGTH;

typedef struct
{
    SCI_SPEED               speed;
    SCI_PARITY              parity;
    SCI_STOPBIT             stopbit;
    SCI_DATA_LENGTH         data_length;
}
SCI_CONFIG_STRUCT;

typedef void (*SCI_RX_NEW_BYTE_HOOK)(U8* byte_ptr, U8 length);

/// prototype for the TxDataNeeded event
/// @param "data_ptr" pointer to a piece of memory where the event much copy the data, how much data is specified by the length argument
/// @param "length" the length of the data that the driver wants to fetch, this is the datacount that must be copyed into the data_ptr
/// @return the number of data bytes that are available in the event, return 0 if you dont have anymore data to send
typedef U8 (*SCI_TX_GET_NEXT_BYTE_HOOK)(U8* data_ptr, U8 length);

typedef void (*SCI_MSG_COMPLETE)(SCI_CHANNEL channel);
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



#endif /* SCI__ISYSSCI_H */
