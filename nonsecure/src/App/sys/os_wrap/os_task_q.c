/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "os_task_q.h"

#include "os_wrap.h"
#include "amg_task.h"
#include "amg_debug.h"

/*
******************************************************************************
*    Definition
******************************************************************************
*/
#define OS_TASKQ_STACK_SIZE (2048 / 4)
#define OS_TASKQ_BUFF_SIZE 128

#define OS_RX_TASKQ_STACK_SIZE (2048 / 4)
#define OS_RX_TASKQ_BUFF_SIZE 128

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
typedef struct
{
    void (*func)(void *);
    void *args;
} TASKQ;

/*
******************************************************************************
*    LOCAL VARIABLE
******************************************************************************
*/
static OS_STK os_taskq_stack[OS_TASKQ_STACK_SIZE];
static OS_FLAG_GRP *os_taskq_event = NULL;
static uint32_t os_taskq_wr = 0;
static uint32_t os_taskq_rd = 0;
static TASKQ os_taskq_buff[OS_TASKQ_BUFF_SIZE];

static OS_STK os_rx_taskq_stack[OS_RX_TASKQ_STACK_SIZE];
static OS_FLAG_GRP *os_rx_taskq_event = NULL;
static uint32_t os_rx_taskq_wr = 0;
static uint32_t os_rx_taskq_rd = 0;
static TASKQ os_rx_taskq_buff[OS_RX_TASKQ_BUFF_SIZE];

/*
******************************************************************************
*    LOCAL FUNCTIONS
******************************************************************************
*/

static void os_taskq_exec(void)
{
    uint8_t err;

    if (os_taskq_rd != os_taskq_wr)
    {
        if (os_taskq_buff[os_taskq_rd].func)
        {
            (*os_taskq_buff[os_taskq_rd].func)(os_taskq_buff[os_taskq_rd].args);
        }

        os_taskq_rd = (os_taskq_rd + 1) & (OS_TASKQ_BUFF_SIZE - 1);

        if (os_taskq_rd != os_taskq_wr)
        {
            OSFlagPost(os_taskq_event, EVENT_MASK_TASKQ, OS_FLAG_SET, &err);
        }
    }
}

static void os_rx_taskq_exec(void)
{
    uint8_t err;

    if (os_rx_taskq_rd != os_rx_taskq_wr)
    {
        if (os_rx_taskq_buff[os_rx_taskq_rd].func)
        {
            (*os_rx_taskq_buff[os_rx_taskq_rd].func)(
                os_rx_taskq_buff[os_rx_taskq_rd].args);
        }

        os_rx_taskq_rd = (os_rx_taskq_rd + 1) & (OS_RX_TASKQ_BUFF_SIZE - 1);

        if (os_rx_taskq_rd != os_rx_taskq_wr)
        {
            OSFlagPost(os_rx_taskq_event, EVENT_MASK_TASKQ, OS_FLAG_SET, &err);
        }
    }
}

static void os_taskq_main(void *pdata)
{
    uint8_t err;
    OS_FLAGS masked_event;
    uint32_t run_time;

    UNUSED(pdata);

    while (1)
    {
        masked_event = OSFlagPend(os_taskq_event, EVENT_MASK_TASKQ,
                                  (OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME),
                                  OS_TICK_FOR_SEC, &err);
        run_time = OS_TIME_GET();

        if (masked_event & EVENT_MASK_TASKQ)
        {
            os_taskq_exec();
        }

        OS_CHECK_TASK_BLOCK("taskq", run_time);

#if 0 /* bccho, 2023-07-20 */
        osDelay(1);
#else
        vTaskDelay(1);
#endif
    }
}

static void os_rx_taskq_main(void *pdata)
{
    uint8_t err;
    OS_FLAGS masked_event;
    uint32_t run_time;

    UNUSED(pdata);

    while (1)
    {
        masked_event = OSFlagPend(
            os_rx_taskq_event, EVENT_MASK_TASKQ | EVENT_MASK_TIMER,
            (OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME), OS_TICK_FOR_SEC, &err);
        run_time = OS_TIME_GET();

        if (masked_event & EVENT_MASK_TASKQ)
        {
            os_rx_taskq_exec();
        }

        OS_CHECK_TASK_BLOCK("rxtaskq", run_time);
#if 0 /* bccho, 2023-07-20 */
        osDelay(1);
#else
        vTaskDelay(1);
#endif
    }
}

/*
******************************************************************************
*    GLOBAL FUNCTIONS
******************************************************************************
*/
void os_taskq_init(void)
{
    uint8_t err;

    os_taskq_wr = os_taskq_rd = 0;
    os_rx_taskq_wr = os_rx_taskq_rd = 0;

    os_taskq_event = OSFlagCreate(0x00, &err);
    os_rx_taskq_event = OSFlagCreate(0x00, &err);

    OSTaskCreateExt("taskq", os_taskq_main, (void *)0,
                    &os_taskq_stack[OS_TASKQ_STACK_SIZE - 1], TASK_PRI_TASKQ,
                    TASK_TASKQ, os_taskq_stack, OS_TASKQ_STACK_SIZE, (void *)0,
                    0);

    OSTaskCreateExt("rxtaskq", os_rx_taskq_main, (void *)0,
                    &os_rx_taskq_stack[OS_RX_TASKQ_STACK_SIZE - 1],
                    TASK_PRI_RX_TASKQ, TASK_RX_TASKQ, os_rx_taskq_stack,
                    OS_RX_TASKQ_STACK_SIZE, (void *)0, 0);
}

bool os_taskq_reg(void (*func)(void *), void *args)
{
    uint32_t next;
    uint8_t err;

    next = (os_taskq_wr + 1) & (OS_TASKQ_BUFF_SIZE - 1);

    if (os_taskq_rd == next)
    {
        DPRINTF(DBG_ERR, "os_task_q alloc failed, func_p=%X\r\n", func);
        return FALSE;
    }
    else
    {
        os_taskq_buff[os_taskq_wr].func = func;
        os_taskq_buff[os_taskq_wr].args = args;
        os_taskq_wr = next;
        OSFlagPost(os_taskq_event, EVENT_MASK_TASKQ, OS_FLAG_SET, &err);

        return TRUE;
    }
}

bool os_rx_taskq_reg(void (*func)(void *), void *args)
{
    uint32_t next;
    uint8_t err;

    next = (os_rx_taskq_wr + 1) & (OS_RX_TASKQ_BUFF_SIZE - 1);

    if (os_rx_taskq_rd == next)
    {
        DPRINTF(DBG_ERR, "os_rx_task_q alloc failed, func_p=%X\r\n", func);
        return FALSE;
    }
    else
    {
        os_rx_taskq_buff[os_rx_taskq_wr].func = func;
        os_rx_taskq_buff[os_rx_taskq_wr].args = args;
        os_rx_taskq_wr = next;
        OSFlagPost(os_rx_taskq_event, EVENT_MASK_TASKQ, OS_FLAG_SET, &err);

        return TRUE;
    }
}
