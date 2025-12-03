
#ifndef EOB_H
#define EOB_H 1

#include "whm.h"
#include "eoi.h"
#include "amg_sec.h"

#define MAX_MREADING 6
#define MAX_MREADING_nPRD 4
#define MAX_MREADING_SEASON 4
#define NUM_MREADING_SAVE \
    (MAX_MREADING + 1)  // previous + 1 extra month meter_reading save
#define NUM_MREADING_nPRD_SAVE \
    (MAX_MREADING_nPRD + 1)  // previous + 1 extra month meter_reading save
#define NUM_MREADING_SEASON_SAVE \
    (MAX_MREADING_SEASON + 1)  // previous + 1 extra month meter_reading save

#define EOB_PERIOD_FLAG BIT0  /* 정기 */
#define EOB_nPERIOD_FLAG BIT1 /* 비정기 */
#define EOB_SEASON_FLAG BIT2  /* 계절 */

typedef enum
{
    PERIOD_GRP_NULL,
    PERIOD_GRP_B /* 정기 */,
    nPERIOD_GRP_B /* 비정기 */,
    SEASON_GRP_B /* 계절 */,
    numbills
} billing_data_grp_type;

#define VZ_LASTEST_VAL 101

typedef struct
{
    uint32_t ch[numCHs]; /* 전력량 */
    float pf[numDirChs]; /* 역률 */
    uint16_t CRC_M;
} mr_ch_type;

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
/* bccho, 2024-09-05, 삼상 */
typedef struct
{
    uint8_t ecdsa[DLMS_DS_LEN]; /*전자서명*/
    uint16_t CRC_M;
} mr_ecdsa_type;
#endif

typedef struct
{
    mr_ch_type accm[numRates];

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
    /* bccho, 2024-09-05, 삼상 */
    mr_ecdsa_type acc_ecdsa[numDirChs]; /*전자서명*/
#endif
} mr_data_accm_type;

typedef struct
{
    uint32_t cum_mxdm[numDmCHs];
    max_dm_info_type mxdm[numDmCHs];
    uint16_t CRC_M;
} mr_dm_type;

typedef struct
{
    mr_dm_type dm[numRates];

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
    /* bccho, 2024-09-05, 삼상 */
    mr_ecdsa_type dm_ecdsa[numDirChs]; /*전자서명*/
#endif
} mr_data_dm_type;

typedef struct
{
    date_time_type dt;
    uint8_t mtdir;
    ratekind_type rtkind;
    uint8_t selreact;
    uint16_t CRC_M;
} mr_data_info_type;

#define mr_dt mr_data_info.dt
#define mr_mtdir mr_data_info.mtdir
#define mr_rtkind mr_data_info.rtkind
#define mr_selreact mr_data_info.selreact

#define RATE_TO_DIR_BIT(x) (0x01 << x)

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
/* bccho, 2024-09-05, 삼상 */
extern mr_data_accm_type rd_data_accm;
#endif

extern mr_data_accm_type mr_data_accm;
extern mr_data_dm_type mr_data_dm;
extern mr_data_info_type mr_data_info;
extern npbill_date_type npBill_date;
extern bool b_eob_proc;

void eob_init(void);
void sr_dt_init(void);
void pwrtn_eob_proc(pwrfail_info_type *pfinfo, uint8_t *tptr);
void eob_proc(uint8_t *tptr);
void mr_capture_accm(mr_data_accm_type *accm);
void mr_capture_dm(mr_data_dm_type *dm, bool isdr);
void mr_capture_rolldm(rate_type rt, demand_ch_type dmch,
                       rolling_dm_ch_type *rollch);
void man_sr_dr_proc(date_time_type *pdt, uint8_t *tptr);
bool is_mrdt_within_5days(void);

#if 0  // jp.kim 25.06.22
uint32_t get_bf_mxdm(uint8_t month, rate_type rt, demand_ch_type ch,
                     uint8_t *tptr);
void get_bf_mxtime(date_time_type *dt, uint8_t month, rate_type rt,
                   demand_ch_type ch, uint8_t *tptr);
uint32_t get_bf_cumdm(uint8_t month, rate_type rt, demand_ch_type ch,
                      uint8_t *tptr);
uint32_t get_bf_cumdm_nprd(uint8_t month, rate_type rt, demand_ch_type ch,
                           uint8_t *tptr);
uint8_t get_bf_pf(uint8_t month, rate_type rt, energy_dir_type dir,
                  uint8_t *tptr);
#endif
#if 1  // jp.kim 25.06.22
uint32_t get_ch_val_PrdNprdSeason(uint8_t month, rate_type rt,
                                  energy_dir_type dir, energy_kind_type enkind,
                                  uint8_t *tptr);
uint32_t get_bf_mxdm_PrdNprdSeason(uint8_t month, rate_type rt,
                                   demand_ch_type ch, uint8_t *tptr);
void get_bf_mxtime_PrdNprdSeason(date_time_type *dt, uint8_t month,
                                 rate_type rt, demand_ch_type ch,
                                 uint8_t *tptr);
uint32_t get_bf_cumdm_PrdNprdSeason(uint8_t month, rate_type rt,
                                    demand_ch_type ch, uint8_t *tptr);
uint8_t get_bf_pf_PrdNprdSeason(uint8_t month, rate_type rt,
                                energy_dir_type dir, uint8_t *tptr);
#endif

uint8_t get_cur_pf(rate_type rt, energy_dir_type dir);
uint8_t month_to_grp_f(uint8_t month);
void sr_dr_proc(uint8_t eob_type, uint8_t srdr, date_time_type *pdt,
                uint8_t *tptr);
float calc_pf(uint32_t wh, uint32_t varh);
int8_t tchg_is_within_peob(date_time_type *oldt, date_time_type *newdt);
void sel_react_cancel(void);
void sel_react_monitor_rtchg(date_time_type *dt);
void npBill_date_init(void);
void npBill_date_load(void);
void npBill_date_load_forFut_notAvailxx(prog_dl_type *progdl);
int8_t bill_date_range_check(day_time_type *billdt, date_time_type *from,
                             date_time_type *to, bool chgdir);
int8_t eob_timechg_npEOB(date_time_type *oldt, date_time_type *newdt);
void sr_dr_proc_manual(uint8_t eob_type, date_time_type dt);
#endif
