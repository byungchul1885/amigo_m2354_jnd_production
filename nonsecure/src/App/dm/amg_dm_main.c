/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_sw_timer.h"
#include "amg_uart.h"
#include "amg_shell.h"
#include "amg_task_primitive.h"
/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/
#define DM_TASK_STACK_SIZE ((2048 + 1024 + 1024) / 4)
#define DM_MSGQ_SIZE 32
#define EVENT_MASK_DEBUG_RX (1UL << 0)

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/
static OS_FLAG_GRP *dm_event;

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
void dsm_dm_debug_uart_rx_cb(void)
{
    uint8_t err;
    OSFlagPost(dm_event, EVENT_MASK_DEBUG_RX, OS_FLAG_SET, &err);
}

static void dsm_dm_task_main(void *pdata)
{
#if 1 /* bccho, 2023-07-07 */
    portALLOCATE_SECURE_CONTEXT(configMINIMAL_SECURE_STACK_SIZE);
#endif

    uint8_t err;
    OS_FLAGS masked_event;
    OS_FLAGS waiting_event;

    UNUSED(pdata);
    waiting_event = EVENT_MASK_PRIMITIVE | EVENT_MASK_DEBUG_RX;

    while (1)
    {
        masked_event = OSFlagPend(dm_event, waiting_event,
                                  (OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME),
                                  OS_TICK_FOR_SEC, &err);

        if (masked_event & EVENT_MASK_DEBUG_RX)
        {
            char buff[64];
            uint32_t size;

            size = dsm_uart_gets(DEBUG_COM, buff, sizeof(buff));
            while (size)
            {
                shell_main(buff, size);
                size = dsm_uart_gets(DEBUG_COM, buff, sizeof(buff));
            }
        }

        vTaskDelay(10);
    }
}

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/
void dsm_dm_initialize(void)
{
    uint8_t err;

    dm_event = OSFlagCreate(0x00, &err);
    ASSERT(dm_event);

#if 0
    /* Create the APP Task */
    OSTaskCreateExt("dm", dsm_dm_task_main, (void *)0,
                    &dm_task_stack[DM_TASK_STACK_SIZE - 1], TASK_PRI_DM,
                    TASK_DM, dm_task_stack, DM_TASK_STACK_SIZE, (void *)0, 0);
#else
    /* Create the APP Task */
    OSTaskCreateExt("dm", dsm_dm_task_main, (void *)0, 0, TASK_PRI_DM, TASK_DM,
                    0, DM_TASK_STACK_SIZE, (void *)0, 0);
#endif

#if 0 /* bccho, 2023-07-07. 사용하지 않음 */
    dsm_primitive_create(TASK_DM, dm_event, DM_MSGQ_SIZE, NULL);
#endif

    dsm_uart_reg_rx_callback(DEBUG_COM, dsm_dm_debug_uart_rx_cb);

    shell_init();
}
