
#ifndef EOI_H
#define EOI_H 1

#define EOI_EVENT_MIN_CHG BIT0
#define EOI_EVENT_TIME_CHG BIT1
#define EOI_EVENT_INTV_CHG BIT2
#define EOI_EVENT_DR_EVT BIT3
#define EOI_EVENT_RATE_CHG BIT4
#define EOI_EVENT_SUBINTV_CHG BIT5

#define NUM_DM_INTV 6

#define MAX_ROLLING_NUM 15

typedef struct
{
    uint8_t sublock_cnt;
    uint8_t sublock_idx;
    uint32_t sublock_dm[numDmCHs][MAX_ROLLING_NUM];
    uint16_t CRC_M;
} rolling_dm_type;

#define dm_subintv_cnt rolling_dm.sublock_cnt
#define dm_subintv_idx rolling_dm.sublock_idx
#define dm_subintv(ch) rolling_dm.sublock_dm[ch][rolling_dm.sublock_idx]
#define dm_subintv_dm rolling_dm.sublock_dm

#define dm_subintv_act_fwd \
    rolling_dm.sublock_dm[eDmChDeliAct][rolling_dm.sublock_idx]
#define dm_subintv_app_fwd \
    rolling_dm.sublock_dm[eDmChDeliApp][rolling_dm.sublock_idx]
#define dm_subintv_act_bwd \
    rolling_dm.sublock_dm[eDmChReceiAct][rolling_dm.sublock_idx]
#define dm_subintv_app_bwd \
    rolling_dm.sublock_dm[eDmChReceiApp][rolling_dm.sublock_idx]

typedef struct
{
    uint8_t cnt;
    uint16_t sublock[MAX_ROLLING_NUM];
    uint16_t CRC_M;
} rolling_dm_ch_type;

typedef struct
{
    rolling_dm_ch_type ch[numDmCHs];
} rolling_dm_data_type;

typedef struct
{
    rolling_dm_data_type roll[numRates];
} rolling_dm_rate_type;

typedef struct
{
    uint32_t val;
    date_time_type dt;
} max_dm_info_type;

typedef struct
{
    max_dm_info_type dm[numDmCHs];
} max_dm_data_type;

typedef struct
{
    max_dm_data_type max[numRates];
    uint16_t CRC_M;
} max_demand_type;

typedef struct
{
    uint32_t dm[numDmCHs];
    date_time_type dt;
    uint16_t CRC_M;
} recent_demand_type;

extern max_demand_type max_dm;
extern date_time_type dr_dt;
extern uint8_t dm_period_num;
extern rolling_dm_type rolling_dm;
extern int eoi_deactive_timer;
extern bool b_eoi_deactive;

void eoi_init(void);
void dr_dt_init(void);
void eoi_proc(uint8_t *tptr);
void dm_reset(date_time_type *pdt);
uint8_t get_dm_interval(uint8_t idx);
uint8_t get_dmintv_index(uint8_t intv);
bool is_dm_intv_valid(uint8_t intv);
uint32_t get_cur_mxdm(rate_type rt, demand_ch_type ch);
void get_cur_mxtime(date_time_type *dt, rate_type rt, demand_ch_type ch);
uint32_t get_cur_cumdm(rate_type rt, demand_ch_type ch);
void eoi_parm_set(uint8_t evt);
void eoi_proc_dr(date_time_type *pdt, bool isdr, uint8_t *tptr);
void get_cur_dmdt(date_time_type *pdt);
void cur_dmdt_set(date_time_type *pdt);
void eoi_proc_pwrtn(pwrfail_info_type *pfinfo, int32_t dur, uint8_t *tptr);
void eoi_proc_timechg(date_time_type *bfdt, rate_type rt, uint8_t *tptr,
                      bool fut_exe);
void eoi_proc_ratechg(rate_type rt, uint8_t *tptr);
void fill_rcnt_ch(uint32_t *eoich, uint8_t intv);
uint32_t get_curr_rcnt_ch(bool rcnt, demand_ch_type ch, uint8_t *tptr);
uint8_t get_rcnt_pf(energy_dir_type dir);
void dm_intv_nvdelete(uint8_t *tptr);
void dm_intv_init(void);
float calc_pf_for_vah(uint32_t wh, uint32_t vah);
void dm_intv_save(void);
void dm_intv_load(void);
void eoi_or_pulse_select(void);

#endif
