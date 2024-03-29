//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// collection of all sorts of crc generating functions
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CRC__STDCRC_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef CRC__STDCRC_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               CRC__STDCRC_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//STANDARD lib include section
#include "crc\StdCrc.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// I M P O R T E D   V A R I A B L E S   A N D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
U16 StdCrcGenerateCrc16IBM(U8* data_ptr, U32 length)//also known as CRC-16-ANSI
{
    U16 crc_start = 0xFFFF;
    return StdCrcGenerateCrc16IBMPart(data_ptr, length, crc_start);
}
//------------------------------------------------------------------------------------------------//
U16 StdCrcGenerateCrc16IBMPart(U8* data_ptr, U32 length, U16 crc_start)
{
    //copy of modbus crc, "modubus crc" = "IBM crc" zie http://en.wikipedia.org/wiki/Cyclic_redundancy_check
    U16     crc = crc_start;
    U8      crc_move;
    U8      lsb;

    do
    {
        crc ^= (U16)(*data_ptr);

        crc_move = 0;

        do
        {
            lsb = (U8)(crc & 0x0001);
            crc >>= 1;
            crc_move++;

            if(lsb == 1)
            {
                crc ^= 0xA001;
            }
        }
        while(crc_move < 8);
        length--;
        data_ptr++;
    }
    while(length > 0);

    return crc;
}
//------------------------------------------------------------------------------------------------//
U16 StdCrcGenerateCrc16CCITT(U8* data_ptr, U32 length)
{
    U16     crc = 0x0000;
    U16     i = 0;
    
    do
    {
        crc = crc ^ ((U16)(*data_ptr) << 8);
        for(i = 0; i < 8; i++)
        {
            if(crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc <<= 1;
            }
        }
        length--;
        data_ptr++;
    }
    while(length > 0);
    
    return crc;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//
