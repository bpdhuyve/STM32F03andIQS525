//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module that handles the configuration of the PicoTCP stack
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef PICOTCP_STDPICOTCPCONFIG_H
#define PICOTCP_STDPICOTCPCONFIG_H
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
    PICOTCPCONFIG_PARA_IP_ADDRESS           = 0,
    PICOTCPCONFIG_PARA_NETWORK_MASK         = 1,
    PICOTCPCONFIG_PARA_DEFAULT_GATEWAY      = 2,
    PICOTCPCONFIG_PARA_DHCP_ACTIVE          = 3,
    PICOTCPCONFIG_PARA_HOSTNAME             = 4,
}
PICOTCPCONFIG_PARA;

typedef enum
{
    PICOTCPCONFIG_INFO_IP_ADDRESS           = 0,
    PICOTCPCONFIG_INFO_GATEWAY              = 1,
    PICOTCPCONFIG_INFO_MAC_ADDRESS          = 2,
}
PICOTCPCONFIG_INFO;

typedef void (*DHCP_CALLBACK)(U32 status);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init function
// @param   mac_ptr is a pointer to an array of dimension PICO_SIZE_ETH, containing the MAC address
// @param   mdns_name is the default name to be used in MDNS.
BOOL StdPicoTcpConfig_Init(U8* mac_ptr, const STRING mdns_name);

// @remark  Background handler
void StdPicoTcpConfig_Handler(void);

// @remark  getter and setter of picotcp parameters
BOOL StdPicoTcpConfig_SetParameter(PICOTCPCONFIG_PARA parameter, U32 value);
BOOL StdPicoTcpConfig_SetParameterAsString(PICOTCPCONFIG_PARA parameter, STRING string_value);
BOOL StdPicoTcpConfig_GetParameter(PICOTCPCONFIG_PARA parameter, U32* value_ptr);
BOOL StdPicoTcpConfig_GetParameterAsString(PICOTCPCONFIG_PARA parameter, STRING string_value, U8 string_len);

// @remark  function to get info on the active configuration
BOOL StdPicoTcpConfig_GetInfo(PICOTCPCONFIG_INFO info, U32* value_ptr);
BOOL StdPicoTcpConfig_GetInfoAsString(PICOTCPCONFIG_INFO info, STRING string_value, U8 string_len);

// @remark  register a callback for a DHCP event
void StdPicoTcpConfig_RegisterDhcpCallback(DHCP_CALLBACK dhcp_callback);
//================================================================================================//



#endif /* PICOTCP_STDPICOTCPCONFIG_H */

