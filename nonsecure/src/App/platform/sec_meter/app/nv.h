
#ifndef NV_H
#define NV_H 1

#include "meter_app.h"
#include "eeprom_at24cm02.h"
#include "whm_1.h"
#include "disp.h"
#include "eoi.h"
#include "amg_sec.h"
#include "eob.h"
#include "appl.h"
#include "set_req.h"
#include "lp.h"
#include "whm_log.h"
#include "program.h"
#include "amg_imagetransfer.h"
#include "amg_stock_op_mode.h"
#include "amg_mtp_process.h"
#include "amg_push_datanoti.h"
#include "amg_modemif_prtl.h"
#include "amg_stock_op_mode.h"

#define NUM_CAP_WEAR 20

#define HOL_OF_BLOCK 0x00
#define HEAD_OF_BLOCK 0xff

extern pwf_date_type pwf_date_time;

typedef struct
{
    uint8_t ver;
    uint16_t CRC_M;
} nv_version_type;

// !!!!!!!!!!!!! P must +1 than A (for example, I_HOLIDAYS_P = I_HOLIDAYS_A+1)
// !!!!!!!!!!!!!!!
/* _A 현재, _P 예약, _DL 다운로드 */
typedef enum
{
    I_VERSION,
    I_VERSION1,
    I_ALL_CLEAR,
    I_MTINIT_LOG,
    I_SAG_FLAG,
    I_MT_CONFIG,
    I_WHM_OP,
    I_MT_ACCM,
    I_MT_READ_DATA,
    I_MAX_DEMAND,
    I_LOG_DATA,
    I_LOG_DATA1,
    I_LOG_SCURR,
    I_PROG_INFO,
    I_LP_DATA,
    I_LPAVG_DATA,
    I_SERIAL_NO,
    I_DEVICE_ID,
    I_CAL_NAME_ACTIVE,
    I_SEASON_PROFILE_ACTIVE,
    I_WEEK_PROFILE_ACTIVE,
    I_DAY_PROFILE_ACTIVE,
    I_CAL_NAME_PASSIVE,
    I_SEASON_PROFILE_PASSIVE,
    I_WEEK_PROFILE_PASSIVE,
    I_DAY_PROFILE_PASSIVE,
    I_ACT_PAS_TIME,
    I_CUST_NO,
    I_NP_BILLDATE,
    I_HOLIDAYS_A,
    I_HOLIDAYS_P,
    I_TMP_HOLIDAYS_A,
    I_TMP_HOLIDAYS_P,
    I_NP_BILLDATE_A,
    I_NP_BILLDATE_P,
    I_DISP_SUPP_A,
    I_DISP_SUPP_P,
    I_SUPPDSP_ITEM_A,
    I_SUPPDSP_ITEM_P,
    I_CUR_PROG_DL,
    I_FUT_PROG_DL,
    I_TX_SEG_FRAME,
    I_WHM_OP_CLR,
    I_SEASON_PROFILE_DL,
    I_WEEK_PROFILE_DL,
    I_DAY_PROFILE_DL,
    I_HOLIDAYS_DL,
    I_NP_BILLDATE_DL,
    I_DISP_SUPP_DL,
    I_SUPPDSP_ITEM_DL,
    I_TMP_HOLIDAYS_DL,
    I_WHM_LOG_LEN,
    I_WHM_LOG,
    I_ABS_ACCESS,
    I_BM_INP_QUE,
    I_RTC_COMP,
    I_BAT_USE,
    I_DR_LIMIT,
    I_MAG_CAL_DATA,
    I_TEMP_CAL_DATA,
    I_CAL_DATA,
    I_PFINF_PACCM,
    I_RCNT_DEMAND,
    I_CUR_PF_DATA,
    I_CUR_PF_DATA_CLEAR,
    I_RCNT_DM_CAP,
    I_CUR_DM_CAP,
    I_CUR_DATA_CAP,
    I_MT_RATE_CAP,
    I_LOG_CNT_CAP,
    I_UTIL_PASSWORD,
    I_485_PASSWORD,
    I_BAT_INST,
    I_IMAX_LOG,
    I_TOVER_LOG,
    I_PREPAY,
    I_SAG_SWELL,
    I_DM_INTV_DATA,
    I_DM_SUBLOCKS_DATA,
    I_JIG_PASSWORD,
    I_UNINIT_DATA,
    I_MIN_FREQ,
    I_MAX_FREQ,
#if 1 /* bccho, 2024-09-05, 삼상 */
    I_MIN_VOLT_L1,
    I_MAX_VOLT_L1,
    I_MIN_VOLT_L2,
    I_MAX_VOLT_L2,
    I_MIN_VOLT_L3,
    I_MAX_VOLT_L3,
#else
    I_MIN_VOLT,
    I_MAX_VOLT,
#endif
    I_LP_DATA_BLKED,
    I_DISP_SUPP_RCNT,
    I_SUPPDSP_ITEM_RCNT,
    I_ADJ_TEMP_DATA,
    I_MANUFACT_ID,
    I_TOU_SET_CNT,
    I_BACKUP_PROG_DL,
    I_HOL_DATE_BLOCK,
    I_DEVICE_ID_KEPCO,
    I_LOG_CERT_DATA,
    I_TOU_IMG_INFO,
    I_TOU_IMG_DATA,
    I_FW_IMG_DOWNLOAD_INFO,
    I_FW_IMG_INFO,
    I_SUB_FW_IMG_DOWNLOAD_INFO,
    I_SUB_FW_IMG_DATA,
    I_MT_READ_DATA_nPRD,
    I_MT_READ_DATA_SEASON,
    I_LPRT_DATA,
    I_RANDOM_SEED,
    I_MTP_CAL_POINT,
    I_MTP_PARM,
    I_MTP_SAG_SWELL,
    I_PUSH_ACTI_ERR,
    I_PUSH_SCRIPT_TABLE,
    I_PUSH_SETUP_TABLE,
    I_MT_RST_TIME,
    I_EXT_MODEM_ID,
    I_STOCK_OP_TIMES,
    I_MT_CONFIG_2,
    I_ZCD_RESULT_TIME,
    I_MODEM_BAUD,
    I_BOOT_AFTER_SWRST,
    I_CERTI_HASH,    /* bccho, 2024-09-04 */
    I_WORKPWR_FLAG,  // jp.kim 24.10.28
    I_PWF_DATE_TIME  // jp.kim 25.05.26
} nv_item_type;

typedef enum
{
    NV_OK,
    NV_WRITE_ERROR,
    NV_READ_ERROR,
    NV_VERIFY_READ_ERROR,
    NV_VERIFY_ERROR,
    NV_CRC_ERROR,
    NV_RANGE_ERROR,
    NV_UNDEF_ITEM_ERROR
} error_nv_type;

#define TX_SEG_FRAME_SIZE 600  // should be smaller than GLOBAL_BUFF_SIZE
typedef struct
{
    uint8_t seg_buf[TX_SEG_FRAME_SIZE];
} tx_seg_type;

typedef struct
{
    mr_ch_type accm[numRates];
    mr_dm_type dm[numRates];
    mr_data_info_type info;

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
    /* bccho, 2024-09-05, 삼상 */
    mr_ecdsa_type acc_ecdsa[numDirChs];
    mr_ecdsa_type dm_ecdsa[numDirChs];
#endif
} meter_read_type;

typedef struct
{
    holiday_date_type holidays[HOLIDAY_BLOCK_LEN];
    npbill_date_type np_billdate;
    dsp_supply_type dsp_supp;
    dsp_supply_set_type suppitem;
    season_date_type season_profile;
    week_date_type week_profile;
    dayid_table_type day_profile[DAY_PROF_SIZE];
} program_type;

typedef struct
{
    uint8_t cnt;
    log_data_type log;
    uint16_t CRC_M;
} mtinit_log_data_type;

typedef struct
{
    // 지울 수 없는 meter 고유 정보 영역
    // ===================================================
    nv_version_type nver;
    cal_data_type cal_data;
    mag_cal_data_type mag_cal;
    temp_cal_data_type temp_cal;
    temp_adj_data_type temp_adj;
    ser_no_type serno;
    device_id_type devid;
    manufact_id_type manfid;
    mtinit_log_data_type mtinit_log;
    latchon_data_type uninit_data;
    bat_used_time_type bat_use;
    bat_install_type bat_inst;
    device_id_type devid_kepco;
    uint8_t ds_cert_eeprom[980];  // jp.kim 25.05.26
    uint8_t pwf_date_time[20];    // jp.kim 25.05.26

    // 중요영역  ==========================================================
    ST_RANDOM random;
    ST_MTP_CAL_POINT mtp_calpoint;

    ST_MT_RST_TIME mt_rst_time;
    ST_MDM_ID ext_modem_id;
    ST_STOCK_OP_TIMES stock_op_times;
    uint8_t nv_dummy_11[1000];

    // program 영역   tou ================================================
    program_type pgm[NUM_PGM_AREA];
    uint8_t nv_dummy_02[1000];

    // 지우는 영역  1  ===================================================
    uint8_t init_start;
    uint8_t sag_flag[SAG_FLAG_LENGTH];
    tou_set_cnt_type tousetcnt;
    holiday_date_type holdateblock;
    mt_conf_type mtconf;
    whm_op_type whmop;
    mt_acc_type mtacc;
    meter_read_type mrinfo[NUM_MREADING_SAVE];  // 3 previous months + 1(bcz
                                                // power fail while nv-writing)
    uint8_t nv_dummy_19[sizeof(meter_read_type) * NUM_MREADING_SEASON_SAVE];
    max_demand_type mxdm;
    recent_demand_type rcntdm[NUM_CAP_WEAR];
    program_info_type proginfo[2];  // active, passive
    prog_dl_type pgm_dl_a;
    prog_dl_type pgm_dl_p;
    prog_dl_type pgm_dl_b;  // backup
    prepay_info_type prepay;
    imax_log_type imaxlog[3];
    tempover_log_type toverlog;
    sag_swell_info_type sag_swell;
    auth_pwd_type utilpwd;
    auth_pwd_type comm485pwd;
    rolling_dm_type rolldm;
    rolling_dm_rate_type sublock;
    rolling_dm_rate_type mr_rolldm[NUM_MREADING_SAVE];
    min_max_data_type _min_freq;
    min_max_data_type _max_freq;
#if 1 /* bccho, 2024-09-05, 삼상 */
    min_max_data_type _min_volt[3];
    min_max_data_type _max_volt[3];
#else
    min_max_data_type _min_volt;
    min_max_data_type _max_volt;
#endif

    Work_PwrF_data_type _work_pwr_flag;  // jp.kim 24.10.28
    uint8_t init_stop;
    uint8_t nv_dummy_31[1000]; /* bccho, 2024-09-04, 32 바이트 인증서 hash
                                  사용 */

    // 지우는 영역  2  ===================================================
    uint8_t init_extn_start;
    ST_MTP_PARM mtp_parm;
    ST_MTP_SAGSWELL mtp_sagswell;
    ST_PUSH_ACTI_ERRCODE push_act_err;
    ST_PUSH_SCRIPT_TABLE push_script_table;
    ST_PUSH_SETUP_TABLE push_setup_table;
    mt_conf_2_type mtconf_2;
    ST_ZCD_RESULT_TIME zcd_result_time;
    ST_AT_BAUD baud;
    ST_SWRST_BOOT_FLAG swrst_boot_flag;
    uint8_t init_extn_stop;
    uint8_t nv_dummy_03[1000 - (sizeof(mt_conf_2_type)) -
                        (sizeof(ST_ZCD_RESULT_TIME)) - (sizeof(ST_AT_BAUD)) -
                        (sizeof(ST_SWRST_BOOT_FLAG))];

    // data 영역 ===================================================
    meter_read_type curr_mr_cap[NUM_CAP_WEAR];     // I_CUR_DATA_CAP
    recent_demand_type curr_dm_cap[NUM_CAP_WEAR];  // I_CUR_DM_CAP
    recent_demand_type rcnt_dm_cap[NUM_CAP_WEAR];  // I_RCNT_DM_CAP
    rolling_dm_rate_type sublock_cap[NUM_CAP_WEAR];
    log_data_type
        log_data[numLogs];  // numLogs !!! ==> to include eLogSCurrNonSel
    log_data1_type log_data1;
    scurr_log_data_type log_data_scur;
    tx_seg_type tx_seg[NUM_CAP_WEAR];
    uint8_t nv_dummy_13[1000];  // jp.kim 24.10.28

    log_cert_data_type log_cert_data[eLogCert_MAX];
    ST_TOU_IMG_INFO tou_image_info;
    ST_TOU_IMG_DATA tou_image_data;
    ST_FW_INFO fw_info[FWINFO_MAX_NUM];
    ST_FW_IMG_DOWNLOAD_INFO fw_image_dlinfo;
    ST_SUB_FW_IMG_DL_INFO sub_fw_info;
    ST_SUB_FW_IMG_DL sub_fw_img;
    uint8_t nv_dummy_14[1000];

    meter_read_type
        mrinfo_nprd[NUM_MREADING_nPRD_SAVE];  // 3 previous months + 1(bcz power
                                              // fail while nv-writing)
    meter_read_type
        mrinfo_season[NUM_MREADING_SEASON_SAVE];  // 3 previous months + 1(bcz
                                                  // power fail while
                                                  // nv-writing)
    uint8_t nv_dummy_15[sizeof(meter_read_type) * NUM_MREADING_SEASON_SAVE * 2];

    lp_record_type lprec_blked[10];
    uint8_t page_gap2[_PAGE_SIZE];

    /************************************************************************/
    /* 1phs 3phs 각각의 저장공간 or feature 로 분리 컨셉 고려 및 적용 필요..*/
    /************************************************************************/
    uint8_t page_gap4[_PAGE_SIZE];
    lpavg_record_type
        lpavg_record[LPAVG_BUF_SIZE];  // 15 분 주기 5일 저장 buffer
    uint8_t nv_dummy_16[sizeof(lpavg_record_type) * LPAVG_BUF_SIZE];

    /************************************************************************/
    /* 1phs 3phs 각각의 저장공간 or feature 로 분리 컨셉 고려 및 적용 필요..*/
    /************************************************************************/
    uint8_t page_gap5[_PAGE_SIZE];
    lprt_record_3phs lprt_record[LPRT_BUF_SIZE];
    uint8_t nv_dummy_17[sizeof(lprt_record_3phs) * LPRT_BUF_SIZE];
    uint8_t nv_dummy_24[1000];

    uint8_t nv_dummy_lp_start[3000];
    uint8_t page_gap3[_PAGE_SIZE];
    uint8_t lp_recstart;  // LP record start
} Nv_type;

typedef struct
{
    uint16_t offset;
    uint16_t len;
} seg_acs_type;

typedef struct
{
    uint16_t idx;
    uint8_t len;
    uint8_t rec_siz;
} rel_acs_type;

typedef enum
{
    eMrAccm,
    eMrDm,
    eMrInfo,

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
    /* bccho, 2024-09-05, 삼상 */
    eMrAccmEcdsa,
    eMrDmEcdsa,
#endif

    eMrSublocks
} mr_data_sel_type;

typedef struct
{
    rate_type rt;
    mr_data_sel_type sel;
    demand_ch_type chsel;
} cur_mr_acs_type;

typedef struct
{
    uint16_t mrcnt;
    rate_type rt;

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
    /* bccho, 2024-09-05, 삼상 */
    energy_dir_type dir;
#endif

    mr_data_sel_type sel;
    demand_ch_type chsel;
} mr_acs_type;

typedef struct
{
    uint8_t dl_idx;
    uint16_t offset;
    uint8_t len;
} set_acs_type;

typedef struct
{
    uint32_t addr;
    uint16_t len;
} abs_acs_type;

typedef union
{
    uint8_t ch[4];
    uint16_t i16[2];
    seg_acs_type seg;
    rel_acs_type rel;
    cur_mr_acs_type cur;
    mr_acs_type mr;
    set_acs_type set;
    abs_acs_type abs;
} nv_sub_info_type;

typedef struct
{
    lprt_record_3phs lprt_record[LPRT_BUF_SIZE];
} ram_type;

#define nv_read(a, b) nv_proc((a), (b), false)
#define nv_write(a, b) nv_proc((a), (b), true)

#define ram_read(a, b) ram_proc((a), (b), false)
#define ram_write(a, b) ram_proc((a), (b), true)

extern nv_sub_info_type nv_sub_info;

extern nv_sub_info_type ram_sub_info;

bool nv_proc(nv_item_type nv_item, uint8_t *nv_data, bool wr);
void inc_idx_cap_wear(void);
void set_prog_dl_idx(void);
uint8_t get_prog_dl_idx(void);
void inc_rcntdm_wear_idx(void);
void set_hol_dl_idx(void);
uint8_t get_hol_dl_idx(void);

bool ram_proc(nv_item_type ram_item, uint8_t *ram_data, bool wr);

#endif
