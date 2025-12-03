/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_sw_timer.h"
#include "amg_typedef.h"
#include "timers.h"
#include "amg_task.h"

/*
******************************************************************************
*    Definition
******************************************************************************
*/

/*
******************************************************************************
*    MACRO
******************************************************************************
*/

/*
******************************************************************************
*    DATA TYPE
******************************************************************************
*/

/*
******************************************************************************
*    GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/
typedef struct copy_xTimer_t /* The old naming convention is used to prevent
                                breaking kernel aware debuggers. */
{
    const char				*pcTimerName;		/*<< Text name.  This is not used by the kernel, it is included simply to make debugging easier. */ /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    ListItem_t xTimerListItem; /*<< Standard linked list item as used by all
                                  kernel features for event management. */
    TickType_t
        xTimerPeriodInTicks; /*<< How quickly and often the timer expires. */
    void *pvTimerID; /*<< An ID to identify the timer.  This allows the timer to
                        be identified when the same callback is used for
                        multiple timers. */
    TimerCallbackFunction_t
        pxCallbackFunction; /*<< The function that will be called when the timer
                               expires. */
#if (configUSE_TRACE_FACILITY == 1)
    UBaseType_t uxTimerNumber; /*<< An ID assigned by trace tools such as
                                  FreeRTOS+Trace */
#endif
    uint8_t ucStatus; /*<< Holds bits to say if the timer was statically
                         allocated or not, and if it is active or not. */
} TMR_t;
/*
******************************************************************************
*    LOCAL FUNCTIONS
******************************************************************************
*/

/*  bccho, 2023-07-07, 10ms 마다 호출된다
    pOsTmr는 TimerHandle_t 인데...
    pvTimerID를 얻어내기 위해서 TMR_t를 정의해서 쓰고 있는 것 같다.
    pvTimerGetTimerID()을 쓰면 될 것 같은데...  TBD

    그리고 ISR 아니다.
    All software timer callback functions execute in the context of the same
    RTOS daemon (or ‘timer service’) task
 */
static void dsm_sw_timer_isr_proc(OS_TMR *pOsTmr)
{
    TMR_t *pOsTimer = (TMR_t *)pOsTmr;
    SW_TIMER_CNTX *pTimerCntx = (SW_TIMER_CNTX *)pOsTimer->pvTimerID;
    SW_TIMER *pTmr;
    uint32_t noOfTimer;
    uint32_t bTrigger = FALSE;
    uint8_t err;

#if 0 /* bccho, 타이머 도는 것 확인, 2023-07-21 */
    static int c;
    c++;
    if (c > 100)
    {
        MSG01("---------%d", pTimerCntx->baseInterval);
        c = 0;
    }
#endif

    ASSERT(pTimerCntx);

    noOfTimer = pTimerCntx->noOfTimer;
    pTmr = pTimerCntx->pList;

    while (noOfTimer--)
    {
        if (pTmr->bEnable)
        {
            if (pTmr->expireCnt)
            {
                pTmr->expireCnt--;
            }

            if (pTmr->expireCnt == 0)
            {
                bTrigger = TRUE;
                pTmr->triggerCnt++;

                if (pTmr->bPeriod)
                {
                    pTmr->expireCnt = pTmr->interval;
                }
                else
                {
                    pTmr->bEnable = FALSE;
                    if (pTimerCntx->noOfActTimer)
                    {
                        pTimerCntx->noOfActTimer--;
                    }
                }
            }
        }
        pTmr++;
    }

    if (bTrigger && pTimerCntx->pRcvTaskFlag)
    {
        OSFlagPost(pTimerCntx->pRcvTaskFlag, EVENT_MASK_TIMER, OS_FLAG_SET,
                   &err);
    }
}

/*
******************************************************************************
*    GLOBAL FUNCTIONS
******************************************************************************
*/

void dsm_sw_timer_init(SW_TIMER_CNTX *pTimerCntx, SW_TIMER *pTimerList,
                       uint32_t noOfTimer,
                       uint32_t baseInterval,     /* 10 ticks --> 10 ms */
                       OS_FLAG_GRP *pRcvTaskFlag) /* EventGroupHandle_t */
{
    uint8_t err;

    pTimerCntx->pList = pTimerList;
    pTimerCntx->noOfTimer = noOfTimer;
    pTimerCntx->pRcvTaskFlag = pRcvTaskFlag;
    pTimerCntx->baseInterval = baseInterval / portTICK_PERIOD_MS;

    memset(pTimerList, 0x00, noOfTimer * sizeof(SW_TIMER));

    pTimerCntx->pOsTimer = OSTmrCreate(baseInterval, /* 사용안함 */
                                       baseInterval, OS_TMR_OPT_PERIODIC,
                                       (OS_TMR_CALLBACK)dsm_sw_timer_isr_proc,
                                       (uint32_t)pTimerCntx, /* pvTimerID */
                                       (void *)"SW_TIMER", &err);
}

uint32_t dsm_sw_timer_start(SW_TIMER_CNTX *pTimerCntx, uint32_t timerId,
                            uint32_t bPeriod, uint32_t ms_interval,
                            SW_TIMER_CALLBACK timerCb)
{
    SW_TIMER *pTmr;
    uint8_t err;

    ASSERT(pTimerCntx);

    if (!pTimerCntx->pList)
    {
        return FALSE;
    }

    ASSERT(pTimerCntx->noOfTimer > timerId);

    pTmr = &pTimerCntx->pList[timerId];

    if (ms_interval == 0)
    {
        ms_interval = 1;
    }

    if (!pTmr->bEnable)
    {
        pTimerCntx->noOfActTimer++;
        pTmr->bEnable = TRUE;
    }
    else
    {
        if (((ms_interval / pTimerCntx->baseInterval) == pTmr->interval) &&
            ((uint8_t)bPeriod == pTmr->bPeriod))
        {
            return TRUE;
        }
    }

    pTmr->bPeriod = (uint8_t)bPeriod;
    pTmr->interval = ms_interval / pTimerCntx->baseInterval;
    pTmr->expireCnt = pTmr->interval;
    pTmr->triggerCnt = 0;
    pTmr->timerCb = timerCb;

    if (!pTimerCntx->bOsTimerStarted)
    {
        pTimerCntx->bOsTimerStarted = TRUE;
        OSTmrStart(pTimerCntx->pOsTimer, &err);
        ASSERT(err == OS_NO_ERR);
        return TRUE;
    }

    return TRUE;
}

uint32_t dsm_sw_timer_stop(SW_TIMER_CNTX *pTimerCntx, uint32_t timerId)
{
    SW_TIMER *pTmr;
    uint8_t err;

    ASSERT(pTimerCntx);

    if (!pTimerCntx->pList)
    {
        return FALSE;
    }
    ASSERT(pTimerCntx->noOfTimer > timerId);

    if (!pTimerCntx->noOfActTimer)
    {
        return FALSE;
    }

    pTmr = &pTimerCntx->pList[timerId];

    if (pTmr->bEnable)
    {
        pTmr->bEnable = FALSE;
        pTmr->triggerCnt = 0;
        pTimerCntx->noOfActTimer--;

        if (pTimerCntx->noOfActTimer == 0 && pTimerCntx->bOsTimerStarted)
        {
            pTimerCntx->bOsTimerStarted = FALSE;
            OSTmrStop(pTimerCntx->pOsTimer, OS_TMR_OPT_NONE, NULL, &err);
            ASSERT(err == OS_NO_ERR);
            return TRUE;
        }
    }

    return TRUE;
}

uint32_t dsm_sw_timer_get_status(SW_TIMER_CNTX *pTimerCntx, uint32_t timerId)
{
    SW_TIMER *pTmr;
    uint32_t err = FALSE;

    ASSERT(pTimerCntx);

    ASSERT(pTimerCntx->noOfTimer > timerId);

    pTmr = &pTimerCntx->pList[timerId];

    if (pTmr->bEnable)
        err = TRUE;

    return err;
}

void dsm_sw_timer_task_proc(SW_TIMER_CNTX *pTimerCntx)
{
    SW_TIMER *pTmr;
    uint32_t noOfTimer, i, cnt;

    ASSERT(pTimerCntx);

    noOfTimer = pTimerCntx->noOfTimer;

    for (i = 0; i < noOfTimer; i++)
    {
        pTmr = &pTimerCntx->pList[i];

        if (pTmr->triggerCnt)
        {
            cnt = pTmr->triggerCnt;
            pTmr->triggerCnt = 0;

            if (cnt && pTmr->timerCb)
            {
                while (cnt--)
                {
                    (pTmr->timerCb)(pTimerCntx, i);
                }
            }
        }
    }
}
// clang-format on