/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "os_msgq.h"
#include "amg_typedef.h"
#include "amg_task.h"
/*
******************************************************************************
*    Definition
******************************************************************************
*/

/*
******************************************************************************
*    Data Types
******************************************************************************
*/

/*
******************************************************************************
*    Local Variables
******************************************************************************
*/

/*
******************************************************************************
*    GLOBAL FUNCTIONS
******************************************************************************
*/
void os_msgq_init (OS_MSGQ *p_q, OS_MSG *p_msg_base, OS_FLAG_GRP *p_rcv_event, uint32_t size, const char *name)
{
    ASSERT (p_q);

    p_q->p_rcv_event = p_rcv_event;
    p_q->buff_alloc_on = !p_msg_base;
    p_q->p_base = p_msg_base ? p_msg_base : pvPortMalloc(sizeof (OS_MSG) * size);
    ASSERT (p_q->p_base);

    p_q->used = 0;
    p_q->size = size;
    p_q->wr = p_q->rd = 0;
    p_q->p_name = name;
}

void os_msgq_close (OS_MSGQ *p_q)
{
    ASSERT (p_q);

    if (p_q->buff_alloc_on && p_q->p_base)
    {
    	vPortFree (p_q->p_base);
    }
}

uint32_t os_msgq_get_rcv_size (OS_MSGQ *p_q)
{
    uint32_t sz;

    ASSERT (p_q);

    OS_ENTER_CRITICAL ();
    sz = p_q->used;
    OS_EXIT_CRITICAL ();

    return sz;
}

uint32_t os_msgq_get_avail_size (OS_MSGQ *p_q)
{
    uint32_t sz;
    ASSERT (p_q);

    OS_ENTER_CRITICAL ();
    sz = p_q->size - p_q->used;
    OS_EXIT_CRITICAL ();

    return sz;
}

bool os_msgq_send (OS_MSGQ *p_q, void *p_msg, uint32_t msec)
{
    uint8_t err;

    ASSERT (p_q);

    OS_ENTER_CRITICAL ();

    if (p_q->used == p_q->size)
    {
        OS_EXIT_CRITICAL ();
        return FALSE;
    }

    p_q->p_base[p_q->wr].p_msg = p_msg;
    p_q->wr++;

    if (p_q->wr == p_q->size)
    {
        p_q->wr = 0;
    }
    p_q->used++;

    OS_EXIT_CRITICAL ();

    if (p_q->p_rcv_event)
    {
        OSFlagPost (p_q->p_rcv_event, EVENT_MASK_MSGQ_RCV, OS_FLAG_SET, &err);
    }

    return TRUE;
}


bool os_msgq_receive (OS_MSGQ *p_q, void **p_msg, uint32_t msec)
{
    ASSERT (p_q);

    OS_ENTER_CRITICAL ();

    if (p_q->used == 0)
    {
        OS_EXIT_CRITICAL ();
        return FALSE;
    }

    *p_msg = p_q->p_base[p_q->rd].p_msg;
    p_q->rd++;
    if (p_q->rd == p_q->size)
    {
        p_q->rd = 0;
    }
    p_q->used--;

    OS_EXIT_CRITICAL ();

    return TRUE;
}


