//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the Sensirion SDP8xx sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORSENSIRIONSDP8XX_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORSENSIRIONSDP8XX_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORSENSIRIONSDP8XX_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of Sdp6xx/5xx sensors
#ifndef SDP8XX_COUNT
	#define SDP8XX_COUNT                1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// DRV
#include "sensor\DrvSensorSensirionSdp8xx.h"
#include "i2c\DrvI2cMasterDevice.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define CRC_POLYNOMIAL                              0x131               //P(x)=x^8+x^5+x^4+1
//------------------------------------------------------------------------------------------------//
#define START_CONTINUOUS_PRESSURE_MEAS_WITHOUT_AVG  0x361E
#define INVALID_SCALE_FACTOR                        0
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    SDP8XX_NONE                 = 0,
    SDP8XX_READ_PRESSURE        = 1,
}
SDP8XX_ACTION;

typedef struct
{
	I2C_DEVICE_ID	i2c_device_id;
    BOOL            i2c_is_present;
    BOOL            successfull_reading;
    SDP8XX_ACTION   sdp8xx_action;
    S16             pressure;
    U16             scale_factor;
}
SDP8XX_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp8xx_CheckCrc(U8* data, U8 data_length, U8 checksum);
static BOOL DrvSensorSensirionSdp8xx_WriteCommand(SENSOR_ID sensor_id, U16 command);
static BOOL DrvSensorSensirionSdp8xx_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorSensirionSdp8xx_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_sdp8xx_type, BOOL wait_to_complete);
static BOOL DrvSensorSensirionSdp8xx_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_sdp8xx_type, U16* data_ptr);
static void DrvSensorSensirionSdp8xx_I2cMessageComplete(BOOL success);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static const SENSOR_HOOK_LIST       sensor_sdp8xx_hook_list = {DrvSensorSensirionSdp8xx_IsPresent, DrvSensorSensirionSdp8xx_ReadSensor, DrvSensorSensirionSdp8xx_GetValue};
static U8							sensor_sdp8xx_count;
static SDP8XX_CTRL                  sensor_sdp8xx_ctrl[SDP8XX_COUNT];
static SENSOR_STRUCT				sensor_sdp8xx_struct[SDP8XX_COUNT];
static BOOL							sensor_sdp8xx_success;
static SENSOR_ID					sensor_sdp8xx_active;
static U8							sensor_sdp8xx_i2c_data[3];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp8xx_CheckCrc(U8* data, U8 data_length, U8 checksum)
{
    U8  crc = 0xFF;
    U8  byte_counter;
    U8  i;
    
    for(byte_counter = 0; byte_counter < data_length; byte_counter++)
    {
        crc ^= data[byte_counter];
        for(i = 8; i > 0; i--)
        {
            if(crc & 0x80)
            {
               crc = (crc << 1) ^ CRC_POLYNOMIAL;
            }
            else
            {
               crc = (crc << 1);
            }
        }
    }
    return (BOOL)(crc == checksum);
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp8xx_WriteCommand(SENSOR_ID sensor_id, U16 command)
{
    SDP8XX_CTRL*    sdp8xx_ctrl_ptr = &sensor_sdp8xx_ctrl[sensor_id];
    U8              data[2];
    
    if(sensor_id >= sensor_sdp8xx_count)
    {
        return FALSE;
    }
    
    // A write command consists of:
    // - Address (byte 1)
    // - Command (bytes 2 and 3)
    data[0] = (command >> 8) & 0xFF;
    data[1] = command & 0xFF;
    
    return (BOOL)(DrvI2cMasterDevice_WriteData(sdp8xx_ctrl_ptr->i2c_device_id, data, 2, TRUE) && (sensor_sdp8xx_success == TRUE));
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp8xx_IsPresent(SENSOR_ID sensor_id)
{
    SDP8XX_CTRL*    sdp8xx_ctrl_ptr = &sensor_sdp8xx_ctrl[sensor_id];
    U8              data[9];
    
    if(sensor_id >= sensor_sdp8xx_count)
    {
        return FALSE;
    }
    
    if(sensor_sdp8xx_active == INVALID_SENSOR_ID)
    {
        sensor_sdp8xx_active = sensor_id;
        sdp8xx_ctrl_ptr->i2c_is_present = FALSE;
        sdp8xx_ctrl_ptr->sdp8xx_action = SDP8XX_NONE;
        
        MEMSET((VPTR)data, 0, SIZEOF(data));
        
        // set continuous measuring without averaging + get scale factor
        DrvSensorSensirionSdp8xx_WriteCommand(sensor_id, START_CONTINUOUS_PRESSURE_MEAS_WITHOUT_AVG);
        if(DrvI2cMasterDevice_ReadData(sdp8xx_ctrl_ptr->i2c_device_id, data, 9, TRUE))
        {
            if((sensor_sdp8xx_success == TRUE) &&
               DrvSensorSensirionSdp8xx_CheckCrc(&data[0], 2, data[2]) &&
               DrvSensorSensirionSdp8xx_CheckCrc(&data[3], 2, data[5]) &&
               DrvSensorSensirionSdp8xx_CheckCrc(&data[6], 2, data[8]))
            {
                sdp8xx_ctrl_ptr->scale_factor = (U16)((data[6] << 8) | data[7]);
                sdp8xx_ctrl_ptr->i2c_is_present = TRUE;
            }
        }
        
        sensor_sdp8xx_active = INVALID_SENSOR_ID;
    }
    
    return sdp8xx_ctrl_ptr->i2c_is_present;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp8xx_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_sdp8xx_type, BOOL wait_to_complete)
{
	SDP8XX_CTRL*    sdp8xx_ctrl_ptr = &sensor_sdp8xx_ctrl[sensor_id];
    
    if(sensor_id >= sensor_sdp8xx_count)
    {
        return FALSE;
    }
    
    if(sensor_sdp8xx_type == SENSOR_TYPE_PRESSURE)
    {
        sdp8xx_ctrl_ptr->sdp8xx_action = SDP8XX_READ_PRESSURE;
        sdp8xx_ctrl_ptr->pressure = 0;
        sdp8xx_ctrl_ptr->successfull_reading = FALSE;
    }
    else
    {
        return FALSE;
    }
    
    if((sdp8xx_ctrl_ptr->i2c_is_present) && (sensor_sdp8xx_active == INVALID_SENSOR_ID))
	{
        sensor_sdp8xx_active = sensor_id;
        MEMSET((VPTR)sensor_sdp8xx_i2c_data, 0, SIZEOF(sensor_sdp8xx_i2c_data));
        if(DrvI2cMasterDevice_ReadData(sdp8xx_ctrl_ptr->i2c_device_id, sensor_sdp8xx_i2c_data, 3, wait_to_complete))
        {
            return TRUE;
        }
        sensor_sdp8xx_active = INVALID_SENSOR_ID;
	}
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSensirionSdp8xx_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_sdp8xx_type, U16* data_ptr)
{
	SDP8XX_CTRL*    sdp8xx_ctrl_ptr = &sensor_sdp8xx_ctrl[sensor_id];
    
    if(sensor_id >= sensor_sdp8xx_count)
    {
        return FALSE;
    }
    
    if(sdp8xx_ctrl_ptr->i2c_is_present)
    {
        if(sensor_sdp8xx_type == SENSOR_TYPE_PRESSURE)
        {
            // pressure is expressed in dPa
            *data_ptr = (U16)sdp8xx_ctrl_ptr->pressure;
            return sdp8xx_ctrl_ptr->successfull_reading;
        }
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorSensirionSdp8xx_I2cMessageComplete(BOOL success)
{
    SDP8XX_CTRL*    sdp8xx_ctrl_ptr = &sensor_sdp8xx_ctrl[sensor_sdp8xx_active];
    
	sensor_sdp8xx_success = success;
    
	if(sensor_sdp8xx_active < sensor_sdp8xx_count)
	{
		sensor_sdp8xx_active = INVALID_SENSOR_ID;
        if(sdp8xx_ctrl_ptr->sdp8xx_action == SDP8XX_READ_PRESSURE)
        {
            if(DrvSensorSensirionSdp8xx_CheckCrc(&sensor_sdp8xx_i2c_data[0], 2, sensor_sdp8xx_i2c_data[2]) && (sdp8xx_ctrl_ptr->scale_factor != 0))
            {
                sdp8xx_ctrl_ptr->successfull_reading = TRUE;
                sdp8xx_ctrl_ptr->pressure = (((S16)((sensor_sdp8xx_i2c_data[0] << 8) | sensor_sdp8xx_i2c_data[1]) * 10) / sdp8xx_ctrl_ptr->scale_factor);
            }
        }
        sdp8xx_ctrl_ptr->sdp8xx_action = SDP8XX_NONE;
	}
}
//================================================================================================//



//================================================================================================//
// C O M M A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
#if (TERM_LEVEL > TERM_LEVEL_NONE)
#endif
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorSensirionSdp8xx_Init(void)
{
	sensor_sdp8xx_count = 0;
	sensor_sdp8xx_active = INVALID_SENSOR_ID;
    
	MEMSET((VPTR)sensor_sdp8xx_ctrl, 0, SIZEOF(sensor_sdp8xx_ctrl));
    MEMSET((VPTR)sensor_sdp8xx_struct, 0, SIZEOF(sensor_sdp8xx_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorSensirionSdp8xx_Register(I2C_CHANNEL_HNDL i2c_channel)
{
    SENSOR_HNDL     sensor_struct_ptr = &sensor_sdp8xx_struct[sensor_sdp8xx_count];
    SDP8XX_CTRL*    sdp8xx_ctrl_ptr = &sensor_sdp8xx_ctrl[sensor_sdp8xx_count];
    
	if(sensor_sdp8xx_count < SDP8XX_COUNT)
	{
		sdp8xx_ctrl_ptr->i2c_device_id = DrvI2cMasterDevice_Register(i2c_channel, 0x25, 400000);
		if(sdp8xx_ctrl_ptr->i2c_device_id == INVALID_I2C_DEVICE_ID)
		{
			LOG_WRN("SDP8xx i2c registration failed");
			return NULL;
		}
        DrvI2cMasterDevice_MsgComplete(sdp8xx_ctrl_ptr->i2c_device_id, DrvSensorSensirionSdp8xx_I2cMessageComplete);
		sdp8xx_ctrl_ptr->i2c_is_present         = FALSE;
		sdp8xx_ctrl_ptr->pressure               = 0;
        sdp8xx_ctrl_ptr->scale_factor           = INVALID_SCALE_FACTOR;
        sdp8xx_ctrl_ptr->sdp8xx_action          = SDP8XX_NONE;
        sdp8xx_ctrl_ptr->successfull_reading    = FALSE;
        
        sensor_struct_ptr->hook_list_ptr        = (SENSOR_HOOK_LIST*)&sensor_sdp8xx_hook_list;
		sensor_struct_ptr->sensor_id            = sensor_sdp8xx_count;
		sensor_sdp8xx_count++;
		return sensor_struct_ptr;
	}
	LOG_ERR("SDP8xx count overrun");
	return NULL;
}
//================================================================================================//
