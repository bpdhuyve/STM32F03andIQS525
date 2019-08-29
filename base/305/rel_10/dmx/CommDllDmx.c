//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Dmx Protocoll driver
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define COMM_DLL_DMX_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"

#ifndef DMX_LOG_LEVEL
	#define CORELOG_LEVEL                                   LOG_LEVEL_DEFAULT
#else
	#define CORELOG_LEVEL                                   SWIPECORE_LOG_LEVEL
#endif
//------------------------------------------------------------------------------------------------//
//queue
#define Q_PREFIX(postfix)                                   Dmx_##postfix
#ifndef Q_SIZE
    #define Q_SIZE                                          10
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"
#include "dmx\CommDllDmx.h"

//SYSTEM include section
#include "gpio\SysPin.h"

//DRIVER include section
#include "sci\DrvSciQTxTpl.h"

//STANDARD include section
//APPLICATION include section
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
SCI_CHANNEL_HNDL             channel_hndl;
DRVGPIO_PIN_HNDL             appmain_pin_TX;
DRVGPIO_PIN_HNDL             appmain_pin_RX;
SCI_CONFIG_STRUCT            testconfigrs;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void CommDllDmx_Init(SCI_CHANNEL_HNDL channel, DRVGPIO_PIN_HNDL dirpin)
{
    channel_hndl = channel;

    //Make sure your dirpin is configured as output
    DrvGpio_SetPin(dirpin, TRUE);//VAN PROCESSOR NAAR RS485 Logger
    Dmx_SciQTx_Create(channel_hndl);

    testconfigrs.data_length = SCI_DATA_LENGTH_8_BITS;
    testconfigrs.speed       = SCI_SPEED_250000_BPS;
    testconfigrs.parity      = SCI_PARITY_NONE;
    testconfigrs.stopbit     = SCI_STOPBIT_2;

    DrvSciChannel_Config(channel_hndl,&testconfigrs);
}
//------------------------------------------------------------------------------------------------//
static void DelayMs(U32 ms)
{
    U32 temp;
    while (ms>0)
    {
        temp = 26; //@120mhz this is 1 µs
        while (temp>0)
        {
            temp--;
        }
        ms--;
    }
}
//------------------------------------------------------------------------------------------------//
void CommDllDmx_SendDMXdata(U8 *data_ptr, U8 lenght)
{
    //send de break sequence die het dmx512 protocol gebruikt om het begin van hde 512 databytes aan te kondigen
    //	deze code werkt meestal (op sommige swipedemos niet ?)
    U8 breakarray[1] = {0};

    //change speed to fake the break and MAB by sending on an other baudrate
    testconfigrs.speed = SCI_SPEED_19200_BPS;
    DrvSciChannel_Config(channel_hndl,&testconfigrs);
    Dmx_SciQTx_Write(&breakarray[0],1);

    //change speed bakc to normal
    testconfigrs.speed = SCI_SPEED_250000_BPS;
    DrvSciChannel_Config(channel_hndl,&testconfigrs);

//dit werk bij swipestat maar je mag geen processor pin nummering gebruiken in een lib, dit stukje code zou moetengemaakt worden met tasks in de plaats

//    //config uart tx pin as gpio
//    //SysPin_InitPin(GPIO_PORT_1, 15,PIN_OUTPUT);
//    //SysGpio_SetPinMask(GPIO_PORT_1, 1<<15,FALSE);
//    SysPin_InitPin(GPIO_PORT_A, 9,PIN_OUTPUT);

//    //send 92ms break (line low)
//    SysGpio_SetPinMask(GPIO_PORT_A, 1<<9,FALSE);
//    DelayMs(92);
//
//    //send 12ms MAB (line high)
//    //SysGpio_SetPinMask(GPIO_PORT_1, 1<<15,TRUE);
//    SysGpio_SetPinMask(GPIO_PORT_A, 1<<9,TRUE);
//    DelayMs(12);

    //config pin back as uart tx
    //SysPin_InitPin(GPIO_PORT_1, 15,PIN_ALTERNAT_P1_15_U2_TXD);
    //SysPin_InitPin(GPIO_PORT_A, 9,PIN_ALTERNAT_SCI_TX);

    //Send the data
    Dmx_SciQTx_Write(data_ptr,lenght);
}
//================================================================================================//