#ifndef __AMG_SW_TIMER_H__
#define __AMG_SW_TIMER_H__
/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "os_wrap.h"
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
typedef	void (*SW_TIMER_CALLBACK)(void *pTmrCntx, uint32_t timerId);

typedef struct
{
    SW_TIMER_CALLBACK timerCb;
    uint32_t  interval;
    uint32_t  expireCnt;
    uint32_t  triggerCnt;
    uint8_t   bEnable;
    uint8_t   bPeriod;
}
SW_TIMER;

typedef struct
{
    SW_TIMER    *pList;
    OS_TMR      *pOsTimer;
    OS_FLAG_GRP *pRcvTaskFlag;
    uint32_t      baseInterval;
    uint8_t       noOfTimer;
    uint8_t       bOsTimerStarted;
    uint8_t       noOfActTimer;
}
SW_TIMER_CNTX;

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

/*
******************************************************************************
*    LOCAL FUNCTIONS
******************************************************************************
*/

extern void dsm_sw_timer_init  (SW_TIMER_CNTX *pTimerCntx, SW_TIMER *pTimerList, uint32_t noOfTimer, uint32_t baseInterval, OS_FLAG_GRP *pRcvTaskFlag);
extern uint32_t dsm_sw_timer_start (SW_TIMER_CNTX *pTimerCntx, uint32_t timerId, uint32_t bPeriod, uint32_t interval, SW_TIMER_CALLBACK timerCb);
extern uint32_t dsm_sw_timer_stop (SW_TIMER_CNTX *pTimerCntx, uint32_t timerId);
extern uint32_t dsm_sw_timer_get_status(SW_TIMER_CNTX *pTimerCntx, uint32_t timerId);
extern void dsm_sw_timer_task_proc (SW_TIMER_CNTX *pTimerCntx);

#endif /* __XN_SW_TIMER_H__ */



