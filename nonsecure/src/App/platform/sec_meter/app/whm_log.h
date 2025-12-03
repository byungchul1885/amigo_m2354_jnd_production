
#ifndef WHM_LOG_H
#define WHM_LOG_H 1

#include "whm_1.h"

typedef struct
{
    date_time_type dt[LOG_BUFF_SIZE];
} log_data_type;

typedef struct
{
    date_time_type dt;
    uint32_t durtime;
} evt_durtime_type;

typedef struct
{
    evt_durtime_type evt[LOG_BUFF_SIZE];
} log_data1_type;

typedef struct
{
    date_time_type dt;
    uint8_t ng_case;
} evt_cert_time_type;

typedef struct
{
    evt_cert_time_type evt[LOG_CERT_BUFF_SIZE];
} log_cert_data_type;

typedef struct
{
    date_time_type dt;
    uint8_t scurrcnt;
    uint16_t limit;
    uint16_t limit2;
    uint8_t limitcnt;
    uint8_t limitcnt_n1;
} scurr_log_info_type;

typedef struct
{
    scurr_log_info_type evt[LOG_BUFF_SIZE];
} scurr_log_data_type;

void log_data_reset(void);
void log_mt_initialization(uint8_t *tptr);
void log_mt_init_clear(uint8_t *tptr);
uint16_t get_mtinit_log_cnt(uint8_t *tptr);
void pwrtn_log_sag(pwrfail_info_type *pfinfo);
void log_sag_swell(elog_kind_type elog);
void log_sr_dr(date_time_type *dt, uint8_t srdr);
void log_mDR(date_time_type *dt);
void log_prog_chg(date_time_type *pdt);
void log_rtc_chg(date_time_type *fr_dt, date_time_type *to_dt);
void log_scurr_limit(void);
void log_scurr_nonsel(void);
void log_pwr_FR(pwrfail_info_type *pfinfo);
void log_rLoad_ctrl(void);
void log_magnet_det(date_time_type *pdt, uint32_t dur);
void log_cover_open(elog_kind_type elog);
void log_wrong_conn(void);
void imax_log_init(void);
void imax_val_set(uint8_t ch, imax_log_type *imax);
void imax_log_proc(void);
void temp_over_mon(void);
void imax_log_reset(void);
void temp_over_log_init(void);
void mt_abnorm_mon_init(void);
void mt_abnorm_mon(void);
void bat_det_mon(void);
void log_cert_ng(uint8_t cert_ng);

#endif
