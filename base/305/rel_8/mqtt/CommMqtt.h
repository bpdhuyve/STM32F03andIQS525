//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the MQTT application layer
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef COMMMQTT_H
#define COMMMQTT_H
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
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
void CommMqtt_Init(void);

// @remark  Background handler
void CommMqtt_Handler(void);

BOOL CommMqtt_Connect(const U8* ip, U16 port, const CHAR* client_id, const CHAR* username, const CHAR* password);

BOOL CommMqtt_Disconnect(void);

void CommMqtt_SetKeepalive(U16 keepalive);

U16 CommMqtt_Publish(const CHAR* topic, const CHAR* content, U32 content_len, U8 qos, BOOL retain);

BOOL CommMqtt_Connected();

BOOL CommMqtt_Disconnected();

U16 CommMqtt_Completed();
//================================================================================================//



#endif /* COMMMQTT_H */
