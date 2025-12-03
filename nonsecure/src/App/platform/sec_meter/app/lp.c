#include "main.h"
#include "options.h"
#include "meter_app.h"
#include "lp.h"
#include "nv.h"
#include "get_req.h"
#include "amg_push_datanoti.h"
#include "amg_wdt.h"
#include "whm.h"
#include "amg_imagetransfer.h"

#define _D "[LP] "

extern bool bat_rtc_backuped, eob_pwrtn;
extern date_time_type pwrtn_EOB_dt;

bool b_lpavg_ready = false;
bool b_lpavg_monitor;
bool b_lp_pf_monitor;
bool b_lprt_ready;
bool b_lprt_monitor;
float lpavg_volt[4];
float lpavg_volt_ltol[4];

uint8_t lpindex_while_lpblocked;
void lp_record_write_while_blocked(lp_record_type *lp_rec);

static void lp_event_bits_setup(void);
static void lp_event_bits_adj(void);
static void lp_record_write(lp_record_type *lp_rec, uint32_t idx);
void dsp_rcntpf_modified_set(void);
extern void dsm_data_noti_lastLP_evt_send(void);
extern uint32_t dsm_get_dm_out_measure_print_chkcount(void);

float get_lpavg_ltol(U8 line);

void LP_init(void)
{
    b_lprt_monitor = false;
    b_lpavg_monitor = false;
    b_lp_pf_monitor = FALSE;
    lp_event_unset(LPE_VOL_LOW);  // jp.kim 24.11.30
}

void LP_proc(void)
{
    date_time_type lpdt;
    uint32_t trgevt;

    if (LP_nextdt.month == 0)
    {
        LP_nextdt_set(&cur_rtc);
        return;
    }

    if (eob_pwrtn)
        lpdt = pwrtn_EOB_dt;
    else
        lpdt = cur_rtc;

    eob_pwrtn = 0;

    if (LP_event & LPE_PWR_FAIL)
    {
#if 0
		lp_event_unset(LPE_SAG_SWELL);
#endif
        trgevt = LP_event;
        LP_nextdt_set(&cur_rtc);
        trgevt &= ~(LPE_PWR_FAIL | LPE_NO_BAT);

        LP_event = trgevt;
        lp_event_bits_setup();
    }

    if (cmp_date_time(&cur_rtc, &LP_nextdt) >= 0)
    {
        lp_event_unset(LPE_IRRG_WR);

        LP_save(&lpdt);
        LP_nextdt_set(&cur_rtc);
        DPRINTF(DBG_TRACE, "%s: push_is_enable[%d]\r\n", __func__,
                dsm_push_is_enable(PUSH_SCRIPT_ID_LAST_LP));
        if (dsm_push_is_enable(PUSH_SCRIPT_ID_LAST_LP))
        {
            dsm_data_noti_lastLP_evt_send();
        }
    }
    else if (lp_save_is_triged())
    {
        lp_event_set(LPE_IRRG_WR);

        LP_save(&lpdt);
        LP_nextdt_set(&cur_rtc);
    }
    else
    {
        lp_event_bits_setup();
    }
}

void LP_nextdt_set(date_time_type *pdt)
{
    date_time_type dt;

    dt = *pdt;
    get_next_interval_boundary(&dt, lp_interval);
    LP_nextdt = dt;
}

void pwrtn_LP_proc(pwrfail_info_type *pfinfo)
{
    LP_event |= (LPE_IRRG_WR | LPE_PWR_FAIL);

    if (dsm_swrst_is_bootflag())
    {
        LP_nextdt_set(&cur_rtc);
#if 1  // jp.kim 24.12.13  //펌업시 재 부팅후 lp_event relay 상태 유지
        uint32_t LP_event_back = (LP_event & (LPE_LATCH_OFF | LPE_PROGRAM_CHG));
#endif
        lp_event_clear();
#if 1  // jp.kim 24.12.13
        LP_event |= LP_event_back;
#endif
        DPRINTF(DBG_ERR, "%s: sysfwup_bool\r\n", __func__);
    }
    else
    {
        lp_event_unset(LPE_VOL_LOW);  // JP.KIM 24.11.30
        LP_save(&pfinfo->dt);
        LP_nextdt_set(&cur_rtc);
    }

    if (relay_is_load_on())
    {
        lp_event_set(LPE_LATCH_ON);
    }
    else
    {
        lp_event_set(LPE_LATCH_OFF);
    }
}

void LP_read(uint8_t *cp, uint32_t idx, uint8_t len, uint8_t mtdir)
{
    uint8_t len1, recsize;
    uint16_t bufidx;

    recsize = recsize_for_lp;
    bufidx = (uint16_t)(idx % buffsize_for_lp);

    bufidx = buffsize_for_lp - 1 - bufidx;

    if ((bufidx + len) > buffsize_for_lp)
    {
        len1 = buffsize_for_lp - bufidx;

        nv_sub_info.rel.idx = bufidx;
        nv_sub_info.rel.len = len1;
        nv_sub_info.rel.rec_siz = recsize;
        if (!nv_read(I_LP_DATA, cp))
            memset(cp, 0x00, (uint16_t)(len1 * recsize));

        cp += (uint16_t)(len1 * recsize);

        bufidx = 0;
        len -= len1;
    }

    nv_sub_info.rel.idx = bufidx;
    nv_sub_info.rel.len = len;
    nv_sub_info.rel.rec_siz = recsize;
    if (!nv_read(I_LP_DATA, cp))
    {
        memset(cp, 0x00, (uint16_t)(len * recsize));
    }
}

/*
 보안 일반 규격 : LP 기능 [200417_spec]
계량항목 8채널에 대한 LP 자료 기록
LP 기록 시 발생일시를 함께 기록
이벤트 발생 시, 발생 시점 LP 주기에 이벤트를 기록하여 LP를 저장
*/
/*
lhh_add_desc :
    1. date and time 압축
    2. eTrate 에 해당하는 ch 별로 축적된 전력 값을 lp_rec.ch 로 get 한다.
    3. LP 를 NV 에 write 한다.
    4. LP event clear 하고 재 설정 한다.
    5. 역률 값 을 업데이트 한다. (display or dlms 에서 사용함. )
*/
void LP_save(date_time_type *dt)
{
    lp_record_type lp_rec = {0};

    if (((dt->min % lp_interval) == 0) && (dt->sec == 0))
    {
        lp_event_unset(LPE_IRRG_WR);
    }

    // event
#if 1
    if (LP_event & LPE_PWR_FAIL)
    {
        lp_event_unset(LPE_SAG_SWELL);
    }
#endif
    lp_rec.evt[0] = (uint8_t)((LP_event >> 16) & 0xff);  // Byte 2
    lp_rec.evt[1] = (uint8_t)((LP_event >> 8) & 0xff);   // Byte 1
    lp_rec.evt[2] = (uint8_t)(LP_event & 0xff);          // Byte 0

    TOTAL_LP_EVENT_CNT += 1L;
    lp_rec.lp_cnt = TOTAL_LP_EVENT_CNT;
    // time
    compress_time(lp_rec.dt, dt);

    // channel
    get_accm_chs(eTrate, lp_rec.ch);

    if (!lp_mtdir_is_chged_and_blocked())
    {
        lp_record_write(&lp_rec, LP_index);
        LP_index += 1L;
        DPRINTF(DBG_TRACE, _D "%s: TOTAL_LP_EVENT_CNT[%d] LP_index[%d]\r\n",
                __func__, TOTAL_LP_EVENT_CNT, LP_index);
    }
    else
    {
        lp_record_write_while_blocked(&lp_rec);
    }

    lp_event_clear();
    lp_event_bits_setup();
    lp_event_bits_adj();

    update_lp_intv_pf();
}

void lp_save_batmode(uint32_t lpe)
{
    uint32_t t32;

    t32 = LP_event;
    LP_event = lpe;
    LP_save(&cur_rtc);

    LP_event = t32;
}

void lp_clear(void)
{
    uint32_t t32;

    LP_index = 0L;
    TOTAL_LP_EVENT_CNT = 0L;
    t32 = LP_event & LPE_TRIG;  // 정전 LP event 삭제 함
    lp_event_clear();
    lp_event_bits_setup();
    LP_event |= t32;
}

void lp_save_manual(void)
{
    uint8_t chlen;
    uint16_t i, k, t16;
    date_time_type dt;

    dt = cur_rtc;

    LP_index = 0L;
    TOTAL_LP_EVENT_CNT = 0L;
    chlen = numCHs;

    for (k = 0; k < numRates; k++)
    {
        for (i = 0; i < chlen; i++)
        {
            ACC_CH(k, i) = mxaccm_dgt_cnt - 10;
        }
    }

    t16 = LP_SIZE;
    for (i = 0; i < t16; i++)
    {
        vTaskDelay(2);

        dsm_wdt_ext_toggle_immd();

        LP_save(&dt);
        time_up(&dt, 1);
    }

    nv_write(I_MT_ACCM, (uint8_t *)&mt_accm);
    nv_write(I_WHM_OP, (uint8_t *)&whm_op);
}

void expand_time(date_time_type *dt, uint8_t *cdt)
{
    dt->year = cdt[0] & 0x3f;
    dt->month = (cdt[0] & 0xc0) >> 4;
    dt->month |= cdt[1] >> 6;
    dt->min = cdt[1] & 0x3f;
    dt->date = (cdt[2] & 0xe0) >> 3;
    dt->date |= cdt[3] >> 6;
    dt->hour = cdt[2] & 0x1f;
    dt->sec = cdt[3] & 0x3f;
}

void lp_event_set_curate(void)
{
    uint32_t t32;

    lp_event_unset(LPE_TARIFF);
    t32 = (uint32_t)cur_rate << 8;
    lp_event_set(t32);
}

bool lp_last_record_dt(date_time_type *pdt, uint8_t *tptr)
{
    uint8_t recsize;
    uint16_t bufidx;
    lp_record_type *lp;

    DPRINTF(DBG_TRACE, _D "%s: LP_index[%d]\r\n", __func__, LP_index);

    if (LP_index == 0L)
        return false;

    recsize = sizeof(lp_record_type);
    bufidx = (uint16_t)((LP_index - 1) % LP_BUF_SIZE);
    bufidx = LP_BUF_SIZE - 1 - bufidx;

    nv_sub_info.rel.idx = bufidx;
    nv_sub_info.rel.len = 1;
    nv_sub_info.rel.rec_siz = recsize;
    if (!nv_read(I_LP_DATA, tptr))
        return false;

    lp = (lp_record_type *)tptr;
    expand_time(pdt, &lp->dt[0]);

    return true;
}

bool lp_last_record_data(uint8_t *tptr)
{
    uint8_t recsize;
    uint16_t bufidx;

    DPRINTF(DBG_TRACE, _D "%s: LP_index[%d]\r\n", __func__, LP_index);

    if (LP_index == 0L)
        return false;

    recsize = sizeof(lp_record_type);
    bufidx = (uint16_t)((LP_index - 1) % LP_BUF_SIZE);
    bufidx = LP_BUF_SIZE - 1 - bufidx;

    nv_sub_info.rel.idx = bufidx;
    nv_sub_info.rel.len = 1;
    nv_sub_info.rel.rec_siz = recsize;
    if (!nv_read(I_LP_DATA, tptr))
        return false;

    return true;
}

void fill_last_lp_record(void)
{
    lp_last_record_data(appl_tbuff);
    LP_last_record_to_pPdu(appl_tbuff);
}

static void lp_event_bits_setup(void)
{
    lp_event_set_curate();
    if (relay_is_load_on())
    {
        lp_event_set(LPE_LATCH_ON);
    }
    else
    {
        lp_event_set(LPE_LATCH_OFF);
    }

    if ((WMStatus & GE_NOBAT) != 0L)
        lp_event_set(LPE_NO_BAT);

    if ((tamper_det_bit & MAGNET_DETED_BIT) != 0L)
        lp_event_set(LPE_MAGNET_DET);

    if ((WMStatus & TEMPOVER) != 0L)
        lp_event_set(LPE_TEMPOVER);

    if ((WMStatus & (WRONG_NEUT | WRONG_CONN)) != 0L)
        lp_event_set(LPE_WRONGCONN_WRONGNEUT);

    if (IS_TCOVER_OPEN)
        lp_event_set(LPE_TCOVER_OPEN);

    if ((WMStatus & RLYERRMASK) != 0L)
        lp_event_set(LPE_LATCH_ERROR);

    if ((WMStatus & IOVERMASK) != 0L)
        lp_event_set(LPE_IOVER);

    if (WMStatus & (LA_V_LOW | LB_V_LOW | LC_V_LOW))
        lp_event_set(LPE_VOL_LOW);

    if (WMStatus & (LA_V_HIGH | LB_V_HIGH | LC_V_HIGH))
        lp_event_set(LPE_VOL_OVER);
}

static void lp_event_bits_adj(void)
{
    if (!mt_is_uni_dir())
    {
        if (((WMStatus & WRONG_CONN) != 0L) && ((WMStatus & WRONG_NEUT) == 0L))
        {
            lp_event_unset(LPE_WRONGCONN_WRONGNEUT);
        }
    }

    if (relay_is_load_on())
    {
        lp_event_unset(LPE_LATCH_ERROR);
    }
}

static void lp_record_write(lp_record_type *lp_rec, uint32_t idx)
{
    nv_sub_info.rel.idx = (uint16_t)(idx % LP_BUF_SIZE);
    nv_sub_info.rel.idx = LP_BUF_SIZE - 1 - nv_sub_info.rel.idx;
    nv_sub_info.rel.len = 1;
    nv_sub_info.rel.rec_siz = sizeof(lp_record_type);
    nv_write(I_LP_DATA, (uint8_t *)lp_rec);
}

void lp_record_read_while_blocked(uint8_t *cp, uint8_t idx)
{
    uint8_t recsize;

    recsize = sizeof(lp_record_type);
    nv_sub_info.rel.idx = idx;
    nv_sub_info.rel.rec_siz = recsize;
    if (!nv_read(I_LP_DATA_BLKED, cp))
    {
        memset(cp, 0x00, (uint16_t)recsize);
    }
}

/*
    lp_mtdir_is_chged_and_blocked 수행 중
    LP 를 별도 저장 역활 수행
*/
void lp_record_write_while_blocked(lp_record_type *lp_rec)
{
    nv_sub_info.rel.idx = lpindex_while_lpblocked;
    nv_sub_info.rel.rec_siz = sizeof(lp_record_type);
    nv_write(I_LP_DATA_BLKED, (uint8_t *)lp_rec);
    lpindex_while_lpblocked += 1;
}

void lp_record_move_while_blocked(void)
{
    uint8_t i;
    lp_record_type lp_rec;

    LP_index = 0;
    for (i = 0; i < lpindex_while_lpblocked; i++)
    {
        lp_record_read_while_blocked((uint8_t *)&lp_rec, i);

        lp_record_write(&lp_rec, LP_index);
        LP_index += 1;
    }
}

void compress_time(uint8_t *cdt, date_time_type *dt)
{
    cdt[0] = dt->year;
    cdt[0] |= (dt->month & 0x0c) << 4;
    cdt[1] = dt->min;
    cdt[1] |= (dt->month & 0x03) << 6;
    cdt[2] = dt->hour;
    cdt[2] |= (dt->date & 0x1c) << 3;
    cdt[3] = dt->sec;
    cdt[3] |= (dt->date & 0x03) << 6;
}

// -------------------- 평균 전압 LP -------------------
void LPavg_reset(void)
{
    LPavg_init();

    lpavg_v_index = 0;
}

void pwrtn_LPavg_proc(pwrfail_info_type *pfinfo)
{
    if (dsm_swrst_is_bootflag())
    {
        DPRINTF(DBG_ERR, "%s: sysfwup_bool\r\n", __func__);
    }
    else
    {
        LPavg_save(&pfinfo->dt);
        LPavg_init();
    }
}

void timechg_LPavg_proc(date_time_type *bf, date_time_type *af)
{
    LPavg_save(bf);
    LPavg_init();
    af = bf;
}

void monitor_v_over_low(void)
{
    uint8_t i;
    for (i = 0; i < 4; i++)
    {
        lpavg_volt[i] = get_lpavg_v(i);
    }
    for (i = 0; i < 3; i++)
    {
        lpavg_volt_ltol[i] = get_lpavg_ltol(i);
    }
#if 0 /* bccho, 2024-06-03 */
    if ((220.0 * (100.0 - 6.0) / 100.0) > get_lpavg_v(0))
    {
        lp_event_set(LPE_VOL_LOW);
        LOWVOT_A_trigger();
        WMStatus &= ~(LA_V_HIGH);
    }
    else if ((220.0 * (100.0 + 6.0) / 100.0) < get_lpavg_v(0))
    {
        lp_event_set(LPE_VOL_OVER);
        OVERVOT_A_trigger();
        WMStatus &= ~(LA_V_LOW);
    }
    else
    {
        WMStatus &= ~(LA_V_LOW | LA_V_HIGH);
    }
#endif
}

/* bccho, 2024-09-05, 삼상 */
void lpavg_v_sum_cal(float v, float v_ltol, uint8_t i)
{
    float fval;

    lpavg_v_sum[i] += v;
    lpavg_ltol_sum[i] += v_ltol;
    fval = get_inst_volt_THD(i);

    if (lpavg_vi_cnt <= 30)
    {
        lpavg_thd_last[i] += (U16)(fval * 10.0);
    }

    lpavg_i_sum[i] += get_inst_curr(i);
}

void LPavg_proc(void)
{
    float fval;
    date_time_type tdt;
#if 1 /* bccho, 2024-09-05, 삼상 */
    float v_a, v_b, v_c;
    float v_ab, v_bc, v_ca;
#else
    float v_ab;
#endif

    if (!b_lpavg_ready)
        goto LPavg_proc1;

    b_lpavg_ready = false;

/* bccho, 2024-09-05, 삼상 */
#if PHASE_NUM == SINGLE_PHASE
    v_a = vrms0_mon;
    lpavg_v_sum[0] += v_a;
    fval = get_inst_volt_THD(0);

    if (lpavg_vi_cnt <= 30)
    {
        lpavg_thd_last[0] += (U16)(fval * 10.0);
    }

    lpavg_i_sum[0] += get_inst_curr(0);

    lpavg_vi_cnt++;
#else
    v_ab = get_inst_LtoL(0);
    v_bc = get_inst_LtoL(1);
    v_ca = get_inst_LtoL(2);
    lpavg_v_sum_cal(vrms0_mon, v_ab, 0);
    lpavg_v_sum_cal(vrms1_mon, v_bc, 1);
    lpavg_v_sum_cal(vrms2_mon, v_ca, 2);

    lpavg_vi_cnt++;
#endif

LPavg_proc1:
    if (b_lpavg_monitor && (cur_min % lpavg_interval) == 0)
    {
        tdt = cur_rtc;
        tdt.sec = 0;
        LPavg_save(&tdt);
        LPavg_init();
    }

    b_lpavg_monitor = false;
}

void lpavg_record_read(uint8_t *cp, uint32_t idx, uint8_t len)
{
    uint8_t len1;
    uint16_t bufidx;

    bufidx = (uint16_t)(idx % LPAVG_BUF_SIZE);

    bufidx = LPAVG_BUF_SIZE - 1 - bufidx;

    if ((bufidx + len) > LPAVG_BUF_SIZE)
    {
        len1 = LPAVG_BUF_SIZE - bufidx;

        nv_sub_info.rel.idx = bufidx;
        nv_sub_info.rel.len = len1;
        if (!nv_read(I_LPAVG_DATA, cp))
            memset(cp, 0x00, (uint16_t)(len1 * sizeof(lpavg_record_type)));

        cp += (uint16_t)(len1 * sizeof(lpavg_record_type));

        bufidx = 0;
        len -= len1;
    }

    nv_sub_info.rel.idx = bufidx;
    nv_sub_info.rel.len = len;
    if (!nv_read(I_LPAVG_DATA, cp))
        memset(cp, 0x00, (uint16_t)(len * sizeof(lpavg_record_type)));
}

void lpavg_save_manual(void)
{
    uint16_t i;
    date_time_type tdt;

    tdt = cur_rtc;
    lpavg_v_index = 0;

    for (i = 0; i < (LPAVG_BUF_SIZE + 10); i++)
    {
        vTaskDelay(2);

        dsm_wdt_ext_toggle_immd();

        LPavg_save(&tdt);
        time_up(&tdt, 1);
    }

    nv_write(I_WHM_OP, (uint8_t *)&whm_op);
}

static void lp_avgsum_init(void)
{
    lpavg_vi_cnt = 0;
    lpavg_v_sum[0] = 0.0;
    lpavg_v_sum[1] = 0.0;
    lpavg_v_sum[2] = 0.0;

    /* bccho, 2024-09-05, 삼상 */
    lpavg_ltol_sum[0] = 0.0;
    lpavg_ltol_sum[1] = 0.0;
    lpavg_ltol_sum[2] = 0.0;

    lpavg_i_sum[0] = 0.0;
    lpavg_i_sum[1] = 0.0;
    lpavg_i_sum[2] = 0.0;
    lpavg_thd_last[0] = 0;
    lpavg_thd_last[1] = 0;
    lpavg_thd_last[2] = 0;
}

/* bccho, 2024-09-05, 삼상 */
float get_lpavg_ltol(uint8_t line)
{
    float fval;
    if (lpavg_vi_cnt == 0)
        return 0.0;
#if defined(FEATURE_CPPcheck)
    if (line >= 3)
        return 0.0;
#endif

    // DPRINTF(DBG_INFO, "lpavg_vi_cnt: %d\r\n", (UINT32)(lpavg_vi_cnt)); // cnt
    // 0 DPRINTF(DBG_INFO, "lpavg_ltol_sum: %d.%03d\r\n",
    // (UINT32)(lpavg_ltol_sum[line]), (UINT32)((lpavg_ltol_sum[line] -
    // (UINT32)(lpavg_ltol_sum[line]))*1000)); // cnt 0

    // fval = lpavg_ltol_sum[line] / lpavg_vi_cnt;
    fval = lpavg_ltol_sum[line] / (float)lpavg_vi_cnt;
    // DPRINTF(DBG_INFO, "fval: %d.%03d\r\n", (UINT32)(fval), (UINT32)((fval -
    // (UINT32)(fval))*1000)); // cnt 0

    return fval;
}

void LPavg_init(void) { lp_avgsum_init(); }

float get_lpavg_v(uint8_t line)
{
    if (lpavg_vi_cnt == 0)
        return 0.0;

    if (line >= 3)
    {
        return 0.0;
    }

    return lpavg_v_sum[line] / lpavg_vi_cnt;
}

float get_lpavg_i(uint8_t line)
{
    float fval_i;
    if (lpavg_vi_cnt == 0)
        return 0.0;

#if 0
	return lpavg_i_sum[line] / lpavg_vi_cnt;
#else
    fval_i = lpavg_i_sum[line] / lpavg_vi_cnt;

    if (fval_i < StartCur)
        fval_i = 0.0;  // jp.kim 25.04.03

    return fval_i;
#endif
}

#if 1  // max 30회 평균
U16 get_lpavg_v_thd(U8 line)
{
    U16 data_16, sub_data = 0;

    if (lpavg_vi_cnt == 0)
        return 0.0;

    if (lpavg_vi_cnt > 30)
    {
        sub_data = ((U16)(lpavg_v_sum[line] * 1000.0)) %
                   10;  // 소숫점 이하 ramdom 값 생성(0~9)
        // DPRINTF(DBG_ERR, "get_inst_volt_THD :  sub_data:%d
        // lpavg_thd_last[0]:%d  lpavg_vi_cnt= %d\r\n",  (U16)(sub_data),
        // lpavg_thd_last[0], (UINT32)(lpavg_vi_cnt) ); // cnt 0
        data_16 = ((U16)(lpavg_thd_last[line] / 30)) * (10);
        data_16 += sub_data;
        // DPRINTF(DBG_ERR, "get_inst_volt_THD :  data_16:%d
        // lpavg_thd_last[0]:%d  lpavg_vi_cnt= %d\r\n",  (U16)(data_16),
        // lpavg_thd_last[0], (UINT32)(lpavg_vi_cnt) ); // cnt 0
    }
    else
    {
        sub_data = ((U16)(lpavg_v_sum[line] * 1000.0)) %
                   10;  // 소숫점 이하 ramdom 값 생성(0~9)
        // DPRINTF(DBG_ERR, "get_inst_volt_THD :  sub_data:%d
        // lpavg_thd_last[0]:%d  lpavg_vi_cnt= %d\r\n",  (U16)(sub_data),
        // lpavg_thd_last[0], (UINT32)(lpavg_vi_cnt) ); // cnt 0
        data_16 = ((U16)(lpavg_thd_last[line] / lpavg_vi_cnt)) * (10);
        data_16 += sub_data;
        // DPRINTF(DBG_ERR, "get_inst_volt_THD :  data_16:%d
        // lpavg_thd_last[0]:%d  lpavg_vi_cnt= %d\r\n",  (U16)(data_16),
        // lpavg_thd_last[0], (UINT32)(lpavg_vi_cnt) ); // cnt 0
    }
    return data_16;
}
#endif

static void lpavg_record_write(uint8_t *cp, uint32_t idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    nv_sub_info.rel.idx = (uint16_t)(idx % LPAVG_BUF_SIZE);
    nv_sub_info.rel.idx = LPAVG_BUF_SIZE - 1 - nv_sub_info.rel.idx;
    nv_sub_info.rel.len = 1;
    nv_write(I_LPAVG_DATA, (uint8_t *)cp);
}

void LPavg_save(date_time_type *dt)
{
    lpavg_record_type lpavg;
    uint8_t i = 0;

    MSG07("LPavg_save, cnt:%d, idx:%d", lpavg_vi_cnt, lpavg_v_index);

    if (lpavg_vi_cnt == 0)
        return;

    DPRINTF(DBG_TRACE, _D "%s: vi_cnt[%d], v_index[%d]\r\n", __func__,
            lpavg_vi_cnt, lpavg_v_index);

#if PHASE_NUM == SINGLE_PHASE
    lpavg.ch[0] = (uint16_t)(get_lpavg_v(0) * 100.0);
#if 0
  	lpavg.ch[1] = (U16)(get_inst_volt_THD(0) * 100.0);
#else
    lpavg.ch[1] = (U16)(get_lpavg_v_thd(0));
#endif
    lpavg.ch[2] = (uint16_t)(get_lpavg_i(0) * 100.0);
    lpavg.ch[3] = (uint16_t)(get_inst_phase(0) * 100.0);  // 전압 전류 위상각...

    for (i = 4; i < (15 + 3); i++) lpavg.ch[i] = 0;

#else  // 3 PHASE
    lpavg.ch[0] = (uint16_t)(get_lpavg_ltol(0) * 100.0);
    lpavg.ch[1] = (uint16_t)(get_lpavg_v(0) * 100.0);
#if 0
	lpavg.ch[2] = (U16)(get_inst_volt_THD(0) * 100.0);
#else
    lpavg.ch[2] = (U16)(get_lpavg_v_thd(0));
#endif
    lpavg.ch[3] = (uint16_t)(get_lpavg_i(0) * 100.0);
    lpavg.ch[4] = (uint16_t)(get_inst_phase(0) * 100.0);  // 전압 전류 위상각...
    lpavg.ch[5] = 0.0;
    lpavg.ch[6] = (uint16_t)(get_lpavg_ltol(1) * 100.0);
    lpavg.ch[7] = (uint16_t)(get_lpavg_v(1) * 100.0);
#if 0
   	lpavg.ch[8] = (U16)(get_inst_volt_THD(1) * 100.0);
#else
    lpavg.ch[8] = (U16)(get_lpavg_v_thd(1));
#endif
    lpavg.ch[9] = (uint16_t)(get_lpavg_i(1) * 100.0);
    lpavg.ch[10] =
        (uint16_t)(get_inst_phase(1) * 100.0);  // 전압 전류 위상각...
    lpavg.ch[11] = 0.0;
    lpavg.ch[12] = (uint16_t)(get_lpavg_ltol(2) * 100.0);
    lpavg.ch[13] = (uint16_t)(get_lpavg_v(2) * 100.0);
#if 0
    lpavg.ch[14] = (U16)(get_inst_volt_THD(2) * 100.0);
#else
    lpavg.ch[14] = (U16)(get_lpavg_v_thd(2));
#endif
    lpavg.ch[15] = (uint16_t)(get_lpavg_i(2) * 100.0);
    lpavg.ch[16] =
        (uint16_t)(get_inst_phase(2) * 100.0);  // 전압 전류 위상각...
    lpavg.ch[17] = 0.0;
#endif

    compress_time((uint8_t *)&lpavg.dt[0], dt);

    lpavg_record_write((uint8_t *)&lpavg, lpavg_v_index);
    lpavg_v_index += 1L;

    DPRINTF(DBG_TRACE,
            _D
            "%s: lpavg_vi_cnt[%d], lpavg_interval[%d], lpavg_v_sum[0][%d]\r\n",
            __func__, lpavg_vi_cnt, lpavg_interval, (uint32_t)lpavg_v_sum[0]);

    monitor_v_over_low();
}

/*
   LP 주기에 동기해서 역률을 구해 LCD 표시 나 DLMS 통신 시 직전 역률에 사용
   됩니다.
*/
void update_lp_intv_pf(void)
{
    uint8_t i;

    lp_intv_pf[eDeliAct] =
        calc_pf(lp_intv_dm[eChDeliAct], get_sel_react32(eDeliAct, lp_intv_dm));
    lp_intv_pf[eReceiAct] = calc_pf(lp_intv_dm[eChReceiAct],
                                    get_sel_react32(eReceiAct, lp_intv_dm));

    for (i = 0; i < numCHs; i++)
    {
        lp_intv_dm[i] = 0L;
    }

    dsp_rcntpf_modified_set();
}

static void lp_rtsum_init(void) { lprt__cnt = 0; }

void LPrt_init(void) { lp_rtsum_init(); }

void LPrt_reset(void)
{
    LPrt_init();

    lprt__index = 0;
}

void pwrtn_LPrt_proc(pwrfail_info_type *pfinfo)
{
    LPrt_save(&pfinfo->dt);

    LPrt_init();
}

void timechg_LPrt_proc(date_time_type *bf, date_time_type *af)
{
    LPrt_save(bf);
    LPrt_init();
    af = bf;
}

/*
주기에 의한 realtime LP 를 NV record에 저장.
*/
void LPrt_proc(void)
{
    date_time_type tdt;

#if 0 /* bccho, 2023-12-11 */
    if (!b_lprt_ready)
        goto LPrt_proc1;
#endif

    b_lprt_ready = false;

    lprt__cnt++;

#if 0 /* bccho, 2023-12-11 */
LPrt_proc1:
#endif
    if (b_lprt_monitor && ((cur_min * 60 + cur_sec) % rt_lp_interval) == 0)
    {
        uint32_t ch_count = dsm_get_dm_out_measure_print_chkcount();
        tdt = cur_rtc;

        if (ch_count % 20 == 5 && (rt_lp_interval < 5))
        {
            DPRINTF(DBG_INFO,
                    _D
                    "%s: cur_min[%d], cur_sec[%d], rt_lp_interval[%d], "
                    "b_lprt_monitor[%d]\r\n",
                    __func__, cur_min, cur_sec, rt_lp_interval, b_lprt_monitor);
        }

        if ((rt_lp_interval % 60) == 0)
        {
            tdt.sec = 0;
        }
        LPrt_save(&tdt);

        LPrt_init();
    }

    b_lprt_monitor = false;
}

void lprt_record_read(uint8_t *cp, uint32_t idx, uint8_t len)
{
    uint8_t len1;
    uint16_t bufidx;
    uint8_t phase_type = 0;

    if (mt_is_onephase())
    {
        phase_type = SINGLE_PHASE;
    }
    else
    {
        phase_type = THREE_PHASE;
    }

    bufidx = (uint16_t)(idx % LPRT_BUF_SIZE);

    bufidx = LPRT_BUF_SIZE - 1 - bufidx;

    if ((bufidx + len) > LPRT_BUF_SIZE)
    {
        len1 = LPRT_BUF_SIZE - bufidx;

        ram_sub_info.rel.idx = bufidx;
        ram_sub_info.rel.len = len1;
        if (!ram_read(I_LPRT_DATA, cp))
        {
            if (phase_type == SINGLE_PHASE)
            {
                memset(cp, 0x00, (uint16_t)(len1 * sizeof(lprt_record_1phs)));
            }
            else
            {
                memset(cp, 0x00, (uint16_t)(len1 * sizeof(lprt_record_3phs)));
            }
        }

        if (phase_type == SINGLE_PHASE)
        {
            cp += (uint16_t)(len1 * sizeof(lprt_record_1phs));
        }
        else
        {
            cp += (uint16_t)(len1 * sizeof(lprt_record_3phs));
        }

        bufidx = 0;
        len -= len1;
    }

    ram_sub_info.rel.idx = bufidx;
    ram_sub_info.rel.len = len;
    if (!ram_read(I_LPRT_DATA, cp))
    {
        if (phase_type == SINGLE_PHASE)
        {
            memset(cp, 0x00, (uint16_t)(len * sizeof(lprt_record_1phs)));
        }
        else
        {
            memset(cp, 0x00, (uint16_t)(len * sizeof(lprt_record_3phs)));
        }
    }
}

static void lprt_record_write(uint8_t *cp, uint32_t idx)
{
    ram_sub_info.rel.idx = (uint16_t)(idx % LPRT_BUF_SIZE);
    ram_sub_info.rel.idx = LPRT_BUF_SIZE - 1 - ram_sub_info.rel.idx;
    ram_sub_info.rel.len = 1;
    ram_write(I_LPRT_DATA, (uint8_t *)cp);
}

void LPrt_save(date_time_type *dt)
{
    uint32_t ch_count = dsm_get_dm_out_measure_print_chkcount();
    float w, var;

    if (lprt__cnt == 0)
        return;

    if (ch_count % 20 == 5 && (rt_lp_interval < 5))
    {
        DPRINTF(DBG_TRACE, _D "%s: vi_cnt[%d], v_index[%d]\r\n", __func__,
                lprt__cnt, lprt__index);
    }

    if (mt_is_onephase())
    {
        lprt_record_1phs lprt_1phs;

        memset(&lprt_1phs, 0x00, sizeof(lprt_record_1phs));
        compress_time((uint8_t *)&lprt_1phs.dt[0], dt);

        w = w0sum_mon;
        if (w >= 0.0)
        {
            lprt_1phs.ch_2[0] = w;
        }
        else
        {
            lprt_1phs.ch_2[1] = fabs(w);
        }
        var = var0sum_mon;
        if (var >= 0.0)
        {
            lprt_1phs.ch_2[2] = var;
        }
        else
        {
            lprt_1phs.ch_2[3] = fabs(var);
        }

        lprt_record_write((uint8_t *)&lprt_1phs, lprt__index);
    }
    else /* bccho, 2024-09-05, 삼상 */
    {
        lprt_record_3phs lprt_3phs;

        memset(&lprt_3phs, 0x00, sizeof(lprt_record_3phs));
        compress_time((uint8_t *)&lprt_3phs.dt[0], dt);

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
        w = w0sum_mon + w1sum_mon + w2sum_mon;
        if (w >= 0.0)
        {
            lprt_3phs.ch_2[0] = w;
        }
        else
        {
            lprt_3phs.ch_2[1] = fabs(w);
        }
        var = var0sum_mon + var1sum_mon + var2sum_mon;
        if (var >= 0.0)
        {
            lprt_3phs.ch_2[2] = var;
        }
        else
        {
            lprt_3phs.ch_2[3] = fabs(var);
        }
#endif

        w = w0sum_mon;
        if (w >= 0.0)
        {
            lprt_3phs.ch_2[4] = w;
        }
        else
        {
            lprt_3phs.ch_2[5] = fabs(w);
        }
        var = var0sum_mon;
        if (var >= 0.0)
        {
            lprt_3phs.ch_2[6] = var;
        }
        else
        {
            lprt_3phs.ch_2[7] = fabs(var);
        }

        w = w1sum_mon;
        if (w >= 0.0)
        {
            lprt_3phs.ch_2[0 + 4 * 2] = w;
        }
        else
        {
            lprt_3phs.ch_2[1 + 4 * 2] = fabs(w);
        }
        var = var1sum_mon;
        if (var >= 0.0)
        {
            lprt_3phs.ch_2[2 + 4 * 2] = var;
        }
        else
        {
            lprt_3phs.ch_2[3 + 4 * 2] = fabs(var);
        }

        w = w2sum_mon;
        if (w >= 0.0)
        {
            lprt_3phs.ch_2[0 + 4 * 3] = w;
        }
        else
        {
            lprt_3phs.ch_2[1 + 4 * 3] = fabs(w);
        }
        var = var2sum_mon;
        if (var >= 0.0)
        {
            lprt_3phs.ch_2[2 + 4 * 3] = var;
        }
        else
        {
            lprt_3phs.ch_2[3 + 4 * 3] = fabs(var);
        }

#if 1 /* bccho, 2024-09-24, 삼상 */
        for (int i = 0; i < 4; i++)
        {
            lprt_3phs.ch_2[i] = lprt_3phs.ch_2[i + 4] +
                                lprt_3phs.ch_2[i + 4 * 2] +
                                lprt_3phs.ch_2[i + 4 * 3];
        }
#endif

        lprt_record_write((uint8_t *)&lprt_3phs, lprt__index);
    }

    lprt__index += 1L;
}
