//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the Sensirion SHT11 sensor driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define SENSOR__DRVSENSORSENSIRIONSHT11_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef SENSOR__DRVSENSORSENSIRIONSHT11_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               SENSOR__DRVSENSORSENSIRIONSHT11_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the number of SE95 sensors
#ifndef SHT11_COUNT
	#define SHT11_COUNT                 1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "sensor\DrvSensorSensirionSht11.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define CMD_READ_TEMPERATURE		0x03
#define CMD_READ_HUMIDITY		    0x05
#define CMD_WRITE_STATUS_REGISTER	0x06
#define CMD_READ_STATUS_REGISTER	0x07
#define CMD_SOFT_RESET			    0x1E

#define Sht11_ClkLow(x)             DrvGpio_SetPin(x->clk_hndl, FALSE)
#define Sht11_ClkHigh(x)            DrvGpio_SetPin(x->clk_hndl, TRUE)
#define Sht11_Data(x, y)            DrvGpio_SetPin(x->data_hndl, y)
#define Sht11_DataLow(x)            DrvGpio_SetPin(x->data_hndl, FALSE)
#define Sht11_DataHigh(x)           DrvGpio_SetPin(x->data_hndl, TRUE)
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    SHT11_NONE = 0x00,
    SHT11_TEMP = 0x03,
    SHT11_RH   = 0x05
}
SHT11_ACTION;

typedef struct
{
	DRVGPIO_PIN_HNDL    clk_hndl;
    DRVGPIO_PIN_HNDL    data_hndl;
	BOOL			    is_present;
	S16				    temperature;
	U16				    rh_value;
    SHT11_ACTION        sht11_action;
}
SHT11_CTRL;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void Sht11SoftReset(SHT11_CTRL* sht11_ctrl_ptr);
static void Sht11StartTransmission(SHT11_CTRL* sht11_ctrl_ptr);
static BOOL Sht11WaitForDataReady(SHT11_CTRL* sht11_ctrl_ptr);
static U8 Sht11ReadStatusRegister(SHT11_CTRL* sht11_ctrl_ptr);
static BOOL ShtSendByte(SHT11_CTRL* sht11_ctrl_ptr, U8 data_byte);
static U8 ShtReadByte(SHT11_CTRL* sht11_ctrl_ptr, BOOL ack);

static BOOL Sht11IsPresent(SENSOR_ID sensor_id);
static BOOL Sht11ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete);
static BOOL Sht11GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static SENSOR_HOOK_LIST				sht11_hook_list;
static U8							sht11_count;
static SHT11_CTRL					sht11_ctrl[SHT11_COUNT];
static SENSOR_STRUCT				sht11_struct[SHT11_COUNT];
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void Sht11SoftReset(SHT11_CTRL* sht11_ctrl_ptr)
{
	U8 i;
    
    DrvGpio_ReInitPin(sht11_ctrl_ptr->data_hndl, GPIO_PIN_OUTPUT_START_HIGH);

	for(i = 0; i < 9; i++)//toggle clock 9 times
    {
        Sht11_ClkHigh(sht11_ctrl_ptr);
        Sht11_ClkLow(sht11_ctrl_ptr);
	}

    Sht11StartTransmission(sht11_ctrl_ptr); //send transmission start
    
	ShtSendByte(sht11_ctrl_ptr, CMD_SOFT_RESET);
}
//------------------------------------------------------------------------------------------------//
static void Sht11StartTransmission(SHT11_CTRL* sht11_ctrl_ptr)
{
    DrvGpio_ReInitPin(sht11_ctrl_ptr->data_hndl, GPIO_PIN_OUTPUT_START_HIGH);

    Sht11_ClkHigh(sht11_ctrl_ptr);
    Sht11_DataLow(sht11_ctrl_ptr);
    Sht11_ClkLow(sht11_ctrl_ptr);

    Sht11_ClkHigh(sht11_ctrl_ptr);
    Sht11_DataHigh(sht11_ctrl_ptr);
    Sht11_ClkLow(sht11_ctrl_ptr);
}
//------------------------------------------------------------------------------------------------//
static BOOL Sht11WaitForDataReady(SHT11_CTRL* sht11_ctrl_ptr)
{
    DrvGpio_ReInitPin(sht11_ctrl_ptr->data_hndl, GPIO_PIN_INPUT);
    return (BOOL)(DrvGpio_GetPin(sht11_ctrl_ptr->data_hndl) == FALSE);
}
//------------------------------------------------------------------------------------------------//
static U8 Sht11ReadStatusRegister(SHT11_CTRL* sht11_ctrl_ptr)
{
    Sht11StartTransmission(sht11_ctrl_ptr);
    ShtSendByte(sht11_ctrl_ptr, CMD_READ_STATUS_REGISTER);
    if(Sht11WaitForDataReady(sht11_ctrl_ptr) == FALSE)
    {
        return 0xFF;
    }
    return ShtReadByte(sht11_ctrl_ptr, FALSE);
}
//------------------------------------------------------------------------------------------------//
static BOOL ShtSendByte(SHT11_CTRL* sht11_ctrl_ptr, U8 data_byte)
{
	U8      i;
	BOOL    ack;
    
    DrvGpio_ReInitPin(sht11_ctrl_ptr->data_hndl, GPIO_PIN_OUTPUT);

    //send 8 bits data
	for(i = 0; i < 8; i++)
    {
        //set data
        Sht11_Data(sht11_ctrl_ptr, (BOOL)((data_byte & 0x80) > 0));
        //clk high
  		Sht11_ClkHigh(sht11_ctrl_ptr);
        //clk low
  		Sht11_ClkLow(sht11_ctrl_ptr);
        //shift data
        data_byte <<= 1;
	}

    //read ack bit
    DrvGpio_ReInitPin(sht11_ctrl_ptr->data_hndl, GPIO_PIN_INPUT);
    //clk high
  	Sht11_ClkHigh(sht11_ctrl_ptr);
    //get ack
	ack = DrvGpio_GetPin(sht11_ctrl_ptr->data_hndl);
    //clk low
    Sht11_ClkLow(sht11_ctrl_ptr);

    DrvGpio_ReInitPin(sht11_ctrl_ptr->data_hndl, GPIO_PIN_OUTPUT_START_HIGH);
    
    return ack;
}
//------------------------------------------------------------------------------------------------//
static U8 ShtReadByte(SHT11_CTRL* sht11_ctrl_ptr, BOOL ack)
{
	U8      i;
	U8      data_byte = 0;

    DrvGpio_ReInitPin(sht11_ctrl_ptr->data_hndl, GPIO_PIN_INPUT);

    //read 8 bits
	for(i = 0; i < 8; i++)
    {
        //shift data
        data_byte <<= 1;
        //clock high
  		Sht11_ClkHigh(sht11_ctrl_ptr);
        //get data
        data_byte |= (U8)DrvGpio_GetPin(sht11_ctrl_ptr->data_hndl);
        //clock low
  		Sht11_ClkLow(sht11_ctrl_ptr);
	}

    //send ack if needed
    DrvGpio_ReInitPin(sht11_ctrl_ptr->data_hndl, GPIO_PIN_OUTPUT_START_HIGH);
    //set ack
    Sht11_Data(sht11_ctrl_ptr, (BOOL)(ack == FALSE));
    //clk high
    Sht11_ClkHigh(sht11_ctrl_ptr);
    //clk low
    Sht11_ClkLow(sht11_ctrl_ptr);

	return data_byte;
}
//------------------------------------------------------------------------------------------------//
static BOOL Sht11IsPresent(SENSOR_ID sensor_id)
{
    SHT11_CTRL*		sht11_ctrl_ptr = &sht11_ctrl[sensor_id];
    
    if(sensor_id < sht11_count)
    {
        sht11_ctrl_ptr->is_present = (BOOL)(Sht11ReadStatusRegister(sht11_ctrl_ptr) != 0xFF);
        return sht11_ctrl_ptr->is_present;
    }
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL Sht11ReadSensor(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, BOOL wait_to_complete)
{
    SHT11_CTRL*		sht11_ctrl_ptr = &sht11_ctrl[sensor_id];
    
    if((sensor_id < sht11_count) && sht11_ctrl_ptr->is_present)
    {
        sht11_ctrl_ptr->sht11_action = SHT11_NONE;
        if(sensor_type == SENSOR_TYPE_TEMPERATURE)
        {
            sht11_ctrl_ptr->sht11_action = SHT11_TEMP;
            Sht11StartTransmission(sht11_ctrl_ptr);
            ShtSendByte(sht11_ctrl_ptr, (U8)CMD_WRITE_STATUS_REGISTER);
            ShtSendByte(sht11_ctrl_ptr, (U8)0x01);      // set to 8bit RH / 12bit Temp. resolution
        }
        else if(sensor_type == SENSOR_TYPE_RH)
        {
            sht11_ctrl_ptr->sht11_action = SHT11_RH;
            Sht11StartTransmission(sht11_ctrl_ptr);
            ShtSendByte(sht11_ctrl_ptr, (U8)CMD_WRITE_STATUS_REGISTER);
            ShtSendByte(sht11_ctrl_ptr, (U8)0x00);      // set to 12bit RH / 14bit Temp. resolution
        }
        if(sht11_ctrl_ptr->sht11_action != SHT11_NONE)
        {
            Sht11StartTransmission(sht11_ctrl_ptr);
            ShtSendByte(sht11_ctrl_ptr, (U8)sht11_ctrl_ptr->sht11_action);
            return TRUE;
        }
    }
	return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL Sht11GetValue(SENSOR_ID sensor_id, SENSOR_TYPE sensor_type, U16* data_ptr)
{
    SHT11_CTRL*		sht11_ctrl_ptr = &sht11_ctrl[sensor_id];
    U8              data[2];
    U16             so_rh;
    S32             temp;
    
    if((sensor_id < sht11_count) && sht11_ctrl_ptr->is_present)
    {
        if(sensor_type == SENSOR_TYPE_TEMPERATURE)
        {
            if((sht11_ctrl_ptr->sht11_action == SHT11_TEMP) && Sht11WaitForDataReady(sht11_ctrl_ptr))
            {
                data[0] = ShtReadByte(sht11_ctrl_ptr, TRUE);
                data[1] = ShtReadByte(sht11_ctrl_ptr, FALSE);
                
                *data_ptr = ((((U16)data[0] << 8 | (U16)data[1]) << 2) - 3965) / 10;
                
                sht11_ctrl_ptr->sht11_action = SHT11_NONE;
                return TRUE;
            }
            LOG_DEV("Get Temp NOK");
        }
        else if(sensor_type == SENSOR_TYPE_RH)
        {
            if((sht11_ctrl_ptr->sht11_action == SHT11_RH) && Sht11WaitForDataReady(sht11_ctrl_ptr))
            {
                data[0] = ShtReadByte(sht11_ctrl_ptr, TRUE);
                data[1] = ShtReadByte(sht11_ctrl_ptr, FALSE);
                so_rh = (U16)data[0] << 8 | (U16)data[1];
                
                // RH = -2.0468 + .0367 * SOrh - 1.5955e-6 * SOrh²
                // 100*RH = (-52398 + (240517 * SOrh - 2677 * (SOrh² / 2^8)) / 2^8) / 2^8
                
                temp = (S32)((U32)so_rh * (U32)so_rh);      // max 2^24
                temp >>= 8;                                 // max 2^16
                temp *= (S32)(-2677);                       // max 2^28
                temp += (240517 * (U32)so_rh);              // max 2^30
                temp >>= 8;                                 // max 2^22
                temp -= 52398;
                temp >>= 8;
                
                *data_ptr = (U16)(temp);
                
                sht11_ctrl_ptr->sht11_action = SHT11_NONE;
                return TRUE;
            }
            LOG_DEV("Get RH NOK");
        }
    }
    sht11_ctrl_ptr->sht11_action = SHT11_NONE;
	return FALSE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvSensorSensirionSht11_Init(void)
{
	sht11_hook_list.is_present_hook = Sht11IsPresent;
	sht11_hook_list.read_sensor_hook = Sht11ReadSensor;
	sht11_hook_list.get_value_hook = Sht11GetValue;
    
    sht11_count = 0;
    
    MEMSET((VPTR)sht11_ctrl, 0, SIZEOF(sht11_ctrl));
    MEMSET((VPTR)sht11_struct, 0, SIZEOF(sht11_struct));
}
//------------------------------------------------------------------------------------------------//
// @remark  the data_hndl must be able to switch between input and output!!
SENSOR_HNDL DrvSensorSensirionSht11_Register(DRVGPIO_PIN_HNDL clk_hndl, DRVGPIO_PIN_HNDL data_hndl)
{
    SENSOR_HNDL sensor_hndl = &sht11_struct[sht11_count];
    SHT11_CTRL* sht11_ctrl_ptr = &sht11_ctrl[sht11_count];
    
    if(sht11_count < SHT11_COUNT)
    {
        sht11_ctrl_ptr->clk_hndl = clk_hndl;
        sht11_ctrl_ptr->data_hndl = data_hndl;
        sht11_ctrl_ptr->is_present = FALSE;
        sht11_ctrl_ptr->temperature = 0;
        sht11_ctrl_ptr->rh_value = 0;
        
        Sht11_ClkLow(sht11_ctrl_ptr);
        Sht11_DataLow(sht11_ctrl_ptr);
        
        Sht11SoftReset(sht11_ctrl_ptr);
        
        sensor_hndl->hook_list_ptr = &sht11_hook_list;
        sensor_hndl->sensor_id = sht11_count;
        sht11_count++;
        return sensor_hndl;
    }
    return NULL;
}
//================================================================================================//
