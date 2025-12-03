#include <stdlib.h>
#include "options.h"
#if 0 /* bccho, FLASH, 2023-07-15 */
#include "flash.h"
#endif /* bccho */
#include "meter_app.h"
#include "tmp.h"
#include "comm.h"
#include "eoi.h"
#include "eob.h"
#include "lp.h"
#include "program.h"
#include "nv.h"
#include "key.h"
#include "get_req.h"
#include "dl.h"
#include "bat.h"
#include "port.h"
#include "pulse_src.h"
#include "utils.h"
#include "amg_imagetransfer.h"
#include "act_req.h"
#include "amg_stock_op_mode.h"
#include "amg_power_mnt.h"
#include "amg_utc_util.h"
#include "amg_rtc.h"
#include "amg_wdt.h"
#include "amg_mtp_process.h"
extern ST_MIF_METER_PARM g_mtp_meter_parm;
extern bool power_notify_tx_set;
extern bool atcmd_modem_bearer_q_task_set;
extern int8_t no_inst_curr_chk_zon_cnt;

#include "program.h"

#define _D "[WHM] "

extern int auto_bidir_chg_chk_cnt;
// bool b_season_changed_rtc_back, rtc_season_chg_chk;

#define METER_RST_TIME_OFFSET 8

extern float PULSE_ADD_MODIFY_DATA, PULSE_ADD_MODIFY_DATA_VA,
    PULSE_ADD_MODIFY_DATA_VAR;
extern bool METER_FW_UP_ING_STS;
extern bool METER_FW_UP_END_PULSE_MODIFY;
extern U16 METER_FW_UP_ING_CNT;

#if 1  // jp.kim 25.02.04
bool mif_meter_para_get_wait_ing = 0;
bool mif_meter_para_set_wait_ing = 0;
uint8_t mif_meter_para_set_wait_seq_cnt = 0;
// extern uint8_t direct_reverse_monitor;
extern ST_MIF_METER_PARM monitor_mtp_meter_parm;
#endif

#if 1  // jp.kim 25.03.12
bool mif_meter_sagswe_get_wait_ing = 0;
bool mif_meter_sagswe_set_wait_ing = 0;
uint8_t mif_meter_sagswe_set_wait_seq_cnt = 0;
extern ST_MTP_SAGSWELL monitor_mtp_sagswell;
#endif

U8 first_pulse_delete_flag = 0;
U8 first_pulse_delete_cnt = 0;

int run_mode;
buff_struct_type ubuff;

mt_conf_type mt_conf;
whm_op_type whm_op;

bool bat_rtc_backuped, eob_pwrtn;
date_time_type cur_rtc;
rate_type cur_rate;
rate_type cur_rate_before;
ratekind_type rtkind_history;
uint8_t mtdir_history;
uint8_t mr_date_history;
float temp_caled;
float adj_currtemp;

uint8_t b_q_display;
int b_q_dspkind;

bool b_mtaccm_prd_bakup;
bool b_whmop_prd_bakup;

uint32_t mxdm_dgt_cnt;
uint32_t mxaccm_dgt_cnt;
pwrfail_info_type pwrfail_info;
bool b_pwrtn_for_dst;

int32_t last_sec;
mt_conf_2_type mt_conf_2;
static void mt_conf_2_restore(bool saged);
void mt_conf_2_default(void);
static void mt_conf_2_backup_check(void);
#if 1  // jp.kim 24.10.29
static void default_bill_parm(bool ket_set);
#else
static void default_bill_parm(void);
#endif
extern void error_code_event_clear(void);
void sr_log_dt_init(void);

static void whm_rtc_event(uint8_t* tptr);
static void mt_data_restore(uint8_t* tptr);
static void mt_conf_restore(bool saged);
static void whm_op_restore(bool saged);
static void mt_accm_restore(bool saged);
void whm_op_sag_save(void);
static void mt_conf_backup_check(void);
static void mt_accm_save(void);
static void bat_usedtime_restore(void);
static void supp_dsp_restore(void);
static void whmop_bakup_crc_calc(void);
static void mtaccm_bakup_crc_calc(void);
static void sag_flag_set(void);
static void pwrfail_info_get(pwrfail_info_type* pfinfo);
static void sag_flag_unset(void);
void mt_conf_default(void);
static void whm_op_default(bool default_relay_on);  // jp.kim 24.11.07
static void mt_accm_reset(void);
static void sag_flag_chk(void);

static void update_history_var(uint8_t dir, ratekind_type kind);
void tmp_meas_trig(void);
void is_sCurr_rev_cur_to_bidir(uint8_t* tptr);
void dsm_push_err_code_set(void);
void dsm_mtp_fsm_tx_proc_parmset(void);
bool meter_parm_wr_rd_cmp(void);

#define VER_FW_NV_SIZE 10
const uint8_t ver_fw_nv[] = {VERSION_FW_NV};
const U8 COSEM_METER_ID_VER[] = {"233"};
const U8 zcrs_sig[] = {1, 0, 0, 0};  // 1000 ms

/*
통신 규격
2.3.2 논리적 장치명 (Logical Device Name : LDN)의 구조:
3.4.2.3.1.2 COSEM 계기 식별자 설명:
    장치 관리자용(Management)
        제조사 고유코드 : “SMT”, 제조 일자 : “2020.03.01.”, 제조관리번호 : “A”
        Logical Device 번호 : 장치 관리용 = 1, 한전 관리용 = 2
        규격 버전 : “3.X”이상 (보안 계기) “2.X”(비 보안 계기)

    한전 관리용
        제조사 고유코드 : “SMT”, 제조 일자 : “2020.03.01.”, 제조관리번호 : “A”
        Logical Device 번호 : 장치 관리용 = 1, 한전 관리용 = 2
        규격 버전 : “3.X”이상 (보안 계기) “2.X”(비 보안 계기)
ex)
    uint8_t COSEM_METER_ID[16];
    COSEM_METER_ID[0] = 'S';
    COSEM_METER_ID[1] = 'M';
    COSEM_METER_ID[2] = 'T';
    COSEM_METER_ID[3] = ' ';
    COSEM_METER_ID[4] = '2';
    COSEM_METER_ID[5] = '0';
    COSEM_METER_ID[6] = '0';
    COSEM_METER_ID[7] = '3';
    COSEM_METER_ID[8] = '0';
    COSEM_METER_ID[9] = '1';
    COSEM_METER_ID[10] = 'A';
    COSEM_METER_ID[11] = ' ';
    COSEM_METER_ID[12] = ' ';
    COSEM_METER_ID[13] = '1';
    COSEM_METER_ID[14] = '3';
    COSEM_METER_ID[15] = '0';
*/
const uint8_t logical_device_name_r[DEVICE_ID_SIZE] = {
    /*  Flag(B1~B3)                   ,rev(B4)        ,제조연월일 (B5~B10)
       ,제조관리번(B11)        ,rev(B12~14)   ,LD(B14, '1')   spec ver(B15B16,
       ascill)*/
    FLAG_ID1, FLAG_ID2, FLAG_ID3, ' ', '2', '4', '0', '1',
    '2',      '3',      'A',      ' ', ' ', '1', '3', '2'};

/*
Kepco Management
제조사 고유코드 : "XXX”, 제조년월(BCD) : “2003” -> 2020년 03월
소프트웨어 버전 : 「보안 강화형 전력량계 일반규격 3.8항 표 75 소프트웨어 버전
참조」 '0'(법정) '0'(단상) '2' '5'(제조사 코드) '0' '0'(소프트웨어 버전 )
Logical Device 번호 : 장치 관리용 = 1, 한전 관리용 = 2
규격 버전(BCD) : “30” -> 버전 3.0

kepco_spec update : 2020.08.20 Device Management 방식과 동일하게 처리 -> 단 LD
'2'
*/
uint8_t logical_device_name_r_kepco[DEVICE_ID_SIZE] = {
    /*  Flag(B1~B3)                  ,제조연월(B4B5)       ,fw ver(B6~B11)
       ,reserved(B12~14)     ,LD(B14, '2')   ,spec ver(B15, BCD)*/
    /*	FLAG_ID1,FLAG_ID2,FLAG_ID3,   0x20,0x02,       '0',    '0',    '2','5',
       '0','1',   ' ', ' ', ' ',        '2',            0x30 				*/

    /*  Flag(B1~B3)                   ,rev(B4)        ,제조연월일 (B5~B10)
       ,제조관리번(B11)        ,rev(B12~14)   ,LD(B14, '1')   spec ver(B15B16,
       ascill)*/
    FLAG_ID1, FLAG_ID2, FLAG_ID3, ' ', '2', '4', '0', '1',
    '2',      '3',      'A',      ' ', ' ', '2', '3', '3'};

#define DFT_A_BEGIN 9   // 9:00 hour
#define DFT_B_BEGIN 23  // 23:00 hour
static const tou_struct_type tou_default_twokind[MAX_TOU_DIV_DLMS] = {
    {DFT_A_BEGIN, 00,
     RATE_TS_TO_SELECTOR(eArate, SW_CTRL_OFF)},  // hour, min, rate
    {DFT_B_BEGIN, 00,
     RATE_TS_TO_SELECTOR(eBrate, SW_CTRL_ON)},  // hour, min, rate
    {DFT_B_BEGIN, 00,
     RATE_TS_TO_SELECTOR(eBrate, SW_CTRL_ON)},  // hour, min, rate
    {DFT_B_BEGIN, 00,
     RATE_TS_TO_SELECTOR(eBrate, SW_CTRL_ON)},  // hour, min, rate
};

static const tou_struct_type tou_default_onekind[MAX_TOU_DIV_DLMS] = {
    {00, 00, RATE_TS_TO_SELECTOR(eArate, SW_CTRL_OFF)},  // hour, min, rate
    {00, 00, RATE_TS_TO_SELECTOR(eArate, SW_CTRL_OFF)},  // hour, min, rate
    {00, 00, RATE_TS_TO_SELECTOR(eArate, SW_CTRL_OFF)},  // hour, min, rate
    {00, 00, RATE_TS_TO_SELECTOR(eArate, SW_CTRL_OFF)},  // hour, min, rate
};

void whm_init(void)
{
    uint8_t* tptr;
    tptr = adjust_tptr(&global_buff[0]);
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    eob_pwrtn = 0;
    // rtc_season_chg_chk = false;
    // b_season_changed_rtc_back = false;
    auto_bidir_chg_chk_cnt = 0;

    b_q_display = 0;

    cur_sec = 0xff;

    // meter global status is inited
    WMStatus = 0L;
    WMStatus_intern = 0L;
    MTStatus = 0L;

    b_mtaccm_prd_bakup = false;
    b_whmop_prd_bakup = false;

    PULSE_ADD_MODIFY_DATA = 0.0;
    PULSE_ADD_MODIFY_DATA_VA = 0.0;
    PULSE_ADD_MODIFY_DATA_VAR = 0.0;
    METER_FW_UP_ING_STS = 0;
    METER_FW_UP_END_PULSE_MODIFY = 0;
    METER_FW_UP_ING_CNT = 0;

    power_notify_tx_set = 0;

    min_max_timeset(T10SEC);

    init_softtmr();

    whm_init_1();

    mt_data_restore(tptr);

#if 1
    set_prog_dl_idx();  // prog_dl_idx is initialized because of CTT
    set_hol_dl_idx();   // used when tou continued to download and CTT
#endif

    program_init();

    eoi_init();

    LP_init();

    eob_init();  // whm_op should be initialized (bcz of mr_cnt)

    amr_init();

    disp_init();

    key_init();

    dsm_gpio_relay_port_init();

    load_ctrl_init();

    imax_log_init();

    temp_over_log_init();

    mt_abnorm_mon_init();

    var_history_init();

    adjust_max_whcnt(lcd_w_point, lcd_wh_point);

    curr_rate_update();
    DPRINTF(DBG_TRACE, _D "%s: END\r\n", __func__);

    /* bccho, 2024-09-05, 삼상 */
    if (IS_AC_ON)
    {
        dsm_gpio_imodem_pf_high(); /* PF unset */
        DPRINTF(DBG_ERR, "dsm_gpio_imodem_pf_high()\r\n");
    }
}

extern uint8_t bat_inst_mon;
void cur_rtc_restore(date_time_type* back_dt, uint8_t* tptr)
{
    DATE_TIME_T curr_data_time;
    date_time_type dt;
    bool rtc_is_inited;

    rtc_is_inited = rtc_init();

    if (!dsm_rtc_get_hw_time(&curr_data_time))
    {
        rtc_is_inited = FALSE;
    }

    struct tm SystemTime;
    ST_TIME_BCD stITime;

    util_get_system_time(&SystemTime, &stITime);

    if (SystemTime.tm_year == 100)
    {
        rtc_is_inited = FALSE;
    }

    bat_rtc_backuped = rtc_is_inited;

    if (!bat_rtc_backuped)
    {
        if (back_dt != 0)
        {
            dt = *back_dt;
            write_rtc(&dt);
        }
        else
        {
            reset_rtc();
        }

        cur_rtc_update();

        if (run_is_main_power())
        {
            bat_inst_clear();
        }
        else
        {
            if (cur_year == 0)
            {
                bat_inst_clear();
            }
            else
            {
                bat_inst_set();
            }
        }
    }
    else
    {
        cur_rtc_update();
    }
}

int dsm_run_power_get(void) { return run_mode; }

char* dsm_run_power_string(uint32_t idx)
{
    switch (idx)
    {
    case RUN_MAIN_POWER:
        return "MAIN_POWER";
    case RUN_BAT_POWER:
        return "BAT_POWER";

    default:
        return "RUN_Unknown";
    }
}

void cur_rtc_restore_batmode(void)
{
    uint8_t* tptr;
    tptr = adjust_tptr(&global_buff[0]);

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    if ((MTStatus & MT_SAGED) && is_good_date_time(&pwrfail_dt))
    {
        cur_rtc_restore(&pwrfail_dt, tptr);
    }
    else
    {
        cur_rtc_restore(&pwrfail_dt, tptr);
    }
}

int get_ver_fw_nv(uint8_t* tptr)
{
    memcpy(tptr, &ver_fw_nv[0], VER_FW_NV_SIZE);

    return VER_FW_NV_SIZE;
}

bool mt_is_registered(void)
{
    int32_t diff;

    if (mt_register_dt.month == 0)
        return 0;

    diff = calc_date_time_diff(&cur_rtc, &mt_register_dt);
    if (diff >= 0 && diff < 24 * 60 * 60L)
        return 1;

    return 0;
}

void var_history_init(void)
{
    rtkind_history = 0;
    mtdir_history = 0;
}

void tmp_bat_meas(void)
{
#define VBAT_TEMP_MEAS_PRD 3  // 3 sec

    if (tmp_read_ready)
    {
        static int vbat_tmeas_prd = 0;
        tmp_read_ready = false;

        if (++vbat_tmeas_prd >= VBAT_TEMP_MEAS_PRD)
        {
            vbat_tmeas_prd = 0;

            temp_over_mon();
        }

        bat_det_mon();
    }
}

void pwr_rtn_proc(void)
{
    int32_t pfdur;
    uint8_t* tptr;
    tptr = adjust_tptr(&global_buff[0]);

    DPRINTF(DBG_TRACE, _D "%s: WMStatus[0x%08X], MTStatus[0x%08X]\r\n",
            __func__, WMStatus, MTStatus);

    // jp.kim 25.03.12
    no_inst_curr_chk_zon_cnt = 10;

    b_pwrtn_for_dst = false;
    if ((MTStatus & MT_SAGED) && is_good_date_time(&pwrfail_dt))
    {
        MSG07("pwr_rtn_proc()_After SAG");
        b_pwrtn_for_dst = true;

        pwrfail_info_get(&pwrfail_info);

        DPRINTF(DBG_TRACE, _D "%s 1\r\n", __func__);

        cur_rtc_restore(&pwrfail_info.dt, tptr);

        curr_rate_update();

        pfdur = calc_dtime_diff(&cur_rtc, &pwrfail_info.dt);
        DR_limit_adj(pfdur);

        if (eoi_processed)
            cur_dmdt_set(&cur_rtc);
        else
            eoi_proc_pwrtn(&pwrfail_info, pfdur, tptr);

        pwrtn_LP_proc(&pwrfail_info);

        prog_changed_event_clr();

        MSG07("prog_fut_is_available %d", prog_fut_is_available());
        MSG07("<R>: %d-%d-%d %d:%d:%d", mt_conf.prog.futwork.year,
              mt_conf.prog.futwork.month, mt_conf.prog.futwork.date,
              mt_conf.prog.futwork.hour, mt_conf.prog.futwork.min,
              mt_conf.prog.futwork.sec);
        MSG07("<C>: %d-%d-%d %d:%d:%d", cur_rtc.year, cur_rtc.month,
              cur_rtc.date, cur_rtc.hour, cur_rtc.min, cur_rtc.sec);

        program_proc(tptr);

        pwrtn_eob_proc(&pwrfail_info, tptr);

        pwrtn_LPavg_proc(&pwrfail_info);

        pwrtn_LPrt_proc(&pwrfail_info);

        pwrtn_meas_proc(tptr);

        if (dsm_swrst_is_bootflag())
        {
            dsm_swrst_init_bootflag();
            atcmd_modem_bearer_q_task_set = 1;  // JP.KIM 25.01.17
            first_pulse_delete_flag = 'T';      // JP.KIM 25.03.02
            first_pulse_delete_cnt = 2;         // JP.KIM 25.03.02
        }
        else
        {
            log_pwr_FR(&pwrfail_info);
        }

        dm_intv_nvdelete(tptr);

        dst_mon(false, &cur_rtc, tptr);

        bat_used_time_proc(bat_rtc_backuped, &cur_rtc);

        sag_flag_unset();  // after pwr_rtn_proc() is completed
        error_code_event_clear();
    }
    else if ((MTStatus & MT_INITED_BY_COMM))
    {
        cur_rtc_restore(&cur_rtc, tptr);
    }
    else
    {
        DPRINTF(DBG_TRACE, _D "%s 2\r\n", __func__);
        cur_rtc_restore(0, tptr);
    }
}

/*
 보안 일반 규격 : 검침 기능 [200417_spec]
정기검침
- 현장 또는 원격으로 설정한 검침일에 '정기검침' 실시
- 정기검침 : 자동으로 검침 값 확정(SR) 및 수요전력 확정(DR)을 수행
- 정기검침 기록값 : 누적전력량, 최대수요전력, 역률

비 정기검침
- 현장 또는 원격으로 설정한 비정기 검침일에 실시
- 현장에서는 버튼을 이용한 수동검침으로 실시
- 프로그램 입력, 프로그램 예약 적용, 계량 모드 변경 시 실시
- 자동으로 검침 값 확정(SR) 및 수요전력 확정(DR)을 수행
- 비 정기검침 기록값 : 누적전력량, 최대수요전력, 역률

계절변경 검침
- 현장 또는 원격으로 설정한 계절 TOU에 맞춰 계절변경 검침 실시
- 자동으로 검침 값 확정(SR)을 수행
- 계절변경 검침 기록값 : 누적전력량, 최대수요전력, 역률

저장항목
- '정기검침' : 6개월분 데이터
- '비 정기검침' : 4회분 데이터
- '계절변경 검침' : 4회분 데이터

저장특성
- 정기, 비 정기, 계절변경 검침 데이터를 각각 별도 변수로 저장
- 저장데이터의 항목은 저장하는 데이터는 정기검침의 데이터와 동일
*/
/*
lhh_add_desc :
1. rtc 를 sec 단위로 읽는다.
2. program 정보를 업데이트 한다.
3. end of bill 처리 한다.
4. 평균전압 전류 처리 한다.
5. daylight 처리한다.
6. eoi_proc (max demand 및 eoi time 설정) 수행한다.
7. hdlc/dlms/cosem 처리
8. mt_abnorm_mon - 비정상 적인 상태 체크 및 관련 설정을 한다.
*/

void update_rtc(void)
{
    uint8_t* tptr;
    tptr = adjust_tptr(&global_buff[0]);

    static uint16_t count;
    MSG00("rtc_read");
    rtc_read(); /* rtc_copy와 cur_rtc 업데이트 */

    if (run_is_bat_power())
    {
        if (dsm_pmnt_get_op_mode() != PMNT_NO_VOLT_SET_OP)
        {
            cur_rtc_update();
            return;
        }
    }

    whm_rtc_event(tptr);
}

void whm_proc(void)
{
    uint8_t* tptr;
    tptr = adjust_tptr(&global_buff[0]);

#if 1 /* bccho, 2023-09-24 */
    static uint16_t count;
    if ((count++ % 300) == 0)
    {
        MSG00("rtc_read#1");
        rtc_read(); /* rtc_copy와 cur_rtc 업데이트 */
        count = 1;

        if (run_is_bat_power())
        {
            if (dsm_pmnt_get_op_mode() != PMNT_NO_VOLT_SET_OP)
            {
                cur_rtc_update();
                return;
            }
        }
    }
#endif

    whm_rtc_event(tptr);

    prog_changed_event_clr();

    // program process
    program_proc(tptr);  // tou data update

    eob_proc(tptr);  // should be done after whm_rtc_event() and program_proc()
                     // due to b_eob_proc and season_chg_proc()
    // => should process before rtc is changed by dst_mon()
    LPavg_proc();
    LPrt_proc();
    // ==> should before eoi_proc() => eoi
    dst_mon(true, &cur_rtc, tptr);
    //
    eoi_proc(tptr);

    amr_proc();

    mt_abnorm_mon();
    sig_sel_out_proc();
    tamper_det_proc();
    imax_log_proc();
    tamper_det_proc();
    tmp_bat_meas();
    wrong_conn_mon();
    prepay_info_save(false);
    monitor_sag_swell();

#if 1  // jp.kim 25.02.04
    void dsm_mtp_fsm_send(void);
    bool sagswell_wr_rd_cmp(void);

    if (mif_meter_para_set_wait_ing)
    {
        if (mif_meter_para_set_wait_timeout() &&
            (dsm_mtp_get_fsm() != MTP_FSM_CAL_SET) &&
            (dsm_mtp_get_fsm() != MTP_FSM_CAL_GET))
        {
            dsm_mtp_set_fsm(MTP_FSM_PARM_GET);
            dsm_mtp_fsm_send();
            mif_meter_para_set_wait_ing = 0;
            mif_meter_para_get_wait_ing = 1;
            mif_meter_para_set_wait_timeset(T500MS);
        }
    }
    else if (mif_meter_para_get_wait_ing)
    {
        if (mif_meter_para_set_wait_timeout())
        {
            mif_meter_para_get_wait_ing = 0;

            if (!meter_parm_wr_rd_cmp())
            {
                mif_meter_para_set_wait_seq_cnt++;
                if (mif_meter_para_set_wait_seq_cnt < 3)
                {
                    dsm_mtp_set_fsm(MTP_FSM_PARM_SET);
                    dsm_mtp_fsm_send();
                    mif_meter_para_set_wait_ing = 1;
                    mif_meter_para_set_wait_timeset(T500MS);
                }
                else
                {
                    mif_meter_para_set_wait_ing = 0;
                    mif_meter_para_get_wait_ing = 0;
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                }
            }
            else
            {
                dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
            }
        }
    }

    // jp.kim 25.03.12
    if (mif_meter_sagswe_set_wait_ing)
    {
        if (mif_meter_sagswe_set_wait_timeout() &&
            (dsm_mtp_get_fsm() != MTP_FSM_CAL_SET) &&
            (dsm_mtp_get_fsm() != MTP_FSM_CAL_GET))
        {
            DPRINTF(DBG_TRACE,
                    " if(mif_meter_sagswe_set_wait_ing) "
                    "mif_meter_segswe_set_wait_timeout() "
                    "mif_meter_segswe_set_\r\n");

            dsm_mtp_set_fsm(MTP_FSM_SAG_SWELL_GET);
            dsm_mtp_fsm_send();
            mif_meter_sagswe_set_wait_ing = 0;
            mif_meter_sagswe_get_wait_ing = 1;
            mif_meter_sagswe_set_wait_timeset(T500MS);
        }
    }
    else if (mif_meter_sagswe_get_wait_ing)
    {
        if (mif_meter_sagswe_set_wait_timeout())
        {
            mif_meter_sagswe_get_wait_ing = 0;

            if (!sagswell_wr_rd_cmp())
            {
                DPRINTF(DBG_ERR,
                        "\r\nmeter sagswell get set com FAIL RETRY!!!! \r\n");
                mif_meter_sagswe_set_wait_seq_cnt++;
                if (mif_meter_sagswe_set_wait_seq_cnt < 3)
                {
                    dsm_mtp_set_fsm(MTP_FSM_SAG_SWELL);
                    dsm_mtp_fsm_send();
                    mif_meter_sagswe_set_wait_ing = 1;
                    mif_meter_sagswe_set_wait_timeset(T500MS);
                }
                else
                {
                    mif_meter_sagswe_set_wait_ing = 0;
                    mif_meter_sagswe_get_wait_ing = 0;
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                }
            }
            else
            {
                dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                DPRINTF(DBG_ERR, "\r\nmeter sagswell get set com ok  \r\n");
            }
        }
    }
#endif
}

void whm_data_save_sag(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    mt_conf_save();
    mt_conf_2_save();
    whm_op_sag_save();
    mt_accm_save();
    dm_intv_save();
    bat_used_time_backup(&cur_rtc);
    prepay_info_save(true);
    prog_dl_backup_save();
    tou_set_cnt_save();
    prog_dlcmd_avail = false;

    sag_flag_set();
#ifdef STOCK_OP /* bccho, 2024-09-26 */
    dsm_stock_op_times_write();
#endif
}

void whm_data_unsave_sag(void)
{
    DPRINTF(DBG_ERR, _D "%s\r\n", __func__);

    if (LP_index)
        LP_index -= 1;
    if (TOTAL_LP_EVENT_CNT)
        TOTAL_LP_EVENT_CNT -= 1L;

    LP_nextdt_set(&cur_rtc);
}

void whm_data_save_fromBM(void)
{
    mt_conf_save();
    mt_conf_2_save();
    whm_op_save();
}

void whm_bakup_crc_calc(void)
{
    mt_conf_backup_check();

    mt_conf_2_backup_check();

    whmop_bakup_crc_calc();

    mtaccm_bakup_crc_calc();
}

void set_cust_id(ser_no_type* serno)
{
    nv_write(I_SERIAL_NO, (uint8_t*)serno);

    dl_server_macaddr_set();
}

bool get_cust_id(uint8_t* tptr)
{
    ser_no_type serial_number;
    if (nv_read(I_SERIAL_NO, (uint8_t*)&serial_number))
    {
        memcpy(tptr, serial_number.ser, SERIAL_NO_SIZE);
        return true;
    }

    memset(tptr, '0', SERIAL_NO_SIZE);

    return false;
}

#ifdef M2354_CAN /* bccho, 2023-11-28 */
uint32_t Generate_CAN_ID(void)
{
    /*
    계기 CAN-ID(10진수 8자리) : 제조사 번호 3자리 + 시리얼 번호 6자리 중, 하위
    5자리 (0-99,999,999)
    // TODO: Check
    */
    uint8_t meter_id[MANUF_ID_SIZE + 1] = {0}, hash[32] = {0};
    uint32_t can_meter_id = 0;
    uint8_t cnt, rand_num = 0;

    get_manuf_id(meter_id);

    // MM TT 12 34567
    for (cnt = 6; cnt < MANUF_ID_SIZE; cnt++)  // last 5 digit of meter id
    {
        if (meter_id[cnt] != 0x30)
        {
            can_meter_id = strtoul((const char*)&meter_id[cnt], 0, 10);
            break;
        }
    }
    DPRINTF(DBG_TRACE, "CAN_MID: %05ld\r\n", can_meter_id);

    /*
       CAN ID 24bits : Random Number 7bits + Meter Serial 17bits
       Random Number : 계기 ID (제조사 번호 + 계기 형식 + 일련번호) 11자리로
       SHA256 값을 생성하여 최상위 7bit 사용 Meter Serial : 계기 ID 일련번호
       끝에서 5자리
    */
#ifdef M2354_CAN /* bccho, 2023-11-28 */
    if (axiocrypto_hash(HASH_SHA_256, meter_id, MANUF_ID_SIZE, hash, 32) !=
        CRYPTO_SUCCESS)
    {
        MSGERROR("Generate_CAN_ID, axiocrypto_hash()");
    }
#else
    // _kcmvpSha256(hash, meter_id, MANUF_ID_SIZE);
#endif

    rand_num = hash[0] >> 1;
    DPRINTF(DBG_TRACE, "CAN_RN: %02Xh\r\n", rand_num);

    can_meter_id |= (rand_num << 17);
    DPRINTF(DBG_TRACE, "CAN_ID: %06lXh\r\n", can_meter_id);

    return can_meter_id;
}
#endif

uint8_t get_server_hdlc_addr_to_dec(void)
{
    uint8_t sno0, sno1, ret_dec;
    ser_no_type serno;

    get_cust_id((uint8_t*)&serno);

    sno0 = serno.ser[SERIAL_NO_SIZE - 1] - '0';
    sno1 = serno.ser[SERIAL_NO_SIZE - 2] - '0';

    ret_dec = sno1 * 10 + sno0;

    DPRINTF(DBG_NONE, "dec[%d] = %d * 10 + %d\r\n", ret_dec, sno1, sno0);
    return ret_dec;
}

void get_manuf_id(uint8_t* tptr)
{
    /*
    Meter ID: XX530123456
        Company ID: XX (제조사 코드 두 자리는 숫자와 알파벳으로 구성)
        Meter Type: 53
        Meter Serial No.: 0123456
    */
    memset(tptr, '0', MANUF_ID_SIZE);

    *tptr++ = COMPANY_ID_1;
    *tptr++ = COMPANY_ID_2;
    *tptr++ = (METER_ID / 10) + '0';
    *tptr++ = (METER_ID % 10) + '0';

    get_cust_id(tptr);
}

void get_nms_dms_id(uint8_t* tptr)
{
    memset(tptr, '0', NMS_DMS_ID_SIZE);

    // dummy = 3
    tptr += 3;

    // manufacture id (구형 모뎀 연계용 미터 ID : byte 4~5번의 “00”을
    // ASCII로 표기, 제조사 번호가 영문 알파벳이 포함되어 인식을 못하기
    // 때문에 '0'으로 채움)
    tptr += 2;

    *tptr++ = (METER_ID / 10) + '0';
    *tptr++ = (METER_ID % 10) + '0';
}

void rtc_chg_proc(date_time_type* pdt, uint8_t* tptr)
{
    int8_t chg;
    uint32_t LPevt_backup;
    date_time_type bfdt;
    bool selreact_mon = false;
    uint8_t rcnt_season_id_back;
    uint8_t srdr = 0;
    uint8_t eob_type = 0;
    bool fut_exe_flag = false;

#if 1
    rcnt_season_id_back = rcnt_season_id;
    DPRINTF(DBG_ERR,
            "%s: rcnt_season_id_back = rcnt_season_id;  rcnt_season_id[%d], "
            "rcnt_season_id_back[%d]\r\n",
            __func__, rcnt_season_id, rcnt_season_id_back);
#endif

    bfdt = cur_rtc;
    DPRINTF(DBG_TRACE, _D "%s: Current Time %02d.%02d.%02d %02d:%02d:%02d\r\n",
            __func__, bfdt.year, bfdt.month, bfdt.date, bfdt.hour, bfdt.min,
            bfdt.sec);

    log_rtc_chg(&bfdt, pdt);

    if (run_is_main_power())
    {
        LP_event |= (LPE_IRRG_WR | LPE_TIME_CHG);
        LP_save(&bfdt);
    }
    else
    {
        LPevt_backup = LP_event;  // restored below
        LP_event = (LPE_IRRG_WR | LPE_TIME_CHG);
        LP_save(&bfdt);
        LP_event = LPevt_backup;
    }

    // 시간 변경하고 나서 rtc update 부분은 안전하게 ISR과 context switch
    // 일어나지 않도록. bccho, 2025-09-22
    vPortEnterCritical();
    write_rtc(pdt);
    cur_rtc_update();
    vPortExitCritical();

    LP_nextdt_set(&cur_rtc);

    // rtc_season_chg_chk = true;
    tou_update_by_rtchg(tptr);

    lp_event_set_curate();

    chg = tchg_is_within_peob(&bfdt, pdt);

    if (chg < 0)
    {
        DPRINTF(DBG_INFO, "%s: P_EOB SR Log Init\r\n", __func__);
        sr_log_dt_init();
    }
    if (prog_fut_is_available())
    {
        DPRINTF(DBG_INFO, "%s: Future Enable\r\n", __func__);
        fut_exe_flag = true;
    }

    prog_changed_event_clr();
    program_proc(tptr);  // 프로그램 변경 적용될 수 있음.

    // eoi process
    if (fut_exe_flag == true && (!prog_fut_is_available()))
    {
        // PROG_FUT_BIT is CLEAR
        DPRINTF(DBG_INFO, "%s: Future Disable\r\n", __func__);
        if (eoi_processed == 0)
        {
            DPRINTF(DBG_INFO, "%s: EOI_PROC is NULL\r\n", __func__);
            eoi_proc_timechg(&bfdt, eoi_rate, tptr, true);
        }
    }
    else  // fut_exe_flag is false or PROG_FUT_BIT is SET
    {
        eoi_proc_timechg(&bfdt, eoi_rate, tptr, false);
    }
    chg = tchg_is_within_peob(&bfdt, pdt);

    if (chg < 0)  // 검침일 이전으로 시간 변경을 하는 경우
    {
        DPRINTF(DBG_INFO, "%s: SR/DR P_EOB DT Init\r\n", __func__);
        sr_dt_init();
        dr_dt_init();
    }
    else
    {
        if (chg > 0)  // 검침일 혹은 그 이후로 시간 변경
        {
            DPRINTF(DBG_INFO, "%s: SR/DR P_EOB Set\r\n",
                    __func__);  // 구간 내 시간 과거

            srdr |= pEOB_sr_dr_type;
            selreact_mon = true;

            if (pEOB_sr_dr_type)
            {
                DPRINTF(DBG_INFO, "%s: P_EOB is Enable\r\n",
                        __func__);  // 구간 내 시간 과거
                eob_type |= EOB_PERIOD_FLAG;
            }
        }

        chg = eob_timechg_npEOB(
            &bfdt,
            pdt);     // 비정기 검침일이 설정되어 있는 경우 ?, 검침 주기 외 ?
        if (chg < 0)  // 비정기 검침일 이전으로 시간 변경을 하는 경우 ??
        {
            DPRINTF(DBG_INFO, "%s: SR/DR NP_EOB DT Init\r\n", __func__);
            sr_dt_init();
            dr_dt_init();
        }
        else if (chg > 0)  // 비정기 검침일 혹은 그 이후로 시간 변경 ??
        {
            DPRINTF(DBG_INFO, "%s: SR/DR NP_EOB Set\r\n", __func__);
            srdr |= npEOB_sr_dr_type;
            if (npEOB_sr_dr_type)
            {
                DPRINTF(DBG_INFO, "%s: NP_EOB is Enable\r\n", __func__);
                eob_type |= EOB_nPERIOD_FLAG;
            }
        }
    }

    DPRINTF(DBG_ERR,
            "%s: if(rcnt_season_id_back != rcnt_season_id)  "
            "rcnt_season_id[%d], rcnt_season_id_back[%d]\r\n",
            __func__, rcnt_season_id, rcnt_season_id_back);

    if (rcnt_season_id_back != rcnt_season_id)
        b_season_changed = true;
    else
        b_season_changed = false;

    if ((bfdt.year != 0) && prog_season_changed())
    {
        if (seasonCHG_sr_dr_type)
        {
            eob_type |= EOB_SEASON_FLAG;
        }
        srdr |= seasonCHG_sr_dr_type;
        prog_season_changed_clr();
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

    if ((srdr & (MR_SR_BIT | MR_DR_BIT)) != 0)
    {
        DPRINTF(DBG_INFO, "%s: 1-5 SR/DR Process %d\r\n", __func__,
                srdr);  // 구간 내 시간 과거 srdr 3
        sr_dr_proc(eob_type, srdr, pdt, tptr);
    }

    if (selreact_mon)
    {
        sel_react_monitor_rtchg(pdt);
    }

    dsp_circ_time_set(bfdt.sec, pdt->sec);

    timechg_LPavg_proc(&bfdt, pdt);  // LP Average

    timechg_LPrt_proc(&bfdt, pdt);  // LP RealTime

    dst_mon_rtchg(pdt);

    if (run_is_bat_power())
    {
        bat_used_time_re_set(&bfdt, &cur_rtc);
    }

    bat_inst_proc(BIEVT_TIMECHG);  // after update cur_rtc bcz cur_rtc is
                                   // used in bat_inst_proc()

    if (run_is_main_power())
    {
        timechg_meas_proc(tptr);
    }

    phol_nv_read(nPERIODIC_HOL);

    // rtc_season_chg_chk = false;
}

#if 1 /* bccho, 2024-01-10, 1227 포팅 */
void prog_tou_start(U8* tptr, bool curr_prog_in);
void prog_info_write_by_key(bool curprog, U8 progcnt, date_time_type* pdt,
                            U8* tptr);

void prog_chg_proc_by_key(U8 bfmr_date, ratekind_type bfkind,
                          meas_method_type bfmeas, U8* tptr)
{
    if (mt_rtkind != bfkind)
    {
        dsm_touimage_default(mt_rtkind);
        prog_tou_start(tptr, true);  // tou data update

#if 1  // jp.kim 24.10.29
        default_bill_parm(1);
        // mt_conf_save();
        prog_info_write_by_key(true, 1, &cur_rtc, tptr);
        log_prog_chg(&cur_rtc);
#endif
    }

    lp_event_set(LPE_PROGRAM_CHG);

    if ((mt_rtkind != bfkind) || (meas_method != bfmeas))
    {
        uint8_t eob_type = 0;

        if (pgmCHG_sr_dr_type)
            eob_type = EOB_nPERIOD_FLAG;

        sr_dr_proc(eob_type, (pgmCHG_sr_dr_type), &cur_rtc, tptr);
    }

    update_history_var(bfmr_date, bfkind);

    prog_in_state = E_PROG_KEY;
}
#else
void prog_chg_proc_by_key(uint8_t bfdir, ratekind_type bfkind,
                          meas_method_type bfmeas, uint8_t* tptr)
{
    uint32_t t32;
    rate_type bkup_rate;
    uint8_t eob_type = 0;
    // cur_rate should be updated later due to demand accumulation of
    // cur_rate
    bkup_rate = cur_rate;
    curr_rate_update();
    if (bkup_rate != cur_rate)
    {
        eoi_proc_ratechg(bkup_rate, tptr);
        cur_rate = bkup_rate;
    }

    if (run_is_main_power())
    {
        lp_event_set(LPE_PROGRAM_CHG);

        if ((mt_rtkind != bfkind) || (mt_dir != bfdir) ||
            (meas_method != bfmeas))
        {
            if (pgmCHG_sr_dr_type)
                eob_type = EOB_nPERIOD_FLAG;
            sr_dr_proc(eob_type, (pgmCHG_sr_dr_type), &cur_rtc, tptr);
        }
    }
    else
    {
        t32 = LP_event;
        LP_event = LPE_PROGRAM_CHG;

        if ((mt_rtkind != bfkind) || (mt_dir != bfdir) ||
            (meas_method != bfmeas))
        {
            if (pgmCHG_sr_dr_type)
                eob_type |= EOB_nPERIOD_FLAG;
            sr_dr_proc(eob_type, (pgmCHG_sr_dr_type), &cur_rtc, tptr);
        }
        LP_event = t32;
    }

    if (mt_rtkind != bfkind)
    {
        if (mt_is_one_ratekind())
        {
            tou_conf_init_onekind();
        }
    }

    update_history_var(bfdir, bfkind);

    prog_in_state = E_PROG_KEY;
}
#endif

void sr_dr_proc_PENDING(uint8_t eob_type, uint8_t srdr, date_time_type* pdt,
                        uint8_t* tptr)
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
}

/*
lhh_add_desc :
    1. program 정보를 NV에 write 한다.
    2. program log 를 NV에 write 한다.
    3. LP event 를 업데이트 한다.
    4. LP 를 저장 및 next LP date and time 을 설정한다.
    5. SR/DR를 수행한다.
*/
void prog_chg_proc_by_comm(prog_dl_type* progdl, uint8_t* tptr,
                           bool curr_prog_in)
{
    uint8_t eob_type = 0;

    // jp.kim 24.11.08 TOU 파일 통신 파일 다운로드 이력 표현
    tou_id_change_sts = 1;

    prog_info_write(true, 1, progdl, &cur_rtc, tptr);

    log_prog_chg(&cur_rtc);

    lp_event_set(LPE_PROGRAM_CHG);

    if (curr_prog_in)
    {
        if (pgmCHG_sr_dr_type)
            eob_type = EOB_nPERIOD_FLAG;
        sr_dr_proc(eob_type, pgmCHG_sr_dr_type, &cur_rtc, tptr);
    }
    else
    {
        if (pgmCHG_sr_dr_type)
            b_prog_changed_event = true;
        sr_dr_proc_PENDING(eob_type, pgmCHG_sr_dr_type, &cur_rtc, tptr);
    }
}

#if 0 /* bccho, 2024-01-10, 1227 포팅 */
void tou_conf_init_onekind(void)
{
    int i;

    tou_data_cnt = 1;
    for (i = 0; i < MAX_TOU_DIV_DLMS; i++)
    {
        memcpy((uint8_t *)&tou_data_conf[i], &tou_default_onekind[i],
               sizeof(tou_struct_type));
    }
}

void tou_conf_init_twokind(void)
{
    int i;

    tou_data_cnt = 2;
    for (i = 0; i < MAX_TOU_DIV_DLMS; i++)
    {
        memcpy((uint8_t *)&tou_data_conf[i], &tou_default_twokind[i],
               sizeof(tou_struct_type));
    }
}
#endif

void write_rtc(date_time_type* pdt)
{
    DPRINTF(DBG_TRACE, _D "%s: %02d.%02d.%02d %02d:%02d:%02d\r\n", __func__,
            pdt->year, pdt->month, pdt->date, pdt->hour, pdt->min, pdt->sec);

    RTC_YEAR = pdt->year + BASE_YEAR;
    RTC_MON = pdt->month;
    RTC_DATE = pdt->date;
    RTC_HOUR = pdt->hour;
    RTC_MIN = pdt->min;
    RTC_SEC = pdt->sec;

    rtc_write();
}

void reset_rtc(void)
{
    date_time_type dt;

#if 1 /* bccho, 2023-12-19 */
    nv_read(I_PWF_DATE_TIME, (U8*)&pwf_date_time);
    memcpy(&dt, (U8*)&pwf_date_time, sizeof(date_time_type));
#else
    set_start_time(&dt);
#endif

    write_rtc(&dt);
}

void get_date_time(date_time_type* pdt)
{
    struct tm SystemTime;
#if 0 /* bccho, 2023-12-19 */
    ST_TIME_BCD stITime;
    util_get_system_time(&SystemTime, &stITime);
#else
    util_get_system_time(&SystemTime, NULL);
#endif

    pdt->year = SystemTime.tm_year - 100;
    pdt->month = SystemTime.tm_mon + 1;
    pdt->date = SystemTime.tm_mday;
    pdt->hour = SystemTime.tm_hour;
    pdt->min = SystemTime.tm_min;
    pdt->sec = SystemTime.tm_sec;
}

void cur_rtc_update(void)
{
    MSG06("cur_rtc_update()");
    get_date_time(&cur_rtc);
}

bool cur_rtc_is_set(void) { return (cur_rtc.year != 0); }

void set_start_time(date_time_type* dt)
{
    memset((uint8_t*)dt, 0, sizeof(date_time_type));
    dt->month = 01;
    dt->date = 01;
}

static void timeshift_meas_proc(uint8_t* tptr)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
}

void pwrtn_meas_proc(uint8_t* tptr) { timeshift_meas_proc(tptr); }

void timechg_meas_proc(uint8_t* tptr) { timeshift_meas_proc(tptr); }

static uint8_t get_nv_version(void)
{
    uint8_t ver;

    ver = (ver_fw_nv[8] - '0') * 10;
    ver += (ver_fw_nv[9] - '0');
    return ver;
}

static void nv_header_chk(void)
{
    nv_version_type nver;

    if (!nv_read(I_VERSION, (uint8_t*)&nver))
    {
        MTStatus |= MT_FIRST_BOOT;
        MSG06("nv_header_chk--------I_VERSION");
        return;
    }

#if 1  // jp.kim 25.04.26
    if ((nver.ver == 0xff) && (nver.CRC_M == 0xffff))
    {
        MTStatus |= MT_FIRST_BOOT;
        return;
    }
#endif

    if (nver.ver == get_nv_version())
        return;

    if (nver.ver == NV_COMM_INITED)
    {
        MTStatus |= MT_INITED_BY_COMM;
    }
    else if (nver.ver == NV_PROD_INITED)
    {
        MTStatus |= MT_INITED_BY_PROD;
    }
    else
    {
        MTStatus |= MT_INITED_BY_VER;
    }
}

void nv_header_set(uint8_t ver)
{
    nv_version_type nver;

    nver.ver = ver;

    nv_write(I_VERSION, (uint8_t*)&nver);
}

static void nv_clear(void) { nv_write(I_ALL_CLEAR, (uint8_t*)NULL); }

static void sag_flag_set(void)
{
    uint8_t sagflag[SAG_FLAG_LENGTH];

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    sagflag[0] = 0x18;
    sagflag[1] = 0x29;
    sagflag[2] = 0x3a;
    sagflag[3] = 0x4b;
    sagflag[4] = 0x5c;
    sagflag[5] = 0x6d;
    sagflag[6] = 0x7e;
    sagflag[7] = 0x8f;

    nv_write(I_SAG_FLAG, sagflag);
}

static void sag_flag_unset(void)
{
    uint8_t sagflag[SAG_FLAG_LENGTH];

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    memset(sagflag, 0x12, SAG_FLAG_LENGTH);

    nv_write(I_SAG_FLAG, sagflag);
}

static void sag_flag_chk(void)
{
    uint8_t sagflag[SAG_FLAG_LENGTH];

    if (nv_read(I_SAG_FLAG, sagflag))
    {
        if (sagflag[0] == 0x18 && sagflag[1] == 0x29 && sagflag[2] == 0x3a &&
            sagflag[3] == 0x4b && sagflag[4] == 0x5c && sagflag[5] == 0x6d &&
            sagflag[6] == 0x7e && sagflag[7] == 0x8f)
        {
            MTStatus |= MT_SAGED;
        }
        else
        {
        }
    }
    else
    {
    }
}

void init_mif_task_init(bool firm_up_sts);
bool sagswell_wr_rd_cmp(void);

static void mt_data_restore(uint8_t* tptr)
{
    bool nv_inited = false;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    if (nv_read(I_TEMP_CAL_DATA, tptr))
    {
        temp_cal_data_type* temp_cal = (temp_cal_data_type*)tptr;
        temp_caled = temp_cal->T_cal_temp;
    }
    else
    {
        temp_caled = 0;
    }

    if (nv_read(I_ADJ_TEMP_DATA, tptr))
    {
        temp_adj_data_type* temp_adj = (temp_adj_data_type*)tptr;
        adj_currtemp = temp_adj->T_adj_temp;
    }
    else
    {
        adj_currtemp = 0.0;
    }

    if (nv_read(I_MAG_CAL_DATA, tptr))
    {
        mag_cal_data_type* mag_cal = (mag_cal_data_type*)tptr;
        cal_magnet = mag_cal->T_cal_magnet;
    }
    else
    {
        cal_magnet = MAGNET_DEFAULT_VALUE;
    }

    nv_header_chk();

    if (MTStatus & (MT_FIRST_BOOT | MT_INITED_BY_VER | MT_INITED_BY_COMM |
                    MT_INITED_BY_PROD))
    {
        DPRINTF(DBG_TRACE, _D "%s: Status[0x%08X] 1\r\n", __func__, MTStatus);

        MSG06("MT_FIRST_BOOT: %08X", MTStatus & MT_FIRST_BOOT);
        MSG06("MT_INITED_BY_VER: %08X", MTStatus & MT_INITED_BY_VER);

        dsm_wdt_ext_toggle_immd();
        nv_clear();
        nv_header_set(get_nv_version());
        nv_inited = true;
    }

    DPRINTF(DBG_TRACE, _D "Meter Init[%d]\r\n", nv_inited);

    if (nv_inited)
    {
        void mif_meter_parm_set(void);

        mt_conf_default();
        mt_conf_2_default();
        mt_conf_save();
        mt_conf_2_save();
        mif_meter_parm_set();   /* bccho, 2024-09-24, 삼상  */
        whm_op_default(false);  // jp.kim 24.11.07
        mt_accm_reset();
        dm_intv_init();
        prepay_reset();
        npBill_date_init();
        default_min_max_val();
        DPRINTF(DBG_TRACE, _D " default_min_max_val: %d\r\n",
                (uint16_t)min_freq);

        if (MTStatus & MT_FIRST_BOOT)
        {
        }
        if (MTStatus & MT_INITED_BY_VER)
        {
            reset_rtc();
            log_mt_init_clear(tptr);
        }
        if (MTStatus & (MT_FIRST_BOOT | MT_INITED_BY_VER | MT_INITED_BY_COMM |
                        MT_INITED_BY_PROD))
        {
/* bccho, 2024-09-05, 삼상 */
#if PHASE_NUM == SINGLE_PHASE
            dsm_touimage_default(ONE_RATE_KIND);
#else
            dsm_touimage_default(TWO_RATE_KIND);
#endif
        }

#ifdef STOCK_OP /* bccho, 2024-09-26 */
        dsm_stock_op_times_reset();
#endif

        tou_set_cnt_reset();

        dsm_imgtrfr_con_init(IMG__TOU);
        dsm_imgtrfr_touimage_info_save();

        dsm_sys_fwinfo_initial_set(false);

        dsm_imgtrfr_fw_subimage_restore();

        dsm_modemif_default_baudrate();

        init_mif_task_init(0);
    }
    else
    {
        bool b_saged;
        sag_flag_chk();
        b_saged = (MTStatus & MT_SAGED) ? true : false;
        DPRINTF(DBG_TRACE, _D "SAG occurred[%d]\r\n", b_saged);
        mt_conf_restore(b_saged);
        mt_conf_2_restore(b_saged);
        whm_op_restore(b_saged);
        mt_accm_restore(b_saged);

        supp_dsp_restore();

        dm_intv_load();
        npBill_date_load();  // after mt_conf is loaded !!!
        min_max_load();
        prepay_info_restore(b_saged);
        tou_set_cnt_restore();
        dsm_imgtrfr_touimage_restore();
#ifdef STOCK_OP /* bccho, 2024-09-26 */
        dsm_stock_op_times_read();
#endif
    }

    bat_usedtime_restore();

    DPRINTF(DBG_TRACE, _D "%s: MTStatus[0x%08X] END\r\n", __func__, MTStatus);
}

static void pwrfail_info_get(pwrfail_info_type* pfinfo)
{
    pfinfo->dt = pwrfail_dt;
    pfinfo->rtkind = mt_rtkind;
    pfinfo->mtdir = mt_dir;
    pfinfo->accrt = accm_rate;
    pfinfo->lpevt = LP_event;
}

static void mt_conf_backup_check(void)
{
    if (!crc16_chk((uint8_t*)&mt_conf, sizeof(mt_conf_type), false))
    {
        mt_conf_save();
    }
}

static void mt_conf_2_backup_check(void)
{
    if (!crc16_chk((uint8_t*)&mt_conf_2, sizeof(mt_conf_2_type), false))
    {
        mt_conf_2_save();
    }
}

static void whmop_bakup_crc_calc(void)
{
    if (b_whmop_prd_bakup)
    {
        b_whmop_prd_bakup = false;
        whm_op_save();
    }
    else
    {
        crc16_chk((uint8_t*)&whm_op, sizeof(whm_op_type), true);
    }
}

static void mtaccm_bakup_crc_calc(void)
{
    if (b_mtaccm_prd_bakup)
    {
        b_mtaccm_prd_bakup = false;
        mt_accm_save();
    }
    else
    {
        crc16_chk((uint8_t*)&mt_accm, sizeof(mt_acc_type), true);
    }
}

/* bccho, 2024-09-05, 삼상 */
static void set_mt_ratekind_default(void)
{
#if PHASE_NUM == SINGLE_PHASE
    mt_rtkind = ONE_RATE_KIND;
#else
    mt_rtkind = TWO_RATE_KIND;
#endif

    // tou_id_change_sts = 0;  // 24.10.08
}

#if 1  // jp.kim 24.10.29
static void default_bill_parm(bool ket_set)
#else
static void default_bill_parm(void)
#endif
{
    MmodeCHG_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
    dmintvCHG_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
    mDR_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
    pgmCHG_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
    timCHG_sr_dr_type = 0;
    seasonCHG_sr_dr_type = 0;

    if (!ket_set)  // jp.kim 24.10.29
    {
        mt_dir = DEFAULT_SEL_ACT;
        mt_selreact = DEFAULT_SEL_REACT;
        mr_selreact = DEFAULT_SEL_REACT;
        g_mtp_meter_parm.direct_reverse = mt_dir;
        g_mtp_meter_parm.reactive_select = mt_selreact;
        mDR_limit_sec = DR_LIMIT_TIME;
    }
}

void default_scurr_parm(void)
{
    scurr_limit_level = DEFAULT_sCURR_VAL;
    scurr_limit_level_2 = DEFAULT_sCURR_VAL;
    scurr_autortn_cnt = DEFAULT_sCURR_AUTORTN;
    scurr_det_hold = 5;     // 5 sec
    scurr_rtn_dur_1 = 60;   // 60 sec
    scurr_rtn_dur_2 = 600;  // 600 sec
    scurr_cnt_n1 = 3;       // 3 회
}

void mt_conf_default(void)
{
    reg_mr_date = 1;
    dm_interval = DEFAULT_DM_INTERVAL;
    dm_period_num = 1;
    dm_sub_interval = DEFAULT_DM_INTERVAL;
    lp_interval = DEFAULT_LP_INTERVAL;
    lpavg_interval = DEFAULT_LPavg_INTERVAL;
    sig_sel = DEFAULT_SIG_SEL;
    meas_method = DEFAULT_MEAS_METHOD;

    /* bccho, 2024-09-24 */
    g_mtp_meter_parm.meter_method = meas_method;
    err_pulse_react = DEFAULT_ERR_PULSE;
    g_mtp_meter_parm.pulse_select = err_pulse_react;  // jp.kim 24.10.30
    condensor_en = 0;

    rtc_shift_range = DEFAULT_shftrng;
    ct_ratio_prog = DEFAULT_ct_ratio_prog;
    pt_ratio_prog = DEFAULT_pt_ratio_prog;
    self_error_ref = DEFAULT_self_error_ref;
    zcrs_sig_out_dur = DEFAULT_zcrs_sig_out_dur;
    // zcrs_sig_out_cmps		=		DEFAULT_zcrs_sig_out_cmps;
    // //jp.kim 24.11.14 상수값 사용
    auto_mode_sel = DEFAULT_auto_mode_sel;
    holiday_sel1 = DEFAULT_holiday_sel;
    thd_rec_period = DEFAULT_thd_rec_period;

    lcd_dsp_mode = DEFAULT_DSP_MODE;
    lcd_set_parm = DEFAULT_LCD_SET_PARM;
    mdm_baud = BAUD_9600;
    dls_info.enabled = false;
    pEOB_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
    npEOB_sr_dr_type = MR_SR_BIT | MR_DR_BIT;

    prog_available = 0;
    currprog_available_bits = 0;
    futprog_available_bits = 0;
    aprog_area = E_AREA_0;
    pprog_area = E_AREA_0;
    ahol_area = E_AREA_0;
    phol_area = E_AREA_0;

    comm_en_coveropen = 1;
    overcurr_cut_en = 0;

    /* bccho, 2024-09-05, 삼상 */
    set_mt_ratekind_default();

    default_bill_parm(0);  // jp.kim 24.10.29

    default_ts_zone();

    default_scurr_parm();

    rt_lp_interval = DEFAULT_RT_LP_INTERVAL;

    working_fault_min = DEFAULT_working_fault_min;

    dsm_push_err_code_set();  // jp.kim 24.10.22 추가

    nv_write(I_MTP_PARM, (uint8_t*)&g_mtp_meter_parm);  // jp.kim 24.10.30
}

void mt_conf_2_default(void) { futprog_partition_tou = 0; }

void default_ts_zone(void)
{
    ts_conf_cnt = 2;
    ts_conf_ctrl = 0x02;

    ts_conf_zone[0].hour = 9;
    ts_conf_zone[0].min = 0;
    ts_conf_zone[1].hour = 23;
    ts_conf_zone[1].min = 0;
}

static void mt_conf_restore(bool saged)
{
    if (!saged)
    {
        if (crc16_chk((uint8_t*)&mt_conf, sizeof(mt_conf_type), false))
        {
            return;
        }
    }
    if (!nv_read(I_MT_CONFIG, (uint8_t*)&mt_conf))
    {
        if ((reg_mr_date < 1) || (reg_mr_date > 30))
            reg_mr_date = 1;
        if ((dm_interval < 1) || (dm_interval > 60))
            dm_interval = DEFAULT_DM_INTERVAL;

        dm_period_num = 1;  // block interval
        dm_sub_interval = dm_interval;
        if ((lp_interval < 1) || (lp_interval > 60))
            lp_interval = DEFAULT_LP_INTERVAL;
        if ((lpavg_interval < 1) || (lpavg_interval > 60))
            lpavg_interval = DEFAULT_LPavg_INTERVAL;
        if ((sig_sel > 16))
            sig_sel = DEFAULT_SIG_SEL;

        if ((meas_method > E_BASIC))
            meas_method = DEFAULT_MEAS_METHOD;
        g_mtp_meter_parm.meter_method = meas_method;

        if ((err_pulse_react >= 3))
            err_pulse_react = DEFAULT_ERR_PULSE;
        g_mtp_meter_parm.pulse_select = err_pulse_react;

        if ((condensor_en > 1))
            condensor_en = 0;

        rtc_shift_range = DEFAULT_shftrng;
        ct_ratio_prog = DEFAULT_ct_ratio_prog;
        pt_ratio_prog = DEFAULT_pt_ratio_prog;
        self_error_ref = DEFAULT_self_error_ref;
        zcrs_sig_out_dur = DEFAULT_zcrs_sig_out_dur;
        auto_mode_sel = DEFAULT_auto_mode_sel;
        holiday_sel1 = DEFAULT_holiday_sel;
        thd_rec_period = DEFAULT_thd_rec_period;

        lcd_dsp_mode = DEFAULT_DSP_MODE;
        lcd_set_parm = DEFAULT_LCD_SET_PARM;

        if (mdm_baud > BAUD_115200)
            mdm_baud = BAUD_9600;
        dls_info.enabled = false;
        pEOB_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
        npEOB_sr_dr_type = MR_SR_BIT | MR_DR_BIT;

        futprog_available_bits = 0;  // 14.8.1

        if (aprog_area >= NUM_PGM_AREA)
            aprog_area = E_AREA_0;
        if (pprog_area >= NUM_PGM_AREA)
            pprog_area = E_AREA_0;
        if (ahol_area >= NUM_PGM_AREA)
            ahol_area = E_AREA_0;
        if (phol_area >= NUM_PGM_AREA)
            phol_area = E_AREA_0;

        comm_en_coveropen = 1;

        overcurr_cut_en = 0;

        if (mt_rtkind >= NUM_RATE_KIND)
            set_mt_ratekind_default();

        MmodeCHG_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
        dmintvCHG_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
        mDR_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
        pgmCHG_sr_dr_type = MR_SR_BIT | MR_DR_BIT;
        timCHG_sr_dr_type = 0;
        seasonCHG_sr_dr_type = 0;

        if (mt_dir > 2)
            mt_dir = DEFAULT_SEL_ACT;
        if (mt_selreact > 5)
            mt_selreact = DEFAULT_SEL_REACT;
        if (mr_selreact > 5)
            mr_selreact = DEFAULT_SEL_REACT;  //
        g_mtp_meter_parm.direct_reverse = mt_dir;
        g_mtp_meter_parm.reactive_select = mt_selreact;
        mDR_limit_sec = DR_LIMIT_TIME;

        default_ts_zone();
        default_scurr_parm();

        if ((rt_lp_interval < 1) || (rt_lp_interval > 60))
            rt_lp_interval = DEFAULT_RT_LP_INTERVAL;
        if ((working_fault_min < 1) || (working_fault_min > 60))
            working_fault_min = DEFAULT_working_fault_min;

        nv_write(I_MTP_PARM, (uint8_t*)&g_mtp_meter_parm);

        DPRINTF(DBG_ERR,
                _D
                "%s: if(!nv_read(I_MT_CONFIG, (U8 *)&mt_conf))  RESTORE CRC "
                "ERROR!!   \r\n",
                __func__);
    }
    else
    {
        DPRINTF(DBG_TRACE, _D "%s: Restored\r\n", __func__);
    }

    if (holiday_sel1)
    {
        DPRINTF(DBG_TRACE, _D "%s: Holiday Apply\r\n", __func__);
    }
    else
    {
        DPRINTF(DBG_TRACE, _D "%s: Holiday Not Apply\r\n", __func__);
    }
}

mt_conf_type* mt_conf_get(void) { return &mt_conf; }

void mt_conf_save(void) { nv_write(I_MT_CONFIG, (uint8_t*)&mt_conf); }

void mt_conf_2_save(void) { nv_write(I_MT_CONFIG_2, (uint8_t*)&mt_conf_2); }

static void mt_conf_2_restore(bool saged)
{
    if (!saged)
    {
        if (crc16_chk((uint8_t*)&mt_conf_2, sizeof(mt_conf_2_type), false))
        {
            return;
        }
    }
    if (!nv_read(I_MT_CONFIG_2, (uint8_t*)&mt_conf_2))
    {
        mt_conf_2_default();
        DPRINTF(DBG_ERR,
                _D
                "%s: if(!nv_read(I_MT_CONFIG_2, (U8 *)&mt_conf_2))  RESTORE "
                "CRC ERROR!!   \r\n",
                __func__);
    }
    else
    {
        DPRINTF(DBG_TRACE, _D "%s: Restored\r\n", __func__);
    }
}

static void whm_op_default(bool default_relay_on)  // jp.kim 24.11.07
{
    memset((uint8_t*)&whm_op, 0, sizeof(whm_op_type));

    if (MTStatus & MT_INITED_BY_COMM)
    {
        whm_op.progin = E_PROG_INIT;
    }
    else
    {
        whm_op.progin = E_PROG_NONE;
    }

    whm_op.tempthrshld = DEFAULT_TEMP_THRSHLD;

    if (default_relay_on)  // jp.kim 24.11.07
    {
        whm_op.ldinited = 1;
        whm_op.ldctrl = LOAD_ON;
    }
    else
    {
        whm_op.ldctrl = LOAD_OFF;
    }

    rcntdm_wear_idx = 0xff;
    eoi_rate = numRates;   // uninitialized
    sel_react_yr = 0x00;   // not set
    sel_react_mon = 0x00;  // not set

    tou_id_change_sts = 2;  // 24.10.31
}

static void whm_op_restore(bool saged)
{
    if (!saged)
    {
        if (crc16_chk((uint8_t*)&whm_op, sizeof(whm_op_type), false))
        {
            return;
        }
    }

    if (!nv_read(I_WHM_OP, (uint8_t*)&whm_op))
    {
        whm_op.tempthrshld = DEFAULT_TEMP_THRSHLD;
        whm_op.ldinited = 1;  // relay on
        whm_op.ldctrl = LOAD_ON;

        if (rcntdm_wear_idx >= NUM_CAP_WEAR)
        {
            rcntdm_wear_idx = 0;
        }
        if (eoi_rate >= numRates)
        {
            eoi_rate = cur_rate;
        }
        DPRINTF(DBG_ERR,
                _D
                "%s: if(!nv_read(I_MT_CONFIG_2, (U8 *)&mt_conf_2))  RESTORE "
                "CRC ERROR!!   \r\n",
                __func__);
    }

    DPRINTF(DBG_INFO, _D "%s: relay[%d] inited[%d]\r\n", __func__,
            whm_op.ldctrl, whm_op.ldinited);
}

whm_op_type* whm_op_get(void) { return &whm_op; }

void whm_op_save(void)
{
    nv_write(I_WHM_OP, (uint8_t*)&whm_op);
    DPRINTF(DBG_NONE, "%s\r\n", __func__);
}

void whm_op_sag_save(void)
{
    pwrfail_dt = cur_rtc;

    LP_event |= LPE_PWR_FAIL;

    whm_op_save();

    memcpy((U8*)&pwf_date_time, (U8*)&cur_rtc, sizeof(date_time_type));
    nv_write(I_PWF_DATE_TIME, (U8*)&pwf_date_time);
}

static void mt_accm_reset(void)
{
    memset((uint8_t*)&mt_accm, 0, sizeof(mt_acc_type));

    intv_cumaccm_idx = -1;
    intv_dmaccm_idx = -1;

    lp_intv_pf[eDeliAct] = (float)-1.0;
    lp_intv_pf[eReceiAct] = (float)-1.0;
}

static void mt_accm_restore(bool saged)
{
    if (!saged)
    {
        if (crc16_chk((uint8_t*)&mt_accm, sizeof(mt_acc_type), false))
            return;
    }

    if (!nv_read(I_MT_ACCM, (uint8_t*)&mt_accm))
    {
        DPRINTF(DBG_ERR,
                _D
                "%s: if(!nv_read(I_MT_ACCM, (U8 *)&mt_accm)))  RESTORE CRC "
                "ERROR!!   \r\n",
                __func__);
    }
    LPrt_reset();
}

static void mt_accm_save(void) { nv_write(I_MT_ACCM, (uint8_t*)&mt_accm); }

static void bat_usedtime_restore(void)
{
    if (!nv_read(I_BAT_USE, (uint8_t*)&bat_used_time))
        bat_used_time_clear();
}

static void supp_dsp_restore(void)
{
    if (!nv_read(I_DISP_SUPP_A, (uint8_t*)&circ_state_suppdsp_mode))
    {
        memset((uint8_t*)&circ_state_suppdsp_mode, 0, sizeof(dsp_supply_type));
    }
}

static void whm_rtc_event(uint8_t* tptr)
{
    bool b_sec, b_min, b_date;

    b_sec = false;
    b_min = false;
    b_date = false;

    if (RTC_SEC != cur_sec)
        b_sec = true;
    if (RTC_MIN != cur_min)
        b_min = true;
    if (RTC_DATE != cur_date)
        b_date = true;

    if (b_sec)
    {
        MSG00("b_sec: %d", rtc_copy.tm_sec);
        cur_rtc.year = rtc_copy.tm_year - 100;
        cur_rtc.month = rtc_copy.tm_mon + 1;
        cur_rtc.date = rtc_copy.tm_mday;
        cur_rtc.hour = rtc_copy.tm_hour;
        cur_rtc.min = rtc_copy.tm_min;
        cur_rtc.sec = rtc_copy.tm_sec;

        DR_limit_dec();

        b_lprt_monitor = true;
    }

    if (b_min)
    {
        MSG00("b_min:%d", cur_min);
        b_lpavg_monitor = true;
        b_lp_pf_monitor = TRUE;

        if (cur_hour == 3 || cur_hour == 15)
        {
            if (cur_min == 12)
            {
                b_mtaccm_prd_bakup = true;
            }
            else if (cur_min == 18)
            {
                b_whmop_prd_bakup = true;
            }
        }
    }

    b_eob_proc = false;
    if (b_date)
    {
        MSG00("b_date");
        b_eob_proc = true;
    }
}

static void update_history_var(U8 mr_date, ratekind_type kind)
{
    if (mr_date > mr_date_history)
        mr_date_history = mr_date;
    if (kind > rtkind_history)
        rtkind_history = kind;
}
