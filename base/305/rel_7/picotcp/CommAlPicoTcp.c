//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Abstraction layer for the pico TCP stack to upper blocks
// This layer needs FREERTOS
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COMMALPICOTCP_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef COMMALPICOTCP_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               COMMALPICOTCP_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SOCKET_RX_BUFSIZE
    #define SOCKET_RX_BUFSIZE           1024
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#define STDLIB_MEMCOPY
#include "Core.h"

/* PicoTCP includes */
#include "pico_defines.h"
#include "pico_stack.h"
#include "pico_config.h"
#include "pico_ipv4.h"
#include "pico_socket.h"
#include "pico_icmp4.h"
#include "pico_device.h"
#include "pico_dev_emac.h"
#include "pico_http_util.h"
#include "pico_http_server.h"
#include "pico_http_client.h"
#include "pico_dhcp_client.h"

//#include "www_data.h"
#include "pico_main.h"

//APP include section
#include "CommAlPicoTcp.h"

#define MUTEX_PREFIX(postfix)                                   sendmsg_mutex##postfix
#include "PsiMutexTpl.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void TcpCallback(U16 event, struct pico_socket *s);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
struct pico_socket*                 socket;
static BOOL                         socket_writable;
static U8                           rx_buffer[SOCKET_RX_BUFSIZE];

static SOCKET_READ_EVENT_HOOK       read_event_hook;
static EVENT_CALLBACK               error_event_hook;
static EVENT_CALLBACK               close_event_hook;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void TcpCallback(U16 event, struct pico_socket *s)
{
    S32 tcp_read = 0;
    U16 port;
    U8  received_bytes[SOCKET_RX_BUFSIZE];    
    U8 i, j;
    
    /* process read event, data available */
    if (event & PICO_SOCK_EV_RD)
    {
        do
        {
            tcp_read = pico_socket_read(s, rx_buffer, SOCKET_RX_BUFSIZE);
            if(read_event_hook != NULL)
            {
                read_event_hook(rx_buffer, tcp_read);
            }
            LOG_DBG("Socket RX - %02h", PU8A(rx_buffer, tcp_read));
        }
        while(tcp_read > 0);
    }
    
    // process write event, data available
    if (event & PICO_SOCK_EV_WR)
    {
        socket_writable = TRUE;
    }
    
    // process error event, socket error occured */
    if (event & PICO_SOCK_EV_ERR)
    {
        LOG_TRM("Socket error received");
        if(error_event_hook != NULL)
        {
            error_event_hook();
        }
    }
    
    // process close event, receiving socket received close from peer */
    if ((event & PICO_SOCK_EV_CLOSE) || (event & PICO_SOCK_EV_FIN))
    {
        LOG_TRM("Socket received close from peer");
        if(close_event_hook != NULL)
        {
            close_event_hook();
        }
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommAlPicoTcp_Init(void)
{
    MEMSET(rx_buffer, 0, SOCKET_RX_BUFSIZE);
    
    socket           = NULL;
    read_event_hook  = NULL;
    error_event_hook = NULL;
    close_event_hook = NULL;
    socket_writable  = FALSE;
}
//------------------------------------------------------------------------------------------------//
void CommAlPicoTcp_RegisterReadEventHook(SOCKET_READ_EVENT_HOOK hook)
{
    read_event_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommAlPicoTcp_RegisterErrorEventHook(EVENT_CALLBACK hook)
{
    error_event_hook = hook;
}
//------------------------------------------------------------------------------------------------//
void CommAlPicoTcp_RegisterCloseEventHook(EVENT_CALLBACK hook)
{
    close_event_hook = hook;
}
//------------------------------------------------------------------------------------------------//
BOOL CommAlPicoTcp_Connect(U8* ip, U16 port)
{
    LOG_TRM("Opening socket...");
    
    socket = pico_socket_open(PICO_PROTO_IPV4, PICO_PROTO_TCP, &TcpCallback);
    
    if (!socket)
    {
        LOG_TRM("Opening socket failed");
        return FALSE;
    }
    
    if(pico_socket_connect(socket, (void*)(ip), short_be(port)) < 0)
    {
        LOG_TRM("Connecting to socket failed");
        return FALSE;
    }
    
    LOG_TRM("Socket opened and connected to");
    
    socket_writable = TRUE;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommAlPicoTcp_Disconnect(void)
{
    if(socket != NULL)
    {
        pico_socket_del(socket);
        socket = NULL;
    }
	socket_writable = FALSE;
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL CommAlPicoTcp_SendFrame(U8* frame_ptr, U32 length)
{
    S32 tcp_written;
    
    if((frame_ptr == NULL) || (length == 0))
    {
        LOG_WRN("Inputs invalid");
        return FALSE;
    }
    
    sendmsg_mutex_Take(portMAX_DELAY);                 // take mutex
    
    while(!socket_writable)
    {
        if(socket == NULL)
        {
            return FALSE;
        }
    }
    
    tcp_written = pico_socket_write(socket, frame_ptr, length);
    if(tcp_written < 0)
    {
        LOG_TRM("AlPicoTcp: Writing mssg failed");
        sendmsg_mutex_Give();                   // give mutex
        return FALSE;
    }
    else
    {
        LOG_DBG("Socket TX - %02h", PU8A(frame_ptr, (U8)length));
    }
    
    sendmsg_mutex_Give();                       // give mutex
    return TRUE;
}
//================================================================================================//
