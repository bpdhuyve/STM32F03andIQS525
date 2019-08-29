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
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DUCO LIB
#include "TestLibMotionBipolarStepper.h"

// DRV
#include "stepper\DrvStepperAmis30521.h"
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
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void TestLibMotion_DoStep(void);
static void TestLibMotion_SetDir(void);
static void TestLibMotion_Enable(void);
static void TestLibMotion_GetErrorStatus(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static DRVGPIO_PIN_HNDL                 amis_reset_pin;
static DRVGPIO_PIN_HNDL                 amis_dir_pin;
static DRVGPIO_PIN_HNDL                 amis_step_pin;
static DRVGPIO_PIN_HNDL                 amis_error_pin;
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
    DrvGpio_SetPin(amis_step_pin, TRUE);
    DrvGpio_SetPin(amis_step_pin, FALSE);
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void TestLibMotion_SetDir(void)
{
    DrvGpio_SetPin(amis_dir_pin, (BOOL)CoreTerm_GetArgumentAsU32(0));
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void TestLibMotion_Enable(void)
{
    if((BOOL)(CoreTerm_GetArgumentAsU32(0) > 0))
    {
        CoreTerm_PrintFeedback((BOOL)(DrvStepperAmis30521_SetSleepState(FALSE) &&
                                      DrvStepperAmis30521_SetStepsize(STEP_SIZE_FULL_STEP) &&
                                      DrvStepperAmis30521_SetCurrent(CURRENT_200_MA) &&
                                      DrvStepperAmis30521_SetMotorState(TRUE)));
    }
    else
    {
        CoreTerm_PrintFeedback(DrvStepperAmis30521_SetMotorState(FALSE));
    }
}
//------------------------------------------------------------------------------------------------//
static void TestLibMotion_GetErrorStatus(void)
{
    U16     i;
    U32     error_status;
    
    error_status = ((U32)DrvStepperAmis30521_ReadRegister(REGISTER_SR0) |
                    ((U32)DrvStepperAmis30521_ReadRegister(REGISTER_SR1) << 8) |
                    ((U32)DrvStepperAmis30521_ReadRegister(REGISTER_SR2) << 16) |
                    ((U32)DrvStepperAmis30521_ReadRegister(REGISTER_SR3) << 24));
    
    if(DrvGpio_GetPin(amis_error_pin) == FALSE)
    {
        DrvGpio_SetPin(amis_reset_pin, TRUE);
        for(i=0; i<500; i++)
        {
            asm("nop");
        }
        DrvGpio_SetPin(amis_reset_pin, FALSE);
    }
    
    LOG_TRM("ERROR: 0x%08h", PU32(error_status));
    CoreTerm_PrintAcknowledge();
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void TestLibMotionBipolarStepper_Init(SPI_DEVICE_ID amis_spi_device_id,
                                      DRVGPIO_PIN_HNDL amis_reset_pin_hndl,
                                      DRVGPIO_PIN_HNDL amis_dir_pin_hndl,
                                      DRVGPIO_PIN_HNDL amis_step_pin_hndl,
                                      DRVGPIO_PIN_HNDL amis_error_pin_hndl)
{
    MODULE_INIT_ONCE();
        
    // init AMIS direct lines
    amis_reset_pin = amis_reset_pin_hndl;
    amis_dir_pin = amis_dir_pin_hndl;
    amis_step_pin = amis_step_pin_hndl;
    amis_error_pin = amis_error_pin_hndl;
    
    // init AMIS chip
    DrvStepperAmis30521_Init(amis_spi_device_id);

    CoreTerm_RegisterCommand("MotorDoStep", "MOTOR do step", 0, TestLibMotion_DoStep, TRUE);
    CoreTerm_RegisterCommand("MotorSetDir", "MOTOR set direction", 1, TestLibMotion_SetDir, TRUE);
    CoreTerm_RegisterCommand("MotorSetState", "MOTOR enable/disable motor", 1, TestLibMotion_Enable, TRUE);
    CoreTerm_RegisterCommand("MotorErrorStatus", "MOTOR get error status", 0, TestLibMotion_GetErrorStatus, TRUE);
    
    MODULE_INIT_DONE();
}
//================================================================================================//