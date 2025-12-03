#ifndef WHM_1_H
#define WHM_1_H 1

#define PWRFAIL_THRSHLD 95.0

#define MAGNET_DEFAULT_VALUE (1962)
#define MAGNET_MARGIN_VALUE (0x0080)

#define MAGNET_DET_HIGH (cal_magnet + MAGNET_MARGIN_VALUE)
#define MAGNET_DET_LOW (cal_magnet - MAGNET_MARGIN_VALUE)

#define MAGNET_DETED_BIT 0x02
#define MCOVEROPEN_DETED_BIT 0x04
#define TCOVEROPEN_DETED_BIT 0x08

#define MAGNET_DETED_BIT_THISMONTH 0x01
#define MorTCOVEROPEN_DETED_BIT_THISMONTH 0x02
#define TEMPOVER_DETED_BIT_THISMONTH 0x04
#define RELAYERR_DETED_BIT_THISMONTH 0x08
#define TAMPER_DET_BITS_THISMONTH \
    (MAGNET_DETED_BIT_THISMONTH | MorTCOVEROPEN_DETED_BIT_THISMONTH)

#define IS_MAGNET_DETED_THISMONTH \
    ((tamper_det_bit_thismonth & MAGNET_DETED_BIT_THISMONTH) != 0)
#define IS_COVEROPEN_DETED_THISMONTH \
    ((tamper_det_bit_thismonth & MorTCOVEROPEN_DETED_BIT_THISMONTH) != 0)
#define IS_TEMPOVER_DETED_THISMONTH \
    ((tamper_det_bit_thismonth & TEMPOVER_DETED_BIT_THISMONTH) != 0)
#define IS_RELAYERR_DETED_THISMONTH \
    ((tamper_det_bit_thismonth & RELAYERR_DETED_BIT_THISMONTH) != 0)
#define IS_MorTCOVER_OPEN_THISMONTH \
    ((tamper_det_bit_thismonth & MorTCOVEROPEN_DETED_BIT_THISMONTH) != 0)

#define IS_MCOVER_OPEN ((tamper_det_bit & MCOVEROPEN_DETED_BIT) != 0)
#define IS_TCOVER_OPEN ((tamper_det_bit & TCOVEROPEN_DETED_BIT) != 0)
#define IS_MorTCOVER_OPEN \
    ((tamper_det_bit & (MCOVEROPEN_DETED_BIT | TCOVEROPEN_DETED_BIT)) != 0)
#define IS_MAGNET_DET ((tamper_det_bit & MAGNET_DETED_BIT) != 0)

#define LOW_V_CNT 7   // 5					// 5 sec
#define DROP_V_CNT 5  // 5 sec
#define OVER_I_CNT 3  // 3 sec
#define WRONG_WH_CNT 5

#define neut_line_is_wrong() (WMStatus & WRONG_NEUT)
#define line_is_wrong_connected() (WMStatus & WRONG_CONN)

#define VBAT_NO_THRSHLD 1100000L
#define VBAT_LOW_THRSHLD (VBAT_NO_THRSHLD - 170000L)
#define VBAT_JUMP_THRSHLD 100000L

#define relay_is_load_on() (relay_load_state == LOAD_ON)
#define relay_is_load_off() (relay_load_state == LOAD_OFF)

typedef enum
{
    LOAD_OFF,
    LOAD_ON
} load_state_type;

typedef enum
{
    SW_CTRL_OFF,
    SW_CTRL_ON
} time_sw_state_type;

typedef struct
{
    uint8_t sag_level;    // percent
    float sag_time;       // cycle
    uint8_t swell_level;  // percent
    float swell_time;     // cycle
    uint16_t CRC_M;
} sag_swell_info_type;

typedef struct
{
    uint32_t imaxint;
    int32_t imaxce;
    date_time_type dt;
    uint16_t CRC_M;
} imax_log_type;

typedef struct
{
    int8_t val;
    date_time_type dt;
    uint16_t CRC_M;
} tempover_log_type;

typedef struct
{
    uint8_t batstate;
    uint8_t installed;
    date_time_type instime;
    uint16_t CRC_M;
} bat_install_type;

typedef struct
{
    uint32_t bat_used;
    date_time_type batused_dt;
    uint16_t CRC_M;
} bat_used_time_type;

typedef struct
{
    uint8_t _savedt;
    int32_t remenergy;
    uint32_t remmoney;
    uint32_t remtime;
    uint32_t buyenergy;
    uint32_t buymoney;
    bool enable;
    bool cancel_flag;
    uint16_t CRC_M;
} prepay_info_type;

extern uint32_t WMStatus_intern;
extern uint32_t WMStatus;
extern int32_t VdropThrsld;
extern bool wrong_conn_prev;
extern int vdrop_cnt[3];
extern int v_low_cnt[3];
extern int whbwd_cnt;
extern int iover_cnt[3];

extern uint8_t relay_onff_timer;
extern bool relay_onff_req;
extern int32_t cal_magnet;
extern bool b_imax_ready;

/* bccho, 2024-09-05, 삼상 */
extern uint8_t imax_cnt[3];

extern uint32_t imax_int[3];
extern int32_t imax_ce[3];
extern float imax_sum[3];

extern bool b_temp_mon_ready;
extern int8_t temp_over_val;

extern bool sag_det_enabled;
extern bool swell_det_enabled;
extern bool sag_det_enabled_set;
extern bool swell_det_enabled_set;
extern int sag_dur_time;
extern int sag_detected_cnt;
extern int swell_dur_time;
extern int swell_detected_cnt;
extern uint16_t g_sag_thr, g_swell_thr;
extern uint32_t g_sag_thr_2p, g_swell_thr_2p;
extern uint32_t g_sag_thr_org, g_swell_thr_org;
extern bat_used_time_type bat_used_time;
extern bool bat_installed_inputmode;
extern bool bat_rtchg_history;
extern date_time_type dst_dt;
extern bool b_scurr_mon_ready;
extern prepay_info_type prepay_info;
extern bool b_abnormal_chk_ready;
extern bool first_relay_ctrl_skip;

void whm_init_1(void);
void sig_sel_out_proc(void);
void load_ctrl_init(void);
void load_initialize(void);
void relay_ctrl(bool ctrl);
void tamper_det_proc(void);
void sig_sel_proc(uint8_t sel);
bool scurr_is_limiting(void);
bool scurr_is_limiting_forever(void);
bool ts_conf_available(void);
void ts_ctrl(void);
bool ts_is_zone_on(void);
void ts_test_on_start(void);
void ts_test_off(void);
uint16_t ts_test_total_used(void);
void scurr_limit_init(void);
void scurr_limit_on(void);
void scurr_limit_off(void);
bool is_sCurr_valid(uint16_t cur);
void input_mode_bat_installed_set(void);

bool crc16_chk(uint8_t *buf, uint16_t len, bool set);
void get_bf_interval_boundary(date_time_type *pdate_time, uint8_t intv);
void get_interval_boundary(date_time_type *pdate_time, uint8_t intv);
void get_next_interval_boundary(date_time_type *pdate_time, uint8_t intv);
uint8_t last_date_of_month(uint8_t yr, uint8_t mon);
bool rtc_is_valid(date_time_type *dt);
int8_t cmp_date(date_time_type *ed, date_time_type *st);
int8_t cmp_mmdd(mm_dd_type *ed, mm_dd_type *st);
int8_t cmp_hhmm(uint8_t ed_h, uint8_t ed_m, uint8_t st_h, uint8_t st_m);
int8_t cmp_time(date_time_type *ed, date_time_type *st);
int8_t cmp_date_time(date_time_type *ed, date_time_type *st);
int16_t calc_date_diff(date_time_type *ed, date_time_type *st);
int32_t calc_dtime_diff(date_time_type *todt, date_time_type *fromdt);
int32_t calc_date_time_diff(date_time_type *ed, date_time_type *st);
uint8_t calc_dayofweek(date_time_type *dt);
void date_up(date_time_type *dt, uint8_t _up);
void date_down(date_time_type *dt, uint8_t _down);
void time_up(date_time_type *r_time, uint8_t min);
void time_down(date_time_type *r_time, uint8_t min);
bool is_good_day(enum touDAY _day);
bool is_good_date(date_time_type *dt);
bool is_good_time(date_time_type *dt);
bool is_good_date_time(date_time_type *dt);
void set_billing_parm(uint8_t *parm);
void rtn_billing_parm(uint8_t *parm);
void dr_limit_time_clear(void);
void DR_limit_adj(int32_t padj);
void DR_limit_dec(void);
void adjust_max_whcnt(uint8_t w_pnt, uint8_t wh_pnt);
uint16_t conv_point_to_mul(uint8_t pt);
void meas_method_adj_dirchg(void);
uint8_t conv_sel_to_DRkind(uint8_t sel);
uint8_t conv_DRkind_to_sel(uint8_t dr_kind);
void default_sag_swell(sag_swell_info_type *sagswell);
void sag_swell_save_set_start(uint8_t kind, uint8_t val, float fval);
void prepay_reset(void);
void prepay_info_save(bool _force);
void prepay_info_restore(bool saged);
uint32_t get_prepay_info(uint8_t kind);
prepay_info_type *get_prepay_all(void);
void prepay_proc(uint32_t sec_energy);
void set_pEOB_dr(uint8_t sel);
void set_npEOB_dr(uint8_t sel);
void rload_ctrl_set(uint8_t rctrl);
bool dst_mon(bool _normal, date_time_type *dt, uint8_t *tptr);
void wrong_conn_mon(void);
void bat_used_time_proc(bool rtc_backed, date_time_type *dt);
void bat_inst_proc(bat_inst_evt_type evt);
void bat_inst_clear(void);
void bat_inst_completely_clear(void);
void bat_inst_set(void);
void bat_used_time_clear(void);
void bat_used_time_backup(date_time_type *dt);
void bat_used_time_re_set(date_time_type *olddt, date_time_type *newdt);
uint32_t get_bat_used_time(void);
bool bat_is_installed(void);
void bat_used_time_add_save(int32_t u_tim, date_time_type *dt);
void dst_chk_rtchg(date_time_type *dt);
void dst_mon_rtchg(date_time_type *dt);
void latchon_cnt_inc(void);
void latchon_cnt_clear(void);
uint16_t get_latchon_cnt(void);
void prepay_load_off(void);
void prepay_load_on(void);

#endif
