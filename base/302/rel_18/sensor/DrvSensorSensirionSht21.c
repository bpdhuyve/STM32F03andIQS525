//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the Sensirion SHT21 sensor driver
// REMARK: This Implementation can also be used for:
//				Si7021 from Silicon Labs
//				HTU21 from TE connectivity
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORSENSIRIONSHT21_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORSENSIRIONSHT21_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORSENSIRIONSHT21_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of SHT21 sensors
#ifndef SHT21_COUNT
	#define SHT21_COUNT                 1
#endif

// CRC
#define INIT_VECTOR 					0
#define POLYNOMIAL 						0x131
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sensor\DrvSensorSensirionSht21.h"
#include "i2c\DrvI2cMasterDevice.h"

//CRC
#include "crc\StdCrc.h"
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
    SHT21_NONE = 0x00,
    SHT21_TEMP = 0xE3,
    SHT21_RH   = 0xE5
}
SHT21_ACTION;

typedef struct
{
	I2C_DEVICE_ID	i2c_device_id;
	BOOL			i2c_is_present;
	S16				temperature;
	U16				rh_value;
    SHT21_ACTION    sht21_action;
    BOOL            successfull_reading_temp;
    BOOL            successfull_reading_rh;
}
SHT21_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorSensirionSht21_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorSensirionSht21_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
static BOOL DrvSensorSensirionSht21_Reset(SENSOR_ID sensor_id);
static void DrvSensorSensirionSht21_I2cMessageComplete(BOOL success);
static BOOL DrvSensorSensirionSht21_checkCrcSerialNumber(U8* data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const SENSOR_HOOK_LIST		sensor_sht21_hook_list = {	DrvSensorSensirionSht21_IsPresent, 
																DrvSensorSensirionSht21_ReadSensor, 
																DrvSensorSensirionSht21_GetValue, 
																DrvSensorSensirionSht21_Reset};
static U8							sensor_sht21_count;
static SHT21_CTRL					sensor_sht21_ctrl[SHT21_COUNT];
static SENSOR_STRUCT				sensor_sht21_struct[SHT21_COUNT];
static BOOL							sensor_sht21_success;
static SENSOR_ID					sensor_sht21_active;
static U8							sensor_sht21_i2c_data[3];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_IsPresent(SENSOR_ID sensor_id)
{
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_id];
	U8				data[8];

    if(sensor_id >= sensor_sht21_count)
    {
        return FALSE;
    }

	sht21_ctrl_ptr->i2c_is_present = FALSE;
    data[0] = 0xFA;
    data[1] = 0x0F;
    if(DrvI2cMasterDevice_WriteData(sht21_ctrl_ptr->i2c_device_id, data, 2, TRUE))
    {
        MEMSET((VPTR)data, 0, SIZEOF(data));
        if((sensor_sht21_success) && DrvI2cMasterDevice_ReadData(sht21_ctrl_ptr->i2c_device_id, data, 8, TRUE))
        {
            sht21_ctrl_ptr->i2c_is_present = (BOOL)(sensor_sht21_success && DrvSensorSensirionSht21_checkCrcSerialNumber(data));
        }
    }
    return sht21_ctrl_ptr->i2c_is_present;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_id];
    
    if(sensor_id >= sensor_sht21_count)
    {
        return FALSE;
    }

    if(sensor_type == SENSOR_TYPE_TEMPERATURE)
    {
        sht21_ctrl_ptr->sht21_action = SHT21_TEMP;
        sht21_ctrl_ptr->temperature = 0;
        sht21_ctrl_ptr->successfull_reading_temp = FALSE;
    }
    else if(sensor_type == SENSOR_TYPE_RH)
    {
        sht21_ctrl_ptr->sht21_action = SHT21_RH;
        sht21_ctrl_ptr->rh_value = 0;
        sht21_ctrl_ptr->successfull_reading_rh = FALSE;
    }
    else
    {
        return FALSE;
    }
    
	if((sht21_ctrl_ptr->i2c_is_present) && (sensor_sht21_active == INVALID_SENSOR_ID))
	{
        if(DrvI2cMasterDevice_WriteData(sht21_ctrl_ptr->i2c_device_id, (U8*)&sht21_ctrl_ptr->sht21_action, 1, TRUE))
        {
            if(sensor_sht21_success)
            {
                sensor_sht21_active = sensor_id;
                MEMSET((VPTR)sensor_sht21_i2c_data, 0, SIZEOF(sensor_sht21_i2c_data));
                if(DrvI2cMasterDevice_ReadData(sht21_ctrl_ptr->i2c_device_id, sensor_sht21_i2c_data, 3, wait_to_complete))
                {
                    return TRUE;
                }
                sensor_sht21_active = INVALID_SENSOR_ID;
            }
        }
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_id];
    
    if(sensor_id >= sensor_sht21_count)
    {
        return FALSE;
    }

    if(sht21_ctrl_ptr->i2c_is_present)
    {
        if(sensor_type == SENSOR_TYPE_TEMPERATURE)
        {
            *data_ptr = (U16)sht21_ctrl_ptr->temperature;
            return sht21_ctrl_ptr->successfull_reading_temp;
        }
        if(sensor_type == SENSOR_TYPE_RH)
        {
            *data_ptr = (U16)sht21_ctrl_ptr->rh_value;
            return sht21_ctrl_ptr->successfull_reading_rh;
        }
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_Reset(SENSOR_ID sensor_id)
{
	SHT21_CTRL*	sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_id];
	U8				data[1];

	if(sensor_id >= sensor_sht21_count)
    {
        return FALSE;
    }
	
	sht21_ctrl_ptr->i2c_is_present = FALSE;
    data[0] = 0xFE;
    return DrvI2cMasterDevice_WriteData(sht21_ctrl_ptr->i2c_device_id, data, 1, TRUE);
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorSensirionSht21_I2cMessageComplete(BOOL success)
{
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_sht21_active];

	sensor_sht21_success = success;

	if(sensor_sht21_active < sensor_sht21_count)
	{
		sensor_sht21_active = INVALID_SENSOR_ID;
        if(sht21_ctrl_ptr->sht21_action == SHT21_TEMP)
        {
            sht21_ctrl_ptr->successfull_reading_temp = success;
            if(success)
            {
                sht21_ctrl_ptr->temperature = ((((S32)(((U16)sensor_sht21_i2c_data[0] << 8) | (U16)sensor_sht21_i2c_data[1]) * 17572) >> 16) - 4680) / 10;
            }
        }
        else if(sht21_ctrl_ptr->sht21_action == SHT21_RH)
        {
            sht21_ctrl_ptr->successfull_reading_rh = success;
            if(success)
            {
                sht21_ctrl_ptr->rh_value = (((U32)(((U16)sensor_sht21_i2c_data[0] << 8) | (U16)sensor_sht21_i2c_data[1]) * 12500L) >> 16) - 600;
            }
        }
        sht21_ctrl_ptr->sht21_action = SHT21_NONE;
	}
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSht21_checkCrcSerialNumber(U8* data_ptr)
{
	U8 	next_checksum = *(data_ptr+1);
		
	if(!StdCrcCheckCrc8(data_ptr, 1, INIT_VECTOR, POLYNOMIAL, next_checksum))
	{
		return FALSE;
	}
	return TRUE;
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorSensirionSht21_Init(void)
{
	sensor_sht21_count = 0;
	sensor_sht21_active = INVALID_SENSOR_ID;

	MEMSET((VPTR)sensor_sht21_ctrl, 0, SIZEOF(sensor_sht21_ctrl));
	MEMSET((VPTR)sensor_sht21_struct, 0, SIZEOF(sensor_sht21_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorSensirionSht21_Register(I2C_CHANNEL_HNDL i2c_channel)
{
	SENSOR_STRUCT*	sensor_struct_ptr = &sensor_sht21_struct[sensor_sht21_count];
	SHT21_CTRL*		sht21_ctrl_ptr = &sensor_sht21_ctrl[sensor_sht21_count];

	if(sensor_sht21_count < SHT21_COUNT)
	{
		sht21_ctrl_ptr->i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, 0x40, 24000);
		if(sht21_ctrl_ptr->i2c_device_id == INVALID_I2C_DEVICE_ID)
		{
			LOG_WRN("SHT21 i2c registration failed");
			return NULL;
		}
		DrvI2cMasterDevice_MsgComplete(sht21_ctrl_ptr->i2c_device_id, DrvSensorSensirionSht21_I2cMessageComplete);
		sht21_ctrl_ptr->i2c_is_present = FALSE;
		sht21_ctrl_ptr->temperature = 0;
        sht21_ctrl_ptr->rh_value = 0;
        sht21_ctrl_ptr->sht21_action = SHT21_NONE;
        sht21_ctrl_ptr->successfull_reading_temp = FALSE;
        sht21_ctrl_ptr->successfull_reading_rh = FALSE;

		sensor_struct_ptr->hook_list_ptr = (SENSOR_HOOK_LIST*)&sensor_sht21_hook_list;
		sensor_struct_ptr->sensor_id = sensor_sht21_count;
		sensor_sht21_count++;
		return sensor_struct_ptr;
	}
	LOG_WRN("SHT21 count overrun");
	return NULL;
}
//================================================================================================//
