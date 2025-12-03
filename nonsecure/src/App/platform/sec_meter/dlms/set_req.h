
#ifndef SET_REQ_H
#define SET_REQ_H 1

#include "disp.h"

#define SETBITS_DLS_BGN (uint32_t) BIT0
#define SETBITS_DLS_END (uint32_t) BIT1
#define SETBITS_DLS_DEV (uint32_t) BIT2
#define SETBITS_DLS_ENA (uint32_t) BIT3
#define SETBITS_LP_INTV (uint32_t) BIT4
#define SETBITS_SIG_SEL (uint32_t) BIT5
#define SETBITS_MODEM_BAUD (uint32_t) BIT6
#define SETBITS_PBILL_DRSEL (uint32_t) BIT7
#define SETBITS_PBILL_DRTYPE (uint32_t) BIT8
#define SETBITS_PBILL_DATE (uint32_t) BIT9
#define SETBITS_NPBILL_DRSEL (uint32_t) BIT10
#define SETBITS_NPBILL_DRTYPE (uint32_t) BIT11
#define SETBITS_NPBILL_DATE (uint32_t) BIT12
#define SETBITS_BILLING_PARM (uint32_t) BIT13
#define SETBITS_DM_PRD (uint32_t) BIT14
#define SETBITS_PGM_NAME (uint32_t) BIT15
#define SETBITS_TOU_SEASON (uint32_t) BIT16
#define SETBITS_TOU_WEEK (uint32_t) BIT17
#define SETBITS_TOU_DAY (U32) BIT18
#define SETBITS_PAS_TIME (U32) BIT19  // 예약 프로그램 적용 일자/시간
#define SETBITS_HOLIDAYS (U32) BIT20
#define SETBITS_SUPPDSP_ITEM (uint32_t) BIT21
#define SETBITS_LCDSET_PARM (uint32_t) BIT22
#define SETBITS_sCURR_LMT_VAL (uint32_t) BIT23
#define SETBITS_sCURR_AUTORTN (uint32_t) BIT24
#define SETBITS_PWD_CHG (uint32_t) BIT25
#define SETBITS_TS_CTRL (uint32_t) BIT26
#define SETBITS_TS_ZONE (uint32_t) BIT27
#define SETBITS_DM_PRD_NUM (uint32_t) BIT28
#define SETBITS_TMP_HOLIDAYS (uint32_t) BIT29
#define SETBITS_LPAVG_INTV (uint32_t) BIT30
#define SETBITS_sCURR_LMT2_VAL (uint32_t) BIT31

#define SETBITS_EXT_SCURR_1 (uint32_t) BIT5
#define SETBITS_HOLIDAY_SEL_1 (uint32_t) BIT6

#define PARSE_ARRAY(v)              \
    v = packed_ptr[packed_idx + 1]; \
    packed_idx += 2
#define PARSE_U8(v)                 \
    v = packed_ptr[packed_idx + 1]; \
    packed_idx += 2
#define PARSE_U16(v)                                \
    ToH16((U8_16*)&v, &packed_ptr[packed_idx + 1]); \
    packed_idx += 3

#define SUPPDSP_ITEM_SIZE 17
typedef struct
{
    uint8_t dummy[2 + MAX_SUPP_DSP_NUM * SUPPDSP_ITEM_SIZE];
} dsp_supply_set_type;

typedef struct _st_zcd_result_
{
    uint32_t time;
    uint16_t CRC_M;
} ST_ZCD_RESULT_TIME;

#define set_req_is_first_block() (appl_is_first_block)

extern prog_dl_type prog_dl;
extern int appl_set_save_idx;
extern uint8_t appl_set_save_result;
extern uint8_t* packed_ptr;
extern int packed_idx;

void approc_set_req(int idx);
bool prog_get_fut_dl(prog_dl_type* pdl);
int parse_ext_len_2(uint8_t* cp, uint16_t* extlen);
int size_ext_len_2(uint8_t* cp);
void hol_date_block_init(void);
disp_mode_type dspmode_from_suppdsp_set(uint8_t* obis, uint8_t attid);

uint8_t dsm_zcd_action_get_start_flag(void);
void dsm_zcd_action_set_start_flag(uint8_t val);

uint32_t dsm_touETC_suply_disp(uint8_t* p_val, uint16_t val_len);
uint32_t dsm_touETC_nperiod_billdate(uint16_t att_id, uint8_t* p_val,
                                     uint16_t val_len);
uint32_t dsm_touETC_daylight_saving_enabled(uint16_t att_id, uint8_t* p_val,
                                            uint16_t val_len);
uint32_t dsm_touETC_billing_parm(uint16_t att_id, uint8_t* p_val,
                                 uint16_t val_len);
uint32_t dsm_touETC_lcdset_parm(uint16_t att_id, uint8_t* p_val,
                                uint16_t val_len);
uint32_t dsm_touETC_period_billdate(uint16_t att_id, uint8_t* p_val,
                                    uint16_t val_len);
uint32_t dsm_touETC_lp_interval(uint16_t att_id, uint8_t* p_val,
                                uint16_t val_len);
uint32_t dsm_touETC_curr_last_demand(uint16_t att_id, uint8_t* p_val,
                                     uint16_t val_len);
uint32_t dsm_touETC_dm_month(uint16_t att_id, uint8_t* p_val, uint16_t val_len);
uint32_t dsm_touETC_contract_intv(uint16_t att_id, uint8_t* p_val,
                                  uint16_t val_len);
uint32_t dsm_touETC_dmsched_intv(uint32_t _which, uint16_t att_id,
                                 uint8_t* p_val, uint16_t val_len);
uint32_t dsm_touETC_daily_meas_sched(uint16_t att_id, uint8_t* p_val,
                                     uint16_t val_len);
uint32_t dsm_touETC_active_passive_time(uint16_t att_id, uint8_t* p_val,
                                        uint16_t val_len);
void dsm_touETC_holiday_sel(uint16_t att_id, uint8_t* p_val, uint16_t val_len);
#endif
