#include "options.h"
#include "port.h"
#include "eoi.h"
#include "nv.h"

extern bool init_mif_task_set;
max_demand_type max_dm;
date_time_type dr_dt;
static date_time_type cur_dm_dt;

uint8_t dm_period_num;
rolling_dm_type rolling_dm;

int eoi_deactive_timer;
bool b_eoi_deactive = false;
bool mxdm_processed;

extern float PULSE_ADD_MODIFY_DATA, PULSE_ADD_MODIFY_DATA_VA,
    PULSE_ADD_MODIFY_DATA_VAR;
extern bool METER_FW_UP_ING_STS;
extern bool METER_FW_UP_END_PULSE_MODIFY;
extern U16 METER_FW_UP_ING_CNT;
extern bool init_mif_task_firm_up;
extern bool meter_firmup_delay_ing;
extern ST_MIF_METER_PARM g_mtp_meter_parm;

static void maxdem_proc(uint8_t intv, bool force, bool rtchg, rate_type rt,
                        date_time_type *pdt, uint8_t *tptr);
static void fill_eoi_ch(uint32_t *eoich, uint8_t intv);
static void dm_intv_reset(uint8_t intv_bound, bool rtchg);
static void eoi_pulse_set(void);

static const uint8_t dm_interval_r[NUM_DM_INTV] = {1, 5, 10, 15, 30, 60};

void eoi_init(void)
{
    b_eoi_deactive = false;
    mxdm_processed = false;
    dm_period_num = 1;

    dr_dt_init();

    if (!nv_read(I_MAX_DEMAND, (uint8_t *)&max_dm))
        memset((uint8_t *)&max_dm, 0, sizeof(max_demand_type));
}

void dr_dt_init(void) { dr_dt.year = 0xff; }

/*
lhh_add_desc :
1. max demand 를 업데이트 한다.
2. eoi pulse 를 위한 timer 를 설정한다.
*/
void eoi_proc(uint8_t *tptr)
{
    bool rtchg;
    date_time_type dt;

    if (eoi_lastdt.month == 0)
        eoi_lastdt = cur_rtc;

    if (eoi_rate >= numRates)
        eoi_rate = cur_rate;

    dt = eoi_lastdt;
    get_next_interval_boundary(&dt, dm_sub_interval);

    if ((cmp_date_time(&cur_rtc, &dt) < 0) &&
        (cmp_date_time(&cur_rtc, &eoi_lastdt) >= 0))
    {
        mxdm_processed = false;
        eoi_processed = 0;
        return;
    }

    rtchg = (bool)(eoi_rate != cur_rate);
    maxdem_proc(dm_interval, false, rtchg, eoi_rate, &dt, tptr);

    if (eoi_evt & (EOI_EVENT_INTV_CHG | EOI_EVENT_SUBINTV_CHG))
    {
        // 이벤트 적용을 maxdem_proc() 이후 함
        eoi_parm_set(eoi_evt);
    }

    eoi_rate = cur_rate;
    eoi_evt = 0;
    eoi_lastdt = cur_rtc;
    eoi_processed = 1;

    eoi_pulse_set();
}

void eoi_proc_pwrtn(pwrfail_info_type *pfinfo, int32_t dur, uint8_t *tptr)
{
    bool rtchg;
    date_time_type dt1;
    recent_demand_type *dm;
    int i;
    uint8_t pf_cnt;
    date_time_type dt2;

    dm = (recent_demand_type *)tptr;
    tptr += sizeof(recent_demand_type);

    dt1 = pfinfo->dt;

    rtchg = (eoi_rate != cur_rate);
    if (dur < 0 || rtchg)
    {
        maxdem_proc(dm_interval, false, rtchg, eoi_rate, &dt1, tptr);
        eoi_rate = cur_rate;
        eoi_lastdt = cur_rtc;
        eoi_processed = 1;
        cur_dmdt_set(&cur_rtc);
        return;
    }

    get_interval_boundary(&dt1, dm_sub_interval);
    if (cmp_date_time(&cur_rtc, &dt1) < 0)
        goto eoi_proc_pwrtn1;

    dt2 = cur_rtc;
    get_interval_boundary(&dt2, dm_sub_interval);
    pf_cnt = calc_date_time_diff(&dt2, &dt1) / (dm_sub_interval * 60);
    rtchg = (eoi_rate != cur_rate);
    if (rtchg)
    {
        maxdem_proc(dm_interval, false, true, eoi_rate, &pfinfo->dt, tptr);
    }
    else
    {
        if (pf_cnt > dm_period_num)
            pf_cnt = dm_period_num;

        dt2 = pfinfo->dt;
        for (i = 0; i < pf_cnt; i++)
        {
            get_next_interval_boundary(&dt2, dm_sub_interval);
            maxdem_proc(dm_interval, false, rtchg, eoi_rate, &dt2, tptr);
        }
    }

    eoi_rate = cur_rate;
    eoi_lastdt = cur_rtc;
    eoi_processed = 1;

    if (mxdm_processed)
    {
        cur_dmdt_set(&cur_rtc);
    }
    else
    {
    eoi_proc_pwrtn1:
        if (nv_read(I_RCNT_DEMAND, (uint8_t *)dm))
        {
            if (dm->dt.month >= 1 && dm->dt.month <= 12)
            {
                cur_dmdt_set(&dm->dt);
            }
            else
            {
                set_start_time(&cur_dm_dt);
            }
        }
        else
        {
            set_start_time(&cur_dm_dt);
        }
    }
}

void eoi_proc_dr(date_time_type *pdt, bool isdr, uint8_t *tptr)
{
    date_time_type dt;

    if (eoi_processed == 0)
    {
        dt = *pdt;
        get_interval_boundary(&dt, dm_sub_interval);
        eoi_pulse_set();
    }
    else
    {
        dt = *pdt;
    }

    if (mxdm_processed == false)
    {
        maxdem_proc(dm_interval, true, false, eoi_rate, &dt, tptr);
    }

    if (isdr)
    {
        dm_intv_init();
    }

    eoi_rate = cur_rate;
    eoi_lastdt = *pdt;
    eoi_processed = 1;
}

void eoi_proc_ratechg(rate_type rt, uint8_t *tptr)
{
    date_time_type dt;

    dt = cur_rtc;
    get_interval_boundary(&dt, dm_sub_interval);
    maxdem_proc(dm_interval, false, true, rt, &dt, tptr);
    eoi_pulse_set();

    eoi_rate = cur_rate;
    eoi_lastdt = cur_rtc;
    eoi_processed = 1;
}

void eoi_proc_timechg(date_time_type *bfdt, rate_type rt, uint8_t *tptr,
                      bool fut_exe)
{
    bool rtchg;
    date_time_type dt1;
    date_time_type dt2;
    date_time_type dt3 = *bfdt;

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    dt1 = *bfdt;
    dt2 = *bfdt;

    get_bf_interval_boundary(&dt1, dm_sub_interval);
    get_interval_boundary(&dt2, dm_sub_interval);

    if (fut_exe == true && (cmp_date_time(&cur_rtc, &dt1) < 0 ||
                            cmp_date_time(&cur_rtc, &dt2) >= 0))
    {
        rtchg = (rt != cur_rate);
        maxdem_proc(dm_interval, false, rtchg, rt, &dt2, tptr);
        eoi_pulse_set();
        eoi_rate = cur_rate;
        eoi_lastdt = cur_rtc;
        eoi_processed = 1;
    }
    else if (fut_exe == false && (cmp_date_time(&cur_rtc, &dt1) < 0 ||
                                  cmp_date_time(&cur_rtc, &dt2) >= 0 ||
                                  cmp_date_time(&cur_rtc, &dt3) < 0))
    {
        rtchg = (rt != cur_rate);
        maxdem_proc(dm_interval, false, rtchg, rt, &dt2, tptr);
        eoi_pulse_set();
        eoi_rate = cur_rate;
        eoi_lastdt = cur_rtc;
        eoi_processed = 1;
    }
}

void dm_reset(date_time_type *pdt)
{
    uint8_t i, j;

    DPRINTF(DBG_INFO, "Demand Reset\r\n");

    dr_dt = *pdt;

    for (i = 0; i < numRates; i++)
    {
        for (j = 0; j < numDmCHs; j++)
        {
            max_dm.max[i].dm[j].val = 0L;
        }
    }

    nv_write(I_MAX_DEMAND, (uint8_t *)&max_dm);
    dm_intv_init();

    if (run_is_bat_power())
    {
        dm_intv_save();
    }
}

uint8_t get_dm_interval(uint8_t idx) { return dm_interval_r[idx]; }

uint8_t get_dmintv_index(uint8_t intv)
{
    uint8_t i;

    for (i = 0; i < NUM_DM_INTV; i++)
    {
        if (dm_interval_r[i] == intv)
            break;
    }

    return i;
}

bool is_dm_intv_valid(uint8_t intv)
{
    if (get_dmintv_index(intv) < NUM_DM_INTV)
        return true;

    return false;
}

uint32_t get_cur_mxdm(rate_type rt, demand_ch_type ch)
{
    return max_dm.max[rt].dm[ch].val;
}

void get_cur_mxtime(date_time_type *dt, rate_type rt, demand_ch_type ch)
{
    if (max_dm.max[rt].dm[ch].val)
    {
        *dt = max_dm.max[rt].dm[ch].dt;
    }
    else
    {
        memset((uint8_t *)dt, 0, sizeof(date_time_type));
    }
}

uint32_t get_cur_cumdm(rate_type rt, demand_ch_type ch)
{
    return (get_cur_mxdm(rt, ch) + mr_data_dm.dm[rt].cum_mxdm[ch]) %
           mxdm_dgt_cnt;
}

void get_cur_dmdt(date_time_type *pdt) { *pdt = cur_dm_dt; }

void cur_dmdt_set(date_time_type *pdt) { cur_dm_dt = *pdt; }

void dm_intv_init(void)
{
    int i;

    dm_subintv_cnt = 0;
    dm_subintv_idx = 0;

    for (i = 0; i < numDmCHs; i++)
    {
        dm_subintv(i) = 0L;
    }
}

void dm_intv_load(void)
{
    if (!nv_read(I_DM_INTV_DATA, (uint8_t *)&rolling_dm))
    {
        dm_intv_init();
    }
}

void dm_intv_nvdelete(uint8_t *tptr)
{
    rolling_dm_type *rollingdm;
    rollingdm = (rolling_dm_type *)tptr;
    memset((uint8_t *)rollingdm, 0, sizeof(rolling_dm_type));
    nv_write(I_DM_INTV_DATA, (uint8_t *)rollingdm);
}

void dm_intv_save(void) { nv_write(I_DM_INTV_DATA, (uint8_t *)&rolling_dm); }

void eoi_parm_set(uint8_t evt)
{
    if (evt & EOI_EVENT_INTV_CHG)
    {
        dm_sub_interval = eoi_intv;
    }

    if (evt & EOI_EVENT_SUBINTV_CHG)
    {
        dm_period_num = 1;
    }

    dm_interval = dm_sub_interval * dm_period_num;
}

void fill_rcnt_ch(uint32_t *eoich, uint8_t intv) { fill_eoi_ch(eoich, intv); }

uint32_t get_curr_rcnt_ch(bool rcnt, demand_ch_type ch, uint8_t *tptr)
{
    uint32_t eoich[numDmCHs];
    recent_demand_type *rcntdm;

    if (rcnt == 0)
    {
        fill_rcnt_ch(eoich, dm_interval);
        return eoich[ch];
    }

    rcntdm = (recent_demand_type *)tptr;

    // recent demand
    if (!nv_read(I_RCNT_DEMAND, (uint8_t *)rcntdm))
        return 0L;

    return rcntdm->dm[ch];
}

uint8_t get_rcnt_pf(energy_dir_type dir)
{
    return (lp_intv_pf[dir] != -1.0) ? (uint8_t)(lp_intv_pf[dir] * 100.0)
                                     : 0xff;
}

static void fill_eoi_ch(uint32_t *eoich, uint8_t intv)
{
    uint8_t eoinum;
    int i, j, k;
    int kwnum;

    kwnum = 60 / intv;

    eoinum = dm_subintv_cnt + 1;
    if (eoinum > dm_period_num)
        eoinum = dm_period_num;

    for (i = 0; i < numDmCHs; i++)
    {
        eoich[i] = 0L;

        k = rolling_dm.sublock_idx;
        for (j = 0; j < eoinum; j++)
        {
            eoich[i] += rolling_dm.sublock_dm[i][k];
            if (k == 0)
                k = MAX_ROLLING_NUM;
            k -= 1;
        }

        eoich[i] *= kwnum;
    }
}

void get_sublocks_data(demand_ch_type dmch, rolling_dm_ch_type *rollch)
{
    uint8_t i, j, cnt;

    cnt = dm_subintv_cnt + 1;

    cnt = (cnt > dm_period_num) ? dm_period_num : cnt;
    j = dm_subintv_idx;
    rollch->cnt = cnt;

    for (i = 0; i < cnt; i++)
    {
        rollch->sublock[i] = rolling_dm.sublock_dm[dmch][j];
        if (j == 0)
            j = MAX_ROLLING_NUM;
        j -= 1;
    }
}

static void maxdem_proc(uint8_t intv, bool force, bool rtchg, rate_type rt,
                        date_time_type *pdt, uint8_t *tptr)
{
    uint8_t i;
    bool b_updated;
    uint32_t eoich[numDmCHs];
    recent_demand_type *rcnt;
    rolling_dm_ch_type *rollch;

    b_updated = false;

    //---------- max demand --------------

    if (!force && !rtchg)
    {
        if ((dm_subintv_cnt + 1) < dm_period_num)
        {
            goto mxdm_intv_update;
        }
    }

    fill_eoi_ch(eoich, intv);

    rollch = (rolling_dm_ch_type *)tptr;
    for (i = 0; i < numDmCHs; i++)
    {
        get_sublocks_data(i, rollch);

        if (eoich[i] > max_dm.max[rt].dm[i].val)
        {
            b_updated = true;

            max_dm.max[rt].dm[i].val = eoich[i];
            max_dm.max[rt].dm[i].dt = *pdt;

            nv_sub_info.cur.rt = rt;
            nv_sub_info.cur.chsel = i;
            nv_write(I_DM_SUBLOCKS_DATA, (uint8_t *)rollch);
        }

        if (eoich[i] > max_dm.max[eTrate].dm[i].val)
        {
            b_updated = true;

            max_dm.max[eTrate].dm[i].val = eoich[i];
            max_dm.max[eTrate].dm[i].dt = *pdt;

            nv_sub_info.cur.rt = eTrate;
            nv_sub_info.cur.chsel = i;
            nv_write(I_DM_SUBLOCKS_DATA, (uint8_t *)rollch);
        }
    }

    if (b_updated)
    {
        nv_write(I_MAX_DEMAND, (uint8_t *)&max_dm);
    }

    mxdm_processed = true;

    //---------- recent demand --------------

    rcnt = (recent_demand_type *)tptr;

    for (i = 0; i < numDmCHs; i++)
    {
        rcnt->dm[i] = eoich[i];
    }

    rcnt->dt = *pdt;

    cur_dmdt_set(pdt);  // current demand time is same as recent

    dsp_rcntdm_modified_set();
    inc_rcntdm_wear_idx();
    DPRINTF(DBG_NONE, "MaxDemand: Recent Demand\r\n");
    nv_write(I_RCNT_DEMAND, (uint8_t *)rcnt);

mxdm_intv_update:
    if (eoi_processed == 0)
    {
        // demand is reset
        dm_intv_reset(pdt->min % intv, rtchg);
    }
}

static void dm_intv_reset(uint8_t intv_bound, bool rtchg)
{
    int i;

    if (rtchg)
    {
        dm_intv_init();
    }
    else
    {
        if (dm_subintv_cnt < dm_period_num)
        {
            dm_subintv_cnt += 1;
        }

        if ((++dm_subintv_idx) >= MAX_ROLLING_NUM)
            dm_subintv_idx = 0;

        for (i = 0; i < numDmCHs; i++)
        {
            dm_subintv(i) = 0L;
        }
    }
}

static void eoi_pulse_set(void)
{
    if (METER_FW_UP_ING_STS || METER_FW_UP_END_PULSE_MODIFY ||
        init_mif_task_firm_up || meter_firmup_delay_ing)
    {
        // meter_fw_up 증에는 다른 컴멤드 일체금지
    }
    else
    {
        if (g_mtp_meter_parm.pulse_select != 1)
        {
            void dsm_mtp_fsm_send(void);
            dsm_mtp_set_op_mode(MTP_OP_NORMAL);
            dsm_mtp_set_fsm(MTP_FSM_EOI_PULSE_ST);
            dsm_mtp_fsm_send();
        }
    }

    switch (err_pulse_react)
    {
    case 0:  // apparent + EOI
        /* EOI1 LED ON : 유효/EOI 펄스 LED */
        DPRINTF(DBG_TRACE, "EOI Pulse[%d]: EOI 1\r\n", err_pulse_react);
        EOI_VPORT_DISABLE;  // eoi disable, v pulse out
        EOI_WPORT_ACTIVE;   // eoi enable, pulse off, led on
        eoi_deactive_timer = T180MS;
        b_eoi_deactive = 1;
        break;

    case 1:  // wpulse + varpulse
        DPRINTF(DBG_TRACE, "EOI Pulse[%d]: NOP\r\n", err_pulse_react);
        EOI_WPORT_DISABLE;  // eoi disable, w pulse out
        EOI_VPORT_DISABLE;  // eoi disable, v pulse out
        break;

    case 2:  // EOI + wpulse
        /* EOI2 LED ON : 무효/피상/EOI 펄스 LED */
        DPRINTF(DBG_TRACE, "EOI Pulse[%d]: EOI 2\r\n", err_pulse_react);
        EOI_WPORT_DISABLE;  // eoi disable, w pulse out
        EOI_VPORT_ACTIVE;   // eoi enable, pulse off, led on
        eoi_deactive_timer = T180MS;
        b_eoi_deactive = 1;
        break;
    }
}

void eoi_or_pulse_select(void)
{
    switch (err_pulse_react)
    {
    case 0:  // apparent + EOI
             /* EOI1 LED ON : 유효/EOI 펄스 LED */
        DPRINTF(DBG_TRACE, "EOI Pulse[%d]: EOI 1\r\n", err_pulse_react);
        EOI_VPORT_DISABLE;     // eoi disable, v pulse out
        EOI_WPORT_DEACTIVE;    // LED OFF
        PULSE_WPORT_DEACTIVE;  // w pulse_oFF, EOI W SELECT
        break;

    case 1:  // wpulse + varpulse
        DPRINTF(DBG_TRACE, "EOI Pulse[%d]: NOP\r\n", err_pulse_react);
        EOI_WPORT_DISABLE;  // eoi disable, w pulse out
        EOI_VPORT_DISABLE;  // eoi disable, v pulse out
        break;

    case 2:  // EOI + wpulse
             /* EOI2 LED ON : 무효/피상/EOI 펄스 LED */
        DPRINTF(DBG_TRACE, "EOI Pulse[%d]: EOI 2\r\n", err_pulse_react);
        EOI_WPORT_DISABLE;     // eoi disable, v pulse out
        EOI_VPORT_DEACTIVE;    // LED OFF
        PULSE_VPORT_DEACTIVE;  // v pulse_oFF,EOI VAR SELECT
        break;
    }
}