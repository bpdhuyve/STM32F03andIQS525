//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// slider main header file.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DRV_SLIDER_H
#define DRV_SLIDER_H
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
typedef U8                      SLIDER_ID;
//------------------------------------------------------------------------------------------------//
// @remark   brief comment
typedef BOOL (*SLIDER_GET_VALUE_HOOK)(SLIDER_ID slider_id, U16* slider_value_ptr);
//------------------------------------------------------------------------------------------------//
typedef struct
{
    SLIDER_GET_VALUE_HOOK       get_value_hook;     //function pointer to a get value function
    SLIDER_ID                   slider_id;          //this id counts up from 0 and is used so the get value hook knows where to get its value from (for example 2 slider chips are connected)
}
SLIDER_STRUCT;
//------------------------------------------------------------------------------------------------//
// @remark  definition what a slider handle is, surprise its a pointer to a slider struct
//          in your driver you have to create one or more slider structs if you want to use DrvSlider
typedef SLIDER_STRUCT*          SLIDER_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void DrvSlider_Init(void);

// @remark   this function gets the slider value from the specified slider
//you will have to provide the address of a U16 where the slider value has to be placed,0 is the minimum, 65535 is the maximum
//this function returns true if the slider value is present (touched), if false nothing will be placed in the slider_value_ptr
BOOL DrvSlider_GetValue(SLIDER_HNDL slider_hndl, U16* slider_value_ptr);

//functions to create and refresh a buffered slider
//a buffered slider works as a value buffer ontop of a normal slider
//you normally will use this if you want to limit the communication to the physical device (you can call the DrvSlider_RefreshBufferedSlider
//on a regual base (performing the real communciation) and the application ontop can use the DrvSlider_GetValue as much as they want to ask the latest value from the buffered slider)
//the bufferd slider value can only be refreshed using the DrvSlider_RefreshBufferedSlider function
SLIDER_HNDL DrvSlider_CreateBufferedSlider(SLIDER_HNDL slider);
void DrvSlider_RefreshBufferedSlider(SLIDER_HNDL slider);

//function to create a subslider from another slider, use this to split a big slider in to multiple small sliders
SLIDER_HNDL DrvSlider_CreateSubSlider(SLIDER_HNDL slider, U16 boundary_min, U16 boundary_max, BOOL reversed);
//================================================================================================//

#endif /* DRV_SLIDER_H */