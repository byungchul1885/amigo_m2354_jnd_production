#ifndef METER_H
#define METER_H 1

#if 0 /* bccho, 2023-07-20 */
#include "rtc.h"
#else
#include "App/platform/sec_meter/rtc/rtc.h"
#endif

extern uint16_t ce_interrupt;  // CE Interrupt status set by CE_ISR

#define CEBUSY 0x0001     // ce busy bit mask
#define XFER_DONE 0x0002  // ce xfer bit mask
#define XFER_LOST 0x0800  // ce xfer data lost bit mask
#define VPULSE 0x0004     // vpulse bit mask
#define WPULSE 0x0008     // wpulse bit mask
#define XPULSE 0x0010     // xpulse bit mask
#define YPULSE 0x0020     // ypylse bit mask
#define SYNC 0x0080       // sync bit mask

extern uint32_t state;

extern void meter_run(void);

/* update the front panel simulated on the serial line */
void update_line_display(void);

// Saves a copy of the meter's registers.
void put_reg(void);

// Restores the best available copy of the meter's registers.
void get_reg(void);

// Saves a copy of the meter's calibration.
void put_cal(void);

// Control power
void power_low(void);
void power_on(void);

// Restores the best available copy of the meter's calibration.
// A valid calibration is always produced.
// Returns true when a stored calibration is restored.
// False indicates a default is installed.
uint8_t get_cal(void);

// Force the factory default CE and MPU calibration.
void reset_cal(void);

/* number of calibration items in the DSP/AFE */
#define AFE_CAL_BEGIN 0x10 /* starting address */
#define AFE_CAL_END 0x3F   /* ending address */
#define AFE_CAL_CNT 0x30   /* number of items */

// watchdog operations.
#define disable_watchdog(_WD_) wd_want &= ~(_WD_)
#define enable_watchdog(_WD_) wd_want |= (_WD_)
#define CE_WD 1

// Calibration data.
#define SHNT_K_CNT 5
#define PHASE_K_CNT 6
#define GAIN_ADJ_CNT 3

enum eCALSRC
{
    is_default = 0,
    is_read,
    is_user
};
struct cal_s
{
    int32_t afe_cal_a[22];  // Calibration data for AFE
    int32_t i_min_e;        // )0, Min. permitted current (LSB of CE's i0sqsum)
    uint8_t cfg_e;          // )1, Configuration bits.
    int32_t v_min_e;        // )2, Min. permitted voltage (LSB of CE's v0sqsum)
    int16_t i_max_e;        // )3, maximum current. (2080 = 208.0A)
    int16_t v_max_e;        // )4, maximum voltage. (6000 = 600.0V)
    int32_t wrate_mpu_e;    // )5, (w0sqsum counts)/pulse
    int32_t i_limit_e;      // )6, max. permitted current (LSB of CE's i0sqsum)
    int32_t v_limit_e;      // )7, Max permitted voltage (LSB of CE's v0sqsum)
    int16_t i_max_neutral_e;  // )8, Maximum current for neutral current sensor.
    uint8_t interval_e;       // )9, Minutes of the demand interval.
    int16_t
        s_cal_e;  // )A, accumulation intervals of autocalibration measurement.
    int16_t v_cal_e;         // )B, 0.1 Volts rms of autocalibration.
    int16_t i_cal_e;         // )C, 0.1 Amp rms of autocalibration.
    int16_t theta_cal_e;     // )D, angle of autocalibration signal, degrees.
    uint16_t lcd_idx_e;      // )E, Selects LCD display.
    uint32_t lcd_bit_e;      // )F, Each bit selects an LCD display item.
    uint32_t mfr_id_e;       // )10, 3-character manufacturer ID string.
    int32_t meter_id_e;      // )11, Meter ID number
    int16_t gmt_offset_e;    // )12, Minutes of offset from GMT
    int32_t tcab_e;          // )13, register image of TCAB
    int32_t tccd_e;          // )14, register image of TCCD
    int32_t rtc_cal_e;       // )15, register image of RTC_CAL
    uint16_t pulse_src_e;    // )1a, pulse source
    enum eCALSRC cal_src_e;  // )16 Calibration source
    uint8_t cal_cnt_e;       // )17, Count of calibrations, >255=255
    uint8_t ver_hash_e;      // )18, hash of firmware version.
    uint16_t data_ok_cal_e;  // )19, check data
};
typedef struct cal_s Cal_t;

// Register data. Keep it small so it can be saved quickly
// to EEPROM during a power failure.
// A very simple meter could cut everything except
// state_bit_ary, wpulse_cnt, ver_hash and data_ok_reg.
struct reg_s
{
    uint32_t state_bit_ary_e;  // )40, Status bits. Mandatory.
    int32_t wpulse_cnt_e;      // )41, Wh, pulse count. Mandatory.
    int32_t wpulse_cnt_ex_e;   // )42, Wh, exported, pulse count. Optional.
    int32_t rpulse_cnt_e;      // )43, VARh, pulse count. Optional.
    int32_t rpulse_cnt_ex_e;   // )44, VARh, exported, pulse count. Optional.
    int32_t dmd_max_e;         // )45, Max demand, W. Optional.
    tm_t dmd_max_tm_s;         // )46..4A, Time starting max demand. Optional.
    int32_t tamper_sec_e;      // )4B, Seconds of tamper. Optional.
    int32_t sag_sec_e;         // )4C, Seconds Vmain < v_min. Optional.
    int32_t in_sec_e;          // )4D, Seconds In > in_max. Optional.
    tm_t tm_s;                 // Last clock time. Mandatory for RTC.
#ifdef trim                    // trim is defined in options.h
    int32_t trim_e;            // rtc_mpu_comp.c uses it to compensate time off
    int32_t trim_cnt_e;        // rtc_mpu_comp.c uses it to compensate time off
#endif
    int32_t operating_sec_e;  // )4E, Seconds of operation. Optional.
    int8_t reg_cnt_e;         // )50, Count of register saves.  Optional.
    uint8_t ver_hash_e;       // )51, hash of firmware version. Mandatory.
    uint16_t data_ok_reg_e;   // )52, Check-data for registers. Mandatory.
};
typedef struct reg_s Reg_t;

#define SHNT_CNT 2
struct misc_s
{
    int16_t temp_c_e[5];       // )16..1A, temp, LSB = 0.1C
    uint32_t acc_cnt_e;        // )3F, Count of accumulation intervals.
    uint8_t v_bat_rtc_e;       // )3D, Most recent read of RTC battery voltage.
    uint8_t v_bat_e;           // )3C, Most recent read of non-rtc battery V.
    uint8_t v_bat_date_e;      // Date of most recent battery read.
    uint8_t in_cnt_e;          // I neutral hysteresis (down counter)
    uint8_t reg_lock_timer_e;  // 0 = power registers are unlocked.
    int16_t lcd_timer_e;       // seconds to next scroll of LCD.
    int32_t wpulse_frac_e;     // Wh "fractional pulse counter"
    int32_t rpulse_frac_e;     // VARh "fractional pulse counter"
    int32_t wpulse_frac_ex_e;  // Wh export "fractional pulse counter"
    int32_t rpulse_frac_ex_e;  // VARh export "fractional pulse counter"
    int32_t vapulse_cnt_e;     // VA "pulse count"
    int32_t vapulse_frac_e;    // VA "fractional pulse count"
    int32_t dmd_cnt_e;         // demand "pulse count"
    int32_t dmd_frac_e;        // demand "fractional pulse count"
    int32_t dmd_acc_cnt_e;     // acc_cnt from start of demand interval.
    tm_t dmd_tm_s;             // Start of most recent demand interval.
    int32_t zc_cnt_e;          // Count of zero crossings in main.
    int32_t last_sec_e;        // detect change of second.
    uint8_t last_min_e;        // detect change of minute.
    uint8_t last_hr_e;         // detect change of hour.
    uint16_t lcd_idx_old_e;    // detect change of LCD.
    uint8_t wd_want_e;         // watchdog events expected (bit per event).
    uint8_t wd_set_e;          // watchdog events so far (set as bit).
    uint8_t wd_timer_e;        // software watchdog timer.
    uint8_t mains_subsec_e;    // count mains cycles for s/w RTC
    uint8_t wake_secs_e;       // count of wake seconds.
    int8_t remote_idx_e;       // remote whose temperature is being read.
    int8_t save_cnt_e;         // Last read register save count
};
typedef struct misc_s Misc_t;

#define PI 3.1415926535897932384626433832795

#define DEGREES_PER_RADIAN (180.0 / PI)

#define RADIAN_PER_DEGREES (1 / DEGREES_PER_RADIAN)

#if METER_ID == 53
#define PulseKwh 2000
#define PulseKvarh 2000
#define PulseKvah 2000
#define IoverThrsld 60.0  // 50.0
#define MaxCur 60.0       // 50.0
#define BasicCur 5.0
#define AutoModeCur (MaxCur * 0.25)
#define MaxPower (220.0 * 60.0)
#define AutoModePower (MaxPower * 0.25)
#elif METER_ID == 54
#define PulseKwh 2000
#define PulseKvarh 2000
#define PulseKvah 2000
#define IoverThrsld 120.0
#define MaxCur 120.0
#define BasicCur 10.0
#define AutoModeCur (MaxCur * 0.125)
#define MaxPower (220.0 * 120.0)
#define AutoModePower (MaxPower * 0.125)
#elif METER_ID == 55 /* bccho, 2024-09-05, 삼상 */
#define PulseKwh 1000
#define PulseKvarh 1000
#define PulseKvah 1000
#define IoverThrsld 120.0
#define MaxCur 120.0
#define BasicCur 10.0
#define AutoModeCur (MaxCur * 0.125)
#define MaxPower (220.0 * 120.0)
#define AutoModePower (MaxPower * 0.125)
#else
#warning "[CAUTION] Meter Type Error"
#endif

#define StartCur (BasicCur * 3.0 / 1000.0)

// uint32_t LineStatus;
#define LA_WHBACK \
    (uint32_t)BIT0  // A and B and C should be continuous, A < B < C !!!!!!!
                    // ==> see abnormal check
#define LB_WHBACK (uint32_t)BIT1
#define LC_WHBACK (uint32_t)BIT2
#define LA_VDROP (uint32_t)BIT3
#define LB_VDROP (uint32_t)BIT4
#define LC_VDROP (uint32_t)BIT5
#define LA_VOVER (uint32_t)BIT6
#define LB_VOVER (uint32_t)BIT7
#define LC_VOVER (uint32_t)BIT8
#define LA_IOVER (uint32_t)BIT9
#define LB_IOVER (uint32_t)BIT10
#define LC_IOVER (uint32_t)BIT11
// #define LA_VMIN (uint32_t) BIT12
// #define LB_VMIN (uint32_t) BIT13
// #define LC_VMIN (uint32_t) BIT14

#define GE_MEM (uint32_t)BIT1
#define GE_MEM1 (uint32_t)BIT5
#define CLOCK_UNSET (uint32_t)BIT8
#define GE_LOWBAT (uint32_t)BIT9
#define GE_NOBAT_CHKED (uint32_t)BIT10

#define GE_NOBAT (uint32_t)BIT0
/*
3.21.4 작업 정전
(1) 작업 정전 조건
- 전력량계 단자 커버 오픈
- 외부 버튼 입력 신호 발생 ([메뉴] 또는 [이동] 버튼 입력 신호 발생 시)
(2) (1)항의 조건 중 1가지 이벤트가 발생한 시점에서 1,800초(30분) 이내에 정전이
발생하면 작업 정전으로 간주한다.
*/
#define WORK_BLACKOUT (uint32_t)BIT1
#define COVER_OPEN (uint32_t)BIT2
#define MAGNET_ERROR (uint32_t)BIT3
#define SECURITY_ERR (uint32_t)BIT4
#define PRE_PAID_OUT (uint32_t)BIT6
#define POWER_FAIL_STS (uint32_t)BIT7

#define LOAD_WRONG_CONNECTION (uint32_t)BIT10
#define TEMPOVER (uint32_t)BIT11
#define WRONG_CONN (uint32_t)BIT12
#define STOVERIA (uint32_t)BIT13
#define STOVERIB (uint32_t)BIT14
#define STOVERIC (uint32_t)BIT15

/* 자기진단 3 */
#define SAGVA \
    (uint32_t)BIT16  // A and B and C should be continuous, A < B < C !!!!!!!
                     // ==> see abnormal check
#define SAGVB (uint32_t)BIT17
#define SAGVC (uint32_t)BIT18
#define WRONG_NEUT (uint32_t)BIT19
#define SELF_PLS_ERR (uint32_t)BIT20
#define RELAY_ERRA (uint32_t)BIT21
#define RELAY_ERRB (uint32_t)BIT22
#define RELAY_ERRC (uint32_t)BIT23

/* 자기진단 4 */
#define SWELL_STS (uint32_t)BIT24
#define SAG_STS (uint32_t)BIT25
#define LA_V_LOW (uint32_t)BIT26
#define LB_V_LOW (uint32_t)BIT27
#define LC_V_LOW (uint32_t)BIT28
#define LA_V_HIGH (uint32_t)BIT29
#define LB_V_HIGH (uint32_t)BIT30
#define LC_V_HIGH (uint32_t)BIT31

#define ERR_MASK_2 (GE_MEM | GE_MEM1 | GE_LOWBAT)
#define ERR_MASK (GE_NOBAT)

#if PHASE_NUM == THREE_PHASE
#define IOVERMASK (STOVERIA | STOVERIB | STOVERIC)
#define RLYERRMASK (RELAY_ERRA | RELAY_ERRB | RELAY_ERRC)
#else
#define IOVERMASK (STOVERIA)
#define RLYERRMASK (RELAY_ERRA)
#endif

extern uint32_t MTStatus;
#define MT_INITED_BY_PROD (uint32_t)BIT19
#define MT_SAGED (uint32_t)BIT28
#define MT_INITED_BY_COMM (uint32_t)BIT29
#define MT_INITED_BY_VER (uint32_t)BIT30
#define MT_FIRST_BOOT (uint32_t)BIT31

#define curr_is_noload_A() (no_load_statusA == true)
#define curr_is_noload_B() (no_load_statusB == true)
#define curr_is_noload_C() (no_load_statusC == true)
#if PHASE_NUM == SINGLE_PHASE
#define curr_is_noload() curr_is_noload_A()
#else
#define curr_is_noload() \
    (curr_is_noload_A() && curr_is_noload_B() && curr_is_noload_C())
#endif
#define curr_is_yesload_A() (start_curr_statusA == true)
#define curr_is_yesload_B() (start_curr_statusB == true)
#define curr_is_yesload_C() (start_curr_statusC == true)

#define lcd_plsquart_is_skip() (lcd_pls_quart_skip_cnt != 0)

typedef struct
{
    float w0;
    float w1;
    float w2;
    float var0;
    float var1;
    float var2;
    float va0;
    float va1;
    float va2;
    float v0;
    float v1;
    float v2;
    float i0;
    float i1;
    float i2;
    float v0h;
    float v1h;
    float v2h;
    float frq;
    float phAB;
    float phAC;
    float traw;
} outputs_mon_type;

extern outputs_mon_type outputs_mon;
/* 유효 */
#define w0sum_mon outputs_mon.w0
#define w1sum_mon outputs_mon.w1
#define w2sum_mon outputs_mon.w2
/* 무효 */
#define var0sum_mon outputs_mon.var0
#define var1sum_mon outputs_mon.var1
#define var2sum_mon outputs_mon.var2
/* 피상 */
#define va0sum_mon outputs_mon.va0
#define va1sum_mon outputs_mon.va1
#define va2sum_mon outputs_mon.va2
#define v0sqsum_mon outputs_mon.v0
#define v1sqsum_mon outputs_mon.v1
#define v2sqsum_mon outputs_mon.v2
#define i0sqsum_mon outputs_mon.i0
#define i1sqsum_mon outputs_mon.i1
#define i2sqsum_mon outputs_mon.i2

#define v0sqsum_h_mon outputs_mon.v0h
#define v1sqsum_h_mon outputs_mon.v1h
#define v2sqsum_h_mon outputs_mon.v2h
#define freq_mon outputs_mon.frq
#define PH_AtoB_mon outputs_mon.phAB
#define PH_AtoC_mon outputs_mon.phAC
#define temp_raw_mon outputs_mon.traw

typedef struct
{
    float v0;
    float v1;
    float v2;
    float i0;
    float i1;
    float i2;
    float ph0;
    float ph1;
    float ph2;
    float w0;
    float var0;
} rms_mon_type;

extern rms_mon_type rms_mon;

#define vrms0_mon rms_mon.v0
#define vrms1_mon rms_mon.v1
#define vrms2_mon rms_mon.v2
#define irms0_mon rms_mon.i0
#define irms1_mon rms_mon.i1
#define irms2_mon rms_mon.i2
#define ph0_mon rms_mon.ph0
#define ph1_mon rms_mon.ph1
#define ph2_mon rms_mon.ph2
#define pwr0_mon rms_mon.w0
#define pwrvar0_mon rms_mon.var0

#define AC_POWER_OFF 0
#define AC_POWER_ON 1

extern uint8_t g_ac_power_fromONtoOff;
extern uint8_t g_ac_power_on_pre_state;

#define m_ac_power_off_trigger() (g_ac_power_fromONtoOff = TRUE)
#define m_ac_power_off_not_trigger() (g_ac_power_fromONtoOff = FALSE)
#define m_ac_power_off_is_trigger() (g_ac_power_fromONtoOff == TRUE)
#define m_ac_power_off_is_not_trigger() (g_ac_power_fromONtoOff == FALSE)

#define m_ac_power_set_high() (g_ac_power_on_pre_state = AC_POWER_ON)
#define m_ac_power_set_low() (g_ac_power_on_pre_state = AC_POWER_OFF)
#define m_ac_power_is_high() (g_ac_power_on_pre_state == AC_POWER_ON)
#define m_ac_power_is_low() (g_ac_power_on_pre_state == AC_POWER_OFF)

extern bool start_curr_statusA;
extern bool start_curr_statusB;
extern bool start_curr_statusC;

extern bool no_load_statusA;
extern bool no_load_statusB;
extern bool no_load_statusC;

extern uint16_t
    xfer_done_timer;  // should be unsigned type (in case of overflow counter)

extern bool sag_swell_enabled;
extern int sag_swell_delayed;
extern bool sag_det_state;
extern bool swell_det_state;
extern int sag_det_cnt;
extern int swell_det_cnt;
extern int sag_swell_happened;

extern bool b_emb_display_ok;
extern bool b_lpavg_ready;
extern bool b_lp_kwh_ready;
extern bool b_imax_ready;
extern bool b_temp_mon_ready;
extern bool b_scurr_mon_ready;
extern bool b_abnormal_chk_ready;

extern u16_t ln_mrcnt_idx;

extern int lpsts_intv_mon;
extern uint32_t lpsts_cntidx;

// extern lpsts_data_type lpsts_data[MAX_LPSTS_SIZE];
// extern ami_prof_data_type ami_prof_data;

extern uint8_t lcd_pls_quart_skip_cnt;

#define sag_swell_enable() \
    sag_swell_delayed = 1; \
    sag_swell_enabled = 1
#define sag_swell_disable() (sag_swell_enabled = 0)

uint32_t get_LineStatus(void);
void meter_task_init(void);
uint32_t meter_get_measure_flag(void);
void meter_set_measure_flag(uint32_t flag);
uint32_t meter_get_measure_xdone_timer(void);
void meter_set_measure_xdone_timer(uint32_t value);
void meter_task(void);
float get_inst_volt(int line);
float get_inst_curr(int line);
float get_inst_curr_allphase(void);
float get_inst_power_allphase(void);
float get_inst_volt_THD(int line);
float get_inst_power(bool _w, int line);
float get_inst_pf(int line);
float get_inst_phase(int line);
float get_inst_vphase(int line);
float get_inst_freq(void);
bool emb_disp_is_ready(void);
float get_inst_LtoL(int line);
uint8_t *adjust_tptr(uint8_t *tptr);
void background(void);
void monitor_sag_swell(void);

bool ac_power_is_failed(bool print_enable);

#endif
