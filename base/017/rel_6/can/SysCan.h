//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// CAN peripheral interface
//
// Processor       : specific
// Implementation  : independant
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CAN__SYSCAN_H
#define CAN__SYSCAN_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "can\ISysCan.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   enumeration for the available receive mailboxes
typedef enum
{
    RX_MAILBOX_0 = 0,
    RX_MAILBOX_1 = 1
}
RX_MAILBOX;

// @brief   enumeration for the available TX types
typedef enum
{
    TX_ONCE,                 //as dataframe
    TX_ONCE_AS_RR            //as remote request
    //TX_ONLY_AS_RR_REPLY,     //prepare MB as reply for a remote request
    //TX_ONCE_AND_AS_RR_REPLY  //send once as dataframe and prepare MB as reply for further remote requests
}
TX_TYPE;

// @brief   enumeration for the available direct mailboxes
typedef enum
{
    DIRECT_TX_MAILB_0 = 2
}
DIRECT_TX_MAILBOX;

// @brief   enumeration for the available acceptance mask registers
typedef enum
{
    ACCEPTANCE_FILTER_0,
    ACCEPTANCE_FILTER_1,
    ACCEPTANCE_FILTER_2,
    ACCEPTANCE_FILTER_3,
    ACCEPTANCE_FILTER_4,
    ACCEPTANCE_FILTER_5,
    ACCEPTANCE_FILTER_6,
    ACCEPTANCE_FILTER_7,
    ACCEPTANCE_FILTER_8,
    ACCEPTANCE_FILTER_9,
    ACCEPTANCE_FILTER_10,
    ACCEPTANCE_FILTER_11,
    ACCEPTANCE_FILTER_12,
    ACCEPTANCE_FILTER_13
}
ACCEPTANCE_FILTER;

// @brief   acceptance filtering setup struct
//typedef struct ACCEPTANCE_FILTER_CONFIG
//{
//	U32             id_mask;    // which bits are important
//	U32             id_match;   // specifies wether these these important bits must be 0 or 1
//	IDENTIFIER_TYPE id_type;    // what's the expected identifier type
//  FRAME_TYPE      frame_type; // data or remote frame
//}
//ACCEPTANCE_CONFIG;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//

//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void SysCan_LogError(CAN_CHANNEL channel, CAN_ERROR error);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* CAN__SYSCAN_H */
