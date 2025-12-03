/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/

#include "main.h"
#include "os_wrap.h"
#include "amg_debug.h"
#include "amg_shell.h"
#include "amg_ansi.h"

#define GET_IRQ_MODE() (__get_IPSR() != 0U)

uint32_t OSIntNesting = 0;
uint32_t OSIntCount = 0;
uint32_t OSIntProfPreTime;
uint32_t OSIntProfRunTime = 0;

/* bccho, 2023-07-10. 사용하지 않음 */
void OS_PrintCurrentTaskInfo(void) {}

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
                     uint32_t stk_size, void *pext, uint16_t opt)
{
    BaseType_t result;

    /* remove warning */
    UNUSED(ptos);
    UNUSED(pbos);
    UNUSED(id);
    UNUSED(opt);
    UNUSED(name);

    result = xTaskCreate(task, name, stk_size, p_arg, prio, NULL);

    if (result != pdPASS)
    {
        ASSERT(0);
    }
}

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

/* bccho, 2023-07-10. 사용하지 않음 */
void OSTaskNameSet(uint8_t prio, uint8_t *pname, uint8_t *err)
{
    /* remove warning */
    UNUSED(prio);
    UNUSED(pname);
    UNUSED(err);
}

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
OS_FLAG_GRP *OSFlagCreate(OS_FLAGS flags, uint8_t *err)
{
    UNUSED(flags);
    UNUSED(err);

    return xEventGroupCreate();
}

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
                    uint16_t timeout, uint8_t *err)
{
    EventBits_t uxBits;
    TickType_t ltimeout;

    UNUSED(wait_type);
    UNUSED(err);

    if (timeout == 0)
    {
        ltimeout = portMAX_DELAY;
    }
    else
    {
        ltimeout = (TickType_t)timeout;
    }

    uxBits = xEventGroupWaitBits(pgrp, flags, pdTRUE, pdFALSE, ltimeout);

    return (OS_FLAGS)uxBits;
}

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
                    uint8_t *err)
{
    BaseType_t result, taskWoken = pdFALSE;

    UNUSED(opt);
    UNUSED(err);

    /* in interrupt */
    if (GET_IRQ_MODE())
    {
        result = xEventGroupSetBitsFromISR(pgrp, flags, &taskWoken);
        if (result == pdPASS)
            portEND_SWITCHING_ISR(taskWoken);
    }
    /* in normal */
    else
    {
        result = xEventGroupSetBits(pgrp, flags);
    }

    return (OS_FLAGS)result;
}

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
/* bccho, 2023-07-10. 사용하지 않음 */
void OSTimeTick(void) {}

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

/* bccho, 2023-07-10. 사용하지 않음 */
OS_SEMAPHORE *OSSemCreate(uint16_t cnt)
{
    xSemaphoreHandle xSemaphore;

    vSemaphoreCreateBinary(xSemaphore);

    if (xSemaphore == NULL)
    {
        DPRINTF(DBG_ERR, "OSSemCreate Error");
    }

    if (cnt == 0)  // Means it can't be taken
    {
        xSemaphoreTake(xSemaphore, 1);
    }

    return xSemaphore;
}

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

/* bccho, 2023-07-10. 사용하지 않음 */
OS_SEMAPHORE *OSSemDel(OS_SEMAPHORE *pevent, uint8_t opt, uint8_t *err)
{
    UNUSED(opt);
    UNUSED(err);

    vSemaphoreDelete(pevent);

    return NULL;
}

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

/* bccho, 2023-07-10. 사용하지 않음 */
void OSSemPend(OS_SEMAPHORE *semaphore, uint16_t timeout, uint8_t *err)
{
    int status;

    status = xSemaphoreTake(semaphore, timeout);

    if (status != pdTRUE)
    {
        DPRINTF(DBG_ERR, "OSSemPend Error\r\n");
        *err = OS_ERR_EVENT_TYPE;
        return;
    }

    *err = OS_NO_ERR;
}

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

/* bccho, 2023-07-10. 사용하지 않음 */
uint8_t OSSemPost(OS_SEMAPHORE *semaphore)
{
    int status;

    status = xSemaphoreGive(semaphore);

    if (status != pdTRUE)
    {
        DPRINTF(DBG_ERR, "OSSemPost Error\r\n");
        return OS_ERR_EVENT_TYPE;
    }

    return OS_NO_ERR;
}

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

// clang-format off
OS_TMR *OSTmrCreate(
    int32_t dly, 
    int32_t period, 
    int8_t opt,
    OS_TMR_CALLBACK callback, 
    uint32_t callback_arg,
    uint8_t *pname, 
    uint8_t *perr)
{
    OS_TMR *ptmr;

    UNUSED(dly);
    UNUSED(callback_arg);

    ptmr = xTimerCreate(
        (char *)pname,                                  /* pcTimerName */
        period,                                         /* xTimerPeriodInTicks */
        (opt == OS_TMR_OPT_PERIODIC) ? pdTRUE : pdFALSE,/* uxAutoReload */
        (void *)callback_arg,                           /* pvTimerID */
        callback                                        /* pxCallbackFunction */
    );

    *perr = OS_NO_ERR;

    return ptmr;
}
// clang-format on

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
bool OSTmrStart(OS_TMR *ptmr, uint8_t *perr)
{
    xTimerStart(ptmr, (portTickType)0);

    *perr = OS_NO_ERR;

    return TRUE;
}

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
bool OSTmrStop(OS_TMR *ptmr, int8_t opt, void *callback, uint8_t *perr)
{
    UNUSED(opt);

    xTimerStop(ptmr, (portTickType)0);

    *perr = OS_NO_ERR;

    return TRUE;
}

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

/* bccho, 2023-07-10. 사용하지 않음 */
bool OSTmrDel(OS_TMR *ptmr, uint8_t *perr)
{
    xTimerDelete(ptmr, (portTickType)0);

    *perr = OS_NO_ERR;

    return TRUE;
}

/* bccho, 2023-07-10. 사용하지 않음 */
bool OSTmrGetStatus(OS_TMR *p_tmr) { return (bool)xTimerIsTimerActive(p_tmr); }

/* bccho, 2023-07-10. 사용하지 않음 */
void OSTmrChangeProperty(OS_TMR *p_tmr, uint32_t period, uint32_t arg)
{
    xTimerChangePeriod(p_tmr, period, 100);
    vTimerSetTimerID(p_tmr, (void *)arg);
}

/* bccho, 2023-07-10. 사용하지 않음 */
OS_EVENT OSQCreate(void **start, uint16_t size)
{
    return xQueueCreate(size, sizeof(void *));
}

/* bccho, 2023-07-10. 사용하지 않음 */
void *OSQAccept(OS_EVENT pEvent)
{
    void *msg = NULL;

    if (!xQueueReceive(pEvent, &msg, 0))
    {
        msg = NULL;
    }

    return msg;
}

/* bccho, 2023-07-10. 사용하지 않음 */
void *OSQPend(OS_EVENT pEvent, uint16_t timeout, uint8_t *err)
{
    void *msg = NULL;

    if (xQueueReceive(pEvent, &msg, timeout))
    {
        *err = OS_TIMEOUT;
    }
    else

    {
        *err = OS_NO_ERR;
    }
    return msg;
}

/* bccho, 2023-07-10. 사용하지 않음 */
uint8_t OSQPost(OS_EVENT pEvent, void *msg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (uxQueueSpacesAvailable(pEvent))
    {
        if (GET_IRQ_MODE())
        {
            BaseType_t pass = xQueueSendFromISR(pEvent, (const void *)&msg,
                                                &xHigherPriorityTaskWoken);
            portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
            return (pass == pdPASS) ? OS_NO_ERR : OS_Q_FULL;
        }
        else if (xQueueSend(pEvent, (const void *)&msg, 0))
        {
            return OS_NO_ERR;
        }
    }
    return OS_Q_FULL;
}

/* bccho, 2023-07-10. 사용하지 않음 */
uint8_t OSQQuery(OS_EVENT pEvent, OS_Q_DATA *pdata)
{
    pdata->OSNMsgs = uxQueueMessagesWaiting(pEvent);
    return OS_NO_ERR;
}

/* bccho, 2023-07-10. 사용하지 않음 */
OS_MUTEX OSMutexCreate(void) { return xSemaphoreCreateRecursiveMutex(); }

/* bccho, 2023-07-10. 사용하지 않음 */
uint8_t OSMutexPend(OS_MUTEX p_mutex, uint16_t timeout)
{
    if (xSemaphoreTakeRecursive(p_mutex, timeout) == pdPASS)
        return OS_NO_ERR;
    return OS_TIMEOUT;
}

/* bccho, 2023-07-10. 사용하지 않음 */
void OSMutexPost(OS_MUTEX p_mutex) { xSemaphoreGiveRecursive(p_mutex); }
