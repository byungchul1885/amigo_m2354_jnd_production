#if !defined(__OS_WRAPPER_H__)
#define __OS_WRAPPER_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "amg_debug.h"
#include "FreeRTOS.h"
#include "event_groups.h"
#include "semphr.h"
#include "timers.h"

#if 1 /* bccho, ADD, 2023-07-20 */
extern volatile uint32_t tmr1_count;
#endif
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/
typedef uint32_t OS_STK;
typedef uint32_t OS_FLAGS;

#define OS_FLAG_GRP void
#define OS_SEMAPHORE void
#define OS_TMR void

#define OS_TMR_OPT_NONE 0u
#define OS_TMR_OPT_ONE_SHOT 1u
#define OS_TMR_OPT_PERIODIC 2u

#define OSIntEnter()                          \
    {                                         \
        OSIntNesting++;                       \
        OSIntCount++;                         \
        if (OSIntNesting == 1)                \
            OSIntProfPreTime = OS_TIME_GET(); \
    }
#define OSIntExit()                                                    \
    {                                                                  \
        OSIntNesting--;                                                \
        if (OSIntNesting == 0)                                         \
            OSIntProfRunTime += OS_TIME_GET() - OSIntProfPreTime;      \
        portEND_SWITCHING_ISR((OSIntNesting == 0) ? pdTRUE : pdFALSE); \
    }
#define OS_PrintStackUsage()
#define OS_WAIT_INFINITE 0xFFFFFFFF
#define OS_EVENT QueueHandle_t
#define OS_MUTEX QueueHandle_t
#define OS_MS_FOR_ONE_TICK 1
#define OS_MS2TICK(a) ((a) / OS_MS_FOR_ONE_TICK)
#define OS_SEC2TICK(a) (OS_MS2TICK(a) * 1000)
#define OS_TICK_FOR_SEC OS_MS2TICK(1000)
#define STACK_INIT_PATTERN 0xFDB97531

#define OS_TRUE 1
#define OS_FALSE 0

#define OS_NO_ERR 0u

#define OS_ERR_EVENT_TYPE 1u
#define OS_ERR_PEND_ISR 2u
#define OS_ERR_POST_NULL_PTR 3u
#define OS_ERR_PEVENT_NULL 4u
#define OS_ERR_POST_ISR 5u
#define OS_ERR_QUERY_ISR 6u
#define OS_ERR_INVALID_OPT 7u
#define OS_ERR_TASK_WAITING 8u
#define OS_ERR_PDATA_NULL 9u

#define OS_TIMEOUT 10u
#define OS_TASK_NOT_EXIST 11u
#define OS_ERR_EVENT_NAME_TOO_LONG 12u
#define OS_ERR_FLAG_NAME_TOO_LONG 13u
#define OS_ERR_TASK_NAME_TOO_LONG 14u
#define OS_ERR_PNAME_NULL 15u
#define OS_ERR_TASK_CREATE_ISR 16u
#define OS_ERR_PEND_LOCKED 17u

#define OS_MBOX_FULL 20u

#define OS_Q_FULL 30u
#define OS_Q_EMPTY 31u

#define OS_PRIO_EXIST 40u
#define OS_PRIO_ERR 41u
#define OS_PRIO_INVALID 42u

#define OS_SEM_OVF 50u

#define OS_TASK_DEL_ERR 60u
#define OS_TASK_DEL_IDLE 61u
#define OS_TASK_DEL_REQ 62u
#define OS_TASK_DEL_ISR 63u

#define OS_NO_MORE_TCB 70u

#define OS_TIME_NOT_DLY 80u
#define OS_TIME_INVALID_MINUTES 81u
#define OS_TIME_INVALID_SECONDS 82u
#define OS_TIME_INVALID_MILLI 83u
#define OS_TIME_ZERO_DLY 84u

#define OS_TASK_SUSPEND_PRIO 90u
#define OS_TASK_SUSPEND_IDLE 91u

#define OS_TASK_RESUME_PRIO 100u
#define OS_TASK_NOT_SUSPENDED 101u

#define OS_MEM_INVALID_PART 110u
#define OS_MEM_INVALID_BLKS 111u
#define OS_MEM_INVALID_SIZE 112u
#define OS_MEM_NO_FREE_BLKS 113u
#define OS_MEM_FULL 114u
#define OS_MEM_INVALID_PBLK 115u
#define OS_MEM_INVALID_PMEM 116u
#define OS_MEM_INVALID_PDATA 117u
#define OS_MEM_INVALID_ADDR 118u
#define OS_MEM_NAME_TOO_LONG 119u

#define OS_ERR_NOT_MUTEX_OWNER 120u

#define OS_TASK_OPT_ERR 130u

#define OS_ERR_DEL_ISR 140u
#define OS_ERR_CREATE_ISR 141u

#define OS_FLAG_INVALID_PGRP 150u
#define OS_FLAG_ERR_WAIT_TYPE 151u
#define OS_FLAG_ERR_NOT_RDY 152u
#define OS_FLAG_INVALID_OPT 153u
#define OS_FLAG_GRP_DEPLETED 154u

#define OS_ERR_PIP_LOWER 160u

#define OS_ERR_TMR_INVALID_DLY 170u
#define OS_ERR_TMR_INVALID_PERIOD 171u
#define OS_ERR_TMR_INVALID_OPT 172u
#define OS_ERR_TMR_INVALID_NAME 173u
#define OS_ERR_TMR_NON_AVAIL 174u
#define OS_ERR_TMR_INACTIVE 175u
#define OS_ERR_TMR_INVALID_DEST 176u
#define OS_ERR_TMR_INVALID_TYPE 177u
#define OS_ERR_TMR_INVALID 178u
#define OS_ERR_TMR_ISR 179u
#define OS_ERR_TMR_NAME_TOO_LONG 180u
#define OS_ERR_TMR_INVALID_STATE 181u
#define OS_ERR_TMR_STOPPED 182u
#define OS_ERR_TMR_NO_CALLBACK 183u

#define OS_FLAG_WAIT_CLR_ALL \
    0u /* Wait for ALL    the bits specified to be CLR (i.e. 0)   */
#define OS_FLAG_WAIT_CLR_AND 0u

#define OS_FLAG_WAIT_CLR_ANY \
    1u /* Wait for ANY of the bits specified to be CLR (i.e. 0)   */
#define OS_FLAG_WAIT_CLR_OR 1u

#define OS_FLAG_WAIT_SET_ALL \
    2u /* Wait for ALL    the bits specified to be SET (i.e. 1)   */
#define OS_FLAG_WAIT_SET_AND 2u

#define OS_FLAG_WAIT_SET_ANY \
    3u /* Wait for ANY of the bits specified to be SET (i.e. 1)   */
#define OS_FLAG_WAIT_SET_OR 3u

#define OS_FLAG_CONSUME \
    0x80u /* Consume the flags if condition(s) satisfied             */

#define OS_FLAG_CLR 0u
#define OS_FLAG_SET 1u

#define OS_TASK_OPT_STK_CHK 0u
#define OS_TASK_OPT_STK_CLR 0u

#define OS_DEL_NO_PEND 0u
#define OS_DEL_ALWAYS 1u

/*
******************************************************************************
*	MACRO
******************************************************************************
*/
#define OSTimeDly(delay) vTaskDelay(delay)
typedef void (*OS_TMR_CALLBACK)(TimerHandle_t xTimer);
#define OS_ENTER_CRITICAL portENTER_CRITICAL
#define OS_EXIT_CRITICAL portEXIT_CRITICAL
#define OS_TASK_NAME_SIZE 8
#if 0 /* bccho, TIMER, 2023-07-21 */
#define OS_TIME_GET32() TIM2->CNT
#else
#if 0 /* bccho, TIMER, 2023-12-14 */
#define OS_TIME_GET32() tmr1_count
#else
#define OS_TIME_GET32() xTaskGetTickCount()
#endif
#endif
#define OS_TIME_GET() OS_TIME_GET32()
#if 0 /* bccho, TIMER, 2023-12-14 */
#define TIMER_RESOLUTION_ONE_MS 1000
#else
#define TIMER_RESOLUTION_ONE_MS 1
#endif
#define MS2TIMER(a) ((a)*TIMER_RESOLUTION_ONE_MS)
#define TIMER2MS(a) ((a) / TIMER_RESOLUTION_ONE_MS)
#define TIMER2US(a) (a)
#define OS_YIELD()
/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/
typedef struct
{
    uint32_t OSNMsgs;
} OS_Q_DATA;
/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/
extern uint32_t OSIntNesting;
extern uint32_t OSIntCount;
extern uint32_t OSIntProfPreTime;
extern uint32_t OSIntProfRunTime;

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/

/*****************************************************************************
** Function name: OSTaskCreateExt
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
void OSTaskCreateExt(const char *name, void (*task)(void *p_arg), void *p_arg,
                     OS_STK *ptos, uint8_t prio, uint16_t id, OS_STK *pbos,
                     uint32_t stk_size, void *pext, uint16_t opt);

/*****************************************************************************
** Function name: OSTaskNameSet
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
void OSTaskNameSet(uint8_t prio, uint8_t *pname, uint8_t *err);

/*****************************************************************************
** Function name: OSFlagCreate
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
OS_FLAG_GRP *OSFlagCreate(OS_FLAGS flags, uint8_t *err);

/*****************************************************************************
** Function name: OSFlagPend
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
OS_FLAGS OSFlagPend(OS_FLAG_GRP *pgrp, OS_FLAGS flags, uint8_t wait_type,
                    uint16_t timeout, uint8_t *err);

/*****************************************************************************
** Function name: OSFlagPost
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
OS_FLAGS OSFlagPost(OS_FLAG_GRP *pgrp, OS_FLAGS flags, uint8_t opt,
                    uint8_t *err);

/*****************************************************************************
** Function name: OSTimeTick
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
void OSTimeTick(void);

/*****************************************************************************
** Function name: OSSemCreate
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
OS_SEMAPHORE *OSSemCreate(uint16_t cnt);

/*****************************************************************************
** Function name: OSSemDel
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
OS_SEMAPHORE *OSSemDel(OS_SEMAPHORE *pevent, uint8_t opt, uint8_t *err);

/*****************************************************************************
** Function name: OSSemPend
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
void OSSemPend(OS_SEMAPHORE *semaphore, uint16_t timeout, uint8_t *err);

/*****************************************************************************
** Function name: OSSemPost
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
uint8_t OSSemPost(OS_SEMAPHORE *semaphore);

/*****************************************************************************
** Function name: OSTmrCreate
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
OS_TMR *OSTmrCreate(int32_t dly, int32_t period, int8_t opt,
                    OS_TMR_CALLBACK callback, uint32_t callback_arg,
                    uint8_t *pname, uint8_t *perr);

/*****************************************************************************
** Function name: OSTmrStart
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
bool OSTmrStart(OS_TMR *ptmr, uint8_t *perr);

/*****************************************************************************
** Function name: OSTmrStop
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
bool OSTmrStop(OS_TMR *ptmr, int8_t opt, void *callback, uint8_t *perr);

/*****************************************************************************
** Function name: OSTmrDel
** Descriptions:
** input parameters:
**
**
**
** output parameters:
** Returned value:
*****************************************************************************/
bool OSTmrDel(OS_TMR *ptmr, uint8_t *perr);
bool OSTmrGetStatus(OS_TMR *p_tmr);
void OSTmrChangeProperty(OS_TMR *p_tmr, uint32_t period, uint32_t arg);
void OS_PrintCurrentTaskInfo(void);

OS_EVENT OSQCreate(void **start, uint16_t size);
void *OSQAccept(OS_EVENT pEvent);
void *OSQPend(OS_EVENT pEvent, uint16_t timeout, uint8_t *err);
uint8_t OSQPost(OS_EVENT pEvent, void *msg);
uint8_t OSQQuery(OS_EVENT pEvent, OS_Q_DATA *pdata);
OS_MUTEX OSMutexCreate(void);
uint8_t OSMutexPend(OS_MUTEX p_mutex, uint16_t timeout);
void OSMutexPost(OS_MUTEX p_mutex);

typedef struct
{
    uint32_t line;
} _TASK_TCB;

#define _TASK_BEGIN(__pTcb) \
    switch ((__pTcb)->line) \
    {                       \
    case 0:

#define _TASK_END(__pTcb) \
    }                     \
    (__pTcb)->line = 0;

#define _TASK_YIELD_CONTINUE(__pTcb) \
    (__pTcb)->line = __LINE__;       \
    continue;                        \
    case (__LINE__):                 \
        (__pTcb)->line = 0;

#define _TASK_YIELD(__pTcb)    \
    (__pTcb)->line = __LINE__; \
    return;                    \
    case (__LINE__):           \
        (__pTcb)->line = 0;

#define _TASK_DELAY_COND(__pTcb, ___cond) \
    (__pTcb)->line = __LINE__;            \
    case (__LINE__):                      \
        if (!(___cond))                   \
        {                                 \
            return;                       \
        }                                 \
        (__pTcb)->line = 0;

#define _TASK_DELAY_COND_CONTINUE(__pTcb, ___cond) \
    (__pTcb)->line = __LINE__;                     \
    case (__LINE__):                               \
        if (!(___cond))                            \
        {                                          \
            continue;                              \
        }                                          \
        (__pTcb)->line = 0;

#define _TASK_YIELD_GO_NEXT(__pTcb) \
    (__pTcb)->line = __LINE__;      \
    goto next;                      \
    case (__LINE__):                \
        (__pTcb)->line = 0;

#define BLOCK_DURATION_MS 1000

#define OS_CHECK_TASK_BLOCK(name, pre_time)                                 \
    {                                                                       \
        static uint32_t max_interval;                                       \
        uint32_t dt = TIMER2MS(OS_TIME_GET() - pre_time);                   \
        if (max_interval < dt)                                              \
        {                                                                   \
            max_interval = dt;                                              \
            DPRINTF(DBG_WARN, "%s Task maximum duration is updated %d\r\n", \
                    name, dt);                                              \
        }                                                                   \
        if (dt > BLOCK_DURATION_MS)                                         \
        {                                                                   \
            DPRINTF(DBG_ERR, "%s Task blocked during %dms\r\n", name, dt);  \
        }                                                                   \
    }

#define LOOP_TIMEOUT_MS(a)                   \
    uint32_t ___pre_time___ = OS_TIME_GET(); \
    while (OS_TIME_GET() - ___pre_time___ < MS2TIMER(a))

#endif /*__OS_WRAPPER_H__*/
