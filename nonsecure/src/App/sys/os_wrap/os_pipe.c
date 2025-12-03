/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "os_pipe.h"
#include "amg_task.h"
#include "amg_typedef.h"
#include "amg_debug.h"

/*
******************************************************************************
*    Definition
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
void os_pipe_create (OS_PIPE *p_pipe, OS_FLAG_GRP *p_rcv_event, void *p_buff, uint32_t block_size, uint32_t total_blocks,  const char *p_name)
{
    ASSERT (p_pipe);
    ASSERT (block_size);
    ASSERT (total_blocks);

    memset (p_pipe, 0x00, sizeof (OS_PIPE));

    if (!p_buff)
    {
        p_pipe->p_buf = (uint8_t*)pvPortMalloc(block_size * total_blocks);
        ASSERT (p_pipe->p_buf);
    }
    else
    {
        p_pipe->p_buf = p_buff;
    }

    memset (p_pipe->p_buf, 0x00, sizeof (block_size * total_blocks));
    p_pipe->p_name = p_name;
    p_pipe->block_size = block_size;
    p_pipe->total_blocks = total_blocks;
    p_pipe->p_rcv_event = p_rcv_event;

    DPRINTF (DBG_WARN, "[%s] pipe (%p) initialized\r\n", p_name, p_pipe);
}

void os_pipe_reset (OS_PIPE *p_pipe)
{
    ASSERT (p_pipe);

    OS_ENTER_CRITICAL ();
    p_pipe->wr = p_pipe->rd = 0;
    p_pipe->used_blocks = 0;
    OS_EXIT_CRITICAL ();
}

bool os_pipe_is_empty (OS_PIPE *p_pipe)
{
    ASSERT (p_pipe);

    return p_pipe->used_blocks == 0;
}

uint32_t os_pipe_get_rcv_size (OS_PIPE *p_pipe)
{
    ASSERT (p_pipe);
    return p_pipe->used_blocks;
}

uint32_t os_pipe_get_avail_size (OS_PIPE *p_pipe)
{
    ASSERT (p_pipe);
    return p_pipe->total_blocks - p_pipe->used_blocks;
}

bool os_pipe_send (OS_PIPE *p_pipe, void *p_data, uint32_t no_of_blocks)
{
    uint32_t avail, upper, lower;

    ASSERT (p_pipe);
    ASSERT (p_data);
    ASSERT (no_of_blocks);

    OS_ENTER_CRITICAL ();

    avail = p_pipe->total_blocks - p_pipe->used_blocks;
    if (avail < no_of_blocks)
    {
        OS_EXIT_CRITICAL ();
        return FALSE;
    }
    upper = p_pipe->total_blocks - p_pipe->wr;

    if (upper >= no_of_blocks)
    {
        memcpy ((void*)&p_pipe->p_buf[p_pipe->wr * p_pipe->block_size],
                (void*)p_data,
                no_of_blocks * p_pipe->block_size);
    }
    else
    {
        uint8_t *p_buff = (uint8_t*)p_data;

        lower = no_of_blocks - upper;

        memcpy ((void*)&p_pipe->p_buf[p_pipe->wr * p_pipe->block_size],
                (void*)p_data,
                upper * p_pipe->block_size);

        memcpy ((void*)p_pipe->p_buf,
                (void*)&p_buff[upper * p_pipe->block_size],
                lower * p_pipe->block_size);
    }

    p_pipe->wr = (p_pipe->wr + no_of_blocks)  % p_pipe->total_blocks;
    p_pipe->used_blocks += no_of_blocks;

    OS_EXIT_CRITICAL ();

    if (p_pipe->p_rcv_event)
    {
        uint8_t err;
        OSFlagPost (p_pipe->p_rcv_event, EVENT_MASK_PIPE_RCV, OS_FLAG_SET, &err);
        return err == OS_NO_ERR;
    }
    return TRUE;
}

bool os_pipe_peek (OS_PIPE *p_pipe, void *p_data, uint32_t no_of_blocks)
{
    uint32_t upper, lower;

    ASSERT (p_pipe);
    ASSERT (p_data);
    ASSERT (no_of_blocks);

    OS_ENTER_CRITICAL ();

    if (p_pipe->used_blocks < no_of_blocks)
    {
        OS_EXIT_CRITICAL ();
        return FALSE;
    }

    upper = p_pipe->total_blocks - p_pipe->wr;

    if (p_pipe->rd <= p_pipe->wr)
    {
        memcpy (p_data,
                (void*)&p_pipe->p_buf[p_pipe->rd * p_pipe->block_size],
                no_of_blocks * p_pipe->block_size);
    }
    else
    {
        upper = p_pipe->total_blocks - p_pipe->rd;

        if (upper >= no_of_blocks)
        {
            memcpy (p_data,
                    (void*)&p_pipe->p_buf[p_pipe->rd * p_pipe->block_size],
                    no_of_blocks * p_pipe->block_size);
        }
        else
        {
            uint8_t *p_buff = (uint8_t*)p_data;
            lower = no_of_blocks - upper;

            memcpy (p_data,
                    (void*)&p_pipe->p_buf[p_pipe->rd * p_pipe->block_size],
                    upper * p_pipe->block_size);

            memcpy ((void*)&p_buff[upper * p_pipe->block_size],
                    (void*)p_pipe->p_buf,
                    lower * p_pipe->block_size);
        }
    }

    OS_EXIT_CRITICAL ();

    return TRUE;
}

bool os_pipe_receive (OS_PIPE *p_pipe, void *p_data, uint32_t *p_no_of_blocks)
{
    uint32_t upper, lower;

    ASSERT (p_pipe);
    ASSERT (p_data);
    ASSERT (p_no_of_blocks);

    OS_ENTER_CRITICAL ();

    if (!p_pipe->used_blocks)
    {
        *p_no_of_blocks = 0;
        OS_EXIT_CRITICAL ();
        return FALSE;
    }

    if (p_pipe->used_blocks < *p_no_of_blocks)
    {
        *p_no_of_blocks = p_pipe->used_blocks;
    }

    upper = p_pipe->total_blocks - p_pipe->wr;

    if (p_pipe->rd <= p_pipe->wr)
    {
        memcpy (p_data,
                (void*)&p_pipe->p_buf[p_pipe->rd * p_pipe->block_size],
                *p_no_of_blocks * p_pipe->block_size);
    }
    else
    {
        upper = p_pipe->total_blocks - p_pipe->rd;

        if (upper >= *p_no_of_blocks)
        {
            memcpy (p_data,
                    (void*)&p_pipe->p_buf[p_pipe->rd * p_pipe->block_size],
                    *p_no_of_blocks * p_pipe->block_size);
        }
        else
        {
            uint8_t *p_buff = (uint8_t*)p_data;
            lower = *p_no_of_blocks - upper;

            memcpy (p_data,
                    (void*)&p_pipe->p_buf[p_pipe->rd * p_pipe->block_size],
                    upper * p_pipe->block_size);

            memcpy ((void*)&p_buff[upper * p_pipe->block_size],
                    (void*)p_pipe->p_buf,
                    lower * p_pipe->block_size);
        }
    }

    p_pipe->rd = (p_pipe->rd + *p_no_of_blocks)  % p_pipe->total_blocks;
    p_pipe->used_blocks -= *p_no_of_blocks;

    OS_EXIT_CRITICAL ();

    return TRUE;
}


