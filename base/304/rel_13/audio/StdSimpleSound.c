//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// way to play simpel audio with a dac or pwm
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STD_SIMPLE_SOUND_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STD_SIMPLE_SOUND_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               STD_SIMPLE_SOUND_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  defines the maximum data length that can be handled.
#ifndef STD_SIMPLE_SOUND_OUTPUT_SAMPLE_DEPTH_IN_BITS
    #define STD_SIMPLE_SOUND_OUTPUT_SAMPLE_DEPTH_IN_BITS            10
#endif
//------------------------------------------------------------------------------------------------//
// @brief  defines the maximum data length that can be handled.
#define STD_SIMPLE_SOUND_OUTPUT_METHOD_TIMER_BASED                  0
#define STD_SIMPLE_SOUND_OUTPUT_METHOD_DMA_BASED                    1

#ifndef STD_SIMPLE_SOUND_OUTPUT_METHOD
    #define STD_SIMPLE_SOUND_OUTPUT_METHOD                          STD_SIMPLE_SOUND_OUTPUT_METHOD_TIMER_BASED
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section

//STD lib include section

//COM lib include section

//APP include section
#include "audio/StdSimpleSound.h"
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
static void Task_SetSample(VPTR data_ptr);
static TASK_HNDL	task_hndl_SetSample = NULL;
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();


BOOL module_inited = FALSE;
EVENT_NEW_AUDIO_OUT_SAMPLE event_set_audio_out_value;
EVENT_NEW_AUDIO_OUT_SAMPLES event_set_audio_out_values;
SOUND active_sound;
U32 activeSampleCounter = 0;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//task routine
static void Task_SetSample(VPTR data_ptr)
{
    event_set_audio_out_value(active_sound.location_in_flash[activeSampleCounter]/*>>6*/);//smijt onderste 4 bits weg
    //event_set_audio_out_value((U16*)active_sound.location_in_flash);
    activeSampleCounter++;

    if (activeSampleCounter >= active_sound.sample_count)
    {
        CoreTask_Stop(task_hndl_SetSample); //stop yourself
    }
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if STD_SIMPLE_SOUND_OUTPUT_METHOD == STD_SIMPLE_SOUND_OUTPUT_METHOD_TIMER_BASED
void StdSimpleSound_Init_TimerDriven(EVENT_NEW_AUDIO_OUT_SAMPLE event)
{
    MODULE_INIT_ONCE();

    //task creation
    task_hndl_SetSample = CoreTask_RegisterTask(1, Task_SetSample, NULL, 1, "Task_SetSample");
    //CoreTask_Start(task_hndl_SetSample);

    MODULE_INIT_DONE();
    //place init code which must executed on every re-init here

    event_set_audio_out_value = event;

    module_inited = TRUE;
}
#endif
//------------------------------------------------------------------------------------------------//
#if STD_SIMPLE_SOUND_OUTPUT_METHOD == STD_SIMPLE_SOUND_OUTPUT_METHOD_DMA_BASED
void StdSimpleSound_Init_DmaDriven(EVENT_NEW_AUDIO_OUT_SAMPLES event)
{
    MODULE_INIT_ONCE();

    //task creation

    MODULE_INIT_DONE();
    //place init code which must executed on every re-init here

    event_set_audio_out_values = event;

    module_inited = TRUE;
}
#endif
//------------------------------------------------------------------------------------------------//
void StdSimpleSound_Handler(void)
{
}
//------------------------------------------------------------------------------------------------//
void StdSimpleSound_Play(SOUND sound)
{
    StdSimpleSound_PlayPitched(sound,1,NULL);
}
//------------------------------------------------------------------------------------------------//
void StdSimpleSound_PlayWithOptions(SOUND sound, PLAYBACK_OPTIONS options)
{
    StdSimpleSound_PlayPitched(sound,1,options);
}
//------------------------------------------------------------------------------------------------//
void StdSimpleSound_PlayPitched(SOUND sound, F32 pitch, PLAYBACK_OPTIONS options) //pitch in %
{
    if (module_inited != TRUE)
    {
        return; //module not inited, do nothing
    }

    #if STD_SIMPLE_SOUND_OUTPUT_METHOD == STD_SIMPLE_SOUND_OUTPUT_METHOD_TIMER_BASED
        F32 temp = sound.sample_rate*pitch;
        temp = 1/temp*1e6; //convert to µseconds
        CoreTask_SetPeriod(task_hndl_SetSample,(U32)temp);
        activeSampleCounter = 0;
        active_sound = sound;
        CoreTask_Start(task_hndl_SetSample);
    #elif STD_SIMPLE_SOUND_OUTPUT_METHOD == STD_SIMPLE_SOUND_OUTPUT_METHOD_DMA_BASED
        SOUND tempsound;
        tempsound.format = sound.format;
        tempsound.location_in_flash = sound.location_in_flash;
        tempsound.sample_count = sound.sample_count;
        tempsound.sample_rate = sound.sample_rate*pitch;
        if (options & PLAYBACK_OPTION_DONT_ABORT)
        {
            event_set_audio_out_values(tempsound, FALSE);
        }
        else
        {
            event_set_audio_out_values(tempsound, TRUE);
        }
    #endif
}
//================================================================================================//