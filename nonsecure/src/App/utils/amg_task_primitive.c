/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_task_primitive.h"

/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/
typedef struct
{
    OS_FLAG_GRP *p_rcv_event;
    xQueueHandle q;
} PRIMITIVE_INFO;
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
static PRIMITIVE_INFO prim_inf_list[TASK_MAX];

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/

/* bccho, 2023-07-07. 사용하지 않음 */
OS_FLAG_GRP *dsm_primitive_get_event(TASK_ID id)
{
    ASSERT(id < TASK_MAX);
    return prim_inf_list[id].p_rcv_event;
}

/* bccho, 2023-07-07. 사용하지 않음 */
void dsm_primitive_create(TASK_ID id, OS_FLAG_GRP *p_rcv_event, uint32_t q_size,
                          void **q_buf)
{
    xQueueHandle q;

    ASSERT(id < TASK_MAX);
    q = xQueueCreate(q_size, sizeof(void *));
    ASSERT(q);

    prim_inf_list[id].p_rcv_event = p_rcv_event;
    prim_inf_list[id].q = q;
}

/* bccho, 2023-07-07. 사용하지 않음 */
PRIMITIVE *dsm_primitive_alloc(TASK_ID from, TASK_ID to, PRIM_CODE type,
                               uint32_t len, void *p_value)
{
    PRIMITIVE *p_prim;

    p_prim = (PRIMITIVE *)pvPortMalloc(sizeof(PRIMITIVE) - 1 + len);
    ASSERT(p_prim);

    p_prim->from = from;
    p_prim->to = to;
    p_prim->type = type;
    p_prim->len = len;

    if (len)
    {
        ASSERT(p_value);
        memcpy(p_prim->value, p_value, len);
    }

    return p_prim;
}

/* bccho, 2023-07-07. 사용하지 않음 */
void dsm_primitive_free(PRIMITIVE *p_prim)
{
    ASSERT(p_prim);
    vPortFree(p_prim);
}

/* bccho, 2023-07-07. 사용하지 않음 */
void dsm_primitive_send(PRIMITIVE *p_prim, uint32_t wait_ms)
{
    PRIMITIVE_INFO *p_inf;
    uint8_t err = OS_NO_ERR;

    ASSERT(p_prim);
    ASSERT(p_prim->to < TASK_MAX);

    p_inf = &prim_inf_list[p_prim->to];
    ASSERT(p_inf->q);

    if (uxQueueSpacesAvailable(p_inf->q))
    {
        if (wait_ms)
        {
            wait_ms = wait_ms / portTICK_PERIOD_MS;
            if (!wait_ms)
            {
                wait_ms = 1;
            }
        }
        xQueueSend(p_inf->q, (void *)&p_prim, wait_ms);
    }

    if (err == OS_NO_ERR && p_inf->p_rcv_event)
    {
        OSFlagPost(p_inf->p_rcv_event, EVENT_MASK_PRIMITIVE, OS_FLAG_SET, &err);
    }
}

/* bccho, 2023-07-07. 사용하지 않음 */
PRIMITIVE *dsm_primitive_recv(TASK_ID id)
{
    PRIMITIVE_INFO *p_inf;
    PRIMITIVE *p_prim;

    ASSERT(id < TASK_MAX);
    p_inf = &prim_inf_list[id];
    ASSERT(p_inf->q);

    if (!uxQueueMessagesWaiting(p_inf->q))
    {
        return NULL;
    }
    if (!xQueueReceive(p_inf->q, &p_prim, 10))
    {
        return NULL;
    }
    return p_prim;
}

/* bccho, 2023-07-07. 사용하지 않음 */
uint32_t dsm_primitive_get_recv_count(TASK_ID id)
{
    PRIMITIVE_INFO *p_inf;

    ASSERT(id < TASK_MAX);
    p_inf = &prim_inf_list[id];
    ASSERT(p_inf->q);

    return uxQueueMessagesWaiting(p_inf->q);
}
