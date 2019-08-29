//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// code to control a hd44780 based alphanumeric lcd display
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define DISPLAY__DRVCHARDISPLAYHD44780_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
#ifndef DISPLAY__DRVCHARDISPLAYHD44780_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               DISPLAY__DRVCHARDISPLAYHD44780_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the tick period of the task tick in µs
#ifndef CHARDISPLAY_LINES
    #error "define  CHARDISPLAY_LINES"
#endif
//------------------------------------------------------------------------------------------------//
// @brief  Defines the tick period of the task tick in µs
#ifndef CHARDISPLAY_LINEWIDTH
    #error "define CHARDISPLAY_LINEWIDTH"
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

//APPLICATION include section
//STANDARD include section
//DRIVER include section
//SYSTEM include section

#include "display\DrvCharDisplayHd44780.h"
//================================================================================================//



//================================================================================================//
// L O C A L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
#define INSTRUCTION_CLEAR_DISPLAY           0x01
#define INSTRUCTION_RETURN_HOME             0x02
#define INSTRUCTION_ENTRY_MODE_SET          0x04
#define INSTRUCTION_DISPLAY_ON_OFF_CONTROL  0x08
#define INSTRUCTION_CURSOR_OR_DISPLAY_SHIFT 0x10
#define INSTRUCTION_FUNCTION_SET            0x20
#define INSTRUCTION_SET_CGRAM_ADDRESS       0x40
#define INSTRUCTION_SET_DDRAM_ADDRESS       0x80
//================================================================================================//



//================================================================================================//
// L O C A L   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
typedef enum
{
    INSTRUCTION_REGISTER,
    DATA_REGISTER,
}
HD44780_REGISTERS;
//================================================================================================//


//================================================================================================//
// L O C A L   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
void WriteRegister(HD44780_REGISTERS reg ,U8 data);
U8 ReadRegister(HD44780_REGISTERS reg);
void waitlcd(volatile unsigned int x);
static void DrvCharDisplayHd44780_CmdWriteTask(VPTR data_ptr);
void DrvCharDisplayHd44780_WriteScreen(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
static DRVGPIO_PIN_HNDL disp_D4;
static DRVGPIO_PIN_HNDL disp_D5;
static DRVGPIO_PIN_HNDL disp_D6;
static DRVGPIO_PIN_HNDL disp_D7;
static DRVGPIO_PIN_HNDL disp_RS;
static DRVGPIO_PIN_HNDL disp_E;    //aka clock
static DRVGPIO_PIN_HNDL disp_RW;
static U8*              ScreenBuffer;
static BOOL             ScreenBufferWriteToScreen;
static U8               ScreenBufferWriteToScreenPosition;
static DRV_CHAR_DISPLAY_STRUCT display_struct;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
static void DrvCharDisplayHd44780_CmdWriteTask(VPTR data_ptr)
{
    static BOOL Hd44780_InitDone = FALSE;
    static U8 InitState = 0;
    static BOOL ddramAddressNeedsToBeSet = TRUE;

    if (!Hd44780_InitDone)
    {
        switch(InitState)
        {
            case 0:
                //function set: set data length 4 bit, 2 lines, character font 5x8dots
                WriteRegister(INSTRUCTION_REGISTER, INSTRUCTION_FUNCTION_SET | 0x08);
                break;
            case 1:
                //display on/off control: set entire display on, cursor off, blinking cursor off
                WriteRegister(INSTRUCTION_REGISTER, INSTRUCTION_DISPLAY_ON_OFF_CONTROL | 0x04);
                break;
            case 2:
                //entry mode set: increment, display does not shift
                WriteRegister(INSTRUCTION_REGISTER, INSTRUCTION_ENTRY_MODE_SET | 0x02);
                break;
            case 3:
                //set DDRAM address to 0
                WriteRegister(INSTRUCTION_REGISTER,INSTRUCTION_SET_DDRAM_ADDRESS | 0);
                break;
            case 4:
                //clear display
                WriteRegister(INSTRUCTION_REGISTER, INSTRUCTION_CLEAR_DISPLAY); //this function takes 1.52ms so wait 1,6ms, is automaticly done by task
                break;
            case 5:  //extra wait for clear display instruction
                Hd44780_InitDone = TRUE;
                break;
        }
        InitState++;
    }
    else    //normal operation
    {
        if (ScreenBufferWriteToScreen)
        {
            if (ScreenBufferWriteToScreenPosition == 0 && ddramAddressNeedsToBeSet) //if at position 0 first the address needs to be set
            {
                //set DDRAM address to 0
                WriteRegister(INSTRUCTION_REGISTER,INSTRUCTION_SET_DDRAM_ADDRESS | 0);
                ddramAddressNeedsToBeSet = FALSE;
            }
            else if (ScreenBufferWriteToScreenPosition == CHARDISPLAY_LINEWIDTH & ddramAddressNeedsToBeSet) //if at position ? (seccond line) first the address needs to be set
            {
                //set DDRAM address to 40 (this is the next line)
                WriteRegister(INSTRUCTION_REGISTER,INSTRUCTION_SET_DDRAM_ADDRESS | 40);
                ddramAddressNeedsToBeSet = FALSE;
            }
            else //just write, the display will autoincrement the address
            {
                WriteRegister(DATA_REGISTER,(U8)*(ScreenBuffer+ScreenBufferWriteToScreenPosition));
                ScreenBufferWriteToScreenPosition++;//advance position
                ddramAddressNeedsToBeSet = TRUE; //flag that the address needs te be set when starting at new line

                if(ScreenBufferWriteToScreenPosition >= (CHARDISPLAY_LINEWIDTH* CHARDISPLAY_LINES))
                {
                    ScreenBufferWriteToScreen = FALSE; //done writing the buffer to the screen
                }
            }
        }
    }
}
//------------------------------------------------------------------------------------------------//
void WriteRegister(HD44780_REGISTERS reg ,U8 data)
{
    if (reg == INSTRUCTION_REGISTER)
    {
        DrvGpio_SetPin(disp_RS,FALSE);
    }
    else
    {
        DrvGpio_SetPin(disp_RS,TRUE);
    }

    DrvGpio_SetPin(disp_RW,FALSE);

    DrvGpio_SetPin(disp_E ,TRUE);   //clock high

    //high nible
    DrvGpio_SetPin(disp_D7,(BOOL)((data>>7)&1));
    DrvGpio_SetPin(disp_D6,(BOOL)((data>>6)&1));
    DrvGpio_SetPin(disp_D5,(BOOL)((data>>5)&1));
    DrvGpio_SetPin(disp_D4,(BOOL)((data>>4)&1));

    DrvGpio_SetPin(disp_E ,FALSE);  //clock low, display neemt signalen binnen op clock low flank
    //delay_100us_hook();//wacht tot ie het heeft kunnen inlezen

    //low nible
    DrvGpio_SetPin(disp_E ,TRUE);   //clock high

    DrvGpio_SetPin(disp_D7,(BOOL)((data>>3)&1));
    DrvGpio_SetPin(disp_D6,(BOOL)((data>>2)&1));
    DrvGpio_SetPin(disp_D5,(BOOL)((data>>1)&1));
    DrvGpio_SetPin(disp_D4,(BOOL)((data>>0)&1));

    DrvGpio_SetPin(disp_E ,FALSE);  //clock low, display neemt signalen binnen op clock low flank

    /*U8 i;
    for(i = 0; i < 10; i++)
    {
        delay_100us_hook(); //wacht de noodzakelijke 80µs
    }*/
}
//------------------------------------------------------------------------------------------------//
void DrvCharDisplayHd44780_WriteScreen(void)
{
    ScreenBufferWriteToScreenPosition = 0;
    ScreenBufferWriteToScreen = TRUE;
}
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
DRV_CHAR_DISPLAY_HNDL DrvCharDisplayHd44780_Init(   DRVGPIO_PIN_HNDL D4,
                                                    DRVGPIO_PIN_HNDL D5,
                                                    DRVGPIO_PIN_HNDL D6,
                                                    DRVGPIO_PIN_HNDL D7,
                                                    DRVGPIO_PIN_HNDL RS,
                                                    DRVGPIO_PIN_HNDL E,
                                                    DRVGPIO_PIN_HNDL RW)
{
    //create buffer
    ScreenBuffer = CoreBuffer_CreateStaticU8(CHARDISPLAY_LINES*CHARDISPLAY_LINEWIDTH,"lcd");
    MEMSET(ScreenBuffer,' ',CHARDISPLAY_LINES*CHARDISPLAY_LINEWIDTH); //init buffer with spaces

    //fill in gpio hndls
    disp_D4 = D4;
    disp_D5 = D5;
    disp_D6 = D6;
    disp_D7 = D7;
    disp_RS = RS;
    disp_E  = E ; //aka clock
    disp_RW = RW;

    //fill in display struct so drvchar can use it, for now only one fixed struct, this means you can only control one display with it
    display_struct.line_count = CHARDISPLAY_LINES;
    display_struct.line_width = CHARDISPLAY_LINEWIDTH;
    display_struct.buffer_ptr = ScreenBuffer;
    display_struct.write_buffer_to_screen_hook = DrvCharDisplayHd44780_WriteScreen;

    //set diplay now already to 4 bit mode
    WriteRegister(INSTRUCTION_REGISTER, INSTRUCTION_FUNCTION_SET | 0x08);

    //create task with 1 ms cycle, evry time this task runs a command will be written
    CoreTask_Start(CoreTask_RegisterTask(1000, DrvCharDisplayHd44780_CmdWriteTask, (VPTR)NULL, 200, "DrvCharDisplayHd44780_CmdWriteTask"));

    return &display_struct;
}
//------------------------------------------------------------------------------------------------//
/*void DelayLcd(void);
void DelayLcd(void)
{
    U8 i;
    for (i=0;i<=65;i++)
    {
        asm("nop;");
    }
}
void DrvCharDisplayHd44780_ProgramCustomCharPattern(U8 charAddress, U8* pattern)
{
    charAddress = charAddress-8;    //ik gebruik position 8-15 ip 0-7 want 0 = null terminatie en das is dus moelijk om in een string te zetten,
                                    //8-15 is gemirrored in het gebruik en is dus hetzelfde zoals 0-7 maar in het schrijven van het custom symbool is dat niet mogelijk dus daarom -8
    U8 i;
    for(i = 0; i < 8; i++)
    {
        WriteRegister(INSTRUCTION_REGISTER,INSTRUCTION_SET_CGRAM_ADDRESS | ((charAddress <<3)+i));
        DelayLcd();
        WriteRegister(DATA_REGISTER,*(pattern+i));
        DelayLcd();
    }
}*/
//================================================================================================//
