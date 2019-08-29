//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this module contains all the logic to contol the multiplexed rgb leds
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define AppLogic_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef AppLogic_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               AppLogic_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
#include "gpio\DrvGpio.h"
#include "gpio\DrvGpioSys.h"
#include "timer\DrvPwmSys.h"

//STD lib include section
#include "math\StdMath.h"

//COM lib include section

//APP include section
#include "AppLeds.h"
#include "AppLogic.h"
#include "AppTouch.h"
#include "AppLedStrip.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    LOGIC_MODE_INACTIVE,
    LOGIC_MODE_ACTIVE,
    LOGIC_MODE_ANIMATION
}
LOGIC_MODE;

typedef enum
{
    TOUCH_STATE_NONE,
    TOUCH_STATE_SLIDER,
    TOUCH_STATE_BUTTON
}
TOUCH_STATE;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void AppLogic_ChangeMode(LOGIC_MODE mode);
static void Task_AutoColorChanger(VPTR data_ptr);
static void Task_AutoColorChanger_Ledstrip(VPTR data_ptr);
static void Task_GetTouchStatus(VPTR data_ptr);
static void DecodeTouches(U16 x, U16 y);
static void SetLeds(U16 hoek);
static void AnitmateLeds(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static TASK_HNDL	            task_hndl_AutoColorChanger = NULL;
static TASK_HNDL	            task_hndl_AutoColorChanger_Ledstrip = NULL;
static U16                          x_cor;
static U16                          y_cor;

static U16                          slider_hoek;
static U16                          inactive_counter;
static LOGIC_MODE                   logic_mode;
static TOUCH_STATE                  touch_state;

static const U16 x_c[361] = {500, 507, 514, 521, 528, 535, 542, 549, 556, 563, 569, 576, 583, 590, 597, 603, 610, 617, 624, 630, 637, 643, 650, 656, 663, 669, 675, 682, 688, 694, 700, 706, 712, 718, 724, 729, 735, 741, 746, 752, 757, 762, 768, 773, 778, 783, 788, 792, 797, 802, 806, 811, 815, 819, 823, 828, 832, 835, 839, 843, 846, 850, 853, 856, 859, 862, 865, 868, 871, 873, 876, 878, 880, 882, 884, 886, 888, 890, 891, 893, 894, 895, 896, 897, 898, 898, 899, 899, 900, 900, 900, 900, 900, 899, 899, 899, 898, 897, 896, 895, 894, 893, 891, 890, 888, 886, 885, 883, 881, 878, 876, 874, 871, 868, 866, 863, 860, 857, 853, 850, 847, 843, 839, 836, 832, 828, 824, 820, 815, 811, 807, 802, 798, 793, 788, 783, 778, 773, 768, 763, 757, 752, 747, 741, 736, 730, 724, 718, 712, 706, 700, 694, 688, 682, 676, 670, 663, 657, 650, 644, 637, 631, 624, 618, 611, 604, 597, 591, 584, 577, 570, 563, 556, 549, 542, 535, 529, 522, 515, 508, 501, 494, 487, 480, 473, 466, 459, 452, 445, 438, 431, 424, 418, 411, 404, 397, 390, 384, 377, 370, 364, 357, 351, 344, 338, 332, 325, 319, 313, 307, 301, 295, 289, 283, 277, 271, 266, 260, 254, 249, 243, 238, 233, 228, 223, 218, 213, 208, 203, 199, 194, 190, 185, 181, 177, 173, 169, 165, 161, 158, 154, 151, 147, 144, 141, 138, 135, 132, 129, 127, 124, 122, 120, 118, 116, 114, 112, 110, 109, 108, 106, 105, 104, 103, 102, 102, 101, 101, 100, 100, 100, 100, 100, 100, 101, 101, 102, 103, 104, 105, 106, 107, 109, 110, 112, 113, 115, 117, 119, 121, 124, 126, 129, 131, 134, 137, 140, 143, 146, 150, 153, 157, 160, 164, 168, 172, 176, 180, 184, 188, 193, 197, 202, 207, 211, 216, 221, 226, 232, 237, 242, 247, 253, 258, 264, 270, 275, 281, 287, 293, 299, 305, 311, 317, 324, 330, 336, 343, 349, 356, 362, 369, 375, 382, 389, 395, 402, 409, 416, 422, 429, 436, 443, 450, 457, 464, 471, 478, 485, 492, 499};
static const U16 y_c[361] = {900, 900, 900, 899, 899, 898, 898, 897, 896, 895, 894, 893, 891, 890, 888, 886, 885, 883, 880, 878, 876, 873, 871, 868, 865, 863, 860, 856, 853, 850, 846, 843, 839, 836, 832, 828, 824, 820, 815, 811, 807, 802, 797, 793, 788, 783, 778, 773, 768, 763, 757, 752, 746, 741, 735, 730, 724, 718, 712, 706, 700, 694, 688, 682, 676, 669, 663, 657, 650, 644, 637, 630, 624, 617, 611, 604, 597, 590, 583, 577, 570, 563, 556, 549, 542, 535, 528, 521, 514, 507, 500, 493, 486, 479, 472, 465, 459, 452, 445, 438, 431, 424, 417, 410, 404, 397, 390, 383, 377, 370, 364, 357, 351, 344, 338, 331, 325, 319, 313, 306, 300, 294, 288, 283, 277, 271, 265, 260, 254, 249, 243, 238, 233, 228, 222, 217, 213, 208, 203, 198, 194, 189, 185, 181, 177, 173, 169, 165, 161, 157, 154, 150, 147, 144, 141, 138, 135, 132, 129, 127, 124, 122, 120, 118, 116, 114, 112, 110, 109, 107, 106, 105, 104, 103, 102, 102, 101, 101, 100, 100, 100, 100, 100, 101, 101, 101, 102, 103, 104, 105, 106, 107, 109, 110, 112, 113, 115, 117, 119, 122, 124, 126, 129, 132, 134, 137, 140, 143, 146, 150, 153, 157, 160, 164, 168, 172, 176, 180, 184, 189, 193, 198, 202, 207, 212, 217, 222, 227, 232, 237, 242, 248, 253, 259, 264, 270, 276, 281, 287, 293, 299, 305, 311, 318, 324, 330, 337, 343, 349, 356, 362, 369, 376, 382, 389, 396, 402, 409, 416, 423, 430, 437, 443, 450, 457, 464, 471, 478, 485, 492, 499, 506, 513, 520, 527, 534, 541, 548, 555, 562, 568, 575, 582, 589, 596, 603, 609, 616, 623, 629, 636, 642, 649, 655, 662, 668, 674, 681, 687, 693, 699, 705, 711, 717, 723, 729, 734, 740, 745, 751, 756, 762, 767, 772, 777, 782, 787, 792, 797, 801, 806, 810, 815, 819, 823, 827, 831, 835, 839, 842, 846, 849, 853, 856, 859, 862, 865, 868, 870, 873, 875, 878, 880, 882, 884, 886, 888, 889, 891, 892, 894, 895, 896, 897, 898, 898, 899, 899, 900, 900, 900};
//static const U16 temparture[361] = {65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65535, 65444, 65353, 65262, 65171, 65080, 64989, 64898, 64807, 64716, 64625, 64534, 64443, 64352, 64261, 64170, 64079, 63988, 63897, 63806, 63715, 63624, 63533, 63442, 63350, 63259, 63168, 63077, 62986, 62895, 62804, 62713, 62622, 62531, 62440, 62349, 62258, 62167, 62076, 61985, 61894, 61803, 61712, 61621, 61530, 61439, 61348, 61257, 61166, 61075, 60984, 60893, 60802, 60711, 60620, 60529, 60438, 60347, 60256, 60165, 60074, 59983, 59892, 59801, 59710, 59619, 59528, 59437, 59346, 59255, 59164, 59073, 58981, 58890, 58799, 58708, 58617, 58526, 58435, 58344, 58253, 58162, 58071, 57980, 57889, 57798, 57707, 57616, 57525, 57434, 57343, 57252, 57161, 57070, 56979, 56888, 56797, 56706, 56615, 56524, 56433, 56342, 56251, 56160, 56069, 55978, 55887, 55796, 55705, 55614, 55523, 55432, 55341, 55250, 55159, 55068, 54977, 54886, 54795, 54704, 54612, 54521, 54430, 54339, 54248, 54157, 54066, 53975, 53884, 53793, 53702, 53611, 53520, 53429, 53338, 53247, 53156, 53065, 52974, 52883, 52792, 52701, 52610, 52519, 52428, 52337, 52246, 52155, 52064, 51973, 51882, 51791, 51700, 51609, 51518, 51427, 51336, 51245, 51154, 51063, 50972, 50881, 50790, 50699, 50608, 50517, 50426, 50335, 50243, 50152, 50061, 49970, 49879, 49788, 49697, 49606, 49515, 49424, 49333, 49242, 49151, 49060, 48969, 48878, 48787, 48696, 48605, 48514, 48423, 48332, 48241, 48150, 48059, 47968, 47877, 47786, 47695, 47604, 47513, 47422, 47331, 47240, 47149, 47058, 46967, 46876, 46785, 46694, 46603, 46512, 46421, 46330, 46239, 46148, 46057, 45966, 45874, 45783, 45692, 45601, 45510, 45419, 45328, 45237, 45146, 45055, 44964, 44873, 44782, 44691, 44600, 44509, 44418, 44327, 44236, 44145, 44054, 43963, 43872, 43781, 43690, 43599, 43508, 43417, 43326, 43235, 43144, 43053, 42962, 42871, 42780, 42689, 42598, 42507, 42416, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690, 43690};

static U8 animation_offset;
static COLOR_HSV color_var1 = {200, 100, 100};
static COLOR_HSV color_var2 = {200, 100, 100};
static COLOR_HSV color_fixed = {200, 100, 0};

static COLOR_HSV  auto_color_hsv ={0,0,0};
static COLOR_HSV  auto_color_hsv_ledstrip={0,0,0};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void AppLogic_ChangeMode(LOGIC_MODE mode)
{
    switch(logic_mode)
    {
    case LOGIC_MODE_INACTIVE:
        CoreTask_Stop(task_hndl_AutoColorChanger);
        break;
    case LOGIC_MODE_ACTIVE:
        CoreTask_Start(task_hndl_AutoColorChanger_Ledstrip);
        break;
    default:
    case LOGIC_MODE_ANIMATION:
        CoreTask_Start(task_hndl_AutoColorChanger_Ledstrip);
//        CoreTask_SetPeriod(task_hndl_AutoColorChanger_Ledstrip, 50e3);
        break;
    }

    switch(mode)
    {
    case LOGIC_MODE_INACTIVE:
        CoreTask_Start(task_hndl_AutoColorChanger);
        break;
    case LOGIC_MODE_ACTIVE:
        CoreTask_Stop(task_hndl_AutoColorChanger_Ledstrip);
        break;
    default:
    case LOGIC_MODE_ANIMATION:
        animation_offset = 0;
        color_var1.value = 0;
        auto_color_hsv_ledstrip.value = 0;
//        CoreTask_SetPeriod(task_hndl_AutoColorChanger_Ledstrip, 1e3);
        CoreTask_Stop(task_hndl_AutoColorChanger_Ledstrip);
        AppLedStrip_SetRgb(CoreConvert_HsvToRgb(auto_color_hsv_ledstrip));
        break;
    }

    logic_mode = mode;
}
//------------------------------------------------------------------------------------------------//
static void Task_AutoColorChanger(VPTR data_ptr)
{
    //if task task is active change the hue of the color FOR THE LEDRING

    //in auto mode always 100% saturation and brighness, if lower (@startup) ramp up to that
    if (auto_color_hsv.value < 50)
    {
        auto_color_hsv.value++;
    }
    if (auto_color_hsv.saturation < 100)
    {
        auto_color_hsv.saturation = 100;
    }

    //change the hue
    if (auto_color_hsv.hue <360)
    {
        auto_color_hsv.hue+=4;
    }
    else
    {
        auto_color_hsv.hue = 0;
    }

    //set ledring
    COLOR_HSV individual_led_color;
    individual_led_color = auto_color_hsv;

    U8 i;
    for(i = 0; i < 12; i++)
    {
        AppLeds_SetLed((LEDS) (1<<i),CoreConvert_HsvToRgb(individual_led_color));
        individual_led_color.hue += (360/12);
        if(individual_led_color.hue >=360)
        {
            individual_led_color.hue -=360;
        }
    }

}//------------------------------------------------------------------------------------------------//
static void Task_AutoColorChanger_Ledstrip(VPTR data_ptr)
{
    //if task task is active change the hue of the color FOR THE LEDSTRIP

    //in auto mode always 100% saturation and brighness, if lower (@startup) ramp up to that
    if (auto_color_hsv_ledstrip.value < 10)
    {
        auto_color_hsv_ledstrip.value++;
    }
    if (auto_color_hsv_ledstrip.saturation < 100)
    {
        auto_color_hsv_ledstrip.saturation = 100;
    }

    //change the hue
    if (auto_color_hsv_ledstrip.hue <360)
    {
        auto_color_hsv_ledstrip.hue++;
    }
    else
    {
        auto_color_hsv_ledstrip.hue = 0;
    }

    COLOR_RGB ledstrip_color;

    ledstrip_color = CoreConvert_HsvToRgb(auto_color_hsv_ledstrip);
    //reduce brightness van leds
//    ledstrip_color.red = ledstrip_color.red/10;
//    ledstrip_color.green = ledstrip_color.green/10;
//    ledstrip_color.blue = ledstrip_color.blue/10;

    AppLedStrip_SetRgb(ledstrip_color);
}
//------------------------------------------------------------------------------------------------//
static void Task_GetTouchStatus(VPTR data_ptr)
{
    U16 x, y;
    U8 touches = 0; //AppTouch_GetTouch(&x, &y);

    if(touches > 0)
    {
        y_cor = 1024 - x;
        x_cor = y;
//        LOG_TRM("%c%c;%c%c", PU8((x_cor>>8) & 0xFF),PU8(x_cor), PU8(y_cor>>8),PU8(y_cor));
        //LOG_TRM("%d;%d;%d", PU8((data_buffer[0] & 0x0F),PU16(x), PU16(y));
        DecodeTouches(x_cor, y_cor);
        inactive_counter = 0;
    }
    else
    {
        //happens every 10ms
        inactive_counter++;
        if(inactive_counter >= 1333) //20 000 / 15
        {
            //inactivtie for 1 minut
            inactive_counter = 0;
            AppLogic_ChangeMode(LOGIC_MODE_INACTIVE);
        }
        touch_state = TOUCH_STATE_NONE;
    }
    U8 touches2 = AppTouch_GetTouch2(&x, &y);
    switch(logic_mode)
    {
    case LOGIC_MODE_INACTIVE:
        break;
    case LOGIC_MODE_ACTIVE:
        SetLeds(slider_hoek);
        break;
    default:
    case LOGIC_MODE_ANIMATION:
        AnitmateLeds();
        break;
    }
}
//------------------------------------------------------------------------------------------------//
static void DecodeTouches(U16 x, U16 y)
{
    U32 min = 0xFFFFFFFF;
    U32 i_value = 0xFFFFFFFF;
    U32 result;

    if( (x > 250) && (x < 720) && (y > 880) && ((touch_state == TOUCH_STATE_NONE) || (touch_state == TOUCH_STATE_BUTTON)))
    {
        if(logic_mode != LOGIC_MODE_ANIMATION)
        {
            AppLogic_ChangeMode(LOGIC_MODE_ANIMATION);
        }

        if((animation_offset >= 12))
        {
            animation_offset = 0;
        }
        touch_state = TOUCH_STATE_BUTTON;
    }
    else if((touch_state == TOUCH_STATE_NONE) || (touch_state == TOUCH_STATE_SLIDER))
    {
        AppLogic_ChangeMode(LOGIC_MODE_ACTIVE);

        for(int i = 0; i < 360; i++)
        {
            S32 delta_x = x - x_c[i];
            S32 delta_y = y - y_c[i];
            U32 schuin = (delta_x * delta_x) + (delta_y * delta_y);
            if(schuin < min)
            {
                min = schuin;
                i_value = 360 - i;
                result = schuin;
            }
        }

        if(result < 20000 && i_value > 50 && i_value < 310)
        {
            if(i_value > 300)
            {
                i_value = 300;
            }
            if(i_value < 60)
            {
                i_value = 60;
            }

            slider_hoek = i_value;
            touch_state = TOUCH_STATE_SLIDER;
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void SetLeds(U16 hoek)
{
    COLOR_RGB color;
    COLOR_HSV my_color_hsv;

    U16 hoek_teller = 60;
    U16 temp = 0;

     for(int i = 9; i < 12; i++)
     {
         if(hoek_teller < hoek)
         {
            color.red = 255;
            color.green = 0;
            color.blue = 0;
         }
         else
         {
             if((hoek_teller -30) < hoek)
             {
                temp = ((hoek - hoek_teller) * 255) / 30;
                color.red = temp;
                color.green = 0;
                color.blue = 255 - temp;
             }
             else
             {
                color.red = 0;
                color.green = 0;
                color.blue = 255;
             }
         }
          hoek_teller += 30;
          AppLeds_SetLed((LEDS)(1<<i),color);
     }

    for(int i = 0; i < 6; i++)
    {
        if(hoek_teller <= hoek)
        {
            color.red = 255;
            color.green = 0;
            color.blue = 0;
        }
        else
        {
             if( hoek_teller -30 < hoek)
             {
                temp = ((hoek - hoek_teller) * 255) / 30;
                color.red = temp;
                color.green = 0;
                color.blue = 255 - temp;
             }
             else
             {
                color.red = 0;
                color.green = 0;
                color.blue = 255;
             }
        }
        hoek_teller += 30;
        AppLeds_SetLed((LEDS)(1<<i),color);
    }

    for(int i = 6; i < 9; i++)
    {
        color.red = 0;
        color.green = 0;
        color.blue = 0;
        AppLeds_SetLed((LEDS)(1<<i),color);
    }

    my_color_hsv.hue = 240 - (hoek - 60);
    my_color_hsv.value = 10;
    my_color_hsv.saturation = 100;

    AppLedStrip_SetRgb(CoreConvert_HsvToRgb(my_color_hsv));
}
//------------------------------------------------------------------------------------------------//
static void AnitmateLeds(void)
{
    static COLOR_HSV* test [12] = {&color_fixed, &color_fixed, &color_fixed, &color_fixed, &color_var1, &color_var2, &color_fixed, &color_fixed, &color_fixed, &color_fixed, &color_fixed, &color_fixed};

    S16 temp = 0;

    for(U8 i = 0; i < 12; i++)
    {
       temp = i + animation_offset;
       while(temp > 11)
       {
           temp -= 12;
       }

       temp -= 11;
       if(temp < 0)
       {
           temp = - temp;
       }
       AppLeds_SetLed((LEDS)(1<<i), CoreConvert_HsvToRgb(*test[temp]));
//       AppLeds_SetLed((LEDS)(1<<i), *test[temp]);
    }

     color_var1.hue ++;
     color_var2.hue ++;

    if(color_var1.value >= 100)
    {
        animation_offset++;
        color_var1.value = 0;
        color_var2.value = 100;
    }

    if((animation_offset > 12))
    {
        AppLogic_ChangeMode(LOGIC_MODE_INACTIVE);
    }

    color_var1.value += 50;
    color_var2.value = 100 - color_var1.value;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void AppLogic_Init(void)
{
    MODULE_INIT_ONCE();


    task_hndl_AutoColorChanger = CoreTask_RegisterTask(5e3, Task_AutoColorChanger, NULL, 200, "");
    CoreTask_Start(task_hndl_AutoColorChanger);
    task_hndl_AutoColorChanger_Ledstrip = CoreTask_RegisterTask(50e3, Task_AutoColorChanger_Ledstrip, NULL, 200, "");
    CoreTask_Start(task_hndl_AutoColorChanger_Ledstrip);

    inactive_counter = 0;
    animation_offset = 0;
    touch_state = TOUCH_STATE_NONE;

    CoreTask_Start(CoreTask_RegisterTask(15000, Task_GetTouchStatus, (VPTR)NULL, 128, "Button"));             // 10ms
    
    MODULE_INIT_DONE();
}

//================================================================================================//
