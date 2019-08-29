//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the prototypes of the core terminal module.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CORETERM_H
#define CORETERM_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the terminal level (to be taken from definitions below)
#ifndef TERM_LEVEL_SET
    #define TERM_LEVEL_SET          TERM_LEVEL_FULL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//DRIVER lib include section
#include "sci\DrvSciChannel.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// @remark  term level definitions
#define TERM_LEVEL_FULL                     4
#define TERM_LEVEL_NO_HELP_TEXT             3
#define TERM_LEVEL_NO_HELP                  2
#define TERM_LEVEL_SELF                     1
#define TERM_LEVEL_NONE                     0
//------------------------------------------------------------------------------------------------//
// @remark  actual term level, if log level did include enable terminal, terminal is disabled
#if (CORELOG_LEVEL_MASTER) & (CORELOG_LEVEL) & LOG_LEVEL_TERM
    #define TERM_LEVEL          TERM_LEVEL_SET
#else
    #define TERM_LEVEL          TERM_LEVEL_NONE
#endif
//------------------------------------------------------------------------------------------------//
// @remark  define to use with the "CoreTerm_RegisterCommand" function, put this define in the "numberOfArguments" position if you want to defien a command that can take a variable number of arguments
#define CMD_VAR_ARGUMENTS                   255
//------------------------------------------------------------------------------------------------//
// @remark  definitions needed for forward compatibility
#define CMD_HIDDEN                          FALSE
#define CMD_PUBLIC                          TRUE
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @remark  Prototype of the command handling function
typedef void (*TERMHOOK_COMMAND_RECEIVED)(CHAR* command_ptr, U8 length);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreTerm_Init(SCI_CHANNEL_HNDL channel_hndl);
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreTerm_SwitchSciChannel(SCI_CHANNEL_HNDL channel_hndl);
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreTerm_Handler(void);
//------------------------------------------------------------------------------------------------//
// @remark  functions are prototyped depending on terminal level
#if TERM_LEVEL == TERM_LEVEL_SELF
    // @remark  possibility to register hook from application to handle the RX messages
    void CoreTerm_RegisterCommandHook(TERMHOOK_COMMAND_RECEIVED command_hook);
#else
    // @remark  definition to avoid compiler errors
    #define CoreTerm_RegisterCommandHook(x1)
#endif
//------------------------------------------------------------------------------------------------//
// @remark  functions are prototyped depending on terminal level
#if TERM_LEVEL == TERM_LEVEL_NONE
    // @remark  definition to avoid compiler errors
    #define CoreTerm_RegisterCommand(x1,x2,x3,x4,x5)

    // @remark  definition to avoid compiler errors
    #define CoreTerm_GetArgumentAsU32(x1)

    // @remark  definition to avoid compiler errors
    #define CoreTerm_GetArgumentAsS32(x1)

    // @remark  definition to avoid compiler errors
    #define CoreTerm_GetArgumentAsBool(x1)

    // @remark  definition to avoid compiler errors
    #define CoreTerm_GetArgumentAsString(x1,x2)
#else
    
    #if TERM_LEVEL == TERM_LEVEL_FULL
        #define CoreTerm_RegisterCommand(x1,x2,x3,x4,x5)        CoreTerm_RegisterCommandWrap(x1,x2,x3,x4,x5)
    #else
        #define CoreTerm_RegisterCommand(x1,x2,x3,x4,x5)        CoreTerm_RegisterCommandWrap(x1,"",x3,x4,x5)
    #endif
    
    // @remark  registers a command to the terminal
    void CoreTerm_RegisterCommandWrap(STRING command, STRING helpText, U8 numberOfArguments, EVENT_CALLBACK commandoCallback, BOOL show);

    // @remark  none
    U32 CoreTerm_GetArgumentAsU32(U8 argumentNumber);

    // @remark  none
    S32 CoreTerm_GetArgumentAsS32(U8 argumentNumber);

    // @remark  none
    BOOL CoreTerm_GetArgumentAsBool(U8 argumentNumber);

    // @remark  none
    void CoreTerm_GetArgumentAsString(U8 argumentNumber, STRING stringPointer);    //string is given in all lowercase characters
#endif
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreTerm_PrintAcknowledge(void);
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreTerm_PrintFailed(void);
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreTerm_PrintFeedback(BOOL success);
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreTerm_PrintBool(BOOL state);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* CORETERM_H */
