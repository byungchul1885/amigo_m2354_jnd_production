#ifndef __OS_PIPE_H__
#define __OS_PIPE_H__
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
typedef struct
{
    uint32_t      wr, rd;
    uint32_t      block_size;
    uint32_t      total_blocks;
    uint32_t      used_blocks;
    uint8_t       *p_buf;
    const char  *p_name;
    OS_FLAG_GRP *p_rcv_event;
}
OS_PIPE;

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
void os_pipe_create (OS_PIPE *p_pipe, OS_FLAG_GRP *p_rcv_event, void *p_buff, uint32_t block_size, uint32_t total_blocks,  const char *p_name);
void os_pipe_reset (OS_PIPE *p_pipe);
bool os_pipe_is_empty (OS_PIPE *p_pipe);
uint32_t os_pipe_get_rcv_size (OS_PIPE *p_pipe);
uint32_t os_pipe_get_avail_size (OS_PIPE *p_pipe);
bool os_pipe_send (OS_PIPE *p_pipe, void *p_data, uint32_t no_of_blocks);
bool os_pipe_peek (OS_PIPE *p_pipe, void *p_data, uint32_t no_of_blocks);
bool os_pipe_receive (OS_PIPE *p_pipe, void *p_data, uint32_t *p_no_of_blocks);

#endif /* __OS_PIPE_H__*/
