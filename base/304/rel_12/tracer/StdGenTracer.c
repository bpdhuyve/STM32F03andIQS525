//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This file contains the general tracer module, usefull for all types of communication
// This file contains the general tracer module, usefull for all types of communication
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TRACER__STDGENTRACER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STDGENTRACER_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               STDGENTRACER_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief   max nr of trace channels
#ifndef STDGENTRACER_MAX_TRACE_CHANNELS
    #error "STDGENTRACER_MAX_TRACE_CHANNELS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   max nr of trigger channels
#ifndef STDGENTRACER_MAX_TRIGGER_CHANNELS
    #error "STDGENTRACER_MAX_TRIGGER_CHANNELS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   the nr of buffers of each trace channel
#ifndef STDGENTRACER_NB_OF_BUFFERS
    #error "STDGENTRACER_NB_OF_BUFFERS not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   the length (in words) of the buffer which is filled in background
#ifndef STDGENTRACER_BUFFER_LENGTH
    #error "STDGENTRACER_BUFFER_LENGTH not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   the data length (in words) of each 'traced data' transfer
#ifndef STDGENTRACER_WORD_LENGTH
    #error "STDGENTRACER_WORD_LENGTH not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STD lib include section
#include "tracer\StdGenTracer.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define USED_UNUSED_MASK        		128

#define DATA_AVAILABLE                  0x08
#define NO_DATA_AVAILABLE               0x00

#define TRACE_COMPLETE                  0x00
#define WAITING_FOR_TRIGGER             0x10
#define TRACE_BUSY                      0x20

#define PREDEFINED_ADDRESS_MASK     	0x80000000

#define TRIGGER_NOT_FOUND       		0
#define TRIGGER_FOUND           		1

#define ACCESS_MASK             		0x80
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void StdGenTracer_StartTrace(void);
static void StdGenTracer_CheckTrigger(void);
static void StdGenTracer_StopTrace(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static U8                       trace_status;
static TRACE_CONTROL            trace_start_stop;
static U16                      trace_counter;

static U8                       trigger_status;
static TRIGGER_MODE             trigger_mode;
static U16                      trigger_position;

static U16                      time_base;

static S16*                     address_small_buf_ptr;
static S16                      msec_trace_buffer[STDGENTRACER_NB_OF_BUFFERS][STDGENTRACER_MAX_TRACE_CHANNELS]@ "ram_eth_usb_data";;
static U16                      msec_read_pointer;
static U16                      msec_write_pointer;

static S16*                     address_big_buf_ptr;
static S16                      big_buffer[STDGENTRACER_BUFFER_LENGTH];
static U32                      read_pointer;
static U32                      write_pointer;

static U16                      record_words_read;
static U16                      record_length;

static U16                      number_of_records;
static U16                      maximum_number_of_records;

static U32                      trace_buffer_end;

static TRACE_SPEC               trace_specs[STDGENTRACER_MAX_TRACE_CHANNELS]@ "ram_eth_usb_data";
static TRIGGER_SPEC             trigger_specs[STDGENTRACER_MAX_TRIGGER_CHANNELS]@ "ram_eth_usb_data";

static const PREDEF*            predefineds_ptr;

static U8                       stdtracer_nb_of_predefs;
static BOOL			ext_triggered;
static U16		        nr_of_pre_trigger_samples;
static U16		        remaining_samples_to_be_taken;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void StdGenTracer_StartTrace(void)
{
   U32 i;
   U16 length;
   
   trace_status = WAITING_FOR_TRIGGER;  // to start tracing again

   read_pointer = 0;                    // reset all read and write pointers
   write_pointer = 0;
   msec_write_pointer = 0;
   msec_read_pointer = 0;
   record_words_read = 0;

   trigger_status = TRIGGER_NOT_FOUND;  // no trigger found yet
   trigger_position = 0;
   trace_counter = 1;
   ext_triggered = FALSE;
   nr_of_pre_trigger_samples = 0;
   remaining_samples_to_be_taken = 65535;


   length = 0;

   // calculate record length :
   // The record length is always positive
   // --------------------------------------
   i = 0;
   while ((i < STDGENTRACER_MAX_TRACE_CHANNELS) && (trace_specs[i].status & USED_UNUSED_MASK) != UNUSED)
   {
       switch (trace_specs[i].status)
       {
       case USED + SIZE_IS_SBYTE:
       case USED + SIZE_IS_UBYTE:
           length++;   //always use WORD size
           break;
       case USED + SIZE_IS_SWORD:
       case USED + SIZE_IS_UWORD:
           length++;   //always use WORD size
           break;
       case USED + SIZE_IS_SLONG:
       case USED + SIZE_IS_ULONG:
           length += 2;
           break;
       default:
           i = STDGENTRACER_MAX_TRACE_CHANNELS - 1;
           length = 0;
           break;
       }
       i++;
   }

   // calculate number of records and the trace-buffer end
   // ----------------------------------------------------
   if(length != 0)
   {
       number_of_records = 0;
       maximum_number_of_records = STDGENTRACER_BUFFER_LENGTH / length;
       trace_buffer_end = maximum_number_of_records * length;
       record_length = length; //record length in words
       
       //Default values in case of ext triggering mode: will be overwritten at setting the ext trigger. 
       //If an internal trigger is found before ext trigger is set, this will act as a centre trigger
       nr_of_pre_trigger_samples = maximum_number_of_records >> 1;
       if(trigger_mode == EXT_TRIGGER)
       {
           remaining_samples_to_be_taken = 1000;
       }
       
       trace_start_stop = START_TRACE;
   }
}
//------------------------------------------------------------------------------------------------//
static void StdGenTracer_CheckTrigger(void)
{
    U32 i;
    S32   data_read;
    S32   data;

    if(trigger_status == TRIGGER_NOT_FOUND)
    {
        if((ext_triggered == TRUE) && (trigger_mode == EXT_TRIGGER))
    	{
    		trigger_status = TRIGGER_FOUND;
    		ext_triggered = FALSE;
    	}
        /*
           As long as a the trigger is not found, check 'm check if T1+T2+T3+T4 is valid
           When one of the used triggers becomes valid, T5+T6+T7+T8 is checked.
           If T1 is not used, then there is no trigger defined and we set the
           trigger-status to TRIGGER_FOUND and the trigger-mode to NO_TRIGGER.
           ------------------------------------------------------------------------------
        */
        else if(((trigger_specs[0].status & USED_UNUSED_MASK) != UNUSED) && (trigger_mode != NO_TRIGGER))
        {
            i = 0;
            //check half of the triggers
            while( (i < (STDGENTRACER_MAX_TRIGGER_CHANNELS / 2))
                 && ((trigger_specs[i].status & USED_UNUSED_MASK) != UNUSED)
                 && (trigger_status == TRIGGER_NOT_FOUND))
            {
                 //vb gebruiker stelt trigger 1 in op position > 25
                 //dan is trigger_specs[0].data = 25
                 switch (trigger_specs[i].status)
                 {
                 case SIZE_IS_SBYTE + USED:
                    data_read = (S8) *(trigger_specs[i].address.byte_ptr);
                    data = (S8)trigger_specs[i].data;
                    break;
                 case SIZE_IS_UBYTE + USED:
                    data_read = (U8) *(trigger_specs[i].address.byte_ptr);
                    data = (U8) trigger_specs[i].data;
                    break;
                 case SIZE_IS_SWORD + USED:
                    data_read = (S16) *(trigger_specs[i].address.word_ptr);
                    data = (S16) trigger_specs[i].data;
                    break;
                 case SIZE_IS_UWORD + USED:
                    data_read = (U16) *(trigger_specs[i].address.word_ptr);
                    data = (U16) trigger_specs[i].data;
                    break;
                 case SIZE_IS_SLONG + USED:
                    data_read = (S32) *trigger_specs[i].address.long_ptr;
                    data = (S32) trigger_specs[i].data;
                    break;
                 case SIZE_IS_ULONG + USED:
                    data_read = (U32) *trigger_specs[i].address.long_ptr;
                    data = (U32) trigger_specs[i].data;
                    break;
                 default:
                    break;
                 }

                switch(trigger_specs[i].operation)
                {
                case TRIGGER_EQUAL_DATA:
					if(data == data_read)
					{
						trigger_status = TRIGGER_FOUND;
					}
                   	break;
				case TRIGGER_NOT_EQUAL_DATA:
					if(data != data_read)
					{
						trigger_status = TRIGGER_FOUND;
					}
					break;
				case TRIGGER_BITS_HIGH:
					if((data & data_read) == data)
					{
						trigger_status = TRIGGER_FOUND;
					}
					break;
				case TRIGGER_BITS_LOW:
					if((data & data_read) == 0)
					{
						trigger_status = TRIGGER_FOUND;
					}
					break;
				case TRIGGER_LOWER_DATA:
					if((U32)data_read < (U32)data)
					{
						trigger_status = TRIGGER_FOUND;
					}
					break;
				case TRIGGER_HIGHER_DATA:
					if((U32)data_read > (U32)data)
					{
						trigger_status = TRIGGER_FOUND;
					}
					break;
				case TRIGGER_SIGNED_LOWER_DATA:
					if(data_read < data)
					{
						trigger_status = TRIGGER_FOUND;
					}
					break;
				case TRIGGER_SIGNED_HIGHER_DATA:
					if(data_read > data)
					{
						trigger_status = TRIGGER_FOUND;
					}
					break;
				default:
					break;
                }
                i++;
            } // end while loop

            /*
            If T1+T2+T3+T4 is valid them check if T5+T6+T7+T8 is also valid
            If T5 is not used, then the trigger-status is TRIGGER_FOUND.
            ---------------------------------------------------------------
            */
            if(trigger_status == TRIGGER_FOUND)
            {
                /*
                check if T5+T6+T7+T8 is valid
                When one of the used triggers becomes valid, the trigger-status becomes
                = TRIGGER_FOUND
                --------------------------------------------------------------------------
                */
                if((trigger_specs[STDGENTRACER_MAX_TRIGGER_CHANNELS / 2].status & USED_UNUSED_MASK) != UNUSED)
                {
                    trigger_status = TRIGGER_NOT_FOUND; // suppose trigger not found
                    i = STDGENTRACER_MAX_TRIGGER_CHANNELS / 2;

                    while(   (i < (STDGENTRACER_MAX_TRIGGER_CHANNELS))
                          && ((trigger_specs[i].status & USED_UNUSED_MASK) != UNUSED)
                          && (trigger_status == TRIGGER_NOT_FOUND))
                    {
                        switch (trigger_specs[i].status)
                        {
                             case SIZE_IS_SBYTE + USED:
                                data_read = (S8) *trigger_specs[i].address.byte_ptr;
                                data = (S8) trigger_specs[i].data;
                                break;
                             case SIZE_IS_UBYTE + USED:
                                data_read = (U8) *trigger_specs[i].address.byte_ptr;
                                data = (U8) trigger_specs[i].data;
                                break;
                             case SIZE_IS_SWORD + USED:
                                data_read = (S16) *trigger_specs[i].address.word_ptr;
                                data = (S16) trigger_specs[i].data;
                                break;
                             case SIZE_IS_UWORD + USED:
                                data_read = (U16) *trigger_specs[i].address.word_ptr;
                                data = (U16) trigger_specs[i].data;
                                break;
                             case SIZE_IS_SLONG + USED:
                                data_read = (S32) *trigger_specs[i].address.long_ptr;
                                data = (S32) trigger_specs[i].data;
                                break;
                             case SIZE_IS_ULONG + USED:
                                data_read = (U32) *trigger_specs[i].address.long_ptr;
                                data = (U32) trigger_specs[i].data;
                                break;
                             default:
                                //cop register handlen
                                break;
                        }
                        switch (trigger_specs[i].operation)
                        {
                            case TRIGGER_EQUAL_DATA:
                                if(data == data_read)
                                {
                                    trigger_status = TRIGGER_FOUND;
                                }
                                break;
                            case TRIGGER_NOT_EQUAL_DATA:
                                if(data != data_read)
                                {
                                    trigger_status = TRIGGER_FOUND;
                                }
                                break;
                            case TRIGGER_BITS_HIGH:
                                if((data & data_read) == data)
                                {
                                    trigger_status = TRIGGER_FOUND;
                                }
                                break;
                            case TRIGGER_BITS_LOW:
                                if((data & data_read) == 0)
                                {
                                    trigger_status = TRIGGER_FOUND;
                                }
                                break;
                            case TRIGGER_LOWER_DATA:
                                if((U32)data_read < (U32)data)
                                {
                                    trigger_status = TRIGGER_FOUND;
                                }
                                break;
                            case TRIGGER_HIGHER_DATA:
                                if((U32)data_read > (U32)data)
                                {
                                    trigger_status = TRIGGER_FOUND;
                                }
                                break;
                            case TRIGGER_SIGNED_LOWER_DATA:
                                if(data_read < data)
                                {
                                    trigger_status = TRIGGER_FOUND;
                                }
                                break;
                            case TRIGGER_SIGNED_HIGHER_DATA:
                                if(data_read > data)
                                {
                                    trigger_status = TRIGGER_FOUND;
                                }
                                break;
                            default:
                                //cop register handlen
                                break;
                        }
                        i++;
                    } // end while loop
                }
            } //end if (trigger_status == TRIGGER_FOUND)
        }
        else
        {
            //If no external trigger mode active and no trigger is specified then start tracing until buffer is full.
            //The trigger-status is set to TRIGGER_FOUND and the trigger-mode is set to NO_TRIGGER.
            if(trigger_mode != EXT_TRIGGER)
            {
            	trigger_mode = NO_TRIGGER;
            	trigger_status = TRIGGER_FOUND;
            }
        }
        if(trigger_status == TRIGGER_FOUND)
        {
            if(trigger_mode == EXT_TRIGGER)
            {
            	//if already more samples available in buffer then desired, adapt this to the desired value and 
            	//adapt read pointer to difference of both
                if(number_of_records > nr_of_pre_trigger_samples) 
            	{
                    read_pointer += ((number_of_records - nr_of_pre_trigger_samples) * record_length);
                    if(read_pointer >= trace_buffer_end)
                    {
                            read_pointer -= trace_buffer_end;
                    }
                    number_of_records = nr_of_pre_trigger_samples;
            	}
            }
            
            trace_status = TRACE_BUSY;
            trigger_position = (U16)(number_of_records * record_length * 2);
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void StdGenTracer_StopTrace(void)
{
    trace_start_stop = STOP_TRACE;      // stopped
    trace_status = TRACE_COMPLETE;      // there can be some data if trace was busy
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void StdGenTracer_Init(const PREDEF* predefs_ptr, U8 length_predefs)
{
    U8  i;

    predefineds_ptr = predefs_ptr;
    stdtracer_nb_of_predefs = length_predefs;

    MODULE_INIT_ONCE();
    StdGenTracer_StopTrace();

    trigger_mode = NO_TRIGGER;
    trigger_status = TRIGGER_NOT_FOUND;
    trigger_position = 0;
    ext_triggered = FALSE;
    nr_of_pre_trigger_samples = 0;
    remaining_samples_to_be_taken = 65535;

    trace_status = TRACE_COMPLETE;
    number_of_records = 0;

    time_base = 1;          	//1 ms, used for tracing on interrupt

    msec_write_pointer = 0; 	//read and write pointer for the msec tracer
    msec_read_pointer = 0;

    read_pointer = 0;       	//read and write pointer for the big buffer
    write_pointer = 0;

    record_words_read = 0;

    address_big_buf_ptr = (S16*)&big_buffer[0];

    // set trigger-addresses to OFF
    for(i = 0; i < STDGENTRACER_MAX_TRIGGER_CHANNELS; i++)
    {
        trigger_specs[i].address.word_ptr = 0;
        trigger_specs[i].data = 0;
        trigger_specs[i].operation = TRIGGER_EQUAL_DATA;
        trigger_specs[i].status = UNUSED + SIZE_IS_SWORD;
    }

    // set trace-addresses to OFF
    for(i = 0; i < STDGENTRACER_MAX_TRACE_CHANNELS; i++)
    {
        trace_specs[i].address.word_ptr = 0;
        trace_specs[i].status = UNUSED + SIZE_IS_SWORD;
    }
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void StdGenTracer_Handler(void)
{
   U8  i;
   U16 stop_copy;

   if(trace_start_stop == START_TRACE)
   {
       StdGenTracer_CheckTrigger();
       stop_copy = 0;
       while((msec_write_pointer != msec_read_pointer) && (stop_copy == 0))
       {
           //copy samples in Trace Array to the big buffer in RAM
           address_small_buf_ptr = (S16*) &msec_trace_buffer[msec_read_pointer][0];
           address_big_buf_ptr = (S16*) &big_buffer[0] + write_pointer;
           for(i = 0; i < record_length; i++)
           {
               *address_big_buf_ptr = *address_small_buf_ptr;
               address_small_buf_ptr++;
               address_big_buf_ptr++;
           }
           number_of_records++;

           if(trigger_status == TRIGGER_FOUND)
           {
                if(remaining_samples_to_be_taken != 65535)
                {
                    remaining_samples_to_be_taken--;
                }
                if((number_of_records == (maximum_number_of_records - 1)) ||
                (remaining_samples_to_be_taken == 0))
                {
                    //  The trigger is found
                    //  and enough data captured
                    //  ------------------------
                    StdGenTracer_StopTrace();
                    trace_status = TRACE_COMPLETE;
                    stop_copy = 1;
                }
           }
           else
           {
               //  The trigger is stil not found
               //  -----------------------------
               switch(trigger_mode)
               {
               case POST_TRIGGER:
                   if(number_of_records > (maximum_number_of_records >> 4))
                   {
                       number_of_records--;
                       read_pointer += record_length;
                       if(read_pointer == trace_buffer_end)
                       {
                           read_pointer = 0;
                       }
                   }
                   break;
               case CENTRE_TRIGGER:
                   if(number_of_records > (maximum_number_of_records >> 1))
                   {
                       number_of_records--;
                       read_pointer +=  record_length;
                       if(read_pointer == trace_buffer_end)
                       {
                           read_pointer = 0;
                       }
                   }
                   break;
               case PRE_TRIGGER:
                   if(number_of_records > (maximum_number_of_records - (maximum_number_of_records >> 4)))
                   {
                       number_of_records--;
                       read_pointer += record_length;
                       if(read_pointer == trace_buffer_end)
                       {
                           read_pointer = 0;
                       }
                   }
                   break;
                case EXT_TRIGGER:
                //keep as much as possible samples in buffer
                //when trigger happens, we will adapt the number_of_records and the read_pointer
                if(number_of_records > maximum_number_of_records)
                {
                    number_of_records--;
                    read_pointer += record_length;
                    if(read_pointer == trace_buffer_end)
                    {
                        read_pointer = 0;
                    }
                }
                break; 	
               default:/* BUG : GET FIRED */
                   //COP register handlen
                   break;
               }
           }

           write_pointer += record_length;
           if(write_pointer >= trace_buffer_end)
           {
               write_pointer = 0;
               address_big_buf_ptr = (S16*) &big_buffer[0];
           }
           if(++msec_read_pointer == STDGENTRACER_NB_OF_BUFFERS)
           {
               msec_read_pointer = 0;
           }
       }
   }
}
//------------------------------------------------------------------------------------------------//
void StdGenTracer_Trace(void)
{
    U32 i;
    if((trace_start_stop == START_TRACE)  && (--trace_counter == 0))
    {                                                              // if trace active and no break is set and if timeout
        trace_counter = time_base;  // then reset timer and trace
        //write one sample of the variables to the Trace Array
        address_small_buf_ptr = (S16*) &msec_trace_buffer[msec_write_pointer][0];
                                                               //point to first variable of 'msec_wite_pointer'th buffer

        i = 0;

        while((i < STDGENTRACER_MAX_TRACE_CHANNELS) && (trace_specs[i].status & USED_UNUSED_MASK) != UNUSED)
        {
            switch(trace_specs[i].status)
            {
            case USED + SIZE_IS_SBYTE:
                *address_small_buf_ptr = (*trace_specs[i].address.word_ptr & 0x00FF) << 8;
                address_small_buf_ptr++;
                break;
            case USED + SIZE_IS_UBYTE:
                *address_small_buf_ptr = (U16)(*trace_specs[i].address.word_ptr & 0x00FF) << 8;
                address_small_buf_ptr++;
                break;
            case USED + SIZE_IS_SWORD:
                *address_small_buf_ptr = *trace_specs[i].address.word_ptr;
                address_small_buf_ptr++;
                break;
            case USED + SIZE_IS_UWORD:
                *address_small_buf_ptr = (U16) *trace_specs[i].address.word_ptr;
                address_small_buf_ptr++;
                break;
            case USED + SIZE_IS_SLONG:
                *address_small_buf_ptr = *(trace_specs[i].address.word_ptr + 1);
                address_small_buf_ptr++;
                *address_small_buf_ptr = *trace_specs[i].address.word_ptr;
                address_small_buf_ptr++;
                break;
            case USED + SIZE_IS_ULONG:
                *address_small_buf_ptr = (U16) *(trace_specs[i].address.word_ptr + 1);
                address_small_buf_ptr++;
                *address_small_buf_ptr = (U16) *trace_specs[i].address.word_ptr;
                address_small_buf_ptr++;
                break;
            default:
                i = STDGENTRACER_MAX_TRACE_CHANNELS - 1;
            break;
            }
            i++;
        }

        if(++msec_write_pointer == STDGENTRACER_NB_OF_BUFFERS)
        {
            msec_write_pointer = 0;
        }
    }
}
//------------------------------------------------------------------------------------------------//
void StdGenTracer_SetMemoryByte(U32 address, U8 data)
{
	U16* 	write_ptr;
	U16		temp;

	write_ptr = (U16*)address;
	temp = (*write_ptr & 0xFF00);
	temp |= (U16)data;
    *write_ptr = temp;
}
//------------------------------------------------------------------------------------------------//
U8 StdGenTracer_GetMemoryByte(U32 address)
{
	U16* 	read_ptr;

	read_ptr = (U16*)address;
	return((U8)(*read_ptr & 0x00FF));
}
//------------------------------------------------------------------------------------------------//
void StdGenTracer_SetMemoryWord(U32 address, U16 data)
{
	U16* 	write_ptr;

	write_ptr = (U16*)address;
    *write_ptr = data;
}
//------------------------------------------------------------------------------------------------//
U16 StdGenTracer_GetMemoryWord(U32 address)
{
	U16* 	read_ptr;

	read_ptr = (U16*)address;
	return(*read_ptr);
}
//------------------------------------------------------------------------------------------------//
void StdGenTracer_SetMemoryLong(U32 address, U32 data)
{
	U32* 	write_ptr;

	write_ptr = (U32*)address;
    *write_ptr = data;
}
//------------------------------------------------------------------------------------------------//
U32 StdGenTracer_GetMemoryLong(U32 address)
{
	U32* 	read_ptr;

	read_ptr = (U32*)address;
	return(*read_ptr);
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_GetPredefinedName(U8 	predef_nr,
						 			  U8 	segment_nr,
						 			  U8 	char_length,
						 			  U8* 	data_ptr)
{
	U8 i;
    U8 j;

    j = segment_nr * char_length;

    if((predef_nr < stdtracer_nb_of_predefs) && (j < STDGENTRACER_NAME_LENGTH))
    {
		i = 0;
        while((j < STDGENTRACER_NAME_LENGTH) && (i < char_length))
        {
            data_ptr[i++] = predefineds_ptr[predef_nr].name[j++];
        }
        while(i < char_length)
        {
			data_ptr[i++] = 0x00;			//fill up last segment with zero chars in ASCII
		}
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_GetPredefinedAddress(U8 predef_nr, U32* address_ptr)
{
    if(predef_nr < stdtracer_nb_of_predefs)
    {
        *address_ptr = (U32)(predefineds_ptr[predef_nr].address_ptr);
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_GetPredefinedInfo(U8 predef_nr, U8* data_ptr)
{
    if(predef_nr < stdtracer_nb_of_predefs)
    {
        *data_ptr = (U8)(predefineds_ptr[predef_nr].info);
        return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
U8 StdGenTracer_GetTraceStatus(void)
{
	U8	temp;

    temp = trace_status;

    if(((trace_status == TRACE_BUSY) || (trace_status == TRACE_COMPLETE)) && (number_of_records != 0)) // there is data
    {
        temp |= DATA_AVAILABLE;
    }
    return temp;
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_SetTraceControl(TRACE_CONTROL control)
{
    switch(control)
    {
    case INIT_TRACE:
        StdGenTracer_Init(&predefineds_ptr[0], stdtracer_nb_of_predefs);
        break;
    case STOP_TRACE:
        StdGenTracer_StopTrace();
        break;
    case START_TRACE:
        StdGenTracer_StartTrace();
        break;
    default:
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_SetTimeBase(U16 data)
{
	if(trace_start_stop != START_TRACE)
	{
		time_base = data;

		if(time_base == 0)
		{
			time_base++;
		}
		return TRUE;
	}
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
U16 StdGenTracer_GetTimeBase(void)
{
	return time_base;
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_SetTraceChannelSettings(U8 channel_nb, U32 address, U8 info)
{
    if (channel_nb > (STDGENTRACER_MAX_TRACE_CHANNELS - 1))
    {
        return FALSE;
    }
    else
    {
		if(trace_start_stop == START_TRACE)
		{
			return FALSE; // not allowed while trace is running
		}
		else
		{
			if((address & PREDEFINED_ADDRESS_MASK) != 0)
			//in this case (MSB = 1) address is an offset within the predefineds array
			{
				if((address & ~PREDEFINED_ADDRESS_MASK) < stdtracer_nb_of_predefs)
				{
					//address  & ~PREDEFINED_ADDRESS_MASK = offset binnen predefineds array
					trace_specs[channel_nb].address.word_ptr =
								   (S16 *)(predefineds_ptr[(address & ~PREDEFINED_ADDRESS_MASK)].address_ptr);
					trace_specs[channel_nb].status = predefineds_ptr[(address & ~PREDEFINED_ADDRESS_MASK)].info;
				}
				else
				{
					return FALSE;
				}
			}
			else    //in this case address is an effective address eg $3000
			{
				switch(info & ~ACCESS_MASK)
				{
				case SIZE_IS_SBYTE:
				case SIZE_IS_UBYTE:
				case SIZE_IS_SWORD:
				case SIZE_IS_UWORD:
				case SIZE_IS_SLONG:
				case SIZE_IS_ULONG:
					//it is the address that is being read and not the byte at the address
					trace_specs[channel_nb].address.word_ptr = (S16*)address;
					trace_specs[channel_nb].status =  info;
					return TRUE;
				default :
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_GetTraceChannelSettings(U8 channel_nb, U32* address_ptr, U8* info_ptr)
{
	U8   j;

	if (channel_nb > (STDGENTRACER_MAX_TRACE_CHANNELS - 1))
	{
		return FALSE;
	}
	else
    {
		j = 0;

		while((j < stdtracer_nb_of_predefs)
				&& ((S16*)predefineds_ptr[j].address_ptr != trace_specs[channel_nb].address.word_ptr))
		{
			j++;
		}

		if(j == stdtracer_nb_of_predefs)     //not one of the predefineds so address must be in trace specs array
		{
			*address_ptr = (U32)(trace_specs[channel_nb].address.word_ptr);
			*info_ptr = (U8)(trace_specs[channel_nb].status);   //size + enabled/disabled
		}
		else
		{
			*address_ptr = (U32)(PREDEFINED_ADDRESS_MASK) | (U32)j;
			*info_ptr = (U8)(predefineds_ptr[j].info);
		}
	}
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
U8 StdGenTracer_GetTracedData(U8* data_ptr)
{
    U16  i;
    U16  the_data[STDGENTRACER_WORD_LENGTH];
    U16* the_data_ptr;

	for(i = 0; i < STDGENTRACER_WORD_LENGTH; i++)
	{
		the_data[i] = 0;
	}

    if((trace_status == TRACE_BUSY) || (trace_status == TRACE_COMPLETE)) // maybe there is data
    {
        i = 0;
        while((number_of_records != 0) && (i < STDGENTRACER_WORD_LENGTH))
        {
            the_data_ptr = (U16*)(&big_buffer[0] + read_pointer);
            the_data[i] = (U16) *the_data_ptr;

            data_ptr[i * 2] = the_data[i] >> 8;
        	data_ptr[i * 2 + 1] = the_data[i] & 0x00FF;

            read_pointer++;
            i++;

            if(++record_words_read == record_length)
            {
                record_words_read = 0;
                number_of_records--;
                if(read_pointer == trace_buffer_end)
                {
                    read_pointer = 0;
                }
            }
        }

        if(i == 0)
        {
            return(trace_status | NO_DATA_AVAILABLE);
        }
        else
        {
            return(trace_status | DATA_AVAILABLE | (i * 2));
        }
    }
    else
    {
        return(trace_status | NO_DATA_AVAILABLE);
    }
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_SetTriggerChannelSettings(U8 trigger_nb, U32 address, U8 info)
{
    if (trigger_nb > (STDGENTRACER_MAX_TRIGGER_CHANNELS - 1))
    {
        return FALSE;
    }
    else
    {
		if(trace_start_stop == START_TRACE)
		{
			return FALSE; 	// not allowed while trace is running
		}
		else
		{
			if((address & PREDEFINED_ADDRESS_MASK) != 0)
			//in this case (MSB = 1) address is an offset within the predefineds array
			{
				if((address & ~PREDEFINED_ADDRESS_MASK) < stdtracer_nb_of_predefs)
				{
					//address  & ~PREDEFINED_ADDRESS_MASK = offset binnen predefineds array
					trigger_specs[trigger_nb].address.word_ptr =
								   (S16 *)(predefineds_ptr[(address & ~PREDEFINED_ADDRESS_MASK)].address_ptr);
					trigger_specs[trigger_nb].status = predefineds_ptr[(address & ~PREDEFINED_ADDRESS_MASK)].info;
				}
				else
				{
					return FALSE;
				}
			}
			else    //in this case address is an effective address eg $3000
			{
				switch(info & ~ACCESS_MASK)
				{
				case SIZE_IS_SBYTE:
				case SIZE_IS_UBYTE:
				case SIZE_IS_SWORD:
				case SIZE_IS_UWORD:
				case SIZE_IS_SLONG:
				case SIZE_IS_ULONG:

					if((address & 1) != 0)		//why?????? TO CHECK!!!!!!!
					{
						return FALSE;
					}
					else
					{
						trigger_specs[trigger_nb].address.word_ptr = (S16*)address;
						trigger_specs[trigger_nb].status = info;
					}
					return TRUE;

				default :
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_GetTriggerChannelSettings(U8 trigger_nb, U32* address_ptr, U8* info_ptr)
{
    U8   j;

    if (trigger_nb > (STDGENTRACER_MAX_TRIGGER_CHANNELS - 1))
    {
        return FALSE;
    }
    else
    {
        j = 0;
        
        while((j < stdtracer_nb_of_predefs)
              && ((S16*)predefineds_ptr[j].address_ptr != trigger_specs[trigger_nb].address.word_ptr))
        {
            j++;
        }
        
        if(j == stdtracer_nb_of_predefs)     //not one of the predefineds so address must be in trigger specs array
        {
            *address_ptr = (U32)(trigger_specs[trigger_nb].address.word_ptr);
            *info_ptr = (U8)(trigger_specs[trigger_nb].status);   //size + enabled/disabled
        }
        else
        {
            *address_ptr = (U32)(PREDEFINED_ADDRESS_MASK) | (U32)j;
            *info_ptr = (U8)(predefineds_ptr[j].info);
        }
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_SetTriggerChannelData(U8 trigger_nb, S32 data, TRIGGER_OPERATOR operation)
{
    if(trigger_nb > (STDGENTRACER_MAX_TRIGGER_CHANNELS - 1))
    {
        return FALSE;
    }
    else
    {
	if(trace_start_stop != START_TRACE)
	{
            if((operation == TRIGGER_EQUAL_DATA)        ||
            (operation == TRIGGER_NOT_EQUAL_DATA)	||
            (operation == TRIGGER_BITS_HIGH)		||
            (operation == TRIGGER_BITS_LOW)		||
            (operation == TRIGGER_LOWER_DATA)		||
            (operation == TRIGGER_HIGHER_DATA)		||
            (operation == TRIGGER_SIGNED_LOWER_DATA)	||
            (operation == TRIGGER_SIGNED_HIGHER_DATA))
            {
                trigger_specs[trigger_nb].data = data;
                trigger_specs[trigger_nb].operation = operation;
                return TRUE;
            }
	}
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_GetTriggerChannelData(U8 trigger_nb, U32* data_ptr, U8* operation_ptr)
{
    if(trigger_nb > (STDGENTRACER_MAX_TRIGGER_CHANNELS - 1))
    {
        return FALSE;
    }
    else
    {
    *data_ptr = (U32)(trigger_specs[trigger_nb].data);
    *operation_ptr = (U8)(trigger_specs[trigger_nb].operation);
    return TRUE;
     }
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_SetTriggerMode(TRIGGER_MODE mode)
{
    if(trace_start_stop == STOP_TRACE)
    {
            trigger_mode = mode;
            return TRUE;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
TRIGGER_MODE StdGenTracer_GetTriggerMode(void)
{
    return trigger_mode;
}
//------------------------------------------------------------------------------------------------//
U16 StdGenTracer_GetTriggerPosition(void)
{
    return trigger_position;
}
//------------------------------------------------------------------------------------------------//
BOOL StdGenTracer_SetExternalTrigger(U16 pre_samples, U16 post_samples)
{
    U16 max_pre;
    if((trace_start_stop == START_TRACE) && (trigger_mode == EXT_TRIGGER))
    {
        //calc the nr_of_pre_trigger_samples as the minimum of the request nr of pre samples and the available 
        //buffer size
        if(maximum_number_of_records > 5)
        {
                max_pre = maximum_number_of_records - 5;
        }
        else
        {
                max_pre = maximum_number_of_records;
        }
        if(pre_samples > max_pre)
        {
                nr_of_pre_trigger_samples = max_pre;
        }
        else
        {
                nr_of_pre_trigger_samples = pre_samples;
        }
        //maximum nr of post samples to be taken
        remaining_samples_to_be_taken = post_samples;
        
        ext_triggered = TRUE;
        return TRUE;
    }
    return FALSE;
}
//================================================================================================//
