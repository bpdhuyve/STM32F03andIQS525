//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the SenseAire S8 sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORSENSEAIRS8_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORSENSEAIRS8_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORSENSEAIRS8_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of SE95 sensors
#ifndef SENSOR_COUNT
	#define SENSOR_COUNT                1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sensor\DrvSensorSenseAirS8.h"

#define Q_PREFIX(postfix)                   SenseAirS8_##postfix
#define Q_SIZE                              20
#include "sci\DrvSciQRxTpl.h"

#define Q_PREFIX(postfix)                   SenseAirS8_##postfix
#define Q_SIZE                              20
#include "sci\DrvSciQTxTpl.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//commands
#define CMD_READ        0x02
#define CMD_UPDATE      0x03
#define CMD_STATUS      0xB6
#define CMD_SELF_TEST   0xC0
#define CMD_LOOPBACK    0x00

//aditional data
#define GAS_PPM         0x03
#define SERIAL_NUMBER   0x01
#define ELEVATION       0x0f
#define START           0x00
#define RESULTS         0x01
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef struct
{
	SCI_CHANNEL_HNDL    sci_channel;
	BOOL			    is_present;
	S16				    co2_level;
}
SENSEAIRS8_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSenseAirS8_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorSenseAirS8_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorSenseAirS8_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
static void DrvSensorSenseAirS8_SendMsg(void);
static void DrvSensorSenseAirS8_Task(VPTR data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SENSOR_HOOK_LIST				sensor_senseairs8_hook_list;
static U8							sensor_senseairs8_count;
static SENSEAIRS8_CTRL				sensor_senseairs8_ctrl[SENSOR_COUNT];
static SENSOR_STRUCT				sensor_senseairs8_struct[SENSOR_COUNT];
static SENSOR_ID					sensor_senseairs8_active;
static TASK_HNDL                    sensor_senseairs8_task;
static U16                          sensor_senseairs8_task_count;

static const SCI_CONFIG_STRUCT      sensor_senseairs8_sci_config = {SCI_SPEED_9600_BPS, SCI_PARITY_NONE, SCI_STOPBIT_1, SCI_DATA_LENGTH_8_BITS};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSenseAirS8_IsPresent(SENSOR_ID sensor_id)
{
	SENSEAIRS8_CTRL*    senseairs8_ctrl_ptr = &sensor_senseairs8_ctrl[sensor_id];

	senseairs8_ctrl_ptr->is_present = DrvSensorSenseAirS8_ReadSensor(sensor_id, SENSOR_TYPE_CO2, TRUE);
    
    if(senseairs8_ctrl_ptr->is_present == FALSE)
    {
        LOG_DEV("SENSEAIR S8 not found");
    }
    
    return senseairs8_ctrl_ptr->is_present;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSenseAirS8_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	SENSEAIRS8_CTRL*    senseairs8_ctrl_ptr = &sensor_senseairs8_ctrl[sensor_id];
    U8                  recv_data[13];
    U16                 countdown = 0x8000;
    
    if(sensor_senseairs8_active == INVALID_SENSOR_ID)
    {
        // take SCI channel
        DrvSciChannel_Config(senseairs8_ctrl_ptr->sci_channel, (SCI_CONFIG_STRUCT*)&sensor_senseairs8_sci_config);
        SenseAirS8_SciQRx_SwitchSciChannel(senseairs8_ctrl_ptr->sci_channel);
        SenseAirS8_SciQTx_SwitchSciChannel(senseairs8_ctrl_ptr->sci_channel);
        
        DrvSensorSenseAirS8_SendMsg();
        
        if(wait_to_complete)
        {
            while(SenseAirS8_SciQRx_GetCount() < 13)
            {
                if(--countdown == 0)
                {
                    return FALSE;
                }
            }
            SenseAirS8_SciQRx_Read(recv_data, 13);
            SenseAirS8_SciQRx_DropAll();
            LOG_DEV("SENSEAIR S8 RX : %02h (i = %d)", PU8A(recv_data, 13), PU16(countdown));
            senseairs8_ctrl_ptr->co2_level = ((U16)recv_data[9] << 8) | (U16)recv_data[10];
        }
        else
        {
            sensor_senseairs8_active = sensor_id;
            sensor_senseairs8_task_count = 0;
            CoreTask_Start(sensor_senseairs8_task);
        }
        return TRUE;
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorSenseAirS8_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	SENSEAIRS8_CTRL*		senseairs8_ctrl_ptr = &sensor_senseairs8_ctrl[sensor_id];
    
    if((sensor_type == SENSOR_TYPE_CO2) && (senseairs8_ctrl_ptr->is_present))
    {
        *data_ptr = senseairs8_ctrl_ptr->co2_level;
        return TRUE;
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorSenseAirS8_SendMsg(void)
{
    U8  data[8] = {0xFE, 0x04, 0x00, 0x00, 0x00, 0x04, 0xE5, 0xC6};
    
    LOG_DEV("SENSEAIR S8 TX : %02h", PU8A(data, 8));
    
    SenseAirS8_SciQRx_DropAll();   // Flush RX
    
    SenseAirS8_SciQTx_Write(data, 8);
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorSenseAirS8_Task(VPTR data_ptr)
{
    SENSEAIRS8_CTRL*    senseairs8_ctrl_ptr = &sensor_senseairs8_ctrl[sensor_senseairs8_active];
    U8                  recv_data[13];
    
    if(sensor_senseairs8_active < sensor_senseairs8_count)
    {
        if(SenseAirS8_SciQRx_GetCount() >= 13)
        {
            SenseAirS8_SciQRx_Read(recv_data, 13);
            senseairs8_ctrl_ptr->co2_level = ((U16)recv_data[9] << 8) | (U16)recv_data[10];
        }
        else if(++sensor_senseairs8_task_count < 350)
        {
            return;
        }
    }
    LOG_DEV("SENSEAIR S8 meas count %d", PU16(sensor_senseairs8_task_count));
    sensor_senseairs8_active = INVALID_SENSOR_ID;
    CoreTask_Stop(sensor_senseairs8_task);
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorSenseAirS8_Init(void)
{
	sensor_senseairs8_hook_list.is_present_hook = DrvSensorSenseAirS8_IsPresent;
	sensor_senseairs8_hook_list.read_sensor_hook = DrvSensorSenseAirS8_ReadSensor;
	sensor_senseairs8_hook_list.get_value_hook = DrvSensorSenseAirS8_GetValue;

	sensor_senseairs8_count = 0;
	sensor_senseairs8_active = INVALID_SENSOR_ID;
    
    SenseAirS8_SciQRx_Create(NULL);
    SenseAirS8_SciQTx_Create(NULL);
    
    sensor_senseairs8_task_count = 0;
    sensor_senseairs8_task = CoreTask_RegisterTask(1000, DrvSensorSenseAirS8_Task, NULL, 120, "SenseAirS8");

	MEMSET((VPTR)sensor_senseairs8_ctrl, 0, SIZEOF(sensor_senseairs8_ctrl));
	MEMSET((VPTR)sensor_senseairs8_struct, 0, SIZEOF(sensor_senseairs8_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorSenseAirS8_Register(SCI_CHANNEL_HNDL sci_channel)
{
	SENSOR_STRUCT*	    sensor_struct_ptr = &sensor_senseairs8_struct[sensor_senseairs8_count];
	SENSEAIRS8_CTRL*	senseairs8_ctrl_ptr = &sensor_senseairs8_ctrl[sensor_senseairs8_count];

	if(sensor_senseairs8_count < SENSOR_COUNT)
	{
		senseairs8_ctrl_ptr->sci_channel = sci_channel;
		senseairs8_ctrl_ptr->is_present = FALSE;
		senseairs8_ctrl_ptr->co2_level = 0;

		sensor_struct_ptr->hook_list_ptr = &sensor_senseairs8_hook_list;
		sensor_struct_ptr->sensor_id = sensor_senseairs8_count;
		sensor_senseairs8_count++;
		return sensor_struct_ptr;
	}
	LOG_WRN("SENSEAIR S8 count overrun");
	return NULL;
}
//================================================================================================//
