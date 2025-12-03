#ifndef METER_APP_H
#define METER_APP_H 1

#include "options_def.h"
#include "whm.h"

typedef enum
{
    /* 상한 표시자 : 유효 및 무효전력 벡터 */
    /* 1상한, 2상한, 3상한, 4상한 */
    eVIq1,
    eVIq2,
    eVIq3,
    eVIq4,
    num_eVIq
} vi_quarter_type;

typedef struct
{
    uint32_t ch[numCHs];
} acc_ch_type;

typedef struct
{
    uint32_t index;
    uint16_t cnt;
    float sum_v[3];

    /* bccho, 2024-09-05, 삼상 */
    float sum_ltol[3];

    float sum_i[3];
    float sum_[3];
    uint16_t thd[3];
} lpavg_info_type;

typedef struct
{
    uint32_t index;
    uint16_t cnt;
} lprt_info_type;

enum _extMeas_intv
{
    eIntvCum,
    eIntvDm,
    eIntvNum
};

typedef struct
{
    int8_t _idx;
    uint32_t _meas;
} intv_meas_info_type;

typedef struct
{
    rate_type accrt;
    acc_ch_type acc[numRates];
    uint32_t lpdm[numCHs];
    float lppf[numDirChs];
    lpavg_info_type lpavg;
    uint32_t Drlimit;
    intv_meas_info_type _intvmeas[eIntvNum];
    lprt_info_type lprt;
    uint16_t CRC_M;
} mt_acc_type;

#define accm_rate mt_accm.accrt

#define ACC_CH(rt, i) mt_accm.acc[rt].ch[i]
#define ACCM_ENERGY(acc, w)    \
    acc += w;                  \
    if (acc >= mxaccm_dgt_cnt) \
    acc -= mxaccm_dgt_cnt
#define lp_intv_dm mt_accm.lpdm
#define lp_intv_pf mt_accm.lppf
#define lpavg_v_index mt_accm.lpavg.index
#define lpavg_vi_cnt mt_accm.lpavg.cnt
#define lpavg_v_sum mt_accm.lpavg.sum_v

/* bccho, 2024-09-05, 삼상 */
#define lpavg_ltol_sum mt_accm.lpavg.sum_ltol

#define lpavg_i_sum mt_accm.lpavg.sum_i
#define lpavg_thd_last mt_accm.lpavg.thd
#define DR_limit_timer_sec mt_accm.Drlimit
#define intv_cumaccm_idx mt_accm._intvmeas[eIntvCum]._idx
#define intv_cumaccm_meas mt_accm._intvmeas[eIntvCum]._meas
#define intv_dmaccm_idx mt_accm._intvmeas[eIntvDm]._idx
#define intv_dmaccm_meas mt_accm._intvmeas[eIntvDm]._meas
#define lprt__index mt_accm.lprt.index
#define lprt__cnt mt_accm.lprt.cnt

typedef struct
{
    float _val;
    date_time_type _dt;
    uint16_t CRC_M;
} min_max_data_type;

typedef struct
{
    date_time_type _dt;
    uint16_t CRC_M;
} pwf_date_type;

#if 1  // jp.kim 24.10.28
typedef struct
{
    uint8_t _val;
    uint16_t CRC_M;
} Work_PwrF_data_type;
#endif

extern uint32_t LineStatus;
extern uint8_t b_temp_vgain;

void Calculate_RMS(bool eskip);
void Acc_Watt_Var(void);
void Calculate_PF(void);
void Run_Meter(bool eskip);
void Print_Meter(void);
void Calculate_VAH(void);
void Test_Mode(uint8_t move_state);
void Ext_Power_Assignment(void);
void SET_EXPORT_LAG_VARH(void);
void Calculate_Freq_THD_VPhase(void);
void Run_EMB_ISR(void);
uint32_t get_ch_val(uint8_t month, rate_type rt, energy_dir_type dir,
                    energy_kind_type enkind, uint8_t *tptr);
uint32_t get_ch_val_nprd(uint8_t month, rate_type rt, energy_dir_type dir,
                         energy_kind_type enkind, uint8_t *tptr);
vi_quarter_type get_vi_quarter(void);
void get_accm_chs(rate_type rt, uint32_t *pch);
void accm_ch_copy(uint32_t *pch1, uint32_t *pch2);
void accm_ch_diff(uint32_t *pch, uint32_t *pch1, uint32_t *pch2);
uint32_t get_sel_react32(energy_dir_type dir, uint32_t *chptr);
uint32_t VAh_WVAR(int32_t w0, int32_t v0);
float VAh_WVAR_f(int32_t w0, int32_t v0);
void Id_Current(void);
void default_min_max_val(void);
void min_max_load(void);
char *dsm_rate_string(rate_type rate);
void background(void);
void foreground(void);

/* bccho, 2024-09-05, 삼상 */
void volt_drop_thrd_set(void);

extern volatile uint8_t b_emb_display;
extern volatile uint8_t b_low_voltage;
extern bool b_emb_started;
extern bool b_emb_display_ok;
extern bool tmp_read_ready;
extern mt_acc_type mt_accm;
extern bool pulse_dir;
extern int thd_meas_idx;
extern uint32_t VAs[PHASE_NUM];
extern int32_t WATTs[PHASE_NUM];
extern int32_t VARs[PHASE_NUM];
extern float min_freq, max_freq;

#if 1 /* bccho, 2024-09-05, 삼상 */
extern float min_volt[3], max_volt[3];
#else
extern float min_volt, max_volt;
#endif

extern float i_rms_n;

#endif
