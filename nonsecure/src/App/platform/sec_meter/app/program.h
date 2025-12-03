
#ifndef PROGRAM_H
#define PROGRAM_H 1

#include "appl.h"  // OBIS_ID_SIZE

#define PROG_EVT_FUT_WORK 0x01
#define PROG_EVT_MIN_CHG 0x02

#define prog_evt_set_fut_work() (program_event |= PROG_EVT_FUT_WORK)

#define prog_season_changed() (b_season_changed)
#define prog_season_changed_clr() (b_season_changed = 0)

#define prog_changed_event() (b_prog_changed_event)
#define prog_changed_event_clr() (b_prog_changed_event = 0)

#define PROG_DAY_ID_SIZE DAY_PROF_SIZE
#define PROG_DAY_ID_MAX \
    (PROG_DAY_ID_SIZE - 1)  // 0 <= day_id <= PROG_DAY_ID_MAX

#define PERIODIC_HOL 0
#define nPERIODIC_HOL 1

typedef struct
{
    uint16_t cnt;
    uint8_t name[PROG_ID_SIZE];
    date_time_type dt;
    uint16_t CRC_M;
} program_info_type;

#define EXT_TOU_ID_SIZE 16
#define EXT_TOU_UNIQ_MSB_POS 8
#define EXT_TOU_VCRC_MSB_POS 14

typedef struct
{
    uint8_t ext_tou_id[EXT_TOU_ID_SIZE];
    uint16_t cosem_cnt;
    uint16_t last_classid;
    uint8_t last_obis[OBIS_ID_SIZE];
    uint8_t last_attid;
    uint16_t last_arrayidx;
    uint16_t tou_crc;
    uint16_t CRC_M;
} tou_set_cnt_type;

typedef struct _npbill_date_backup_
{
    bool curprog_available;
    npbill_date_type date;
} ST_npBILL_BACKUP;

extern bool prog_dlcmd_avail;
extern tou_set_cnt_type tou_set_cnt;

extern uint8_t program_event;
extern bool b_season_changed, b_prog_changed_event;
extern uint8_t aprog_area_rcnt;

void program_init(void);
void program_proc(uint8_t *tptr);
void tou_update_by_rtchg(uint8_t *tptr);
void prog_info_reset(uint8_t *tptr);
void prog_info_write(bool curprog, uint8_t progcnt, prog_dl_type *progdl,
                     date_time_type *pdt, uint8_t *tptr);
void prog_tou_refresh(uint8_t *tptr, bool season_chk);
void curr_rate_update(void);
bool prog_season_profile_proc(uint8_t *tptr, bool curr_prog_in,
                              bool season_chk);
meas_method_type parse_meas_method(uint8_t lcdparm);
void edit_holidays(bool insert, bool fut, uint16_t idx, uint16_t yr,
                   holiday_struct_type *hol, uint8_t *tptr);
void season_chg_proc(date_time_type *pdt, uint8_t *tptr);
void prog_fut_mon(uint8_t *tptr);
rate_type rtn_curr_rate(date_time_type *dt);
void prog_cur_tou_suppdsp_delete(void);
void prog_cur_tou_delete(void);
void prog_dl_backup_restore(void);
void prog_dl_backup_save(void);
void tou_set_cnt_restore(void);
bool tou_set_cnt_save(void);
void tou_set_cnt_reset(void);
bool tou_set_is_reset(void);  // 16.11.11
bool touset_extprogid_set(uint8_t *extprogid);
bool touset_last_obj_set(uint16_t classid, uint8_t *obis, uint8_t attid,
                         uint16_t arrayidx, uint8_t *msg, int msglen);
bool prog_work_is_valid(bool is_curr);
void program_default_init(void);
uint16_t prog_setbits_to_availbits(uint32_t setbits, uint16_t availbits);

bool is_weekends(void);
bool is_phol_nphoiday(uint8_t hol_type);
void phol_nv_read(uint8_t hol_type);

void prog_curr_npbill_date_backup(void);
ST_npBILL_BACKUP *prog_get_curr_npbill_date_backup(void);

#endif
