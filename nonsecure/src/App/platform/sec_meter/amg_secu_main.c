#if 0 /* bccho, 2025-04-09 */
#include "main.h"
#include "amg_task_primitive.h"
#include "amg_sec.h"
#include "amg_meter_main.h"

#define EVENT_MASK_SECU_INIT (1UL << 0)
static OS_FLAG_GRP *secu_event;

void dsm_secu_module_init(void)
{
    uint8_t err;
    OSFlagPost(secu_event, EVENT_MASK_SECU_INIT, OS_FLAG_SET, &err);
}

static void dsm_secu_task_main(void *pdata)
{
    portALLOCATE_SECURE_CONTEXT(configMINIMAL_SECURE_STACK_SIZE);

    uint8_t err;
    OS_FLAGS masked_event;
    OS_FLAGS waiting_event;

    UNUSED(pdata);
    waiting_event = EVENT_MASK_SECU_INIT;

    while (1)
    {
        masked_event = OSFlagPend(secu_event, waiting_event,
                                  (OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME),
                                  OS_MS2TICK(2), &err);

        if (masked_event & EVENT_MASK_SECU_INIT)
        {
            hls_sec_init();
            dsm_sec_module_initialized();
        }

        vTaskDelay(10);
    }
}

void dsm_secu_task_initialize(void)
{
    uint8_t err;
    secu_event = OSFlagCreate(0x00, &err);
    ASSERT(secu_event);

    OSTaskCreateExt("sec", dsm_secu_task_main, (void *)0, 0, /* 사용안함 */
                    TASK_PRI_METER - 1,                      /* 가장 낮게 */
                    TASK_METER,                              /* 사용안함 */
                    0,                                       /* 사용안함 */
                    2048,                                    /* 스택 크기 */
                    (void *)0,                               /* 사용안함 */
                    0                                        /* 사용안함 */
    );
}
#endif