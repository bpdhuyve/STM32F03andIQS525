//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the source file for the core terminal module.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORETERM_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CORETERM_LOG_LEVEL
	#define CORELOG_LEVEL               (LOG_LEVEL_DEFAULT|LOG_LEVEL_TERM)
#else
	#define CORELOG_LEVEL               (CORETERM_LOG_LEVEL|LOG_LEVEL_TERM)
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max length of one coreterm_command of the terminal
#ifndef COMMAND_LENGTH
    #define COMMAND_LENGTH       							    50
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max number of commands that can be registered
#ifndef MAX_NUMBER_OF_COMMANDS
    #define MAX_NUMBER_OF_COMMANDS   							50
#elif MAX_NUMBER_OF_COMMANDS > 255
    #error "MAX_NUMBER_OF_COMMANDS can be maximum 255"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the prompt string to be used after handling a command
#ifndef PROMPT_STRING
    #define PROMPT_STRING             							"\r> "
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the help menu has to be sorted alphabetically
#ifndef HELP_SORTED
    #define HELP_SORTED         						        1
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the space between the commmand and the help text
#ifndef MIN_CHAR_SPACE_BETWEEN_COMMANDS_AND_HELP
    #define MIN_CHAR_SPACE_BETWEEN_COMMANDS_AND_HELP	        4
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the terminal baudrate to be used
#ifndef TERMINAL_BAUDRATE
    #define TERMINAL_BAUDRATE	                                SCI_SPEED_230400_BPS
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRIVER lib include section
#define Q_PREFIX(postfix)                   CoreTerm_##postfix
#define Q_SIZE                              COMMAND_LENGTH
//#include "sci\DrvSciQRxTpl.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define VERSION_STRING              CORE_CONVERT_TO_STRING(PRODUCT_ID_NUMBER)            \
                                    "." CORE_CONVERT_TO_STRING(PRODUCT_VERSION_NUMBER)   \
                                    "." CORE_CONVERT_TO_STRING(PRODUCT_REVISION_NUMBER)  \
                                    "." CORE_CONVERT_TO_STRING(PRODUCT_TEST_NUMBER)
#define BUILD_STRING                "build @ " __DATE__ " " __TIME__

#define RULER                       "-------------------------------------------------"
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if TERM_LEVEL > TERM_LEVEL_NONE
static void CoreTermLight_SciRxNewByteHook(U8* byte_ptr, U8 length);
#endif
#if TERM_LEVEL > TERM_LEVEL_SELF
static void EventHandler_CommandReceived(CHAR* coreterm_command_ptr, U8 length);
static U8 CoreTerm_GetOffset(U8 argumentNumber);
static void CoreTerm_PrintSwVersion(void);
#endif
#if TERM_LEVEL > TERM_LEVEL_NO_HELP
static void CoreTerm_PrintHelp(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
#if TERM_LEVEL > TERM_LEVEL_NONE
MODULE_DECLARE();

static SCI_CHANNEL_HNDL                 coreterm_rx_channel_hndl;

static TERMHOOK_COMMAND_RECEIVED        coreterm_command_hook;
static CHAR                             coreterm_rx_data[2][COMMAND_LENGTH+1];
static U8                               coreterm_rx_data_write_index;
static U8                               coreterm_rx_data_read_index;

static CHAR*                            coreterm_rx_read_ptr;
static CHAR*                            coreterm_rx_write_ptr;

#if TERM_LEVEL > TERM_LEVEL_SELF
static CHAR                             coreterm_rx_command[COMMAND_LENGTH];
static CHAR                             coreterm_temp_string[COMMAND_LENGTH];

static U16                              coreterm_command_count = 0;

static STRING                           coreterm_commands[MAX_NUMBER_OF_COMMANDS];
static EVENT_CALLBACK                   coreterm_cmd_hooks[MAX_NUMBER_OF_COMMANDS];
#endif
#if TERM_LEVEL > TERM_LEVEL_NO_HELP
static U8                               coreterm_cmd_arg_count[MAX_NUMBER_OF_COMMANDS];
#endif
static BOOL                             coreterm_first_pass = TRUE;
static BOOL                             coreterm_unhandled_command = FALSE;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if TERM_LEVEL > TERM_LEVEL_NONE
static void CoreTermLight_SciRxNewByteHook(U8* byte_ptr, U8 length)
{
    static U8 coreterm_ignore_char = 0;
    
    if(coreterm_ignore_char)
    {
        coreterm_ignore_char--;
    }
    else
    {
        if((*byte_ptr >= 'A') && (*byte_ptr <= 'Z'))
        {
            *byte_ptr += 'a' - 'A';
        }

        switch(*byte_ptr)
        {
        case 0x08:  // backspace
            if(coreterm_rx_write_ptr > coreterm_rx_data[coreterm_rx_data_write_index])
            {
                coreterm_rx_write_ptr--;
                *coreterm_rx_write_ptr = 0x00;
            }
            break;
        case 0x1B: // pijltjes en zo
            coreterm_ignore_char = 2;
            break;
        case '\n': // line feed
            break;
        case '\r': // carriage return
            if(coreterm_rx_write_ptr != coreterm_rx_data[coreterm_rx_data_write_index])
            {
                if(coreterm_rx_data_write_index != coreterm_rx_data_read_index)
                {
                    //coreterm_unhandled_command = TRUE;
                }
                else
                {
                    coreterm_rx_data_write_index++;
                    coreterm_rx_data_write_index &= 0x01;
                }
                coreterm_rx_write_ptr = coreterm_rx_data[coreterm_rx_data_write_index];
            }
            return;
        case 0x09: // tab
        case 0x20: // spatie
            if(coreterm_rx_write_ptr == coreterm_rx_data[coreterm_rx_data_write_index])
            {
                break;
            }
            //no break;
        default:
            if(coreterm_rx_write_ptr < &coreterm_rx_data[coreterm_rx_data_write_index][COMMAND_LENGTH])
            {
                *coreterm_rx_write_ptr++ = (CHAR)*byte_ptr;
            }
            break;
        }
    }
}
#endif
//------------------------------------------------------------------------------------------------//
#if TERM_LEVEL > TERM_LEVEL_SELF
static void EventHandler_CommandReceived(CHAR* coreterm_command_ptr, U8 length)
{
    U8  i;

    if(length == 0)
    {
        return;
    }

    //trim the arguments of coreterm_rx_read_ptr and put it in coreterm_rx_command
    CoreString_SubString(coreterm_rx_read_ptr, coreterm_rx_command, 0, CoreString_FindChar(coreterm_rx_read_ptr,' '));

    //parse the coreterm_command and call the eventhandler
    for(i = 0; i < coreterm_command_count; i++)
    {
        //copy string to compare and converrt to lowercase
        CoreString_CopyString((STRING)(coreterm_commands[i]),coreterm_temp_string);
        CoreString_ToLowerCase(coreterm_temp_string);

        if(CoreString_Equals(coreterm_rx_command,coreterm_temp_string))
        {
            ((EVENT_CALLBACK)(coreterm_cmd_hooks[i]))();
            return;
        }
    }
    CoreLog_Print("Unknown Cmd\r", LOG_LEVEL_TERM, 0);
}
//------------------------------------------------------------------------------------------------//
static U8 CoreTerm_GetOffset(U8 argumentNumber)
{
	U8 param_offset = 0;

    if(CoreString_CountChar(coreterm_rx_read_ptr, ' ') > argumentNumber)
    {
        argumentNumber++;
        while(argumentNumber)
        {
            param_offset += CoreString_FindChar(coreterm_rx_read_ptr + param_offset,' ') + 1; //find char after second space
            argumentNumber--;
        }
    }
    return param_offset;
}
//------------------------------------------------------------------------------------------------//
static void CoreTerm_PrintSwVersion(void)
{
    LOG_TRM(VERSION_STRING);
    LOG_TRM(BUILD_STRING);
}
#endif
//------------------------------------------------------------------------------------------------//
#if TERM_LEVEL > TERM_LEVEL_NO_HELP
static void CoreTerm_PrintHelp(void)
{
    U8  i;
    U8  j;
	U8  max_cmd_len = 0;
    BOOL     show_all;
    U8  temp;
    U8  item;
    STRING      coreterm_command;
    STRING      rx_string = coreterm_rx_read_ptr + 5;

    show_all = CoreString_StartsWith(rx_string, "/all");
    if(show_all)
    {
        rx_string += 5;
    }

	//print header
    LOG_TRM(RULER);
    LOG_TRM("Command Help [%c]", PCHARA(rx_string, CoreString_GetLength(rx_string)));
    LOG_TRM(RULER);
	
	//determine de max lengte van alle coreterm_commands (inclusief params abc letters)
    for(i = 0; i < coreterm_command_count; i++)
    {
		temp = CoreString_GetLength((STRING)coreterm_commands[i]) + (coreterm_cmd_arg_count[i]*2);
		if(temp > max_cmd_len)
		{
			max_cmd_len = temp;
		}
	}

	//print de coreterm_commands + helptexts
    for(i = 0; i < coreterm_command_count; i++)
    {
        item = i;
        coreterm_command = (STRING)coreterm_commands[item];

        CoreString_CopyString(coreterm_command,coreterm_temp_string);
        CoreString_ToLowerCase(coreterm_temp_string);

        if(CoreString_StartsWith(coreterm_temp_string, rx_string))
        {
            MEMSET((VPTR)coreterm_temp_string, ' ', COMMAND_LENGTH);
            if(coreterm_cmd_arg_count[item] == CMD_VAR_ARGUMENTS)
            {
                coreterm_temp_string[0] = '.';
                coreterm_temp_string[1] = '.';
                coreterm_temp_string[2] = '.';
                j = 3;
            }
            else
            {
                for(j = 0; j < coreterm_cmd_arg_count[item]; j++)
                {
                    coreterm_temp_string[(j<<1)+1] = 'a' + j;
                }
                j <<= 1;
            }
            LOG_TRM("%s%c",
                    PCSTR(coreterm_command),
                    PCHARA(coreterm_temp_string, j));
        }
    }
    LOG_TRM(RULER);
    if(show_all)
    {
        LOG_TRM("Use: %d/%d", PU8(coreterm_command_count), PU8(MAX_NUMBER_OF_COMMANDS));
    }
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CoreTerm_Init(SCI_CHANNEL_HNDL channel_hndl)
{
#if TERM_LEVEL > TERM_LEVEL_NONE
    MODULE_INIT_ONCE();

    MEMSET((VPTR)coreterm_rx_data, 0, SIZEOF(coreterm_rx_data));
    coreterm_rx_data_write_index = 0;
    coreterm_rx_data_read_index = 0;
    
    coreterm_rx_write_ptr = coreterm_rx_data[coreterm_rx_data_write_index];
    
    CoreTerm_SwitchSciChannel(channel_hndl);

#if TERM_LEVEL > TERM_LEVEL_NO_HELP
    CoreTerm_RegisterCommand("Help","Print HELP menu", 0, CoreTerm_PrintHelp, FALSE);
#endif

#if TERM_LEVEL > TERM_LEVEL_SELF
    CoreTerm_RegisterCommand("SwVersion","Print software version", 0, CoreTerm_PrintSwVersion, FALSE);
    CoreTerm_RegisterCommand("Reset","Perform soft reset", 0, System_Reset, FALSE);
    coreterm_command_hook = EventHandler_CommandReceived;
#else
    coreterm_command_hook = NULL;
#endif

    MODULE_INIT_DONE();
#endif
}
//------------------------------------------------------------------------------------------------//
void CoreTerm_SwitchSciChannel(SCI_CHANNEL_HNDL channel_hndl)
{
    SCI_CONFIG_STRUCT                sci_cfg_struct;
    
#if TERM_LEVEL > TERM_LEVEL_NONE
    DrvSciChannel_RegisterRxHook(coreterm_rx_channel_hndl, NULL);
    coreterm_rx_channel_hndl = channel_hndl;
    DrvSciChannel_RegisterRxHook(coreterm_rx_channel_hndl, CoreTermLight_SciRxNewByteHook);
#endif
    if(channel_hndl != NULL)
    {
        sci_cfg_struct.speed = TERMINAL_BAUDRATE;
        sci_cfg_struct.parity = SCI_PARITY_NONE;
        sci_cfg_struct.stopbit = SCI_STOPBIT_1;
        sci_cfg_struct.data_length = SCI_DATA_LENGTH_8_BITS;
        DrvSciChannel_Config(channel_hndl, &sci_cfg_struct);
    }
}
//------------------------------------------------------------------------------------------------//
void CoreTerm_Handler(void)
{
#if TERM_LEVEL > TERM_LEVEL_NONE
    coreterm_rx_read_ptr = coreterm_rx_data[coreterm_rx_data_read_index];
    
    if(coreterm_first_pass == TRUE)
    {
        CoreLog_Print(PROMPT_STRING, LOG_LEVEL_TERM, 0);
        coreterm_first_pass = FALSE;
    }
    
    if(coreterm_rx_data_write_index != coreterm_rx_data_read_index)
    {
        if(coreterm_unhandled_command == FALSE)
        {
            LOG_TRM("%c", PCHARA(coreterm_rx_read_ptr, CoreString_GetLength(coreterm_rx_read_ptr)));
            if(coreterm_command_hook != NULL)
            {
                coreterm_unhandled_command = TRUE;
                coreterm_command_hook(coreterm_rx_read_ptr, CoreString_GetLength(coreterm_rx_read_ptr));
            }
            else
            {
                CoreTerm_PrintFailed();
            }
        }
        else
        {
            CoreTerm_PrintFailed();
        }
        
        coreterm_unhandled_command = FALSE;
        CoreLog_Print(PROMPT_STRING, LOG_LEVEL_TERM, 0);
        MEMSET((VPTR)coreterm_rx_read_ptr, 0, COMMAND_LENGTH);
        coreterm_rx_data_read_index = coreterm_rx_data_write_index;
    }
#endif
}
//------------------------------------------------------------------------------------------------//
#if TERM_LEVEL == TERM_LEVEL_SELF
void CoreTerm_RegisterCommandHook(TERMHOOK_COMMAND_RECEIVED command_hook)
{
    coreterm_command_hook = command_hook;
}
#endif
//------------------------------------------------------------------------------------------------//
#if TERM_LEVEL > TERM_LEVEL_NONE
void CoreTerm_RegisterCommandWrap(STRING command, STRING helpText, U8 numberOfArguments, EVENT_CALLBACK coreterm_commandoCallback, BOOL show)
{
#if TERM_LEVEL > TERM_LEVEL_SELF
    if(coreterm_command_count < MAX_NUMBER_OF_COMMANDS)
    {
        coreterm_commands[coreterm_command_count] = command;
        coreterm_cmd_hooks[coreterm_command_count] = coreterm_commandoCallback;
#if TERM_LEVEL > TERM_LEVEL_NO_HELP
        coreterm_cmd_arg_count[coreterm_command_count] = numberOfArguments;
#endif
        coreterm_command_count++;
    }
#endif
}
//------------------------------------------------------------------------------------------------//
U32 CoreTerm_GetArgumentAsU32(U8 argumentNumber)
{
#if TERM_LEVEL > TERM_LEVEL_SELF
	U8 param_offset = CoreTerm_GetOffset(argumentNumber);

    if(param_offset)
    {
        return CoreConvert_StringToU32(coreterm_rx_read_ptr + param_offset);
    }
#endif
    return 0;
}
//------------------------------------------------------------------------------------------------//
S32 CoreTerm_GetArgumentAsS32(U8 argumentNumber)
{
#if TERM_LEVEL > TERM_LEVEL_SELF
	U8 param_offset = CoreTerm_GetOffset(argumentNumber);

    if(param_offset)
    {
        return CoreConvert_DecimalStringToS32(coreterm_rx_read_ptr + param_offset);
    }
#endif
    return 0;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTerm_GetArgumentAsBool(U8 argumentNumber)
{
#if TERM_LEVEL > TERM_LEVEL_SELF
	U8 param_offset = CoreTerm_GetOffset(argumentNumber);

    if(param_offset)
    {
        return CoreConvert_BoolStringToBool(coreterm_rx_read_ptr + param_offset);
    }
#endif
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void CoreTerm_GetArgumentAsString(U8 argumentNumber, STRING stringPointer)
{
#if TERM_LEVEL > TERM_LEVEL_SELF
	U8 param_offset = CoreTerm_GetOffset(argumentNumber);

    stringPointer[0] = 0;
    if(param_offset)
    {
        CoreString_SubString(coreterm_rx_read_ptr, stringPointer, param_offset, CoreString_FindChar(coreterm_rx_read_ptr + param_offset,' '));
    }
#endif
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTerm_GetByteStream(U8* data_ptr, U16* data_len_ptr, U16 timeout_in_ms)
{
    return FALSE;
}
#endif
//------------------------------------------------------------------------------------------------//
void CoreTerm_PrintAcknowledge(void)
{
    LOG_TRM("Done");
}
//------------------------------------------------------------------------------------------------//
void CoreTerm_PrintFailed(void)
{
    LOG_TRM("Failed");
}
//------------------------------------------------------------------------------------------------//
void CoreTerm_PrintFeedback(BOOL success)
{
    if(success)
    {
        CoreTerm_PrintAcknowledge();
    }
    else
    {
        CoreTerm_PrintFailed();
    }
}
//------------------------------------------------------------------------------------------------//
void CoreTerm_PrintBool(BOOL state)
{
    if(state)
    {
        LOG_TRM("TRUE");
    }
    else
    {
        LOG_TRM("FALSE");
    }
}
//================================================================================================//
