//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Type definitions for cross platform use
// This file is a processor specific implementation for the <em>fixed type definitions</em>
// and some basic macro definitions.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef SYSTYPES_H
#define SYSTYPES_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// NILL definition
#define NILL                 ((VPTR)0)

// NULL definition
#define NULL                 0

// Returns the number of bytes reserved for a variable
#define SIZEOF(x)                       sizeof(x)

// Mem copy function
#define MEMCPY(dest, src, size)         memcpy(dest, src, size)

// Mem set function
#define MEMSET(dest, val, size)         memset(dest, val, size)

// Peripheral counts
#define ISR_COUNT           31
#define ADC_MODULE_COUNT    1
#define ADC_CHANNEL_COUNT   19
#define CAN_CHANNEL_COUNT   1
#define I2C_CHANNEL_COUNT   2
#define SCI_CHANNEL_COUNT   6
#define SPI_CHANNEL_COUNT   2
#define	TIMER_COUNT         8
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
//new platform 2.0 defs
//unsigned defs
typedef unsigned char                   U8;
typedef unsigned short                  U16;
typedef unsigned long                   U32;
typedef unsigned long long   			U64;

//signed defs
typedef signed char                     S8;
typedef signed short                    S16;
typedef signed long                     S32;
typedef signed long long   				S64;

//floats
typedef float                           F32;
typedef double                          F64;

//specials
typedef enum
{
    FALSE = 0,
    TRUE  = 1
}
BOOL;
typedef char                			CHAR;
typedef char*                			STRING;
typedef U8*                             VPTR;

//ISR VECTOR
typedef IRQn_Type                       ISR_VECTOR;

//ADC
typedef enum
{
    ADC_MODULE_1    = 0,
}
ADC_MODULE;

typedef enum
{
    ADC_CHANNEL_0   = 0,
    ADC_CHANNEL_1   = 1,
    ADC_CHANNEL_2   = 2,
    ADC_CHANNEL_3   = 3,
    ADC_CHANNEL_4   = 4,
    ADC_CHANNEL_5   = 5,
    ADC_CHANNEL_6   = 6,
    ADC_CHANNEL_7   = 7,
    ADC_CHANNEL_8   = 8,
    ADC_CHANNEL_9   = 9,
    ADC_CHANNEL_10  = 10,
    ADC_CHANNEL_11  = 11,
    ADC_CHANNEL_12  = 12,
    ADC_CHANNEL_13  = 13,
    ADC_CHANNEL_14  = 14,
    ADC_CHANNEL_15  = 15,
    ADC_CHANNEL_16  = 16,
    ADC_CHANNEL_17  = 17,
    ADC_CHANNEL_18  = 18,
}
ADC_CHANNEL;

typedef enum
{
    ADC_1_5_CYCLES      = 0,
    ADC_7_5_CYCLES      = 1,
    ADC_13_5_CYCLES     = 2,
    ADC_28_5_CYCLES     = 3,
    ADC_41_5_CYCLES     = 4,
    ADC_55_5_CYCLES     = 5,
    ADC_71_5_CYCLES     = 6,
    ADC_239_5_CYCLES    = 7,
}
ADC_SAMPLE_TIME;

//CAN
typedef enum
{
    CAN_CHANNEL_1  = 0,
}
CAN_CHANNEL;

//GPIO
typedef enum
{
    GPIO_PORT_A     = 0,
    GPIO_PORT_B     = 1,
    GPIO_PORT_C     = 2,
    GPIO_PORT_D     = 3,
    GPIO_PORT_E     = 4,
    GPIO_PORT_F     = 5,
    GPIO_PORT_G     = 6,
}
GPIO_PORT;

typedef enum
{
    PIN_NO_FUNC                               = 0,            // 0xABCD : A   = AFR, B   = OSPEED, C   = PUPD << 2 | OTYPE, D   = MODE
    
    // INPUT
    PIN_INPUT                                 = 0x0000,
    PIN_INPUT_FLOATING                        = 0x0000,
    PIN_INPUT_PULL_UP                         = 0x0040,
    PIN_INPUT_PULL_DOWN                       = 0x0080,
    
    // OUTPUT
    PIN_OUTPUT                                = 0x0101,
    PIN_OUTPUT_PUSH_PULL                      = 0x0101,
    PIN_OUTPUT_PUSH_PULL_LOW_SPD              = 0x0001,
    PIN_OUTPUT_PUSH_PULL_MED_SPD              = 0x0101,
    PIN_OUTPUT_PUSH_PULL_HIGH_SPD             = 0x0301,
    PIN_OUTPUT_OPEN_DRAIN                     = 0x0111,
    PIN_OUTPUT_OPEN_DRAIN_LOW_SPD             = 0x0011,
    PIN_OUTPUT_OPEN_DRAIN_MED_SPD             = 0x0111,
    PIN_OUTPUT_OPEN_DRAIN_HIGH_SPD            = 0x0311,
    
    // ALTERNATIVE
    PIN_ALTERNAT_ANALOG                       = 0x0003,       // alternative function to couple to ADC
    
    PIN_ALTERNAT_FUNCTION_0                   = 0x0002,       // see datasheet p 30-31
    PIN_ALTERNAT_FUNCTION_1                   = 0x1002,
    PIN_ALTERNAT_FUNCTION_2                   = 0x2002,
    PIN_ALTERNAT_FUNCTION_3                   = 0x3002,
    PIN_ALTERNAT_FUNCTION_4                   = 0x4002,
    PIN_ALTERNAT_FUNCTION_5                   = 0x5002,
    PIN_ALTERNAT_FUNCTION_6                   = 0x6002,
    PIN_ALTERNAT_FUNCTION_7                   = 0x7002,
    
    PIN_ALTERNAT_TIMER_INPUT_FLOATING         = 0x0002,       // OR with alternative function which is pin dependent...
    PIN_ALTERNAT_TIMER_INPUT_PULL_UP          = 0x0042,
    PIN_ALTERNAT_TIMER_INPUT_PULL_DOWN        = 0x0082,
    PIN_ALTERNAT_TIMER_PUSH_PULL              = 0x0102,
    PIN_ALTERNAT_TIMER_PUSH_PULL_LOW_SPD      = 0x0002,
    PIN_ALTERNAT_TIMER_PUSH_PULL_MED_SPD      = 0x0102,
    PIN_ALTERNAT_TIMER_PUSH_PULL_HIGH_SPD     = 0x0302,
    PIN_ALTERNAT_TIMER_OPEN_DRAIN             = 0x0112,
    PIN_ALTERNAT_TIMER_OPEN_DRAIN_LOW_SPD     = 0x0012,
    PIN_ALTERNAT_TIMER_OPEN_DRAIN_MED_SPD     = 0x0112,
    PIN_ALTERNAT_TIMER_OPEN_DRAIN_HIGH_SPD    = 0x0312,
    
    PIN_ALTERNAT_I2C_SCL_ON_                  = 0x0312,       // OR with alternative function which is pin dependent...
    PIN_ALTERNAT_I2C_SDA_ON_                  = 0x0312,
    
    PIN_ALTERNAT_SPI_NSS_ON_                  = 0x0302,       // OR with alternative function which is pin dependent...
    PIN_ALTERNAT_SPI_SCLK_ON_                 = 0x0302,
    PIN_ALTERNAT_SPI_MISO_ON_                 = 0x0302,
    PIN_ALTERNAT_SPI_MOSI_ON_                 = 0x0302,
    
    PIN_ALTERNAT_SPI_MISO_PULL_UP             = 0x0042,       // OR with alternative function which is pin dependent...
    PIN_ALTERNAT_SPI_MOSI_PULL_UP             = 0x0342,
    
    PIN_ALTERNAT_SCI_TX_ON_                   = 0x0302,       // OR with alternative function which is pin dependent...
    PIN_ALTERNAT_SCI_RX_ON_                   = 0x0042,
    
    PIN_ALTERNAT_CAN_TX_ON_                   = 0x0302,       // OR with alternative function which is pin dependent...
    PIN_ALTERNAT_CAN_RX_ON_                   = 0x0042,
}
SYS_PIN_FUNC;

// defines for backward compatibility
#define PIN_ALTERNAT_I2C_SCL                    (PIN_ALTERNAT_I2C_SCL_ON_|PIN_ALTERNAT_FUNCTION_1)
#define PIN_ALTERNAT_I2C_SDA                    (PIN_ALTERNAT_I2C_SDA_ON_|PIN_ALTERNAT_FUNCTION_1)
#define PIN_ALTERNAT_I2C_SCL_PORTA              (PIN_ALTERNAT_I2C_SCL_ON_|PIN_ALTERNAT_FUNCTION_4)
#define PIN_ALTERNAT_I2C_SDA_PORTA              (PIN_ALTERNAT_I2C_SDA_ON_|PIN_ALTERNAT_FUNCTION_4)
#define PIN_ALTERNAT_SPI_SCLK                   (PIN_ALTERNAT_SPI_SCLK_ON_|PIN_ALTERNAT_FUNCTION_0)
#define PIN_ALTERNAT_SPI_MISO                   (PIN_ALTERNAT_SPI_MISO_ON_|PIN_ALTERNAT_FUNCTION_0)
#define PIN_ALTERNAT_SPI_MOSI                   (PIN_ALTERNAT_SPI_MOSI_ON_|PIN_ALTERNAT_FUNCTION_0)
#define PIN_ALTERNAT_SCI_TX                     (PIN_ALTERNAT_SCI_TX_ON_|PIN_ALTERNAT_FUNCTION_1)
#define PIN_ALTERNAT_SCI_RX                     (PIN_ALTERNAT_SCI_RX_ON_|PIN_ALTERNAT_FUNCTION_1)
#define PIN_ALTERNAT_SCI_TX_PORTB               (PIN_ALTERNAT_SCI_TX_ON_|PIN_ALTERNAT_FUNCTION_0)
#define PIN_ALTERNAT_SCI_RX_PORTB               (PIN_ALTERNAT_SCI_RX_ON_|PIN_ALTERNAT_FUNCTION_0)

//I2C
typedef enum
{
    I2C_CHANNEL_1   = 0,
    I2C_CHANNEL_2   = 1,
}
I2C_CHANNEL;

//SCI
typedef enum
{
    SCI_CHANNEL_USART1  = 0,
    SCI_CHANNEL_USART2  = 1,
    SCI_CHANNEL_USART3  = 2,
    SCI_CHANNEL_USART4  = 3,
    SCI_CHANNEL_USART5  = 4,
    SCI_CHANNEL_USART6  = 5,
}
SCI_CHANNEL;

//SPI
typedef enum
{
    SPI_CHANNEL_1   = 0,
    SPI_CHANNEL_2   = 1,
}
SPI_CHANNEL;

//TIMER
typedef enum
{
    TIMER_1			= 0,
    TIMER_2			= 1,
    TIMER_3			= 2,
    TIMER_6			= 3,
    TIMER_14		= 4,
    TIMER_15		= 5,
    TIMER_16		= 6,
    TIMER_17		= 7,
}
TIMER;

typedef enum
{
    COMPARE_1       = 0,
    COMPARE_2       = 1,
    COMPARE_3       = 2,
    COMPARE_4       = 3,
}
COMPARE_NUMBER;

typedef enum
{
    PWM_CHANNEL_1   = 0,
    PWM_CHANNEL_2   = 1,
    PWM_CHANNEL_3   = 2,
    PWM_CHANNEL_4   = 3,
    PWM_CHANNEL_1N  = 4,
    PWM_CHANNEL_2N  = 5,
    PWM_CHANNEL_3N  = 6,
}
PWM_CHANNEL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
extern void memcpy(VPTR dest_ptr, VPTR source_ptr, U16 size);
extern void memset(VPTR dest_ptr, U16 value, U16 size);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* SYSTYPES_H */
