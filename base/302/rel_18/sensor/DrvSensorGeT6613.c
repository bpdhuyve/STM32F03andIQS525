//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the NXP SE95 sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORGET6613_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORGET6613_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORGET6613_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of SE95 sensors
#ifndef T6613_COUNT
	#define T6613_COUNT                  1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sensor\DrvSensorGeT6613.h"

#define Q_PREFIX(postfix)                   GeT6613_##postfix
#define Q_SIZE                              20
#include "sci\DrvSciQRxTpl.h"

#define Q_PREFIX(postfix)                   GeT6613_##postfix
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
T6613_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorGeT6613_IsPresent(SENSOR_ID sensor_id);
static BOOL DrvSensorGeT6613_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL DrvSensorGeT6613_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
static void DrvSensorGeT6613_SendMsg(U8* data_ptr, U8 data_len);
static void DrvSensorGeT6613_Task(VPTR data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SENSOR_HOOK_LIST				sensor_t6613_hook_list;
static U8							sensor_t6613_count;
static T6613_CTRL					sensor_t6613_ctrl[T6613_COUNT];
static SENSOR_STRUCT				sensor_t6613_struct[T6613_COUNT];
static SENSOR_ID					sensor_t6613_active;
static TASK_HNDL                    sensor_t6613_task;
static U16                          sensor_t6613_task_count;

static const SCI_CONFIG_STRUCT      sensor_t6613_sci_config = {SCI_SPEED_19200_BPS, SCI_PARITY_NONE, SCI_STOPBIT_1, SCI_DATA_LENGTH_8_BITS};
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorGeT6613_IsPresent(SENSOR_ID sensor_id)
{
	T6613_CTRL*		t6613_ctrl_ptr = &sensor_t6613_ctrl[sensor_id];
    U8              send_data[4] = {CMD_LOOPBACK, 'A', 'B', 'C'};
    U8              recv_data[6];
    U16             countdown = 0x1000;

	t6613_ctrl_ptr->is_present = FALSE;
    
    // take SCI channel
    DrvSciChannel_Config(t6613_ctrl_ptr->sci_channel, (SCI_CONFIG_STRUCT*)&sensor_t6613_sci_config);
    GeT6613_SciQRx_SwitchSciChannel(t6613_ctrl_ptr->sci_channel);
    GeT6613_SciQTx_SwitchSciChannel(t6613_ctrl_ptr->sci_channel);
    
    // send test data
    DrvSensorGeT6613_SendMsg(send_data, 4);
    
    while(GeT6613_SciQRx_GetCount() < 6)
    {
        if(--countdown == 0)
        {
            LOG_DEV("T6613 not found");
            return FALSE;
        }
    }
    GeT6613_SciQRx_Read(recv_data, 6);
    LOG_DEV("T6613 RX : %02h (i = %d)", PU8A(recv_data, 6) ,PU16(countdown));
    
    t6613_ctrl_ptr->is_present = (BOOL)((recv_data[0] == 0xFF) && (recv_data[1] == 0xFA) && (recv_data[2] == 3) &&
                                        (recv_data[3] == send_data[1]) && (recv_data[4] == send_data[2]) && (recv_data[5] == send_data[3]));
    
    GeT6613_SciQRx_DropAll();
    return t6613_ctrl_ptr->is_present;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorGeT6613_ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
	T6613_CTRL*		t6613_ctrl_ptr = &sensor_t6613_ctrl[sensor_id];
    U8              send_data[2] = {CMD_READ, GAS_PPM};
    U8              recv_data[5];
    U16             countdown = 0x1000;
    
    if(sensor_t6613_active == INVALID_SENSOR_ID)
    {
        // take SCI channel
        DrvSciChannel_Config(t6613_ctrl_ptr->sci_channel, (SCI_CONFIG_STRUCT*)&sensor_t6613_sci_config);
        GeT6613_SciQRx_SwitchSciChannel(t6613_ctrl_ptr->sci_channel);
        GeT6613_SciQTx_SwitchSciChannel(t6613_ctrl_ptr->sci_channel);
        
        DrvSensorGeT6613_SendMsg(send_data, 2);
        
        if(wait_to_complete)
        {
            while(GeT6613_SciQRx_GetCount() < 5)
            {
                if(--countdown == 0)
                {
                    return FALSE;
                }
            }
            GeT6613_SciQRx_Read(recv_data, 5);
            if((recv_data[0] == 0xFF) && (recv_data[1] == 0xFA) && (recv_data[2] == 2))
            {
                t6613_ctrl_ptr->co2_level = ((U16)recv_data[3] << 8) | (U16)recv_data[4];
            }
        }
        else
        {
            sensor_t6613_active = sensor_id;
            sensor_t6613_task_count = 0;
            CoreTask_Start(sensor_t6613_task);
        }
        return TRUE;
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL DrvSensorGeT6613_GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
	T6613_CTRL*		t6613_ctrl_ptr = &sensor_t6613_ctrl[sensor_id];
    
    if((sensor_type == SENSOR_TYPE_CO2) && (t6613_ctrl_ptr->is_present))
    {
        *data_ptr = t6613_ctrl_ptr->co2_level;
        return TRUE;
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorGeT6613_SendMsg(U8* data_ptr, U8 data_len)
{
    U8  header[3] = {0xFF, 0xFE, data_len};
    
    GeT6613_SciQRx_DropAll();   // Flush RX
    
    GeT6613_SciQTx_Write(header, 3);
    if(data_len > 0)
    {
        GeT6613_SciQTx_Write(data_ptr, data_len);
    }
}
//------------------------------------------------------------------------------------------------//
static void DrvSensorGeT6613_Task(VPTR data_ptr)
{
    T6613_CTRL*		t6613_ctrl_ptr = &sensor_t6613_ctrl[sensor_t6613_active];
    U8              recv_data[5];
    
    if(sensor_t6613_active < sensor_t6613_count)
    {
        if(GeT6613_SciQRx_GetCount() >= 5)
        {
            GeT6613_SciQRx_Read(recv_data, 5);
            if((recv_data[0] == 0xFF) && (recv_data[1] == 0xFA) && (recv_data[2] == 2))
            {
                t6613_ctrl_ptr->co2_level = ((U16)recv_data[3] << 8) | (U16)recv_data[4];
            }
        }
        else if(++sensor_t6613_task_count < 350)
        {
            return;
        }
    }
    LOG_DEV("T6613 meas count %d", PU16(sensor_t6613_task_count));
    sensor_t6613_active = INVALID_SENSOR_ID;
    CoreTask_Stop(sensor_t6613_task);
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorGeT6613_Init(void)
{
	sensor_t6613_hook_list.is_present_hook = DrvSensorGeT6613_IsPresent;
	sensor_t6613_hook_list.read_sensor_hook = DrvSensorGeT6613_ReadSensor;
	sensor_t6613_hook_list.get_value_hook = DrvSensorGeT6613_GetValue;

	sensor_t6613_count = 0;
	sensor_t6613_active = INVALID_SENSOR_ID;
    
    GeT6613_SciQRx_Create(NULL);
    GeT6613_SciQTx_Create(NULL);
    
    sensor_t6613_task_count = 0;
    sensor_t6613_task = CoreTask_RegisterTask(1000, DrvSensorGeT6613_Task, NULL, 120, "GeT6613");

	MEMSET((VPTR)sensor_t6613_ctrl, 0, SIZEOF(sensor_t6613_ctrl));
	MEMSET((VPTR)sensor_t6613_struct, 0, SIZEOF(sensor_t6613_struct));
}
//------------------------------------------------------------------------------------------------//
SENSOR_HNDL DrvSensorGeT6613_Register(SCI_CHANNEL_HNDL sci_channel)
{
	SENSOR_STRUCT*	sensor_struct_ptr = &sensor_t6613_struct[sensor_t6613_count];
	T6613_CTRL*		t6613_ctrl_ptr = &sensor_t6613_ctrl[sensor_t6613_count];

	if(sensor_t6613_count < T6613_COUNT)
	{
		t6613_ctrl_ptr->sci_channel = sci_channel;
		t6613_ctrl_ptr->is_present = FALSE;
		t6613_ctrl_ptr->co2_level = 0;

		sensor_struct_ptr->hook_list_ptr = &sensor_t6613_hook_list;
		sensor_struct_ptr->sensor_id = sensor_t6613_count;
		sensor_t6613_count++;
		return sensor_struct_ptr;
	}
	LOG_WRN("T6613 count overrun");
	return NULL;
}
//================================================================================================//
