#include "options.h"
#include "afe.h"
#include "tmp.h"
#include "comm.h"
#include "eoi.h"
#include "eob.h"
#include "lp.h"
#include "program.h"
#include "nv.h"
#include "set_req.h"
#include "get_req.h"
#include "key.h"
#include "port.h"
#include "amg_pwr.h"
#include "amg_mif_prtl.h"
#include "amg_pwr.h"

#define _D "[WHM_1] "

#define VBAT_BUF_SIZE 6

ST_MIF_SAGSWELL_SETUP *dsm_mtp_get_sagswell(void);

void mif_meter_parm_set(void);
extern bool MmodeCHG_sr_dr_type_sr_dr_run_pending_sts;
extern uint8_t mif_meter_sagswe_set_wait_seq_cnt;
extern bool mif_meter_sagswe_set_wait_ing;

uint8_t relay_onff_timer;
bool relay_onff_req;
int32_t cal_magnet;

bat_used_time_type bat_used_time;
bool bat_installed_inputmode;
bool bat_rtchg_history;

static uint8_t scurr_step;
static bool ts_testing;
static uint8_t ts_test_sec;
static uint8_t ts_test_used_now;

bool wrong_conn_prev;
int vdrop_cnt[3];
int v_low_cnt[3];
int whbwd_cnt;
int iover_cnt[3];

int32_t VdropThrsld;
uint32_t WMStatus;
uint32_t MTStatus;
uint32_t WMStatus_intern;

/* bccho, 2024-09-05, 삼상 */
uint8_t imax_cnt[3];

uint32_t imax_int[3];
int32_t imax_ce[3];
float imax_sum[3];
int8_t temp_over_val;

extern U8 g_avg3_index;
extern float g_v_thd_avg_buf[3][3];

bool sag_det_enabled = false;    // first should be inited !!!
bool swell_det_enabled = false;  // first should be inited !!!
bool sag_det_enabled_set;
bool swell_det_enabled_set;
int sag_dur_time;
int sag_detected_cnt;
int swell_dur_time;
int swell_detected_cnt;
uint16_t g_sag_thr, g_swell_thr;
uint32_t g_sag_thr_org, g_swell_thr_org;
uint32_t g_sag_thr_2p, g_swell_thr_2p;
bool g_cover_open_sts = 0;  // jp.kim 24.12.16

date_time_type dst_dt;
prepay_info_type prepay_info;

static bool scurr_timer_is_started;
static uint16_t scurr_relay_timer;
static uint8_t scurr_relay_sec;

static void scurr_limit_mon(void);
static void latch_relay_mon(void);
static void load_inited_set(void);
static void tamper_monthbit_set(uint8_t tamperbits);

extern void dsm_mtp_fsm_send(void);

void whm_init_1(void)
{
    U8 i, k;

    g_cover_open_sts = 0;  // jp.kim 24.12.16

    relay_onff_req = false;
    ts_testing = false;

    sag_detected_cnt = 0;
    swell_detected_cnt = 0;

    b_scurr_mon_ready = false;
    scurr_timer_is_started = false;

#if 0 /* bccho, 2024-09-05, 삼상 */
    intv_mon_dt.min = 0xff;
#endif

    // -------- initialized for CTT test (due to non-bss-init) ------------
    appl_set_save_idx = 0;
    appl_set_save_result = 0;

#if 0
	set_prog_dl_idx(); // prog_dl_idx is initialized because of CTT
	set_hol_dl_idx();  // used when tou continued to download and CTT
#endif

    g_avg3_index = 0;
    for (i = 0; i < 3; i++)
    {
        for (k = 0; k < 3; k++)
        {
            g_v_thd_avg_buf[i][k] = 0;
        }
    }
}

// -------------- 부가 신호 제어 ----------------
void sig_sel_out_proc(void)
{
    if (load_inited)
    {
        ts_ctrl();

        scurr_limit_mon();
    }

    latch_relay_mon();
}

void sig_sel_proc(uint8_t sel)
{
    if (load_inited)
    {
        if (sel != sig_sel)
        {
            switch (sel)
            {
            case SIG_NOSEL:
                relay_ctrl(1);
                break;
            case SIG_rLOAD_CONTROL:
                break;
            case SIG_TS_CONTROL:
                break;
            case SIG_sCURR_LIMIT:
                relay_ctrl(1);
                break;
            }
            if (sig_sel == SIG_sCURR_LIMIT)
            {
                scurr_limit_init();
                log_scurr_nonsel();
            }
        }
    }
    sig_sel = sel;
}

// --------------- 릴레이 제어 -----------------
void load_ctrl_init(void)
{
    if (run_is_bat_power())
    {
        MSG06("load_ctrl_init_run_is_bat_power()");
        return;
    }

    if (mt_is_time_sw())
    {
        MSG06("load_ctrl_init_mt_is_time_sw()");
        return;
    }

    if (mt_is_sCurr_limited())
    {
        MSG06("load_ctrl_init_mt_is_sCurr_limited()");
        if (scurr_autortn_cnt == 0)
        {
            scurr_step = 0;
            lp_event_unset(LPE_sCURR_LIMIT);

            scurr_limit_off();
        }
        else
        {
            if (scurr_limit_cnt < scurr_autortn_cnt)
            {
                scurr_step = 0;
                lp_event_unset(LPE_sCURR_LIMIT);

                scurr_limit_off();
            }
            else
            {
                scurr_step = 3;
                lp_event_set(LPE_sCURR_LIMIT);

                scurr_limit_on();
            }
        }
    }
    else
    {
        lp_event_unset(LPE_sCURR_LIMIT);

        if (relay_is_load_on())
        {
            MSG06("load_ctrl_init_relay_ctrl(1)");
            relay_ctrl(1);
        }
        else
        {
            MSG06("load_ctrl_init_relay_ctrl(0)");
            relay_ctrl(0);
        }
    }
}

void load_initialize(void)
{
    load_inited_set();
    relay_ctrl(1);
}

static void load_inited_set(void) { load_inited = 1; }

void relay_ctrl(bool ctrl)
{
    DPRINTF(DBG_TRACE, "Relay Ctrl: %d\r\n", ctrl);
    if (ctrl)
    {
        /* 부하 개방 */
        if (relay_load_state == LOAD_OFF)
        {
            dsp_load_dot_timeset(T5SEC);
            latchon_cnt_inc();
        }

        relay_dly_timeset(T1SEC);
        relay_err_timeset(T8SEC);
        /* LOAD O */
        relay_set_on();
        relay_load_state = LOAD_ON;
        whm_op_save();
    }
    else
    {
        /* 부하 차단 */
        if (relay_load_state == LOAD_ON)
        {
            dsp_load_dot_timeset(T5SEC);
        }

        relay_dly_timeset(T1SEC);
        relay_err_timeset(T8SEC);
        /* LOAD X */
        relay_set_off();
        relay_load_state = LOAD_OFF;
        whm_op_save();
    }

    relay_onff_timer = /*T50MS*/ T100MS;
    relay_onff_req = 1;
}

static void latch_relay_mon(void)
{
#if PHASE_NUM == SINGLE_PHASE
    if (relay_is_load_off() && relay_err_is_timeout())
    {
        if (curr_is_yesload_A())
        {
            LATCH_ERR_A_trigger();
            // WMStatus |= RELAY_ERRA;
            tamper_monthbit_set(RELAYERR_DETED_BIT_THISMONTH);
        }
        else
        {
            WMStatus &= ~RELAY_ERRA;
        }
        WMStatus &= ~(RELAY_ERRB | RELAY_ERRC);
    }
    else
    {
        WMStatus &= ~RLYERRMASK;
    }
#else
    if (relay_is_load_off() && relay_err_is_timeout())
    {
        if (curr_is_yesload_A())
        {
            LATCH_ERR_A_trigger();
            // WMStatus |= RELAY_ERRA;
            tamper_monthbit_set(RELAYERR_DETED_BIT_THISMONTH);
        }
        else
        {
            WMStatus &= ~RELAY_ERRA;
        }

        if (curr_is_yesload_B())
        {
            LATCH_ERR_B_trigger();
            // WMStatus |= RELAY_ERRB;
            tamper_monthbit_set(RELAYERR_DETED_BIT_THISMONTH);
        }
        else
        {
            WMStatus &= ~RELAY_ERRB;
        }
        if (curr_is_yesload_C())
        {
            LATCH_ERR_C_trigger();
            // WMStatus |= RELAY_ERRC;
            tamper_monthbit_set(RELAYERR_DETED_BIT_THISMONTH);
        }
        else
        {
            WMStatus &= ~RELAY_ERRC;
        }
    }
    else
    {
        WMStatus &= ~RLYERRMASK;
    }
#endif
}

void latchon_cnt_inc(void)
{
    latchon_data_type _latchon;

    if (!nv_read(I_UNINIT_DATA, (uint8_t *)&_latchon))
    {
        _latchon._latchoncnt = 0;
    }

    _latchon._latchoncnt += 1;
    nv_write(I_UNINIT_DATA, (uint8_t *)&_latchon);
}

void latchon_cnt_clear(void)
{
    latchon_data_type _latchon;

    _latchon._latchoncnt = 0;
    nv_write(I_UNINIT_DATA, (uint8_t *)&_latchon);
}

uint16_t get_latchon_cnt(void)
{
    latchon_data_type _latchon;

    if (!nv_read(I_UNINIT_DATA, (uint8_t *)&_latchon))
    {
        _latchon._latchoncnt = 0;
    }

    return _latchon._latchoncnt;
}

void rload_ctrl_set(uint8_t rctrl)
{
    DPRINTF(DBG_TRACE, "Load Ctrl: %d\r\n", rctrl);
    if (mt_is_sCurr_limited())
    {
        scurr_limit_init();

        log_scurr_nonsel();
    }

    sig_sel = SIG_rLOAD_CONTROL;

    if (rctrl)
    {
        load_inited_set();
        relay_ctrl(1);  // LOAD O
    }
    else
    {
        relay_ctrl(0);  // LOAD X
    }

    log_rLoad_ctrl();
}

bool ts_conf_available(void)
{
    if (ts_conf_cnt != 0 && ts_conf_ctrl != 0x00 && ts_conf_ctrl != 0xff)
        return true;

    return false;
}

void ts_ctrl(void)
{
    if (mt_is_time_sw())
    {
        if (ts_test_info.date != cur_date)
        {
            ts_test_info.used = 0;
            ts_test_info.date = cur_date;
            ts_test_sec = cur_sec;
        }

        if (ts_is_zone_on())
        {
            ts_test_off();
            if (!relay_is_load_on())
                relay_ctrl(1);
        }
        else
        {
            if (relay_is_load_on())
            {
                if (ts_testing == 0)
                {
                    relay_ctrl(0);
                }
                else
                {
                    if (ts_test_sec != cur_sec)
                    {
                        ts_test_sec = cur_sec;

                        ts_test_used_now += 1;
                        ts_test_info.used += 1;

                        if (ts_test_used_now >= 3 * 60 ||
                            ts_test_info.used >= 30 * 60)
                        {
                            relay_ctrl(0);
                            ts_test_off();
                        }
                    }
                }
            }
        }
    }
}

static time_sw_state_type rtn_ts_ctrl(uint8_t hh, uint8_t mm)
{
    uint8_t i;

    if (ts_conf_cnt == 0)
        return SW_CTRL_OFF;

    for (i = 0; i < ts_conf_cnt; i++)
    {
        if (cmp_hhmm(hh, mm, ts_conf_zone[i].hour, ts_conf_zone[i].min) < 0)
            break;
    }
    if (i == 0)
        i = ts_conf_cnt;

    i -= 1;
    return SELECTOR_TO_TS(i);
}

bool ts_is_zone_on(void)
{
    return rtn_ts_ctrl(cur_hour, cur_min) == SW_CTRL_ON;
}

void ts_test_on_start(void)
{
    ts_testing = 1;
    ts_test_sec = cur_sec;
    ts_test_used_now = 0;
}

void ts_test_off(void) { ts_testing = 0; }

uint16_t ts_test_total_used(void) { return ts_test_info.used; }

static bool scurr_is_within_step1(void)
{
    if (scurr_cnt_n1 == 0)
        return true;

    return (scurr_limit_cnt < scurr_cnt_n1) ? true : false;
}

static float get_scurr_limit_val(void)
{
    if (scurr_is_within_step1())
    {
        return (float)scurr_limit_level;
    }

    return (float)scurr_limit_level_2;
}

static void scurr_limit_mon(void)
{
    static int mon_cnt;
    float fval;
    float fval1;

    if (b_scurr_mon_ready == false)
        return;

    b_scurr_mon_ready = false;

    fval = get_inst_curr_allphase();
    fval *= 10.0;  // scaler (-1)

    /*
    구매 규격 3.17.2 시험 모드 표시항목
        ※ 과부하 전류 차단 기능이 OC-E로 설정될 경우, 과부하 전류 차단 동작 조건
    : 전류 계측값이 최대전류 (변성 	기부 계기의 경우 최대전류는
    정격전류의 1.2배)의 1.2배 이상이고, 온도 측정값이 +85℃ 이상인 경우로서, 두
    가지 조건이 모두 충족된 시점부터 1시간 이상 지속될 경우 자동으로 부하
    차단하여야 한다.
    */
    if (overcurr_cut_en)
    {
        if (fval >= (IoverThrsld * 1.2 * 10.0) && get_inst_temp() >= 85.0)
        {
            if (scurr_timer_is_started == false)
            {
                scurr_timer_is_started = true;

                scurr_relay_timer = 60 * 60;  // 60 분
                scurr_relay_sec = cur_sec;
            }
            else
            {
                if (scurr_relay_sec != cur_sec)
                {
                    scurr_relay_sec = cur_sec;
                    if (scurr_relay_timer)
                    {
                        if (--scurr_relay_timer == 0)
                        {
                            if (relay_is_load_on())
                            {
                                relay_ctrl(0);
                            }
                        }
                    }
                }
            }
            return;
        }
        else
        {
            scurr_timer_is_started = false;
        }
    }
    else
    {
        scurr_timer_is_started = false;
    }

    if (mt_is_sCurr_limited())
    {
#if 0
		fval = get_inst_power(true, 0);
#else
        /* JPKIM, 2024-10-04 */
        fval = get_inst_power_allphase();
#endif

        DPRINTF(DBG_TRACE, _D "%s: fval power[%d]\r\n", __func__,
                (uint32_t)fval);
        if (!kact_scurr_is_blocked())
        {
            switch (scurr_step)
            {
            case 0:
                fval1 = get_scurr_limit_val();
                if (fval > fval1 * 1.2)  // 20% up
                {
                    mon_cnt = scurr_det_hold;

                    scurr_step = 1;
                }
                break;

            case 1:
                fval1 = get_scurr_limit_val();
                if (fval > fval1 * 1.2)  // 20% up
                {
                    if (--mon_cnt == 0)
                    {
                        scurr_limit_on();
                        if (scurr_is_within_step1())
                        {
                            mon_cnt = scurr_rtn_dur_1;
                        }
                        else
                        {
                            mon_cnt = scurr_rtn_dur_2;
                        }

                        lp_event_set(LPE_sCURR_LIMIT);

                        log_scurr_limit();

                        scurr_limit_cnt += 1;

                        if ((scurr_autortn_cnt != 0) &&
                            (scurr_limit_cnt >= scurr_autortn_cnt))
                        {
                            scurr_step = 3;
                        }
                        else
                        {
                            scurr_step = 2;
                        }
                    }
                }
                else
                {
                    scurr_step = 0;
                }
                break;

            case 2:

                if (--mon_cnt == 0)
                {
                    scurr_limit_off();
                    scurr_step = 0;
                }
                else
                {
                    if ((scurr_autortn_cnt != 0) &&
                        (scurr_limit_cnt >= scurr_autortn_cnt))
                    {
                        scurr_step = 3;
                    }
                }
                break;

            default:
                break;
            }
        }
    }
    else
    {
        scurr_limit_init();
    }
}

void scurr_limit_init(void)
{
    scurr_step = 0;
    scurr_limit_cnt = 0;
}

bool scurr_is_limiting(void)
{
    if (mt_is_sCurr_limited() && scurr_step >= 2)
        return true;

    return false;
}

bool scurr_is_limiting_forever(void)
{
    if (mt_is_sCurr_limited() && scurr_step > 2)
        return true;

    return false;
}

void scurr_limit_on(void) { relay_ctrl(0); }

void scurr_limit_off(void) { relay_ctrl(1); }

bool is_sCurr_valid(uint16_t cur)
{
    if (cur >= 5 && cur <= (uint16_t)(MaxCur * 10.0 * 1.2))  // scale (-1)
        return true;

    return false;
}

int auto_bidir_chg_chk_cnt;

void is_sCurr_rev_cur_to_bidir(uint8_t *tptr)
{
/* bccho, 2024-09-05, 삼상 */
#if PHASE_NUM == SINGLE_PHASE
    //  2024.05.28 jp   전류검출방식  -> 유효전력 검출방식으로 변경
    if ((auto_mode_sel) && (mt_is_uni_dir()) && (meas_method != E_SINGLE_DIR) &&
        (!pulse_dir) && (get_inst_power(true, 0) >= (AutoModePower)))
#else /* bccho, 2024-09-24 */
    if ((auto_mode_sel) && (mt_is_uni_dir()) &&
        (((w0sum_mon < 0.0) && (get_inst_power(true, 0) >= (AutoModePower))) ||
         ((w1sum_mon < 0.0) && (get_inst_power(true, 1) >= (AutoModePower))) ||
         ((w2sum_mon < 0.0) && (get_inst_power(true, 2) >= (AutoModePower)))))
#endif
    {
        if (auto_bidir_chg_chk_cnt < 60)
        {
            //
            auto_bidir_chg_chk_cnt++;
        }
        else
        {
            uint8_t _mtdir;
            uint8_t eob_type = 0;
            ST_MIF_METER_PARM *pst_mif_meter_parm = dsm_mtp_get_meter_parm();

            DPRINTF(DBG_TRACE,
                    _D
                    "%s  2: R - curr[%d w]  auto_bidir_chg_chk_cnt[%d]  "
                    "mt_is_uni_dir()[%d] (AutoModePower)[%d w] \r\n",
                    __func__, (U32)(get_inst_power(true, 0)),
                    auto_bidir_chg_chk_cnt, mt_is_uni_dir(),
                    (U32)(AutoModePower));
            DPRINTF(DBG_TRACE,
                    _D
                    "%s  2: S - curr[%d w]  auto_bidir_chg_chk_cnt[%d]  "
                    "mt_is_uni_dir()[%d] (AutoModePower)[%d w] \r\n",
                    __func__, (U32)(get_inst_power(true, 1)),
                    auto_bidir_chg_chk_cnt, mt_is_uni_dir(),
                    (U32)(AutoModePower));
            DPRINTF(DBG_TRACE,
                    _D
                    "%s  2: T - curr[%d w]  auto_bidir_chg_chk_cnt[%d]  "
                    "mt_is_uni_dir()[%d] (AutoModePower)[%d w] \r\n",
                    __func__, (U32)(get_inst_power(true, 2)),
                    auto_bidir_chg_chk_cnt, mt_is_uni_dir(),
                    (U32)(AutoModePower));

            _mtdir = MT_BOTH_DIR;  // 0: 수전, 1: 송수전
            mt_dir = _mtdir;
            meas_method_adj_dirchg();

            pst_mif_meter_parm->direct_reverse = mt_dir;
            pst_mif_meter_parm->meter_method = meas_method;
            mif_meter_parm_set();

            MmodeCHG_sr_dr_type_sr_dr_time_set(T500MS);
            MmodeCHG_sr_dr_type_sr_dr_run_pending_sts = 1;

#if 0
            if (MmodeCHG_sr_dr_type)
                eob_type = EOB_nPERIOD_FLAG;

            sr_dr_proc(eob_type, MmodeCHG_sr_dr_type, &cur_rtc, tptr);
            lp_event_set(LPE_PROGRAM_CHG);
#endif
        }
    }
    else
    {
        auto_bidir_chg_chk_cnt = 0;
    }
}

static void tamper_monthbit_set(uint8_t tamperbits) { tamperbits = 0; }

void tamper_det_proc(void)
{
    static uint8_t mon_cnt = 0;
    static uint8_t mon_cnt3 = 0;

    if (tamp_det_is_timeout())
    {
        tamp_det_timeset(T1SEC);

        if (!MAGN_SENS_STATE)
        {
            if ((++mon_cnt) >= 4)
            {
                mon_cnt = 4;
                if ((tamper_det_bit & MAGNET_DETED_BIT) == 0)
                {
                    // magnet detected !!!
                    magnet_dt = cur_rtc;
                    magnet_det_dur = 1;
                }
                else
                {
                    magnet_det_dur += 1;
                }
                MAGNET_SENS_trigger();
                tamper_monthbit_set(MAGNET_DETED_BIT_THISMONTH);
            }
        }
        else
        {
            if (mon_cnt)
                mon_cnt -= 1;

            if (mon_cnt == 0)
            {
                if ((tamper_det_bit & MAGNET_DETED_BIT) != 0)
                {
                    log_magnet_det(&magnet_dt, magnet_det_dur);
                }

                tamper_det_bit &= ~MAGNET_DETED_BIT;
            }
        }

        tamper_det_bit &= ~MCOVEROPEN_DETED_BIT;

        if (TCOVER_OPEN_IN)
        {
            if ((++mon_cnt3) >= 2)
            {
                mon_cnt3 = 2;
                if ((tamper_det_bit & TCOVEROPEN_DETED_BIT) == 0)
                {
                    log_cover_open(eLogTCoverOpen);
                }
                tCOVER_OPEN_trigger();
                tamper_monthbit_set(MorTCOVEROPEN_DETED_BIT_THISMONTH);

                if (!g_cover_open_sts)  // jp.kim 24.12.17
                {
                    g_cover_open_sts = 1;

                    /* Work BlackOut */
                    if (run_is_main_power())
                    {
                        sagswell_start_timeset(T1SEC * working_fault_min * 60);
                        DPRINTF(DBG_ERR,
                                _D
                                "%s: sagswell_start_timeset "
                                "working_fault_min[%d]\r\n",
                                __func__, working_fault_min);
                    }
                }
            }
        }
        else
        {
            if (mon_cnt3)
            {
                mon_cnt3 -= 1;
            }

            if (mon_cnt3 == 0)
            {
                tamper_det_bit &= ~TCOVEROPEN_DETED_BIT;
                g_cover_open_sts = 0;  // jp.kim 24.12.16
            }
        }
    }
}

void meas_method_adj_dirchg(void)
{
    if (!mt_is_uni_dir())
    {
#if PHASE_NUM == SINGLE_PHASE
        meas_method = E_BASIC;
#else
        meas_method = E_VECTSUM;
#endif
    }
}

void set_billing_parm(uint8_t *parm)
{
    bool mt_transfer_set = 0;
    ST_MIF_METER_PARM *pst_mif_meter_parm = dsm_mtp_get_meter_parm();

    uint8_t *tptr;
    tptr = adjust_tptr(&global_buff[0]);
    uint8_t srdr;
#if defined(FEATURE_SEC)
    uint8_t eob_type = 0;
#endif
    srdr = 0;

#if 0  // defined(FEATURE_JP_seasonCHG_sr_dr_type_to_SET_SR)
	if((parm[0] & 0x10)&&(!(sr_dr_type[E_seasonCHG] & MR_SR_BIT)))
		{
			srdr |= MR_SR_BIT;
#if defined(FEATURE_SEC)
			if(srdr) eob_type |= EOB_SEASON_FLAG;
#endif		
		
			if((srdr & (MR_SR_BIT |MR_DR_BIT )) != 0)
			{
#if defined(FEATURE_SEC)
				sr_dr_proc(eob_type, srdr, &cur_rtc, tptr);
#else
				sr_dr_proc(srdr, &cur_rtc, tptr);
#endif
			}
		}
#endif

    sr_dr_type[E_mDR] = 0;
    sr_dr_type[E_pgmCHG] = 0;
    sr_dr_type[E_timCHG] = 0;
    sr_dr_type[E_seasonCHG] = 0;
    sr_dr_type[E_dmintvCHG] = 0;
    sr_dr_type[E_MmodeCHG] = 0;

    // self read(manual DR)
    if (parm[0] & 0x01)
        sr_dr_type[E_mDR] |= MR_SR_BIT;
    // demand reset(manual DR)
    if (parm[1] & 0x01)
        sr_dr_type[E_mDR] |= MR_DR_BIT;
    // self read(Program change)
    if (parm[0] & 0x02)
        sr_dr_type[E_pgmCHG] |= MR_SR_BIT;
    // demand reset(Program change)
    if (parm[1] & 0x02)
        sr_dr_type[E_pgmCHG] |= MR_DR_BIT;
    // self read(season change)
    if (parm[0] & 0x08)
        sr_dr_type[E_dmintvCHG] |= MR_SR_BIT;
    // demand reset(season change)
    if (parm[1] & 0x08)
        sr_dr_type[E_dmintvCHG] |= MR_DR_BIT;
    // self read(season change)
    if (parm[0] & 0x10)
        sr_dr_type[E_seasonCHG] |= MR_SR_BIT;
    // demand reset(season change)
    if (parm[1] & 0x10)
        sr_dr_type[E_seasonCHG] |= MR_DR_BIT;
    // self read(meter mode change)
    if (parm[0] & 0x40)
        sr_dr_type[E_MmodeCHG] |= MR_SR_BIT;
    // demand reset(meter mode change)
    if (parm[1] & 0x40)
        sr_dr_type[E_MmodeCHG] |= MR_DR_BIT;

    mt_transfer_set = 0;
    if (parm[2] != mt_dir)
    {
        mt_transfer_set = 1;
        mt_dir = parm[2];
    }

    if (parm[3] != mt_selreact)
    {
        mt_transfer_set = 1;
        mt_selreact = parm[3];
    }
    ToH32((U8_16_32 *)&mDR_limit_sec, &parm[4]);

    if (mt_transfer_set)
    {
        pst_mif_meter_parm->direct_reverse = mt_dir;
        pst_mif_meter_parm->reactive_select = mt_selreact;  // 선택 무효
        mif_meter_parm_set();
    }
}

void rtn_billing_parm(uint8_t *parm)
{
    /* 통신 규격 3.4.2.8.3 검침 파라미터 */

    uint8_t condition;

    /* 검침(self-read) 조건 :
        bit 6: 계량 모드 변경
        bit 5: 통신[향후 사용]
        bit 4: 계절 변경 검침
        bit 3: 수요시한 주기 변경
        bit 2: 날짜/시간 변경
        bit 1: 프로그램 변경
        bit 0: 수동검침
    */
    condition = 0;
    if (sr_dr_type[E_mDR] & MR_SR_BIT)
    {
        condition |= 0x01;  // 전력량 확정 (SR)
    }
    if (sr_dr_type[E_pgmCHG] & MR_SR_BIT)
    {
        condition |= 0x02;
    }
    if (sr_dr_type[E_timCHG] & MR_SR_BIT)
    {
        condition |= 0x04;
    }
    if (sr_dr_type[E_dmintvCHG] & MR_SR_BIT)
    {
        condition |= 0x08;
    }
    if (sr_dr_type[E_seasonCHG] & MR_SR_BIT)
    {
        condition |= 0x10;
    }
    if (sr_dr_type[E_MmodeCHG] & MR_SR_BIT)
    {
        condition |= 0x40;
    }
    parm[0] = condition;

    /* 수요전력 복귀(demand-reset) 조건 :
        bit 6: 계량 모드 변경
        bit 5: 통신[향후 사용]
        bit 4: 계절 변경 검침
        bit 3: 수요시한 주기 변경
        bit 2: 날짜/시간 변경
        bit 1: 프로그램 변경
        bit 0: 수동검침
    */
    condition = 0;
    if (sr_dr_type[E_mDR] & MR_DR_BIT)
    {
        condition |= 0x01;  // 최대수요전력 확정 (DR)
    }
    if (sr_dr_type[E_pgmCHG] & MR_DR_BIT)
    {
        condition |= 0x02;
    }
    if (sr_dr_type[E_timCHG] & MR_DR_BIT)
    {
        condition |= 0x04;
    }
    if (sr_dr_type[E_dmintvCHG] & MR_DR_BIT)
    {
        condition |= 0x08;
    }
    if (sr_dr_type[E_seasonCHG] & MR_DR_BIT)
    {
        condition |= 0x10;
    }
    if (sr_dr_type[E_MmodeCHG] & MR_DR_BIT)
    {
        condition |= 0x40;
    }
    parm[1] = condition;

    /* 선택 유효전력량 : 0 수전, 1 송/수전 */
    parm[2] = mt_dir;

    /* 선택 무효 전력량 : 1 지상, 2 진상, 3 지상+진상, 4 피상전려량 */
    parm[3] = mt_selreact;

    /* 수동 수요전력 복귀 방지 시간 : seconds */
    ToComm32(&parm[4], (U8_16_32 *)&mDR_limit_sec);
}

uint8_t conv_sel_to_DRkind(uint8_t sel)
{
    uint8_t rtn;

    rtn = 0;
    sel &= 0x03;

    if (sel == SEL_SR_DR)
    {
        rtn = MR_SR_BIT | MR_DR_BIT;
    }
    else if (sel == SEL_SR)
    {
        rtn = MR_SR_BIT;
    }

    return rtn;
}

uint8_t conv_DRkind_to_sel(uint8_t dr_kind)
{
    dr_kind &= 0x03;

    if (dr_kind == MR_SR_BIT)
        return SEL_SR;
    if (dr_kind == (MR_SR_BIT | MR_DR_BIT))
        return SEL_SR_DR;

    return SEL_SR_DR;
}

void set_pEOB_dr(uint8_t sel) { pEOB_sr_dr_type = conv_sel_to_DRkind(sel); }

void set_npEOB_dr(uint8_t sel)
{
    npEOB_sr_dr_type = conv_sel_to_DRkind(sel);
    DPRINTF(DBG_ERR, "%s: sel[%X], npEOB_sr_dr_type[%d]\r\n", __func__, sel,
            npEOB_sr_dr_type);
}

void dr_limit_time_clear(void) { DR_limit_timer_sec = 0L; }

void DR_limit_adj(int32_t padj)
{
    if (padj > 0)
    {
        if (padj >= DR_limit_timer_sec)
            DR_limit_timer_sec = 0L;
        else
            DR_limit_timer_sec -= padj;
    }
}

void DR_limit_dec(void)
{
    if (DR_limit_timer_sec > mDR_limit_sec)
        DR_limit_timer_sec = mDR_limit_sec;

    if (DR_limit_timer_sec)
        DR_limit_timer_sec--;
}

void prepay_reset(void)
{
    memset((uint8_t *)&prepay_info, 0, sizeof(prepay_info_type));
    nv_write(I_PREPAY, (uint8_t *)&prepay_info);
}

void prepay_info_save(bool _force)
{
    if (_force || (prepay_info._savedt != cur_date && cur_min != 0))
    {
        prepay_info._savedt = cur_date;

        nv_write(I_PREPAY, (uint8_t *)&prepay_info);
    }
}

void prepay_info_restore(bool saged)
{
    if (!saged)
    {
        if (crc16_chk((uint8_t *)&prepay_info, sizeof(prepay_info_type), false))
            return;
    }

    if (!nv_read(I_PREPAY, (uint8_t *)&prepay_info))
    {
        prepay_reset();
    }
    prepay_info._savedt = cur_date;
}

uint32_t get_prepay_info(uint8_t prekind)
{
    switch (prekind)
    {
    case 0:
        return prepay_info.buyenergy;
    case 1:
        return prepay_info.buymoney;
    case 2:
        return prepay_info.remenergy;
    case 3:
        return prepay_info.remmoney;
    case 4:
        return prepay_info.remtime;
    case 5:
        return prepay_info.enable;
    case 6:
        return prepay_info.cancel_flag;
    }

    return 0L;  // dummy
}

prepay_info_type *get_prepay_all(void) { return &prepay_info; }

BOOL is_prepay_load_on_continue_for_remenergy_0(void)
{
    BOOL ret = FALSE;

    if (is_weekends())
        return TRUE;

    if (is_phol_nphoiday(PERIODIC_HOL))
        return TRUE;

    if (is_phol_nphoiday(nPERIODIC_HOL))
        return TRUE;

    return ret;
}

void prepay_load_off(void)
{
    if (relay_is_load_on())
    {
        relay_ctrl(0);
        DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    }
}

void prepay_load_on(void)
{
    if (relay_is_load_off())
    {
        relay_ctrl(1);
        DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    }
}

#define ABS(v) ((v) < 0 ? -(v) : (v))
extern uint32_t dsm_get_dm_out_measure_print_chkcount(void);
extern void ENERGY_REMAINING_trigger(void);
extern void error_code_event_clear_for_prepay(void);
void prepay_proc(uint32_t sec_energy)
{
    bool plus2minus = false;
    int32_t pre_remenergy = 0;
    uint32_t remenergy_abs = 0;
    prepay_info_type *pst_prepay_all = get_prepay_all();
    uint32_t ch_count = dsm_get_dm_out_measure_print_chkcount();

    if (pst_prepay_all->enable == 1)
    {
        if (pst_prepay_all->remenergy > 0)
        {
            plus2minus = true;
            pre_remenergy = pst_prepay_all->remenergy;
        }
        pst_prepay_all->remenergy -= sec_energy;
        if (pst_prepay_all->remenergy <= 0)
        {
            if (plus2minus == true)
            {
                remenergy_abs = ABS(pst_prepay_all->remenergy);
                if (pst_prepay_all->remenergy == 0)
                {
                    ENERGY_REMAINING_trigger();
                }
                DPRINTF(DBG_WARN, "%s: pre_rem[%d], rem[%d], sec[%d]\r\n",
                        __func__, pre_remenergy, pst_prepay_all->remenergy,
                        sec_energy);
            }
            else
                plus2minus = false;
        }
        else
            plus2minus = false;

        if (ch_count % 10 == 1)
        {
            DPRINTF(DBG_TRACE, "%s: rem[%d], 1sec[%d]\r\n", __func__,
                    pst_prepay_all->remenergy, sec_energy);
        }

        if (pst_prepay_all->remenergy <= 0)
        {
            if (is_prepay_load_on_continue_for_remenergy_0() &&
                pst_prepay_all->cancel_flag)
            {
                if (pst_prepay_all->remenergy < 0)
                {
                    uint32_t buyenergy = pst_prepay_all->buyenergy;

                    if (plus2minus)
                    {
                        pst_prepay_all->buyenergy += remenergy_abs;
                        DPRINTF(DBG_WARN, "%s: buy[%d = %d  + %d]\r\n",
                                __func__, pst_prepay_all->buyenergy, buyenergy,
                                remenergy_abs);
                    }
                    else
                    {
                        pst_prepay_all->buyenergy += sec_energy;
                        if (ch_count % 10 == 1)
                        {
                            DPRINTF(DBG_TRACE, "%s: buy[%d = %d  + %d]\r\n",
                                    __func__, pst_prepay_all->buyenergy,
                                    buyenergy, sec_energy);
                        }
                    }
                }
                prepay_load_on();
            }
            else
            {
                prepay_load_off();
            }
        }
        else
        {
            if (is_WMStatus_prepay_remaining_0())
                error_code_event_clear_for_prepay();

            prepay_load_on();
        }
    }
}

// ------------ Sag and Swell ---------------------------

void mif_meter_sagswell_set(void)
{
    /* bccho, 2024-09-05, 삼상 */
    float fval, sag_level;

    ST_MIF_SAGSWELL_SETUP *pst_mif_sagswell = dsm_mtp_get_sagswell();

    nv_write(I_MTP_SAG_SWELL, (uint8_t *)pst_mif_sagswell);

    DPRINTF(DBG_TRACE, _D "SagSwell_moniter\r\n");
    ToHFloat((U8_Float *)&fval, &pst_mif_sagswell->pf_level[0]);
    DPRINTF(DBG_TRACE, _D "pf_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    DPRINTF(DBG_TRACE, _D "pf_contiue_time: 0x%04X\r\n",
            pst_mif_sagswell->pf_continue_time);

    ToHFloat((U8_Float *)&fval, &pst_mif_sagswell->sag_level[0]);

    /* bccho, 2024-09-05, 삼상 */
    sag_level = fval;
    DPRINTF(DBG_TRACE, _D "sag_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    DPRINTF(DBG_TRACE, _D "sag_time: 0x%02X\r\n", pst_mif_sagswell->sag_time);
    ToHFloat((U8_Float *)&fval, &pst_mif_sagswell->swell_level[0]);
    DPRINTF(DBG_TRACE, _D "swell_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    DPRINTF(DBG_TRACE, _D "swell_time: 0x%02X\r\n",
            pst_mif_sagswell->swell_time);

    /* bccho, 2024-09-05, 삼상 */
    volt_drop_thrd_set();

    dsm_mtp_set_fsm(MTP_FSM_SAG_SWELL);
    dsm_mtp_fsm_send();

#if 1  // jp.kim 25.03.12
    mif_meter_sagswe_set_wait_seq_cnt = 0;
    mif_meter_sagswe_set_wait_ing = 1;
    DPRINTF(DBG_ERR, "mif_meter_sagswe_set_wait_ing[%d] \r\n", __func__,
            mif_meter_sagswe_set_wait_ing);
    mif_meter_sagswe_set_wait_timeset(T500MS);
#endif
}

void sag_swell_save_set_start(uint8_t kind, uint8_t val, float fval)
{
    ST_MIF_SAGSWELL_SETUP *pst_mif_sagswell = dsm_mtp_get_sagswell();
    ST_MTP_SAGSWELL st_mtp_sagswell;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (!nv_read(I_MTP_SAG_SWELL, (uint8_t *)&st_mtp_sagswell))
    {
        dsm_mtp_default_sagswell(&st_mtp_sagswell);
    }
    switch (kind)
    {
    case 1:  // sag level
        fval = 220.0 * ((float)val) / 100.0;
        ToCommFloat(&st_mtp_sagswell.val.sag_level[0], (U8_Float *)&fval);
        break;

    case 2:  // sag time
        st_mtp_sagswell.val.sag_time = (uint8_t)fval;
        break;

    case 3:  // swell level
        fval = 220.0 * ((float)val) / 100.0;
        ToCommFloat(&st_mtp_sagswell.val.swell_level[0], (U8_Float *)&fval);
        break;

    case 4:  // swell time
        st_mtp_sagswell.val.swell_time = (uint8_t)fval;
        break;
    }

    memcpy((uint8_t *)pst_mif_sagswell, (uint8_t *)&st_mtp_sagswell.val,
           sizeof(ST_MIF_SAGSWELL_SETUP));
    mif_meter_sagswell_set();

    sag_swell_enable();
}

// ---------------- Bat installed and used time ------------------------

bool bat_is_installed(void)
{
    bat_install_type bat_inst;

    if (!nv_read(I_BAT_INST, (uint8_t *)&bat_inst))
    {
        bat_inst.installed = 0;
    }

    return bat_inst.installed != 0;
}

void input_mode_bat_installed_set(void)
{
    bat_installed_inputmode = bat_is_installed();
}

void bat_inst_completely_clear(void)
{
    bat_install_type bat_inst;

    bat_inst.batstate = 0;
    bat_inst.installed = 0;
    nv_write(I_BAT_INST, (uint8_t *)&bat_inst);
}

void bat_inst_clear(void)
{
    bat_install_type bat_inst;

    if (!nv_read(I_BAT_INST, (uint8_t *)&bat_inst))
    {
        bat_inst.installed = 0;
    }
    bat_inst.batstate = 0;
    nv_write(I_BAT_INST, (uint8_t *)&bat_inst);
}

void bat_inst_set(void)
{
    bat_install_type bat_inst;

    bat_inst.batstate = 1;
    bat_inst.installed = 1;
    bat_inst.instime = cur_rtc;

    nv_write(I_BAT_INST, (uint8_t *)&bat_inst);

    bat_used_time_clear();
}

void bat_inst_proc(bat_inst_evt_type evt)
{
    bat_install_type bat_inst;

    switch (evt)
    {
    case BIEVT_BAT_OUT:
        MSG06("---BAT_OUT");
        if (!nv_read(I_BAT_INST, (uint8_t *)&bat_inst))
        {
            bat_inst.installed = 0;
        }
        bat_inst.batstate = 0;

        nv_write(I_BAT_INST, (uint8_t *)&bat_inst);
        break;

    case BIEVT_BAT_IN:
        MSG06("---BAT_IN");
        if (!nv_read(I_BAT_INST, (uint8_t *)&bat_inst))
        {
            bat_inst.batstate = 0;
            bat_inst.installed = 0;
        }

        if (bat_inst.batstate == 0)
        {
            if (cur_year != 0)
            {
                bat_inst_set();
            }
            else
            {
                bat_inst.batstate = 1;
                nv_write(I_BAT_INST, (uint8_t *)&bat_inst);
            }
        }
        break;

    case BIEVT_TIMECHG:
        if (!nv_read(I_BAT_INST, (uint8_t *)&bat_inst))
        {
            bat_inst.batstate = 0;
            bat_inst.installed = 0;
        }

        if (cur_year != 0)
        {
            if (!run_is_main_power())
                goto bat_inst_proc1;

            if (bat_inst.batstate != 0)
            {
                if (dsp_is_input_state())
                {
                bat_inst_proc1:
                    if (!bat_installed_inputmode)
                    {
                        bat_inst_set();
                    }
                }
                else
                {
                    if (bat_inst.installed == 0)
                    {
                        bat_inst_set();
                    }
                }
            }
        }
        break;
    }
}

void bat_used_time_clear(void)
{
    bat_used_time.bat_used = 0L;
    bat_used_time.batused_dt = cur_rtc;
    nv_write(I_BAT_USE, (uint8_t *)&bat_used_time);
}

void bat_used_time_add_save(int32_t u_tim, date_time_type *dt)
{
    bat_used_time.bat_used += u_tim;
    bat_used_time.batused_dt = *dt;
    nv_write(I_BAT_USE, (uint8_t *)&bat_used_time);
}

void bat_used_time_backup(date_time_type *dt)
{
    if (!nv_read(I_BAT_USE, (uint8_t *)&bat_used_time))
    {
        bat_used_time.bat_used = 0L;
    }
    bat_used_time.batused_dt = *dt;

    nv_write(I_BAT_USE, (uint8_t *)&bat_used_time);
}

uint32_t get_bat_used_time(void)
{
    uint32_t t32;

    t32 = 0L;
    if (bat_is_installed())
    {
        if (nv_read(I_BAT_USE, (uint8_t *)&bat_used_time))
        {
            t32 = bat_used_time.bat_used;
        }
    }

    return t32;
}

void bat_used_time_proc(bool rtc_backed, date_time_type *dt)
{
    if (rtc_backed)
    {
        bat_used_time_re_set(dt, dt);
    }
}

void bat_used_time_re_set(date_time_type *olddt, date_time_type *newdt)
{
    int32_t t32;

    if (bat_is_installed())
    {
        if (nv_read(I_BAT_USE, (uint8_t *)&bat_used_time))
        {
            t32 = calc_dtime_diff(olddt, &bat_used_time.batused_dt);
            if (t32 < 0)
                t32 = 0;

            bat_used_time_add_save(t32, newdt);
        }
        else
        {
            bat_used_time.bat_used = 0L;
        }
    }
}

// ------------- Daylight Savings Time (DST) ----------------

static void update_dst_date(date_time_type *_dt, uint8_t wk)
{
    uint8_t i;
    uint8_t _firstwk;

    _firstwk = calc_dayofweek(_dt);
    for (i = 0; i < 7; i++)
    {
        if (_firstwk == wk)
            break;

        _dt->date += 1;
        if (++_firstwk > 7)
            _firstwk = 1;
    }
}

static void dst_bgn_dt_get(uint8_t yr, date_time_type *rtn_dt)
{
    *rtn_dt = dls_info.bgn_dt;
    if (dls_info.bgn_dt.year == 0xff)
    {
        rtn_dt->year = yr;

        if ((rtn_dt->date % 7) == 1 && dls_info.bgn_week != 0xff)
        {
            update_dst_date(rtn_dt, dls_info.bgn_week);
        }
    }
}

static void dst_end_dt_get(uint8_t yr, date_time_type *rtn_dt)
{
    *rtn_dt = dls_info.end_dt;
    if (dls_info.end_dt.year == 0xff)
    {
        if (dls_info.bgn_dt.month < dls_info.end_dt.month)
        {
            rtn_dt->year = yr;
        }
        else
        {
            rtn_dt->year = yr + 1;
        }

        if ((rtn_dt->date % 7) == 1 && dls_info.end_week != 0xff)
        {
            update_dst_date(rtn_dt, dls_info.end_week);
        }
    }
}

static bool DLS_is_active_zone(date_time_type *dt)
{
    date_time_type dst_dt;

    dst_bgn_dt_get(dt->year, &dst_dt);
    if (cmp_date_time(dt, &dst_dt) >= 0)
    {
        dst_end_dt_get(dt->year, &dst_dt);
        if (cmp_date_time(dt, &dst_dt) < 0)
        {
            return true;
        }
    }
    return false;
}

void dst_mon_rtchg(date_time_type *dt)
{
    /* DayLightSaving */
    if (DLS_is_enabled())
    {
        if (DLS_is_active_zone(dt))
        {
            DLS_active_set();
            DLS_bgn_dev_set();
            DLS_end_dev_unset();
        }
        else
        {
            DLS_active_unset();
            DLS_bgn_dev_unset();
            DLS_end_dev_unset();
        }
    }
    else
    {
        DLS_active_unset();
    }
}

bool dst_mon(bool _normal, date_time_type *dt, uint8_t *tptr)
{
    bool is_dev;
    rate_type rt;
    date_time_type _bakdt;
    date_time_type _dst_dt;

    is_dev = false;
    _bakdt = *dt;

    /*DayLightSaving*/
    if (DLS_is_enabled())
    {
        if (!DLS_is_bgn_dev())
        {
            dst_bgn_dt_get(dt->year, &_dst_dt);
            if (cmp_date_time(dt, &_dst_dt) >= 0)
            {
                dst_end_dt_get(dt->year, &_dst_dt);
                if (cmp_date_time(dt, &_dst_dt) < 0)
                {
                    DLS_bgn_dev_set();

                    DLS_active_set();

                    cur_rtc = *dt;
                    if (dls_info.dev > 0)
                    {
                        time_up(&cur_rtc, dls_info.dev);
                    }
                    else
                    {
                        time_down(&cur_rtc, (-1) * dls_info.dev);
                    }
                dst_mon1:
                    write_rtc(&cur_rtc);

                    if (_normal)
                    {
                        rt = cur_rate_before;
                        eoi_proc_timechg(dt, rt, tptr, false);

                        dst_dt = _bakdt;
                    }

                    is_dev = true;
                }
                else
                {
                    DLS_bgn_dev_set();
                    DLS_end_dev_set();
                }
            }
        }
        else if (!DLS_is_end_dev())
        {
            dst_end_dt_get(dt->year, &_dst_dt);
            if (cmp_date_time(dt, &_dst_dt) >= 0)
            {
                DLS_end_dev_set();

                DLS_active_unset();

                cur_rtc = *dt;
                if (dls_info.dev > 0)
                {
                    time_down(&cur_rtc, dls_info.dev);
                }
                else
                {
                    time_up(&cur_rtc, (-1) * dls_info.dev);
                }

                goto dst_mon1;
            }
        }
        else
        {
            dst_end_dt_get(dt->year, &_dst_dt);
            if (cmp_date_time(dt, &_dst_dt) >= 0)
            {
                DLS_bgn_dev_unset();
                DLS_end_dev_unset();
            }
        }
    }
    else
    {
        DLS_active_unset();
    }

    return is_dev;
}

// ----------------- Wrong Connection Monitor ------------------

void wrong_conn_mon(void)
{
    if (line_is_wrong_connected())
    {
        if (wrong_conn_state == 0)
        {
            wrong_conn_state = 1;
            log_wrong_conn();
        }
    }
    else
    {
        wrong_conn_state = 0;
    }
}

uint32_t get_max_lcd_kwh(uint8_t pnt)
{
    switch (pnt)
    {
    case 0:
        return MAX_LCD_DIGIT_KWH;
    case 1:
        return MAX_LCD_DIGIT_KWH / 10L;
    case 2:
        return MAX_LCD_DIGIT_KWH / 100L;
    case 3:
        return MAX_LCD_DIGIT_KWH / 1000L;
    case 4:
        return MAX_LCD_DIGIT_KWH / 10000L;
    case 5:
        return MAX_LCD_DIGIT_KWH / 100000L;
    default:
        return MAX_LCD_DIGIT_KWH;
    }
}

void adjust_max_whcnt(uint8_t w_pnt, uint8_t wh_pnt)
{
    mxdm_dgt_cnt = get_max_lcd_kwh(w_pnt);
    mxdm_dgt_cnt *= (uint32_t)PulseKwh;

    mxaccm_dgt_cnt = get_max_lcd_kwh(wh_pnt);
    mxaccm_dgt_cnt *= (uint32_t)PulseKwh;
}

uint16_t conv_point_to_mul(uint8_t pt)
{
    switch (pt)
    {
    case 0:
        return 1;
    case 1:
        return 10;
    case 2:
        return 100;
    case 3:
        return 1000;
    case 4:
        return 10000;
    default:
        return 1;
    }
}

bool crc16_chk(uint8_t *buf, uint16_t len, bool set)
{
    return fcs16(buf, len, set);
}

void get_bf_interval_boundary(date_time_type *pdate_time, uint8_t intv)
{
    uint8_t t8;

    if ((pdate_time->min % intv) == 0 && pdate_time->sec == 0)
        return;

    t8 = pdate_time->min % intv;
    pdate_time->min -= t8;

    pdate_time->sec = 0;
}

void get_interval_boundary(date_time_type *pdate_time, uint8_t intv)
{
    if ((pdate_time->min % intv) == 0 && pdate_time->sec == 0)
        return;

    get_next_interval_boundary(pdate_time, intv);
}

void get_next_interval_boundary(date_time_type *pdate_time, uint8_t intv)
{
    uint8_t tmp;

    time_up(pdate_time, intv);

    tmp = pdate_time->min % intv;
    pdate_time->min -= tmp;

    pdate_time->sec = 0;
}

bool leap_year(uint16_t yr)
{
    if ((yr % 400) == 0)
        return true;
    else if ((yr % 100) == 0)
        return false;
    else if ((yr % 4) == 0)
        return true;

    return false;
}

const enum touDAY day_tbl[7] = {_SAT, _SUN, _MON, _TUE, _WED, _THU, _FRI};
uint8_t calc_dayofweek(date_time_type *dt)
{
    int16_t t16;
    date_time_type ref_dayofweek = {
        00,
        01,
        01,
    };  // 2000.01.01 (SAT)

    if (dt->year == 0xff)
        return 0xff;  // not specified in commnunication

    t16 = calc_date_diff(dt, &ref_dayofweek);
    t16 %= 7;

    return (uint8_t)day_tbl[t16];
}

const uint8_t date_per_month[12] = {31, 28, 31, 30, 31, 30,
                                    31, 31, 30, 31, 30, 31};
int16_t calc_date_diff(date_time_type *ed, date_time_type *st)
{
    uint8_t tmp2, tmp3;
    uint16_t i, tmp1;
    uint16_t tmp4;

    if (cmp_date(ed, st) < 0)
        return (-1);

    tmp1 = (ed->year - st->year) * 12 + ed->month - st->month;
    tmp2 = st->month - 1;
    tmp3 = st->year;
    tmp4 = 0;  // date calculation
    for (i = 0; i < tmp1; i++)
    {
        tmp4 += date_per_month[tmp2];

        if ((tmp2 + 1) == 2)
        {  // Feburary
            if (leap_year((uint16_t)tmp3 + BASE_YEAR))
                tmp4++;
        }

        if (tmp2 == 11)
        {
            tmp2 = 0;
            tmp3++;  // cross year
        }
        else
            tmp2++;
    }

    tmp4 += ed->date;
    tmp4 -= st->date;

    return tmp4;
}

int32_t calc_date_time_diff(date_time_type *ed, date_time_type *st)
{
    int8_t tmp_rtn1, tmp_rtn2;
    int16_t tmp1;
    uint32_t tmp2;

    tmp_rtn1 = cmp_date(ed, st);
    tmp_rtn2 = cmp_time(ed, st);

    if (tmp_rtn1 == (-1) || (tmp_rtn1 == 0 && tmp_rtn2 == (-1)))
        return (-1);

    tmp1 = calc_date_diff(ed, st);

    tmp2 = (uint32_t)tmp1 * (uint32_t)86400;  // sec (24*60*60)
    tmp2 += ((uint32_t)ed->hour * 3600 + (uint32_t)ed->min * 60 +
             (uint32_t)ed->sec);
    tmp2 -= ((uint32_t)st->hour * 3600 + (uint32_t)st->min * 60 +
             (uint32_t)st->sec);

    return tmp2;
}

int32_t calc_dtime_diff(date_time_type *todt, date_time_type *fromdt)
{
    int32_t diff;

    diff = calc_date_time_diff(todt, fromdt);
    if (diff == (-1))
    {
        diff = calc_date_time_diff(fromdt, todt);
        diff = (-1) * diff;
    }

    return diff;
}

// Returns:
//		ed > st : 1
//		ed = st : 0
//		ed < st : -1
int8_t cmp_date(date_time_type *ed, date_time_type *st)
{
    if (ed->year > st->year)
        return (1);
    else if (ed->year < st->year)
        return (-1);

    if (ed->month > st->month)
        return (1);
    else if (ed->month < st->month)
        return (-1);

    if (ed->date > st->date)
        return (1);
    else if (ed->date < st->date)
        return (-1);
    else
        return (0);
}

// Returns:
//		ed > st : 1
//		ed = st : 0
//		ed < st : -1
int8_t cmp_mmdd(mm_dd_type *ed, mm_dd_type *st)
{
    if (ed->month > st->month)
        return (1);
    if (ed->month < st->month)
        return (-1);

    if (ed->date > st->date)
        return (1);
    if (ed->date < st->date)
        return (-1);

    return (0);
}

// Returns:
//		ed > st : 1
//		ed = st : 0
//		ed < st : -1
int8_t cmp_hhmm(uint8_t ed_h, uint8_t ed_m, uint8_t st_h, uint8_t st_m)
{
    if (ed_h > st_h)
        return (1);
    if (ed_h < st_h)
        return (-1);

    if (ed_m > st_m)
        return (1);
    if (ed_m < st_m)
        return (-1);

    return (0);
}

// Returns:
//		ed > st : 1
//		ed = st : 0
//		ed < st : -1
int8_t cmp_time(date_time_type *ed, date_time_type *st)
{
    if (ed->hour > st->hour)
        return (1);
    if (ed->hour < st->hour)
        return (-1);

    if (ed->min > st->min)
        return (1);
    if (ed->min < st->min)
        return (-1);

    if (ed->sec > st->sec)
        return (1);
    if (ed->sec < st->sec)
        return (-1);

    return (0);
}

// Returns:
//		ed > st : 1
//		ed = st : 0
//		ed < st : -1
int8_t cmp_date_time(date_time_type *ed, date_time_type *st)
{
    int8_t t8;

    t8 = cmp_date(ed, st);
    if (t8 != 0)
        return t8;

    return cmp_time(ed, st);
}

const uint8_t date_per_month_sel[13] = {0,  31, 28, 31, 30, 31, 30,
                                        31, 31, 30, 31, 30, 31};
uint8_t last_date_of_month(uint8_t yr, uint8_t mon)
{
    if (mon == 2)
    {
        if (leap_year(yr + BASE_YEAR))
            return 29;
        return 28;
    }
    return date_per_month_sel[mon];
}

bool rtc_is_valid(date_time_type *dt)
{
    uint8_t last_date;

    if (dt->month < 1 || dt->month > 12)
        return false;

    last_date = last_date_of_month(dt->year, dt->month);
    if (dt->date < 1 || dt->date > last_date)
        return false;

    if (dt->hour > 23 || dt->min > 59 || dt->sec > 59)
        return false;

    return true;
}

// _up is within 1 month
void date_up(date_time_type *dt, uint8_t _up)
{
    uint8_t t8;

    dt->date += _up;

    t8 = last_date_of_month(dt->year, dt->month);

    if (dt->date > t8)
    {
        dt->date -= t8;

        dt->month++;
        if (dt->month > 12)
        {
            dt->month = 1;
            dt->year++;
        }
    }
}

// _down is within 1 month
void date_down(date_time_type *dt, uint8_t _down)
{
    uint8_t t8;

    if (dt->date > _down)
    {
        dt->date -= _down;
        return;
    }

    if (dt->month == 1)
    {
        dt->year -= 1;
        dt->month = 12;
    }
    else
    {
        dt->month -= 1;
    }

    t8 = last_date_of_month(dt->year, dt->month);

    dt->date = dt->date + t8 - _down;
}

void time_up(date_time_type *dt, uint8_t min)
{
    dt->min += min;
    if (dt->min < 60)
        return;

    while (dt->min >= 60)
    {
        dt->min -= 60;
        dt->hour++;
    };

    if (dt->hour < 24)
        return;

    dt->hour -= 24;

    date_up(dt, 1);
}

void time_down(date_time_type *dt, uint8_t min)
{
    uint8_t t8;

    t8 = min / 60;
    min %= 60;

    if (dt->min >= min)
    {
        dt->min -= min;
    }
    else
    {
        t8 += 1;
        dt->min = 60 + dt->min - min;
    }

    if (t8 == 0)
        return;

    if (dt->hour >= t8)
    {
        dt->hour -= t8;
        return;
    }

    dt->hour = 24 + dt->hour - t8;

    date_down(dt, 1);
}

bool is_good_day(enum touDAY _day)
{
    if (_day >= _MON && _day <= _SUN)
        return true;

    return false;
}

bool is_good_date(date_time_type *dt)
{
    uint8_t t8;

    t8 = last_date_of_month(dt->year, dt->month);

    if ((dt->month > 0 && dt->month < 13) && (dt->date > 0 && dt->date <= t8))
        return true;

    return false;
}

bool is_good_time(date_time_type *dt)
{
    if (dt->hour < 24 && dt->min < 60 && dt->sec < 60)
        return true;

    return false;
}

bool is_good_date_time(date_time_type *dt)
{
    if (is_good_date(dt) && is_good_time(dt))
        return true;

    return false;
}
