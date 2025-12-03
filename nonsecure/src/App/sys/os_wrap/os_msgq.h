#ifndef __OS_MSGQ_H__
#define __OS_MSGQ_H__

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
typedef struct
{
    void        *p_msg;
}
OS_MSG;

typedef struct
{
    OS_FLAG_GRP *p_rcv_event;
    OS_MSG      *p_base;
    bool        buff_alloc_on;
    uint32_t      size;
    uint32_t      used;
    uint32_t      wr;
    uint32_t      rd;
    const char  *p_name;
}
OS_MSGQ;

/*
******************************************************************************
*    GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*    GLOBAL FUNCTIONS
******************************************************************************
*/
void os_msgq_init (OS_MSGQ *p_q, OS_MSG *p_msg_base, OS_FLAG_GRP *p_rcv_event, uint32_t size, const char *name);
void os_msgq_close (OS_MSGQ *p_q);
uint32_t os_msgq_get_rcv_size (OS_MSGQ *p_q);
uint32_t os_msgq_get_avail_size (OS_MSGQ *p_q);
bool os_msgq_send (OS_MSGQ *p_q, void *p_msg, uint32_t ms_time);
bool os_msgq_receive (OS_MSGQ *p_q, void **p_msg, uint32_t ms_time);

#endif


