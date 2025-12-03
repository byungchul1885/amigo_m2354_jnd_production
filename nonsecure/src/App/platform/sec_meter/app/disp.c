#include "amg_typedef.h"
#include "options.h"
#include "meter_app.h"
#include "cal.h"
#include "afe.h"
#include "tmp.h"
#include "comm.h"
#include "eoi.h"
#include "amg_sec.h"
#include "eob.h"
#include "lp.h"
#include "program.h"
#include "nv.h"
#include "key.h"
#include "port.h"
#include "disp.h"
#include "amg_lcd.h"
#include "platform.h"
#include "amg_time.h"
#include "utils.h"
#include "amg_media_mnt.h"
#include "amg_meter_main.h"
#include "amg_pwr.h"
#include "get_req.h"
#include "whm.h"
#include "amg_modemif_prtl.h"

#define _D "[DSP] "

#define DISP_SKIP_MARGIN 10  // 10 sec

// sync with uint8_t unit_of_digit[]
typedef enum
{
    eUnitNull,
    eUnitClock1,
    eUnitClock2,
    eUnitV,
    eUnitA,
    eUnitPercent,
    eUnitKw,
    eUnitKwh,
    eUnitKvar,
    eUnitKvarh,
    eUnitKva,
    eUnitKvah,
    eUnitHz,
    eUnitNum
} dsp_unit_type;

typedef enum /* digit_font_b[LDIGIT_NUM] */
{
    LDIGIT_0,
    LDIGIT_1,
    LDIGIT_2,
    LDIGIT_3,
    LDIGIT_4,
    LDIGIT_5,
    LDIGIT_6,
    LDIGIT_7,
    LDIGIT_8,
    LDIGIT_9,
    LDIGIT_A,
    LDIGIT_b,
    LDIGIT_C,
    LDIGIT_d,
    LDIGIT_E,
    LDIGIT_F,  // A ~ F
    LDIGIT_G,
    LDIGIT_H,
    LDIGIT_I,
    LDIGIT_J,
    LDIGIT_L,
    LDIGIT_n,
    LDIGIT_o,
    LDIGIT_P,
    LDIGIT_q,
    LDIGIT_r,
    LDIGIT_S,
    LDIGIT_t,
    LDIGIT_U,
    LDIGIT_u,
    LDIGIT_y,
    LDIGIT_SPACE,
    LDIGIT_BAR,
    LDIGIT_NUM
} Ldigit_type;

#define DOT_COM1 (uint32_t)0x01
#define DOT_COM2 (uint32_t)0x02
#define DOT_COM3 (uint32_t)0x04
#define DOT_COM4 (uint32_t)0x08

void mif_meter_parm_set(void);
void lcd_ASSOCIATED_disp(void);
void dsp_cal_mem_bk_state(void);

extern ST_MTP_CON g_st_mtp_con;
extern ratekind_type rtkind_bakup;
extern U8 mrdate_bakup;

extern uint8_t g_mdlink_mode;
extern bool test_lcd_on_sts;  // jp.kim 24.11.08  LCD창 "TEST" 표시
extern bool cal_monitor_only;

#if 1  // jp.kim 25.06.30
extern int prev_keyrd;
extern int keyrd_chat;  // initially different from keyrd
extern int keychat;
extern int keycont_s;
extern int keycont_l;
#endif

uint8_t mt_kind_key_set_buf = 0;
uint8_t mr_date_key_set_buf = 0;

uint8_t lcd_map_comm[LCD_MAP_SIZE];

/* bccho, 주석추가, 2023-08-04, 8개 digit에 표시할 문자 */
uint8_t lcd_digit_buf[NUM_MODE_DIGIT];

uint8_t lcd_input_buf[NUM_DIGIT];

disp_state_type disp_state;
disp_mode_type dsp_mode_in_state;
uint8_t lcd_allon_step;

dsp_supply_type circ_state_suppdsp_mode;
const disp_mode_type* dsp_circ_mode_kind;

uint8_t dsp_circ_mode_index;
uint8_t dsp_circ_mode_start_index;
uint8_t dsp_circ_mode_end_index;
bool disp_circ_mode_init;
bool dsp_mr_modified;
bool dsp_mxdm_modified;
bool dsp_rcntdm_modified;
bool dsp_rcntpf_modified;
uint8_t circ_mode_chg_sec;

uint8_t dsp_test_mode_index;

bool dr_dsp, r_sun_dsp, dsp_comm_ing;
bool on_sun_dsp;
uint8_t blink_timer;

bool dispinp_err;
bool dsp_off_blank;

// cal 관련
bool dsp_cal_st_ing;    // cal 시작 표시, push 에서 설정
bool dsp_cal_mode_ing;  // cal 종료, 전류 표시, push 에서 설정
bool dsp_cal_mode_end;  // cal get 성공, mtp rx 에서 설정

int dsp_pulse_inc;
uint8_t dsp_point_pos;
bool dsp_dot_minus;
bool skip_dsp_circ_blank;
bool circ_mode_chg_manually;

bool dsp_mode_chged;
dsp_unit_type dsp_unit;
disp_input_type disp_input_mode;
uint8_t digit_pos;
bool disp_circ_end_bm;
bool prepay_fract;
bool b_dsp_mxdm_time;
int dsp_toggle_for_v;

bool b_dsp_pulse_inc_timer = false;
uint8_t accmed_pls_inc;
int dsp_pulse_inc_timer;
int dsp_pulse_inc_timer_bak;

bool b_dsp_bm_finished;

#if 0
uint32_t DSP_SETTING_TABLE[8][2][2]=
{
    {{SEG26, LRAM_L_ADDR},  {SEG27, LRAM_L_ADDR}},
    {{SEG24, LRAM_L_ADDR},  {SEG25, LRAM_L_ADDR}},
    {{SEG34, LRAM_H_ADDR},  {SEG35, LRAM_H_ADDR}},
    {{SEG32, LRAM_H_ADDR},  {SEG33, LRAM_H_ADDR}},
    {{SEG29, LRAM_L_ADDR},  {SEG30, LRAM_L_ADDR}},
    {{SEG15, LRAM_L_ADDR},  {SEG28, LRAM_L_ADDR}},
    {{SEG12, LRAM_L_ADDR},  {SEG13, LRAM_L_ADDR}},
    {{SEG05, LRAM_L_ADDR},  {SEG06, LRAM_L_ADDR}},
};
#else
/*
uint32_t DSP_SETTING_TABLE[8][2][2]=
{
    {{SEG12, LRAM_L_ADDR},  {SEG11, LRAM_L_ADDR}},
    {{SEG14, LRAM_L_ADDR},  {SEG13, LRAM_L_ADDR}},
    {{SEG30, LRAM_L_ADDR},  {SEG15, LRAM_L_ADDR}},
    {{SEG32, LRAM_H_ADDR},  {SEG31, LRAM_L_ADDR}},
    {{SEG35, LRAM_H_ADDR},  {SEG34, LRAM_H_ADDR}},
    {{SEG25, LRAM_L_ADDR},  {SEG24, LRAM_L_ADDR}},
    {{SEG28, LRAM_L_ADDR},  {SEG27, LRAM_L_ADDR}},
    {{SEG36, LRAM_H_ADDR},  {SEG29, LRAM_L_ADDR}},
};
*/

#if 1 /* bccho, 2023-08-04 */
uint32_t DSP_SETTING_TABLE[8][2][2] = {
    /* 0 번째 숫자 */
    {{SEG04, LRAM_L_ADDR},                        /* abcd */
     {SEG03, LRAM_L_ADDR}},                       /* efg */
    {{SEG06, LRAM_L_ADDR}, {SEG05, LRAM_L_ADDR}}, /* 1 번째 숫자 */
    {{SEG08, LRAM_L_ADDR}, {SEG07, LRAM_L_ADDR}}, /* 2 번째 숫자 */
    {{SEG10, LRAM_L_ADDR}, {SEG09, LRAM_L_ADDR}}, /* 3 번째 숫자 */
    {{SEG13, LRAM_L_ADDR}, {SEG12, LRAM_L_ADDR}}, /* 4 번째 숫자 */
    {{SEG15, LRAM_L_ADDR}, {SEG14, LRAM_L_ADDR}}, /* 5 번째 숫자 */
    {{SEG18, LRAM_L_ADDR}, {SEG17, LRAM_L_ADDR}}, /* 6 번째 숫자 */
    {{SEG20, LRAM_L_ADDR}, {SEG19, LRAM_L_ADDR}}, /* 7 번째 숫자 */
};
#else
uint32_t DSP_SETTING_TABLE[8][2][2] = {
    /* 0 번째 숫자 */
    {{SEG12, LRAM_L_ADDR},  /* abcd */
     {SEG11, LRAM_L_ADDR}}, /* efg */

    {{SEG14, LRAM_L_ADDR}, {SEG13, LRAM_L_ADDR}}, /* 1 번째 숫자 */
    {{SEG30, LRAM_L_ADDR}, {SEG15, LRAM_L_ADDR}}, /* 2 번째 숫자 */
    {{SEG32, LRAM_H_ADDR}, {SEG31, LRAM_L_ADDR}}, /* 3 번째 숫자 */
    {{SEG35, LRAM_H_ADDR}, {SEG34, LRAM_H_ADDR}}, /* 4 번째 숫자 */
    {{SEG25, LRAM_L_ADDR}, {SEG24, LRAM_L_ADDR}}, /* 5 번째 숫자 */
    {{SEG40, LRAM_H_ADDR}, {SEG27, LRAM_L_ADDR}}, /* 6 번째 숫자 */
    {{SEG36, LRAM_H_ADDR}, {SEG41, LRAM_H_ADDR}}, /* 7 번째 숫자 */
};
#endif

#endif
static void dsp_input_state(void);
void dsp_lcdmem_move(void);
static void dsp_circ_state(uint8_t* tptr);
static void dsp_inp_mode_num(void);
static void dsp_inp_mode_digit(void);
static void dsp_inp_mode_dot(void);
static void dsp_item_num(uint8_t item);
void dsp_date(date_time_type* dt);
void dsp_time(date_time_type* dt);
static void dsp_energy(bool wh_w, uint32_t val);
static void dsp_prog_id_reg_date(void); /* bccho, 2024-05-17 */
static void dsp_reg_date(void);
static void dsp_test_reg_date(uint8_t t8);
static void dsp_test_tariff_rate(uint8_t t8);
static void dsp_error(void);
void dsp_digit(int32_t val, uint8_t dgt, uint8_t point, bool lead_zero);
static void dsp_drive_dot(void);
static void dsp_drive_digit(void);
static void dsp_dot_unit(void);
static void dsp_dot_point(void);
static void dsp_dot_pls_quart(void);
static void dsp_dot_load(void);
static void dsp_dot_phase(void);
static void dsp_dot_rate(void);  // A B C D
static bool dsp_is_circ_update(void);
static void dsp_all_mode(uint8_t* tptr);
static void dsp_next_circ_mode(void);
static void dsp_test_state(uint8_t* tptr);
static uint8_t dsp_calc_digit(uint8_t ten, uint8_t one);
static void dsp_mode_chged_set(void);
static bool dsp_is_mode_chged(void);
static void dsp_tS(void);
static void dsp_smode(void);
static void dsp_pvt(void);
static void dsp_pf(uint8_t _pf);
static uint16_t dsp_error_mode_val(void);
static void dsp_fill_lcd_map(const uint8_t* tptr);
static void dsp_dot_secret(void);
static void dsp_dot_batmem(void);

/* bccho, 2024-09-05, 삼상 */
static void lcd_SUN_disp(void);

static void dsp_dot_test(void);
static void dsp_dot_CL(void);
static void dsp_dot_err(void);
static void dsp_mDR_num(void);
static void dsp_lcd_map_comm_clear(void);
static void dsp_fill_lcd_map_comm(uint8_t* tptr);
static void dsp_fill_decimal_4digit(uint8_t* _buf, uint16_t _val);
static uint16_t dsp_calc_3digit(uint8_t hund, uint8_t ten, uint8_t one);
static void dsp_fill_decimal_5digit(uint8_t* _buf, uint16_t _val);
static void dsp_fill_decimal_6digit(uint8_t* _buf, uint32_t _val);
static void dsp_latchon(void);

static void dsp_overcurr_set(void);

static bool dsp_fill_error_item(uint8_t* dgt_buf);

static void dsp_SWVER_sys(void);
static void dsp_SWVER_modem(void);
static void dsp_SWVER_mtp(void);
static void dsp_VAR_VA_SEL(uint8_t pls);
static void dsp_COMSPEED(baudrate_type baud);
static void dsp_auto_bi_dir_SEL(void);
static void dsp_condensor_test(uint8_t conds);
void dsp_test_condensor_toggle(void);
void dsp_test_err_pusle_toggle(void);
bool dsp_is_test_condensor(void);
bool dsp_is_test_err_pusle(void);
bool dsp_is_test_auto_bidir(void);
void dsp_test_auto_bidir_toggle(void);
void dsp_r_sun_state(void);
void dsp_on_sun_state(void);

#if PHASE_NUM != SINGLE_PHASE
static void dsp_err_rate1(void);
static void dsp_err_rate2(void);
static void dsp_err_rate3(void);

#endif

void dsp_cal_end_state(void);

//////////////////////////
// 규격 3.17.1 순환 표시항목 //
//////////////////////////

// private display mode
static const disp_mode_type circ_state_pvt_mode[] = {DSPMODE_DATE,
                                                     DSPMODE_TIME};

// simple display mode (단방향)
static const disp_mode_type circ_state_smode_unidir[] = {
    /* 단순 검침 모드 표시항목 : 수전 (항목:01) */
    DSPMODE_CU_IMP_ACT_T /* 현재 누적 수전 유효전력량 (kWh)_전체 */};

// simple display mode (양방향)
static const disp_mode_type circ_state_smode_bothdir[] = {
    /* 단순 검침 모드 표시항목 : 송/수전 (항목:02) */
    DSPMODE_CU_IMP_ACT_T, /* 현재 누적 수전 유효전력량 (kWh)_전체 */
    DSPMODE_CU_EXP_ACT_T  /* 현재 누적 송전 유효전력량 (kWh)_전체 */
};

#if METER_ID == 53
// non-simple display mode (1종 단방향)
static const disp_mode_type circ_state_nsmode_onerate_unidir[] = {
    /* 1종 순환 표시항목(단독 1P2W 60A 품목) : 수전 (항목:10) */
    /*
        현재날짜 01
        현재시간 02
        정기 검침일 03
        전월 누적 수전 유효전력량(kWh)_전체 04
        전월 누적 수전 유효 최대수요전력 (kW) 05
        전전월 누적 수전 유효 최대수요전력 (kW) 06
        현월 누적 수전 유효전력량(kWh)_전체 07
        현월 수전 유효 최대수요전력(kW) 08
        현월 수전 유효 최대수요전력 발생 날짜 09
        현월 수전 유효 최대수요전력 발생 시간 09
        직전 수요전력(kW) 10
    */
    DSPMODE_DATE, /* 현재 날짜 */
    DSPMODE_TIME, /* 현재 시간 */

#if 1 /* bccho, 2024-05-17 */
    DSPMODE_TOU_PROG_ID,
#else
    DSPMODE_REG_DATE, /* 정기 검침일 */
#endif

    DSPMODE_BF_IMP_ACT_T,        /* 전월 누적 수전 유효전력량(kWh)_전체 */
    DSPMODE_BF_IMP_ACT_CUMMX_T,  /* 전월 누적 수전 유효 최대수요전력 (kW) */
    DSPMODE_BBF_IMP_ACT_CUMMX_T, /* 전전월 누적 수전 유효 최대수요전력 (kW) */
    DSPMODE_CU_IMP_ACT_T,        /* 현월 누적 수전 유효전력량(kWh)_전체 */
    DSPMODE_CU_IMP_ACT_MX_T,     /* 현월 수전 유효 최대수요전력(kW) */
    DSPMODE_CU_IMP_ACT_MXTIM_T, /* 현월 수전 유효 최대수요전력 발생 날짜/시간 */
    DSPMODE_LAST_IMP_ACT,       /* 직전 수요전력(kW) */
};

// non-simple display mode (1종 양방향)
static const /*__code*/ disp_mode_type circ_state_nsmode_onerate_bothdir[] = {
    /* 1종 순환 표시항목(단독 1P2W 60A 품목) : 송/수전 (항목:12) */
    /*
        현재날짜 01
        현재시간 02
        정기 검침일 03
        전월 누적 수전 유효전력량(kWh)_전체 04
        전월 누적 송전 유효전력량(kWh)_전체 05
        전월 누적 수전 유효 최대수요전력 (kW) 06
        전전월 누적 수전 유효 최대수요전력 (kW) 07
        현월 누적 수전 유효전력량(kWh)_전체 08
        현월 누적 송전 유효전력량(kWh)_전체 09
        현월 수전 유효 최대수요전력(kW) 10
        현월 수전 유효 최대수요전력 발생 날짜 11
        현월 수전 유효 최대수요전력 발생 시간 11
        직전 수요전력(kW) 12
    */
    DSPMODE_DATE,
    DSPMODE_TIME,

#if 1 /* bccho, 2024-05-17 */
    DSPMODE_TOU_PROG_ID,
#else
    DSPMODE_REG_DATE, /* 정기 검침일 */
#endif

    DSPMODE_BF_IMP_ACT_T,
    DSPMODE_BF_EXP_ACT_T, /* 전월 누적 송전 유효전력량(kWh)_전체 */
    DSPMODE_BF_IMP_ACT_CUMMX_T,
    DSPMODE_BBF_IMP_ACT_CUMMX_T,
    DSPMODE_CU_IMP_ACT_T,
    DSPMODE_CU_EXP_ACT_T, /* 현월 누적 송전 유효전력량(kWh)_전체 */
    DSPMODE_CU_IMP_ACT_MX_T,
    DSPMODE_CU_IMP_ACT_MXTIM_T,
    DSPMODE_LAST_IMP_ACT,
};

// non-simple display mode (2종 단방향)
static const /*__code*/ disp_mode_type circ_state_nsmode_tworate_unidir[] = {
    /* 2종 순환 표시항목(단독 1P2W 60A 품목) : 수전 (항목:12) */
    /*
        현재날짜 01
        현재시간 02
        정기 검침일 03
        전월 누적 수전 유효전력량 (kWh)_중간부하 04
        전월 누적 수전 유효전력량 (kWh)_경부하 05
        전월 누적 수전 유효 최대수요전력(kW) 06
        전전월 누적 수전 유효 최대수요전력(kW) 07
        현월 누적 수전 유효전력량 (kWh)_중간부하 08
        현월 누적 수전 유효전력량 (kWh)_경부하 09
        현월 수전 유효 최대수요전력(kW) 10
        현월 수전 유효 최대수요전력 발생 날짜 11
        현월 수전 유효 최대수요전력 발생 시간 11
        직전 수요전력(kW) 12
    */
    DSPMODE_DATE,
    DSPMODE_TIME,

#if 1 /* bccho, 2024-05-17 */
    DSPMODE_TOU_PROG_ID,
#else
    DSPMODE_REG_DATE, /* 정기 검침일 */
#endif

    DSPMODE_BF_IMP_ACT_B, /* 전월 누적 수전 유효전력량 (kWh)_중간부하 */
    DSPMODE_BF_IMP_ACT_A, /* 전월 누적 수전 유효전력량 (kWh)_경부하 */
    DSPMODE_BF_IMP_ACT_CUMMX_T,
    DSPMODE_BBF_IMP_ACT_CUMMX_T,
    DSPMODE_CU_IMP_ACT_B, /* 현월 누적 수전 유효전력량 (kWh)_중간부하 */
    DSPMODE_CU_IMP_ACT_A, /* 현월 누적 수전 유효전력량 (kWh)_경부하 */
    DSPMODE_CU_IMP_ACT_MX_T,
    DSPMODE_CU_IMP_ACT_MXTIM_T,
    DSPMODE_LAST_IMP_ACT,
};

// non-simple display mode (2종 양방향)
static const /*__code*/ disp_mode_type circ_state_nsmode_tworate_bothdir[] = {
    /* 2종 순환 표시항목(단독 1P2W 60A 품목) : 송/수전 (항목:14) */
    /*
        현재날짜 01
        현재시간 02
        정기 검침일 03
        전월 누적 수전 유효전력량 (kWh)_중간부하 04
        전월 누적 수전 유효전력량 (kWh)_경부하 05
        전월 누적 송전 유효전력량 (kWh)_전체 06
        전월 누적 수전 유효 최대수요전력(kW) 07
        전전월 누적 수전 유효 최대수요전력(kW) 08
        현월 누적 수전 유효전력량 (kWh)_중간부하 09
        현월 누적 수전 유효전력량 (kWh)_경부하 10
        현월 누적 송전 유효전력량 (kWh)_전체 11
        현월 수전 유효 최대수요전력(kW) 12
        현월 수전 유효 최대수요전력 발생 날짜 13
        현월 수전 유효 최대수요전력 발생 시간 13
        직전 수요전력(kW) 14
    */
    DSPMODE_DATE,
    DSPMODE_TIME,

#if 1 /* bccho, 2024-05-17 */
    DSPMODE_TOU_PROG_ID,
#else
    DSPMODE_REG_DATE, /* 정기 검침일 */
#endif

    DSPMODE_BF_IMP_ACT_B,
    DSPMODE_BF_IMP_ACT_A,
    DSPMODE_BF_EXP_ACT_T, /* 전월 누적 송전 유효전력량 (kWh)_전체 */
    DSPMODE_BF_IMP_ACT_CUMMX_T,
    DSPMODE_BBF_IMP_ACT_CUMMX_T,
    DSPMODE_CU_IMP_ACT_B,
    DSPMODE_CU_IMP_ACT_A,
    DSPMODE_CU_EXP_ACT_T, /* 현월 누적 송전 유효전력량 (kWh)_전체 */
    DSPMODE_CU_IMP_ACT_MX_T,
    DSPMODE_CU_IMP_ACT_MXTIM_T,
    DSPMODE_LAST_IMP_ACT,
};

#else  // 1p 60A   이외 모델
// non-simple display mode (1종 단방향)
static const /*__code*/ disp_mode_type circ_state_nsmode_onerate_unidir[] = {
    /* 1종 순환 표시항목(단독 1P2W 60A 품목) : 수전 (항목:10) */
    /*
        현재날짜 01
        현재시간 02
        정기 검침일 03
        전월 누적 수전 유효전력량(kWh)_전체 04
        전월 수전 유효 최대수요전력 (kW) 05
        전월 수전 유효 최대수요전력 발생 날짜 06
        전월 수전 유효 최대수요전력 발생 시간 06
        현월 누적 수전 유효전력량(kWh)_전체 07
        현월 수전 유효 최대수요전력(kW) 08
        현월 수전 유효 최대수요전력 발생 날짜 09
        현월 수전 유효 최대수요전력 발생 시간 09
        직전 수요전력(kW) 10
    */
    DSPMODE_DATE, /* 현재 날짜 */
    DSPMODE_TIME, /* 현재 시간 */
#ifdef DISPLAY_MODE_TOU_PROG_ID
    DSPMODE_TOU_PROG_ID,  //
#else
    DSPMODE_REG_DATE, /* 정기 검침일 */
#endif
    DSPMODE_BF_IMP_ACT_T, /* 전월 누적 수전 유효전력량(kWh)_전체 */
#if 0
	DSPMODE_BF_IMP_ACT_CUMMX_T, /* 전월 누적 수전 유효 최대수요전력 (kW) */
#else
    DSPMODE_BF_IMP_ACT_MX_T, /* 전월 수전 유효 최대수요전력 (kW) */
#endif
    DSPMODE_BF_IMP_ACT_MXTIM_T, /* 전월 수전 유효 최대수요전력 발생 날짜/시간 */
    DSPMODE_CU_IMP_ACT_T,       /* 현월 누적 수전 유효전력량(kWh)_전체 */
    DSPMODE_CU_IMP_ACT_MX_T,    /* 현월 수전 유효 최대수요전력(kW) */
    DSPMODE_CU_IMP_ACT_MXTIM_T, /* 현월 수전 유효 최대수요전력 발생 날짜/시간 */
    DSPMODE_LAST_IMP_ACT,       /* 직전 수요전력(kW) */

};

// non-simple display mode (1종 양방향)
static const /*__code*/ disp_mode_type circ_state_nsmode_onerate_bothdir[] = {
    /* 1종 순환 표시항목(단독 1P2W 60A 품목) : 송/수전 (항목:12) */
    /*
        현재날짜 01
        현재시간 02
        정기 검침일 03
        전월 누적 수전 유효전력량(kWh)_전체 04
        전월 누적 송전 유효전력량(kWh)_전체 05
        전월 수전 유효 최대수요전력 (kW) 06
        전월 수전 유효 최대수요전력 발생 날짜 07
        전월 수전 유효 최대수요전력 발생 시간 07
        현월 누적 수전 유효전력량(kWh)_전체 08
        현월 누적 송전 유효전력량(kWh)_전체 09
        현월 수전 유효 최대수요전력(kW) 10
        현월 수전 유효 최대수요전력 발생 날짜 11
        현월 수전 유효 최대수요전력 발생 시간 11
        직전 수요전력(kW) 12
    */
    DSPMODE_DATE,
    DSPMODE_TIME,
#ifdef DISPLAY_MODE_TOU_PROG_ID
    DSPMODE_TOU_PROG_ID,  //
#else
    DSPMODE_REG_DATE, /* 정기 검침일 */
#endif
    DSPMODE_BF_IMP_ACT_T,
    DSPMODE_BF_EXP_ACT_T, /* 전월 누적 송전 유효전력량(kWh)_전체 */
#if 0
	DSPMODE_BF_IMP_ACT_CUMMX_T, /* 전월 누적 수전 유효 최대수요전력 (kW) */
#else
    DSPMODE_BF_IMP_ACT_MX_T, /* 전월 수전 유효 최대수요전력 (kW) */
#endif
    DSPMODE_BF_IMP_ACT_MXTIM_T, /* 전월 수전 유효 최대수요전력 발생 날짜/시간 */
    DSPMODE_CU_IMP_ACT_T,
    DSPMODE_CU_EXP_ACT_T, /* 현월 누적 송전 유효전력량(kWh)_전체 */
    DSPMODE_CU_IMP_ACT_MX_T,
    DSPMODE_CU_IMP_ACT_MXTIM_T,
    DSPMODE_LAST_IMP_ACT,

};

// non-simple display mode (2종 단방향)
static const /*__code*/ disp_mode_type circ_state_nsmode_tworate_unidir[] = {
    /* 2종 순환 표시항목(단독 1P2W 60A 품목) : 수전 (항목:16) */
    /*
    현재날짜 01
    현재시간 02
    정기 검침일 03
    전월 누적 수전 유효전력량 (kWh)_중간부하 04
    전월 누적 수전 유효전력량 (kWh)_경부하 05
    전월 누적 수전 선택무효전력량 (kWh)_중간부하06
    //전월 누적 수전 유효 최대수요전력(kW) 08
    전월 수전 유효 최대수요전력(kW) 07
    전월 수전 유효 최대수요전력 발생 날짜 08
    전월 수전 유효 최대수요전력 발생 시간 08
    전월 수전 평균 역율(%)_중간부하 09
    현월 누적 수전 유효전력량 (kWh)_중간부하 10
    현월 누적 수전 유효전력량 (kWh)_경부하 11
    현월 누적 수전 선택무효전력량 (kWh)_중간부하12
    현월 수전 유효 최대수요전력(kW) 13
    현월 수전 유효 최대수요전력 발생 날짜 14
    현월 수전 유효 최대수요전력 발생 시간 14
    현월 수전 평균 역율(%)_중간부하 15
    직전 수요전력(kW) 16
    */
    DSPMODE_DATE,
    DSPMODE_TIME,
#ifdef DISPLAY_MODE_TOU_PROG_ID
    DSPMODE_TOU_PROG_ID,  //
#else
    DSPMODE_REG_DATE, /* 정기 검침일 */
#endif

    DSPMODE_BF_IMP_ACT_B, /* 전월 누적 수전 유효전력량 (kWh)_중간부하 */
    DSPMODE_BF_IMP_ACT_A, /* 전월 누적 수전 유효전력량 (kWh)_경부하 */
    DSPMODE_BF_IMP_REACT_B,
#if 0	
	DSPMODE_BF_IMP_ACT_CUMMX_T,
#endif
    DSPMODE_BF_IMP_ACT_MX_T,
    DSPMODE_BF_IMP_ACT_MXTIM_T, /* 전월 수전 유효 최대수요전력 발생 날짜/시간 */
    DSPMODE_BF_IMP_PF_B,

    DSPMODE_CU_IMP_ACT_B, /* 현월 누적 수전 유효전력량 (kWh)_중간부하 */
    DSPMODE_CU_IMP_ACT_A, /* 현월 누적 수전 유효전력량 (kWh)_경부하 */
    DSPMODE_CU_IMP_REACT_B,
    DSPMODE_CU_IMP_ACT_MX_T,
    DSPMODE_CU_IMP_ACT_MXTIM_T,
    DSPMODE_CU_IMP_PF_B,

    DSPMODE_LAST_IMP_ACT,

};

// non-simple display mode (2종 양방향)
static const /*__code*/ disp_mode_type circ_state_nsmode_tworate_bothdir[] = {
    /* 2종 순환 표시항목(단독 1P2W 60A 품목) : 송/수전 (항목:14) */
    /*
        현재날짜 01
        현재시간 02
        정기 검침일 03
        전월 누적 수전 유효전력량 (kWh)_중간부하 04
        전월 누적 수전 유효전력량 (kWh)_경부하 05
        전월 누적 선택 무효전력량 (kWh)_중간부하 06
        전월 누적 송전 유효전력량 (kWh)_전체 07
        전월 누적 수전 유효 최대수요전력(kW) 08
        전월 수전 유효 최대수요전력(kW) 09
        전월 수전 유효 최대수요전력 발생 날짜 10
        전월 수전 유효 최대수요전력 발생 시간 10
        전월 수전 평균 역율(%)_중간부하 11

        현월 누적 수전 유효전력량 (kWh)_중간부하 12
        현월 누적 수전 유효전력량 (kWh)_경부하 13
        현월 누적 수전 선택무효전력량 (kWh)_중간부하14
        현월 누적 송전 유효전력량 (kWh)_전체 15
        현월 수전 유효 최대수요전력(kW) 16
        현월 수전 유효 최대수요전력 발생 날짜 17
        현월 수전 유효 최대수요전력 발생 시간 17
        현월 수전 평균 역율(%)_중간부하 18
        직전 수요전력(kW) 19
    */
    DSPMODE_DATE,
    DSPMODE_TIME,
#ifdef DISPLAY_MODE_TOU_PROG_ID
    DSPMODE_TOU_PROG_ID,  //
#else
    DSPMODE_REG_DATE, /* 정기 검침일 */
#endif

    DSPMODE_BF_IMP_ACT_B,
    DSPMODE_BF_IMP_ACT_A,
    DSPMODE_BF_IMP_REACT_B,
    DSPMODE_BF_EXP_ACT_T, /* 전월 누적 송전 유효전력량 (kWh)_전체 */
#if 0	
	DSPMODE_BF_IMP_ACT_CUMMX_T,
#endif
    DSPMODE_BF_IMP_ACT_MX_T,    /* 전월 수전 유효 최대수요전력 (kW) */
    DSPMODE_BF_IMP_ACT_MXTIM_T, /* 전월 수전 유효 최대수요전력 발생 날짜/시간 */
    DSPMODE_BF_IMP_PF_B,

    DSPMODE_CU_IMP_ACT_B,
    DSPMODE_CU_IMP_ACT_A,
    DSPMODE_CU_IMP_REACT_B,
    DSPMODE_CU_EXP_ACT_T, /* 현월 누적 송전 유효전력량 (kWh)_전체 */
    DSPMODE_CU_IMP_ACT_MX_T,
    DSPMODE_CU_IMP_ACT_MXTIM_T,
    DSPMODE_CU_IMP_PF_B,

    DSPMODE_LAST_IMP_ACT,

};

#endif

#if PHASE_NUM == SINGLE_PHASE
/* 3.17.2 시험 모드 표시항목 */
static const /*__code*/ disp_mode_type test_state_mode1[] = {
    /* 1P2W 시험 모드 표시항목 순서 */
    DSPMODE_V1,     /* A상 상전압 (XXX.XXX [V]), A-B 선간 전압 (XXX.XXX [V]) */
    DSPMODE_I1,     /* A상 상전류 (XXX.XXX [A]) */
    DSPMODE_P1,     /* A상 전압·전류 위상 각 (XXX : 0 ~ 360) */
    DSPMODE_THD_V1, /* A상 전압 THD (XXX.XXX [%]) */
    DSPMODE_tS, /* tS 제어 (타임스위치 제어 시험), [메뉴] 버튼으로 제어 시험
                   가능 */
    DSPMODE_TEMP, /* 온도표시 (XX) */
    DSPMODE_SMODE, /* S-OFF/S-On (단순 검침 모드 설정/해제), [메뉴] 버튼으로
                      설정 가능 */
    DSPMODE_FREQ, /* 주파수 (Hz) */
    DSPMODE_PVT, /* Pvt-E/Pvt-d (무부하 시 부하 동작 표시 설정/해제), [메뉴]
                    버튼으로 설정 가능 */
    DSPMODE_SYSP_SW_VER, /* 운영 소프트웨어 버전 */
    DSPMODE_MODEM_VER,   /* 내장 모뎀 소프트웨어 버전 */
    DSPMODE_MTP_SW_VER,  /* 계량 소프트웨어 버전 */
    DSPMODE_485_BPS,     /* RS-485 현재 통신 속도 */
    DSPMODE_CONDENSOR_EN, /* C-d/C-E (오결선 : 콘덴서 미부설/부설), [메뉴]
                             버튼으로 설정 가능 */
    DSPMODE_ERR_PULSE, /* 무효 (rt), 피상 (Pt), 수요 시한 (EOI) 선택, [메뉴]
                          버튼으로 설정 가능, 피상으로 설정 시 유효전력량 펄스는
                          수요 시한 펄스로 출력하며 펄스 폭은 50 ~ 200ms 이내로
                          한다 */
    DSPMODE_AUTO_BI_DIR_MODE, /* 자동모드 전환 On/OFF, [메뉴] 버튼으로 설정 가능
                               */
    DSPMODE_OVERCURR, /* OC-E/OC-d (과부하 전류 차단 : 최대전류의 1.2배 계측 시
                         차단 설정/복귀), [메뉴] 버튼으로 설정 가능 - 래치
                         릴레이 사용 시 */
    DSPMODE_TEST_REG_DATE, /* 정기검침일, [메뉴] 버튼으로 설정 가능(01 ~ 28) */
    DSPMODE_TARIFF_RATE, /* 종별설정(1~2종), [메뉴] 버튼으로 설정 가능(1 ~ 2) */
    NUM_DSPMODE          // end of table
};
#else /* bccho, 2024-09-05, 삼상 */
static const /*__code*/ disp_mode_type test_state_mode1[] = {
    /* 3P4W 시험 모드 표시항목 순서 */
    DSPMODE_LtoL1,  /* A상 상전압 (XXX.XXX [V]), A-B 선간 전압 (XXX.XXX [V]) */
    DSPMODE_LtoL2,  /* B상 상전압 (XXX.XXX [V]), A-B 선간 전압 (XXX.XXX [V]) */
    DSPMODE_LtoL3,  /* C상 상전압 (XXX.XXX [V]), A-B 선간 전압 (XXX.XXX [V]) */
    DSPMODE_I1,     /* A상 상전류 (XXX.XXX [A]) */
    DSPMODE_I2,     /* B상 상전류 (XXX.XXX [A]) */
    DSPMODE_I3,     /* C상 상전류 (XXX.XXX [A]) */
    DSPMODE_P1,     /* A상 전압·전류 위상 각 (XXX : 0 ~ 360) */
    DSPMODE_P2,     /* B상 전압·전류 위상 각 (XXX : 0 ~ 360) */
    DSPMODE_P3,     /* C상 전압·전류 위상 각 (XXX : 0 ~ 360) */
    DSPMODE_THD_V1, /* A상 전압 THD (XXX.XXX [%]) */
    DSPMODE_THD_V2, /* B상 전압 THD (XXX.XXX [%]) */
    DSPMODE_THD_V3, /* C상 전압 THD (XXX.XXX [%]) */
    DSPMODE_tS, /* tS 제어 (타임스위치 제어 시험), [메뉴] 버튼으로 제어 시험
                   가능 */
    DSPMODE_TEMP, /* 온도표시 (XX) */
    DSPMODE_SMODE, /* S-OFF/S-On (단순 검침 모드 설정/해제), [메뉴] 버튼으로
                      설정 가능 */
    DSPMODE_FREQ, /* 주파수 (Hz) */
    DSPMODE_PVT, /* Pvt-E/Pvt-d (무부하 시 부하 동작 표시 설정/해제), [메뉴]
                    버튼으로 설정 가능 */
    DSPMODE_SYSP_SW_VER, /* 운영 소프트웨어 버전 */
    DSPMODE_MODEM_VER,   /* 내장 모뎀 소프트웨어 버전 */
    DSPMODE_MTP_SW_VER,  /* 계량 소프트웨어 버전 */
    DSPMODE_485_BPS,     /* RS-485 현재 통신 속도 */
    DSPMODE_CONDENSOR_EN, /* C-d/C-E (오결선 : 콘덴서 미부설/부설), [메뉴]
                             버튼으로 설정 가능 */
    DSPMODE_ERR_PULSE, /* 무효 (rt), 피상 (Pt), 수요 시한 (EOI) 선택, [메뉴]
                          버튼으로 설정 가능, 피상으로 설정 시 유효전력량 펄스는
                          수요 시한 펄스로 출력하며 펄스 폭은 50 ~ 200ms 이내로
                          한다 */
    DSPMODE_AUTO_BI_DIR_MODE, /* 자동모드 전환 On/OFF, [메뉴] 버튼으로 설정 가능
                               */
    DSPMODE_OVERCURR, /* OC-E/OC-d (과부하 전류 차단 : 최대전류의 1.2배 계측 시
                         차단 설정/복귀), [메뉴] 버튼으로 설정 가능 - 래치
                         릴레이 사용 시 */
    DSPMODE_TEST_REG_DATE, /* 정기 검침일, [메뉴] 버튼으로 설정 가능 (01 ~ 28)
                            */
    DSPMODE_TARIFF_RATE, /* 종별 설정 (1~2종), [메뉴] 버튼으로 설정 가능 (1 ~ 2)
                          */
    NUM_DSPMODE  // end of table
};
#endif

#define sega (0x08 << 8)
#define segb (0x10 << 8)
#define segc (0x20 << 8)

#define segd (0x40)
#define sege (0x20)
#define segf (0x08)
#define segg (0x10)

static const /*__code*/ uint8_t digit_font_b[LDIGIT_NUM] = {
    //     	   abcd, fge0
    0xfa,  // 0:       1111  1010
    0x60,  // 1:       0110  0000
    0xd6,  //  2:      1101  0110
    0xf4,  //  3:      1111  0100
    0x6c,  //  4:      0110  1100
    0xbc,  //  5:      1011  1100
    0xbe,  //  6:      1011  1110
    0xe8,  //  7:      1110  1000
    0xfe,  //  8:      1111  1110
    0xfc,  //  9:      1111  1100
    0xee,  //  A:      1110  1110
    0x3e,  //  b:      0011  1110
    0x9a,  //  C:      1001  1010
    0x76,  //  d:      0111  0110
    0x9e,  //  E:      1001  1110
    0x8e,  //  F:      1000  1110
    0xba,  //  G: 	   1011  1010
    0x6e,  //  H:      0110  1110
    0x0a,  //  I: 	   0000  1010
    0x72,  //  J: 	   0111  0010
    0x1a,  //  L:      0001  1010
    0x26,  //  n:      0010  0110
    0x36,  //  o:	   0011  0110
    0xce,  //	P:	   1100  1110
    0xec,  //  q:	   1110  1100
    0x06,  //  r:      0000  0110
    0xbc,  //  S:      1011  1100
    0x1e,  //  t:      0001  1110
    0x7a,  //  U:      0111  1010
    0x32,  //  u:      0011  0010
    0x7c,  //  y:      0111  1100
    0x00,  //   :      0000  0000
    0x04,  //  -:      0000  0100
};

void disp_init(void)
{
    dsp_circ_state_init();

    lcd_allon_step = 0;
    dr_dsp = 0;
    dsp_cal_mode_ing = false;
    dsp_cal_mode_end = false;
    dsp_cal_st_ing = false;
    r_sun_dsp = 0;
    on_sun_dsp = 0;
    dsp_comm_ing = 0;
    dsp_mr_modified = 0;
    dsp_mxdm_modified = 0;
    dsp_rcntdm_modified = 0;
    dsp_rcntpf_modified = 0;
    dsp_pulse_inc = 0;

    skip_dsp_circ_blank = 1;
    circ_mode_chg_manually = 0;

    b_dsp_mxdm_time = 0;
    prepay_fract = 0;
    b_dsp_pulse_inc_timer = false;
    dsp_pulse_inc_timer = 0;
    accmed_pls_inc = 0;

    b_dsp_bm_finished = false;
    dsp_update_timeset(0);
}

void dsp_pulse_inc_set(int incr)
{
    b_dsp_pulse_inc_timer = false;
    accmed_pls_inc += incr;
    if (accmed_pls_inc)
    {
        dsp_pulse_inc_timer_bak =
            (int)(1000.0 / (float)TICK_PERIOD / (float)accmed_pls_inc);

        b_dsp_pulse_inc_timer = true;
    }
}

void dsp_circ_state_init(void)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    disp_state = DISP_CIRC_STATE;  // display main state initialized

    disp_circ_mode_init = 1;
    circ_mode_chg_manually = false;
    disp_circ_end_bm = false;
    key_circ_step_mode_clear();
}

void dsp_circ_state_mode_init(void) { disp_circ_mode_init = 1; }

void dsp_test_state_init(void)
{
    disp_state = DISP_TEST_STATE;
    dsp_test_mode_index = 0;
    test_lcd_on_sts = 0;  // jp.kim 24.11.08  LCD창 "TEST" 표시 off

    dsp_mode_chged_set();
}

void dsp_inp_init(date_time_type* pdt)
{
    disp_state = DISP_INPUT_STATE;
    dsp_instate_timeset(T30SEC);

    disp_input_mode = DISPINP_DATE;
    digit_pos = 5;
    dispinp_err = 0;

    lcd_input_buf[0] = (uint8_t)(pdt->year / 10);
    lcd_input_buf[1] = (uint8_t)(pdt->year % 10);
    lcd_input_buf[2] = (uint8_t)(pdt->month / 10);
    lcd_input_buf[3] = (uint8_t)(pdt->month % 10);
    lcd_input_buf[4] = (uint8_t)(pdt->date / 10);
    lcd_input_buf[5] = (uint8_t)(pdt->date % 10);

    input_mode_bat_installed_set();
}

bool dsp_is_circ_state(void) { return (bool)(disp_state == DISP_CIRC_STATE); }

bool dsp_is_input_state(void) { return (bool)(disp_state == DISP_INPUT_STATE); }

bool dsp_is_test_state(void) { return (bool)(disp_state == DISP_TEST_STATE); }

static disp_mode_type get_circ_dsp_mode(uint8_t _idx)
{
    if (dsp_circ_mode_kind == (const disp_mode_type*)NULL)
    {
        return (disp_mode_type)circ_state_suppdsp_mode.dsp_mode[_idx];
    }
    return dsp_circ_mode_kind[_idx];
}

disp_state_type get_disp_state(void) { return disp_state; }

void dsp_circ_time_set(U8 bf_sec, U8 ss)
{
    U8 t8;

    // 잔여 시간 구하기
    if (circ_mode_chg_sec < bf_sec)
        t8 = (U8)((60 + circ_mode_chg_sec) - bf_sec);
    else
        t8 = (U8)(circ_mode_chg_sec - bf_sec);

    // 변경후 시간 + 잔여시간
    t8 += ss;

#if 0
//1초 변경 순간을 위한 여유시간 (최악의 경우 1초 길어 질 수 있음)   
	t8 += 1;
#endif

    if (t8 >= 60)
        t8 -= 60;

    circ_mode_chg_sec = t8;
}

void dsp_blink_timer_proc(void)
{
    if (dsp_blink_is_timeout())
    {
        blink_timer++;
        dsp_blink_timeset(T500MS);
    }
}

static void dsp_state_monitor(void)
{
#if 0 /* bccho, 2024-01-10, 1227 포팅 */   
    // input state mode exits due to timeout
    if (dsp_is_input_state() && dsp_instate_is_timeout())
    {
        kact_inp_dsp_exit();
    }
#endif

    if (dsp_is_test_state())
    {
#if 1  // jp.kim 25.06.30
        if (test_lcd_on_sts)
        {
            test_lcd_on_sts = 0;

            prev_keyrd = 0;
            keyrd_chat = -1;  // initially different from keyrd
            keychat = 0;
            keycont_s = 0;
            keycont_l = 0;
            key_pressed = false;

            kact_test_dsp_exit();
        }
#endif

        if (dsp_is_test_mode_tS() && mt_is_time_sw() && !ts_is_zone_on() &&
            relay_is_load_on())
        {
            if (ts_mdexit_is_timeout())
            {
                kact_test_dsp_exit();
            }
        }
        else
        {
            if (dsp_mdexit_is_timeout())
            {
                kact_test_dsp_exit();
            }
        }
    }

    if (dr_dsp)
    {
        if (dsp_dr_is_timeout())
            dr_dsp = false;
    }
    if (r_sun_dsp)
    {
        if (dsp_r_sun_is_timeout())
            r_sun_dsp = false;
    }
    if (on_sun_dsp)
    {
        if (dsp_on_sun_is_timeout())
        {
            on_sun_dsp = false;
            // 디스플레이 표시 시간은 만료가 되지만. g_pmnt_op_mode 변수는
            // PMNT_NO_VOLT_SET_OP 으로 유지되고 있으므로, 무전압 설정 모드
            // 상태임.
            dsp_circ_state_init();
        }
    }
    if (dsp_comm_ing)
    {
        if (dsp_comm_is_ing_timeout())
            dsp_comm_ing = false;
    }

#if 0 /* bccho, 2025-09-18, 삭제 */    
    if (dsp_cal_st_ing)
    {
        if (dsp_cal_st_is_ing_timeout())
        {
            dsp_cal_st_ing = false;
            dsp_cal_mode_ing = false;
            dsp_cal_mode_end = false;
        }
    }
    if (dsp_cal_mode_ing)
    {
        if (dsp_cal_mode_is_ing_timeout())
            dsp_cal_mode_ing = false;
    }
#endif

    if (dsp_cal_mode_end)  // cal get 성공, mtp rx 에서 설정
    {
        if (dsp_cal_end_is_ing_timeout())
        {
            MSG09("dsp_cal_end_is_ing_timeout");
            dsp_cal_mode_end = false;
        }
    }
}

static bool dsp_update_q(void)
{
    if (dsp_update_is_timeout())
    {
        dsp_update_timeset(T500MS);
        return true;
    }

    return false;
}

static void dsp_mode_chged_set(void) { dsp_mode_chged = true; }

static bool dsp_is_mode_chged(void) { return dsp_mode_chged; }

static void dsp_var_init(void)
{
    dsp_point_pos = 0;
    dsp_dot_minus = 0;
    dsp_unit = eUnitNull;
}

static void dsp_var_clear(void)
{
    dsp_mode_chged = 0;
    disp_circ_mode_init = 0;
    circ_mode_chg_manually = 0;
    skip_dsp_circ_blank = 0;
    dsp_mr_modified = 0;
    dsp_mxdm_modified = 0;
    dsp_rcntdm_modified = 0;
    dsp_off_blank = 0;
    dsp_rcntpf_modified = 0;
}

void disp_proc(void)
{
    uint8_t* tptr;
    disp_state_type tdsp;

    tptr = adjust_tptr(&global_buff[0]);

    if (lcd_allon_step == 0)
    {
        lcd_allon_step = 2;
    }
    else if (lcd_allon_step == 1)
    {
        if (!dsp_update_is_timeout())
            return;
        lcd_allon_step++;
    }

    dsp_blink_timer_proc();

    dsp_state_monitor();

    if (!dsp_update_q())  // display update time
        return;

    if (dr_dsp)
        tdsp = DISP_DR_STATE;
    else if (r_sun_dsp)
        tdsp = DISP_R_SUN_STATE;
    else if (on_sun_dsp)
        tdsp = DISP_ON_SUN_STATE;
    else if (dsp_cal_st_ing)  // cal 시작 표시
        tdsp = DISP_CAL_STATE;
    else if (dsp_cal_mode_ing)  // 전류 표시
        tdsp = DISP_CAL_END_STATE;
    else if (dsp_cal_mode_end)  // cal 종료 표시
        tdsp = DISP_CAL_MEM_BK_STATE;
    else
        tdsp = disp_state;

    switch (tdsp)
    {
    case DISP_CIRC_STATE:
        dsp_circ_state(tptr);
        break;

        // basic configuration input state
    case DISP_INPUT_STATE:
        break;

    case DISP_TEST_STATE:
        dsp_test_state(tptr);
        break;

    case DISP_CAL_STATE:  // cal 시작 표시
        dsp_cal_state();
        break;

    case DISP_DR_STATE:
        dsp_dr_state();
        break;

    case DISP_CALFAIL_STATE:
        dsp_calfail_state();
        break;

    case DISP_R_SUN_STATE:
        dsp_r_sun_state();
        break;
    case DISP_ON_SUN_STATE:
        dsp_on_sun_state();
        break;
    case DISP_CAL_END_STATE:  // 전류 표시
        dsp_cal_end_state();
        break;
    case DISP_CAL_MEM_BK_STATE:  // cal 종료 표시
        dsp_cal_mem_bk_state();
        break;
    }

    if (dsp_is_circ_state() && !dsp_mdchg_is_timeout())
    {
        dsp_off_blank = 1;
    }
    else
    {
        dsp_off_blank = 0;
    }

    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    dsp_drive_dot();
    dsp_drive_digit();
    dsp_lcdmem_move();
    dsp_var_clear();
}

// called and refreshed by other module
void dsp_cal_state(void)
{
    dsp_var_init();

    lcd_digit_buf[2] = LDIGIT_SPACE;
    lcd_digit_buf[3] = LDIGIT_SPACE;
    lcd_digit_buf[4] = LDIGIT_SPACE;
    lcd_digit_buf[5] = LDIGIT_C;
    lcd_digit_buf[6] = LDIGIT_A;
    lcd_digit_buf[7] = LDIGIT_L;
}

void dsp_cal_end_state(void)
{
    MSG09("[dsp] cal_end_state");
    dsp_var_init();

    dsp_digit((int32_t)(get_inst_curr(0) * 1000.0), 6, 3, true);
    dsp_unit = eUnitA;
}

void dsp_cal_mem_bk_state(void)
{
    dsp_var_init();

    lcd_digit_buf[2] = LDIGIT_C;
    lcd_digit_buf[3] = LDIGIT_A;
    lcd_digit_buf[4] = LDIGIT_L;
    lcd_digit_buf[5] = LDIGIT_E;
    lcd_digit_buf[6] = LDIGIT_n;
    lcd_digit_buf[7] = LDIGIT_d;
}

// Sr-dr display
void dsp_dr_state(void)
{
    dsp_var_init();

    lcd_digit_buf[2] = LDIGIT_SPACE;
    lcd_digit_buf[3] = LDIGIT_5;
    lcd_digit_buf[4] = LDIGIT_r;
    lcd_digit_buf[5] = LDIGIT_BAR;
    lcd_digit_buf[6] = LDIGIT_d;
    lcd_digit_buf[7] = LDIGIT_r;
}

void dsp_r_sun_state(void)
{
    dsp_var_init();

    lcd_digit_buf[2] = LDIGIT_r;
    lcd_digit_buf[3] = LDIGIT_BAR;
    lcd_digit_buf[4] = LDIGIT_C;
    lcd_digit_buf[5] = LDIGIT_A;
    lcd_digit_buf[6] = LDIGIT_L;
    lcd_digit_buf[7] = LDIGIT_L;
}

void dsp_on_sun_state(void)
{
    dsp_var_init();

    lcd_digit_buf[2] = LDIGIT_o;
    lcd_digit_buf[3] = LDIGIT_n;
    lcd_digit_buf[4] = LDIGIT_BAR;
    lcd_digit_buf[5] = LDIGIT_5;
    lcd_digit_buf[6] = LDIGIT_U;
    lcd_digit_buf[7] = LDIGIT_n;
}

void dsp_calfail_state(void)
{
    dsp_var_init();

    lcd_digit_buf[2] = LDIGIT_C;
    lcd_digit_buf[3] = LDIGIT_A;
    lcd_digit_buf[4] = LDIGIT_F;
    lcd_digit_buf[5] = LDIGIT_A;
    lcd_digit_buf[6] = LDIGIT_I;
    lcd_digit_buf[7] = LDIGIT_L;
}

void dsp_circ_mode_chg_set(void)
{
    circ_mode_chg_manually = 1;
    skip_dsp_circ_blank = 1;
}

uint8_t get_dsp_circ_period(void)
{
    if (dsp_circ_mode_kind == (const disp_mode_type*)NULL)
        return circ_state_suppdsp_mode.dsptime;
    return DISP_CIRC_PERIOD;
}

bool dsp_circ_mode_chg_q(void)
{
    bool rtn;

    rtn = false;
    if (disp_circ_mode_init)
    {
        // initialized when entering circ_state first
        circ_mode_chg_sec =
            cur_sec + get_dsp_circ_period() + 1;  // include blank time (1)
        if (circ_mode_chg_sec >= 60)
            circ_mode_chg_sec -= 60;

        rtn = true;
    }
    else
    {
        if (circ_mode_chg_manually)
            goto dsp_circ_mode_chg_q1;

        if (cur_sec >= circ_mode_chg_sec)
        {
            // in case of power on when DISP_CIRC_PERIOD = 1
            if ((cur_sec - circ_mode_chg_sec) <= DISP_SKIP_MARGIN)
            {
            dsp_circ_mode_chg_q1:
                circ_mode_chg_sec = cur_sec + get_dsp_circ_period() +
                                    1;  // include blank time (1)
                if (circ_mode_chg_sec >= 60)
                    circ_mode_chg_sec -= 60;

                rtn = true;
            }
        }
        else
        {
            if (circ_mode_chg_sec > (cur_sec + get_dsp_circ_period() + 1))
            {
                goto dsp_circ_mode_chg_q1;
            }
        }
    }

    return rtn;
}

void dsp_get_circ_mode_kind(void)
{
    ratekind_type mtkind;

    mtkind = mt_rtkind;  // TOU 종별

    if (circdsp_is_pvt_mode())
    {
        dsp_circ_mode_kind = circ_state_pvt_mode;

        dsp_circ_mode_start_index = 0;
        dsp_circ_mode_end_index =
            (uint8_t)(sizeof(circ_state_pvt_mode) / sizeof(disp_mode_type) - 1);
    }
    else if (circdsp_is_smode())
    {
        // 단순 표시 모드
        if (mt_is_uni_dir())
        {
            // 수전, 단방향 (정방향)
            dsp_circ_mode_kind = circ_state_smode_unidir;

            dsp_circ_mode_start_index = 0;
            dsp_circ_mode_end_index =
                (uint8_t)(sizeof(circ_state_smode_unidir) /
                              sizeof(disp_mode_type) -
                          1);
        }
        else
        {
            // 송/수전, 양방향 (정방향/역방향)
            dsp_circ_mode_kind = circ_state_smode_bothdir;

            dsp_circ_mode_start_index = 0;
            dsp_circ_mode_end_index =
                (uint8_t)(sizeof(circ_state_smode_bothdir) /
                              sizeof(disp_mode_type) -
                          1);
        }
    }
    else
    {
        // non-단순 표시 모드
        if (lcd_is_user_dspmode())
        {
            // 사용자 모드
            if (mtkind == ONE_RATE_KIND)
            {
                if (mt_is_uni_dir())
                {
                    // 1종 수전
                    dsp_circ_mode_kind = circ_state_nsmode_onerate_unidir;

                    if (circdsp_is_bill5days() && is_mrdt_within_5days())
                    {
                        dsp_circ_mode_start_index = 5;
                        dsp_circ_mode_end_index = 7;
                    }
                    else
                    {
                        dsp_circ_mode_start_index = 0;
                        dsp_circ_mode_end_index =
                            (uint8_t)(sizeof(circ_state_nsmode_onerate_unidir) /
                                          sizeof(disp_mode_type) -
                                      1);
                    }
                }
                else
                {
                    // 1종 송/수전
                    dsp_circ_mode_kind = circ_state_nsmode_onerate_bothdir;

                    if (circdsp_is_bill5days() && is_mrdt_within_5days())
                    {
                        dsp_circ_mode_start_index = 6;
                        dsp_circ_mode_end_index = 9;
                    }
                    else
                    {
                        dsp_circ_mode_start_index = 0;
                        dsp_circ_mode_end_index =
                            (uint8_t)(sizeof(
                                          circ_state_nsmode_onerate_bothdir) /
                                          sizeof(disp_mode_type) -
                                      1);
                    }
                }
            }
            else
            {
                if (mt_is_uni_dir())
                {
                    // 2종 수전
                    dsp_circ_mode_kind = circ_state_nsmode_tworate_unidir;

                    if (circdsp_is_bill5days() && is_mrdt_within_5days())
                    {
                        dsp_circ_mode_start_index = 12;
                        dsp_circ_mode_end_index = 19;
                    }
                    else
                    {
                        dsp_circ_mode_start_index = 0;
                        dsp_circ_mode_end_index =
                            (uint8_t)(sizeof(circ_state_nsmode_tworate_unidir) /
                                          sizeof(disp_mode_type) -
                                      1);
                    }
                }
                else
                {
                    // 2종 송/수전
                    dsp_circ_mode_kind = circ_state_nsmode_tworate_bothdir;

                    if (circdsp_is_bill5days() && is_mrdt_within_5days())
                    {
                        dsp_circ_mode_start_index = 16;
                        dsp_circ_mode_end_index = 25;
                    }
                    else
                    {
                        dsp_circ_mode_start_index = 0;
                        dsp_circ_mode_end_index =
                            (uint8_t)(sizeof(
                                          circ_state_nsmode_tworate_bothdir) /
                                          sizeof(disp_mode_type) -
                                      1);
                    }
                }
            }
        }
        else
        {
            // 공급자 모드, 관리자 순환 표시 모드
            if (dsp_is_suppdsp_available())
            {
                dsp_circ_mode_kind = (const disp_mode_type*)NULL;

                dsp_circ_mode_start_index = 0;
                dsp_circ_mode_end_index = circ_state_suppdsp_mode.mode_cnt - 1;
            }
            else
            {
                dsp_circ_mode_kind = circ_state_nsmode_onerate_unidir;

                dsp_circ_mode_start_index = 0;
                dsp_circ_mode_end_index = 0;
            }
        }
    }
}

static void dsp_circ_state(uint8_t* tptr)
{
    dsp_get_circ_mode_kind();

    if (dsp_circ_mode_chg_q())
    {
        dsp_mode_chged_set();

        dsp_next_circ_mode();
        if (skip_dsp_circ_blank)
        {
            dsp_mdchg_timeset(0);
        }
        else
        {
            dsp_mdchg_timeset(T1SEC);
        }
    }

    if (dsp_is_mode_chged())
    {
        if (!circ_mode_chg_manually)
        {
            key_circ_step_mode_clear();
        }
        goto dsp_circ_state1;
    }

    if (dsp_is_circ_update())
    {
    dsp_circ_state1:
        dsp_var_init();
        dsp_all_mode(tptr);
        dsp_item_num(dsp_circ_mode_index + 1);
    }
}

static bool dsp_is_circ_update(void)
{
    if (dsp_mode_in_state == DSPMODE_DATE || dsp_mode_in_state == DSPMODE_TIME)
        return true;

    if (dsp_mode_in_state >= DSPMODE_CU_IMP_ACT_A &&
        dsp_mode_in_state <= DSPMODE_CU_EXP_APP_T)
    {
        return true;
    }

    if (dsp_mode_in_state >= DSPMODE_CU_IMP_ACT_MX_A &&
        dsp_mode_in_state <= DSPMODE_CU_EXP_APP_CUMMX_T)
    {
        return dsp_mxdm_modified;
    }

    if (dsp_mode_in_state >= DSPMODE_CU_IMP_PF_A &&
        dsp_mode_in_state <= DSPMODE_CU_EXP_PF_T)
    {
        // return false;
        return true;  // 15.5.8
    }

    if (dsp_mode_in_state >= DSPMODE_BF_IMP_ACT_A &&
        dsp_mode_in_state < DSPMODE_CURR_IMP_ACT)
    {
        return dsp_mr_modified;
    }

    if (dsp_mode_in_state >= DSPMODE_CURR_IMP_ACT &&
        dsp_mode_in_state <= DSPMODE_CURR_EXP_APP)
    {
        return true;
    }

    if (dsp_mode_in_state >= DSPMODE_LAST_IMP_ACT &&
        dsp_mode_in_state <= DSPMODE_LAST_EXP_APP)
    {
        return dsp_rcntdm_modified;
    }

    if (dsp_mode_in_state == DSPMODE_LAST_EXP_PF ||
        dsp_mode_in_state == DSPMODE_LAST_IMP_PF)
    {
        return dsp_rcntpf_modified;
    }

    return false;
}

bool dsp_is_test_update(void)
{
    if (dsp_is_mode_chged())
    {
        return true;
    }

    if (((dsp_mode_in_state >= DSPMODE_V1) &&
         (dsp_mode_in_state <= DSPMODE_tRaw)) ||
        (dsp_mode_in_state == DSPMODE_TEST_REG_DATE) ||
        (dsp_mode_in_state == DSPMODE_TARIFF_RATE))
    {
        return emb_disp_is_ready();
    }

    if (dsp_mode_in_state == DSPMODE_LATCHON)
    {
        return false;  // eeprom is accessed
    }
    return true;
}

void dsp_mr_modified_set(void) { dsp_mr_modified = true; }

void dsp_mxdm_modified_set(void) { dsp_mxdm_modified = true; }

void dsp_rcntdm_modified_set(void) { dsp_rcntdm_modified = true; }

void dsp_rcntpf_modified_set(void) { dsp_rcntpf_modified = true; }

bool dsp_is_error_mode(void) { return (dsp_mode_in_state == DSPMODE_ERROR); }

static bool dsp_mode_is_mxtim(disp_mode_type dspmd)
{
    if (dspmd >= DSPMODE_CU_IMP_ACT_MXTIM_A &&
        dspmd <= DSPMODE_CU_EXP_APP_MXTIM_T)
        return true;

    if (dspmd >= DSPMODE_BF_IMP_ACT_MXTIM_A &&
        dspmd <= DSPMODE_BF_EXP_APP_MXTIM_T)
        return true;

    if (dspmd >= DSPMODE_BBF_IMP_ACT_MXTIM_A &&
        dspmd <= DSPMODE_BBF_EXP_APP_MXTIM_T)
        return true;

    return false;
}

static void dsp_next_circ_mode(void)
{
    disp_mode_type tdspmode;

    if (disp_circ_mode_init == 0)
    {
        tdspmode = get_circ_dsp_mode(dsp_circ_mode_index);

        if (tdspmode == DSPMODE_BUY_ENERGY || tdspmode == DSPMODE_REM_ENERGY)
        {
            if (!prepay_fract)
            {
                prepay_fract = true;
            }
            else
            {
                prepay_fract = false;
                dsp_circ_mode_index++;
            }
        }
        else if (dsp_mode_is_mxtim(tdspmode))
        {
            if (!b_dsp_mxdm_time)
            {
                b_dsp_mxdm_time = true;
            }
            else
            {
                b_dsp_mxdm_time = false;
                dsp_circ_mode_index++;
            }
        }
        else
        {
            dsp_circ_mode_index++;
            b_dsp_mxdm_time = false;
        }

        if (dsp_circ_mode_index > dsp_circ_mode_end_index)
        {
            b_dsp_bm_finished = true;
            dsp_circ_mode_index = dsp_circ_mode_start_index;
        }
    }
    else
    {
        dsp_circ_mode_index = dsp_circ_mode_start_index;

        disp_circ_mode_init = 0;
    }

    dsp_mode_in_state = get_circ_dsp_mode(dsp_circ_mode_index);
}

disp_mode_type gen_pre_dsp_mode_in_state;
static void dsp_all_mode(uint8_t* tptr)
{
    static date_time_type mxdt;
    static uint16_t prepay_fract_val;
    uint32_t val;
    rate_type rt;
    float fval;

    if (dsp_mode_in_state != gen_pre_dsp_mode_in_state)
    {
        uint8_t mode_index = 0;
        if (dsp_is_circ_state())
        {
            mode_index = dsp_circ_mode_index + 1;
        }
        else if (dsp_is_test_state())
        {
            mode_index = dsp_test_mode_index + 1;
        }

        DPRINTF(
            DBG_NONE, _D "%s: dsp_mode_in_state 0x%02X -> 0x%02X No.%02d\r\n",
            __func__, gen_pre_dsp_mode_in_state, dsp_mode_in_state, mode_index);

#if 1  // jp 230907
        if ((g_en_run_media_HDLC == MEDIA_RUN_NONE) &&
            ((appl_get_conn_state() != APPL_IDLE_STATE) ||
             (g_en_run_media_HDLC != MEDIA_RUN_NONE)))
        {
            DPRINTF(DBG_ERR, "media_if HDLC fsm[%s -> %s]\r\n",
                    dsm_media_if_fsm_string(g_en_run_media_HDLC),
                    dsm_media_if_fsm_string(g_en_run_media_HDLC));
            dsm_media_if_init();
            amr_init();
        }
#endif
    }

    switch (dsp_mode_in_state)
    {
    case DSPMODE_DATE: /* 현재 날짜 */
    {
        dsp_date(&cur_rtc);
        dsp_unit = eUnitNull;
        break;
    }
    case DSPMODE_TIME: /* 현재 시간 */
    {
        dsp_time(&cur_rtc);
        dsp_unit = eUnitClock1;
        break;
    }

    case DSPMODE_CU_IMP_ACT_A: /* 현월 누적 수전 유효전력량 (kWh)_경부하 */
    case DSPMODE_CU_IMP_ACT_B: /* 현월 누적 수전 유효전력량 (kWh)_중간부하
                                */
    case DSPMODE_CU_IMP_ACT_C: /* 현월 누적 수전 유효전력량 (kWh)_최대부하
                                */
    case DSPMODE_CU_IMP_ACT_D:
    case DSPMODE_CU_IMP_ACT_T: /* 현월(현재) 누적 수전 유효전력량 (kWh)_전체
                                */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_ACT_A);
        val = get_ch_val(0, rt, eDeliAct, eActEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKwh;
        break;

    case DSPMODE_CU_EXP_ACT_A: /* 현월 누적 송전 유효전력량 (kWh)_경부하 */
    case DSPMODE_CU_EXP_ACT_B: /* 현월 누적 송전 유효전력량 (kWh)_중간부하
                                */
    case DSPMODE_CU_EXP_ACT_C: /* 현월 누적 송전 유효전력량 (kWh)_최대부하
                                */
    case DSPMODE_CU_EXP_ACT_D:
    case DSPMODE_CU_EXP_ACT_T: /* 현월(현재) 누적 송전 유효전력량 (kWh)_전체
                                */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_ACT_A);
        val = get_ch_val(0, rt, eReceiAct, eActEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKwh;
        break;

    case DSPMODE_CU_IMP_REACT_A:
    case DSPMODE_CU_IMP_REACT_B:
    case DSPMODE_CU_IMP_REACT_C:
    case DSPMODE_CU_IMP_REACT_D:
    case DSPMODE_CU_IMP_REACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_REACT_A);
        val = get_ch_val(0, rt, eDeliAct, eReactEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_CU_IMP_lagREACT_A:
    case DSPMODE_CU_IMP_lagREACT_B:
    case DSPMODE_CU_IMP_lagREACT_C:
    case DSPMODE_CU_IMP_lagREACT_D:
    case DSPMODE_CU_IMP_lagREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_lagREACT_A);
        val = get_ch_val(0, rt, eDeliAct, eLagReactEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_CU_IMP_leadREACT_A:
    case DSPMODE_CU_IMP_leadREACT_B:
    case DSPMODE_CU_IMP_leadREACT_C:
    case DSPMODE_CU_IMP_leadREACT_D:
    case DSPMODE_CU_IMP_leadREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_leadREACT_A);
        val = get_ch_val(0, rt, eDeliAct, eLeadReactEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_CU_EXP_REACT_A:
    case DSPMODE_CU_EXP_REACT_B:
    case DSPMODE_CU_EXP_REACT_C:
    case DSPMODE_CU_EXP_REACT_D:
    case DSPMODE_CU_EXP_REACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_REACT_A);
        val = get_ch_val(0, rt, eReceiAct, eReactEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_CU_EXP_lagREACT_A:
    case DSPMODE_CU_EXP_lagREACT_B:
    case DSPMODE_CU_EXP_lagREACT_C:
    case DSPMODE_CU_EXP_lagREACT_D:
    case DSPMODE_CU_EXP_lagREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_lagREACT_A);
        val = get_ch_val(0, rt, eReceiAct, eLagReactEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_CU_EXP_leadREACT_A:
    case DSPMODE_CU_EXP_leadREACT_B:
    case DSPMODE_CU_EXP_leadREACT_C:
    case DSPMODE_CU_EXP_leadREACT_D:
    case DSPMODE_CU_EXP_leadREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_leadREACT_A);
        val = get_ch_val(0, rt, eReceiAct, eLeadReactEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_CU_IMP_APP_A:
    case DSPMODE_CU_IMP_APP_B:
    case DSPMODE_CU_IMP_APP_C:
    case DSPMODE_CU_IMP_APP_D:
    case DSPMODE_CU_IMP_APP_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_APP_A);
        val = get_ch_val(0, rt, eDeliAct, eAppEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKvah;
        break;

    case DSPMODE_CU_EXP_APP_A:
    case DSPMODE_CU_EXP_APP_B:
    case DSPMODE_CU_EXP_APP_C:
    case DSPMODE_CU_EXP_APP_D:
    case DSPMODE_CU_EXP_APP_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_APP_A);
        val = get_ch_val(0, rt, eReceiAct, eAppEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKvah;
        break;

    case DSPMODE_CU_IMP_ACT_MX_A:
    case DSPMODE_CU_IMP_ACT_MX_B:
    case DSPMODE_CU_IMP_ACT_MX_C:
    case DSPMODE_CU_IMP_ACT_MX_D:
    case DSPMODE_CU_IMP_ACT_MX_T: /* 현월 수전 유효 최대수요전력(kW) */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_ACT_MX_A);
        val = get_cur_mxdm(rt, eDmChDeliAct);
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_CU_EXP_ACT_MX_A:
    case DSPMODE_CU_EXP_ACT_MX_B:
    case DSPMODE_CU_EXP_ACT_MX_C:
    case DSPMODE_CU_EXP_ACT_MX_D:
    case DSPMODE_CU_EXP_ACT_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_ACT_MX_A);
        val = get_cur_mxdm(rt, eDmChReceiAct);
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_CU_IMP_APP_MX_A:
    case DSPMODE_CU_IMP_APP_MX_B:
    case DSPMODE_CU_IMP_APP_MX_C:
    case DSPMODE_CU_IMP_APP_MX_D:
    case DSPMODE_CU_IMP_APP_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_APP_MX_A);
        val = get_cur_mxdm(rt, eDmChDeliApp);
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_CU_EXP_APP_MX_A:
    case DSPMODE_CU_EXP_APP_MX_B:
    case DSPMODE_CU_EXP_APP_MX_C:
    case DSPMODE_CU_EXP_APP_MX_D:
    case DSPMODE_CU_EXP_APP_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_APP_MX_A);
        val = get_cur_mxdm(rt, eDmChReceiApp);
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_CU_IMP_ACT_MXTIM_A:
    case DSPMODE_CU_IMP_ACT_MXTIM_B:
    case DSPMODE_CU_IMP_ACT_MXTIM_C:
    case DSPMODE_CU_IMP_ACT_MXTIM_D:
    case DSPMODE_CU_IMP_ACT_MXTIM_T: /* 현월 수전 유효 최대수요전력 발생
                                        날짜/시간 */
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_ACT_MXTIM_A);
            get_cur_mxtime(&mxdt, rt, eDmChDeliAct);
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_CU_EXP_ACT_MXTIM_A:
    case DSPMODE_CU_EXP_ACT_MXTIM_B:
    case DSPMODE_CU_EXP_ACT_MXTIM_C:
    case DSPMODE_CU_EXP_ACT_MXTIM_D:
    case DSPMODE_CU_EXP_ACT_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_ACT_MXTIM_A);
            get_cur_mxtime(&mxdt, rt, eDmChReceiAct);
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_CU_IMP_APP_MXTIM_A:
    case DSPMODE_CU_IMP_APP_MXTIM_B:
    case DSPMODE_CU_IMP_APP_MXTIM_C:
    case DSPMODE_CU_IMP_APP_MXTIM_D:
    case DSPMODE_CU_IMP_APP_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_APP_MXTIM_A);
            get_cur_mxtime(&mxdt, rt, eDmChDeliApp);
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_CU_EXP_APP_MXTIM_A:
    case DSPMODE_CU_EXP_APP_MXTIM_B:
    case DSPMODE_CU_EXP_APP_MXTIM_C:
    case DSPMODE_CU_EXP_APP_MXTIM_D:
    case DSPMODE_CU_EXP_APP_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_APP_MXTIM_A);
            get_cur_mxtime(&mxdt, rt, eDmChReceiApp);
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_CU_IMP_ACT_CUMMX_A:
    case DSPMODE_CU_IMP_ACT_CUMMX_B:
    case DSPMODE_CU_IMP_ACT_CUMMX_C:
    case DSPMODE_CU_IMP_ACT_CUMMX_D:
    case DSPMODE_CU_IMP_ACT_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_ACT_CUMMX_A);
        val = get_cur_cumdm(rt, eDmChDeliAct);
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_CU_EXP_ACT_CUMMX_A:
    case DSPMODE_CU_EXP_ACT_CUMMX_B:
    case DSPMODE_CU_EXP_ACT_CUMMX_C:
    case DSPMODE_CU_EXP_ACT_CUMMX_D:
    case DSPMODE_CU_EXP_ACT_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_ACT_CUMMX_A);
        val = get_cur_cumdm(rt, eDmChReceiAct);
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_CU_IMP_APP_CUMMX_A:
    case DSPMODE_CU_IMP_APP_CUMMX_B:
    case DSPMODE_CU_IMP_APP_CUMMX_C:
    case DSPMODE_CU_IMP_APP_CUMMX_D:
    case DSPMODE_CU_IMP_APP_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_APP_CUMMX_A);
        val = get_cur_cumdm(rt, eDmChDeliApp);
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_CU_EXP_APP_CUMMX_A:
    case DSPMODE_CU_EXP_APP_CUMMX_B:
    case DSPMODE_CU_EXP_APP_CUMMX_C:
    case DSPMODE_CU_EXP_APP_CUMMX_D:
    case DSPMODE_CU_EXP_APP_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_APP_CUMMX_A);
        val = get_cur_cumdm(rt, eDmChReceiApp);
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_CU_IMP_PF_A:
    case DSPMODE_CU_IMP_PF_B:
    case DSPMODE_CU_IMP_PF_C:
    case DSPMODE_CU_IMP_PF_D:
    case DSPMODE_CU_IMP_PF_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_IMP_PF_A);
        dsp_pf(get_cur_pf(rt, eDeliAct));
        dsp_unit = eUnitPercent;
        break;

    case DSPMODE_CU_EXP_PF_A:
    case DSPMODE_CU_EXP_PF_B:
    case DSPMODE_CU_EXP_PF_C:
    case DSPMODE_CU_EXP_PF_D:
    case DSPMODE_CU_EXP_PF_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_CU_EXP_PF_A);
        dsp_pf(get_cur_pf(rt, eReceiAct));
        dsp_unit = eUnitPercent;
        break;

    case DSPMODE_BF_IMP_ACT_A: /* 전월 누적 수전 유효전력량 (kWh)_경부하 */
    case DSPMODE_BF_IMP_ACT_B: /* 전월 누적 수전 유효전력량 (kWh)_중간부하
                                */
    case DSPMODE_BF_IMP_ACT_C: /* 전월 누적 수전 유효전력량 (kWh)_최대부하
                                */
    case DSPMODE_BF_IMP_ACT_D:
    case DSPMODE_BF_IMP_ACT_T: /* 전월 누적 수전 유효전력량(kWh)_전체 */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_ACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eDeliAct, eActEn, tptr);
#else
        val = get_ch_val(1, rt, eDeliAct, eActEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKwh;
        break;

#if 0  // jp.kim 25.06.22     
    case DSPMODE_BF_IMP_ACT_A_NPRD: /* 비정기 누적 수전 유효전력량 (kWh)_경부하
                                     */
    case DSPMODE_BF_IMP_ACT_B_NPRD: /* 비정기 누적 수전 유효전력량
                                       (kWh)_중간부하 */
    case DSPMODE_BF_IMP_ACT_C_NPRD: /* 비정기 누적 수전 유효전력량
                                       (kWh)_최대부하 */
    case DSPMODE_BF_IMP_ACT_D_NPRD:
    case DSPMODE_BF_IMP_ACT_T_NPRD: /* 비정기 누적 수전 유효전력량(kWh)_전체 */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_ACT_A_NPRD);
        val = get_ch_val_nprd(1, rt, eDeliAct, eActEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKwh;
        break;
#endif
    case DSPMODE_BBF_IMP_ACT_A:
    case DSPMODE_BBF_IMP_ACT_B:
    case DSPMODE_BBF_IMP_ACT_C:
    case DSPMODE_BBF_IMP_ACT_D:
    case DSPMODE_BBF_IMP_ACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_ACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eDeliAct, eActEn, tptr);
#else
        val = get_ch_val(2, rt, eDeliAct, eActEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKwh;
        break;

    case DSPMODE_BF_EXP_ACT_A: /* 전월 누적 송전 유효전력량(kWh)_경부하 */
    case DSPMODE_BF_EXP_ACT_B: /* 전월 누적 송전 유효전력량(kWh)_중간부하 */
    case DSPMODE_BF_EXP_ACT_C: /* 전월 누적 송전 유효전력량(kWh)_최대부하 */
    case DSPMODE_BF_EXP_ACT_D:
    case DSPMODE_BF_EXP_ACT_T: /* 전월 누적 송전 유효전력량(kWh)_전체 */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_ACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eReceiAct, eActEn, tptr);
#else
        val = get_ch_val(1, rt, eReceiAct, eActEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKwh;
        break;

#if 0  // jp.kim 25.06.22
    case DSPMODE_BF_EXP_ACT_A_NPRD: /* 비정기 누적 송전 유효전력량(kWh)_경부하
                                     */
    case DSPMODE_BF_EXP_ACT_B_NPRD: /* 비정기 누적 송전 유효전력량(kWh)_중간부하
                                     */
    case DSPMODE_BF_EXP_ACT_C_NPRD: /* 비정기 누적 송전 유효전력량(kWh)_최대부하
                                     */
    case DSPMODE_BF_EXP_ACT_D_NPRD:
    case DSPMODE_BF_EXP_ACT_T_NPRD: /* 비정기 누적 송전 유효전력량(kWh)_전체 */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_ACT_A_NPRD);
        val = get_ch_val_nprd(1, rt, eReceiAct, eActEn, tptr);
        dsp_energy(1, val);
        dsp_unit = eUnitKwh;
        break;
#endif

    case DSPMODE_BBF_EXP_ACT_A:
    case DSPMODE_BBF_EXP_ACT_B:
    case DSPMODE_BBF_EXP_ACT_C:
    case DSPMODE_BBF_EXP_ACT_D:
    case DSPMODE_BBF_EXP_ACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_ACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eReceiAct, eActEn, tptr);
#else
        val = get_ch_val(2, rt, eReceiAct, eActEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKwh;
        break;

    case DSPMODE_BF_IMP_REACT_A:
    case DSPMODE_BF_IMP_REACT_B:
    case DSPMODE_BF_IMP_REACT_C:
    case DSPMODE_BF_IMP_REACT_D:
    case DSPMODE_BF_IMP_REACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_REACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eDeliAct, eReactEn, tptr);
#else
        val = get_ch_val(1, rt, eDeliAct, eReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BF_IMP_lagREACT_A:
    case DSPMODE_BF_IMP_lagREACT_B:
    case DSPMODE_BF_IMP_lagREACT_C:
    case DSPMODE_BF_IMP_lagREACT_D:
    case DSPMODE_BF_IMP_lagREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_lagREACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eDeliAct, eLagReactEn, tptr);
#else
        val = get_ch_val(1, rt, eDeliAct, eLagReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BF_IMP_leadREACT_A:
    case DSPMODE_BF_IMP_leadREACT_B:
    case DSPMODE_BF_IMP_leadREACT_C:
    case DSPMODE_BF_IMP_leadREACT_D:
    case DSPMODE_BF_IMP_leadREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_leadREACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eDeliAct, eLeadReactEn, tptr);
#else
        val = get_ch_val(1, rt, eDeliAct, eLeadReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BBF_IMP_REACT_A:
    case DSPMODE_BBF_IMP_REACT_B:
    case DSPMODE_BBF_IMP_REACT_C:
    case DSPMODE_BBF_IMP_REACT_D:
    case DSPMODE_BBF_IMP_REACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_REACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eDeliAct, eReactEn, tptr);
#else
        val = get_ch_val(2, rt, eDeliAct, eReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BBF_IMP_lagREACT_A:
    case DSPMODE_BBF_IMP_lagREACT_B:
    case DSPMODE_BBF_IMP_lagREACT_C:
    case DSPMODE_BBF_IMP_lagREACT_D:
    case DSPMODE_BBF_IMP_lagREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_lagREACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eDeliAct, eLagReactEn, tptr);
#else
        val = get_ch_val(2, rt, eDeliAct, eLagReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BBF_IMP_leadREACT_A:
    case DSPMODE_BBF_IMP_leadREACT_B:
    case DSPMODE_BBF_IMP_leadREACT_C:
    case DSPMODE_BBF_IMP_leadREACT_D:
    case DSPMODE_BBF_IMP_leadREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_leadREACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eDeliAct, eLeadReactEn, tptr);
#else
        val = get_ch_val(2, rt, eDeliAct, eLeadReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BF_EXP_REACT_A:
    case DSPMODE_BF_EXP_REACT_B:
    case DSPMODE_BF_EXP_REACT_C:
    case DSPMODE_BF_EXP_REACT_D:
    case DSPMODE_BF_EXP_REACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_REACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eReceiAct, eReactEn, tptr);
#else
        val = get_ch_val(1, rt, eReceiAct, eReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BF_EXP_lagREACT_A:
    case DSPMODE_BF_EXP_lagREACT_B:
    case DSPMODE_BF_EXP_lagREACT_C:
    case DSPMODE_BF_EXP_lagREACT_D:
    case DSPMODE_BF_EXP_lagREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_lagREACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eReceiAct, eLagReactEn, tptr);
#else
        val = get_ch_val(1, rt, eReceiAct, eLagReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BF_EXP_leadREACT_A:
    case DSPMODE_BF_EXP_leadREACT_B:
    case DSPMODE_BF_EXP_leadREACT_C:
    case DSPMODE_BF_EXP_leadREACT_D:
    case DSPMODE_BF_EXP_leadREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_leadREACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eReceiAct, eLeadReactEn, tptr);
#else
        val = get_ch_val(1, rt, eReceiAct, eLeadReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BBF_EXP_REACT_A:
    case DSPMODE_BBF_EXP_REACT_B:
    case DSPMODE_BBF_EXP_REACT_C:
    case DSPMODE_BBF_EXP_REACT_D:
    case DSPMODE_BBF_EXP_REACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_REACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eReceiAct, eReactEn, tptr);
#else
        val = get_ch_val(2, rt, eReceiAct, eReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BBF_EXP_lagREACT_A:
    case DSPMODE_BBF_EXP_lagREACT_B:
    case DSPMODE_BBF_EXP_lagREACT_C:
    case DSPMODE_BBF_EXP_lagREACT_D:
    case DSPMODE_BBF_EXP_lagREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_lagREACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eReceiAct, eLagReactEn, tptr);
#else
        val = get_ch_val(2, rt, eReceiAct, eLagReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BBF_EXP_leadREACT_A:
    case DSPMODE_BBF_EXP_leadREACT_B:
    case DSPMODE_BBF_EXP_leadREACT_C:
    case DSPMODE_BBF_EXP_leadREACT_D:
    case DSPMODE_BBF_EXP_leadREACT_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_leadREACT_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eReceiAct, eLeadReactEn, tptr);
#else
        val = get_ch_val(2, rt, eReceiAct, eLeadReactEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvarh;
        break;

    case DSPMODE_BF_IMP_APP_A:
    case DSPMODE_BF_IMP_APP_B:
    case DSPMODE_BF_IMP_APP_C:
    case DSPMODE_BF_IMP_APP_D:
    case DSPMODE_BF_IMP_APP_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_APP_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eDeliAct, eAppEn, tptr);
#else
        val = get_ch_val(1, rt, eDeliAct, eAppEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvah;
        break;

    case DSPMODE_BBF_IMP_APP_A:
    case DSPMODE_BBF_IMP_APP_B:
    case DSPMODE_BBF_IMP_APP_C:
    case DSPMODE_BBF_IMP_APP_D:
    case DSPMODE_BBF_IMP_APP_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_APP_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eDeliAct, eAppEn, tptr);
#else
        val = get_ch_val(2, rt, eDeliAct, eAppEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvah;
        break;

    case DSPMODE_BF_EXP_APP_A:
    case DSPMODE_BF_EXP_APP_B:
    case DSPMODE_BF_EXP_APP_C:
    case DSPMODE_BF_EXP_APP_D:
    case DSPMODE_BF_EXP_APP_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_APP_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(1, rt, eReceiAct, eAppEn, tptr);
#else
        val = get_ch_val(1, rt, eReceiAct, eAppEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvah;
        break;

    case DSPMODE_BBF_EXP_APP_A:
    case DSPMODE_BBF_EXP_APP_B:
    case DSPMODE_BBF_EXP_APP_C:
    case DSPMODE_BBF_EXP_APP_D:
    case DSPMODE_BBF_EXP_APP_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_APP_A);
#if 1  // jp.kim 25.06.22
        val = get_ch_val_PrdNprdSeason(2, rt, eReceiAct, eAppEn, tptr);
#else
        val = get_ch_val(2, rt, eReceiAct, eAppEn, tptr);
#endif
        dsp_energy(1, val);
        dsp_unit = eUnitKvah;
        break;

    case DSPMODE_BF_IMP_ACT_MX_A:
    case DSPMODE_BF_IMP_ACT_MX_B:
    case DSPMODE_BF_IMP_ACT_MX_C:
    case DSPMODE_BF_IMP_ACT_MX_D:
    case DSPMODE_BF_IMP_ACT_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_ACT_MX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_mxdm_PrdNprdSeason(1, rt, eDmChDeliAct, tptr);
#else
        val = get_bf_mxdm_PrdNprdSeason(1, rt, eDmChDeliAct, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_BF_EXP_ACT_MX_A:
    case DSPMODE_BF_EXP_ACT_MX_B:
    case DSPMODE_BF_EXP_ACT_MX_C:
    case DSPMODE_BF_EXP_ACT_MX_D:
    case DSPMODE_BF_EXP_ACT_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_ACT_MX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_mxdm_PrdNprdSeason(1, rt, eDmChReceiAct, tptr);
#else
        val = get_bf_mxdm(1, rt, eDmChReceiAct, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_BF_IMP_ACT_MXTIM_A:
    case DSPMODE_BF_IMP_ACT_MXTIM_B:
    case DSPMODE_BF_IMP_ACT_MXTIM_C:
    case DSPMODE_BF_IMP_ACT_MXTIM_D:
    case DSPMODE_BF_IMP_ACT_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_ACT_MXTIM_A);
#if 1  // jp.kim 25.06.22
            get_bf_mxtime_PrdNprdSeason(&mxdt, 1, rt, eDmChDeliAct, tptr);
#else
            get_bf_mxtime(&mxdt, 1, rt, eDmChDeliAct, tptr);
#endif
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_BF_EXP_ACT_MXTIM_A:
    case DSPMODE_BF_EXP_ACT_MXTIM_B:
    case DSPMODE_BF_EXP_ACT_MXTIM_C:
    case DSPMODE_BF_EXP_ACT_MXTIM_D:
    case DSPMODE_BF_EXP_ACT_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_ACT_MXTIM_A);
#if 1  // jp.kim 25.06.22
            get_bf_mxtime_PrdNprdSeason(&mxdt, 1, rt, eDmChReceiAct, tptr);
#else
            get_bf_mxtime(&mxdt, 1, rt, eDmChReceiAct, tptr);
#endif
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_BBF_IMP_ACT_MX_A:
    case DSPMODE_BBF_IMP_ACT_MX_B:
    case DSPMODE_BBF_IMP_ACT_MX_C:
    case DSPMODE_BBF_IMP_ACT_MX_D:
    case DSPMODE_BBF_IMP_ACT_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_ACT_MX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_mxdm_PrdNprdSeason(2, rt, eDmChDeliAct, tptr);
#else
        val = get_bf_mxdm(2, rt, eDmChDeliAct, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_BBF_EXP_ACT_MX_A:
    case DSPMODE_BBF_EXP_ACT_MX_B:
    case DSPMODE_BBF_EXP_ACT_MX_C:
    case DSPMODE_BBF_EXP_ACT_MX_D:
    case DSPMODE_BBF_EXP_ACT_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_ACT_MX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_mxdm_PrdNprdSeason(2, rt, eDmChReceiAct, tptr);
#else
        val = get_bf_mxdm(2, rt, eDmChReceiAct, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_BBF_IMP_ACT_MXTIM_A:
    case DSPMODE_BBF_IMP_ACT_MXTIM_B:
    case DSPMODE_BBF_IMP_ACT_MXTIM_C:
    case DSPMODE_BBF_IMP_ACT_MXTIM_D:
    case DSPMODE_BBF_IMP_ACT_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_ACT_MXTIM_A);
#if 1  // jp.kim 25.06.22
            get_bf_mxtime_PrdNprdSeason(&mxdt, 2, rt, eDmChDeliAct, tptr);
#else
            get_bf_mxtime(&mxdt, 2, rt, eDmChDeliAct, tptr);
#endif
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_BBF_EXP_ACT_MXTIM_A:
    case DSPMODE_BBF_EXP_ACT_MXTIM_B:
    case DSPMODE_BBF_EXP_ACT_MXTIM_C:
    case DSPMODE_BBF_EXP_ACT_MXTIM_D:
    case DSPMODE_BBF_EXP_ACT_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_ACT_MXTIM_A);
#if 1  // jp.kim 25.06.22
            get_bf_mxtime_PrdNprdSeason(&mxdt, 2, rt, eDmChReceiAct, tptr);
#else
            get_bf_mxtime(&mxdt, 2, rt, eDmChReceiAct, tptr);
#endif
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_BF_IMP_APP_MX_A:
    case DSPMODE_BF_IMP_APP_MX_B:
    case DSPMODE_BF_IMP_APP_MX_C:
    case DSPMODE_BF_IMP_APP_MX_D:
    case DSPMODE_BF_IMP_APP_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_APP_MX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_mxdm_PrdNprdSeason(1, rt, eDmChDeliApp, tptr);
#else
        val = get_bf_mxdm(1, rt, eDmChDeliApp, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_BF_EXP_APP_MX_A:
    case DSPMODE_BF_EXP_APP_MX_B:
    case DSPMODE_BF_EXP_APP_MX_C:
    case DSPMODE_BF_EXP_APP_MX_D:
    case DSPMODE_BF_EXP_APP_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_APP_MX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_mxdm_PrdNprdSeason(1, rt, eDmChReceiApp, tptr);
#else
        val = get_bf_mxdm(1, rt, eDmChReceiApp, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_BF_IMP_APP_MXTIM_A:
    case DSPMODE_BF_IMP_APP_MXTIM_B:
    case DSPMODE_BF_IMP_APP_MXTIM_C:
    case DSPMODE_BF_IMP_APP_MXTIM_D:
    case DSPMODE_BF_IMP_APP_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_APP_MXTIM_A);
#if 1  // jp.kim 25.06.22
            get_bf_mxtime_PrdNprdSeason(&mxdt, 1, rt, eDmChDeliApp, tptr);
#else
            get_bf_mxtime(&mxdt, 1, rt, eDmChDeliApp, tptr);
#endif
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_BF_EXP_APP_MXTIM_A:
    case DSPMODE_BF_EXP_APP_MXTIM_B:
    case DSPMODE_BF_EXP_APP_MXTIM_C:
    case DSPMODE_BF_EXP_APP_MXTIM_D:
    case DSPMODE_BF_EXP_APP_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_APP_MXTIM_A);
#if 1  // jp.kim 25.06.22
            get_bf_mxtime_PrdNprdSeason(&mxdt, 1, rt, eDmChReceiApp, tptr);
#else
            get_bf_mxtime(&mxdt, 1, rt, eDmChReceiApp, tptr);
#endif
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_BBF_IMP_APP_MX_A:
    case DSPMODE_BBF_IMP_APP_MX_B:
    case DSPMODE_BBF_IMP_APP_MX_C:
    case DSPMODE_BBF_IMP_APP_MX_D:
    case DSPMODE_BBF_IMP_APP_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_APP_MX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_mxdm_PrdNprdSeason(2, rt, eDmChDeliApp, tptr);
#else
        val = get_bf_mxdm(2, rt, eDmChDeliApp, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_BBF_EXP_APP_MX_A:
    case DSPMODE_BBF_EXP_APP_MX_B:
    case DSPMODE_BBF_EXP_APP_MX_C:
    case DSPMODE_BBF_EXP_APP_MX_D:
    case DSPMODE_BBF_EXP_APP_MX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_APP_MX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_mxdm_PrdNprdSeason(2, rt, eDmChReceiApp, tptr);
#else
        val = get_bf_mxdm(2, rt, eDmChReceiApp, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_BBF_IMP_APP_MXTIM_A:
    case DSPMODE_BBF_IMP_APP_MXTIM_B:
    case DSPMODE_BBF_IMP_APP_MXTIM_C:
    case DSPMODE_BBF_IMP_APP_MXTIM_D:
    case DSPMODE_BBF_IMP_APP_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_APP_MXTIM_A);
#if 1  // jp.kim 25.06.22
            get_bf_mxtime_PrdNprdSeason(&mxdt, 2, rt, eDmChDeliApp, tptr);
#else
            get_bf_mxtime(&mxdt, 2, rt, eDmChDeliApp, tptr);
#endif
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_BBF_EXP_APP_MXTIM_A:
    case DSPMODE_BBF_EXP_APP_MXTIM_B:
    case DSPMODE_BBF_EXP_APP_MXTIM_C:
    case DSPMODE_BBF_EXP_APP_MXTIM_D:
    case DSPMODE_BBF_EXP_APP_MXTIM_T:
        if (!b_dsp_mxdm_time)
        {
            rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_APP_MXTIM_A);
#if 1  // jp.kim 25.06.22
            get_bf_mxtime_PrdNprdSeason(&mxdt, 2, rt, eDmChReceiApp, tptr);
#else
            get_bf_mxtime(&mxdt, 2, rt, eDmChReceiApp, tptr);
#endif
            dsp_date(&mxdt);
            dsp_unit = eUnitNull;
        }
        else
        {
            dsp_time(&mxdt);
            dsp_unit = eUnitClock1;
        }
        break;

    case DSPMODE_BF_IMP_ACT_CUMMX_A:
    case DSPMODE_BF_IMP_ACT_CUMMX_B:
    case DSPMODE_BF_IMP_ACT_CUMMX_C:
    case DSPMODE_BF_IMP_ACT_CUMMX_D:
    case DSPMODE_BF_IMP_ACT_CUMMX_T: /* 전월 누적 수전 유효 최대수요전력
                                        (kW) */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_ACT_CUMMX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_cumdm_PrdNprdSeason(1, rt, eDmChDeliAct, tptr);
#else
        val = get_bf_cumdm(1, rt, eDmChDeliAct, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

#if 0  // jp.kim 25.06.22
    case DSPMODE_BF_IMP_ACT_CUMMX_A_NPRD:
    case DSPMODE_BF_IMP_ACT_CUMMX_B_NPRD:
    case DSPMODE_BF_IMP_ACT_CUMMX_C_NPRD:
    case DSPMODE_BF_IMP_ACT_CUMMX_D_NPRD:
    case DSPMODE_BF_IMP_ACT_CUMMX_T_NPRD: /* 비정기 누적 수전 최대수요전력(kW)
                                           */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_ACT_CUMMX_A_NPRD);
        val = get_bf_cumdm_nprd(1, rt, eDmChDeliAct, tptr);
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;
#endif

    case DSPMODE_BBF_IMP_ACT_CUMMX_A:
    case DSPMODE_BBF_IMP_ACT_CUMMX_B:
    case DSPMODE_BBF_IMP_ACT_CUMMX_C:
    case DSPMODE_BBF_IMP_ACT_CUMMX_D:
    case DSPMODE_BBF_IMP_ACT_CUMMX_T: /* 전전월 누적 수전 유효 최대수요전력
                                       * (kW)
                                       */
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_ACT_CUMMX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_cumdm_PrdNprdSeason(2, rt, eDmChDeliAct, tptr);
#else
        val = get_bf_cumdm(2, rt, eDmChDeliAct, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_BF_IMP_APP_CUMMX_A:
    case DSPMODE_BF_IMP_APP_CUMMX_B:
    case DSPMODE_BF_IMP_APP_CUMMX_C:
    case DSPMODE_BF_IMP_APP_CUMMX_D:
    case DSPMODE_BF_IMP_APP_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_APP_CUMMX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_cumdm_PrdNprdSeason(1, rt, eDmChDeliApp, tptr);
#else
        val = get_bf_cumdm(1, rt, eDmChDeliApp, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;
    case DSPMODE_BBF_IMP_APP_CUMMX_A:
    case DSPMODE_BBF_IMP_APP_CUMMX_B:
    case DSPMODE_BBF_IMP_APP_CUMMX_C:
    case DSPMODE_BBF_IMP_APP_CUMMX_D:
    case DSPMODE_BBF_IMP_APP_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_APP_CUMMX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_cumdm_PrdNprdSeason(2, rt, eDmChDeliApp, tptr);
#else
        val = get_bf_cumdm(2, rt, eDmChDeliApp, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_BF_EXP_ACT_CUMMX_A:
    case DSPMODE_BF_EXP_ACT_CUMMX_B:
    case DSPMODE_BF_EXP_ACT_CUMMX_C:
    case DSPMODE_BF_EXP_ACT_CUMMX_D:
    case DSPMODE_BF_EXP_ACT_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_ACT_CUMMX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_cumdm_PrdNprdSeason(1, rt, eDmChReceiAct, tptr);
#else
        val = get_bf_cumdm(1, rt, eDmChReceiAct, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_BBF_EXP_ACT_CUMMX_A:
    case DSPMODE_BBF_EXP_ACT_CUMMX_B:
    case DSPMODE_BBF_EXP_ACT_CUMMX_C:
    case DSPMODE_BBF_EXP_ACT_CUMMX_D:
    case DSPMODE_BBF_EXP_ACT_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_ACT_CUMMX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_cumdm_PrdNprdSeason(2, rt, eDmChReceiAct, tptr);
#else
        val = get_bf_cumdm(2, rt, eDmChReceiAct, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_BF_EXP_APP_CUMMX_A:
    case DSPMODE_BF_EXP_APP_CUMMX_B:
    case DSPMODE_BF_EXP_APP_CUMMX_C:
    case DSPMODE_BF_EXP_APP_CUMMX_D:
    case DSPMODE_BF_EXP_APP_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_APP_CUMMX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_cumdm_PrdNprdSeason(1, rt, eDmChReceiApp, tptr);
#else
        val = get_bf_cumdm(1, rt, eDmChReceiApp, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_BBF_EXP_APP_CUMMX_A:
    case DSPMODE_BBF_EXP_APP_CUMMX_B:
    case DSPMODE_BBF_EXP_APP_CUMMX_C:
    case DSPMODE_BBF_EXP_APP_CUMMX_D:
    case DSPMODE_BBF_EXP_APP_CUMMX_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_APP_CUMMX_A);
#if 1  // jp.kim 25.06.22
        val = get_bf_cumdm_PrdNprdSeason(2, rt, eDmChReceiApp, tptr);
#else
        val = get_bf_cumdm(2, rt, eDmChReceiApp, tptr);
#endif
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_BF_IMP_PF_A:
    case DSPMODE_BF_IMP_PF_B:
    case DSPMODE_BF_IMP_PF_C:
    case DSPMODE_BF_IMP_PF_D:
    case DSPMODE_BF_IMP_PF_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_IMP_PF_A);
#if 1  // jp.kim 25.06.22
        dsp_pf(get_bf_pf_PrdNprdSeason(1, rt, eDeliAct, tptr));
#else
        dsp_pf(get_bf_pf(1, rt, eDeliAct, tptr));
#endif
        dsp_unit = eUnitPercent;
        break;

    case DSPMODE_BBF_IMP_PF_A:
    case DSPMODE_BBF_IMP_PF_B:
    case DSPMODE_BBF_IMP_PF_C:
    case DSPMODE_BBF_IMP_PF_D:
    case DSPMODE_BBF_IMP_PF_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_IMP_PF_A);
#if 1  // jp.kim 25.06.22
        dsp_pf(get_bf_pf_PrdNprdSeason(2, rt, eDeliAct, tptr));
#else
        dsp_pf(get_bf_pf(2, rt, eDeliAct, tptr));
#endif
        dsp_unit = eUnitPercent;
        break;

    case DSPMODE_BF_EXP_PF_A:
    case DSPMODE_BF_EXP_PF_B:
    case DSPMODE_BF_EXP_PF_C:
    case DSPMODE_BF_EXP_PF_D:
    case DSPMODE_BF_EXP_PF_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BF_EXP_PF_A);
#if 1  // jp.kim 25.06.22
        dsp_pf(get_bf_pf_PrdNprdSeason(1, rt, eReceiAct, tptr));
#else
        dsp_pf(get_bf_pf(1, rt, eReceiAct, tptr));
#endif
        dsp_unit = eUnitPercent;
        break;

    case DSPMODE_BBF_EXP_PF_A:
    case DSPMODE_BBF_EXP_PF_B:
    case DSPMODE_BBF_EXP_PF_C:
    case DSPMODE_BBF_EXP_PF_D:
    case DSPMODE_BBF_EXP_PF_T:
        rt = (rate_type)(dsp_mode_in_state - DSPMODE_BBF_EXP_PF_A);
#if 1  // jp.kim 25.06.22
        dsp_pf(get_bf_pf_PrdNprdSeason(2, rt, eReceiAct, tptr));
#else
        dsp_pf(get_bf_pf(2, rt, eReceiAct, tptr));
#endif
        dsp_unit = eUnitPercent;
        break;

    case DSPMODE_CURR_IMP_ACT:
        val = get_curr_rcnt_ch(0, eDmChDeliAct, tptr);
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_CURR_IMP_APP:
        val = get_curr_rcnt_ch(0, eDmChDeliApp, tptr);
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_LAST_IMP_ACT: /* 직전 수요전력(kW) */
        val = get_curr_rcnt_ch(1, eDmChDeliAct, tptr);
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_LAST_IMP_APP:
        val = get_curr_rcnt_ch(1, eDmChDeliApp, tptr);
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_CURR_EXP_ACT:
        val = get_curr_rcnt_ch(0, eDmChReceiAct, tptr);
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_CURR_EXP_APP:
        val = get_curr_rcnt_ch(0, eDmChReceiApp, tptr);
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_LAST_EXP_ACT:
        val = get_curr_rcnt_ch(1, eDmChReceiAct, tptr);
        dsp_energy(0, val);
        dsp_unit = eUnitKw;
        break;

    case DSPMODE_LAST_EXP_APP:
        val = get_curr_rcnt_ch(1, eDmChReceiApp, tptr);
        dsp_energy(0, val);
        dsp_unit = eUnitKva;
        break;

    case DSPMODE_LAST_IMP_PF:
        dsp_pf(get_rcnt_pf(eDeliAct));
        dsp_unit = eUnitPercent;
        break;

    case DSPMODE_LAST_EXP_PF:
        dsp_pf(get_rcnt_pf(eReceiAct));
        dsp_unit = eUnitPercent;
        break;

#if 1 /* bccho, 2024-05-17 */
    case DSPMODE_TOU_PROG_ID:
        dsp_prog_id_reg_date();
        dsp_unit = eUnitNull;
        break;
#endif

    case DSPMODE_REG_DATE: /* 정기 검침일, [메뉴] 버튼으로 설정 가능 (01 ~
                            * 28)
                            */
        dsp_reg_date();
        dsp_unit = eUnitNull;
        break;

#if 1                           // Used only test_mode
    case DSPMODE_TEST_REG_DATE: /* 정기 검침일, [메뉴] 버튼으로 설정 가능
                                   (01 ~ 28) */
        dsp_test_reg_date(mr_date_key_set_buf);
        dsp_unit = eUnitNull;
        break;

    case DSPMODE_TARIFF_RATE:
        dsp_test_tariff_rate(mt_kind_key_set_buf);
        dsp_unit = eUnitNull;
        break;
#endif

    case DSPMODE_mDR_NUM:
        dsp_mDR_num();
        dsp_unit = eUnitNull;
        break;

    case DSPMODE_BUY_ENERGY:
    case DSPMODE_REM_ENERGY:
        if (prepay_fract == 0)
        {
            if (dsp_mode_in_state == DSPMODE_BUY_ENERGY)
                val = get_prepay_info(0);
            else
                val = get_prepay_info(2);

            dsp_digit(val / 1000, NUM_DIGIT, 0,
                      true);  // kwh unit	scale (-3) : 정수부
            dsp_unit = eUnitKwh;

            prepay_fract_val = val % 1000;  // scale (-3) : 소수부
        }
        else
        {
            dsp_digit((uint32_t)prepay_fract_val, 4, 3, false);
            dsp_unit = eUnitKwh;
        }
        break;

    case DSPMODE_BUY_MONEY:
        val = get_prepay_info(1);
        dsp_digit(val, NUM_DIGIT, 0, true);
        dsp_unit = eUnitNull;
        break;

    case DSPMODE_REM_MONEY:
        val = get_prepay_info(3);
        dsp_digit(val, NUM_DIGIT, 0, true);
        dsp_unit = eUnitNull;
        break;

    case DSPMODE_REM_TIME:
        val = get_prepay_info(4);
        dsp_digit(val, NUM_DIGIT, 0, true);
        dsp_unit = eUnitNull;
        break;

    case DSPMODE_ERROR:
        dsp_error();
        dsp_unit = eUnitNull;
        break;

    case DSPMODE_tRaw:
        dsp_unit = eUnitNull;
        break;

    case DSPMODE_LtoL1:
        if ((dsp_toggle_for_v / 3) & 0x01)
        {
            fval = get_inst_LtoL(0);
        }
        else
        {
            fval = get_inst_volt(0);
        }
        dsp_digit((int32_t)(fval * 1000.0), 6, 3, true);

        dsp_toggle_for_v += 1;
        dsp_unit = eUnitV;
        break;
    case DSPMODE_LtoL2:
        if ((dsp_toggle_for_v / 3) & 0x01)
        {
            fval = get_inst_LtoL(1);
        }
        else
        {
            fval = get_inst_volt(1);
        }
        dsp_digit((int32_t)(fval * 1000.0), 6, 3, true);

        dsp_toggle_for_v += 1;
        dsp_unit = eUnitV;
        break;
    case DSPMODE_LtoL3:
        if ((dsp_toggle_for_v / 3) & 0x01)
        {
            fval = get_inst_LtoL(2);
        }
        else
        {
            fval = get_inst_volt(2);
        }
        dsp_digit((int32_t)(fval * 1000.0), 6, 3, true);

        dsp_toggle_for_v += 1;
        dsp_unit = eUnitV;
        break;
    case DSPMODE_V1: /* A상 상전압 (XXX.XXX [V]), A-B 선간 전압 (XXX.XXX
                        [V]) */
        dsp_digit((S32)(get_inst_volt(0) * 1000.0), 6, 3, true);
        dsp_unit = eUnitV;
        break;
    case DSPMODE_V2: /* B상 상전압 (XXX.XXX [V]), B-C 선간 전압 (XXX.XXX
                        [V]) */
        dsp_digit((S32)(get_inst_volt(1) * 1000.0), 6, 3, true);
        dsp_unit = eUnitV;
        break;
    case DSPMODE_V3: /* C상 상전압 (XXX.XXX [V]), C-A 선간 전압 (XXX.XXX
                        [V]) */
        dsp_digit((S32)(get_inst_volt(2) * 1000.0), 6, 3, true);
        dsp_unit = eUnitV;
        break;
    case DSPMODE_I1: /* A상 상전류 (XXX.XXX [A]) */
        dsp_digit((S32)(get_inst_curr(0) * 1000.0), 6, 3, true);
        dsp_unit = eUnitA;
        break;
    case DSPMODE_I2: /* B상 상전류 (XXX.XXX [A]) */
        dsp_digit((S32)(get_inst_curr(1) * 1000.0), 6, 3, true);
        dsp_unit = eUnitA;
        break;
    case DSPMODE_I3: /* C상 상전류 (XXX.XXX [A]) */
        dsp_digit((S32)(get_inst_curr(2) * 1000.0), 6, 3, true);
        dsp_unit = eUnitA;
        break;
    case DSPMODE_P1: /* A상 전압·전류 위상 각 (XXX : 0 ~ 360) */
        dsp_digit((S32)get_inst_phase(0), 6, 0, false);
        dsp_unit = eUnitNull;
        break;
    case DSPMODE_P2: /* B상 전압·전류 위상 각 (XXX : 0 ~ 360) */
        dsp_digit((S32)get_inst_phase(1), 6, 0, false);
        dsp_unit = eUnitNull;
        break;
    case DSPMODE_P3: /* C상 전압·전류 위상 각 (XXX : 0 ~ 360) */
        dsp_digit((S32)get_inst_phase(2), 6, 0, false);
        dsp_unit = eUnitNull;
        break;
    case DSPMODE_THD_V1: /* A상 전압 THD (XXX.XXX [%]) */
        fval = get_inst_volt_THD(0);
        dsp_digit((S32)(fval * 1000.0), 6, 3, true);
        dsp_unit = eUnitPercent;
        break;
    case DSPMODE_THD_V2: /* B상 전압 THD (XXX.XXX [%]) */
        fval = get_inst_volt_THD(1);
        dsp_digit((S32)(fval * 1000.0), 6, 3, true);
        dsp_unit = eUnitPercent;
        break;
    case DSPMODE_THD_V3: /* C상 전압 THD (XXX.XXX [%]) */
        fval = get_inst_volt_THD(2);
        dsp_digit((S32)(fval * 1000.0), 6, 3, true);
        dsp_unit = eUnitPercent;
        break;
    case DSPMODE_tS: /* tS 제어 (타임스위치 제어 시험), [메뉴] 버튼으로 제어
                        시험 가능 */
        dsp_tS();
        break;
    case DSPMODE_TEMP: /* 온도표시 (XX) */
        dsp_digit((S32)get_inst_temp(), 6, 0, false);
        break;
    case DSPMODE_SMODE: /* S-OFF/S-On (단순 검침 모드 설정/해제), [메뉴]
                           버튼으로 설정 가능 */
        dsp_smode();
        break;
    case DSPMODE_PVT: /* Pvt-E/Pvt-d (무부하 시 부하 동작 표시 설정/해제),
                         [메뉴] 버튼으로 설정 가능 */
        dsp_pvt();
        break;
    case DSPMODE_FREQ: /* 주파수 (Hz) */
        fval = get_inst_freq();
        dsp_digit((S32)(fval * 10.0), 6, 1, false);
        dsp_unit = eUnitHz;
        break;

    case DSPMODE_SYSP_SW_VER: /* 운영 소프트웨어 버전 */
        if (dsp_mode_in_state !=
            gen_pre_dsp_mode_in_state)  // eeprom을 반복해서 읽는 것을
                                        // 방지함.
        {
            dsp_SWVER_sys();
        }
        break;

    case DSPMODE_MODEM_VER: /* 내장 모뎀 소프트웨어 버전 */
        if (dsp_mode_in_state != gen_pre_dsp_mode_in_state)
        {
            dsp_SWVER_modem();
        }
        break;

    case DSPMODE_MTP_SW_VER: /* 계량 소프트웨어 버전 */
        if (dsp_mode_in_state != gen_pre_dsp_mode_in_state)
        {
            dsp_SWVER_mtp();
        }
        break;

    case DSPMODE_485_BPS: /* RS-485 현재 통신 속도 */
        dsp_COMSPEED(mdm_baud);
        break;

    case DSPMODE_CONDENSOR_EN: /* C-d/C-E (오결선 : 콘덴서 미부설/부설),
                                  [메뉴] 버튼으로 설정 가능 */
        dsp_condensor_test(condensor_en);
        break;

    case DSPMODE_ERR_PULSE: /* 무효 (rt), 피상 (Pt), 수요 시한 (EOI) 선택,
                               [메뉴] 버튼으로 설정 가능, 피상으로 설정 시
                               유효전력량 펄스는 수요 시한 펄스로 출력하며
                               펄스 폭은 50 ~ 200ms 이내로 한다 */
        dsp_VAR_VA_SEL(err_pulse_react);
        break;

    case DSPMODE_AUTO_BI_DIR_MODE: /* 자동모드 전환 On/OFF, [메뉴] 버튼으로
                                      설정 가능 */
        dsp_auto_bi_dir_SEL();
        break;

#if PHASE_NUM != SINGLE_PHASE
    case DSPMODE_ERR_RATE1: /* A 상 기준 비 오차율 (XXX.XXX [%]) */
        dsp_err_rate1();
        break;

    case DSPMODE_ERR_RATE2: /* B 상 기준 비 오차율 (XXX.XXX [%]) */
        dsp_err_rate2();
        break;

    case DSPMODE_ERR_RATE3: /* C 상 기준 비 오차율 (XXX.XXX [%]) */
        dsp_err_rate3();
        break;
#endif

    case DSPMODE_LATCHON:
        dsp_latchon();
        break;

    case DSPMODE_OVERCURR: /* OC-E/OC-d (과부하 전류 차단 : 최대전류의 1.2배
                              계측 시 차단 설정/복귀), [메뉴] 버튼으로 설정
                              가능
                              - 래치 릴레이 사용 시 */
        dsp_overcurr_set();
        break;

    default:
        break;
    }

#if 1
    gen_pre_dsp_mode_in_state = dsp_mode_in_state;
#endif
}

static void dsp_test_state(uint8_t* tptr)
{
    dsp_mode_in_state = test_state_mode1[dsp_test_mode_index];

    if (!dsp_is_test_update())
        return;

    dsp_var_init();
    dsp_all_mode(tptr);
    dsp_item_num(dsp_test_mode_index + 1);
}

extern int rx_state_disp;
extern int pls_cnt_mon;
extern int rxstate_mon;
extern int rxbuf_cnt;
extern int rtc_rest_mon;
extern int rtc_init_mon;
extern int sag_flag_mon;
extern int sag_sts_mon;
extern int ser_rxcnt_mon;
extern int mtinit_reason;
extern uint32_t sag_exe_mon;
extern bool sag_brownmode_mon;
extern uint8_t bat_inst_mon;
extern int inter_err_mon;

static void dsp_item_num(uint8_t item)
{
    if (dsp_mode_in_state == DSPMODE_ERROR)
    {
        lcd_digit_buf[0] = LDIGIT_SPACE;
        lcd_digit_buf[1] = LDIGIT_SPACE;
    }
    else
    {
        lcd_digit_buf[0] = item / 10;
        lcd_digit_buf[1] = item % 10;
    }
}

static void dsp_energy(bool wh_w, uint32_t val)
{
    uint8_t point;
    uint16_t mul;

    if (wh_w)
        point = lcd_wh_point;
    else
        point = lcd_w_point;

    mul = conv_point_to_mul(point);
    if (mul < PulseKwh)
    {
        mul = PulseKwh / mul;
        val /= mul;
    }
    else
    {
        mul = mul / PulseKwh;
        val *= mul;
    }

    dsp_digit(val, NUM_DIGIT, point, true);
}

void dsp_date(date_time_type* dt)
{
    dsp_point_pos = 5;

    if (dt->month != 0)
    {
        lcd_digit_buf[2] = dt->year / 10;
        lcd_digit_buf[3] = dt->year % 10;
        lcd_digit_buf[4] = dt->month / 10;
        lcd_digit_buf[5] = dt->month % 10;
        lcd_digit_buf[6] = dt->date / 10;
        lcd_digit_buf[7] = dt->date % 10;
    }
    else
    {
        memset(&lcd_digit_buf[2], LDIGIT_BAR, NUM_DIGIT);
    }
}

void dsp_time(date_time_type* dt)
{
    if (dt->month != 0)
    {
        lcd_digit_buf[2] = dt->hour / 10;
        lcd_digit_buf[3] = dt->hour % 10;
        lcd_digit_buf[4] = dt->min / 10;
        lcd_digit_buf[5] = dt->min % 10;
        lcd_digit_buf[6] = dt->sec / 10;
        lcd_digit_buf[7] = dt->sec % 10;
    }
    else
    {
        memset(&lcd_digit_buf[2], LDIGIT_BAR, NUM_DIGIT);
    }
}

/* bccho, 2024-05-17 */
static void dsp_prog_id_reg_date(void)
{
    // 종별
    lcd_digit_buf[2] = 1 + mt_rtkind;
    // 검침일
    lcd_digit_buf[3] = (reg_mr_date / 10);
    lcd_digit_buf[4] = (reg_mr_date % 10);

    // 선택 유효 전력량
    if (mt_is_uni_dir())
    {
        lcd_digit_buf[5] = LDIGIT_r;
        lcd_digit_buf[6] = LDIGIT_r;
    }
    else
    {
        lcd_digit_buf[5] = LDIGIT_S;
        lcd_digit_buf[6] = LDIGIT_r;
    }
    // 게량 모드
    if (meas_method == E_SINGLE_DIR)
        lcd_digit_buf[7] = LDIGIT_d;
#if 1 /* bccho, 2024-09-05, 삼상 */
    else if (meas_method == E_BASIC)
        lcd_digit_buf[7] = LDIGIT_S;
    else
        lcd_digit_buf[7] = LDIGIT_U;
#else
    else
        lcd_digit_buf[7] = LDIGIT_S;
#endif
}

static void dsp_reg_date(void)
{
    // 종별
    lcd_digit_buf[2] = 1 + mt_rtkind;
    // 검침일
    lcd_digit_buf[3] = (reg_mr_date / 10);
    lcd_digit_buf[4] = (reg_mr_date % 10);

    // 선택 유효 전력량
    if (mt_is_uni_dir())
    {
        lcd_digit_buf[5] = LDIGIT_r;
        lcd_digit_buf[6] = LDIGIT_r;
    }
    else
    {
        lcd_digit_buf[5] = LDIGIT_S;
        lcd_digit_buf[6] = LDIGIT_r;
    }
    // 게량 모드
    if (meas_method == E_SINGLE_DIR)
        lcd_digit_buf[7] = LDIGIT_d;
#if 1 /* bccho, 2024-09-05, 삼상 */
    else if (meas_method == E_BASIC)
        lcd_digit_buf[7] = LDIGIT_S;
    else
        lcd_digit_buf[7] = LDIGIT_U;
#else
    else
        lcd_digit_buf[7] = LDIGIT_S;
#endif
}

static void dsp_test_reg_date(U8 t8)
{
    dsp_digit((S32)t8, 2, 0, true);
    /* "“dAy-01”" */
    lcd_digit_buf[2] = LDIGIT_d;
    lcd_digit_buf[3] = LDIGIT_A;
    lcd_digit_buf[4] = LDIGIT_y;
    lcd_digit_buf[5] = LDIGIT_BAR;
}

static void dsp_test_tariff_rate(U8 t8)
{
    t8 += 1;
    dsp_digit((S32)t8, 1, 0, false);

    /* "rAtE-1" */
    lcd_digit_buf[2] = LDIGIT_r;
    lcd_digit_buf[3] = LDIGIT_A;
    lcd_digit_buf[4] = LDIGIT_t;
    lcd_digit_buf[5] = LDIGIT_E;
    lcd_digit_buf[6] = LDIGIT_BAR;
}

static void dsp_mDR_num(void)
{
    dsp_digit((int32_t)log_cnt[eLogDRm], NUM_DIGIT, 0, false);
}

static void dsp_error(void)
{
    dsp_digit((int32_t)dsp_error_mode_val(), NUM_DIGIT, 0, true);
}

static void dsp_tS(void)
{
    lcd_digit_buf[2] = LDIGIT_SPACE;
    lcd_digit_buf[3] = LDIGIT_SPACE;
    lcd_digit_buf[4] = LDIGIT_SPACE;
    lcd_digit_buf[5] = LDIGIT_SPACE;
    lcd_digit_buf[6] = LDIGIT_t;
    lcd_digit_buf[7] = LDIGIT_5;
}

static void dsp_SWVER_all(uint8_t pos)
{
    uint8_t i, val;
    ST_FW_INFO fwinfo;

    dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, pos);

    for (i = 0; i < FW_VERSION_SIZE; i++)
    {
        val = fwinfo.version[i];
        if ((val >= '0') && (val <= '9'))
            lcd_digit_buf[i + 2] = val - '0';
        else if ((val >= 'A') && (val <= 'F'))
            lcd_digit_buf[i + 2] = val - 'A' + LDIGIT_A;
        else
            lcd_digit_buf[i + 2] = LDIGIT_BAR;
    }
}

static void dsp_SWVER_sys(void)
{
#if 0  // MAKER_DEFINE == JND
    lcd_digit_buf[2] = LDIGIT_1;
    lcd_digit_buf[3] = LDIGIT_0;
    lcd_digit_buf[4] = LDIGIT_7;
    lcd_digit_buf[5] = LDIGIT_9;
    lcd_digit_buf[6] = LDIGIT_0;
    lcd_digit_buf[7] = LDIGIT_0;
#else
    dsp_SWVER_all(FWINFO_CUR_SYS);
#endif
}

static void dsp_SWVER_modem(void)
{
#if 0  // MAKER_DEFINE == JND
    lcd_digit_buf[2] = LDIGIT_1;
    lcd_digit_buf[3] = LDIGIT_1;
    lcd_digit_buf[4] = LDIGIT_7;
    lcd_digit_buf[5] = LDIGIT_9;
    lcd_digit_buf[6] = LDIGIT_0;
    lcd_digit_buf[7] = LDIGIT_0;
#else
    dsp_SWVER_all(FWINFO_CUR_MODEM);
#endif
}

static void dsp_SWVER_mtp(void) { dsp_SWVER_all(FWINFO_CUR_METER); }

static void dsp_VAR_VA_SEL(uint8_t pls)
{
    lcd_digit_buf[2] = LDIGIT_SPACE;
    lcd_digit_buf[3] = LDIGIT_SPACE;
    lcd_digit_buf[4] = LDIGIT_SPACE;

    switch (pls)
    {
    case 0:  // 피상
        lcd_digit_buf[5] = LDIGIT_SPACE;
        lcd_digit_buf[6] = LDIGIT_P;
        lcd_digit_buf[7] = LDIGIT_t;
        break;
    case 1:  // 무효
        lcd_digit_buf[5] = LDIGIT_SPACE;
        lcd_digit_buf[6] = LDIGIT_r;
        lcd_digit_buf[7] = LDIGIT_t;
        break;
    case 2:  // EOI
        lcd_digit_buf[5] = LDIGIT_E;
        lcd_digit_buf[6] = LDIGIT_0;
        lcd_digit_buf[7] = LDIGIT_I;
        break;
    }
}

static void dsp_COMSPEED(baudrate_type baud)
{
    lcd_digit_buf[2] = LDIGIT_SPACE;
    if (baud == BAUD_9600)
    {
        lcd_digit_buf[3] = LDIGIT_SPACE;
        lcd_digit_buf[4] = LDIGIT_9;
        lcd_digit_buf[5] = LDIGIT_6;
    }
    else if (baud == BAUD_19200)
    {
        lcd_digit_buf[3] = LDIGIT_1;
        lcd_digit_buf[4] = LDIGIT_9;
        lcd_digit_buf[5] = LDIGIT_2;
    }

    else if (baud == BAUD_38400)
    {
        lcd_digit_buf[3] = LDIGIT_3;
        lcd_digit_buf[4] = LDIGIT_8;
        lcd_digit_buf[5] = LDIGIT_4;
    }
    else if (baud == BAUD_57600)
    {
        lcd_digit_buf[3] = LDIGIT_5;
        lcd_digit_buf[4] = LDIGIT_7;
        lcd_digit_buf[5] = LDIGIT_6;
    }
    else if (baud == BAUD_115200)  // jp.kim 24.11.21
    {
        lcd_digit_buf[2] = LDIGIT_1;
        lcd_digit_buf[3] = LDIGIT_1;
        lcd_digit_buf[4] = LDIGIT_5;
        lcd_digit_buf[5] = LDIGIT_2;
        lcd_digit_buf[6] = LDIGIT_0;
        lcd_digit_buf[7] = LDIGIT_0;
    }

    lcd_digit_buf[6] = LDIGIT_0;
    lcd_digit_buf[7] = LDIGIT_0;
}

static void dsp_auto_bi_dir_SEL(void)
{
    if (auto_mode_sel)
    {
        lcd_digit_buf[2] = LDIGIT_SPACE;
        lcd_digit_buf[3] = LDIGIT_SPACE;
        lcd_digit_buf[4] = LDIGIT_SPACE;
        lcd_digit_buf[5] = LDIGIT_o;
        lcd_digit_buf[6] = LDIGIT_n;
        lcd_digit_buf[7] = LDIGIT_SPACE;
    }
    else
    {
        lcd_digit_buf[2] = LDIGIT_SPACE;
        lcd_digit_buf[3] = LDIGIT_SPACE;
        lcd_digit_buf[4] = LDIGIT_SPACE;
        lcd_digit_buf[5] = LDIGIT_o;
        lcd_digit_buf[6] = LDIGIT_F;
        lcd_digit_buf[7] = LDIGIT_F;
    }
}

#if PHASE_NUM != SINGLE_PHASE
static void dsp_err_rate1(void) {}
static void dsp_err_rate2(void) {}
static void dsp_err_rate3(void) {}
#endif

static void dsp_condensor_test(uint8_t conds)
{
    lcd_digit_buf[2] = LDIGIT_SPACE;
    lcd_digit_buf[3] = LDIGIT_SPACE;
    lcd_digit_buf[4] = LDIGIT_SPACE;
    lcd_digit_buf[5] = LDIGIT_C;
    lcd_digit_buf[6] = LDIGIT_BAR;
    if (conds)
        lcd_digit_buf[7] = LDIGIT_E;
    else
        lcd_digit_buf[7] = LDIGIT_d;
}

static void dsp_smode(void)
{
    if (circdsp_is_smode())
    {
        lcd_digit_buf[2] = LDIGIT_SPACE;
        lcd_digit_buf[3] = LDIGIT_SPACE;
        lcd_digit_buf[4] = LDIGIT_5;
        lcd_digit_buf[5] = LDIGIT_BAR;
        lcd_digit_buf[6] = LDIGIT_o;
        lcd_digit_buf[7] = LDIGIT_n;
    }
    else
    {
        lcd_digit_buf[2] = LDIGIT_SPACE;
        lcd_digit_buf[3] = LDIGIT_5;
        lcd_digit_buf[4] = LDIGIT_BAR;
        lcd_digit_buf[5] = LDIGIT_o;
        lcd_digit_buf[6] = LDIGIT_F;
        lcd_digit_buf[7] = LDIGIT_F;
    }
}

static void dsp_pvt(void)
{
    if (circdsp_is_pvt_mode())
    {
        /* PVT Enable */
        lcd_digit_buf[2] = LDIGIT_SPACE;
        lcd_digit_buf[3] = LDIGIT_P;
        lcd_digit_buf[4] = LDIGIT_u;
        lcd_digit_buf[5] = LDIGIT_t;
        lcd_digit_buf[6] = LDIGIT_BAR;
        lcd_digit_buf[7] = LDIGIT_E;
    }
    else
    {
        /* PVT Disable */
        lcd_digit_buf[2] = LDIGIT_SPACE;
        lcd_digit_buf[3] = LDIGIT_P;
        lcd_digit_buf[4] = LDIGIT_u;
        lcd_digit_buf[5] = LDIGIT_t;
        lcd_digit_buf[6] = LDIGIT_BAR;
        lcd_digit_buf[7] = LDIGIT_d;
    }
}

static void dsp_pf(uint8_t _pf)
{
    if (_pf != 0xff)
    {
        dsp_digit((int32_t)_pf, NUM_DIGIT, 0, FALSE);
    }
    else
    {
        dsp_point_pos = 0;

        lcd_digit_buf[2] = LDIGIT_SPACE;
        lcd_digit_buf[3] = LDIGIT_SPACE;
        lcd_digit_buf[4] = LDIGIT_SPACE;
        lcd_digit_buf[5] = LDIGIT_BAR;
        lcd_digit_buf[6] = LDIGIT_BAR;
        lcd_digit_buf[7] = LDIGIT_BAR;
    }
}

static void dsp_latchon(void)
{
    uint16_t _cnt;

    _cnt = get_latchon_cnt();
    dsp_digit((uint32_t)_cnt, 6, 0, true);
    lcd_digit_buf[2] = LDIGIT_L;
}

static void dsp_overcurr_set(void)
{
    if (overcurr_cut_en)
    {
        lcd_digit_buf[2] = LDIGIT_SPACE;
        lcd_digit_buf[3] = LDIGIT_SPACE;
        lcd_digit_buf[4] = LDIGIT_o;
        lcd_digit_buf[5] = LDIGIT_C;
        lcd_digit_buf[6] = LDIGIT_BAR;
        lcd_digit_buf[7] = LDIGIT_E;
    }
    else
    {
        lcd_digit_buf[2] = LDIGIT_SPACE;
        lcd_digit_buf[3] = LDIGIT_SPACE;
        lcd_digit_buf[4] = LDIGIT_o;
        lcd_digit_buf[5] = LDIGIT_C;
        lcd_digit_buf[6] = LDIGIT_BAR;
        lcd_digit_buf[7] = LDIGIT_d;
    }
}

static bool dsp_fill_error_item(uint8_t* dgt_buf)
{
    /* 구매 규격 3.16.3 자기진단 항목 */
    bool rslt;

    if (run_is_bat_power())
        return false;  // 14.11.16

    rslt = false;

    *(dgt_buf + 0) = LDIGIT_SPACE;
    *(dgt_buf + 1) = LDIGIT_SPACE;
    memset(dgt_buf + 2, LDIGIT_BAR, (NUM_MODE_DIGIT - 2));

    if (line_is_wrong_connected())
    {
        dgt_buf[2] = LDIGIT_C;
        rslt = true;
    }

    if (IS_MAGNET_DET || IS_MAGNET_DETED_THISMONTH)
    {
        dgt_buf[3] = LDIGIT_n;
        rslt = true;
    }

    if ((WMStatus & TEMPOVER) || IS_TEMPOVER_DETED_THISMONTH)
    {
        dgt_buf[4] = LDIGIT_t;
        rslt = TRUE;
    }

    if ((WMStatus & RLYERRMASK) || IS_RELAYERR_DETED_THISMONTH)
    {
        dgt_buf[5] = LDIGIT_L;
        rslt = true;
    }

    if ((WMStatus & IOVERMASK) != 0L)
    {
        dgt_buf[6] = LDIGIT_o;
        rslt = true;
    }

    return rslt;
}

static bool dsp_is_error_item(void)
{
    if (run_is_bat_power())
        return false;

    if (line_is_wrong_connected())
        return true;

    if (IS_MAGNET_DET || IS_MAGNET_DETED_THISMONTH)
        return true;

    if ((WMStatus & TEMPOVER) || IS_TEMPOVER_DETED_THISMONTH)
        return true;

    if ((WMStatus & RLYERRMASK) || IS_RELAYERR_DETED_THISMONTH)
        return true;

    if ((WMStatus & IOVERMASK) != 0L)
        return true;

    return false;
}

bool dsp_is_test_overcurr(void)
{
    return (test_state_mode1[dsp_test_mode_index] == DSPMODE_OVERCURR);
}

void dsp_test_overcurr_toggle(void)
{
    if (overcurr_cut_en)
        overcurr_cut_en = 0;
    else
        overcurr_cut_en = 1;
}

void dsp_test_condensor_toggle(void)
{
    if (condensor_en)
        condensor_en = 0;
    else
        condensor_en = 1;
}

void dsp_test_err_pusle_toggle(void)
{
    ST_MIF_METER_PARM* pst_mif_meter_parm = dsm_mtp_get_meter_parm();

    if ((!err_pulse_react) || (err_pulse_react >= 3))
        err_pulse_react = 2;
    else
        err_pulse_react--;

    eoi_or_pulse_select();

    pst_mif_meter_parm->pulse_select = err_pulse_react;
    mif_meter_parm_set();
}

void dsp_test_auto_bidir_toggle(void)
{
    if (auto_mode_sel)
        auto_mode_sel = 0;
    else
        auto_mode_sel = 1;
}

static uint16_t dsp_error_mode_val(void)
{
    uint16_t t16;
    t16 = 0;
    if (line_is_wrong_connected())
        t16 += 1;
    if (IS_MAGNET_DET || IS_MAGNET_DETED_THISMONTH)
        t16 += 10;
    if ((WMStatus & TEMPOVER) || IS_TEMPOVER_DETED_THISMONTH)
        t16 += 100;
    if ((WMStatus & RLYERRMASK) || IS_RELAYERR_DETED_THISMONTH)
        t16 += 1000;

    return t16;
}

void dsp_test_smode_toggle(void) { circdsp_smode_toggle(); }

void dsp_test_pvt_toggle(void) { circdsp_pvt_mode_toggle(); }

bool dsp_test_mode_inc(void)
{
    disp_mode_type dm;

    dsp_mode_chged_set();
    dsp_toggle_for_v = 0;

    ++dsp_test_mode_index;
    dm = test_state_mode1[dsp_test_mode_index];
    if (dm == NUM_DSPMODE)
    {
        dsp_test_mode_index = 0;
        return 0;
    }

    return 1;
}

bool dsp_is_test_mode_tS(void)
{
    return (test_state_mode1[dsp_test_mode_index] == DSPMODE_tS);
}

bool dsp_is_test_smode(void)
{
    return (test_state_mode1[dsp_test_mode_index] == DSPMODE_SMODE);
}

bool dsp_is_test_pvt(void)
{
    return (test_state_mode1[dsp_test_mode_index] == DSPMODE_PVT);
}

bool dsp_is_test_condensor(void)
{
    return (test_state_mode1[dsp_test_mode_index] == DSPMODE_CONDENSOR_EN);
}

bool dsp_is_test_err_pusle(void)
{
    return (test_state_mode1[dsp_test_mode_index] == DSPMODE_ERR_PULSE);
}

bool dsp_is_test_auto_bidir(void)
{
    return (test_state_mode1[dsp_test_mode_index] == DSPMODE_AUTO_BI_DIR_MODE);
}

/* 1P2W 시험 모드 표시항목 - 정기 검침일, [메뉴] 버튼으로 설정 가능 (01 ~
 * 28) */
bool dsp_is_test_reg_mr_date(void)
{
    return (test_state_mode1[dsp_test_mode_index] == DSPMODE_TEST_REG_DATE);
}

void dsp_test_reg_mr_date_change_check(U8* tptr)
{
    if (reg_mr_date != mr_date_key_set_buf)
    {
        tou_id_change_sts = 0;

        mrdate_bakup = reg_mr_date;
        reg_mr_date = mr_date_key_set_buf;
        prog_chg_proc_by_key(mrdate_bakup, mt_rtkind, meas_method, tptr);

        // prog_in_state = E_PROG_KEY;

        if (run_is_bat_power())
        {  // 23.12.28 // 14.7.24
            whm_op_save();
            mt_conf_save();
            mt_conf_2_save();
        }

        dsm_progname_update_forReport();  // TOU 프로그램 이름 변경
    }
}

void dsp_test_reg_mr_date_change(void)
{
    /* 테스트 모드에서 정기 검침일 설정 */

    // uint8_t mr_date;

    uint8_t* tptr;
    tptr = (uint8_t*)adjust_tptr(&global_buff[0]);

    // mr_date = reg_mr_date;
    if (mr_date_key_set_buf < 28)
    {
        mr_date_key_set_buf++;
    }
    else
    {
        mr_date_key_set_buf = 1;
    }
#if 0	
	dsp_digit((int32_t)mr_date, 2, 0, true);
#else
    dsp_test_reg_date(mr_date_key_set_buf);
#endif

    //
    /*
        TODO: (WD) 날짜 반영 및 eeprom 기록은 따로 할 필요가 있음. 수명
       문제. (항목이 바뀔 경우 or 테스트 모드를 빠져 나갈 경우), 검침일이
       변경 되었는지 판단할 수 있는 코드가 필요함. 현재 어디서 데이터를
       판단하여 저장을 하고 있는지 정확하게 모르겠음. 현장에서 버튼으로 정기
       검침일을 바꿀 경우가(이사 or 계기 교체 정도이려나..) 거의 없을 것으로
       판단되기 때문에 그리 중요하지는 않을 듯.
    */
}

/* 1P2W 시험 모드 표시항목 - 종별 설정, [메뉴] 버튼으로 설정 가능 (1 ~ 2) */
bool dsp_is_test_tariff_rate(void)
{
    return (test_state_mode1[dsp_test_mode_index] == DSPMODE_TARIFF_RATE);
}

void dsp_test_tariff_rate_change_check(U8* tptr)
{
    /*
        TODO: (WD) 날짜 반영 및 eeprom 기록은 따로 할 필요가 있음. 수명
       문제. (항목이 바뀔 경우 or 테스트 모드를 빠져 나갈 경우), 검침일이
       변경 되었는지 판단할 수 있는 코드가 필요함. 현재 어디서 데이터를
       판단하여 저장을 하고 있는지 정확하게 모르겠음. 현장에서 버튼으로 정기
       검침일을 바꿀 경우가(이사 or 계기 교체 정도이려나..) 거의 없을 것으로
       판단되기 때문에 그리 중요하지는 않을 듯.
    */
    if (mt_rtkind != mt_kind_key_set_buf)
    {
        tou_id_change_sts = 0;

        rtkind_bakup = mt_rtkind;
        mt_rtkind = mt_kind_key_set_buf;

        prog_cur_tou_suppdsp_delete();
        prog_fut_delete();

        prog_chg_proc_by_key(reg_mr_date, rtkind_bakup, meas_method, tptr);

        lcd_dsp_mode = DEFAULT_DSP_MODE;
        holiday_sel1 = DEFAULT_holiday_sel;

        if (run_is_bat_power())  // 14.7.24
        {
            whm_op_save();
            mt_conf_save();
            mt_conf_2_save();
        }

        dsm_progname_update_forReport();  // TOU 프로그램 이름 변경
    }
}

void dsp_test_tariff_rate_change(void)
{
    /* 테스트 모드에서 종별 설정*/

    // uint8_t mt_kind;
    tou_struct_type ttou;

    uint8_t* tptr;
    tptr = (uint8_t*)adjust_tptr(&global_buff[0]);

    // mt_kind = mt_rtkind;
    if (mt_kind_key_set_buf == ONE_RATE_KIND)
    {
        mt_kind_key_set_buf = TWO_RATE_KIND;
    }
    else
    {
        mt_kind_key_set_buf = ONE_RATE_KIND;
    }

    dsp_test_tariff_rate(mt_kind_key_set_buf);

    //
    /*
        TODO: (WD) 날짜 반영 및 eeprom 기록은 따로 할 필요가 있음. 수명
       문제. (항목이 바뀔 경우 or 테스트 모드를 빠져 나갈 경우), 검침일이
       변경 되었는지 판단할 수 있는 코드가 필요함. 현재 어디서 데이터를
       판단하여 저장을 하고 있는지 정확하게 모르겠음. 현장에서 버튼으로 정기
       검침일을 바꿀 경우가(이사 or 계기 교체 정도이려나..) 거의 없을 것으로
       판단되기 때문에 그리 중요하지는 않을 듯.
    */
}

#if 0 /* bccho, 2024-01-10, 1227 포팅 */
disp_input_type get_dsp_inputmode(void) { return disp_input_mode; }

bool dsp_is_inp_end(void) { return (disp_input_mode == DISPINP_END); }

static void dsp_input_state(void)
{
    dsp_var_init();

    dsp_inp_mode_num();

    dsp_inp_mode_digit();

    dsp_inp_mode_dot();
}

static void dsp_inp_mode_num(void)
{
    switch (disp_input_mode)
    {
    case DISPINP_DATE:
    case DISPINP_TIME:
    case DISPINP_REGREAD_DATE:
    case DISPINP_SIGSEL:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = disp_input_mode - DISPINP_DATE + 1;
        break;

    case DISPINP_sCURR:
        lcd_digit_buf[0] = LDIGIT_C;
        lcd_digit_buf[1] = LDIGIT_2;
        break;

    case DISPINP_sCURR_aRTN:
        lcd_digit_buf[0] = LDIGIT_C;
        lcd_digit_buf[1] = LDIGIT_1;
        break;

    case DISPINP_sCURR_2:
        lcd_digit_buf[0] = LDIGIT_C;
        lcd_digit_buf[1] = LDIGIT_3;
        break;

    case DISPINP_TS:
        lcd_digit_buf[0] = LDIGIT_t;
        lcd_digit_buf[1] = get_ts_inp_index() + 1;
        break;

    case DISPINP_RATEKIND:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_A;
        break;

    case DISPINP_TOU:
        lcd_digit_buf[0] = LDIGIT_d;
        lcd_digit_buf[1] = get_tou_inp_index() + 1;
        break;

    case DISPINP_DMINTV:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_b;
        break;

    case DISPINP_SN_1:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_C;
        break;

    case DISPINP_SN_2:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_d;
        break;

    case DISPINP_MT_DIR:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_E;
        break;

    case DISPINP_MEAS:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_F;
        break;

    case DISPINP_BAUD:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_6;
        break;

    case DISPINP_TEMP:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_H;
        break;

    case DISPINP_ERR_PLS:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_P;
        break;

    case DISPINP_CONDENSOR:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_U;
        break;

    case DISPINP_COMMEN_COVEROPEN:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_0;
        break;

    case DISPINP_sCURR_HOLD:
        lcd_digit_buf[0] = LDIGIT_E;
        lcd_digit_buf[1] = LDIGIT_1;
        break;

    case DISPINP_sCURR_RTN1:
        lcd_digit_buf[0] = LDIGIT_E;
        lcd_digit_buf[1] = LDIGIT_2;
        break;

    case DISPINP_sCURR_RTN_DUR1:
        lcd_digit_buf[0] = LDIGIT_E;
        lcd_digit_buf[1] = LDIGIT_3;
        break;

    case DISPINP_sCURR_RTN_DUR2:
        lcd_digit_buf[0] = LDIGIT_E;
        lcd_digit_buf[1] = LDIGIT_4;
        break;

    case DSPINP_CONTRACT_MONTH:
        lcd_digit_buf[0] = LDIGIT_5;
        lcd_digit_buf[1] = LDIGIT_5;
        break;

    case DISPINP_END:
        lcd_digit_buf[0] = LDIGIT_SPACE;
        lcd_digit_buf[1] = LDIGIT_SPACE;
        break;

    default:
        break;
    }

    if (dispinp_err)
    {
        if (!(blink_timer & 0x01))
        {
            lcd_digit_buf[0] = LDIGIT_SPACE;
            lcd_digit_buf[1] = LDIGIT_SPACE;
        }
    }
}

static void dsp_inp_mode_digit(void)
{
    memcpy(&lcd_digit_buf[2], lcd_input_buf, NUM_DIGIT);

    if (disp_input_mode != DISPINP_SN_1 && disp_input_mode != DISPINP_SN_2 &&
        disp_input_mode != DISPINP_END)
    {
        if (!(blink_timer & 0x01))
        {
            if (disp_input_mode == DISPINP_DMINTV ||
                disp_input_mode == DISPINP_SIGSEL ||
                disp_input_mode == DISPINP_MT_DIR ||
                disp_input_mode == DISPINP_MEAS ||
                disp_input_mode == DISPINP_BAUD ||
                disp_input_mode == DISPINP_ERR_PLS ||
                disp_input_mode == DISPINP_CONDENSOR ||
                disp_input_mode == DISPINP_COMMEN_COVEROPEN)
            {
                memset(&lcd_digit_buf[2], LDIGIT_SPACE, NUM_DIGIT);
            }
            else
            {
                lcd_digit_buf[2 + digit_pos] = LDIGIT_SPACE;
            }
        }
    }
}

static void dsp_inp_mode_dot(void)
{
    switch (disp_input_mode)
    {
    case DISPINP_DATE:
        dsp_point_pos = 5;
        break;
    case DISPINP_TIME:
        dsp_unit = eUnitClock1;
        break;
    case DISPINP_TS:
    case DISPINP_TOU:
        dsp_unit = eUnitClock2;
        break;
    default:
        break;
    }
}

void dsp_inp_time_init(date_time_type *pdt)
{
    disp_input_mode = DISPINP_TIME;
    digit_pos = 3;
    dispinp_err = 0;

    lcd_input_buf[0] = pdt->hour / 10;
    lcd_input_buf[1] = pdt->hour % 10;
    lcd_input_buf[2] = pdt->min / 10;
    lcd_input_buf[3] = pdt->min % 10;
    lcd_input_buf[4] = 0;
    lcd_input_buf[5] = 0;
}

void dsp_inp_regrd_init(uint8_t regrd)
{
    disp_input_mode = DISPINP_REGREAD_DATE;
    digit_pos = 5;
    dispinp_err = 0;

    memset(lcd_input_buf, LDIGIT_SPACE, NUM_DIGIT);

    lcd_input_buf[4] = regrd / 10;
    lcd_input_buf[5] = regrd % 10;
}

void dsp_inp_sigsel_init(ad_sig_type sg)
{
    disp_input_mode = DISPINP_SIGSEL;
    digit_pos = 5;
    dispinp_err = 0;

    memset(lcd_input_buf, LDIGIT_SPACE, NUM_DIGIT);
    dsp_inp_sig_sel(sg);
}

void dsp_inp_scurr_init(uint16_t scurr)
{
    disp_input_mode = DISPINP_sCURR;
    dispinp_err = 0;
    digit_pos = 5;

    lcd_input_buf[0] = LDIGIT_SPACE;
    dsp_fill_decimal_5digit(&lcd_input_buf[1], scurr);
}

void dsp_inp_sCurrCnt_init(uint8_t cnt)
{
    disp_input_mode = DISPINP_sCURR_aRTN;
    digit_pos = 5;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_t;
    lcd_input_buf[1] = LDIGIT_C;
    lcd_input_buf[2] = LDIGIT_n;
    lcd_input_buf[3] = LDIGIT_t;
    lcd_input_buf[4] = cnt / 10;
    lcd_input_buf[5] = cnt % 10;
}

void dsp_inp_ts_init(ts_struct_type *ts)
{
    disp_input_mode = DISPINP_TS;
    digit_pos = 5;
    dispinp_err = 0;

    dsp_inp_ts_time(ts);
}

void dsp_inp_contract_month_init(uint32_t _contract)
{
    disp_input_mode = DSPINP_CONTRACT_MONTH;
    digit_pos = 5;
    dispinp_err = 0;

    dsp_fill_decimal_6digit(&lcd_input_buf[0], _contract / 1000);  // kw unit
}

void dsp_inp_ratekind_init(ratekind_type mk)
{
    disp_input_mode = DISPINP_RATEKIND;
    digit_pos = 5;
    dispinp_err = 0;

    memset(lcd_input_buf, LDIGIT_SPACE, NUM_DIGIT);
    lcd_input_buf[5] = mk + 1;
}

void dsp_inp_tou_rate_init(tou_struct_type *tou)
{
    disp_input_mode = DISPINP_TOU;
    digit_pos = 5;
    dispinp_err = 0;

    dsp_inp_tou_rate_time(tou);
}

uint8_t dsp_inp_lpintv_init(uint8_t intv)
{
    uint8_t idx;

    disp_input_mode = DISPINP_DMINTV;
    dispinp_err = 0;

    memset(lcd_input_buf, LDIGIT_SPACE, NUM_DIGIT);

    idx = get_dmintv_index(intv);
    dsp_dmintv_set(idx);

    return idx;
}

void dsp_dmintv_set(uint8_t idx)
{
    lcd_input_buf[4] = get_dm_interval(idx) / 10;
    lcd_input_buf[5] = get_dm_interval(idx) % 10;
}

void dsp_inp_sn1_init(void)
{
    uint8_t tbuf[sizeof(ser_no_type)];

    disp_input_mode = DISPINP_SN_1;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_SPACE;

    get_manuf_id(tbuf);
    lcd_input_buf[1] = tbuf[0] - '0';
    lcd_input_buf[2] = tbuf[1] - '0';
    lcd_input_buf[3] = tbuf[2] - '0';
    lcd_input_buf[4] = tbuf[3] - '0';
    if (get_cust_id(tbuf))
        lcd_input_buf[5] = tbuf[0] - '0';
    else
        lcd_input_buf[5] = LDIGIT_BAR;
}

void dsp_inp_sn2_init(void)
{
    uint8_t i;
    uint8_t tbuf[sizeof(ser_no_type)];

    disp_input_mode = DISPINP_SN_2;
    dispinp_err = 0;

    if (get_cust_id(tbuf))
    {
        for (i = 0; i < NUM_DIGIT; i++) lcd_input_buf[i] = tbuf[i + 1] - '0';
    }
    else
    {
        memset(&lcd_input_buf[0], LDIGIT_BAR, NUM_DIGIT);
    }
}

void dsp_inp_mtdir_init(uint8_t dir)
{
    disp_input_mode = DISPINP_MT_DIR;
    dispinp_err = 0;

    memset(lcd_input_buf, LDIGIT_SPACE, NUM_DIGIT);

    if (dir == MT_UNI_DIR)
    {
        lcd_input_buf[NUM_DIGIT - 2] = LDIGIT_r;
        lcd_input_buf[NUM_DIGIT - 1] = LDIGIT_r;
    }
    else
    {
        lcd_input_buf[NUM_DIGIT - 2] = LDIGIT_5;
        lcd_input_buf[NUM_DIGIT - 1] = LDIGIT_r;
    }
}

void dsp_inp_meas_init(meas_method_type meas)
{
    disp_input_mode = DISPINP_MEAS;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_SPACE;
    lcd_input_buf[1] = LDIGIT_SPACE;
    lcd_input_buf[2] = LDIGIT_SPACE;
    lcd_input_buf[3] = LDIGIT_SPACE;
    lcd_input_buf[4] = LDIGIT_SPACE;

    if (meas == E_BASIC)
    {
        lcd_input_buf[5] = LDIGIT_5;
    }
    else
    {
        lcd_input_buf[5] = LDIGIT_d;
    }
}

void dsp_inp_baud_init(baudrate_type baud)
{
    disp_input_mode = DISPINP_BAUD;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_SPACE;
    if (baud == BAUD_9600)
    {
        lcd_input_buf[1] = LDIGIT_SPACE;
        lcd_input_buf[2] = LDIGIT_9;
        lcd_input_buf[3] = LDIGIT_6;
    }
    else if (baud == BAUD_19200)
    {
        lcd_input_buf[1] = LDIGIT_1;
        lcd_input_buf[2] = LDIGIT_9;
        lcd_input_buf[3] = LDIGIT_2;
    }
    else if (baud == BAUD_38400)
    {
        lcd_input_buf[1] = LDIGIT_1;
        lcd_input_buf[2] = LDIGIT_9;
        lcd_input_buf[3] = LDIGIT_2;
    }
    else if (baud == BAUD_57600)
    {
        lcd_input_buf[1] = LDIGIT_5;
        lcd_input_buf[2] = LDIGIT_7;
        lcd_input_buf[3] = LDIGIT_6;
    }

    if (baud == BAUD_115200)
    {
        lcd_input_buf[4] = LDIGIT_2;
        lcd_input_buf[5] = LDIGIT_0;
    }
    else
    {
        lcd_input_buf[4] = LDIGIT_0;
        lcd_input_buf[5] = LDIGIT_0;
    }
}

void dsp_inp_temp_init(int8_t temp)
{
    disp_input_mode = DISPINP_TEMP;
    digit_pos = 5;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_SPACE;
    lcd_input_buf[1] = LDIGIT_SPACE;
    lcd_input_buf[2] = LDIGIT_SPACE;
    lcd_input_buf[3] = LDIGIT_SPACE;
    lcd_input_buf[4] = temp / 10;
    lcd_input_buf[5] = temp % 10;
}

void dsp_inp_errpls_init(uint8_t pls)
{
    disp_input_mode = DISPINP_ERR_PLS;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_SPACE;
    lcd_input_buf[1] = LDIGIT_SPACE;
    lcd_input_buf[2] = LDIGIT_SPACE;

    switch (pls)
    {
    case 0:  // 피상
        lcd_input_buf[3] = LDIGIT_SPACE;
        lcd_input_buf[4] = LDIGIT_P;
        lcd_input_buf[5] = LDIGIT_t;
        break;
    case 1:  // 무효
        lcd_input_buf[3] = LDIGIT_SPACE;
        lcd_input_buf[4] = LDIGIT_r;
        lcd_input_buf[5] = LDIGIT_t;
        break;
    case 2:  // EOI
        lcd_input_buf[3] = LDIGIT_E;
        lcd_input_buf[4] = LDIGIT_0;
        lcd_input_buf[5] = LDIGIT_I;
        break;
    }
}

void dsp_inp_condensor_init(uint8_t conds)
{
    disp_input_mode = DISPINP_CONDENSOR;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_SPACE;
    lcd_input_buf[1] = LDIGIT_SPACE;
    lcd_input_buf[2] = LDIGIT_SPACE;
    lcd_input_buf[3] = LDIGIT_C;
    lcd_input_buf[4] = LDIGIT_BAR;
    if (conds)
        lcd_input_buf[5] = LDIGIT_E;
    else
        lcd_input_buf[5] = LDIGIT_d;
}

void dsp_inp_commen_init(uint8_t commen)
{
    disp_input_mode = DISPINP_COMMEN_COVEROPEN;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_SPACE;
    lcd_input_buf[1] = LDIGIT_SPACE;
    lcd_input_buf[2] = LDIGIT_C;
    lcd_input_buf[3] = LDIGIT_0;
    lcd_input_buf[4] = LDIGIT_BAR;
    if (commen)
        lcd_input_buf[5] = LDIGIT_E;
    else
        lcd_input_buf[5] = LDIGIT_d;
}

void dsp_inp_scurr_2_init(int16_t scurr)
{
    disp_input_mode = DISPINP_sCURR_2;
    dispinp_err = 0;
    digit_pos = 5;

    lcd_input_buf[0] = LDIGIT_SPACE;
    dsp_fill_decimal_5digit(&lcd_input_buf[1], scurr);
}

void dsp_inp_scurr_hold_init(uint16_t _hold)
{
    disp_input_mode = DISPINP_sCURR_HOLD;
    digit_pos = 5;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_H;
    lcd_input_buf[1] = LDIGIT_t;
    dsp_fill_decimal_4digit(&lcd_input_buf[2], _hold);
}

void dsp_inp_scurr_n1_init(uint8_t _cnt)
{
    disp_input_mode = DISPINP_sCURR_RTN1;
    digit_pos = 5;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_1;
    lcd_input_buf[1] = LDIGIT_r;
    lcd_input_buf[2] = LDIGIT_C;
    lcd_input_buf[3] = LDIGIT_t;
    lcd_input_buf[4] = _cnt / 10;
    lcd_input_buf[5] = _cnt % 10;
}

void dsp_inp_scurr_dur1_init(uint16_t _dur1)
{
    disp_input_mode = DISPINP_sCURR_RTN_DUR1;
    digit_pos = 5;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_A;
    lcd_input_buf[1] = LDIGIT_r;
    dsp_fill_decimal_4digit(&lcd_input_buf[2], _dur1);
}

void dsp_inp_scurr_dur2_init(uint16_t _dur2)
{
    disp_input_mode = DISPINP_sCURR_RTN_DUR2;
    digit_pos = 5;
    dispinp_err = 0;

    lcd_input_buf[0] = LDIGIT_b;
    lcd_input_buf[1] = LDIGIT_r;
    dsp_fill_decimal_4digit(&lcd_input_buf[2], _dur2);
}

void dsp_inp_end_init(void)
{
    disp_input_mode = DISPINP_END;

    lcd_input_buf[0] = LDIGIT_SPACE;
    lcd_input_buf[1] = LDIGIT_SPACE;
    lcd_input_buf[2] = LDIGIT_SPACE;
    lcd_input_buf[3] = LDIGIT_E;
    lcd_input_buf[4] = LDIGIT_n;
    lcd_input_buf[5] = LDIGIT_d;
}

static void dsp_fill_decimal_4digit(uint8_t *_buf, uint16_t _val)
{
    *_buf++ = _val / 1000;
    _val %= 1000;
    *_buf++ = _val / 100;
    _val %= 100;
    *_buf++ = _val / 10;
    *_buf = _val % 10;
}

static void dsp_fill_decimal_5digit(uint8_t *_buf, uint16_t _val)
{
    *_buf++ = _val / 10000;
    _val %= 10000;

    dsp_fill_decimal_4digit(_buf, _val);
}

static void dsp_fill_decimal_6digit(uint8_t *_buf, uint32_t _val)
{
    *_buf++ = _val / 100000;
    _val %= 100000;

    *_buf++ = _val / 10000;
    _val %= 10000;

    dsp_fill_decimal_4digit(_buf, (uint16_t)_val);
}

void dsp_inp_digit_pos_move(uint8_t limit, uint8_t initval)
{
    if (digit_pos == limit)
        digit_pos = initval;
    else
        digit_pos--;
}

void dsp_inp_digit_pos_move_tou(ratekind_type rtkind)
{
    if (rtkind > TWO_RATE_KIND)
        return;

    if (digit_pos == 1)
    {
        digit_pos = 5;
    }
    else
    {
        digit_pos--;
        if (rtkind == ONE_RATE_KIND)
        {
            if (digit_pos == 1)
                digit_pos = 5;
        }
    }
}

void dsp_inp_err_set(void) { dispinp_err = 1; }

uint8_t dsp_inp_digit_pos(void) { return digit_pos; }

void dsp_inp_digit_inc(uint8_t mx)
{
    if (++lcd_input_buf[digit_pos] > mx)
        lcd_input_buf[digit_pos] = 0;
}

void dsp_inp_digit_inc_range(uint8_t mn, uint8_t mx)
{
    if (++lcd_input_buf[digit_pos] > mx)
        lcd_input_buf[digit_pos] = mn;
}

void dsp_inp_mtkind_inc(ratekind_type kind)
{
    ++lcd_input_buf[digit_pos];
    if (lcd_input_buf[digit_pos] > 2)
    {
        if (kind <= TWO_RATE_KIND)
        {
            lcd_input_buf[digit_pos] = 1;
        }
        else
        {
            if (lcd_input_buf[digit_pos] < (kind + 1))
                lcd_input_buf[digit_pos] = kind + 1;
            else if (lcd_input_buf[digit_pos] > (kind + 1))
                lcd_input_buf[digit_pos] = 1;
        }
    }
}

void dsp_inp_mtdir_toggle(void)
{
    if (lcd_input_buf[NUM_DIGIT - 2] == LDIGIT_r)
        lcd_input_buf[NUM_DIGIT - 2] = LDIGIT_5;
    else
        lcd_input_buf[NUM_DIGIT - 2] = LDIGIT_r;
}

void dsp_inp_meas_toggle(void)
{
    if (mt_is_uni_dir())
    {
        if (lcd_input_buf[5] == LDIGIT_5)
        {
            // single
            lcd_input_buf[5] = LDIGIT_d;
        }
        else
        {
            // basic
            lcd_input_buf[5] = LDIGIT_5;
        }
    }
}

void dsp_inp_baud_toggle(void)
{
    if (lcd_input_buf[1] == LDIGIT_SPACE)
    {
        lcd_input_buf[1] = LDIGIT_1;
        lcd_input_buf[2] = LDIGIT_9;
        lcd_input_buf[3] = LDIGIT_2;
    }
    else if (lcd_input_buf[1] == LDIGIT_1)
    {
        lcd_input_buf[1] = LDIGIT_3;
        lcd_input_buf[2] = LDIGIT_8;
        lcd_input_buf[3] = LDIGIT_4;
    }
    else
    {
        lcd_input_buf[1] = LDIGIT_SPACE;
        lcd_input_buf[2] = LDIGIT_9;
        lcd_input_buf[3] = LDIGIT_6;
    }
}

void dsp_inp_tourate_toggle(void)
{
    if (lcd_input_buf[1] == LDIGIT_A)
        lcd_input_buf[1] = LDIGIT_b;
    else
        lcd_input_buf[1] = LDIGIT_A;
}

void dsp_inp_ts_toggle(uint8_t pos)
{
    if (lcd_input_buf[pos] == LDIGIT_d)
        lcd_input_buf[pos] = LDIGIT_E;
    else
        lcd_input_buf[pos] = LDIGIT_d;
}

void dsp_inp_errpls_toggle(void)
{
    uint8_t pls;

    if (lcd_input_buf[4] == LDIGIT_P)
    {
        pls = 2;
    }
    else if (lcd_input_buf[4] == LDIGIT_r)
    {
        pls = 0;
    }
    else
    {
        pls = 1;
    }

    switch (pls)
    {
    case 0:  // 피상
        lcd_input_buf[3] = LDIGIT_SPACE;
        lcd_input_buf[4] = LDIGIT_P;
        lcd_input_buf[5] = LDIGIT_t;
        break;
    case 1:  // 무효
        lcd_input_buf[3] = LDIGIT_SPACE;
        lcd_input_buf[4] = LDIGIT_r;
        lcd_input_buf[5] = LDIGIT_t;
        break;
    case 2:  // EOI
        lcd_input_buf[3] = LDIGIT_E;
        lcd_input_buf[4] = LDIGIT_0;
        lcd_input_buf[5] = LDIGIT_I;
        break;
    }
}

void dsp_inp_condensor_toggle(void)
{
    if (lcd_input_buf[5] == LDIGIT_d)
        lcd_input_buf[5] = LDIGIT_E;
    else
        lcd_input_buf[5] = LDIGIT_d;
}

void dsp_inp_commen_toggle(void)
{
    if (lcd_input_buf[5] == LDIGIT_d)
        lcd_input_buf[5] = LDIGIT_E;
    else
        lcd_input_buf[5] = LDIGIT_d;
}

void dsp_inp_ts_time(ts_struct_type *ts)
{
    digit_pos = 5;

    if (ts->on_off == SW_CTRL_ON)
        lcd_input_buf[1] = LDIGIT_E;
    else
        lcd_input_buf[1] = LDIGIT_d;

    lcd_input_buf[2] = ts->hour / 10;
    lcd_input_buf[3] = ts->hour % 10;
    lcd_input_buf[4] = ts->min / 10;
    lcd_input_buf[5] = ts->min % 10;
}

void dsp_inp_tou_rate(rate_type rt)
{
    switch (rt)
    {
    case eArate:
        lcd_input_buf[1] = LDIGIT_A;
        break;
    case eBrate:
        lcd_input_buf[1] = LDIGIT_b;
        break;
    case eCrate:
        lcd_input_buf[1] = LDIGIT_C;
        break;
    case eDrate:
        lcd_input_buf[1] = LDIGIT_d;
        break;
    default:
        break;
    }
}

void dsp_inp_tou_rate_time(tou_struct_type *tou)
{
    digit_pos = 5;

    lcd_input_buf[0] = LDIGIT_SPACE;
    dsp_inp_tou_rate(SELECTOR_TO_RATE(tou->rate));
    lcd_input_buf[2] = tou->hour / 10;
    lcd_input_buf[3] = tou->hour % 10;
    lcd_input_buf[4] = tou->min / 10;
    lcd_input_buf[5] = tou->min % 10;
}

void dsp_inp_sig_sel(ad_sig_type sg)
{
    switch (sg)
    {
    case SIG_NOSEL:
        lcd_input_buf[4] = LDIGIT_n;
        lcd_input_buf[5] = LDIGIT_0;
        break;
    case SIG_TS_CONTROL:
        lcd_input_buf[4] = LDIGIT_t;
        lcd_input_buf[5] = LDIGIT_5;
        break;
    case SIG_rLOAD_CONTROL:
        lcd_input_buf[4] = LDIGIT_r;
        lcd_input_buf[5] = LDIGIT_L;
        break;
    case SIG_sCURR_LIMIT:
        lcd_input_buf[4] = LDIGIT_C;
        lcd_input_buf[5] = LDIGIT_L;
        break;
    }
}

uint8_t get_dispinp_year(void)
{
    return dsp_calc_digit(lcd_input_buf[0], lcd_input_buf[1]);
}

uint8_t get_dispinp_month(void)
{
    return dsp_calc_digit(lcd_input_buf[2], lcd_input_buf[3]);
}

uint8_t get_dispinp_date(void)
{
    return dsp_calc_digit(lcd_input_buf[4], lcd_input_buf[5]);
}

uint8_t get_dispinp_hour(void)
{
    return dsp_calc_digit(lcd_input_buf[0], lcd_input_buf[1]);
}

uint8_t get_dispinp_min(void)
{
    return dsp_calc_digit(lcd_input_buf[2], lcd_input_buf[3]);
}

uint8_t get_dispinp_ts_ctrl(void)
{
    return (lcd_input_buf[1] == LDIGIT_E) ? SW_CTRL_ON : SW_CTRL_OFF;
}

uint8_t get_dispinp_ratekind(void) { return lcd_input_buf[5]; }

uint8_t get_dispinp_ts_tourate(void)
{
    rate_type rt;

    rt = (lcd_input_buf[1] == LDIGIT_A) ? eArate : eBrate;

    return RATE_TS_TO_SELECTOR(rt, ts);
}

uint8_t get_dispinp_tou_hour(void)
{
    return dsp_calc_digit(lcd_input_buf[2], lcd_input_buf[3]);
}

uint8_t get_dispinp_tou_min(void)
{
    return dsp_calc_digit(lcd_input_buf[4], lcd_input_buf[5]);
}

uint16_t get_dispinp_sCurr(void)
{
    uint16_t t16;

    t16 = dsp_calc_3digit(lcd_input_buf[1], lcd_input_buf[2], lcd_input_buf[3]);
    t16 *= 100;
    t16 += dsp_calc_digit(lcd_input_buf[4], lcd_input_buf[5]);

    return t16;
}

uint8_t get_dispinp_sCurrCnt(void)
{
    return dsp_calc_digit(lcd_input_buf[4], lcd_input_buf[5]);
}

uint8_t get_dispinp_lpintv(void)
{
    return dsp_calc_digit(lcd_input_buf[4], lcd_input_buf[5]);
}

uint8_t get_dispinp_mtdir(void)
{
    if (lcd_input_buf[NUM_DIGIT - 2] == LDIGIT_r)
        return MT_UNI_DIR;

    return MT_BOTH_DIR;
}

meas_method_type get_dispinp_meas(void)
{
    if (lcd_input_buf[5] == LDIGIT_5)
        return E_BASIC;

    return E_SINGLE_DIR;
}

baudrate_type get_dispinp_baud(void)
{
    if (lcd_input_buf[1] == LDIGIT_SPACE)
        return BAUD_9600;

    if (lcd_input_buf[1] == LDIGIT_1)
        return BAUD_19200;

    return BAUD_38400;
}

int8_t get_dispinp_temp(void)
{
    return (int8_t)dsp_calc_digit(lcd_input_buf[4], lcd_input_buf[5]);
}

uint8_t get_dispinp_errpls(void)
{
    if (lcd_input_buf[4] == LDIGIT_P)
        return 0;
    if (lcd_input_buf[4] == LDIGIT_r)
        return 1;

    return 2;
}

uint8_t get_dispinp_condensor(void)
{
    return (lcd_input_buf[5] == LDIGIT_E) ? 1 : 0;
}

uint8_t get_dispinp_commen(void)
{
    return (lcd_input_buf[5] == LDIGIT_E) ? 1 : 0;
}

uint16_t get_dispinp_scurr_dur(void)
{
    uint16_t t16;

    t16 = dsp_calc_digit(lcd_input_buf[2], lcd_input_buf[3]);
    t16 *= 100;
    t16 += dsp_calc_digit(lcd_input_buf[4], lcd_input_buf[5]);

    return t16;
}

uint8_t get_dispinp_scurr_n1(void)
{
    return dsp_calc_digit(lcd_input_buf[4], lcd_input_buf[5]);
}

uint32_t get_dispinp_contract_month(void)
{
    uint32_t t32;

    t32 = (dsp_calc_digit(lcd_input_buf[0], lcd_input_buf[1]) * 10000);
    t32 += (dsp_calc_digit(lcd_input_buf[2], lcd_input_buf[3]) * 100);
    t32 += dsp_calc_digit(lcd_input_buf[4], lcd_input_buf[5]);

    return t32;
}
#endif

extern bool pulse_dir;
bool is_pls_inc_dir(void) { return pulse_dir; }

void dsp_DR_set(void)
{
    dr_dsp = true;
    dsp_dr_timeset(T3SEC);
}

void dsp_comm_is_ing_set(void)
{
    dsp_comm_ing = true;
    dsp_comm_is_ing_timeset(T2SEC);
}

void dsp_cal_mode_is_ing_set(void)
{
    // LCD 전류 표시 상태 혹은 cal 종료상태인 경우 return
    if (dsp_cal_mode_ing || dsp_cal_mode_end)
    {
        MSG09("<return> push, cal_mode_is_ing, mode:%d, end:%d",
              dsp_cal_mode_ing, dsp_cal_mode_end);
        return;
    }

    // LCD cal 시작상태가 아니면 return
    if (!dsp_cal_st_ing)
    {
        MSG09("<return> push, cal_mode_is_ing, st:%d", dsp_cal_st_ing);
        return;
    }

    MSG09("push, cal_mode_is_ing, false -> true");

    // LCD cal 시작 표시 --> false
    dsp_cal_st_ing = false;

    // LCD 전류 표시 시작
    dsp_cal_mode_ing = true;
}

void dsp_cal_st_is_ing_set(void)
{
    // CAL 진행중에는 새로운 시작이 불가능
    if (dsp_cal_st_ing || dsp_cal_mode_ing || dsp_cal_mode_end)
    {
        MSG09("<return> push, cal_st_is_ing, st:%d, mode:%d, end:%d",
              dsp_cal_st_ing, dsp_cal_mode_ing, dsp_cal_mode_end);
        return;
    }

    MSG09("push, st_is_ing_set, false -> true", dsp_cal_st_ing);

    // LCD cal 시작 표시
    dsp_cal_st_ing = true;
}

void dsp_r_sun_dsp_set(void)
{
    r_sun_dsp = true;
    dsp_r_sun_timeset(T3SEC);
}

void dsp_on_sun_dsp_set(void)
{
    on_sun_dsp = true;
    DPRINTF(DBG_ERR, "%s on_sun_dsp[%d]\r\n", __func__, on_sun_dsp);

    /* 무전압 설정 수행 : LCD에 On-Sun 3초간 표시 후, 무전압 검침 모드로
     * 순환(총 5분) */
    dsp_on_sun_timeset(T3SEC);
}

void dsp_CalFail_set(void) { disp_state = DISP_CALFAIL_STATE; }

void dsp_calfail_state_exit(void) { dsp_circ_state_init(); }

void dsp_key_touched(void)
{
#if 0 /* bccho, 2024-01-10, 1227 포팅 */    
    if (dsp_is_input_state() && !dsp_is_inp_end())
        dsp_instate_timeset(T30SEC);
#endif

    dsp_mdexit_timeset(T30SEC);
    ts_mdexit_timeset(T240SEC);
}

static uint16_t dsp_calc_3digit(uint8_t hund, uint8_t ten, uint8_t one)
{
    return (hund * 100 + ten * 10 + one);
}

static uint8_t dsp_calc_digit(uint8_t ten, uint8_t one)
{
    return (ten * 10 + one);
}

void dsp_digit(int32_t val, uint8_t dgt, uint8_t point, bool lead_zero)
{
    uint8_t i;

    dsp_point_pos = point;

    if (val < 0)
    {
        dsp_dot_minus = true;
        val *= (-1);
    }
    else
    {
        dsp_dot_minus = false;
    }

    memset(&lcd_digit_buf[2], 0, NUM_DIGIT);

    for (i = 0; i < NUM_DIGIT; i++)
    {
        lcd_digit_buf[NUM_MODE_DIGIT - 1 - i] = val % 10;
        val /= 10;
        if (val == 0)
            break;
    }

    if (val == 0)
    {
        for (i = 2; i < NUM_MODE_DIGIT - dgt; i++)
        {
            lcd_digit_buf[i] = LDIGIT_SPACE;
        }

        for (; i < NUM_MODE_DIGIT - point - 1; i++)
        {
            if (lcd_digit_buf[i] != 0)
                break;

            if (lead_zero)
            {
                lcd_digit_buf[i] = LDIGIT_0;
            }
            else
            {
                lcd_digit_buf[i] = LDIGIT_SPACE;
            }
        }

        if (!lead_zero && dsp_dot_minus)
        {
            if (i > 2)
            {
                lcd_digit_buf[i - 1] = LDIGIT_BAR;
            }
        }
    }
    else
    {
        memset(&lcd_digit_buf[2], LDIGIT_BAR, NUM_DIGIT);
    }
}

extern int32_t apulsew_mon;
extern date_time_type bak_dt;
extern date_time_type lp_dt;

static void dsp_drive_digit(void)
{
    uint8_t tbuf[NUM_MODE_DIGIT];

    if (dsp_off_blank)
    {
        if (dsp_fill_error_item(tbuf) == false)
        {
            memset(tbuf, LDIGIT_SPACE, NUM_MODE_DIGIT);
            LCD_DERR_OFF;
        }
        else
            LCD_DERR_ON;
    }
    else
    {
        memcpy(tbuf, &lcd_digit_buf[0], NUM_MODE_DIGIT);
    }

    dsp_fill_lcd_map(tbuf);

    dsp_fill_lcd_map_comm(tbuf);
}

static void dsp_drive_dot(void)
{
    dsp_dot_unit();
    dsp_dot_point();

    if (run_is_bat_power())
        return;

    dsp_dot_secret();
    dsp_dot_batmem();

    // energy direction and pulse increase
    // 부하 : 상한 및 회전 표시
    dsp_dot_pls_quart();

    lcd_ASSOCIATED_disp();

    /* bccho, 2024-09-05, 삼상 */
    lcd_SUN_disp();

    // Test state
    dsp_dot_test();

    // CL dot (current limit)
    dsp_dot_CL();

    // A B C D
    dsp_dot_rate();

    // Load and OX
    dsp_dot_load();

    // Voltage phase
    dsp_dot_phase();

    dsp_dot_err();
}

void dsp_lcdmem_move(void)
{
#if 1 /* bccho, LCD, 2023-08-04 */
    uint8_t seg_com[24][4];
    for (int i = 0; i < 24; i++) /* segment 24개 */
    {
        for (int j = 0; j < 4; j++) /* com 4개 */
        {
            seg_com[i][j] = (st_lcd_ram.com_xx_s31_00[j] >> i) & 0x01;
        }
    }

    uint32_t lcd_data[6];
    for (int i = 0, j = 0; j < 6; i += 4, j++)
    {
        lcd_data[j] = (((seg_com[i][3] << 3) | (seg_com[i][2] << 2) |
                        (seg_com[i][1] << 1) | (seg_com[i][0] << 0))
                       << 0) | /* segment n */
                      (((seg_com[i + 1][3] << 3) | (seg_com[i + 1][2] << 2) |
                        (seg_com[i + 1][1] << 1) | (seg_com[i + 1][0] << 0))
                       << 8) | /* segment n+1 */
                      (((seg_com[i + 2][3] << 3) | (seg_com[i + 2][2] << 2) |
                        (seg_com[i + 2][1] << 1) | (seg_com[i + 2][0] << 0))
                       << 16) | /* segment n+2 */
                      (((seg_com[i + 3][3] << 3) | (seg_com[i + 3][2] << 2) |
                        (seg_com[i + 3][1] << 1) | (seg_com[i + 3][0] << 0))
                       << 24); /* segment n+3 */
    }

    for (int j = 0; j < 6; j++) /* segment 4개씩 6개 */
    {
        LCD->DATA[j] = lcd_data[j];
    }
#else
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER0, 0xFFFFFFFF, /* COM0, SEQ[00-31] */
                  st_lcd_ram.com_xx_s31_00[0]);
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER1, 0xFFFFFFFF, /* COM0, SEQ[32-44] */
                  st_lcd_ram.com_xx_s43_32[0]);

    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER2, 0xFFFFFFFF, /* COM1, SEQ[00-31]  */
                  st_lcd_ram.com_xx_s31_00[1]);
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER3, 0xFFFFFFFF, /* COM1, SEQ[32-44] */
                  st_lcd_ram.com_xx_s43_32[1]);

    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER4, 0xFFFFFFFF, /* COM2, SEQ[00-31]  */
                  st_lcd_ram.com_xx_s31_00[2]);
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER5, 0xFFFFFFFF, /* COM2, SEQ[32-44] */
                  st_lcd_ram.com_xx_s43_32[2]);

    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER6, 0xFFFFFFFF, /* COM3, SEQ[00-31]  */
                  st_lcd_ram.com_xx_s31_00[3]);
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER7, 0xFFFFFFFF, /* COM3, SEQ[32-44] */
                  st_lcd_ram.com_xx_s43_32[3]);

    HAL_LCD_UpdateDisplayRequest(&hlcd);
#endif /* bccho */
}

static void dsp_dot_err(void)
{
    if ((WMStatus & ERR_MASK) || (WMStatus_intern & ERR_MASK_2) ||
        (IS_MorTCOVER_OPEN || IS_MorTCOVER_OPEN_THISMONTH) ||
        dsp_is_error_item())
    {
        LCD_DERR_ON;
    }
}

static void dsp_dot_secret(void)
{
    if (IS_MorTCOVER_OPEN || IS_MorTCOVER_OPEN_THISMONTH)
    {
        if (blink_timer & 0x01)
            LCD_DSECRET_ON;
        else
            LCD_DSECRET_OFF;
    }
    else
    {
        LCD_DSECRET_OFF;
    }
}

void lcd_ASSOCIATED_disp(void)
{
    if ((comm_is_connected()) &&
        (appl_get_conn_state() >= APPL_ASSOCIATED_STATE))
    {
#if 1 /* bccho, 2024-09-05, 삼상 */
        if ((dsp_comm_ing) && (blink_timer & 0x01))
            LCD_DCONN_OFF;
        else
            LCD_DCONN_ON;
#else
        switch (dsm_media_get_fsm_if_hdlc())
        {
        case MEDIA_RUN_RS485:
        case MEDIA_RUN_CAN:
        case MEDIA_RUN_EXT:
            if ((dsp_comm_ing) && (blink_timer & 0x01))
                LCD_DCONN_OFF;
            else
                LCD_DCONN_ON;
            break;
        case MEDIA_RUN_SUN:
            if ((dsp_comm_ing) && (blink_timer & 0x01))
                LCD_DSUN_COMM_OFF;
            else
                LCD_DSUN_COMM_ON;
            break;
        default:
            LCD_DCONN_OFF;
            LCD_DSUN_COMM_OFF;
            break;
        }
#endif
    }
    else
    {
        LCD_DCONN_OFF;
#if 0 /* JPKIM, 2024-09-30 */
        LCD_DSUN_COMM_OFF;
#endif
    }
}

/* bccho, 2024-09-05, 삼상 */
static void lcd_SUN_disp(void)
{
    if (g_mdlink_mode == SUN_CORDI)
    {
#if 0 /* JPKIM, 2024-09-30 */
			if((dsp_comm_ing)&&(blink_timer & 0x01)) LCD_DSUN_COMM_OFF;
#else
        if (blink_timer & 0x01)
            LCD_DSUN_COMM_OFF;
#endif
        else
            LCD_DSUN_COMM_ON;
    }
    else if (g_mdlink_mode == SUN_DEVICE_ON_ASSO)
    {
        LCD_DSUN_COMM_ON;
    }
    else
    {
        LCD_DSUN_COMM_OFF;
    }
}

static void dsp_dot_test(void)
{
    // jp.kim 24.11.08  LCD창 "TEST" 표시 ON
    if (dsp_is_test_state() || test_lcd_on_sts)
    {
        LCD_DTEST_ON;
    }
    else
    {
        LCD_DTEST_OFF;
    }
}

static void dsp_dot_CL(void)
{
    if (scurr_is_limiting_forever())
    {
        if (blink_timer & 0x01)
            LCD_DCL_ON;
        else
            LCD_DCL_OFF;
    }
    else if (scurr_is_limiting())
    {
        LCD_DCL_ON;
    }
    else
    {
        LCD_DCL_OFF;
    }
}

static void dsp_dot_point(void)
{
    uint8_t pos;

    if (dsp_off_blank)
        pos = 0;
    else
        pos = dsp_point_pos;
    switch (pos)
    {
    case 0:
        LCD_DP4_OFF;  // P1 (소수 1자리)
        LCD_DP3_OFF;  // P2 (소수 2자리)
        LCD_DP2_OFF;  // P3 (소수 3자리)
        LCD_DP1_OFF;  // P4 (소수 4자리)
        break;
    case 1:
        LCD_DP4_ON;   // P1 (소수 1자리)
        LCD_DP3_OFF;  // P2 (소수 2자리)
        LCD_DP2_OFF;  // P3 (소수 3자리)
        LCD_DP1_OFF;  // P4 (소수 4자리)
        break;
    case 2:
        LCD_DP4_OFF;  // P1 (소수 1자리)
        LCD_DP3_ON;   // P2 (소수 2자리)
        LCD_DP2_OFF;  // P3 (소수 3자리)
        LCD_DP1_OFF;  // P4 (소수 4자리)
        break;
    case 3:
        LCD_DP4_OFF;  // P1 (소수 1자리)
        LCD_DP3_OFF;  // P2 (소수 2자리)
        LCD_DP2_ON;   // P3 (소수 3자리)
        LCD_DP1_OFF;  // P4 (소수 4자리)
        break;
    case 4:
        LCD_DP4_OFF;  // P1 (소수 1자리)
        LCD_DP3_OFF;  // P2 (소수 2자리)
        LCD_DP2_OFF;  // P3 (소수 3자리)
        LCD_DP1_ON;   // P4 (소수 4자리)
        break;
    case 5:
        LCD_DP4_OFF;  // P1 (소수 1자리)
        LCD_DP3_ON;   // P2 (소수 2자리)
        LCD_DP2_OFF;  // P3 (소수 3자리)
        LCD_DP1_ON;   // P4 (소수 4자리)
        break;
    }
}

static void dsp_dot_batmem(void)
{
    if ((WMStatus & ERR_MASK) || (WMStatus_intern & ERR_MASK_2))
    {
        if ((WMStatus & GE_NOBAT) || (WMStatus_intern & GE_LOWBAT))
        {
            if (blink_timer & 0x02)
            {
                LCD_DBAT_ON;
            }
            else
            {
                LCD_DBAT_OFF;
            }
        }
        else
        {
            LCD_DBAT_OFF;
        }
    }
    else
    {
        LCD_DBAT_OFF;
    }
}

static void dsp_dot_load(void)
{
    /* LOAD O / X */

    if (dsp_load_dot_is_timeout())
    {
        LCD_DLOAD_ON;
        if (relay_is_load_on())
        {
            LCD_DLOADX_OFF;
            if (!curr_is_noload())
            {
                LCD_DLOADO_ON;
            }
            else
            {
                if (blink_timer & 0x01)
                    LCD_DLOADO_ON;
                else
                    LCD_DLOADO_OFF;
            }
        }
        else
        {
            LCD_DLOADO_OFF;
            if (curr_is_noload())
            {
                LCD_DLOADX_ON;
            }
            else
            {
                if (blink_timer & 0x01)
                    LCD_DLOADX_ON;
                else
                    LCD_DLOADX_OFF;
            }
        }
    }
    else
    {
        if (blink_timer & 0x01)
        {
            LCD_DLOAD_ON;
            if (relay_is_load_on())
            {
                LCD_DLOADO_ON;
                LCD_DLOADX_OFF;
            }
            else
            {
                LCD_DLOADO_OFF;
                LCD_DLOADX_ON;
            }
        }
        else
        {
            LCD_DLOAD_OFF;
            LCD_DLOADO_OFF;
            LCD_DLOADX_OFF;
        }
    }
}

static void dsp_dot_phase(void)
{
    /* bccho, 2024-09-05, 삼상 */
#if PHASE_NUM == SINGLE_PHASE
    LCD_DV1_OFF;  // V1
    LCD_DV2_OFF;
    LCD_DV3_OFF;
#else
    if (neut_line_is_wrong())
    {
        if (blink_timer & 0x01)
        {
            LCD_DV1_ON;
            LCD_DV2_ON;
            LCD_DV3_ON;
        }
        else
        {
            LCD_DV1_OFF;
            LCD_DV2_OFF;
            LCD_DV3_OFF;
        }
    }
    else
    {
        if (WMStatus & SAGVA && !(blink_timer & 0x01))  // V1
            LCD_DV1_OFF;
        else
            LCD_DV1_ON;

        if (WMStatus & SAGVB && !(blink_timer & 0x01))  // V2
            LCD_DV2_OFF;
        else
            LCD_DV2_ON;

        if (WMStatus & SAGVC && !(blink_timer & 0x01))  // V3
            LCD_DV3_OFF;
        else
            LCD_DV3_ON;
    }
#endif
}

static void dsp_dot_unit(void)
{
    if (dsp_off_blank)
        return;

    switch (dsp_unit)
    {
    case eUnitClock1:
        LCD_DCOLON1_ON;
        LCD_DCOLON2_ON;
        break;
    case eUnitClock2:
        LCD_DCOLON1_OFF;
        LCD_DCOLON2_ON;
        break;
    case eUnitV:
        LCD_DUNIT_V2_ON;
        break;
    case eUnitA:
        LCD_DUNIT_A_ON;
        break;
    case eUnitPercent:
        LCD_DPERCENT_ON;
        break;
    case eUnitKw:
        LCD_DUNIT_k_ON;
        LCD_DUNIT_V1_ON;
        LCD_DUNIT_V2_ON;
        break;
    case eUnitKwh:
        LCD_DUNIT_k_ON;
        LCD_DUNIT_V1_ON;
        LCD_DUNIT_V2_ON;
        LCD_DUNIT_h_ON;
        break;
    case eUnitKvar:
        LCD_DUNIT_k_ON;
        LCD_DUNIT_V2_ON;
        LCD_DUNIT_A_ON;
        LCD_DUNIT_r_ON;
        break;
    case eUnitKvarh:
        LCD_DUNIT_k_ON;
        LCD_DUNIT_V2_ON;
        LCD_DUNIT_A_ON;
        LCD_DUNIT_r_ON;
        LCD_DUNIT_h_ON;
        break;
    case eUnitKva:
        LCD_DUNIT_k_ON;
        LCD_DUNIT_V2_ON;
        LCD_DUNIT_A_ON;
        break;
    case eUnitKvah:
        LCD_DUNIT_k_ON;
        LCD_DUNIT_V2_ON;
        LCD_DUNIT_A_ON;
        LCD_DUNIT_h_ON;
        break;
    case eUnitHz:
        LCD_DUNIT_h_ON;
        LCD_DUNIT_z_ON;
        break;
    case eUnitNull:
        break;
    default:
        break;
    }
}

static void dsp_dot_pls_quart(void)
{
    uint8_t t8;
    vi_quarter_type quart;

    if (circdsp_is_pvt_mode())
    {
        if (pls_inc_is_timeout())
        {
            if (++dsp_pulse_inc >= 5)
            {
                dsp_pulse_inc = 0;
            }
            pls_inc_timeset(T500MS);
        }
        goto dsp_dot_pls_quart1;
    }
    if (curr_is_noload())
    {
        /* 무부하 */
        t8 = 5;
        dsp_pulse_inc = 0;
    }
    else
    {
    dsp_dot_pls_quart1:
        if (is_pls_inc_dir() || circdsp_is_pvt_mode())
        {
            // 정방향
            t8 = dsp_pulse_inc;
        }
        else
        {
            // 역방향
            t8 = dsp_pulse_inc;
            t8 = t8 ? (5 - t8) : 0;
        }
    }

    /*
        회전 표시자 (부하 동작 표시) :
            반시계방향 회전 시, 송전
            시계방향 회전 시, 수전
    */
    switch (t8)
    {
    case 0: /* S1 */
        LCD_DPLS1_OFF;
        LCD_DPLS2_ON;
        LCD_DPLS3_ON;
        LCD_DPLS4_ON;
        LCD_DPLS5_ON;
        break;
    case 1: /* S2 */
        LCD_DPLS1_ON;
        LCD_DPLS2_OFF;
        LCD_DPLS3_ON;
        LCD_DPLS4_ON;
        LCD_DPLS5_ON;
        break;
    case 2: /* S3 */
        LCD_DPLS1_ON;
        LCD_DPLS2_ON;
        LCD_DPLS3_OFF;
        LCD_DPLS4_ON;
        LCD_DPLS5_ON;
        break;
    case 3: /* S4 */
        LCD_DPLS1_ON;
        LCD_DPLS2_ON;
        LCD_DPLS3_ON;
        LCD_DPLS4_OFF;
        LCD_DPLS5_ON;
        break;
    case 4: /* S5 */
        LCD_DPLS1_ON;
        LCD_DPLS2_ON;
        LCD_DPLS3_ON;
        LCD_DPLS4_ON;
        LCD_DPLS5_OFF;
        break;
    case 5: /* 무부하 */
        LCD_DPLS1_ON;
        LCD_DPLS2_ON;
        LCD_DPLS3_ON;
        LCD_DPLS4_ON;
        LCD_DPLS5_ON;
        break;
    }

    if (circdsp_is_pvt_mode())
    {
        quart = eVIq1;
        goto dsp_dot_pls_quart2;
    }

    if (curr_is_noload())
    {
        LCD_DQUP_OFF;
        LCD_DQDOWN_OFF;
        LCD_DQLEFT_OFF;
        LCD_DQRIGHT_OFF;
    }
    else
    {
        quart = get_vi_quarter();
    dsp_dot_pls_quart2:

        /*
        상한 표시자 (동작 상한 표시 (송/수전)) :
            1상한 수전유효, 지상무효 (▲ ▶)
            2상한 송전유효, 진상무효 (◀ ▲)
            3상한 송전유효, 지상무효 (◀ ▼)
            4상한 수전유효, 진상무효 (▼ ▶)
        */
        switch (quart)
        {
        case eVIq1:
            LCD_DQUP_ON;
            LCD_DQDOWN_OFF;
            LCD_DQLEFT_OFF;
            LCD_DQRIGHT_ON;
            break;
        case eVIq2:
            LCD_DQUP_ON;
            LCD_DQDOWN_OFF;
            LCD_DQLEFT_ON;
            LCD_DQRIGHT_OFF;
            break;
        case eVIq3:
            LCD_DQUP_OFF;
            LCD_DQDOWN_ON;
            LCD_DQLEFT_ON;
            LCD_DQRIGHT_OFF;
            break;
        case eVIq4:
            LCD_DQUP_OFF;
            LCD_DQDOWN_ON;
            LCD_DQLEFT_OFF;
            LCD_DQRIGHT_ON;
            break;
        default:
            break;
        }
    }
}

static void dsp_dot_rate(void)
{
    ratekind_type mtkind;
    rate_type rt;

    // A-type: rate ( bit3=D, bit2=C, bit1=B, bit0=A )
    mtkind = mt_rtkind;
    switch (mtkind)
    {
    case ONE_RATE_KIND:
        LCD_DRATE_A_ON;
        break;
    case TWO_RATE_KIND:
        LCD_DRATE_A_ON;
        LCD_DRATE_B_ON;
        break;
    case THREE_RATE_KIND:
        LCD_DRATE_A_ON;
        LCD_DRATE_B_ON;
        LCD_DRATE_C_ON;
        break;
    case FOUR_RATE_KIND:
        LCD_DRATE_A_ON;
        LCD_DRATE_B_ON;
        LCD_DRATE_C_ON;
        LCD_DRATE_D_ON;
        break;
    default:
        break;
    }
    if (mtkind != ONE_RATE_KIND && !(blink_timer & 0x01))
    {
        rt = cur_rate;
        switch (rt)
        {
        case eArate:
            LCD_DRATE_A_OFF;
            break;
        case eBrate:
            LCD_DRATE_B_OFF;
            break;
        case eCrate:
            LCD_DRATE_C_OFF;
            break;
        case eDrate:
            LCD_DRATE_D_OFF;
            break;
        default:
            break;
        }
    }
}

void dsp_alloff(void) {}

void dsp_allon(void) {}

void dsp_pwr_on(void)
{
    static int pwrfail_isr_cnt = 0;

    pwrfail_isr_cnt++;

    lcd_digit_buf[0] = LDIGIT_SPACE;
    lcd_digit_buf[1] = LDIGIT_SPACE;
    lcd_digit_buf[2] = LDIGIT_SPACE;
    lcd_digit_buf[3] = LDIGIT_SPACE;
    lcd_digit_buf[4] = LDIGIT_SPACE;
    lcd_digit_buf[5] = LDIGIT_SPACE;
    lcd_digit_buf[6] = LDIGIT_o;
    lcd_digit_buf[7] = LDIGIT_n;

    dsm_lcd_dsp_lcdmem_clear();
    dsp_fill_lcd_map(lcd_digit_buf);
    dsm_dsp_lcdmem_move();

    dsp_lcd_map_comm_clear();
    dsp_fill_lcd_map_comm(lcd_digit_buf);
}

void dsp_pwr_fail(void)
{
    lcd_digit_buf[0] = LDIGIT_SPACE;
    lcd_digit_buf[1] = LDIGIT_SPACE;
    lcd_digit_buf[2] = LDIGIT_L;
    lcd_digit_buf[3] = LDIGIT_o;
    lcd_digit_buf[4] = LDIGIT_BAR;
    lcd_digit_buf[5] = LDIGIT_u;
    lcd_digit_buf[6] = LDIGIT_o;
    lcd_digit_buf[7] = LDIGIT_L;

    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    dsp_drive_digit();
    dsp_lcdmem_move();
}

void dsp_for_dbg(int mark)
{
    lcd_digit_buf[0] = LDIGIT_SPACE;
    lcd_digit_buf[1] = LDIGIT_SPACE;
    lcd_digit_buf[2] = LDIGIT_SPACE;
    lcd_digit_buf[3] = LDIGIT_SPACE;
    lcd_digit_buf[4] = LDIGIT_SPACE;
    lcd_digit_buf[5] = mark / 100;
    mark %= 100;
    lcd_digit_buf[6] = mark / 10;
    lcd_digit_buf[7] = mark % 10;

    dsm_lcd_dsp_lcdmem_clear();
    dsp_fill_lcd_map(lcd_digit_buf);
    dsm_dsp_lcdmem_move();
}

static void dsp_lcd_map_comm_clear(void)
{
    int i;

    for (i = 0; i < LCD_MAP_SIZE; i++)
    {
        lcd_map_comm[i] = 0;
    }
}

static void dsp_fill_lcd_map_comm(uint8_t* tbuf) {}

void dsp_fill_lcd_dot_comm(void)
{
    if (LCD_DQUP_IS_ON)
        lcd_map_comm[8] |= 0x80;
    if (LCD_DQLEFT_IS_ON)
        lcd_map_comm[8] |= 0x40;
    if (LCD_DQDOWN_IS_ON)
        lcd_map_comm[8] |= 0x20;
    if (LCD_DQRIGHT_IS_ON)
        lcd_map_comm[8] |= 0x10;

    if (LCD_DPLS1_IS_ON)
        lcd_map_comm[8] |= 0x08;
    if (LCD_DPLS2_IS_ON)
        lcd_map_comm[8] |= 0x04;
    if (LCD_DPLS3_IS_ON)
        lcd_map_comm[8] |= 0x02;
    if (LCD_DPLS4_IS_ON)
        lcd_map_comm[8] |= 0x01;
    if (LCD_DPLS5_IS_ON)
        lcd_map_comm[9] |= 0x80;

    if (LCD_DRATEA_IS_ON)
        lcd_map_comm[9] |= 0x40;
    if (LCD_DRATEB_IS_ON)
        lcd_map_comm[9] |= 0x20;
    if (LCD_DRATEC_IS_ON)
        lcd_map_comm[9] |= 0x10;
    if (LCD_DRATED_IS_ON)
        lcd_map_comm[9] |= 0x08;

    if (LCD_DTEST_IS_ON)
        lcd_map_comm[9] |= 0x04;

    if (LCD_DLOAD_IS_ON)
        lcd_map_comm[9] |= 0x02;
    if (LCD_DLOADX_IS_ON)
        lcd_map_comm[9] |= 0x01;
    if (LCD_DLOADO_IS_ON)
        lcd_map_comm[10] |= 0x80;

    if (LCD_DV1_IS_ON)
        lcd_map_comm[10] |= 0x40;
    if (LCD_DV2_IS_ON)
        lcd_map_comm[10] |= 0x20;
    if (LCD_DV3_IS_ON)
        lcd_map_comm[10] |= 0x10;

    if (LCD_DUNITPERC_IS_ON)
        lcd_map_comm[10] |= 0x08;
    if (LCD_DUNITK_IS_ON)
        lcd_map_comm[10] |= 0x04;
    if (LCD_DUNITV1_IS_ON)
        lcd_map_comm[10] |= 0x02;
    if (LCD_DUNITV2_IS_ON)
        lcd_map_comm[10] |= 0x01;
    if (LCD_DUNITA_IS_ON)
        lcd_map_comm[11] |= 0x80;
    if (LCD_DUNITr_IS_ON)
        lcd_map_comm[11] |= 0x40;
    if (LCD_DUNITh_IS_ON)
        lcd_map_comm[11] |= 0x20;
    if (LCD_DUNITz_IS_ON)
        lcd_map_comm[11] |= 0x10;

    if (LCD_DSECRET_IS_ON)
        lcd_map_comm[11] |= 0x08;
    if (LCD_DCONN_IS_ON)
        lcd_map_comm[11] |= 0x04;
    if (LCD_DBAT_IS_ON)
        lcd_map_comm[11] |= 0x02;
    if (LCD_DMEM_IS_ON)
        lcd_map_comm[11] |= 0x01;
    if (LCD_DERR_IS_ON)
        lcd_map_comm[12] |= 0x80;
    if (LCD_DCL_IS_ON)
        lcd_map_comm[12] |= 0x40;

    if (LCD_DCOLON1_IS_ON)
        lcd_map_comm[12] |= 0x20;
    if (LCD_DCOLON2_IS_ON)
        lcd_map_comm[12] |= 0x10;

    if (LCD_DP4_IS_ON)
        lcd_map_comm[12] |= 0x08;
    if (LCD_DP3_IS_ON)
        lcd_map_comm[12] |= 0x04;
    if (LCD_DP2_IS_ON)
        lcd_map_comm[12] |= 0x02;
    if (LCD_DP1_IS_ON)
        lcd_map_comm[12] |= 0x01;
}

bool dsp_is_suppmode_and_available(void)
{
    if (lcd_is_supp_dspmode() && dsp_is_suppdsp_available())
        return true;

    return false;
}

uint32_t dsp_get_seg_info(uint32_t type, uint32_t digit_pos,
                          uint32_t abcd_or_egfx)
{
    if (type == DSP_GET_V)
    {
        return DSP_SETTING_TABLE[digit_pos][abcd_or_egfx][0];
    }
    else
    {
        return DSP_SETTING_TABLE[digit_pos][abcd_or_egfx][1];
    }
}

uint32_t* dsp_get_ram_position(uint32_t com, uint32_t dsp_lram_addr_type)
{
    if (dsp_lram_addr_type == LRAM_L_ADDR)
        return &st_lcd_ram.com_xx_s31_00[com];
    else
        return &st_lcd_ram.com_xx_s43_32[com];
}

/* bccho, 주석추가, 2023-08-04, 8개 digit을 LCD 레지스터에 적재 준비 */
static void dsp_fill_lcd_map(const uint8_t* dgt)
{
    uint32_t* p_ram_abcd;
    uint32_t addr_info;
    uint32_t set_value;

    /* bccho, 주석추가, 2023-08-04, 숫자 8개 */
    for (int idx = 0; idx < 8; idx++)
    {
        /* bccho, 주석추가, 2023-08-04, 숫자 오른쪽 4개. 0(0xFA)은 0xF */
        uint8_t seg_abcd = digit_font_b[dgt[idx]] >> 4;

        /* bccho, 주석추가, 2023-08-04, 숫자 왼쪽 3개. 0(0xFA)은 0xA */
        uint8_t seg_egfx = digit_font_b[dgt[idx]] & 0x0f;

        int32_t com_idx = 3;

        while (com_idx >= 0)
        {
            if ((seg_abcd >> com_idx) & 0x01)
            {
                /* bccho, 주석추가, 2023-08-04, 주소가 Lo 또는 Hi */
                addr_info = dsp_get_seg_info(DSP_GET_ADDR_INFO, idx, FONT_ABCD);

                /* bccho, 주석추가, 2023-08-04, 설정할 레지스터 */
                set_value = dsp_get_seg_info(DSP_GET_V, idx, FONT_ABCD);

                p_ram_abcd = dsp_get_ram_position(com_idx, addr_info);
                *p_ram_abcd |= set_value;
            }

            if ((seg_egfx >> com_idx) & 0x01)
            {
                addr_info = dsp_get_seg_info(DSP_GET_ADDR_INFO, idx, FONT_EGFx);
                set_value = dsp_get_seg_info(DSP_GET_V, idx, FONT_EGFx);
                p_ram_abcd = dsp_get_ram_position(com_idx, addr_info);
                *p_ram_abcd |= set_value;
            }

            com_idx--;
        }
    }
}

#define DISP_TEST_TIME 1700
void dsp_test(void)
{
    uint8_t idx = 0, idx1 = 0;

    // UP, DOWN, LEFT, RIGHT on
    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    LCD_DPLS1_ON;
    LCD_DPLS2_ON;
    LCD_DPLS3_ON;
    LCD_DPLS4_ON;
    LCD_DPLS5_ON;
    dsm_dsp_lcdmem_move();
    OSTimeDly(OS_MS2TICK(DISP_TEST_TIME));

    // Load, Secret, colon on
    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    LCD_DLOAD_ON;
    LCD_DLOADO_ON;
    LCD_DLOADX_ON;
    LCD_DSECRET_ON;
    LCD_DCOLON1_ON;
    LCD_DCOLON2_ON;
    dsm_dsp_lcdmem_move();
    OSTimeDly(OS_MS2TICK(DISP_TEST_TIME));

    // Load, Secret, colon on
    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    LCD_DUNIT_k_ON;
    LCD_DUNIT_V1_ON;
    LCD_DUNIT_V2_ON;
    LCD_DUNIT_A_ON;
    LCD_DUNIT_r_ON;
    LCD_DUNIT_h_ON;
    LCD_DUNIT_z_ON;
    LCD_DPERCENT_ON;
    dsm_dsp_lcdmem_move();
    OSTimeDly(OS_MS2TICK(DISP_TEST_TIME));

    // digit 0 ~ 9
    for (idx1 = 0; idx1 < 10; idx1++)
    {
        for (idx = 0; idx < 8; idx++)
        {
            lcd_digit_buf[idx] = idx1;
        }
        dsm_lcd_dsp_clear();
        dsm_lcd_dsp_lcdmem_clear();
        dsp_fill_lcd_map(lcd_digit_buf);
        dsm_dsp_lcdmem_move();
        OSTimeDly(OS_MS2TICK(DISP_TEST_TIME));
    }

    // char A, b ~ N
    for (idx1 = 0; idx1 < 19; idx1++)
    {
        for (idx = 0; idx < 8; idx++)
        {
            switch (idx1)
            {
            case 0:
                lcd_digit_buf[idx] = LDIGIT_A;
                break;
            case 1:
                lcd_digit_buf[idx] = LDIGIT_b;
                break;
            case 2:
                lcd_digit_buf[idx] = LDIGIT_C;
                break;
            case 3:
                lcd_digit_buf[idx] = LDIGIT_d;
                break;
            case 4:
                lcd_digit_buf[idx] = LDIGIT_E;
                break;
            case 5:
                lcd_digit_buf[idx] = LDIGIT_F;
                break;
            case 6:
                lcd_digit_buf[idx] = LDIGIT_SPACE;
                break;
            case 7:
                lcd_digit_buf[idx] = LDIGIT_BAR;
                break;
            case 8:
                lcd_digit_buf[idx] = LDIGIT_n;
                break;
            case 9:
                lcd_digit_buf[idx] = LDIGIT_r;
                break;
            case 10:
                lcd_digit_buf[idx] = LDIGIT_L;
                break;
            case 11:
                lcd_digit_buf[idx] = LDIGIT_t;
                break;
            case 12:
                lcd_digit_buf[idx] = LDIGIT_C;
                break;
            case 13:
                lcd_digit_buf[idx] = LDIGIT_r;
                break;
            case 14:
                lcd_digit_buf[idx] = LDIGIT_H;
                break;
            case 15:
                lcd_digit_buf[idx] = LDIGIT_P;
                break;
            case 16:
                lcd_digit_buf[idx] = LDIGIT_U;
                break;
            case 17:
                lcd_digit_buf[idx] = LDIGIT_u;
                break;
            case 18:
                lcd_digit_buf[idx] = LDIGIT_n;
                break;
            }
        }

        dsm_lcd_dsp_lcdmem_clear();
        dsm_lcd_dsp_clear();
        dsp_fill_lcd_map(lcd_digit_buf);
        dsm_dsp_lcdmem_move();
        OSTimeDly(OS_MS2TICK(DISP_TEST_TIME));
    }

    // error
    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    LCD_DERR_ON;
    dsm_dsp_lcdmem_move();
    OSTimeDly(OS_MS2TICK(DISP_TEST_TIME));

    // rate A,B,C,D on
    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    LCD_DRATE_A_ON;
    LCD_DRATE_B_ON;
    LCD_DRATE_C_ON;
    LCD_DRATE_D_ON;
    dsm_dsp_lcdmem_move();
    OSTimeDly(OS_MS2TICK(DISP_TEST_TIME));

    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    LCD_DP4_ON;
    LCD_DP3_ON;
    LCD_DP2_ON;
    LCD_DP1_ON;
    dsm_dsp_lcdmem_move();
    OSTimeDly(OS_MS2TICK(DISP_TEST_TIME * 2));

    // UP, DOWN, LEFT, RIGHT on
    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    LCD_DQUP_ON;
    LCD_DQDOWN_ON;
    LCD_DQLEFT_ON;
    LCD_DQRIGHT_ON;
    dsm_dsp_lcdmem_move();
    OSTimeDly(OS_MS2TICK(DISP_TEST_TIME));
}

void dsm_dsp_digit_update(void) { dsp_fill_lcd_map(lcd_digit_buf); }

void dsp_enter_lpm(void)
{
    /* LCD : LP-Sun */
    lcd_digit_buf[0] = LDIGIT_SPACE;
    lcd_digit_buf[1] = LDIGIT_SPACE;
    //
    lcd_digit_buf[2] = LDIGIT_L;
    lcd_digit_buf[3] = LDIGIT_o;
    lcd_digit_buf[4] = LDIGIT_P;
    lcd_digit_buf[5] = LDIGIT_r;
    lcd_digit_buf[6] = LDIGIT_5;
    lcd_digit_buf[7] = LDIGIT_u;
    //
    dsm_lcd_dsp_lcdmem_clear();
    dsp_drive_digit();
    dsm_dsp_lcdmem_move();
}

void dsp_low_pwr_entry_state(uint32_t dsp_idx)
{
    lcd_digit_buf[2] = LDIGIT_L;
    lcd_digit_buf[3] = LDIGIT_P;
    lcd_digit_buf[4] = LDIGIT_BAR;
    lcd_digit_buf[5] = LDIGIT_BAR;
    lcd_digit_buf[6] = LDIGIT_BAR;

    switch (dsp_idx)
    {
    case 0:
        lcd_digit_buf[7] = LDIGIT_0;
        break;
    case 1:
        lcd_digit_buf[7] = LDIGIT_1;
        break;
    case 2:
        lcd_digit_buf[7] = LDIGIT_2;
        break;
    case 3:
        lcd_digit_buf[7] = LDIGIT_3;
        break;
    case 4:
        lcd_digit_buf[7] = LDIGIT_4;
        break;
    case 5:
        lcd_digit_buf[7] = LDIGIT_5;
        break;
    case 6:
        lcd_digit_buf[7] = LDIGIT_6;
        break;
    case 7:
        lcd_digit_buf[7] = LDIGIT_7;
        break;
    case 8:
        lcd_digit_buf[7] = LDIGIT_8;
        break;
    case 9:
        lcd_digit_buf[7] = LDIGIT_9;
        break;
    default:
        lcd_digit_buf[7] = /*LDIGIT_9*/ LDIGIT_BAR;
        break;
    }

    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    dsp_drive_digit();
    dsm_dsp_lcdmem_move();
}

void dsp_debug_state(uint32_t dsp_idx)
{
    lcd_digit_buf[2] = LDIGIT_d;
    lcd_digit_buf[3] = LDIGIT_b;
    // lcd_digit_buf[4] = LDIGIT_t;
    lcd_digit_buf[4] = LDIGIT_9;
    lcd_digit_buf[5] = LDIGIT_BAR;
    lcd_digit_buf[6] = LDIGIT_BAR;

    switch (dsp_idx)
    {
    case 0:
        lcd_digit_buf[7] = LDIGIT_0;
        break;
    case 1:
        lcd_digit_buf[7] = LDIGIT_1;
        break;
    case 2:
        lcd_digit_buf[7] = LDIGIT_2;
        break;
    case 3:
        lcd_digit_buf[7] = LDIGIT_3;
        break;
    case 4:
        lcd_digit_buf[7] = LDIGIT_4;
        break;
    case 5:
        lcd_digit_buf[7] = LDIGIT_5;
        break;
    case 6:
        lcd_digit_buf[7] = LDIGIT_6;
        break;
    case 7:
        lcd_digit_buf[7] = LDIGIT_7;
        break;
    case 8:
        lcd_digit_buf[7] = LDIGIT_8;
        break;
    case 9:
        lcd_digit_buf[7] = LDIGIT_9;
        break;
    default:
        lcd_digit_buf[7] = /*LDIGIT_9*/ LDIGIT_BAR;
        break;
    }

    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    dsp_drive_digit();
    dsm_dsp_lcdmem_move();
}

void dsp_up_pwr_on_state(void)
{
    lcd_digit_buf[2] = LDIGIT_U;
    lcd_digit_buf[3] = LDIGIT_P;
    lcd_digit_buf[4] = LDIGIT_BAR;
    lcd_digit_buf[5] = LDIGIT_o;
    lcd_digit_buf[6] = LDIGIT_n;
    lcd_digit_buf[7] = LDIGIT_BAR;

    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    dsp_drive_digit();
    dsm_dsp_lcdmem_move();
}
