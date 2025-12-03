#include "options.h"
#include "nv.h"
#include "comm.h"  // crc16_update()
#include "program.h"
#include "whm.h"
#include "amg_wdt.h"

#include "get_req.h"
#define _D "[PROG] "

#define PROG_STS_TOU_ERR 0x01
#define PROG_STS_NPHOL_ARR_ERR 0x02
#define PROG_STS_NPHOL_NV_ERR 0x04
#define PROG_STS_SEASON_NV_ERR 0x08
#define PROG_STS_WEEK_NV_ERR 0x10

extern date_time_type sr_dt_nprd;
extern bool b_season_changed_pwtr_back, b_season_changed_rtc_back,
    rtc_season_chg_chk;

static uint8_t program_status;
uint8_t program_event;

static uint8_t npHol_info_year;
static uint8_t npHol_info_block_idx;
static date_time_type toupdate_time;

bool b_season_changed, b_prog_changed_event;
static bool season_info_available;
static uint8_t curr_season_id;
static season_date_type season_prof;

static uint8_t curr_week_id;
static week_struct_type week_info;

static uint8_t curr_day_id;

static uint16_t season_recovery_cnt;
static uint16_t week_recovery_cnt;
uint8_t aprog_area_rcnt;  // recent

bool prog_dlcmd_avail;
tou_set_cnt_type tou_set_cnt;

holiday_date_type g_pholiday;
holiday_date_type g_npholiday;

ST_npBILL_BACKUP gst_npBillback;

static void prog_fut_work_proc(uint8_t *tptr);
static void prog_nphol_info_update(uint8_t *tptr);
static ratekind_type conv_rate_to_ratekind(rate_type rt);
static void prog_tou_update(uint8_t *tptr, bool curr_prog_in, bool season_chk);
static uint8_t get_nphol_block_idx(uint8_t yr, uint8_t *tptr);
static void prog_cur_work_proc(prog_dl_type *progdl, uint8_t *tptr,
                               bool fut_ok);
static void pgm_daylight_saving(prog_dl_type *progdl);
static void pgm_out_sig_sel(prog_dl_type *progdl);
static void pgm_pbilling(prog_dl_type *progdl);
static void pgm_npbilling(prog_dl_type *progdl);
static void pgm_suppdsp(prog_dl_type *progdl, uint8_t *tptr);
static void pgm_billing_parm(prog_dl_type *progdl);
static void pgm_dm_intv(prog_dl_type *progdl);
static void pgm_lp_intv(prog_dl_type *progdl);
static void pgm_lpavg_intv(prog_dl_type *progdl);
static void pgm_scurr_limit(prog_dl_type *progdl);
static void pgm_lcd_set_parm(prog_dl_type *progdl);
static void pgm_pwd_chg(prog_dl_type *progdl);
static void pgm_ts_conf(prog_dl_type *progdl);
static void pgm_scurr_hold(prog_dl_type *progdl);
static void pgm_scurr_rtn_1(prog_dl_type *progdl);
static void pgm_scurr_rtn_2(prog_dl_type *progdl);
static void pgm_scurr_cnt_n1(prog_dl_type *progdl);
static void pgm_holiday_sel1(prog_dl_type *progdl, uint8_t *tptr,
                             bool season_chk);

static ratekind_type prog_get_day_profile_ratekind(uint8_t *tptr);
static rate_type get_curr_rate(void);
uint16_t prog_setbits_to_availbits(uint32_t setbits, uint16_t availbits);

void mif_meter_parm_set(void);

void program_init(void)
{
    b_season_changed = false;
    season_info_available = false;
    curr_week_id = 0xff;
    curr_day_id = 0xff;

    program_status = 0;
    program_event = 0;

    npHol_info_year = 0xff;
    toupdate_time.year = 0xff;

    season_recovery_cnt = 0;
    week_recovery_cnt = 0;
}

void program_default_init(void)
{
    b_season_changed = true;
    season_info_available = true;
    curr_week_id = 0xff;
    curr_day_id = 0xff;

    program_status = 0;
    program_event = 0;

    npHol_info_year = 0xff;
    toupdate_time.year = 0xff;

    season_recovery_cnt = 0;
    week_recovery_cnt = 0;
}

void prog_cur_tou_suppdsp_delete(void)
{
    currprog_available_bits &=
        ~(PROG_HOLIDAY_BIT | PROG_SEASON_PROFILE_BIT | PROG_WEEK_PROFILE_BIT |
          PROG_DAY_PROFILE_BIT | PROG_SUPP_DISP_BIT);
}

void prog_cur_tou_delete(void)
{
    currprog_available_bits &= ~(PROG_HOLIDAY_BIT | PROG_SEASON_PROFILE_BIT |
                                 PROG_WEEK_PROFILE_BIT | PROG_DAY_PROFILE_BIT);
}

void program_proc(uint8_t *tptr)
{
    // tptr is temp buffer

    prog_fut_mon(tptr);

    prog_nphol_info_update(tptr);  // non periodic holiday

    prog_tou_update(tptr, 0, true);

    curr_rate_update();
}

void tou_update_by_rtchg(uint8_t *tptr)
{
    prog_nphol_info_update(tptr);

    prog_tou_update(tptr, 0, true);

    curr_rate_update();
}

void prog_tou_start(uint8_t *tptr, bool curr_prog_in)
{
    DPRINTF(DBG_TRACE, _D "%s", __func__);
    program_init();

    prog_nphol_info_update(tptr);

    prog_tou_update(tptr, curr_prog_in, true);

    curr_rate_update();
}

void prog_info_reset(uint8_t *tptr)
{
    program_info_type *proginfo;

    proginfo = (program_info_type *)tptr;

    memset((uint8_t *)proginfo, 0x00, sizeof(program_info_type));

    nv_sub_info.ch[0] = 0;
    nv_write(I_PROG_INFO, (uint8_t *)proginfo);
    nv_sub_info.ch[0] = 1;
    nv_write(I_PROG_INFO, (uint8_t *)proginfo);
}

void prog_info_write(bool curprog, uint8_t progcnt, prog_dl_type *progdl,
                     date_time_type *pdt, uint8_t *tptr)
{
    program_info_type *proginfo;

    proginfo = (program_info_type *)tptr;

    nv_sub_info.ch[0] = (uint8_t)(curprog ? 0 : 1);
    if (!nv_read(I_PROG_INFO, (uint8_t *)proginfo))
    {
        memset((uint8_t *)proginfo, 0x00, sizeof(program_info_type));
    }

    // 1) count
    proginfo->cnt += progcnt;
    // 2) date and time
    proginfo->dt = *pdt;
    // 3) name
    if (progdl->set_bits & SETBITS_PGM_NAME)
    {
        memcpy(proginfo->name, progdl->pgm_name, PROG_ID_SIZE);
    }
    else
    {
        memset(proginfo->name, ' ', PROG_ID_SIZE);
    }
    // 4) save
    nv_sub_info.ch[0] = curprog ? 0 : 1;
    nv_write(I_PROG_INFO, (uint8_t *)proginfo);
}

void prog_info_write_by_key(bool curprog, U8 progcnt, date_time_type *pdt,
                            U8 *tptr)
{
    program_info_type *proginfo;

    proginfo = (program_info_type *)tptr;

    nv_sub_info.ch[0] = (U8)(curprog ? 0 : 1);
    if (!nv_read(I_PROG_INFO, (U8 *)proginfo))
    {
        memset((U8 *)proginfo, 0x00, sizeof(program_info_type));
    }

    // 1) count
    proginfo->cnt += progcnt;
    // 2) date and time
    proginfo->dt = *pdt;
// 3) name
#if 0
	if(progdl->set_bits & SETBITS_PGM_NAME) {
		memcpy(proginfo->name, progdl->pgm_name, PROG_ID_SIZE);
	} else {
		memset(proginfo->name, ' ', PROG_ID_SIZE);
	}
#endif
    // 4) save
    nv_sub_info.ch[0] = curprog ? 0 : 1;
    nv_write(I_PROG_INFO, (U8 *)proginfo);
}

void prog_cur_dl_proc(prog_dl_type *progdl, uint8_t *tptr)
{
    DPRINTF(DBG_ERR, "%s\r\n", __func__);

    aprog_area_rcnt = aprog_area;
    // program eeprom area
    aprog_area = get_prog_dl_idx();

    nv_write(I_CUR_PROG_DL, (uint8_t *)progdl);

    // program data activation
    prog_cur_work_proc(progdl, tptr, false);
}

bool prog_fut_dl_proc(prog_dl_type *progdl, uint8_t *tptr)
{
    date_time_type dt;

    DPRINTF(DBG_ERR, "%s\r\n", __func__);

    // future program activation
    if (progdl->set_bits & SETBITS_PAS_TIME)
    {
        if (sel_react_mon != 0)
        {
            dt.year = sel_react_yr;
            dt.month = sel_react_mon;
            dt.date = 31;
            if (cmp_date(&progdl->active_passive_time, &dt) <= 0)
            {
                return false;
            }
        }

        fut_prog_work_time = progdl->active_passive_time;
    }

    // program eeprom area
    pprog_area = get_prog_dl_idx();

    // program data
    nv_write(I_FUT_PROG_DL, (uint8_t *)progdl);

    // program
    prog_info_write(false, 1, progdl, &cur_rtc, tptr);

    if (progdl->set_bits & SETBITS_HOLIDAYS)
    {
        phol_area = get_hol_dl_idx();
    }
    else
    {
        if (prog_curr_is_available() && prog_hol_available())
        {
            progdl->set_bits |= SETBITS_HOLIDAYS;
            phol_area = ahol_area;
        }
    }
    futprog_available_bits = prog_setbits_to_availbits(progdl->set_bits, 0);
    // program available
    prog_fut_add();

    return true;
}

bool prog_get_fut_dl(prog_dl_type *pdl)
{
    return nv_read(I_FUT_PROG_DL, (uint8_t *)pdl);
}

void prog_tou_refresh(U8 *tptr, bool season_chk)
{
    toupdate_time.year = 0xff;
    prog_tou_update(tptr, 0, season_chk);
    curr_rate_update();
}

bool prog_season_profile_proc(uint8_t *tptr, bool curr_prog_in, bool season_chk)
{
    uint8_t i;
    uint8_t eob_type = 0;

    if (season_chk)
    {
        b_season_changed = false;

        if ((program_status & PROG_STS_SEASON_NV_ERR) == 0)
        {
        prog_season_profile_proc1:
            if (prog_curr_is_available() && prog_tou_available())
            {
                if (!season_info_available)
                {
                    if (nv_read(I_SEASON_PROFILE_ACTIVE,
                                (uint8_t *)&season_prof))
                    {
                        goto prog_season_profile_proc2;
                    }
                    else
                    {
                        program_status |= PROG_STS_SEASON_NV_ERR;
                    }
                }
                else
                {
                prog_season_profile_proc2:
                    season_info_available = true;

                    for (i = 0; i < (season_prof.cnt - 1); i++)
                    {
                        if (cmp_mmdd(
                                (mm_dd_type *)&cur_rtc.month,
                                (mm_dd_type *)&season_prof.season[i].month) >=
                                0 &&
                            cmp_mmdd((mm_dd_type *)&cur_rtc.month,
                                     (mm_dd_type *)&season_prof.season[i + 1]
                                         .month) < 0)
                            break;
                    }

                    curr_season_id = i;
                    DPRINTF(DBG_WARN,
                            "%s:   b_season_changed[%d]  rcnt_season_id[%d]  "
                            "curr_season_id[%d]\r\n",
                            __func__, b_season_changed, rcnt_season_id,
                            curr_season_id);
                    if (rcnt_season_id != curr_season_id)
                    {
                        b_season_changed = true;
                        DPRINTF(DBG_ERR,
                                "%s:  if(rcnt_season_id != curr_season_id)  "
                                "b_season_changed[%d]  rcnt_season_id[%d]  "
                                "curr_season_id[%d]\r\n",
                                __func__, b_season_changed, rcnt_season_id,
                                curr_season_id);

#if defined(FEATURE_JP_seasonCHG_sr_dr_type_to_SET_SR)
                        if ((seasonCHG_sr_dr_type) &&
                            (curr_prog_in))  // 시간변경 이면 즉시 안하고
                                             // 모아서 처리함  .
                        {
                            prog_season_changed_clr();
                            {
                                eob_type |= EOB_SEASON_FLAG;

                                sr_dr_proc(eob_type, seasonCHG_sr_dr_type,
                                           &cur_rtc, tptr);
                            }
                        }
#endif
                    }

                    rcnt_season_id = curr_season_id;
                }
            }
            else
            {
                season_info_available = false;
            }
        }
        else
        {
            season_info_available = false;

            if ((++season_recovery_cnt) >= 36000)
            {  // 10 hours
                season_recovery_cnt = 0;
                goto prog_season_profile_proc1;
            }
        }
    }
    return b_season_changed;
}

void curr_rate_update(void)
{
    cur_rate_before = cur_rate;
    cur_rate = get_curr_rate();

    if (cur_rate_before != cur_rate)
    {
        DPRINTF(DBG_TRACE, _D "%s: Rate Update %d -> %d\r\n", __func__,
                cur_rate_before, cur_rate);
    }
    if (eoi_processed)
    {
        if (eoi_rate != cur_rate)
        {
            eoi_rate = cur_rate;
            dm_intv_init();
        }
    }
}

meas_method_type parse_meas_method(uint8_t lcdparm)
{
    uint8_t t8;
    meas_method_type meas;

    t8 = lcdparm & 0x07;
#if 1 /* bccho, 2024-09-05, 삼상 */
    if (t8 == 0x01 || t8 == 0x02 || t8 == 0x04)
    {  // just one bit is allowed
        if (t8 == 0x01)
        {
            meas = E_BASIC;
        }
#if PHASE_NUM != SINGLE_PHASE
        else
        {
            meas = E_VECTSUM;
        }
#else
        else
        {
            meas = E_SINGLE_DIR;
        }
#endif
    }
    else
    {
#if PHASE_NUM != SINGLE_PHASE
        meas = E_VECTSUM;
#endif
        meas = E_BASIC;
    }
#else
    if (t8 == 0x01 || t8 == 0x04)
    {  // just one bit is allowed
        if (t8 == 0x01)
        {
            meas = E_BASIC;
        }
        else
        {
            meas = E_SINGLE_DIR;
        }
    }
    else
    {
        meas = E_BASIC;
    }
#endif
    return meas;
}

void edit_holidays(bool insert, bool fut, uint16_t idx, uint16_t yr,
                   holiday_struct_type *hol, uint8_t *tptr)
{
    uint8_t blk, offs;
    holiday_date_type *hol_date;
    nv_item_type nv_i;

    if (fut == false)
    {
        // 현재
        if (!prog_hol_available())
        {
            DPRINTF(DBG_TRACE, "A_SDT is not available\r\n");
            return;
        }
        nv_i = I_HOLIDAYS_A;
    }
    else
    {
        // 예약
        if (!futprog_hol_available())
        {
            DPRINTF(DBG_TRACE, "P_SDT is not available\r\n");
            return;
        }
        nv_i = I_HOLIDAYS_P;
    }

    blk = idx / HOLIDAYS_PER_BLOCK;
    offs = idx % HOLIDAYS_PER_BLOCK;
    if (blk > HOLIDAY_BLOCK_LEN)
    {
        return;
    }

    nv_sub_info.ch[0] = blk;
    nv_sub_info.ch[1] = HOL_OF_BLOCK;
    if (nv_read(nv_i, tptr))
    {
        hol_date = (holiday_date_type *)tptr;

        if (insert)
        {
            // 추가
            if (idx >= HOLIDAYS_PER_BLOCK)
            {
                // 비정기 휴일
                if (yr != hol_date->yr)
                {
                    return;
                }
            }
            hol_date->holiday[offs].month = hol->month;
            hol_date->holiday[offs].date = hol->date;
            hol_date->holiday[offs].day_id = hol->day_id;
        }
        else
        {
            // 삭제
            hol_date->holiday[offs].month = 0xff;
            hol_date->holiday[offs].date = 0xff;
        }

        nv_write(nv_i, tptr);

        if (nv_i == I_HOLIDAYS_A)
            lp_event_set(LPE_PROGRAM_CHG);
    }
}

void prog_fut_delete(void)
{
    prog_available &= ~PROG_FUT_BIT;
    futprog_available_bits = 0;
}

void prog_fut_mon(uint8_t *tptr)
{
    if (prog_fut_is_available())
    {
        int8_t t = cmp_date_time(&fut_prog_work_time, &cur_rtc);
        if (t <= 0)
        {
            MSG07(">>prog_fut_mon()_change, %d", t);
            MSG07("R: %d-%d-%d %d:%d:%d", fut_prog_work_time.year,
                  fut_prog_work_time.month, fut_prog_work_time.date,
                  fut_prog_work_time.hour, fut_prog_work_time.min,
                  fut_prog_work_time.sec);
            MSG07("C: %d-%d-%d %d:%d:%d", cur_rtc.year, cur_rtc.month,
                  cur_rtc.date, cur_rtc.hour, cur_rtc.min, cur_rtc.sec);

            dsm_wdt_ext_toggle_immd();
            // future program work
            prog_fut_work_proc(tptr);
            // program module event set
            prog_evt_set_fut_work();
            // future program delete
            prog_fut_delete();

            dsm_progname_update_forReport();
        }
    }
}

static void prog_cur_work_proc(prog_dl_type *progdl, uint8_t *tptr, bool fut_ok)
{
    prog_in_state = E_PROG_COMM;

    // program change process -> inf write, sr/dr, log
    prog_chg_proc_by_comm(progdl, tptr, !fut_ok);

    // daylight saving
    pgm_daylight_saving(progdl);

    // out signal selection
    pgm_out_sig_sel(progdl);

    // period billing
    pgm_pbilling(progdl);

    // non-period billing
    pgm_npbilling(progdl);

    // supply mode display
    pgm_suppdsp(progdl, tptr);

    // billing parameter processing
    pgm_billing_parm(progdl);

    // demand interval processing
    pgm_dm_intv(progdl);

    // LP interval processing
    pgm_lp_intv(progdl);

    // LPavg interval processing
    pgm_lpavg_intv(progdl);

    // small current limit value
    pgm_scurr_limit(progdl);

    // process lcd mode parameter
    pgm_lcd_set_parm(progdl);

    // password change
    pgm_pwd_chg(progdl);

    // TS configure
    pgm_ts_conf(progdl);

    pgm_scurr_hold(progdl);

    pgm_scurr_rtn_1(progdl);

    pgm_scurr_rtn_2(progdl);

    pgm_scurr_cnt_n1(progdl);

    DPRINTF(DBG_ERR,
            "%s -1  prog_available[%d] aprog_area[%d]  pprog_area[%d] \r\n",
            __func__, prog_available, aprog_area, pprog_area);
    DPRINTF(DBG_ERR, "  aprog_area_rcnt[%d] ahol_area[%d] phol_area[%d] \r\n",
            aprog_area_rcnt, ahol_area, phol_area);

    DPRINTF(DBG_ERR,
            "  currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
            "progdl->set_bits_1[0x%x]\r\n",
            currprog_available_bits, progdl->set_bits, progdl->set_bits_1);

    if (progdl->set_bits & SETBITS_HOLIDAYS)
    {
        if (!fut_ok)
            ahol_area = get_hol_dl_idx();
        else
            ahol_area = phol_area;
    }
    else
    {
        if (prog_curr_is_available() && prog_hol_available())
        {
            progdl->set_bits |= SETBITS_HOLIDAYS;
            // ahol_area is no change
        }
    }

    DPRINTF(DBG_ERR,
            "%s -2 prog_available[%d] aprog_area[%d]  pprog_area[%d] \r\n",
            __func__, prog_available, aprog_area, pprog_area);
    DPRINTF(DBG_ERR, "  aprog_area_rcnt[%d] ahol_area[%d] phol_area[%d] \r\n",
            aprog_area_rcnt, ahol_area, phol_area);
    DPRINTF(DBG_ERR,
            "  currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
            "progdl->set_bits_1[0x%x]\r\n",
            currprog_available_bits, progdl->set_bits, progdl->set_bits_1);

// TOU 정기/비정기 휴일 적용
#if 0
	if (progdl->set_bits_1 & SETBITS_HOLIDAY_SEL_1)
	{
		DPRINTF(DBG_TRACE, "%s: ======== Holiday Apply ========\r\n", __func__);
		holiday_sel = 1;
		//progdl->set_bits_1 &= ~SETBITS_HOLIDAY_SEL_1;
	}
	else
	{
		DPRINTF(DBG_TRACE, "%s: ======== Holiday Not Apply ========\r\n", __func__);
		holiday_sel = 0;
	}
#else
    pgm_holiday_sel1(progdl, tptr, false);

    // DPRINTF (DBG_ERR, "%s:	-2-2  currprog_available_bits[0x%x]
    // futprog_available_bits[0x%x] \r\n", __func__,	currprog_available_bits,
    // futprog_available_bits);
#endif

    DPRINTF(DBG_ERR,
            "%s -3  prog_available[%d] aprog_area[%d]	pprog_area[%d] \r\n",
            __func__, prog_available, aprog_area, pprog_area);
    DPRINTF(DBG_ERR, "	aprog_area_rcnt[%d] ahol_area[%d] phol_area[%d] \r\n",
            aprog_area_rcnt, ahol_area, phol_area);

    DPRINTF(DBG_ERR,
            "  currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
            "progdl->set_bits_1[0x%x]\r\n",
            currprog_available_bits, progdl->set_bits, progdl->set_bits_1);

    currprog_available_bits = prog_setbits_to_availbits(progdl->set_bits, 0);

    // program available
    prog_cur_add();

    DPRINTF(DBG_ERR,
            "%s -4  prog_available[%d] aprog_area[%d]  pprog_area[%d] \r\n",
            __func__, prog_available, aprog_area, pprog_area);
    DPRINTF(DBG_ERR, "  aprog_area_rcnt[%d] ahol_area[%d] phol_area[%d] \r\n",
            aprog_area_rcnt, ahol_area, phol_area);

    DPRINTF(DBG_ERR,
            "  currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
            "progdl->set_bits_1[0x%x]\r\n",
            currprog_available_bits, progdl->set_bits, progdl->set_bits_1);

    // --------- post process  -------------

    npBill_date_load();

    if (fut_ok)
        npBill_date_load_forFut_notAvailxx(progdl);

    prog_tou_start(tptr, !fut_ok);  // tou data update

// meter rate kind
#ifdef TOU_INDIVIDUAL_RESERVATION_NAME_FIX /* bccho, 2023-11-14, undefine */
    if (progdl->set_bits & SETBITS_PGM_NAME)
    {
        // rate, meter reading date, direction, measure,... Ref.
        // fill_prog_name() and ob_prog_id()
        program_info_type prog_info = {0};
        if (nv_read(I_PROG_INFO, (uint8_t *)&prog_info))
        {
            uint8_t val = 0;

            /* 종별 */
            val = prog_info.name[0] - '1';
            if (val >= 0 && val < 4)  // 1 ~ 4 종
            {
                mt_rtkind = val;
            }

            /* 검침일 */
            val = (prog_info.name[1] - '0') * 10;
            val += (prog_info.name[2] - '0');
            if (val > 0 && val <= 31)  // 1 ~ 31 일
            {
                reg_mr_date = val;
            }

            /* 선택 유효전력량 */
            if (prog_info.name[3] == 'R' && prog_info.name[4] == 'R')  // 수전
            {
                mt_dir = MT_UNI_DIR;
            }
            else if (prog_info.name[3] == 'S' &&
                     prog_info.name[4] == 'R')  // 송/수전
            {
                mt_dir = MT_BOTH_DIR;
            }

            /* 계량 모드 */
            switch (prog_info.name[5])
            {
            case 'D':
#if PHASE_NUM == SINGLE_PHASE
                // 수전 단방향 : 단상
                if (meas_method != E_SINGLE_DIR)
                {
                    meas_method = E_SINGLE_DIR;
                }
                break;
#endif

            case 'S':
                // 수전 및 송/수전 모드 : 단상
                if (meas_method != E_BASIC)
                {
                    meas_method = E_BASIC;
                }
                break;

            case 'U':
                /* bccho, 2024-09-05, 삼상 */
                // 수전 및 송/수전 합성 모드 : 3상
                if (meas_method != E_VECTSUM)
                {
                    meas_method = E_VECTSUM;
                }
                break;

            default:
                break;
            }
        }
    }

    else if ((progdl->set_bits & SETBITS_TOU_DAY) ||
             (progdl->set_bits & SETBITS_TOU_WEEK))
    {
        mt_rtkind = prog_get_day_profile_ratekind(tptr);
    }
    DPRINTF(DBG_TRACE, "%s: ======== RATE %d ========\r\n", __func__,
            mt_rtkind + 1);
#else
    if ((progdl->set_bits & SETBITS_TOU_DAY) ||
        (progdl->set_bits & SETBITS_TOU_WEEK))
    {
#if 1  // 24.10.08
        uint8_t mt_kind;
        mt_kind = prog_get_day_profile_ratekind(tptr);
        if (mt_rtkind != mt_kind)
            tou_id_change_sts = 1;
        mt_rtkind = mt_kind;
#else
        mt_rtkind = prog_get_day_profile_ratekind(tptr);
#endif
    }
#endif

    if (eoi_evt & (EOI_EVENT_INTV_CHG | EOI_EVENT_SUBINTV_CHG))
    {
        if (eoi_processed)
        {
            eoi_parm_set(eoi_evt);
            eoi_evt = 0;
        }
    }

    dsm_progname_update_forReport();
}

static void season_copy_curr_fut(void)
{
    if (prog_season_available())
    {
        if (nv_read(I_SEASON_PROFILE_ACTIVE, (uint8_t *)&season_prof))
        {
            nv_write(I_SEASON_PROFILE_DL, (uint8_t *)&season_prof);
            pdl_set_bits |= SETBITS_TOU_SEASON;
        }
    }
}

static void week_copy_curr_fut(void)
{
    week_date_type week_info;
    if (prog_week_available())
    {
        if (nv_read(I_WEEK_PROFILE_ACTIVE, (uint8_t *)&week_info))
        {
            nv_write(I_WEEK_PROFILE_DL, (uint8_t *)&week_info);
            pdl_set_bits |= SETBITS_TOU_WEEK;
        }
    }
}

static void day_copy_curr_fut(void)
{
    uint8_t i;
    dayid_table_type daytable;
    if (prog_day_available())
    {
        for (i = 0; i < DAY_PROF_SIZE; i++)
        {
            nv_sub_info.ch[0] = i;
            if (nv_read(I_DAY_PROFILE_ACTIVE, (uint8_t *)&daytable))
            {
                nv_write(I_DAY_PROFILE_DL, (uint8_t *)&daytable);
            }
        }
        pdl_set_bits |= SETBITS_TOU_DAY;
    }
}

void prog_cur_tou_fut_proc(void)
{
    if (prog_curr_is_available())
    {
        season_copy_curr_fut();
        week_copy_curr_fut();
        day_copy_curr_fut();
    }
}

void prog_curr_npbill_date_backup(void)
{
    memset(&gst_npBillback, 0x00, sizeof(ST_npBILL_BACKUP));

    if (prog_npbill_available())
    {
        gst_npBillback.curprog_available = true;

        nv_read(I_NP_BILLDATE_A, (uint8_t *)&gst_npBillback.date);
        DPRINTF(DBG_WARN, "%s: gst_npBillback.cnt[%d]\r\n", __func__,
                gst_npBillback.date.cnt);
    }
    else
    {
        gst_npBillback.curprog_available = false;
    }
}

ST_npBILL_BACKUP *prog_get_curr_npbill_date_backup(void)
{
    return &gst_npBillback;
}

static void prog_fut_work_proc(uint8_t *tptr)
{
    prog_dl_type *progdl;
    progdl = (prog_dl_type *)tptr;
    tptr += sizeof(prog_dl_type);

    DPRINTF(DBG_ERR,
            "%s -1  prog_available[%d] aprog_area[%d]  pprog_area[%d] \r\n",
            __func__, prog_available, aprog_area, pprog_area);
    DPRINTF(DBG_ERR, "  aprog_area_rcnt[%d] ahol_area[%d] phol_area[%d] \r\n",
            aprog_area_rcnt, ahol_area, phol_area);
    DPRINTF(DBG_ERR,
            "  currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
            "progdl->set_bits_1[0x%x]\r\n",
            currprog_available_bits, progdl->set_bits, progdl->set_bits_1);

    if (!prog_get_fut_dl(progdl))
        return;

    aprog_area_rcnt = aprog_area;
    aprog_area = pprog_area;

    DPRINTF(DBG_ERR,
            "%s -2  prog_available[%d] aprog_area[%d]  pprog_area[%d] \r\n",
            __func__, prog_available, aprog_area, pprog_area);
    DPRINTF(DBG_ERR, "  aprog_area_rcnt[%d] ahol_area[%d] phol_area[%d] \r\n",
            aprog_area_rcnt, ahol_area, phol_area);

    DPRINTF(DBG_ERR,
            "  currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
            "progdl->set_bits_1[0x%x]\r\n",
            currprog_available_bits, progdl->set_bits, progdl->set_bits_1);

    // program data
    nv_write(I_CUR_PROG_DL, (uint8_t *)progdl);

    DPRINTF(DBG_ERR,
            "%s -3  prog_available[%d] aprog_area[%d]  pprog_area[%d] \r\n",
            __func__, prog_available, aprog_area, pprog_area);
    DPRINTF(DBG_ERR, "  aprog_area_rcnt[%d] ahol_area[%d] phol_area[%d] \r\n",
            aprog_area_rcnt, ahol_area, phol_area);
    DPRINTF(DBG_ERR,
            "  currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
            "progdl->set_bits_1[0x%x]\r\n",
            currprog_available_bits, progdl->set_bits, progdl->set_bits_1);

    // program data activation
    prog_cur_work_proc(progdl, tptr, true);

    DPRINTF(DBG_ERR,
            "%s -4  prog_available[%d] aprog_area[%d]	pprog_area[%d] \r\n",
            __func__, prog_available, aprog_area, pprog_area);
    DPRINTF(DBG_ERR, "	aprog_area_rcnt[%d] ahol_area[%d] phol_area[%d] \r\n",
            aprog_area_rcnt, ahol_area, phol_area);
    DPRINTF(DBG_ERR,
            "  currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
            "progdl->set_bits_1[0x%x]\r\n",
            currprog_available_bits, progdl->set_bits, progdl->set_bits_1);

    currprog_available_bits = prog_setbits_to_availbits(progdl->set_bits, 0);

    DPRINTF(DBG_ERR,
            "%s -5  prog_available[%d] aprog_area[%d]  pprog_area[%d] \r\n",
            __func__, prog_available, aprog_area, pprog_area);
    DPRINTF(DBG_ERR, "  aprog_area_rcnt[%d] ahol_area[%d] phol_area[%d] \r\n",
            aprog_area_rcnt, ahol_area, phol_area);
    DPRINTF(DBG_ERR,
            "  currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
            "progdl->set_bits_1[0x%x]\r\n",
            currprog_available_bits, progdl->set_bits, progdl->set_bits_1);

    // program available
    prog_cur_add();
}

uint16_t prog_setbits_to_availbits(uint32_t setbits, uint16_t availbits)
{
    if (setbits & SETBITS_HOLIDAYS)
        availbits |= PROG_HOLIDAY_BIT;

    if (setbits & SETBITS_TOU_SEASON)
        availbits |= PROG_SEASON_PROFILE_BIT;

    if (setbits & SETBITS_TOU_WEEK)
        availbits |= PROG_WEEK_PROFILE_BIT;

    if (setbits & SETBITS_TOU_DAY)
        availbits |= PROG_DAY_PROFILE_BIT;

    if (setbits & SETBITS_PBILL_DATE)
        availbits |= PROG_pBILL_DATE_BIT;

    if (setbits & SETBITS_NPBILL_DATE)
        availbits |= PROG_npBILL_DATE_BIT;

    if (setbits & SETBITS_SUPPDSP_ITEM)
        availbits |= PROG_SUPP_DISP_BIT;

    if (setbits & SETBITS_LCDSET_PARM)
        availbits |= PROG_LCDPARM_BIT;

    if (setbits & SETBITS_BILLING_PARM)
        availbits |= PROG_BILLPARM_BIT;

    if (setbits & SETBITS_TMP_HOLIDAYS)
        availbits |= PROG_TMP_HOLIDAY_BIT;

    return availbits;
}

static bool prog_nphol_is_available(void)
{
    return (npHol_info_block_idx != 0xff);
}

static void prog_nphol_info_update(uint8_t *tptr)
{
    if (prog_hol_available())
    {
        if (npHol_info_year != cur_year)
        {
            npHol_info_year = cur_year;

            npHol_info_block_idx = get_nphol_block_idx(npHol_info_year, tptr);
        }
    }
}

// return : block_idx, (0xff -> fail)
static uint8_t get_nphol_block_idx(uint8_t yr, uint8_t *tptr)
{
    uint8_t i, idx;
    uint16_t arr_len, tcnt;

    idx = 0xff;

    nv_sub_info.ch[0] = 0;  // block 0
    nv_sub_info.ch[1] = HEAD_OF_BLOCK;
    if (nv_read(I_HOLIDAYS_A, tptr))
    {
        arr_len = HOLIDAY_LEN;  // TODO: Check

        // i = 0 ==> periodic holiday
        for (i = 1; i < HOLIDAY_BLOCK_LEN; i++)
        {
            tcnt = HOLIDAYS_PER_BLOCK * (uint16_t)i;
            if (tcnt >= arr_len)
                break;

            nv_sub_info.ch[0] = i;
            nv_sub_info.ch[1] = HEAD_OF_BLOCK;
            if (nv_read(I_HOLIDAYS_A, tptr))
            {
                if (yr == ((holiday_date_type *)tptr)->yr)
                {
                    idx = i;
                    break;
                }
            }
            else
            {
                program_status |= PROG_STS_NPHOL_ARR_ERR;
            }
        }
    }
    else
    {
        program_status |= PROG_STS_NPHOL_ARR_ERR;
    }

    return idx;
}

static uint8_t get_holiday_day_id(uint8_t blk, uint8_t *tptr)
{
    uint8_t rtn_id, i;
    holiday_date_type *hol;

    rtn_id = 0xff;

    nv_sub_info.ch[0] = blk;
    nv_sub_info.ch[1] = HOL_OF_BLOCK;
    if (nv_read(I_HOLIDAYS_A, tptr))
    {
        hol = (holiday_date_type *)tptr;

        for (i = 0; i < HOLIDAYS_PER_BLOCK; i++)
        {
            if (hol->holiday[i].month == cur_month &&
                hol->holiday[i].date == cur_date)
            {
                rtn_id = hol->holiday[i].day_id;
                break;
            }
        }
    }

    return rtn_id;
}

bool is_weekends(void)
{
    bool ret = false;
    uint8_t day = calc_dayofweek(&cur_rtc);

    if (day == _SAT || day == _SUN)
        ret = true;

    return ret;
}

void phol_nv_read(uint8_t hol_type)
{
    if (PERIODIC_HOL == hol_type)
    {
        nv_sub_info.ch[0] = 0;
    }
    else
    {
        nv_sub_info.ch[0] = npHol_info_block_idx;
    }

    nv_sub_info.ch[1] = HOL_OF_BLOCK;

    if (PERIODIC_HOL == hol_type)
    {
        nv_read(I_HOLIDAYS_A, (uint8_t *)&g_pholiday);
    }
    else
    {
        nv_read(I_HOLIDAYS_A, (uint8_t *)&g_npholiday);
    }
}

holiday_date_type *get_pholiday(void) { return &g_pholiday; }

holiday_date_type *get_npholiday(void) { return &g_npholiday; }

bool is_phol_nphoiday(uint8_t hol_type)
{
    bool ret = false;
    uint8_t i;
    holiday_date_type *hol;

    if (PERIODIC_HOL == hol_type)
    {
        if ((cur_hour == 0 || cur_hour == 24) && (cur_min == 0) &&
            (cur_sec == 0 || cur_sec == 1))
        {
            phol_nv_read(PERIODIC_HOL);
        }
        hol = get_pholiday();
    }
    else
    {
        if ((cur_hour == 0 || cur_hour == 24) && (cur_min == 0) &&
            (cur_sec == 0 || cur_sec == 1))
        {
            phol_nv_read(nPERIODIC_HOL);
        }
        hol = get_npholiday();
    }

    for (i = 0; i < HOLIDAYS_PER_BLOCK; i++)
    {
        if (hol->holiday[i].month == cur_month &&
            hol->holiday[i].date == cur_date)
        {
            ret = true;
            break;
        }
    }
    return ret;
}

static uint8_t prog_nphol_proc(uint8_t *tptr)
{
    if (prog_nphol_is_available() && prog_day_available())
    {
        return get_holiday_day_id(npHol_info_block_idx, tptr);
    }

    return 0xff;
}

static uint8_t prog_phol_proc(uint8_t *tptr)
{
    if (prog_hol_available() && prog_day_available())
    {
        return get_holiday_day_id(0, tptr);
    }

    return 0xff;
}

static uint8_t prog_week_profile_proc(uint8_t *tptr)
{
    uint8_t i, rtn_id;
    uint8_t wkid, day;
    week_date_type *wk;

    rtn_id = 0xff;

    if (!season_info_available)
    {
        curr_week_id = 0xff;
        return rtn_id;
    }

    wkid = season_prof.season[curr_season_id].week_id;

    if ((program_status & PROG_STS_WEEK_NV_ERR) == 0)
    {
        if (wkid == curr_week_id)
            goto prog_week_profile_proc2;

    prog_week_profile_proc1:
        if (nv_read(I_WEEK_PROFILE_ACTIVE, tptr))
        {
            wk = (week_date_type *)tptr;
            for (i = 0; i < wk->cnt; i++)
            {
                if (wkid == wk->week[i].week_id)
                {
                    curr_week_id = wkid;
                    week_info = wk->week[i];
                    break;
                }
            }

            if (i != wk->cnt)
            {
            prog_week_profile_proc2:
                day = calc_dayofweek(&cur_rtc);
                rtn_id = week_info.day_id[day - 1];  // MON = 1, TUE = 2....
            }
        }
        else
        {
            program_status |= PROG_STS_WEEK_NV_ERR;
        }
    }
    else
    {
        curr_week_id = 0xff;

        if ((++week_recovery_cnt) >= 36000)
        {  // 10 hours
            week_recovery_cnt = 0;
            goto prog_week_profile_proc1;
        }
    }

    return rtn_id;
}

static uint8_t prog_tou_profile_proc(uint8_t *tptr, bool curr_prog_in,
                                     bool season_chk)
{
    if (!prog_tou_available())
    {
        return 0xff;
    }

    prog_season_profile_proc(tptr, curr_prog_in, season_chk);

    return prog_week_profile_proc(tptr);
}

static void get_day_profile_tou(uint8_t dayid, uint8_t *tptr)
{
    uint8_t i;
    dayid_table_type *daytable;

    if (dayid != curr_day_id)
    {
        curr_day_id = dayid;

        nv_sub_info.ch[0] = dayid;
        if (nv_read(I_DAY_PROFILE_ACTIVE, tptr))
        {
            daytable = (dayid_table_type *)tptr;

            if (dayid == daytable->day_id)
            {
                for (i = 0; i < daytable->tou_conf_cnt; i++)
                {
                    tou_data_conf[i].hour = daytable->tou_conf[i].hour;
                    tou_data_conf[i].min = daytable->tou_conf[i].min;
                    tou_data_conf[i].rate = daytable->tou_conf[i].rate;
                }
                tou_data_cnt = daytable->tou_conf_cnt;
            }
        }
    }
}

static void prog_tou_update(uint8_t *tptr, bool curr_prog_in, bool season_chk)
{
    uint8_t day_id = 0xff;

    if (cmp_date(&toupdate_time, &cur_rtc) == 0)
        return;

    toupdate_time = cur_rtc;

    if (holiday_sel1)
    {
        if (day_id > PROG_DAY_ID_MAX)
        {
            day_id = prog_nphol_proc(tptr);
        }

        if (day_id > PROG_DAY_ID_MAX)
        {
            day_id = prog_phol_proc(tptr);
        }
    }

    // TOU profile (season, week, day)
    if (day_id > PROG_DAY_ID_MAX)
    {
        day_id = prog_tou_profile_proc(tptr, curr_prog_in, season_chk);
    }
    else
    {
        prog_season_profile_proc(tptr, curr_prog_in, season_chk);
    }

    // TOU data configure =>
    if (day_id <= PROG_DAY_ID_MAX)
    {
        get_day_profile_tou(day_id, tptr);
    }
}

static ratekind_type conv_rate_to_ratekind(rate_type rt)
{
    ratekind_type rtn;

    switch (rt)
    {
    case eArate:
        rtn = ONE_RATE_KIND;
        break;
    case eBrate:
        rtn = TWO_RATE_KIND;
        break;
    case eCrate:
        rtn = THREE_RATE_KIND;
        break;
    case eDrate:
        rtn = FOUR_RATE_KIND;
        break;
    default:
        rtn = ONE_RATE_KIND;
        break;
    }

    return rtn;
}

static void pgm_daylight_saving(prog_dl_type *progdl)
{
    if (progdl->set_bits & SETBITS_DLS_BGN)
    {
        dls_info.bgn_dt = progdl->dlsinfo.bgn_dt;
        dls_info.bgn_week = progdl->dlsinfo.bgn_week;
    }

    if (progdl->set_bits & SETBITS_DLS_END)
    {
        dls_info.end_dt = progdl->dlsinfo.end_dt;
        dls_info.end_week = progdl->dlsinfo.end_week;
    }

    if (progdl->set_bits & SETBITS_DLS_DEV)
        dls_info.dev = progdl->dlsinfo.dev;

    if (progdl->set_bits & SETBITS_DLS_ENA)
    {
        if (progdl->dlsinfo.enabled)
        {
            // DLS_enable();
            DLS_enable_init();
        }
        else
        {
            DLS_disable();
        }
    }
}

static void pgm_out_sig_sel(prog_dl_type *progdl)
{
    if (progdl->set_bits & SETBITS_SIG_SEL)
    {
        sig_sel_proc(progdl->out_sig);
    }
}

/*
lhh_add_desc : program 으로 부터 pEOB_sr_dr_type, reg_mr_date 를 설정.
*/
static void pgm_pbilling(prog_dl_type *progdl)
{
    uint8_t mr_date;

    if (progdl->set_bits & SETBITS_PBILL_DRSEL)
#if 0  // 23.11.15 jp
		set_pEOB_dr(progdl->pEOB.dr_sel);
#else
        pEOB_sr_dr_type = progdl->pEOB.dr_sel;
#endif

    if (progdl->set_bits & SETBITS_PBILL_DATE)
    {
#if 1  // 24.10.08
        mr_date = progdl->regread_date;
        if (reg_mr_date != mr_date)
            tou_id_change_sts = 1;
        reg_mr_date = mr_date;
#else
            reg_mr_date = progdl->regread_date;
#endif
    }
}

/*
lhh_add_desc : program 으로 부터 npEOB_sr_dr_type 를 설정.
*/
static void pgm_npbilling(prog_dl_type *progdl)
{
    if (progdl->set_bits & SETBITS_NPBILL_DRSEL)
#if 0  // 23.11.15 jp
		set_npEOB_dr(progdl->npEOB.dr_sel);
#else
        npEOB_sr_dr_type = progdl->npEOB.dr_sel;
#endif
}

static bool pgm_suppdsp_item_from_rcnt(uint8_t progarea, uint8_t *tptr)
{
#define ITEM_READ_UNIT 5
    bool rslt;
    uint8_t i, k, itemcnt;
    uint16_t itemoffs;

    rslt = false;
    itemoffs = 0;

    // ----- disp supp item is moved from recent ----
    nv_sub_info.ch[0] = progarea;
    if (!nv_read(I_DISP_SUPP_RCNT, tptr))
        return false;

    nv_write(I_DISP_SUPP_A, tptr);

    // ---- disp supp dlms item is moved from recent ----
    nv_sub_info.seg.offset = 0;
    nv_sub_info.seg.len = 2;
    if (!nv_read(I_SUPPDSP_ITEM_RCNT, tptr))
        return false;

    nv_write(I_SUPPDSP_ITEM_A, tptr);
    itemoffs += nv_sub_info.seg.len;

    // tptr[0] is array tag
    itemcnt = tptr[1];  // array counter

    i = 0;
    while (true)
    {
        if ((i + ITEM_READ_UNIT) <= itemcnt)
        {
            k = ITEM_READ_UNIT;
        }
        else
        {
            k = itemcnt - i;
        }

        nv_sub_info.seg.offset = itemoffs;
        nv_sub_info.seg.len = k * SUPPDSP_ITEM_SIZE;
        if (!nv_read(I_SUPPDSP_ITEM_RCNT, tptr))
        {
            break;
        }

        nv_write(I_SUPPDSP_ITEM_A, tptr);
        itemoffs += nv_sub_info.seg.len;

        i += k;
        if (i >= itemcnt)
        {
            rslt = true;
            break;
        }
    }

    return rslt;
}

static void pgm_suppdsp(prog_dl_type *progdl, uint8_t *tptr)
{
    if (progdl->set_bits & SETBITS_SUPPDSP_ITEM)
    {
        if (nv_read(I_DISP_SUPP_A, tptr))
        {
            memcpy((uint8_t *)&circ_state_suppdsp_mode, tptr,
                   sizeof(dsp_supply_type));

            lcd_dsp_mode = LCD_SUPP_DSPMODE;

            dsp_circ_state_mode_init();
        }
    }
    else
    {
        if (dsp_is_suppmode_and_available())
        {
            if (pgm_suppdsp_item_from_rcnt(aprog_area_rcnt, tptr))
            {
                progdl->set_bits |=
                    SETBITS_SUPPDSP_ITEM;  // used in set of availablebits
            }
            else
            {
                // progdl->set_bits is cleared and used in set of availablebits
                lcd_dsp_mode = LCD_USER_DSPMODE;

                dsp_circ_state_mode_init();
            }
        }
    }
}

static void pgm_billing_parm(prog_dl_type *progdl)
{
    if (progdl->set_bits & SETBITS_BILLING_PARM)
        set_billing_parm(progdl->bill_parm);
}

static void pgm_dm_intv(prog_dl_type *progdl)
{
    if (progdl->set_bits & SETBITS_DM_PRD)
    {
        eoi_intv = progdl->dm_prd;
        eoi_evt |= EOI_EVENT_INTV_CHG;  // updated value of demand_intv
                                        // below is used in eoi_proc()
    }

    if (progdl->set_bits & SETBITS_DM_PRD_NUM)
    {
        eoi_intv_num = 1;
        eoi_evt |= EOI_EVENT_SUBINTV_CHG;
    }
}

static void pgm_lp_intv(prog_dl_type *progdl)
{
    if (progdl->set_bits & SETBITS_LP_INTV)
    {
        lp_interval = progdl->lp_intv;
    }
}

static void pgm_lpavg_intv(prog_dl_type *progdl)
{
    if (progdl->set_bits & SETBITS_LPAVG_INTV)
    {
        LPavg_save(&cur_rtc);
        LPavg_init();

        lpavg_interval = progdl->lpavg_intv;
    }
}

static void pgm_scurr_limit(prog_dl_type *progdl)
{
    if (progdl->set_bits & SETBITS_sCURR_LMT_VAL)
    {
        scurr_limit_level = progdl->scurr_lmt;
    }

    if (progdl->set_bits & SETBITS_sCURR_LMT2_VAL)
    {
        scurr_limit_level_2 = progdl->scurr_lmt2;
    }

    if (progdl->set_bits & SETBITS_sCURR_AUTORTN)
    {
        scurr_autortn_cnt = progdl->scurr_autortn;
    }
}

static void pgm_lcd_set_parm(prog_dl_type *progdl)
{
    ST_MIF_METER_PARM *pst_mif_meter_parm = dsm_mtp_get_meter_parm();

    if (progdl->set_bits & SETBITS_LCDSET_PARM)
    {
        // simple mode
        if (progdl->lcdsetparm & 0x08)
        {
            circdsp_smode_set();
        }
        else
        {
            circdsp_smode_reset();
        }

        // measure method
        meas_method = parse_meas_method(progdl->lcdsetparm);

        pst_mif_meter_parm->direct_reverse = mt_dir;
        pst_mif_meter_parm->meter_method = meas_method;

        mif_meter_parm_set();
    }
}

static void pgm_pwd_chg(prog_dl_type *progdl)
{
    auth_pwd_type auth;

    if (progdl->set_bits & SETBITS_PWD_CHG)
    {
        auth.pwd = progdl->pwd;
        if (appl_is_sap_utility())
        {
            nv_write(I_UTIL_PASSWORD, (uint8_t *)&auth);
            appl_util_pwd = auth.pwd;
        }
        else if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
        {
        }
        else
        {
            nv_write(I_485_PASSWORD, (uint8_t *)&auth);
            appl_485_pwd = auth.pwd;
        }
    }
}

static void pgm_ts_conf(prog_dl_type *progdl)
{
    if ((progdl->set_bits & (SETBITS_TS_CTRL | SETBITS_TS_ZONE)) ==
        (SETBITS_TS_CTRL | SETBITS_TS_ZONE))
    {
        ts_conf = prog_dl.ts;
    }
}

static void pgm_scurr_hold(prog_dl_type *progdl)
{
    if (progdl->set_bits_1 & SETBITS_EXT_SCURR_1)
    {
        scurr_det_hold = progdl->_scurr_det_hold;
    }
}

static void pgm_scurr_rtn_1(prog_dl_type *progdl)
{
    if (progdl->set_bits_1 & SETBITS_EXT_SCURR_1)
    {
        scurr_rtn_dur_1 = progdl->_scurr_rtn_dur1;
    }
}

static void pgm_scurr_rtn_2(prog_dl_type *progdl)
{
    if (progdl->set_bits_1 & SETBITS_EXT_SCURR_1)
    {
        scurr_rtn_dur_2 = progdl->_scurr_rtn_dur2;
    }
}

static void pgm_scurr_cnt_n1(prog_dl_type *progdl)
{
    if (progdl->set_bits_1 & SETBITS_EXT_SCURR_1)
    {
        scurr_cnt_n1 = progdl->_scurr_cnt_n1;
    }
}

static void pgm_holiday_sel1(prog_dl_type *progdl, uint8_t *tptr,
                             bool season_chk)
{
    // U8 *tptr;
    // tptr = adjust_tptr(&global_buff[0]);

    if (progdl->set_bits_1 & SETBITS_HOLIDAY_SEL_1)
    {
        if (holiday_sel1 != progdl->hol_sel1)
        {
            holiday_sel1 = progdl->hol_sel1;

            DPRINTF(DBG_ERR,
                    "%s:  1  currprog_available_bits[0x%x] "
                    "futprog_available_bits[0x%x] \r\n",
                    __func__, currprog_available_bits, futprog_available_bits);

            DPRINTF(DBG_ERR,
                    "	currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
                    "progdl->set_bits_1[0x%x]\r\n",
                    currprog_available_bits, progdl->set_bits,
                    progdl->set_bits_1);

            prog_tou_refresh(tptr, season_chk);

            DPRINTF(DBG_ERR,
                    "%s:  2  currprog_available_bits[0x%x] "
                    "futprog_available_bits[0x%x] \r\n",
                    __func__, currprog_available_bits, futprog_available_bits);

            DPRINTF(DBG_ERR,
                    "	currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
                    "progdl->set_bits_1[0x%x]\r\n",
                    currprog_available_bits, progdl->set_bits,
                    progdl->set_bits_1);

            lp_event_set(LPE_PROGRAM_CHG);

            DPRINTF(DBG_ERR,
                    "%s:  3  currprog_available_bits[0x%x] "
                    "futprog_available_bits[0x%x] \r\n",
                    __func__, currprog_available_bits, futprog_available_bits);

            DPRINTF(DBG_ERR,
                    "	currprog_available_bits[0x%x] progdl->set_bits[0x%x] "
                    "progdl->set_bits_1[0x%x]\r\n",
                    currprog_available_bits, progdl->set_bits,
                    progdl->set_bits_1);
        }
    }
}

static uint8_t prog_get_week_profile_same_check(uint8_t *tptr)
{
    uint8_t i, j;
    week_date_type *weektable;
    uint8_t day_type, day_type1;

    if (nv_read(I_WEEK_PROFILE_ACTIVE, tptr))
    {
        weektable = (week_date_type *)tptr;

        day_type = weektable->week[0].day_id[0];

        DPRINTF(DBG_ERR, _D "%s: day_type[%d]\r\n", __func__, day_type);

        for (i = 0; i < WEEK_PROF_SIZE; i++)
        {
            for (j = 0; j < WEEK_LEN; j++)
            {
                day_type1 = weektable->week[i].day_id[j];
                if (day_type1 != day_type)
                    return 0xff;
            }
        }
    }
    else
        return 0xff;

    return day_type;
}

static ratekind_type prog_get_day_profile_ratekind(uint8_t *tptr)
{
    uint8_t i, j;
    dayid_table_type *daytable;
    rate_type rt, rt1;
    uint8_t day_type;

    rt = eArate;

    day_type = prog_get_week_profile_same_check(tptr);

    if (0xff != day_type)
    {
        nv_sub_info.ch[0] = day_type;
        if (nv_read(I_DAY_PROFILE_ACTIVE, tptr))
        {
            daytable = (dayid_table_type *)tptr;

            for (j = 0; j < daytable->tou_conf_cnt; j++)
            {
                rt1 = SELECTOR_TO_RATE(daytable->tou_conf[j].rate);
                if (rt1 > rt)
                    rt = rt1;
            }
        }
    }
    else
    {
        for (i = 0; i < DAY_PROF_SIZE; i++)
        {
            nv_sub_info.ch[0] = i;
            if (nv_read(I_DAY_PROFILE_ACTIVE, tptr))
            {
                daytable = (dayid_table_type *)tptr;

                for (j = 0; j < daytable->tou_conf_cnt; j++)
                {
                    rt1 = SELECTOR_TO_RATE(daytable->tou_conf[j].rate);
                    if (rt1 > rt)
                        rt = rt1;
                }
            }
        }
    }
    return conv_rate_to_ratekind(rt);
}

static rate_type get_curr_rate(void)
{
    uint8_t i;

    if (tou_data_cnt == 0)
        return SELECTOR_TO_RATE(tou_data_conf[0].rate);

    for (i = 0; i < tou_data_cnt; i++)
    {
        if (cmp_hhmm(cur_hour, cur_min, tou_data_conf[i].hour,
                     tou_data_conf[i].min) < 0)
            break;
    }

    if (i == 0)
        i = tou_data_cnt;

    return SELECTOR_TO_RATE(tou_data_conf[--i].rate);
}

#if 0 /* bccho, 2024-01-10, 1227 포팅 */
rate_type rtn_curr_rate(date_time_type *dt)
{
    uint8_t i;

    if (mt_is_one_ratekind() || tou_data_cnt == 0)
        return SELECTOR_TO_RATE(tou_data_conf[0].rate);

    for (i = 0; i < tou_data_cnt; i++)
    {
        if (cmp_hhmm(dt->hour, dt->min, tou_data_conf[i].hour,
                     tou_data_conf[i].min) < 0)
            break;
    }

    if (i == 0)
        i = tou_data_cnt;

    return SELECTOR_TO_RATE(tou_data_conf[--i].rate);
}
#endif

void prog_dl_backup_restore(void)
{
    if (!nv_read(I_BACKUP_PROG_DL, (uint8_t *)&prog_dl))
    {
        memset((uint8_t *)&prog_dl, 0, sizeof(prog_dl_type));
    }
}

void prog_dl_backup_save(void)
{
    if (prog_dlcmd_avail)
    {
        nv_write(I_BACKUP_PROG_DL, (uint8_t *)&prog_dl);
    }
}

void tou_set_cnt_restore(void)
{
    if (!nv_read(I_TOU_SET_CNT, (uint8_t *)&tou_set_cnt))
    {
        tou_set_cnt_reset();
    }
}

bool tou_set_cnt_save(void)
{
    if (prog_dlcmd_avail)
    {
        nv_write(I_TOU_SET_CNT, (uint8_t *)&tou_set_cnt);
    }
    return 1;
}

void ext_tou_id_reset(void)
{
    int i;

    i = 0;

    // TOU ID -> same as prog_id (attid = 2 or 6 of cal object)
    memset(&tou_set_cnt.ext_tou_id[i], ' ', PROG_ID_SIZE);
    i += PROG_ID_SIZE;

    // unique code ( 4byte = 32 bit, bit31 means 0-> current prog, 1 -> future
    // prog)
    tou_set_cnt.ext_tou_id[i++] = 0x7f;
    tou_set_cnt.ext_tou_id[i++] = 0xff;
    tou_set_cnt.ext_tou_id[i++] = 0xff;
    tou_set_cnt.ext_tou_id[i++] = 0xff;

    // reserved (2 byte)
    tou_set_cnt.ext_tou_id[i++] = 0x00;
    tou_set_cnt.ext_tou_id[i++] = 0x00;

    // V_CRC -> total tou value CRC
    tou_set_cnt.ext_tou_id[i++] = 0xff;
    tou_set_cnt.ext_tou_id[i++] = 0xff;
}

void tou_set_cnt_reset(void)
{
    ext_tou_id_reset();

    tou_set_cnt.cosem_cnt = 0;
    tou_set_cnt.last_classid = 0;

    memset(tou_set_cnt.last_obis, 0, OBIS_ID_SIZE);

    tou_set_cnt.last_attid = 0;
    tou_set_cnt.last_arrayidx = 0xffff;
    tou_set_cnt.tou_crc = 0xffff;
}

bool tou_set_is_reset(void)
{
    return (tou_set_cnt.cosem_cnt == 0) ? true : false;
}

bool touset_extprogid_set(uint8_t *extprogid)
{
    memcpy(tou_set_cnt.ext_tou_id, extprogid, EXT_TOU_ID_SIZE);

    return true;
}

bool tou_set_is_new(uint8_t *obis, uint8_t attid)
{
    if ((memcmp(tou_set_cnt.last_obis, obis, 6) != 0) ||
        (tou_set_cnt.last_attid != attid))
        return true;

    return false;
}

bool touset_last_obj_set(uint16_t classid, uint8_t *obis, uint8_t attid,
                         uint16_t arrayidx, uint8_t *msg, int msglen)
{
    if (tou_set_is_new(obis, attid))
    {
        tou_set_cnt.cosem_cnt += 1;

        tou_set_cnt.last_classid = classid;
        memcpy(tou_set_cnt.last_obis, obis, OBIS_ID_SIZE);
        tou_set_cnt.last_attid = attid;
    }

    tou_set_cnt.last_arrayidx = arrayidx;

    tou_set_cnt.tou_crc = crc16_update(tou_set_cnt.tou_crc, msg, msglen);

    return true;
}

bool prog_work_is_valid(bool is_curr)
{
    uint16_t v_crc;

    v_crc = ((uint16_t)tou_set_cnt.ext_tou_id[EXT_TOU_VCRC_MSB_POS]) << 8 |
            tou_set_cnt.ext_tou_id[EXT_TOU_VCRC_MSB_POS + 1];

    if (v_crc == 0xffff)
        return true;  // not crc check

    if (tou_set_cnt.tou_crc == v_crc)
    {
        if (is_curr)
        {
            if ((tou_set_cnt.ext_tou_id[EXT_TOU_UNIQ_MSB_POS] & 0x80) == 0)
                return true;
        }
        else
        {
            if ((tou_set_cnt.ext_tou_id[EXT_TOU_UNIQ_MSB_POS] & 0x80) != 0)
                return true;
        }
    }

    return false;
}
