//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the Silicon Labs Si7021 sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORSI7021_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORSI7021_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORSI7021_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of SE95 sensors
#ifndef SI7021_COUNT
	#define SI7021_COUNT                 1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sensor\DrvSensorSi7021.h"
#include "i2c\DrvI2cMasterDevice.h"
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
    SI7021_NONE = 0x00,
    SI7021_TEMP = 0xE3,
    SI7021_RH   = 0xE5
}
SI7021_ACTION;

typedef struct
{
	I2C_DEVICE_ID	i2c_device_id;
	BOOL			i2c_is_present;
	S16				temperature;
	U16				rh_value;
    SI7021_ACTION   si7021_action;
    BOOL            successfull_reading_temp;
    BOOL            successfull_reading_rh;
}
SI7021_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSi7021_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorSi7021_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorSi7021_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
static void DrvSensorSi7021_I2cMessageComplete(BOOL success);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const SENSOR_HOOK_LIST		sensor_si7021_hook_list = {DrvSensorSi7021_IsPresent, DrvSensorSi7021_ReadSensor, DrvSensorSi7021_GetValue};
static U8							sensor_si7021_count;
static SI7021_CTRL					sensor_si7021_ctrl[SI7021_COUNT];
static SENSOR_STRUCT				sensor_si7021_struct[SI7021_COUNT];
static BOOL							sensor_si7021_success;
static SENSOR_ID					sensor_si7021_active;
static U8							sensor_si7021_i2c_data[3];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSi7021_IsPresent(SENSOR_ID sensor_id)
{
	SI7021_CTRL*	si7021_ctrl_ptr = &sensor_si7021_ctrl[sensor_id];
	U8				data[2];

    if(sensor_id >= sensor_si7021_count)
    {
        return FALSE;
    }

	si7021_ctrl_ptr->i2c_is_present = FALSE;
    data[0] = 0xFA;
    data[1] = 0x0F;
    if(DrvI2cMasterDevice_WriteData(si7021_ctrl_ptr->i2c_device_id, data, 2, TRUE))
    {
        MEMSET((VPTR)data, 0, SIZEOF(data));
        if((sensor_si7021_success) && DrvI2cMasterDevice_ReadData(si7021_ctrl_ptr->i2c_device_id, data, 2, TRUE))
        {
            si7021_ctrl_ptr->i2c_is_present = (BOOL)(sensor_si7021_success && (data[0] != 0) && (data[1] != 0));
        }
    }
    return si7021_ctrl_ptr->i2c_is_present;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSi7021_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	SI7021_CTRL* si7021_ctrl_ptr = &sensor_si7021_ctrl[sensor_id];

    if(sensor_id >= sensor_si7021_count)
    {
        return FALSE;
    }

    if(sensor_type == SENSOR_TYPE_TEMPERATURE)
    {
        si7021_ctrl_ptr->si7021_action = SI7021_TEMP;
        si7021_ctrl_ptr->temperature = 0;
        si7021_ctrl_ptr->successfull_reading_temp = FALSE;
    }
    else if(sensor_type == SENSOR_TYPE_RH)
    {
        si7021_ctrl_ptr->si7021_action = SI7021_RH;
        si7021_ctrl_ptr->rh_value = 0;
        si7021_ctrl_ptr->successfull_reading_rh = FALSE;
    }
    else
    {
        return FALSE;
    }

	if((si7021_ctrl_ptr->i2c_is_present) && (sensor_si7021_active == INVALID_SENSOR_ID))
	{
        if(DrvI2cMasterDevice_WriteData(si7021_ctrl_ptr->i2c_device_id, (U8*)&si7021_ctrl_ptr->si7021_action, 1, TRUE))
        {
            if(sensor_si7021_success)
            {
                sensor_si7021_active = sensor_id;
                MEMSET((VPTR)sensor_si7021_i2c_data, 0, SIZEOF(sensor_si7021_i2c_data));
                if(DrvI2cMasterDevice_ReadData(si7021_ctrl_ptr->i2c_device_id, sensor_si7021_i2c_data, 3, wait_to_complete))
                {
                    return TRUE;
                }
                sensor_si7021_active = INVALID_SENSOR_ID;
            }
        }
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSi7021_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	SI7021_CTRL*		si7021_ctrl_ptr = &sensor_si7021_ctrl[sensor_id];

    if(sensor_id >= sensor_si7021_count)
    {
        return FALSE;
    }

    if(si7021_ctrl_ptr->i2c_is_present)
    {
        if(sensor_type == SENSOR_TYPE_TEMPERATURE)
        {
            *data_ptr = (U16)si7021_ctrl_ptr->temperature;
            return si7021_ctrl_ptr->successfull_reading_temp;
        }
        if(sensor_type == SENSOR_TYPE_RH)
        {
            *data_ptr = (U16)si7021_ctrl_ptr->rh_value;
            return si7021_ctrl_ptr->successfull_reading_rh;
        }
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorSi7021_I2cMessageComplete(BOOL success)
{
	SI7021_CTRL* si7021_ctrl_ptr = &sensor_si7021_ctrl[sensor_si7021_active];

	sensor_si7021_success = success;

	if(sensor_si7021_active < sensor_si7021_count)
	{
		sensor_si7021_active = INVALID_SENSOR_ID;
        if(si7021_ctrl_ptr->si7021_action == SI7021_TEMP)
        {
            si7021_ctrl_ptr->successfull_reading_temp = success;
            if(success)
            {
                si7021_ctrl_ptr->temperature = ((((S32)(((U16)sensor_si7021_i2c_data[0] << 8) | (U16)sensor_si7021_i2c_data[1]) * 17572) >> 16) - 4680) / 10;
            }
        }
        else if(si7021_ctrl_ptr->si7021_action == SI7021_RH)
        {
            si7021_ctrl_ptr->successfull_reading_rh = success;
            if(success)
            {
                si7021_ctrl_ptr->rh_value = (((U32)(((U16)sensor_si7021_i2c_data[0] << 8) | (U16)sensor_si7021_i2c_data[1]) * 12500L) >> 16) - 600;
            }
        }
        si7021_ctrl_ptr->si7021_action = SI7021_NONE;
	}
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorSi7021_Init(void)
{
	sensor_si7021_count = 0;
	sensor_si7021_active = INVALID_SENSOR_ID;

	MEMSET((VPTR)sensor_si7021_ctrl, 0, SIZEOF(sensor_si7021_ctrl));
	MEMSET((VPTR)sensor_si7021_struct, 0, SIZEOF(sensor_si7021_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorSi7021_Register(I2C_CHANNEL_HNDL i2c_channel)
{
	SENSOR_STRUCT*	sensor_struct_ptr = &sensor_si7021_struct[sensor_si7021_count];
	SI7021_CTRL*	si7021_ctrl_ptr   = &sensor_si7021_ctrl[sensor_si7021_count];

	if(sensor_si7021_count < SI7021_COUNT)
	{
		si7021_ctrl_ptr->i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, 0x40, 24000);
		if(si7021_ctrl_ptr->i2c_device_id == INVALID_I2C_DEVICE_ID)
		{
			LOG_WRN("SI7021 i2c registration failed");
			return NULL;
		}
		DrvI2cMasterDevice_MsgComplete(si7021_ctrl_ptr->i2c_device_id, DrvSensorSi7021_I2cMessageComplete);
		si7021_ctrl_ptr->i2c_is_present = FALSE;
		si7021_ctrl_ptr->temperature = 0;
        si7021_ctrl_ptr->rh_value = 0;
        si7021_ctrl_ptr->si7021_action = SI7021_NONE;
        si7021_ctrl_ptr->successfull_reading_temp = FALSE;
        si7021_ctrl_ptr->successfull_reading_rh = FALSE;

		sensor_struct_ptr->hook_list_ptr = (SENSOR_HOOK_LIST*)&sensor_si7021_hook_list;
		sensor_struct_ptr->sensor_id = sensor_si7021_count;
		sensor_si7021_count++;
		return sensor_struct_ptr;
	}
	LOG_WRN("SI7021 count overrun");
	return NULL;
}
//------------------------------------------------------------------------------------------------//
void DrvSensorSi7021_Reset(void)
{
	SI7021_CTRL*	si7021_ctrl_ptr = &sensor_si7021_ctrl[0];
	U8				data[1];

	si7021_ctrl_ptr->i2c_is_present = FALSE;
    data[0] = 0xFE;
    DrvI2cMasterDevice_WriteData(si7021_ctrl_ptr->i2c_device_id, data, 1, TRUE);
}
//================================================================================================//
