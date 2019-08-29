//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// generic interface to icon displays
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DISPLAY__DRVICONDISPLAYHT1621_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
#ifndef DISPLAY__DRVICONDISPLAYHT1621_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               DISPLAY__DRVICONDISPLAYHT1621_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
#ifndef DISPLAY_COUNT
    #define DISPLAY_COUNT               1
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//DRV include section
#include "display/DrvIconDisplayHt1621.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define HT1621_ID_WRITE             0x05 //0b101
#define HT1621_ID_COMMAND           0x04 //0b100

#define Ht1621_Select(x)            DrvGpio_SetPin(x->cs_hndl, FALSE)
#define Ht1621_Deselect(x)          DrvGpio_SetPin(x->cs_hndl, TRUE)
#define Ht1621_RdLow(x)             DrvGpio_SetPin(x->rd_hndl, FALSE)
#define Ht1621_RdHigh(x)            DrvGpio_SetPin(x->rd_hndl, TRUE)
#define Ht1621_WrLow(x)             DrvGpio_SetPin(x->wr_hndl, FALSE)
#define Ht1621_WrHigh(x)            DrvGpio_SetPin(x->wr_hndl, TRUE)
#define Ht1621_Data(x, y)           DrvGpio_SetPin(x->data_hndl, y)
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    HT1621_COMMAND_SYSTEM_ENABLE    = 0x01, //0b0000_0001
    HT1621_COMMAND_SYSTEM_DISABLE   = 0x00, //0b0000_0000
    HT1621_COMMAND_LCD_ON           = 0x03, //0b0000_0011
    HT1621_COMMAND_LCD_OFF          = 0x02, //0b0000_0010
    HT1621_COMMAND_12BIAS_2COMMON   = 0x20, //0b0010_0000
    HT1621_COMMAND_12BIAS_3COMMON   = 0x24, //0b0010_0100
    HT1621_COMMAND_12BIAS_4COMMON   = 0x28, //0b0010_1000
    HT1621_COMMAND_13BIAS_2COMMON   = 0x21, //0b0010_0001
    HT1621_COMMAND_13BIAS_3COMMON   = 0x25, //0b0010_0101
    HT1621_COMMAND_13BIAS_4COMMON   = 0x29, //0b0010_1001
}
HT1621_COMMANDS;

typedef struct
{
    U8                  bitstream[3];
    DRVGPIO_PIN_HNDL    cs_hndl;
    DRVGPIO_PIN_HNDL    rd_hndl;
    DRVGPIO_PIN_HNDL    wr_hndl;
    DRVGPIO_PIN_HNDL    data_hndl;
}
HT1621_CTRL_STRUCT;
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
static void Ht1621_WriteBitsMsbFirst(HT1621_CTRL_STRUCT* ctrl_struct_ptr, U8* data_ptr, U16 nb_of_bits);
static void Ht1621_WriteBitsLsbFirst(HT1621_CTRL_STRUCT* ctrl_struct_ptr, U8* data_ptr, U16 nb_of_bits);
static void Ht1621_SendCommand(HT1621_CTRL_STRUCT* ctrl_struct_ptr, HT1621_COMMANDS command);
static BOOL Ht1621_WriteData(ICON_DISPLAY_ID display_id, U16 address, U8* data_ptr, U16 size_in_bits);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static ICON_DISPLAY_HOOK_LIST       ht1621_display_hook_list;
static ICON_DISPLAY_STRUCT          ht1621_display_struct[DISPLAY_COUNT];
static HT1621_CTRL_STRUCT           ht1621_display_ctrl_struct[DISPLAY_COUNT];
static U8                           ht1621_display_count;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @brief   Low level function that puts a number of bits on the data line (MSB first !) and puts a clock on the write line.
static void Ht1621_WriteBitsMsbFirst(HT1621_CTRL_STRUCT* ctrl_struct_ptr, U8* data_ptr, U16 nb_of_bits)
{
    U16 cnt_8;
    U8  byte;

    LOG_DEV("WR MSB %02h (%d)", PU8A(data_ptr, (nb_of_bits + 7) >> 3), PU16(nb_of_bits));

    byte = *data_ptr;

    for(cnt_8 = 0; cnt_8 < nb_of_bits; cnt_8++)
    {
        Ht1621_WrLow(ctrl_struct_ptr);
        Ht1621_Data(ctrl_struct_ptr, (BOOL)((byte & 0x80) > 0));
        Ht1621_WrHigh(ctrl_struct_ptr);
        byte <<= 1;
        if((cnt_8 & 7) == 7)    //point to next byte
        {
            data_ptr++;
            byte = *data_ptr;
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void Ht1621_WriteBitsLsbFirst(HT1621_CTRL_STRUCT* ctrl_struct_ptr, U8* data_ptr, U16 nb_of_bits)
{
    U16 cnt_8;
    U8  byte;

    LOG_DEV("WR LSB %02h (%d)", PU8A(data_ptr, (nb_of_bits + 7) >> 3), PU16(nb_of_bits));

    byte = *data_ptr;

    for(cnt_8 = 0; cnt_8 < nb_of_bits; cnt_8++)
    {
        Ht1621_WrLow(ctrl_struct_ptr);
        Ht1621_Data(ctrl_struct_ptr, (BOOL)(byte & 0x01));
        Ht1621_WrHigh(ctrl_struct_ptr);
        byte >>= 1;
        if((cnt_8 & 7) == 7)    //point to next byte
        {
            data_ptr++;
            byte = *data_ptr;
        }
    }
}
//------------------------------------------------------------------------------------------------//
static void Ht1621_SendCommand(HT1621_CTRL_STRUCT* ctrl_struct_ptr, HT1621_COMMANDS command)
{
    ctrl_struct_ptr->bitstream[0] = (HT1621_ID_COMMAND << 5) | (command >> 3);
    ctrl_struct_ptr->bitstream[1] = (command << 5);

    Ht1621_Select(ctrl_struct_ptr);
    Ht1621_WriteBitsMsbFirst(ctrl_struct_ptr, ctrl_struct_ptr->bitstream, 12);   // 3 ID + 8 command + 1 don't care bit
    Ht1621_Deselect(ctrl_struct_ptr);
}
//------------------------------------------------------------------------------------------------//
static BOOL Ht1621_WriteData(ICON_DISPLAY_ID display_id, U16 address, U8* data_ptr, U16 size_in_bits)
{
    HT1621_CTRL_STRUCT* ctrl_struct_ptr = &ht1621_display_ctrl_struct[display_id];

    if((display_id < ht1621_display_count) && (address <= 0x3F))
    {
        LOG_DEV("Writing");
        ctrl_struct_ptr->bitstream[0] = (HT1621_ID_WRITE << 5) | (address >> 1);
        ctrl_struct_ptr->bitstream[1] = (address << 7);
        Ht1621_Select(ctrl_struct_ptr);
        Ht1621_WriteBitsMsbFirst(ctrl_struct_ptr, ctrl_struct_ptr->bitstream, 9);   // 3 ID + 6 address
        Ht1621_WriteBitsLsbFirst(ctrl_struct_ptr, data_ptr, size_in_bits);
        Ht1621_Deselect(ctrl_struct_ptr);
        return TRUE;
    }
    return FALSE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void DrvIconDisplayHt1621_Init()
{
    ht1621_display_hook_list.write_hook = Ht1621_WriteData;

    MEMSET((VPTR)ht1621_display_ctrl_struct, 0, SIZEOF(ht1621_display_ctrl_struct));
    MEMSET((VPTR)ht1621_display_struct, 0, SIZEOF(ht1621_display_struct));
    ht1621_display_count = 0;
}
//------------------------------------------------------------------------------------------------//
ICON_DISPLAY_HNDL DrvIconDisplayHt1621_Register(DRVGPIO_PIN_HNDL cs_hndl,
                                                DRVGPIO_PIN_HNDL rd_hndl,
                                                DRVGPIO_PIN_HNDL wr_hndl,
                                                DRVGPIO_PIN_HNDL data_hndl,
                                                HT1621_BIAS_SETTING bias_setting)
{
    ICON_DISPLAY_HNDL   display_hndl = &ht1621_display_struct[ht1621_display_count];
    HT1621_CTRL_STRUCT* ctrl_struct_ptr = &ht1621_display_ctrl_struct[ht1621_display_count];

    if(ht1621_display_count < DISPLAY_COUNT)
    {
        ctrl_struct_ptr->cs_hndl = cs_hndl;
        ctrl_struct_ptr->rd_hndl = rd_hndl;
        ctrl_struct_ptr->wr_hndl = wr_hndl;
        ctrl_struct_ptr->data_hndl = data_hndl;

        Ht1621_Deselect(ctrl_struct_ptr);
        Ht1621_RdHigh(ctrl_struct_ptr);
        Ht1621_WrHigh(ctrl_struct_ptr);
        Ht1621_Data(ctrl_struct_ptr, TRUE);

        Ht1621_SendCommand(ctrl_struct_ptr, HT1621_COMMAND_SYSTEM_ENABLE);
        Ht1621_SendCommand(ctrl_struct_ptr, (HT1621_COMMANDS) bias_setting);
        Ht1621_SendCommand(ctrl_struct_ptr, HT1621_COMMAND_LCD_ON);

        display_hndl->hook_list_ptr = &ht1621_display_hook_list;
        display_hndl->display_id = ht1621_display_count;
        ht1621_display_count++;
        return display_hndl;
    }
    return NULL;
}
//================================================================================================//
