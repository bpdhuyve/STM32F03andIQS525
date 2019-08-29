//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module that handles the configuration of the PicoTCP stack
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define PICOTCP_STDPICOTCPCONFIG_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef PICOTCP_STDPICOTCPCONFIG_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               PICOTCP_STDPICOTCPCONFIG_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef MODULE_ID_PICOTCP
    #error "MODULE_ID_PICOTCP not defined in AppConfig.h" 
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_IP_ADDRESS_3
    #define DEFAULT_IP_ADDRESS_3   192
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_IP_ADDRESS_2
    #define DEFAULT_IP_ADDRESS_2   168
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_IP_ADDRESS_1
    #define DEFAULT_IP_ADDRESS_1   100
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_IP_ADDRESS_0
    #define DEFAULT_IP_ADDRESS_0   150
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_NETMASK_3
    #define DEFAULT_NETMASK_3      255
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_NETMASK_2
    #define DEFAULT_NETMASK_2      255
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_NETMASK_1
    #define DEFAULT_NETMASK_1      255
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_NETMASK_0
    #define DEFAULT_NETMASK_0      0
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_GATEWAY_3
    #define DEFAULT_GATEWAY_3      192
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_GATEWAY_2
    #define DEFAULT_GATEWAY_2      168
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_GATEWAY_1
    #define DEFAULT_GATEWAY_1      100
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_GATEWAY_0
    #define DEFAULT_GATEWAY_0      0
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DEFAULT_DHCP_ACTIVE
    #define DEFAULT_DHCP_ACTIVE    FALSE
#endif
//------------------------------------------------------------------------------------------------//
#ifndef INSTANCE_NAME
    #define INSTANCE_NAME          "PSI"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#define STDLIB_MEMCOPY
#include "Core.h"
#include <stdio.h>

// SYS

// DRV

// STD
#include "memory/StdDataMngr.h"
#include "picotcp/StdPicoTcpConfig.h"

// CORE
#include "CoreHeap.h"

// PICOTCP
//#include "pico_defines.h"
#include "pico_stack.h"
//#include "pico_config.h"
#include "pico_ipv4.h"
//#include "pico_socket.h"
#include "pico_icmp4.h"
//#include "pico_device.h"
#include "pico_dev_emac.h"
#include "pico_mdns.h"
//#include "pico_dns_client.h"
#include "pico_dns_sd.h"
//#include "pico_http_util.h"
//#include "pico_http_server.h"
//#include "pico_http_client.h"
#include "pico_dhcp_client.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define MDNS_NAME_LEN               30
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    U8      ip_address[4];
    U8      ip_route[4];
    U8      ip_mask[4];
    U8      ip_gateway[4];
    
    BOOL    dhcp_active;
}
PICOTCP_STORE_V1;

typedef struct
{
    U8      ip_address[4];
    U8      ip_mask[4];
    U8      ip_gateway[4];
    
    BOOL    dhcp_active;
    
    CHAR    hostname[MDNS_NAME_LEN + 1];
}
PICOTCP_STORE_V2;
//------------------------------------------------------------------------------------------------//
typedef enum pico_err_e PICOTCP_ERROR; // picoTCP internal errors

typedef enum
{
    PICO_APP_NO_ERROR = 0,
    MEM_ERROR_FREE_NULL = 1,
    MEM_ERROR_CORRUPT = 2,
    MEM_ERROR_ALLOC_FAILED = 3,
    ERROR_CREATING_EMAC = 4,
    ERROR_CREATING_EMAC_NO_MEM = 5,
    ERROR_CREATING_EMAC_PHY_INIT = 6,
    ERROR_CREATING_EMAC_DMA_NOT_RESET = 7,
    ERROR_CREATING_EMAC_INIT_FAILED = 8,
}
PICO_APP_ERROR;

typedef enum
{
    PICOTCP_ADDR_MODE_INIT          = 0,
    PICOTCP_ADDR_MODE_STATIC        = 1,
    PICOTCP_ADDR_MODE_DHCP          = 2,
}
PICOTCP_ADDR_MODE;

typedef struct
{
    U8      mac_address[6];
    U8      dhcp_ip_address[4];
    U8      dhcp_ip_gateway[4];
    
    PICOTCP_ERROR       pico_error;
    PICO_APP_ERROR      pico_app_error;
    PICOTCP_ADDR_MODE   pico_addr_mode;
}
PICOTCP_DATA;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_LoadStoreDefaults(void);
static void PicoTcpConfig_ConvertStoreData(U8* stored_data_ptr, U16 stored_data_len, U8  stored_data_version);

static void PicoTcpConfig_UpdateAddress(void);
static void PicoTcpConfig_DHCPCallback(void *cli, int code);

static void PicoTcpConfig_MdnsInit(void);
static void PicoTcpConfig_MdnsCheckReinit(void);
static void PicoTcpConfig_MdnsInitCallback(pico_mdns_rtree * rtree, char *str, void *arg);
static void PicoTcpConfig_MdnsRegisterService(void);
static void PicoTcpConfig_MdnsRegisterServiceCallback(pico_mdns_rtree * rtree, char *str, void *arg);

#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_TCPParaGet(void);
static void Command_TCPParaSet(void);
static void Command_TCPInfo(void);
static void Command_TCPPing(void);
static void PicoTcpConfig_cb_ping(struct pico_icmp4_stats *s);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static STRING                   default_hostname;
static U8                       mac[PICO_SIZE_ETH];
static struct pico_device*      emac;
static U32                      xid;

static PICOTCP_STORE_V2         picotcp_store;
static PICOTCP_DATA             picotcp_data;

static TASK_HNDL                mdns_delay_init_task;
static BOOL                     mdns_reinit_needed;

static DHCP_CALLBACK            dhcp_callback_hook = NULL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
volatile uint32_t               lpc_tick = 0;
volatile pico_time              full_tick = 0;

// this has to be experted from the linker using
// export symbol __ICFEDIT_size_heap__;
extern const U32                __ICFEDIT_size_heap__;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_LoadStoreDefaults(void)
{
    picotcp_store.ip_address[0] = DEFAULT_IP_ADDRESS_3;
    picotcp_store.ip_address[1] = DEFAULT_IP_ADDRESS_2;
    picotcp_store.ip_address[2] = DEFAULT_IP_ADDRESS_1;
    picotcp_store.ip_address[3] = DEFAULT_IP_ADDRESS_0;
    
    picotcp_store.ip_mask[0]    = DEFAULT_NETMASK_3;
    picotcp_store.ip_mask[1]    = DEFAULT_NETMASK_2;
    picotcp_store.ip_mask[2]    = DEFAULT_NETMASK_1;
    picotcp_store.ip_mask[3]    = DEFAULT_NETMASK_0;
    
    picotcp_store.ip_gateway[0] = DEFAULT_GATEWAY_3;
    picotcp_store.ip_gateway[1] = DEFAULT_GATEWAY_2;
    picotcp_store.ip_gateway[2] = DEFAULT_GATEWAY_1;
    picotcp_store.ip_gateway[3] = DEFAULT_GATEWAY_0;
    
    picotcp_store.dhcp_active   = DEFAULT_DHCP_ACTIVE;
    
    MEMSET((VPTR)picotcp_store.hostname, 0, SIZEOF(picotcp_store.hostname));
    if(CoreString_Contains(default_hostname, ".local") == FALSE)
    {
        CoreString_SubString(default_hostname, picotcp_store.hostname, 0, MDNS_NAME_LEN-6);
        CoreString_Concat(picotcp_store.hostname, ".local");
    }
    else
    {
        CoreString_SubString(default_hostname, picotcp_store.hostname, 0, MDNS_NAME_LEN);
    }
}
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_ConvertStoreData(U8* stored_data_ptr, U16 stored_data_len, U8  stored_data_version)
{
    if(stored_data_version == 1)
    {
        MEMCPY((VPTR)picotcp_store.ip_address,  (VPTR)(&stored_data_ptr[0]),  4);
        MEMCPY((VPTR)picotcp_store.ip_mask,     (VPTR)(&stored_data_ptr[4]),  4);
        MEMCPY((VPTR)picotcp_store.ip_gateway,  (VPTR)(&stored_data_ptr[12]), 4);
        picotcp_store.dhcp_active = ((PICOTCP_STORE_V1*)stored_data_ptr)->dhcp_active;
    }
}
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_UpdateAddress(void)
{
    struct pico_ip4     v4_ip_host;
    struct pico_ip4     v4_mask;
    struct pico_ip4     v4_gateway;
    struct pico_ip4     v4_ip_host_any  = {0};
    struct pico_ip4     v4_mask_any     = {0};
    
    if(picotcp_data.pico_app_error == PICO_APP_NO_ERROR && emac)
    {
        // stop existing
        if(picotcp_data.pico_addr_mode == PICOTCP_ADDR_MODE_STATIC)
        {
            // remove existing link
            pico_ipv4_cleanup_links(emac);
            // remove default gateway
            pico_ipv4_route_del(v4_ip_host_any, v4_mask_any, 10);
        }
        else if(picotcp_data.pico_addr_mode == PICOTCP_ADDR_MODE_DHCP)
        {
            pico_dhcp_client_abort(xid);
        }
        
        // config new
        if(picotcp_store.dhcp_active)
        {
            pico_dhcp_initiate_negotiation(emac, PicoTcpConfig_DHCPCallback, (uint32_t*)&xid);
            
            picotcp_data.pico_addr_mode = PICOTCP_ADDR_MODE_DHCP;
        }
        else
        {
            v4_ip_host.addr = (U32)(picotcp_store.ip_address[0]) | ((U32)(picotcp_store.ip_address[1]) << 8) |
                              ((U32)(picotcp_store.ip_address[2]) << 16) | ((U32)(picotcp_store.ip_address[3]) << 24);
            
            v4_mask.addr = (U32)(picotcp_store.ip_mask[0]) | ((U32)(picotcp_store.ip_mask[1]) << 8) |
                           ((U32)(picotcp_store.ip_mask[2]) << 16) | ((U32)(picotcp_store.ip_mask[3]) << 24);
            
            v4_gateway.addr = (U32)(picotcp_store.ip_gateway[0]) | ((U32)(picotcp_store.ip_gateway[1]) << 8) |
                              ((U32)(picotcp_store.ip_gateway[2]) << 16) | ((U32)(picotcp_store.ip_gateway[3]) << 24);
            
            pico_ipv4_link_add(emac, v4_ip_host, v4_mask);
            
            pico_ipv4_route_add(v4_ip_host_any, v4_mask_any, v4_gateway, 10, NULL);
            
            picotcp_data.pico_addr_mode = PICOTCP_ADDR_MODE_STATIC;
            
            mdns_reinit_needed = TRUE;
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_DHCPCallback(void *cli, int code)
{
    struct pico_ip4 v4_ip_host;
    struct pico_ip4 v4_gateway;
    
    if(code == PICO_DHCP_ERROR)
    {
        picotcp_data.pico_error = pico_err;
    }
    else if(code == PICO_DHCP_SUCCESS)
    {
        v4_ip_host = pico_dhcp_get_address(cli);
        picotcp_data.dhcp_ip_address[0] =  (U8)(v4_ip_host.addr & 0x000000FF);
        picotcp_data.dhcp_ip_address[1] =  (U8)((v4_ip_host.addr & 0x0000FF00) >> 8);
        picotcp_data.dhcp_ip_address[2] =  (U8)((v4_ip_host.addr & 0x00FF0000) >> 16);
        picotcp_data.dhcp_ip_address[3] =  (U8)((v4_ip_host.addr & 0xFF000000) >> 24);
        
        v4_gateway = pico_dhcp_get_gateway(cli);
        picotcp_data.dhcp_ip_gateway[0] =  (U8)(v4_gateway.addr & 0x000000FF);
        picotcp_data.dhcp_ip_gateway[1] =  (U8)((v4_gateway.addr & 0x0000FF00) >> 8);
        picotcp_data.dhcp_ip_gateway[2] =  (U8)((v4_gateway.addr & 0x00FF0000) >> 16);
        picotcp_data.dhcp_ip_gateway[3] =  (U8)((v4_gateway.addr & 0xFF000000) >> 24);
        
        mdns_reinit_needed = TRUE;
    }
    else if(code == PICO_DHCP_RESET)
    {
        //TODO
    }
    
    if(dhcp_callback_hook != NULL)
    {
        dhcp_callback_hook((U32)code);
    }
}
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_MdnsInit(void)
{
    if(pico_mdns_init(picotcp_store.hostname, pico_ipv4_link_by_dev(emac)->address, &PicoTcpConfig_MdnsInitCallback, NULL) != 0)
    {
        LOG_WRN("[mDNS] initialisation failed");
    }
}
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_MdnsCheckReinit(void)
{
    if(mdns_reinit_needed)
    {
        mdns_reinit_needed = FALSE;
        
        if(pico_mdns_init(picotcp_store.hostname, pico_ipv4_link_by_dev(emac)->address, &PicoTcpConfig_MdnsInitCallback, NULL) != 0)
        {
            LOG_WRN("[mDNS] initialisation failed");
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_MdnsInitCallback(pico_mdns_rtree * rtree, char *str, void *arg) 
{ 
    if(CoreString_Equals(str, picotcp_store.hostname))
    {
        LOG_DBG("[mDNS] initialised with hostname: %s", PCSTR(picotcp_store.hostname));
        PicoTcpConfig_MdnsRegisterService();
    }
}
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_MdnsRegisterService(void)
{
    U8          i;
    CHAR        temp_buffer[64];
    U8          len;
    
    PICO_DNS_SD_KV_VECTOR_DECLARE(key_value_pair_vector);
    
    // host
    pico_dns_sd_kv_vector_add(&key_value_pair_vector, "host", picotcp_store.hostname);
    
    // MAC
    MEMSET((VPTR)temp_buffer, 0, SIZEOF(temp_buffer));
    for(i=0; i<6; i++)
    {
        CoreConvert_U8ToHexString(picotcp_data.mac_address[i], &temp_buffer[3*i], FALSE, TRUE);
        temp_buffer[3*i+2] = ':';
    }
    temp_buffer[17] = 0;
    pico_dns_sd_kv_vector_add(&key_value_pair_vector, "MAC", temp_buffer);
    
    // service name
    MEMSET((VPTR)temp_buffer, 0, SIZEOF(temp_buffer));
    CoreString_CopyString(INSTANCE_NAME,temp_buffer);
    len = CoreString_GetLength(INSTANCE_NAME);
    temp_buffer[len++] = ' ';
    temp_buffer[len++] = '[';
    for(i=0; i<6; i++)
    {
        CoreConvert_U8ToHexString(picotcp_data.mac_address[i], &temp_buffer[2*i+len], FALSE, TRUE);
    }
    len += 12;
    temp_buffer[len++] = ']';
    
    if(pico_dns_sd_register_service(temp_buffer, "_http._tcp", 80, &key_value_pair_vector, 240, PicoTcpConfig_MdnsRegisterServiceCallback, NULL) < 0)
    {
        LOG_WRN("[mDNS] service registration failed");
    }
}
//------------------------------------------------------------------------------------------------//
static void PicoTcpConfig_MdnsRegisterServiceCallback(pico_mdns_rtree * rtree, char *str, void *arg) 
{
    LOG_DBG("[mDNS] service CB %c", PDSTR(str));
    if(CoreString_Equals(str, picotcp_store.hostname))
    {
        LOG_DBG("[mDNS] service registered successfully");
    }
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Command_TCPParaGet(void)
{
    const STRING    false_true[] = {"FALSE", "TRUE"};
    CHAR            temp_string[16];
    
    StdPicoTcpConfig_GetParameterAsString(PICOTCPCONFIG_PARA_IP_ADDRESS, temp_string, SIZEOF(temp_string));
    LOG_TRM(" 0. IP ADDRESS       : %c", PDSTR(temp_string));
    StdPicoTcpConfig_GetParameterAsString(PICOTCPCONFIG_PARA_NETWORK_MASK, temp_string, SIZEOF(temp_string));
    LOG_TRM(" 1. NETWORK MASK     : %c", PDSTR(temp_string));
    StdPicoTcpConfig_GetParameterAsString(PICOTCPCONFIG_PARA_DEFAULT_GATEWAY, temp_string, SIZEOF(temp_string));
    LOG_TRM(" 2. DEFAULT GATEWAY  : %c", PDSTR(temp_string));
    LOG_TRM(" 3. DHCP ACTIVE      : %s (%d) [0-1]", PCSTR(false_true[picotcp_store.dhcp_active]), PU8(picotcp_store.dhcp_active));
    LOG_TRM(" 4. HOSTNAME         : %s", PCSTR(picotcp_store.hostname));
    
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_TCPParaSet(void)
{
    PICOTCPCONFIG_PARA  para = (PICOTCPCONFIG_PARA)CoreTerm_GetArgumentAsU32(0);
    CHAR                temp_string[MDNS_NAME_LEN + 1];
    
    if(para == PICOTCPCONFIG_PARA_DHCP_ACTIVE)
    {
        CoreTerm_PrintFeedback(StdPicoTcpConfig_SetParameter(para, CoreTerm_GetArgumentAsU32(1)));
    }
    else
    {
        CoreTerm_GetArgumentAsString(1, temp_string);
        CoreTerm_PrintFeedback(StdPicoTcpConfig_SetParameterAsString(para, temp_string));
    }
}
//------------------------------------------------------------------------------------------------//
static void Command_TCPInfo(void)
{
    const STRING string_offon[] = {"OFF","ON"};
    CHAR temp_string[20];
    
    LOG_TRM("PICO TCP info");
    LOG_TRM("-------------");
    StdPicoTcpConfig_GetInfoAsString(PICOTCPCONFIG_INFO_MAC_ADDRESS, temp_string, SIZEOF(temp_string));
    LOG_TRM(" MAC               %c", PDSTR(temp_string));
    LOG_TRM(" DHCP              %s", PCSTR(string_offon[picotcp_store.dhcp_active]));
    StdPicoTcpConfig_GetInfoAsString(PICOTCPCONFIG_INFO_IP_ADDRESS, temp_string, SIZEOF(temp_string));
    LOG_TRM(" IP                %c", PDSTR(temp_string));
    StdPicoTcpConfig_GetInfoAsString(PICOTCPCONFIG_INFO_GATEWAY, temp_string, SIZEOF(temp_string));
    LOG_TRM(" GATEWAY           %c", PDSTR(temp_string));
    LOG_TRM(" HOSTNAME          %s", PCSTR(picotcp_store.hostname));
    LOG_TRM(" PICO error        %d", PU32(picotcp_data.pico_error));
    LOG_TRM(" PICO APP error    %d", PU32(picotcp_data.pico_app_error));
#ifdef PICO_SUPPORT_RTOS
#ifdef USE_FREERTOS
    LOG_TRM(" PICO mem          act %d - hwm %d - max %d", PU32((U32)(xPortGetTotalHeapSize() - xPortGetFreeHeapSize())), PU32((U32)(xPortGetTotalHeapSize() - xPortGetMinimumFreeHeapSize())), PU32((U32)xPortGetTotalHeapSize()));
#endif
#endif
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Command_TCPPing(void)
{
    U32     id;
    CHAR    ping_ip_buffer[16];
    
    CoreTerm_GetArgumentAsString(0, ping_ip_buffer);

    LOG_TRM("starting ping");
    id = pico_icmp4_ping(ping_ip_buffer, 4, 1000, 10000, 1040, PicoTcpConfig_cb_ping);
}
//------------------------------------------------------------------------------------------------//
/* gets called when the ping receives a reply, or encounters a problem */
static void PicoTcpConfig_cb_ping(struct pico_icmp4_stats *s)
{
    CHAR host[16];
    
    pico_ipv4_to_string(host, s->dst.addr);
    
    if (s->err == 0)
    {
        /* if all is well, print some pretty info */
        LOG_TRM("%d bytes from %c: icmp_req=%d ttl=%d time=%d ms", PU32(s->size), PDSTR(host), PU32(s->seq), PU32(s->ttl), PU32((U32)s->time));
    }
    else
    {
        /* if something went wrong, print it and signal we want to stop */
        LOG_TRM("PING %d to %c: Error %d", PU32(s->seq), PDSTR(host), PS32(s->err));
    }
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
BOOL StdPicoTcpConfig_Init(U8* mac_ptr, const STRING hostname)
{
    MODULE_INIT_ONCE();
    
    default_hostname = hostname;
    
    pico_stack_init();
    
#if (TERM_LEVEL > TERM_LEVEL_NONE)
    CoreTerm_RegisterCommand("TCPParaGet", "TCP get parameter values", 0, Command_TCPParaGet, TRUE);
    CoreTerm_RegisterCommand("TCPParaSet", "TCP set parameter a (0-4) to b", 2, Command_TCPParaSet, TRUE);
    CoreTerm_RegisterCommand("TCPInfo", "TCP get info", 0, Command_TCPInfo, TRUE);
    CoreTerm_RegisterCommand("TCPPing", "TCP ping to an IP address (a)", 1, Command_TCPPing, TRUE);
#endif
    
    StdDataMngr_Register(MODULE_ID_PICOTCP, PicoTcpConfig_ConvertStoreData, PicoTcpConfig_LoadStoreDefaults, (VPTR)&picotcp_store, SIZEOF(picotcp_store), 2, TRUE, "PICOTCP");

    memcpy((VPTR)picotcp_data.mac_address, (VPTR)mac_ptr, PICO_SIZE_ETH);
    
    // create EMAC
    emac = (struct pico_device *) pico_emac_create("lpc-emac", mac_ptr);
    if(!emac)
    {
        if(picotcp_data.pico_app_error == PICO_APP_NO_ERROR)
        {
            picotcp_data.pico_app_error = ERROR_CREATING_EMAC;
        }
        LOG_ERR("PICO TCP init failed");
        return FALSE;
    }
    
    // set up IP
    picotcp_data.pico_addr_mode = PICOTCP_ADDR_MODE_INIT;
    PicoTcpConfig_UpdateAddress();
    
    // set up MDNS
    PicoTcpConfig_MdnsInit();
    mdns_reinit_needed = FALSE;
    
    MODULE_INIT_DONE();
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
void StdPicoTcpConfig_Handler(void)
{
    MODULE_CHECK();
    
    PicoTcpConfig_MdnsCheckReinit();
    pico_stack_tick();
}
//------------------------------------------------------------------------------------------------//
BOOL StdPicoTcpConfig_SetParameter(PICOTCPCONFIG_PARA parameter, U32 value)
{
    BOOL    update_needed = TRUE;
    
    switch(parameter)
    {
    case PICOTCPCONFIG_PARA_IP_ADDRESS:
        picotcp_store.ip_address[0] = (U8)(value & 0x000000FF);
        picotcp_store.ip_address[1] = (U8)((value & 0x0000FF00) >> 8);
        picotcp_store.ip_address[2] = (U8)((value & 0x00FF0000) >> 16);
        picotcp_store.ip_address[3] = (U8)((value & 0xFF000000) >> 24);
        break;
        
    case PICOTCPCONFIG_PARA_NETWORK_MASK:
        picotcp_store.ip_mask[0] =    (U8)(value & 0x000000FF);
        picotcp_store.ip_mask[1] =    (U8)((value & 0x0000FF00) >> 8);
        picotcp_store.ip_mask[2] =    (U8)((value & 0x00FF0000) >> 16);
        picotcp_store.ip_mask[3] =    (U8)((value & 0xFF000000) >> 24);
        break;
        
    case PICOTCPCONFIG_PARA_DEFAULT_GATEWAY:
        picotcp_store.ip_gateway[0] = (U8)(value & 0x000000FF);
        picotcp_store.ip_gateway[1] = (U8)((value & 0x0000FF00) >> 8);
        picotcp_store.ip_gateway[2] = (U8)((value & 0x00FF0000) >> 16);
        picotcp_store.ip_gateway[3] = (U8)((value & 0xFF000000) >> 24);
        break;
        
    case PICOTCPCONFIG_PARA_DHCP_ACTIVE:
        if(value > 1)
        {
            return FALSE;
        }
        update_needed = (BOOL)(picotcp_store.dhcp_active != (BOOL)value);
        picotcp_store.dhcp_active = (BOOL)value;
        break;
        
    case PICOTCPCONFIG_PARA_HOSTNAME:
    default:
        return FALSE;
    }
    
    if(update_needed)
    {
        StdDataMngr_SaveData();
        PicoTcpConfig_UpdateAddress();
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdPicoTcpConfig_SetParameterAsString(PICOTCPCONFIG_PARA parameter, STRING string_value)
{
    U32     ipv4_value;
    BOOL    update_needed = TRUE;
    
    switch(parameter)
    {
    case PICOTCPCONFIG_PARA_IP_ADDRESS:
    case PICOTCPCONFIG_PARA_NETWORK_MASK:
    case PICOTCPCONFIG_PARA_DEFAULT_GATEWAY:
        pico_string_to_ipv4(string_value, (uint32_t*)&ipv4_value);
        return StdPicoTcpConfig_SetParameter(parameter, ipv4_value);
        
    case PICOTCPCONFIG_PARA_DHCP_ACTIVE:
        return StdPicoTcpConfig_SetParameter(parameter, CoreConvert_StringToU32(string_value));
        
    case PICOTCPCONFIG_PARA_HOSTNAME:
        MEMSET((VPTR)picotcp_store.hostname, 0, SIZEOF(picotcp_store.hostname));
        if(CoreString_Contains(string_value, ".local") == FALSE)
        {
            if(CoreString_GetLength(string_value) > (MDNS_NAME_LEN-6))
            {
                return FALSE;
            }
            CoreString_SubString(string_value, picotcp_store.hostname, 0, MDNS_NAME_LEN-6);
            CoreString_Concat(picotcp_store.hostname, ".local");
        }
        else
        {
            if(CoreString_GetLength(string_value) > MDNS_NAME_LEN)
            {
                return FALSE;
            }
            CoreString_SubString(string_value, picotcp_store.hostname, 0, MDNS_NAME_LEN);
        }
        mdns_reinit_needed = TRUE;        
        break;
        
    default:
        return FALSE;
    }
    
    if(update_needed)
    {
        StdDataMngr_SaveData();
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdPicoTcpConfig_GetParameter(PICOTCPCONFIG_PARA parameter, U32* value_ptr)
{
    switch(parameter)
    {
    case PICOTCPCONFIG_PARA_IP_ADDRESS:
        *value_ptr = (U32)(picotcp_store.ip_address[0]) |
                     ((U32)(picotcp_store.ip_address[1]) << 8) |
                     ((U32)(picotcp_store.ip_address[2]) << 16) |
                     ((U32)(picotcp_store.ip_address[3]) << 24);
        break;
        
    case PICOTCPCONFIG_PARA_NETWORK_MASK:
        *value_ptr = (U32)(picotcp_store.ip_mask[0]) |
                     ((U32)(picotcp_store.ip_mask[1]) << 8) |
                     ((U32)(picotcp_store.ip_mask[2]) << 16) |
                     ((U32)(picotcp_store.ip_mask[3]) << 24);
        break;
        
    case PICOTCPCONFIG_PARA_DEFAULT_GATEWAY:
        *value_ptr = (U32)(picotcp_store.ip_gateway[0]) |
                     ((U32)(picotcp_store.ip_gateway[1]) << 8) |
                     ((U32)(picotcp_store.ip_gateway[2]) << 16) |
                     ((U32)(picotcp_store.ip_gateway[3]) << 24);
        break;
        
    case PICOTCPCONFIG_PARA_DHCP_ACTIVE:
        *value_ptr = (U32)picotcp_store.dhcp_active;
        break;
        
    case PICOTCPCONFIG_PARA_HOSTNAME:
    default:
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdPicoTcpConfig_GetParameterAsString(PICOTCPCONFIG_PARA parameter, STRING string_value, U8 string_len)
{
    U32 ipv4_value;
    
    MEMSET((VPTR)string_value, 0, string_len);
    switch(parameter)
    {
    case PICOTCPCONFIG_PARA_IP_ADDRESS:
    case PICOTCPCONFIG_PARA_NETWORK_MASK:
    case PICOTCPCONFIG_PARA_DEFAULT_GATEWAY:
        if(string_len < 16)
        {
            return FALSE;
        }
        StdPicoTcpConfig_GetParameter(parameter, &ipv4_value);
        pico_ipv4_to_string(string_value, ipv4_value);
        break;
        
    case PICOTCPCONFIG_PARA_DHCP_ACTIVE:
        snprintf(string_value, string_len, "%d", picotcp_store.dhcp_active);
        break;
        
    case PICOTCPCONFIG_PARA_HOSTNAME:
        snprintf(string_value, string_len, picotcp_store.hostname);
        break;
        
    default:
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
// @remark  function to get info on the active configuration
BOOL StdPicoTcpConfig_GetInfo(PICOTCPCONFIG_INFO info, U32* value_ptr)
{
    switch(info)
    {
    case PICOTCPCONFIG_INFO_IP_ADDRESS:
        *value_ptr = pico_ipv4_link_by_dev(emac)->address.addr;
        break;
        
    case PICOTCPCONFIG_INFO_GATEWAY:
        if(picotcp_store.dhcp_active)
        {
            *value_ptr = pico_dhcp_get_gateway(pico_dhcp_get_identifier(xid)).addr;
        }
        else
        {
            StdPicoTcpConfig_GetParameter(PICOTCPCONFIG_PARA_DEFAULT_GATEWAY, value_ptr);
        }
        break;
        
    case PICOTCPCONFIG_INFO_MAC_ADDRESS:
        *value_ptr = (U32)(picotcp_data.mac_address[5]) |
                     ((U32)(picotcp_data.mac_address[4]) << 8) |
                     ((U32)(picotcp_data.mac_address[3]) << 16) |
                     ((U32)(picotcp_data.mac_address[2]) << 24);
        break;
    default:
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdPicoTcpConfig_GetInfoAsString(PICOTCPCONFIG_INFO info, STRING string_value, U8 string_len)
{
    U32 temp_u32;
    
    MEMSET((VPTR)string_value, 0, string_len);
    switch(info)
    {
    case PICOTCPCONFIG_INFO_IP_ADDRESS:
        if(string_len < 16)
        {
            return FALSE;
        }
        pico_ipv4_to_string(string_value, pico_ipv4_link_by_dev(emac)->address.addr);
        break;
        
    case PICOTCPCONFIG_INFO_GATEWAY:
        if(string_len < 16)
        {
            return FALSE;
        }
        StdPicoTcpConfig_GetInfo(PICOTCPCONFIG_INFO_GATEWAY, &temp_u32);
        pico_ipv4_to_string(string_value, temp_u32);
        break;
        
    case PICOTCPCONFIG_INFO_MAC_ADDRESS:
        if(string_len < 18)
        {
            return FALSE;
        }
        snprintf(string_value, string_len,
                 "%02x:%02x:%02x:%02x:%02x:%02x",
                 picotcp_data.mac_address[0],
                 picotcp_data.mac_address[1],
                 picotcp_data.mac_address[2],
                 picotcp_data.mac_address[3],
                 picotcp_data.mac_address[4],
                 picotcp_data.mac_address[5]);
    break;
        
    default:
        return FALSE;
    }
    
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
void StdPicoTcpConfig_RegisterDhcpCallback(DHCP_CALLBACK dhcp_callback)
{
    dhcp_callback_hook = dhcp_callback;
}
//------------------------------------------------------------------------------------------------//
#ifndef PICO_SUPPORT_RTOS
#ifdef MEM_MEASURE
void pico_ffree(void* x)
{
    CoreHeap_Free(x);
}
//------------------------------------------------------------------------------------------------//
void * pico_zzalloc(size_t x)
{
    VPTR    data_ptr = CoreHeap_Alloc(x);
    if(data_ptr != NULL)
    {
        MEMSET(data_ptr, 0, x);
    }
    return data_ptr;
}
#endif /* MEM_MEASURE */
#endif /* PICO_SUPPORT_RTOS */
//================================================================================================//
