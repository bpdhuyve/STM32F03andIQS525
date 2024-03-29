//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// code to control a hd44780 based alphanumeric lcd display
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef DISPLAY__DRVCHARDISPLAYHD44780_H
#define DISPLAY__DRVCHARDISPLAYHD44780_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "display\DrvCharDisplay.h"
#include "gpio\DrvGpio.h"
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//@brief   User defined function to delay. 100�s
typedef void (*DRVCHAR_DISPLAYHD44780_DELAY_FUNCTION) (void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
DRV_CHAR_DISPLAY_HNDL DrvCharDisplayHd44780_Init(   DRVGPIO_PIN_HNDL D4,
                                                    DRVGPIO_PIN_HNDL D5,
                                                    DRVGPIO_PIN_HNDL D6,
                                                    DRVGPIO_PIN_HNDL D7,
                                                    DRVGPIO_PIN_HNDL RS,
                                                    DRVGPIO_PIN_HNDL E,
                                                    DRVGPIO_PIN_HNDL RW);

//charAddress: range from 8 to 15, then you can display these chars by sending this number to the display
//U8*: pointer to arry of 8 x byte were the 5 lsb bits of each byte represent 1 line in the character
//example pointer to "U8 customCharSquareEmpty[8] = {0x1F,0x11,0x11,0x11,0x11,0x11,0x11,0x1F};"
//void DrvCharDisplayHd44780_ProgramCustomCharPattern(U8 charAddress, U8* pattern);
//================================================================================================//



#endif /* DISPLAY__DRVCHARDISPLAYHD44780_H */
