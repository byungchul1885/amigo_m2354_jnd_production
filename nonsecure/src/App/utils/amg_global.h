#if 0 /* bccho, UNUSED, 2023-07-15 */
#if !defined(__AMG_GLOBAL_H__)
#define __AMG_GLOBAL_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "kconf.h"

#define FEATURE_TIM_ENABLED

#if defined(METER_DEV_M)
#include "stm32l4xx_hal.h"
#else
#include "stm32f4xx.h"
#endif

#if defined(FreeRTOS)
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#endif

#include "amg_typedef.h"
#include "os_wrap.h"
#include "os_memory.h"
#include "amg_task_primitive.h"
#include "amg_uart.h"
#include "amg_debug.h"

#if defined(FEATURE_USE_MEMORY_POOL)
#include "amg_partition_memory.h"
#endif /* FEATURE_USE_MEMORY_POOL */

/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/

/*
******************************************************************************
*	MACRO
******************************************************************************
*/
/* Macro to enable all interrupts. */
#define EnableInterrupts asm("CPSIE  i")

/* Macro to disable all interrupts. */
#define DisableInterrupts asm("CPSID  i")

/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/

#endif /*__AMG_GLOBAL_H__*/

#endif /* bccho */