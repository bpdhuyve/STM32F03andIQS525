//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// this module contains all the logic to contol the multiplexed rgb leds
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define APPLEDS_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef APPLEDS_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               APPLEDS_LOG_LEVEL
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

//COM lib include section

//APP include section
#include "AppLeds.h"
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
static void Task_DriveLeds(VPTR data_ptr);
//static void Task_DelayedOn(VPTR data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static DRVGPIO_PIN_HNDL         led_select_pins[12];
//static DRVGPIO_PIN_HNDL         led_red;
//static DRVGPIO_PIN_HNDL         led_green;
//static DRVGPIO_PIN_HNDL         led_blue;

static PWM_CHANNEL_HNDL         pwm_leds_red, pwm_leds_green, pwm_leds_blue;
static TASK_HNDL	            task_hndl_DriveLeds = NULL;
//static TASK_HNDL	            task_hndl_Delayed_on = NULL;
static COLOR_RGB                led_values[12]; //this arry contains the led values that have to be visualised
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void Task_DriveLeds(VPTR data_ptr)
{
    static U8 selected_led_nr = 0;
    static U8 previous_led_nr;

    //turn old led off
    DrvGpio_SetPin(led_select_pins[previous_led_nr], FALSE);

    //change pwm
    DrvPwmSys_Timer_SetState(TIMER_1,FALSE);    //timer off
    DrvPwmSys_Timer_SetState(TIMER_3,FALSE);    //timer off
    DrvPwm_SetState(pwm_leds_red, FALSE);    //channel on
    DrvPwm_SetState(pwm_leds_green, FALSE);    //channel on
    DrvPwm_SetState(pwm_leds_blue, FALSE);    //channel on
    DrvPwm_SetDutyCycle(pwm_leds_red,   led_values[selected_led_nr].red<<8);
    DrvPwm_SetDutyCycle(pwm_leds_green, led_values[selected_led_nr].green<<8);
    DrvPwm_SetDutyCycle(pwm_leds_blue,  led_values[selected_led_nr].blue<<8);
    DrvPwmSys_Timer_SetState(TIMER_1,TRUE);    //timer on, this will reset the current counter value
    DrvPwmSys_Timer_SetState(TIMER_3,TRUE);    //timer on, this will reset the current counter value
    DrvPwm_SetState(pwm_leds_red, TRUE);    //channel on
    DrvPwm_SetState(pwm_leds_green, TRUE);    //channel on
    DrvPwm_SetState(pwm_leds_blue, TRUE);    //channel on

   //     CoreTask_Start(task_hndl_Delayed_on);


    /*
    old testcode
    DrvGpio_SetPin(led_red, led_values[selected_led_nr].red);
    DrvGpio_SetPin(led_green, led_values[selected_led_nr].green);
    DrvGpio_SetPin(led_blue, led_values[selected_led_nr].blue);
*/

    //new led on
    DrvGpio_SetPin(led_select_pins[selected_led_nr], TRUE);

    //cycle led count
    previous_led_nr = selected_led_nr;
    selected_led_nr++;
    if (selected_led_nr >= 12)
    {
        selected_led_nr = 0;
    }
}
//------------------------------------------------------------------------------------------------//
//static void Task_DelayedOn(VPTR data_ptr)
//{
//    DrvPwm_SetState(pwm_leds_red, TRUE);    //channel on
//    DrvPwm_SetState(pwm_leds_green, TRUE);    //channel on
//    DrvPwm_SetState(pwm_leds_blue, TRUE);    //channel on
//    CoreTask_Stop(task_hndl_Delayed_on); //stop yourself
//}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void AppLeds_Init(void)
{
    MODULE_INIT_ONCE();

    //set up led selector pins  //pisn are all on port b in sequence starting from pin0
    U8 i;
    for(i = 0; i < 12; i++)
    {
        SysPin_InitPin(GPIO_PORT_B,  i, PIN_OUTPUT_PUSH_PULL);          // LED
    }
    led_select_pins[0] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 1, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[1] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 2, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[2] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 10, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[3] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 11, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[4] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 3, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[5] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 4, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[6] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 5, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[7] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 6, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[8] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 7, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[9] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 8, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[10] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 9, PIN_OUTPUT_PUSH_PULL);
    led_select_pins[11] = DrvGpioSys_RegisterPin(GPIO_PORT_B, 0, PIN_OUTPUT_PUSH_PULL);

    //setup pwms
    DrvPwm_Init();
    DrvPwmSys_Init();
    DrvPwmSys_Timer_Init(TIMER_1, 100000, PWM_MODE_EDGE_ALIGN);
    DrvPwmSys_Timer_SetState(TIMER_1,TRUE);    //timer on
    DrvPwmSys_Timer_Init(TIMER_3, 100000, PWM_MODE_EDGE_ALIGN);
    DrvPwmSys_Timer_SetState(TIMER_3,TRUE);    //timer on

    //channel red
    SysPin_InitPin(GPIO_PORT_A, 8,  PIN_ALTERNAT_TIMER1_1_PUSH_PULL);
    pwm_leds_red = DrvPwmSys_Register(TIMER_1, PWM_CHANNEL_1, PWM_POLARITY_ACTIVE_HIGH);
    DrvPwm_SetState(pwm_leds_red, TRUE);    //channel on

    //channel green
    SysPin_InitPin(GPIO_PORT_A, 6,  PIN_ALTERNAT_TIMER3_1_PUSH_PULL);
    pwm_leds_green = DrvPwmSys_Register(TIMER_3, PWM_CHANNEL_1, PWM_POLARITY_ACTIVE_HIGH);
    DrvPwm_SetState(pwm_leds_green, TRUE);    //channel on

    //channel blue
    SysPin_InitPin(GPIO_PORT_A, 7,  PIN_ALTERNAT_TIMER3_2_PUSH_PULL);
    pwm_leds_blue = DrvPwmSys_Register(TIMER_3, PWM_CHANNEL_2, PWM_POLARITY_ACTIVE_HIGH);
    DrvPwm_SetState(pwm_leds_blue, TRUE);    //channel on

    //multiplex task
    task_hndl_DriveLeds = CoreTask_RegisterTask(1000, Task_DriveLeds, NULL, 100, "Task_DriveLeds"); //task op isr
    CoreTask_Start(task_hndl_DriveLeds);
  //  task_hndl_Delayed_on = CoreTask_RegisterTask(1 , Task_DelayedOn, NULL, 100, "task_hndl_Delayed_on");//2e3 fixed het maar dit is veel te traag

    /*
    old testcode
    led_red = DrvGpioSys_RegisterPin(GPIO_PORT_A, 8, PIN_OUTPUT_PUSH_PULL);
    led_green = DrvGpioSys_RegisterPin(GPIO_PORT_A, 6, PIN_OUTPUT_PUSH_PULL);
    led_blue = DrvGpioSys_RegisterPin(GPIO_PORT_A, 7, PIN_OUTPUT_PUSH_PULL);
*/
    //init led values to zero
    for(i = 0; i < 12; i++)
    {
        led_values[i].red = 0;
        led_values[i].green = 0;
        led_values[i].blue = 0;
    }

    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void AppLeds_SetLed(LEDS led, COLOR_RGB color)
{
    U8 i;
    for(i = 0; i < 12; i++)
    {
        if (led & (1<<i))  //led is selected
        {
            led_values[i] = color;//(COLOR_RGB){red, green, blue};
        }
    }
}
//================================================================================================//