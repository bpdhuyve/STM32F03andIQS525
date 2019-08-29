//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Implementation of the flash manipulation
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#define RTC__SYSRTC_C
//================================================================================================//



//================================================================================================//
// V E R I F Y    C O N F I G U R A T I O N
//------------------------------------------------------------------------------------------------//
#include "AppConfig.h"
//------------------------------------------------------------------------------------------------//
// @brief  Defines the log level of the module
#ifndef MEM__SYSRTC_LOG_LEVEL
	#define CORELOG_LEVEL               LOG_LEVEL_NONE
#else
	#define CORELOG_LEVEL               MEM__SYSRTC_LOG_LEVEL
#endif
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//
#include "Core.h"

// SYS
#include "rtc\SysRtc.h"
#include "core\stm32f0xx_rtc.h"
#include "core\stm32f0xx_pwr.h"

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

static RTC_InitTypeDef  RTC_InitStruct;

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

void SysRtc_Init(void)
{
	/* Enable the PWR APB1 Clock Interface */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

	/* Allow access to BKP Domain */
	PWR_BackupAccessCmd(ENABLE);
  
	if (RTC_ReadBackupRegister(RTC_BKP_DR2) != 0xA5A5)
        {
		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		/* Allow access to RTC */
		PWR_BackupAccessCmd(ENABLE);
		/* Enable the LSE OSC */
		RCC_LSEConfig(RCC_LSE_ON);

		/* Wait till LSE is ready */  
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
		{
		}

		/* Select the RTC Clock Source */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

		/* Enable the RTC Clock */
		RCC_RTCCLKCmd(ENABLE);

		/* Wait for RTC APB registers synchronisation */
		RTC_WaitForSynchro();
		
		/* Allow access to BKP Domain */
		PWR_BackupAccessCmd(ENABLE);
	
		/* Write to the first RTC Backup Data Register */
		RTC_WriteBackupRegister(RTC_BKP_DR2,0xA5A5);
		
                RTC_StructInit(&RTC_InitStruct);  // select RTC_HourFormat_24
                RTC_Init(&RTC_InitStruct);

	}
	else
        {
	
		/* Enable the PWR clock */
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

		/* Allow access to RTC */
		PWR_BackupAccessCmd(ENABLE);

		/* Wait for RTC APB registers synchronisation */
		RTC_WaitForSynchro();
 
	}

}




BOOL SysRtc_SetTime(RTC_TIME* rtc_time_ptr)
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;

  
  RTC_TimeStruct.RTC_Hours = rtc_time_ptr->hour;
  RTC_TimeStruct.RTC_Minutes = rtc_time_ptr->minute;
  RTC_TimeStruct.RTC_Seconds = rtc_time_ptr->second;
  RTC_TimeStruct.RTC_H12 = RTC_H12_AM;   // 24 hour format

  RTC_DateStruct.RTC_Year = rtc_time_ptr->year;
  RTC_DateStruct.RTC_Month = rtc_time_ptr->month;
  RTC_DateStruct.RTC_Date = rtc_time_ptr->day_of_month;
  RTC_DateStruct.RTC_WeekDay = rtc_time_ptr->day_of_week + 1;  // RTC_TIME.DAY_OF_WEEK has range 0..6
  
  if (SUCCESS != RTC_SetTime(RTC_Format_BIN, &RTC_TimeStruct))
     return FALSE;
  
  if (SUCCESS != RTC_SetDate(RTC_Format_BIN, &RTC_DateStruct))
     return FALSE;
  return TRUE;
}


BOOL SysRtc_GetTime(RTC_TIME* rtc_time_ptr)
{
  RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;
  
  
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
  RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);

  rtc_time_ptr->hour = RTC_TimeStruct.RTC_Hours;
  rtc_time_ptr->minute = RTC_TimeStruct.RTC_Minutes;
  rtc_time_ptr->second = RTC_TimeStruct.RTC_Seconds;
  
  rtc_time_ptr->year = RTC_DateStruct.RTC_Year;
  rtc_time_ptr->month = (MONTH)RTC_DateStruct.RTC_Month;
  rtc_time_ptr->day_of_month = RTC_DateStruct.RTC_Date;
  rtc_time_ptr->day_of_week = (DAY_OF_WEEK)(RTC_DateStruct.RTC_WeekDay - 1);  // RTC_TIME.DAY_OF_WEEK has range 0..6
  
  return TRUE;
}

