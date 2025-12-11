#ifndef WHM_H
#define WHM_H 1

#include "phy.h"
#include "options_def.h"

typedef enum
{
    eAppRstByNoReason,
    eAppRstByPwrOnWhenBatMode,
    eAppRstByInitialization

} app_rst_reason_type;

extern const /*__code*/ U8 COSEM_METER_ID_VER[];
extern const /*DEFAULT_zcrs_sig_out_cmps*/ U8 zcrs_sig[];

#define RUN_MAIN_POWER 0
#define RUN_BAT_POWER 1

#define run_set_main_power() (run_mode = RUN_MAIN_POWER)
#define run_set_bat_power() (run_mode = RUN_BAT_POWER)
#define run_is_main_power() (run_mode == RUN_MAIN_POWER) /* 전원 공급 상태 */
#define run_is_bat_power() (run_mode == RUN_BAT_POWER)   /* 무전압 상태 */

extern int run_mode;

#define NV_COMM_INITED 0xaa
#define NV_PROD_INITED 0xbb

#define DEFAULT_TEMP_CAL_DATA 23.0

#define SAG_FLAG_LENGTH 8

typedef enum
{
    eBOOT_FIRST,
    eBOOT_INITED,
    eBOOT_SAGED,
    eBOOT_WD_DETECTED,
    eBOOT_OTHER  // includes forcedly reset
} boot_reason_type;

// #define GLOBAL_BUFF_SIZE (1024 + 100)
#define GLOBAL_BUFF_SIZE (1500)
#define CLI_BUFF_SIZE (1024 + 60)
#define TX_BUFF_SIZE (1024 + 100)

typedef struct
{
    uint8_t _global[GLOBAL_BUFF_SIZE];
    uint8_t _cli[CLI_BUFF_SIZE];
    uint8_t _tx[TX_BUFF_SIZE];
} buff_struct_type;

#define global_buff ubuff._global
#define cli_buff ubuff._cli
#define tx_buff ubuff._tx

#define SEG_BUFF_SIZE (1024 + 270)

#define BASE_YEAR 2000
#define DEFAULT_YEAR 0
#define DEFAULT_MONTH 1
#define DEFAULT_DATE 1

enum touDAY
{
    _MON = 1,
    _TUE,
    _WED,
    _THU,
    _FRI,
    _SAT,
    _SUN
};

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} date_time_type;

typedef struct
{
    date_time_type dt;
    enum touDAY day;
} day_time_type;

typedef struct
{
    uint8_t year;
    uint8_t month;
} yy_mm_type;

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t date;
} yy_mm_dd_type;

typedef struct
{
    uint8_t hour;
    uint8_t min;
} hh_mm_type;

typedef struct
{
    uint8_t month;
    uint8_t date;
} mm_dd_type;

//
typedef enum
{
    /* 수전 : 유효전력, 지상 무효전력, 진상 무효전력, 피상전력 */
    eChDeliAct,
    eChDLagReact,
    eChDLeadReact,
    eChDeliApp,
    /* 송전 : 유효전력, 진상 무효전력, 지상 무효전력, 피상전력 */
    eChReceiAct,
    eChRLeadReact,
    eChRLagReact,
    eChReceiApp,
    numCHs
} energy_ch_type;

/* 최대수요전력 계산
단위 : kW, kVA
적용 범위: 수전 유효전력, 수전 피상전력, 송전 유효전력, 송전 피상전력
산출 방법: Block Demand : 설정된 수요 시한 동안 사용된 수요전력, 수요 시한 마감
시점마다 산출 */
typedef enum
{
    /* 수전 : 유효, 피상 */
    eDmChDeliAct,
    eDmChDeliApp,
    /* 송전 : 유효, 피상 */
    eDmChReceiAct,
    eDmChReceiApp,
    numDmCHs
} demand_ch_type;

typedef enum
{
    eArate,
    eBrate,
    eCrate,
    eDrate,
    eTrate,
    numRates
} rate_type;

typedef enum
{
    /* 1종, 2종, 3종, 4종 */
    ONE_RATE_KIND,
    TWO_RATE_KIND,
    THREE_RATE_KIND,
    FOUR_RATE_KIND,
    NUM_RATE_KIND
} ratekind_type;

#define SELECTOR_TO_RATE(sel) (rate_type)((sel & 0x0f) - 1)
#define TS_TO_SELECTOR(idx, onff) ((onff == SW_CTRL_ON) ? (0x01 << idx) : 0x00)
#define SELECTOR_TO_TS(sel)                                          \
    (time_sw_state_type)((ts_conf_ctrl & (0x01 << sel)) ? SW_CTRL_ON \
                                                        : SW_CTRL_OFF)

#define RATE_TS_TO_SELECTOR(rt, ts) (uint8_t)(rt + 1)

// groupE 의 값과 순서가 일치 해야 함
typedef enum
{
    /* 통신 규격 3.4.2.6.1.1 이력 기록 OBIS 코드 참조 (e1 - 1의 값) */

    eLogPwrF /*정전- 작업정전*/,
    eLogPwrR /*복전- 작업복전*/,
    eLogRtcF /*시간 변경(From)*/,
    eLogRtcT /*시간 변경(To)*/,
    eLogDRa /*수요전력 복귀*/,
    eLogDRm /*수동검침*/,
    eLogSR /*자기 검침(SR)*/,
    eLogPgmChg /*프로그램 변경*/,
    eLogSCurrLimit /*부하 제한*/,
    eLogCoverOpen,
    eLogMagnetDet /*자계 감지*/,
    eLogrLoadCtrl /*원격부하 개폐*/,
    eLogSCurrNonSel /*부하 제한 해제*/,
    eLogTCoverOpen /*단자 커버 열림*/,
    eLogMtInit /*초기화*/,
    eLogWrongConn /*오 결선*/,
    /*
    9, 10, 13, 20, 21 is not used
    17 == eLogSCurrLimit,
    18 == eLogSCurrNonSel
    */
    eLogErrDiagonist /*자기 오차진단*/,
    numLog1s,
    eLogSag = numLog1s,
    eLogSwell,
    eLogSysSwUp /*운영부 업데이트*/,
    eLogMtrSwUp /*계량부 업데이트*/,
    eLogInModemUp /*내장모뎀 업데이트*/,
    eLogExModemUp /*착탈형모뎀 업데이트*/,

    // jp.kim 24.10.28
    eLogWorkPwrF,       // 작업정전
    eLogPwrF_WorkPwrF,  // 총정전
    eLogPwrR_WorkPwrR,  // 총복전

    numLogs
} elog_kind_type;

typedef enum
{
    /* 통신 규격 3.4.2.6.1.1 이력 기록 OBIS 코드 참조 (e2 - 1의 값) */
    eLogSagOcur /*Sag 발생*/,
    eLogSwellOcur /*Swell 발생*/,
    eLogMagnetDetOcur,
    numLog2s
} elog_durtime_kind_type;

typedef enum
{
    eLogCert_NG,
    eLogCert_MAX
} elog_cert_kind_type;

typedef struct
{
    uint8_t len;
    uint8_t pwd[12];
} auth_val_type;

typedef struct
{
    auth_val_type pwd;
    uint16_t CRC_M;
} auth_pwd_type;
typedef struct
{
    uint8_t len;
    uint8_t random[64];
} auth_val_asso3_type;

typedef struct
{
    date_time_type dt;
    uint8_t mtdir;
    ratekind_type rtkind;
    uint32_t lpevt;
    rate_type accrt;
} pwrfail_info_type;

#define SERIAL_NO_SIZE 7
#define MANUF_ID_SIZE 11
#define MANUF_ID_SIZE_OLD 7
#define NMS_DMS_ID_SIZE 7
#define DEVICE_ID_SIZE 16

#define DEFAULT_DM_INTERVAL 15
#define DEFAULT_LP_INTERVAL DEFAULT_LP_INT_VAL
#define DEFAULT_LPavg_INTERVAL 15
#define DEFAULT_RT_LP_INTERVAL DEFAULT_RT_LP_INT_VAL

#define DEFAULT_shftrng 10
#define DEFAULT_ct_ratio_prog 1
#define DEFAULT_pt_ratio_prog 1
#define DEFAULT_self_error_ref (-10L)
#define DEFAULT_zcrs_sig_out_dur 0
#if 0                                // jp.kim 24.11.12
#define DEFAULT_zcrs_sig_out_cmps 1  // ms
#else
#define DEFAULT_zcrs_sig_out_cmps                                \
    (zcrs_sig[0] * 1000 + zcrs_sig[1] * 100 + zcrs_sig[2] * 10 + \
     zcrs_sig[3])  // 1000       //us
#endif
#define DEFAULT_auto_mode_sel 1
#define DEFAULT_holiday_sel 0
#define DEFAULT_thd_rec_period 10

#define LCD_USER_DSPMODE 0
#define LCD_SUPP_DSPMODE 1
#define DEFAULT_DSP_MODE LCD_USER_DSPMODE

#define DEFAULT_LCD_SET_PARM 0x00  // non-simple display mode
#define DR_LIMIT_TIME 30 * 60      // 30 min

#define DEFAULT_working_fault_min 30

typedef struct
{
    uint8_t ser[SERIAL_NO_SIZE];
    uint16_t CRC_M;
} ser_no_type;

typedef struct
{
    uint8_t devid[DEVICE_ID_SIZE];
    uint16_t CRC_M;
} device_id_type;

typedef struct
{
    uint8_t manfid[MANUF_ID_SIZE];
    uint16_t CRC_M;
} manufact_id_type;

typedef struct
{
    int32_t T_cal_i0;
    int32_t T_cal_v0;
    int32_t T_cal_i1;
    int32_t T_cal_v1;
    int32_t T_cal_i2;
    int32_t T_cal_v2;
    int32_t T_cal_p0;
    int32_t T_cal_p1;
    int32_t T_cal_p2;
    uint16_t CRC_M;
} cal_data_type;

typedef struct
{
    int32_t T_cal_magnet;
    uint16_t dummy;
    uint16_t CRC_M;
} mag_cal_data_type;

typedef struct
{
    float T_cal_temp;
    uint8_t dummy[3];
    uint16_t CRC_M;
} temp_cal_data_type;

typedef struct
{
    float T_adj_temp;
    uint8_t dummy[3];
    uint16_t CRC_M;
} temp_adj_data_type;

// ------------- SR/DR -----------------
typedef enum
{
    E_pEOB /* 정기 검침 */,
    E_npEOB /* 비정기 검침 */,
    E_mDR /* 수동 검침(전력량 확정) */,
    E_pgmCHG /* 프로그램 변경 */,
    E_timCHG /* 날짜/시간 변경 */,
    E_MmodeCHG /* 계량 모드 변경 */,
    E_seasonCHG /* 계절 변경 검침 */,
    E_dmintvCHG /* 수요시간 주기 변경 */,
    numSrDrKind
} eSrDr_kind_type;

#define SR_DR_KIND_NUM numSrDrKind

#define MAX_TOU_DIV_TWOKIND 4
#define MAX_TOU_DIV_DLMS 12
#define LOG_RECORD_LEN 10
#define LOG_BUFF_SIZE (LOG_RECORD_LEN + 4)
#define PROG_ID_SIZE 8

#define LOG_CERT_RECORD_LEN 50
#define LOG_CERT_BUFF_SIZE (LOG_CERT_RECORD_LEN + 4)

#define BILLING_PARM_SIZE 8

#define PGM_NAME_LEN \
    PROG_ID_SIZE  // program name length   => must even to align into integer
                  // type in structure
#define SEASON_PROF_SIZE 4               // season profile array size
#define SEASON_NAME_LEN 1                // season name length
#define WEEK_PROF_SIZE 4                 // week profile array size
#define WEEK_NAME_LEN 1                  // week name length
#define WEEK_LEN 7                       // week length (monday..sunday)
#define DAY_PROF_SIZE 10                 // day profile array size
#define DAY_STRUCT_LEN MAX_TOU_DIV_DLMS  // day struct array length
#define OBIS_NAME_LEN 6                  // OBIS length
#define HOLIDAY_BLOCK_LEN 21             // holiday_block number (1~21)
#define HOLIDAYS_PER_BLOCK_2 10          // (holidays per block)/2
#define HOLIDAYS_PER_BLOCK 20            // holidays per block
#define HOLIDAY_LEN \
    (HOLIDAY_BLOCK_LEN * HOLIDAYS_PER_BLOCK)  // total holidays(0x26c)
#define DATE_TAG_LEN 5
#define TIME_TAG_LEN 4
#define DATE_TIME_TAG_LEN 12
#define NP_BILLDATE_LEN 12
#define TMP_HOLIDAYS_LEN 5

#define KEY_INFO_SIZE 3
#define KEY_DATA_SIZE 128

typedef enum
{
    E_PROG_NONE,
    E_PROG_INIT,
    E_PROG_KEY,
    E_PROG_COMM
} prog_in_type;

typedef enum
{
    SIG_NOSEL = 0,
    SIG_rLOAD_CONTROL = 4,
    SIG_TS_CONTROL = 8,
    SIG_sCURR_LIMIT = 16
} ad_sig_type;

typedef enum
{
    /* 수전 및 송/수전 모드(S), 수전 및 송/수전 합성모드(U), 수전 단방향(D) */
    E_BASIC,
    E_VECTSUM,
    E_SINGLE_DIR
} meas_method_type;

#if PHASE_NUM == SINGLE_PHASE
#define DEFAULT_MEAS_METHOD E_BASIC
#else
#define DEFAULT_MEAS_METHOD E_VECTSUM
#endif

#define DEFAULT_ERR_PULSE 1
#define DEFAULT_SIG_SEL SIG_NOSEL
#define DEFAULT_sCURR_VAL 660  // 660 W
#define DEFAULT_sCURR_AUTORTN 0
// 구매 규격 4.1 납품 시 요구사항 : 비정상 기준 온도 설정 80˚C
#define DEFAULT_TEMP_THRSHLD 80

typedef enum
{
    E_AREA_0,
    E_AREA_1,
    E_AREA_2,
    NUM_PGM_AREA
} pgm_area_type;

#define MT_UNI_DIR 0                // 수전 : 단방향(정방향)
#define MT_BOTH_DIR 1               // 송/수전 : 양방향(정방향/역방향)
#define DEFAULT_SEL_ACT MT_UNI_DIR  // 0: 수전, 1: 송/수전
#define DEFAULT_SEL_REACT 1         // 1: 지상, 2: 진상, 3: 지상+진상, 4: 피상

typedef struct
{
    uint8_t hour;
    uint8_t min;
    uint8_t on_off;
} ts_struct_type;

#define TS_ZONE_SIZE 8
typedef struct
{
    uint8_t cnt;
    uint8_t ctrl;
    hh_mm_type zone[TS_ZONE_SIZE];
} ts_conf_type;

typedef struct
{
    uint16_t _latchoncnt;
    uint16_t CRC_M;
} latchon_data_type;

// ---------- 비정기 검침일 ----------------
typedef struct
{
    uint8_t cnt;
    day_time_type npbill[NP_BILLDATE_LEN];
    uint16_t CRC_M;
} npbill_date_type;

// ---------- 정기/비정기, 임시 휴일  --------
typedef struct
{
    uint8_t month;
    uint8_t date;
    uint8_t day_id;
} holiday_struct_type;

typedef struct
{
    uint16_t arr_len;
    uint8_t yr;
    holiday_struct_type holiday[HOLIDAYS_PER_BLOCK];
    uint16_t CRC_M;
} holiday_date_type;

typedef struct
{
    uint8_t cnt;
    uint8_t yr;
    holiday_struct_type holiday[TMP_HOLIDAYS_LEN];
    uint16_t CRC_M;
} tmp_holiday_date_type;

// ------------- TOU ------------------
typedef struct
{
    uint8_t month;
    uint8_t date;
    uint8_t week_id;
} season_struct_type;

typedef struct
{
    uint8_t cnt;
    season_struct_type season[SEASON_PROF_SIZE];
    uint16_t CRC_M;
} season_date_type;

typedef struct
{
    uint8_t week_id;
    uint8_t day_id[WEEK_LEN];
} week_struct_type;

typedef struct
{
    uint8_t cnt;
    week_struct_type week[WEEK_PROF_SIZE];
    uint16_t CRC_M;
} week_date_type;

typedef struct
{
    uint8_t hour;
    uint8_t min;
    uint8_t rate;
} tou_struct_type;

typedef struct
{
    uint8_t day_id;
    uint8_t tou_conf_cnt;
    tou_struct_type tou_conf[MAX_TOU_DIV_DLMS];
    uint16_t CRC_M;
} dayid_table_type;

typedef struct
{
    uint8_t cnt;
    tou_struct_type conf[MAX_TOU_DIV_DLMS];
} tou_data_type;

typedef struct
{
    uint8_t avail;
    uint8_t aprogarea;
    uint8_t pprogarea;
    uint8_t aholarea;
    uint8_t pholarea;
    uint16_t curravailbits;
    uint16_t futavailbits;
    date_time_type futwork;
} prog_info_type;

typedef struct
{
    uint8_t data_len;
    uint8_t id;
    uint8_t data[KEY_DATA_SIZE];
} key_agree_type;

typedef struct
{
    uint8_t cnt;
    key_agree_type key_agree[KEY_INFO_SIZE];
    uint16_t CRC_M;
} key_info_type;

// ----------- Meter Configure ----------------
typedef struct
{
    uint8_t srdrtype[SR_DR_KIND_NUM];
    uint8_t sel_act;
    uint8_t sel_react;
    uint32_t mDRLimit;
} billing_parm_type;

typedef struct
{
    uint8_t enabled;
    date_time_type bgn_dt;
    date_time_type end_dt;
    int8_t dev;
    uint8_t bgn_week;
    uint8_t end_week;
} daylightsaving_time_type;

typedef struct
{
    uint16_t level;
    uint8_t autortncnt;
    uint16_t level2;
    uint16_t det_hold;
    uint16_t rtn_dur1;
    uint16_t rtn_dur2;
    uint8_t cnt_n1;
} scurr_info_type;

typedef struct
{
    uint8_t mr_date;         // 정기 검침 일
    ratekind_type mtkind;    // 계기 종 (1종, 2종, 3종, 4종)
    uint8_t dmintv;          // 수요 주기
    uint8_t dmsubintv;       // 수요 sub 주기
    uint8_t lpintv;          // LP 주기
    uint8_t lpavgintv;       // 평균 LP 주기
    uint8_t errpls;          // 에러 펄스 출력 방식 선택
    uint8_t autobidir;       // 자동 송-수전 모두 en
    uint8_t conds;           // 역률 보정용 condensor 여부 설정
    uint8_t dspmode;         // LCD 표시 (사용자 모드 혹은 공급자 모드)
    uint8_t lcdparm;         // LCD 파라미터 (규격서 참조)
    ad_sig_type sigsel;      // 릴레이 제어 방법 선택
    meas_method_type meas;   // 계량 방식 (기본, 단방향)
    baudrate_type baud;      // 통신 baud rate
    billing_parm_type bill;  //  billing 파라미터 (규격서 참조)
    daylightsaving_time_type dlsinfo;  // daylightsaving 정보
    scurr_info_type scurr;             // 저전류 제한 동작 관련 설정 값
    tou_data_type tou;                 // 일 단위 TOU 결정 관련 정보
    ts_conf_type ts;                   // Time Switch 처리 관련 정보
    prog_info_type prog;               // 현재 입력 된 프로그램 정보
    U8 comm_en;                        // 커버 오픈 시 통신 가능 여부 설정
    uint8_t ovrcurcut_en;              // 과전류 (60 A) 시 릴레이 제어 여부
    uint32_t shftrng;
    uint16_t ct_ratio_prog;
    uint16_t pt_ratio_prog;
    uint32_t zcrs_sig_out_dur, zcrs_sig_out_cmps;
    int16_t self_error_ref;
    bool auto_mode_sel;
    bool holiday_sel;
    uint16_t thd_rec_period;
    uint16_t realtime_lpintv;  // 실시간 상별 LP 주기 second
    U8 working_fault;
    uint8_t CRC_M;
} mt_conf_type;

typedef struct
{
    bool futprog_partition_tou;
    uint8_t mt_conf_2_type_gap_1[3];
    uint8_t mt_conf_2_type_gap[20];

    uint16_t CRC_M;
} mt_conf_2_type;

#define futprog_partition_tou mt_conf_2.futprog_partition_tou
#define reg_mr_date mt_conf.mr_date /* 검침일 */
#define mt_rtkind mt_conf.mtkind    /* TOU 프로그램 : 종별 */
#define dm_interval mt_conf.dmintv
#define dm_sub_interval mt_conf.dmsubintv
#define lp_interval mt_conf.lpintv
#define lpavg_interval mt_conf.lpavgintv
#define sig_sel mt_conf.sigsel
#define meas_method mt_conf.meas /* 계량 모드 */
#define err_pulse_react mt_conf.errpls
#define condensor_en mt_conf.conds
#define lcd_dsp_mode mt_conf.dspmode
#define lcd_set_parm mt_conf.lcdparm
#define mdm_baud mt_conf.baud
#define dls_info mt_conf.dlsinfo
#define sr_dr_type                                                                              \
    mt_conf.bill.srdrtype /* 검침 파라미터 : 검침 조건 및 수요전력 복귀 조건 \
                           */
#define mt_dir mt_conf.bill.sel_act /* 선택 유효전력량(송/수전 구분) */
#define mt_selreact \
    mt_conf.bill    \
        .sel_react /* 선택 무효전력량(지상, 진상, 지상+진상, 피상전력량) */
#define mDR_limit_sec mt_conf.bill.mDRLimit /* 수동 수요전력 복귀 방지 시간 */
#define scurr_limit_level mt_conf.scurr.level
#define scurr_autortn_cnt mt_conf.scurr.autortncnt
#define scurr_limit_level_2 mt_conf.scurr.level2
#define scurr_det_hold mt_conf.scurr.det_hold
#define scurr_rtn_dur_1 mt_conf.scurr.rtn_dur1
#define scurr_rtn_dur_2 mt_conf.scurr.rtn_dur2
#define scurr_cnt_n1 mt_conf.scurr.cnt_n1
#define tou_data mt_conf.tou
#define tou_data_cnt tou_data.cnt
#define tou_data_conf tou_data.conf
#define ts_conf mt_conf.ts
#define ts_conf_cnt ts_conf.cnt
#define ts_conf_ctrl ts_conf.ctrl
#define ts_conf_zone ts_conf.zone
#define prog_available mt_conf.prog.avail
#define aprog_area mt_conf.prog.aprogarea /* 현재 프로그램 (Active) */
#define pprog_area mt_conf.prog.pprogarea /* 예약 프로그램 (Passive) */
#define ahol_area mt_conf.prog.aholarea   /* 현재 정기/비정기 휴일 (Active) */
#define phol_area mt_conf.prog.pholarea   /* 예약 정기/비정기 휴일 (Passive) */
#define currprog_available_bits mt_conf.prog.curravailbits
#define futprog_available_bits mt_conf.prog.futavailbits
#define fut_prog_work_time \
    mt_conf.prog.futwork /* TOU 예약 프로그램 적용 시간 */
#define comm_en_coveropen mt_conf.comm_en
#define overcurr_cut_en mt_conf.ovrcurcut_en
#define working_fault_min mt_conf.working_fault
#define rtc_shift_range mt_conf.shftrng
#define ct_ratio_prog mt_conf.ct_ratio_prog
#define pt_ratio_prog mt_conf.pt_ratio_prog
#define self_error_ref mt_conf.self_error_ref
#define zcrs_sig_out_dur mt_conf.zcrs_sig_out_dur
// 상수값 사용
// #define  zcrs_sig_out_cmps mt_conf.zcrs_sig_out_cmps //jp.kim 24.11.14
#define auto_mode_sel mt_conf.auto_mode_sel
#define holiday_sel1 mt_conf.holiday_sel
#define thd_rec_period mt_conf.thd_rec_period
#define rt_lp_interval mt_conf.realtime_lpintv
#define pEOB_sr_dr_type sr_dr_type[E_pEOB]           /* 정기 검침 */
#define npEOB_sr_dr_type sr_dr_type[E_npEOB]         /* 비정기 검침 */
#define mDR_sr_dr_type sr_dr_type[E_mDR]             /* 수동 검침 */
#define pgmCHG_sr_dr_type sr_dr_type[E_pgmCHG]       /* 프로그램 변경 */
#define timCHG_sr_dr_type sr_dr_type[E_timCHG]       /* 날짜/시간 변경 */
#define dmintvCHG_sr_dr_type sr_dr_type[E_dmintvCHG] /* 수요시간 주기 변경 */
#define MmodeCHG_sr_dr_type sr_dr_type[E_MmodeCHG]   /* 계량 모드 변경 */
#define seasonCHG_sr_dr_type sr_dr_type[E_seasonCHG] /* 계절 변경 검침 */
#define lcd_is_user_dspmode() (lcd_dsp_mode == LCD_USER_DSPMODE)
#define lcd_is_supp_dspmode() (lcd_dsp_mode == LCD_SUPP_DSPMODE)

#define MR_SR_BIT BIT0 /* SR : (Self Read) 자기검침 */
#define MR_DR_BIT BIT1 /* DR : (Demand Reset) 수요전력 복귀 */

#define SEL_SR 0x02
#define SEL_SR_DR 0x03

#define PROG_CURR_BIT BIT0
#define PROG_FUT_BIT BIT1

#define PROG_HOLIDAY_BIT BIT0
#define PROG_SEASON_PROFILE_BIT BIT1
#define PROG_WEEK_PROFILE_BIT BIT2
#define PROG_DAY_PROFILE_BIT BIT3
#define PROG_pBILL_DATE_BIT BIT4
#define PROG_npBILL_DATE_BIT BIT5
#define PROG_SUPP_DISP_BIT BIT6
#define PROG_LCDPARM_BIT BIT7
#define PROG_BILLPARM_BIT BIT8
#define PROG_TMP_HOLIDAY_BIT BIT9

#define prog_curr_is_available() ((prog_available & PROG_CURR_BIT) != 0)
#define prog_cur_add() (prog_available |= PROG_CURR_BIT)
#define prog_cur_unavailable() (prog_available &= ~PROG_CURR_BIT)
#define prog_fut_is_available() ((prog_available & PROG_FUT_BIT) != 0)
#define prog_fut_add() (prog_available |= PROG_FUT_BIT)
extern void prog_fut_delete(void);

#define prog_tmp_holidays_add() \
    (currprog_available_bits |= PROG_TMP_HOLIDAY_BIT)

#define prog_hol_available() ((currprog_available_bits & PROG_HOLIDAY_BIT) != 0)
#define prog_tmp_holidays_available() \
    ((currprog_available_bits & PROG_TMP_HOLIDAY_BIT) != 0)
#define prog_season_available() \
    ((currprog_available_bits & PROG_SEASON_PROFILE_BIT) != 0)
#define prog_week_available() \
    ((currprog_available_bits & PROG_WEEK_PROFILE_BIT) != 0)
#define prog_day_available() \
    ((currprog_available_bits & PROG_DAY_PROFILE_BIT) != 0)
#define prog_pbill_available() \
    ((currprog_available_bits & PROG_pBILL_DATE_BIT) != 0)
#define prog_npbill_available() \
    ((currprog_available_bits & PROG_npBILL_DATE_BIT) != 0)
#define prog_tou_available() \
    (prog_season_available() && prog_week_available() && prog_day_available())
#define prog_suppdisp_available() \
    ((currprog_available_bits & PROG_SUPP_DISP_BIT) != 0)

#define futprog_hol_available() \
    ((futprog_available_bits & PROG_HOLIDAY_BIT) != 0)
#define futprog_tmp_holidays_available() \
    ((futprog_available_bits & PROG_TMP_HOLIDAY_BIT) != 0)
#define futprog_season_available() \
    ((futprog_available_bits & PROG_SEASON_PROFILE_BIT) != 0)
#define futprog_week_available() \
    ((futprog_available_bits & PROG_WEEK_PROFILE_BIT) != 0)
#define futprog_day_available() \
    ((futprog_available_bits & PROG_DAY_PROFILE_BIT) != 0)
#define futprog_pbill_available() \
    ((futprog_available_bits & PROG_pBILL_DATE_BIT) != 0)
#define futprog_npbill_available() \
    ((futprog_available_bits & PROG_npBILL_DATE_BIT) != 0)
#define futprog_suppdisp_available() \
    ((futprog_available_bits & PROG_SUPP_DISP_BIT) != 0)
#define futprog_lcdparm_available() \
    ((futprog_available_bits & PROG_LCDPARM_BIT) != 0)
#define futprog_billparm_available() \
    ((futprog_available_bits & PROG_BILLPARM_BIT) != 0)

#define mt_is_uni_dir() (mt_dir == MT_UNI_DIR)
#define mt_is_one_ratekind() (mt_rtkind == ONE_RATE_KIND)
#define mt_prev_is_uni_dir() (mr_mtdir == MT_UNI_DIR)
#define mt_prev_is_one_ratekind() (mr_rtkind == ONE_RATE_KIND)
#define mt_is_no_sel() (sig_sel == SIG_NOSEL)
#define mt_is_rload_ctrl() (sig_sel == SIG_rLOAD_CONTROL)
#define mt_is_time_sw() (sig_sel == SIG_TS_CONTROL)
#define mt_is_sCurr_limited() (sig_sel == SIG_sCURR_LIMIT)
#define meas_is_basic() (meas_method == E_BASIC)
#define meas_is_single() (meas_method == E_SINGLE_DIR)

#define circdsp_is_smode() (lcd_set_parm & 0x01)
#define circdsp_smode_toggle() (lcd_set_parm ^= 0x01)
#define circdsp_smode_set() (lcd_set_parm |= 0x01)
#define circdsp_smode_reset() (lcd_set_parm &= ~0x01)
#define circdsp_pvt_mode_enable() (lcd_set_parm |= 0x02)
#define circdsp_pvt_mode_disable() (lcd_set_parm &= ~0x02)
#define circdsp_pvt_mode_toggle() (lcd_set_parm ^= 0x02)
#define circdsp_is_pvt_mode() (lcd_set_parm & 0x02)
#define wrong_conn_set() (lcd_set_parm |= 0x04)
#define wrong_conn_unset() (lcd_set_parm &= ~0x04)
#define wrong_conn_is_set() (lcd_set_parm & 0x04)
#define bwd_conn_set_a() (lcd_set_parm |= 0x08)
#define bwd_conn_unset_a() (lcd_set_parm &= ~0x08)
#define bwd_conn_is_set_a() (lcd_set_parm & 0x08)
#define bwd_conn_set_b() (lcd_set_parm |= 0x10)
#define bwd_conn_unset_b() (lcd_set_parm &= ~0x10)
#define bwd_conn_is_set_b() (lcd_set_parm & 0x10)
#define bwd_conn_set_c() (lcd_set_parm |= 0x20)
#define bwd_conn_unset_c() (lcd_set_parm &= ~0x20)
#define bwd_conn_is_set_c() (lcd_set_parm & 0x20)

#define circdsp_is_bill5days() (false)
#define mt_is_onephase() (PHASE_NUM == 1)

// ----------- Meter Operation ----------------
typedef struct
{
    uint8_t evt;
    uint8_t intv;
    uint8_t intv_num;
    rate_type rt;
    date_time_type dt1;
    date_time_type dt2;
    date_time_type lastdt;
    uint8_t lastevt;
    uint8_t processed;
} eoi_info_type;

typedef struct
{
    uint32_t idx;
    uint32_t evt;
    date_time_type dt1;
    date_time_type dt2;
    date_time_type nxtdt;
} lp_info_type;

typedef struct
{
    uint8_t date;
    uint16_t used;  // sec
} ts_test_info_type;

typedef struct
{
    uint8_t det_bit;
    uint8_t det_bit_thismonth;
    uint8_t det_bit_nextmonth;
    uint32_t magdet_dur;
    date_time_type mag_dt;
    uint8_t enter5days;
} tamper_info_type;

typedef struct
{
    uint8_t yr;
    uint8_t mon;
    uint8_t sel;
} sel_react_type;

typedef struct
{
    uint8_t mtconn;             // not used
    date_time_type pwrfaildt;   // 정전 시간 저장
    uint16_t mrcnt;             // 월별 검침 횟수
    uint16_t mrcnt_nprd;        // 비정기 검침 횟수
    uint16_t mrcnt_season_chg;  // season change 검침 횟수
    uint8_t ldctrl;             // 현재 릴레이 on/off state
    uint8_t ldinited;           // 최초 릴리이 on 수행 여부
    uint8_t
        rcntdmwearidx;  // 직전 수요값 저장 시 nv memory 영역 분산 저장 목적으로
                        // 사용할 index -> 직전 수요값은 index 해당 영역에
                        // 저장해서 한곳에만 저장 되지 않고 분산되어 저장 됨
    uint8_t scurr;      // 전류 제한 횟수 -> 한전 규격 이해가 필요 함
    uint8_t seasonid;   // 계절 변경 여부 판단 하기 위해 마지막 season id 값을
                        // 저장하고 있슴
    date_time_type regdt;        // 계기 등록 시간 (최초 LP 읽기 수행 한 시간)
    uint16_t logcnt[numLogs];    // 각 이벤트 로그 횟수
    eoi_info_type eoi;           // eoi 관련 처리 info 를 담고 있슴
    lp_info_type lp;             // LP 처리 관련 내용을 담고 있슴
    ts_test_info_type ts_tinfo;  // Time Switch 기능 처리 내용
    tamper_info_type tamper;     // tamper (자계감지, 커버 오픈) 기능 처리
    int8_t tempthrshld;          // 온도 초과 로그 용 threshold 값
    prog_in_type
        progin;  // 최근 프로그램 변경 종류 (키 입력, 초기화, 프로그램입력)
    sel_react_type selrea;  // 무효 선택에 대한 info (선택무효, 적용 년/월)
    uint8_t wrongconn;      // 오결선 상태
    uint8_t
        lpblockedcnt;  // LP 저장이 blocked 된 이후 LP 저장 갯수(정전 시 저장용)
    uint32_t TOTAL_LPEVENTCNT;
    uint8_t tou_id_change;
    uint16_t logcertcnt[eLogCert_MAX];  // 각 보안 이벤트 로그 횟수
    // jp.kim 25.06.22
    uint8_t _mr_bill_prd[2];  // sr시에 정기 비정기 계절 종류를 저장한다, 전월
                              // 및 전전월
    uint16_t CRC_M;
} whm_op_type;

#define mt_has_conn whm_op.mtconn
#define pwrfail_dt whm_op.pwrfaildt
#define mr_cnt whm_op.mrcnt
#define mrcnt_nprd whm_op.mrcnt_nprd
#define mrcnt_season_chg whm_op.mrcnt_season_chg
#define relay_load_state whm_op.ldctrl
#define load_inited whm_op.ldinited
#define rcntdm_wear_idx whm_op.rcntdmwearidx
#define scurr_limit_cnt whm_op.scurr
#define rcnt_season_id whm_op.seasonid
#define mt_register_dt whm_op.regdt
#define log_cnt whm_op.logcnt
#define log_cert_cnt whm_op.logcertcnt
#define eoi_evt whm_op.eoi.evt
#define eoi_intv whm_op.eoi.intv
#define eoi_intv_num whm_op.eoi.intv_num
#define eoi_rate whm_op.eoi.rt
#define eoi_dt1 whm_op.eoi.dt1
#define eoi_dt2 whm_op.eoi.dt2
#define eoi_lastdt whm_op.eoi.lastdt
#define eoi_last_evt whm_op.eoi.lastevt
#define eoi_processed whm_op.eoi.processed
#define LP_index whm_op.lp.idx
#define LP_event whm_op.lp.evt
#define LP_trigdt1 whm_op.lp.dt1
#define LP_trigdt2 whm_op.lp.dt2
#define LP_nextdt whm_op.lp.nxtdt
#define ts_test_info whm_op.ts_tinfo
#define tamper_det_bit whm_op.tamper.det_bit
#define tamper_det_bit_thismonth whm_op.tamper.det_bit_thismonth
#define tamper_det_bit_nextmonth whm_op.tamper.det_bit_nextmonth
#define tamper_enter_5days whm_op.tamper.enter5days
#define magnet_det_bit tamper_det_bit
#define magnet_det_dur whm_op.tamper.magdet_dur
#define magnet_dt whm_op.tamper.mag_dt
#define coveropen_det_bit tamper_det_bit
#define temp_thrshld whm_op.tempthrshld
#define lpavgintv_chg whm_op.lpavgintvchg
#define prog_in_state whm_op.progin
#define sel_react_yr whm_op.selrea.yr
#define sel_react_mon whm_op.selrea.mon
#define sel_react_sel whm_op.selrea.sel
#define wrong_conn_state whm_op.wrongconn
#define lpblocked_cnt whm_op.lpblockedcnt
#define TOTAL_LP_EVENT_CNT whm_op.TOTAL_LPEVENTCNT
#define tou_id_change_sts whm_op.tou_id_change
#define mr_bill_prd_type whm_op._mr_bill_prd  // jp.kim 25.06.22

// jp.kim 24.10.28
#define pwr_sf_cnt log_cnt[eLogPwrF]
#define pwr_sr_cnt log_cnt[eLogPwrR]
#define pwr_wf_cnt log_cnt[eLogWorkPwrF]
#define pwr_f_cnt log_cnt[eLogPwrF_WorkPwrF]
#define pwr_r_cnt log_cnt[eLogPwrR_WorkPwrR]

#define rtc_chg_f_cnt log_cnt[eLogRtcF]
#define rtc_chg_t_cnt log_cnt[eLogRtcT]
#define auto_dr_cnt log_cnt[eLogDRa]
#define man_dr_cnt log_cnt[eLogDRm]
#define sr_cnt log_cnt[eLogSR]
#define pgm_chg_cnt log_cnt[eLogPgmChg]
#define scurr_limit_log_cnt log_cnt[eLogSCurrLimit]
#define cover_open_cnt log_cnt[eLogCoverOpen]
#define magnet_det_cnt log_cnt[eLogMagnetDet]
#define rload_ctrl_cnt log_cnt[eLogrLoadCtrl]
#define sag_log_cnt log_cnt[eLogSag]
#define swell_log_cnt log_cnt[eLogSwell]
#define scurr_nonsel_cnt log_cnt[eLogSCurrNonSel]

#define SysSwUp_cnt log_cnt[eLogSysSwUp]
#define MtrSwUp_cnt log_cnt[eLogMtrSwUp]
#define InModemUp_cnt log_cnt[eLogInModemUp]
#define ExModemUp_cnt log_cnt[eLogExModemUp]

typedef enum
{
    /* 수전, 송전 */
    eDeliAct,
    eReceiAct,
    numDirChs
} energy_dir_type;
#define numDmDirChs 1

// 통신 에서의  act_rea 와 일치 !!!
typedef enum
{
    /* 유효, 무효, 지상무효, 진상무효, 피상 */
    eActEn,
    eReactEn,
    eLagReactEn,
    eLeadReactEn,
    eAppEn,
    numEns
} energy_kind_type;

typedef enum
{
    BIEVT_BAT_OUT,
    BIEVT_BAT_IN,
    BIEVT_TIMECHG
} bat_inst_evt_type;

typedef struct
{
    uint8_t dr_sel;
    uint8_t type;
} EOB_type;

/*
    프로그램 입력 시 항목단위로 파싱 된 값
*/
typedef struct
{
    uint32_t set_bits;
    uint32_t set_bits_1;
    daylightsaving_time_type dlsinfo;
    uint8_t out_sig;
    baudrate_type baud;
    uint8_t regread_date;
    EOB_type pEOB;
    EOB_type npEOB;
    uint8_t bill_parm[BILLING_PARM_SIZE];
    uint8_t dm_prd;
    uint8_t dm_prd_num;
    uint8_t pgm_name[PROG_ID_SIZE];
    date_time_type active_passive_time;
    uint16_t scurr_lmt;
    uint16_t scurr_lmt2;
    uint8_t scurr_autortn;
    uint8_t lp_intv;
    uint8_t lpavg_intv;
    uint8_t lcdsetparm;
    auth_val_type pwd;
    ts_conf_type ts;
    uint16_t _scurr_det_hold;
    uint16_t _scurr_rtn_dur1;
    uint16_t _scurr_rtn_dur2;
    uint8_t _scurr_cnt_n1;
    bool hol_sel1;  // 2023.11.14 jp
    uint16_t CRC_M;
} prog_dl_type;

#define pdl_set_bits prog_dl.set_bits
#define pdl_set_bits_1 prog_dl.set_bits_1
#define pdl_set_bits_clr() \
    (pdl_set_bits = 0L);   \
    (pdl_set_bits_1 = 0L)

#define CLKSTS_INVALID BIT0
#define CLKSTS_DOUBTFUL BIT1
#define CLKSTS_DIFF_BASE BIT2
#define CLKSTS_INVALID_STS BIT3
#define CLKSTS_DAYLIGHT_SAVING BIT7

#define DLS_ENABLED BIT0
#define DLS_BGN_DEV BIT4
#define DLS_END_DEV BIT5
#define DLS_WAITING BIT6
#define DLS_ACTIVE BIT7

#define DLS_enable_init() (dls_info.enabled = DLS_ENABLED)

#define DLS_is_enabled() (dls_info.enabled & DLS_ENABLED)
#define DLS_is_active() (dls_info.enabled & DLS_ACTIVE)
#define DLS_is_bgn_dev() (dls_info.enabled & DLS_BGN_DEV)
#define DLS_is_end_dev() (dls_info.enabled & DLS_END_DEV)

#define DLS_enable() (dls_info.enabled |= DLS_ENABLED)
#define DLS_disable() (dls_info.enabled &= ~DLS_ENABLED)
#define DLS_active_set() (dls_info.enabled |= DLS_ACTIVE)
#define DLS_active_unset() (dls_info.enabled &= ~DLS_ACTIVE)
#define DLS_bgn_dev_set() (dls_info.enabled |= DLS_BGN_DEV)
#define DLS_bgn_dev_set() (dls_info.enabled |= DLS_BGN_DEV)
#define DLS_bgn_dev_unset() (dls_info.enabled &= ~DLS_BGN_DEV)
#define DLS_end_dev_set() (dls_info.enabled |= DLS_END_DEV)
#define DLS_end_dev_unset() (dls_info.enabled &= ~DLS_END_DEV)

#define cur_year cur_rtc.year
#define cur_month cur_rtc.month
#define cur_date cur_rtc.date
#define cur_hour cur_rtc.hour
#define cur_min cur_rtc.min
#define cur_sec cur_rtc.sec

#define NaN 0xffffffff       // not a number
#define plusINF 0x7f800000   // positive overflow
#define minusINF 0xff800000  // negative overflow

typedef union ufloat
{
    float f;
    uint32_t ul;
} ufloat_type;

extern bool bat_rtc_backuped;
extern uint8_t b_q_display;
extern int b_q_dspkind;
extern buff_struct_type ubuff;
extern mt_conf_type mt_conf;
extern whm_op_type whm_op;
extern date_time_type cur_rtc;
extern rate_type cur_rate;
extern rate_type cur_rate_before;
extern uint32_t mxdm_dgt_cnt;
extern uint32_t mxaccm_dgt_cnt;
extern const uint8_t logical_device_name_r[];
extern ratekind_type rtkind_history;
extern uint8_t mtdir_history;
extern float temp_caled;
extern float adj_currtemp;
extern pwrfail_info_type pwrfail_info;

extern bool b_pwrtn_for_dst;

extern bool b_protect_released;
extern int32_t last_sec;
extern uint8_t logical_device_name_r_kepco[DEVICE_ID_SIZE];

extern mt_conf_2_type mt_conf_2;

void mt_conf_2_save(void);
void mt_conf_2_default(void);
void whm_init(void);
void default_ts_zone(void);
void pwr_rtn_proc(void);
void cur_rtc_restore_batmode(void);
void whm_proc(void);
void whm_data_save_sag(void);
void whm_data_unsave_sag(void);
void whm_data_save_fromBM(void);
void set_cust_id(ser_no_type *serno);
bool get_cust_id(uint8_t *tptr);
void get_manuf_id(uint8_t *tptr);
void get_nms_dms_id(uint8_t *tptr);
bool mt_is_registered(void);
void rtc_chg_proc(date_time_type *dt, uint8_t *tptr);
void var_history_init(void);
void prog_chg_proc_by_comm(prog_dl_type *progdl, U8 *tptr, bool curr_prog_in);
void prog_chg_proc_by_key(uint8_t bfdir, ratekind_type bfkind,
                          meas_method_type bfmeas, uint8_t *tptr);
void tou_conf_init_onekind(void);
void tou_conf_init_twokind(void);
void write_rtc(date_time_type *pdt);
void get_date_time(date_time_type *pdt);
void cur_rtc_update(void);
bool cur_rtc_is_set(void);
int get_ver_fw_nv(uint8_t *tptr);
void whm_bakup_crc_calc(void);
void set_start_time(date_time_type *dt);
void default_scurr_parm(void);
void reset_rtc(void);
void nv_header_set(uint8_t ver);
void whm_op_save(void);
void mt_conf_default(void);
mt_conf_type *mt_conf_get(void);
whm_op_type *whm_op_get(void);
void mt_conf_save(void);
void pwrtn_meas_proc(uint8_t *tptr);
void timechg_meas_proc(uint8_t *tptr);
uint32_t Generate_CAN_ID(void);
uint8_t get_server_hdlc_addr_to_dec(void);
int dsm_run_power_get(void);
char *dsm_run_power_string(uint32_t idx);

#endif
