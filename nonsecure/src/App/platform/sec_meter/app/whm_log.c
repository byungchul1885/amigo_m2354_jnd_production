#include "options.h"
#include "port.h"
#include "afe.h"
#include "tmp.h"
#include "nv.h"
#include "bat.h"
#include "whm_log.h"
#include "get_req.h"

#include "main.h"

#define _D "[WHM_L] "

#define TEMP_DEBOUNCE 2

extern date_time_type sr_log_dt;

static void imax_var_init(void);
static void log_dt_data(elog_kind_type elog, date_time_type* dt);
static void log_dt_dur_data(elog_kind_type elog, elog_durtime_kind_type edurlog,
                            date_time_type* pdt, uint32_t dur);
static void v_abnormal_chk(uint32_t _linests);
static void wrong_neut_chk(uint32_t _linests);
static void i_abnormal_chk(uint32_t _linests);
static void w_abnormal_chk(uint32_t _linests);
static void wrong_conn_chk(void);
static void log_dt_cert_data(elog_cert_kind_type elog, date_time_type* pdt,
                             uint8_t cert_ng);
void LOWVOT_trigger(uint8_t line);
void OVERVOT_trigger(uint8_t line);

void log_data_reset(void)
{
    int i;

    for (i = 0; i < numLogs; i++) whm_op.logcnt[i] = 0;
}

void log_mt_initialization(uint8_t* tptr)
{
    uint8_t logcnt;
    mtinit_log_data_type* mtinit;

    mtinit = (mtinit_log_data_type*)tptr;
    if (nv_read(I_MTINIT_LOG, (uint8_t*)mtinit))
    {
        logcnt = mtinit->cnt;
    }
    else
    {
        logcnt = 0;
    }

    mtinit->log.dt[logcnt % LOG_BUFF_SIZE] = cur_rtc;
    mtinit->cnt = logcnt + 1;

    nv_write(I_MTINIT_LOG, (uint8_t*)mtinit);
}

void log_mt_init_clear(uint8_t* tptr)
{
    mtinit_log_data_type* mtinit;

    mtinit = (mtinit_log_data_type*)tptr;
    mtinit->cnt = 0;
    nv_write(I_MTINIT_LOG, (uint8_t*)mtinit);
}

uint16_t get_mtinit_log_cnt(uint8_t* tptr)
{
    mtinit_log_data_type* mtinit;

    mtinit = (mtinit_log_data_type*)tptr;

    if (nv_read(I_MTINIT_LOG, tptr))
    {
        return (uint16_t)mtinit->cnt;
    }

    return 0;
}

void log_sag_swell(elog_kind_type elog)
{
    if (elog == eLogSag)
    {
        DPRINTF(DBG_TRACE, _D "%s: SAG\r\n", __func__);
    }
    else if (elog == eLogSwell)
    {
        DPRINTF(DBG_TRACE, _D "%s: SWELL\r\n", __func__);
    }
    else
    {
        DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    }
    log_dt_data(elog, &cur_rtc);
}

void log_sr_dr(date_time_type* dt, uint8_t srdr)
{
    DPRINTF(DBG_TRACE, _D "%s: %d\r\n", __func__, srdr);
    if (srdr & MR_SR_BIT)
    {
        // 자기 검침 (Self Read)
        log_dt_data(eLogSR, dt);
        sr_log_dt = mr_dt;
    }

    if (srdr & MR_DR_BIT)
    {
        // 수요전력 복귀 (Demand Reset)
        log_dt_data(eLogDRa, dt);
    }
}

void log_mDR(date_time_type* dt) { log_dt_data(eLogDRm, dt); }

void log_prog_chg(date_time_type* pdt)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    log_dt_data(eLogPgmChg, pdt);
}

void log_rtc_chg(date_time_type* fr_dt, date_time_type* to_dt)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    log_dt_data(eLogRtcF, fr_dt);
    log_dt_data(eLogRtcT, to_dt);
}

void log_scurr_limit(void)
{
    scurr_log_info_type scur;

    scur.dt = cur_rtc;
    scur.scurrcnt = scurr_limit_cnt + 1;
    scur.limit = scurr_limit_level;
    scur.limit2 = scurr_limit_level_2;
    scur.limitcnt = scurr_autortn_cnt;
    scur.limitcnt_n1 = scurr_cnt_n1;
    nv_sub_info.ch[0] = log_cnt[eLogSCurrLimit] % LOG_BUFF_SIZE;
    nv_write(I_LOG_SCURR, (uint8_t*)&scur);
    log_cnt[eLogSCurrLimit] += 1;
}

void log_scurr_nonsel(void) { log_dt_data(eLogSCurrNonSel, &cur_rtc); }

void log_pwr_FR(pwrfail_info_type* pfinfo)
{
    Work_PwrF_data_type _val_flag;
    //    DPRINTF(DBG_ERR, _D"%s\r\n", __func__);
    // DPRINTF(DBG_TRACE, _D"%s\r\n", __func__);
    log_dt_data(eLogPwrF_WorkPwrF, &pfinfo->dt);
    log_dt_data(eLogPwrR_WorkPwrR, &cur_rtc);

#if 1  // jp.kim 24.10.28
    if (nv_read(I_WORKPWR_FLAG, (U8*)&_val_flag))
    {
        if (_val_flag._val != 'T')
        {
            log_dt_data(eLogPwrF, &pfinfo->dt);
            log_dt_data(eLogPwrR, &cur_rtc);
        }
        else
        {
            log_dt_data(eLogWorkPwrF, &pfinfo->dt);
        }

        DPRINTF(DBG_ERR,
                " 1 %s if(nv_read(I_WORKPWR_FLAG, (U8 *)&_val_flag)) "
                "_val_flag[%c][0x%x] \r\n",
                __func__, _val_flag, _val_flag);
    }
    else
    {
        log_dt_data(eLogPwrF, &pfinfo->dt);
        log_dt_data(eLogPwrR, &cur_rtc);

        DPRINTF(DBG_ERR,
                " 2 %s if(nv_read(I_WORKPWR_FLAG, (U8 *)&_val_flag)) "
                "_val_flag[%c][0x%x] \r\n",
                __func__, _val_flag, _val_flag);
    }
#endif
}

void log_rLoad_ctrl(void) { log_dt_data(eLogrLoadCtrl, &cur_rtc); }

void log_magnet_det(date_time_type* pdt, uint32_t dur)
{
    log_dt_dur_data(eLogMagnetDet, eLogMagnetDetOcur, pdt, dur);
}

void log_cover_open(elog_kind_type elog) { log_dt_data(elog, &cur_rtc); }

void log_wrong_conn(void) { log_dt_data(eLogWrongConn, &cur_rtc); }

void log_sys_sw_up(void) { log_dt_data(eLogSysSwUp, &cur_rtc); }

void log_mtr_sw_up(void) { log_dt_data(eLogMtrSwUp, &cur_rtc); }

void log_int_modem_up(void) { log_dt_data(eLogInModemUp, &cur_rtc); }

void log_ext_modem_up(void) { log_dt_data(eLogExModemUp, &cur_rtc); }

void imax_log_init(void)
{
    uint8_t i;
    imax_log_type imax;

    imax_var_init();

    for (i = 0; i < PHASE_NUM; i++)
    {
        nv_sub_info.ch[0] = i;
        if (!nv_read(I_IMAX_LOG, (uint8_t*)&imax))
        {
            memset((uint8_t*)&imax, 0, sizeof(imax_log_type));
        }

        imax_val_set(i, &imax);
    }
}

void temp_over_log_init(void)
{
    tempover_log_type tover;

    b_temp_mon_ready = false;

    if (nv_read(I_TOVER_LOG, (uint8_t*)&tover))
    {
        temp_over_val = tover.val;
    }
    else
    {
        temp_over_val = 0;
    }
}

void imax_val_set(uint8_t ch, imax_log_type* imax)
{
    imax_int[ch] = imax->imaxint;
    imax_ce[ch] = imax->imaxce;
}

#if 1 /* bccho, 2024-09-05, 삼상 */
void imax_log_proc_line(float curr_in, uint8_t i)
{
    uint32_t irms;
    imax_log_type imax;

    imax_sum[i] += curr_in;

    if ((++imax_cnt[i]) >= 5)
    {
        imax_sum[i] /= 5;
        if (imax_sum[i] > imax_ce[i])
        {
            imax_ce[i] = imax_sum[i];
            irms = (uint32_t)rtn_inst_irms(imax_sum[i] * 1000);
            if (irms > imax_int[i])
            {
                imax_int[i] = irms;

                imax.imaxint = imax_int[i];
                imax.imaxce = imax_ce[i];
                imax.dt = cur_rtc;

                nv_sub_info.ch[0] = i;
                nv_write(I_IMAX_LOG, (U8*)&imax);
            }
        }

        imax_sum[i] = 0L;
        imax_cnt[i] = 0;
    }
}

void imax_log_proc(void)
{
    if (!b_imax_ready)
        return;

    b_imax_ready = false;

    imax_log_proc_line(i0sqsum_mon, 0);
    imax_log_proc_line(i1sqsum_mon, 1);
    imax_log_proc_line(i2sqsum_mon, 2);
}
#else
void imax_log_proc(void)
{
    uint8_t i;
    uint32_t irms;
    imax_log_type imax;

    if (!b_imax_ready)
        return;

    b_imax_ready = false;

    imax_sum[0] += i0sqsum_mon;
    if ((++imax_cnt) >= 5)
    {
        for (i = 0; i < PHASE_NUM; i++)
        {
            imax_sum[i] /= 5;
            if (imax_sum[i] > imax_ce[i])
            {
                imax_ce[i] = imax_sum[i];

                irms = (uint32_t)rtn_inst_irms(imax_sum[i] * 1000);
                if (irms > imax_int[i])
                {
                    imax_int[i] = irms;

                    imax.imaxint = imax_int[i];
                    imax.imaxce = imax_ce[i];
                    imax.dt = cur_rtc;

                    nv_sub_info.ch[0] = i;
                    nv_write(I_IMAX_LOG, (uint8_t*)&imax);
                }
            }
            imax_sum[i] = 0L;
        }
        imax_cnt = 0;
    }
}
#endif

void bat_det_mon(void)
{
    bool batok;

    GPIO_SetPullCtl(PC, BIT13, GPIO_PUSEL_PULL_DOWN);

    // MSG07("bat: %d, %d", BAT_DET_IN, cur_rtc.sec);
    vTaskDelay(1);

    if (!BAT_DET_IN)
    {
        if (nobat_cnt < NOBAT_DECISION_CNT)
        {
            ++nobat_cnt;
        }
    }
    else
    {
        if (nobat_cnt)
        {
            nobat_cnt--;
        }
    }

    GPIO_SetPullCtl(PC, BIT13, GPIO_PUSEL_DISABLE);

    if (nobat_cnt == 0)
    {
        batok = true;
    }
    else if (nobat_cnt >= NOBAT_DECISION_CNT)
    {
        batok = false;
    }
    else
    {
        return;  // exit
    }

    WMStatus_intern |= GE_NOBAT_CHKED;
    if (batok)
    {
        WMStatus &= ~GE_NOBAT;

        if (bat_state != BATSTATE_IN)
        {
            bat_state = BATSTATE_IN;
            bat_inst_proc(BIEVT_BAT_IN);
        }
    }
    else
    {
        NO_BATTERY_trigger();
        WMStatus_intern &= ~GE_LOWBAT;  // low bat not used

        lp_event_set(LPE_NO_BAT);

        if (bat_state != BATSTATE_OUT)
        {
            bat_state = BATSTATE_OUT;
            bat_inst_proc(BIEVT_BAT_OUT);
        }
    }
}

void temp_over_mon(void)
{
    static uint8_t debounce_cnt = 0;
    int8_t tempval;
    tempover_log_type tover;

    tempval = (int8_t)get_inst_temp();
    if ((temp_thrshld != 0) && (tempval > temp_thrshld))
    {
        if (debounce_cnt < TEMP_DEBOUNCE)
        {
            debounce_cnt++;
        }
        else
        {
            TEMPOVER_trigger();

            if (tempval > temp_over_val)
            {
                tover.val = (tempval < 85) ? tempval : 85;
                tover.dt = cur_rtc;
                nv_write(I_TOVER_LOG, (uint8_t*)&tover);

                temp_over_val = tempval;
            }
        }
    }
    else
    {
        if (debounce_cnt)
            --debounce_cnt;

        if (debounce_cnt == 0)
        {
            WMStatus &= ~TEMPOVER;
        }
    }
}

void magnet_err_mon(void)
{
    if ((tamper_det_bit & MAGNET_DETED_BIT) != 0L)
    {
        WMStatus |= MAGNET_ERROR;
    }
    else
    {
        WMStatus &= ~MAGNET_ERROR;
    }
}

void tcover_open_mon(void)
{
    if (IS_TCOVER_OPEN)
        WMStatus |= COVER_OPEN;
    else
        WMStatus &= ~COVER_OPEN;
}

void mt_abnorm_mon_init(void)
{
    b_abnormal_chk_ready = false;

    wrong_conn_prev = false;

    vdrop_cnt[0] = vdrop_cnt[1] = vdrop_cnt[2] = DROP_V_CNT;
    iover_cnt[0] = iover_cnt[1] = iover_cnt[2] = OVER_I_CNT;

#if 1
    v_low_cnt[0] = v_low_cnt[1] = v_low_cnt[2] = LOW_V_CNT;
#endif

    whbwd_cnt = WRONG_WH_CNT;
}

void mt_abnorm_mon(void)
{
    uint32_t _linests;

    if (b_abnormal_chk_ready == false)
        return;

    b_abnormal_chk_ready = false;

    _linests = get_LineStatus();

    v_abnormal_chk(_linests);

#if PHASE_NUM != SINGLE_PHASE
    wrong_neut_chk(_linests);
#endif

    i_abnormal_chk(_linests);

    w_abnormal_chk(_linests);

    wrong_conn_chk();
    magnet_err_mon();
    tcover_open_mon();
}

/* bccho, 2024-06-17, 김종필대표 패치 적용 */
extern ST_MTP_PUSH_DATA g_mtp_pushdata;

float vrms_mon_get(uint8_t line)
{
    switch (line)
    {
    case 1:
        return vrms1_mon;
        break;

    case 2:
        return vrms2_mon;
        break;

    default:
        return vrms0_mon;

        break;
    }
}

void LOWVOT_off(uint8_t line)
{
    switch (line)
    {
    case 1:
        WMStatus &= ~(LB_V_LOW);
        break;

    case 2:
        WMStatus &= ~(LC_V_LOW);
        break;

    default:
        WMStatus &= ~(LA_V_LOW);
        break;
    }
}

void OVERVOT_off(uint8_t line)
{
    switch (line)
    {
    case 1:
        WMStatus &= ~(LB_V_HIGH);
        break;

    case 2:
        WMStatus &= ~(LC_V_HIGH);
        break;

    default:
        WMStatus &= ~(LA_V_HIGH);
        break;
    }
}

static void v_abnormal_chk(uint32_t _linests)
{
#if PHASE_NUM == SINGLE_PHASE
    (void)_linests;

    WMStatus &= ~(SAGVA | SAGVB | SAGVC);

    if (low_volt_detect_mask_zon_cnt <= 0)
    {
        int i;
        i = 0;

        if ((220.0 * (100.0 - 6.0) / 100.0) > vrms_mon_get(i))
        {
            if (v_low_cnt[i] == 0)
            {
                lp_event_set(LPE_VOL_LOW);
                LOWVOT_trigger(i);
                OVERVOT_off(i);
            }
            else
            {
                v_low_cnt[i] -= 1;
            }
        }
        else
        {
            if (v_low_cnt[i] >= LOW_V_CNT)
            {
                LOWVOT_off(i);
                v_low_cnt[i] = LOW_V_CNT;
            }
            else
            {
                v_low_cnt[i] += 1;
            }
        }

        if ((220.0 * (100.0 + 6.0) / 100.0) < vrms_mon_get(i))
        {
            lp_event_set(LPE_VOL_OVER);
            OVERVOT_trigger(i);
            LOWVOT_off(i);
        }
        else
        {
            OVERVOT_off(i);
        }
    }
#else /* bccho, 2025-01-03, 삼상 */
    int i;

    if (low_volt_detect_mask_zon_cnt <= 0)
    {
        for (i = 0; i < PHASE_NUM; i++)
        {
            if ((220.0 * (100.0 - 6.0) / 100.0) > vrms_mon_get(i))
            {
                if (v_low_cnt[i] == 0)
                {
                    lp_event_set(LPE_VOL_LOW);
                    LOWVOT_trigger(i);
                    OVERVOT_off(i);
                }
                else
                {
                    v_low_cnt[i] -= 1;
                }
            }
            else
            {
                if (v_low_cnt[i] >= LOW_V_CNT)
                {
                    LOWVOT_off(i);
                    v_low_cnt[i] = LOW_V_CNT;
                }
                else
                {
                    v_low_cnt[i] += 1;
                }
            }

            if ((220.0 * (100.0 + 6.0) / 100.0) < vrms_mon_get(i))
            {
                lp_event_set(LPE_VOL_OVER);
                OVERVOT_trigger(i);
                LOWVOT_off(i);
            }
            else
            {
                OVERVOT_off(i);
            }
        }
    }

    /* 결상 체크 */
    extern U8 phase_fail_back[3];
    extern U8 phase_fail_sts[3];

    for (i = 0; i < PHASE_NUM; i++)
    {
        for (i = 0; i < PHASE_NUM; i++)
        {
            if (!phase_fail_back[i] && phase_fail_sts[i])
            {
                // 결상 시작 push
                NO_PHASE_set_trigger(i);
                // WMStatus |= (SAGVA << i);
            }
            else if (phase_fail_back[i] && !phase_fail_sts[i])
            {
                // 결상 복구 push
                NO_PHASE_rcv_trigger(i);
                // WMStatus &= ~(SAGVA << i);
            }
        }
    }
#endif
}

#if PHASE_NUM != SINGLE_PHASE
static void wrong_neut_chk(uint32_t _linests)
{
    if (_linests & (LA_VOVER | LB_VOVER | LC_VOVER))
    {
        NEUT_WRONGCONN_trigger();
    }
    else
    {
        WMStatus &= ~WRONG_NEUT;
    }
}
#endif

static void i_abnormal_chk(uint32_t _linests)
{
    int i;

    for (i = 0; i < PHASE_NUM; i++)
    {
        if (_linests & (LA_IOVER << i))
        {
            if (iover_cnt[i] == 0)
            {
                i = OVERCURR_trigger(i);
            }
            else
            {
                iover_cnt[i] -= 1;
            }
        }
        else
        {
            if (iover_cnt[i] >= OVER_I_CNT)
            {
                WMStatus &= ~(STOVERIA << i);
                iover_cnt[i] = OVER_I_CNT;
            }
            else
            {
                iover_cnt[i] += 1;
            }
        }
    }
}

static void w_abnormal_chk(uint32_t _linests)
{
    if (mt_is_uni_dir())
    {
#if PHASE_NUM == SINGLE_PHASE
        if ((_linests & LA_WHBACK) != 0)
#else
        if ((_linests & (LA_WHBACK | LB_WHBACK | LC_WHBACK)) != 0)
#endif
        {
            if (whbwd_cnt == 0)
            {
                WRONGCONN_trigger();
            }
            else
            {
                whbwd_cnt -= 1;
            }
        }
        else
        {
            if (whbwd_cnt >= WRONG_WH_CNT)
            {
                WMStatus &= ~WRONG_CONN;
                whbwd_cnt = WRONG_WH_CNT;
            }
            else
            {
                whbwd_cnt += 1;
            }
        }
    }
    else
    {
        WMStatus &= ~WRONG_CONN;
    }
}

static void wrong_conn_chk(void)
{
    if ((WMStatus & (WRONG_NEUT | WRONG_CONN)) != 0L)
    {
        if (!wrong_conn_prev)
        {
            wrong_conn_prev = true;

            lp_event_set(LPE_WRONGCONN_WRONGNEUT);
        }
    }
    else
    {
        wrong_conn_prev = false;
    }
}

static void imax_var_init(void)
{
    int i;

    b_imax_ready = false;

    /* bccho, 2024-09-05, 삼상 */
    imax_cnt[0] = 0;
    imax_cnt[1] = 0;
    imax_cnt[2] = 0;

    for (i = 0; i < PHASE_NUM; i++) imax_sum[i] = 0.0;
}

static void log_dt_data(elog_kind_type elog, date_time_type* pdt)
{
    date_time_type dt;

    dt = *pdt;
    nv_sub_info.ch[0] = elog;
    nv_sub_info.ch[1] = log_cnt[elog] % LOG_BUFF_SIZE;
    nv_write(I_LOG_DATA, (uint8_t*)&dt);
    log_cnt[elog] += 1;
}

static void log_dt_dur_data(elog_kind_type elog, elog_durtime_kind_type edurlog,
                            date_time_type* pdt, uint32_t dur)
{
    evt_durtime_type evt;

    evt.dt = *pdt;
    evt.durtime = dur;
    nv_sub_info.ch[0] = log_cnt[elog] % LOG_BUFF_SIZE;
    nv_write(I_LOG_DATA1, (uint8_t*)&evt);
    log_cnt[elog] += 1;
}

static void log_dt_cert_data(elog_cert_kind_type elog, date_time_type* pdt,
                             uint8_t cert_ng)
{
    evt_cert_time_type evt;

    evt.dt = *pdt;
    evt.ng_case = cert_ng;

    nv_sub_info.ch[0] = elog;
    nv_sub_info.ch[1] = log_cert_cnt[elog] % LOG_CERT_BUFF_SIZE;
    nv_write(I_LOG_CERT_DATA, (uint8_t*)&evt);
    log_cert_cnt[elog] += 1;
}

void log_cert_ng(uint8_t cert_ng)
{
    DPRINTF(DBG_WARN, _D "%s: cert_ng[0x%02X]\r\n", __func__, cert_ng);
    log_dt_cert_data(eLogCert_NG, &cur_rtc, cert_ng);
}
