#include "main.h"
#include "options.h"
#include "meter_app.h"
#include "comm.h"
#include "eoi.h"
#include "eob.h"
#include "lp.h"
#include "program.h"
#include "nv.h"
#include "phy.h"
#include "key.h"
#include "amg_meter_main.h"

#define _D "[KEYIN] "

typedef union
{
    date_time_type dt;
    tou_struct_type touset[MAX_TOU_DIV_TWOKIND];
    ts_struct_type tsset[4];
    ad_sig_type sigsel;
    uint8_t lpintv;
} inp_var_type;

#define dt_bakup inp_var.dt
#define inp_touset inp_var.touset
#define inp_tsset inp_var.tsset
#define inp_sig_sel inp_var.sigsel
#define inp_lpintv inp_var.lpintv

static inp_var_type inp_var;

static bool rtkind_chged;
ratekind_type rtkind_bakup;
U8 mrdate_bakup;

static uint8_t tou_inp_index;
static bool tou_inp_err;
static bool tou_inp_chged;
static uint8_t ts_inp_index;
static bool ts_inp_err;
static bool sig_sel_chged;
static bool lpe_proced_in_sigsel;
static uint8_t err_pulse_backup;

bool scurr_block_dur_kinp;
bool input_state_by_comm;

static void kinp_dsp_date(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_time(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_regrd(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_sigsel(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_sCurr(kaction_type kact);
static void kinp_dsp_sCurrCnt(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_ts(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_mtkind(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_tou(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_lpintv(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_sn1(kaction_type kact);
static void kinp_dsp_sn2(kaction_type kact);
static void kinp_dsp_mtdir(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_meas(kaction_type kact, uint8_t *tptr);
static void kinp_dsp_baud(kaction_type kact);
static void kinp_dsp_temp(kaction_type kact);
static void kinp_dsp_errpls(kaction_type kact);
static void kinp_dsp_condensor(kaction_type kact);
static void kinp_dsp_commen(kaction_type kact);
static bool hh_mm_is_valid(uint8_t hh, uint8_t mn);
static void kinp_dsp_sCurr_2(kaction_type kact);
static void act_dispinp_scurrhold(kaction_type kact);
static void act_dispinp_scurr_n1(kaction_type kact);
static void act_dispinp_scurr_dur1(kaction_type kact);
static void act_dispinp_scurr_dur2(kaction_type kact);
static void act_dispinp_contract_month(kaction_type kact);
bool sCurr_n1_is_valid(uint8_t _n1);
void mif_meter_parm_set(void);

#if 0 /* bccho, 2024-01-10, 1227 포팅 */
void kinp_init(void)
{
    scurr_block_dur_kinp = false;
    dt_bakup = cur_rtc;
    dsp_inp_init(&dt_bakup);

    input_mode_bat_installed_set();
}

void kact_inp_dsp_exit(void) { dsp_circ_state_init(); }

void kact_inp_exit(void)
{
    scurr_block_dur_kinp = false;
    dsp_inp_end_init();
}

void kact_inp_dsp(kaction_type kact, uint8_t *tptr)
{
    disp_input_type dspinp;

    dspinp = get_dsp_inputmode();

    DPRINTF(DBG_TRACE, _D "%s: dspinp[%d], Kact[0x%02X]\r\n", __func__, dspinp,
            kact);

    switch (dspinp)
    {
    case DISPINP_DATE:  // date input mode
        kinp_dsp_date(kact, tptr);
        break;

    case DISPINP_TIME:  // time input mode
        kinp_dsp_time(kact, tptr);
        break;

    case DISPINP_REGREAD_DATE:  // regular reading date input mode
        kinp_dsp_regrd(kact, tptr);
        break;

    case DISPINP_SIGSEL:  // signal selection input mode
        kinp_dsp_sigsel(kact, tptr);
        break;

    case DISPINP_sCURR:
        kinp_dsp_sCurr(kact);
        break;

    case DISPINP_sCURR_aRTN:
        kinp_dsp_sCurrCnt(kact, tptr);
        break;

    case DISPINP_TS:
        kinp_dsp_ts(kact, tptr);
        break;

    case DSPINP_CONTRACT_MONTH:
        act_dispinp_contract_month(kact);
        break;

    case DISPINP_RATEKIND:  // rate kind input mode
        kinp_dsp_mtkind(kact, tptr);
        break;

    case DISPINP_TOU:  // TOU input mode (only TWO rate kind)
        kinp_dsp_tou(kact, tptr);
        break;

    case DISPINP_DMINTV:  // demand interval input mode
        kinp_dsp_lpintv(kact, tptr);
        break;

    case DISPINP_SN_1:  // can not modify
        kinp_dsp_sn1(kact);
        break;

    case DISPINP_SN_2:  // can not modify
        kinp_dsp_sn2(kact);
        break;

    case DISPINP_MT_DIR:
        kinp_dsp_mtdir(kact, tptr);
        break;

    case DISPINP_MEAS:
        kinp_dsp_meas(kact, tptr);
        break;

    case DISPINP_BAUD:
        kinp_dsp_baud(kact);
        break;

    case DISPINP_TEMP:
        kinp_dsp_temp(kact);
        break;

    case DISPINP_ERR_PLS:
        kinp_dsp_errpls(kact);
        break;

    case DISPINP_CONDENSOR:
        kinp_dsp_condensor(kact);
        break;

    case DISPINP_COMMEN_COVEROPEN:
        kinp_dsp_commen(kact);
        break;

    case DISPINP_sCURR_2:
        kinp_dsp_sCurr_2(kact);
        break;

    case DISPINP_sCURR_HOLD:
        act_dispinp_scurrhold(kact);
        break;

    case DISPINP_sCURR_RTN1:
        act_dispinp_scurr_n1(kact);
        break;

    case DISPINP_sCURR_RTN_DUR1:
        act_dispinp_scurr_dur1(kact);
        break;

    case DISPINP_sCURR_RTN_DUR2:
        act_dispinp_scurr_dur2(kact);
        break;

    case DISPINP_END:
        break;
    }
}

uint8_t get_tou_inp_index(void) { return tou_inp_index; }

static void inp_tou_get(uint8_t idx, tou_struct_type *tou)
{
    if (tou_inp_err)
    {
        *tou = inp_touset[idx];
    }
    else
    {
        if (idx >= tou_data_cnt)
        {
            idx = (uint8_t)(tou_data_cnt - 1);
        }

        *tou = tou_data_conf[idx];
    }
}

static uint8_t get_diff_rate_time(rate_type rt, tou_struct_type *conf,
                                  uint8_t idx, uint8_t mx)
{
    for (; idx < mx; idx++)
    {
        if (rt != SELECTOR_TO_RATE(conf[idx].rate))
            break;
    }

    if (idx == mx)
        idx = 0xff;

    return idx;
}

bool is_inp_tou_chged(ratekind_type rtkind)
{
    rate_type rt;
    uint8_t idx1, idx2;

    if (rtkind != TWO_RATE_KIND)
        return 0;

    if (tou_data_cnt == 0)
        return 1;

    rt = SELECTOR_TO_RATE(inp_touset[MAX_TOU_DIV_TWOKIND - 1]
                              .rate);  // 첫 번재의 시작 rate 를 찾기 위함

    idx1 = idx2 = 0;
    while (1)
    {
        idx1 = get_diff_rate_time(rt, inp_touset, idx1, MAX_TOU_DIV_TWOKIND);
        idx2 = get_diff_rate_time(rt, tou_data_conf, idx2, tou_data_cnt);

        if (idx1 == 0xff || idx2 == 0xff)
            break;

        if (inp_touset[idx1].hour != tou_data_conf[idx2].hour ||
            inp_touset[idx1].min != tou_data_conf[idx2].min)
            break;

        rt = SELECTOR_TO_RATE(inp_touset[idx1].rate);
    }

    if (idx1 == 0xff && idx2 == 0xff)
        return 0;

    return 1;
}

static bool is_tou_conf_valid(void)
{
    uint8_t i;
    int8_t k;

    // 2 종인 경우 만

    for (i = 1; i < MAX_TOU_DIV_TWOKIND; i++)
    {
        if (inp_touset[0].rate != inp_touset[i].rate)
            break;
    }
    if (i == MAX_TOU_DIV_TWOKIND)
        return false;

    for (i = 0; i < MAX_TOU_DIV_TWOKIND; i++)
    {
        if (hh_mm_is_valid(inp_touset[i].hour, inp_touset[i].min) == false)
            break;
    }
    if (i < MAX_TOU_DIV_TWOKIND)
        return false;

    for (i = 0; i < (MAX_TOU_DIV_TWOKIND - 1); i++)
    {
        k = cmp_hhmm(inp_touset[i].hour, inp_touset[i].min,
                     inp_touset[i + 1].hour, inp_touset[i + 1].min);
        if (k > 0)
            break;

        if (k == 0)
        {
            if (inp_touset[i].rate != inp_touset[i + 1].rate)
                break;
        }
    }

    return i == (MAX_TOU_DIV_TWOKIND - 1);
}

static void inp_mt_rate_conf(void)
{
    uint8_t i;

    tou_data_cnt = MAX_TOU_DIV_TWOKIND;

    for (i = 0; i < MAX_TOU_DIV_TWOKIND; i++)
    {
        tou_data_conf[i].rate = inp_touset[i].rate;
        tou_data_conf[i].hour = inp_touset[i].hour;
        tou_data_conf[i].min = inp_touset[i].min;
    }
}

static void inp_ts_get(uint8_t idx, ts_struct_type *ts)
{
    if (ts_inp_err)
    {
        *ts = inp_tsset[idx];
    }
    else
    {
        if (idx >= ts_conf_cnt)
            idx = ts_conf_cnt - 1;

        ts->hour = ts_conf_zone[idx].hour;
        ts->min = ts_conf_zone[idx].min;
        ts->on_off = SELECTOR_TO_TS(idx);
    }
}

static uint8_t get_diff_ts_time1(uint8_t onff, uint8_t idx)
{
    for (; idx < 4; idx++)
    {
        if (onff != inp_tsset[idx].on_off)
            break;
    }

    if (idx == 4)
        idx = 0xff;

    return idx;
}

static uint8_t get_diff_ts_time2(uint8_t onff, uint8_t idx)
{
    for (; idx < ts_conf_cnt; idx++)
    {
        if (onff != SELECTOR_TO_TS(idx))
            break;
    }

    if (idx == ts_conf_cnt)
        idx = 0xff;

    return idx;
}

static bool is_inp_ts_chged(void)
{
    uint8_t onff;
    uint8_t idx1, idx2;

    onff = inp_tsset[3].on_off;  // 첫 번재의 시작 on_off 를 찾기 위함

    idx1 = idx2 = 0;
    while (1)
    {
        idx1 = get_diff_ts_time1(onff, idx1);
        idx2 = get_diff_ts_time2(onff, idx2);

        if (idx1 == 0xff || idx2 == 0xff)
            break;

        if (inp_tsset[idx1].hour != ts_conf_zone[idx2].hour ||
            inp_tsset[idx1].min != ts_conf_zone[idx2].min)
            break;

        onff = inp_tsset[idx1].on_off;
    }

    if (idx1 == 0xff && idx2 == 0xff)
        return false;

    return true;
}

static bool is_ts_conf_valid(void)
{
    uint8_t i;
    int8_t k;

    for (i = 0; i < 4; i++)
    {
        if (hh_mm_is_valid(inp_tsset[i].hour, inp_tsset[i].min) == false)
            break;
    }
    if (i < 4)
        return false;

    for (i = 0; i < 3; i++)
    {
        k = cmp_hhmm(inp_tsset[i].hour, inp_tsset[i].min, inp_tsset[i + 1].hour,
                     inp_tsset[i + 1].min);
        if (k > 0)
            break;

        if (k == 0)
        {
            if (inp_tsset[i].on_off != inp_tsset[i + 1].on_off)
                break;
        }
    }

    return i == 3;
}

static void inp_mt_ts_conf(void)
{
    uint8_t i;

    ts_conf_cnt = 4;

    ts_conf_ctrl = 0;
    for (i = 0; i < 4; i++)
    {
        ts_conf_ctrl |= TS_TO_SELECTOR(i, inp_tsset[i].on_off);
        ts_conf_zone[i].hour = inp_tsset[i].hour;
        ts_conf_zone[i].min = inp_tsset[i].min;
    }
}

uint8_t get_ts_inp_index(void) { return ts_inp_index; }

// ----------------- 키 입력 모드 ------------------

static void kinp_dsp_date(kaction_type kact, uint8_t *tptr)
{
    date_time_type tdt;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(0, 5);
        break;

    case KACT_MENU:
        tdt.year = get_dispinp_year();
        tdt.month = get_dispinp_month();
        tdt.date = get_dispinp_date();

        if (is_good_date(&tdt))
        {
            if (cmp_date(&tdt, &dt_bakup) != 0)
            {
                tdt.hour = cur_hour;
                tdt.min = cur_min;
                tdt.sec = cur_sec;

                rtc_chg_proc(&tdt, tptr);
                if (run_is_bat_power())
                {
                    whm_op_save();
                }
            }
        }

        dt_bakup = cur_rtc;
        dsp_inp_time_init(&dt_bakup);

        break;

    default:
        break;
    }
}

static void kinp_dsp_time(kaction_type kact, uint8_t *tptr)
{
    date_time_type tdt;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(0, 3);
        break;

    case KACT_MENU:
        tdt.hour = get_dispinp_hour();
        tdt.min = get_dispinp_min();
        tdt.sec = dt_bakup.sec;

        if (is_good_time(&tdt))
        {
            if (cmp_time(&tdt, &dt_bakup) != 0)
            {
                tdt.year = dt_bakup.year;
                tdt.month = dt_bakup.month;
                tdt.date = dt_bakup.date;
                tdt.sec = 0;

                rtc_chg_proc(&tdt, tptr);
                if (run_is_bat_power())
                {
                    whm_op_save();
                }
            }
        }

        dsp_inp_regrd_init(reg_mr_date);

        break;

    default:
        break;
    }
}

static void kinp_dsp_regrd(kaction_type kact, uint8_t *tptr)
{
    // TODO : (WD) Check, 정기 검침일 설정

    uint8_t t8;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(4, 5);
        break;

    case KACT_MENU:
        t8 = get_dispinp_date();
        if (t8 > 28)
            t8 = 1;
        if (t8 > 0)
        {
            if (t8 != reg_mr_date)
            {
                reg_mr_date = t8;

                prog_chg_proc_by_key(mt_dir, mt_rtkind, meas_method, tptr);
                if (run_is_bat_power())
                {
                    whm_op_save();
                    mt_conf_save();
                    mt_conf_2_save();
                }
            }
        }
        inp_sig_sel = sig_sel;

        dsp_inp_sigsel_init(sig_sel);
        break;

    default:
        break;
    }
}

static void kinp_dsp_sigsel(kaction_type kact, uint8_t *tptr)
{
    bool needsave;
    ts_struct_type tts;

    switch (kact)
    {
    case KACT_MOVE:
        break;

    case KACT_MENU:
        sig_sel_chged = 0;
        lpe_proced_in_sigsel = 0;
        needsave = false;
        if (inp_sig_sel != sig_sel)
        {
            sig_sel_chged = 1;
            lpe_proced_in_sigsel = 1;

            sig_sel_proc(inp_sig_sel);

            if (load_inited)
            {
                ts_ctrl();
            }

            prog_chg_proc_by_key(mt_dir, mt_rtkind, meas_method, tptr);

            needsave = true;
        }

        if (sig_sel == SIG_sCURR_LIMIT)
        {
            scurr_block_dur_kinp = true;
            if (sig_sel_chged)
            {
                default_scurr_parm();
                needsave = true;
            }
            dsp_inp_sCurrCnt_init(scurr_autortn_cnt);
        }
        else if (sig_sel == SIG_TS_CONTROL)
        {
            ts_inp_index = 0;
            ts_inp_err = FALSE;
            if (sig_sel_chged)
            {
                default_ts_zone();
                needsave = true;
            }
            inp_ts_get(ts_inp_index, &tts);
            dsp_inp_ts_init(&tts);
        }
        else
        {
            dsp_inp_contract_month_init(contract_dm_month);
        }

        if (run_is_bat_power() && needsave)
        {
            whm_op_save();
            mt_conf_save();
            mt_conf_2_save();
        }

        break;

    default:
        break;
    }
}

static void kinp_dsp_sCurr(kaction_type kact)
{
    uint16_t t16;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(1, 5);
        break;

    case KACT_MENU:
        t16 = get_dispinp_sCurr();
        if (sCurr_limit_is_valid(t16))
        {
            if (t16 != scurr_limit_level)
            {
                scurr_limit_level = t16;

                if (lpe_proced_in_sigsel == false)
                {
                    lpe_proced_in_sigsel = true;

                    prog_in_state = E_PROG_KEY;

                    if (run_is_main_power())
                    {
                        lp_event_set(LPE_PROGRAM_CHG);
                    }
                    else
                    {
                        lp_save_batmode(LPE_PROGRAM_CHG);
                        whm_op_save();
                    }
                }

                if (run_is_bat_power())
                {
                    mt_conf_save();
                    mt_conf_2_save();
                }
            }

            dsp_inp_scurr_dur1_init(scurr_rtn_dur_1);
        }
        else
        {
            dsp_inp_scurr_init(t16);
            dsp_inp_err_set();
        }
        break;

    default:
        break;
    }
}

static void kinp_dsp_sCurrCnt(kaction_type kact, uint8_t *tptr)
{
    uint8_t t8;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(4, 5);
        break;

    case KACT_MENU:
        t8 = get_dispinp_sCurrCnt();
        if (t8 != scurr_autortn_cnt)
        {
            if (t8 == 0)
            {
                scurr_limit_init();
                scurr_limit_off();
            }

            scurr_autortn_cnt = t8;

            if (lpe_proced_in_sigsel == false)
            {
                lpe_proced_in_sigsel = true;
                if (run_is_main_power())
                {
                    lp_event_set(LPE_PROGRAM_CHG);
                }
                else
                {
                    lp_save_batmode(LPE_PROGRAM_CHG);
                    whm_op_save();
                }
            }

            if (run_is_bat_power())
            {
                mt_conf_save();
                mt_conf_2_save();
            }
        }

        dsp_inp_scurr_hold_init(scurr_det_hold);
        break;

    default:
        break;
    }
}

static void kinp_dsp_ts(kaction_type kact, uint8_t *tptr)
{
    ts_struct_type tts;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(1, 5);
        break;

    case KACT_MENU:
        inp_tsset[ts_inp_index].hour = get_dispinp_tou_hour();
        inp_tsset[ts_inp_index].min = get_dispinp_tou_min();
        inp_tsset[ts_inp_index].on_off = get_dispinp_ts_ctrl();
        ts_inp_index++;

        if (ts_inp_index == 4)
        {
            if (sig_sel_chged || is_inp_ts_chged())
            {
                if (is_ts_conf_valid())
                {
                    inp_mt_ts_conf();

                    if (!lpe_proced_in_sigsel)
                    {
                        prog_chg_proc_by_key(mt_dir, mt_rtkind, meas_method,
                                             tptr);
                        if (run_is_bat_power())
                        {
                            whm_op_save();
                        }
                    }

                    if (run_is_bat_power())
                    {
                        mt_conf_save();
                        mt_conf_2_save();
                    }

                    lpe_proced_in_sigsel = true;
                }
                else
                {
                    ts_inp_err = TRUE;
                    ts_inp_index = 0;
                    inp_ts_get(ts_inp_index, &tts);
                    dsp_inp_ts_init(&tts);
                    dsp_inp_err_set();
                    break;
                }
            }

            dsp_inp_contract_month_init(contract_dm_month);
        }
        else
        {
            inp_ts_get(ts_inp_index, &tts);
            dsp_inp_ts_time(&tts);
        }
        break;

    default:
        break;
    }
}

static void kinp_dsp_mtkind(kaction_type kact, uint8_t *tptr)
{
    tou_struct_type ttou;

    switch (kact)
    {
    case KACT_MOVE:
        break;

    case KACT_MENU:
        mt_rtkind = get_dispinp_ratekind() - 1;
        if (rtkind_bakup != mt_rtkind)
            rtkind_chged = true;
        else
            rtkind_chged = false;

        if (rtkind_chged)
        {
            prog_cur_tou_suppdsp_delete();
            prog_fut_delete();

            prog_chg_proc_by_key(mt_dir, rtkind_bakup, meas_method, tptr);

            rtkind_bakup = mt_rtkind;
            if (mt_rtkind == TWO_RATE_KIND)
            {
                tou_conf_init_twokind();
            }

            lcd_dsp_mode = LCD_USER_DSPMODE;

            if (run_is_bat_power())
            {
                whm_op_save();
                mt_conf_save();
                mt_conf_2_save();
            }
        }

        if (mt_rtkind == ONE_RATE_KIND)
        {
            inp_lpintv = dsp_inp_lpintv_init(lp_interval);
        }
        else
        {
            tou_inp_index = 0;
            tou_inp_err = FALSE;
            tou_inp_chged = FALSE;
            inp_tou_get(tou_inp_index, &ttou);
            dsp_inp_tou_rate_init(&ttou);
        }
        break;

    default:
        break;
    }
}

static void kinp_dsp_tou(kaction_type kact, uint8_t *tptr)
{
    tou_struct_type ttou;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move_tou(mt_rtkind);
        break;

    case KACT_MENU:
        inp_touset[tou_inp_index].rate = get_dispinp_ts_tourate();
        inp_touset[tou_inp_index].hour = get_dispinp_tou_hour();
        inp_touset[tou_inp_index].min = get_dispinp_tou_min();

        tou_inp_index++;
        if (tou_inp_index == MAX_TOU_DIV_TWOKIND)
        {
            if (rtkind_chged || is_inp_tou_chged(mt_rtkind))
                tou_inp_chged = 1;
            else
                tou_inp_chged = 0;

            if (tou_inp_chged)
            {
                if (!is_tou_conf_valid())
                {
                    tou_inp_err = TRUE;
                    tou_inp_index = 0;
                    inp_tou_get(tou_inp_index, &ttou);
                    dsp_inp_tou_rate_init(&ttou);
                    dsp_inp_err_set();
                    break;
                }

                inp_mt_rate_conf();

                prog_cur_tou_delete();
                prog_fut_delete();

                if (run_is_main_power())
                {
                    prog_chg_proc_by_key(mt_dir, rtkind_bakup, meas_method,
                                         tptr);

                    if (rtkind_chged)
                    {
                        lp_event_unset(LPE_PROGRAM_CHG);
                    }
                }
                else
                {
                    if (!rtkind_chged)
                    {
                        lp_save_batmode(LPE_PROGRAM_CHG);
                        whm_op_save();
                    }
                }

                if (run_is_bat_power())
                {
                    mt_conf_save();
                    mt_conf_2_save();
                }
            }

            inp_lpintv = dsp_inp_lpintv_init(lp_interval);
        }
        else
        {
            inp_tou_get(tou_inp_index, &ttou);
            dsp_inp_tou_rate_time(&ttou);
        }
        break;

    default:
        break;
    }
}

static void kinp_dsp_lpintv(kaction_type kact, uint8_t *tptr)
{
    switch (kact)
    {
    case KACT_MOVE:
        break;

    case KACT_MENU:
        if (inp_lpintv != get_dmintv_index(lp_interval))
        {
            lp_interval = get_dispinp_lpintv();
            prog_chg_proc_by_key(mt_dir, rtkind_bakup, meas_method, tptr);

            if (run_is_bat_power())
            {
                whm_op_save();
                mt_conf_save();
                mt_conf_2_save();
            }
        }

        dsp_inp_sn1_init();
        break;

    default:
        break;
    }
}

static void kinp_dsp_sn1(kaction_type kact)
{
    switch (kact)
    {
    case KACT_MOVE:
        break;

    case KACT_MENU:
        dsp_inp_sn2_init();
        break;
    default:
        break;
    }
}

static void kinp_dsp_sn2(kaction_type kact)
{
    switch (kact)
    {
    case KACT_MOVE:
        break;
    case KACT_MENU:
        dsp_inp_mtdir_init(mt_dir);
        break;
    default:
        break;
    }
}

static void kinp_dsp_mtdir(kaction_type kact, uint8_t *tptr)
{
    uint8_t bfdir;
    meas_method_type bfmeas;

    switch (kact)
    {
    case KACT_MOVE:
        break;

    case KACT_MENU:
        if (mt_dir != get_dispinp_mtdir())
        {
            bfdir = mt_dir;
            bfmeas = meas_method;

            mt_dir = get_dispinp_mtdir();

            meas_method_adj_dirchg();
            prog_chg_proc_by_key(bfdir, rtkind_bakup, bfmeas, tptr);

            if (run_is_bat_power())
            {
                whm_op_save();
                mt_conf_save();
                mt_conf_2_save();
            }
        }

        dsp_inp_meas_init(meas_method);
        break;

    default:
        break;
    }
}

static void kinp_dsp_meas(kaction_type kact, uint8_t *tptr)
{
    meas_method_type bfmeas;

    switch (kact)
    {
    case KACT_MOVE:
        break;

    case KACT_MENU:
        bfmeas = meas_method;
        meas_method = get_dispinp_meas();
        if (meas_method != bfmeas)
        {
            prog_chg_proc_by_key(mt_dir, rtkind_bakup, bfmeas, tptr);

            if (run_is_bat_power())
            {
                whm_op_save();
                mt_conf_save();
                mt_conf_2_save();
            }
        }
        dsp_inp_baud_init(mdm_baud);
        break;

    default:
        break;
    }
}

static void kinp_dsp_baud(kaction_type kact)
{
    baudrate_type baud;

    switch (kact)
    {
    case KACT_MOVE:
        break;

    case KACT_MENU:
        baud = get_dispinp_baud();
        if (baud != mdm_baud)
        {
            mdm_baud = baud;
            modem_conf(mdm_baud, FALSE);

            if (run_is_main_power())
            {
                lp_event_set(LPE_PROGRAM_CHG);
            }
            else
            {
                lp_save_batmode(LPE_PROGRAM_CHG);
            }
            prog_in_state = E_PROG_KEY;  // key TOU program (key TOU = 32)

            if (run_is_bat_power())
            {
                whm_op_save();
                mt_conf_save();
                mt_conf_2_save();
            }
        }

        dsp_inp_temp_init(temp_thrshld);
        break;

    default:
        break;
    }
}

static void kinp_dsp_temp(kaction_type kact)
{
    int8_t t8;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(4, 5);
        break;

    case KACT_MENU:
        t8 = get_dispinp_temp();
        if (t8 >= 50 && t8 <= 85)
        {
            if (t8 != temp_thrshld)
            {
                temp_thrshld = t8;

                if (run_is_main_power())
                {
                    lp_event_set(LPE_PROGRAM_CHG);
                }
                else
                {
                    lp_save_batmode(LPE_PROGRAM_CHG);
                }

                prog_in_state = E_PROG_KEY;  // key TOU program (key TOU = 32)

                if (run_is_bat_power())
                {
                    whm_op_save();
                    mt_conf_save();
                    mt_conf_2_save();
                }
            }

            err_pulse_backup = err_pulse_react;
            dsp_inp_errpls_init(err_pulse_react);
        }
        else
        {
            dsp_inp_err_set();
        }

    default:
        break;
    }
}

static void kinp_dsp_errpls(kaction_type kact)
{
    ST_MIF_METER_PARM *pst_mif_meter_parm = dsm_mtp_get_meter_parm();

    switch (kact)
    {
    case KACT_MOVE:
        break;

    case KACT_MENU:
        err_pulse_react = get_dispinp_errpls();
        if (err_pulse_react != err_pulse_backup)
        {
            // pulse and eoi_port is selected in pulse_out_quart_dir()

            eoi_or_pulse_select();
            pst_mif_meter_parm->reactive_select = err_pulse_react;
            mif_meter_parm_set();
            if (run_is_main_power())
            {
                lp_event_set(LPE_PROGRAM_CHG);
            }
            else
            {
                lp_save_batmode(LPE_PROGRAM_CHG);
            }

            prog_in_state = E_PROG_KEY;  // key TOU program (key TOU = 32)

            if (run_is_bat_power())
            {
                whm_op_save();
                mt_conf_save();
                mt_conf_2_save();
            }
        }

        dsp_inp_condensor_init(condensor_en);
        break;

    default:
        break;
    }
}

static void kinp_dsp_condensor(kaction_type kact)
{
    uint8_t condens;

    switch (kact)
    {
    case KACT_MOVE:
        break;

    case KACT_MENU:
        condens = get_dispinp_condensor();
        if (condens != condensor_en)
        {
            condensor_en = condens;

            if (run_is_main_power())
            {
                lp_event_set(LPE_PROGRAM_CHG);
            }
            else
            {
                lp_save_batmode(LPE_PROGRAM_CHG);
            }

            prog_in_state = E_PROG_KEY;  // key TOU program (key TOU = 32)

            if (run_is_bat_power())
            {
                whm_op_save();
                mt_conf_save();
                mt_conf_2_save();
            }
        }

        dsp_inp_commen_init(comm_en_coveropen);
        break;

    default:
        break;
    }
}

static void kinp_dsp_commen(kaction_type kact)
{
    uint8_t commen;

    switch (kact)
    {
    case KACT_MOVE:
        break;
    case KACT_MENU:
        commen = get_dispinp_commen();
        if (commen != comm_en_coveropen)
        {
            comm_en_coveropen = commen;

            if (run_is_main_power())
            {
                lp_event_set(LPE_PROGRAM_CHG);
            }
            else
            {
                lp_save_batmode(LPE_PROGRAM_CHG);
            }

            prog_in_state = E_PROG_KEY;  // key TOU program (key TOU = 32)

            if (run_is_bat_power())
            {
                whm_op_save();
                mt_conf_save();
                mt_conf_2_save();
            }
        }

        kact_inp_exit();
        break;

    default:
        break;
    }
}

static void kinp_dsp_sCurr_2(kaction_type kact)
{
    uint16_t t16;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(1, 5);
        break;

    case KACT_MENU:
        t16 = get_dispinp_sCurr();
        if (sCurr_limit_is_valid(t16) && (t16 <= scurr_limit_level))
        {
            if (t16 != scurr_limit_level_2)
            {
                scurr_limit_level_2 = t16;

                if (lpe_proced_in_sigsel == false)
                {
                    lpe_proced_in_sigsel = true;

                    prog_in_state = E_PROG_KEY;

                    if (run_is_main_power())
                    {
                        lp_event_set(LPE_PROGRAM_CHG);
                    }
                    else
                    {
                        lp_save_batmode(LPE_PROGRAM_CHG);
                        whm_op_save();
                    }
                }

                if (run_is_bat_power())
                {
                    mt_conf_save();
                    mt_conf_2_save();
                }
            }

            dsp_inp_scurr_dur2_init(scurr_rtn_dur_2);
        }
        else
        {
            dsp_inp_scurr_2_init(t16);
            dsp_inp_err_set();
        }
        break;

    default:
        break;
    }
}

static void act_dispinp_scurrhold(kaction_type kact)
{
    uint16_t t16;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(2, 5);
        break;

    case KACT_MENU:
        t16 = get_dispinp_scurr_dur();
        if (sCurr_dur_is_valid(t16))
        {
            if (t16 != scurr_det_hold)
            {
                scurr_det_hold = t16;

                if (lpe_proced_in_sigsel == false)
                {
                    lpe_proced_in_sigsel = true;

                    prog_in_state = E_PROG_KEY;

                    if (run_is_main_power())
                    {
                        lp_event_set(LPE_PROGRAM_CHG);
                    }
                    else
                    {
                        lp_save_batmode(LPE_PROGRAM_CHG);
                        whm_op_save();
                    }
                }

                if (run_is_bat_power())
                {
                    mt_conf_save();
                    mt_conf_2_save();
                }
            }

            dsp_inp_scurr_n1_init(scurr_cnt_n1);
        }
        else
        {
            dsp_inp_scurr_hold_init(t16);
            dsp_inp_err_set();
        }
        break;

    default:
        break;
    }
}

static void act_dispinp_scurr_n1(kaction_type kact)
{
    uint8_t t8;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(4, 5);
        break;

    case KACT_MENU:
        t8 = get_dispinp_scurr_n1();
        if (sCurr_n1_is_valid(t8))
        {
            if (t8 != scurr_cnt_n1)
            {
                scurr_cnt_n1 = t8;

                if (lpe_proced_in_sigsel == false)
                {
                    lpe_proced_in_sigsel = true;

                    prog_in_state = E_PROG_KEY;

                    if (run_is_main_power())
                    {
                        lp_event_set(LPE_PROGRAM_CHG);
                    }
                    else
                    {
                        lp_save_batmode(LPE_PROGRAM_CHG);
                        whm_op_save();
                    }
                }

                if (run_is_bat_power())
                {
                    mt_conf_save();
                    mt_conf_2_save();
                }
            }

            dsp_inp_scurr_init(scurr_limit_level);
        }
        else
        {
            dsp_inp_scurr_n1_init(t8);
            dsp_inp_err_set();
        }
        break;

    default:
        break;
    }
}

static void act_dispinp_scurr_dur1(kaction_type kact)
{
    uint16_t t16;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(2, 5);
        break;

    case KACT_MENU:
        t16 = get_dispinp_scurr_dur();
        if (sCurr_dur_is_valid(t16))
        {
            if (t16 != scurr_rtn_dur_1)
            {
                scurr_rtn_dur_1 = t16;

                if (lpe_proced_in_sigsel == false)
                {
                    lpe_proced_in_sigsel = true;

                    prog_in_state = E_PROG_KEY;

                    if (run_is_main_power())
                    {
                        lp_event_set(LPE_PROGRAM_CHG);
                    }
                    else
                    {
                        lp_save_batmode(LPE_PROGRAM_CHG);
                        whm_op_save();
                    }
                }

                if (run_is_bat_power())
                {
                    mt_conf_save();
                    mt_conf_2_save();
                }
            }

            if (scurr_cnt_n1 == 0 || scurr_cnt_n1 == scurr_autortn_cnt)
            {
                dsp_inp_contract_month_init(contract_dm_month);
            }
            else
            {
                dsp_inp_scurr_2_init(scurr_limit_level_2);
            }
        }
        else
        {
            dsp_inp_scurr_dur1_init(t16);
            dsp_inp_err_set();
        }
        break;

    default:
        break;
    }
}

static void act_dispinp_scurr_dur2(kaction_type kact)
{
    uint16_t t16;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(2, 5);
        break;

    case KACT_MENU:
        t16 = get_dispinp_scurr_dur();
        if (sCurr_dur_is_valid(t16))
        {
            if (t16 != scurr_rtn_dur_2)
            {
                scurr_rtn_dur_2 = t16;

                if (lpe_proced_in_sigsel == false)
                {
                    lpe_proced_in_sigsel = true;

                    prog_in_state = E_PROG_KEY;

                    if (run_is_main_power())
                    {
                        lp_event_set(LPE_PROGRAM_CHG);
                    }
                    else
                    {
                        lp_save_batmode(LPE_PROGRAM_CHG);
                        whm_op_save();
                    }
                }

                if (run_is_bat_power())
                {
                    mt_conf_save();
                    mt_conf_2_save();
                }
            }

            scurr_block_dur_kinp = false;

            dsp_inp_contract_month_init(contract_dm_month);
        }
        else
        {
            dsp_inp_scurr_dur2_init(t16);
            dsp_inp_err_set();
        }
        break;

    default:
        break;
    }
}

static void act_dispinp_contract_month(kaction_type kact)
{
    uint32_t t32;

    switch (kact)
    {
    case KACT_MOVE:
        dsp_inp_digit_pos_move(0, 5);
        break;

    case KACT_MENU:
        t32 = get_dispinp_contract_month();
        if (t32 != (contract_dm_month / 1000))
        {  // kw unit
            contract_dm_month = t32 * 1000;

            prog_in_state = E_PROG_KEY;

            if (run_is_main_power())
            {
                lp_event_set(LPE_PROGRAM_CHG);
            }
            else
            {
                lp_save_batmode(LPE_PROGRAM_CHG);
            }

            if (run_is_bat_power())
            {
                whm_op_save();
                mt_conf_save();
                mt_conf_2_save();
            }
        }

        rtkind_bakup = mt_rtkind;
        dsp_inp_ratekind_init(mt_rtkind);
        break;

    default:
        break;
    }
}
#endif

bool sCurr_n1_is_valid(uint8_t _n1)
{
    if (_n1 == 0 || scurr_autortn_cnt == 0)
        return true;

    if (_n1 <= scurr_autortn_cnt)
        return true;

    return false;
}

bool sCurr_limit_is_valid(uint16_t _val)
{
    if (_val >= 110 && _val <= 11000)
        return true;

    return false;
}

bool sCurr_dur_is_valid(uint16_t _dur)
{
    if (_dur >= 1 && _dur <= 9000)
        return true;

    return false;
}

static bool hh_mm_is_valid(uint8_t hh, uint8_t mn)
{
    if (hh >= 24 || mn >= 60)
        return false;

    return true;
}
