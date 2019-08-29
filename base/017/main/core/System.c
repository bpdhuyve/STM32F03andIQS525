//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// CPU and System functionality
// This system module offers functionality for initialising CPU and other system parts.\n\n\n
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define CORE__SYSTEM_C
//================================================================================================//


//  This file configures the system clock as follows:
//  *=============================================================================
//  *=============================================================================
//  *        System Clock source                    | PLL (HSE)
//  *-----------------------------------------------------------------------------
//  *        SYSCLK(Hz)                             | 48000000
//  *-----------------------------------------------------------------------------
//  *        HCLK(Hz)                               | 48000000
//  *-----------------------------------------------------------------------------
//  *        AHB Prescaler                          | 1
//  *-----------------------------------------------------------------------------
//  *        APB Prescaler                          | 1
//  *-----------------------------------------------------------------------------
//  *        HSE Frequency(Hz)                      | 24000000
//  *----------------------------------------------------------------------------
//  *        PLLMUL                                 | 2
//  *-----------------------------------------------------------------------------
//  *        PREDIV                                 | 1
//  *-----------------------------------------------------------------------------
//  *        Flash Latency(WS)                      | 1
//  *-----------------------------------------------------------------------------
//  *        Prefetch Buffer                        | ON
//  *-----------------------------------------------------------------------------

//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the clock source used as system
//          see IS_RCC_SYSCLK_SOURCE() for valid options
#ifndef SYS_SCLK_SOURCE
#error "SYS_SCLK_SOURCE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This defines the PLL source
//          see IS_RCC_PLL_SOURCE() for valid options
// @remark  Only used when SYS_SCLK_SOURCE == RCC_SYSCLKSource_PLLCLK
#ifndef SYS_PLL_SOURCE
#error "SYS_PLL_SOURCE not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This defines the PLL predivider
//          See IS_RCC_PREDIV1() for the valid options
// @remark  Only used when SYS_SCLK_SOURCE == RCC_SYSCLKSource_PLLCLK
#ifndef SYS_PLL_PREDIV
#error "SYS_PLL_PREDIV not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This defines the PLL multiplier
//          See IS_RCC_PLL_MUL() for the valid options
// @remark  Only used when SYS_SCLK_SOURCE == RCC_SYSCLKSource_PLLCLK
#ifndef SYS_PLL_MUL
#error "SYS_PLL_MUL not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the oscilator frequency in HZ (4MHz - 16MHz)
// @remark  Only used when HSE is used
#ifndef SYS_OSC_CLOCK_IN_HZ
#error "SYS_OSC_CLOCK_IN_HZ not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the AHB clock.
//          see IS_RCC_HCLK() for valid options
#ifndef SYS_AHB_CONFIG
#error "SYS_AHB_CONFIG not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the low speed APB clock (PCLK1)
//          This clock is derived from the AHB clock (HCLK).
//          see IS_RCC_PCLK() for valid options
#ifndef SYS_PCLK_CONFIG
#error "SYS_PCLK_CONFIG not defined in AppConfig.h"
#endif
//------------------------------------------------------------------------------------------------//
// @brief   This define specifies the clock source used for the ADC
//          see IS_RCC_ADCCLK() for valid options
#ifndef SYS_ADC_CLK_SOURCE
#error "SYS_ADC_CLK_SOURCE not defined in AppConfig.h"
#endif

//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "core\SysLibAll.h"
#include "core\stm32f0xx_pwr.h"
#include "core\stm32f0xx_iwdg.h"
#include "core\stm32f0xx_flash.h"
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
static void System_InitDataSegments(void);
static void System_InitClocks(void);
//================================================================================================//



//================================================================================================//
// L O C A L   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
__no_init SYSTEM_SIGNATURES       system_signatures @ "ram_signature";
//================================================================================================//



//================================================================================================//
// L O C A L   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
// @brief   Device reset function
// should be static, but CANNOT be because of debugger/IAR adding extra (overhead) startupcode to binary
static void System_InitDataSegments(void)
{
    unsigned char *Src;
    unsigned char *Dest;
    U32 size;
    U32 i;

#pragma section = ".data_init"
#pragma section = ".data"
#pragma section = ".bss"
#pragma section = ".noinit"

    Src = __section_begin(".data_init");
    Dest = __section_begin(".data");
    size = __section_size(".data_init");

    //copy the initializers from ROM to RAM
    for(i = 0; i < size; i++)
    {
        *Dest++=*Src++;
    }

    Dest = __section_begin(".bss");
    size = __section_size(".bss");

    //zero init bss segment
    for(i = 0; i < size; i++)
    {
        *Dest++=0;
    }
}
//------------------------------------------------------------------------------------------------//
static void SetSysClock(void)
{
#if (SYS_USE_EXTERNAL_OSCILLATOR == 1)
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

/******************************************************************************/
/*            PLL (clocked by HSE) used as System clock source                */
/******************************************************************************/

  /* SYSCLK, HCLK, PCLK configuration ----------------------------------------*/
  /* Enable HSE */    
  RCC->CR |= ((uint32_t)RCC_CR_HSEON);

  /* Wait till HSE is ready and if Time out is reached exit */
  do
  {
    HSEStatus = RCC->CR & RCC_CR_HSERDY;
    StartUpCounter++;  
  } while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

  if ((RCC->CR & RCC_CR_HSERDY) != RESET)
  {
    HSEStatus = (uint32_t)0x01;
  }
  else
  {
    HSEStatus = (uint32_t)0x00;
  }  

  if (HSEStatus == (uint32_t)0x01)
  {
    /* Enable Prefetch Buffer and set Flash Latency */
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

     /* HCLK = SYSCLK / 1 */
     RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;
       
     /* PCLK = HCLK / 1 */
     RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;

    /* PLL configuration */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
        
#if SYS_OSC_CLOCK_IN_HZ==8000000
        
        RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLMULL6);
        
#elif SYS_OSC_CLOCK_IN_HZ==24000000
        
        RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_PREDIV1 | RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLMULL2);
        
#else

#error "Oscillator frequency not supported yet."

#endif
        
        /* Enable PLL */
        RCC->CR |= RCC_CR_PLLON;

        /* Wait till PLL is ready */
        while((RCC->CR & RCC_CR_PLLRDY) == 0)
        {
        }

        /* Select PLL as system clock source */
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;    

        /* Wait till PLL is used as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
        {
        }
    }
    else
    {
        /* If HSE fails to start-up, the application will have wrong clock
             configuration. User can add here some code to deal with this error */
    }
#else  // #if SYS_USE_EXTERNAL_OSCILLATOR == 1
    
    /******************************************************************************/
    /*            PLL (clocked by HSI) used as System clock source                */
    /******************************************************************************/

    /* At this stage the HSI is already enabled and used as System clock source */

    /* SYSCLK, HCLK, PCLK configuration ----------------------------------------*/

    /* Enable Prefetch Buffer and set Flash Latency */
    FLASH->ACR = FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY;

    /* HCLK = SYSCLK / 1 */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

    /* PCLK = HCLK / 1 */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE_DIV1;

    /* PLL configuration */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_Div2 | RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLMULL12);
            
    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till PLL is ready */
    while((RCC->CR & RCC_CR_PLLRDY) == 0)
    {
    }

    /* Select PLL as system clock source */
    RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;    

    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL)
    {
    }

#endif // #if SYS_USE_EXTERNAL_OSCILLATOR == 1
}
//------------------------------------------------------------------------------------------------//
static void System_InitClocks(void)
{
#if (SYS_USE_EXTERNAL_OSCILLATOR == 1)
    
  /* Set HSION bit */
  RCC->CR |= (uint32_t)0x00000001;

  /* Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE and MCOSEL[2:0] bits */
  RCC->CFGR &= (uint32_t)0xF8FFB80C;
  
  /* Reset HSEON, CSSON and PLLON bits */
  RCC->CR &= (uint32_t)0xFEF6FFFF;

  /* Reset HSEBYP bit */
  RCC->CR &= (uint32_t)0xFFFBFFFF;

  /* Reset PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
  RCC->CFGR &= (uint32_t)0xFFC0FFFF;

  /* Reset PREDIV1[3:0] bits */
  RCC->CFGR2 &= (uint32_t)0xFFFFFFF0;

  /* Reset USARTSW[1:0], I2CSW, CECSW and ADCSW bits */
  RCC->CFGR3 &= (uint32_t)0xFFFFFEAC;

  /* Reset HSI14 bit */
  RCC->CR2 &= (uint32_t)0xFFFFFFFE;

  /* Disable all interrupts */
  RCC->CIR = 0x00000000;
    
#else // #if SYS_USE_EXTERNAL_OSCILLATOR == 1
    
    /* Set HSION bit */
    RCC->CR |= (uint32_t)0x00000001;

    /* Reset SW[1:0], HPRE[3:0], PPRE[2:0], ADCPRE and MCOSEL[2:0] bits */
    RCC->CFGR &= (uint32_t)0xF8FFB80C;

    /* Reset HSEON, CSSON and PLLON bits */
    RCC->CR &= (uint32_t)0xFEF6FFFF;

    /* Reset HSEBYP bit */
    RCC->CR &= (uint32_t)0xFFFBFFFF;

    /* Reset PLLSRC, PLLXTPRE and PLLMUL[3:0] bits */
    RCC->CFGR &= (uint32_t)0xFFC0FFFF;

    /* Reset PREDIV1[3:0] bits */
    RCC->CFGR2 &= (uint32_t)0xFFFFFFF0;

    /* Reset USARTSW[1:0], I2CSW, CECSW and ADCSW bits */
    RCC->CFGR3 &= (uint32_t)0xFFFFFEAC;

    /* Reset HSI14 bit */
    RCC->CR2 &= (uint32_t)0xFFFFFFFE;

    /* Disable all interrupts */
    RCC->CIR = 0x00000000;

#endif  // #if SYS_USE_EXTERNAL_OSCILLATOR == 1
    
    /* Configure the System clock frequency, AHB/APBx prescalers and Flash settings */
    SetSysClock();
}
//================================================================================================//


//================================================================================================//
// E X P O R T E D   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
void System_Init(void)
{
    System_InitDataSegments();

    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    System_InitClocks();
}
//------------------------------------------------------------------------------------------------//
void System_EnableWatchdog(U32 overflow_time_in_ms)
{
    U32 reload_value    = overflow_time_in_ms * 10; // LSI 40kHz using prescaler 4
    U8  prescaler       = IWDG_Prescaler_4;

    while((reload_value > 0xFFF) && (prescaler < 6))
    {
        // reload valeu to high, adjust prescaler
        reload_value >>= 1;
        prescaler++;
    }
    // trim reload value to max if necessary
    if(reload_value > 0xFFF)
    {
        reload_value = 0xFFF;
    }

    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(prescaler);
    IWDG_SetReload(reload_value);
    IWDG_ReloadCounter();
    IWDG_Enable();
}
//------------------------------------------------------------------------------------------------//
void System_KickDog(void)
{
    IWDG_ReloadCounter();
}
//------------------------------------------------------------------------------------------------//
void System_Reset(void)
{
    NVIC_SystemReset();
}
//------------------------------------------------------------------------------------------------//
U32 SysGetAHBClk(void)
{
    U32 sys_clk;
    U32 ahb_clk;

    sys_clk = SysGetSysClk();

    if (SYS_AHB_CONFIG == RCC_SYSCLK_Div1)
    {
        ahb_clk = sys_clk;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div2)
    {
        ahb_clk = sys_clk / 2;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div4)
    {
        ahb_clk = sys_clk / 4;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div8)
    {
        ahb_clk = sys_clk / 8;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div16)
    {
        ahb_clk = sys_clk / 16;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div64)
    {
        ahb_clk = sys_clk / 64;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div128)
    {
        ahb_clk = sys_clk / 128;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div256)
    {
        ahb_clk = sys_clk / 256;
    }
    else if(SYS_AHB_CONFIG == RCC_SYSCLK_Div512)
    {
        ahb_clk = sys_clk / 512;
    }

    return(ahb_clk);
}
//------------------------------------------------------------------------------------------------//
U32 SysGetPreDivClk(void)
{
    if(SYS_PLL_PREDIV == RCC_PREDIV1_Div1)
    {
        return SYS_OSC_CLOCK_IN_HZ;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div2)
    {
        return SYS_OSC_CLOCK_IN_HZ / 2;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div3)
    {
        return SYS_OSC_CLOCK_IN_HZ / 3;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div4)
    {
        return SYS_OSC_CLOCK_IN_HZ / 4;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div5)
    {
        return SYS_OSC_CLOCK_IN_HZ / 5;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div6)
    {
        return SYS_OSC_CLOCK_IN_HZ / 6;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div7)
    {
        return SYS_OSC_CLOCK_IN_HZ / 7;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div8)
    {
        return SYS_OSC_CLOCK_IN_HZ / 8;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div9)
    {
        return SYS_OSC_CLOCK_IN_HZ / 9;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div10)
    {
        return SYS_OSC_CLOCK_IN_HZ / 10;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div11)
    {
        return SYS_OSC_CLOCK_IN_HZ / 11;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div12)
    {
        return SYS_OSC_CLOCK_IN_HZ / 12;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div13)
    {
        return SYS_OSC_CLOCK_IN_HZ / 13;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div14)
    {
        return SYS_OSC_CLOCK_IN_HZ / 14;
    }
    else if(SYS_PLL_PREDIV == RCC_PREDIV1_Div15)
    {
        return SYS_OSC_CLOCK_IN_HZ / 15;
    }
    else
    {
        return SYS_OSC_CLOCK_IN_HZ / 16;
    }
}
//------------------------------------------------------------------------------------------------//
U32 SysGetSysClk(void)
{
    U32 pll_src;
    U32 pll_clk;

    if (SYS_SCLK_SOURCE == RCC_SYSCLKSource_HSI)
    {
        return 8000000;
    }
    else if (SYS_SCLK_SOURCE == RCC_SYSCLKSource_HSE)
    {
        return SYS_OSC_CLOCK_IN_HZ;
    }
    else
    {
        if (SYS_PLL_SOURCE == RCC_PLLSource_HSI_Div2)
        {
            pll_src = 4000000;
        }
        else if (SYS_PLL_SOURCE == RCC_PLLSource_PREDIV1)
        {
            pll_src = SysGetPreDivClk();
        }

        if (SYS_PLL_MUL == RCC_PLLMul_2)
        {
            pll_clk = 2 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_3)
        {
            pll_clk = 3 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_4)
        {
            pll_clk = 4 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_5)
        {
            pll_clk = 5 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_6)
        {
            pll_clk = 6 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_7)
        {
            pll_clk = 7 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_8)
        {
            pll_clk = 8 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_9)
        {
            pll_clk = 9 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_10)
        {
            pll_clk = 10 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_11)
        {
            pll_clk = 11 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_12)
        {
            pll_clk = 12 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_13)
        {
            pll_clk = 13 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_14)
        {
            pll_clk = 14 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_15)
        {
            pll_clk = 15 * pll_src;
        }
        else if (SYS_PLL_MUL == RCC_PLLMul_16)
        {
            pll_clk = 16 * pll_src;
        }
    }
    return (pll_clk);
}
//------------------------------------------------------------------------------------------------//
U32 SysGetAPBClk(void)
{
    RCC_ClocksTypeDef clocks ;
    
    RCC_GetClocksFreq(&clocks) ;
    
    U32 sys_clk = clocks.SYSCLK_Frequency ;
    
    if(SYS_PCLK_CONFIG == RCC_HCLK_Div2)
    {
        return (sys_clk >> 1);
    }
    else if(SYS_PCLK_CONFIG == RCC_HCLK_Div4)
    {
        return (sys_clk >> 2);
    }
    else if(SYS_PCLK_CONFIG == RCC_HCLK_Div8)
    {
        return (sys_clk >> 3);
    }
    else if(SYS_PCLK_CONFIG == RCC_HCLK_Div16)
    {
        return (sys_clk >> 4);
    }
    else
    {
        return sys_clk ;
    }
}
//------------------------------------------------------------------------------------------------//
U32 SysGetTimClk(void)
{
    if(SYS_PCLK_CONFIG == RCC_HCLK_Div1)
    {
        return SysGetAPBClk();
    }
    else
    {
        return (SysGetAPBClk() << 1);
    }
}
//------------------------------------------------------------------------------------------------//
U32 SysGetHSEClk(void)
{
    return SYS_OSC_CLOCK_IN_HZ;
}
//------------------------------------------------------------------------------------------------//
void SysSetMCOClockSource(U8 clock_source)
{
    RCC_MCOConfig(clock_source, RCC_MCOPrescaler_1);
}
//------------------------------------------------------------------------------------------------//
void System_Stop(void)
{
    PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFE);

    System_InitClocks();
}
//================================================================================================//
