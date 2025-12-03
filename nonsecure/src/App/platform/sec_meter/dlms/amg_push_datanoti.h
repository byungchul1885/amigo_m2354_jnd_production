#if !defined(__AMG_DATA_NOTI_H__)
#define __AMG_DATA_NOTI_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "whm.h"

/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/
#define PUSHACTI_CODE1_PWR_OFF (1 << 7)
#define PUSHACTI_CODE1_ENERGY_REMAINING (1 << 6)
#define PUSHACTI_CODE1_SECURITY_ERR (1 << 4)
#define PUSHACTI_CODE1_MAGNET_SENS (1 << 3)
#define PUSHACTI_CODE1_COVER_OPEN (1 << 2)
#define PUSHACTI_CODE1_WORK_BLACKOUT (1 << 1)
#define PUSHACTI_CODE1_NO_BATTERY (1 << 0)

#define PUSHACTI_CODE2_OVERCURR_C (1 << 7)
#define PUSHACTI_CODE2_OVERCURR_B (1 << 6)
#define PUSHACTI_CODE2_OVERCURR_A (1 << 5)
#define PUSHACTI_CODE2_WRONGCONN (1 << 4)
#define PUSHACTI_CODE2_TEMP (1 << 3)

#define PUSHACTI_CODE3_LATCH_ERR_C (1 << 7)
#define PUSHACTI_CODE3_LATCH_ERR_B (1 << 6)
#define PUSHACTI_CODE3_LATCH_ERR_A (1 << 5)
#define PUSHACTI_CODE3_SELF_ERR (1 << 4)
#define PUSHACTI_CODE3_NEUT_WRONGCONN (1 << 3)
#define PUSHACTI_CODE3_NO_PHASE_C (1 << 2)
#define PUSHACTI_CODE3_NO_PHASE_B (1 << 1)
#define PUSHACTI_CODE3_NO_PHASE_A (1 << 0)

#define PUSHACTI_CODE4_OVERVOT_C (1 << 7)
#define PUSHACTI_CODE4_OVERVOT_B (1 << 6)
#define PUSHACTI_CODE4_OVERVOT_A (1 << 5)
#define PUSHACTI_CODE4_LOWVOT_C (1 << 4)
#define PUSHACTI_CODE4_LOWVOT_B (1 << 3)
#define PUSHACTI_CODE4_LOWVOT_A (1 << 2)
#define PUSHACTI_CODE4_SAG (1 << 1)
#define PUSHACTI_CODE4_SWELL (1 << 0)

#define PUSH_SCRIPT_MAX_NUM 10
#define PUSH_SETUP_MAX_NUM 10

#define PUSH_SCRIPT_ID_ERR_CODE 1
#define PUSH_SCRIPT_ID_LAST_LP 2

#define PUSH_SETUP_ENABLE 1
#define PUSH_SETUP_DISABLE 0

#if 1  // 2023.0921 JP
#define PUSH_SETUP_DEFAULT_RANDOM_ST_INTVAL_ERR_CODE 0
#define PUSH_SETUP_DEFAULT_RANDOM_ST_INTVAL_LAST_LP 10
#else
#define PUSH_SETUP_DEFAULT_RANDOM_ST_INTVAL 10
#endif

#define PUSH_WINDOW_MAX_NUM 1

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
typedef enum
{
    ERR_CODE_1_IDX,
    ERR_CODE_2_IDX,
    ERR_CODE_3_IDX,
    ERR_CODE_4_IDX,
    ERR_CODE_MAX_IDX
} EN_ERR_CODE_IDX;

typedef struct _push_acti_errocode_
{
    uint8_t code[ERR_CODE_MAX_IDX];
    uint16_t CRC_M;
} ST_PUSH_ACTI_ERRCODE;

typedef struct _push_script_
{
    uint8_t identifier;
    uint16_t obj_idx;
} ST_PUSH_SCRIPT;

typedef struct _push_window_
{
    date_time_type st_dt;
    date_time_type sp_dt;
} ST_PUSH_WINDOW;

typedef struct _push_setup_
{
    uint8_t identifier;
    uint8_t window_cnt;
    ST_PUSH_WINDOW window[PUSH_WINDOW_MAX_NUM];
    uint8_t retry_times;
    uint8_t random_start_intval;
    uint8_t repetition_delay;
} ST_PUSH_SETUP;

typedef struct _push_script_table_
{
    uint8_t cnt;
    ST_PUSH_SCRIPT script[PUSH_SCRIPT_MAX_NUM];
    uint16_t CRC_M;

} ST_PUSH_SCRIPT_TABLE;

typedef struct _push_setup_table_
{
    uint8_t cnt;
    ST_PUSH_SETUP setup[PUSH_SETUP_MAX_NUM];
    uint16_t CRC_M;

} ST_PUSH_SETUP_TABLE;

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

uint32_t dsm_push_err_code_nvread(ST_PUSH_ACTI_ERRCODE* pst_push_acti);
uint32_t dsm_push_err_code_nvwrite(ST_PUSH_ACTI_ERRCODE* pst_push_acti);
uint8_t dsm_covert_grp_e_2_errcodeidx(uint8_t grp_e);
uint32_t dsm_push_script_nvread(ST_PUSH_SCRIPT_TABLE* pst_push_script);
uint32_t dsm_push_script_nvwrite(ST_PUSH_SCRIPT_TABLE* pst_push_script);
uint32_t dsm_push_get_encoded_script_table(uint8_t* len, uint8_t* data);
uint32_t dsm_push_setup_nvread(ST_PUSH_SETUP_TABLE* pst_push_setup);
uint32_t dsm_push_setup_nvwrite(ST_PUSH_SETUP_TABLE* pst_push_setup);
ST_PUSH_SETUP_TABLE* dsm_push_setup_get_setup_table(void);
void dsm_push_push_setup_table_default(void);
uint32_t dsm_push_setup_is_id(uint8_t id, uint8_t* pidx);
void dsm_push_disable(void);
void dsm_can_set_push_flag(uint32_t on_off);
uint32_t dsm_can_get_push_flag(void);
void dsm_data_push_init(void);
uint32_t dsm_push_is_enable(uint8_t script_id);
void appl_push_msg_lastLP(void);
void appl_push_msg_errcode(void);
void dsm_push_data_noti_proc(uint32_t type);

#endif /* __AMG_DATA_NOTI_H__ */
