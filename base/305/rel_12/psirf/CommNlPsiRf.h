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
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "psirf\CommDllPsiRf.h"
#include "psirf\CommNlPsiRfUtil.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define NLPSIRF_DATA_VERSION            1
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   routing struct to be saved
typedef struct
{
    union
    {
        U8      master_hop_via_node;
        U8      slave_rssi_master_to_hop;
    }
    path;
    U8          rssi_to_node;
}
ROUTE_INFO;

// @brief   network struct to be saved by the application
typedef struct
{
    U8          network_address;
    U32         network_hid;
    U32         device_hid;
    ROUTE_INFO  route[STDNLPSIRF_MAX_NODES];
}
NLPSIRF_DATA;

// @brief   Prototype of send frame function (default send frame hook is CommDllPsiRf_SendFrame)
typedef void (*NLPSIRFHOOK_SEND_FRAME)(U8* data_ptr);

// @brief   Prototype of function where NlPsiRfHandleReceivedMsg is registered(default send frame hook is CommDllPsiRf_RegisterFrameHook)
typedef void (*NLPSIRFHOOK_REGISTER_RECV_FRAME)(DLLPSIRFHOOK_FRAME_RECEIVED frame_hook);

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

// @brief   Function to load the defaults into the data structure
void CommNlPsiRf_LoadDefaults(NLPSIRF_DATA* data_ptr, U32 device_hid, BOOL as_master);
// @brief   Function to convert an old data set to the latest version (current version defined by NLPSIRF_DATA_VERSION
void CommNlPsiRf_ConvertData(NLPSIRF_DATA* data_ptr, U8* old_data_ptr, U16 old_data_len, U8 old_data_version);

// @brief   Function to register the hook to be called to send out a message (default is CommDllPsiRf_SendFrame)
void CommNlPsiRf_RegisterSendFrameHook(NLPSIRFHOOK_SEND_FRAME send_frame_hook);
// @brief   Function to the indicate where NlPsiRfHandleReceivedMsg must be registered to (default is CommDllPsiRf_RegisterFrameHook)
void CommNlPsiRf_RegisterRecvFrameHookTo(NLPSIRFHOOK_REGISTER_RECV_FRAME registration_hook);

// @brief   Function to register the hook to be called when a new application message was received
void CommNlPsiRf_RegisterFrameHook(NLPSIRFHOOK_FRAME_RECEIVED hook);
// @brief   Function to register the hook to be called when the link status has changed
void CommNlPsiRf_RegisterLinkStatusHook(NLPSIRFHOOK_LINK_STATUS hook);
// @brief   Function to register the hook to be called when when this module gets a new network id (inclusion in network) or when its network id is set to 0 (exclusion of network) (SLAVE only)
void CommNlPsiRf_RegisterSetNidHook(NLPSIRFHOOK_SET_NID hook);
// @brief   Function to register the hook to be called when a node was added/removed from the network (MASTER only)
void CommNlPsiRf_RegisterAddRemoveNodeHook(NLPSIRFHOOK_ADD_REMOVE_NODE hook);
// @brief   Function to register the hook to be called to inform the application if the message transmission was successful or not
void CommNlPsiRf_RegisterMsgFeedbackHook(NLPSIRFHOOK_MSG_FEEDBACK hook);
// @brief   Function to register the hook to be called to trigger data save
void CommNlPsiRf_RegisterSaveDataHook(NLPSIRFHOOK_SAVE_DATA hook);

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

// function to print the NL PsiRF status
void CommNlPsiRf_PrintStatus(void);

// function to get the routing info
void CommNlPsiRf_GetRouteInfo(U8 tnid, U8* rssi_to_node_ptr, U8* hop_via_node_ptr, U8* rssi_node_to_hop_ptr);
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

