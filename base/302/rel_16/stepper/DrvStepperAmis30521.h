//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// amis stepper driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef STEPPER__DRVSTEPPERAMIS30521_H
#define STEPPER__DRVSTEPPERAMIS30521_H
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines if SLA functionality has to be included
#ifndef INCLUDE_AMIS30521_SLA
    #define INCLUDE_AMIS30521_SLA               0
#else
    // @brief   Defines the accuracy of the ADC samples
    #ifndef SAMPLE_ACCURACY_IN_BITS
        #error "SAMPLE_ACCURACY_IN_BITS not defined in AppConfig.h"
    #else
        #if(SAMPLE_ACCURACY_IN_BITS > 32)
            #error "SAMPLE_ACCURACY_IN_BITS cannot be more than 32"
        #endif
    #endif
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "spi\DrvSpiMasterDevice.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//#define AMIS_ERRORMASK_BIT_OPEN_CIRCUIT_PHASE_Y            0x0001
//#define AMIS_ERRORMASK_BIT_OPEN_CIRCUIT_PHASE_X            0x0002
//#define AMIS_ERRORMASK_BIT_CHARGE_PUMP_FAIL                0x0008
//#define AMIS_ERRORMASK_BIT_THERMAL_WARNING                 0x0010
//#define AMIS_ERRORMASK_BIT_THERMAL_SHUTDOWN                0x0020
//#define AMIS_ERRORMASK_BIT_OVERCURRENT_PHASE_Y_NB          0x0040
//#define AMIS_ERRORMASK_BIT_OVERCURRENT_PHASE_Y_NT          0x0080
//#define AMIS_ERRORMASK_BIT_OVERCURRENT_PHASE_Y_PB          0x0100
//#define AMIS_ERRORMASK_BIT_OVERCURRENT_PHASE_Y_PT          0x0200
//#define AMIS_ERRORMASK_BIT_OVERCURRENT_PHASE_X_NB          0x0400
//#define AMIS_ERRORMASK_BIT_OVERCURRENT_PHASE_X_NT          0x0800
//#define AMIS_ERRORMASK_BIT_OVERCURRENT_PHASE_X_PB          0x1000
//#define AMIS_ERRORMASK_BIT_OVERCURRENT_PHASE_X_PT          0x2000
//
//#define AMIS_ERRORMASK_OPEN_CIRCUIT                        0x0003
//#define AMIS_ERRORMASK_SHORT_CIRCUIT                       0x3FC0
//#define AMIS_ERRORMASK_ONLY_ERRORS                         0xFFEF
//#define AMIS_ERRORMASK_WARNING_AND_ERRORS                  0xFFFF
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @brief   amis current enum
typedef enum
{
	CURRENT_30_MA = 0       ,
	CURRENT_60_MA = 1       ,
	CURRENT_90_MA = 2       ,
	CURRENT_100_MA = 3      ,
	CURRENT_110_MA = 4      ,
	CURRENT_120_MA = 5      ,
	CURRENT_135_MA = 6      ,
	CURRENT_150_MA = 7      ,
	CURRENT_160_MA = 8      ,
	CURRENT_180_MA = 9      ,
	CURRENT_200_MA = 10     ,
	CURRENT_220_MA = 11     ,
	CURRENT_240_MA = 12     ,
	CURRENT_270_MA = 13     ,
	CURRENT_300_MA = 14     ,
	CURRENT_325_MA = 15     ,
	CURRENT_365_MA = 16     ,
	CURRENT_400_MA = 17     ,
	CURRENT_440_MA = 18     ,
	CURRENT_485_MA = 19     ,
	CURRENT_535_MA = 20     ,
	CURRENT_595_MA = 21     ,
	CURRENT_650_MA = 22     ,
	CURRENT_725_MA = 23     ,
	CURRENT_800_MA = 24     ,
	CURRENT_885_MA = 25     ,
	CURRENT_970_MA = 26     ,
	CURRENT_1070_MA = 27    ,
	CURRENT_1190_MA = 28    ,
	CURRENT_1300_MA = 29    ,
	CURRENT_1450_MA = 30    ,
	CURRENT_1600_MA = 31
}
AMIS30521_CURRENT;

// @brief   amis direction enum
typedef enum
{
	AMIS30521_DIRECTION_CLOCKWISE          = 0,
	AMIS30521_DIRECTION_COUNTER_CLOCKWISE  = 1
}
AMIS30521_DIRECTION;

// @brief   amis stepsize enum
typedef enum
{
	AMIS30521_STEP_SIZE_1_32_STEP 				= 0,
   	AMIS30521_STEP_SIZE_1_16_STEP 				= 1,
	AMIS30521_STEP_SIZE_1_8_STEP 					= 2,
	AMIS30521_STEP_SIZE_1_4_STEP 					= 3,
	AMIS30521_STEP_SIZE_1_2_STEP_UNCOMPENSATED 	= 4,
	AMIS30521_STEP_SIZE_1_2_STEP_COMPESATED 		= 5,
	AMIS30521_STEP_SIZE_FULL_STEP 				= 6
}
AMIS30521_STEPSIZE;

typedef enum
{
	AMIS30521_REGISTER_CR0 	= 0x01,
   	AMIS30521_REGISTER_CR1 	= 0x02,
	AMIS30521_REGISTER_CR2 	= 0x03,
	AMIS30521_REGISTER_SR0 	= 0x04,
	AMIS30521_REGISTER_SR1 	= 0x05,
	AMIS30521_REGISTER_SR2 	= 0x06,
	AMIS30521_REGISTER_SR3 	= 0x07
}
AMIS30521_REGISTER;

#if(SAMPLE_ACCURACY_IN_BITS <= 16)
    typedef U16           AMIS30521_SAMPLE_DATATYPE;
#elif(SAMPLE_ACCURACY_IN_BITS <= 32)
    typedef U32           AMIS30521_SAMPLE_DATATYPE;
#endif
    
typedef struct
{
    U8                          filter_order;
    AMIS30521_SAMPLE_DATATYPE   delta_limit;
    AMIS30521_SAMPLE_DATATYPE   init_level;
    AMIS30521_SAMPLE_DATATYPE   trigger_level;
}
AMIS30521_SLA_CTRL_STRUCT;

typedef U8                          AMIS30521_ID;

typedef struct
{
    AMIS30521_ID                         amis_id;
}
AMIS30521_STRUCT;

typedef AMIS30521_STRUCT*                AMIS30521_HNDL;
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
void DrvStepperAmis30521_Init(void);
AMIS30521_HNDL DrvStepperAmis30521_Register(SPI_DEVICE_ID spi_device_id);

// @brief	Function to set the motor's current
// @param	current : the max current of the motor
BOOL DrvStepperAmis30521_SetCurrent(AMIS30521_HNDL amis_hndl, AMIS30521_CURRENT current);

// @brief	Function to set the motor's stepsize
// @param	stepsize : the stepsize of the motor
BOOL DrvStepperAmis30521_SetStepsize(AMIS30521_HNDL amis_hndl, AMIS30521_STEPSIZE stepsize);

// @brief	Function to set the motor's direction (trough spi)
// @param	dir : the direction of the motor
BOOL DrvStepperAmis30521_SetDirection(AMIS30521_HNDL amis_hndl, AMIS30521_DIRECTION dir);

// @brief	Function to set the motor's current
// @param	state : the max current of the motor
BOOL DrvStepperAmis30521_SetMotorState(AMIS30521_HNDL amis_hndl, BOOL state);

// @brief	Function to set the motor's sleepstate
// @param	state : the max current of the motor
BOOL DrvStepperAmis30521_SetSleepState(AMIS30521_HNDL amis_hndl, BOOL state);

// @brief	Function to set the SLA involved bits
// @param	slat_bit : TRUE will set the SLAT bit, FALSE will clear the SLAT bit
// @param	slag_bit : TRUE will set the SLAG bit, FALSE will clear the SLAG bit
BOOL DrvStepperAmis30521_SetSla(AMIS30521_HNDL amis_hndl, BOOL slat_bit, BOOL slag_bit);

// @brief	Function to check if the motor is connected to the amis
// @return 	TRUE if the motor is connected, FALSE if there is something wrong
BOOL DrvStepperAmis30521_MotorTest(AMIS30521_HNDL amis_hndl);

// @brief	Function to check if the amis is functioning properly 	
// @return 	TRUE if all is ok, FALSE if there is something wrong
BOOL DrvStepperAmis30521_SelfTest(AMIS30521_HNDL amis_hndl);

///@brief	Function to write raw data to amis register
///@param	registerNumber: the register from the registernumber enum
///@param 	registerData: 8 bit value of the registerng
BOOL DrvStepperAmis30521_WriteRegister(AMIS30521_HNDL amis_hndl, AMIS30521_REGISTER register_number, U8 register_data);

// @brief	Function to read raw data from amis register
// @param	registerNumber: the register from the registernumber enum
// @return: 8 bit value of the register
U8 DrvStepperAmis30521_ReadRegister(AMIS30521_HNDL amis_hndl, AMIS30521_REGISTER register_number);

// @brief	Function that returns an errormask : all relevant bits from the AMIS status registers are mapped
//          into this 16bit errormask, you can use the "AMIS_ERRORMASK_BIT_..." defines to access the correct bits.
// @return 	16bit errormask
U16 DrvStepperAmis30521_GetErrorMask(AMIS30521_HNDL amis_hndl);

//------------------------------------------------------------------------------------------------//
#if INCLUDE_AMIS30521_SLA
    // @param	slag_bit :      TRUE will set the SLAG bit (divide SLA signal by 4)
    //                          FALSE will clear the SLAG bit (divide SLA signal by 2)
    // @param	sla_ctrl_struct_ptr : pointer to struct that contains control data where:
    //  order :         the order of the filter used to filter the samples
    //  delta :         maximum jump in output value of the filter (0 = no limit)
    //  init_level :    initialization value before feeding the first sample to the filter
    //  trigger_level : if (init_level > trigger_level) : if (filtered value <= trigger_level) ==> collision is detected
    //                  if (init_level <= trigger_level) : if (filtered value >= trigger_level) ==> collision is detected
    void DrvStepperAmis30521_Sla_Init(AMIS30521_HNDL amis_hndl, BOOL slag_bit, AMIS30521_SLA_CTRL_STRUCT* sla_ctrl_struct_ptr);
    
    // @brief	Resets filters and other cached data of previous collision detection.
    //          Must be called at the start of every new stepper-move
    void DrvStepperAmis30521_Sla_ResetAlgorithm(AMIS30521_HNDL amis_hndl);
    
    // @brief	Feeds the algorithm with a new sample of the SLA-pin, returns TRUE if collision is detected
    BOOL DrvStepperAmis30521_Sla_NewSample(AMIS30521_HNDL amis_hndl, AMIS30521_SAMPLE_DATATYPE new_sample);
#else
    #define DrvStepperAmis30521_Sla_Init(x1,x2,x3,x4,x5)
    
    #define DrvStepperAmis30521_Sla_ResetAlgorithm
    
    #define DrvStepperAmis30521_Sla_NewSample(x)             FALSE
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* STEPPER__DRVSTEPPERAMIS30521_H */
