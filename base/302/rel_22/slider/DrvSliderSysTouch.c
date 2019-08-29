//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// brief explanation
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DRV_SLIDER_Ad7147_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"

#define CORELOG_LEVEL                                       LOG_LEVEL_TERM
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "slider\DrvSliderSysTouch.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#ifndef MAX_SLIDER_PAD_COUNT
    #define MAX_SLIDER_PAD_COUNT 10
#endif
//------------------------------------------------------------------------------------------------//
#ifndef MAX_BUTTON_PAD_COUNT
    #define MAX_BUTTON_PAD_COUNT 10
#endif
//------------------------------------------------------------------------------------------------//
#ifndef SLIDER_TRESHHOLD
    #define SLIDER_TRESHHOLD 50
#endif
//------------------------------------------------------------------------------------------------//
#ifndef BUTTON_TRESHHOLD
    #define BUTTON_TRESHHOLD 100
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL GetSliderValue(U8 slider_id, U16* slider_value_ptr);
static U16 CalculateSliderPosition(U16 * slider_pad_values_ptr, U8 slider_pads_count);
static BOOL GetPadAsButton(U16 pad_id);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SLIDER_STRUCT                    slider_struct;
//static U16                              slider_pads_mask = 0;

U8 pad_count;
TOUCH_PAD* pads = 0;

U16 sliderpad_ambient_values[MAX_SLIDER_PAD_COUNT];
U16 buttonpad_ambient_values[MAX_BUTTON_PAD_COUNT];
TOUCH_PAD buttonpads[MAX_BUTTON_PAD_COUNT];

static U8                           buttonpad_count = 0;
static DRV_GPIO_PIN_STRUCT          gpio_pin_struct[MAX_BUTTON_PAD_COUNT];
static const GPIO_DRV_HOOK_LIST     gpio_pin_hook_list = {NULL, NULL, GetPadAsButton};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL GetSliderValue(U8 slider_id, U16* slider_value_ptr)
{
    U16 raw_slider_pad_values[MAX_SLIDER_PAD_COUNT];
    static U16 slider_pad_values[MAX_SLIDER_PAD_COUNT];
    U8 i;

    //get raw values
    for(i = 0; i < pad_count; i++)
    {
        raw_slider_pad_values[i] = SysTouch_GetPadValue(*(pads+i));
    }

    //subtract ambient values
    for(i = 0; i < pad_count; i++)
    {
        if (raw_slider_pad_values[i] > sliderpad_ambient_values[i])
        {
            slider_pad_values[i] = raw_slider_pad_values[i] - sliderpad_ambient_values[i];
        }
        else
        {
            slider_pad_values[i] = 0;
        }
    }

    //check if a vaulue is above the threshold, if so calculate position
    for(i = 0; i < pad_count; i++)
    {
        if (slider_pad_values[i] > SLIDER_TRESHHOLD)
        {
            *slider_value_ptr = CalculateSliderPosition(slider_pad_values, pad_count);
            return TRUE;
        }
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
// Calculates and returns the position based on an array of values. Return value between 0 and FFFF
// argument is a ptr to the array with the pad values
static U16 CalculateSliderPosition(U16 * slider_pad_values_ptr, U8 slider_pads_count)
{
    U16 slider_pos = 0;
    U32 relative_position;
    U16 pad_ranges = ( 0x10000 / slider_pads_count ) + 1; //0x10000 is full range
    U16 max_center_offset = ( pad_ranges / 2 );
    U16 range_extension = 0xBD0*2; //increased to avoid bump
    U16 difference;

    //search pad with the highest value
    U8 pad_with_higest_value = 0;
    for (U8 i = 0; i < slider_pads_count; i++ )
    {
        if(slider_pad_values_ptr[pad_with_higest_value] < slider_pad_values_ptr[i] )
        {
            pad_with_higest_value = i;
        }
    }

    U16 pad_before_higest_value = ( pad_with_higest_value + slider_pads_count-1 ) % slider_pads_count;
    U16 pad_after_higest_value = ( pad_with_higest_value + 1 ) % slider_pads_count;

   // #if LINEAR_SLIDER == 1
    if ((pad_with_higest_value == 0) && (pad_before_higest_value > pad_after_higest_value)) //special case: user is on the beginning of the linear slider, so dotn do the normal calculation (only if lineair slider)
    {
    	return 0;
    }
	//#endif

    if( slider_pad_values_ptr[pad_after_higest_value] > slider_pad_values_ptr[pad_before_higest_value] )
    {
        difference = ( slider_pad_values_ptr[pad_after_higest_value] - slider_pad_values_ptr[pad_before_higest_value] );
        relative_position = ( difference * range_extension ) / ( slider_pad_values_ptr[pad_after_higest_value] ); //range_extension mult. is used here instead of the next line to void a round-down to 0 when dividing
        if ( relative_position > max_center_offset )
        {
            relative_position = max_center_offset;
        }
        slider_pos = ( pad_with_higest_value * pad_ranges ) + relative_position;
    }
    else
    {
        difference = ( slider_pad_values_ptr[pad_before_higest_value] - slider_pad_values_ptr[pad_after_higest_value] );
        relative_position = ( difference * range_extension ) / ( slider_pad_values_ptr[pad_before_higest_value] ); //range_extension mult. is used here instead of the next line to void a round-down to 0 when dividing
        if ( relative_position > max_center_offset )
        {
            relative_position = max_center_offset;
        }
        slider_pos = ( pad_with_higest_value * pad_ranges ) - relative_position;
    }

    return slider_pos;
}
//------------------------------------------------------------------------------------------------//
static BOOL GetPadAsButton(U16 pad_id)
{
    U16 raw_val = SysTouch_GetPadValue(buttonpads[pad_id]);

    if (raw_val > buttonpad_ambient_values[pad_id])
    {
        U16 val = raw_val - buttonpad_ambient_values[pad_id];

        if (val >= BUTTON_TRESHHOLD)
        {
            return TRUE;
        }
    }
    return FALSE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
/// inits the module,d o before use
void DrvSliderSysTouch_Init(void)
{
    MODULE_INIT_ONCE();

    SysTouch_Init();

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
SLIDER_HNDL DrvSliderSysTouch_RegisterSlider(U8 slider_pad_count, TOUCH_PAD* slider_pads)
{
    MODULE_CHECK();

    if (pad_count != 0)
    {
        LOG_ERR("only one sys touch slider is supported");
    }

    if (slider_pad_count > MAX_SLIDER_PAD_COUNT)
    {
        LOG_ERR("MAX_SLIDER_PAD_COUNT is to smal for this number of slider pads");
    }

    pad_count = slider_pad_count;
    pads = slider_pads;

    //lees nu eerst 1 maal de waarden uit en gebruik dat als de ambient waarden
    U8 i;
    for(i = 0; i < pad_count; i++)
    {
        #ifdef FIXED_AMBIENT_VALUE
            sliderpad_ambient_values[i] = FIXED_AMBIENT_VALUE;
        #else
            sliderpad_ambient_values[i] = SysTouch_GetPadValue(*(pads+i));
        #endif
    }

    slider_struct.get_value_hook = GetSliderValue;  //save Address of function in the hook
    slider_struct.slider_id = 0;

    return &slider_struct;
}
//------------------------------------------------------------------------------------------------//
DRVGPIO_PIN_HNDL DrvSliderSysTouch_RegisterButton(TOUCH_PAD button_pad)
{
    MODULE_CHECK();

    DRVGPIO_PIN_HNDL        pin_hndl;
    U16                     pad_id = buttonpad_count;

    //check all registred pins and check if it has been registred yet
    for(pin_hndl = gpio_pin_struct; pin_hndl < &gpio_pin_struct[pad_id]; pin_hndl++)
    {
        if(pin_hndl->pin_id == pad_id)
        {
            LOG_WRN("re-register port %d pin %d", PU8(io_port), PU8(io_pin_nr));
            return pin_hndl;  //same pin_id found, return same handle
        }
    }

    //failsafe check
    if(buttonpad_count > MAX_BUTTON_PAD_COUNT)
    {
        LOG_ERR("Pin register count overrun");
        return NULL;   //pin hndl null means no pin
    }

    //init lower sys lib
    SysTouch_InitPad(button_pad);

    //save first value as ambient value
    #ifdef FIXED_AMBIENT_VALUE
        buttonpad_ambient_values[pad_id] = FIXED_AMBIENT_VALUE;
    #else
        buttonpad_ambient_values[pad_id] = SysTouch_GetPadValue(button_pad);
    #endif
    buttonpads[pad_id] = button_pad;//save pad ref in arry

    //create handle
    pin_hndl->pin_id = pad_id;
    pin_hndl->hook_list_ptr = (GPIO_DRV_HOOK_LIST*)&gpio_pin_hook_list;
    buttonpad_count++;
    return pin_hndl;
}
//================================================================================================//
