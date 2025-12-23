#ifndef SOFTIMER_H
#define SOFTIMER_H 1

#include "options.h"

#define TICK_PERIOD 10  // 10 msec

/////////////   Time definition   ////////////////////////////
#define T10MS (10 / TICK_PERIOD)
#define T20MS (20 / TICK_PERIOD)
#define T25MS (25 / TICK_PERIOD)
#define T30MS (30 / TICK_PERIOD)
#define T50MS (50 / TICK_PERIOD)
#define T60MS (60 / TICK_PERIOD)
#define T70MS (70 / TICK_PERIOD)
#define T100MS (100 / TICK_PERIOD)
#define T110MS (110 / TICK_PERIOD)
#define T180MS (180 / TICK_PERIOD)
#define T200MS (200 / TICK_PERIOD)
#define T300MS (300 / TICK_PERIOD)
#define T400MS (400 / TICK_PERIOD)
#define T500MS (500 / TICK_PERIOD)
#define T600MS (600 / TICK_PERIOD)
#define T700MS (700 / TICK_PERIOD)
#define T800MS (800 / TICK_PERIOD)
#define T900MS (900 / TICK_PERIOD)
#define T1500MS (1500 / TICK_PERIOD)

#define T1SEC (1000 / TICK_PERIOD)
#define T1_5SEC (1500 / TICK_PERIOD)
#define T2SEC (2000 / TICK_PERIOD)
#define T3SEC (3000 / TICK_PERIOD)
#define T4SEC (4000 / TICK_PERIOD)
#define T5SEC (5000 / TICK_PERIOD)
#define T6SEC (6000 / TICK_PERIOD)
#define T7SEC (7000 / TICK_PERIOD)
#define T7_5SEC (7500 / TICK_PERIOD)
#define T8SEC (8000 / TICK_PERIOD)
#define T9SEC (9000 / TICK_PERIOD)

#define T10SEC (10000 / TICK_PERIOD)
#define T20SEC (20000 / TICK_PERIOD)
#define T30SEC (30000 / TICK_PERIOD)
#define T60SEC (60000 / TICK_PERIOD)
#define T120SEC (120000L / TICK_PERIOD)
#define T240SEC (240000L / TICK_PERIOD)
#define T300SEC (300000L / TICK_PERIOD)

enum
{
    E_TMR_SAG,
    E_TMR_QMON,
    E_TMR_TX_DELAY,
    E_TMR_COMM_INACT,
    E_TIMR_RELAY_DLY,
    E_TIMR_RELAY_ERR,
    E_TMR_DSP_UPDATE,
    E_TMR_DSP_INSTATE,
    E_TMR_DSP_BLINK,
    E_TMR_DSP_MDCHG,
    E_TMR_DSP_MDEXIT,
    E_TMR_TS_MDEXIT,
    E_TMR_DSP_DR,
    E_TMR_DSP_LOADDOT,
    E_TMR_CALKEY_SKIP,
    E_TMR_TAMP_DET,
    E_TMR_VBAT_MON,
    E_TMR_KEY_RD,
    E_TMR_KEY_CONT,
    E_PLS_INC,
    E_COMM_KEY,
    E_TMR_SAG_SWELL,
    E_AUX_ADC_CHECK,
    E_DELAYED_ACT,
    E_PROTECT_RELEASE,
    E_MIN_MAX,
    E_TMR_TMP_MEAS,
    E_TMR_INTER_FRAME,
    E_TMR_DSP_R_SUN,
    E_TMR_DSP_ON_SUN,
    E_TMR_DSP_COMM_ING,
    E_TMR_DSP_CAL_ING,
    E_TMR_DSP_CAL_ST,
    E_TMR_MET_FWUP_ING,
    E_TMR_INIT_MIF_TASK,
    E_MmodeCHG_sr_dr,
#if 1                           // jp.kim 25.02.04
    E_TMR_PARM_SET_WAIT_MIF,    // 25.02.04 jp.kim
    E_TMR_SAGSWE_SET_WAIT_MIF,  // 25.03.12 jp.kim
#endif
    E_TMR_DSP_CAL_END,
    E_TMR_DSP_SUN_VER_ERR,
    NUM_TMR
};

#define sag_recovery_is_timeout() (timer_table[E_TMR_SAG] == 0)
#define sag_recovery_timeset(x) (timer_table[E_TMR_SAG] = x)
#define qmon_is_timeout() (timer_table[E_TMR_QMON] == 0)
#define qmon_timeset(x) (timer_table[E_TMR_QMON] = x)
#define txdly_is_timeout() (timer_table[E_TMR_TX_DELAY] == 0)
#define txdly_timeset(x) (timer_table[E_TMR_TX_DELAY] = x)
#define comminact_is_timeout() (timer_table[E_TMR_COMM_INACT] == 0)
#define comm_inact_timeset(x) (timer_table[E_TMR_COMM_INACT] = x)
extern uint32_t scurr_mon_timer;
extern uint32_t scurr_limit_timer;
#define scurr_mon_is_timeout() (scurr_mon_timer == 0)
#define scurr_mon_timeset(x) (scurr_mon_timer = (1000L / TICK_PERIOD) * (x))
#define scurr_limit_is_timeout() (scurr_limit_timer == 0)
#define scurr_limit_timeset(x) \
    (scurr_limit_timer =       \
         (1000L / TICK_PERIOD) * (x + 2))  // +2 초 => refer to spec
#define relay_dly_is_timeout() (timer_table[E_TIMR_RELAY_DLY] == 0)
#define relay_dly_timeset(x) (timer_table[E_TIMR_RELAY_DLY] = x)
#define relay_err_is_timeout() (timer_table[E_TIMR_RELAY_ERR] == 0)
#define relay_err_timeset(x) (timer_table[E_TIMR_RELAY_ERR] = x)
#define dsp_update_is_timeout() (timer_table[E_TMR_DSP_UPDATE] == 0)
#define dsp_update_timeset(x) (timer_table[E_TMR_DSP_UPDATE] = x)
#define dsp_instate_is_timeout() (timer_table[E_TMR_DSP_INSTATE] == 0)
#define dsp_instate_timeset(x) (timer_table[E_TMR_DSP_INSTATE] = x)
#define dsp_blink_is_timeout() (timer_table[E_TMR_DSP_BLINK] == 0)
#define dsp_blink_timeset(x) (timer_table[E_TMR_DSP_BLINK] = x)
#define dsp_mdchg_is_timeout() (timer_table[E_TMR_DSP_MDCHG] == 0)
#define dsp_mdchg_timeset(x) (timer_table[E_TMR_DSP_MDCHG] = x)
#define dsp_dr_is_timeout() (timer_table[E_TMR_DSP_DR] == 0)
#define dsp_dr_timeset(x) (timer_table[E_TMR_DSP_DR] = x)
#define dsp_mdexit_is_timeout() (timer_table[E_TMR_DSP_MDEXIT] == 0)
#define dsp_mdexit_timeset(x) (timer_table[E_TMR_DSP_MDEXIT] = x)
#define ts_mdexit_is_timeout() (timer_table[E_TMR_TS_MDEXIT] == 0)
#define ts_mdexit_timeset(x) (timer_table[E_TMR_TS_MDEXIT] = x)
#define dsp_load_dot_is_timeout() (timer_table[E_TMR_DSP_LOADDOT] == 0)
#define dsp_load_dot_timeset(x) (timer_table[E_TMR_DSP_LOADDOT] = x)
#define calkey_skip_is_timeout() (timer_table[E_TMR_CALKEY_SKIP] == 0)
#define calkey_skip_timeset(x) (timer_table[E_TMR_CALKEY_SKIP] = x)
#define tamp_det_is_timeout() (timer_table[E_TMR_TAMP_DET] == 0)
#define tamp_det_timeset(x) (timer_table[E_TMR_TAMP_DET] = x)
#define vbat_step_is_timeout() (timer_table[E_TMR_VBAT_MON] == 0)
#define vbat_step_timeset(x) (timer_table[E_TMR_VBAT_MON] = x)
#define key_read_is_timeout() (timer_table[E_TMR_KEY_RD] == 0)
#define key_read_timeset(x)      \
    (timer_table[E_TMR_KEY_RD] = \
         x) /* 사용되는 곳이 없어서 Wake-Up 시 버튼 인식 개선에 사용함. */
#define key_cont_is_timeout() (timer_table[E_TMR_KEY_CONT] == 0)
#define key_cont_timeset(x) (timer_table[E_TMR_KEY_CONT] = x)
#define pls_inc_is_timeout() (timer_table[E_PLS_INC] == 0)
#define pls_inc_timeset(x) (timer_table[E_PLS_INC] = x)

#define comm_key_is_timeout() (timer_table[E_COMM_KEY] == 0)
#define comm_key_timeset(x) (timer_table[E_COMM_KEY] = x)
#define sagswell_start_is_timeout() (timer_table[E_TMR_SAG_SWELL] == 0)
#define sagswell_start_timeset(x)   \
    (timer_table[E_TMR_SAG_SWELL] = \
         x) /* 사용되는 곳이 없어서 작업 정전 이벤트에 사용함. */
#define auxadc_chk_is_timeout() (timer_table[E_AUX_ADC_CHECK] == 0)
#define auxadc_chk_timeset(x) (timer_table[E_AUX_ADC_CHECK] = x)
#define delayed_act_is_timeout() (timer_table[E_DELAYED_ACT] == 0)
#define delayed_act_timeset(x) (timer_table[E_DELAYED_ACT] = x)
#define protect_rel_timeout() (timer_table[E_PROTECT_RELEASE] == 0)
#define protect_rel_timeset(x) (timer_table[E_PROTECT_RELEASE] = x)
#define min_max_is_timeout() (timer_table[E_MIN_MAX] == 0)
#define min_max_timeset(x) (timer_table[E_MIN_MAX] = x)
#define tmp_meas_is_timeout() (timer_table[E_TMR_TMP_MEAS] == 0)
#define tmp_meas_timeset(x) (timer_table[E_TMR_TMP_MEAS] = x)
#define inter_frame_is_timeout() (timer_table[E_TMR_INTER_FRAME] == 0)
#define inter_frame_timeset(x) (timer_table[E_TMR_INTER_FRAME] = x)
#define dsp_r_sun_is_timeout() (timer_table[E_TMR_DSP_R_SUN] == 0)
#define dsp_r_sun_timeset(x) (timer_table[E_TMR_DSP_R_SUN] = x)
#define dsp_on_sun_is_timeout() (timer_table[E_TMR_DSP_ON_SUN] == 0)
#define dsp_on_sun_timeset(x) (timer_table[E_TMR_DSP_ON_SUN] = x)
#define dsp_comm_is_ing_timeout() (timer_table[E_TMR_DSP_COMM_ING] == 0)
#define dsp_comm_is_ing_timeset(x) (timer_table[E_TMR_DSP_COMM_ING] = x)
#define dsp_cal_mode_is_ing_timeout() (timer_table[E_TMR_DSP_CAL_ING] == 0)
#define dsp_cal_mode_is_ing_timeset(x) (timer_table[E_TMR_DSP_CAL_ING] = x)
#define dsp_cal_st_is_ing_timeout() (timer_table[E_TMR_DSP_CAL_ST] == 0)
#define dsp_cal_st_is_ing_timeset(x) (timer_table[E_TMR_DSP_CAL_ST] = x)
#define dsp_cal_end_is_ing_timeout() (timer_table[E_TMR_DSP_CAL_END] == 0)
#define dsp_cal_end_is_ing_timeset(x) (timer_table[E_TMR_DSP_CAL_END] = x)
#define dsp_sun_ver_err_is_ing_timeout() \
    (timer_table[E_TMR_DSP_SUN_VER_ERR] == 0)
#define dsp_sun_ver_err_ing_timeset(x) (timer_table[E_TMR_DSP_SUN_VER_ERR] = x)
#define meter_firmup_delay_timeout() (timer_table[E_TMR_MET_FWUP_ING] == 0)
#define meter_firmup_delay_timeset(x) (timer_table[E_TMR_MET_FWUP_ING] = x)
#define init_mif_task_timeout() (timer_table[E_TMR_INIT_MIF_TASK] == 0)
#define init_mif_task_timeset(x) (timer_table[E_TMR_INIT_MIF_TASK] = x)
#if 1  // jp.kim 25.02.04
#define mif_meter_para_set_wait_timeout() \
    (timer_table[E_TMR_PARM_SET_WAIT_MIF] == 0)
#define mif_meter_para_set_wait_timeset(x) \
    (timer_table[E_TMR_PARM_SET_WAIT_MIF] = x)
#endif
#if 1  // jp.kim 25.03.12
#define mif_meter_sagswe_set_wait_timeout() \
    (timer_table[E_TMR_SAGSWE_SET_WAIT_MIF] == 0)
#define mif_meter_sagswe_set_wait_timeset(x) \
    (timer_table[E_TMR_SAGSWE_SET_WAIT_MIF] = x)
#endif
#define MmodeCHG_sr_dr_type_sr_dr_is_timeout() \
    (timer_table[E_MmodeCHG_sr_dr] == 0)
#define MmodeCHG_sr_dr_type_sr_dr_time_set(x) \
    (timer_table[E_MmodeCHG_sr_dr] = x)

extern int timer_table[NUM_TMR];
extern int sag_exit_timer;
extern uint8_t b_sag_exit_timer;

void init_softtmr(void);
void timer_process(void);
void softtimer_set(uint32_t timer_id, uint32_t tick);
#endif
