//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// amis stepper driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STEPPER__DRVSTEPPERAMIS30521_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STEPPER__DRVSTEPPERAMIS30521_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               STEPPER__DRVSTEPPERAMIS30521_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRIVER lib include section
#include "stepper\DrvStepperAmis30521.h"

#if INCLUDE_SLA
#include "filter\StdIirFilter.h"
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#if(SAMPLE_ACCURACY_IN_BITS <= 16)
    #define StdIirFilter_Function    StdIirFilter_U16
#elif(SAMPLE_ACCURACY_IN_BITS <= 32)
    #define StdIirFilter_Function    StdIirFilter_U32
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Amis30521info(void);
static void Amis30521ReadReg(void);
static void Amis30521WriteReg(void);
#endif
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static SPI_DEVICE_ID                amis_spi_device_id;
static U8                           control_reg[3];
#if INCLUDE_SLA
static SLA_CTRL_STRUCT*             amis_sla_ctrl_struct_ptr;
static SAMPLE_DATATYPE              amsi_sla_filtered_value;
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
static void Amis30521info(void)
{
    U8  i;
    for(i=1;i<=7;i++)
    {
        LOG_TRM("Reg %02h : %02h", PU8(i), PU8(DrvStepperAmis30521_ReadRegister((STEPPER_REGISTER)i)));
    }
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Amis30521ReadReg(void)
{
    U8  reg = CoreTerm_GetArgumentAsU32(0);
    
    LOG_TRM("Reg %02h : %02h", PU8(reg), PU8(DrvStepperAmis30521_ReadRegister((STEPPER_REGISTER)reg)));
    CoreTerm_PrintAcknowledge();
}
//------------------------------------------------------------------------------------------------//
static void Amis30521WriteReg(void)
{
    CoreTerm_PrintFeedback(DrvStepperAmis30521_WriteRegister((STEPPER_REGISTER)CoreTerm_GetArgumentAsU32(0), CoreTerm_GetArgumentAsU32(1)));
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvStepperAmis30521_Init(SPI_DEVICE_ID spi_device_id)
{
    SPI_CONFIG_STRUCT   amis_spi_cfg = {500000, MODE_0, 8, FALSE};
    
    MODULE_INIT_ONCE();
    
    amis_spi_device_id = spi_device_id;
    DrvSpiMasterDevice_Config(spi_device_id, &amis_spi_cfg);
    
    MEMSET((VPTR)control_reg, 0, SIZEOF(control_reg));
    
    CoreTerm_RegisterCommand("AmisInfo", "AMIS info", 0, Amis30521info, FALSE);
    CoreTerm_RegisterCommand("AmisReadReg", "AMIS read register", 1, Amis30521ReadReg, FALSE);
    CoreTerm_RegisterCommand("AmisWriteReg", "AMIS write register", 2, Amis30521WriteReg, FALSE);

    MODULE_INIT_DONE();
    
	//reset register data
    control_reg[0] = DrvStepperAmis30521_ReadRegister(REGISTER_CR0);
    control_reg[1] = DrvStepperAmis30521_ReadRegister(REGISTER_CR1);
    control_reg[2] = DrvStepperAmis30521_ReadRegister(REGISTER_CR2);
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_SetCurrent(STEPPER_CURRENT current)
{
	control_reg[0] &= ~0x1F; //clear current bits
	control_reg[0] |= current;

	return DrvStepperAmis30521_WriteRegister(REGISTER_CR0, control_reg[0]);
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_SetStepsize(STEPPER_STEPSIZE stepsize)
{
	control_reg[0] &= 0x1F; //clear stepsize bits : notice that the mask is not inverted !!
	control_reg[0] |= (stepsize << 5);

	return DrvStepperAmis30521_WriteRegister(REGISTER_CR0, control_reg[0]);
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_SetDirection(STEPPER_DIRECTION dir)
{
	if(dir == DIRECTION_CLOCKWISE)
	{
		control_reg[1] &= ~0x80;
	}
	else
	{
	  	control_reg[1] |= 0x80;
	}

	return DrvStepperAmis30521_WriteRegister(REGISTER_CR1, control_reg[1]);
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_SetMotorState(BOOL state)
{
	if(state)
	{
		control_reg[2] |= 0x80;
	}
	else
	{
		control_reg[2] &= ~0x80;
	}

	return DrvStepperAmis30521_WriteRegister(REGISTER_CR2, control_reg[2]);
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_SetSleepState(BOOL state)
{
	if(state)
	{
		control_reg[2] |= 0x40;
	}
	else
	{
		control_reg[2] &= ~0x40;
	}

	return DrvStepperAmis30521_WriteRegister(REGISTER_CR2, control_reg[2]);
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_SetSla(BOOL slat_bit, BOOL slag_bit)
{
	if(slat_bit)
	{
		control_reg[2] |= 0x10;
	}
	else
	{
		control_reg[2] &= ~0x10;
	}
	if(slag_bit)
	{
		control_reg[2] |= 0x20;
	}
	else
	{
		control_reg[2] &= ~0x20;
	}

	return DrvStepperAmis30521_WriteRegister(REGISTER_CR2, control_reg[2]);
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_MotorTest(void)
{
	U8	regdata;

	//first check if motor is enabled and not in sleep (motor must be enabled and not in sleep for over 200mz!!!)
	regdata = DrvStepperAmis30521_ReadRegister(REGISTER_CR2);
	if ((~regdata & 0x80) || (regdata & 0x40))
	{
		return FALSE;  //return false if motor is not enabled or in sleep (to test motor must be enabled and not in sleep for over 200mz!!!)
	}

	//check open coil
	regdata = DrvStepperAmis30521_ReadRegister(REGISTER_SR0);
	if (regdata & 0x0C)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_SelfTest(void)
{
	U8	regdata;

	regdata = DrvStepperAmis30521_ReadRegister(REGISTER_SR0);
	if (regdata & 0x60)//check thermal warning and charge pump failure
	{
		return FALSE;
	}
	regdata = DrvStepperAmis30521_ReadRegister(REGISTER_SR1);
	if (regdata & 0x78)//check over current detection
	{
		return FALSE;
	}
	regdata = DrvStepperAmis30521_ReadRegister(REGISTER_SR2);
	if (regdata & 0x7C)//check over current detection & thermal shutdown
	{
		return FALSE;
	}

	return TRUE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_WriteRegister(STEPPER_REGISTER registerNumber, U8 registerData)
{
    U8    	data[2];
    
    MODULE_CHECK();

    if(registerNumber <= REGISTER_CR2)
    {
        data[0] = (registerNumber & 0x1F) | 0x80;
        data[1] = registerData;
    
        DrvSpiMasterDevice_SelectWriteData(amis_spi_device_id, data, 2);
        
        control_reg[registerNumber-1] = DrvStepperAmis30521_ReadRegister(registerNumber);
        
        if((BOOL)(control_reg[registerNumber-1] == registerData))
        {
            return TRUE;
        }
        LOG_DEV("Writing CR %02h to %02h failed [%02h]", PU8(registerNumber), PU8(registerData), PU8(control_reg[registerNumber-1]));
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
U8 DrvStepperAmis30521_ReadRegister(STEPPER_REGISTER registerNumber)
{
	U8	data;
    
    MODULE_CHECK();

	data = registerNumber & 0x1F; //write byte must be = 0 => read
	DrvSpiMasterDevice_Select(amis_spi_device_id);
   	DrvSpiMasterDevice_WriteData(amis_spi_device_id, &data, 1);
	DrvSpiMasterDevice_ReadData(amis_spi_device_id, &data, 1);
	DrvSpiMasterDevice_Deselect(amis_spi_device_id);

    if(registerNumber >= REGISTER_SR0)
    {
	    return (data & 0x7F);
    }
    return data;
}
//------------------------------------------------------------------------------------------------//
U16 DrvStepperAmis30521_GetErrorMask(void)
{
	U16	error_mask;

	error_mask = ((U16)(DrvStepperAmis30521_ReadRegister(REGISTER_SR0) & 0x7C)) >> 2;
	error_mask |= ((U16)(DrvStepperAmis30521_ReadRegister(REGISTER_SR2) & 0x7C)) << 3;
	error_mask |= ((U16)(DrvStepperAmis30521_ReadRegister(REGISTER_SR1) & 0x7C)) << 7;

	return error_mask;
}
//------------------------------------------------------------------------------------------------//
#if INCLUDE_SLA
void DrvStepperAmis30521_Sla_Init(BOOL slag_bit, SLA_CTRL_STRUCT* sla_ctrl_struct_ptr)
{
    DrvStepperAmis30521_SetSla(FALSE, slag_bit);
    amis_sla_ctrl_struct_ptr = sla_ctrl_struct_ptr;
    amsi_sla_filtered_value = sla_ctrl_struct_ptr->init_level;
}
//------------------------------------------------------------------------------------------------//
void DrvStepperAmis30521_Sla_ResetAlgorithm(void)
{
    amsi_sla_filtered_value = amis_sla_ctrl_struct_ptr->init_level;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30521_Sla_NewSample(SAMPLE_DATATYPE new_sample)
{
    StdIirFilter_Function(&amsi_sla_filtered_value,
                          new_sample,
                          amis_sla_ctrl_struct_ptr->filter_order,
                          amis_sla_ctrl_struct_ptr->delta_limit);
    
    LOG_DBG("%d - %d", PU32(new_sample), PU32(amsi_sla_filtered_value));
    
    if(amis_sla_ctrl_struct_ptr->init_level > amis_sla_ctrl_struct_ptr->trigger_level)
    {
        return (BOOL)(amsi_sla_filtered_value <= amis_sla_ctrl_struct_ptr->trigger_level);
    }
    else
    {
        return (BOOL)(amsi_sla_filtered_value >= amis_sla_ctrl_struct_ptr->trigger_level);
    }
}
#endif
//================================================================================================//
