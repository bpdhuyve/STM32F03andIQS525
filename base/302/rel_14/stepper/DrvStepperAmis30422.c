//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// amis 30422 stepper driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define STEPPER__DRVSTEPPERAMIS30422_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef STEPPER__DRVSTEPPERAMIS30422_LOG_LEVEL
    #define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
    #define CORELOG_LEVEL               STEPPER__DRVSTEPPERAMIS30422_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DRVSTEPPERAMIS30422_COUNT
    #define DRVSTEPPERAMIS30422_COUNT        SPI_CHANNEL_COUNT
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV lib include section

//STD lib include section
#if INCLUDE_AMIS30422_SLA
    #include "filter\StdIirFilter.h"
#endif

//COM lib include section

//APP include section
#include "DrvStepperAmis30422.h"
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
typedef struct
{
    SPI_DEVICE_ID               spi_device_id;
    U8                          control_reg[4];
    #if INCLUDE_AMIS30422_SLA
    AMIS30422_SLA_CTRL_STRUCT   sla_ctrl_struct;
    AMIS30422_SAMPLE_DATATYPE   sla_filtered_value;
    #endif
}
AMIS30422_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static AMIS30422_CTRL_STRUCT            amis_ctrl_struct[DRVSTEPPERAMIS30422_COUNT];
static AMIS30422_STRUCT                 amis_struct[DRVSTEPPERAMIS30422_COUNT];
static U8                               amis_count;

#if (CORELOG_LEVEL & LOG_LEVEL_ERROR) != 0
static const STRING                     drvstepper_illegal_device_string = "AMIS illegal device - ";
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
static void Amis30422info(void)
{
    AMIS30422_HNDL  amis_hndl;
    U8              i;
    
    for(amis_hndl = amis_struct; amis_hndl < &amis_struct[amis_count]; amis_hndl++)
    {
        if(amis_hndl->amis_id == CoreTerm_GetArgumentAsU32(0))
        {
            for(i = 0; i <= (U8)AMIS30422_REGISTER_PDRV3; i++)
            {
                LOG_TRM("Reg %02h : %02h", PU8(i), PU8(DrvStepperAmis30422_ReadRegister(amis_hndl, (AMIS30422_REGISTER)i)));
            }
            CoreTerm_PrintAcknowledge();
            return;
        }
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
}
//------------------------------------------------------------------------------------------------//
static void Amis30422ReadReg(void)
{
    AMIS30422_HNDL      amis_hndl;
    U8                  reg = CoreTerm_GetArgumentAsU32(1);
    
    for(amis_hndl = amis_struct; amis_hndl < &amis_struct[amis_count]; amis_hndl++)
    {
        if(amis_hndl->amis_id == CoreTerm_GetArgumentAsU32(0))
        {
            LOG_TRM("Reg %02h : %02h", PU8(reg), PU8(DrvStepperAmis30422_ReadRegister(amis_hndl, (AMIS30422_REGISTER)reg)));
            CoreTerm_PrintAcknowledge();
            return;
        }
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
}
//------------------------------------------------------------------------------------------------//
static void Amis30422WriteReg(void)
{
    AMIS30422_HNDL      amis_hndl;
    
    for(amis_hndl = amis_struct; amis_hndl < &amis_struct[amis_count]; amis_hndl++)
    {
        if(amis_hndl->amis_id == CoreTerm_GetArgumentAsU32(0))
        {
            CoreTerm_PrintFeedback(DrvStepperAmis30422_WriteRegister(amis_hndl, (AMIS30422_REGISTER)CoreTerm_GetArgumentAsU32(1), CoreTerm_GetArgumentAsU32(2)));
            return;
        }
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
}
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvStepperAmis30422_Init(void)
{
    CoreTerm_RegisterCommand("Amis30422Info", "AMIS info", 1, Amis30422info, FALSE);
    CoreTerm_RegisterCommand("Amis30422ReadReg", "AMIS read register", 2, Amis30422ReadReg, FALSE);
    CoreTerm_RegisterCommand("Amis30422WriteReg", "AMIS write register", 3, Amis30422WriteReg, FALSE);
    
    MEMSET((VPTR)amis_struct, 0, SIZEOF(amis_struct));
    MEMSET((VPTR)amis_ctrl_struct, 0, SIZEOF(amis_ctrl_struct));
    amis_count = 0;
}
//------------------------------------------------------------------------------------------------//
AMIS30422_HNDL DrvStepperAmis30422_Register(SPI_DEVICE_ID spi_device_id)
{
    AMIS30422_HNDL              amis_hndl;
    AMIS30422_CTRL_STRUCT*      amis_ctrl_struct_ptr;
    SPI_CONFIG_STRUCT           spi_config_struct = {500000, MODE_0, 8, FALSE};
    
    for(amis_hndl = amis_struct, amis_ctrl_struct_ptr = amis_ctrl_struct; amis_hndl < &amis_struct[amis_count]; amis_hndl++, amis_ctrl_struct_ptr++)
    {
        if(amis_ctrl_struct_ptr->spi_device_id == spi_device_id)
        {
            return amis_hndl;
        }
    }
    
    if(amis_count < DRVSTEPPERAMIS30422_COUNT)
    {
        DrvSpiMasterDevice_Config(spi_device_id, &spi_config_struct);
        amis_ctrl_struct_ptr->spi_device_id = spi_device_id;
        amis_hndl->amis_id = amis_count;
        amis_count++;
        amis_ctrl_struct_ptr->control_reg[0] = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_CR0);
        amis_ctrl_struct_ptr->control_reg[1] = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_CR1);
        amis_ctrl_struct_ptr->control_reg[2] = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_CR2);
        amis_ctrl_struct_ptr->control_reg[3] = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_CR3);
        DrvStepperAmis30422_WriteRegister(amis_hndl, AMIS30422_REGISTER_CR3, 0);
        return amis_hndl;
    }
    return NULL;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_SetCurrent(AMIS30422_HNDL amis_hndl, AMIS30422_CURRENT current)
{
    AMIS30422_CTRL_STRUCT*          amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        amis_ctrl_struct_ptr->control_reg[0] &= ~0x07;      //clear current bits
        amis_ctrl_struct_ptr->control_reg[0] |= current;
        return DrvStepperAmis30422_WriteRegister(amis_hndl, AMIS30422_REGISTER_CR0, amis_ctrl_struct_ptr->control_reg[0]);
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_SetStepsize(AMIS30422_HNDL amis_hndl, AMIS30422_STEPSIZE stepsize)
{
    AMIS30422_CTRL_STRUCT*          amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        amis_ctrl_struct_ptr->control_reg[0] &= 0x0F;       //clear stepsize bits : notice that the mask is not inverted !!
        amis_ctrl_struct_ptr->control_reg[0] |= (stepsize << 4);
        return DrvStepperAmis30422_WriteRegister(amis_hndl, AMIS30422_REGISTER_CR0, amis_ctrl_struct_ptr->control_reg[0]);
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_SetDirection(AMIS30422_HNDL amis_hndl, AMIS30422_DIRECTION dir)
{
    AMIS30422_CTRL_STRUCT*          amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        if(dir == AMIS30422_DIRECTION_CLOCKWISE)
        {
            amis_ctrl_struct_ptr->control_reg[1] &= ~0x80;
        }
        else
        {
            amis_ctrl_struct_ptr->control_reg[1] |= 0x80;
        }
        return DrvStepperAmis30422_WriteRegister(amis_hndl, AMIS30422_REGISTER_CR1, amis_ctrl_struct_ptr->control_reg[1]);
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_SetMotorState(AMIS30422_HNDL amis_hndl, BOOL state)
{
	AMIS30422_CTRL_STRUCT*          amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        if(state)
        {
            amis_ctrl_struct_ptr->control_reg[2] |= 0x80;
        }
        else
        {
            amis_ctrl_struct_ptr->control_reg[2] &= ~0x80;
        }
        return DrvStepperAmis30422_WriteRegister(amis_hndl, AMIS30422_REGISTER_CR2, amis_ctrl_struct_ptr->control_reg[2]);
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_SetSleepState(AMIS30422_HNDL amis_hndl, BOOL state)
{
    AMIS30422_CTRL_STRUCT*          amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        if(state)
        {
            amis_ctrl_struct_ptr->control_reg[2] |= 0x40;
        }
        else
        {
            amis_ctrl_struct_ptr->control_reg[2] &= ~0x40;
        }
        return DrvStepperAmis30422_WriteRegister(amis_hndl, AMIS30422_REGISTER_CR2, amis_ctrl_struct_ptr->control_reg[2]);
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_SetSla(AMIS30422_HNDL amis_hndl, BOOL slat_bit, U8 slag_bits, BOOL sla_offset_bit)
{
	AMIS30422_CTRL_STRUCT*       amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        if(slag_bits > 7)
        {
            return FALSE;
        }
        else
        {
            amis_ctrl_struct_ptr->control_reg[2] &= ~0x0E;
            amis_ctrl_struct_ptr->control_reg[2] |= (slag_bits << 1);
        }
        if(slat_bit)
        {
            amis_ctrl_struct_ptr->control_reg[2] |= 0x10;
        }
        else
        {
            amis_ctrl_struct_ptr->control_reg[2] &= ~0x10;
        }
        if(sla_offset_bit)
        {
            amis_ctrl_struct_ptr->control_reg[2] |= 0x01;
        }
        else
        {
            amis_ctrl_struct_ptr->control_reg[2] &= ~0x01;
        }        
        return DrvStepperAmis30422_WriteRegister(amis_hndl, AMIS30422_REGISTER_CR2, amis_ctrl_struct_ptr->control_reg[2]);
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_MotorTest(AMIS30422_HNDL amis_hndl)
{
    U8  regdata;
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        //first check if motor is enabled and not in sleep (motor must be enabled and not in sleep for over 200mz!!!)
        regdata = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_CR2);
        if ((~regdata & 0x80) || (regdata & 0x40))
        {
            return FALSE;  //return false if motor is not enabled or in sleep (to test motor must be enabled and not in sleep for over 200mz!!!)
        }

        //check open coil
        regdata = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_SR0);
        if (regdata & 0x02)
        {
            return FALSE;
        }
        else
        {
            regdata = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_SR1);
            if(regdata & 0x02)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_SelfTest(AMIS30422_HNDL amis_hndl)
{
	U8  regdata;
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        regdata = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_SR0);
        // Check thermal warning
        if(regdata & 0x60)
        {
            return FALSE;
        }
        regdata = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_SR1);
        // X-coil: check over current detection, charge pump failure
        if (regdata & 0x7C)
        {
            return FALSE;
        }
        regdata = DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_SR2);
        // Y-coil: check over current detection & thermal shutdown
        if (regdata & 0x7C)
        {
            return FALSE;
        }
        return TRUE;
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_WriteRegister(AMIS30422_HNDL amis_hndl, AMIS30422_REGISTER register_number, U8 register_data)
{
	AMIS30422_CTRL_STRUCT*          amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
    U8    	                        data[2];
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        if((register_number <= AMIS30422_REGISTER_CR3) || ((register_number >= AMIS30422_REGISTER_PDRV0) && (register_number <= AMIS30422_REGISTER_PDRV3)))
        {
            data[0] = (register_number & 0x1F) | 0x80;
            data[1] = register_data;
            
            DrvSpiMasterDevice_SelectWriteData(amis_ctrl_struct_ptr->spi_device_id, data, 2);
            
            amis_ctrl_struct_ptr->control_reg[register_number - 1] = DrvStepperAmis30422_ReadRegister(amis_hndl, register_number);
            
            if((BOOL)(amis_ctrl_struct_ptr->control_reg[register_number - 1] == register_data))
            {
                return TRUE;
            }
            LOG_DEV("Writing CR %02h to %02h failed [%02h]", PU8(register_number), PU8(register_data), PU8(amis_ctrl_struct_ptr->control_reg[register_number-1]));
        }
        return FALSE;
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
U8 DrvStepperAmis30422_ReadRegister(AMIS30422_HNDL amis_hndl, AMIS30422_REGISTER register_number)
{
	AMIS30422_CTRL_STRUCT*      amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
	U8	                        data;
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        data = register_number & 0x1F; //write byte must be = 0 => read
        DrvSpiMasterDevice_Select(amis_ctrl_struct_ptr->spi_device_id);
        DrvSpiMasterDevice_WriteData(amis_ctrl_struct_ptr->spi_device_id, &data, 1);
        DrvSpiMasterDevice_ReadData(amis_ctrl_struct_ptr->spi_device_id, &data, 1);
        DrvSpiMasterDevice_Deselect(amis_ctrl_struct_ptr->spi_device_id);

        if((register_number >= AMIS30422_REGISTER_SR0) && (register_number <= AMIS30422_REGISTER_SR2))
        {
            return (data & 0x7F);
        }
        return data;
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return 0;
}
//------------------------------------------------------------------------------------------------//
U16 DrvStepperAmis30422_GetErrorMask(AMIS30422_HNDL amis_hndl)
{
    U16     error_mask;
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        error_mask = ((U16)(DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_SR0) & 0x7E)) >> 4;
        error_mask |= ((U16)(DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_SR1) & 0x7E)) << 2;
        error_mask |= ((U16)(DrvStepperAmis30422_ReadRegister(amis_hndl, AMIS30422_REGISTER_SR2) & 0x7E)) << 8;
        return error_mask;
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return 0;
}
//------------------------------------------------------------------------------------------------//
#if INCLUDE_AMIS30422_SLA
void DrvStepperAmis30422_Sla_Init(AMIS30422_HNDL amis_hndl, U8 slag_bits, BOOL sla_offset_bit, AMIS30422_SLA_CTRL_STRUCT* sla_ctrl_struct_ptr)
{
    AMIS30422_CTRL_STRUCT*      amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        DrvStepperAmis30422_SetSla(amis_hndl, FALSE, slag_bits, sla_offset_bit);
        amis_ctrl_struct_ptr->sla_ctrl_struct = *sla_ctrl_struct_ptr;
        amis_ctrl_struct_ptr->sla_filtered_value = amis_ctrl_struct_ptr->sla_ctrl_struct.init_level;
        return;
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
}
//------------------------------------------------------------------------------------------------//
void DrvStepperAmis30422_Sla_ResetAlgorithm(AMIS30422_HNDL amis_hndl)
{
    AMIS30422_CTRL_STRUCT*       amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
     
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        amis_ctrl_struct_ptr->sla_filtered_value = amis_ctrl_struct_ptr->sla_ctrl_struct.init_level;
        return;
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
}
//------------------------------------------------------------------------------------------------//
BOOL DrvStepperAmis30422_Sla_NewSample(AMIS30422_HNDL amis_hndl, AMIS30422_SAMPLE_DATATYPE new_sample)
{
    AMIS30422_CTRL_STRUCT*       amis_ctrl_struct_ptr = &amis_ctrl_struct[amis_hndl->amis_id];
    
    if((amis_hndl != NULL) && (amis_hndl->amis_id < amis_count))
    {
        StdIirFilter_Function(&amis_ctrl_struct_ptr->sla_filtered_value,
                              new_sample,
                              amis_ctrl_struct_ptr->sla_ctrl_struct.filter_order,
                              amis_ctrl_struct_ptr->sla_ctrl_struct.delta_limit);
        
        LOG_DBG("%d - %d", PU32(new_sample), PU32(amis_ctrl_struct_ptr->sla_filtered_value));
        
        if(amis_ctrl_struct_ptr->sla_ctrl_struct.init_level > amis_ctrl_struct_ptr->sla_ctrl_struct.trigger_level)
        {
            return (BOOL)(amis_ctrl_struct_ptr->sla_filtered_value <= amis_ctrl_struct_ptr->sla_ctrl_struct.trigger_level);
        }
        else
        {
            return (BOOL)(amis_ctrl_struct_ptr->sla_filtered_value >= amis_ctrl_struct_ptr->sla_ctrl_struct.trigger_level);
        }
    }
    LOG_ERR("%s%d", PCSTR(drvstepper_illegal_device_string), PU8(amis_hndl->amis_id));
    return FALSE;
}
#endif
//================================================================================================//
