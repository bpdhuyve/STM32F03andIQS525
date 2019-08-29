//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// way to play simpel audio with a dac or pwm
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef STD_SIMPLE_SOUND_H
#define STD_SIMPLE_SOUND_H
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
    SOUND_FORMAT_MONO_16bit,
}
SOUND_FORMATS;
//------------------------------------------------------------------------------------------------//
typedef struct
{
    SOUND_FORMATS format;
    U32 sample_rate;        //aka samples per second
    U32 sample_count;
    const U16* location_in_flash;
}
SOUND;
//------------------------------------------------------------------------------------------------//
/// bitmask that can be used to flag specific options so we can limit the number of fuctions to play sound
typedef enum
{
	PLAYBACK_OPTION_DONT_ABORT                 = 1  ,             // dont abort sound that is already playing (standard the sound is aborted and the new sound is played)
}
PLAYBACK_OPTIONS;
//------------------------------------------------------------------------------------------------//
typedef void (*EVENT_NEW_AUDIO_OUT_SAMPLE)(U16 sample);
typedef void (*EVENT_NEW_AUDIO_OUT_SAMPLES)(SOUND sound, BOOL abort_previous_sound);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  Init module
/// @brief  Init module, only one of these 2 should be used according to the define in the c file
/// @param  "event" event that will be called when the dac/pwn needs to set a new value
void StdSimpleSound_Init_TimerDriven(EVENT_NEW_AUDIO_OUT_SAMPLE event);
/// @param  "event" event that will be called when the dac/pwn needs to set a new set of values
void StdSimpleSound_Init_DmaDriven(EVENT_NEW_AUDIO_OUT_SAMPLES event);

// @remark  Background handler
void StdSimpleSound_Handler(void);

void StdSimpleSound_Play(SOUND sound);
void StdSimpleSound_PlayWithOptions(SOUND sound, PLAYBACK_OPTIONS options);
void StdSimpleSound_PlayPitched(SOUND sound, F32 pitch, PLAYBACK_OPTIONS options); //pitch = 1 is normal speed, 2 is double speed
//================================================================================================//



#endif /* STD_SIMPLE_SOUND_H */

