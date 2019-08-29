//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Angle implementation specific for PICAN/PCMS PT1024
// Known restrictions:
//   - only 1 ANGLE_HNDL at runtime, so not possible to register for 2 handles
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define ANGLE_DRVANGLEPT1024_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef DRV_ANGLE_PT1024_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               DRV_ANGLE_PT1024_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of pulses that the sw should auto correct in case they were lost or faulty picked up
#ifndef MAX_ALLOWED_NOISE_PULSES
	#define MAX_ALLOWED_NOISE           (5 << 6)  //convert to fraction
#else
	#define MAX_ALLOWED_NOISE           (MAX_ALLOWED_NOISE_PULSES << 6)
#endif
//------------------------------------------------------------------------------------------------//
// @brief  softnr 14034 for indicating if 1024 pulses are received
#ifndef IDICATED_PT1024_FAULT
	#define IDICATED_PT1024_FAULT        0  //convert to fraction
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section
#include "gpio\DrvGpio.h"
#include "gpio\DrvGpioSys.h"
#include "angle\DrvAnglePt1024.h"

//STD lib include section

//COM lib include section

//APP include section
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
BOOL OnIndexPulse(ANGLE_BUS angle_bus, U32 angle_at_index_pulse);
void OnCompare(ANGLE_BUS angle_bus, U32 compare_angle);
BOOL GlueFunc_AngleConfig(ANGLE_ID angle_id, ANGLE_CONFIG_STRUCT* config_struct_ptr);
BOOL GlueFunc_AngleSetCompareAngle(ANGLE_ID angle_id, U32 angle);
void GlueFunc_AngleSetAbsolutePosition(ANGLE_ID angle_id, U32 angle);
U32 GlueFunc_AngleGetAbsolutePosition(ANGLE_ID angle_id);
U16 GlueFunc_AngleGetRelativePosition(ANGLE_ID angle_id);
void GlueFunc_AngleSetCountingMode(ANGLE_ID angle_id, COUNTING_MODE mode);
#if IDICATED_PT1024_FAULT
static void GlueFunc_AngleResetErrorPin(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();
static ANGLE_HOOK_LIST                 angle_hook_list;
static ANGLE_STRUCT                    angle_struct;
#if IDICATED_PT1024_FAULT
static DRVGPIO_PIN_HNDL                ledpin_erroroccurred_hndl;
static DRVGPIO_PIN_HNDL                errortriggerpin_hndl;
static DRVGPIO_PIN_HNDL                ledpin_errorcycle_hndl;
static DRVGPIO_PIN_HNDL                ledpin_goodcycle_hndl;
#endif
static PT1024_ZERO_PULSE_DIAGNOSTIC_HOOK    on_zero_pulse_diagnostic_hook;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
BOOL OnIndexPulse(ANGLE_BUS angle_bus, U32 angle_at_index_pulse)
{
    U32 angle_now;
    U32 diff;
    S16 error;
    //don't call DrvAngl for this: use our own implementation for this !

    //use the fractional part to give the error, it should be zero to be perfect
    //  if error < -MAX_ALLOWED_NOISE : noise error !!
    //  if -MAX_ALLOWED_NOISE <= error < -MAX_ALLOWED_NOISE/2: noise warning !!
    //  if error > MAX_ALLOWED_NOISE : noise error !!
    //  if MAX_ALLOWED_NOISE >= error > MAX_ALLOWED_NOISE/2: noise warning !!
    error = (S16)(angle_at_index_pulse & 0xFFFF);
    if(error != 0)
    {
        #if IDICATED_PT1024_FAULT
        DrvGpio_SetPin(ledpin_erroroccurred_hndl,   TRUE);
        DrvGpio_SetPin(errortriggerpin_hndl,        TRUE);
        #endif

        //Always correct, different from implementation in platform 1 !!!
        //We must always correct otherwise the FULL U32 angle format will be screwed up.
        //Because the sw cycle counter is incremented/decremented on the zeropuls, the fractional part must be reset to zero as well !
        //Otherwise you end up with a cycle jump !!
        angle_now = SysAnglePt1024_Bus_GetAbsolutePosition((ANGLE_BUS)angle_struct.angle_id);
        diff = angle_now - angle_at_index_pulse;
        angle_at_index_pulse += 0x8000;        //in case we lost some pulses
        angle_at_index_pulse &= 0xFFFF0000;    //only keep the cycle info

        SysAnglePt1024_Bus_SetAbsolutePosition((ANGLE_BUS)angle_struct.angle_id, angle_at_index_pulse + diff);
        LOG_DEV("Pt1024 widePulse correction: %d (%d pulses)", PS16(error), PS16(error>>6));
        if((error < -MAX_ALLOWED_NOISE) || (error > MAX_ALLOWED_NOISE))             //NOK -> error!!
        {
            //LOG_ERR("Pt1024 widePulse noise: error=%d", PS16(error));
            if(on_zero_pulse_diagnostic_hook != NULL)
            {
                on_zero_pulse_diagnostic_hook(TRUE, (error>>6));
            }
        }
        else if((error < -MAX_ALLOWED_NOISE/2) || (error > MAX_ALLOWED_NOISE/2))    //NOK -> warning!!
        {
            //LOG_WRN("Pt1024 widePulse noise: error=%d", PS16(error));
            if(on_zero_pulse_diagnostic_hook != NULL)
            {
                on_zero_pulse_diagnostic_hook(FALSE, (error>>6));
            }
        }

        #if IDICATED_PT1024_FAULT
        DrvGpio_SetPin(errortriggerpin_hndl,        FALSE);
        DrvGpio_SetPin(ledpin_errorcycle_hndl,      TRUE);
        DrvGpio_SetPin(ledpin_goodcycle_hndl,       FALSE);
        #endif

        return TRUE; // yes we corrected the position, so caller-code, please don't touch the software cycle counter anymore!!
    }
    #if IDICATED_PT1024_FAULT
    DrvGpio_SetPin(ledpin_errorcycle_hndl,      FALSE);
    DrvGpio_SetPin(ledpin_goodcycle_hndl,       TRUE);
    #endif
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
void OnCompare(ANGLE_BUS angle_bus, U32 compare_angle)
{
    //simplified code since we only have 1 runtime instance
    DrvAngle_OnCompare(&angle_struct, compare_angle);
}
//------------------------------------------------------------------------------------------------//
BOOL GlueFunc_AngleConfig(ANGLE_ID angle_id, ANGLE_CONFIG_STRUCT* config_struct_ptr)
{
    return SysAnglePt1024_Bus_Config((ANGLE_BUS)angle_id, config_struct_ptr);
}
//------------------------------------------------------------------------------------------------//
BOOL GlueFunc_AngleSetCompareAngle(ANGLE_ID angle_id, U32 angle)
{
    return SysAnglePt1024_Bus_SetCompareAngle((ANGLE_BUS)angle_id, angle);
}
//------------------------------------------------------------------------------------------------//
void GlueFunc_AngleSetAbsolutePosition(ANGLE_ID angle_id, U32 angle)
{
    SysAnglePt1024_Bus_SetAbsolutePosition((ANGLE_BUS)angle_id, angle);
}
//------------------------------------------------------------------------------------------------//
U32 GlueFunc_AngleGetAbsolutePosition(ANGLE_ID angle_id)
{
    return SysAnglePt1024_Bus_GetAbsolutePosition((ANGLE_BUS)angle_id);
}
//------------------------------------------------------------------------------------------------//
U16 GlueFunc_AngleGetRelativePosition(ANGLE_ID angle_id)
{
    return SysAnglePt1024_Bus_GetRelativePosition((ANGLE_BUS)angle_id);
}
//------------------------------------------------------------------------------------------------//
void GlueFunc_AngleSetCountingMode(ANGLE_ID angle_id, COUNTING_MODE mode)
{
    SysAnglePt1024_Bus_SetCountingMode((ANGLE_BUS)angle_id, mode);
}
//------------------------------------------------------------------------------------------------//
#if IDICATED_PT1024_FAULT
static void GlueFunc_AngleResetErrorPin(void)
{
    DrvGpio_SetPin(ledpin_erroroccurred_hndl,   FALSE);
    DrvGpio_SetPin(ledpin_errorcycle_hndl,      FALSE);
    DrvGpio_SetPin(ledpin_goodcycle_hndl,       FALSE);
    DrvGpio_SetPin(errortriggerpin_hndl,        FALSE);
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvAnglePt1024_Init(void)
{
    MODULE_INIT_ONCE();
    //place init code which must only be executed once here
    SysAnglePt1024_Init();
    SysAnglePt1024_RegisterOnIndexPulse(OnIndexPulse);
    SysAnglePt1024_RegisterOnCompare(OnCompare);

    angle_struct.angle_id = 0;
    angle_struct.cycle_step =  0x10000;
    angle_struct.hook_list_ptr = NULL;

    on_zero_pulse_diagnostic_hook = NULL;

    //init glue
    angle_hook_list.config_hook = GlueFunc_AngleConfig;
    angle_hook_list.get_absolute_position_hook = GlueFunc_AngleGetAbsolutePosition;
    angle_hook_list.get_relative_position_hook = GlueFunc_AngleGetRelativePosition;
    angle_hook_list.set_absolute_position_hook = GlueFunc_AngleSetAbsolutePosition;
    angle_hook_list.set_compare_angle_hook = GlueFunc_AngleSetCompareAngle;
    angle_hook_list.set_counting_mode_hook = GlueFunc_AngleSetCountingMode;

    #if IDICATED_PT1024_FAULT
    ledpin_erroroccurred_hndl   = DrvGpioSys_RegisterPin(GPIO_PORT_5, 6, PIN_OUTPUT_PUSH_PULL_DRV_WEAK); //RUN_LED
    ledpin_errorcycle_hndl      = DrvGpioSys_RegisterPin(GPIO_PORT_6, 0, PIN_OUTPUT_PUSH_PULL_DRV_WEAK); //FD2_LED
    ledpin_goodcycle_hndl       = DrvGpioSys_RegisterPin(GPIO_PORT_6, 1, PIN_OUTPUT_PUSH_PULL_DRV_WEAK); //FD3_LED
    errortriggerpin_hndl        = DrvGpioSys_RegisterPin(GPIO_PORT_1, 0, PIN_OUTPUT_PUSH_PULL_DRV_WEAK);

    DrvGpio_SetPin(ledpin_erroroccurred_hndl,   FALSE);
    DrvGpio_SetPin(ledpin_errorcycle_hndl,      FALSE);
    DrvGpio_SetPin(ledpin_goodcycle_hndl,       FALSE);
    DrvGpio_SetPin(errortriggerpin_hndl,        FALSE);

    CoreTerm_RegisterCommand("ResetErrorPin","resets the pin that indicates an pt1024 error", 0, GlueFunc_AngleResetErrorPin, TRUE);
    #endif

    MODULE_INIT_DONE();
    //place init code which must executed on every re-init here

}
//------------------------------------------------------------------------------------------------//
ANGLE_HNDL DrvAnglePt1024_Register(ANGLE_BUS angle_bus)
{
    //simplified code since we only have 1 runtime instance
    if(SysAnglePt1024_Bus_Init(angle_bus) == TRUE)
    {
        angle_struct.hook_list_ptr = &angle_hook_list;
        angle_struct.angle_id = angle_bus;
        return &angle_struct;
    }
    LOG_ERR("illegal bus - %d", PU8(angle_bus));
    return NULL;
}
//------------------------------------------------------------------------------------------------//
U32 DrvAnglePt1024_GetFullWmPos(void)
{
    if(angle_struct.hook_list_ptr == NULL)
    {
        LOG_WRN("Pt1024 not known yet");
        return 0;
    }
    return angle_struct.hook_list_ptr ->get_absolute_position_hook(angle_struct.angle_id);
}
//------------------------------------------------------------------------------------------------//
U16 DrvAnglePt1024_GetRelativeWmPos(void)
{
    if(angle_struct.hook_list_ptr == NULL)
    {
        LOG_WRN("Pt1024 not known yet");
        return 0;
    }
    return angle_struct.hook_list_ptr ->get_relative_position_hook(angle_struct.angle_id);
}
//------------------------------------------------------------------------------------------------//
void DrvAnglePt1024_InstallZeroPulseDiagnosticHook(PT1024_ZERO_PULSE_DIAGNOSTIC_HOOK hook)
{
    on_zero_pulse_diagnostic_hook = hook;
}
//================================================================================================//
