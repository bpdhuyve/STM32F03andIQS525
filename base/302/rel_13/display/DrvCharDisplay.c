//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// code to control a hd44780 based alphanumeric lcd display
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DISPLAY__DRVCHARDISPLAY_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
#ifndef DISPLAY__DRVCHARDISPLAY_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               DISPLAY__DRVCHARDISPLAY_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRV_CHAR_DISPLAY_ANIMATION_TASK_PERIOD
	#define DRV_CHAR_DISPLAY_ANIMATION_TASK_PERIOD               300000
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

#include "display/DrvCharDisplay.h"

//APPLICATION include section
//STANDARD include section
//DRIVER include section
//SYSTEM include section
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define SCROLLING_TEXT_BUFFER_SIZE  50
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void DrvCharDisplay_AnimationTask(VPTR data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static BOOL                     animation_init_done = FALSE;;
static DRV_CHAR_DISPLAY_HNDL    animation_display;
static U32                      display_scrolling_text_active = 0; //variable size beperkt aantal display lines tot 32
static U8*                      display_scrolling_text_buffer;
static U8*                      display_scrolling_text_buffer_ofset_counter;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DrvCharDisplay_AnimationTask(VPTR data_ptr)
{
    if (display_scrolling_text_active != 0)
    {
        //overloop alle lijnen en check ofdat de scroll bit aan staat zoja scroll die lijn
        U8 i;
        for(i = 0; i < animation_display->line_count; i++)//line loop
        {
            if (display_scrolling_text_active & (0x01<<i)) //scroll line active?
            {
                //copy string to displaybuffer
                BOOL endofStringFound = FALSE;
                U8 j;
                for(j = 0; j < animation_display->line_width; j++) //char loop
                {
                    CHAR tempchar = *(display_scrolling_text_buffer+(SCROLLING_TEXT_BUFFER_SIZE*i)+display_scrolling_text_buffer_ofset_counter[i]+j);
                    if (tempchar == 0 | endofStringFound)
                    {
                        *((animation_display->buffer_ptr)+((animation_display->line_width)*i)+j) = ' '; //if string is at its end fill up with spaces
                        endofStringFound = TRUE;
                        if (j==0) //end of string is directly found at beginning -> reset ofset counter
                        {
                            display_scrolling_text_buffer_ofset_counter[i] = 0;
                            break;
                        }
                    }
                    else
                    {
                        *((animation_display->buffer_ptr)+((animation_display->line_width)*i)+j) = tempchar; //copy char into screenbuffer
                    }
                }

                display_scrolling_text_buffer_ofset_counter[i]++;
            }
        }
        animation_display->write_buffer_to_screen_hook(); //perform write
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvCharDisplay_Init()
{
    //create task with 1 ms cycle, evry time this task runs a command will be written
    CoreTask_Start(CoreTask_RegisterTask(DRV_CHAR_DISPLAY_ANIMATION_TASK_PERIOD, DrvCharDisplay_AnimationTask, (VPTR)NULL, 150, "DrvCharDisplay_AnimationTask"));
}
//------------------------------------------------------------------------------------------------//
void DrvCharDisplay_WriteLine(DRV_CHAR_DISPLAY_HNDL display, U8 lineNr, STRING text)
{
    if (display == NULL)
    {
        return;
    }
    if (CoreString_GetLength(text)<= display->line_width) //check if string can fit on the screen
    {
        //copy string directly into displaybuffer buffer
        U8 i;
        for(i = 0; (i < display->line_width); i++)
        {
            if (((U8)*(text+i)) != 0) //check if end of string has not been found
            {
                *(display->buffer_ptr +((display->line_width)*lineNr) + i) = *(text+i);
            }
            else //if there is no string left fill line with spaces
            {
                *(display->buffer_ptr +((display->line_width)*lineNr) + i) = ' ';
            }
        }
        display_scrolling_text_active &= ~(1<<lineNr);   //scroll off
        display->write_buffer_to_screen_hook(); //perform write
    }
    else //string is to long for the screen -> activate scroll
    {
        if (!animation_init_done) //first time somthing animated needs to be done init the buffers
        {
            animation_init_done = TRUE;

            //copy display hndl
            animation_display = display;

            //create scroll buffers
            display_scrolling_text_buffer = CoreBuffer_CreateStaticU8(SCROLLING_TEXT_BUFFER_SIZE * display->line_count,"display_text_buffer");
            display_scrolling_text_buffer_ofset_counter = CoreBuffer_CreateStaticU8(display->line_count,"display_text_ofset_counter");
        }

        //copy string in to scroll buffer and start scroll
        MEMSET(display_scrolling_text_buffer+(lineNr*SCROLLING_TEXT_BUFFER_SIZE),' ',display->line_width); //fill witdh of buffer first width spaces
        CoreString_CopyString(text,(STRING)display_scrolling_text_buffer+(lineNr*SCROLLING_TEXT_BUFFER_SIZE)+display->line_width);//copy string form there
        display_scrolling_text_buffer_ofset_counter[lineNr] = display->line_width; //start scrollign @ width of buffer to avoid the leading space in the beginning
        display_scrolling_text_active |= (1<<lineNr); //set scroll on
    }
}
//------------------------------------------------------------------------------------------------//
void DrvCharDisplay_ClearLine(DRV_CHAR_DISPLAY_HNDL display, U8 lineNr)
{
    if (display == NULL)
    {
        return;
    }

    display_scrolling_text_active &= ~(1<<lineNr);   //scroll off for that line
    MEMSET(display->buffer_ptr + (lineNr * display->line_width) , ' ', display->line_width); //fill line with spaces
    display->write_buffer_to_screen_hook(); //perform write
}
//------------------------------------------------------------------------------------------------//
void DrvCharDisplay_ClearScreen(DRV_CHAR_DISPLAY_HNDL display)
{
    if (display == NULL)
    {
        return;
    }

    display_scrolling_text_active = 0;   //scroll off for all lines
    MEMSET(display->buffer_ptr, ' ', display->line_count * display->line_width); //fill entire buffer with spaces
    display->write_buffer_to_screen_hook(); //perform write
}
//================================================================================================//
