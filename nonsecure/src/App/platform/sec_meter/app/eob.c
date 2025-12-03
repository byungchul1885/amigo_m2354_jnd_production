#include "options.h"
#include "meter_app.h"
#include "eoi.h"
#include "amg_sec.h"
#include "eob.h"
#include "lp.h"
#include "nv.h"
#include "get_req.h"
#include "amg_debug.h"
#include "whm.h"

extern bool bat_rtc_backuped, eob_pwrtn;
extern bool b_season_changed_pwtr_back, b_season_changed_rtc_back;

/* bccho, 2024-09-05, 삼상 */
uint8_t float_to_byte(float idata, uint8_t *odata);

date_time_type pwrtn_EOB_dt;
mr_data_accm_type mr_data_accm;
mr_data_dm_type mr_data_dm;
mr_data_info_type mr_data_info;

date_time_type sr_dt_prd;
date_time_type sr_dt_nprd;
date_time_type sr_dt_sch;
date_time_type sr_log_dt;

npbill_date_type npBill_date;
bool b_eob_proc;

static void mr_reading(uint8_t eob_type, date_time_type *pdt, bool isdr,
                       uint8_t *tptr);
static bool eob_get_npEOB(date_time_type *pdt, uint8_t *tptr);
static float calc_diff_pf(uint32_t *ch1, uint32_t *ch2, energy_dir_type dir);
static void sel_react_monitor(date_time_type *dt);
void sr_log_dt_init(void);

#if 0 /* bccho, 2024-09-05, 삼상, delete */
bool mr_prdnprdseason_from_nv(uint8_t gb, mr_data_sel_type sel, uint8_t gc,
                              uint8_t gf, rate_type rt, energy_dir_type dir,
                              uint8_t *tptr);
uint8_t clock_to_format_for_sign(date_time_type *dt, uint8_t *p_odata);
uint8_t u32_to_byte(uint32_t idata, uint8_t *odata);
#endif

/************************************************/
/*        비정기 / season 이 추가 되면서        */
/*        mr_data_accm/mr_data_dm/mr_data_info  */
/*        NV로 부터 로딩 컨셉 확정 필요..       */
/************************************************/
/*        차후 업데이트 필요 함                 */
/************************************************/
void eob_init(void)
{
    int i;

    if (mr_cnt)
    {
        for (i = 0; i < numRates; i++)
        {
            nv_sub_info.mr.mrcnt = mr_cnt;
            nv_sub_info.mr.sel = eMrAccm;
            nv_sub_info.mr.rt = (rate_type)i;
            if (!nv_read(I_MT_READ_DATA, (uint8_t *)&mr_data_accm.accm[i]))
                memset((uint8_t *)&mr_data_accm.accm[i], 0, sizeof(mr_ch_type));

            nv_sub_info.mr.mrcnt = mr_cnt;
            nv_sub_info.mr.sel = eMrDm;
            nv_sub_info.mr.rt = (rate_type)i;
            if (!nv_read(I_MT_READ_DATA, (uint8_t *)&mr_data_dm.dm[i]))
                memset((uint8_t *)&mr_data_dm.dm[i], 0, sizeof(mr_dm_type));
        }
        nv_sub_info.mr.mrcnt = mr_cnt;
        nv_sub_info.mr.sel = eMrInfo;
        if (!nv_read(I_MT_READ_DATA, (uint8_t *)&mr_data_info))
            memset((uint8_t *)&mr_data_info, 0, sizeof(mr_data_info_type));
    }
    else
    {
        memset((uint8_t *)&mr_data_accm, 0, sizeof(mr_data_accm_type));
        memset((uint8_t *)&mr_data_dm, 0, sizeof(mr_data_dm_type));
        memset((uint8_t *)&mr_data_info, 0, sizeof(mr_data_info_type));
    }

    sr_dt_init();
    sr_log_dt_init();
}

void npBill_date_init(void) { npBill_date.cnt = 0; }

/*
lhh_add_desc : 비정기 date 로딩...
*/
void npBill_date_load(void)
{
    if (prog_npbill_available())
    {
        if (!nv_read(I_NP_BILLDATE_A, (uint8_t *)&npBill_date))
        {
            npBill_date.cnt = 0;
        }
    }
    else
    {
        npBill_date.cnt = 0;
    }
}

void npBill_date_load_forFut_notAvailxx(prog_dl_type *progdl)
{
    if (!futprog_npbill_available())
    {
        ST_npBILL_BACKUP *pst_npBILL_date = prog_get_curr_npbill_date_backup();

        if (pst_npBILL_date->curprog_available)
        {
            memcpy((uint8_t *)&npBill_date, (uint8_t *)&pst_npBILL_date->date,
                   sizeof(npbill_date_type));
            progdl->set_bits |= SETBITS_NPBILL_DATE;
            DPRINTF(DBG_WARN, "%s: npcnt[%d]\r\n", __func__, npBill_date.cnt);
        }
    }
    else
    {
#if 0  // 2024.04.19 bug fix  jp
		 npBill_date.cnt = 0;
#endif
    }
}

void sr_log_dt_init(void) { sr_log_dt.year = 0xff; }

void sr_dt_init(void)
{
    sr_dt_prd.year = 0xff;
    sr_dt_nprd.year = 0xff;
    sr_dt_sch.year = 0xff;
}

/*
lhh_add_desc : pfinfo (PF 정보 ) 의 time 정보와 정기/비정기 검침일 과 비교하여
SR/DR 처리 season이 변경되엇는지 체크하여 SR/DR 처리 선택 무효 체크 및
sel_react_monitor_rtchg 실행..
*/
void pwrtn_eob_proc(pwrfail_info_type *pfinfo, uint8_t *tptr)
{
    date_time_type tdt;
    bool selreact_mon;
    uint8_t srdr;
    uint8_t eob_type = 0;
    srdr = 0;

    selreact_mon = false;

    if (pfinfo->dt.year == 0)
        return;

    tdt = pfinfo->dt;

    if (tchg_is_within_peob(&tdt, &cur_rtc) > 0)
    {
        srdr |= pEOB_sr_dr_type;
        selreact_mon = true;
        if (pEOB_sr_dr_type)
            eob_type |= EOB_PERIOD_FLAG;
        eob_pwrtn = true;
        pwrtn_EOB_dt = cur_rtc;
    }

    if (eob_timechg_npEOB(&tdt, &cur_rtc) > 0)
    {
        srdr |= npEOB_sr_dr_type;
        if (npEOB_sr_dr_type)
            eob_type |= EOB_nPERIOD_FLAG;
    }

    /* 다른 비 정기 event 체크 */
#if 1
    if (prog_changed_event())
    {
        prog_changed_event_clr();
        srdr |= npEOB_sr_dr_type;
        if (npEOB_sr_dr_type)
            eob_type |= EOB_nPERIOD_FLAG;
    }
#endif

    if (prog_season_changed())
    {
        prog_season_changed_clr();
        srdr |= seasonCHG_sr_dr_type;
        if (seasonCHG_sr_dr_type)
            eob_type |= EOB_SEASON_FLAG;
    }

    if ((srdr & (MR_SR_BIT | MR_DR_BIT)) != 0)
    {
        sr_dr_proc(eob_type, srdr, &cur_rtc, tptr);
    }

    if (selreact_mon)
    {
        sel_react_monitor_rtchg(&cur_rtc);
    }
}

/*
lhh_add_desc : end of bill 처리
정기, 비정기, Season 변경 체크하여 SR, DR 처리..
*/
void eob_proc(uint8_t *tptr)
{
    uint8_t srdr;
    uint8_t eob_type = 0;
    srdr = 0;

    /* 다른 비 정기 event 체크 */
#if 1
    if (prog_changed_event())
    {
        prog_changed_event_clr();
        srdr |= npEOB_sr_dr_type;
        if (npEOB_sr_dr_type)
            eob_type |= EOB_nPERIOD_FLAG;
    }
#endif

    /* Season 변경 체크*/
    if (prog_season_changed())
    {
        prog_season_changed_clr();
        srdr |= seasonCHG_sr_dr_type;
        if (seasonCHG_sr_dr_type)
            eob_type |= EOB_SEASON_FLAG;
    }

    if (b_eob_proc)
    {
        b_eob_proc = false;
        /* 정기 검침일 체크*/
        if (cur_date == reg_mr_date)
        {
            srdr |= pEOB_sr_dr_type;
            sel_react_monitor(&cur_rtc);
            if (pEOB_sr_dr_type)
                eob_type |= EOB_PERIOD_FLAG;
        }

        /* 비 정기 검침일 체크*/
        if (eob_get_npEOB(&cur_rtc, tptr))
        {
            srdr |= npEOB_sr_dr_type;
            if (npEOB_sr_dr_type)
                eob_type |= EOB_nPERIOD_FLAG;
        }
    }

    if ((srdr & (MR_SR_BIT | MR_DR_BIT)) != 0)
    {
        sr_dr_proc(eob_type, srdr, &cur_rtc, tptr);
    }
}

/*
lhh_add_desc :
    RATE별 ch 별로 전력량 및 pf 를 accm 에 get 한다.
*/
void mr_capture_accm(mr_data_accm_type *accm)
{
    rate_type i;
    float pf[numDirChs];
    uint32_t ch[numCHs];

    // meter read of each rate
    for (i = eArate; i < numRates; i++)
    {
        get_accm_chs(i, ch);

        pf[eDeliAct] = calc_diff_pf(ch, mr_data_accm.accm[i].ch, eDeliAct);
        pf[eReceiAct] = calc_diff_pf(ch, mr_data_accm.accm[i].ch, eReceiAct);

        // accm ch
        accm_ch_copy(accm->accm[i].ch, ch);
        // pf
        accm->accm[i].pf[eDeliAct] = pf[eDeliAct];
        accm->accm[i].pf[eReceiAct] = pf[eReceiAct];
    }
}

/*
lhh_add_desc :
    RATE별 ch 별로 수요 전력을 accm 에 get 한다.
*/
void mr_capture_dm(mr_data_dm_type *dm, bool isdr)
{
    rate_type i;
#if defined(FEATURE_DRIVER_PORT_AT_EXT_SOLUTION)
    demand_ch_type j;  // 원 소스 오류로 보임...
#else
    energy_ch_type j;
#endif

    // meter read of each rate
    for (i = eArate; i < numRates; i++)
    {
        for (j = 0; j < numDmCHs; j++)
        {
            // cum max dm
            dm->dm[i].cum_mxdm[j] = mr_data_dm.dm[i].cum_mxdm[j];
            if (isdr)
            {
                dm->dm[i].cum_mxdm[j] += max_dm.max[i].dm[j].val;
            }

            if (dm->dm[i].cum_mxdm[j] >= mxdm_dgt_cnt)
                dm->dm[i].cum_mxdm[j] -= mxdm_dgt_cnt;

            // max dm
            dm->dm[i].mxdm[j] = max_dm.max[i].dm[j];
        }
    }
}

void mr_capture_rolldm(rate_type rt, demand_ch_type dmch,
                       rolling_dm_ch_type *rollch)
{
    nv_sub_info.cur.rt = rt;
    nv_sub_info.cur.chsel = dmch;
    if (!nv_read(I_DM_SUBLOCKS_DATA, (uint8_t *)rollch))
    {
        memset((uint8_t *)rollch, 0, sizeof(rolling_dm_ch_type));
    }
}

void man_sr_dr_proc(date_time_type *pdt, uint8_t *tptr)
{
    uint8_t eob_type = 0;
    if (mDR_sr_dr_type)
        eob_type = EOB_nPERIOD_FLAG;
    sr_dr_proc(eob_type, mDR_sr_dr_type, pdt, tptr);
    log_mDR(pdt);
    lp_event_set(LPE_IRRG_WR | LPE_nPERIOD);
}

bool is_mrdt_within_5days(void) { return false; }
// month 0(cur), 1(pre), 2
uint8_t month_to_grp_f(uint8_t month)
{
    uint8_t grp_f = 0xff;
    if (month != 0)
    {
        grp_f = month + VZ_LASTEST_VAL - 1;
    }

    return grp_f;
}

#if 0  // jp.kim 25.06.22
uint32_t get_bf_mxdm(uint8_t month, rate_type rt, demand_ch_type ch,
                     uint8_t *tptr)
{
    uint32_t t32;
    mr_dm_type *mr;

    t32 = 0L;

    if (mr_cnt >= month)
    {
        if (month == 1)
        {
            t32 = mr_data_dm.dm[rt].mxdm[ch].val;
        }
        else
        {
            nv_sub_info.mr.mrcnt = get_mr_idx(month_to_grp_f(month));
            nv_sub_info.mr.rt = rt;
            nv_sub_info.mr.sel = eMrDm;
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                mr = (mr_dm_type *)tptr;
                t32 = mr->mxdm[ch].val;
            }
        }
    }

    return t32;
}

void get_bf_mxtime(date_time_type *dt, uint8_t month, rate_type rt,
                   demand_ch_type ch, uint8_t *tptr)
{
    mr_dm_type *mr;

    if (mr_cnt >= month)
    {
        if (month == 1)
        {
            *dt = mr_data_dm.dm[rt].mxdm[ch].dt;
            return;
        }
        else
        {
            nv_sub_info.mr.mrcnt = get_mr_idx(month_to_grp_f(month));
            nv_sub_info.mr.rt = rt;
            nv_sub_info.mr.sel = eMrDm;
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                mr = (mr_dm_type *)tptr;
                *dt = mr->mxdm[ch].dt;
                return;
            }
        }
    }
    memset((uint8_t *)dt, 0, sizeof(date_time_type));
}

uint32_t get_bf_cumdm_nprd(uint8_t month, rate_type rt, demand_ch_type ch,
                           uint8_t *tptr)
{
    uint32_t t32;
    mr_dm_type *mr;

    t32 = 0L;

    if (mrcnt_nprd >= month)
    {
        nv_sub_info.mr.mrcnt = get_mr_idx_nperiod(month_to_grp_f(month));
        nv_sub_info.mr.rt = rt;
        nv_sub_info.mr.sel = eMrDm;
        if (nv_read(I_MT_READ_DATA_nPRD, tptr))
        {
            mr = (mr_dm_type *)tptr;
            t32 = mr->cum_mxdm[ch];
        }
    }

    return t32;
}

uint32_t get_bf_cumdm(uint8_t month, rate_type rt, demand_ch_type ch,
                      uint8_t *tptr)
{
    uint32_t t32;
    mr_dm_type *mr;

    t32 = 0L;

    if (mr_cnt >= month)
    {
        if (month == 1)
        {
            t32 = mr_data_dm.dm[rt].cum_mxdm[ch];
        }
        else
        {
            nv_sub_info.mr.mrcnt = get_mr_idx(month_to_grp_f(month));
            nv_sub_info.mr.rt = rt;
            nv_sub_info.mr.sel = eMrDm;
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                mr = (mr_dm_type *)tptr;
                t32 = mr->cum_mxdm[ch];
            }
        }
    }

    return t32;
}
#endif

uint8_t get_cur_pf(rate_type rt, energy_dir_type dir)
{
    float pf;
    uint32_t ch[numCHs];

    get_accm_chs(rt, ch);
    pf = calc_diff_pf(ch, mr_data_accm.accm[rt].ch, dir);

    if (pf == -1.0)
    {
        return 0xff;
    }
    return (uint8_t)(pf * 100.0);
}

#if 0  // jp.kim 25.06.22
uint8_t get_bf_pf(uint8_t month, rate_type rt, energy_dir_type dir,
                  uint8_t *tptr)
{
    float pf;

    pf = -1.0;
    if (mr_cnt >= month)
    {
        if (month == 1)
        {
            pf = mr_data_accm.accm[rt].pf[dir];
        }
        else
        {
            nv_sub_info.mr.mrcnt = get_mr_idx(month_to_grp_f(month));
            nv_sub_info.mr.rt = rt;
            nv_sub_info.mr.sel = eMrAccm;
            if (nv_read(I_MT_READ_DATA, tptr))
            {
                pf = ((mr_ch_type *)tptr)->pf[dir];
            }
        }
    }

    if (pf == -1.0)
    {
        return 0xff;
    }
    return (uint8_t)(pf * 100.0);
}
#endif

/*
lhh_add_desc :
    1. SR, DR 처리하는 함수
    2. SR 먼저 수행 후 DR 수행
    3. DR 인 경우 demand reset 처리
*/
void sr_dr_proc(uint8_t eob_type, uint8_t srdr, date_time_type *pdt,
                uint8_t *tptr)
{
    DPRINTF(DBG_ERR, "%s: eob_type[0x%x], srdr[0x%x]\r\n", __func__, eob_type,
            srdr);

    if (srdr & (MR_DR_BIT | MR_SR_BIT))
    {
        bool isdr;
        isdr = (srdr & MR_DR_BIT) ? true : false;
        // SR 이전 처리 해야 함
        eoi_proc_dr(pdt, isdr, tptr);
    }

    if ((srdr & MR_SR_BIT) && (((cmp_date_time(pdt, &sr_dt_prd) != 0) &&
                                (eob_type & EOB_PERIOD_FLAG)) ||
                               ((cmp_date_time(pdt, &sr_dt_nprd) != 0) &&
                                (eob_type & EOB_nPERIOD_FLAG)) ||
                               ((cmp_date_time(pdt, &sr_dt_sch) != 0) &&
                                (eob_type & EOB_SEASON_FLAG))))
    {
        bool isdr;
        isdr = (srdr & MR_DR_BIT) ? true : false;

        mr_reading(eob_type, pdt, isdr, tptr);

        if (cmp_date_time(pdt, &sr_log_dt) != 0)
        {
            log_sr_dr(pdt, MR_SR_BIT);
#if 1  // jp.kim 25.06.22
       // 전월 event -> 전 전월 event
            mr_bill_prd_type[1] = mr_bill_prd_type[0];
            mr_bill_prd_type[0] = eob_type;
            DPRINTF(DBG_ERR,
                    "%s:  mr_bill_prd_type[0]:%d  mr_bill_prd_type[1]:%d  \r\n",
                    __func__, mr_bill_prd_type[0], mr_bill_prd_type[1]);
        }
#endif
    }

    if ((srdr & MR_DR_BIT) && (cmp_date_time(pdt, &dr_dt) != 0))
    {
        DPRINTF(DBG_INFO, "DR: Demand Reset\r\n");  // 구간 내 시간 과거
        dm_reset(pdt);
        log_sr_dr(pdt, MR_DR_BIT);
    }
    if (eob_type & EOB_PERIOD_FLAG)
        lp_event_set(LPE_PERIOD);
    if (eob_type & EOB_nPERIOD_FLAG)
        lp_event_set(LPE_nPERIOD);
    if (eob_type & EOB_SEASON_FLAG)
        lp_event_set(LPE_SEASON);
}

extern bool mxdm_processed;
void sr_dr_proc_manual(uint8_t eob_type, date_time_type dt)
{
    /* this function only called from console command */
    uint8_t tptr[512];
    mxdm_processed = false;
    eoi_proc_dr(&dt, MR_DR_BIT, tptr);
    mr_reading(eob_type, &dt, (MR_SR_BIT | MR_DR_BIT), tptr);
    dm_reset(&dt);
    log_sr_dr(&dt, MR_SR_BIT);
    log_sr_dr(&dt, MR_DR_BIT);

    switch (eob_type)
    {
    case EOB_PERIOD_FLAG:
        lp_event_set(LPE_PERIOD);
        break;
    case EOB_nPERIOD_FLAG:
        lp_event_set(LPE_nPERIOD);
        break;
    case EOB_SEASON_FLAG:
        lp_event_set(LPE_SEASON);
        break;
    }

    time_up(&dt, 10);
}

float calc_pf(uint32_t wh, uint32_t varh)
{
    int32_t t32;
    ufloat_type uf;
    float wval, vaval;

    if (wh == 0L && varh == 0L)
        return (float)-1.0;

#if 1  // jp.kim 25.04.30
    if (wh == 1L && varh == 0L)
        return (float)-1.0;
    if (wh == 0L && varh == 1L)
        return (float)-1.0;
    if (wh == 1L && varh == 1L)
        return (float)-1.0;
#endif

    if (mt_selreact == 0x04)
    {
        // varh is apparent
        t32 = varh;
        vaval = (float)varh;
    }
    else
    {
        // varh is reactive
        if (varh == 0L)
            return (float)1.0;

        vaval = VAh_WVAR_f(wh, varh);
        t32 = (uint32_t)vaval;
    }
    if (wh == 0L || t32 == 0L)
        return (float)0.0;

    wval = (float)wh;

    if (wval > vaval)
        return (float)1.0;

    uf.f = wval / vaval;
    if (uf.ul == NaN || uf.ul == plusINF || uf.ul == minusINF)
        return (float)0.0;

    return uf.f;
}

void sel_react_cancel(void)
{
    sel_react_yr = 0x00;
    sel_react_mon = 0x00;
}

typedef enum
{
    eBILL_JUST_DATE,
    eBILL_EVERY_YEAR,
    eBILL_EVERY_MONTH,
    eBILL_EVERY_WEEK,
    eBILL_EVERY_DAY
} bill_date_type;

bill_date_type get_bill_date_type(day_time_type *pday)
{
    if (pday->dt.year != 0xff)
        return eBILL_JUST_DATE;

    if (pday->dt.month != 0xff)
        return eBILL_EVERY_YEAR;

    if (pday->dt.date != 0xff)
        return eBILL_EVERY_MONTH;

    if (is_good_day(pday->day))
        return eBILL_EVERY_WEEK;

    return eBILL_EVERY_DAY;
}

void fill_toward_bill_date(date_time_type *dt, bill_date_type _type,
                           day_time_type *billdt, date_time_type *from,
                           bool dir)
{
    bool pos;
    uint8_t _wk;
    uint8_t t8;

    switch (_type)
    {
    case eBILL_JUST_DATE:
        *dt = billdt->dt;
        break;

    case eBILL_EVERY_YEAR:
        *dt = billdt->dt;
        dt->year = from->year;
        pos = cmp_date_time(dt, from) > 0 ? true : false;
        if (dir)
        {
            if (!pos)
            {
                dt->year += 1;
            }
        }
        else
        {
            if (pos)
            {
                dt->year -= 1;
            }
        }
        break;

    case eBILL_EVERY_MONTH:
        *dt = billdt->dt;
        dt->year = from->year;
        dt->month = from->month;
        pos = cmp_date_time(dt, from) > 0 ? true : false;
        if (dir)
        {
            if (!pos)
            {
                dt->month += 1;
                if (dt->month > 12)
                {
                    dt->year += 1;
                    dt->month = 1;
                }
            }
        }
        else
        {
            if (pos)
            {
                dt->month -= 1;
                if (dt->month == 0)
                {
                    if (dt->month == 0)
                    {
                        dt->year -= 1;
                        dt->month = 12;
                    }
                }
            }
        }
        break;

    case eBILL_EVERY_WEEK:
        *dt = *from;
        _wk = calc_dayofweek(dt);
        if (dir)
        {
            if (billdt->day > _wk)
            {
                t8 = billdt->day - _wk;
            }
            else
            {
                t8 = 7 + billdt->day - _wk;
            }
            date_up(dt, t8);
        }
        else
        {
            if (_wk >= billdt->day)
            {
                t8 = _wk - billdt->day;
            }
            else
            {
                t8 = 7 + _wk - billdt->day;
            }
            date_down(dt, t8);
        }
        break;

    case eBILL_EVERY_DAY:
        *dt = billdt->dt;
        dt->year = from->year;
        dt->month = from->month;
        dt->date = from->date;
        pos = cmp_date_time(dt, from) > 0 ? true : false;
        if (dir)
        {
            if (!pos)
            {
                date_up(dt, 1);
            }
        }
        else
        {
            if (pos)
            {
                date_down(dt, 1);
            }
        }
        break;
    }
}

// return
//       1 : from < billdt <= to
//      -1 : to < billdt < from
//       0 : billdt is out of range
int8_t bill_date_range_check(day_time_type *billdt, date_time_type *from,
                             date_time_type *to, bool chgdir)
{
    date_time_type dt;
    bill_date_type billdt_type;

    billdt_type = get_bill_date_type(billdt);

    if (billdt_type == eBILL_JUST_DATE)
    {
        if (chgdir)  // from 보다 to 가 미래인 경우
        {
            if (cmp_date_time(&billdt->dt, from) > 0 &&
                cmp_date_time(to, &billdt->dt) >= 0)
            {
                // 검침일보다 현재 시간이 과거인 경우  & 검침일보다 설정할
                // 시간이 과거가 아닌 경우 현재 시간이 검침일 이전인 경우 &
                // 설정할 시간이 검침일이거나 검침일 이후인 경우
                DPRINTF(DBG_INFO, "%s: Before - MR\r\n", __func__);
                return 1;
            }
        }
        else  // from 보다 to 가 미래가 아닌 경우 (현재 or 과거)
        {
            if (cmp_date_time(&billdt->dt, to) > 0 &&
                cmp_date_time(from, &billdt->dt) > 0)
            {
                // 검침일이 설정할 시간보다 미래인 경우 & 검침일보다 현재의
                // 시간이 미래인 경우 현재 시간이 검침일 이후인 경우 & 설정할
                // 시간이 검침일 이전인 경우
                DPRINTF(DBG_INFO, "%s: After - MR\r\n", __func__);
                return -1;
            }
        }
        return 0;
    }

    fill_toward_bill_date(&dt, billdt_type, billdt, from, chgdir);

    if (chgdir)  // from 보다 to 가 미래인 경우
    {
        if (cmp_date_time(&dt, to) <= 0)
        {
            DPRINTF(DBG_INFO, "%s: Before - Every\r\n", __func__);
            // 현재 시간이 검침일 이전인 경우 & 설정할 시간이 검침일이거나
            // 검침일 이후인 경우 ?
            return 1;
        }
    }
    else  // from 보다 to 가 미래가 아닌 경우 (현재 or 과거)
    {
        if (cmp_date_time(&dt, to) > 0)
        {
            DPRINTF(DBG_INFO, "%s: After - Every\r\n", __func__);
            // 현재 시간이 검침일 이후인 경우 & 설정할 시간이 검침일 이전인 경우
            // ?
            return -1;
        }
    }

    DPRINTF(DBG_INFO, "%s: Unknown - Every\r\n", __func__);
    return 0;
}

int8_t eob_timechg_npEOB(date_time_type *oldt, date_time_type *newdt)
{
    int8_t rslt = 0;
    uint8_t i;
    bool chgdir;

    if (oldt->year == 0)
    {
        DPRINTF(DBG_INFO, "%s: dt year is zero, result[%d]\r\n", __func__,
                rslt);
        return rslt;
    }

    if (prog_npbill_available())
    {
        // 비정기 검침일이 설정되어 있는 경우 ?
        chgdir = cmp_date_time(oldt, newdt) < 0 ? true : false;

        DPRINTF(DBG_INFO, "%s: cmp_dt[%d]\r\n", __func__, chgdir);

        for (i = 0; i < npBill_date.cnt; i++)
        {
            rslt = bill_date_range_check(&npBill_date.npbill[i], oldt, newdt,
                                         chgdir);
            if (rslt != 0)
            {
                break;
            }
        }
    }

    DPRINTF(DBG_INFO, "%s: result[%d]\r\n", __func__, rslt);

    return rslt;
}

bool npBill_date_check(date_time_type *curdt, day_time_type *npday)
{
    bool wildcard;
    enum touDAY _curday;

    wildcard = true;

    _curday = (enum touDAY)calc_dayofweek(curdt);

    if (npday->dt.year != 0xff)
    {
        if (npday->dt.year != curdt->year)
            return false;

        wildcard = false;
    }

    if (npday->dt.month != 0xff)
    {
        if (npday->dt.month != curdt->month)
            return false;

        wildcard = false;
    }

    if (npday->dt.date != 0xff)
    {
        if (npday->dt.date != curdt->date)
            return false;

        wildcard = false;
    }

    if (wildcard && npday->day != 0xff)
    {
        if (npday->day != _curday)
            return false;
    }

    return cmp_time(&npday->dt, curdt) == 0 ? true : false;
}

static bool eob_get_npEOB(date_time_type *curdt, uint8_t *tptr)
{
    uint8_t i;

    tptr = 0;

    if (prog_npbill_available() && (npBill_date.cnt != 0))
    {
        for (i = 0; i < npBill_date.cnt; i++)
        {
            if (npBill_date_check(curdt, &npBill_date.npbill[i]))
                break;
        }

        if (i < npBill_date.cnt)
            return true;
    }

    return false;
}

int8_t tchg_is_within_peob(date_time_type *oldt, date_time_type *newdt)
{
    bool chgdir;
    day_time_type dayt;
    int8_t result;

    if (oldt->year == 0)
    {
        return 0;
    }

    dayt.dt.year = dayt.dt.month = 0xff;  // eBILL_EVERY_MONTH
    dayt.dt.date = reg_mr_date;
    dayt.dt.hour = dayt.dt.min = dayt.dt.sec = 0;

    chgdir = cmp_date_time(oldt, newdt) < 0
                 ? true
                 : false;  // oldt 보다 newdt 가 미래(반환 값이 -1)인 경우 true
    result = bill_date_range_check(&dayt, oldt, newdt, chgdir);

    DPRINTF(DBG_INFO, "%s: cmp_dt[%d], result[%d]\r\n", __func__, chgdir,
            result);

    return result;
}

/*
lhh_add_desc :
    1. RATE별 ch 별로  전력 및 수요 전력,  RollDM 을 NV에 write 한다.
    2. mr_data_info 를 NV에 write 한다.
*/
void eob_acc_nv_write(uint8_t eob_case, uint8_t i, uint8_t mrcnt)
{
    nv_sub_info.mr.sel = eMrAccm;
    nv_sub_info.mr.rt = i;
    nv_sub_info.mr.mrcnt = mrcnt;

    switch (eob_case)
    {
    case EOB_PERIOD_FLAG:
        nv_write(I_MT_READ_DATA, (uint8_t *)&mr_data_accm.accm[i]);
        break;
    case EOB_nPERIOD_FLAG:
        nv_write(I_MT_READ_DATA_nPRD, (uint8_t *)&mr_data_accm.accm[i]);
        break;
    case EOB_SEASON_FLAG:
        nv_write(I_MT_READ_DATA_SEASON, (uint8_t *)&mr_data_accm.accm[i]);
        break;
    }
}

#if 0 /* bccho, 2024-09-24, 삼상, delete */
/* bccho, 2024-09-05, 삼상 */
void eob_acc_ecdsa_nv_write(uint8_t eob_case, uint8_t i, uint8_t mrcnt)
{
    nv_sub_info.mr.sel = eMrAccmEcdsa;
    nv_sub_info.mr.dir = i;
    nv_sub_info.mr.mrcnt = mrcnt;

    switch (eob_case)
    {
    case EOB_PERIOD_FLAG:
        nv_write(I_MT_READ_DATA, (U8 *)&mr_data_accm.acc_ecdsa[i]);
        break;
    case EOB_nPERIOD_FLAG:
        nv_write(I_MT_READ_DATA_nPRD, (U8 *)&mr_data_accm.acc_ecdsa[i]);
        break;
    case EOB_SEASON_FLAG:
        nv_write(I_MT_READ_DATA_SEASON, (U8 *)&mr_data_accm.acc_ecdsa[i]);
        break;
    }
}

/* bccho, 2024-09-05, 삼상 */
void eob_dm_ecdsa_nv_write(uint8_t eob_case, uint8_t i, uint8_t mrcnt)
{
    nv_sub_info.mr.sel = eMrDmEcdsa;
    nv_sub_info.mr.dir = i;
    nv_sub_info.mr.mrcnt = mrcnt;

    switch (eob_case)
    {
    case EOB_PERIOD_FLAG:
        nv_write(I_MT_READ_DATA, (U8 *)&mr_data_dm.dm_ecdsa[i]);
        break;
    case EOB_nPERIOD_FLAG:
        nv_write(I_MT_READ_DATA_nPRD, (U8 *)&mr_data_dm.dm_ecdsa[i]);
        break;
    case EOB_SEASON_FLAG:
        nv_write(I_MT_READ_DATA_SEASON, (U8 *)&mr_data_dm.dm_ecdsa[i]);
        break;
    }
}
#endif

void eob_dm_nv_write(uint8_t eob_case, uint8_t i, uint8_t mrcnt)
{
    nv_sub_info.mr.sel = eMrDm;
    nv_sub_info.mr.rt = i;
    nv_sub_info.mr.mrcnt = mrcnt;

    switch (eob_case)
    {
    case EOB_PERIOD_FLAG:
        nv_write(I_MT_READ_DATA, (uint8_t *)&mr_data_dm.dm[i]);
        break;
    case EOB_nPERIOD_FLAG:
        nv_write(I_MT_READ_DATA_nPRD, (uint8_t *)&mr_data_dm.dm[i]);
        break;
    case EOB_SEASON_FLAG:
        nv_write(I_MT_READ_DATA_SEASON, (uint8_t *)&mr_data_dm.dm[i]);
        break;
    }
}

void eob_subblocks_nv_write(uint8_t eob_case, uint8_t i, uint8_t j,
                            uint8_t mrcnt, rolling_dm_ch_type *rollch)
{
    nv_sub_info.mr.rt = i;
    nv_sub_info.mr.sel = eMrSublocks;
    nv_sub_info.mr.chsel = j;
    nv_sub_info.mr.mrcnt = mrcnt;

    switch (eob_case)
    {
    case EOB_PERIOD_FLAG:
        nv_write(I_MT_READ_DATA, (uint8_t *)rollch);
        break;
    case EOB_nPERIOD_FLAG:
        nv_write(I_MT_READ_DATA_nPRD, (uint8_t *)rollch);
        break;
    case EOB_SEASON_FLAG:
        nv_write(I_MT_READ_DATA_SEASON, (uint8_t *)rollch);
        break;
    }
}

void eob_datainfo_nv_write(uint8_t eob_case, uint8_t mrcnt)
{
    nv_sub_info.mr.sel = eMrInfo;
    nv_sub_info.mr.mrcnt = mrcnt;

    switch (eob_case)
    {
    case EOB_PERIOD_FLAG:
        nv_write(I_MT_READ_DATA, (uint8_t *)&mr_data_info);
        break;
    case EOB_nPERIOD_FLAG:
        nv_write(I_MT_READ_DATA_nPRD, (uint8_t *)&mr_data_info);
        break;
    case EOB_SEASON_FLAG:
        nv_write(I_MT_READ_DATA_SEASON, (uint8_t *)&mr_data_info);
        break;
    }
}

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
/* bccho, 2024-09-05, 삼상 */
int16_t dsm_sec_signing_for_month_profile_acc_in(uint8_t eob_case, uint8_t gf,
                                                 uint8_t dir,
                                                 date_time_type *pdt,
                                                 uint8_t *tptr)
{
    uint8_t i;
    uint8_t t8;
    uint32_t t32;
    mr_ch_type *mr;
    // mr_data_accm_type *mr_all;
    uint8_t data_s[256];
    uint16_t data_cnt = 0;

    data_cnt += clock_to_format_for_sign(pdt, &data_s[data_cnt]);

    get_manuf_id(tptr);
    memcpy(&data_s[data_cnt], tptr, MANUF_ID_SIZE);
    data_cnt += MANUF_ID_SIZE;

    i = eTrate;
    do
    {
        if (!mr_prdnprdseason_from_nv(eob_case, eMrAccm, 98, gf, i, dir, tptr))
        {
            memset(tptr, 0, sizeof(mr_ch_type));
        }

        mr = (mr_ch_type *)tptr;

        if (dir == eDeliAct)
        {
            /* 수전 */
            data_cnt += u32_to_byte(mr->ch[eChDeliAct], &data_s[data_cnt]);
            data_cnt += u32_to_byte(mr->ch[eChDeliApp], &data_s[data_cnt]);
            data_cnt += u32_to_byte(mr->ch[eChDLagReact], &data_s[data_cnt]);
            data_cnt += u32_to_byte(mr->ch[eChDLeadReact], &data_s[data_cnt]);
            data_cnt += float_to_byte(mr->pf[eDeliAct], &data_s[data_cnt]);
        }

        else
        {
            /* 송전 */
            data_cnt += u32_to_byte(mr->ch[eChReceiAct], &data_s[data_cnt]);
            data_cnt += u32_to_byte(mr->ch[eChReceiApp], &data_s[data_cnt]);
            data_cnt += u32_to_byte(mr->ch[eChRLeadReact], &data_s[data_cnt]);
            data_cnt += u32_to_byte(mr->ch[eChRLagReact], &data_s[data_cnt]);
            data_cnt += float_to_byte(mr->pf[eReceiAct], &data_s[data_cnt]);
        }

        if (i == eDrate)
            break;  // exit
        if (++i >= numRates)
            i = 0;
    } while (1);

    if (dsm_sec_signing_for_month_profile(tptr, data_s, data_cnt))
    {
        memcpy((uint8_t *)&mr_data_accm.acc_ecdsa[dir], tptr, DLMS_DS_LEN);
    }
    else
    {
    }

    return TRUE;
}

/* bccho, 2024-09-05, 삼상 */
int16_t dsm_sec_signing_for_month_profile_dm_in(uint8_t eob_case, uint8_t gf,
                                                uint8_t dir,
                                                date_time_type *pdt,
                                                uint8_t *tptr)
{
    uint8_t i;
    uint8_t t8;
    uint32_t t32;
    mr_dm_type *mr;
    // mr_data_dm_type *mr_all;
    uint8_t data_s[256];
    uint16_t data_cnt = 0;

    data_cnt += clock_to_format_for_sign(pdt, &data_s[data_cnt]);

    get_manuf_id(tptr);
    memcpy(&data_s[data_cnt], tptr, MANUF_ID_SIZE);
    data_cnt += MANUF_ID_SIZE;

    i = eTrate;
    do
    {
        if (!mr_prdnprdseason_from_nv(eob_case, eMrDm, 98, gf, i, dir, tptr))
        {
            memset(tptr, 0, sizeof(mr_dm_type));
        }

        mr = (mr_dm_type *)tptr;

        if (dir == eDeliAct)
        {
            /* 수전 */

            data_cnt +=
                u32_to_byte(mr->mxdm[eDmChDeliAct].val, &data_s[data_cnt]);
            if (mr->mxdm[eDmChDeliAct].val)
            {
                data_cnt += clock_to_format_for_sign(&mr->mxdm[eDmChDeliAct].dt,
                                                     &data_s[data_cnt]);
            }
            else
            {
                memset(&data_s[data_cnt], 0xff, 12);
                data_cnt += 12;
            }
            data_cnt +=
                u32_to_byte(mr->cum_mxdm[eDmChDeliAct], &data_s[data_cnt]);
            data_cnt +=
                u32_to_byte(mr->mxdm[eDmChDeliApp].val, &data_s[data_cnt]);
            if (mr->mxdm[eDmChDeliApp].val)
            {
                data_cnt += clock_to_format_for_sign(&mr->mxdm[eDmChDeliApp].dt,
                                                     &data_s[data_cnt]);
            }
            else
            {
                memset(&data_s[data_cnt], 0xff, 12);
                data_cnt += 12;
            }
            data_cnt +=
                u32_to_byte(mr->cum_mxdm[eDmChDeliApp], &data_s[data_cnt]);
        }
        else
        {
            // Receivered active
            data_cnt +=
                u32_to_byte(mr->mxdm[eDmChReceiAct].val, &data_s[data_cnt]);
            if (mr->mxdm[eDmChReceiAct].val)
            {
                data_cnt += clock_to_format_for_sign(
                    &mr->mxdm[eDmChReceiAct].dt, &data_s[data_cnt]);
            }
            else
            {
                memset(&data_s[data_cnt], 0xff, 12);
                data_cnt += 12;
            }
            data_cnt +=
                u32_to_byte(mr->cum_mxdm[eDmChReceiAct], &data_s[data_cnt]);
            data_cnt +=
                u32_to_byte(mr->mxdm[eDmChReceiApp].val, &data_s[data_cnt]);
            if (mr->mxdm[eDmChReceiApp].val)
            {
                data_cnt += clock_to_format_for_sign(
                    &mr->mxdm[eDmChReceiApp].dt, &data_s[data_cnt]);
            }
            else
            {
                memset(&data_s[data_cnt], 0xff, 12);
                data_cnt += 12;
            }
            data_cnt +=
                u32_to_byte(mr->cum_mxdm[eDmChReceiApp], &data_s[data_cnt]);
        }

        if (i == eDrate)
        {
            break;
        }

        if (++i >= numRates)
        {
            i = 0;
        }
    } while (1);

    if (dsm_sec_signing_for_month_profile(tptr, data_s, data_cnt))
    {
        memcpy((uint8_t *)&mr_data_dm.dm_ecdsa[dir], tptr, DLMS_DS_LEN);
    }
    else
    {
    }
    return TRUE;
}
#endif

static void mr_reading(uint8_t eob_type, date_time_type *pdt, bool isdr,
                       uint8_t *tptr)
{
    uint8_t i;
    demand_ch_type j;
    rolling_dm_ch_type *rollch;

    mr_capture_accm(&mr_data_accm);
    mr_capture_dm(&mr_data_dm, isdr);

    if (eob_type & EOB_PERIOD_FLAG)
        mr_cnt += 1;
    if (eob_type & EOB_nPERIOD_FLAG)
        mrcnt_nprd += 1;
    if (eob_type & EOB_SEASON_FLAG)
        mrcnt_season_chg += 1;

    for (i = eArate; i < numRates; i++)
    {
        if (eob_type & EOB_PERIOD_FLAG)
        {
            eob_acc_nv_write(EOB_PERIOD_FLAG, i, mr_cnt);
            eob_dm_nv_write(EOB_PERIOD_FLAG, i, mr_cnt);
        }
        if (eob_type & EOB_nPERIOD_FLAG)
        {
            eob_acc_nv_write(EOB_nPERIOD_FLAG, i, mrcnt_nprd);
            eob_dm_nv_write(EOB_nPERIOD_FLAG, i, mrcnt_nprd);
        }
        if (eob_type & EOB_SEASON_FLAG)
        {
            eob_acc_nv_write(EOB_SEASON_FLAG, i, mrcnt_season_chg);
            eob_dm_nv_write(EOB_SEASON_FLAG, i, mrcnt_season_chg);
        }
    }

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
    for (i = eDeliAct; i < numDirChs; i++)
    {
        if (eob_type & EOB_PERIOD_FLAG)
        {
            dsm_sec_signing_for_month_profile_acc_in(PERIOD_GRP_B, 101, i, pdt,
                                                     tptr);
            eob_acc_ecdsa_nv_write(EOB_PERIOD_FLAG, i, mr_cnt);
            dsm_sec_signing_for_month_profile_dm_in(PERIOD_GRP_B, 101, i, pdt,
                                                    tptr);
            eob_dm_ecdsa_nv_write(EOB_PERIOD_FLAG, i, mr_cnt);
        }
        if (eob_type & EOB_nPERIOD_FLAG)
        {
            dsm_sec_signing_for_month_profile_acc_in(nPERIOD_GRP_B, 101, i, pdt,
                                                     tptr);
            eob_acc_ecdsa_nv_write(EOB_nPERIOD_FLAG, i, mrcnt_nprd);
            dsm_sec_signing_for_month_profile_dm_in(nPERIOD_GRP_B, 101, i, pdt,
                                                    tptr);
            eob_dm_ecdsa_nv_write(EOB_nPERIOD_FLAG, i, mrcnt_nprd);
        }
        if (eob_type & EOB_SEASON_FLAG)
        {
            dsm_sec_signing_for_month_profile_acc_in(SEASON_GRP_B, 101, i, pdt,
                                                     tptr);
            eob_acc_ecdsa_nv_write(EOB_SEASON_FLAG, i, mrcnt_season_chg);
            dsm_sec_signing_for_month_profile_dm_in(SEASON_GRP_B, 101, i, pdt,
                                                    tptr);
            eob_dm_ecdsa_nv_write(EOB_SEASON_FLAG, i, mrcnt_season_chg);
        }
    }
#else
    rollch = (rolling_dm_ch_type *)tptr;
    for (i = eArate; i < numRates; i++)
    {
        for (j = 0; j < numDmCHs; j++)
        {
            mr_capture_rolldm(i, j, rollch);

            if (eob_type & EOB_PERIOD_FLAG)
            {
                eob_subblocks_nv_write(EOB_PERIOD_FLAG, i, j, mr_cnt, rollch);
            }
            if (eob_type & EOB_nPERIOD_FLAG)
            {
                eob_subblocks_nv_write(EOB_nPERIOD_FLAG, i, j, mrcnt_nprd,
                                       rollch);
            }
            if (eob_type & EOB_SEASON_FLAG)
            {
                eob_subblocks_nv_write(EOB_SEASON_FLAG, i, j, mrcnt_season_chg,
                                       rollch);
            }
        }
    }
#endif

    nv_sub_info.mr.mrcnt = mr_cnt;

    mr_dt = *pdt;
    mr_mtdir = (mt_dir >= mtdir_history) ? mt_dir : mtdir_history;
    mr_rtkind = (mt_rtkind >= rtkind_history) ? mt_rtkind : rtkind_history;
    mr_selreact = mt_selreact;

    if (eob_type & EOB_PERIOD_FLAG)
    {
        sr_dt_prd = mr_dt;
    }
    if (eob_type & EOB_nPERIOD_FLAG)
    {
        sr_dt_nprd = mr_dt;
    }
    if (eob_type & EOB_SEASON_FLAG)
    {
        sr_dt_sch = mr_dt;
    }

    var_history_init();

    if (eob_type & EOB_PERIOD_FLAG)
    {
        eob_datainfo_nv_write(EOB_PERIOD_FLAG, mr_cnt);
    }
    if (eob_type & EOB_nPERIOD_FLAG)
    {
        eob_datainfo_nv_write(EOB_nPERIOD_FLAG, mrcnt_nprd);
    }
    if (eob_type & EOB_SEASON_FLAG)
    {
        eob_datainfo_nv_write(EOB_SEASON_FLAG, mrcnt_season_chg);
    }
}

static float calc_diff_pf(uint32_t *ch1, uint32_t *ch2, energy_dir_type dir)
{
    uint32_t act;
    uint32_t rea;
    uint32_t ch[numCHs];
    float ret_pf;

    accm_ch_diff(ch, ch1, ch2);

    if (dir == eDeliAct)
    {
        act = ch[eChDeliAct];
    }
    else
    {
        act = ch[eChReceiAct];
    }
    rea = get_sel_react32(dir, ch);

    ret_pf = calc_pf(act, rea);

    return ret_pf;
}

/*
선택 무호
*/
static void sel_react_monitor(date_time_type *dt)
{
    if (dt->year == sel_react_yr && dt->month == sel_react_mon)
    {
        mt_selreact = sel_react_sel;

        lp_event_set(LPE_PROGRAM_CHG);
        sel_react_cancel();
    }
}

/*
lhh_add_desc : 선택 무효 판별 및 LP 관련 event 설정
*/
void sel_react_monitor_rtchg(date_time_type *dt)
{
    if (sel_react_yr != 0 && sel_react_mon != 0)
    {
        if (dt->year < sel_react_yr)
            return;

        if (dt->year > sel_react_yr)
            goto sel_react_monitor_rtchg1;

        if (dt->month >= sel_react_mon)
        {
        sel_react_monitor_rtchg1:
            mt_selreact = sel_react_sel;

            lp_event_set(LPE_PROGRAM_CHG);
            sel_react_cancel();
        }
    }
}
