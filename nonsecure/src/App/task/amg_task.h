#if !defined(__AMG_TASK_H__)
#define __AMG_TASK_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "os_wrap.h"
#include "FreeRTOS.h"
#include "task.h"
#if 0 /* bccho, TIMER, 2023-07-15 */
#include "cmsis_os2.h"
#endif /* bccho */
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/
#define EVENT_MASK_NODE_INFO_SEND (1UL << 17)
#define EVENT_MASK_PROTO_RESET (1UL << 18)
#define EVENT_MASK_MSGQ_RCV (1UL << 19)
#define EVENT_MASK_TASKQ (1UL << 20)
#define EVENT_MASK_PIPE_RCV (1UL << 21)
#define EVENT_MASK_PRIMITIVE (1UL << 22)
#define EVENT_MASK_TIMER (1UL << 23)

/*
******************************************************************************
*	MACRO
******************************************************************************
*/

/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/
typedef enum
{
    TASK_DM,
    TASK_AMR,
    TASK_AMRDTX,
    TASK_METER,
    TASK_TASKQ,
    TASK_RX_TASKQ,
    TASK_MAX
} TASK_ID;

typedef enum
{
#if 0 /* bccho, TIMER, 2023-07-15 */
    TASK_PRI_AMR = ((osPriority_t)osPriorityLow),
#else
    TASK_PRI_AMR = 8,
#endif /* bccho */
    TASK_PRI_AMRDTX,
    TASK_PRI_METER,
    TASK_PRI_DM,
    TASK_PRI_TASKQ,
    TASK_PRI_RX_TASKQ,
    TASK_PRI_TIMER,
    TASK_PRI_MAX
} TASK_PRI;

typedef enum TIMER_CODE_TAG
{
    MAX_TIMER_CODE
} T_TIMER_CODE;

#endif /*__AMG_TASK_H__*/
