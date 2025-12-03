#include <string.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "options.h"  // Define the IC, CE code and other options.
#include "batmode.h"  // To use battery mode code.
#include "rtc.h"      // Real time clock.
#include "ser.h"      // Serial I/O.
#ifdef SER_1
#include "ser1.h"  // Serial I/O driver for UART 1
#endif
#include "meter.h"  // Meter API and data structures.
#include "afe.h"    // To read the AFE.
#include "bat.h"    // To read the batteries.
#include "tmp.h"    // To read the temperature.
#include "irq.h"    // Interrupt controls.
#if 0               /* bccho, LCD, 2023-07-15 */
#include "lcd.h"    // To write to the LCD.
#endif              /* bccho */
#if 0               /* bccho, FLASH, 2023-07-15 */
#include "flash.h"  // To read and write the flash.
#endif              /* bccho */
#include "delay.h"  // A calibrated delay.
#include "cal.h"
#include "eeprom_at24cm02.h"
#include "pulse_src.h"
#include "nv.h"
#include "whm_1.h"
#include "meter_app.h"
#include "dbgprintf.h"
#include "port.h"
#include "get_req.h"
#include "amg_mtp_process.h"
#include "amg_pwr.h"
#include "amg_power_mnt.h"
#include "amg_wdt.h"
#include "amg_media_mnt.h"

#define _D "[METER] "

/* bccho, 2024-06-17, 김종필대표 패치 적용 */
extern ST_MIF_SAGSWELL_SETUP g_mtp_sagswell;

/* bccho, 2024-09-25 */
extern ST_MIF_METER_PARM g_mtp_meter_parm;

extern float PULSE_ADD_MODIFY_DATA, PULSE_ADD_MODIFY_DATA_VA,
    PULSE_ADD_MODIFY_DATA_VAR;
extern bool METER_FW_UP_ING_STS;
extern bool METER_FW_UP_END_PULSE_MODIFY;
extern U16 METER_FW_UP_ING_CNT;
extern U8 PULSE_DIR_MODIFY_BACK;

extern int8_t no_inst_curr_chk_zon_cnt;

extern ST_MTP_CAL_DATA monitor_mtp_caldata;
extern ST_MIF_METER_PARM monitor_mtp_meter_parm;
extern ST_MIF_SAGSWELL_SETUP monitor_mtp_sagswell;

bool init_mif_task_set = 0;
U8 init_mif_task_seq = 0;
bool init_mif_task_firm_up = 0;
bool cal_monitor_only = 0;

typedef enum
{
    init_mif_none_set,
    cal_data_set,
    cal_data_get,
    cal_data_cmp,

    mif_sagswell_set,
    mif_sagswell_get,
    mif_sagswell_cmp,

    // jp.kim 25.03.12
    mif_parm_set,
    mif_parm_get,
    mif_parm_cmp,

    mif_firm_ware_check,
    mif_firm_ware_get,
    i_modem_bearer_q,  // JP.KIM 25.01.17
    e_modem_bearer_q   // JP.KIM 25.01.17
} _Init_mif_task_t;

uint32_t state;

bool xfer_done = false;     // XFER done, from CE interrupt.
uint16_t ce_interrupt = 0;  // CE Interrupt Status
uint16_t xfer_done_timer =
    0;  // should be unsigned type (in case of overflow counter)
uint16_t xfer_skipped_cnt = 0;

outputs_mon_type outputs_mon;
rms_mon_type rms_mon;

bool start_curr_statusA;
bool start_curr_statusB;
bool start_curr_statusC;

bool no_load_statusA;
bool no_load_statusB;
bool no_load_statusC;

static bool sagswell_started = false;

uint32_t LineStatus;
mt_acc_type mt_accm;

bool b_minmax_ready;
float min_freq, max_freq;
int minfreq_cnt, maxfreq_cnt;

#if 1 /* bccho, 2024-09-05, 삼상 */
float min_volt[3], max_volt[3];
int minvolt_cnt[3], maxvolt_cnt[3];
#else
float min_volt, max_volt;
int minvolt_cnt, maxvolt_cnt;
#endif

bool b_emb_display_ok = false;
bool tmp_read_ready = false;

bool b_mt_printout = false;
bool b_mt_print_ready = false;

bool sag_swell_enabled = false;
int sag_swell_delayed;
bool sag_det_state;
bool swell_det_state;
int sag_det_cnt;
int swell_det_cnt;

bool b_lp_kwh_ready = false;
bool b_imax_ready = false;
bool b_temp_mon_ready = false;
bool b_scurr_mon_ready = false;
bool b_abnormal_chk_ready = false;

U8 g_avg3_index = 0;
float g_v_thd_avg_buf[3][3] = {
    0,
};

uint16_t ln_mrcnt_idx;

int lpsts_intv_mon;
uint32_t lpsts_cntidx;

uint8_t lcd_pls_quart_skip_cnt;
volatile bool clock_idle_set = true;

uint8_t g_ac_power_fromONtoOff = FALSE;
uint8_t g_ac_power_on_pre_state = AC_POWER_ON;

#if 1  // JP.KIM 25.01.17
bool atcmd_modem_bearer_q_task_set = 0;
void ami_atcmd_get_bearer(uint32_t poll_flag);
#endif

void meter_status_set(void);
void outputs_for_mon(void);
int32_t VARh_WVA(int32_t w0, int32_t v0);
void Monitor_MinMax(void);
uint16_t get_mr_idx(uint8_t grp_f);
void mt_print_out(void);
void is_sCurr_rev_cur_to_bidir(uint8_t* tptr);
void whm_op_sag_save(void);
void push_data_user_back_up_data_for_meter_fwup(void);
bool meter_fwup_delay_sag_protect(void);
bool meter_fwup_ing(void);

float fabs_m(float fval)
{
    if (0 < fval)
        return fval;
    else
        return -fval;
}

void afe_init_start(void)
{
    DPRINTF(DBG_TRACE, "%s \r\n", __func__);
    no_load_chk_zon_cnt = 3;
}

void dsm_mtp_default_cal_data(U8* tptr)
{
    cal_data_type* cal;
    bool ok;
    ST_MIF_CAL_DATA* p_mtp_caldata = dsm_mtp_get_cal_data();

    cal = (cal_data_type*)tptr;

    cal->T_cal_i0 = 0x40000000;
    cal->T_cal_v0 = 0x40000000;
    cal->T_cal_p0 = 0x00000000;

    cal->T_cal_i1 = 0x40000000;
    cal->T_cal_v1 = 0x40000000;
    cal->T_cal_p1 = 0x00000000;

    cal->T_cal_i2 = 0x40000000;
    cal->T_cal_v2 = 0x40000000;
    cal->T_cal_p2 = 0x00000000;

    DPRINTF(DBG_TRACE, _D "%s: OK\r\n", __func__);

    nv_write(I_CAL_DATA, tptr);
}

// -------- meter task --------------

extern void dsm_mtp_fsm_send(void);

void meter_task_init(void)
{
    uint8_t* tptr;
    tptr = adjust_tptr(&global_buff[0]);

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    dsm_wdt_ext_toggle_immd();
    state = 0;

    xfer_done = false;
    extern uint8_t afe_up;
    afe_up = 0;
    no_load_chk_zon_cnt = 3;  // 무부하 동작 관련 수정
    starting_cnt = 3;         // ce_val_drop() 값과 동일하게 맞춤
    no_load_chk_zon_cnt = 5;  // 6;
    sag_detect_mask_zon_cnt = 5;
    low_volt_detect_mask_zon_cnt = 8;

    ce_interrupt = 0;  // CE Interrupt Status

    b_emb_display_ok = false;
    tmp_read_ready = false;

    start_curr_statusA = false;
    start_curr_statusB = false;
    start_curr_statusC = false;

    no_load_statusA = true;
    no_load_statusB = true;
    no_load_statusC = true;

    sagswell_started = false;
    sag_swell_disable();  // enabled later
    sag_det_state = swell_det_state = 0;
    sag_det_cnt = swell_det_cnt = 0;

    LineStatus = 0L;

    minfreq_cnt = 0;
    maxfreq_cnt = 0;

#if 1 /* bccho, 2024-09-05, 삼상 */
    minvolt_cnt[0] = 0;
    maxvolt_cnt[0] = 0;
    minvolt_cnt[1] = 0;
    maxvolt_cnt[1] = 0;
    minvolt_cnt[2] = 0;
    maxvolt_cnt[2] = 0;
#else
    minvolt_cnt = 0;
    maxvolt_cnt = 0;
#endif

    b_minmax_ready = false;
    b_mt_print_ready = false;
    // varialble initialization(IDATA) is unsatable
    b_mt_printout = false;

    lcd_pls_quart_skip_cnt = 2;  // 의미 없음....
#if 0
	cal_point_init();
#endif
    afe_init();

    if (cal_restore(tptr))
    {
        dsm_mtp_set_op_mode(MTP_OP_NORMAL);
        dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
        dsm_mtp_fsm_send();
    }
    else
    {
        dsm_mtp_default_cal_data(tptr);

#if 1  // jp.kim 25.04.26
        cal_restore(tptr);
#endif
    }

    wrate_mpu_set();

    pcnt_variable_init();

    dsm_wdt_ext_toggle_immd();
}

uint32_t meter_get_measure_flag(void) { return xfer_done; }

void meter_set_measure_flag(uint32_t flag) { xfer_done = flag; }
uint32_t meter_get_measure_xdone_timer(void) { return xfer_done_timer; }

void meter_set_measure_xdone_timer(uint32_t value) { xfer_done_timer = value; }

bool ac_power_is_failed(bool print_enable)
{
    if (IS_AC_OFF && !meter_fwup_delay_sag_protect()) /* AC파워 OFF */
    {
        /* Power Fail */
        if (m_ac_power_is_high())
        {
            m_ac_power_off_trigger();
        }
        else
        {
            m_ac_power_off_not_trigger();
        }

        m_ac_power_set_low();

        if (print_enable)
        {
            if (m_ac_power_off_is_not_trigger())
            {
                static uint32_t pwr_fail_state = 0;
                if ((pwr_fail_state++ % 100) == 0)
                {
                    DPRINTF(DBG_TRACE,
                            "POWER_FAIL: 0 - run_pwr[%s], pmnt[%s], "
                            "off-off[%d]\r\n",
                            dsm_run_power_string(dsm_run_power_get()),
                            dsm_pmnt_op_mode_string(dsm_pmnt_get_op_mode()),
                            g_ac_power_fromONtoOff);
                }
            }
            else
            {
                DPRINTF(DBG_TRACE,
                        "POWER_FAIL: 0 - run_pwr[%s], pmnt[%s], on-off[%d]\r\n",
                        dsm_run_power_string(dsm_run_power_get()),
                        dsm_pmnt_op_mode_string(dsm_pmnt_get_op_mode()),
                        g_ac_power_fromONtoOff);
            }
        }

        return true;
    }
    else /* AC파워 ON */
    {
        m_ac_power_off_not_trigger();
        m_ac_power_set_high();

        return false;
    }
}

void ac_power_polling_check(void)
{
    if ((IS_AC_ON || meter_fwup_delay_sag_protect()) && run_is_main_power())
    { /*ac_power_on 및 main power 인 상황에서 power off trigger 를 위해서 임으로
         high 를 설정한다*/
        m_ac_power_set_high();
    }
}

/* 배터리 동작: true */
bool cests_is_sag(void)
{
    if (is_dc33_off())
        return true;
    else
        return false;
}

void ce_val_drop(void)
{
    starting_cnt = 3;
    no_load_chk_zon_cnt = 3;
    sag_detect_mask_zon_cnt = 5;
    low_volt_detect_mask_zon_cnt = 8;
    xfer_done = false;
}

#define SAG_DURATION 60
#define AFE_POWER_OFF_DELAY (280 / 10)  //  280ms/10ms  //최대값 29까지만 가능
void dsm_atcmd_set_trap_power_notify(uint32_t poll_flag, uint8_t sel);

void sag_detect_proc(void)
{
    Work_PwrF_data_type _val_flag;
    uint32_t LP_event_back, sag_cnt, i;
    uint32_t op = dsm_pmnt_get_op_mode();
    uint8_t* tptr = adjust_tptr(&global_buff[0]);

    if (IS_AC_ON || meter_fwup_delay_sag_protect())
    {
        dsm_gpio_imodem_pf_high(); /* PF unset */
    }

    ac_power_polling_check();

    if (run_is_main_power())
    {
        if ((is_dc33_off() || (IS_AC_OFF && !meter_fwup_delay_sag_protect())) &&
            op == PMNT_ACTIVE_OP)
        {
            sag_exit_timer = T5SEC;
            b_sag_exit_timer = 1;

            // ac_power_fail 이 처음으로 발생하는 경우에만 nv 저장 및 push 수행
            if (ac_power_is_failed(TRUE) && m_ac_power_off_is_trigger())
            {
                MSGALWAYS("####   Sag Detected, PB.0:%d, PA.10:%d", PB0, PA10);

                m_ac_power_off_not_trigger();

                lp_event_unset(LPE_VOL_LOW);
                whm_data_save_sag();

                dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
                dsm_atcmd_set_trap_power_notify(FALSE, '0');

                if (sagswell_start_is_timeout())
                {
                    _val_flag._val = 'F';
                }
                else
                {
                    _val_flag._val = 'T';
                }
                nv_write(I_WORKPWR_FLAG, (U8*)&_val_flag);

                /* (SAG_DURATION * 10) ms 동안 AC파워 체크 --> SAG 지속 체크 */
                sag_cnt = 0;
                for (i = 0; i < (SAG_DURATION - AFE_POWER_OFF_DELAY); i++)
                {
                    sag_cnt++;
                    vTaskDelay(10);

                    if (sag_cnt == (30 - AFE_POWER_OFF_DELAY))
                    {
                        if (sagswell_start_is_timeout())
                        {
                            PWR_OFF_trigger();
                        }
                        else
                        {
                            /* 작업 정전 */
                            Work_BlackOut_trigger();
                        }
                    }

                    /* 1이면 AC파워 회복 */
                    if (IS_AC_ON || meter_fwup_delay_sag_protect())
                        i = 1000;
                }

                /* 정전 검출 조건: 기준전압의 80% 미만 600msec 이상 지속 시
                (검출 후 PUSH 전송을 완료해야 함) */
                if (sag_cnt >= (60 - AFE_POWER_OFF_DELAY))
                {
                    dsm_gpio_imodem_pf_low();

                    ce_val_drop();

                    MSGALWAYS("####   Sag Timeout, PB0:%d, PA10:%d", PB0, PA10);

                } /* 진짜 SAG 발생 */
            }

            while (1)
            {
                /* AC: OFF && VDD3.3: OFF */
                /* DC가 OFF 되어야 sleep 으로 빠진다 */
                if (is_dc33_off())
                {
                    dsp_pwr_fail();

                    /* bccho, 2023-12-07, PUSH 보내기 위한 delay */
                    vTaskDelay(100);

                    /* Sleep !!!!!! */
                    op = dsm_pmnt_EntryLowPwr_n_waitingForwakeup(
                        PWR_STOP_2,
                        (WUP_S_RTC_ALA | WUP_S_MENU_KEY | WUP_S_MAIN_POWER));

                    dsp_up_pwr_on_state();
                }

                /* ----------> */
                /* AC는 OFF 이지만 DC가 살아있는 경우: 전압이 174V 이하인 상태
                 */

                OSTimeDly(OS_MS2TICK(10));

                if (!is_dc33_off() && op == PMNT_ACTIVE_OP)
                {
                    /* AC파워 회복 */
                    if (IS_AC_ON || meter_fwup_delay_sag_protect())
                    {
                        MSG07("####   AC recover, PB0:%d, PA10:%d", PB0, PA10);
                        dsp_debug_state(1);
                        ce_val_drop();
                    }

                    MSG00("####   SAG but VDD3.3 OK --> break ####");
                    break;
                }
                else
                {
                    if (op == PMNT_NO_VOLT_OP)
                    {
                        break;
                    }

                    if (b_sag_exit_timer == 0)
                    {
                        dsp_debug_state(2);
                    }
                }
            } /* while(1) */

            /* AC파워 회복 */
            if (IS_AC_ON || meter_fwup_delay_sag_protect())
            {
                dm_intv_nvdelete(tptr);
            }
        }
        /** 메인파워 모드에서
              -> SAG가 발생하지 않았거나
              -> PMNT_ACTIVE_OP가 아닌 경우 */
        /* else of .... if ((cests_is_sag() || (!IS_AC_ON)) && op
                        == PMNT_ACTIVE_OP) */
        else
        {
            if (is_dc33_off()) /* 메인전원OFF && VDD3.3OFF */
            {
                port_inmode();

                dsp_low_pwr_entry_state(2);

                dsm_pmnt_EntryLowPwr_n_waitingForwakeup(
                    PWR_STOP_2,
                    (WUP_S_RTC_ALA | WUP_S_MENU_KEY | WUP_S_MAIN_POWER));

                dsp_up_pwr_on_state();
            }
        }
    }
    else /* run_is_bat_power() */
    {
        if (is_dc33_off()) /* 메인전원OFF && VDD3.3OFF */
        {
            /* 무전압설정 */
            if (PMNT_NO_VOLT_SET_OP == dsm_pmnt_get_op_mode())
            {
                uint32_t evt = dsm_pmnt_get_wakeup_dn_evt();

                if ((evt & WDN_S_NOVOLTSET_COMM_x) ||
                    (evt & WDN_S_NOVOLTSET_DISC))
                {
                    dsm_pmnt_fsm(evt);

                    port_inmode();

                    dsm_pmnt_EntryLowPwr_n_waitingForwakeup(
                        PWR_STOP_2,
                        (WUP_S_RTC_ALA | WUP_S_MENU_KEY | WUP_S_MAIN_POWER));
                }
            }
            else /* 무전압검침 또는 재고관리 */
            {
                if (b_dsp_bm_finished)
                {
                    port_inmode();

                    dsm_pmnt_EntryLowPwr_n_waitingForwakeup(
                        PWR_STOP_2,
                        (WUP_S_RTC_ALA | WUP_S_MENU_KEY | WUP_S_MAIN_POWER));
                }
            }
        }
        else /* 배터리 동작중인데 -> !is_dc33_off() */
        {
#if 1 /* bccho, 2023-08-23 */
            goto_loader_S();
#endif
        }
    } /* end of___run_is_bat_power() */
}

uint32_t get_LineStatus(void) { return LineStatus; }

bool cal_data_wr_rd_cmp(void)
{
#if PHASE_NUM == SINGLE_PHASE
    if ((monitor_mtp_caldata.r_current_gain == g_mtp_caldata.r_current_gain) &&
        (monitor_mtp_caldata.r_voltage_gain == g_mtp_caldata.r_voltage_gain) &&
        (monitor_mtp_caldata.r_phase_gain == g_mtp_caldata.r_phase_gain))
    {
        return TRUE;
    }
#else
    if ((monitor_mtp_caldata.r_current_gain == g_mtp_caldata.r_current_gain) &&
        (monitor_mtp_caldata.r_voltage_gain == g_mtp_caldata.r_voltage_gain) &&
        (monitor_mtp_caldata.r_phase_gain == g_mtp_caldata.r_phase_gain) &&
        (monitor_mtp_caldata.s_current_gain == g_mtp_caldata.s_current_gain) &&
        (monitor_mtp_caldata.s_voltage_gain == g_mtp_caldata.s_voltage_gain) &&
        (monitor_mtp_caldata.s_phase_gain == g_mtp_caldata.s_phase_gain) &&
        (monitor_mtp_caldata.t_current_gain == g_mtp_caldata.t_current_gain) &&
        (monitor_mtp_caldata.t_voltage_gain == g_mtp_caldata.t_voltage_gain) &&
        (monitor_mtp_caldata.t_phase_gain == g_mtp_caldata.t_phase_gain))
    {
        return TRUE;
    }
#endif

    return FALSE;
}

bool sagswell_wr_rd_cmp(void)
{
    if ((memcmp(&monitor_mtp_sagswell.sag_level[0],
                &g_mtp_sagswell.sag_level[0], 4) == 0) &&
        (monitor_mtp_sagswell.sag_time == g_mtp_sagswell.sag_time) &&
        (memcmp(&monitor_mtp_sagswell.swell_level[0],
                &g_mtp_sagswell.swell_level[0], 4) == 0) &&
        (monitor_mtp_sagswell.swell_time == g_mtp_sagswell.swell_time))

        return TRUE;

    return FALSE;
}

bool meter_parm_wr_rd_cmp(void)
{
    if ((monitor_mtp_meter_parm.direct_reverse ==
         g_mtp_meter_parm.direct_reverse) &&
        (monitor_mtp_meter_parm.reactive_select ==
         g_mtp_meter_parm.reactive_select) &&
        (monitor_mtp_meter_parm.meter_method ==
         g_mtp_meter_parm.meter_method) &&
        (monitor_mtp_meter_parm.pulse_select == g_mtp_meter_parm.pulse_select))

        return TRUE;

    return FALSE;
}

void init_mif_task_init(bool firm_up_sts)
{
    if (!is_dc33_off())
    {
        init_mif_task_set = 1;
        if (firm_up_sts)
            init_mif_task_firm_up = 1;
        else
            init_mif_task_firm_up = 0;
        init_mif_task_seq = cal_data_set;
        init_mif_task_timeset(T100MS);
    }
    else
    {
        init_mif_task_seq = 0;
        init_mif_task_set = 0;
        init_mif_task_firm_up = 0;
    }
}

#if 0
void init_mif_task_proc(void)
{
    if (init_mif_task_set)
    {
        switch (init_mif_task_seq)
        {
        case cal_data_set:
            dsm_mtp_set_fsm(MTP_FSM_CAL_SET);
            dsm_mtp_fsm_send();
            init_mif_task_timeset(T100MS);
            init_mif_task_seq++;
            break;
        case mif_parm_set:
            if (init_mif_task_timeout())
            {
                g_mtp_meter_parm.meter_method = meas_method;
                dsm_mtp_set_fsm(MTP_FSM_PARM_SET);
                dsm_mtp_fsm_send();
                init_mif_task_timeset(T100MS);
                init_mif_task_seq++;
            }
            break;
        case mif_sagwsell_set:
            if (init_mif_task_timeout())
            {
                dsm_mtp_set_fsm(MTP_FSM_SAG_SWELL);
                dsm_mtp_fsm_send();
                if (init_mif_task_firm_up)
                {
                    init_mif_task_timeset(T5SEC);
                    init_mif_task_seq++;
                }
                else
                {
                    init_mif_task_seq = 0;
                    init_mif_task_set = 0;
                }
            }
            break;

        case mif_firm_ware_get:
            if (init_mif_task_timeout())
            {
#if defined(FEATURE_MTP_FWVER_MANAGE)
                if (dsm_pmnt_get_op_mode() == PMNT_ACTIVE_OP)
                {
                    DPRINTF(DBG_TRACE, "\r\nmeter firmver get\r\n");
                    dsm_mif_getreq_firmware_ver_data();
                }
#endif
                init_mif_task_seq = 0;
                init_mif_task_set = 0;
                init_mif_task_firm_up = 0;
            }
            break;

        default:
            init_mif_task_seq = 0;
            init_mif_task_set = 0;
            init_mif_task_firm_up = 0;
            break;
        }
    }
}
#endif

void init_mif_task_proc(void)
{
    if (init_mif_task_set)
    {
        switch (init_mif_task_seq)
        {
        case cal_data_set:
            if ((dsm_mtp_get_fsm() != MTP_FSM_PARM_GET) &&
                (dsm_mtp_get_fsm() != MTP_FSM_PARM_SET))
            {
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_CAL_SET);
                dsm_mtp_fsm_send();
                init_mif_task_timeset(T500MS);  // 충분한 delay
                init_mif_task_seq++;
            }
            break;

        case cal_data_get:
            if (init_mif_task_timeout() &&
                (dsm_mtp_get_fsm() != MTP_FSM_PARM_GET) &&
                (dsm_mtp_get_fsm() != MTP_FSM_PARM_SET))

            {
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                cal_monitor_only = 1;
                dsm_mtp_set_fsm(MTP_FSM_CAL_GET);
                dsm_mtp_fsm_send();
                init_mif_task_timeset(T200MS);
                init_mif_task_seq++;
            }
            break;

        case cal_data_cmp:
            if (init_mif_task_timeout())
            {
                dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                init_mif_task_timeset(T200MS);
                if (!cal_data_wr_rd_cmp())
                {
                    init_mif_task_seq = cal_data_set;
                }
                else
                    init_mif_task_seq++;
            }
            break;

        case mif_sagswell_set:
            if (init_mif_task_timeout() &&
                (dsm_mtp_get_fsm() != MTP_FSM_PARM_GET) &&
                (dsm_mtp_get_fsm() != MTP_FSM_PARM_SET))

            {
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_SAG_SWELL);
                dsm_mtp_fsm_send();
                init_mif_task_timeset(T200MS);
                init_mif_task_seq++;
            }
            break;

        case mif_sagswell_get:
            if (init_mif_task_timeout() &&
                (dsm_mtp_get_fsm() != MTP_FSM_PARM_GET) &&
                (dsm_mtp_get_fsm() != MTP_FSM_PARM_SET))
            {
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_SAG_SWELL_GET);
                dsm_mtp_fsm_send();
                init_mif_task_timeset(T200MS);
                init_mif_task_seq++;
            }
            break;

        case mif_sagswell_cmp:
            if (init_mif_task_timeout())
            {
                dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                init_mif_task_timeset(T200MS);

                if (!sagswell_wr_rd_cmp())
                {
                    init_mif_task_seq = mif_sagswell_set;
                }
                else
                    init_mif_task_seq++;
            }
            break;

            // jp.kim 25.03.12
        case mif_parm_set:
            if (init_mif_task_timeout())
            {
                g_mtp_meter_parm.meter_method = meas_method;
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_PARM_SET);
                dsm_mtp_fsm_send();
                init_mif_task_timeset(T200MS);
                init_mif_task_seq++;
            }
            break;

        case mif_parm_get:
            if (init_mif_task_timeout())
            {
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_PARM_GET);
                dsm_mtp_fsm_send();
                init_mif_task_timeset(T200MS);
                init_mif_task_seq++;
            }
            break;

        case mif_parm_cmp:
            if (init_mif_task_timeout())
            {
                dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                init_mif_task_timeset(T200MS);
                if (!meter_parm_wr_rd_cmp())
                    init_mif_task_seq = mif_parm_set;
                else
                    init_mif_task_seq++;
            }
            break;

        case mif_firm_ware_check:
            if (init_mif_task_firm_up)
            {
                init_mif_task_timeset(T5SEC);
                init_mif_task_seq = mif_firm_ware_get;
            }
            else
            {
                if (atcmd_modem_bearer_q_task_set)
                {
                    atcmd_modem_bearer_q_task_set = 0;
                    init_mif_task_timeset(T100MS);
                    init_mif_task_seq = i_modem_bearer_q;
                }
                else
                {
                    init_mif_task_seq = 0;
                    init_mif_task_set = 0;
                }
            }
            break;

        case mif_firm_ware_get:
            if (init_mif_task_timeout())
            {
#if defined(FEATURE_MTP_FWVER_MANAGE)
                if (dsm_pmnt_get_op_mode() == PMNT_ACTIVE_OP)
                {
                    DPRINTF(DBG_TRACE, "\r\nmeter firmver get\r\n");
                    dsm_mif_getreq_firmware_ver_data();
                }
#endif
                if (atcmd_modem_bearer_q_task_set)
                {
                    atcmd_modem_bearer_q_task_set = 0;
                    init_mif_task_timeset(T100MS);
                    init_mif_task_seq = i_modem_bearer_q;
                    init_mif_task_firm_up = 0;
                }
                else
                {
                    init_mif_task_seq = 0;
                    init_mif_task_set = 0;
                    init_mif_task_firm_up = 0;
                }
            }
            break;

        case i_modem_bearer_q:
            if (init_mif_task_timeout())
            {
                dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
                ami_atcmd_get_bearer(FALSE);
                init_mif_task_timeset(T100MS);
                init_mif_task_seq = e_modem_bearer_q;
                init_mif_task_firm_up = 0;
            }
            break;

        case e_modem_bearer_q:
            if (init_mif_task_timeout())
            {
                dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
                ami_atcmd_get_bearer(FALSE);
                init_mif_task_timeset(T100MS);
                init_mif_task_seq = 0;
                init_mif_task_set = 0;
                init_mif_task_firm_up = 0;
            }

            break;

        default:
            init_mif_task_seq = 0;
            init_mif_task_set = 0;
            init_mif_task_firm_up = 0;
            break;
        }
    }
}

void meter_task(void)
{
    init_mif_task_proc();

#ifdef BATTERY_ACTIVE_OP /* bccho, 2023-11-26 */
                         /* do nothing */
#else
    sag_detect_proc();  // power fail check
#endif

    meter_run(); /* lhh_add_desc : metering 처리 */

    /* bccho, 2024-06-17, 김종필대표 패치 적용 */
    if (no_load_chk_zon_cnt <= 0)
    {
        Monitor_MinMax(); /* lhh_add_desc : 주파수, 전압 min, max 를
                             체크하여 NV에 저장한다. */
    }
}

extern bool b_first_afe_up;

void meter_run(void)
{
    bool meas_skip;

    uint8_t* tptr;
    tptr = adjust_tptr(&global_buff[0]);

    if (xfer_done)
    {
        xfer_done = false;
        if (xfer_done_timer >= T200MS)
        {
            xfer_skipped_cnt++;
            DPRINTF(DBG_INFO, _D "%s: xfer_skipped_cnt[%d]\r\n", __func__,
                    xfer_skipped_cnt);
            return;
        }

        afe_ready_data();
        calibrate(tptr);
        outputs_for_mon();
        meter_status_set();  // load and line state

        meas_skip = false;
        if (b_first_afe_up)
        {
            b_first_afe_up = false;

            if ((get_inst_power(true, 0) < (220 * BasicCur)) &&
                (get_inst_power(false, 0) < (220 * BasicCur)))
            {
                meas_skip = true;
            }
        }

        if (meas_skip == false)
        {
            if (METER_FW_UP_ING_STS)
            {
                if (METER_FW_UP_ING_CNT > (3 * 60))  // 3분 이상은 안함.
                {
                    METER_FW_UP_ING_CNT = 0;
                }
                else
                {
                    METER_FW_UP_ING_CNT++;
                }
            }

            SelectPulses();    // for software generated pulse outputs
            pcnt_accumulate(); /* lhh_add_desc : energy, dm etc are
                                * accumulated
                                */
        }

        is_sCurr_rev_cur_to_bidir(tptr);

        if (lcd_pls_quart_skip_cnt)
            lcd_pls_quart_skip_cnt -= 1;

        b_emb_display_ok = true;
        b_lpavg_ready = true;
        b_lprt_ready = true;
        b_imax_ready = true;
        b_temp_mon_ready = true;
        b_abnormal_chk_ready = true;
        b_minmax_ready = true;
        bat_test_ready = true;
        b_scurr_mon_ready = true;

        b_mt_print_ready = true;
        tmp_read_ready = true;
    }
}

#if PHASE_NUM == SINGLE_PHASE
void meter_status_set(void)
{
    int32_t t32;

    LineStatus = 0L;

#if 0
	if ((i0sqsum_mon == 0) || (no_load_chk_zon_cnt))	{
#else  // jp.kim 24.11.30
    if ((v0sqsum_mon < v_min_thrshld) || (i0sqsum_mon == 0) ||
        (no_load_chk_zon_cnt))
    {
#endif
        no_load_statusA = true;
    }
    else
    {
        no_load_statusA = false;
    }
    no_load_statusB = true;
    no_load_statusC = true;

    if (i0sqsum_mon > i_min_thrshldx2)
    {
        start_curr_statusA = true;
    }
    else
    {
        start_curr_statusA = false;
    }
    start_curr_statusB = false;
    start_curr_statusC = false;

    // ---------- current over ---------------
    if (i0sqsum_mon > i_over_thrshld)
    {
        LineStatus |= LA_IOVER;
    }

#if 0  // 단상에 없는 기능  
	// ------------ Volt Over 중성선 오결선 ------------
	if(v0sqsum_mon > v_over_thrshld) {
		LineStatus |= LA_VOVER;
	}
#endif
#if 0
	// ------------ Volt Min ------------
	if(v0sqsum_mon < v_min_thrshld) {
		LineStatus |= LA_VMIN;
	}
#endif
#if 0  // 단상에 없는 기능  
	// ------------ Volt Drop ------------
	if(v0sqsum_mon < VdropThrsld) {
		LineStatus |= LA_VDROP;
	}
#endif

    // ------------ Backward Load ----------
    if (condensor_en)
    {
        t32 = i_conden_thrshld;
    }
    else
    {
        t32 = 0L;
    }

    if (i0sqsum_mon >= t32 && w0sum_mon < 0)
    {
        LineStatus |= LA_WHBACK;
    }
}
#else /* bccho, 2024-09-05, 삼상 */
void meter_status_set(void)
{
    int32_t t32;

    LineStatus = 0L;

// R

//  -------- No load status ---------------
#if 0
	if ((i0sqsum_mon == 0) || (no_load_chk_zon_cnt))
#else  // jp.kim 24.11.30
    if ((v0sqsum_mon < v_min_thrshld) || (i0sqsum_mon == 0) ||
        (no_load_chk_zon_cnt))
#endif
    {
        no_load_statusA = true;
    }
    else
    {
        no_load_statusA = false;
    }

    // ---------- start current status ------------
    if (i0sqsum_mon > i_min_thrshldx2)
    {
        start_curr_statusA = true;
    }
    else
    {
        start_curr_statusA = false;
    }

    // ---------- current over ---------------
    if (i0sqsum_mon > i_over_thrshld)
    {
        LineStatus |= LA_IOVER;
    }

    // ------------ Volt Over ------------
    if (v0sqsum_mon > v_over_thrshld)
    {
        LineStatus |= LA_VOVER;
    }

#if 0
	// ------------ Volt Min ------------
	if(v0sqsum_mon < v_min_thrshld) {
		LineStatus |= LA_VMIN;
	}
#endif

    // ------------ Volt Drop ------------
    if (v0sqsum_mon < VdropThrsld)
    {
        LineStatus |= LA_VDROP;
    }

    // ------------ Backward Load ----------
    if (condensor_en)
    {
        t32 = i_conden_thrshld;
    }
    else
    {
        t32 = 0L;
    }

    if (i0sqsum_mon >= t32 && w0sum_mon < 0)
    {
        LineStatus |= LA_WHBACK;
    }

//  S

// -------- No load status ---------------
#if 0
	if ((i1sqsum_mon == 0) || (no_load_chk_zon_cnt))
#else  // jp.kim 24.11.30
    if ((v1sqsum_mon < v_min_thrshld) || (i1sqsum_mon == 0) ||
        (no_load_chk_zon_cnt))
#endif
    {
        no_load_statusB = true;
    }
    else
    {
        no_load_statusB = false;
    }

    // ---------- start current status ------------
    if (i1sqsum_mon > i_min_thrshldx2)
    {
        start_curr_statusB = true;
    }
    else
    {
        start_curr_statusB = false;
    }

    // ---------- current over ---------------
    if (i1sqsum_mon > i_over_thrshld)
    {
        LineStatus |= LB_IOVER;
    }

    // ------------ Volt Over ------------
    if (v1sqsum_mon > v_over_thrshld)
    {
        LineStatus |= LB_VOVER;
    }

#if 0
	// ------------ Volt Min ------------
	if(v1sqsum_mon < v_min_thrshld) {
		LineStatus |= LB_VMIN;
	}
#endif

    // ------------ Volt Drop ------------
    if (v1sqsum_mon < VdropThrsld)
    {
        LineStatus |= LB_VDROP;
    }

    // ------------ Backward Load ----------
    if (condensor_en)
    {
        t32 = i_conden_thrshld;
    }
    else
    {
        t32 = 0L;
    }

    if (i1sqsum_mon >= t32 && w1sum_mon < 0)
    {
        LineStatus |= LB_WHBACK;
    }

    //  T

    // -------- No load status ---------------
#if 0
	if ((i2sqsum_mon == 0) || (no_load_chk_zon_cnt))
#else  // jp.kim 24.11.30
    if ((v2sqsum_mon < v_min_thrshld) || (i2sqsum_mon == 0) ||
        (no_load_chk_zon_cnt))
#endif
    {
        no_load_statusC = true;
    }
    else
    {
        no_load_statusC = false;
    }

    // ---------- start current status ------------
    if (i2sqsum_mon > i_min_thrshldx2)
    {
        start_curr_statusC = true;
    }
    else
    {
        start_curr_statusC = false;
    }

    // ---------- current over ---------------
    if (i2sqsum_mon > i_over_thrshld)
    {
        LineStatus |= LC_IOVER;
    }

    // ------------ Volt Over ------------
    if (v2sqsum_mon > v_over_thrshld)
    {
        LineStatus |= LC_VOVER;
    }

#if 0
	// ------------ Volt Min ------------
	if(v2sqsum_mon < v_min_thrshld) {
		LineStatus |= LC_VMIN;
	}
#endif

    // ------------ Volt Drop ------------
    if (v2sqsum_mon < VdropThrsld)
    {
        LineStatus |= LC_VDROP;
    }

    // ------------ Backward Load ----------
    if (condensor_en)
    {
        t32 = i_conden_thrshld;
    }
    else
    {
        t32 = 0L;
    }

    if (i2sqsum_mon >= t32 && w2sum_mon < 0)
    {
        LineStatus |= LC_WHBACK;
    }
}
#endif

float v_thd_avg_3(U8 line, float data_fl)
{
    U8 i = 0;
    float fdata = 0.0;

    g_v_thd_avg_buf[g_avg3_index][line] = data_fl;

    // DPRINTF(DBG_ERR, _D"%s	2: g_v_thd_avg_buf[0][0][%d.%03d %]
    // g_avg3_index[%d]	 \r\n",
    // __func__,(U32)(g_v_thd_avg_buf[0][0]),(U32)(g_v_thd_avg_buf[0][0]*1000.0),
    // g_avg3_index); DPRINTF(DBG_ERR, _D"%s	2: g_v_thd_avg_buf[1][0][%d.%03d
    // %]		 \r\n",
    // __func__,(U32)(g_v_thd_avg_buf[1][0]),(U32)(g_v_thd_avg_buf[1][0]*1000.0));
    // DPRINTF(DBG_ERR, _D"%s	2: g_v_thd_avg_buf[2][0][%d.%03d %]		 \r\n",
    // __func__,(U32)(g_v_thd_avg_buf[2][0]),(U32)(g_v_thd_avg_buf[2][0]*1000.0));

    for (i = 0; i < 3; i++)
    {
        fdata += g_v_thd_avg_buf[i][line];
    }
    fdata /= 3.0;

    return fdata;
}

void outputs_for_mon(void)
{
    ST_MTP_PUSH_DATA* p_pushdata = dsm_mtp_get_push_data();

    push_data_user_back_up_data_for_meter_fwup();

    // jp.kim 25.03.12
    if (no_inst_curr_chk_zon_cnt)
    {
        i0sqsum_mon = 0.0;  // afe_isqsum(1);
        i1sqsum_mon = 0.0;  // afe_isqsum(2);
        i2sqsum_mon = 0.0;
    }
    else
    {
        i0sqsum_mon = p_pushdata->r_current;  // afe_isqsum(1);
        i1sqsum_mon = p_pushdata->s_current;  // afe_isqsum(2);
        i2sqsum_mon = p_pushdata->t_current;
    }

    if (meter_fwup_ing())
    {
        i0sqsum_mon = p_pushdata->r_current;  // afe_isqsum(1);
        i1sqsum_mon = p_pushdata->s_current;  // afe_isqsum(2);
        i2sqsum_mon = p_pushdata->t_current;
    }

    v0sqsum_mon = p_pushdata->r_voltage;
    v1sqsum_mon = p_pushdata->s_voltage;
    v2sqsum_mon = p_pushdata->t_voltage;

    // 3회 이동 평균값 사용
    v0sqsum_h_mon = v_thd_avg_3(0, p_pushdata->r_vol_thd);
    v1sqsum_h_mon = v_thd_avg_3(1, p_pushdata->s_vol_thd);
    v2sqsum_h_mon = v_thd_avg_3(2, p_pushdata->t_vol_thd);
    g_avg3_index++;
    if (g_avg3_index >= 3)
        g_avg3_index = 0;

    w0sum_mon = p_pushdata->r_act;
    w1sum_mon = p_pushdata->s_act;
    w2sum_mon = p_pushdata->t_act;

    var0sum_mon = p_pushdata->r_react;
    var1sum_mon = p_pushdata->s_react;
    var2sum_mon = p_pushdata->t_react;

    va0sum_mon = v0sqsum_mon * i0sqsum_mon;
    va1sum_mon = v1sqsum_mon * i1sqsum_mon;
    va2sum_mon = v2sqsum_mon * i2sqsum_mon;

#if 0 /* bccho, 2024-06-03 */
	sag_det_cnt += p_pushdata->sag_count;
	swell_det_cnt += p_pushdata->swell_count;
#else
    sag_det_cnt = p_pushdata->sag_count;
    swell_det_cnt = p_pushdata->swell_count;
#endif

    /* bccho, 2024-09-05, 삼상 */
    PH_AtoB_mon = p_pushdata->rs_phase;
    PH_AtoC_mon = p_pushdata->rt_phase;

    freq_mon = p_pushdata->freq;
    temp_raw_mon = p_pushdata->temp;

#if 1 /* bccho, 2024-09-05, 삼상 */
    vrms0_mon = get_inst_volt(0);
    vrms1_mon = get_inst_volt(1);
    vrms2_mon = get_inst_volt(2);
    irms0_mon = get_inst_curr(0);
    irms1_mon = get_inst_curr(1);
    irms2_mon = get_inst_curr(2);
    ph0_mon = get_inst_phase(0);
    ph1_mon = get_inst_phase(1);
    ph2_mon = get_inst_phase(2);
#else
    vrms0_mon = get_inst_volt(0);
    irms0_mon = get_inst_curr(0);
    irms1_mon = get_inst_curr(1);
    ph0_mon = get_inst_phase(0);
    ph1_mon = get_inst_phase(1);
#endif
    pwr0_mon = get_inst_power(true, 0);
    pwrvar0_mon = get_inst_power(false, 0);
}

float get_inst_volt(int line)
{
    float fval = 0.0;
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();

    switch (line)
    {
    case 0:
        fval = pushd->r_voltage;  // A
        break;
    case 1:
        fval = pushd->s_voltage;  // B
        break;
    case 2:
        fval = pushd->t_voltage;  // C
        break;
    }

    return fval;
}

float get_inst_curr(int line)
{
    float fval = 0.0;
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();

    switch (line)
    {
    case 0:
        fval = pushd->r_current;  // A
        break;
    case 1:
        fval = pushd->s_current;  // B
        break;
    case 2:
        fval = pushd->t_current;  // C
        break;
    }

#if 1  // 0 //defined(FEATURE_MTP_FWUP_DATA_STARTING_PROTECTION)
    if (no_inst_curr_chk_zon_cnt)
        fval = 0.0;
#endif

    if (fval < StartCur)
        fval = 0.0;  // jp.kim 25.04.03

    return fval;
}

float get_inst_curr_allphase(void)
{
#if PHASE_NUM != SINGLE_PHASE
    // RST 상에 대한 처리 필요..  가장 큰전류 검출
    float fval;
    if (i0sqsum_mon > i1sqsum_mon)
        fval = i0sqsum_mon;
    else
        fval = i1sqsum_mon;
    if (fval < i2sqsum_mon)
        fval = i2sqsum_mon;
    return rtn_inst_irms(fval);
#else
    return rtn_inst_irms(i0sqsum_mon);
#endif
}

float get_inst_volt_THD(int line)
{
    float fval = 0.0;

    switch (line)
    {
    case 0:
        fval = v0sqsum_h_mon;
        break;
    case 1:
        fval = v1sqsum_h_mon;
        break;
    case 2:
        fval = v2sqsum_h_mon;
        break;
    }

    if (!afe_is_up())
        fval = 0.0;

    if (fval < 3.0)
        fval = 0.0;

    if (fval < 6.0)
    {
        fval /= 3.5;
    }

    return fval;
}

/* JPKIM, 2024-10-04 */
float get_inst_power_allphase(void)
{
#if PHASE_NUM == SINGLE_PHASE
    return rtn_inst_power(fabs(w0sum_mon));
#else
    // RST 상에 대한 처리 필요..	가장  큰값 검출
    float fval;
    if (w0sum_mon > w1sum_mon)
        fval = w0sum_mon;
    else
        fval = w1sum_mon;
    if (fval < w2sum_mon)
        fval = w2sum_mon;
    return rtn_inst_irms(fval);
#endif
}

float get_inst_power(bool _w, int line)
{
    if (_w)
    {
        if (line == 0)
            return rtn_inst_power(fabs(w0sum_mon));
        else if (line == 1)
            return rtn_inst_power(fabs(w1sum_mon));
        else
            return rtn_inst_power(fabs(w2sum_mon));
    }
    else
    {
        if (line == 0)
            return rtn_inst_power(fabs(var0sum_mon));
        else if (line == 1)
            return rtn_inst_power(fabs(var1sum_mon));
        else
            return rtn_inst_power(fabs(var2sum_mon));
    }
}

float rtn_pf(float w, float var, float va)
{
    ufloat_type uf;
    uint32_t t32;
    float wval, vaval;

    if (mt_selreact == 0x04)
    {
        t32 = va;
        vaval = (float)va;
    }
    else
    {
        if (mt_selreact == 0x01)  // Lag
        {
            if (w > 0.0 && var < 0.0)
            {
                var = 0.0;
            }
            else if (w < 0.0 && var > 0.0)
            {
                var = 0.0;
            }
        }
        else if (mt_selreact == 0x02)  // Lead
        {
            if (w > 0.0 && var > 0.0)
            {
                var = 0.0;
            }
            else if (w < 0.0 && var < 0.0)
            {
                var = 0.0;
            }
        }

        if (w == 0.0 && var == 0.0)
        {
            t32 = 0L;
            vaval = 0.0;
        }
        else
        {
            vaval = VAh_WVAR_f(w, var);
            t32 = (uint32_t)vaval;
        }
    }

    if (w < 0.0)
        w *= (-1.0);

    if (w == 0.0 && t32 == 0L)
        return (float)-1.0;

    if (w == 0.0 || t32 == 0L)
        return (float)0.0;

    wval = (float)w;

    if (wval > vaval)
        return (float)100.0;

    uf.f = wval / vaval;
    if (uf.ul == NaN || uf.ul == plusINF || uf.ul == minusINF)
        return (float)0.0;

    return uf.f * 100.0;
}

float get_inst_current(int line)
{
    float fval = 0.0;
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();

    switch (line)
    {
    case 0:
        fval = pushd->r_current;
        break;
    case 1:
        fval = pushd->s_current;
        break;
    case 2:
        fval = pushd->t_current;
        break;

    default:
        break;
    }

    return fval;
}

float get_inst_pf(int line)
{
    if (get_inst_current(line) == 0)
        return 0.0;
    return (cos((RADIAN_PER_DEGREES * get_inst_phase(line))) * 100.0);
}

float Compute_Phase_Angle(int line)
{
    float f0 = 0.0;
    int32_t w = 0, v = 0 /*, va = 0*/;
    vi_quarter_type quart;

    switch (line)
    {
    case 0:
        if (curr_is_noload_A())
        {
            return 0.0;
        }
        else
        {
            w = w0sum_mon;
            v = var0sum_mon;
        }
        break;
    case 1:
        if (curr_is_noload_B())
        {
            return 0.0;
        }
        else
        {
            w = w1sum_mon;
            v = var1sum_mon;
        }
        break;

    case 2:
        if (curr_is_noload_C())
        {
            return 0.0;
        }
        else
        {
            w = w2sum_mon;
            v = var2sum_mon;
        }
        break;
    }

    quart = decision_quarter(w, v);

#if 1 /* bccho, M2354, 중복, 2023-07-20 */
    w = labs2(w);
    v = labs2(v);
#else
    w = labs(w);
    v = labs(v);
#endif

    f0 = atan((float)v / (float)w);
    f0 *= 180.0 / PI;

    switch (quart)
    {
    case eVIq1:
        break;
    case eVIq2:
        f0 = 180.0 - f0;
        break;
    case eVIq3:
        f0 += 180.0;
        break;
    case eVIq4:
        f0 = 360.0 - f0;
        break;

    default:
        break;
    }

    return f0;
}

float get_inst_phase(int line)
{
    float fval = 0.0;
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();

    switch (line)
    {
    case 0:
        fval = pushd->r_phase;
        break;
    case 1:
        fval = pushd->s_phase;
        break;
    case 2:
        fval = pushd->t_phase;
        break;

    default:
        break;
    }

    if (!afe_is_up())
        fval = 0.0;

    return fval;
}

float get_inst_vphase(int line)
{
#if 1 /* bccho, 2024-09-05, 삼상 */
    float fval = 0.0;
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();

    switch (line)
    {
    case 0:
        fval = pushd->rs_phase;
        break;
    case 1:
        fval = pushd->rt_phase;
        break;
    default:
        fval = pushd->rs_phase;
        break;
    }

    if (!afe_is_up())
        fval = 0.0;

    /* bccho, 2024-09-25, 삼상 */
    if (fval < 0.0)
        fval += 360.0;

    return fval;
#else
    sel = 0;  // to avoid compile warning
    return 0.0;
#endif
}

float get_inst_freq(void)
{
    float fval = 0.0;
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();

    fval = pushd->freq;

    return fval;
}

bool emb_disp_is_ready(void)
{
    bool ret = b_emb_display_ok;
    b_emb_display_ok = false;
    return ret;
}

float get_inst_LtoL(int line)
{
#if PHASE_NUM == SINGLE_PHASE
    (void)line;
    return get_inst_volt(0);
#else /* bccho, 2024-09-05, 삼상 */
    float va, vb, vc;
    float ph_ab, ph_ac;

    va = get_inst_volt(0);
    vb = get_inst_volt(1);
    vc = get_inst_volt(2);
    ph_ab = get_inst_vphase(0);
    ph_ac = get_inst_vphase(1);

    if (line == 0)
    {
        // V_AB
        if (va == 0.0 && vb == 0.0)
        {
            return 0.0;
        }
        else
        {
            return sqrtf(va * va + vb * vb -
                         2.0 * va * vb * cos(ph_ab * PI / 180.0));
        }
    }

    if (line == 1)
    {
        // V_BC
        if (vb == 0.0 && vc == 0.0)
        {
            return 0.0;
        }

        if (ph_ab != 0.0 && ph_ac != 0.0)
        {
            return sqrtf(vb * vb + vc * vc -
                         2.0 * vb * vc * cos((ph_ac - ph_ab) * PI / 180.0));
        }
        else
        {
            if (ph_ab != 0.0)
            {
                return vb;
            }
            else if (ph_ac != 0.0)
            {
                return vc;
            }
            else
            {
                if (va != 0.0)
                {
                    return 0.0;
                }
                else
                {
                    if (vb == 0.0 || vc == 0.0)
                    {
                        return sqrtf(vb * vb + vc * vc);
                    }
                    return sqrtf(vb * vb + vc * vc -
                                 2.0 * vb * vc * cos(120.0 * PI / 180.0));
                }
            }
        }
    }

    if (line == 2)
    {
        // V_AC
        if (va == 0.0 && vc == 0.0)
        {
            return 0.0;
        }
        else
        {
            return sqrtf(va * va + vc * vc -
                         2.0 * va * vc * cos((360.0 - ph_ac) * PI / 180.0));
        }
    }

    return 0.0;

#endif
}

void default_min_max_val(void)
{
    min_freq = 100.0;
    max_freq = 0.0;

#if 1 /* bccho, 2024-09-05, 삼상 */
    min_volt[0] = 1000.0;
    max_volt[0] = 0.0;

    min_volt[1] = 1000.0;
    max_volt[1] = 0.0;

    min_volt[2] = 1000.0;
    max_volt[2] = 0.0;
#else
    min_volt = 1000.0;
    max_volt = 0.0;
#endif
}

void min_max_load(void)
{
    min_max_data_type _mval;

    if (nv_read(I_MIN_FREQ, (uint8_t*)&_mval))
    {
        min_freq = _mval._val;
    }
    else
    {
        min_freq = 100.0;
    }

    if (nv_read(I_MAX_FREQ, (uint8_t*)&_mval))
    {
        max_freq = _mval._val;
    }
    else
    {
        max_freq = 0.0;
    }

#if 1 /* bccho, 2024-09-05, 삼상 */
    if (nv_read(I_MIN_VOLT_L1, (U8*)&_mval))
    {
        min_volt[0] = _mval._val;
    }
    else
    {
        min_volt[0] = 1000.0;
    }

    if (nv_read(I_MAX_VOLT_L1, (U8*)&_mval))
    {
        max_volt[0] = _mval._val;
    }
    else
    {
        max_volt[0] = 0.0;
    }

    if (nv_read(I_MIN_VOLT_L2, (U8*)&_mval))
    {
        min_volt[1] = _mval._val;
    }
    else
    {
        min_volt[1] = 1000.0;
    }

    if (nv_read(I_MAX_VOLT_L2, (U8*)&_mval))
    {
        max_volt[1] = _mval._val;
    }
    else
    {
        max_volt[1] = 0.0;
    }

    if (nv_read(I_MIN_VOLT_L3, (U8*)&_mval))
    {
        min_volt[2] = _mval._val;
    }
    else
    {
        min_volt[2] = 1000.0;
    }

    if (nv_read(I_MAX_VOLT_L3, (U8*)&_mval))
    {
        max_volt[2] = _mval._val;
    }
    else
    {
        max_volt[2] = 0.0;
    }
#else
    if (nv_read(I_MIN_VOLT, (uint8_t*)&_mval))
    {
        min_volt = _mval._val;
    }
    else
    {
        min_volt = 1000.0;
    }

    if (nv_read(I_MAX_VOLT, (uint8_t*)&_mval))
    {
        max_volt = _mval._val;
    }
    else
    {
        max_volt = 0.0;
    }
#endif
}

/* bccho, 2024-09-05, 삼상 */
void volt_minmax_cal_sub(float finst, uint8_t idx)
{
    uint16_t val;
    float fval, fminmax;
    min_max_data_type mval;

    val = (uint16_t)(finst * 10.0);
    fval = (float)val / 10.0;

    val = (uint16_t)(min_volt[idx] * 10.0);  // 14.6.17 --> continue to write to
                                             // NV in case of no-cut of finst
    fminmax = (float)val / 10.0;

    if (fval < fminmax)
    {
        if (++minvolt_cnt[idx] >= 3)
        {  // due to unstable reason, for example power_fail,,,

            DPRINTF(DBG_TRACE, _D " min_volt[%d] = finst: %d V \r\n", idx,
                    (uint32_t)min_volt[idx]);

            // TRACE1("!min_volt = finst", (uint32_t)min_volt);

            // if(( finst == 0.0 )) finst = 0.1;
            min_volt[idx] = finst;

            // TRACE1("!min_volt = finst", (uint32_t)finst);
            DPRINTF(DBG_TRACE, _D " min_volt[%d] = finst: %d V \r\n",
                    (uint32_t)finst);

            mval._val = min_volt[idx];
            mval._dt = cur_rtc;
            if (idx == 1)
            {
                nv_write(I_MIN_VOLT_L2, (uint8_t*)&mval);
            }
            else if (idx == 2)
            {
                nv_write(I_MIN_VOLT_L3, (uint8_t*)&mval);
            }
            else
            {
                nv_write(I_MIN_VOLT_L1, (uint8_t*)&mval);
            }

            minvolt_cnt[idx] = 0;
        }
    }
    else
    {
        minvolt_cnt[idx] = 0;
    }

    val = (uint16_t)(max_volt[idx] * 10.0);  // 14.6.17 --> continue to write to
                                             // NV in case of no-cut of finst
    fminmax = (float)val / 10.0;

    if (fval > fminmax)
    {
        if (++maxvolt_cnt[idx] >= 3)
        {  // due to unstable reason, for example power_fail,,,
            max_volt[idx] = finst;

            mval._val = max_volt[idx];
            mval._dt = cur_rtc;
            if (idx == 1)
            {
                nv_write(I_MAX_VOLT_L2, (uint8_t*)&mval);
            }
            else if (idx == 2)
            {
                nv_write(I_MAX_VOLT_L3, (uint8_t*)&mval);
            }
            else
            {
                nv_write(I_MAX_VOLT_L1, (uint8_t*)&mval);
            }

            maxvolt_cnt[idx] = 0;
        }
    }
    else
    {
        maxvolt_cnt[idx] = 0;
    }
}

void Monitor_MinMax(void)
{
    uint16_t val;
    float finst, fval, fminmax;
    min_max_data_type mval;

    if (!b_minmax_ready)
        return;
    b_minmax_ready = false;

    finst = get_inst_freq();
    val = (uint16_t)(finst * 10.0);
    fval = (float)val / 10.0;

    val = (uint16_t)(min_freq * 10.0);
    fminmax = (float)val / 10.0;

    if (fval < fminmax)
    {
        if (++minfreq_cnt >= 3)
        {
            min_freq = finst;

            mval._val = min_freq;
            mval._dt = cur_rtc;
            nv_write(I_MIN_FREQ, (uint8_t*)&mval);

            minfreq_cnt = 0;
        }
    }
    else
    {
        minfreq_cnt = 0;
    }

    val = (uint16_t)(max_freq * 10.0);
    fminmax = (float)val / 10.0;

    if (fval > fminmax)
    {
        if (++maxfreq_cnt >= 3)
        {
            max_freq = finst;

            mval._val = max_freq;
            mval._dt = cur_rtc;
            nv_write(I_MAX_FREQ, (uint8_t*)&mval);

            maxfreq_cnt = 0;
        }
    }
    else
    {
        maxfreq_cnt = 0;
    }

#if 1 /* bccho, 2024-09-05, 삼상 */
    volt_minmax_cal_sub(vrms0_mon, 0);
    volt_minmax_cal_sub(vrms1_mon, 1);
    volt_minmax_cal_sub(vrms2_mon, 2);
#else
    finst = vrms0_mon;
    val = (uint16_t)(finst * 10.0);
    fval = (float)val / 10.0;

    val = (uint16_t)(min_volt * 10.0);
    fminmax = (float)val / 10.0;

    if (fval < fminmax)
    {
        if (++minvolt_cnt >= 3)
        {
            DPRINTF(DBG_TRACE, _D " min_volt = finst: %d V \r\n",
                    (uint32_t)min_volt);

            min_volt = finst;

            DPRINTF(DBG_TRACE, _D " min_volt = finst: %d V \r\n",
                    (uint32_t)finst);

            mval._val = min_volt;
            mval._dt = cur_rtc;
            nv_write(I_MIN_VOLT, (uint8_t*)&mval);

            minvolt_cnt = 0;
        }
    }
    else
    {
        minvolt_cnt = 0;
    }

    val = (uint16_t)(max_volt * 10.0);
    fminmax = (float)val / 10.0;

    if (fval > fminmax)
    {
        if (++maxvolt_cnt >= 3)
        {
            max_volt = finst;

            mval._val = max_volt;
            mval._dt = cur_rtc;
            nv_write(I_MAX_VOLT, (uint8_t*)&mval);

            maxvolt_cnt = 0;
        }
    }
    else
    {
        maxvolt_cnt = 0;
    }
#endif
}

void get_accm_chs(rate_type rt, uint32_t* pch)
{
    int i;

    for (i = 0; i < numCHs; i++)
    {
        pch[i] = ACC_CH(rt, i);
    }
}

void accm_ch_copy(uint32_t* pch1, uint32_t* pch2)
{
    int i;

    for (i = 0; i < numCHs; i++)
    {
        pch1[i] = pch2[i];
    }
}

void accm_ch_diff(uint32_t* pch, uint32_t* pch1, uint32_t* pch2)
{
    int i;

    for (i = 0; i < numCHs; i++)
    {
        pch[i] = (mxaccm_dgt_cnt + pch1[i] - pch2[i]) % mxaccm_dgt_cnt;
    }
}

uint32_t get_sel_react32(energy_dir_type dir, uint32_t* chptr)
{
    if (dir == eDeliAct)
    {
        if (mt_selreact == 0x01)
            return chptr[eChDLagReact];
        else if (mt_selreact == 0x02)
            return chptr[eChDLeadReact];
        else if (mt_selreact == 0x03)
            return (chptr[eChDLagReact] + chptr[eChDLeadReact]);
        else
            return chptr[eChDeliApp];
    }
    else
    {
        if (mt_selreact == 0x01)
            return chptr[eChRLagReact];
        else if (mt_selreact == 0x02)
            return chptr[eChRLeadReact];
        else if (mt_selreact == 0x03)
            return (chptr[eChRLagReact] + chptr[eChRLeadReact]);
        else
            return chptr[eChReceiApp];
    }
}

float VAh_WVAR_f(int32_t w0, int32_t v0)
{
    float w, v;

    w = (float)(w0);
    v = (float)(v0);
    return sqrtf(w * w + v * v);
}

#if 0  // jp.kim 25.06.22
uint32_t get_ch_val_nprd(uint8_t month, rate_type rt, energy_dir_type dir,
                         energy_kind_type enkind, uint8_t *tptr)
{
    bool ok;
    uint32_t *acch;
    uint32_t t32;

    ok = false;
    switch (month)
    {
    case 0:
        acch = mt_accm.acc[rt].ch;
        ok = true;
        break;
    case 1:
        if (mrcnt_nprd < 1)
        {
            ok = false;
            break;
        }
    case 2:
        if ((month == 2) && (mrcnt_nprd < 2))
        {
            ok = false;
            break;
        }
        nv_sub_info.mr.mrcnt = get_mr_idx_nperiod(month_to_grp_f(month));
        nv_sub_info.mr.sel = eMrAccm;
        nv_sub_info.mr.rt = rt;
        if (nv_read(I_MT_READ_DATA_nPRD, tptr))
        {
            acch = ((mr_ch_type *)tptr)->ch;
            ok = true;
        }
        break;
    }

    if (ok)
    {
        if (dir == eDeliAct)
        {
            if (enkind == eActEn)
            {
                return acch[eChDeliAct];
            }
            if (enkind == eAppEn)
                return acch[eChDeliApp];
            if (enkind == eLagReactEn)
                return acch[eChDLagReact];
            if (enkind == eLeadReactEn)
                return acch[eChDLeadReact];
            goto get_ch_val1;
        }
        else
        {
            if (enkind == eActEn)
            {
                return acch[eChReceiAct];
            }
            if (enkind == eAppEn)
                return acch[eChReceiApp];
            if (enkind == eLagReactEn)
                return acch[eChRLagReact];
            if (enkind == eLeadReactEn)
                return acch[eChRLeadReact];
        get_ch_val1:
            t32 = get_sel_react32(dir, acch);
            if (t32 >= mxaccm_dgt_cnt)
                t32 -= mxaccm_dgt_cnt;
            return t32;
        }
    }
    return 0L;
}
#endif

// month : 0(current month), 1(before month), 2(before before month)
uint32_t get_ch_val(uint8_t month, rate_type rt, energy_dir_type dir,
                    energy_kind_type enkind, uint8_t* tptr)
{
    bool ok;
    uint32_t* acch;
    uint32_t t32;

    ok = false;
    switch (month)
    {
    case 0:
        acch = mt_accm.acc[rt].ch;
        ok = true;
        break;
    case 1:
        if (mr_cnt >= 1)
        {
            acch = mr_data_accm.accm[rt].ch;
            ok = true;
        }
        break;
    case 2:
        if (mr_cnt >= 2)
        {
            nv_sub_info.mr.mrcnt = get_mr_idx(month_to_grp_f(month));
            nv_sub_info.mr.sel = eMrAccm;
            nv_sub_info.mr.rt = rt;
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                acch = ((mr_ch_type*)tptr)->ch;
                ok = true;
            }
        }
        break;
    }

    if (ok)
    {
        if (dir == eDeliAct)
        {
            if (enkind == eActEn)
            {
                return acch[eChDeliAct];
            }
            if (enkind == eAppEn)
                return acch[eChDeliApp];
            if (enkind == eLagReactEn)
                return acch[eChDLagReact];
            if (enkind == eLeadReactEn)
                return acch[eChDLeadReact];
            goto get_ch_val1;
        }
        else
        {
            if (enkind == eActEn)
            {
                return acch[eChReceiAct];
            }
            if (enkind == eAppEn)
                return acch[eChReceiApp];
            if (enkind == eLagReactEn)
                return acch[eChRLagReact];
            if (enkind == eLeadReactEn)
                return acch[eChRLeadReact];
        get_ch_val1:
            t32 = get_sel_react32(dir, acch);
            if (t32 >= mxaccm_dgt_cnt)
                t32 -= mxaccm_dgt_cnt;
            return t32;
        }
    }
    return 0L;
}

/* bccho, 2024-09-05, 삼상 */
void volt_drop_thrd_set(void)
{
    float sag_level;
    ToHFloat((U8_Float*)&sag_level, &g_mtp_sagswell.sag_level[0]);

    if ((sag_level != 0.0) && (g_mtp_sagswell.sag_time != 0))
    {
        // drop voltage 설정
        VdropThrsld = (int32_t)rtn_vsqsum(
            sag_level);  // 15.7.16 debug from rtn_vsqsum((U8)f32);
        DPRINTF(DBG_ERR, _D "%s	3: VdropThrsld[%d	v] \r\n", __func__,
                (U32)(VdropThrsld));
    }
}

bool sag_condition(U8 line)
{
    bool sag = 0;
    float fval, fval2;
    ToHFloat((U8_Float*)&fval, &g_mtp_sagswell.sag_level[0]);

    if (line == 1)
        fval2 = vrms1_mon;
    else if (line == 2)
        fval2 = vrms2_mon;
    else
        fval2 = vrms0_mon;

    if (fval2 < fval)
        sag = true;
    else
        sag = false;

    return sag;
}

bool swell_condition(U8 line)
{
    bool swell = 0;
    float fval, fval2;
    ToHFloat((U8_Float*)&fval, &g_mtp_sagswell.swell_level[0]);

    if (line == 1)
        fval2 = vrms1_mon;
    else if (line == 2)
        fval2 = vrms2_mon;
    else
        fval2 = vrms0_mon;

    if (fval2 > fval)
        swell = true;
    else
        swell = false;

    return swell;
}

uint8_t* adjust_tptr(uint8_t* tptr)
{
    if (((uint32_t)tptr & 0x03) == 0)
        return tptr;

    return (uint8_t*)((uint32_t)(tptr + 4) & 0xfffffffc);
}

U8 SAG_SET_PENDING_STS = 0;
void monitor_sag_swell(void)
{
    int tsag, tswell;

    tsag = sag_det_cnt;
    tswell = swell_det_cnt;

    if (sag_detect_mask_zon_cnt > 0)
    {
        SAG_SET_PENDING_STS = 0;
    }

    if (sag_detect_mask_zon_cnt <= 0)
    {
        if ((sag_recovery_is_timeout()) && SAG_SET_PENDING_STS)
        {
            SAG_SET_PENDING_STS = 0;

            lp_event_set(LPE_SAG_SWELL);

            if (!(WMStatus & SAG_STS))
            {
                log_sag_swell(eLogSag);
                SAG_trigger();
            }
        }

        if (tsag)
        {
            sag_recovery_timeset(T1SEC);
            SAG_SET_PENDING_STS = 1;
        }
#if PHASE_NUM == SINGLE_PHASE
        else if (!sag_condition(0))
#else
        else if (!(sag_condition(0) || sag_condition(1) || sag_condition(2)))
#endif
        {
            WMStatus &= ~SAG_STS;
        }
    }

    if (tswell)
    {
        lp_event_set(LPE_SAG_SWELL);
        if (!(WMStatus & SWELL_STS))
        {
            log_sag_swell(eLogSwell);
            SWELL_trigger();
        }
    }
#if PHASE_NUM == SINGLE_PHASE
    else if (!swell_condition(0))
#else
    else if (!(swell_condition(0) || swell_condition(1) || swell_condition(2)))
#endif
    {
        WMStatus &= ~SWELL_STS;
    }
}

#if 1  // jp.kim 25.06.22
// month : 1(before month), 2(before before month)
uint8_t get_bf_pf_PrdNprdSeason(uint8_t month, rate_type rt,
                                energy_dir_type dir, uint8_t* tptr)
{
    bool ok = false;
    float pf = -1.0;
    uint8_t mr_month_type;

    // 정기,비정기, 계절변경을 통합하여 가장최근 (전월) 2번째 최근(전전월)
    // 정보를 구한다.

    if ((month) && (mr_bill_prd_type[month - 1]))
    {  // 해당월에 sr이 존재해야함
        if (month == 1)
            mr_month_type = 101;

        nv_sub_info.mr.sel = eMrAccm;
        nv_sub_info.mr.rt = rt;

        if (mr_bill_prd_type[month - 1] & EOB_PERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_PERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx(mr_month_type);
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                pf = ((mr_ch_type*)tptr)->pf[dir];
                ok = true;
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_nPERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_nPERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_nperiod(mr_month_type);
            if (nv_read(I_MT_READ_DATA_nPRD, tptr))
            {
                pf = ((mr_ch_type*)tptr)->pf[dir];
                ok = true;
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_SEASON_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_SEASON_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_season(mr_month_type);
            if (nv_read(I_MT_READ_DATA_SEASON, tptr))
            {
                pf = ((mr_ch_type*)tptr)->pf[dir];
                ok = true;
            }
        }
    }

    DPRINTF(
        DBG_ERR,
        "%s:  month:%d mr_bill_prd_type[0]:%d  mr_bill_prd_type[1]:%d  \r\n",
        __func__, month, mr_bill_prd_type[0], mr_bill_prd_type[1]);

    if (!ok)
        return 0xff;
    if (pf == -1.0)
    {
        return 0xff;
    }
    return (uint8_t)(pf * 100.0);
}

// month : 1(before month), 2(before before month)
uint32_t get_bf_cumdm_PrdNprdSeason(uint8_t month, rate_type rt,
                                    demand_ch_type ch, uint8_t* tptr)
{
    uint32_t t32 = 0L;
    uint8_t mr_month_type;

    // 정기,비정기, 계절변경을 통합하여 가장최근 (전월) 2번째 최근(전전월)
    // 정보를 구한다.

    if ((month) && (mr_bill_prd_type[month - 1]))
    {  // 해당월에 sr이 존재해야함
        if (month == 1)
            mr_month_type = 101;

        nv_sub_info.mr.sel = eMrDm;
        nv_sub_info.mr.rt = rt;

        if (mr_bill_prd_type[month - 1] & EOB_PERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_PERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx(mr_month_type);
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                t32 = ((mr_dm_type*)tptr)->cum_mxdm[ch];
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_nPERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_nPERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_nperiod(mr_month_type);
            if (nv_read(I_MT_READ_DATA_nPRD, tptr))
            {
                t32 = ((mr_dm_type*)tptr)->cum_mxdm[ch];
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_SEASON_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_SEASON_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_season(mr_month_type);
            if (nv_read(I_MT_READ_DATA_SEASON, tptr))
            {
                t32 = ((mr_dm_type*)tptr)->cum_mxdm[ch];
            }
        }
    }

    DPRINTF(DBG_ERR,
            "%s:  month:%d mr_bill_prd_type[0]:%d  mr_bill_prd_type[1]:%d "
            "t32:%d \r\n",
            __func__, month, mr_bill_prd_type[0], mr_bill_prd_type[1], t32);

    return t32;
}

// month : 1(before month), 2(before before month)
void get_bf_mxtime_PrdNprdSeason(date_time_type* dt, uint8_t month,
                                 rate_type rt, demand_ch_type ch, uint8_t* tptr)
{
    bool ok = false;
    uint8_t mr_month_type;

    // 정기,비정기, 계절변경을 통합하여 가장최근 (전월) 2번째 최근(전전월)
    // 정보를 구한다.

    if ((month) && (mr_bill_prd_type[month - 1]))
    {  // 해당월에 sr이 존재해야함
        if (month == 1)
            mr_month_type = 101;

        nv_sub_info.mr.sel = eMrDm;
        nv_sub_info.mr.rt = rt;

        if (mr_bill_prd_type[month - 1] & EOB_PERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_PERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx(mr_month_type);
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                *dt = ((mr_dm_type*)tptr)->mxdm[ch].dt;
                ok = true;
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_nPERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_nPERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_nperiod(mr_month_type);
            if (nv_read(I_MT_READ_DATA_nPRD, tptr))
            {
                *dt = ((mr_dm_type*)tptr)->mxdm[ch].dt;
                ok = true;
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_SEASON_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_SEASON_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_season(mr_month_type);
            if (nv_read(I_MT_READ_DATA_SEASON, tptr))
            {
                *dt = ((mr_dm_type*)tptr)->mxdm[ch].dt;
                ok = true;
            }
        }
    }

    DPRINTF(
        DBG_ERR,
        "%s:  month:%d mr_bill_prd_type[0]:%d  mr_bill_prd_type[1]:%d  \r\n",
        __func__, month, mr_bill_prd_type[0], mr_bill_prd_type[1]);

    if (ok)
    {
    }
    else
    {
        memset((uint8_t*)dt, 0, sizeof(date_time_type));
    }
}

// month : 1(before month), 2(before before month)
uint32_t get_bf_mxdm_PrdNprdSeason(uint8_t month, rate_type rt,
                                   demand_ch_type ch, uint8_t* tptr)
{
    uint32_t t32 = 0L;
    uint8_t mr_month_type;

    // 정기,비정기, 계절변경을 통합하여 가장최근 (전월) 2번째 최근(전전월)
    // 정보를 구한다.

    if ((month) && (mr_bill_prd_type[month - 1]))
    {  // 해당월에 sr이 존재해야함
        if (month == 1)
            mr_month_type = 101;

        nv_sub_info.mr.sel = eMrDm;
        nv_sub_info.mr.rt = rt;

        if (mr_bill_prd_type[month - 1] & EOB_PERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_PERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx(mr_month_type);
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                t32 = ((mr_dm_type*)tptr)->mxdm[ch].val;
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_nPERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_nPERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_nperiod(mr_month_type);
            if (nv_read(I_MT_READ_DATA_nPRD, tptr))
            {
                t32 = ((mr_dm_type*)tptr)->mxdm[ch].val;
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_SEASON_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_SEASON_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_season(mr_month_type);
            if (nv_read(I_MT_READ_DATA_SEASON, tptr))
            {
                t32 = ((mr_dm_type*)tptr)->mxdm[ch].val;
            }
        }
    }

    DPRINTF(DBG_ERR,
            "%s:  month:%d mr_bill_prd_type[0]:%d  mr_bill_prd_type[1]:%d "
            "t32:%d \r\n",
            __func__, month, mr_bill_prd_type[0], mr_bill_prd_type[1], t32);

    return t32;
}

// month : 1(before month), 2(before before month)
uint32_t get_ch_val_PrdNprdSeason(uint8_t month, rate_type rt,
                                  energy_dir_type dir, energy_kind_type enkind,
                                  uint8_t* tptr)
{
    // TODO: (WD)

    bool ok = false;
    uint32_t* acch;
    uint32_t t32 = 0L;
    uint8_t mr_month_type;

    // 정기,비정기, 계절변경을 통합하여 가장최근 (전월) 2번째 최근(전전월)
    // 정보를 구한다.

    if ((month) && (mr_bill_prd_type[month - 1]))
    {  // 해당월에 sr이 존재해야함
        if (month == 1)
            mr_month_type = 101;

        nv_sub_info.mr.sel = eMrAccm;
        nv_sub_info.mr.rt = rt;

        if (mr_bill_prd_type[month - 1] & EOB_PERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_PERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx(mr_month_type);
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                acch = ((mr_ch_type*)tptr)->ch;
                ok = true;
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_nPERIOD_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_nPERIOD_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_nperiod(mr_month_type);
            if (nv_read(I_MT_READ_DATA_nPRD, tptr))
            {
                acch = ((mr_ch_type*)tptr)->ch;
                ok = true;
            }
        }
        else if (mr_bill_prd_type[month - 1] & EOB_SEASON_FLAG)
        {
            if (month == 2)
            {
                if (mr_bill_prd_type[0] & EOB_SEASON_FLAG)
                    mr_month_type = 102;
                else
                    mr_month_type = 101;
            }
            nv_sub_info.mr.mrcnt = get_mr_idx_season(mr_month_type);
            if (nv_read(I_MT_READ_DATA_SEASON, tptr))
            {
                acch = ((mr_ch_type*)tptr)->ch;
                ok = true;
            }
        }
    }

    DPRINTF(DBG_ERR,
            "%s:  month:%d mr_bill_prd_type[0]:%d  mr_bill_prd_type[1]:%d "
            "t32:%d \r\n",
            __func__, month, mr_bill_prd_type[0], mr_bill_prd_type[1], t32);

    if (ok)
    {
        if (dir == eDeliAct)
        {
            if (enkind == eActEn)
            {
                return acch[eChDeliAct];
            }
            if (enkind == eAppEn)
                return acch[eChDeliApp];
            if (enkind == eLagReactEn)
                return acch[eChDLagReact];
            if (enkind == eLeadReactEn)
                return acch[eChDLeadReact];
            goto get_ch_val1_all;
        }
        else
        {
            if (enkind == eActEn)
            {
                return acch[eChReceiAct];
            }
            if (enkind == eAppEn)
                return acch[eChReceiApp];
            if (enkind == eLagReactEn)
                return acch[eChRLagReact];
            if (enkind == eLeadReactEn)
                return acch[eChRLeadReact];
        get_ch_val1_all:
            t32 = get_sel_react32(dir, acch);
            if (t32 >= mxaccm_dgt_cnt)
                t32 -= mxaccm_dgt_cnt;
            return t32;
        }
    }
    return 0L;
}
#endif
