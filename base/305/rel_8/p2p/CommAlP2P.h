//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// The application layer of the P2P UART communication protocol
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef P2P__COMMALP2P_H
#define P2P__COMMALP2P_H
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
typedef U8                          P2P_SERVICE;

// @brief  Enumeration of possible response states
typedef enum
{
    RESPONSE_STATE_MESSAGE_SENT         = 0x00,     // message was sent successfully, no response was expected
    RESPONSE_STATE_RESPONSE_RECEIVED    = 0x01,     // message was sent successfully and response was received
    RESPONSE_STATE_ACKOWLEDGE_TO        = 0x02,     // request has timed out, no acknowledge received
    RESPONSE_STATE_RESPONSE_TO          = 0x03,     // request has timed out, no response received
    RESPONSE_STATE_SERVICE_NA           = 0x04,     // service not available
}
RESPONSE_STATE;

// @brief  Enumeration of possible decode states of the service handling
typedef enum
{
    DECODESTATE_IDLE                = 0,
    DECODESTATE_REQUESTED           = 1,
    DECODESTATE_PARSING             = 2,
    DECODESTATE_RESPONSEREADY       = 3,
}
COMMALP2P_DECODESTATE;

// @brief  prototype of the function to be called upon reception of a request
typedef BOOL (*COMMALP2P_REQUESTHANDLER)(U8* data_ptr, U8 data_len);

// @brief  prototype of the function to be called to get the response of a handled request
typedef COMMALP2P_DECODESTATE (*COMMALP2P_RESPONSEHANDLER)(U8* data_ptr, U8* data_len_ptr);

// @brief  prototype of the function to be called upon reception of the response
typedef void (*COMMALP2P_RESPONSECALLBACK)(RESPONSE_STATE response_state, U8* data_ptr, U8 data_len);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
extern const STRING  p2presponsestatenames[];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommAlP2P_Init(void);

// @remark  Background handler
void CommAlP2P_Handler(void);

// @remark  function to register a service handler to handle the received requests
// @return  returns TRUE if registering succeeded.
BOOL CommAlP2P_RegisterServiceHandler(P2P_SERVICE service, COMMALP2P_REQUESTHANDLER request_handler, COMMALP2P_RESPONSEHANDLER response_handler);

// @remark  function to send a service request to the other side
// @remark  if wait_for_answer is set to FALSE, no response is expected, only acknowledge of sending
// @return  returns TRUE if sending succeeded.
BOOL CommAlP2P_SendServiceRequest(P2P_SERVICE service, U8* data_ptr, U8 data_len, BOOL wait_for_answer, COMMALP2P_RESPONSECALLBACK response_callback);

// @remark  function to check if peer processor is present
// @remark  this check is done based on the reception of a valid message is the last 5 seconds
// @return  returns TRUE if peer was detected.
BOOL CommAlP2P_IsPeerPresent(void);
//================================================================================================//



#endif /* P2P__COMMALP2P_H */

