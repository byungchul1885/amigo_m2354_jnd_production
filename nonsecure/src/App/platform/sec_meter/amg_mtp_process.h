#if !defined(__AMG_MTP_PROCESS_H__)
#define __AMG_MTP_PROCESS_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_mif_prtl.h"
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/

#define ToHFloat(a, b)      \
    (a)->c[HI_HI] = (b)[0]; \
    (a)->c[HI_LO] = (b)[1]; \
    (a)->c[LO_HI] = (b)[2]; \
    (a)->c[LO_LO] = (b)[3]

#define ToCommFloat(a, b)   \
    (a)[0] = (b)->c[HI_HI]; \
    (a)[1] = (b)->c[HI_LO]; \
    (a)[2] = (b)->c[LO_HI]; \
    (a)[3] = (b)->c[LO_LO]

typedef enum
{
    MTP_FSM_NONE,
    MTP_FSM_CAL_ST,
    MTP_FSM_CAL_GET,
    MTP_FSM_CAL_SET,
    MTP_FSM_PARM_SET,
    MTP_FSM_SAG_SWELL,
    MTP_FSM_PARM_GET,
    MTP_FSM_SAG_SWELL_GET,
    MTP_FSM_MESURE_READY,
    MTP_FSM_FW_START_SET,
    MTP_FSM_FW_END_SET,
    MTP_FSM_FW_SET,
    MTP_FSM_FW_GET,
    MTP_FSM_EOI_PULSE_ST,
#ifdef MTP_ZCD_ON_OFF
    MTP_FSM_ZCD_PULSE_ST,
    MTP_FSM_ZCD_PULSE_END,
#endif
} EN_MTP_FSM;

typedef enum
{
    MTP_OP_NORMAL,
    MTP_OP_CAL,
} EN_MTP_OP_MODE;

typedef enum
{
    MTP_FW_TYPE0 = 0,
    MTP_FW_TYPE1,
    MTP_FW_TYPE2,
    MTP_FW_TYPE_START,
} TYPE_MTP_FW_IMAGE;

#define MTP_RUNNING_LOW_BANK 1
#define MTP_RUNNING_HIGH_BANK 2
#define MTP_RUNNING_NOINFO_BANK 0xFF

typedef struct _mtp_fsm_con_
{
    uint8_t calst_ok;
    uint8_t calset_ok;
    uint8_t parmset_ok;
    uint8_t sagswell_ok;
    uint8_t retry;
    uint8_t TO_retry;
} ST_MTP_CON;

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
typedef struct _mtp_cal_data_t
{
    uint32_t r_current_gain;
    uint32_t r_voltage_gain;
    uint32_t r_phase_gain;
    uint32_t s_current_gain;
    uint32_t s_voltage_gain;
    uint32_t s_phase_gain;
    uint32_t t_current_gain;
    uint32_t t_voltage_gain;
    uint32_t t_phase_gain;
    uint8_t ok;
} __PACKED ST_MTP_CAL_DATA;

//
typedef struct _mtp_accum_energy_t
{
    float DeliAct;    /* 수전 유효 */
    float DLagReact;  /* 수전 지상무효 */
    float DLeadReact; /* 수전 진상무효 */
                      // float DeliApp;
    float ReceiAct;   /* 송전 유효 */
    float RLeadReact; /* 송전 진상무효 */
    float RLagReact;  /* 송전 지상무효 */
    // float ReceiApp;
} ST_MTP_ACCUM_EG;

//
typedef struct _mtp_accum_rst_energy_t
{
    float DeliAct;    /* 수전 유효 */
    float DLagReact;  /* 수전 지상무효 */
    float DLeadReact; /* 수전 진상무효 */
    float DeliApp;    /* 수전 피상 */
    float ReceiAct;   /* 송전 유효 */
    float RLeadReact; /* 송전 진상무효 */
    float RLagReact;  /* 송전 지상무효 */
    float ReceiApp;   /* 송전 피상 */
} ST_MTP_ACCUM_RST_EG;

typedef struct _mtp_push_data_t
{
    float r_voltage;
    float r_current;
    float r_phase;
    float r_act;
    float r_react;
    float r_vol_thd;

    float s_voltage;
    float s_current;
    float s_phase;
    float s_act;
    float s_react;
    float s_vol_thd;

    float t_voltage;
    float t_current;
    float t_phase;
    float t_act;
    float t_react;
    float t_vol_thd;

    float freq;

    float rs_phase;
    float rt_phase; /* 21개 */

    ST_MTP_ACCUM_RST_EG rst_accum; /* bccho, 2023-07-11, 8개 */
    ST_MTP_ACCUM_EG r_accum;       /* 6개 */
    ST_MTP_ACCUM_EG s_accum;       /* 6개 */
    ST_MTP_ACCUM_EG t_accum;       /* 6개 */

    /* float 47개 --> 188 bytes */

    uint8_t sag_count;
    uint8_t swell_count;
    float temp;
    uint8_t cal_disp_sts;
} ST_MTP_PUSH_DATA;

typedef struct _mtp_cal_start_t
{
    float ref_voltage;
    float ref_current;
    float ref_phase;
    uint8_t process_time;
    uint16_t act_const;
    uint16_t react_const;
    uint16_t app_const;
    uint8_t ok;
} ST_MTP_CAL_START;

typedef struct _st_mtp_cal_point_
{
    ST_MIF_CAL_START val;
    uint16_t CRC_M;
} ST_MTP_CAL_POINT;

typedef struct _st_mtp_parm_
{
    ST_MIF_METER_PARM val;
    uint16_t CRC_M;
} ST_MTP_PARM;

typedef struct _st_mtp_sagswell_
{
    ST_MIF_SAGSWELL_SETUP val;
    uint16_t CRC_M;
} ST_MTP_SAGSWELL;

/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/
extern ST_MTP_CAL_DATA g_mtp_caldata;

#define ZBM_NO_ERROR 0
#define ZBM_ERROR_MAX ZBM_NO_ERROR
#define ZBM_NOT_A_COMMAND -1
#define ZBM_BAD_PIP -2
#define ZBM_NO_IMAGE -3
#define ZBM_CODE_PTR_AFTER_EOF -4
#define ZBM_CODE_PTR_NOT_ZERO -5
#define ZBM_CODE_PTR_AFTER_TBL -6
#define ZBM_CODE_SIZE_TOO_SMALL -7
#define ZBM_CODE_ENDS_AFTER_EOF -8
#define ZBM_BAD_RESET_JUMP -9
#define ZBM_BAD_HARDWARE -10
#define ZBM_BAD_VER_PTR -11
#define ZBM_BAD_CHECKSUM -12
#define ZBM_BAD_CRC -13
#define ZBM_BAD_SHA256 -14
#define ZBM_BAD_TBL_TYPE -15
#define ZBM_CODE_PTR_NULL -16
#define ZBM_CRC_TIMEOUT -17
#define ZBM_ERROR_MIN ZBM_CRC_TIMEOUT

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/
void dsm_mtp_set_fsm(uint8_t state);
uint8_t dsm_mtp_get_fsm(void);
void dsm_mtp_set_op_mode(uint8_t mode);
uint8_t dsm_mtp_get_op_mode(void);
void dsm_mtp_init(void);
void dsm_mtp_rx_process(uint8_t rsp, uint8_t length, uint8_t* p_body);
uint32_t dsm_mtp_fsm_tx_proc(void);
uint32_t dsm_mtp_fsm_rx_proc(uint32_t evt);
ST_MIF_CAL_DATA* dsm_mtp_get_cal_data(void);
ST_MTP_PUSH_DATA* dsm_mtp_get_push_data(void);
void dsm_mtp_default_cal_point(ST_MTP_CAL_POINT* pst_mtp_cal_point);
void dsm_mtp_default_parm(ST_MTP_PARM* pst_mtp_parm);
void dsm_mtp_default_sagswell(ST_MTP_SAGSWELL* pst_mtp_sagswell);
void dsm_mtp_tx_retry_TO_proc(void);
ST_MIF_METER_PARM* dsm_mtp_get_meter_parm(void);
uint8_t dsm_mtp_get_fw_type(void);
void dsm_mtp_set_fw_type(uint8_t mode);
uint8_t dsm_mtp_get_fw_index(void);
void dsm_mtp_set_fw_index(uint8_t mode);
uint8_t dsm_mtp_get_curr_running_bank(void);
bool dsm_mtp_is_valid_curr_runnning_bank(void);
void dsm_mtp_fsm_tx_proc_fw_data_get_currunningbank(void);
void dsm_mtp_fsm_tx_proc_fw_data_get(uint8_t type, uint8_t idx);
bool dsm_mtp_fsm_tx_proc_fw_data_set(uint8_t type, uint8_t idx);
void dsm_mtp_fsm_tx_proc_fw_data_act_runbank(uint8_t bank);
uint16_t dsm_mtp_rx_fw_set_data(uint8_t length, uint8_t* p_body);
void dsm_set_meter_fw_down_complete(uint8_t val);
uint8_t dsm_get_meter_fw_down_complete(void);

#endif /* __AMG_MTP_PROCESS_H__ */
