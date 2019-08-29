//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Header file for the Network Layer for the PsiRf-protocol.
// The main function of this layer is to transmit frames of characters between master and salve equipment. The
// layer serves as a communication medium to the application layer.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef PSIRF__COMMNLPSIRF_H
#define PSIRF__COMMNLPSIRF_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max number of nodes in the network
#ifndef STDNLPSIRF_MAX_NODES
    #error "STDNLPSIRF_MAX_NODES not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the protocol version to be used
#ifndef STDNLPSIRF_PROTOCOL_VERSION
    #error "STDNLPSIRF_PROTOCOL_VERSION not defined in AppConfig.h"
#else
    #if STDNLPSIRF_PROTOCOL_VERSION == 0
        #define STDNLPSIRF_HEADER_LENGTH            10
    #else
        #error "STDNLPSIRF_PROTOCOL_VERSION defined in AppConfig.h is unkown"
    #endif
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "psirf\CommDllPsiRf.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   Prototype of the data structure that contains the data to be saved
typedef struct
{
    U8  network_address;
    U32 network_hid;
    U32 device_hid;
    U8  routing_table[STDNLPSIRF_MAX_NODES][2];
}
NLPSIRF_DATA;

// @brief   Prototype of the frame handling function
typedef void (*NLPSIRFHOOK_FRAME_RECEIVED)(U8 sender, U8 dest, U8* frame_ptr, U8 len);


// @brief   Prototype of the link status handling function
typedef void (*NLPSIRFHOOK_LINK_STATUS)(BOOL link_state);

// @brief   Prototype of the set nid handling function
typedef void (*NLPSIRFHOOK_SET_NID)(U8* data_ptr, U8 data_len);

// @brief   Prototype of the get nid handling function
typedef BOOL (*NLPSIRFHOOK_ADD_REMOVE_NODE)(U8 sender,
                                               BOOL add_node,
                                               U8* data_ptr,        //warning: when "add_node" is true the first data byte is the nid of the module you want to include, you have to specify this even if you dont have application data comming after it
                                               U8* data_len_ptr);

// @brief   Prototype of msg feedback function
typedef void (*NLPSIRFHOOK_MSG_FEEDBACK)(U8 msg_id, BOOL success);

// @brief   Prototype of save data function
typedef void (*NLPSIRFHOOK_SAVE_DATA)(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialiser for the DataLink Layer entity
void CommNlPsiRf_Init(NLPSIRF_DATA* data_ptr);

//this handler will only be called when application messages are received
void CommNlPsiRf_RegisterFrameHook(NLPSIRFHOOK_FRAME_RECEIVED hook);
void CommNlPsiRf_RegisterLinkStatusHook(NLPSIRFHOOK_LINK_STATUS hook);
//this hook is called when this module gets a new network id (inclusion in network) or when its network id is set to 0 (exclusion of network)
void CommNlPsiRf_RegisterSetNidHook(NLPSIRFHOOK_SET_NID hook);
void CommNlPsiRf_RegisterAddRemoveNodeHook(NLPSIRFHOOK_ADD_REMOVE_NODE hook);
void CommNlPsiRf_RegisterMsgFeedbackHook(NLPSIRFHOOK_MSG_FEEDBACK hook);
void CommNlPsiRf_RegisterSaveDataHook(NLPSIRFHOOK_SAVE_DATA hook);

// @brief   Function to be called in the background loop
// This function will be called in background. It will call be executed when a complete new message is received.
// The frame handling function will be called and the message will be interpreted.
void CommNlPsiRf_Handler(void);

//sets the master in linking mode, ready to give out a message on incomming explore messages
U8 CommNlPsiRf_SetLink(U8 state);

//sends a application message, when tnid is 0 this message is broadcasted
U8 CommNlPsiRf_SendApplic(U8 tnid, U8 len, U8* data_ptr);

//sends explore message, this is used to ask for a adress to the master
U8 CommNlPsiRf_SendExplore(U8* data_ptr, U8 data_len);

//sends a request to te master that this slave wants to be excluded from the network (use this if a slave wants to be excluded from the network)
U8 CommNlPsiRf_SendRemNid(void);

//use this if a master wants to exclude a module from the network
U8 CommNlPsiRf_SendSetNidToZero(U8 nid);

//get current node id adress, you can use this to determine if you are in a network or not
U8 CommNlPsiRf_GetNid(void);

//get network homeID
U32 CommNlPsiRf_GetNetworkHid(void);

//forces this module adress to 0 (no adress) whitout leting the master know
void CommNlPsiRf_ForceRemoveFromNetwork(void);

// enable/disable fast p2p messaging (reducing the keep silent timeout after p2p message)
void CommNlPsiRf_SetFastP2PMessaging(BOOL set_fast);

void CommNlPsiRf_PrintStatus(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// C L E A R / U N D E F    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* PSIRF__COMMNLPSIRF_H */

