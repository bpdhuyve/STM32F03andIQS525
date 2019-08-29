//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// amis 30422 stepper driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef STEPPER__DRVSTEPPERAMIS30422_H
#define STEPPER__DRVSTEPPERAMIS30422_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "spi\DrvSpiMasterDevice.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   amis current enum
// Icoil = CUR[2:0]/Rsense (page 35)
typedef enum
{
	CURRENT_VREF_DIV_40_RSENSE          = 0,
	CURRENT_VREF_DIV_20_RSENSE          = 1,
	CURRENT_3_VREF_DIV_40_RSENSE        = 2,
	CURRENT_VREF_DIV_10_RSENSE          = 3,
	CURRENT_VREF_DIV_8_RSENSE           = 4,
	CURRENT_3_VREF_DIV_20_RSENSE        = 5,
	CURRENT_7_VREF_DIV_40_RSENSE        = 6,
	CURRENT_VREF_DIV_5_RSENSE           = 7,
}
AMIS30422_CURRENT;

// @brief   amis direction enum
typedef enum
{
	AMIS30422_DIRECTION_CLOCKWISE          = 0,
	AMIS30422_DIRECTION_COUNTER_CLOCKWISE  = 1
}
AMIS30422_DIRECTION;

// @brief   amis30422 stepsize enum
typedef enum
{
    AMIS30422_STEP_SIZE_1_128_STEP 				= 0,
    AMIS30422_STEP_SIZE_1_64_STEP 				= 1,
	AMIS30422_STEP_SIZE_1_32_STEP 				= 2,
   	AMIS30422_STEP_SIZE_1_16_STEP 				= 3,
	AMIS30422_STEP_SIZE_1_8_STEP 				= 4,
	AMIS30422_STEP_SIZE_1_4_STEP 				= 5,
	AMIS30422_STEP_SIZE_1_2_STEP 	            = 6,
	AMIS30422_STEP_SIZE_FULL_STEP 				= 7,
	AMIS30422_STEP_SIZE_FULL_STEP_1_2_ROTATION	= 15,
}
AMIS30422_STEPSIZE;

// @brief   amis30422 register enum
typedef enum
{
    AMIS30422_REGISTER_WR     = 0x00,
	AMIS30422_REGISTER_CR0 	= 0x01,
   	AMIS30422_REGISTER_CR1 	= 0x02,
	AMIS30422_REGISTER_CR2 	= 0x03,
	AMIS30422_REGISTER_CR3 	= 0x04,
	AMIS30422_REGISTER_SR0 	= 0x05,
	AMIS30422_REGISTER_SR1 	= 0x06,
	AMIS30422_REGISTER_SR2 	= 0x07,
	AMIS30422_REGISTER_SR3 	= 0x08,
    AMIS30422_REGISTER_SR4    = 0x09,
    AMIS30422_REGISTER_PDRV0  = 0x0A,
    AMIS30422_REGISTER_PDRV1  = 0x0B,
    AMIS30422_REGISTER_PDRV2  = 0x0C,
    AMIS30422_REGISTER_PDRV3  = 0x0D
}
AMIS30422_REGISTER;

#if(SAMPLE_ACCURACY_IN_BITS <= 16)
    typedef U16           AMIS30422_SAMPLE_DATATYPE;
#elif(SAMPLE_ACCURACY_IN_BITS <= 32)
    typedef U32           AMIS30422_SAMPLE_DATATYPE;
#endif
    
typedef struct
{
    U8                          filter_order;
    AMIS30422_SAMPLE_DATATYPE   delta_limit;
    AMIS30422_SAMPLE_DATATYPE   init_level;
    AMIS30422_SAMPLE_DATATYPE   trigger_level;
}
AMIS30422_SLA_CTRL_STRUCT;

typedef U8                          AMIS30422_ID;

typedef struct
{
    AMIS30422_ID                         amis_id;
}
AMIS30422_STRUCT;

typedef AMIS30422_STRUCT*                AMIS30422_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @brief	Function to init the amis stepper driver
// @param	channel : the spi channel the amis is connected on
// @param	speed : the speed of the spi connection with the amis
// @param	cs_port : the port of the cs pin connected to the amis
// @param	cs_pin_nr : the pin number of the cs pin connected to the amis
void DrvStepperAmis30422_Init(void);
AMIS30422_HNDL DrvStepperAmis30422_Register(SPI_DEVICE_ID spi_device_id);

// @brief	Function to set the motor's current
// @param	current : the max current of the motor
BOOL DrvStepperAmis30422_SetCurrent(AMIS30422_HNDL amis_hndl, AMIS30422_CURRENT current);

// @brief	Function to set the motor's stepsize
// @param	stepsize : the stepsize of the motor
BOOL DrvStepperAmis30422_SetStepsize(AMIS30422_HNDL amis_hndl, AMIS30422_STEPSIZE stepsize);

// @brief	Function to set the motor's direction (trough spi)
// @param	dir : the direction of the motor
BOOL DrvStepperAmis30422_SetDirection(AMIS30422_HNDL amis_hndl, AMIS30422_DIRECTION dir);

// @brief	Function to set the motor's current
// @param	state : the max current of the motor
BOOL DrvStepperAmis30422_SetMotorState(AMIS30422_HNDL amis_hndl, BOOL state);

// @brief	Function to set the motor's sleepstate
// @param	state : the max current of the motor
BOOL DrvStepperAmis30422_SetSleepState(AMIS30422_HNDL amis_hndl, BOOL state);

// @brief	Function to set the SLA involved bits
// @param	slat_bit : TRUE will set the SLAT bit, FALSE will clear the SLAT bit
// @param	slag_bit : TRUE will set the SLAG bit, FALSE will clear the SLAG bit
BOOL DrvStepperAmis30422_SetSla(AMIS30422_HNDL amis_hndl, BOOL slat_bit, U8 slag_bits, BOOL sla_offset_bit);

// @brief	Function to check if the motor is connected to the amis
// @return 	TRUE if the motor is connected, FALSE if there is something wrong
BOOL DrvStepperAmis30422_MotorTest(AMIS30422_HNDL amis_hndl);

// @brief	Function to check if the amis is functioning properly 	
// @return 	TRUE if all is ok, FALSE if there is something wrong
BOOL DrvStepperAmis30422_SelfTest(AMIS30422_HNDL amis_hndl);

///@brief	Function to write raw data to amis register
///@param	registerNumber: the register from the registernumber enum
///@param 	registerData: 8 bit value of the registerng
BOOL DrvStepperAmis30422_WriteRegister(AMIS30422_HNDL amis_hndl, AMIS30422_REGISTER register_number, U8 register_data);

// @brief	Function to read raw data from amis register
// @param	registerNumber: the register from the registernumber enum
// @return: 8 bit value of the register
U8 DrvStepperAmis30422_ReadRegister(AMIS30422_HNDL amis_hndl, AMIS30422_REGISTER register_number);

// @brief	Function that returns an errormask : all relevant bits from the AMIS status registers are mapped
//          into this 16bit errormask, you can use the "AMIS_ERRORMASK_BIT_..." defines to access the correct bits.
// @return 	16bit errormask
U16 DrvStepperAmis30422_GetErrorMask(AMIS30422_HNDL amis_hndl);

//------------------------------------------------------------------------------------------------//
#if INCLUDE_AMIS30422_SLA
    // @param	slag_bit :      TRUE will set the SLAG bit (divide SLA signal by 4)
    //                          FALSE will clear the SLAG bit (divide SLA signal by 2)
    // @param	sla_ctrl_struct_ptr : pointer to struct that contains control data where:
    //  order :         the order of the filter used to filter the samples
    //  delta :         maximum jump in output value of the filter (0 = no limit)
    //  init_level :    initialization value before feeding the first sample to the filter
    //  trigger_level : if (init_level > trigger_level) : if (filtered value <= trigger_level) ==> collision is detected
    //                  if (init_level <= trigger_level) : if (filtered value >= trigger_level) ==> collision is detected
    void DrvStepperAmis30422_Sla_Init(AMIS30422_HNDL amis_hndl, U8 slag_bits, BOOL sla_offset_bit, AMIS30422_SLA_CTRL_STRUCT* sla_ctrl_struct_ptr);
    
    // @brief	Resets filters and other cached data of previous collision detection.
    //          Must be called at the start of every new stepper-move
    void DrvStepperAmis30422_Sla_ResetAlgorithm(AMIS30422_HNDL amis_hndl);
    
    // @brief	Feeds the algorithm with a new sample of the SLA-pin, returns TRUE if collision is detected
    BOOL DrvStepperAmis30422_Sla_NewSample(AMIS30422_HNDL amis_hndl, AMIS30422_SAMPLE_DATATYPE new_sample);
#else
    #define DrvStepperAmis30422_Sla_Init(x1,x2,x3,x4,x5)
    
    #define DrvStepperAmis30422_Sla_ResetAlgorithm
    
    #define DrvStepperAmis30422_Sla_NewSample(x)             FALSE
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//



#endif /* STEPPER__DRVSTEPPERAMIS30422_H */