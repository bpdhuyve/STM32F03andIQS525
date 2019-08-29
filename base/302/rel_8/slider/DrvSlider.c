//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// brief explanation
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRV_SLIDER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef DRV_SLIDER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               DRV_SLIDER_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "slider\DrvSlider.h"
//SYSTEM include section
//DRIVER include section
//STANDARD include section
//APPLICATION include section
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define DRVSLIDER_SUBSLIDER_COUNT   4
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    SLIDER_HNDL slider_hndl;    //real slider handle where the non splited value should be fetched
    BOOL reversed;              //if this is set the slider must be reversed
    U16 boundary_min;            //subslider min boundary
    U16 boundary_max;            //subslider max boundary
}
SUBSLIDER_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
BOOL SubSliderGetValue(U8 subslider_id, U16* slider_value_ptr);
BOOL BufferedSliderGetValue(SLIDER_ID subslider_id, U16* slider_value_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SLIDER_STRUCT        bufferedsliderstruct , subslider_handles[DRVSLIDER_SUBSLIDER_COUNT];
static SUBSLIDER_STRUCT     subslider_info[DRVSLIDER_SUBSLIDER_COUNT];
static SLIDER_HNDL          savedsliderhndl;
static U16                  bufferedslidervalue = 0;
BOOL                        touchedbool;
BOOL                        subslider_previous_touched[DRVSLIDER_SUBSLIDER_COUNT];
U16                         subslider_previous_value[DRVSLIDER_SUBSLIDER_COUNT];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
BOOL BufferedSliderGetValue(SLIDER_ID subslider_id, U16* slider_value_ptr)
{
    //ID is ignored, the parameter value
    if (touchedbool)//see DrvSlider_RefreshBufferedSlider function
    {
        *slider_value_ptr = bufferedslidervalue;
        return TRUE;
    }
    else
    {
        return FALSE;
    }

}
//------------------------------------------------------------------------------------------------//
BOOL SubSliderGetValue(SLIDER_ID subslider_id, U16* slider_value_ptr)
{
    SUBSLIDER_STRUCT*   subslider_struct_ptr = &subslider_info[subslider_id];
    U16                 real_slider_value;

    if(subslider_id < DRVSLIDER_SUBSLIDER_COUNT)
    {
        if(DrvSlider_GetValue(subslider_struct_ptr->slider_hndl, &real_slider_value))
        {
            //touch detected in the range of this subslider
            if((real_slider_value >= subslider_struct_ptr->boundary_min) && (real_slider_value <= subslider_struct_ptr->boundary_max))
            {
                //check if another slider was already touched ?
                U8 i;
                for(i = 0; i < DRVSLIDER_SUBSLIDER_COUNT; i++)
                {
                    if(subslider_previous_touched[i] && i != subslider_id)
                    {
                        return FALSE;   //yes some other slider was already touched -> user crossed over the boundary without releasing -> so return that this subslider is not touched, the code below will say that the previous slider is still touched and take its lats value as reference
                    }
                }

                //ok no other slider was previous, so calculate return value for this subslider
                *slider_value_ptr = (U16)(((U32)(real_slider_value - subslider_struct_ptr->boundary_min) * (U32)0xFFFF) / (U32)(subslider_struct_ptr->boundary_max - subslider_struct_ptr->boundary_min));
                if(subslider_struct_ptr->reversed == TRUE)
                {
                    *slider_value_ptr = 0xFFFF - *slider_value_ptr;
                }
                subslider_previous_value[subslider_id] = *slider_value_ptr;
                subslider_previous_touched[subslider_id] = TRUE;
                return TRUE;
            }
            else //real slider is not in the range anymore but if it was previously touched this means user is still touching but crossed over the thresshold -> still flag this subslider as touched!
            {
                if(subslider_previous_touched[subslider_id])
                {
                    if (subslider_previous_value[subslider_id] < 0xFFFF/2)  //use previous value to determine if the slader is at max or min
                    {
                        *slider_value_ptr = 0;
                    }
                    else
                    {
                        *slider_value_ptr = 0xFFFF;
                    }
                    return TRUE;
                }
            }
        }
    }
    subslider_previous_touched[subslider_id] = FALSE;
    return FALSE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSlider_Init(void)
{
    MEMSET((VPTR)subslider_info, 0, SIZEOF(subslider_info));
    MEMSET((VPTR)subslider_handles, 0, SIZEOF(subslider_handles));
}
//------------------------------------------------------------------------------------------------//
BOOL DrvSlider_GetValue(SLIDER_HNDL slider_hndl, U16* slider_value_ptr)
{
    if((slider_hndl != NULL) && (slider_hndl->get_value_hook != NULL))
    {
        return slider_hndl->get_value_hook(slider_hndl->slider_id, slider_value_ptr);//call de hook en vraag hem de slider_value_ptr in te vullen met de waarde van die slider id
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
//todo functions to create a buffered slider
SLIDER_HNDL DrvSlider_CreateBufferedSlider(SLIDER_HNDL slider)
{
    if(slider == NULL)
    {
        LOG_ERR("Invalid slider handle");
        return NULL;
    }

    savedsliderhndl = slider;//Save the real slider handler somewhere
    bufferedsliderstruct.get_value_hook = BufferedSliderGetValue;//the buffered slider handler contains a different hook

    return &bufferedsliderstruct;
}
//------------------------------------------------------------------------------------------------//
void DrvSlider_RefreshBufferedSlider(SLIDER_HNDL slider_hndl)
{
    if(slider_hndl == NULL)
    {
        LOG_ERR("Invalid slider handle");
        return;
    }

     touchedbool = DrvSlider_GetValue(savedsliderhndl,&bufferedslidervalue);
}
//------------------------------------------------------------------------------------------------//
SLIDER_HNDL DrvSlider_CreateSubSlider(SLIDER_HNDL slider, U16 boundary_min, U16 boundary_max, BOOL reversed)
{
    if(slider == NULL)
    {
        LOG_ERR("Invalid slider handle");
        return NULL;
    }

    //check wich entry is free
    U8 i;
    for(i = 0; i < DRVSLIDER_SUBSLIDER_COUNT; i++)
    {
        if (subslider_handles[i].get_value_hook == NULL)  //this subslider handle is still free
        {
            //fill in subslider_info
            subslider_info[i].slider_hndl = slider;
            subslider_info[i].boundary_min = boundary_min;
            subslider_info[i].boundary_max = boundary_max;
            subslider_info[i].reversed = reversed;

            //fill in sub slider handle
            subslider_handles[i].get_value_hook = SubSliderGetValue;
            subslider_handles[i].slider_id = i;

            //return subslider handle
            return &subslider_handles[i];
        }
    }

    return NULL;    //if code gets here there is no subslider handler created
}
//================================================================================================//
