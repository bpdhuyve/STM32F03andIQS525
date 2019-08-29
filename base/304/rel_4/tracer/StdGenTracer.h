//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Header file for the general tracer module, usefull for all types of communication
// Header file for the general tracer module, usefull for all types of communication.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef TRACER__STDGENTRACER_H
#define TRACER__STDGENTRACER_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief   the max nr of characters of a predefined name
#ifndef STDGENTRACER_NAME_LENGTH
    #error "STDGENTRACER_NAME_LENGTH not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// @brief   USED definition
#define USED                    128

// @brief   UNUSED definition
#define UNUSED                  0

// @brief   signed byte size definition
#define SIZE_IS_SBYTE           1

// @brief   signed word size definition
#define SIZE_IS_SWORD           2

// @brief   signed long size definition
#define SIZE_IS_SLONG           3

// @brief   unsigned byte size definition
#define SIZE_IS_UBYTE           4

// @brief   unsigned word size definition
#define SIZE_IS_UWORD           5

// @brief   unsigned long size definition
#define SIZE_IS_ULONG           6
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   trace control enumeration
typedef enum
{
    INIT_TRACE  	= 0,
    STOP_TRACE		= 1,
    START_TRACE		= 2
}
TRACE_CONTROL;

// @brief   trigger mode enumeration
typedef enum
{
    NO_TRIGGER  	= 0,
    CENTRE_TRIGGER	= 1,
    POST_TRIGGER	= 2,
    PRE_TRIGGER		= 3
}
TRIGGER_MODE;

// @brief   trigger operator enumeration
typedef enum
{
	TRIGGER_EQUAL_DATA        	=	0,
	TRIGGER_BITS_HIGH           =	1,
	TRIGGER_BITS_LOW            =	2,
	TRIGGER_LOWER_DATA          =	3,
	TRIGGER_HIGHER_DATA         =	4,
	TRIGGER_SIGNED_LOWER_DATA   =	5,
	TRIGGER_SIGNED_HIGHER_DATA  =	6,
 	TRIGGER_NOT_EQUAL_DATA      =	7
}
TRIGGER_OPERATOR;

// @brief   address definition
typedef union
{
    // @brief   byte address definition
    S8*  byte_ptr;

    // @brief   word address definition
    S16* word_ptr;

    // @brief   long address definition
    S32* long_ptr;
}
ADDR;

// @brief   trace specification definition
typedef struct
{
    // @brief   trace status definition
    U16 		status;

    // @brief   trace address definition
    ADDR     			address;
}
TRACE_SPEC;

// @brief   trigger specification definition
typedef struct
{
    // @brief   trigger status definition
    U16 		status;

    // @brief   trigger address definition
	ADDR     			address;

    // @brief   trigger operation definition
    TRIGGER_OPERATOR 	operation;

    // @brief   trigger data definition
    S32   		data;
}
TRIGGER_SPEC;

// @brief   predefineds struct definition
typedef struct
{
    // @brief	predefineds name definition
    CHAR         	name[STDGENTRACER_NAME_LENGTH];

    // @brief   predefineds address pointer definition
    U16* 	address_ptr;

    // @brief   predefineds status definition
    U16  	info;
}
PREDEF;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief   Initialiser for the general tracer module
// Initialises and registers this module to the Module Manager.\n
// @param   predefs_ptr  	pointer to the table with variables you want to trace
// @param   length_predefs  the number of predefs in the table
void StdGenTracer_Init(const PREDEF* predefs_ptr, U8 length_predefs);

// @brief   Background handler for processing the traced messages
void StdGenTracer_Handler(void);

// @brief   Function to trace variables. This function is only useful when the trace is running
void StdGenTracer_Trace(void);

// @brief	Memory editor function to write a byte in memory
// @param	address	: the destination address in memory
// @param	data	: the data byte to be written in memory
void StdGenTracer_SetMemoryByte(U32 address, U8 data);

// @brief	Memory editor function to read a byte from memory
// @param	address	: the memory address
// @return	the data byte read from memory
U8 StdGenTracer_GetMemoryByte(U32 address);

// @brief	Memory editor function to write a word in memory
// @param	address	: the destination address in memory
// @param	data	: the data word to be written in memory
void StdGenTracer_SetMemoryWord(U32 address, U16 data);

// @brief	Memory editor function to read a word from memory
// @param	address	: the memory address
// @return	the data word read from memory
U16 StdGenTracer_GetMemoryWord(U32 address);

// @brief	Memory editor function to write a long word in memory
// @param	address	: the destination address in memory
// @param	data	: the data long word to be written in memory
void StdGenTracer_SetMemoryLong(U32 address, U32 data);

// @brief	Memory editor function to read a long word from memory
// @param	address	: the memory address
// @return	the data long word read from memory
U32 StdGenTracer_GetMemoryLong(U32 address);

// @brief   Function for retrieving a segment of a predefined trace variable name
// @param	predef_nr 	: the nr of the predefined in the predefineds table
// @param	segment_nr	: the nr of the segment in the name (0,1 ...)
// @param	char_length	: the character length of one segment
// @param	data_ptr	: pointer to the data (byte length = char_length)
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_GetPredefinedName(U8 	predef_nr,
						 			  U8 	segment_nr,
						 			  U8 	char_length,
						 			  U8* 	data_ptr);

// @brief   Function for retrieving the address of a predefined trace variable
// @param	predef_nr 	: the nr of the predefined in the predefineds table
// @param	address_ptr	: pointer to the address
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_GetPredefinedAddress(U8 predef_nr, U32* address_ptr);

// @brief   Function for retrieving the info/status/flags of a predefined trace variable
// @param	predef_nr 	: the nr of the predefined in the predefineds table
// @param	data_ptr	: pointer to the data
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_GetPredefinedInfo(U8 predef_nr, U8* data_ptr);

// @brief   Function for retrieving the status of the tracer
// The status is one of the definitions: TRACE_COMPLETE (0x00), WAITING_FOR_TRIGGER (0x10), TRACE_BUSY (0x20)
// eventually ored with the flag DATA_AVAILABLE (0x08)
// @return	the trace status
// @remark	This status is also given as return value of the function \ref StdGenTracer_GetTracedData
U8 StdGenTracer_GetTraceStatus(void);

// @brief   Function to init, start or stop a trace
// @param	control	: a command from the enumeration \ref TRACE_CONTROL
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_SetTraceControl(TRACE_CONTROL control);

// @brief   Function to set the time base of the tracer
// @param	data : the timebase: data = x means that the values are stored in a buffer every x times
//                 the routine \ref StdGenTracer_Trace is called
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_SetTimeBase(U16 data);

// @brief   Function to set the time base of the tracer
// @return	the time base of the tracer
U16 StdGenTracer_GetTimeBase(void);

// @brief   Function to set a trace channel configuration
// @param	channel_nb 	: the nr of the trace channel
// @param	address		: the address of the variable for this channel
// @param	info		: the size/sign of the variable (byte/word/long, signed/unsigned)
//						  ored with the 'used/unused' flag (0x80)
// @remark	If the MSB of the address is 1, then the address &~MSB is the index in the predefineds table
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_SetTraceChannelSettings(U8 channel_nb, U32 address, U8 info);

// @brief   Function to get a trace channel configuration
// @param	channel_nb 	: the nr of the trace channel
// @param	address_ptr	: a pointer to the address of the variable for this channel
// @param	info_ptr	: a pointer to the info of this trace variable (size, sign and used/unused info)
// @remark	If the MSB of the address is 1, then the address &~MSB is the index in the predefineds table
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_GetTraceChannelSettings(U8 channel_nb, U32* address_ptr, U8* info_ptr);

// @brief   Function to get traced data
// @param	data_ptr	: a pointer to the traced data
// @return	the status of the tracer (see also \ref StdGenTracer_GetTraceStatus)
U8 StdGenTracer_GetTracedData(U8* data_ptr);

// @brief   Function to set a trigger channel configuration
// @param	trigger_nb 	: the nr of the trigger channel
// @param	address		: the address of the variable for this channel
// @param	info		: the size/sign of the variable (byte/word/long, signed/unsigned)
//						  ored with the 'used/unused' flag (0x80)
// @remark	If the MSB of the address is 1, then the address &~MSB is the index in the predefineds table
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_SetTriggerChannelSettings(U8 trigger_nb, U32 address, U8 info);

// @brief   Function to get a trigger channel configuration
// @param	trigger_nb 	: the nr of the trigger channel
// @param	address_ptr	: a pointer to the address of the variable for this channel
// @param	info_ptr	: a pointer to the info of this trigger variable (size, sign and used/unused info)
// @remark	If the MSB of the address is 1, then the address &~MSB is the index in the predefineds table
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_GetTriggerChannelSettings(U8 trigger_nb, U32* address_ptr, U8* info_ptr);

// @brief   Function to set a trigger channel data
// @param	trigger_nb 	: the nr of the trigger channel
// @param	data		: the data for the trigger
// @param	operation	: the operation done with the data (greater then, lower then, ...: see \ref TRIGGER_OPERATOR)
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_SetTriggerChannelData(U8 trigger_nb, S32 data, TRIGGER_OPERATOR operation);

// @brief   Function to get a trigger channel data
// @param	trigger_nb 		: the nr of the trigger channel
// @param	data_ptr		: a pointer to the data for the trigger
// @param	operation_ptr	: a pointer to the operation done with the data (see \ref TRIGGER_OPERATOR)
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_GetTriggerChannelData(U8 trigger_nb, U32* data_ptr, U8* operation_ptr);

// @brief   Function to set the trigger mode
// @param	mode 	: the trigger mode (see enumeration \ref TRIGGER_MODE)
// @return	TRUE or FALSE if successfull or not
BOOL StdGenTracer_SetTriggerMode(TRIGGER_MODE mode);

// @brief   Function to get the trigger mode
// @return	the trigger mode (see enumeration \ref TRIGGER_MODE)
TRIGGER_MODE StdGenTracer_GetTriggerMode(void);

// @brief   Function to get the trigger position
// @return	the trigger position (i.e. the record nr at which the trigger occurred)
U16 StdGenTracer_GetTriggerPosition(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* DEBUG__TRACER__STDGENTRACER_H */
