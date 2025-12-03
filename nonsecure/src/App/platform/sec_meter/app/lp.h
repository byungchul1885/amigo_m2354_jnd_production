#ifndef LP_H
#define LP_H 1

// LP event flag
/****************************************/
/* 과 전압 , 저 전압 관련 코딩 필요...  */
/* P3SX 적용후세밀한 검토 및 적용  필요.*/
/****************************************/
#define LPE_NULL (uint32_t) NO_BITS
#define LPE_PROGRAM_CHG (uint32_t) BIT0  // event pushed & each time save !!!
#define LPE_NO_BAT (uint32_t) BIT2       // always
#define LPE_nPERIOD (uint32_t) BIT4      // event pushed & each time save !!!
#define LPE_TIME_CHG (uint32_t) BIT5     // event pushed & each time save !!!
#define LPE_PWR_FAIL (uint32_t) BIT6     // event pushed & each time save !!!

#define LPE_IRRG_WR (uint32_t) BIT7  // 비주기적 LP WRITE 표시

#define LPE_TARIFF \
    (uint32_t)(BIT9 | BIT8)  // always, but pwr_fail, min_chg, rtc_chg event =>
                             // tariff not updated
#define LPE_sCURR_LIMIT \
    (uint32_t) BIT10  // always ( set ==> CLEAR_MASK 에서 clear)
#define LPE_MAGNET_DET (uint32_t) BIT11  // always
#define LPE_VOL_LOW (uint32_t) BIT12  // event pushed ( 저 전압 ),평균전압 -6%
#define LPE_TEMPOVER (uint32_t) BIT13             // always
#define LPE_WRONGCONN_WRONGNEUT (uint32_t) BIT14  // always
#define LPE_SAG_SWELL \
    (uint32_t) BIT15  // event pushed & always ( ==> no-event 인 경우 만 지속
                      // 여부 확인 )
#define LPE_LATCH_ERROR \
    (uint32_t) BIT16  // always (==> 릴레이 OFF 상태인 경우 4*시동전류 가 감지
                      // 되는 경우)
#define LPE_LATCH_OFF (uint32_t) BIT17    // always
#define LPE_LATCH_ON (uint32_t) BIT18     // always
#define LPE_TCOVER_OPEN (uint32_t) BIT19  // always
#define LPE_VOL_OVER (uint32_t) BIT20  // event pushed ( 과 전압 ),평균전압 +6%
#define LPE_SEASON \
    (uint32_t) BIT21  // event pushed & each time save  ( season )
#define LPE_PERIOD \
    (uint32_t) BIT22  // event pushed & each time save  ( 정기검침 )
#define LPE_IOVER (uint32_t) BIT23  // always

#define LPE_TRIG (LPE_nPERIOD | LPE_PERIOD | LPE_SEASON)

#define lp_save_is_triged() ((LP_event & LPE_TRIG) != 0)

#define lp_event_set(e) LP_event |= (e) /* LP 이벤트 기록 */
#define lp_event_unset(e) LP_event &= ~(e)
#define lp_event_clear() LP_event = LPE_IRRG_WR

#define LP_SIZE_UNI (65 * 4 * 24)  // Uni-dir =>15 min 주기 65 일 동안
/* (1Hour / 15Min = 4Times) x 24Hour x 65Day = LP 6240 */
#define LP_SIZE_BOTH (65 * 4 * 24)  // Both-dir =>15 min 주기 65 일 동안
#define LP_SIZE ((mt_is_uni_dir()) ? LP_SIZE_UNI : LP_SIZE_BOTH)
#define LPAVG_SIZE (3 * 4 * 24)  // 15 min 주기 3 일 동안
#define LPRT_SIZE (4 * 15)  // 15 ea [1/5/15/60(1분)/300(5분)/900(15분) 초]
#define LP_BUF_GAP 20
#define LP_BUF_SIZE (LP_SIZE_UNI + LP_BUF_GAP)

#define LPAVG_BUF_SIZE (LPAVG_SIZE + LP_BUF_GAP)
#define LPRT_BUF_SIZE (LPRT_SIZE + LP_BUF_GAP)

#define NUM_LP_COL_UNI 6

#define LP_BUF_SIZE (LP_SIZE_UNI + LP_BUF_GAP)
#define NUM_LP_COL_BOTH 11
#define LP_COL_MASK_UNI 0xfc00
#define LP_COL_MASK_BOTH 0xffe0

#define LP_COL_LP_CNT_BIT 0x8000
#define LP_COL_ACT_Q14_BIT 0x4000
#define LP_COL_REACT_Q1_BIT 0x2000
#define LP_COL_REACT_Q4_BIT 0x1000
#define LP_COL_APP_Q14_BIT 0x0800
#define LP_COL_CLOCK 0x0400
#define LP_COL_STATUS 0x0200
#define LP_COL_ACT_Q23_BIT 0x0100
#define LP_COL_REACT_Q2_BIT 0x0080
#define LP_COL_REACT_Q3_BIT 0x0040
#define LP_COL_APP_Q23_BIT 0x0020

#define NUM_LPAVG_COL 5

#if 1 /* bccho, 2024-09-05, 삼상 */
#define NUM_LPAVG_COL_3PHS (16 + 3)
#define LPAVG_COL_MASK (int32_t)0x7c000
#define LPAVG_COL_MASK_3PHS (int32_t)0x7ffff
#define LPAVG_COL_CLOCK (int32_t)0x40000
// 1phs
#define LPAVG_COL_V (int32_t)0x20000
#define LPAVG_COL_THD (int32_t)0x10000
#define LPAVG_COL_I (int32_t)0x8000
#define LPAVG_COL_PHASE (int32_t)0x4000
// 3phs
#define LPAVG_COL_V1_L1_2 (int32_t)0x20000
#define LPAVG_COL_V1 (int32_t)0x10000
#define LPAVG_COL_THD1 0x8000
#define LPAVG_COL_I1 0x4000
#define LPAVG_COL_PHASE1 0x2000
#define LPAVG_COL_ERR1 0x1000

#define LPAVG_COL_V2_L2_3 0x0800
#define LPAVG_COL_V2 0x0400
#define LPAVG_COL_THD2 0x0200
#define LPAVG_COL_I2 0x0100
#define LPAVG_COL_PHASE2 0x0080
#define LPAVG_COL_ERR2 0x0040

#define LPAVG_COL_V3_L3_1 0x0020
#define LPAVG_COL_V3 0x0010
#define LPAVG_COL_THD3 0x0008
#define LPAVG_COL_I3 0x0004
#define LPAVG_COL_PHASE3 0x0002
#define LPAVG_COL_ERR3 0x0001
#else
#define NUM_LPAVG_COL_3PHS 16

#define LPAVG_COL_MASK 0xf800
#define LPAVG_COL_MASK_3PHS 0xffff

#define LPAVG_COL_CLOCK 0x8000

// 1phs
#define LPAVG_COL_V 0x4000
#define LPAVG_COL_THD 0x2000
#define LPAVG_COL_I 0x1000
#define LPAVG_COL_PHASE 0x0800
// 3phs
#define LPAVG_COL_V1_L1_2 0x4000
#define LPAVG_COL_V1 0x2000
#define LPAVG_COL_THD1 0x1000
#define LPAVG_COL_I1 0x0800
#define LPAVG_COL_PHASE1 0x0400
#define LPAVG_COL_V2_L2_3 0x0200
#define LPAVG_COL_V2 0x0100
#define LPAVG_COL_THD2 0x0080
#define LPAVG_COL_I2 0x0040
#define LPAVG_COL_PHASE2 0x0020
#define LPAVG_COL_V3_L3_1 0x0010
#define LPAVG_COL_V3 0x0008
#define LPAVG_COL_THD3 0x0004
#define LPAVG_COL_I3 0x0002
#define LPAVG_COL_PHASE3 0x0001
#endif

#define NUM_LPRT_COL_1PHS (1 + 4)
#define NUM_LPRT_COL_3PHS (1 + 4 * 4)
#define LPRT_COL_MASK_1PHS 0xF8000000
#define LPRT_COL_MASK_3PHS 0xFFFF8000

#define LPRT_COL_CLOCK 0x80000000

#define LPRT_COL_ACT_POS_BIT 0x40000000       // 수전 유효
#define LPRT_COL_ACT_NEG_BIT 0x20000000       // 송전 유효
#define LPRT_COL_REACT_Q1Q2_BIT 0x10000000    // 무효 +
#define LPRT_COL_REACT_Q3Q4_BIT 0x08000000    // 무효 -
#define LPRT_COL_ACT_POS_A_BIT 0x04000000     // 수전 유효
#define LPRT_COL_ACT_NEG_A_BIT 0x02000000     // 송전 유효
#define LPRT_COL_REACT_Q1Q2_A_BIT 0x01000000  // 무효 +
#define LPRT_COL_REACT_Q3Q4_A_BIT 0x00800000  // 무효 -
#define LPRT_COL_ACT_POS_B_BIT 0x00400000     // 수전 유효
#define LPRT_COL_ACT_NEG_B_BIT 0x00200000     // 송전 유효
#define LPRT_COL_REACT_Q1Q2_B_BIT 0x00100000  // 무효 +
#define LPRT_COL_REACT_Q3Q4_B_BIT 0x00080000  // 무효 -
#define LPRT_COL_ACT_POS_C_BIT 0x00040000     // 수전 유효
#define LPRT_COL_ACT_NEG_C_BIT 0x00020000     // 송전 유효
#define LPRT_COL_REACT_Q1Q2_C_BIT 0x00010000  // 무효 +
#define LPRT_COL_REACT_Q3Q4_C_BIT 0x00008000  // 무효 -

struct lp_record
{
    uint32_t lp_cnt;
    uint8_t evt[3];
    uint8_t dt[4];
    uint32_t ch[numCHs];
};

typedef struct lp_record lp_record_type;

#if 1 /* bccho, 2024-09-05, 삼상 */
struct _lpavg_record
{
    uint8_t dt[4];
    uint16_t ch[18 /*1phs = 4*/];
};
#else
struct _lpavg_record
{
    uint8_t dt[4];    // compressed time
    uint16_t ch[15];  // 1phs : average voltage, THD per phase, current, phase
                      // between v and i 3phs : voltage(A) between 2 phases,
                      // average voltage(A), THD per phase, current(A), phase(A)
                      // between v and i,
                      //        B, C 에 대하서도 동일
};
#endif

typedef struct _lpavg_record lpavg_record_type;

struct _lprt_record_1phs
{
    uint8_t dt[4];  // compressed time
    float ch_2[4];
};

struct _lprt_record_3phs
{
    uint8_t dt[4];      // compressed time
    float ch_2[4 * 4];  // ch x 4
};

typedef struct _lprt_record_1phs lprt_record_1phs;
typedef struct _lprt_record_3phs lprt_record_3phs;

extern bool b_lprt_ready;
extern bool b_lprt_monitor;

extern bool b_lpavg_ready;
extern bool b_lpavg_monitor;
extern bool b_lp_pf_monitor;
extern uint8_t lpindex_while_lpblocked;
void lp_record_move_while_blocked(void);

void LP_init(void);
void LP_proc(void);
void LP_nextdt_set(date_time_type *pdt);
void pwrtn_LP_proc(pwrfail_info_type *pfinfo);
void LP_save(date_time_type *dt);
void LP_read(uint8_t *cp, uint32_t idx, uint8_t len, uint8_t mtdir);
void lp_save_manual(void);
void lpavg_save_manual(void);
void expand_time(date_time_type *dt, uint8_t *cdt);
void compress_time(uint8_t *cdt, date_time_type *dt);
void lp_clear(void);
void LPavg_reset(void);
void LPavg_proc(void);
void pwrtn_LPavg_proc(pwrfail_info_type *pfinfo);
void timechg_LPavg_proc(date_time_type *bf, date_time_type *af);
void lpavg_record_read(uint8_t *cp, uint32_t idx, uint8_t len);
float get_lpavg_v(uint8_t line);
float get_lpavg_i(uint8_t line);
void update_lp_intv_pf(void);
void lp_event_set_curate(void);
void LPavg_save(date_time_type *dt);
void LPavg_init(void);
bool lp_last_record_dt(date_time_type *pdt, uint8_t *tptr);
void lp_save_batmode(uint32_t lpe);
void LPrt_reset(void);
void LPrt_proc(void);
void pwrtn_LPrt_proc(pwrfail_info_type *pfinfo);
void timechg_LPrt_proc(date_time_type *bf, date_time_type *af);
void lprt_record_read(uint8_t *cp, uint32_t idx, uint8_t len);
void LPrt_save(date_time_type *dt);
void LPrt_init(void);
void fill_last_lp_record(void);

#endif
