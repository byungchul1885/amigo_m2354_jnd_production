#if !defined(__AMG_TASK_PRIMITIVE_H__)
#define __AMG_TASK_PRIMITIVE_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "amg_task.h"
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/
typedef enum
{
    PRIM_DM_START   = 0,

    PRIM_METER_START,
}
PRIM_CODE;


typedef struct
{
    uint8_t   from;
    uint8_t   to;
    uint16_t  type;
    uint16_t  len;
    uint8_t   value[1];
}
PRIMITIVE;

typedef struct _evt_fmt_
{
    uint16_t evt;
    uint16_t len;
    uint8_t  data[512];
}
ST_EVT_FMT;

/*
******************************************************************************
*	MACRO
******************************************************************************
*/

/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/
void        dsm_primitive_create (TASK_ID id, OS_FLAG_GRP *p_rcv_event, uint32_t q_size, void **q_buf);
PRIMITIVE * dsm_primitive_alloc (TASK_ID from, TASK_ID to, PRIM_CODE type, uint32_t len, void *p_value);
void        dsm_primitive_free (PRIMITIVE *p_prim);
void        dsm_primitive_send (PRIMITIVE *p_prim, uint32_t wait_ms);
PRIMITIVE*  dsm_primitive_recv (TASK_ID id);
uint32_t      dsm_primitive_get_recv_count (TASK_ID id);
OS_FLAG_GRP *dsm_primitive_get_event (TASK_ID id);

#endif /*__AMG_TASK_PRIMITIVE_H__*/

