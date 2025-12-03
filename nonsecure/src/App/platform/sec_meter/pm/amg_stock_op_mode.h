#if !defined(__AMG_STOCK_OP_MODE_H__)
#define __AMG_STOCK_OP_MODE_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "options_sel.h"
#include "main.h"
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/
#define __IMODEM_PF_STOCK_OP 1
#define __IMODEM_PF_NORMAL_OP 0

typedef enum
{
    STOCK_FSM_NONE,         /* AC전원 ON */
    STOCK_FSM_WAKE_DEFAULT, /* 20:00에 깨어나는 기다리는 상태 */
    STOCK_FSM_WAKE_ALA_1,   /* 첫번째 알람 */
    STOCK_FSM_WAKE_ALA_2,   /* 두번째 알람 */
} EN_STOCK_FSM;
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
typedef struct _st_random__
{
    uint8_t seed[4];
    uint16_t CRC_M;
} ST_RANDOM;

typedef struct _st_rand_tx_info_
{
    uint32_t sec_1;
    uint16_t ms_1;
    uint32_t sec_2;
    uint16_t ms_2;
    uint32_t seed;
} ST_RAND_TX_INFO;

typedef struct _st_stock_op_times__
{
    uint16_t cnt;
    uint16_t CRC_M;
} ST_STOCK_OP_TIMES;

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
void dsm_stock_op_init(void);
void dsm_stock_op_times_reset(void);
uint16_t dsm_stock_op_times_read(void);
void dsm_stock_op_times_write(void);
char* dsm_stock_op_fsm_string(uint32_t fsm);
void dsm_stock_set_fsm(uint8_t state);
uint8_t dsm_stock_op_get_fsm(void);
void dsm_stock_op_alarm_set_default(void);
void dsm_stock_op_random_seed_init(void);
ST_RAND_TX_INFO* dsm_stock_op_get_rand_txinfo(void);
void dsm_stock_op_create_random_time(ST_RAND_TX_INFO* p_rand_txinfo);
void dsm_stock_op_proc_for_lowpower_entry(uint32_t w_evt);
void dsm_stock_action(void);

#endif /* __AMG_STOCK_OP_MODE_H__ */
