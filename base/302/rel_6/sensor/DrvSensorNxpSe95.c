//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the NXP SE95 sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORNXPSE95_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORNXPSE95_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORNXPSE95_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of SE95 sensors
#ifndef SE95_COUNT
	#define SE95_COUNT                  1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sensor\DrvSensorNxpSe95.h"
#include "i2c\DrvI2cMasterDevice.h"
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
	I2C_DEVICE_ID	i2c_device_id;
	BOOL			i2c_is_present;
	S16				temperature;
    BOOL            successfull_reading;
}
SE95_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorNxpSe95_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorNxpSe95_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorNxpSe95_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
static void DrvSensorNxpSe95_I2cMessageComplete(BOOL success);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const SENSOR_HOOK_LIST	    sensor_se95_hook_list = {DrvSensorNxpSe95_IsPresent, DrvSensorNxpSe95_ReadSensor, DrvSensorNxpSe95_GetValue};
static U8							sensor_se95_count;
static SE95_CTRL					sensor_se95_ctrl[SE95_COUNT];
static SENSOR_STRUCT				sensor_se95_struct[SE95_COUNT];
static BOOL							sensor_se95_success;
static SENSOR_ID					sensor_se95_active;
static U8							sensor_se95_i2c_data[2];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorNxpSe95_IsPresent(SENSOR_ID sensor_id)
{
	SE95_CTRL*		se95_ctrl_ptr = &sensor_se95_ctrl[sensor_id];
	U8				data = 0x05;
    
    if(sensor_id >= sensor_se95_count)
    {
        return FALSE;
    }

	se95_ctrl_ptr->i2c_is_present = FALSE;
    if(DrvI2cMasterDevice_WriteData(se95_ctrl_ptr->i2c_device_id, &data, 1, TRUE))
    {
        if((sensor_se95_success) && DrvI2cMasterDevice_ReadData(se95_ctrl_ptr->i2c_device_id, &data, 1, TRUE))
        {
            if((sensor_se95_success) && (data == 0xA1))
            {
				data = 0x00;
				if(DrvI2cMasterDevice_WriteData(se95_ctrl_ptr->i2c_device_id, &data, 1, TRUE))
				{
					se95_ctrl_ptr->i2c_is_present = sensor_se95_success;
				}
            }
        }
    }
    return se95_ctrl_ptr->i2c_is_present;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorNxpSe95_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	SE95_CTRL*		se95_ctrl_ptr = &sensor_se95_ctrl[sensor_id];

    if(sensor_id >= sensor_se95_count)
    {
        return FALSE;
    }
    
    if(sensor_type == SENSOR_TYPE_TEMPERATURE)
    {
        se95_ctrl_ptr->temperature = 0;
        se95_ctrl_ptr->successfull_reading = FALSE;
    }
    else
    {
        return FALSE;
    }

	if((se95_ctrl_ptr->i2c_is_present) && (sensor_se95_active == INVALID_SENSOR_ID))
	{
		sensor_se95_active = sensor_id;
		MEMSET((VPTR)sensor_se95_i2c_data, 0, SIZEOF(sensor_se95_i2c_data));
		if(DrvI2cMasterDevice_ReadData(se95_ctrl_ptr->i2c_device_id, sensor_se95_i2c_data, 2, wait_to_complete))
		{
			return TRUE;
		}
		sensor_se95_active = INVALID_SENSOR_ID;
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorNxpSe95_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	SE95_CTRL*		se95_ctrl_ptr = &sensor_se95_ctrl[sensor_id];

    if(sensor_id >= sensor_se95_count)
    {
        return FALSE;
    }

	if((sensor_type == SENSOR_TYPE_TEMPERATURE) && (se95_ctrl_ptr->i2c_is_present))
	{
		*data_ptr = (U16)se95_ctrl_ptr->temperature;
		return se95_ctrl_ptr->successfull_reading;
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorNxpSe95_I2cMessageComplete(BOOL success)
{
	SE95_CTRL*		se95_ctrl_ptr = &sensor_se95_ctrl[sensor_se95_active];

	sensor_se95_success = success;

	if(sensor_se95_active < sensor_se95_count)
	{
		sensor_se95_active = INVALID_SENSOR_ID;
        se95_ctrl_ptr->successfull_reading = success;
		if(success)
		{
			se95_ctrl_ptr->temperature = (S16)(((U16)sensor_se95_i2c_data[0] << 8) | (U16)sensor_se95_i2c_data[1]);
			se95_ctrl_ptr->temperature >>= 4;
			se95_ctrl_ptr->temperature *= 10;
			se95_ctrl_ptr->temperature >>= 4;
		}
	}
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorNxpSe95_Init(void)
{
	sensor_se95_count = 0;
	sensor_se95_active = INVALID_SENSOR_ID;

	MEMSET((VPTR)sensor_se95_ctrl, 0, SIZEOF(sensor_se95_ctrl));
	MEMSET((VPTR)sensor_se95_struct, 0, SIZEOF(sensor_se95_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorNxpSe95_Register(I2C_CHANNEL_HNDL i2c_channel, U8 address)
{
	SENSOR_STRUCT*	sensor_struct_ptr = &sensor_se95_struct[sensor_se95_count];
	SE95_CTRL*		se95_ctrl_ptr = &sensor_se95_ctrl[sensor_se95_count];

	if(sensor_se95_count < SE95_COUNT)
	{
		se95_ctrl_ptr->i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, address, 24000);
		if(se95_ctrl_ptr->i2c_device_id == INVALID_I2C_DEVICE_ID)
		{
			LOG_WRN("SE95 i2c registration failed");
			return NULL;
		}
		DrvI2cMasterDevice_MsgComplete(se95_ctrl_ptr->i2c_device_id, DrvSensorNxpSe95_I2cMessageComplete);
		se95_ctrl_ptr->i2c_is_present = FALSE;
		se95_ctrl_ptr->temperature = 0;
        se95_ctrl_ptr->successfull_reading = FALSE;

		sensor_struct_ptr->hook_list_ptr = (SENSOR_HOOK_LIST*)&sensor_se95_hook_list;
		sensor_struct_ptr->sensor_id = sensor_se95_count;
		sensor_se95_count++;
		return sensor_struct_ptr;
	}
	LOG_WRN("SE95 count overrun");
	return NULL;
}
//================================================================================================//
