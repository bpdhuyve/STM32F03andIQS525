//================================================================================================//
// M O D U L E   H E A D E R
//------------------------------------------------------------------------------------------------//
// Module for handling tasks on a fixed tick period.
//
// Copyright (c), PsiControl Mechatronics NV, All rights reserved.
//================================================================================================//
#ifndef CORETASK_H
#define CORETASK_H
//================================================================================================//



//================================================================================================//
// I N C L U D E S
//------------------------------------------------------------------------------------------------//

//================================================================================================//



//================================================================================================//
// E X P O R T E D   S Y M B O L   D E F I N I T I O N S   A N D   M A C R O S
//------------------------------------------------------------------------------------------------//
// @remark  Definition of an invalid task handle
#define INVALID_TASK_HNDL       255
//================================================================================================//



//================================================================================================//
// E X P O R T E D   T Y P E D E F S
//------------------------------------------------------------------------------------------------//
// @remark  Hook prototype for the task control function
typedef void (*TASK_CALLBACK_FUNC)(VPTR data_ptr);
//------------------------------------------------------------------------------------------------//
// @remark  Task handle type definition
typedef U8              TASK_HNDL;
//================================================================================================//



//================================================================================================//
// E X P O R T E D   V A R I A B L E S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



//================================================================================================//
// E X P O R T E D   F U N C T I O N   P R O T O T Y P E S
//------------------------------------------------------------------------------------------------//
// @remark  none
void CoreTask_Init(void);
//------------------------------------------------------------------------------------------------//
// @remark  Hhandles tasks which don't need to be handled directly from the \ref CoreTask_Tick()
void CoreTask_Handler(void);
//------------------------------------------------------------------------------------------------//
// @remark  If the priority is greater than 127 then the TASK is handled by the background handler.
//          Otherwise the TASK is handled directly on the timer interrupt.
//          When the registration fails, an invalid task handle is returned and a warning is thrown
//          When using tasks with a background priority, CoreTask_Handler MUST be called from the background loop
TASK_HNDL CoreTask_RegisterTask(U32 task_period_in_us, TASK_CALLBACK_FUNC callback, VPTR data_ptr, U8 priority, STRING name);
//------------------------------------------------------------------------------------------------//
// @remark  Function to change the period of the specified task
//          The internal down-counter for this task is NOT altered.  If the task is running, then this new period will
//          become active on the next task period match.
BOOL CoreTask_SetPeriod(TASK_HNDL task, U32 task_period_in_us);
U32  CoreTask_GetPeriod(TASK_HNDL task);
//------------------------------------------------------------------------------------------------//
// @remark  Function to change the data_ptr of a specified task
//          The internal down-counter for this task is NOT altered.  If the task is running, then the new data pointer
//          will be used on the next call.
BOOL CoreTask_SetDataPtr(TASK_HNDL task, VPTR data_ptr);
//------------------------------------------------------------------------------------------------//
// @remark  Starts an existing task
//          The internal down-counter for this task is reset to the task period
BOOL CoreTask_Start(TASK_HNDL task);
//------------------------------------------------------------------------------------------------//
// @remark  Stops an existing task
BOOL CoreTask_Stop(TASK_HNDL task);
//------------------------------------------------------------------------------------------------//
// @remark  Stops and unregisters an existing task
BOOL CoreTask_Kill(TASK_HNDL task);
//------------------------------------------------------------------------------------------------//
// @remark  none
BOOL CoreTask_IsTaskRunning(TASK_HNDL task);
//------------------------------------------------------------------------------------------------//
// @remark  Start the scheduler (both interrupt and timer)
BOOL CoreTask_StartAll(void);
//------------------------------------------------------------------------------------------------//
// @remark  Stop the scheduler (both interrupt and timer)
BOOL CoreTask_StopAll(void);
//------------------------------------------------------------------------------------------------//
// @remark  Prints information on the tasks
void CoreTask_Info(void);
//------------------------------------------------------------------------------------------------//
// @remark  resets the counter of the task
BOOL CoreTask_ReStart(TASK_HNDL task);
//------------------------------------------------------------------------------------------------//
// @remark  returns the tick counter. This value is indicative, lost ticks will (most likely) not be recovered
U32 CoreTask_GetTickCount(void);
//================================================================================================//



//================================================================================================//
// E X P O R T E D   S T A T I C   I N L I N E   F U N C T I O N S
//------------------------------------------------------------------------------------------------//
//================================================================================================//



#endif /* CORETASK_H */
