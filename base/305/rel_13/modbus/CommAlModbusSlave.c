//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Source file for the Application Layer(Slave) of the MODBUS-protocol.
// The MODBUS protocol defines a simple protocol data unit (PDU) independent of the underlying communication layers.
// The mapping of MODBUS protocol on specific buses or network can introduce some additional fields on the application
// data unit (ADU).
// The MODBUS application data unit is built by the client that initiates a MODBUS transaction. The function indicates
// to the server what kind of action to perform. The MODBUS application establishes the format of a request initated by
// a client.
// The MODBUS application protocol is described in following document found under the Oxi project directory:\n
// "\Oxi\Specifications\modbus\Modbus_Application_Protocol_V1_1a.pdf"
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define MODBUS__COMMALMODBUSSLAVE_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MODBUS__COMMALMODBUSSLAVE_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL               MODBUS__COMMALMODBUSSLAVE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max number of function codes one wishes to use
#ifndef MODBUS_INTRFC_COUNT
    #error "MODBUS_INTRFC_COUNT not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the max length of one Modbus PDU (Protocol Data Unit)
#ifndef MODBUS_LENGTH
    #error "MODBUS_LENGTH not defined in AppConfig.h"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//COMM lib include section
#include "modbus\CommDllModbus.h"
#include "modbus\CommAlModbusSlave.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
#define MODBUS_ERROR_FUNCTIONCODE   0x80

#define MODBUS_EXCEPTION_INVALID_FUNCTIONCODE   0x01
#define MODBUS_EXCEPTION_INVALID_DATA_ADDRESS   0x02
#define MODBUS_EXCEPTION_INVALID_DATA_VALUE     0x03
#define MODBUS_EXCEPTION_FAILED_FUNCTION        0x04


//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//

// @brief   Prototype of the function to check the MODBUS application data of errors
typedef BOOL (*TEST_HOOK)(U8* data_ptr);

// @brief   Prototype of dummy hook
typedef BOOL (*DUMMY_HOOK)(void);

// @brief   Interface of MODBUS definition
typedef struct
{
    U8              id;             // function code
    TEST_HOOK       hook1;          // hook for "error 3" handling
    TEST_HOOK       hook2;          // hook for "error 2" handling
    DUMMY_HOOK      modbus_hook;    // hook for request processing
}
MODBUS_INTRFC;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void CommAlModbusSlave_InstallHook(MODBUS_CHANNEL modbus_channel, MODBUS_FUNCTIONCODE func_code, TEST_HOOK hook1, TEST_HOOK hook2, DUMMY_HOOK modbus_hook);
static BOOL CommAlModbusSlave_CheckQuantityId1_2(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckQuantityId3_4(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckQuantityId15(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckQuantityId16(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckRangeId(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckOutputValue(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckByteCount(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckSubReq20(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckSubReq21(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckObjId(U8* data_ptr);
static BOOL CommAlModbusSlave_CheckDeviceIdCode(U8* data_ptr);
static BOOL CommAlModbusSlave_FindFunctionCode(MODBUS_CHANNEL modbus_channel, MODBUS_FUNCTIONCODE id, MODBUS_INTRFC* id_info_ptr);
static void CommAlModbusSlave_HandleData(MODBUS_CHANNEL modbus_channel, U8* data_ptr, U8  length, U8* reply_data_ptr, U8* reply_length_ptr);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
MODULE_DECLARE();

static MODBUS_INTRFC                    al_modbus_intrfc[MODBUS_CHANNEL_COUNT*MODBUS_INTRFC_COUNT];
static MODBUS_INTRFC                    id_info;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//


//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void CommAlModbusSlave_InstallHook(MODBUS_CHANNEL modbus_channel, MODBUS_FUNCTIONCODE func_code, TEST_HOOK hook1, TEST_HOOK hook2, DUMMY_HOOK modbus_hook)
{
    if (modbus_channel != INVALID_MODBUS_CHANNEL)
    {
        MODBUS_INTRFC* temp_ptr = &al_modbus_intrfc[(modbus_channel-1)*MODBUS_INTRFC_COUNT];

        while(temp_ptr < &al_modbus_intrfc[modbus_channel*MODBUS_INTRFC_COUNT])
        {
            if(temp_ptr->id == 0)
            {
                temp_ptr->id          = (U8)func_code;
                temp_ptr->hook1       = hook1;
                temp_ptr->hook2       = hook2;
                temp_ptr->modbus_hook = modbus_hook;
                return;
            }
            temp_ptr++;
        }
        LOG_ERR("Failed to install hook %02h", PU8(func_code));
    }
    else
    {
        LOG_WRN("Invalid MODBUS channel");
    }
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckQuantityId1_2(U8* data_ptr)
{
    U16 quantity;

    quantity = ((data_ptr[2]) << 8) + data_ptr[3];

    if((quantity < 0x0001) || (quantity > 0x07D0))
    {
        LOG_WRN("Error: The quantity doesn't fit in the range");
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckQuantityId3_4(U8* data_ptr)
{
    U16 quantity;

    quantity = ((data_ptr[2]) << 8) + data_ptr[3];

    if((quantity < 0x0001) || (quantity > 0x07D))
    {
        LOG_WRN("Error: The quantity doesn't fit in the range");
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckQuantityId15(U8* data_ptr)
{
    U16 quantity;
    U8  byte_count;
    U8  temp;

    quantity = ((data_ptr[2]) << 8) + data_ptr[3];
    byte_count = data_ptr[4];

    temp = quantity >> 3;

    if(quantity - (temp << 3))
    {
        temp += 1;
    }

    if(byte_count != temp)
    {
        LOG_WRN("Error: The byte count is wrong");
        return FALSE;
    }

    if((quantity < 0x0001) || (quantity > 0x07B0))
    {
        LOG_WRN("Error: The quantity doesn't fit in the range");
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckQuantityId16(U8* data_ptr)
{
    U16 quantity;
    U8  byte_count;
    U8  temp;

    quantity = ((data_ptr[2]) << 8) + data_ptr[3];
    byte_count = data_ptr[4];

    temp = quantity << 1; // quantity * 2

    if(byte_count != temp)
    {
        LOG_WRN("Error: The byte count is wrong");
        return FALSE;
    }
    if((quantity < 0x0001) || (quantity > 0x007B))
    {
        LOG_WRN("Error: The quantity doesn't fit in the range");
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckRangeId(U8* data_ptr)
{
    U32 temp;
    U16 addr;
    U16 quantity;

    addr = *data_ptr;
    data_ptr--;
    addr += (((U16)(*data_ptr)) << 8);
    quantity = (((U16)data_ptr[2]) << 8) + data_ptr[3];
    temp = (U32)addr + (U32)quantity;

    if(temp > 0xFFFF)
    {
        LOG_WRN("Error: The range is too large");
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckOutputValue(U8* data_ptr)
{
    U16 value;

    value = ((data_ptr[2]) << 8) + data_ptr[3];

    if((value == 0x0000) || (value == 0xFF00))
    {
        return TRUE;
    }
    
    LOG_WRN("Error: The output value isn't supported");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckByteCount(U8* data_ptr)
{
    U8 data_length;

    data_length = *data_ptr;

    if((data_length < 0x07) || (data_length > 0xF5))
    {
        LOG_WRN("Error: The byte count doesn't fit in the range");
        return FALSE;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckSubReq20(U8* data_ptr)
{
    U8  ref_type;
    U16 record_nb;
    U16 record_length;
    U16 temp;
    U8  byte_count;
    U8  i;
    U8  j;
    static  U8  length_temp = 0;

    byte_count = *(data_ptr - 1);

    for(i = byte_count, j = 0, length_temp = 0; i > 0; i = i - 7, j++)
    {
        ref_type = *data_ptr;

        record_nb = (data_ptr[3] << 8) + data_ptr[4];
        record_length = (data_ptr[5] << 8) + data_ptr[6];
        temp = record_nb + record_length;
        //file 1: record 0-9999, file 2: record 10000-19999, ....
        if((ref_type != 0x06) || (record_nb > 0x270F) || (temp > 0x270F))
        {
            LOG_WRN("Error: The 'sub-request' field is wrong");
            return FALSE;
        }
        data_ptr += 7;
        length_temp += (record_length * 2) + 2; //calculation of sub_response: record(= 2bytes) + file_resp_length
    }                                           //                                        + reference_type
    if(length_temp > (MODBUS_LENGTH - 2))       //-2 because of func code and resp.data length
    {
        LOG_WRN("Error: The length to reply will be too long(check max MODBUS frame length).");
        return FALSE;
    }

    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckSubReq21(U8* data_ptr)
{
    U8  ref_type;
    U16 record_nb;
    U16 record_length;
    U16 temp;
    U8  byte_count;
    U8  i;
    U8  length_temp;

    byte_count = *(data_ptr - 1);

    for(i = byte_count; i > 0; i = i - length_temp)
    {
        ref_type = *data_ptr;

        record_nb = (data_ptr[3] << 8) + data_ptr[4];
        record_length = (data_ptr[5] << 8) + data_ptr[6];
        temp = record_nb + record_length;

        if((ref_type != 0x06) || (record_nb > 0x270F) || (temp > 0x270F))
        {
            LOG_WRN("Error: The 'sub-request' field is wrong");
            return FALSE;
        }
        length_temp = (U8)(7 + (record_length * 2));
        data_ptr += length_temp;
    }
    return TRUE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckObjId(U8* data_ptr)
{
    U8  object_id;

    object_id = data_ptr[2];

    if(object_id < 0x03)
    {
        return TRUE;
    }
    LOG_WRN("Error: The object id is not supported");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_CheckDeviceIdCode(U8* data_ptr)
{
    U8  id_code;

    id_code = *data_ptr;

    if(id_code == 0x01)
    {
        return TRUE;
    }
    LOG_WRN("Error: The 'Read Device ID code' is not accepted");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static BOOL CommAlModbusSlave_FindFunctionCode(MODBUS_CHANNEL modbus_channel, MODBUS_FUNCTIONCODE id, MODBUS_INTRFC* id_info_ptr)
{
    MODBUS_INTRFC* temp_ptr = &al_modbus_intrfc[modbus_channel-1];
    while(temp_ptr < &al_modbus_intrfc[modbus_channel*MODBUS_INTRFC_COUNT])
    {
        if(temp_ptr->id != 0)
        {
            if(temp_ptr->id == id)
            {
                MEMCPY((VPTR)id_info_ptr,(VPTR)(temp_ptr),SIZEOF(MODBUS_INTRFC));
                return TRUE;
            }
        }
        temp_ptr++;
    }
    LOG_WRN("Function code is not supported");
    return FALSE;
}
//------------------------------------------------------------------------------------------------//
static void CommAlModbusSlave_HandleData(MODBUS_CHANNEL modbus_channel, U8* data_ptr, U8  length, U8* reply_data_ptr, U8* reply_length_ptr)
{
  
    U8  i;
    U8  j;
    U8  byte_cnt;
    static U16  output_value_ptr[MODBUS_LENGTH / 2];
    STRING      al_modbus_reply_str[3];
    U16         temp;
    STRING      temp_str;
    U8          length_temp;

    U16 file_number;
    U16 rec_number;
    U16 rec_length;

    U16* reply_temp_ptr_16;     //needed for func code 0x03 & 0x04
    U8* reply_temp_ptr_8;   //needed for func code 0x14
    BOOL     value;          //needed for func code 0x05

    U8  func_type;

    U16 addr;
    U16 byte_count;
    U8  mei_type;

    U16 quantity;
    U16 output_value;
    //U8  device_id;

    //U8  output_length;
    U8  object_id;

    func_type  = *data_ptr;

    //first data for func code 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x0F, 0x10
    addr       = (data_ptr[1] << 8) + data_ptr[2];
    //first data for func code 0x14, 0x15
    byte_count = data_ptr[1];
    //first data for func code 0x2B
    mei_type  = data_ptr[1];

    //second data for func code 0x01, 0x02, 0x03, 0x04, 0x0F, 0x10
    quantity   = (data_ptr[3] << 8) + data_ptr[4];
    //second data for func code 0x05, 0x06
    output_value = (data_ptr[3] << 8) + data_ptr[4];
    //second data for func code 0x2B
    //device_id = data_ptr[2];

    //third data for func code 0x0F, 0x10
    //output_length = data_ptr[5];
    //third data for func code 0x2B
    object_id = data_ptr[3];

    //Calculate the byte count
    // byte_cnt = Quantity of Outputs/8, if the remainder is different of 0 => byte_cnt = byte_cnt+1
    if( quantity - ((quantity >> 3) << 3) )
    {
        byte_cnt = (U8)((quantity >> 3) + 1);
    }
    else
    {
        byte_cnt = (U8)(quantity >> 3);
    }

    if(func_type == ENCAP_INTERF_TRANSPORT)
    {
        if(mei_type != 0x0E)
        {
            LOG_WRN("Function code is not supported");
            //Function code is not supported
            *reply_data_ptr = func_type | MODBUS_ERROR_FUNCTIONCODE;
            reply_data_ptr++;
            *reply_data_ptr = 0x0E;
            reply_data_ptr++;
            *reply_data_ptr = 0x01;
            *reply_length_ptr = 3;
            return;
        }
    }
    if(!CommAlModbusSlave_FindFunctionCode(modbus_channel, (MODBUS_FUNCTIONCODE)func_type, &id_info))
    {
        //Function code is not supported
        *reply_data_ptr = func_type | MODBUS_ERROR_FUNCTIONCODE;
        reply_data_ptr++;
        *reply_data_ptr = MODBUS_EXCEPTION_INVALID_FUNCTIONCODE;
        *reply_length_ptr = 2;
    }
    else if(   ((id_info.hook1 != NULL) && (!id_info.hook1(++data_ptr)))
            || ((id_info.hook2 != NULL) && (!id_info.hook2(++data_ptr))))
    {
        *reply_data_ptr = id_info.id | MODBUS_ERROR_FUNCTIONCODE;
        reply_data_ptr++;
        if(func_type != ENCAP_INTERF_TRANSPORT)
        {
            *reply_data_ptr = MODBUS_EXCEPTION_INVALID_DATA_VALUE;
            *reply_length_ptr = 2;
        }
        else
        {
            *reply_data_ptr = 0x0E;
            reply_data_ptr++;
            *reply_data_ptr = MODBUS_EXCEPTION_INVALID_DATA_ADDRESS;
            *reply_length_ptr = 3;
        }
    }
    else if(id_info.modbus_hook == NULL)
    {
        //MODBUS hook is NULL
        *reply_data_ptr = id_info.id | MODBUS_ERROR_FUNCTIONCODE;
        reply_data_ptr++;
        *reply_data_ptr = MODBUS_EXCEPTION_FAILED_FUNCTION;
        *reply_length_ptr = 2;
    }
    else
    {
        switch(id_info.id)
        {
        case READ_COILS:
            //request processing
            if(((ALHOOK_READ_COILS)(id_info.modbus_hook))(addr + 1, quantity, &reply_data_ptr[2]))
            {
                reply_data_ptr[0] = id_info.id;
                reply_data_ptr[1] = byte_cnt;
                *reply_length_ptr = byte_cnt + 2;
                return;
            }
            break;
        case READ_DISCRETE_INPUTS:
            //request processing
            if(((ALHOOK_READ_DISCR_INP)(id_info.modbus_hook))(addr + 1, quantity, &reply_data_ptr[2]))
            {
                reply_data_ptr[0] = id_info.id;
                reply_data_ptr[1] = byte_cnt;
                *reply_length_ptr = byte_cnt + 2;
                return;
            }
            break;
        case READ_HOLDING_REGISTERS:
            //request processing
            if(((ALHOOK_READ_HOLD_REG)(id_info.modbus_hook))(addr + 1, quantity, &reply_temp_ptr_16))
            {
                reply_data_ptr[0] = id_info.id;
                reply_data_ptr[1] = (U8)(quantity << 1);
                //Split U_16(register values) into two U_8(for replying)
                for(i = 0; i < reply_data_ptr[1]; i = i + 2)
                {
                    reply_data_ptr[i + 2] = (U8)(*(reply_temp_ptr_16) >> 8);
                    reply_data_ptr[i + 3] = (U8)(*reply_temp_ptr_16 & 0x00FF);
                    reply_temp_ptr_16++;
                }
                *reply_length_ptr = reply_data_ptr[1] + 2;  //2 because of "quantity" and "func code"
                return;
            }
            break;
        case READ_INPUT_REGISTERS:
            //request processing
            if(((ALHOOK_READ_INP_REG)(id_info.modbus_hook))(addr + 1, quantity, &reply_temp_ptr_16))
            {
                reply_data_ptr[0] = id_info.id;
                reply_data_ptr[1] = (U8)(quantity << 1);
                //split U_16(register values) into two U_8(for replying)
                for(i = 0; i < reply_data_ptr[1]; i = i + 2)
                {
                    reply_data_ptr[i + 2] = (U8)(*(reply_temp_ptr_16) >> 8);
                    reply_data_ptr[i + 3] = (U8)(*reply_temp_ptr_16 & 0x00FF);
                    reply_temp_ptr_16++;
                }
                *reply_length_ptr = reply_data_ptr[1] + 2; //2 because of "quantity" and "func code"
                return;
            }
            break;
        case WRITE_SINGLE_COIL:
            //check the output value:
            // 0xFF00 = ON and 0x0000 = OFF
            if(output_value == 0xFF00)
            {
                value = TRUE;
            }
            else
            {
                value = FALSE;
            }
            //request processing
            if(((ALHOOK_WR_SINGLE_COIL)(id_info.modbus_hook))(addr + 1, value))
            {
                data_ptr--;
                for(i = 0; i < length; i++)
                {
                    reply_data_ptr[i] = data_ptr[i];
                }
                *reply_length_ptr = length;
                return;
            }
            break;
        case WRITE_SINGLE_REGISTER:
            //request processing
            if(((ALHOOK_WR_SINGLE_HOLD_REG)(id_info.modbus_hook))(addr + 1, output_value))
            {
                for(i = 0; i < length; i++)
                {
                    reply_data_ptr[i] = data_ptr[i];
                }
                *reply_length_ptr = length;
                return;
            }
            break;
        case READ_EXCEPTION_STATUS:
            //request processing
            if(((ALHOOK_READ_EXC_STATUS)(id_info.modbus_hook))(&reply_data_ptr[1]))
            {
                reply_data_ptr[0] = id_info.id;
                *reply_length_ptr = 2;
                return;
            }
            break;
        case WRITE_MULTIPLE_COILS:
            //request processing
            if(((ALHOOK_WR_MULT_COIL)(id_info.modbus_hook))(addr + 1, quantity, &data_ptr[4]))
            {
                //The response returns the function code, starting address and quantity of coils forced
                data_ptr -= 2;
                for(i = 0; i < 5; i++)
                {
                    reply_data_ptr[i] = data_ptr[i];
                }
                *reply_length_ptr = 5;
                return;
            }
            break;
        case WRITE_MULTIPLE_REGISTERS:
            //Paste 2 "U_8" in a "U_16"
            i = 0;
            j = 4; //the place of the first data
            while(i < quantity)
            {
                output_value_ptr[i] = (data_ptr[j] << 8) + data_ptr[j + 1];
                i++;
                j += 2;
            }
            //request processing
            if(((ALHOOK_WR_MULT_HOLD_REG)(id_info.modbus_hook))(addr + 1, quantity, &output_value_ptr[0]))
            {
                //The response returns the function code, starting address and quantity of registers forced
                data_ptr -= 2;
                for(i = 0; i < 5; i++)
                {
                    reply_data_ptr[i] = data_ptr[i];
                }
                *reply_length_ptr = 5;
                return;
            }
            break;
        case READ_FILE_RECORD:
            reply_data_ptr[0] = id_info.id;
            reply_temp_ptr_8 = &reply_data_ptr[0];
            reply_data_ptr += 2;
            *reply_length_ptr = 0;
            for(i = byte_count; i > 0; i = i - 7)
            {
                file_number = (data_ptr[1] << 8) + data_ptr[2];
                rec_number = (data_ptr[3] << 8) + data_ptr[4];
                rec_length = (data_ptr[5] << 8) + data_ptr[6];
                data_ptr += 7;
                //request processing
                if(((ALHOOK_READ_FILE_REC)(id_info.modbus_hook))(file_number,
                                                                 rec_number,
                                                                 rec_length,
                                                                 &reply_data_ptr[2]))
                {
                    temp = rec_length * 2;
                    *reply_data_ptr = (U8)temp + 1;   //file resp. length
                    reply_data_ptr++;
                    *reply_data_ptr = 0x06;                   //ref. type
                    reply_data_ptr++;
                    *reply_length_ptr += (U8)temp + 2;
                    reply_data_ptr += temp;
                }
                else
                {
                    //MODBUS hook returned FALSE
                    reply_data_ptr = reply_temp_ptr_8;
                    *reply_data_ptr = id_info.id | MODBUS_ERROR_FUNCTIONCODE;
                    reply_data_ptr++;
                    *reply_data_ptr = MODBUS_EXCEPTION_FAILED_FUNCTION;
                    *reply_length_ptr = 2;
                    return;
                }
            }
            reply_temp_ptr_8[1] = *reply_length_ptr;
            *reply_length_ptr += 2;  //because of "function code" and "data length"
            return;
        case WRITE_FILE_RECORD:
            for(i = byte_count; i > 0; i -= length_temp + 7)
            {
                file_number = (data_ptr[1] << 8) + data_ptr[2];
                rec_number = (data_ptr[3] << 8) + data_ptr[4];
                rec_length = (data_ptr[5] << 8) + data_ptr[6];
                data_ptr += 7;

                //request processing
                if(((ALHOOK_WR_FILE_REC)(id_info.modbus_hook))(file_number,
                                                               rec_number,
                                                               rec_length,
                                                               &data_ptr[0]))
                {
                    length_temp = (U8)(rec_length * 2);
                    data_ptr += length_temp;
                }
                else
                {
                    //MODBUS hook returned FALSE
                    *reply_data_ptr = id_info.id | MODBUS_ERROR_FUNCTIONCODE;
                    reply_data_ptr++;
                    *reply_data_ptr = MODBUS_EXCEPTION_FAILED_FUNCTION;
                    *reply_length_ptr = 2;
                    return;
                }
            }
            data_ptr -= length;
            for(i = 0; i < length; i++)
            {
                reply_data_ptr[i] = data_ptr[i];
            }
            *reply_length_ptr = length;
            return;
        case ENCAP_INTERF_TRANSPORT:
            reply_data_ptr[0] = id_info.id;
            reply_data_ptr[1] = 0x0E; //MEI Type
            reply_data_ptr[2] = 0x01; //Read Device ID code
            reply_data_ptr[3] = 0x01; //Conformity level
            reply_data_ptr[4] = 0x00; //More Follows
            reply_data_ptr[5] = 0x00; //Next Object Id
            reply_data_ptr[6] = 0x03; //Number of objects
            reply_data_ptr += 6;
            *reply_length_ptr = 7;
            //request processing
            if(((ALHOOK_DEVICE_ID)(id_info.modbus_hook))(&al_modbus_reply_str[0]))
            {
                for(i = object_id; i < 3; i++)
                {
                    temp_str = al_modbus_reply_str[i];
                    temp = CoreString_GetLength(temp_str);
                    *reply_length_ptr += temp;

                    reply_data_ptr++;
                    *reply_data_ptr = i;
                    reply_data_ptr++;
                    *reply_data_ptr = temp;
                    *reply_length_ptr += 2;
                    for(j = 0; j < temp; j++)
                    {
                        reply_data_ptr++;
                        *reply_data_ptr = (U8)temp_str[j];
                    }
                }
                return;
            }
            break;
        default:
            if(((ALHOOK_SPECIFIC)(id_info.modbus_hook))(id_info.id,
                                                        ++data_ptr,
                                                        length - 1,
                                                        reply_data_ptr,
                                                        reply_length_ptr))
            {
                return;
            }
            break;
        }
        //MODBUS hook returned FALSE
        *reply_data_ptr = id_info.id | MODBUS_ERROR_FUNCTIONCODE;
        reply_data_ptr++;
        *reply_data_ptr = MODBUS_EXCEPTION_FAILED_FUNCTION;
        *reply_length_ptr = 2;
        return;
    }
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_Init(void)
{
    MODBUS_INTRFC* temp_ptr = al_modbus_intrfc;

    MODULE_INIT_ONCE();
    
    while(temp_ptr < &al_modbus_intrfc[MODBUS_CHANNEL_COUNT*MODBUS_INTRFC_COUNT])
    {
        temp_ptr->id          = (MODBUS_FUNCTIONCODE)0;
        temp_ptr->hook1       = NULL;
        temp_ptr->hook2       = NULL;
        temp_ptr->modbus_hook = NULL;
        temp_ptr++;
    }
    
    LOG_DBG("[MODBUS AL] initialised");
    
    MODULE_INIT_DONE();
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookReadCoils(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_COILS hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, READ_COILS, CommAlModbusSlave_CheckQuantityId1_2, CommAlModbusSlave_CheckRangeId, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookDiscrInp(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_DISCR_INP hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, READ_DISCRETE_INPUTS, CommAlModbusSlave_CheckQuantityId1_2, CommAlModbusSlave_CheckRangeId, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookReadHoldReg(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_HOLD_REG hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, READ_HOLDING_REGISTERS, CommAlModbusSlave_CheckQuantityId3_4, CommAlModbusSlave_CheckRangeId, (DUMMY_HOOK)hook);
}
 /*-------------------------------------------------------------------------------------------------------------------*/
void CommAlModbusSlave_InstallHookReadInpReg(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_INP_REG hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, READ_INPUT_REGISTERS, CommAlModbusSlave_CheckQuantityId3_4, CommAlModbusSlave_CheckRangeId, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookWrSingleCoils(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_SINGLE_COIL hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, WRITE_SINGLE_COIL, CommAlModbusSlave_CheckOutputValue, NULL, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookWrSingleHoldReg(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_SINGLE_HOLD_REG hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, WRITE_SINGLE_REGISTER, NULL, NULL, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookReadExcStatus(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_EXC_STATUS hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, READ_EXCEPTION_STATUS, NULL, NULL, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookWrMultCoils(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_MULT_COIL hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, WRITE_MULTIPLE_COILS, CommAlModbusSlave_CheckQuantityId15, CommAlModbusSlave_CheckRangeId, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookWrMultHoldReg(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_MULT_HOLD_REG hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, WRITE_MULTIPLE_REGISTERS, CommAlModbusSlave_CheckQuantityId16, CommAlModbusSlave_CheckRangeId, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookReadFileRec(MODBUS_CHANNEL modbus_channel, ALHOOK_READ_FILE_REC hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, READ_FILE_RECORD, CommAlModbusSlave_CheckByteCount, CommAlModbusSlave_CheckSubReq20, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookWrFileRec(MODBUS_CHANNEL modbus_channel, ALHOOK_WR_FILE_REC hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, WRITE_FILE_RECORD, CommAlModbusSlave_CheckByteCount, CommAlModbusSlave_CheckSubReq21, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_InstallHookDeviceId(MODBUS_CHANNEL modbus_channel, ALHOOK_DEVICE_ID hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, ENCAP_INTERF_TRANSPORT, CommAlModbusSlave_CheckObjId, CommAlModbusSlave_CheckDeviceIdCode, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
void CommAlModbusSlave_RegisterFunctionCode(MODBUS_CHANNEL modbus_channel, MODBUS_FUNCTIONCODE func_code, ALHOOK_SPECIFIC hook)
{
    CommAlModbusSlave_InstallHook(modbus_channel, func_code, NULL, NULL, (DUMMY_HOOK)hook);
}
//------------------------------------------------------------------------------------------------//
HOOK_REQUEST_RECEIVED CommAlModbusSlave_GetRequestHook(void)
{
    return CommAlModbusSlave_HandleData;
}
//================================================================================================//
