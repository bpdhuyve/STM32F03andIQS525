//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// This module handles the motion of a bipolar stepper motor
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define TESTLIBMOTIONBIPOLARSTEPPER_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef TESTLIBMOTIONBIPOLARSTEPPER_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               TESTLIBMOTIONBIPOLARSTEPPER_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef TEST_LIB_MOTION_STEPPER_COUNT
    #define TEST_LIB_MOTION_STEPPER_COUNT           SPI_CHANNEL_COUNT
#endif
//------------------------------------------------------------------------------------------------//
#ifndef TEST_LIB_MOTION_INCLUDE_AMIS30521
    #define TEST_LIB_MOTION_INCLUDE_AMIS30521       1
#endif
//------------------------------------------------------------------------------------------------//
#ifndef TEST_LIB_MOTION_INCLUDE_AMIS30422
    #define TEST_LIB_MOTION_INCLUDE_AMIS30422       1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DUCO LIB
#include "TestLibMotionBipolarStepper.h"

// DRV
#if TEST_LIB_MOTION_INCLUDE_AMIS30521 > 0
    #include "stepper\DrvStepperAmis30521.h"
#endif
#if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
    #include "stepper\DrvStepperAmis30422.h"
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
    DRVGPIO_PIN_HNDL    amis_reset_pin;
    DRVGPIO_PIN_HNDL    amis_dir_pin;
    DRVGPIO_PIN_HNDL    amis_step_pin;
    DRVGPIO_PIN_HNDL    amis_error_pin;
    #if TEST_LIB_MOTION_INCLUDE_AMIS30521 > 0
        AMIS30521_HNDL      amis30521_hndl;
    #endif
    #if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
        AMIS30422_HNDL      amis30422_hndl;
    #endif
    AMIS_DEVICE         amis_device;
}
STEPPER_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void TestLibMotion_DoStep(void);
static void TestLibMotion_SetDir(void);
static void TestLibMotion_SetCurrent(void);
static void TestLibMotion_SetStepSize(void);
static void TestLibMotion_Enable(void);
static void TestLibMotion_GetErrorStatus(void);
#if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
static void TestLibMotion_SetPredriverRegister(void);
#endif
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static STEPPER_CTRL_STRUCT      stepper_ctrl_struct[TEST_LIB_MOTION_STEPPER_COUNT];
static U8                       stepper_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void TestLibMotion_DoStep(void)
{
    if(CoreTerm_GetArgumentAsU32(0) < stepper_count)
    {
        DrvGpio_SetPin(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_step_pin, TRUE);
        DrvGpio_SetPin(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_step_pin, FALSE);
        CoreTerm_PrintAcknowledge();
        return;
    }
    CoreTerm_PrintFailed();
}
//------------------------------------------------------------------------------------------------//
static void TestLibMotion_SetDir(void)
{
    if(CoreTerm_GetArgumentAsU32(0) < stepper_count)
    {
        DrvGpio_SetPin(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_dir_pin, (BOOL)CoreTerm_GetArgumentAsU32(1));
        CoreTerm_PrintAcknowledge();
        return;
    }
    CoreTerm_PrintFailed();
}
//------------------------------------------------------------------------------------------------//
static void TestLibMotion_SetStepSize(void)
{
    if(CoreTerm_GetArgumentAsU32(0) < stepper_count)
    {
        if(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_device == AMIS_30521)
        {
            #if TEST_LIB_MOTION_INCLUDE_AMIS30521 > 0
            if((AMIS30521_STEPSIZE)CoreTerm_GetArgumentAsU32(1) <= AMIS30521_STEP_SIZE_FULL_STEP)
            {
                CoreTerm_PrintFeedback((BOOL)DrvStepperAmis30521_SetStepsize(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30521_hndl, (AMIS30521_STEPSIZE)CoreTerm_GetArgumentAsU32(1)));
                return;
            }
            #endif
        }
        else
        {
            #if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
            if((AMIS30422_STEPSIZE)CoreTerm_GetArgumentAsU32(1) <= AMIS30422_STEP_SIZE_FULL_STEP_1_2_ROTATION)
            {
                CoreTerm_PrintFeedback((BOOL)DrvStepperAmis30422_SetStepsize(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30422_hndl, (AMIS30422_STEPSIZE)CoreTerm_GetArgumentAsU32(1)));
                return;
            }
            #endif
        }
    }
    CoreTerm_PrintFailed();
}
//------------------------------------------------------------------------------------------------//
static void TestLibMotion_SetCurrent(void)
{
    if(CoreTerm_GetArgumentAsU32(0) < stepper_count)
    {
        if(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_device == AMIS_30521)
        {
            #if TEST_LIB_MOTION_INCLUDE_AMIS30521 > 0
            if((AMIS30521_CURRENT)CoreTerm_GetArgumentAsU32(1) <= CURRENT_1600_MA)
            {
                CoreTerm_PrintFeedback((BOOL)DrvStepperAmis30521_SetCurrent(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30521_hndl, (AMIS30521_CURRENT)CoreTerm_GetArgumentAsU32(1)));
                return;
            }
            #endif
        }
        else
        {
            #if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
            if((AMIS30422_CURRENT)CoreTerm_GetArgumentAsU32(1) <= CURRENT_VREF_DIV_5_RSENSE)
            {
                CoreTerm_PrintFeedback((BOOL)DrvStepperAmis30422_SetCurrent(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30422_hndl, (AMIS30422_CURRENT)CoreTerm_GetArgumentAsU32(1)));
                return;
            }
            #endif
        }
    }
    CoreTerm_PrintFailed();
}
//------------------------------------------------------------------------------------------------//
static void TestLibMotion_Enable(void)
{
    if(CoreTerm_GetArgumentAsU32(0) < stepper_count)
    {
        if((BOOL)(CoreTerm_GetArgumentAsU32(1) > 0))
        {
            if(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_device == AMIS_30521)
            {
                #if TEST_LIB_MOTION_INCLUDE_AMIS30521 > 0
                CoreTerm_PrintFeedback((BOOL)(DrvStepperAmis30521_SetSleepState(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30521_hndl, FALSE) &&
                                          DrvStepperAmis30521_SetMotorState(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30521_hndl, TRUE)));
                #endif
            }
            else
            {
                #if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
                CoreTerm_PrintFeedback((BOOL)(DrvStepperAmis30422_SetSleepState(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30422_hndl, FALSE) &&
                                          DrvStepperAmis30422_SetMotorState(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30422_hndl, TRUE)));
                #endif
            }
        }
        else
        {
            if(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_device == AMIS_30521)
            {
                #if TEST_LIB_MOTION_INCLUDE_AMIS30521 > 0
                CoreTerm_PrintFeedback((BOOL)(DrvStepperAmis30521_SetMotorState(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30521_hndl, FALSE) &&
                                          DrvStepperAmis30521_SetSleepState(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30521_hndl, TRUE)));
                #endif
            }
            else
            {
                #if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
                CoreTerm_PrintFeedback((BOOL)(DrvStepperAmis30422_SetMotorState(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30422_hndl, FALSE) &&
                                          DrvStepperAmis30422_SetSleepState(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30422_hndl, TRUE)));
                #endif
            }
        }
        return;
    }
    CoreTerm_PrintFailed();
}
//------------------------------------------------------------------------------------------------//
static void TestLibMotion_GetErrorStatus(void)
{
    U16     i;
    U32     error_status;
    
    if(CoreTerm_GetArgumentAsU32(0) < stepper_count)
    {
        if(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_device == AMIS_30521)
        {
            #if TEST_LIB_MOTION_INCLUDE_AMIS30521 > 0
            error_status = DrvStepperAmis30521_GetErrorMask(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30521_hndl);
            #endif
        }
        else
        {
            #if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
            error_status = DrvStepperAmis30422_GetErrorMask(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30422_hndl);
            #endif
        }
        if(DrvGpio_GetPin(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_error_pin) == FALSE)
        {
            DrvGpio_SetPin(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_reset_pin, TRUE);
            for(i=0; i<500; i++)
            {
                asm("nop");
            }
            DrvGpio_SetPin(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_reset_pin, FALSE);
        }
        LOG_TRM("ERROR: 0x%08h", PU32(error_status));
        CoreTerm_PrintAcknowledge();
        return;
    }
    CoreTerm_PrintFailed();
}
//------------------------------------------------------------------------------------------------//
#if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
static void TestLibMotion_SetPredriverRegister(void)
{
    if((CoreTerm_GetArgumentAsU32(0) < stepper_count) && (CoreTerm_GetArgumentAsU32(1) <= 3))
    {
        if(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis_device == AMIS_30422)
        {
            if(DrvStepperAmis30422_WriteRegister(stepper_ctrl_struct[CoreTerm_GetArgumentAsU32(0)].amis30422_hndl, (AMIS30422_REGISTER)(AMIS30422_REGISTER_PDRV0 + CoreTerm_GetArgumentAsU32(1)), (U8)CoreTerm_GetArgumentAsU32(2)) == TRUE)
            {
                CoreTerm_PrintAcknowledge();
                return;
            }
        }
    }
    CoreTerm_PrintFailed();
}
#endif
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibMotionBipolarStepper_Init(void)
{
    MODULE_INIT_ONCE();
    
    CoreTerm_RegisterCommand("MotorDoStep", "MOTOR do step", 1, TestLibMotion_DoStep, TRUE);
    CoreTerm_RegisterCommand("MotorSetDir", "MOTOR set direction", 2, TestLibMotion_SetDir, TRUE);
    CoreTerm_RegisterCommand("MotorSetStepsize", "MOTOR set stepsize", 2, TestLibMotion_SetStepSize, TRUE);
    CoreTerm_RegisterCommand("MotorSetCurrent", "MOTOR set current", 2, TestLibMotion_SetCurrent, TRUE);
    CoreTerm_RegisterCommand("MotorSetState", "MOTOR enable/disable motor", 2, TestLibMotion_Enable, TRUE);
    CoreTerm_RegisterCommand("MotorErrorStatus", "MOTOR get error status", 1, TestLibMotion_GetErrorStatus, TRUE);
    #if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
        CoreTerm_RegisterCommand("MotorSetPredriver", "MOTOR set predriver register of AMIS30422, arg{Mx, Predriver register(0-3), value}", 3, TestLibMotion_SetPredriverRegister, TRUE);
    #endif
    
    MEMSET((VPTR)stepper_ctrl_struct, 0, SIZEOF(stepper_ctrl_struct));
    stepper_count = 0;
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void TestLibMotionBipolarStepper_Register(SPI_DEVICE_ID     amis_spi_device_id,
                                          DRVGPIO_PIN_HNDL  amis_reset_pin_hndl,
                                          DRVGPIO_PIN_HNDL  amis_dir_pin_hndl,
                                          DRVGPIO_PIN_HNDL  amis_step_pin_hndl,
                                          DRVGPIO_PIN_HNDL  amis_error_pin_hndl,
                                          AMIS_DEVICE       amis_device)
{
    
    if(stepper_count < TEST_LIB_MOTION_STEPPER_COUNT)
    {
        // init AMIS chip
        if(amis_device == AMIS_30521)
        {
            #if TEST_LIB_MOTION_INCLUDE_AMIS30521 > 0
            stepper_ctrl_struct[stepper_count].amis30521_hndl = DrvStepperAmis30521_Register(amis_spi_device_id);
            #endif
            #if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
            stepper_ctrl_struct[stepper_count].amis30422_hndl = NULL;
            #endif
        }
        else if(amis_device == AMIS_30422)
        {
            #if TEST_LIB_MOTION_INCLUDE_AMIS30422 > 0
            stepper_ctrl_struct[stepper_count].amis30422_hndl = DrvStepperAmis30422_Register(amis_spi_device_id);
            #endif
            #if TEST_LIB_MOTION_INCLUDE_AMIS30521 > 0
            stepper_ctrl_struct[stepper_count].amis30521_hndl = NULL;
            #endif
        }
        else
        {
            LOG_ERR("Unknown device type");
            return;
        }
        // init AMIS direct lines
        stepper_ctrl_struct[stepper_count].amis_reset_pin = amis_reset_pin_hndl;
        stepper_ctrl_struct[stepper_count].amis_dir_pin = amis_dir_pin_hndl;
        stepper_ctrl_struct[stepper_count].amis_step_pin = amis_step_pin_hndl;
        stepper_ctrl_struct[stepper_count].amis_error_pin = amis_error_pin_hndl;
        stepper_ctrl_struct[stepper_count].amis_device = amis_device;
        stepper_count++;
    }
}
//================================================================================================//