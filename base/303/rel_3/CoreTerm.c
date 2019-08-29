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
// @brief  Defines the RX queue size of the terminal
#ifndef TERM_QUEUE_SIZE
    #define TERM_QUEUE_SIZE                                     COMMAND_LENGTH
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
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the help should be flushed out, if the help is flushed out, the requirements for the log queue size drop drasic
#ifndef FLUSH_HELP
    #define FLUSH_HELP	                                        1
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the terminal interface should be fool proof, this means commands are normalised and checked if the parameters are correct
#ifndef FOOL_PROOF_TERMINAL
    #define FOOL_PROOF_TERMINAL	                                1
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines if the terminal interface should echo the received characters
#ifndef ECHO_ENABLED
    #define ECHO_ENABLED	                                    1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRIVER lib include section
#define Q_PREFIX(postfix)                   CoreTerm_##postfix
#define Q_SIZE                              TERM_QUEUE_SIZE
#include "sci\DrvSciQRxTpl.h"
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
#if TERM_LEVEL > TERM_LEVEL_SELF
static void EventHandler_CommandReceived(CHAR* coreterm_command_ptr, U8 length);
static U8 CoreTerm_GetOffset(U8 argumentNumber);
static void CoreTerm_PrintSwVersion(void);
static void CoreTerm_Timeout(VPTR data_ptr);
#endif

#if TERM_LEVEL > TERM_LEVEL_NO_HELP
static void CoreTerm_PrintHelp(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static TERMHOOK_COMMAND_RECEIVED        coreterm_command_hook;
static CHAR                             coreterm_rx_string[COMMAND_LENGTH + 1];
static CHAR*                            coreterm_rx_string_ptr;
static U8                               coreterm_ignore_char;

#if TERM_LEVEL > TERM_LEVEL_SELF
static CHAR                             coreterm_rx_command[COMMAND_LENGTH + 1];
static CHAR                             coreterm_temp_string[COMMAND_LENGTH + 1];

static U16                              coreterm_command_count = 0;

static U32                              coreterm_commands[MAX_NUMBER_OF_COMMANDS];
static U32                              coreterm_cmd_hooks[MAX_NUMBER_OF_COMMANDS];
static U8                               coreterm_cmd_sort[MAX_NUMBER_OF_COMMANDS];
static U8                               coreterm_cmd_hidden[MAX_NUMBER_OF_COMMANDS];

static TASK_HNDL                        coreterm_timeout_task_hndl;
static BOOL                             coreterm_timeout_passed;
#endif

#if TERM_LEVEL > TERM_LEVEL_NO_HELP
static U8                               coreterm_cmd_arg_count[MAX_NUMBER_OF_COMMANDS];
#endif

#if TERM_LEVEL > TERM_LEVEL_NO_HELP_TEXT
static U32                              coreterm_cmd_help[MAX_NUMBER_OF_COMMANDS];
#endif

static BOOL                             coreterm_first_pass = TRUE;
static BOOL                             coreterm_unhandled_command = FALSE;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if TERM_LEVEL > TERM_LEVEL_SELF
static void EventHandler_CommandReceived(CHAR* coreterm_command_ptr, U8 length)
{
    U8  i;
    U8  j;

    if(length == 0)
    {
        CoreLog_Print(PROMPT_STRING, LOG_LEVEL_TERM, 0);
        return;
    }
    #if (FOOL_PROOF_TERMINAL)
    {
        CoreString_Trim(coreterm_rx_string);    //trim string to remove all unwanted spaces at start and end
    }
    #endif

    //remove the arguments of coreterm_rx_string and put it in coreterm_rx_command
    CoreString_SubString(coreterm_rx_string, coreterm_rx_command, 0, CoreString_SearchChar(coreterm_rx_string,' '));

    //parse the coreterm_command and call the eventhandler
    for(i = 0; i < coreterm_command_count; i++)
    {
        //copy string to compare and converrt to lowercase
        CoreString_CopyString((STRING)(coreterm_commands[i]),coreterm_temp_string);
        CoreString_ToLowerCase(coreterm_temp_string);

        if(CoreString_Equals(coreterm_rx_command,coreterm_temp_string))
        {
            //check if the rx string has the number of parameters that are expected
            #if (FOOL_PROOF_TERMINAL)
            {
    			U8  temp_argument_count_found;
                CoreString_RemoveExcessiveChar(coreterm_rx_string,' '); //remove exessive spaces in command
                temp_argument_count_found = CoreString_CountChar(coreterm_rx_string, ' ');
                if (coreterm_cmd_arg_count[i] != temp_argument_count_found)
                {
                    if(coreterm_cmd_arg_count[i] != CMD_VAR_ARGUMENTS)  //skip this check if it is a var arguments cmd
                    {
                        //if not print error and return
                        CoreLog_Print("Incorrect Cmd Arguments (found %d, expected %d)\r"PROMPT_STRING, LOG_LEVEL_TERM, 3, PU8(temp_argument_count_found), PU8(coreterm_cmd_arg_count[i]));
                        return;
                    }
                }
            }
            #endif

            ((EVENT_CALLBACK)(coreterm_cmd_hooks[i]))();
            CoreLog_Print(PROMPT_STRING, LOG_LEVEL_TERM, 0);
            return;
        }
    }

    //als de code hier komt coreterm_command not found, print '^' op de next lijn to indicate waar het commando niet meer klopt
    i = 0;
    j = 0;
    while((i < CoreString_GetLength(coreterm_rx_command)) && (j < coreterm_command_count))
    {
        CoreString_SubString((STRING)(coreterm_commands[coreterm_cmd_sort[j]]), coreterm_temp_string, 0, i+1);
        CoreString_ToLowerCase(coreterm_temp_string);

        if((CoreString_GetLength(coreterm_temp_string) == i + 1) && CoreString_StartsWith(coreterm_rx_command, coreterm_temp_string))
        {
            i++;
            continue;
        }
        if(CoreString_IsAlphabeticBefore(coreterm_temp_string, coreterm_rx_command))
        {
            j++;
            continue;
        }
        break;
    }
    MEMSET((VPTR)coreterm_temp_string, ' ', i);
    coreterm_temp_string[i] = '^';
    LOG_TRM("%c", PCHARA(coreterm_temp_string, i+1));

    CoreLog_Print("Unknown Cmd\r"PROMPT_STRING, LOG_LEVEL_TERM, 0);
}
//------------------------------------------------------------------------------------------------//
static U8 CoreTerm_GetOffset(U8 argumentNumber)
{
	U8 param_offset = 0;

    if(CoreString_CountChar(coreterm_rx_string, ' ') > argumentNumber)
    {
        argumentNumber++;
        while(argumentNumber)
        {
            param_offset += CoreString_SearchChar(coreterm_rx_string + param_offset,' ') + 1; //find char after second space
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
//------------------------------------------------------------------------------------------------//
static void CoreTerm_Timeout(VPTR data_ptr)
{
    CoreTask_Stop(coreterm_timeout_task_hndl);
    coreterm_timeout_passed = TRUE;
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
    STRING      rx_string = coreterm_rx_string + 5;

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
        if(coreterm_cmd_arg_count[item] == CMD_VAR_ARGUMENTS)
        {
            temp = CoreString_GetLength((STRING)coreterm_commands[i]) + 4;
        }
        else
        {
            temp = CoreString_GetLength((STRING)coreterm_commands[i]) + (coreterm_cmd_arg_count[i]*2);
        }
		if(temp > max_cmd_len)
		{
			max_cmd_len = temp;
		}
	}

	//print de coreterm_commands + helptexts
    for(i = 0; i < coreterm_command_count; i++)
    {
        #if(HELP_SORTED)
        {
            item = coreterm_cmd_sort[i];
        }
        #else
        {
            item = i;
        }
        #endif
        coreterm_command = (STRING)coreterm_commands[item];

        CoreString_CopyString(coreterm_command,coreterm_temp_string);
        CoreString_ToLowerCase(coreterm_temp_string);

        if(((show_all) || (coreterm_cmd_hidden[item] == 0)) && (CoreString_StartsWith(coreterm_temp_string, rx_string)))
        {
            MEMSET((VPTR)coreterm_temp_string, ' ', COMMAND_LENGTH);
            if (coreterm_cmd_arg_count[item] == CMD_VAR_ARGUMENTS)
            {
                //print ... if it is a var argument
                coreterm_temp_string[1] = '.';
                coreterm_temp_string[2] = '.';
                coreterm_temp_string[3] = '.';
            }
            else
            {
                //prin a b c if the number of aruments is known
            for(j = 0; j < coreterm_cmd_arg_count[item]; j++)
            {
                coreterm_temp_string[(j<<1)+1] = 'a' + j;
            }
            }
            #if(TERM_LEVEL > TERM_LEVEL_NO_HELP_TEXT)
            {
                LOG_TRM("%s%c%s",
                        PCSTR(coreterm_command),
                        PCHARA(coreterm_temp_string, (max_cmd_len - CoreString_GetLength(coreterm_command) + MIN_CHAR_SPACE_BETWEEN_COMMANDS_AND_HELP)),
                        PCSTR(coreterm_cmd_help[item]));
            }
            #else
            {
                LOG_TRM("%s%c",
                        PCSTR(coreterm_command),
                        PCHARA(coreterm_temp_string, (max_cmd_len - CoreString_GetLength(coreterm_command))));
            }
            #endif
        }
        #if(FLUSH_HELP)
        {
            CoreLog_Flush();    //flush helptext out to minimise the requirements for the logqueue size, otherwise the main thing that wil determines the log queue size will be the size of the help
        }
        #endif
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
    MODULE_INIT_ONCE();

    CoreTerm_SciQRx_Create(channel_hndl);
    CoreTerm_SwitchSciChannel(channel_hndl);
    
    MEMSET((VPTR)coreterm_rx_string, 0, COMMAND_LENGTH + 1);
    coreterm_rx_string_ptr = coreterm_rx_string;
    coreterm_ignore_char = 0;

    #if(TERM_LEVEL > TERM_LEVEL_SELF)
    {
        coreterm_command_hook = EventHandler_CommandReceived;

        //init buffers
        MEMSET((VPTR)coreterm_rx_command, 0, COMMAND_LENGTH + 1);
        MEMSET((VPTR)coreterm_temp_string, 0, COMMAND_LENGTH + 1);
        MEMSET((VPTR)coreterm_commands, 0, SIZEOF(coreterm_commands));
        MEMSET((VPTR)coreterm_cmd_hooks, 0, SIZEOF(coreterm_cmd_hooks));
        MEMSET((VPTR)coreterm_cmd_sort, 0, MAX_NUMBER_OF_COMMANDS);
        MEMSET((VPTR)coreterm_cmd_hidden, 0, MAX_NUMBER_OF_COMMANDS);
        
        coreterm_timeout_task_hndl = INVALID_TASK_HNDL;
        coreterm_timeout_passed = FALSE;
    }
    #else
    {
        coreterm_command_hook = NULL;
    }
    #endif

    #if(TERM_LEVEL > TERM_LEVEL_NO_HELP_TEXT)
    {
        MEMSET((VPTR)coreterm_cmd_help, 0, SIZEOF(coreterm_cmd_help));
    }
    #endif

    #if(TERM_LEVEL > TERM_LEVEL_NO_HELP)
    {
        MEMSET((VPTR)coreterm_cmd_arg_count, 0, MAX_NUMBER_OF_COMMANDS);
        CoreTerm_RegisterCommand("Help","Print HELP menu", CMD_VAR_ARGUMENTS, CoreTerm_PrintHelp, FALSE);
    }
    #endif

    #if(TERM_LEVEL > TERM_LEVEL_SELF)
    {
        CoreTerm_RegisterCommand("SwVersion","Print software version", 0, CoreTerm_PrintSwVersion, FALSE);
        CoreTerm_RegisterCommand("Reset","Perform soft reset", 0, Core_Reset, FALSE);
    }
    #endif

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CoreTerm_SwitchSciChannel(SCI_CHANNEL_HNDL channel_hndl)
{
    SCI_CONFIG_STRUCT                sci_cfg_struct;

    CoreTerm_SciQRx_SwitchSciChannel(channel_hndl);
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
    U8 cmd_char;

    if(coreterm_first_pass == TRUE)
    {
        CoreLog_Print(PROMPT_STRING, LOG_LEVEL_TERM, 0);
        coreterm_first_pass = FALSE;
    }
    
    if(coreterm_unhandled_command == TRUE)
    {
        coreterm_unhandled_command = FALSE;
        CoreTerm_PrintFailed();
        CoreLog_Print(PROMPT_STRING, LOG_LEVEL_TERM, 0);
        MEMSET((VPTR)coreterm_rx_string, 0, COMMAND_LENGTH);
        coreterm_rx_string_ptr = coreterm_rx_string;
    }

    while(CoreTerm_SciQRx_Read(&cmd_char, 1))
    {
        if(coreterm_ignore_char)
        {
            coreterm_ignore_char--;
        }
        else
        {
            if((cmd_char >= 'A') && (cmd_char <= 'Z'))
            {
                cmd_char += 'a' - 'A';
            }

            switch(cmd_char)
            {
            case 0x08:  // backspace
                if(coreterm_rx_string_ptr > coreterm_rx_string)
                {
                    #if (ECHO_ENABLED == 1)
                    {
                        CoreLog_Print("\b \b", LOG_LEVEL_TERM, 0);
                    }
                    #endif
                    coreterm_rx_string_ptr--;
                    *coreterm_rx_string_ptr = 0x00;
                }
                break;
            case 0x1B: // pijltjes en zo
                coreterm_ignore_char = 2;
                break;
            case '\n': // line feed
                break;
            case '\r': // carriage return
                if(coreterm_rx_string_ptr != coreterm_rx_string)
                {
                    #if (ECHO_ENABLED == 1)
                    {
                        LOG_TRM("");
                    }
                    #endif
                }
                coreterm_unhandled_command = TRUE;
                if(coreterm_command_hook != NULL)
                {
                    coreterm_command_hook(coreterm_rx_string, coreterm_rx_string_ptr - coreterm_rx_string);
                    coreterm_unhandled_command = FALSE;
                }
                MEMSET((VPTR)coreterm_rx_string, 0, COMMAND_LENGTH);
                coreterm_rx_string_ptr = coreterm_rx_string;
                return;
            case 0x09: // tab
            case 0x20: // spatie
                if((coreterm_rx_string_ptr - coreterm_rx_string) == 0)
                {
                    break;
                }
                //no break;
            default:
                if(coreterm_rx_string_ptr < &coreterm_rx_string[COMMAND_LENGTH])
                {
                    *coreterm_rx_string_ptr++ = (CHAR) cmd_char;
                    #if (ECHO_ENABLED == 1)
                    {
                        CoreLog_Print("%c", LOG_LEVEL_TERM, 2, PCHAR(cmd_char));
                    }
                    #endif
                }
                break;
            }
        }
    }
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
    #if(TERM_LEVEL > TERM_LEVEL_SELF)
    {
    U8  i;
    U8  j;

    if(coreterm_command_count < MAX_NUMBER_OF_COMMANDS)
    {
        coreterm_commands[coreterm_command_count] = (U32)command;
        #if (TERM_LEVEL > TERM_LEVEL_NO_HELP_TEXT)
        {
            coreterm_cmd_help[coreterm_command_count] = (U32)helpText;
        }
        #endif
        #if (TERM_LEVEL > TERM_LEVEL_NO_HELP)
        {
            coreterm_cmd_arg_count[coreterm_command_count] = numberOfArguments;
        }
        #endif
        coreterm_cmd_hooks[coreterm_command_count] = (U32)coreterm_commandoCallback;
        coreterm_cmd_hidden[coreterm_command_count] = (U8)(show == FALSE);

        CoreString_CopyString(command, coreterm_rx_command);
        CoreString_ToLowerCase(coreterm_rx_command);

        for(i = 0; i <= coreterm_command_count; i++)
        {
            CoreString_CopyString((STRING)(coreterm_commands[coreterm_cmd_sort[i]]),coreterm_temp_string);
            CoreString_ToLowerCase(coreterm_temp_string);
                if((i == coreterm_command_count) || (CoreString_IsAlphabeticBefore(coreterm_rx_command, coreterm_temp_string)))
            {
                for(j = coreterm_command_count; j > i; j--)
                {
                    coreterm_cmd_sort[j] = coreterm_cmd_sort[j-1];
                }
                coreterm_cmd_sort[i] = coreterm_command_count;
                break;
            }
        }

        coreterm_command_count++;
    }
    }
#endif
}
//------------------------------------------------------------------------------------------------//
U32 CoreTerm_GetArgumentAsU32(U8 argumentNumber)
{
    #if(TERM_LEVEL > TERM_LEVEL_SELF)
    {
        U8 param_offset = CoreTerm_GetOffset(argumentNumber);

        if(param_offset)
        {
            return CoreConvert_StringToU32(coreterm_rx_string + param_offset);
        }
    }
    #endif
    return 0;
}
//------------------------------------------------------------------------------------------------//
S32 CoreTerm_GetArgumentAsS32(U8 argumentNumber)
{
    #if(TERM_LEVEL > TERM_LEVEL_SELF)
    {
        U8 param_offset = CoreTerm_GetOffset(argumentNumber);

        if(param_offset)
        {
            return CoreConvert_DecimalStringToS32(coreterm_rx_string + param_offset);
        }
    }
    #endif
    return 0;
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTerm_GetArgumentAsBool(U8 argumentNumber)
{
    #if(TERM_LEVEL > TERM_LEVEL_SELF)
    {
        U8 param_offset = CoreTerm_GetOffset(argumentNumber);

        if(param_offset)
        {
            return CoreConvert_BoolStringToBool(coreterm_rx_string + param_offset);
        }
    }
    #endif
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void CoreTerm_GetArgumentAsString(U8 argumentNumber, STRING stringPointer)
{
    #if (TERM_LEVEL > TERM_LEVEL_SELF)
    {
        U8 param_offset = CoreTerm_GetOffset(argumentNumber);

        stringPointer[0] = 0;
        if(param_offset)
        {
            CoreString_SubString(coreterm_rx_string, stringPointer, param_offset, CoreString_SearchChar(coreterm_rx_string + param_offset,' '));
        }
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
void CoreTerm_GetAllArgumentAsString(STRING stringPointer)
{
    #if (TERM_LEVEL > TERM_LEVEL_SELF)
    {
        U8 param_offset = CoreTerm_GetOffset(0);
        U8 length;       
        if(param_offset)
        {
           length = CoreString_GetLength(coreterm_rx_string + param_offset);
           CoreString_SubString(coreterm_rx_string, stringPointer, param_offset, length);
        }
    }
    #endif
}
//------------------------------------------------------------------------------------------------//
BOOL CoreTerm_GetByteStream(U8* data_ptr, U16* data_len_ptr, U16 timeout_in_ms)
{
    #if (TERM_LEVEL > TERM_LEVEL_SELF)
    {
        U16 count  = *data_len_ptr;
        U16 qsize;
        U16 qsize_prev;
        
        if(coreterm_timeout_task_hndl == INVALID_TASK_HNDL)
        {
            coreterm_timeout_task_hndl = CoreTask_RegisterTask(1000, CoreTerm_Timeout, NULL, 100, "TERM_TO");
        }
        
        if(coreterm_timeout_task_hndl != INVALID_TASK_HNDL)
        {
            coreterm_timeout_passed = FALSE;
            CoreTask_SetPeriod(coreterm_timeout_task_hndl, (U32)timeout_in_ms * 1000);
            CoreTask_Start(coreterm_timeout_task_hndl);
            qsize = CoreTerm_SciQRx_GetCount();
            qsize_prev = qsize;
            
            while((qsize < count) && (coreterm_timeout_passed == FALSE))
            {
                System_KickDog();
                
                // if Q is growing, kick timeout task
                qsize = CoreTerm_SciQRx_GetCount();
                if(qsize > qsize_prev)
                {
                    CoreTask_Start(coreterm_timeout_task_hndl);
                    qsize_prev = qsize;
                }
                
                // avoid Q overflow
                if(qsize > (TERM_QUEUE_SIZE - 10))
                {
                    if(count < qsize)
                    {
                        qsize = count;
                    }
                    CoreTerm_SciQRx_Read(data_ptr, qsize);
                    data_ptr += qsize;
                    count -= qsize;
                    qsize = 0;
                    qsize_prev = 0;
                }
            }
            CoreTask_Stop(coreterm_timeout_task_hndl);
            
            // read data from Q
            if(count < qsize)
            {
                qsize = count;
            }
            CoreTerm_SciQRx_Read(data_ptr, qsize);
            data_ptr += qsize;
            count -= qsize;
            
            // subtrack left-over count
            *data_len_ptr -= count;
            return (BOOL)((count == 0) && (coreterm_timeout_passed == FALSE));
        }
    }
    #endif
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
