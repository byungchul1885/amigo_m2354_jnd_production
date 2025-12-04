#include "main.h"
#include "options.h"
#if 0 /* bccho, FLASH, 2023-07-15 */
#include "flash.h"
#endif /* bccho */
#include "comm.h"
#include "dlms_todo.h"
#include "aarq.h"
#include "get_req.h"
#include "meter_app.h"
#include "tmp.h"
#include "disp.h"
#include "whm_1.h"
#include "lp.h"
#include "nv.h"
#include "dl.h"
#include "approc.h"
#include "act_req.h"
#include "amg_imagetransfer.h"
#include "amg_sec.h"
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
#include "kse100_stm32l4.h"
#endif /* bccho */
#include "appl.h"
#include "amg_push_datanoti.h"
#include "amg_rtc.h"
#include "amg_stock_op_mode.h"
#include "amg_media_mnt.h"
#include "amg_wdt.h"

#define _D "[GREQ] "

#define TO_ASCII_H(x) (((x) >> 4) > 9) ? ((x) >> 4) + 0x37 : ((x) >> 4) + 0x30
#define TO_ASCII_L(x) \
    (((x) & 0x0F) > 9) ? ((x) & 0xF) + 0x37 : ((x) & 0xF) + 0x30

ST_MIF_SAGSWELL_SETUP* dsm_mtp_get_sagswell(void);
extern ST_PUSH_ACTI_ERRCODE gst_push_acti;

void dsm_data_noti_errcode_evt_send(void);
void amr_send_frame(uint32_t poll_flag);
float get_lpavg_ltol(U8 line);

#define CUR_CAP_UPDATE_SECS_MAX (30)

typedef struct _st_cur_cap_op_decision_info_
{
    uint32_t pre_obj_id;
    uint32_t pre_seconds;
    uint8_t gf;
} ST_CUR_CAP_OP_DECISION_INFO;

// jp.kim 25.01.22
bool at_cmd_rsp_ing = 0;
uint32_t getresp_LP_index;
uint16_t getresp_LP_len;
#if 1 /* bccho, 2024-09-05, 삼상 */
static uint32_t getresp_LP_entry_sels;
#else
static uint16_t getresp_LP_entry_sels;
#endif
static uint32_t getresp_LPrt_entry_sels;
static uint8_t getresp_LP_entry_sels_no;

whm_info_cap_type whm_info_cap;
float lp_intv_pf_cap[numDirChs];

extern ST_ATCMD_TMP_BUF gst_atcmd_from_modem[MAX_MODEM_TYPE];

bool set_test_block_rx = 0;
bool lpinfo_is_caped;
bool mt_uni_dir;
uint8_t mt_dir_for_lp;
uint8_t recsize_for_lp;
uint16_t lpsize_for_lp;
uint16_t buffsize_for_lp;
uint32_t lpindex_for_lp;
ST_CUR_CAP_OP_DECISION_INFO gst_cur_op_cap_info;

void ob_cert_log_case(elog_cert_kind_type elog);
static void approc_fill_get_atcmd_resp_block(bool first);
static void fill_register_scale(int8_t scaler, uint8_t unit);
static void ob_zcrs_sig_out_durtime(void);
static void ob_zcrs_sig_out_cmpstime(void);
static void ob_zcrs_sig_out_resulttime(void);
static void ob_self_error_ref(void);
static void ob_tmsync_range(void);

static void approc_get_req_normal(int idx);
void approc_get_req_block(int idx);
static void approc_fill_get_resp_normal(void);
static void approc_fill_get_resp_block(bool first);
static void approc_get_req_capture(void);
static void get_LP_selcolmn_info(uint8_t from, uint8_t to);
static void get_LPavg_selcolmn_info(uint8_t from, uint8_t to);
static void fill_octet_string_x(uint8_t* str, int len);
static void fill_clock_obj(date_time_type* dt);
static void LP_record_to_pPdu(uint8_t* recbuff, uint8_t len);
static void fill_lp_record(void);
static void fill_lpavg_record(void);
static void LPavg_record_to_pPdu(uint8_t* recbuff, uint8_t len);
static int16_t get_clk_dev(void);
static void fill_day_prof_struct(bool first, bool curr);
static void fill_holidays_struct(bool first, bool curr);
static void if_not_captured_capture(void);
void dsm_cur_cap_op_decision_info_init(void);
ST_CUR_CAP_OP_DECISION_INFO* dsm_cur_op_decision_info_get(void);
bool approc_is_valid_cur_capture_others(void);
void approc_get_req_capture_always(void);
void approc_get_req_capture_others(void);
void obj_id_secs_backup(void);
static void fill_prog_name(bool curprog, uint8_t* name, uint8_t* tptr);
static uint16_t get_log_cnt(elog_kind_type elog, uint8_t* tptr);
static void fill_cert_log_cap(elog_cert_kind_type elog);
static void get_LPrt_selcolmn_info(uint8_t from, uint8_t to);
static void fill_lprt_record(void);
static void LPrt_record_to_pPdu(uint8_t* recbuff, uint8_t len);

static void ob_working_fault_min(void);
static void ob_tou_id_change_sts(void);
static void ob_sys_title_server(void);
static void ob_inst_cert(void);
static void ob_inst_key(void);
static void ob_ass_LN(void);
static void ob_load_profile(void);
static void ob_month_energy(energy_dir_type dir);
static void ob_month_maxdm(energy_dir_type dir);
static void ob_counter_billing(void);
static void ob_num_avail_billing(void);
static void ob_custom_id(void);
static void ob_manuf_id(void);
static void ob_manuf_id_old(void);
static void ob_prog_id(void);
static void ob_mtconst_active(void);
static void ob_mtconst_reactive(void);
static void ob_mtconst_app(void);
static void ob_user_mode_disp(void);
static void ob_supp_mode_disp(void);
static void ob_pvt_mode_disp(void);
static void ob_period_billdate(void);
static void ob_nperiod_billdate(void);
static void ob_sig_sel(void);
static void ob_lp_interval(void);
static void ob_holiday_sel(void);
static void ob_lpavg_interval(void);
static void ob_time_billing(void);
static void ob_tou_cal(void);
static void ob_holidays(void);
static void ob_hdlc_setup(void);
static void ob_date_time(void);
static void ob_lcdset_parm(void);
static void ob_local_time(void);
static void ob_local_date(void);
static void ob_curr_tariff(void);
static void ob_billing_parm(void);
static void ob_selective_act(void);
static void ob_rLOAD_sig(void);
static void ob_log_cnt(elog_kind_type elog);
static void ob_sCURR_autortn_val(void);
static void ob_sCURR_limit(void);
static void ob_sCURR_limit_2(void);
static void ob_inst_profile(void);
static void ob_inst_power(U8 line);
static void ob_inst_freq(void);
static void ob_inst_curr(uint8_t line);
static void ob_inst_volt(uint8_t line);
static void ob_inst_pf(uint8_t line);
static void ob_inst_phase(uint8_t line);
static void ob_inst_volt_THD(uint8_t line);
static void ob_inst_vphase(uint8_t sel);
static void ob_curr_temp(void);
static void ob_evt_log(elog_kind_type elog);
static void ob_lp_status(void);
static void ob_time_bat_use(void);
static void ob_time_bat_inst(void);
static void ob_lpavg(void);
static void ob_rate_pf(energy_dir_type dir);
static void ob_cum_demand(void);
static void ob_max_demand(void);
static void ob_rate_energy(void);
static void ob_rate_both_react_energy(energy_dir_type dir);
static void ob_fut_pgm_chg(void);
static void ob_pgm_chg_num(void);
static void ob_last_pgm_chg(void);
static void ob_temp_thrshld(void);
static void ob_temp_over(void);
static void ob_sag_swell(uint8_t kind);
static void ob_avg_curr(uint8_t line);
static void ob_avg_volt(uint8_t line);
static void ob_avg_volt_ltol(U8 line);
static void ob_imax_log(uint8_t line);
static void ob_prepay_remenergy(void);
static void ob_prepay_buyenergy(void);
static void ob_prepay_enable(void);
static void ob_prepay_loadlimit_cancel(void);
static void ob_condensor_inst(void);
static void ob_last15_pf(energy_dir_type dir);
static void ob_curr_last_demand(void);
static void ob_month_sublocks(void);
static void ob_ts_conf(void);
static void ob_sel_react(void);
static void ob_lp_overlaped_index(void);
static void ob_comm_enable(void);
static void ob_key_value(void);
static void ob_lcd_map(void);
static void ob_scurr_hold(void);
static void ob_scurr_rtn_n1(void);
static void ob_scurr_rtn_n2(void);
static void ob_scurr_cnt_n1(void);
static void ob_latchon_counter(void);
static void ob_min_freq(void);
static void ob_max_freq(void);
#if 1 /* bccho, 2024-09-05, 삼상 */
static void ob_min_volt(uint8_t idx);
static void ob_max_volt(uint8_t idx);
#else
static void ob_min_volt(void);
static void ob_max_volt(void);
#endif
static void ob_overcurr_enable(void);
static void ob_tou_set_cnt(void);
static void ob_ext_prog_id(void);
static void ob_realtime_p_energy(void);
static void ob_realtime_p_load_profile(void);
static void ob_realtime_lp_interval(void);
static void ob_run_modem_info(void);
static void ob_use_mr_data_num(void);
static void ob_nms_dms_id(void);
static void ob_sap_assignment(void);
void ob_security_setup(void);
static void ob_cert_log_cnt(elog_cert_kind_type elog);
static void ob_evt_cert_log(elog_cert_kind_type elog);
static void ob_evt_tou_imagetransfer(void);
static void ob_evt_fw_info(void);
static void ob_evt_fw_apply_date(void);
static void ob_evt_fw_imagetransfer(void);
static void ob_event_info_profile(void);
static void ob_evt_err_code_activate(uint8_t grp_e);
static void ob_evt_push_script(void);
static void ob_evt_push_setup_err_code(void);
static void ob_evt_push_setup_lastLP(void);
static void ob_ext_modem_id(void);
static void ob_modem_atcmd_forSet(uint8_t modem_type);
static void ob_modem_atcmd_forRsp(uint8_t modem_type);
static void ob_stock_op_times(void);
static void ob_lp_tatal_cnt(void);
static void ob_magnet_dur_time(void);
static void ob_max_demand_sign(void);
static void ob_energy_sign(void);
uint8_t clock_to_format_for_sign(date_time_type* dt, uint8_t* p_odata);
static void ob_adj_factor(void);

#define LP_SORTOBJ_SIZE 18
static const /*__code*/ uint8_t LP_sort_object[LP_SORTOBJ_SIZE] = {
    0x02, 0x04,                       // struct
    0x12, 0x00, 0x08,                 // clock class id(long-unsigned : 18)
    0x09, 0x06, OBIS_DATE_TIME_nobr,  // logical name (octet string : 9) =>clock
    0x0f, 0x02,                       // attribute id(integer : 15)
    0x12, 0x00, 0x00  // data_index(long-unsigned : 18) => no meaning
};

#define LP_CAPOBJ_SIZE_UNI (2 + 6 * 18)
static const /*__code*/ uint8_t LP_capture_objects_uni[LP_CAPOBJ_SIZE_UNI] =
    {  // array_tag(2) + array*18
        0x01,
        0x06,  // array
        0x02,
        0x04,  // struct
        0x12,
        0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09,
        0x06,
        OBIS_ENERGY_FWD_ACT_nobr,  // logical name (octet string : 9)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12,
        0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,
        0x12,
        0x00,
        0x03,
        0x09,
        0x06,
        OBIS_ENERGY_FWD_LAG_REACT_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00,
        0x02,
        0x04,
        0x12,
        0x00,
        0x03,
        0x09,
        0x06,
        OBIS_ENERGY_FWD_LEAD_REACT_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00,
        0x02,
        0x04,
        0x12,
        0x00,
        0x03,
        0x09,
        0x06,
        OBIS_ENERGY_FWD_APP_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00,
        0x02,
        0x04,
        0x12,
        0x00,
        0x08,
        0x09,
        0x06,
        OBIS_DATE_TIME_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00,
        0x02,
        0x04,
        0x12,
        0x00,
        0x01,
        0x09,
        0x06,
        OBIS_LP_STATUS_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00};

#define LP_CAPOBJ_SIZE_BOTH (2 + 11 * 18)

static const /*__code*/ uint8_t LP_capture_objects_both[LP_CAPOBJ_SIZE_BOTH] =
    {  // array_tag(2) + array*18
        0x01, 0x0b,
        // data_index(long-unsigned : 18) => no meaning
        0x02, 0x04, 0x12, 0x00, 0x01, 0x09, 0x06, OBIS_LP_OVERLAPED_INDEX_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00,
        // array
        0x02, 0x04,        // struct
        0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_ENERGY_FWD_ACT_nobr,  // logical name (octet string : 9)
        0x0f, 0x02,                // attribute id(integer : 15)
        0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06,
        OBIS_ENERGY_FWD_LAG_REACT_nobr, 0x0f, 0x02, 0x12, 0x00, 0x00, 0x02,
        0x04, 0x12, 0x00, 0x03, 0x09, 0x06, OBIS_ENERGY_FWD_LEAD_REACT_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00, 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06,
        OBIS_ENERGY_FWD_APP_nobr, 0x0f, 0x02, 0x12, 0x00, 0x00, 0x02, 0x04,
        0x12, 0x00, 0x08, 0x09, 0x06, OBIS_DATE_TIME_nobr, 0x0f, 0x02, 0x12,
        0x00, 0x00, 0x02, 0x04, 0x12, 0x00, 0x01, 0x09, 0x06,
        OBIS_LP_STATUS_nobr, 0x0f, 0x02, 0x12, 0x00, 0x00, 0x02, 0x04, 0x12,
        0x00, 0x03, 0x09, 0x06, OBIS_ENERGY_BWD_ACT_nobr, 0x0f, 0x02, 0x12,
        0x00, 0x00, 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06,
        OBIS_ENERGY_BWD_LEAD_REACT_nobr, 0x0f, 0x02, 0x12, 0x00, 0x00, 0x02,
        0x04, 0x12, 0x00, 0x03, 0x09, 0x06, OBIS_ENERGY_BWD_LAG_REACT_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00, 0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06,
        OBIS_ENERGY_BWD_APP_nobr, 0x0f, 0x02, 0x12, 0x00, 0x00};

#define LPavg_CAP_SIZE (2 + 5 * 18)
static const /*__code*/ uint8_t LPavg_capture_objects[LPavg_CAP_SIZE] = {
    // array_tag(2) + array*18
    0x01,
    0x05,  // array
    0x02,
    0x04,  // struct
    0x12,
    0x00,
    0x08,  // register class id(long-unsigned : 18)
    0x09,
    0x06,
    OBIS_DATE_TIME_nobr,  // logical name (octet string : 9)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12,
    0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_VOLT_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_voltTHD_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_CURR_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_PHASE_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
};

#if 1 /* bccho, 2024-09-05, 삼상 */
#define LPavg_3PHS_CAP_SIZE (2 + 19 * 18)
static const U8 LPavg_3PHS_capture_objects[LPavg_3PHS_CAP_SIZE] = {
    // array_tag(2) + array*18
    0x01,
    0x13,
#else
#define LPavg_3PHS_CAP_SIZE (2 + 16 * 18)
static const uint8_t LPavg_3PHS_capture_objects[LPavg_3PHS_CAP_SIZE] = {
    // array_tag(2) + array*18
    0x01,
    0x10,  // array
#endif
    0x02,
    0x04,  // struct
    0x12,
    0x00,
    0x08,  // register class id(long-unsigned : 18)
    0x09,
    0x06,
    OBIS_DATE_TIME_nobr,  // logical name (octet string : 9)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12,
    0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_VOLT_L1_L2_nobr,  // a-b 선간 전압.
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_VOLT_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_voltTHD_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_CURR_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_PHASE_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
#if 1 /* bccho, 2024-09-05, 삼상 */
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_ERROR_RATE_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
#endif
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_VOLT_L2_L3_nobr,  // b-c 선간 전압.
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_VOLT_L2_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_voltTHD_L2_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_CURR_L2_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_PHASE_L2_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,

#if 1 /* bccho, 2024-09-05, 삼상 */
    0x09,
    0x06,
    OBIS_ERROR_RATE_L2_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_VOLT_L3_L1_nobr,  // c-a 선간 전압.
#else
    0x09,
    0x06,
    OBIS_AVG_VOLT_L2_L3_nobr,  // c-a 선간 전압.
#endif

    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_VOLT_L3_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_voltTHD_L3_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_AVG_CURR_L3_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_PHASE_L3_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
#if 1 /* bccho, 2024-09-05, 삼상 */
    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_ERROR_RATE_L3_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
#endif
};

#define LPrt_CAP_SIZE (2 + 2 * 18)
static const /*__code*/ uint8_t LPrt_capture_objects[LPrt_CAP_SIZE] = {
    // array_tag(2) + array*18
    0x01,
    0x02,  // array
    0x02,
    0x04,  // struct
    0x12,
    0x00,
    0x08,  // register class id(long-unsigned : 18)
    0x09,
    0x06,
    OBIS_DATE_TIME_nobr,  // logical name (octet string : 9)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12,
    0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,
    0x12,
    0x00,
    0x01,
    0x09,
    0x06,
    OBIS_RTIME_P_ENERGY_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
};

#define INSTPROF_CAPOBJ_SIZE (2 + 9 * 18)
static const uint8_t INST_PROF_capture_objects[INSTPROF_CAPOBJ_SIZE] = {
    0x01,
    0x09,  // array

    0x02,
    0x04,  // struct
    0x12,
    0x00,
    0x08,  // clock class id(long-unsigned : 18)
    0x09,
    0x06,
    OBIS_DATE_TIME_nobr,  // logical name (octet string : 9)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12,
    0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_POWER_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_VOLT_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_voltTHD_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_CURR_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_PF_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_PHASE_L1_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_INST_FREQ_nobr,  // freq
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,

    0x02,
    0x04,
    0x12,
    0x00,
    0x03,
    0x09,
    0x06,
    OBIS_CURR_TEMP_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
};

#if 1 /* bccho, 2024-09-05, 삼상 */
#define INSTPROF_3PHS_CAPOBJ_SIZE (2 + 27 * 18)
#else
#define INSTPROF_3PHS_CAPOBJ_SIZE (2 + 24 * 18)
#endif
static const uint8_t INST_PROF_3PHS_capture_objects[INSTPROF_3PHS_CAPOBJ_SIZE] =
    {0x01,
     0x18,

     0x02,
     0x04,
     0x12,
     0x00,
     0x08,
     0x09,
     0x06,
     OBIS_DATE_TIME_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_POWER_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_VOLT_L1_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_voltTHD_L1_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_CURR_L1_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_PF_L1_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_PHASE_L1_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
#if 1 /* bccho, 2024-09-05, 삼상 */
     0x09,
     0x06,
     OBIS_ERROR_RATE_L1_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,
     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
#endif
     0x09,
     0x06,
     OBIS_INST_POWER_L2_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_VOLT_L2_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_voltTHD_L2_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_CURR_L2_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_PF_L2_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_PHASE_L2_nobr,
#if 1 /* bccho, 2024-09-05, 삼상 */
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,
     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_ERROR_RATE_L2_nobr,
#endif

     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_POWER_L3_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_VOLT_L3_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_voltTHD_L3_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_CURR_L3_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_PF_L3_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_PHASE_L3_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
#if 1 /* bccho, 2024-09-05, 삼상 */
     0x09,
     0x06,
     OBIS_ERROR_RATE_L3_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
#endif
     0x09,
     0x06,
     OBIS_INST_CURR_N_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_PHASE_L1_2_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_PHASE_L1_3_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_INST_FREQ_nobr,  // freq
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00,

     0x02,
     0x04,
     0x12,
     0x00,
     0x03,
     0x09,
     0x06,
     OBIS_CURR_TEMP_nobr,
     0x0f,
     0x02,
     0x12,
     0x00,
     0x00};

#define ENERGY_CAPOBJ_SIZE (452 + 18 * 2 + 18 * 2)
static const /*__code*/ uint8_t deliENERGY_capture_objects[ENERGY_CAPOBJ_SIZE] =
    {
        0x01,
        0x1c,  // array
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x00, 0x01, 0x02,
        0xff,  // logical name (octet string : 9) =>clock
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
        0xff,  // logical name (octet string : 9) =>manuf id
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x01, 0x08, 0x00,
        0xff,  // logical name (octet string : 9) => Total active energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x09, 0x08, 0x00,
        0xff,  // logical name (octet string : 9) => Total apparent
               // energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x05, 0x08, 0x00,
        0xff,  // logical name (octet string : 9) => Total reactive (Q1)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x08, 0x08, 0x00,
        0xff,  // logical name (octet string : 9) => Total reactive (Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0d, 0x09, 0x00,
        0xff,  // logical name (octet string : 9) => Total Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x01, 0x08, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 active
               // energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x09, 0x08, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 apparent
               // energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x05, 0x08, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q1)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x08, 0x08, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0d, 0x09, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x01, 0x08, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 active
               // energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x09, 0x08, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 apparent
               // energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x05, 0x08, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q1)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x08, 0x08, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0d, 0x09, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x01, 0x08, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 active
               // energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x09, 0x08, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 apparent
               // energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x05, 0x08, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q1)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x08, 0x08, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0d, 0x09, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x01, 0x08, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 active
               // energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x09, 0x08, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 apparent
               // energy(Q1+Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x05, 0x08, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q1)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x08, 0x08, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q4)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0d, 0x09, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00  // data_index(long-unsigned : 18) => no meaning

        ,
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x80, 0x08, 0x80,
        0xff,  // logical name (octet string : 9) => sign (period, deli, energy)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00  // data_index(long-unsigned : 18) => no meaning
};

static const /*__code*/ uint8_t
    receiENERGY_capture_objects[ENERGY_CAPOBJ_SIZE] = {
        0x01,
        0x1c,  // array
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x00, 0x01, 0x02,
        0xff,  // logical name (octet string : 9) =>clock
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
        0xff,  // logical name (octet string : 9) =>manuf id
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x08, 0x00,
        0xff,  // logical name (octet string : 9) => Total active energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x08, 0x00,
        0xff,  // logical name (octet string : 9) => Total apparent
               // energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x06, 0x08, 0x00,
        0xff,  // logical name (octet string : 9) => Total reactive (Q2)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x07, 0x08, 0x00,
        0xff,  // logical name (octet string : 9) => Total reactive (Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x54, 0x09, 0x00,
        0xff,  // logical name (octet string : 9) => Total Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x08, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 active
               // energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x08, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 apparent
               // energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x06, 0x08, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q2)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x07, 0x08, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x54, 0x09, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1 Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x08, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 active
               // energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x08, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 apparent
               // energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x06, 0x08, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q2)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x07, 0x08, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x54, 0x09, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2 Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x08, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 active
               // energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x08, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 apparent
               // energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x06, 0x08, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q2)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x07, 0x08, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x54, 0x09, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3 Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x08, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 active
               // energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x08, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 apparent
               // energy(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x06, 0x08, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q2)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x07, 0x08, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x54, 0x09, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4 Average PF
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00  // data_index(long-unsigned : 18) => no meaning

        ,
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x81, 0x08, 0x80,
        0xff,  // logical name (octet string : 9) => sign (period, rece (송揚?,
               // energy)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00  // data_index(long-unsigned : 18) => no meaning
};

static const uint8_t deliENERGY_nprd_capture_objects[ENERGY_CAPOBJ_SIZE] = {
    0x01,
    0x1c,  // array

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x00, 0x01, 0x02,
    0xff,  // logical name (octet string : 9) =>billing time
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
    0xff,  // logical name (octet string : 9) =>manuf id
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x05, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x08, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0d, 0x09, 0x00,
    0xff,  // logical name (octet string : 9) => Total Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x05, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x08, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0d, 0x09, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x05, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x08, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0d, 0x09, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x05, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x08, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0d, 0x09, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x05, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x08, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0d, 0x09, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning

    ,
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x80, 0x08, 0x80,
    0xff,  // logical name (octet string : 9) => sign (period, deli (수揚?,
           // energy)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
};

static const uint8_t receiENERGY_nprd_capture_objects[ENERGY_CAPOBJ_SIZE] = {
    0x01,
    0x1c,  // array

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x00, 0x01, 0x02,
    0xff,  // logical name (octet string : 9) =>billing time
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
    0xff,  // logical name (octet string : 9) =>manuf id
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x06, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x07, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x54, 0x09, 0x00,
    0xff,  // logical name (octet string : 9) => Total Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x06, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x07, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x54, 0x09, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x06, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x07, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x54, 0x09, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x06, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x07, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x54, 0x09, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x06, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x07, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x54, 0x09, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
    ,
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x81, 0x08, 0x80,
    0xff,  // logical name (octet string : 9) => sign (period, rece (송揚?,
           // energy)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
};

static const uint8_t deliENERGY_season_capture_objects[ENERGY_CAPOBJ_SIZE] = {
    0x01,
    0x1c,  // array

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x00, 0x01, 0x02,
    0xff,  // logical name (octet string : 9) =>billing time
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
    0xff,  // logical name (octet string : 9) =>manuf id
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x05, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x08, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0d, 0x09, 0x00,
    0xff,  // logical name (octet string : 9) => Total Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x05, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x08, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0d, 0x09, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x05, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x08, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0d, 0x09, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x05, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x08, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0d, 0x09, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 active energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 apparent energy(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x05, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q1)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x08, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0d, 0x09, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning

    ,
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x80, 0x08, 0x80,
    0xff,  // logical name (octet string : 9) => sign (period, rece (송揚?,
           // energy)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
};

static const uint8_t receiENERGY_season_capture_objects[ENERGY_CAPOBJ_SIZE] = {
    0x01,
    0x1c,  // array

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x00, 0x01, 0x02,
    0xff,  // logical name (octet string : 9) =>billing time
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
    0xff,  // logical name (octet string : 9) =>manuf id
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x06, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x07, 0x08, 0x00,
    0xff,  // logical name (octet string : 9) => Total reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x54, 0x09, 0x00,
    0xff,  // logical name (octet string : 9) => Total Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x06, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x07, 0x08, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x54, 0x09, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x06, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x07, 0x08, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x54, 0x09, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x06, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x07, 0x08, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x54, 0x09, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 active energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 apparent energy(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x06, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q2)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x07, 0x08, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 reactive (Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x54, 0x09, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 Average PF
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning

    ,
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x81, 0x08, 0x80,
    0xff,  // logical name (octet string : 9) => sign (period, rece (송揚?,
           // energy)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
};

#define MAXDM_CAPOBJ_SIZE (2 + 2 * 18 + 31 * 18)
// 수전
static const /*__code*/ uint8_t deliMAXDM_capture_objects[MAXDM_CAPOBJ_SIZE] = {
    0x01,
    0x21,  // array

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x00, 0x01, 0x02,
    0xff,  // logical name (octet string : 9) =>clock
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
    0xff,  // logical name (octet string : 9) =>manuf id
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total active power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total active power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x02, 0x00,
    0xff,  // logical name (octet string : 9) => Total Cum. active power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x02, 0x00,
    0xff,  // logical name (octet string : 9) => Total Cum. apparent
           // power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  active power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  active power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x02, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  Cum. active
           // power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  apparent power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  apparent power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x02, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  Cum. apparent
           // power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  active power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  active power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x02, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  Cum. active
           // power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  apparent power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  apparent power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x02, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  Cum. apparent
           // power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  active power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  active power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x02, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  Cum. active
           // power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  apparent power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  apparent power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x02, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  Cum. apparent
           // power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  active power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  active power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x01, 0x02, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  Cum. active
           // power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  apparent power(Q1+Q4)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  apparent power(Q1+Q4)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x09, 0x02, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  Cum. apparent
           // power(Q1+Q4)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
    ,
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x01, 0x80, 0x06, 0x80,
    0xff,  // logical name (octet string : 9) => sign (period, rece (송揚?,
           // energy)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
};
// 송전 end
static const /*__code*/ uint8_t receiMAXDM_capture_objects[MAXDM_CAPOBJ_SIZE] =
    {
        0x01,
        0x21,  // array

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x00, 0x01, 0x02,
        0xff,  // logical name (octet string : 9) =>clock
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
        0xff,  // logical name (octet string : 9) =>manuf id
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x00,
        0xff,  // logical name (octet string : 9) => Total active power(Q2+Q3)
               // att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x00,
        0xff,  // logical name (octet string : 9) => Total active power(Q2+Q3)
               // att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x02, 0x00,
        0xff,  // logical name (octet string : 9) => Total Cum. active
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x00,
        0xff,  // logical name (octet string : 9) => Total apparent power(Q2+Q3)
               // att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x00,
        0xff,  // logical name (octet string : 9) => Total apparent power(Q2+Q3)
               // att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x02, 0x00,
        0xff,  // logical name (octet string : 9) => Total Cum. apparent
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1  active
               // power(Q2+Q3) att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1  active
               // power(Q2+Q3) att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x02, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1  Cum. active
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1  apparent
               // power(Q2+Q3) att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1  apparent
               // power(Q2+Q3) att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x02, 0x01,
        0xff,  // logical name (octet string : 9) => Tariff 1  Cum. apparent
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2  active
               // power(Q2+Q3) att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2  active
               // power(Q2+Q3) att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x02, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2  Cum. active
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2  apparent
               // power(Q2+Q3) att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2  apparent
               // power(Q2+Q3) att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x02, 0x02,
        0xff,  // logical name (octet string : 9) => Tariff 2  Cum. apparent
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3  active
               // power(Q2+Q3) att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3  active
               // power(Q2+Q3) att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x02, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3  Cum. active
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3  apparent
               // power(Q2+Q3) att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3  apparent
               // power(Q2+Q3) att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x02, 0x03,
        0xff,  // logical name (octet string : 9) => Tariff 3  Cum. apparent
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4  active
               // power(Q2+Q3) att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x06, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4  active
               // power(Q2+Q3) att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x02, 0x02, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4  Cum. active
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4  apparent
               // power(Q2+Q3) att_id = 2
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x04,  // extended register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x06, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4  apparent
               // power(Q2+Q3) att_id = 5
        0x0f,
        0x05,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x0a, 0x02, 0x04,
        0xff,  // logical name (octet string : 9) => Tariff 4  Cum. apparent
               // power(Q2+Q3)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00  // data_index(long-unsigned : 18) => no meaning
        ,
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06, 0x01, 0x01, 0x81, 0x06, 0x80,
        0xff,  // logical name (octet string : 9) => sign (period, rece (송揚?,
               // energy)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00  // data_index(long-unsigned : 18) => no meaning
};

// 수전
static const uint8_t deliMAXDM_nprd_capture_objects[MAXDM_CAPOBJ_SIZE] = {
    0x01, 0x21,        // array
    0x02, 0x04,        // struct
    0x12, 0x00, 0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x00, 0x01, 0x02,
    0xff,              // logical name (octet string : 9) =>clock
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
    0xff,              // logical name (octet string : 9) =>manuf id
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x00,
    0xff,              // logical name (octet string : 9) => Total active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x00,
    0xff,              // logical name (octet string : 9) => Total active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x02, 0x00,
    0xff,              // logical name (octet string : 9) => Total Cum. active
                       // power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x00,
    0xff,              // logical name (octet string : 9) => Total apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x00,
    0xff,              // logical name (octet string : 9) => Total apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x02, 0x00,
    0xff,              // logical name (octet string : 9) => Total Cum. apparent
                       // power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x02, 0x01,
    0xff,              // logical name (octet string : 9) => Total Cum. active
                       // power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x02, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  Cum.
                       // apparent power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x02, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2 Cum.
                       // active power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x02, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  Cum.
                       // apparent power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x02, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3 Cum.
                       // active power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x02, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  Cum.
                       // apparent power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x06, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x01, 0x02, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4 Cum.
                       // active power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x06, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x09, 0x02, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  Cum.
                       // apparent power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02, 0x04,        // struct
    0x12, 0x00, 0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x80, 0x06, 0x80,
    0xff,             // logical name (octet string : 9) => sign (period, rece
                      // (송揚?, energy)
    0x0f, 0x02,       // attribute id(integer : 15)
    0x12, 0x00, 0x00  // data_index(long-unsigned : 18) => no meaning

};

// 송전
static const uint8_t receiMAXDM_nprd_capture_objects[MAXDM_CAPOBJ_SIZE] = {

    0x01,
    0x21,  // array

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x00, 0x01, 0x02,
    0xff,  // logical name (octet string : 9) =>clock
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
    0xff,  // logical name (octet string : 9) =>manuf id
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x02, 0x00,
    0xff,  // logical name (octet string : 9) => Total Cum. active power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x02, 0x00,
    0xff,  // logical name (octet string : 9) => Total  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x02, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 Cum. active
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x02, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x02, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 1 Cum. active
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x02, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 1  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x02, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 Cum. active
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x02, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x02, 0x02, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 Cum. active
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x0a, 0x02, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
    ,
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x81, 0x06, 0x80,
    0xff,  // logical name (octet string : 9) => sign (period, rece (송揚?,
           // energy)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning

};

static const uint8_t deliMAXDM_season_capture_objects[MAXDM_CAPOBJ_SIZE] = {
    0x01, 0x21,        // array
    0x02, 0x04,        // struct
    0x12, 0x00, 0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x00, 0x01, 0x02,
    0xff,              // logical name (octet string : 9) =>clock
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
    0xff,              // logical name (octet string : 9) =>manuf id
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x00,
    0xff,              // logical name (octet string : 9) => Total active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x00,
    0xff,              // logical name (octet string : 9) => Total active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x02, 0x00,
    0xff,              // logical name (octet string : 9) => Total Cum. active
                       // power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x00,
    0xff,              // logical name (octet string : 9) => Total apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x00,
    0xff,              // logical name (octet string : 9) => Total apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x02, 0x00,
    0xff,              // logical name (octet string : 9) => Total Cum. apparent
                       // power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x02, 0x01,
    0xff,              // logical name (octet string : 9) => Total Cum. active
                       // power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x02, 0x01,
    0xff,              // logical name (octet string : 9) => Tariff 1  Cum.
                       // apparent power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x02, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2 Cum.
                       // active power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x02, 0x02,
    0xff,              // logical name (octet string : 9) => Tariff 2  Cum.
                       // apparent power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x02, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3 Cum.
                       // active power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x02, 0x03,
    0xff,              // logical name (octet string : 9) => Tariff 3  Cum.
                       // apparent power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  active
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x06, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  active
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x01, 0x02, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4 Cum.
                       // active power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  apparent
                       // power(Q1+Q4) att_id = 2
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x06, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  apparent
                       // power(Q1+Q4) att_id = 5
    0x0f, 0x05,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x09, 0x02, 0x04,
    0xff,              // logical name (octet string : 9) => Tariff 4  Cum.
                       // apparent power(Q1+Q4)
    0x0f, 0x02,        // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning
    0x02, 0x04,        // struct
    0x12, 0x00, 0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x80, 0x06, 0x80,
    0xff,             // logical name (octet string : 9) => sign (period, rece
                      // (송揚?, energy)
    0x0f, 0x02,       // attribute id(integer : 15)
    0x12, 0x00, 0x00  // data_index(long-unsigned : 18) => no meaning

};

static const uint8_t receiMAXDM_season_capture_objects[MAXDM_CAPOBJ_SIZE] = {

    0x01,
    0x21,  // array

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x02, 0x00, 0x01, 0x02,
    0xff,  // logical name (octet string : 9) =>clock
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x00, 0x00, 0x00, 0x02,
    0xff,  // logical name (octet string : 9) =>manuf id
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x02, 0x00,
    0xff,  // logical name (octet string : 9) => Total Cum. active power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x00,
    0xff,  // logical name (octet string : 9) => Total apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x02, 0x00,
    0xff,  // logical name (octet string : 9) => Total  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x02, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1 Cum. active
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x02, 0x01,
    0xff,  // logical name (octet string : 9) => Tariff 1  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x02, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 1 Cum. active
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 2  apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x02, 0x02,
    0xff,  // logical name (octet string : 9) => Tariff 1  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x02, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3 Cum. active
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x02, 0x03,
    0xff,  // logical name (octet string : 9) => Tariff 3  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  active power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  active power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x02, 0x02, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4 Cum. active
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  apparent power(Q2+Q3)
           // att_id = 2
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x04,  // extended register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x06, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  apparent power(Q2+Q3)
           // att_id = 5
    0x0f,
    0x05,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x03,  // register class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x0a, 0x02, 0x04,
    0xff,  // logical name (octet string : 9) => Tariff 4  Cum. apparent
           // power(Q2+Q3)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
    ,
    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x01, 0x03, 0x81, 0x06, 0x80,
    0xff,  // logical name (octet string : 9) => sign (period, rece (송揚?,
           // energy)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
};

#define EVTLOG_CAP_SIZE (2 + 2 * 18)
static const /*__code*/ uint8_t event_log_capture_objects[EVTLOG_CAP_SIZE] = {
    0x01, 0x02,  // array

    0x02, 0x04,                       // struct
    0x12, 0x00, 0x08,                 // clock class id(long-unsigned : 18)
    0x09, 0x06, OBIS_DATE_TIME_nobr,  // logical name (octet string : 9) =>clock
    0x0f, 0x02,                       // attribute id(integer : 15)
    0x12, 0x00, 0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02, 0x04,        // struct
    0x12, 0x00, 0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00,        // logical name (octet string : 9) ==> filled by each event log
                 // counter
    0x0f, 0x02,  // attribute id(integer : 15)
    0x12, 0x00, 0x00  // data_index(long-unsigned : 18) => no meaning
};

static const /*__code*/ uint8_t event_log_cnt_objects[][OBIS_ID_SIZE] = {
    OBIS_PWR_FAIL_NUM,    OBIS_PWR_FAIL_NUM,
    OBIS_RTC_CHG_NUM,     OBIS_RTC_CHG_NUM,
    OBIS_aDR_NUM,         OBIS_mDR_NUM,
    OBIS_SR_NUM,          OBIS_PGM_CHG_NUM,
    {0, 0, 0, 0, 0, 0},                        // Scurr Limit -> not used
    OBIS_COVER_OPEN_NUM,  {0, 0, 0, 0, 0, 0},  // Magnet det -> not used
    OBIS_rLOAD_NUM,       OBIS_sCURR_nonSEL_NUM,
    OBIS_TCOVER_OPEN_NUM, OBIS_MTINIT_NUM,
    OBIS_WRONG_CONN_NUM,  OBIS_ERR_DIAGONIST_NUM,
    OBIS_SAG_CNT,         OBIS_SWELL_CNT};

#define SCLOG_CAPOBJ_NUM 6
#define SCLOG_CAPOBJ_SIZE (2 + SCLOG_CAPOBJ_NUM * 18)
static const /*__code*/ uint8_t
    sCurrLimit_LOG_capture_objects[SCLOG_CAPOBJ_SIZE] = {
        0x01,
        SCLOG_CAPOBJ_NUM,  // array

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x08,  // clock class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_DATE_TIME_nobr,  // logical name (octet string : 9) =>clock
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_sCURR_LIMIT_NUM_nobr,  // logical name (octet string : 9) => each
                                    // event log counter
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_sCURR_LIMIT_VAL_nobr,  // logical name (octet string : 9)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_sCURR_LIMIT2_VAL_nobr,  // logical name (octet string : 9)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_sCURR_autoRTN_VAL_nobr,  // logical name (octet string : 9)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00  // data_index(long-unsigned : 18) => no meaning

        ,
        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // register class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_sCURR_COUNTER_N1_nobr,  // logical name (octet string : 9)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00  // data_index(long-unsigned : 18) => no meaning
};

#define MAGLOG_CAPOBJ_SIZE (2 + 3 * 18)
static const /*__code*/ uint8_t Magnet_LOG_capture_objects[MAGLOG_CAPOBJ_SIZE] =
    {
        0x01,
        0x03,  // array

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x08,  // clock class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_DATE_TIME_nobr,  // logical name (octet string : 9) =>clock
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x03,  // register class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_MAGNET_DURTIME_nobr,  // logical name (octet string : 9)
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00,  // data_index(long-unsigned : 18) => no meaning

        0x02,
        0x04,  // struct
        0x12, 0x00,
        0x01,  // data class id(long-unsigned : 18)
        0x09, 0x06,
        OBIS_MAGNET_DET_NUM_nobr,  // logical name (octet string : 9) => each
                                   // event log counter
        0x0f,
        0x02,  // attribute id(integer : 15)
        0x12, 0x00,
        0x00  // data_index(long-unsigned : 18) => no meaning
};

#define EVT_INFO_PROF_CAPOBJ_SIZE (2 + 5 * 18)
static const uint8_t EVT_INFO_PROF_capture_objects[EVT_INFO_PROF_CAPOBJ_SIZE] =
    {
        0x01,
        0x05,

        0x02,
        0x04,
        0x12,
        0x00,
        0x01,
        0x09,
        0x06,
        OBIS_ERR_CODE_1_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00,

        0x02,
        0x04,
        0x12,
        0x00,
        0x01,
        0x09,
        0x06,
        OBIS_ERR_CODE_2_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00,

        0x02,
        0x04,
        0x12,
        0x00,
        0x01,
        0x09,
        0x06,
        OBIS_ERR_CODE_3_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00,

        0x02,
        0x04,
        0x12,
        0x00,
        0x01,
        0x09,
        0x06,
        OBIS_ERR_CODE_4_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00,
        // 평균전압
        0x02,
        0x04,
        0x12,
        0x00,
        0x03,
        0x09,
        0x06,
        OBIS_AVG_VOLT_L1_nobr,
        0x0f,
        0x02,
        0x12,
        0x00,
        0x00,
};

#define EVT_INFO_3PHS_PROF_CAPOBJ_SIZE (2 + 7 * 18)
static const uint8_t
    EVT_INFO_3phs_PROF_capture_objects[EVT_INFO_3PHS_PROF_CAPOBJ_SIZE] = {
        0x01, 0x07,

        0x02, 0x04, 0x12, 0x00, 0x01, 0x09, 0x06, OBIS_ERR_CODE_1_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00,

        0x02, 0x04, 0x12, 0x00, 0x01, 0x09, 0x06, OBIS_ERR_CODE_2_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00,

        0x02, 0x04, 0x12, 0x00, 0x01, 0x09, 0x06, OBIS_ERR_CODE_3_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00,

        0x02, 0x04, 0x12, 0x00, 0x01, 0x09, 0x06, OBIS_ERR_CODE_4_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00,

        0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, OBIS_AVG_VOLT_L1_L2_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00,

        0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, OBIS_AVG_VOLT_L2_L3_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00,

        0x02, 0x04, 0x12, 0x00, 0x03, 0x09, 0x06, OBIS_AVG_VOLT_L3_L1_nobr,
        0x0f, 0x02, 0x12, 0x00, 0x00,
};

static const /*__code*/ uint8_t unspecified_dtime[UNSP_TIME_SIZE] = {
    0x09, 0x0c,  // octet string and length
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff  // TODO: (WD) Check
};

#define CIRCDSP_PVT_MODE_NUM 2
static const /*__code*/ dsp_item_info_type
    circdsp_pvt_mode_table[CIRCDSP_PVT_MODE_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD}  // 시간
};

#define CIRCDSP_SMODE_UNIDIR_NUM 1
static const /*__code*/ dsp_item_info_type
    circdsp_smode_unidir_table[CIRCDSP_SMODE_UNIDIR_NUM] = {
        {0x0003, {0x01, 0x00, 0x01, 0x08, 0x00, 0xff}, 0x02, DISP_CIRC_PERIOD}};

#define CIRCDSP_SMODE_BOTHDIR_NUM 2
static const /*__code*/ dsp_item_info_type
    circdsp_smode_bothdir_table[CIRCDSP_SMODE_BOTHDIR_NUM] = {
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 수전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD}  // 현재 누적 송전 유효 전력량_전체
};

#if 0 /* bccho, 2024-09-24, 삼상, delete  */
/*
통신 규격 3.4.2.8.2 디스플레이 - 납품 시 순환표시 모드
통신 규격 참조할 것.
    3.4.2.1.1.1 전력량 (항목별 자료)
    3.4.2.1.4.1 누적 수요전력 (항목별 자료)
    3.4.2.1.3.1 최대수요전력 (항목별 자료)
    3.4.2.1.10 현재/직전 수요 시한 수요전력
    3.4.2.3.6 디스플레이용 일자/시간
    3.4.2.8.2.1 관리자 순환표시 모드 가능 항목
OBIS : 0 0 96 50 50 255, Attributes 2
TODO : (WD) dsp_item_info_type 데이터형 배열에 변경된 OBIS 코드 반영 필요함.
22년 8월 3일 현재 KVMK 프로그램 R4.12.13 버전에 아직 규격 반영이 안됨.
*/

// 1종 순환 표시항목 (단독 1P2W 60A 품목) - 수전
#define CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM 10
#define CIRCDSP_NSMODE_ONERATE_UNIDIR_STIDX5 0
#define CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM5 CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_onerate_unidir_table[CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간

#if 1 /* bccho, 2024-06-03 */
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 누적 최대 수요 유효 전력_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x66},
         0x02,
         DISP_CIRC_PERIOD},  // 전전월 수전 누적 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체 : [value]
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체 : [capture_time]
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 현재/직전 수요 시한 수요전력_수전 유효전력 : 직전
                            // 수요 전력 [last_average_value]
};

// 1종 순환 표시항목 (단독 1P2W 60A 품목) - 송/수전
#define CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM 12
#define CIRCDSP_NSMODE_ONERATE_BOTHDIR_STIDX5 0
#define CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM5 CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_onerate_bothdir_table[CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간

#if 1 /* bccho, 2024-06-03 */
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 송전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 누적 최대 수요 유효 전력_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x66},
         0x02,
         DISP_CIRC_PERIOD},  // 전전월 수전 누적 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 송전 유효 전력량_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 직전 유효 수요 전력
};

// 2종 순환 표시항목 (단독 1P2W 60A 품목) - 수전
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM 12
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_STIDX5 0
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM5 CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_tworate_unidir_table[CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간

#if 1 /* bccho, 2024-06-03 */
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_A
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 누적 최대 수요 유효 전력_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x66},
         0x02,
         DISP_CIRC_PERIOD},  // 전전월 수전 누적 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_A
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 직전 유효 수요 전력
};

// 2종 순환 표시항목 (단독 1P2W 60A 품목) - 송/수전
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM 14
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_STIDX5 0
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM5 CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_tworate_bothdir_table[CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간

#if 1 /* bccho, 2024-06-03 */
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 누적 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 누적 수전 유효 전력량_A
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 누적 송전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 누적 최대 수요 유효 전력_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x66},
         0x02,
         DISP_CIRC_PERIOD},  // 전전월 수전 누적 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 수전 유효 전력량_A
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 송전 유효 전력량_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 직전 유효 수요 전력
};
#endif

/* bccho, 2024-09-24 */
#if METER_ID == 53  // defined(FEATURE_JP_DISP_AMIGO_OBIS)
// #if 1 //defined(FEATURE_JP_DISP_AMIGO_OBIS)

/*
통신 규격 3.4.2.8.2 디스플레이 - 납품 시 순환표시 모드
통신 규격 참조할 것.
    3.4.2.1.1.1 전력량 (항목별 자료)
    3.4.2.1.4.1 누적 수요전력 (항목별 자료)
    3.4.2.1.3.1 최대수요전력 (항목별 자료)
    3.4.2.1.10 현재/직전 수요 시한 수요전력
    3.4.2.3.6 디스플레이용 일자/시간
    3.4.2.8.2.1 관리자 순환표시 모드 가능 항목
OBIS : 0 0 96 50 50 255, Attributes 2
TODO : (WD) dsp_item_info_type 데이터형 배열에 변경된 OBIS 코드 반영 필요함.
22년 8월 3일 현재 KVMK 프로그램 R4.12.13 버전에 아직 규격 반영이 안됨.
*/

// 1종 순환 표시항목 (단독 1P2W 60A 품목) - 수전
#define CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM 10
#define CIRCDSP_NSMODE_ONERATE_UNIDIR_STIDX5 0
#define CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM5 CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_onerate_unidir_table[CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간
#ifdef DISPLAY_MODE_TOU_PROG_ID
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 누적 최대 수요 유효 전력_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x66},
         0x02,
         DISP_CIRC_PERIOD},  // 전전월 수전 누적 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체 : [value]
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체 : [capture_time]
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 현재/직전 수요 시한 수요전력_수전 유효전력 : 직전
                            // 수요 전력 [last_average_value]

};

// 1종 순환 표시항목 (단독 1P2W 60A 품목) - 송/수전
#define CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM 12
#define CIRCDSP_NSMODE_ONERATE_BOTHDIR_STIDX5 0
#define CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM5 CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_onerate_bothdir_table[CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간
#ifdef DISPLAY_MODE_TOU_PROG_ID
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 송전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 누적 최대 수요 유효 전력_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x66},
         0x02,
         DISP_CIRC_PERIOD},  // 전전월 수전 누적 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 송전 유효 전력량_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 직전 유효 수요 전력

};

// 2종 순환 표시항목 (단독 1P2W 60A 품목) - 수전
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM 12
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_STIDX5 0
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM5 CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_tworate_unidir_table[CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간
#ifdef DISPLAY_MODE_TOU_PROG_ID
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_A
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 누적 최대 수요 유효 전력_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x66},
         0x02,
         DISP_CIRC_PERIOD},  // 전전월 수전 누적 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_A
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 직전 유효 수요 전력

};

// 2종 순환 표시항목 (단독 1P2W 60A 품목) - 송/수전
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM 14
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_STIDX5 0
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM5 CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_tworate_bothdir_table[CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간
#ifdef DISPLAY_MODE_TOU_PROG_ID
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 누적 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 누적 수전 유효 전력량_A
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 누적 송전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 누적 최대 수요 유효 전력_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x02, 0x00, 0x66},
         0x02,
         DISP_CIRC_PERIOD},  // 전전월 수전 누적 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 수전 유효 전력량_A
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 송전 유효 전력량_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 직전 유효 수요 전력

};

#else

/*
통신 규격 3.4.2.8.2 디스플레이 - 납품 시 순환표시 모드
통신 규격 참조할 것.
    3.4.2.1.1.1 전력량 (항목별 자료)
    3.4.2.1.4.1 누적 수요전력 (항목별 자료)
    3.4.2.1.3.1 최대수요전력 (항목별 자료)
    3.4.2.1.10 현재/직전 수요 시한 수요전력
    3.4.2.3.6 디스플레이용 일자/시간
    3.4.2.8.2.1 관리자 순환표시 모드 가능 항목
OBIS : 0 0 96 50 50 255, Attributes 2
TODO : (WD) dsp_item_info_type 데이터형 배열에 변경된 OBIS 코드 반영 필요함.
22년 8월 3일 현재 KVMK 프로그램 R4.12.13 버전에 아직 규격 반영이 안됨.
*/

// 1종 순환 표시항목 (//1p 60A   이외 모델품목) - 수전
#define CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM 10
#define CIRCDSP_NSMODE_ONERATE_UNIDIR_STIDX5 0
#define CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM5 CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_onerate_unidir_table[CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간
#ifdef DISPLAY_MODE_TOU_PROG_ID
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_전체
#if 0
	    {0x0003, 
        {0x01, 0x00, 0x01, 0x02, 0x00, 0x65}, 
        0x02, 
        DISP_CIRC_PERIOD},	// 전월 수전 누적 최대 수요 유효 전력_전체
#else
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 최대 수요 유효 전력_전체
#endif
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0x65},
         0x05,
         DISP_CIRC_PERIOD},  // 전월  수전 최대 수요 시간_전체 : [capture_time]
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체 : [value]
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체 : [capture_time]
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 현재/직전 수요 시한 수요전력_수전 유효전력 : 직전
                            // 수요 전력 [last_average_value]

};

// 1종 순환 표시항목 (//1p 60A   이외 모델품목)- 송/수전
#define CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM 12
#define CIRCDSP_NSMODE_ONERATE_BOTHDIR_STIDX5 0
#define CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM5 CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_onerate_bothdir_table[CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간
#ifdef DISPLAY_MODE_TOU_PROG_ID
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 송전 유효 전력량_전체
#if 0
	    {0x0003, 
        {0x01, 0x00, 0x01, 0x02, 0x00, 0x65}, 
        0x02, 
        DISP_CIRC_PERIOD},	// 전월 수전 누적 최대 수요 유효 전력_전체
#else
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 최대 수요 유효 전력_전체
#endif
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0x65},
         0x05,
         DISP_CIRC_PERIOD},  // 전월  수전 최대 수요 시간_전체 : [capture_time]
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_전체
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 송전 유효 전력량_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체
        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 직전 유효 수요 전력

};

// 2종 순환 표시항목 (//1p 60A   이외 모델품목) - 수전
#if 0
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM 17
#else
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM 16
#endif
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_STIDX5 0
#define CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM5 CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_tworate_unidir_table[CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간
#ifdef DISPLAY_MODE_TOU_PROG_ID
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 유효 전력량_A
#if 0  // JP.KIM 24.11.29
	    {0x0003, 
        {0x01, 0x00, 0x05, 0x08, 0x02, 0x65}, 
        0x02, 
        DISP_CIRC_PERIOD},	// 전월 수전 선택 무효 전력량_B
#else
        {0x0003,
         {0x01, 0x00, 0xfe, 0x08, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 선택 무효 전력량_B  0xfe:수전, 0xff
                             // 송전
#endif

#if 0
	    {0x0003, 
        {0x01, 0x00, 0x01, 0x02, 0x00, 0x65}, 
        0x02, 
        DISP_CIRC_PERIOD},	// 전월 수전 누적 최대 수요 유효 전력_전체
#endif
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0x65},
         0x05,
         DISP_CIRC_PERIOD},  // 전월 수전 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x0D, 0x09, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 평균 역율 _중간부하_B

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 유효 전력량_A
#if 0  // JP.KIM 24.11.29
    	{0x0003, 
        {0x01, 0x00, 0x05, 0x08, 0x02, 0xff}, 
        0x02, 
        DISP_CIRC_PERIOD},	// 현재 수전 선택 무효 전력량_B
#else
        {0x0003,
         {0x01, 0x00, 0xfe, 0x08, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 선택 무효 전력량_B	  0xfe:수전,
                             // 0xff 송전
#endif

#if 0
	    {0x0003, 
        {0x01, 0x00, 0x01, 0x02, 0x00, 0xff}, 
        0x02, 
        DISP_CIRC_PERIOD},	// 현재 수전 누적 최대 수요 유효 전력_전체
#endif
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x0D, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 평균 역율 _중간부하_B

        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 직전 유효 수요 전력
};

// 2종 순환 표시항목 (//1p 60A   이외 모델품목) - 송/수전
#if 0
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM 19
#else
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM 18
#endif
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_STIDX5 0
#define CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM5 CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM
static const /*__code*/ dsp_item_info_type
    circdsp_nsmode_tworate_bothdir_table[CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM] = {
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 날자
        {0x0001,
         {0x01, 0x00, 0x00, 0x09, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 시간
#ifdef DISPLAY_MODE_TOU_PROG_ID
        {0x0001,
         {0x01, 0x00, 0x00, 0x02, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 프로그램 ID
#else
        {0x0016,
         {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff},
         0x04,
         DISP_CIRC_PERIOD},  // 정기 검침일
#endif

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 누적 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 누적 수전 유효 전력량_A
#if 0  // JP.KIM 24.11.29
	    {0x0003, 
        {0x01, 0x00, 0x05, 0x08, 0x02, 0x65}, 
        0x02, 
        DISP_CIRC_PERIOD},	// 전월 수전 선택 무효 전력량_B
#else
        {0x0003,
         {0x01, 0x00, 0xfe, 0x08, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 선택 무효 전력량_B  0xfe:수전, 0xff
                             // 송전
#endif
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 누적 송전 유효 전력량_전체
#if 0
	    {0x0003, 
        {0x01, 0x00, 0x01, 0x02, 0x00, 0x65}, 
        0x02, 
        DISP_CIRC_PERIOD},	// 전월 수전 누적 최대 수요 유효 전력_전체
#endif
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0x65},
         0x05,
         DISP_CIRC_PERIOD},  // 전월 수전 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x0D, 0x09, 0x02, 0x65},
         0x02,
         DISP_CIRC_PERIOD},  // 전월 수전 평균 역율 _중간부하_B

        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 수전 유효 전력량_B
        {0x0003,
         {0x01, 0x00, 0x01, 0x08, 0x01, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 수전 유효 전력량_A
#if 0  // JP.KIM 24.11.29
	    {0x0003, 
        {0x01, 0x00, 0x05, 0x08, 0x02, 0xff}, 
        0x02, 
        DISP_CIRC_PERIOD},		// 현재 수전 선택 무효 전력량_B
#else
        {0x0003,
         {0x01, 0x00, 0xfe, 0x08, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 선택 무효 전력량_B	 0xfe:수전, 0xff
                             // 송전
#endif
        {0x0003,
         {0x01, 0x00, 0x02, 0x08, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 누적 송전 유효 전력량_전체
#if 0
	    {0x0003, 
        {0x01, 0x00, 0x01, 0x02, 0x00, 0xff}, 
        0x02, 
        DISP_CIRC_PERIOD},	// 현재 수전 누적 최대 수요 유효 전력_전체
#endif
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 유효 전력_전체
        {0x0004,
         {0x01, 0x00, 0x01, 0x06, 0x00, 0xff},
         0x05,
         DISP_CIRC_PERIOD},  // 현재 수전 최대 수요 시간_전체
        {0x0003,
         {0x01, 0x00, 0x0D, 0x09, 0x02, 0xff},
         0x02,
         DISP_CIRC_PERIOD},  // 현재 수전 평균 역율 _중간부하_B

        {0x0005,
         {0x01, 0x01, 0x01, 0x04, 0x00, 0xff},
         0x03,
         DISP_CIRC_PERIOD}  // 직전 유효 수요 전력

};
#endif

#define EOB_SCRIPT_LEN 13
static const /*__code*/ uint8_t EOB_script[EOB_SCRIPT_LEN] = {
    0x02, 0x02,  // structure (2) = { string, uint16_t}
    0x09, 0x06, 0x00, 0x00, 0x0a, 0x00, 0x01, 0xff, 0x12, 0x00, 0x01};

#define EOBP_DATE_LEN 17
static const /*__code*/ uint8_t EOBP_date[EOBP_DATE_LEN] = {
    0x01, 0x01,                               // array = 1
    0x02, 0x02,                               // structure = { time, date }
    0x09, 0x04, 0x00, 0x00, 0x00, 0xff,       // time
    0x09, 0x05, 0xff, 0xff, 0xff, 0x01, 0xff  // date
};

#define NPBILL_DATE_LEN 15
static const /*__code*/ uint8_t npbill_date_r[NPBILL_DATE_LEN] = {
    0x02, 0x02,                               // struct type
    0x09, 0x04, 0xff, 0xff, 0xff, 0xff,       // time (octet string : 9)
    0x09, 0x05, 0xff, 0xff, 0xff, 0xff, 0xff  // date (octet string : 9)
};

#define SEASON_INFO_SIZE 22
static const /*__code*/ uint8_t packed_season_info_r[SEASON_INFO_SIZE] = {
    0x02, 0x03,        // struct
    0x09, 0x01, 0x00,  // season profile name
    0x09, 0x0c, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0xfd,
    0xe4, 0x00,       // season start time (not specified,
                      // GMT= 0xfde4)
    0x09, 0x01, 0x00  // week profile id
};

#define WEEK_INFO_SIZE 19
static const /*__code*/ uint8_t packed_week_info_r[WEEK_INFO_SIZE] = {
    0x02, 0x08,        // struct
    0x09, 0x01, 0x00,  // week profile name
    0x11, 0x00,        // monday day_id
    0x11, 0x00,        // tuesday day_id
    0x11, 0x00,        // wednessday day_id
    0x11, 0x00,        // thursday day_id
    0x11, 0x00,        // friday day_id
    0x11, 0x00,        // saturday day_id
    0x11, 0x00         // sunday day_id
};

#define DAY_INFO_SIZE 19
static const /*__code*/ uint8_t packed_day_info_r[DAY_INFO_SIZE] = {
    0x02, 0x03,                          // struct
    0x09, 0x04, 0xff, 0xff, 0xff, 0xff,  // start time
    0x09, 0x06, 0x00, 0x00, 0x0a, 0x00,
    0x64, 0xff,       // logical script name (octet string : 9)
    0x12, 0x00, 0x01  // script selector (long unsigned : 18)
};

#define SPECIAL_DAY_INFO_SIZE 14
static const /*__code*/ uint8_t packed_special_day_r[SPECIAL_DAY_INFO_SIZE] = {
    0x02, 0x03,        // struct type
    0x12, 0x00, 0x00,  // sp index (uint16_t type)
    0x09, 0x05, 0xff, 0xff,
    0xff, 0xff, 0xff,  // periodic holidays (octet string : 9)
    0x11, 0x00         // day id (unsigned : 17)
};

#define TS_CONF_CTRL_LEN 13
static const /*__code*/ uint8_t ts_conf_ctrl_data[TS_CONF_CTRL_LEN] = {
    STRUCTURE_TAG,    0x02,                                      // struct
    OCTSTRING_TAG,    0x06, 0x01, 0x00, 0x80, 0x00, 0x05, 0xff,  // script obis
    LONGUNSIGNED_TAG, 0x00, 0x00                                 // selector
};

#define TS_CONF_ZONE_LEN 15
static const /*__code*/ uint8_t ts_conf_zone_data[TS_CONF_ZONE_LEN] = {
    STRUCTURE_TAG, 0x02,                               // struct
    OCTSTRING_TAG, 0x04, 0x00, 0x00, 0x00, 0x00,       // time
    OCTSTRING_TAG, 0x05, 0xff, 0xff, 0xff, 0xff, 0xff  // date
};

#define CERT_LOG_CAPOBJ_SIZE (2 + 3 * 18)
static const /*__code*/ uint8_t cert_log_capture_objects[MAGLOG_CAPOBJ_SIZE] = {
    0x01,
    0x03,  // array

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x08,  // clock class id(long-unsigned : 18)
    0x09, 0x06,
    OBIS_DATE_TIME_nobr,  // logical name (octet string : 9) =>clock
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06,
    OBIS_CERT_LOG_NUM_nobr,  // logical name (octet string : 9)
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00,  // data_index(long-unsigned : 18) => no meaning

    0x02,
    0x04,  // struct
    0x12, 0x00,
    0x01,  // data class id(long-unsigned : 18)
    0x09, 0x06,
    OBIS_CERT_LOG_CASE_nobr,  // logical name (octet string : 9) => each event
                              // log counter
    0x0f,
    0x02,  // attribute id(integer : 15)
    0x12, 0x00,
    0x00  // data_index(long-unsigned : 18) => no meaning
};

// #define	PUSH_SETUP_ERRCODE_CAPOBJ_SIZE			(2 + 2*18)
/*static const*/ uint8_t
    PUSH_SETUP_ERRCODE_capture_objects[PUSH_SETUP_ERRCODE_CAPOBJ_SIZE] = {
        0x01,          0x02,

        0x02,          0x04, 0x12, 0x00,
        CLS_PushSetUp, 0x09, 0x06, OBIS_PUSH_SETUP_ERR_CODE_nobr,
        0x0f,          0x02, 0x12, 0x00,
        0x00,

        0x02,          0x04, 0x12, 0x00,
        CLS_ProfG,     0x09, 0x06, OBIS_EVENT_INFO_nobr,
        0x0f,          0x02, 0x12, 0x00,
        0x00,
};

// #define	PUSH_SETUP_LAST_LP_CAPOBJ_SIZE			(2 + 2*18)
/*static const*/ uint8_t
    PUSH_SETUP_lastLP_capture_objects[PUSH_SETUP_LAST_LP_CAPOBJ_SIZE] = {
        0x01,          0x02,

        0x02,          0x04, 0x12, 0x00,
        CLS_PushSetUp, 0x09, 0x06, OBIS_PUSH_SETUP_LAST_LP_nobr,
        0x0f,          0x02, 0x12, 0x00,
        0x00,

        0x02,          0x04, 0x12, 0x00,
        CLS_ProfG,     0x09, 0x06, OBIS_LOAD_PROFILE_nobr,
        0x0f,          0x02, 0x12, 0x10,
        0x01,
};

void approc_get_req(int idx)
{
    // DPRINTF(DBG_NONE, _D"%s: choice[%d]\r\n", __func__, appl_reqchoice);
    switch (appl_reqchoice)
    {  // apdu[1]
    case GET_REQ_NORMAL:
        DPRINTF(DBG_TRACE, _D "Normal\r\n");
        approc_get_req_normal(idx);
        break;
    case GET_REQ_NEXT:
        DPRINTF(DBG_TRACE, _D "Next\r\n");
        approc_get_req_block(idx);
        break;
    default:
        appl_resp_choice = GET_RES_NORMAL;  // updated in case of block
        appl_resp_result = GET_RESULT_TYPE_UNMAT;
        break;
    }

    appl_get_resp();
}

static void approc_get_req_normal(int idx)
{
    appl_result_type rslt;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    appl_resp_choice = GET_RES_NORMAL;  // updated in case of block

    idx = appl_cosem_descript(idx);
    appl_selective_acs_descript(idx);
    if (idx > appl_len)
    {
        appl_resp_result = GET_RESULT_TYPE_UNMAT;
        DPRINTF(DBG_ERR, _D "%s: Type Mismatched\r\n", __func__);
        return;
    }

    rslt = appl_obj_enum_and_acs_check();

    if (rslt != RESULT_OK)
    {
        DPRINTF(DBG_ERR, _D "%s: OBJ Result[%d]\r\n", __func__, rslt);

        if (rslt == APPL_RESULT_OBJ_UNDEF)
            appl_resp_result = GET_RESULT_OBJ_UNAVAIL;
        else
            appl_resp_result = GET_RESULT_TYPE_UNMAT;

        DPRINTF(DBG_ERR, _D "%s: Get OBJ Error[%d]\r\n", __func__,
                appl_resp_result);

        return;
    }

    if (!comm_en_coveropen && IS_MorTCOVER_OPEN &&
        appl_obj_id != OBJ_COMM_ENABLE)
    {
        appl_resp_result = GET_RESULT_OBJ_UNAVAIL;
        DPRINTF(DBG_ERR, _D "%s: OBJ unavailable\r\n", __func__);
        return;
    }

    // global_buf 를 사용하기 대문에 appl_selective_acs_descript() 뒤에서 실행
    // application association 이후 get_req 에만 적용하기 위해 이곳에서 처리 함
    if_not_captured_capture();

    approc_fill_get_resp_normal();
}

void approc_get_req_block(int idx)
{
    uint32_t blocknum = 0;

    appl_resp_choice = GET_RES_BLOCK;

    // frame length check
    if ((idx + 4) != appl_len)
    {
        appl_resp_last_block = 1;
        appl_resp_block_num += 1L;

        appl_resp_result = GET_RESULT_BLOCK_APDU_ERR;
        return;
    }

    // block number check
    ToH32((U8_16_32*)&blocknum, &appl_msg[idx]);
    if (blocknum != appl_resp_block_num)
    {
        appl_resp_last_block = 1;
        appl_resp_block_num += 1L;

        appl_resp_result = GET_RESULT_BLOCK_NEXT_ERR;
        return;
    }

    if (at_cmd_rsp_ing)  // jp.kim 25.01.22
    {
        DPRINTF(DBG_TRACE,
                _D
                "%s 2 pPdu_idx[%d]  getresp_LP_len[%d] at_cmd_rsp_ing[%d] \r\n",
                __func__, pPdu_idx, getresp_LP_len, at_cmd_rsp_ing);
        approc_fill_get_atcmd_resp_block(false);
    }
    else
        approc_fill_get_resp_block(false);
}

static void if_not_captured_capture(void)
{
    if (!appl_whm_inf_collected)
    {
        appl_whm_inf_collected = true;

        approc_get_req_capture();

        obj_id_secs_backup();
    }
    else
    {
        approc_get_req_capture_always();
        if (approc_is_valid_cur_capture_others())
        {
            approc_get_req_capture_others();
        }
    }
}

static void curr_rcnt_cap(uint8_t* tptr)
{
    int i;
    uint32_t eoich[numDmCHs];
    recent_demand_type* rcnt;

    rcnt = (recent_demand_type*)tptr;

    // current demand ch
    fill_rcnt_ch(eoich, dm_interval);
    for (i = 0; i < numDmCHs; i++)
    {
        rcnt->dm[i] = eoich[i];
    }

    // current demand time
    get_cur_dmdt(&rcnt->dt);

    nv_write(I_CUR_DM_CAP, (uint8_t*)rcnt);

    if (!nv_read(I_RCNT_DEMAND, (uint8_t*)rcnt))
    {
        memset((uint8_t*)rcnt, 0, sizeof(recent_demand_type));
        memset((uint8_t*)&rcnt->dt, 0xff, sizeof(date_time_type));
    }

    nv_write(I_RCNT_DM_CAP, (uint8_t*)rcnt);

    lp_intv_pf_cap[eDeliAct] = lp_intv_pf[eDeliAct];
    lp_intv_pf_cap[eReceiAct] = lp_intv_pf[eReceiAct];
}

void approc_get_req_capture_always(void)
{
    // comm setup time
    comm_dt = cur_rtc;

    // capture clock status
    cap_clk_sts = 0;
    if (!cur_rtc_is_set())
        cap_clk_sts |= CLKSTS_DOUBTFUL;
    if (DLS_is_active())
        cap_clk_sts |= CLKSTS_DAYLIGHT_SAVING;

    cap_cur_rate = cur_rate;
    cap_LP_event = LP_event;
}

void approc_get_req_capture_others(void)
{
    rate_type i;
    demand_ch_type j;
    uint8_t* tptr;
    mr_data_accm_type* accm;
    mr_data_dm_type* dm;
    rolling_dm_ch_type* rollch;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    tptr = adjust_tptr(&global_buff[0]);

    accm = (mr_data_accm_type*)tptr;
    dm = (mr_data_dm_type*)tptr;

    inc_idx_cap_wear();  // 복전 이후 초기화의 의미도 잇기 대문에 idx_cap_wear
                         // 설정을 먼저 함
    // capture current / recent demand
    curr_rcnt_cap(tptr);

    dsm_wdt_ext_toggle_immd();

    mr_capture_accm(accm);
    for (i = eArate; i < numRates; i++)
    {
        nv_sub_info.cur.rt = i;
        nv_sub_info.cur.sel = eMrAccm;
        nv_write(I_CUR_DATA_CAP, (uint8_t*)&accm->accm[i]);
    }

    dsm_wdt_ext_toggle_immd();

    mr_capture_dm(dm, true);
    for (i = eArate; i < numRates; i++)
    {
        nv_sub_info.cur.rt = i;
        nv_sub_info.cur.sel = eMrDm;
        nv_write(I_CUR_DATA_CAP, (uint8_t*)&dm->dm[i]);
    }

    dsm_wdt_ext_toggle_immd();

    rollch = (rolling_dm_ch_type*)tptr;
    for (i = eArate; i < numRates; i++)
    {
        for (j = 0; j < numDmCHs; j++)
        {
            mr_capture_rolldm(i, j, rollch);

            nv_sub_info.cur.rt = i;
            nv_sub_info.cur.sel = eMrSublocks;
            nv_sub_info.cur.chsel = j;
            nv_write(I_CUR_DATA_CAP, (uint8_t*)rollch);
        }
    }

    dsm_wdt_ext_toggle_immd();
}

void dsm_cur_cap_op_decision_info_init(void)
{
    memset(&gst_cur_op_cap_info, 0x00, sizeof(ST_CUR_CAP_OP_DECISION_INFO));
}

ST_CUR_CAP_OP_DECISION_INFO* dsm_cur_op_decision_info_get(void)
{
    return &gst_cur_op_cap_info;
}

bool obj_id_is_valid_for_acc_dm(obj_id_enum_type obj_id)
{
    if (OBJ_MONTH_ENERGY_DELI == obj_id ||
        OBJ_MONTH_ENERGY_DELI_nPRD == obj_id ||
        OBJ_MONTH_ENERGY_DELI_SEASON == obj_id ||
        OBJ_MONTH_ENERGY_RECEI == obj_id ||
        OBJ_MONTH_ENERGY_RECEI_nPRD == obj_id ||
        OBJ_MONTH_ENERGY_RECEI_SEASON == obj_id ||

        OBJ_MONTH_MAXDM_DELI == obj_id || OBJ_MONTH_MAXDM_DELI_nPRD == obj_id ||
        OBJ_MONTH_MAXDM_DELI_SEASON == obj_id ||
        OBJ_MONTH_MAXDM_RECEI == obj_id ||
        OBJ_MONTH_MAXDM_RECEI_nPRD == obj_id ||
        OBJ_MONTH_MAXDM_RECEI_SEASON == obj_id ||

        OBJ_MONTH_SUBLOCKS == obj_id ||

        OBJ_AVGPF_DELI == obj_id || OBJ_AVGPF_DELI_nPRD == obj_id ||
        OBJ_AVGPF_DELI_SEASON == obj_id ||

        OBJ_AVGPF_RECEI == obj_id || OBJ_AVGPF_RECEI_nPRD == obj_id ||
        OBJ_AVGPF_RECEI_SEASON == obj_id ||

        OBJ_LAST15_PF_DELI == obj_id || OBJ_LAST15_PF_RECEI == obj_id ||
        OBJ_CURR_LAST_DEMAND == obj_id ||

        OBJ_CUM_DEMAND == obj_id ||

        OBJ_MAX_DEMAND == obj_id || OBJ_MAX_DEMAND_nPRD == obj_id ||
        OBJ_MAX_DEMAND_SEASON == obj_id ||

        OBJ_ENERGY == obj_id || OBJ_ENERGY_nPRD == obj_id ||
        OBJ_ENERGY_SEASON == obj_id ||

        OBJ_ENERGY_FWD_BOTH_REACT == obj_id ||
        OBJ_ENERGY_BWD_BOTH_REACT == obj_id)
    {
        DPRINTF(DBG_NONE, "%s: OK\r\n", __func__);
        return true;
    }
    else
    {
        return false;
    }
}

void obj_id_secs_backup(void)
{
    uint32_t cur_secs = dsm_rtc_get_time();
    ST_CUR_CAP_OP_DECISION_INFO* p_cur_info = dsm_cur_op_decision_info_get();

    p_cur_info->pre_obj_id = appl_obj_id;
    p_cur_info->pre_seconds = cur_secs;
    p_cur_info->gf = obis_gf;

    DPRINTF(DBG_TRACE, "%s: obj_idx[%d], cur_secs[%d], gf[0x%02X]\r\n",
            __func__, p_cur_info->pre_obj_id, p_cur_info->pre_seconds,
            p_cur_info->gf);
}

bool approc_is_valid_cur_capture_others(void)
{
    bool ret = false;
    uint32_t cur_secs = dsm_rtc_get_time();
    ST_CUR_CAP_OP_DECISION_INFO* p_cur_info = dsm_cur_op_decision_info_get();

    if (obj_id_is_valid_for_acc_dm(appl_obj_id) && obis_gf == 0xff)  // 현재
    {
        if ((cur_secs > (p_cur_info->pre_seconds + CUR_CAP_UPDATE_SECS_MAX)))

        {
            DPRINTF(DBG_TRACE,
                    "cur_capture OK: obj_idx[ pre %d, cur %d ], secs[ pre %d, "
                    "cur %d ], gf[ pre: 0x%02X, cur: 0x%02X]\r\n",
                    appl_obj_id, p_cur_info->pre_obj_id,
                    p_cur_info->pre_seconds, cur_secs, p_cur_info->gf, obis_gf);

            p_cur_info->pre_obj_id = appl_obj_id;
            p_cur_info->pre_seconds = cur_secs;
            p_cur_info->gf = obis_gf;

            ret = true;
        }
    }

    return ret;
}

static void approc_get_req_capture(void)
{
    rate_type i;
    demand_ch_type j;
    uint8_t* tptr;
    mr_data_accm_type* accm;
    mr_data_dm_type* dm;
    rolling_dm_ch_type* rollch;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    tptr = adjust_tptr(&global_buff[0]);

    accm = (mr_data_accm_type*)tptr;
    dm = (mr_data_dm_type*)tptr;

    inc_idx_cap_wear();  // 복전 이후 초기화의 의미도 잇기 대문에 idx_cap_wear
                         // 설정을 먼저 함

    // comm setup time
    comm_dt = cur_rtc;
    // capture clock status
    cap_clk_sts = 0;
    if (!cur_rtc_is_set())
        cap_clk_sts |= CLKSTS_DOUBTFUL;
    if (DLS_is_active())
        cap_clk_sts |= CLKSTS_DAYLIGHT_SAVING;

    // capture current rate
    cap_cur_rate = cur_rate;

    // capture current lp status
    cap_LP_event = LP_event;

    curr_rcnt_cap(tptr);

    dsm_wdt_ext_toggle_immd();

    // capture current accm / dm
    // mr_capture(accm, dm); ==> memory 크기 대문에 분리 함
    mr_capture_accm(accm);
    for (i = eArate; i < numRates; i++)
    {
        nv_sub_info.cur.rt = i;
        nv_sub_info.cur.sel = eMrAccm;
        nv_write(I_CUR_DATA_CAP, (uint8_t*)&accm->accm[i]);
    }

    dsm_wdt_ext_toggle_immd();

    mr_capture_dm(dm, true);
    for (i = eArate; i < numRates; i++)
    {
        nv_sub_info.cur.rt = i;
        nv_sub_info.cur.sel = eMrDm;
        nv_write(I_CUR_DATA_CAP, (uint8_t*)&dm->dm[i]);
    }

    dsm_wdt_ext_toggle_immd();

    rollch = (rolling_dm_ch_type*)tptr;
    for (i = eArate; i < numRates; i++)
    {
        for (j = 0; j < numDmCHs; j++)
        {
            mr_capture_rolldm(i, j, rollch);

            nv_sub_info.cur.rt = i;
            nv_sub_info.cur.sel = eMrSublocks;
            nv_sub_info.cur.chsel = j;
            nv_write(I_CUR_DATA_CAP, (uint8_t*)rollch);
        }
    }

    dsm_wdt_ext_toggle_immd();
}

static void ob_thd_rec_period(void)
{
    if (appl_att_id == 0x02)
    {
        FILL_U16(thd_rec_period);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 7);  // sec
    }
}

static void approc_fill_get_resp_normal(void)
{
    int pPdu_idx_start;

    pPdu_idx_start = pPdu_idx = APPL_FILL_RESP_DATA_IDX_NORMAL;

    DPRINTF(DBG_TRACE, _D "%s: obj_id[%d], att_id[%d]\r\n", __func__,
            appl_obj_id, appl_att_id);

    if (appl_att_id == 1)
    {
        fill_octet_string_x(appl_obis.id, OBIS_ID_SIZE);
        return;
    }

    switch (appl_obj_id)
    {
    case OBJ_ASSOCIATION_LN:
        ob_ass_LN();
        break;

    case OBJ_COUNTER_BILLING:
        ob_counter_billing();
        break;

    case OBJ_NUM_AVAIL_BILLING:
        ob_num_avail_billing();
        break;

    case OBJ_CUSTOM_ID:
    case OBJ_MANUFACT_ID:
        if (appl_obis.id[GROUP_E] == 0x00)
        {
            ob_custom_id();
        }
        else if (appl_obis.id[GROUP_E] == 0x02)
        {
            ob_manuf_id();
        }
        else if (appl_obis.id[GROUP_E] == 0x01)
        {
            ob_manuf_id_old();
        }
        break;

    case OBJ_PGM_ID:
        ob_prog_id();
        break;

    case OBJ_MTCONST_ACTIVE:
        ob_mtconst_active();
        break;

    case OBJ_MTCONST_REACTIVE:
        ob_mtconst_reactive();
        break;

    case OBJ_MTCONST_APP:
        ob_mtconst_app();
        break;

    case OBJ_USER_DISP_MODE:
        ob_user_mode_disp();
        break;

    case OBJ_SUPPLY_DISP_MODE:
        ob_supp_mode_disp();
        break;

    case OBJ_PVT_DISP_MODE:
        ob_pvt_mode_disp();
        break;

    case OBJ_PERIOD_BILLDATE:
        ob_period_billdate();
        break;

    case OBJ_NPERIOD_BILLDATE:
        ob_nperiod_billdate();
        break;

    case OBJ_OUT_SIG_SEL: /*부가 신호 */
        ob_sig_sel();
        break;

    case OBJ_LP_INTERVAL:
        ob_lp_interval();
        break;

    case OBJ_LPAVG_INTERVAL:
        ob_lpavg_interval();
        break;

    case OBJ_TIME_BILLING: /*time stamp*/
        ob_time_billing();
        break;

    case OBJ_MONTH_ENERGY_DELI:
    case OBJ_MONTH_ENERGY_DELI_nPRD:
    case OBJ_MONTH_ENERGY_DELI_SEASON:
        ob_month_energy(eDeliAct);
        break;

    case OBJ_MONTH_ENERGY_RECEI:
    case OBJ_MONTH_ENERGY_RECEI_nPRD:
    case OBJ_MONTH_ENERGY_RECEI_SEASON:
        ob_month_energy(eReceiAct);
        break;

    case OBJ_MONTH_MAXDM_DELI:
    case OBJ_MONTH_MAXDM_DELI_nPRD:
    case OBJ_MONTH_MAXDM_DELI_SEASON:
        ob_month_maxdm(eDeliAct);
        break;

    case OBJ_MONTH_MAXDM_RECEI:
    case OBJ_MONTH_MAXDM_RECEI_nPRD:
    case OBJ_MONTH_MAXDM_RECEI_SEASON:
        ob_month_maxdm(eReceiAct);
        break;

    case OBJ_MONTH_SUBLOCKS:
        ob_month_sublocks();
        break;

    case OBJ_ERR_CODE_1:
        ob_err_code_1();
        break;

    case OBJ_ERR_CODE_2:
        ob_err_code_2();
        break;

    case OBJ_ERR_CODE_3:
        ob_err_code_3();
        break;

    case OBJ_ERR_CODE_4:
        ob_err_code_4();
        break;

    case OBJ_LP_STATUS:
        ob_lp_status();
        break;

    case OBJ_BAT_USE_TIME:
        ob_time_bat_use();
        break;

    case OBJ_BAT_INSTALL_TIME:
        ob_time_bat_inst();
        break;

    case OBJ_SAGEVT_LOG:
        /* Sag 발생 이력 */
        ob_evt_log(eLogSag);
        break;

    case OBJ_SWELLEVT_LOG:
        /* Swell 발생 이력 */
        ob_evt_log(eLogSwell);
        break;

    case OBJ_EVT_LOG:
        /* 이력 기록 */
        ob_evt_log(numLogs);  // groupE event
        break;

    case OBJ_TOU_CAL:
        /* TOU Calendar */
        ob_tou_cal();
        break;

    case OBJ_HOLIDAYS:
        ob_holidays();
        break;

    case OBJ_HDLC_SETUP:
        ob_hdlc_setup();
        break;

    case OBJ_LOAD_PROFILE:
        ob_load_profile();
        break;

    case OBJ_LPAVG:
        ob_lpavg();
        break;

    case OBJ_DATE_TIME:
        ob_date_time();
        break;

    case OBJ_LCDSET_PARM:
        ob_lcdset_parm();
        break;

    case OBJ_DEVICE_ID:
        ob_device_id();
        break;

    case OBJ_LOCAL_TIME:
        ob_local_time();
        break;

    case OBJ_LOCAL_DATE:
        ob_local_date();
        break;

    case OBJ_MAGNET_DURTIME:
        ob_magnet_dur_time();
        break;

    case OBJ_CURR_TARIFF:
        ob_curr_tariff();
        break;

    case OBJ_BILLING_PARM:
        ob_billing_parm();
        break;

    case OBJ_SELECTIVE_ACT:
        ob_selective_act();
        break;

    case OBJ_rLOAD_SIG:
        ob_rLOAD_sig();
        break;

    case OBJ_AVGPF_DELI:
    case OBJ_AVGPF_DELI_nPRD:
    case OBJ_AVGPF_DELI_SEASON:
        ob_rate_pf(eDeliAct);
        break;

    case OBJ_AVGPF_RECEI:
    case OBJ_AVGPF_RECEI_nPRD:
    case OBJ_AVGPF_RECEI_SEASON:
        ob_rate_pf(eReceiAct);
        break;

    case OBJ_LAST15_PF_DELI:
        ob_last15_pf(eDeliAct);
        break;

    case OBJ_LAST15_PF_RECEI:
        ob_last15_pf(eReceiAct);
        break;

    case OBJ_CURR_LAST_DEMAND:
        ob_curr_last_demand();
        break;

    case OBJ_CUM_DEMAND:
    case OBJ_CUM_DEMAND_nPRD:
    case OBJ_CUM_DEMAND_SEASON:
        ob_cum_demand();
        break;

    case OBJ_MAX_DEMAND:
    case OBJ_MAX_DEMAND_nPRD:
    case OBJ_MAX_DEMAND_SEASON:
        ob_max_demand();
        break;

    case OBJ_ENERGY:
    case OBJ_ENERGY_nPRD:
    case OBJ_ENERGY_SEASON:
        ob_rate_energy();
        break;

    case OBJ_ENERGY_FWD_BOTH_REACT: /*시간대별 순병향 무효전력량*/
        ob_rate_both_react_energy(eDeliAct);
        break;

    case OBJ_ENERGY_BWD_BOTH_REACT: /*시간대별 역병향 무효전력량*/
        ob_rate_both_react_energy(eReceiAct);
        break;

    case OBJ_LAST_PGM_CHG:
        ob_last_pgm_chg();
        break;

    case OBJ_FUTURE_PGM_CHG:
        ob_fut_pgm_chg();
        break;

    case OBJ_PGM_CHG_NUM:
        ob_pgm_chg_num();
        break;

    case OBJ_PWR_FAIL_NUM:
        ob_log_cnt(eLogPwrF);
        break;

#if 1  // jp.kim 24.10.28
    case OBJ_Working_PWR_FAIL_NUM:
        ob_log_cnt(eLogWorkPwrF);
        break;
#endif

    case OBJ_MTINIT_NUM:

        ob_log_cnt(eLogMtInit);
        break;

    case OBJ_WRONG_CONN_NUM:
        ob_log_cnt(eLogWrongConn);
        break;

    case OBJ_RTC_CHG_NUM:
        ob_log_cnt(eLogRtcT);
        break;

    case OBJ_aDR_NUM:
        ob_log_cnt(eLogDRa);
        break;

    case OBJ_mDR_NUM:
        ob_log_cnt(eLogDRm);
        break;

    case OBJ_SR_NUM:
        ob_log_cnt(eLogSR);
        break;

    case OBJ_sCURR_nonSEL_NUM:
        ob_log_cnt(eLogSCurrNonSel);
        break;

    case OBJ_sCURR_LIMIT_NUM:
        ob_log_cnt(eLogSCurrLimit);
        break;

    case OBJ_COVER_OPEN_NUM:
        ob_log_cnt(eLogCoverOpen);
        break;

    case OBJ_TCOVER_OPEN_NUM:
        ob_log_cnt(eLogTCoverOpen);
        break;

    case OBJ_MAGNET_DET_NUM:
        ob_log_cnt(eLogMagnetDet);
        break;

    case OBJ_SAG_CNT:
        ob_log_cnt(eLogSag);
        break;

    case OBJ_SWELL_CNT:
        ob_log_cnt(eLogSwell);
        break;

    case OBJ_rLOAD_NUM:
        ob_log_cnt(eLogrLoadCtrl);
        break;

    case OBJ_ERR_DIAGONIST_NUM:
        ob_log_cnt(eLogErrDiagonist);
        break;

    case OBJ_sCURR_autoRTN_VAL:
        ob_sCURR_autortn_val();
        break;

    case OBJ_sCURR_LIMIT_VAL:
        ob_sCURR_limit();
        break;

    case OBJ_sCURR_LIMIT2_VAL:
        ob_sCURR_limit_2();
        break;

    case OBJ_TEMP_THRSHLD:
        ob_temp_thrshld();
        break;

    case OBJ_TEMP_OVER:
        ob_temp_over();
        break;

    case OBJ_SAG_VAL_SET:
        ob_sag_swell(1);
        break;

    case OBJ_SAG_TIME_SET:
        ob_sag_swell(2);
        break;

    case OBJ_SWELL_VAL_SET:
        ob_sag_swell(3);
        break;

    case OBJ_SWELL_TIME_SET:
        ob_sag_swell(4);
        break;

    case OBJ_INST_PROFILE:
        ob_inst_profile();
        break;

    case OBJ_INST_POWER:
        ob_inst_power(0);
        break;

    case OBJ_INST_POWER_L1:
        ob_inst_power(0);
        break;

    case OBJ_INST_POWER_L2:
        ob_inst_power(1);
        break;

    case OBJ_INST_POWER_L3:
        ob_inst_power(2);
        break;

    case OBJ_INST_FREQ:
        ob_inst_freq();
        break;

    case OBJ_INST_CURR_L1:
        ob_inst_curr(0);
        break;

    case OBJ_INST_voltTHD_L1:
        ob_inst_volt_THD(0);
        break;

    case OBJ_INST_VOLT_L1:
        ob_inst_volt(0);
        break;

    case OBJ_INST_PF_L1:
        ob_inst_pf(0);
        break;

    case OBJ_INST_CURR_L2:
        ob_inst_curr(1);
        break;

    case OBJ_INST_voltTHD_L2:
        ob_inst_volt_THD(1);
        break;

    case OBJ_INST_VOLT_L2:
        ob_inst_volt(1);
        break;

    case OBJ_INST_PF_L2:
        ob_inst_pf(1);
        break;

    case OBJ_INST_CURR_L3:
        ob_inst_curr(2);
        break;

    case OBJ_INST_voltTHD_L3:
        ob_inst_volt_THD(2);
        break;

    case OBJ_INST_VOLT_L3:
        ob_inst_volt(2);
        break;

    case OBJ_INST_PF_L3:
        ob_inst_pf(2);
        break;

    case OBJ_INST_PHASE_U12:
        ob_inst_vphase(0);
        break;

    case OBJ_INST_PHASE_U13:
        ob_inst_vphase(1);
        break;

    case OBJ_INST_PHASE_L1:
        ob_inst_phase(0);
        break;

    case OBJ_INST_PHASE_L2:
        ob_inst_phase(1);
        break;

    case OBJ_INST_PHASE_L3:
        ob_inst_phase(2);
        break;

    case OBJ_CURR_TEMP:
        ob_curr_temp();
        break;

    case OBJ_AVG_CURR_L1:
        ob_avg_curr(0);
        break;

    case OBJ_AVG_CURR_L2:
        ob_avg_curr(1);
        break;

    case OBJ_AVG_CURR_L3:
        ob_avg_curr(2);
        break;

    case OBJ_AVG_VOLT_L1:
        ob_avg_volt(0);
        break;

    case OBJ_AVG_VOLT_L2:
        ob_avg_volt(1);
        break;

    case OBJ_AVG_VOLT_L3:
        ob_avg_volt(2);
        break;

    case OBJ_AVG_VOLT_L1_L2:
        ob_avg_volt_ltol(0);
        break;

    case OBJ_AVG_VOLT_L2_L3:
        ob_avg_volt_ltol(1);
        break;

    case OBJ_AVG_VOLT_L3_L1:
        ob_avg_volt_ltol(2);
        break;

    case OBJ_IMAXLOAD_L1:
        ob_imax_log(0);
        break;

    case OBJ_IMAXLOAD_L2:
        ob_imax_log(1);
        break;

    case OBJ_IMAXLOAD_L3:
        ob_imax_log(2);
        break;

    case OBJ_REM_ENERGY:
        ob_prepay_remenergy();
        break;

    case OBJ_BUY_ENERGY:
        ob_prepay_buyenergy();
        break;

    case OBJ_PREPAY_ENABLE:
        ob_prepay_enable();
        break;

    case OBJ_PREPAY_LOADLIMIT_CANCEL:
        ob_prepay_loadlimit_cancel();
        break;

    case OBJ_CONDENSOR_INST:
        ob_condensor_inst();
        break;

    case OBJ_TS_CONF:
        ob_ts_conf();
        break;

    case OBJ_SEL_REACT: /*선택 무효전력량 개별 예약설정*/
        ob_sel_react();
        break;

    case OBJ_LP_OVERLAPED_INDEX:
        ob_lp_overlaped_index();
        break;

    case OBJ_COMM_ENABLE:
        ob_comm_enable();
        break;

    case OBJ_KEY_VALUE:
        ob_key_value();
        break;

    case OBJ_LCD_MAP:
        ob_lcd_map();
        break;

    case OBJ_sCURR_HOLD: /*부하제한 감지시간*/
        ob_scurr_hold();
        break;

    case OBJ_sCURR_RECOVER_N1: /*부하제한 제 1구간 재복귀시간*/
        ob_scurr_rtn_n1();
        break;

    case OBJ_sCURR_RECOVER_N2: /*부하제한 제 2구간 재복귀시간*/
        ob_scurr_rtn_n2();
        break;

    case OBJ_sCURR_COUNTER_N1: /*부하제한 제 1구간 차단 설정 횟수*/
        ob_scurr_cnt_n1();
        break;

    case OBJ_LATCHON_COUNTER: /*래치 ON 횟수*/
        ob_latchon_counter();
        break;

    case OBJ_MIN_INST_FREQ: /*순시주파수 최소값*/
        ob_min_freq();
        break;

    case OBJ_MAX_INST_FREQ:
        ob_max_freq();
        break;

#if 1 /* bccho, 2024-09-05, 삼상 */
    case OBJ_MIN_INST_VOLT_L1:
        ob_min_volt(0);
        break;

    case OBJ_MAX_INST_VOLT_L1:
        ob_max_volt(0);
        break;

    case OBJ_MIN_INST_VOLT_L2:
        ob_min_volt(1);
        break;

    case OBJ_MAX_INST_VOLT_L2:
        ob_max_volt(1);
        break;

    case OBJ_MIN_INST_VOLT_L3:
        ob_min_volt(2);
        break;

    case OBJ_MAX_INST_VOLT_L3:
        ob_max_volt(2);
        break;
#else
    case OBJ_MIN_INST_VOLT:
        ob_min_volt();
        break;

    case OBJ_MAX_INST_VOLT:
        ob_max_volt();
        break;
#endif

    case OBJ_OVERCURR_ENABLE: /*과부하전류 차단 설정/해제*/
        ob_overcurr_enable();
        break;

    case OBJ_ADJ_FACTOR: /*private obis - 순시 온도*/
        ob_adj_factor();
        break;

    case OBJ_TOU_SET_CNT: /*TOU 수신 상태정보*/
        ob_tou_set_cnt();
        break;

    case OBJ_EXT_PROG_ID:
        ob_ext_prog_id();
        break;

    case OBJ_SAP_ASSIGNM:
        ob_sap_assignment();

        break;
    case OBJ_SECURITY_SETUP_3:
    case OBJ_SECURITY_SETUP_4:
        ob_security_setup();
        break;

    case OBJ_SEC_EVT_LOG_HISTORY:
        ob_evt_cert_log(eLogCert_NG);
        break;

    case OBJ_SEC_EVT_LOG_NUM:
        ob_cert_log_cnt(eLogCert_NG);
        break;

#if 1  // jp.kim 25.12.03  보안로그 개별 obis 읽기 fail 종류 추가
    case OBJ_SEC_EVT_LOG_CASE:
        ob_cert_log_case(eLogCert_NG);
        break;
#endif

    case OBJ_TOU_IMAGE_TRANSFER:
        ob_evt_tou_imagetransfer();
        break;

    case OBJ_SW_INFO:
        ob_evt_fw_info();
        break;

    case OBJ_SW_APPLY_DATE:
        ob_evt_fw_apply_date();
        break;

    case OBJ_SW_IMAGE_TRANSFER:
        ob_evt_fw_imagetransfer();
        break;

    case OBJ_NMS_DMS_ID:
        ob_nms_dms_id();
        break;

    case OBJ_USE_AMR_DATA_NUM:
        ob_use_mr_data_num();
        break;

    case OBJ_RUN_MODEM_INFO:
        ob_run_modem_info();
        break;

    case OBJ_RTIME_P_ENERGY:
        ob_realtime_p_energy();
        break;

    case OBJ_RTIME_P_LP:
        ob_realtime_p_load_profile();
        break;

    case OBJ_RTIME_P_LP_INTERVAL:
        ob_realtime_lp_interval();
        break;

    case OBJ_PHASE_DET_CONT_TIME:
        // 기능 구현 필요..
        ob_zcrs_sig_out_durtime();
        break;

    case OBJ_PHASE_DET_CORRECT_VAL:
        // 기능 구현 필요..
        ob_zcrs_sig_out_cmpstime();
        break;

    case OBJ_PHASE_DET_RESULT_VAL:
        ob_zcrs_sig_out_resulttime();
        break;

    case OBJ_PERMITxx_TIME_LIMIT:
        // 기능 구현 필요..
        ob_tmsync_range();
        break;

    case OBJ_SELF_ERR_REF_VAL:
        // 기능 구현 필요..
        ob_self_error_ref();
        break;

    case OBJ_CT_RATIO:
        FILL_U16(ct_ratio_prog);
        break;

    case OBJ_PT_RATIO:
        FILL_U16(pt_ratio_prog);
        break;

    case OBJ_METERING_TYPE_SEL: /*게량 종별 선택*/
                                // 기능 구현 필요..
        FILL_BOOL(auto_mode_sel);
        break;

    case OBJ_EVENT_INFO:
        ob_event_info_profile();
        break;

    case OBJ_INT_PLC_MODEM_ATCMD:  // jp.kim 25.01.20
        FILL_STRING(0);            // jp.kim 25.01.22
        break;
    case OBJ_INT_MODEM_ATCMD:
        ob_modem_atcmd_forSet(INT_MODEM_TYPE);
        break;
    case OBJ_EXT_MODEM_ATCMD:
        ob_modem_atcmd_forSet(EXT_MODEM_TYPE);  // jp.kim 25.01.22
        break;

    case OBJ_INT_PLC_MODEM_ATCMD_RSP:  // jp.kim 25.01.20
        FILL_STRING(0);                // jp.kim 25.01.22
        break;
    case OBJ_INT_MODEM_ATCMD_RSP:
        ob_modem_atcmd_forRsp(INT_MODEM_TYPE);  // jp.kim 25.01.22
        break;
    case OBJ_EXT_MODEM_ATCMD_RSP:
        // jp.kim 25.01.22
        if (gst_atcmd_from_modem[EXT_MODEM_TYPE].len <= (1024 + 300))
        {
            DPRINTF(DBG_TRACE, _D "%s 1 pPdu_idx[%d] \r\n", __func__, pPdu_idx);
            ob_modem_atcmd_forRsp(EXT_MODEM_TYPE);
            DPRINTF(DBG_TRACE, "%s:  2 pPdu_idx[%d]\r\n", __func__, pPdu_idx);
            break;
        }
        else
        {  // jp.kim 25.01.22
            DPRINTF(DBG_TRACE, _D "%s 4  pPdu_idx[%d] \r\n", __func__,
                    pPdu_idx);
            getresp_LP_len = gst_atcmd_from_modem[EXT_MODEM_TYPE].len;
            getresp_LP_index = 0;
            FILL_STRING_2(getresp_LP_len);  // jp.kim 25.01.21
            approc_fill_get_atcmd_resp_block(true);
#if 0
				set_test_block_rx = 1;
#endif
            break;
        }

        break;

    case OBJ_PUSH_ACT_ERR_CODE_1:
    case OBJ_PUSH_ACT_ERR_CODE_2:
    case OBJ_PUSH_ACT_ERR_CODE_3:
    case OBJ_PUSH_ACT_ERR_CODE_4:
        ob_evt_err_code_activate(obis_ge);
        break;

    case OBJ_PUSH_SCRIPT_TABLE:
        ob_evt_push_script();
        break;

    case OBJ_PUSH_SETUP_ERR_CODE:
        ob_evt_push_setup_err_code();
        break;

    case OBJ_PUSH_SETUP_LAST_LP:
        ob_evt_push_setup_lastLP();
        break;

    case OBJ_EXT_MODEM_ID:
        ob_ext_modem_id();
        break;

    case OBJ_STOCK_OP_TIMES:
        ob_stock_op_times();
        break;

    case OBJ_OLD_METER_TOU_TRANSFER:
        break;

    case OBJ_LP_TOTAL_CNT:
        ob_lp_tatal_cnt();
        break;

    case OBJ_HOLIDAY_SEL:
#if 0
		FILL_BOOL(holiday_sel);
#else
        ob_holiday_sel();
#endif
        break;

    case OBJ_THD_PERIOD_SEL:
        ob_thd_rec_period();
        break;

    case OBJ_SW_UP_CNT_0:
        ob_log_cnt(eLogSysSwUp);
        break;
    case OBJ_SW_UP_CNT_1:
        ob_log_cnt(eLogMtrSwUp);
        break;

    case OBJ_SW_UP_CNT_2:
        ob_log_cnt(eLogInModemUp);
        break;
    case OBJ_SW_UP_CNT_3:
        ob_log_cnt(eLogExModemUp);
        break;

    case OBJ_SW_UP_LOG_0:
        ob_evt_log(eLogSysSwUp);
        break;
    case OBJ_SW_UP_LOG_1:
        ob_evt_log(eLogMtrSwUp);
        break;

    case OBJ_SW_UP_LOG_2:
        ob_evt_log(eLogInModemUp);
        break;

    case OBJ_SW_UP_LOG_3:
        ob_evt_log(eLogExModemUp);
        break;

#if 1  // jp.kim 24.10.28
    case OBJ_Working_PWR_FAIL_LOG:
        ob_evt_log(eLogWorkPwrF);
        break;
#endif

    case OBJ_MAX_DEMAND__SIGN:
    case OBJ_MAX_DEMAND_nPRD__SIGN:
    case OBJ_MAX_DEMAND_SEASON__SIGN:

        ob_max_demand_sign();
        break;

    case OBJ_ENERGY__SIGN:
    case OBJ_ENERGY_nPRD__SIGN:
    case OBJ_ENERGY_SEASON__SIGN:

        ob_energy_sign();

        break;

    case OBJ_WORKING_FAULT_MIN:
        ob_working_fault_min();
        break;

    case OBJ_TOU_ID_CHANGE_STS:
        ob_tou_id_change_sts();
        break;

    case OBJ_SYS_TITLE:
        ob_sys_title_server();
        break;

    case OBJ_INSTALL_CERT:
        ob_inst_cert();
        break;

    case OBJ_INSTALL_KEY:
        ob_inst_key();
        break;

    default:
        break;
    }

    if (pPdu_idx == pPdu_idx_start)
    {
        appl_resp_result = GET_RESULT_OTHER_REASON;
        DPRINTF(DBG_TRACE, _D "%s ######\r\n", __func__);
    }
}

static void approc_fill_get_resp_block(bool first)
{
    uint16_t len, arrlen;
    int idx_start;
    int tmp;

    at_cmd_rsp_ing = 0;

    appl_resp_choice =
        GET_RES_BLOCK;  // in case of when called in approc_get_req_normal()
    pPdu_idx = APPL_FILL_RESP_DATA_IDX_BLOCK;
    pPdu_idx += 3;  // block length field (FILL_LEN_2)

    idx_start = pPdu_idx;

    if (first)
    {
        appl_resp_block_num = 1L;
        // static variable initialization
        appl_idx1_for_block = 0;
        appl_idx2_for_block = 0;

        pPdu_idx += 4;  // total block array number (FILL_ARRAY_2)
    }
    else
    {
        appl_resp_block_num += 1L;
    }

    switch (appl_obj_id)
    {
    case OBJ_ASSOCIATION_LN:
        if (appl_is_sap_public() || appl_is_sap_utility())
        {
            arrlen = NUM_MYOBJ_DEV_MANAGEMENT;
        }
        else if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
        {
            arrlen = NUM_TOTAL_MYOBJ_SEC;
        }
        else if (appl_is_sap_485comm())
        {
            arrlen = NUM_MYOBJ_485COMM;
        }
        else
        {
            arrlen = NUM_MYOBJ_PRIVATE;
        }
        object_list_element_proc();
        break;

    case OBJ_TOU_CAL:  // day_profile (att_id = 5 or 9)
        arrlen = DAY_PROF_SIZE;
        if ((appl_att_id == 9) || !act_is_curr_prog_cmd())
        {
            fill_day_prof_struct(first, false);
        }
        else
        {
            fill_day_prof_struct(first, true);
        }
        break;

    case OBJ_HOLIDAYS:
        arrlen = HOLIDAY_LEN;
        if (act_is_curr_prog_cmd())
        {
            fill_holidays_struct(first, true);
        }
        else
        {
            fill_holidays_struct(first, false);
        }
        break;

    case OBJ_LOAD_PROFILE:
        arrlen = getresp_LP_len;  // record length
        fill_lp_record();
        break;

    case OBJ_LPAVG:
        arrlen = getresp_LP_len;  // record length
        fill_lpavg_record();
        break;

    case OBJ_RTIME_P_LP:
        arrlen = getresp_LP_len;  // record length
        fill_lprt_record();
        break;

    default:

        break;
    }

    // block length
    len = (uint16_t)(pPdu_idx - idx_start);
    // index backup
    tmp = pPdu_idx;
    // block length field and array length field
    pPdu_idx = idx_start - 3;
    FILL_LEN_2(len);
    if (first)
    {
        FILL_ARRAY_2(arrlen);
    }
    // index restore
    pPdu_idx = tmp;
}

void at_cmd_rsp_list_element_proc(void);
#if 1  // jp.kim 25.01.22
static void approc_fill_get_atcmd_resp_block(bool first)
{
    U16 len, arrlen;
    int idx_start;
    int tmp;

    DPRINTF(DBG_TRACE, _D "%s 0 pPdu_idx[%d]  getresp_LP_len[%d] \r\n",
            __func__, pPdu_idx, getresp_LP_len);

    at_cmd_rsp_ing = 1;

    appl_resp_choice =
        GET_RES_BLOCK;  // in case of when called in approc_get_req_normal()

    pPdu_idx = APPL_FILL_RESP_DATA_IDX_BLOCK;

    pPdu_idx += 3;  // block length field (FILL_LEN_2)

    idx_start = pPdu_idx;

    if (first)
    {
        appl_resp_block_num = 1L;

        // static variable initialization
        appl_idx1_for_block = 0;
        appl_idx2_for_block = 0;

        pPdu_idx += 4;  // total block array number (FILL_ARRAY_2)
    }
    else
    {
        appl_resp_block_num += 1L;
    }

    arrlen = getresp_LP_len;  // record length

    DPRINTF(DBG_TRACE, _D "%s 1 pPdu_idx[%d] \r\n", __func__, pPdu_idx);

    at_cmd_rsp_list_element_proc();

    DPRINTF(DBG_TRACE, _D "%s 2 pPdu_idx[%d] \r\n", __func__, pPdu_idx);

    // block length
    len = (U16)(pPdu_idx - idx_start);

    // index backup
    tmp = pPdu_idx;

    // block length field and array length field
    pPdu_idx = idx_start - 3;
    FILL_LEN_2(len);
    if (first)
    {
        FILL_STRING_2(arrlen);
    }

    // index restore
    pPdu_idx = tmp;
    DPRINTF(DBG_TRACE, _D "%s pPdu_idx[%d] \r\n", __func__, pPdu_idx);
}
#endif

static void fill_lp_record(void)
{
    uint8_t reclen;

    if (0)
    {
        reclen =
            (uint8_t)((getresp_LP_len >= BLOCK_LP_UNIT_UNI) ? BLOCK_LP_UNIT_UNI
                                                            : getresp_LP_len);
    }
    else
    {
        reclen = (uint8_t)((getresp_LP_len >= BLOCK_LP_UNIT_BOTH)
                               ? BLOCK_LP_UNIT_BOTH
                               : getresp_LP_len);
    }

    // read from nv_memory
    LP_read(appl_tbuff, getresp_LP_index, reclen, mt_dir_for_lp);
    // encode to packet data unit
    LP_record_to_pPdu(appl_tbuff, reclen);

    getresp_LP_index -= (uint32_t)reclen;
    getresp_LP_len -= (uint16_t)reclen;

    if (getresp_LP_len == 0)
        appl_resp_last_block = 1;  // last block
    else
        appl_resp_last_block = 0;  // non-last block
}

static void LP_record_to_pPdu(uint8_t* recbuff, uint8_t len)
{
    uint8_t i;
    uint8_t recsize;
    date_time_type dt;
    lp_record_type* lp;

    recsize = recsize_for_lp;

    for (i = 0; i < len; i++)
    {
        lp = (lp_record_type*)recbuff;

        FILL_STRUCT(getresp_LP_entry_sels_no);

        if (getresp_LP_entry_sels & LP_COL_LP_CNT_BIT)
        {
            FILL_U32(lp->lp_cnt);
        }

        if (getresp_LP_entry_sels & LP_COL_ACT_Q14_BIT)
        {
            FILL_U32(lp->ch[eChDeliAct]);
        }

        if (getresp_LP_entry_sels & LP_COL_REACT_Q1_BIT)
        {
            FILL_U32(lp->ch[eChDLagReact]);
        }

        if (getresp_LP_entry_sels & LP_COL_REACT_Q4_BIT)
        {
            FILL_U32(lp->ch[eChDLeadReact]);
        }

        if (getresp_LP_entry_sels & LP_COL_APP_Q14_BIT)
        {
            FILL_U32(lp->ch[eChDeliApp]);
        }

        if (getresp_LP_entry_sels & LP_COL_CLOCK)
        {
            expand_time(&dt, &lp->dt[0]);
            fill_clock_obj(&dt);
        }

        if (getresp_LP_entry_sels & LP_COL_STATUS)
        {
            FILL_BS(24);
            FILL_V8(lp->evt[0]);
            FILL_V8(lp->evt[1]);
            FILL_V8(lp->evt[2]);
        }

        if (getresp_LP_entry_sels & LP_COL_ACT_Q23_BIT)
        {
            FILL_U32(lp->ch[eChReceiAct]);
        }

        if (getresp_LP_entry_sels & LP_COL_REACT_Q2_BIT)
        {
            FILL_U32(lp->ch[eChRLeadReact]);
        }

        if (getresp_LP_entry_sels & LP_COL_REACT_Q3_BIT)
        {
            FILL_U32(lp->ch[eChRLagReact]);
        }

        if (getresp_LP_entry_sels & LP_COL_APP_Q23_BIT)
        {
            FILL_U32(lp->ch[eChReceiApp]);
        }

        recbuff += recsize;
    }
}

static void get_LP_selcolmn_info(uint8_t from, uint8_t to)
{
    uint8_t col_num;
    uint16_t col_mask;

    if (0)
    {
        col_num = NUM_LP_COL_UNI;
        col_mask = LP_COL_MASK_UNI;
    }
    else
    {
        col_num = NUM_LP_COL_BOTH;
        col_mask = LP_COL_MASK_BOTH;
    }

    if (to == 0 || to > col_num)
        to = col_num;

    getresp_LP_entry_sels = (col_mask >> (from - 1));
    getresp_LP_entry_sels &= (col_mask << (col_num - to));

    getresp_LP_entry_sels_no = to - from + 1;
}

static void fill_lpavg_record(void)
{
    uint8_t reclen;

    reclen = (uint8_t)((getresp_LP_len >= BLOCK_LPAVG_UNIT) ? BLOCK_LPAVG_UNIT
                                                            : getresp_LP_len);

    // read from nv_memory
    lpavg_record_read(appl_tbuff, getresp_LP_index, reclen);
    // encode to packet data unit
    LPavg_record_to_pPdu(appl_tbuff, reclen);

    getresp_LP_index -= (uint32_t)reclen;
    getresp_LP_len -= (uint16_t)reclen;

    if (getresp_LP_len == 0)
        appl_resp_last_block = 1;  // last block
    else
        appl_resp_last_block = 0;  // non-last block
}

static void LPavg_record_to_pPdu(uint8_t* recbuff, uint8_t len)
{
    uint8_t i;
    uint8_t recsize;
    float fval;
    date_time_type dt;
    lpavg_record_type* lpavg;

    recsize = sizeof(lpavg_record_type);

    if (mt_is_onephase())
    {
        for (i = 0; i < len; i++)
        {
            lpavg = (lpavg_record_type*)recbuff;

            FILL_STRUCT(getresp_LP_entry_sels_no);

            if (getresp_LP_entry_sels & LPAVG_COL_CLOCK)
            {
                expand_time(&dt, &lpavg->dt[0]);

                fill_clock_obj(&dt);
            }

            if (getresp_LP_entry_sels & LPAVG_COL_V)
            {
                fval = (float)lpavg->ch[0] / 100.0;
                FILL_FLOAT(fval);
            }

            if (getresp_LP_entry_sels & LPAVG_COL_THD)
            {
                fval = (float)lpavg->ch[1] / 100.0;
                FILL_FLOAT(fval);
            }

            if (getresp_LP_entry_sels & LPAVG_COL_I)
            {
                fval = (float)lpavg->ch[2] / 100.0;
                FILL_FLOAT(fval);
            }

            if (getresp_LP_entry_sels & LPAVG_COL_PHASE)
            {
                fval = (float)lpavg->ch[3] / 100.0;
                FILL_FLOAT(fval);
            }

            recbuff += recsize;
        }
    }
    else
    {
        for (i = 0; i < len; i++)
        {
            lpavg = (lpavg_record_type*)recbuff;

            /* bccho, 2024-09-05, 삼상 */
            FILL_STRUCT(getresp_LP_entry_sels_no);

            if (getresp_LP_entry_sels & LPAVG_COL_CLOCK)
            {
                expand_time(&dt, &lpavg->dt[0]);

                fill_clock_obj(&dt);
            }

            if (getresp_LP_entry_sels & LPAVG_COL_V1_L1_2)
            {
                fval = (float)lpavg->ch[0] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_V1)
            {
                fval = (float)lpavg->ch[1] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_THD1)
            {
                fval = (float)lpavg->ch[2] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_I1)
            {
                fval = (float)lpavg->ch[3] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_PHASE1)
            {
                fval = (float)lpavg->ch[4] / 100.0;
                FILL_FLOAT(fval);
            }

#if 1 /* bccho, 2024-09-05, 삼상 */
            //(삼상 변성기부 계기에 한함, 단독계기의 경우 0으로 전송)
            fval = 0.0;
            FILL_FLOAT(fval);

            if (getresp_LP_entry_sels & LPAVG_COL_V2_L2_3)
            {
                fval = (float)lpavg->ch[6] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_V1)
            {
                fval = (float)lpavg->ch[7] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_THD2)
            {
                fval = (float)lpavg->ch[8] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_I2)
            {
                fval = (float)lpavg->ch[9] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_PHASE2)
            {
                fval = (float)lpavg->ch[10] / 100.0;
                FILL_FLOAT(fval);
            }

            //(삼상 변성기부 계기에 한함, 단독계기의 경우 0으로 전송)
            fval = 0.0;
            FILL_FLOAT(fval);

            if (getresp_LP_entry_sels & LPAVG_COL_V3_L3_1)
            {
                fval = (float)lpavg->ch[12] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_V3)
            {
                fval = (float)lpavg->ch[13] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_THD3)
            {
                fval = (float)lpavg->ch[14] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_I3)
            {
                fval = (float)lpavg->ch[15] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_PHASE3)
            {
                fval = (float)lpavg->ch[16] / 100.0;
                FILL_FLOAT(fval);
            }

            //(삼상 변성기부 계기에 한함, 단독계기의 경우 0으로 전송)
            fval = 0.0;
            FILL_FLOAT(fval);
#else
            if (getresp_LP_entry_sels & LPAVG_COL_V2_L2_3)
            {
                fval = (float)lpavg->ch[5] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_V1)
            {
                fval = (float)lpavg->ch[6] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_THD2)
            {
                fval = (float)lpavg->ch[7] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_I2)
            {
                fval = (float)lpavg->ch[8] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_PHASE2)
            {
                fval = (float)lpavg->ch[9] / 100.0;
                FILL_FLOAT(fval);
            }

            if (getresp_LP_entry_sels & LPAVG_COL_V3_L3_1)
            {
                fval = (float)lpavg->ch[10] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_V3)
            {
                fval = (float)lpavg->ch[11] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_THD3)
            {
                fval = (float)lpavg->ch[12] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_I3)
            {
                fval = (float)lpavg->ch[13] / 100.0;
                FILL_FLOAT(fval);
            }
            if (getresp_LP_entry_sels & LPAVG_COL_PHASE3)
            {
                fval = (float)lpavg->ch[14] / 100.0;
                FILL_FLOAT(fval);
            }
#endif
            recbuff += recsize;
        }
    }
}

void LP_last_record_to_pPdu(uint8_t* recbuff)
{
    date_time_type dt;
    lp_record_type* lp = (lp_record_type*)recbuff;
    ;

    uint32_t t32 = TOTAL_LP_EVENT_CNT;

    FILL_STRUCT(NUM_LP_COL_BOTH);
    FILL_U32(t32);
    FILL_U32(lp->ch[eChDeliAct]);
    FILL_U32(lp->ch[eChDLagReact]);
    FILL_U32(lp->ch[eChDLeadReact]);
    FILL_U32(lp->ch[eChDeliApp]);

    expand_time(&dt, &lp->dt[0]);
    fill_clock_obj(&dt);

    FILL_BS(24);
    FILL_V8(lp->evt[0]);
    FILL_V8(lp->evt[1]);
    FILL_V8(lp->evt[2]);

    FILL_U32(lp->ch[eChReceiAct]);
    FILL_U32(lp->ch[eChRLeadReact]);
    FILL_U32(lp->ch[eChRLagReact]);
    FILL_U32(lp->ch[eChReceiApp]);
}

static void get_LPavg_selcolmn_info(uint8_t from, uint8_t to)
{
    if (mt_is_onephase())
    {
        if (to == 0 || to > NUM_LPAVG_COL)
            to = NUM_LPAVG_COL;

        getresp_LP_entry_sels = (LPAVG_COL_MASK >> (from - 1));
        getresp_LP_entry_sels &= (LPAVG_COL_MASK << (NUM_LPAVG_COL - to));
    }
    else
    {
        if (to == 0 || to > NUM_LPAVG_COL_3PHS)
            to = NUM_LPAVG_COL_3PHS;

        getresp_LP_entry_sels = (LPAVG_COL_MASK_3PHS >> (from - 1));
        getresp_LP_entry_sels &=
            (LPAVG_COL_MASK_3PHS << (NUM_LPAVG_COL_3PHS - to));
    }
    getresp_LP_entry_sels_no = to - from + 1;
}

static void fill_lprt_record(void)
{
    uint8_t reclen;

    if (mt_is_onephase())
    {
        reclen = (uint8_t)((getresp_LP_len >= BLOCK_LPRT_UNIT_1PHS)
                               ? BLOCK_LPRT_UNIT_1PHS
                               : getresp_LP_len);
    }
    else
    {
        reclen = (uint8_t)((getresp_LP_len >= BLOCK_LPRT_UNIT_3PHS)
                               ? BLOCK_LPRT_UNIT_3PHS
                               : getresp_LP_len);
    }

    // read from nv_memory
    lprt_record_read(appl_tbuff, getresp_LP_index, reclen);
    // encode to packet data unit
    LPrt_record_to_pPdu(appl_tbuff, reclen);

    getresp_LP_index -= (uint32_t)reclen;
    getresp_LP_len -= (uint16_t)reclen;

    if (getresp_LP_len == 0)
        appl_resp_last_block = 1;  // last block
    else
        appl_resp_last_block = 0;  // non-last block
}

static void LPrt_record_to_pPdu(uint8_t* recbuff, uint8_t len)
{
    uint8_t i;
    uint8_t recsize;
    uint32_t t32;
    float tfloat;
    date_time_type dt;
    lprt_record_1phs* lprt_1phs;
    lprt_record_3phs* lprt_3phs;

    for (i = 0; i < len; i++)
    {
        FILL_STRUCT(2);

        if (mt_is_onephase())
        {
            recsize = sizeof(lprt_record_1phs);
            lprt_1phs = (lprt_record_1phs*)recbuff;

            if (getresp_LPrt_entry_sels & LPRT_COL_CLOCK)
            {
                expand_time(&dt, &lprt_1phs->dt[0]);
                fill_clock_obj(&dt);
            }

            FILL_STRUCT(getresp_LP_entry_sels_no - 1);

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_POS_BIT)
            {
                tfloat = lprt_1phs->ch_2[0];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_NEG_BIT)
            {
                tfloat = lprt_1phs->ch_2[1];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q1Q2_BIT)
            {
                tfloat = lprt_1phs->ch_2[2];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q3Q4_BIT)
            {
                tfloat = lprt_1phs->ch_2[3];
                FILL_FLOAT(tfloat);
            }

            recbuff += recsize;
        }
        else
        {
            recsize = sizeof(lprt_record_3phs);

            lprt_3phs = (lprt_record_3phs*)recbuff;

            if (getresp_LPrt_entry_sels & LPRT_COL_CLOCK)
            {
                expand_time(&dt, &lprt_3phs->dt[0]);

                fill_clock_obj(&dt);
            }

            FILL_STRUCT(getresp_LP_entry_sels_no - 1);

#if 1 /* bccho, 2024-09-05, 삼상 */
            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_POS_BIT)
            {
                tfloat = lprt_3phs->ch_2[0];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_NEG_BIT)
            {
                tfloat = lprt_3phs->ch_2[1];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q1Q2_BIT)
            {
                tfloat = lprt_3phs->ch_2[2];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q3Q4_BIT)
            {
                tfloat = lprt_3phs->ch_2[3];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_POS_A_BIT)
            {
                tfloat = lprt_3phs->ch_2[4];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_NEG_A_BIT)
            {
                tfloat = lprt_3phs->ch_2[5];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q1Q2_A_BIT)
            {
                tfloat = lprt_3phs->ch_2[6];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q3Q4_A_BIT)
            {
                tfloat = lprt_3phs->ch_2[7];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_POS_B_BIT)
            {
                tfloat = lprt_3phs->ch_2[8];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_NEG_B_BIT)
            {
                tfloat = lprt_3phs->ch_2[9];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q1Q2_B_BIT)
            {
                tfloat = lprt_3phs->ch_2[10];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q3Q4_B_BIT)
            {
                tfloat = lprt_3phs->ch_2[11];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_POS_C_BIT)
            {
                tfloat = lprt_3phs->ch_2[12];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_NEG_C_BIT)
            {
                tfloat = lprt_3phs->ch_2[13];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q1Q2_C_BIT)
            {
                tfloat = lprt_3phs->ch_2[14];
                FILL_FLOAT(tfloat);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q3Q4_C_BIT)
            {
                tfloat = lprt_3phs->ch_2[15];
                FILL_FLOAT(tfloat);
            }
#else
            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_POS_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[0];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_NEG_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[1];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q1Q2_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[2];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q3Q4_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[3];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_POS_A_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[4];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_NEG_A_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[5];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q1Q2_A_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[6];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q3Q4_A_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[7];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_POS_B_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[8];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_NEG_B_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[9];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q1Q2_B_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[10];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q3Q4_B_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[11];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_POS_C_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[12];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_ACT_NEG_C_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[13];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q1Q2_C_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[14];
                FILL_U32(t32);
            }

            if (getresp_LPrt_entry_sels & LPRT_COL_REACT_Q3Q4_C_BIT)
            {
                t32 = (uint32_t)lprt_3phs->ch_2[15];
                FILL_U32(t32);
            }
#endif
            recbuff += recsize;
        }
    }
}

static void LPrt_energy_to_pPdu(uint8_t* recbuff)
{
    if (mt_is_onephase())
    {
        lprt_record_1phs* lprt_1phs;
        lprt_1phs = (lprt_record_1phs*)recbuff;

        FILL_U32(lprt_1phs->ch_2[0]);
        FILL_U32(lprt_1phs->ch_2[1]);
        FILL_U32(lprt_1phs->ch_2[2]);
        FILL_U32(lprt_1phs->ch_2[3]);
        FILL_U32(lprt_1phs->ch_2[4]);
        FILL_U32(lprt_1phs->ch_2[5]);
    }
    else
    {
        lprt_record_3phs* lprt_3phs;
        lprt_3phs = (lprt_record_3phs*)recbuff;

        FILL_U32(lprt_3phs->ch_2[0]);
        FILL_U32(lprt_3phs->ch_2[1]);
        FILL_U32(lprt_3phs->ch_2[2]);
        FILL_U32(lprt_3phs->ch_2[3]);
        FILL_U32(lprt_3phs->ch_2[4]);
        FILL_U32(lprt_3phs->ch_2[5]);

        FILL_U32(lprt_3phs->ch_2[6]);
        FILL_U32(lprt_3phs->ch_2[7]);
        FILL_U32(lprt_3phs->ch_2[8]);
        FILL_U32(lprt_3phs->ch_2[9]);
        FILL_U32(lprt_3phs->ch_2[10]);
        FILL_U32(lprt_3phs->ch_2[11]);

        FILL_U32(lprt_3phs->ch_2[12]);
        FILL_U32(lprt_3phs->ch_2[13]);
        FILL_U32(lprt_3phs->ch_2[14]);
        FILL_U32(lprt_3phs->ch_2[15]);
        FILL_U32(lprt_3phs->ch_2[16]);
        FILL_U32(lprt_3phs->ch_2[17]);

        FILL_U32(lprt_3phs->ch_2[18]);
        FILL_U32(lprt_3phs->ch_2[19]);
        FILL_U32(lprt_3phs->ch_2[20]);
        FILL_U32(lprt_3phs->ch_2[21]);
        FILL_U32(lprt_3phs->ch_2[22]);
        FILL_U32(lprt_3phs->ch_2[23]);
    }
}

static void get_LPrt_selcolmn_info(uint8_t from, uint8_t to)
{
    if (mt_is_onephase())
    {
        if (to == 0 || to > NUM_LPRT_COL_1PHS)
            to = NUM_LPRT_COL_1PHS;

        getresp_LPrt_entry_sels = (LPRT_COL_MASK_1PHS >> (from - 1));
        getresp_LPrt_entry_sels &=
            (LPRT_COL_MASK_1PHS << (NUM_LPRT_COL_1PHS - to));

        getresp_LP_entry_sels_no = to - from + 1;
    }
    else
    {
        if (to == 0 || to > NUM_LPRT_COL_3PHS)
            to = NUM_LPRT_COL_3PHS;

        getresp_LPrt_entry_sels = (LPRT_COL_MASK_3PHS >> (from - 1));
        getresp_LPrt_entry_sels &=
            (LPRT_COL_MASK_3PHS << (NUM_LPRT_COL_3PHS - to));

        getresp_LP_entry_sels_no = to - from + 1;
    }

    DPRINTF(DBG_TRACE, "%s: from[%d], to[%d], sels[0x%08X], num[%d]\r\n",
            __func__, from, to, getresp_LPrt_entry_sels,
            getresp_LP_entry_sels_no);
}

static void fill_register_scale(int8_t scaler, uint8_t unit)
{
    FILL_STRUCT(2);

    FILL_S08(scaler);
    FILL_ENUM(unit);
}

static void fill_octet_string_x(uint8_t* str, int len)
{
    if (len > 120)  // jp.kim 25.01.21
    {
        FILL_STRING_2(len);  // jp.kim 25.01.21
    }
    else
    {
        FILL_STRING(len);
    }

    for (; len > 0; len--)
    {
        FILL_V8(*str++);
    }
}

static void fill_clock_obj(date_time_type* dt)
{
    uint16_t val;

    FILL_STRING(0x0c);

    if (dt->year == 0xff)
    {
        val = 0xffff;
    }
    else
    {
        val = BASE_YEAR + (uint16_t)dt->year;
    }
    FILL_V16(val);

    FILL_V8(dt->month);
    FILL_V8(dt->date);
    FILL_V8(calc_dayofweek(dt));  // if(day == 0xff) -> return 0xff
    FILL_V8(dt->hour);
    FILL_V8(dt->min);
    FILL_V8(dt->sec);
    FILL_V8(0xff);  // hundredths
    FILL_V8(0xff);
    FILL_V8(0xff);
    FILL_V8(0xff);
}

uint8_t clock_to_format_for_sign(date_time_type* dt, uint8_t* p_odata)
{
    uint16_t val;
    uint8_t cnt = 0;

    if (dt->year == 0xff)
    {
        val = 0xffff;
    }
    else
    {
        val = BASE_YEAR + (uint16_t)dt->year;
    }

    // DPRINTF (DBG_ERR, " 1 : clock_to_format_for_sign   year val(%d)\n", val);

    p_odata[cnt++] = (val >> 8) & 0xff;
    p_odata[cnt++] = val;
    p_odata[cnt++] = dt->month;
    p_odata[cnt++] = dt->date;
    p_odata[cnt++] = calc_dayofweek(dt);
    p_odata[cnt++] = dt->hour;
    p_odata[cnt++] = dt->min;
    p_odata[cnt++] = dt->sec;

    p_odata[cnt++] = 0xff;
    p_odata[cnt++] = 0xff;
    p_odata[cnt++] = 0xff;
    p_odata[cnt++] = 0xff;

    // DPRINT_HEX(

    return cnt;
}

void fill_clock_obj_except_tag(date_time_type* dt)
{
    uint16_t val;

    FILL_V8(0x0c);

    if (dt->year == 0xff)
    {
        val = 0xffff;
    }
    else
    {
        val = BASE_YEAR + (uint16_t)dt->year;
    }
    FILL_V16(val);

    FILL_V8(dt->month);
    FILL_V8(dt->date);
    FILL_V8(calc_dayofweek(dt));  // if(day == 0xff) -> return 0xff
    FILL_V8(dt->hour);
    FILL_V8(dt->min);
    FILL_V8(dt->sec);
    FILL_V8(0xff);  // hundredths
    FILL_V8(0xff);
    FILL_V8(0xff);
    FILL_V8(0xff);
}

static void fill_dst_clock_obj(date_time_type* dt, uint8_t wk)
{
    uint16_t val;

    FILL_STRING(0x0c);

    if (dt->year == 0xff)
    {
        val = 0xffff;
    }
    else
    {
        val = BASE_YEAR + (uint16_t)dt->year;
    }
    FILL_V16(val);

    FILL_V8(dt->month);
    FILL_V8(dt->date);
    FILL_V8(wk);
    FILL_V8(dt->hour);
    FILL_V8(dt->min);
    FILL_V8(dt->sec);
    FILL_V8(0xff);  // hundredths
    FILL_V8(0xff);
    FILL_V8(0xff);
    FILL_V8(0xff);
}

static void fill_unspecified_clock(void)
{
    memcpy(&pPdu[pPdu_idx], &unspecified_dtime[0], UNSP_TIME_SIZE);
    pPdu_idx += UNSP_TIME_SIZE;
}

// fill the period date of end of billing period
static void fill_EOBP_Period_date(uint8_t dt)
{
    memcpy(&pPdu[pPdu_idx], &EOBP_date[0], EOBP_DATE_LEN);
    pPdu_idx += EOBP_DATE_LEN;
    pPdu[pPdu_idx - 2] = dt;
}

static const /*__code*/ dsp_item_info_type* get_user_mode_disp_table(
    uint8_t* stidx, uint8_t* num)
{
    /* 납품 시 순환표시 모드 (순환 표시항목 리스트) */

    bool is5days; /*지금은 없어진 기능 임, 검침 발생 후 5일 동안은 표시 항목을
                     달리 했던 기능 임*/
    ratekind_type mtkind;
    const /*__code*/ dsp_item_info_type* info_table;

    mtkind = mt_rtkind;
    if (is_mrdt_within_5days() && circdsp_is_bill5days())
    {
        is5days = true;
    }
    else
    {
        is5days = false;
    }

    if (circdsp_is_pvt_mode())
    {
        // Private mode
        *stidx = 0;
        *num = CIRCDSP_PVT_MODE_NUM;
        info_table =
            (const /*__code*/ dsp_item_info_type*)circdsp_pvt_mode_table;
    }
    else if (circdsp_is_smode())
    {
        // 단순 표시 모드
        if (mt_is_uni_dir())
        {
            *stidx = 0;
            *num = CIRCDSP_SMODE_UNIDIR_NUM;
            info_table = (const /*__code*/ dsp_item_info_type*)
                circdsp_smode_unidir_table;
        }
        else
        {
            *stidx = 0;
            *num = CIRCDSP_SMODE_BOTHDIR_NUM;
            info_table = (const /*__code*/ dsp_item_info_type*)
                circdsp_smode_bothdir_table;
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
                    if (is5days)
                    {
                        *stidx = CIRCDSP_NSMODE_ONERATE_UNIDIR_STIDX5;
                        *num = CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM5;
                    }
                    else
                    {
                        *stidx = 0;
                        *num = CIRCDSP_NSMODE_ONERATE_UNIDIR_NUM;
                    }

                    info_table = (const dsp_item_info_type*)
                        circdsp_nsmode_onerate_unidir_table;
                }
                else
                {
                    if (is5days)
                    {
                        *stidx = CIRCDSP_NSMODE_ONERATE_BOTHDIR_STIDX5;
                        *num = CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM5;
                    }
                    else
                    {
                        *stidx = 0;
                        *num = CIRCDSP_NSMODE_ONERATE_BOTHDIR_NUM;
                    }

                    info_table = (const dsp_item_info_type*)
                        circdsp_nsmode_onerate_bothdir_table;
                }
            }
            else
            {
                if (mt_is_uni_dir())
                {
                    if (is5days)
                    {
                        *stidx = CIRCDSP_NSMODE_TWORATE_UNIDIR_STIDX5;
                        *num = CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM5;
                    }
                    else
                    {
                        *stidx = 0;
                        *num = CIRCDSP_NSMODE_TWORATE_UNIDIR_NUM;
                    }

                    info_table = (const dsp_item_info_type*)
                        circdsp_nsmode_tworate_unidir_table;
                }
                else
                {
                    if (is5days)
                    {
                        *stidx = CIRCDSP_NSMODE_TWORATE_BOTHDIR_STIDX5;
                        *num = CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM5;
                    }
                    else
                    {
                        *stidx = 0;
                        *num = CIRCDSP_NSMODE_TWORATE_BOTHDIR_NUM;
                    }

                    info_table = (const dsp_item_info_type*)
                        circdsp_nsmode_tworate_bothdir_table;
                }
            }
        }
        else
        {
            // 공급자 모드
            *num = 0;
            info_table = (const /*__code*/ dsp_item_info_type*)
                NULL;  // to avoid compile warning
        }
    }

    return info_table;
}

static int16_t get_clk_dev(void)
{
    int16_t t16;

    t16 = TIME_ZONE_TO_GMT;
    if (cap_clk_sts & CLKSTS_DAYLIGHT_SAVING)
    {
        t16 -= dls_info.dev;
    }

    return t16;
}

uint16_t get_mr_idx(uint8_t grp_f)
{
    uint8_t vz = 0;     // latest buffer index ( maybe pre month )
    uint8_t vz_id = 0;  // difference between grp_f and lastest buffer index
    uint16_t rtn = 0;   // buffer index for grp_f

    if (mr_cnt)
    {
        vz = mr_cnt % NUM_MREADING_SAVE;   // 1 ~ 6
        vz_id = (grp_f - VZ_LASTEST_VAL);  // 0 ~ 5

        if (vz > vz_id)
            rtn = vz - vz_id;
        else
            rtn = vz - vz_id + NUM_MREADING_SAVE;
    }
    DPRINTF(DBG_TRACE, "%s: GRP_F[%d], VZ[%d], buffer_idx[%d]\r\n", __func__,
            grp_f, vz, rtn);
    return rtn;
}

uint16_t get_mr_idx_nperiod(uint8_t grp_f)
{
    uint8_t vz = 0;     // latest buffer index ( maybe pre month )
    uint8_t vz_id = 0;  // difference between grp_f and lastest buffer index
    uint16_t rtn = 0;   // buffer index for grp_f

    if (mrcnt_nprd)
    {
        vz = mrcnt_nprd % NUM_MREADING_nPRD_SAVE;  // 1 ~ 4
        vz_id = (grp_f - VZ_LASTEST_VAL);          // 0 ~ 5

        if (vz > vz_id)
            rtn = vz - vz_id;
        else
            rtn = vz - vz_id + NUM_MREADING_nPRD_SAVE;
    }
    DPRINTF(DBG_TRACE, "%s: GRP_F[%d], VZ[%d], buffer_idx[%d]\r\n", __func__,
            grp_f, vz, rtn);
    return rtn;
}

uint16_t get_mr_idx_season(uint8_t grp_f)
{
    uint8_t vz = 0;     // latest buffer index ( maybe pre month )
    uint8_t vz_id = 0;  // difference between grp_f and lastest buffer index
    uint16_t rtn = 0;   // buffer index for grp_f

    if (mrcnt_season_chg)
    {
        vz = mrcnt_season_chg % NUM_MREADING_SEASON_SAVE;  // 1 ~ 4
        vz_id = (grp_f - VZ_LASTEST_VAL);                  // 0 ~ 5

        if (vz > vz_id)
            rtn = vz - vz_id;
        else
            rtn = vz - vz_id + NUM_MREADING_SEASON_SAVE;
    }
    DPRINTF(DBG_TRACE, "%s: GRP_F[%d], VZ[%d], buffer_idx[%d]\r\n", __func__,
            grp_f, vz, rtn);
    return rtn;
}

static bool mr_read_from_nv(mr_data_sel_type sel, demand_ch_type dmch, U16 cnt,
                            rate_type rt, U8* tptr)
{
    if (cnt == 0)
        return false;

    nv_sub_info.mr.mrcnt = cnt;  // g-type 과 다름 즉 mr_cnt 방식이 다름
    nv_sub_info.mr.rt = rt;

    nv_sub_info.mr.sel = sel;
    nv_sub_info.mr.chsel = dmch;  // eMrSublocks 인 경우 만 해당 됨
    return nv_read(I_MT_READ_DATA, tptr);
}

static bool mr_from_nv(mr_data_sel_type sel, U8 gc, U8 gf, rate_type rt,
                       U8* tptr)
{
    if (gf == 0xff)
    {
        nv_sub_info.cur.rt = rt;
        nv_sub_info.cur.sel = sel;
        nv_sub_info.cur.chsel =
            energy_group_to_dmch_type(gc);  // eMrSublocks 인 경우 만 해당 됨
        return nv_read(I_CUR_DATA_CAP, tptr);
    }

    return mr_read_from_nv(sel, energy_group_to_dmch_type(gc), get_mr_idx(gf),
                           rt, tptr);
}

static bool mr_nprd_read_from_nv(mr_data_sel_type sel, demand_ch_type dmch,
                                 U16 cnt, rate_type rt, U8* tptr)
{
    if (cnt == 0)
        return false;

    nv_sub_info.mr.mrcnt = cnt;  // g-type 과 다름 즉 mr_cnt 방식이 다름
    nv_sub_info.mr.rt = rt;

    nv_sub_info.mr.sel = sel;
    nv_sub_info.mr.chsel = dmch;  // eMrSublocks 인 경우 만 해당 됨
    return nv_read(I_MT_READ_DATA_nPRD, tptr);
}

static bool mr_nprd_from_nv(mr_data_sel_type sel, U8 gc, U8 gf, rate_type rt,
                            U8* tptr)
{
    if (gf == 0xff)
    {
        nv_sub_info.cur.rt = rt;
        nv_sub_info.cur.sel = sel;
        nv_sub_info.cur.chsel = energy_group_to_dmch_type(gc);
        return nv_read(I_CUR_DATA_CAP, tptr);
    }

    return mr_nprd_read_from_nv(sel, energy_group_to_dmch_type(gc),
                                get_mr_idx_nperiod(gf), rt, tptr);
}

static bool mr_season_read_from_nv(mr_data_sel_type sel, demand_ch_type dmch,
                                   U16 cnt, rate_type rt, U8* tptr)
{
    if (cnt == 0)
        return false;

    nv_sub_info.mr.mrcnt = cnt;
    nv_sub_info.mr.rt = rt;

    nv_sub_info.mr.sel = sel;
    nv_sub_info.mr.chsel = dmch;  // eMrSublocks 인 경우 만 해당 됨
    return nv_read(I_MT_READ_DATA_SEASON, tptr);
}

static bool mr_season_from_nv(mr_data_sel_type sel, U8 gc, U8 gf, rate_type rt,
                              U8* tptr)
{
    if (gf == 0xff)
    {
        nv_sub_info.cur.rt = rt;
        nv_sub_info.cur.sel = sel;
        nv_sub_info.cur.chsel =
            energy_group_to_dmch_type(gc);  // eMrSublocks 인 경우 만 해당 됨
        return nv_read(I_CUR_DATA_CAP, tptr);
    }

    return mr_season_read_from_nv(sel, energy_group_to_dmch_type(gc),
                                  get_mr_idx_season(gf), rt, tptr);
}

bool mr_prdnprdseason_from_nv(U8 gb, mr_data_sel_type sel, U8 gc, U8 gf,
                              rate_type rt, U8* tptr)
{
    if (gb == PERIOD_GRP_B)
    {
        return mr_from_nv(sel, gc, gf, rt, tptr);
    }
    else if (gb == nPERIOD_GRP_B)
    {
        return mr_nprd_from_nv(sel, gc, gf, rt, tptr);
    }
    else if (gb == SEASON_GRP_B)
    {
        return mr_season_from_nv(sel, gc, gf, rt, tptr);
    }
    return false;
}

static bool fut_pgm_read_set_bits(uint32_t chkbit, uint8_t* tptr)
{
    prog_dl_type* progdl;

    progdl = (prog_dl_type*)tptr;

    if (!prog_fut_is_available())
        return 0;

    if (!nv_read(I_FUT_PROG_DL, (uint8_t*)progdl))
        return 0;

    return ((progdl->set_bits & chkbit) != 0);
}

static bool fut_pgm_read_set_bits_1(uint32_t chkbit, uint8_t* tptr)
{
    prog_dl_type* progdl;

    progdl = (prog_dl_type*)tptr;

    if (!prog_fut_is_available())
        return 0;

    if (!nv_read(I_FUT_PROG_DL, (uint8_t*)progdl))
        return 0;

    return ((progdl->set_bits_1 & chkbit) != 0);
}

// --------------- each object response ----------------------
/*
    association 0 인 경우 현재에 association 된 정보를 리포트 한다..
    확인후 적용 필요..
*/
static void ob_ass_LN(void)
{
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s, obis_ge[%d]\r\n", __func__, obis_ge);

    switch (appl_att_id)
    {
    case 2:  // object_list
        approc_fill_get_resp_block(true);
        break;

    case 3:  // associated_partners_id
        FILL_STRUCT(0x02);
        FILL_S08(dl_client_macaddr());
        t16 = dl_server_macaddr();
        FILL_U16(t16);
        break;

    case 4:  // application_context_name
        FILL_STRING(7);
        pPdu_idx += appl_fill_context_name(&pPdu[pPdu_idx]);
        break;

    case 5:  // xDLMS_context_info
        FILL_STRUCT(0x06);
        FILL_BS(24);  // conformance
        if (appl_is_sap_assign_kepco_mnt())
        {
            FILL_V8(ASS3_LN_SUPPORTED_CONFORMANCE_1);
            FILL_V8(ASS3_LN_SUPPORTED_CONFORMANCE_2);
            FILL_V8(ASS3_LN_SUPPORTED_CONFORMANCE_3);
        }
        else
        {
            FILL_V8(LN_SUPPORTED_CONFORMANCE_1);
            FILL_V8(LN_SUPPORTED_CONFORMANCE_2);
            FILL_V8(LN_SUPPORTED_CONFORMANCE_3);
        }

        t16 = MAX_RXPDU_SIZE;  // ctt 3.1
        FILL_U16(t16);         // max_receive_pdu_size
        t16 = MAX_TXPDU_SIZE;  // ctt 3.1
        FILL_U16(t16);         // max_send_pdu_size
        FILL_U08(6);           // dlms version
        FILL_S08(0);           // quality of service ==> not used
        FILL_STRING(0);        // cyphering_info(dedicated key parameter of the
                               // DLMS-Initiate.request)
        break;

    case 6:  // authentication_mechanism_name
        FILL_STRING(MECHA_NAME_SIZE);
        if (appl_is_sap_public())
            pPdu_idx += appl_fill_mecha_name_noauth(&pPdu[pPdu_idx]);
        else
            pPdu_idx += appl_fill_mecha_name(&pPdu[pPdu_idx]);
        break;

    case 7:  // LLS_secret (no access in case of SAP_PUBLIC)
        if (appl_is_sap_utility())
        {
            FILL_STRING(appl_util_pwd.len);
            memcpy(&pPdu[pPdu_idx], appl_util_pwd.pwd, appl_util_pwd.len);
            pPdu_idx += appl_util_pwd.len;
        }
        else if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
        {
        }
        else if (appl_is_sap_485comm())
        {
            FILL_STRING(appl_485_pwd.len);
            memcpy(&pPdu[pPdu_idx], appl_485_pwd.pwd, appl_485_pwd.len);
            pPdu_idx += appl_485_pwd.len;
        }
        else
        {
            FILL_STRING(appl_priv_pwd.len);
            memcpy(&pPdu[pPdu_idx], appl_priv_pwd.pwd, appl_priv_pwd.len);
            pPdu_idx += appl_priv_pwd.len;
        }
        break;

    case 8:            // association_status
        FILL_ENUM(2);  // means associated status
        break;
    case 9:  // security_setup_reference

        break;
    case 10:  // user_list

        break;
    case 11:  // current_user

        break;
    }
}

static void ob_counter_billing(void)
{
    uint8_t t8;
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    if (mr_cnt)
        t8 = (uint8_t)(mr_cnt % MAX_BILLING_COUNTER);
    else
        t8 = 0;
    FILL_U08(t8);
}

static void ob_num_avail_billing(void)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    if (mr_cnt >= MAX_MREADING)
        t8 = MAX_MREADING;
    else
        t8 = (uint8_t)mr_cnt;

    FILL_U08(t8);
}

static void ob_custom_id(void)
{
    /*
    사용자 미터 ID (7byte) : 계기 번호의 7자리 번호룰 숫자 ASCII로 표기
    */
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    get_cust_id(appl_tbuff);
    fill_octet_string_x(appl_tbuff, SERIAL_NO_SIZE);
}

static void ob_manuf_id(void)
{
    /*
    계기 ID(11byte) : 계기 번호 11자리 번호를 ASCII로 모두 표기
        (1) 계기 ID는 제조사 번호, 계기 형식, 기기 번호의 11자리로 구성 (11byte)
        (2) 제조사 번호는 알파벳 (대소문자 구분 안 함)과 숫자를 혼용하여 사용
        (3) 예) 계기 ID “AM530123456” (AM : 제조사 번호, 53 : 계기 형식, 0123456
    : 일련번호)
    */
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    get_manuf_id(appl_tbuff);
    fill_octet_string_x(appl_tbuff, MANUF_ID_SIZE);
}

static void ob_manuf_id_old(void)
{
    /*
    구형 모뎀 연계용 미터 ID (7byte) : byte 4, 5번의 "00"을 ASCII로 표기, 0 0 0
    0 0 5 3
    */
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    memset(appl_tbuff, '0', MANUF_ID_SIZE);

    appl_tbuff[5] = (METER_ID / 10) + '0';
    appl_tbuff[6] = (METER_ID % 10) + '0';

    fill_octet_string_x(appl_tbuff, MANUF_ID_SIZE_OLD);
}
static void ob_prog_id(void)
{
    /*
    TOU 프로그램 ID:
        자리 : 1,    2~3,  4~5,         6~7,    8
        내용 : 종별, 검침일, 선택 유효전력량, 계량 모드, 업데이트 버전
        ※ 선택 유효전력량 표기 : 송‧수전 (SR), 수전 (RR)
        ※ 계량 모드 표기 : 수전, 송‧수전 (S), 수전 단방향 (D)
        ※ 업데이트 버전 표기 : 00 ∼ ZZ
        ※ 예시 : 프로그램 ID - 115SRS00 (1종, 검침일 : 15일, 송‧수전 모드)
        ※ 프로그램 변경 시 검침일을 00으로 보낼 경우, 기존에 계기에 설정되어
    있던 검침일로 설정되어야 한다. ※ 예시 : 기존 103RRD01에서 300SRS02로
    프로그램 입력 시, 프로그램 ID는 303SRS02로 설정 ※ 1~6번 자리는 전력량계의
    실제 값에 연동 되어 표시 한다.
    */
    uint8_t name[PROG_ID_SIZE];

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    fill_prog_name(true, name, appl_tbuff);
    fill_octet_string_x(name, PROG_ID_SIZE);
}

static void ob_mtconst_active(void)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = (float)PulseKwh;
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)-3, 45);  // unit: active meter constant
    }
}

static void ob_mtconst_reactive(void)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = (float)PulseKwh;
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)-3, 46);  // unit: reactive meter constant
    }
}

static void ob_mtconst_app(void)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = (float)PulseKwh;
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)-3, 47);  // unit: apparent meter constant
    }
}

static void ob_magnet_dur_time(void)
{
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        t32 = magnet_det_dur;
        FILL_U32(t32);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 7);  // unit: sec
    }
}

static void refill_selective_react_energy(uint8_t* obis)
{
    uint8_t selparm;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

#if 0  // JP.KIM 24.11.29
	if(obis[GROUP_F] == 0xff) {
		selparm = mt_selreact;			//  현재
	} else {
		if(mr_cnt)
			selparm = mr_selreact;			// 전월
		else
			selparm = mt_selreact;			//  현재
	}
#else
    selparm = mt_selreact;  //  현재전월 구분 안함.
#endif

    if ((obis[GROUP_A] == 0x01) && (obis[GROUP_B] == 0x00) &&
        (obis[GROUP_D] == 0x08)  // JP.KIM 24.11.29
        && ((obis[GROUP_C] == 0xfe || obis[GROUP_C] == 0xff)))
    {
        // selective reactive energy
        if (obis[GROUP_C] == 0xfe)
        {
            // 수전
            switch (selparm)
            {
            case 1:  // 지상
                obis[GROUP_C] = 5;
                break;
            case 2:  // 진상
                obis[GROUP_C] = 8;
                break;
            case 3:  // 지상 + 진상
                obis[GROUP_C] = 0x80;
                break;
            case 4:  // 피상
                obis[GROUP_C] = 9;
                break;
            }
        }
        else
        {
            // 송전
            switch (selparm)
            {
            case 1:  // 지상
                obis[GROUP_C] = 7;
                break;
            case 2:  // 진상
                obis[GROUP_C] = 6;
                break;
            case 3:  // 지상 + 진상
                obis[GROUP_C] = 0x81;
                break;
            case 4:  // 피상
                obis[GROUP_C] = 10;
                break;
            }
        }
    }
}

static void fill_dsp_item_struct(const dsp_item_info_type* dspitem,
                                 uint8_t stidx, uint8_t num)
{
    uint8_t i, k;
    uint16_t t16;
    uint8_t obis[OBIS_ID_SIZE];

    k = stidx;

    FILL_ARRAY(num);
    for (i = 0; i < num; i++)
    {
        FILL_STRUCT(4);

        t16 = dspitem[k].classid;
        FILL_U16(t16);
        memcpy(obis, &dspitem[k].obis[0], OBIS_ID_SIZE);
        fill_octet_string_x(obis, OBIS_ID_SIZE);
        refill_selective_react_energy(&pPdu[pPdu_idx - 6]);

        FILL_U08(dspitem[k].attid);
        FILL_U08(dspitem[k].dur);

        k += 1;
    }
}

static void ob_user_mode_disp(void)
{
    uint8_t stidx;
    uint8_t num;
    const dsp_item_info_type* dspitem;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    dspitem = get_user_mode_disp_table(&stidx, &num);
    if (num)
    {
        fill_dsp_item_struct(dspitem, stidx, num);
    }
    else
    {
        FILL_ARRAY(0);
    }
}

// 최대 546 바이트 크기
/*
       운영 측으로 부터 공급자 표시 모드가 설정 되면 normal 표시 항목 대신
       설정 된 표시 항목을 순환모드에서 표시 하게 됨
       표시 항목 개수만큼의 array로 구성 되어 있고
       set request 때 받은 데이터를 parsing 해서 표시에 적용도 하지만
       nv 영역에 저장해서 읽어 응답 데이터로 활용 함

*/
static void ob_supp_mode_disp(void)
{
    uint8_t i;
    uint16_t len;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (act_is_curr_prog_cmd())
    {
        if (lcd_is_user_dspmode())
            goto ob_supp_mode_disp2;
        // 공급자 표시 모드
        i = 0;
        goto ob_supp_mode_disp1;
    }
    else
    {
        if (prog_fut_is_available() && futprog_suppdisp_available())
        {
            i = 1;
        ob_supp_mode_disp1:
            if (!nv_read((nv_item_type)(I_SUPPDSP_ITEM_A + i), &pPdu[pPdu_idx]))
                goto ob_supp_mode_disp2;

            len = pPdu[pPdu_idx + 1];  // array tag parse
            if (len == 0)
                goto ob_supp_mode_disp2;

            len *= SUPPDSP_ITEM_SIZE;
            len += 2;  // array tag & size
            pPdu_idx += len;
        }
        else
        {
        ob_supp_mode_disp2:
            FILL_ARRAY(0);
        }
    }
}

static void ob_pvt_mode_disp(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    if (circdsp_is_pvt_mode())
    {
        FILL_BOOL(1);
    }
    else
    {
        FILL_BOOL(0);
    }
}

// fill the script of end of billing period
static void fill_EOBP(uint8_t dr)
{
    memcpy(&pPdu[pPdu_idx], &EOB_script[0], EOB_SCRIPT_LEN);
    pPdu_idx += EOB_SCRIPT_LEN;
    pPdu[pPdu_idx - 1] = conv_DRkind_to_sel(dr);
}

static void ob_period_billdate(void)
{
    prog_dl_type* pdl;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    switch (appl_att_id)
    {
    case 2:  // [executed_script]
        if (act_is_curr_prog_cmd())
        {
            fill_EOBP(pEOB_sr_dr_type);
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_PBILL_DRSEL, appl_tbuff))
            {
                pdl = (prog_dl_type*)appl_tbuff;
                fill_EOBP(pdl->pEOB.dr_sel);
            }
            else
            {
                fill_EOBP(pEOB_sr_dr_type);
            }
        }
        break;
    case 3:  // [type]
        FILL_ENUM(1);
        break;
    case 4:  // [execution_time]
        if (act_is_curr_prog_cmd())
        {
            fill_EOBP_Period_date(reg_mr_date);
        }
        else
        {
            if (prog_fut_is_available() && futprog_pbill_available())
            {
                pdl = (prog_dl_type*)appl_tbuff;
                if (prog_get_fut_dl(pdl))
                {
                    fill_EOBP_Period_date(pdl->regread_date);
                }
                else
                {
                    fill_EOBP_Period_date(reg_mr_date);
                }
            }
            else
            {
                fill_EOBP_Period_date(reg_mr_date);
            }
        }
        break;
    }
}

static void ob_nperiod_billdate(void)
{
    uint8_t i;
    uint16_t yr;
    int idx;
    prog_dl_type* pdl;
    npbill_date_type* npbill;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 2:  // [executed_script]
        if (act_is_curr_prog_cmd())
        {
            fill_EOBP(npEOB_sr_dr_type);
            DPRINTF(DBG_TRACE, "%s: cur_dr_sel[%d]\r\n", __func__,
                    npEOB_sr_dr_type);
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_NPBILL_DRSEL, appl_tbuff))
            {
                pdl = (prog_dl_type*)appl_tbuff;
                fill_EOBP(pdl->npEOB.dr_sel);
                DPRINTF(DBG_TRACE, "%s: fut_dr_sel[%d]\r\n", __func__,
                        pdl->npEOB.dr_sel);
            }
            else
            {
                fill_EOBP(npEOB_sr_dr_type);
                DPRINTF(DBG_TRACE, "%s: fut_empty  cur_dr_sel[%d]\r\n",
                        __func__, npEOB_sr_dr_type);
            }
        }
        break;
    case 3:  // [type]
        FILL_ENUM(5);
        break;
    case 4:              // [execution_time]
        idx = pPdu_idx;  // index for replace

        // fill unspecified date
        FILL_ARRAY(NP_BILLDATE_LEN);
        for (i = 0; i < NP_BILLDATE_LEN; i++)
        {
            memcpy(&pPdu[pPdu_idx], &npbill_date_r[0], NPBILL_DATE_LEN);
            pPdu_idx += NPBILL_DATE_LEN;
        }

        if (act_is_curr_prog_cmd())
        {
            if (prog_npbill_available())
            {
                if (nv_read(I_NP_BILLDATE_A, appl_tbuff))
                {
                ob_nperiod_billdate1:
                    npbill = (npbill_date_type*)appl_tbuff;

                    idx += 2;  // array tag
                    for (i = 0; i < npbill->cnt; i++)
                    {
                        idx += 2;  // struct tag
                        // time
                        pPdu[idx + 2] = npbill->npbill[i].dt.hour;
                        pPdu[idx + 3] = npbill->npbill[i].dt.min;
                        pPdu[idx + 4] = npbill->npbill[i].dt.sec;
                        idx += 6;
                        // date
                        if (npbill->npbill[i].dt.year != 0xff)
                        {
                            yr = npbill->npbill[i].dt.year + BASE_YEAR;
                            pPdu[idx + 2] = (uint8_t)((yr >> 8) & 0xff);
                            pPdu[idx + 3] = (uint8_t)(yr & 0xff);
                        }

                        pPdu[idx + 4] = npbill->npbill[i].dt.month;
                        pPdu[idx + 5] = npbill->npbill[i].dt.date;
                        pPdu[idx + 6] = npbill->npbill[i].day;
                        idx += 7;
                    }
                }
            }
        }
        else
        {
            if (prog_fut_is_available() && futprog_npbill_available())
            {
                if (nv_read(I_NP_BILLDATE_P, appl_tbuff))
                {
                    goto ob_nperiod_billdate1;
                }
            }
            else
            {
                if (prog_npbill_available())
                {
                    if (nv_read(I_NP_BILLDATE_A, appl_tbuff))
                    {
                        goto ob_nperiod_billdate1;
                    }
                }
            }
        }
        break;
    }
}

static void ob_sig_sel(void)
{
    uint8_t t8;
    prog_dl_type* pdl;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (act_is_curr_prog_cmd())
    {
        t8 = (uint8_t)sig_sel;
    }
    else
    {
        if (fut_pgm_read_set_bits(SETBITS_SIG_SEL, appl_tbuff))
        {
            pdl = (prog_dl_type*)appl_tbuff;
            t8 = pdl->out_sig;
        }
        else
        {
            t8 = (uint8_t)sig_sel;
        }
    }
    FILL_U08(t8);
}

// 2023.11.14 jp
static void ob_holiday_sel(void)
{
    bool t8;
    prog_dl_type* pdl;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (act_is_curr_prog_cmd())
    {
        t8 = holiday_sel1;
        DPRINTF(DBG_TRACE,
                _D "%s cur_pgm_read_set_bits_1 : holiday_sel1 = %d \r\n",
                __func__, t8);
    }
    else
    {
        if (fut_pgm_read_set_bits_1(SETBITS_HOLIDAY_SEL_1, appl_tbuff))
        {
            pdl = (prog_dl_type*)appl_tbuff;
            t8 = pdl->hol_sel1;

            DPRINTF(DBG_TRACE,
                    _D "%s fut_pgm_read_set_bits_1 : holiday_sel1 = %d \r\n",
                    __func__, t8);
        }
        else
        {
            t8 = holiday_sel1;
            DPRINTF(DBG_ERR,
                    _D
                    "%s fut_pgm_read_set_bits_1_EMPTY : holiday_sel1 = %d \r\n",
                    __func__, t8);
        }
    }
    FILL_BOOL(t8);  // unit = minutes
}

static void ob_lp_interval(void)
{
    uint8_t t8;
    prog_dl_type* pdl;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        if (act_is_curr_prog_cmd())
        {
            t8 = lp_interval;
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_LP_INTV, appl_tbuff))
            {
                pdl = (prog_dl_type*)appl_tbuff;
                t8 = pdl->lp_intv;
            }
            else
            {
                t8 = lp_interval;
            }
        }
        FILL_U08(t8);  // unit = minutes
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 6);  // scaler(0), unit(minute)
    }
}

static void ob_lpavg_interval(void)
{
    uint8_t t8;
    prog_dl_type* pdl;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        if (act_is_curr_prog_cmd())
        {
            goto ob_lpavg_interval1;
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_LPAVG_INTV, appl_tbuff))
            {
                pdl = (prog_dl_type*)appl_tbuff;
                t8 = pdl->lpavg_intv;
            }
            else
            {
            ob_lpavg_interval1:
                t8 = lpavg_interval;
            }
        }
        FILL_U08(t8);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 6);  // scaler(0), unit(minute)
    }
}

static void ob_time_billing(void)
{
    mr_data_info_type* mr;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (obis_gf == 0xff)
    {
        comm_dt = cur_rtc;
        fill_clock_obj(&comm_dt);
    }
    else
    {
        if (mr_prdnprdseason_from_nv(obis_gb, eMrInfo, obis_gc, obis_gf,
                                     (rate_type)0, appl_tbuff))
        {
            mr = (mr_data_info_type*)appl_tbuff;
            fill_clock_obj(&mr->dt);
        }
        else
        {
            fill_unspecified_clock();
        }
    }
}

uint8_t float_to_byte(float idata, uint8_t* odata)
{
    U8_16_32* pu8_16_32;
    uint8_t cnt = 0;

    pu8_16_32 = (U8_16_32*)&(idata);
    odata[cnt++] = pu8_16_32->c[HI_HI];
    odata[cnt++] = pu8_16_32->c[HI_LO];
    odata[cnt++] = pu8_16_32->c[LO_HI];
    odata[cnt++] = pu8_16_32->c[LO_LO];

    return cnt;
}

uint8_t u32_to_byte(uint32_t idata, uint8_t* odata)
{
    U8_16_32* pu8_16_32;
    uint8_t cnt = 0;

    pu8_16_32 = (U8_16_32*)&(idata);
    odata[cnt++] = pu8_16_32->c[HI_HI];
    odata[cnt++] = pu8_16_32->c[HI_LO];
    odata[cnt++] = pu8_16_32->c[LO_HI];
    odata[cnt++] = pu8_16_32->c[LO_LO];

    return cnt;
}

static void ob_month_energy(energy_dir_type dir)
{
    uint8_t i;
    uint8_t t8;
    uint32_t t32;
    mr_ch_type* mr;
    uint8_t data_s[256];
    uint16_t data_cnt = 0;

    DPRINTF(DBG_TRACE, _D "%s: obis_gb[%d]\r\n", __func__, obis_gb);

    switch (appl_att_id)
    {
    case 0x02: /* capture_objects */
        FILL_ARRAY(1);
        FILL_STRUCT(0x1C);

        ///////// 검침일자 //////////
        if (obis_gf == 0xff) /* 현재 */
        {
            comm_dt = cur_rtc;
            fill_clock_obj(&comm_dt);
            data_cnt += clock_to_format_for_sign(&comm_dt, &data_s[data_cnt]);
        }
        else
        {
            if (mr_prdnprdseason_from_nv(obis_gb, eMrInfo, obis_gc, obis_gf,
                                         (rate_type)0, appl_tbuff))
            {
                mr_data_info_type* mr_data_info;

                mr_data_info = (mr_data_info_type*)appl_tbuff;
                fill_clock_obj(&mr_data_info->dt);
                data_cnt += clock_to_format_for_sign(&mr_data_info->dt,
                                                     &data_s[data_cnt]);
            }
            else
            {
                fill_unspecified_clock();
                memset(&data_s[data_cnt], 0xff, 12);
                data_cnt += 12;
            }
        }

        ///////// 계기 ID //////////
        get_manuf_id(appl_tbuff);
        fill_octet_string_x(appl_tbuff, MANUF_ID_SIZE);
        memcpy(&data_s[data_cnt], appl_tbuff, MANUF_ID_SIZE);
        data_cnt += MANUF_ID_SIZE;

        ///////// 전력량 //////////
        i = eTrate;  // 전체 시간대
        do
        {
            if (!mr_prdnprdseason_from_nv(obis_gb, eMrAccm, obis_gc, obis_gf, i,
                                          appl_tbuff))
            {
                memset(appl_tbuff, 0, sizeof(mr_ch_type));
            }

            mr = (mr_ch_type*)appl_tbuff;

            if (dir == eDeliAct)
            {
                /* 수전 */
                FILL_U32(mr->ch[eChDeliAct]);     // 유효
                FILL_U32(mr->ch[eChDeliApp]);     // 피상
                FILL_U32(mr->ch[eChDLagReact]);   // 지상
                FILL_U32(mr->ch[eChDLeadReact]);  // 진상
                FILL_FLOAT(mr->pf[eDeliAct]);     // 수전 평균 역률
                data_cnt += u32_to_byte(mr->ch[eChDeliAct], &data_s[data_cnt]);
                data_cnt += u32_to_byte(mr->ch[eChDeliApp], &data_s[data_cnt]);
                data_cnt +=
                    u32_to_byte(mr->ch[eChDLagReact], &data_s[data_cnt]);
                data_cnt +=
                    u32_to_byte(mr->ch[eChDLeadReact], &data_s[data_cnt]);
                data_cnt += float_to_byte(mr->pf[eDeliAct], &data_s[data_cnt]);
            }
            else
            {
                /* 송전 */
                FILL_U32(mr->ch[eChReceiAct]);    // 유효
                FILL_U32(mr->ch[eChReceiApp]);    // 피상
                FILL_U32(mr->ch[eChRLeadReact]);  // 진상
                FILL_U32(mr->ch[eChRLagReact]);   // 지상
                FILL_FLOAT(mr->pf[eReceiAct]);    // 송전 평균 역률
                data_cnt += u32_to_byte(mr->ch[eChReceiAct], &data_s[data_cnt]);
                data_cnt += u32_to_byte(mr->ch[eChReceiApp], &data_s[data_cnt]);
                data_cnt +=
                    u32_to_byte(mr->ch[eChRLeadReact], &data_s[data_cnt]);
                data_cnt +=
                    u32_to_byte(mr->ch[eChRLagReact], &data_s[data_cnt]);
                data_cnt += float_to_byte(mr->pf[eReceiAct], &data_s[data_cnt]);
                // DPRINTF (DBG_ERR, " 1 : sign data_cnt (size=%d)\n",
                // data_cnt);
            }
            if (i == eDrate)
                break;  // exit
            if (++i >= numRates)
                i = 0;
        } while (1);

#if 0 /* bccho, 2024-09-05, 삼상 */
        if (obis_gf == 0xff)  // 현재
        {
            if (dsm_sec_signing_for_month_profile(appl_tbuff, data_s, data_cnt))
            {
                fill_octet_string_x(appl_tbuff, DLMS_DS_LEN);
            }
            else
            {
            }
        }
        else
        {
            if (!mr_prdnprdseason_from_nv(obis_gb, eMrAccmEcdsa, obis_gc,
                                          obis_gf, i, dir, appl_tbuff))
            {
                memset(appl_tbuff, 0, sizeof(mr_ecdsa_type));
            }
            fill_octet_string_x(appl_tbuff, DLMS_DS_LEN);
        }
#else
        if (dsm_sec_signing_for_month_profile(appl_tbuff, data_s, data_cnt))
        {
            fill_octet_string_x(appl_tbuff, DLMS_DS_LEN);
        }
        else
        {
            //
        }
#endif
        break;

    case 0x03:
        if (dir == eDeliAct)
        {
            if (obis_gb == PERIOD_GRP_B)
                memcpy(&pPdu[pPdu_idx], &deliENERGY_capture_objects[0],
                       ENERGY_CAPOBJ_SIZE);
            else if (obis_gb == nPERIOD_GRP_B)
                memcpy(&pPdu[pPdu_idx], &deliENERGY_nprd_capture_objects[0],
                       ENERGY_CAPOBJ_SIZE);
            else if (obis_gb == SEASON_GRP_B)
                memcpy(&pPdu[pPdu_idx], &deliENERGY_season_capture_objects[0],
                       ENERGY_CAPOBJ_SIZE);
        }
        else
        {
            if (obis_gb == PERIOD_GRP_B)
                memcpy(&pPdu[pPdu_idx], &receiENERGY_capture_objects[0],
                       ENERGY_CAPOBJ_SIZE);
            else if (obis_gb == nPERIOD_GRP_B)
                memcpy(&pPdu[pPdu_idx], &receiENERGY_nprd_capture_objects[0],
                       ENERGY_CAPOBJ_SIZE);
            else if (obis_gb == SEASON_GRP_B)
                memcpy(&pPdu[pPdu_idx], &receiENERGY_season_capture_objects[0],
                       ENERGY_CAPOBJ_SIZE);
        }
        pPdu_idx += ENERGY_CAPOBJ_SIZE;
        break;

    case 0x04:
        t32 = 0;
        FILL_U32(t32);  // 0 means No automatic capture
        break;

    case 0x05:
        t8 = 1;
        FILL_ENUM(t8);  // sort method(FIFO)
        break;

    case 0x06:
        memcpy(&pPdu[pPdu_idx], &LP_sort_object[0], LP_SORTOBJ_SIZE);

        pPdu_idx += LP_SORTOBJ_SIZE;
        break;

    case 0x07:
        if (mr_cnt)
        {
            t32 = 1L;
        }
        else
        {
            t32 = 0;
        }
        FILL_U32(t32);
        break;

    case 0x08:
        t32 = 1L;
        FILL_U32(t32);
        break;
    }
}

static void ob_month_maxdm(energy_dir_type dir)
{
    uint8_t i;
    uint8_t t8;
    uint32_t t32;
    mr_dm_type* mr;

    uint8_t data_s[256];
    uint16_t data_cnt = 0;
    DPRINTF(DBG_TRACE, _D "%s: obis_gb[%d]\r\n", __func__, obis_gb);

    switch (appl_att_id)
    {
    case 0x02:
        FILL_ARRAY(1);

        FILL_STRUCT(0x21);  // 2 + 5*6
        if (obis_gf == 0xff)
        {
            comm_dt = cur_rtc;
            fill_clock_obj(&comm_dt);
            data_cnt += clock_to_format_for_sign(&comm_dt, &data_s[data_cnt]);
        }
        else
        {
            if (mr_prdnprdseason_from_nv(obis_gb, eMrInfo, obis_gc, obis_gf,
                                         (rate_type)0, appl_tbuff))
            {  // 0 is no-meaning
                mr_data_info_type* mr_data_info;

                mr_data_info = (mr_data_info_type*)appl_tbuff;
                fill_clock_obj(&mr_data_info->dt);
                data_cnt += clock_to_format_for_sign(&mr_data_info->dt,
                                                     &data_s[data_cnt]);
            }
            else
            {
                fill_unspecified_clock();
                memset(&data_s[data_cnt], 0xff, 12);
                data_cnt += 12;
            }
        }
        get_manuf_id(appl_tbuff);
        fill_octet_string_x(appl_tbuff, MANUF_ID_SIZE);
        memcpy(&data_s[data_cnt], appl_tbuff, MANUF_ID_SIZE);
        data_cnt += MANUF_ID_SIZE;
        i = eTrate;
        do
        {
            if (!mr_prdnprdseason_from_nv(obis_gb, eMrDm, obis_gc, obis_gf, i,
                                          appl_tbuff))
            {
                memset(appl_tbuff, 0, sizeof(mr_dm_type));
            }

            mr = (mr_dm_type*)appl_tbuff;

            if (dir == eDeliAct)
            {
                FILL_U32(mr->mxdm[eDmChDeliAct].val);
                if (mr->mxdm[eDmChDeliAct].val)
                    fill_clock_obj(&mr->mxdm[eDmChDeliAct].dt);
                else
                    fill_unspecified_clock();
                FILL_U32(mr->cum_mxdm[eDmChDeliAct]);
                FILL_U32(mr->mxdm[eDmChDeliApp].val);
                if (mr->mxdm[eDmChDeliApp].val)
                    fill_clock_obj(&mr->mxdm[eDmChDeliApp].dt);
                else
                    fill_unspecified_clock();
                FILL_U32(mr->cum_mxdm[eDmChDeliApp]);
                data_cnt +=
                    u32_to_byte(mr->mxdm[eDmChDeliAct].val, &data_s[data_cnt]);
                if (mr->mxdm[eDmChDeliAct].val)
                {
                    data_cnt += clock_to_format_for_sign(
                        &mr->mxdm[eDmChDeliAct].dt, &data_s[data_cnt]);
                }
                else
                {
                    memset(&data_s[data_cnt], 0xff, 12);
                    data_cnt += 12;
                }
                data_cnt +=
                    u32_to_byte(mr->cum_mxdm[eDmChDeliAct], &data_s[data_cnt]);
                data_cnt +=
                    u32_to_byte(mr->mxdm[eDmChDeliApp].val, &data_s[data_cnt]);
                if (mr->mxdm[eDmChDeliApp].val)
                {
                    data_cnt += clock_to_format_for_sign(
                        &mr->mxdm[eDmChDeliApp].dt, &data_s[data_cnt]);
                }
                else
                {
                    memset(&data_s[data_cnt], 0xff, 12);
                    data_cnt += 12;
                }
                data_cnt +=
                    u32_to_byte(mr->cum_mxdm[eDmChDeliApp], &data_s[data_cnt]);
            }
            else
            {
                // delivered active
                FILL_U32(mr->mxdm[eDmChReceiAct].val);
                if (mr->mxdm[eDmChReceiAct].val)
                    fill_clock_obj(&mr->mxdm[eDmChReceiAct].dt);
                else
                    fill_unspecified_clock();
                FILL_U32(mr->cum_mxdm[eDmChReceiAct]);
                // delivered apparent
                FILL_U32(mr->mxdm[eDmChReceiApp].val);
                if (mr->mxdm[eDmChReceiApp].val)
                    fill_clock_obj(&mr->mxdm[eDmChReceiApp].dt);
                else
                    fill_unspecified_clock();
                FILL_U32(mr->cum_mxdm[eDmChReceiApp]);
                data_cnt +=
                    u32_to_byte(mr->mxdm[eDmChReceiAct].val, &data_s[data_cnt]);
                if (mr->mxdm[eDmChReceiAct].val)
                {
                    data_cnt += clock_to_format_for_sign(
                        &mr->mxdm[eDmChReceiAct].dt, &data_s[data_cnt]);
                }
                else
                {
                    memset(&data_s[data_cnt], 0xff, 12);
                    data_cnt += 12;
                }
                data_cnt +=
                    u32_to_byte(mr->cum_mxdm[eDmChReceiAct], &data_s[data_cnt]);
                data_cnt +=
                    u32_to_byte(mr->mxdm[eDmChReceiApp].val, &data_s[data_cnt]);
                if (mr->mxdm[eDmChReceiApp].val)
                {
                    data_cnt += clock_to_format_for_sign(
                        &mr->mxdm[eDmChReceiApp].dt, &data_s[data_cnt]);
                }
                else
                {
                    memset(&data_s[data_cnt], 0xff, 12);
                    data_cnt += 12;
                }
                data_cnt +=
                    u32_to_byte(mr->cum_mxdm[eDmChReceiApp], &data_s[data_cnt]);
            }

            if (i == eDrate)
            {
                break;
            }

            if (++i >= numRates)
            {
                i = 0;
            }
        } while (1);

#if 0 /* bccho, 2024-09-05, 삼상 */
        if (obis_gf == 0xff)
        {
            if (dsm_sec_signing_for_month_profile(appl_tbuff, data_s, data_cnt))
            {
                fill_octet_string_x(appl_tbuff, DLMS_DS_LEN);
            }
            else
            {
            }
        }
        else
        {
            if (!mr_prdnprdseason_from_nv(obis_gb, eMrDmEcdsa, obis_gc, obis_gf,
                                          i, dir, appl_tbuff))
            {
                memset(appl_tbuff, 0, sizeof(mr_ecdsa_type));
            }

            fill_octet_string_x(appl_tbuff, DLMS_DS_LEN);
        }
#else
        if (dsm_sec_signing_for_month_profile(appl_tbuff, data_s, data_cnt))
        {
            fill_octet_string_x(appl_tbuff, DLMS_DS_LEN);
        }
        else
        {
            //
        }
#endif
        break;

    case 0x03:
        if (dir == eDeliAct)
        {
            if (obis_gb == PERIOD_GRP_B)
                memcpy(&pPdu[pPdu_idx], &deliMAXDM_capture_objects[0],
                       MAXDM_CAPOBJ_SIZE);
            else if (obis_gb == nPERIOD_GRP_B)
                memcpy(&pPdu[pPdu_idx], &deliMAXDM_nprd_capture_objects[0],
                       MAXDM_CAPOBJ_SIZE);
            else if (obis_gb == SEASON_GRP_B)
                memcpy(&pPdu[pPdu_idx], &deliMAXDM_season_capture_objects[0],
                       MAXDM_CAPOBJ_SIZE);

            pPdu_idx += MAXDM_CAPOBJ_SIZE;
        }
        else
        {
            if (obis_gb == PERIOD_GRP_B)
                memcpy(&pPdu[pPdu_idx], &receiMAXDM_capture_objects[0],
                       MAXDM_CAPOBJ_SIZE);
            else if (obis_gb == nPERIOD_GRP_B)
                memcpy(&pPdu[pPdu_idx], &receiMAXDM_nprd_capture_objects[0],
                       MAXDM_CAPOBJ_SIZE);
            else if (obis_gb == SEASON_GRP_B)
                memcpy(&pPdu[pPdu_idx], &receiMAXDM_season_capture_objects[0],
                       MAXDM_CAPOBJ_SIZE);

            pPdu_idx += MAXDM_CAPOBJ_SIZE;
        }
        break;

    case 0x04:
        t32 = 0;
        FILL_U32(t32);  // 0 means No automatic capture
        break;

    case 0x05:
        t8 = 1;
        FILL_ENUM(t8);  // sort method(FIFO)
        break;

    case 0x06:
        memcpy(&pPdu[pPdu_idx], &LP_sort_object[0], LP_SORTOBJ_SIZE);
        pPdu_idx += LP_SORTOBJ_SIZE;
        break;

    case 0x07:
        if (mr_cnt)
        {
            t32 = 1L;
        }
        else
        {
            t32 = 0;
        }
        FILL_U32(t32);
        break;

    case 0x08:
        t32 = 1L;
        FILL_U32(t32);
        break;
    }
}

static void ob_month_sublocks(void)
{
    uint8_t i;
    mr_dm_type* mr;
    rolling_dm_ch_type* rolldm;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (mr_from_nv(eMrDm, obis_gc, obis_gf, energy_group_to_tariff(obis_ge),
                   appl_tbuff))
    {
        mr = (mr_dm_type*)appl_tbuff;
        if (mr->mxdm[energy_group_to_dmch_type(obis_gc)].val)
        {
            if (mr_from_nv(eMrSublocks, obis_gc, obis_gf,
                           energy_group_to_tariff(obis_ge), appl_tbuff))
            {
                rolldm = (rolling_dm_ch_type*)appl_tbuff;
                FILL_ARRAY(rolldm->cnt);
                for (i = 0; i < rolldm->cnt; i++)
                {
                    FILL_U16(rolldm->sublock[i]);
                }

                return;
            }
        }
    }

    FILL_ARRAY(0);
}

bool Is_push_PWR_OFF_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_1_IDX] & PUSHACTI_CODE1_PWR_OFF)
        return (true);
    else
        return (false);
}

void PWR_OFF_trigger(void)
{
    DPRINTF(DBG_ERR, _D "%s: POWER_FAIL_STS_mask[%d]\r\n", __func__,
            Is_push_PWR_OFF_act_mask());

    WMStatus |= POWER_FAIL_STS;

    // if(!(WMStatus & POWER_FAIL_STS))
    {
        if (Is_push_PWR_OFF_act_mask())
        {
            MSG07("Is_push_PWR_OFF_act_mask--TRUE");
            // dsm_data_noti_errcode_evt_send();
            dsm_push_data_noti_proc(PUSH_SCRIPT_ID_ERR_CODE);
            // appl_push_msg_errcode();

            amr_send_frame(TRUE);
        }
    }
    // WMStatus |= POWER_FAIL_STS;
}

bool Is_push_Work_BlackOut_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_1_IDX] & PUSHACTI_CODE1_WORK_BLACKOUT)
        return (true);
    else
        return (false);
}

void Work_BlackOut_trigger(void)
{
    DPRINTF(DBG_ERR, _D "%s: WORK_BLACKOUT_mask[%d]\r\n", __func__,
            Is_push_Work_BlackOut_act_mask());

    WMStatus |= WORK_BLACKOUT;
    if (Is_push_Work_BlackOut_act_mask())
    {
        dsm_push_data_noti_proc(PUSH_SCRIPT_ID_ERR_CODE);
        amr_send_frame(TRUE);
    }
}

bool Is_push_ENERGY_REMAINING_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_1_IDX] & PUSHACTI_CODE1_ENERGY_REMAINING)
        return (true);
    else
        return (false);
}

void ENERGY_REMAINING_trigger(void)
{
    if (!(WMStatus & PRE_PAID_OUT))
    {
        if (Is_push_ENERGY_REMAINING_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= PRE_PAID_OUT;
}

bool Is_push_SECURITY_ERR_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_1_IDX] & PUSHACTI_CODE1_SECURITY_ERR)
        return (true);
    else
        return (false);
}

void SECURITY_ERR_trigger(void)
{
    if (!(WMStatus & SECURITY_ERR))
    {
        if (Is_push_SECURITY_ERR_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= SECURITY_ERR;
}

bool Is_push_MAGNET_SENS_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_1_IDX] & PUSHACTI_CODE1_MAGNET_SENS)
        return (true);
    else
        return (false);
}

void MAGNET_SENS_trigger(void)
{
    if (!(tamper_det_bit & MAGNET_DETED_BIT))
    {
        if (Is_push_MAGNET_SENS_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    tamper_det_bit |= MAGNET_DETED_BIT;
}

bool Is_push_COVER_OPEN_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_1_IDX] & PUSHACTI_CODE1_COVER_OPEN)
        return (true);
    else
        return (false);
}

void tCOVER_OPEN_trigger(void)
{
    if (!(tamper_det_bit & TCOVEROPEN_DETED_BIT))
    {
        if (Is_push_COVER_OPEN_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    tamper_det_bit |= TCOVEROPEN_DETED_BIT;
}

void mCOVER_OPEN_trigger(void)
{
    if (!(tamper_det_bit & MCOVEROPEN_DETED_BIT))
    {
        if (Is_push_COVER_OPEN_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    tamper_det_bit |= MCOVEROPEN_DETED_BIT;
}

bool Is_push_NO_BATTERY_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_1_IDX] & PUSHACTI_CODE1_NO_BATTERY)
        return (true);
    else
        return (false);
}

void NO_BATTERY_trigger(void)
{
    if (!(WMStatus & GE_NOBAT))
    {
        if (Is_push_NO_BATTERY_act_mask())
        {
            dsm_data_noti_errcode_evt_send();
        }
    }
    WMStatus |= GE_NOBAT;
}

#if 1 /* JPKIM, 2024-10-04 */
bool Is_push_OVERCURR_act_mask(int i)
{
    if (gst_push_acti.code[ERR_CODE_2_IDX] & (PUSHACTI_CODE2_OVERCURR_A << i))
        return (true);
    else
        return (false);
}

int OVERCURR_trigger(int i)
{
    if (!(WMStatus & STOVERIA << i))
    {
        if (Is_push_OVERCURR_act_mask(i))
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= (STOVERIA << i);
    return (i);
}
#else
bool Is_push_OVERCURR_a_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_2_IDX] & PUSHACTI_CODE2_OVERCURR_A)
        return (true);
    else
        return (false);
}
bool Is_push_OVERCURR_b_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_2_IDX] & PUSHACTI_CODE2_OVERCURR_B)
        return (true);
    else
        return (false);
}
bool Is_push_OVERCURR_c_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_2_IDX] & PUSHACTI_CODE2_OVERCURR_C)
        return (true);
    else
        return (false);
}

int OVERCURR_a_trigger(int i)
{
    if (!(WMStatus & STOVERIA << i))
    {
        if (Is_push_OVERCURR_a_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= (STOVERIA << i);
    return (i);
}
#endif

bool Is_push_WRONGCONN_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_2_IDX] & PUSHACTI_CODE2_WRONGCONN)
        return (true);
    else
        return (false);
}

void WRONGCONN_trigger(void)
{
    if (!(WMStatus & WRONG_CONN))
    {
        if (Is_push_WRONGCONN_act_mask())
        {
            dsm_data_noti_errcode_evt_send();
        }
    }
    WMStatus |= WRONG_CONN;
}

bool Is_push_temp_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_2_IDX] & PUSHACTI_CODE2_TEMP)
        return (true);
    else
        return (false);
}

void TEMPOVER_trigger(void)
{
    if (!(WMStatus & TEMPOVER))
    {
        if (Is_push_temp_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= TEMPOVER;
}

bool Is_push_LATCH_ERR_A_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_3_IDX] & PUSHACTI_CODE3_LATCH_ERR_A)
        return (true);
    else
        return (false);
}

bool Is_push_LATCH_ERR_B_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_3_IDX] & PUSHACTI_CODE3_LATCH_ERR_B)
        return (true);
    else
        return (false);
}

bool Is_push_LATCH_ERR_C_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_3_IDX] & PUSHACTI_CODE3_LATCH_ERR_C)
        return (true);
    else
        return (false);
}

void LATCH_ERR_A_trigger(void)
{
    if (!(WMStatus & RELAY_ERRA))
    {
        if (Is_push_LATCH_ERR_A_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= RELAY_ERRA;
}

void LATCH_ERR_B_trigger(void)
{
    if (!(WMStatus & RELAY_ERRB))
    {
        if (Is_push_LATCH_ERR_B_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= RELAY_ERRB;
}

void LATCH_ERR_C_trigger(void)
{
    if (!(WMStatus & RELAY_ERRC))
    {
        if (Is_push_LATCH_ERR_C_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= RELAY_ERRC;
}

bool Is_push_SELF_ERR_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_3_IDX] & PUSHACTI_CODE3_SELF_ERR)
        return (true);
    else
        return (false);
}
void SELF_ERR_trigger(void)
{
    if (!(WMStatus & SELF_PLS_ERR))
    {
        if (Is_push_SELF_ERR_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= SELF_PLS_ERR;
}

bool Is_push_NEUT_WRONGCONN_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_3_IDX] & PUSHACTI_CODE3_NEUT_WRONGCONN)
        return (true);
    else
        return (false);
}

void NEUT_WRONGCONN_trigger(void)
{
    if (!(WMStatus & WRONG_NEUT))
    {
        if (Is_push_NEUT_WRONGCONN_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= WRONG_NEUT;
}

#if 1
bool Is_push_NO_PHASE_act_mask(int i)
{
    if (gst_push_acti.code[ERR_CODE_3_IDX] & (PUSHACTI_CODE3_NO_PHASE_A << i))
        return (true);
    else
        return (false);
}

void NO_PHASE_set_trigger(int i)
{
    if (!(WMStatus & (SAGVA << i)))
    {
        if (Is_push_NO_PHASE_act_mask(i))
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= (SAGVA << i);
}

void NO_PHASE_rcv_trigger(int i)
{
    if ((WMStatus & (SAGVA << i)))
    {
        if (Is_push_NO_PHASE_act_mask(i))
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus &= ~(SAGVA << i);
}
#else
bool Is_push_NO_PHASE_A_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_3_IDX] & PUSHACTI_CODE3_NO_PHASE_A)
        return (true);
    else
        return (false);
}

bool Is_push_NO_PHASE_B_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_3_IDX] & PUSHACTI_CODE3_NO_PHASE_B)
        return (true);
    else
        return (false);
}

bool Is_push_NO_PHASE_C_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_3_IDX] & PUSHACTI_CODE3_NO_PHASE_C)
        return (true);
    else
        return (false);
}

void NO_PHASE_A_trigger(void)
{
    if (!(WMStatus & SAGVA))
    {
        if (Is_push_NO_PHASE_A_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= SAGVA;
}

void NO_PHASE_B_trigger(void)
{
    if (!(WMStatus & SAGVB))
    {
        if (Is_push_NO_PHASE_B_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= SAGVB;
}

void NO_PHASE_C_trigger(void)
{
    if (!(WMStatus & SAGVC))
    {
        if (Is_push_NO_PHASE_C_act_mask())
            dsm_data_noti_errcode_evt_send();
    }
    WMStatus |= SAGVC;
}
#endif

void all_error_sts_chk_trigger(uint8_t errcodeidx)
{
    DPRINTF(DBG_TRACE,
            _D "%s WMStatus[%X]  code[errcodeidx][%X] errcodeid[%X]\r\n",
            __func__, WMStatus, gst_push_acti.code[errcodeidx], errcodeidx);

    if (ERR_CODE_MAX_IDX <= errcodeidx)
    {
        if (Is_push_SAG_act_mask())
            WMStatus &= ~(SAG_STS);
        if (Is_push_SWELL_act_mask())
            WMStatus &= ~(SWELL_STS);

        if ((((WMStatus >> (8 * 0)) & 0xff) &
             gst_push_acti.code[ERR_CODE_1_IDX]) ||
            (((WMStatus >> (8 * 1)) & 0xff) &
             gst_push_acti.code[ERR_CODE_2_IDX]) ||
            (((WMStatus >> (8 * 2)) & 0xff) &
             gst_push_acti.code[ERR_CODE_3_IDX]) ||
            (((WMStatus >> (8 * 3)) & 0xff) &
             gst_push_acti.code[ERR_CODE_4_IDX]))
        {
            DPRINTF(DBG_TRACE,
                    _D
                    "%s if(((WMStatus & (0xff << (8*0)))  ---> ok   "
                    "dsm_data_noti_errcode_evt_send(); \r\n",
                    __func__);
            dsm_data_noti_errcode_evt_send();
        }
    }
    else
    {
        if (ERR_CODE_4_IDX == errcodeidx)
        {
            if (Is_push_SAG_act_mask())
                WMStatus &= ~(SAG_STS);
            if (Is_push_SWELL_act_mask())
                WMStatus &= ~(SWELL_STS);
        }

        if (((WMStatus >> (8 * errcodeidx)) & 0xff) &
            gst_push_acti.code[errcodeidx])
        {
            DPRINTF(DBG_TRACE,
                    _D
                    "%s "
                    "WMStatus[errcodeidx][%X]dsm_data_noti_errcode_evt_"
                    "send(); "
                    "\r\n",
                    __func__, ((WMStatus >> (8 * errcodeidx)) & 0xff));
            dsm_data_noti_errcode_evt_send();
        }
    }
}

bool Is_push_OVERVOT_A_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_4_IDX] & PUSHACTI_CODE4_OVERVOT_A)
        return (true);
    else
        return (false);
}

bool Is_push_OVERVOT_B_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_4_IDX] & PUSHACTI_CODE4_OVERVOT_B)
        return (true);
    else
        return (false);
}

bool Is_push_OVERVOT_C_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_4_IDX] & PUSHACTI_CODE4_OVERVOT_C)
        return (true);
    else
        return (false);
}

void OVERVOT_trigger(uint8_t line)
{
    switch (line)
    {
    case 1:
        if (!(WMStatus & LB_V_HIGH))
        {
            if (Is_push_OVERVOT_B_act_mask())
                dsm_data_noti_errcode_evt_send();
        }
        WMStatus |= LB_V_HIGH;
        break;

    case 2:
        if (!(WMStatus & LC_V_HIGH))
        {
            if (Is_push_OVERVOT_C_act_mask())
                dsm_data_noti_errcode_evt_send();
        }
        WMStatus |= LC_V_HIGH;

        break;

    default:
        if (!(WMStatus & LA_V_HIGH))
        {
            if (Is_push_OVERVOT_A_act_mask())
                dsm_data_noti_errcode_evt_send();
        }
        WMStatus |= LA_V_HIGH;

        break;
    }
}

bool Is_push_LOWVOT_A_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_4_IDX] & PUSHACTI_CODE4_LOWVOT_A)
        return (true);
    else
        return (false);
}

bool Is_push_LOWVOT_B_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_4_IDX] & PUSHACTI_CODE4_LOWVOT_B)
        return (true);
    else
        return (false);
}

bool Is_push_LOWVOT_C_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_4_IDX] & PUSHACTI_CODE4_LOWVOT_C)
        return (true);
    else
        return (false);
}

void LOWVOT_trigger(uint8_t line)
{
    switch (line)
    {
    case 1:

        if (!(WMStatus & LB_V_LOW))
        {
            if (Is_push_LOWVOT_B_act_mask())
                dsm_data_noti_errcode_evt_send();
        }
        WMStatus |= LB_V_LOW;

        break;

    case 2:

        if (!(WMStatus & LC_V_LOW))
        {
            if (Is_push_LOWVOT_C_act_mask())
                dsm_data_noti_errcode_evt_send();
        }
        WMStatus |= LC_V_LOW;
        break;

    default:

        if (!(WMStatus & LA_V_LOW))
        {
            if (Is_push_LOWVOT_B_act_mask())
                dsm_data_noti_errcode_evt_send();
        }
        WMStatus |= LA_V_LOW;

        break;
    }
}

bool Is_push_SAG_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_4_IDX] & PUSHACTI_CODE4_SAG)
        return (true);
    else
        return (false);
}

void SAG_trigger(void)
{
    if (!(WMStatus & SAG_STS))
    {
#if 0 /* bccho, 2024-06-03 */
		{ if(Is_push_SAG_act_mask())	dsm_data_noti_errcode_evt_send();}
		WMStatus |= SAG_STS;
#else
        WMStatus |= SAG_STS;
        if (Is_push_SAG_act_mask())
        {
            dsm_push_data_noti_proc(PUSH_SCRIPT_ID_ERR_CODE);
            amr_send_frame(TRUE);
        }
#endif
    }
}

bool Is_push_SWELL_act_mask(void)
{
    if (gst_push_acti.code[ERR_CODE_4_IDX] & PUSHACTI_CODE4_SWELL)
        return (true);
    else
        return (false);
}

void SWELL_trigger(void)
{
    if (!(WMStatus & SWELL_STS))
    {
#if 0 /* bccho, 2024-06-03 */
		{if(Is_push_SWELL_act_mask()) dsm_data_noti_errcode_evt_send();}
		WMStatus |= SWELL_STS;
#else
        WMStatus |= SWELL_STS;
        if (Is_push_SWELL_act_mask())
        {
            dsm_push_data_noti_proc(PUSH_SCRIPT_ID_ERR_CODE);
            amr_send_frame(TRUE);
        }
#endif
    }
}

void error_code_event_clear(void)
{
    WMStatus &= ~POWER_FAIL_STS;
#if 0 /* bccho, 2024-06-03 */    
    WMStatus &= ~SWELL_STS;
    WMStatus &= ~SAG_STS;
#endif
    WMStatus &= ~WORK_BLACKOUT;
}

void error_code_event_clear_for_prepay(void) { WMStatus &= ~PRE_PAID_OUT; }

bool is_WMStatus_prepay_remaining_0(void)
{
    if (WMStatus & PRE_PAID_OUT)
        return true;
    return false;
}

void ob_err_code_1(void)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    t8 = 0x00;
    /*
    (B7) 정전
    (B6) 잔여전력량'0'
    (B4) 보안이상
    (B3) 자계감지
    (B2) Cover Open
    (B0) 배터리 없음
     차후 코딩 필요...
    */
    if (WMStatus & GE_NOBAT)
        t8 |= 0x01;
    if (WMStatus & WORK_BLACKOUT)
    {
        t8 |= 0x02;
    }
    if (IS_MorTCOVER_OPEN)
        t8 |= 0x04;
    if (IS_MAGNET_DET)
        t8 |= 0x08;
    if (WMStatus & SECURITY_ERR)
        t8 |= 0x10;
    if (WMStatus & PRE_PAID_OUT)
        t8 |= 0x40;
    if (WMStatus & POWER_FAIL_STS)
        t8 |= 0x80;
    FILL_BS(8);
    FILL_V8(t8);
}

void ob_err_code_2(void)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    t8 = 0x00;
    /*
    (B7) C상과전류
    (B6) B상 과전류
    (B5) A상 과전류
    (B4) 오결선
    (B3) 온도
    (B0) 시간 초기화
     차후 코딩 필요...
    */
    // if(WMStatus_intern & CLOCK_UNSET)			t8 |= 0x01;
    // if(WMStatus_intern & (GE_MEM |GE_MEM1))		t8 |= 0x02;
    if (WMStatus & TEMPOVER)
        t8 |= 0x08;
    if (WMStatus & WRONG_CONN)
        t8 |= 0x10;
    if (WMStatus & STOVERIA)
        t8 |= 0x20;
    if (WMStatus & STOVERIB)
        t8 |= 0x40;
    if (WMStatus & STOVERIC)
        t8 |= 0x80;
    FILL_BS(8);
    FILL_V8(t8);
}

void ob_err_code_3(void)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    t8 = 0x00;

    /*
    C상Latch_Error
    B상Latch_Error
    A상Latch_Error
    자기 오차 진단
    중성선 오결선
    전압 C 결 상
    전압 B 결 상
    전압 A 결 상
     차후 코딩 필요...
    */
    if (WMStatus & SAGVA)
        t8 |= 0x01;
    if (WMStatus & SAGVB)
        t8 |= 0x02;
    if (WMStatus & SAGVC)
        t8 |= 0x04;
    if (WMStatus & WRONG_NEUT)
        t8 |= 0x08;
    if (WMStatus & SELF_PLS_ERR)
        t8 |= 0x10;
    if (WMStatus & RELAY_ERRA)
        t8 |= 0x20;
    if (WMStatus & RELAY_ERRB)
        t8 |= 0x40;
    if (WMStatus & RELAY_ERRC)
        t8 |= 0x80;

    FILL_BS(8);
    FILL_V8(t8);
}

void ob_err_code_4(void)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    t8 = 0x00;
    /*
    C상과전압
    B상 과전압
    A상 과전압
    C상 저전압
    B상 저전압
    A상 저전압
    Sag
    Swell
     차후 코딩 필요...
    */
    if (WMStatus & SWELL_STS)
        t8 |= 0x01;
    if (WMStatus & SAG_STS)
        t8 |= 0x02;
    if (WMStatus & LA_V_LOW)
        t8 |= 0x04;
    if (WMStatus & LB_V_LOW)
        t8 |= 0x08;
    if (WMStatus & LC_V_LOW)
        t8 |= 0x10;
    if (WMStatus & LA_V_HIGH)
        t8 |= 0x20;
    if (WMStatus & LB_V_HIGH)
        t8 |= 0x40;
    if (WMStatus & LC_V_HIGH)
        t8 |= 0x80;

    DPRINTF(DBG_ERR, _D "%s: get_lpavg_v[%d] WMStatus 0x[%X]  t8 0x[%X]\r\n",
            __func__, (U16)get_lpavg_v(0), WMStatus, t8);

    FILL_BS(8);
    FILL_V8(t8);
}

static void ob_evt_err_code_activate(uint8_t grp_e)
{
    uint8_t errcodeidx = 0;
    ST_PUSH_ACTI_ERRCODE st_push_acti;
    DPRINTF(DBG_TRACE, _D "%s: err_code[%d]\r\n", __func__, grp_e);

    errcodeidx = dsm_covert_grp_e_2_errcodeidx(grp_e);
    if (errcodeidx >= ERR_CODE_MAX_IDX)
    {
        return;
    }

    dsm_push_err_code_nvread(&st_push_acti);
    DPRINT_HEX(DBG_TRACE, "ERR_CODE_ACT", &st_push_acti, ERR_CODE_MAX_IDX,
               DUMP_ALWAYS);
    FILL_BS(8);
    FILL_V8(st_push_acti.code[errcodeidx]);
    DPRINTF(DBG_TRACE, _D "%s: errcodeidx[%d], errcode[0x%02X]\r\n", __func__,
            errcodeidx, st_push_acti.code[errcodeidx]);
}

static void ob_evt_push_script(void)
{
    uint8_t len = 0;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    dsm_push_get_encoded_script_table(&len, &pPdu[pPdu_idx]);
    DPRINT_HEX(DBG_TRACE, "SCRIPTS", &pPdu[pPdu_idx], len, DUMP_ALWAYS);
    pPdu_idx += len;
}

static void ob_evt_push_setup_err_code(void)
{
    int val, i;
    uint8_t array_num = 0;
    uint8_t push_idx = 0;
    ST_PUSH_SETUP_TABLE* pst_push_setup;

    DPRINTF(DBG_TRACE, _D "%s: att_id %d\r\n", __func__, appl_att_id);

    pst_push_setup = dsm_push_setup_get_setup_table();

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:
        memcpy(&pPdu[pPdu_idx], &PUSH_SETUP_ERRCODE_capture_objects[0],
               PUSH_SETUP_ERRCODE_CAPOBJ_SIZE);
        pPdu_idx += PUSH_SETUP_ERRCODE_CAPOBJ_SIZE;

        break;
    case 0x03:
        FILL_STRUCT(0x03);
        FILL_ENUM(PUSH_SND_TP_SVC_HDLC);
        FILL_STRING(1);
        FILL_V8(PUSH_SND_DESTINATION_ASSO_3);
        FILL_ENUM(PUSH_TP_MSG_A_XDR_EN_xDLMS_APDU);

        break;
    case 0x04:  // window
        DPRINTF(DBG_TRACE, "%s: ret = %d, push_idx %d\r\n", __func__,
                dsm_push_setup_is_id(PUSH_SCRIPT_ID_ERR_CODE, &push_idx),
                push_idx);
        dsm_push_setup_is_id(PUSH_SCRIPT_ID_ERR_CODE, &push_idx);

        array_num = pst_push_setup->setup[push_idx].window_cnt;

        if (!array_num)
        {
            FILL_ARRAY(0);
        }
        else
        {
            FILL_ARRAY(array_num);
            for (i = 0; i < array_num; i++)
            {
                FILL_STRUCT(2);
                fill_clock_obj(
                    &pst_push_setup->setup[push_idx].window[0].st_dt);
                pPdu_idx -= 3;
                val = 0xffff;
                FILL_V16(val);
                FILL_V8(0xff);
                fill_clock_obj(
                    &pst_push_setup->setup[push_idx].window[0].sp_dt);
                pPdu_idx -= 3;
                val = 0xffff;
                FILL_V16(val);
                FILL_V8(0xff);
            }
        }
        break;
    case 0x05:  // randomisatin_start_interval
        DPRINTF(DBG_TRACE, "%s: ret = %d, push_idx %d\r\n", __func__,
                dsm_push_setup_is_id(PUSH_SCRIPT_ID_ERR_CODE, &push_idx),
                push_idx);
        if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_ERR_CODE, &push_idx))
        {
            DPRINTF(DBG_TRACE, "idx[%d],intval[%d]\r\n", push_idx,
                    pst_push_setup->setup[push_idx].random_start_intval);
            FILL_U16(pst_push_setup->setup[push_idx].random_start_intval);
        }
        else
        {
            DPRINTF(DBG_TRACE, "idx[%d], intval[%d]\r\n", push_idx,
                    pst_push_setup->setup[push_idx].random_start_intval);
            FILL_U16(pst_push_setup->setup[push_idx].random_start_intval);
        }

        break;
    case 0x06:
        FILL_U08(0);

        break;
    case 0x07:  // repetion_delay
        if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_ERR_CODE, &push_idx))
        {
            DPRINTF(DBG_TRACE, "idx[%d], delay[%d]\r\n", push_idx,
                    pst_push_setup->setup[push_idx].repetition_delay);
            FILL_U16(pst_push_setup->setup[push_idx].repetition_delay);
        }
        break;
    }
}

static void ob_evt_push_setup_lastLP(void)
{
    int val, i;
    uint8_t array_num = 0;
    uint8_t push_idx = 0;
    ST_PUSH_SETUP_TABLE* pst_push_setup;
    DPRINTF(DBG_TRACE, _D "%s: att_id %d\r\n", __func__, appl_att_id);

    pst_push_setup = dsm_push_setup_get_setup_table();

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:  // push_object_list
        memcpy(&pPdu[pPdu_idx], &PUSH_SETUP_lastLP_capture_objects[0],
               PUSH_SETUP_LAST_LP_CAPOBJ_SIZE);
        pPdu_idx += PUSH_SETUP_LAST_LP_CAPOBJ_SIZE;

        break;
    case 0x03:  // send_destination_and_method
        FILL_STRUCT(0x03);
        FILL_ENUM(PUSH_SND_TP_SVC_HDLC);
        FILL_STRING(1);
        FILL_V8(PUSH_SND_DESTINATION_ASSO_3);
        FILL_ENUM(PUSH_TP_MSG_A_XDR_EN_xDLMS_APDU);

        break;
    case 0x04:  // communication window
        DPRINTF(DBG_TRACE, "%s: ret = %d, push_idx %d\r\n", __func__,
                dsm_push_setup_is_id(PUSH_SCRIPT_ID_LAST_LP, &push_idx),
                push_idx);
        dsm_push_setup_is_id(PUSH_SCRIPT_ID_LAST_LP, &push_idx);

        array_num = pst_push_setup->setup[push_idx].window_cnt;

        if (!array_num)
        {
            FILL_ARRAY(0);
        }
        else
        {
            FILL_ARRAY(array_num);
            for (i = 0; i < array_num; i++)
            {
                FILL_STRUCT(2);
                fill_clock_obj(
                    &pst_push_setup->setup[push_idx].window[0].st_dt);
                pPdu_idx -= 3;
                val = 0xffff;
                FILL_V16(val);
                FILL_V8(0xff);
                fill_clock_obj(
                    &pst_push_setup->setup[push_idx].window[0].sp_dt);
                pPdu_idx -= 3;
                val = 0xffff;
                FILL_V16(val);
                FILL_V8(0xff);
            }
        }
        break;
    case 0x05:  // randomisatin_start_interval
        if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_LAST_LP, &push_idx))
        {
            DPRINTF(DBG_TRACE, "idx[%d], random_start_intval[%d]\r\n", push_idx,
                    pst_push_setup->setup[push_idx].random_start_intval);
            FILL_U16(pst_push_setup->setup[push_idx].random_start_intval);
        }
        else
        {
            DPRINTF(DBG_TRACE, "idx[%d], random_start_intval[%d]\r\n", push_idx,
                    pst_push_setup->setup[push_idx].random_start_intval);
            FILL_U16(pst_push_setup->setup[push_idx].random_start_intval);
        }

        break;
    case 0x06:  // number_of_retries
        FILL_U08(0);
        break;
    case 0x07:  // repetion_delay
        if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_LAST_LP, &push_idx))
        {
            DPRINTF(DBG_TRACE, "idx[%d], repetition_delay[%d]\r\n", push_idx,
                    pst_push_setup->setup[push_idx].repetition_delay);
            FILL_U16(pst_push_setup->setup[push_idx].repetition_delay);
        }
        break;
    }
}

static void ob_lp_status(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_BS(24);
    FILL_V8((uint8_t)((cap_LP_event >> 16) & 0xff));
    FILL_V8((uint8_t)((cap_LP_event >> 8) & 0xff));
    FILL_V8((uint8_t)(cap_LP_event & 0xff));
}

static void ob_time_bat_use(void)
{
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    {
        t32 = get_bat_used_time();
        FILL_U32(t32);
    }
}

static void ob_time_bat_inst(void)
{
    bat_install_type batinst;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (nv_read(I_BAT_INST, (uint8_t*)&batinst))
    {
        if (batinst.installed)
        {
            fill_clock_obj(&batinst.instime);
            return;
        }
    }
    fill_unspecified_clock();
}

static void fill_season_prof_struct(bool curr)
{
    int i;
    int idx;
    bool rslt;
    season_date_type* season;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    rslt = false;
    if (curr)
    {
        if (prog_curr_is_available() && prog_season_available())
        {
            if (nv_read(I_SEASON_PROFILE_ACTIVE, appl_tbuff))
                rslt = true;
        }
    }
    else
    {
        if (prog_fut_is_available() && futprog_season_available())
        {
            if (nv_read(I_SEASON_PROFILE_PASSIVE, appl_tbuff))
                rslt = true;
        }
    }

    idx = pPdu_idx;

    // fill unspecified date
    FILL_ARRAY(SEASON_PROF_SIZE);
    idx += 2;  // array tag
    for (i = 0; i < SEASON_PROF_SIZE; i++)
    {
        memcpy(&pPdu[pPdu_idx], &packed_season_info_r[0], SEASON_INFO_SIZE);
        pPdu_idx += SEASON_INFO_SIZE;
    }

    if (rslt)
    {
        // replace unspecified data
        season = (season_date_type*)appl_tbuff;
        for (i = 0; i < season->cnt; i++)
        {
            idx += 2;  // struct tag
                       // season name
            pPdu[idx + 2] = (uint8_t)i;
            idx += 3;
            // seasom start month/date
            pPdu[idx + 4] = season->season[i].month;
            pPdu[idx + 5] = season->season[i].date;
            idx += 14;
            // week id
            pPdu[idx + 2] = season->season[i].week_id;
            idx += 3;
        }
    }
}

static void fill_week_prof_struct(bool curr)
{
    int i, j;
    int idx;
    bool rslt;
    week_date_type* week;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    rslt = false;
    if (curr)
    {
        if (prog_curr_is_available() && prog_week_available())
        {
            if (nv_read(I_WEEK_PROFILE_ACTIVE, appl_tbuff))
                rslt = true;
        }
    }
    else
    {
        if (prog_fut_is_available() && futprog_week_available())
        {
            if (nv_read(I_WEEK_PROFILE_PASSIVE, appl_tbuff))
                rslt = true;
        }
    }

    idx = pPdu_idx;

    // fill unspecified date
    FILL_ARRAY(WEEK_PROF_SIZE);
    idx += 2;
    for (i = 0; i < WEEK_PROF_SIZE; i++)
    {
        memcpy(&pPdu[pPdu_idx], &packed_week_info_r[0], WEEK_INFO_SIZE);
        pPdu_idx += WEEK_INFO_SIZE;
    }

    if (rslt)
    {
        // replace unspecified data
        week = (week_date_type*)appl_tbuff;
        for (i = 0; i < week->cnt; i++)
        {
            idx += 2;  // struct tag
            pPdu[idx + 2] = week->week[i].week_id;
            idx += 3;
            for (j = 0; j < WEEK_LEN; j++)
            {
                pPdu[idx + 1] = week->week[i].day_id[j];
                idx += 2;
            }
        }
    }
}

static void fill_day_prof_struct(bool first, bool curr)
{
    int i;
    int idx;
    bool rslt;
    dayid_table_type* daytbl;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    rslt = false;
    if (curr)
    {
        if (prog_curr_is_available() && prog_day_available())
        {
            nv_sub_info.ch[0] = (uint8_t)(appl_resp_block_num - 1L);
            if (nv_read(I_DAY_PROFILE_ACTIVE, appl_tbuff))
                rslt = true;
        }
    }
    else
    {
        if (prog_fut_is_available() && futprog_day_available())
        {
            nv_sub_info.ch[0] = (uint8_t)(appl_resp_block_num - 1L);
            if (nv_read(I_DAY_PROFILE_PASSIVE, appl_tbuff))
                rslt = true;
        }
    }

    // replace from real nv_data
    daytbl = (dayid_table_type*)appl_tbuff;

    idx = pPdu_idx;

    // fill unspecified data
    FILL_STRUCT(0x02);
    idx += 2;
    if (rslt)
    {
        FILL_U08(daytbl->day_id);
    }
    else
    {
        FILL_U08((uint8_t)(appl_resp_block_num - 1L));
    }
    idx += 2;
    FILL_ARRAY(DAY_STRUCT_LEN);
    idx += 2;
    for (i = 0; i < DAY_STRUCT_LEN; i++)
    {
        memcpy(&pPdu[pPdu_idx], &packed_day_info_r[0], DAY_INFO_SIZE);
        pPdu_idx += DAY_INFO_SIZE;
    }

    if (rslt)
    {
        for (i = 0; i < daytbl->tou_conf_cnt; i++)
        {
            idx += 2;  // struct tag
            // start time
            pPdu[idx + 2] = daytbl->tou_conf[i].hour;
            pPdu[idx + 3] = daytbl->tou_conf[i].min;
            idx += 6;
            // logical script name
            idx += 8;
            // script selector (long unsigned : 18)
            pPdu[idx + 2] = daytbl->tou_conf[i].rate;
            idx += 3;
        }
    }
    else
    {
        if (first && curr)
        {
            // replace from current tou table in case of first block
            for (i = 0; i < mt_conf.tou.cnt; i++)
            {
                idx += 2;  // struct tag
                // start time
                pPdu[idx + 2] = mt_conf.tou.conf[i].hour;
                pPdu[idx + 3] = mt_conf.tou.conf[i].min;
                idx += 6;
                // logical script name
                idx += 8;
                // script selector (long unsigned : 18)
                pPdu[idx + 2] = mt_conf.tou.conf[i].rate;
                idx += 3;
            }
        }
    }

    // last block 체크
    if (appl_resp_block_num < (uint32_t)DAY_PROF_SIZE)
    {
        appl_resp_last_block = 0;
    }
    else
    {
        appl_resp_last_block = 1;
    }
}

static void fill_holidays_struct(bool first, bool curr)
{
    bool rslt;
    int i;
    int idx;
    uint8_t yr_h, yr_l;
    uint16_t t16;
    holiday_date_type* holdate;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    rslt = false;
    if (curr)
    {
        if (prog_curr_is_available() && prog_hol_available())
        {
            nv_sub_info.ch[0] = (uint8_t)(appl_resp_block_num - 1L);
            nv_sub_info.ch[1] = HOL_OF_BLOCK;
            if (nv_read(I_HOLIDAYS_A, appl_tbuff))
                rslt = true;
        }
    }
    else
    {
        if (prog_fut_is_available() && futprog_hol_available())
        {
            nv_sub_info.ch[0] = (uint8_t)(appl_resp_block_num - 1L);
            nv_sub_info.ch[1] = HOL_OF_BLOCK;
            if (nv_read(I_HOLIDAYS_P, appl_tbuff))
                rslt = true;
        }
    }

    idx = pPdu_idx;

    // fill unspecified data
    t16 = (uint16_t)((uint16_t)(appl_resp_block_num - 1L) * HOLIDAYS_PER_BLOCK);

    for (i = 0; i < HOLIDAYS_PER_BLOCK; i++)
    {
        memcpy(&pPdu[pPdu_idx], &packed_special_day_r[0],
               SPECIAL_DAY_INFO_SIZE);
        pPdu[pPdu_idx + 3] = (uint8_t)(t16 >> 8);
        pPdu[pPdu_idx + 4] = (uint8_t)(t16 & 0xff);

        t16 += 1;
        pPdu_idx += SPECIAL_DAY_INFO_SIZE;
    }

    if (rslt)
    {
        // replace from real nv_data
        holdate = (holiday_date_type*)appl_tbuff;

        yr_h = (uint8_t)(((uint16_t)holdate->yr + BASE_YEAR) >> 8);
        yr_l = (uint8_t)(((uint16_t)holdate->yr + BASE_YEAR) & 0xff);

        for (i = 0; i < HOLIDAYS_PER_BLOCK; i++)
        {
            idx += 2;  // struct tag
            idx += 3;  // sp index

            if (holdate->holiday[i].month != 0xff &&
                holdate->holiday[i].date != 0xff)
            {
                if (!first)  // non-periodic
                {
                    pPdu[idx + 2] = yr_h;
                    pPdu[idx + 3] = yr_l;
                }
                pPdu[idx + 4] = holdate->holiday[i].month;
                pPdu[idx + 5] = holdate->holiday[i].date;
                idx += 7;
                pPdu[idx + 1] = holdate->holiday[i].day_id;
                idx += 2;
            }
            else
            {
                idx += 9;
            }
        }
    }

    // last block 체크
    if (appl_resp_block_num < (uint32_t)HOLIDAY_BLOCK_LEN)
    {
        appl_resp_last_block = 0;
    }
    else
    {
        appl_resp_last_block = 1;
    }
}

static void fill_mtinit_log_data(uint8_t* tptr)
{
    uint8_t i, idx, cnt, t8;
    mtinit_log_data_type* mtinit;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (nv_read(I_MTINIT_LOG, tptr))
    {
        mtinit = (mtinit_log_data_type*)tptr;

        t8 = mtinit->cnt;

        if (t8)
        {
            idx = (t8 - 1) % LOG_BUFF_SIZE;
            cnt = (t8 < LOG_RECORD_LEN) ? t8 : LOG_RECORD_LEN;

            FILL_ARRAY(cnt);
            for (i = 0; i < cnt; i++)
            {
                FILL_STRUCT(0x02);
                fill_clock_obj(&mtinit->log.dt[idx]);
                FILL_U08(t8 - i);

                if (idx == 0)
                    idx = LOG_BUFF_SIZE - 1;
                else
                    idx--;
            }
        }
        else
        {
            FILL_ARRAY(0);
        }
    }
    else
    {
        FILL_ARRAY(0);
    }
}

static void fill_log_data(elog_kind_type elog, uint8_t* tptr)
{
    uint8_t i, idx, cnt;
    uint8_t t8;
    uint16_t logcnt, t16;

    logcnt = get_log_cnt(elog, tptr);

    DPRINTF(DBG_TRACE, _D "%s: elog[%d], logcnt[%d]\r\n", __func__, elog,
            logcnt);

    if (logcnt == 0)
    {
        FILL_ARRAY(0);
        return;
    }
    idx = (logcnt - 1) % LOG_BUFF_SIZE;
    cnt = (logcnt < LOG_RECORD_LEN) ? logcnt : LOG_RECORD_LEN;

    switch (elog)
    {
    case eLogSCurrLimit:
        if (nv_read(I_LOG_SCURR, tptr))
        {
            scurr_log_data_type* logdata;
            logdata = (scurr_log_data_type*)tptr;

            FILL_ARRAY(cnt);
            for (i = 0; i < cnt; i++)
            {
                FILL_STRUCT(0x06);
                fill_clock_obj(&logdata->evt[idx].dt);
                FILL_U08(logdata->evt[idx].scurrcnt);
                FILL_U16(logdata->evt[idx].limit);
                FILL_U16(logdata->evt[idx].limit2);
                FILL_U08(logdata->evt[idx].limitcnt);
                FILL_U08(logdata->evt[idx].limitcnt_n1);

                if (idx == 0)
                    idx = LOG_BUFF_SIZE - 1;
                else
                    idx--;
            }
        }
        else
        {
            FILL_ARRAY(0);
        }
        break;

    case eLogMagnetDet:
        if (nv_read(I_LOG_DATA1, tptr))
        {
            log_data1_type* logdata;
            logdata = (log_data1_type*)tptr;

            FILL_ARRAY(cnt);
            for (i = 0; i < cnt; i++)
            {
                FILL_STRUCT(0x03);
                fill_clock_obj(&logdata->evt[idx].dt);
                FILL_U32(logdata->evt[idx].durtime);
                t16 = logcnt - i;
                FILL_U16(t16);

                if (idx == 0)
                    idx = LOG_BUFF_SIZE - 1;
                else
                    idx--;
            }
        }
        else
        {
            FILL_ARRAY(0);
        }
        break;

    case eLogMtInit:
        fill_mtinit_log_data(tptr);
        break;

    default:
        nv_sub_info.ch[0] = elog;
        if (nv_read(I_LOG_DATA, tptr))
        {
            log_data_type* logdata;
            logdata = (log_data_type*)tptr;

            FILL_ARRAY(cnt);
            for (i = 0; i < cnt; i++)
            {
                FILL_STRUCT(0x02);
                fill_clock_obj(&logdata->dt[idx]);
                if (elog == eLogCoverOpen || elog == eLogTCoverOpen ||
                    elog == eLogrLoadCtrl || elog == eLogWrongConn ||
                    elog == eLogSCurrNonSel || elog == eLogErrDiagonist ||
                    elog == eLogSysSwUp || elog == eLogMtrSwUp ||
                    elog == eLogInModemUp || elog == eLogExModemUp

                )
                {
                    t8 = (uint8_t)(logcnt - i);
                    FILL_U08(t8);
                }
                else
                {
                    t16 = logcnt - i;
                    FILL_U16(t16);
                }

                if (idx == 0)
                    idx = LOG_BUFF_SIZE - 1;
                else
                    idx--;
            }
        }
        else
        {
            FILL_ARRAY(0);
        }
        break;
    }
}

static void fill_log_cap(elog_kind_type elog)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (elog)
    {
    case eLogSCurrLimit:
        memcpy(&pPdu[pPdu_idx], &sCurrLimit_LOG_capture_objects[0],
               SCLOG_CAPOBJ_SIZE);
        pPdu_idx += SCLOG_CAPOBJ_SIZE;
        break;

    case eLogMagnetDet:
        memcpy(&pPdu[pPdu_idx], &Magnet_LOG_capture_objects[0],
               MAGLOG_CAPOBJ_SIZE);
        pPdu_idx += MAGLOG_CAPOBJ_SIZE;
        break;

    default:
        memcpy(&pPdu[pPdu_idx], &event_log_capture_objects[0], EVTLOG_CAP_SIZE);
        memcpy(&pPdu[pPdu_idx + 27], &event_log_cnt_objects[elog][0],
               OBIS_ID_SIZE);
        pPdu_idx += EVTLOG_CAP_SIZE;
        break;
    }
}

static void fill_cert_log_cap(elog_cert_kind_type elog)
{
    DPRINTF(DBG_TRACE, _D "%s: elog[%d]\r\n", __func__, elog);

    switch (elog)
    {
    case eLogCert_NG:
        memcpy(&pPdu[pPdu_idx], &cert_log_capture_objects[0],
               CERT_LOG_CAPOBJ_SIZE);
        pPdu_idx += CERT_LOG_CAPOBJ_SIZE;
        break;

    default:

        break;
    }
}

static void ob_evt_log(elog_kind_type elog)
{
    uint8_t* tptr;
    uint32_t t32;

    tptr = appl_tbuff;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (elog == numLogs)
    {
        elog = conv_elog_from_grpe(obis_ge);
    }

    switch (appl_att_id)
    {
    case 0x02:  // [buffer]
        fill_log_data(elog, tptr);
        break;

    case 0x03:  // [capture_objects]
        fill_log_cap(elog);
        break;

    case 0x04:  // [capture_period]
        t32 = 0L;
        FILL_U32(t32);
        break;

    case 0x05:         // [sort_method]
        FILL_ENUM(1);  // sort method(FIFO)
        break;

    case 0x06:  // [sort_object]
        memcpy(&pPdu[pPdu_idx], &LP_sort_object[0], LP_SORTOBJ_SIZE);
        pPdu_idx += LP_SORTOBJ_SIZE;
        break;

    case 0x07:  // [entries_in_use]
        if (elog == eLogMtInit)
        {
            t32 = (uint32_t)get_mtinit_log_cnt(tptr);
            if (t32 > LOG_RECORD_LEN)
                t32 = LOG_RECORD_LEN;
        }
        else
        {
            t32 = (log_cnt[elog] < LOG_RECORD_LEN) ? (uint32_t)log_cnt[elog]
                                                   : (uint32_t)LOG_RECORD_LEN;
        }
        FILL_U32(t32);
        break;

    case 0x08:  // [profile_entries]
        t32 = LOG_RECORD_LEN;
        FILL_U32(t32);
        break;
    }
}

uint8_t g_cur_prg_name[PROG_ID_SIZE + 1];
uint8_t g_fut_prg_name[PROG_ID_SIZE + 1];
void dsm_progname_update_forReport(void)
{
    uint8_t t_buf[32];

    memset(g_cur_prg_name, 0x00, sizeof(g_cur_prg_name));
    memset(g_fut_prg_name, 0x00, sizeof(g_fut_prg_name));

    fill_prog_name(true, g_cur_prg_name, t_buf);
    fill_prog_name(false, g_fut_prg_name, t_buf);
}

uint8_t* dsm_get_progname_forReport(bool curprog)
{
    if (curprog)
        return g_cur_prg_name;
    else
        return g_fut_prg_name;
}

uint8_t g_manuf_id[MANUF_ID_SIZE + 1];
void dsm_meterid_update_forReport(uint8_t* manuf_id)
{
    memset(g_manuf_id, 0x00, sizeof(g_manuf_id));
    memcpy(g_manuf_id, manuf_id, MANUF_ID_SIZE);
}
uint8_t* dsm_get_meterid_forReport(void) { return g_manuf_id; }

void dsm_update_forReport(void)
{
    uint8_t manuf_id[MANUF_ID_SIZE];

    get_manuf_id(manuf_id);

    dsm_meterid_update_forReport(manuf_id);
    dsm_progname_update_forReport();
}

static void fill_prog_name(bool curprog, uint8_t* name, uint8_t* tptr)
{
    program_info_type* proginfo;

    proginfo = (program_info_type*)tptr;

    // 종별
    name[0] = '1' + mt_rtkind;
    // 검침일
    name[1] = '0' + (reg_mr_date / 10);
    name[2] = '0' + (reg_mr_date % 10);
    // 선택 유효 전력량
    if (mt_is_uni_dir())
    {
        name[3] = 'R';
        name[4] = 'R';
    }
    else
    {
        name[3] = 'S';
        name[4] = 'R';
    }
    // 게량 모드
    if (meas_method == E_SINGLE_DIR)
        name[5] = 'D';
#if 1 /* bccho, 2024-09-05, 삼상 */
    else if (meas_method == E_BASIC)
        name[5] = 'S';
    else
        name[5] = 'U';
#else
    else
        name[5] = 'S';
#endif

    // 업데이트 버젼
    if (curprog)
    {
        // 현재 프로그램
        if (prog_in_state == E_PROG_NONE || prog_in_state == E_PROG_INIT)
        {
            name[6] = '0';
            name[7] = '0';
        }
        else if (prog_in_state == E_PROG_KEY)
        {
            name[6] = 'K';
            name[7] = 'E';
        }
        else
        {
            // 통신 프로로그램 업데이트
            nv_sub_info.ch[0] = 0;
            if (nv_read(I_PROG_INFO, (uint8_t*)proginfo))
            {
                name[6] = proginfo->name[PROG_ID_SIZE - 2];
                name[7] = proginfo->name[PROG_ID_SIZE - 1];
            }
            else
            {
                name[6] = '0';
                name[7] = '0';
            }
        }
    }
    else
    {
        // 에약 프로그램
        if (prog_fut_is_available())
        {
            nv_sub_info.ch[0] = 1;
            if (nv_read(I_PROG_INFO, (uint8_t*)proginfo))
            {
                memcpy(name, proginfo->name, PROG_ID_SIZE);
            }
            else
            {
                memset(name, ' ', PROG_ID_SIZE);
            }
        }
        else
        {
            memset(name, ' ', PROG_ID_SIZE);
        }
    }
}

static void ob_tou_cal(void)
{
    /*
    Time of Use : 계시별 요금제
    */

    uint8_t name[PROG_ID_SIZE];

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x02: /* calendar name active : 프로그램 ID */
        fill_prog_name(true, name, appl_tbuff);  // TOU Program ID
        fill_octet_string_x(name, PROG_ID_SIZE);
        break;

    case 0x03: /* season profile active : 계절 구분 */
        if (act_is_curr_prog_cmd())
        {
            fill_season_prof_struct(true);
        }
        else
        {
            fill_season_prof_struct(false);
        }
        break;

    case 0x04: /* week profile table active : 요일 구분 */
        if (act_is_curr_prog_cmd())
        {
            fill_week_prof_struct(true);
        }
        else
        {
            fill_week_prof_struct(false);
        }
        break;

    case 0x05: /* day profile table active : 일 구분 */
        approc_fill_get_resp_block(true);
        break;

    case 0x06: /* calendar name passive : 예약 프로그램 ID */
        fill_prog_name(false, name, appl_tbuff);
        fill_octet_string_x(name, PROG_ID_SIZE);
        break;

    case 0x07: /* season profile passive : 예약 프로그램 계절 구분 */
        fill_season_prof_struct(false);
        break;

    case 0x08: /* week profile table passive : 예약 프로그램 요일 구분
                */
        fill_week_prof_struct(false);
        break;

    case 0x09: /* day profile table passive : 예약 프로그램 일 구분 */
        approc_fill_get_resp_block(true);
        break;

    case 0x0A: /* active passive calendar time : 예약 프로그램 적용
                  일자/시간 */
        if (prog_fut_is_available())
        {
            fill_clock_obj(&fut_prog_work_time);
        }
        else
        {
            fill_unspecified_clock();
        }
        break;
    }
}

static void ob_holidays(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    approc_fill_get_resp_block(true);
}

static void ob_hdlc_setup(void)
{
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x02:  // speed
        FILL_ENUM(mdm_baud);
        break;

    case 0x03:  // window size(tx)
    case 0x04:  // window size(rx)
        FILL_U08(1);
        break;

    case 0x05:  // max tx length
        t16 = MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO;
        FILL_U16(t16);
        break;
    case 0x06:  // max rx length
        t16 = MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO;
        FILL_U16(t16);
        break;

    case 0x07:  // inter_octet_time_out (msec)
        t16 = 500;
        FILL_U16(t16);  // interframe_tick_timer
        break;
    case 0x08:  // inactivity time out (secs)
        t16 = 120;
        FILL_U16(t16);
        break;

    case 0x09:  // device address
        t16 = (uint16_t)dl_meter_addr;
        FILL_U16(t16);
        break;
    }
}

static void ob_load_profile(void)
{
    int i;
    uint32_t entry_from, entry_to;
    uint16_t selcol_from, selcol_to;
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    lpinfo_is_caped = false;
    if (!lpinfo_is_caped)
    {
        lpinfo_is_caped = true;
        mt_dir_for_lp = MT_BOTH_DIR;
        mt_uni_dir = false;
        recsize_for_lp = (uint8_t)(sizeof(lp_record_type));

        lpsize_for_lp = LP_SIZE;
        buffsize_for_lp = LP_BUF_SIZE;
        lpindex_for_lp = LP_index;

        lpindex_while_lpblocked = 0;
    }

    switch (appl_att_id)
    {
    case 0x02:
        if (lpindex_for_lp == 0L)
        {
            getresp_LP_len = 0;
        }
        else
        {
            // selective access ?
            if (appl_selacs_len == 0)
            {
                // non-selective
                getresp_LP_index = lpindex_for_lp - 1;
                getresp_LP_len = ((lpindex_for_lp <= (uint32_t)lpsize_for_lp)
                                      ? (uint16_t)lpindex_for_lp
                                      : lpsize_for_lp);

                // LP column information
                get_LP_selcolmn_info(1, 0);  // full selective column
            }
            else
            {
                // skip appl_selacs_len and tag check
                i = 0;
                if (appl_selacs[i++] ==
                    2)  // support only entry range selection
                {
                    i += 2;
                    i += 1;
                    ToH32((U8_16_32*)&entry_from, &appl_selacs[i]);
                    i += 4;
                    i += 1;  // tag
                    ToH32((U8_16_32*)&entry_to, &appl_selacs[i]);
                    i += 4;
                    i += 1;  // tag
                    ToH16((U8_16*)&selcol_from, &appl_selacs[i]);
                    i += 2;
                    i += 1;  // tag
                    ToH16((U8_16*)&selcol_to, &appl_selacs[i]);
                    i += 2;
                    // entry start index and length

                    if (lpindex_for_lp <= (uint32_t)lpsize_for_lp)
                    {
                        if (entry_from == 0L || entry_from > lpindex_for_lp)
                        {
                            getresp_LP_len = 0;
                        }

                        if (entry_to > lpindex_for_lp || entry_to == 0L)
                        {
                            if ((entry_from > LP_index) ||
                                (entry_to > LP_index))
                            {
                                entry_from = LP_index;
                                entry_to = LP_index;
                            }
                            else
                            {
                                entry_to = lpindex_for_lp;
                            }
                        }
                        getresp_LP_index = entry_to - 1;
                        getresp_LP_len = (uint16_t)(entry_to - entry_from + 1);
                    }
                    else
                    {
                        t32 = (uint32_t)lpsize_for_lp;
                        t32 = (lpindex_for_lp - 1) % t32;
                        t32 += 1;  // overlaped index
                        if (entry_from == 0L ||
                            entry_from > (uint32_t)lpsize_for_lp)
                        {
                            getresp_LP_len = 0;
                        }

                        if (entry_to <= t32)  // overlaped range ?
                        {
                            getresp_LP_index =
                                lpindex_for_lp - t32 + entry_to - 1;
                        }
                        else
                        {
                            getresp_LP_index =
                                lpindex_for_lp - t32 - 1 -
                                ((uint32_t)lpsize_for_lp - entry_to);
                        }
                        getresp_LP_len = (uint16_t)(entry_to - entry_from + 1);
                    }

                    if (getresp_LP_index > (LP_index - 1))
                    {
                        getresp_LP_index = (uint16_t)(LP_index - 1);
                    }

                    if (getresp_LP_len > LP_index)
                    {
                        getresp_LP_index = (uint16_t)(LP_index - 1);
                        getresp_LP_len = (uint16_t)(LP_index);
                    }

                    if (getresp_LP_len)
                    {
                        // LP column information
                        get_LP_selcolmn_info(
                            (uint8_t)selcol_from,
                            (uint8_t)selcol_to);  // selective column
                    }
                }
                else
                {
                    getresp_LP_len = 0;
                }
            }
        }

        if (getresp_LP_len == 0)
        {
            FILL_ARRAY(0);
        }
        else
        {
            approc_fill_get_resp_block(true);
            comm_dt = cur_rtc;
            // meter is registered
            mt_register_dt = comm_dt;
        }
        break;

    case 0x03:
        if (0)
        {
            memcpy(&pPdu[pPdu_idx], &LP_capture_objects_uni[0],
                   LP_CAPOBJ_SIZE_UNI);
            pPdu_idx += LP_CAPOBJ_SIZE_UNI;
        }
        else
        {
            memcpy(&pPdu[pPdu_idx], &LP_capture_objects_both[0],
                   LP_CAPOBJ_SIZE_BOTH);
            pPdu_idx += LP_CAPOBJ_SIZE_BOTH;
        }
        break;

    case 0x04:
        t32 = lp_interval * 60;
        FILL_U32(t32);  // unit = sec
        break;

    case 0x05:
        FILL_ENUM(1);  // sort method(FIFO)
        break;

    case 0x06:
        memcpy(&pPdu[pPdu_idx], &LP_sort_object[0], LP_SORTOBJ_SIZE);
        pPdu_idx += LP_SORTOBJ_SIZE;
        break;

    case 0x07:
        t32 = (lpindex_for_lp < lpsize_for_lp) ? lpindex_for_lp : lpsize_for_lp;
        FILL_U32(t32);
        break;

    case 0x08:
        t32 = lpsize_for_lp;
        FILL_U32(t32);
        break;
    }
}

static void ob_lpavg(void)
{
    int i;
    uint32_t entry_from, entry_to;
    uint16_t selcol_from, selcol_to;
    uint16_t t16;
    uint32_t t32;

    DPRINTF(DBG_INFO, _D "%s: lpavg_v_index[%d]\r\n", __func__, lpavg_v_index);

    switch (appl_att_id)
    {
    case 0x02:
        if (lpavg_v_index == 0L)
        {
            getresp_LP_len = 0;
            ;
        }
        else
        {
            if (appl_selacs_len == 0)
            {
                // non-selective
                getresp_LP_index = lpavg_v_index - 1;
                getresp_LP_len = (lpavg_v_index <= LPAVG_SIZE)
                                     ? (uint16_t)lpavg_v_index
                                     : LPAVG_SIZE;

                // LP column information
                get_LPavg_selcolmn_info(1, 0);  // full selective column
            }
            else
            {
                // skip appl_selacs_len and tag check
                i = 0;
                if (appl_selacs[i++] ==
                    2)  // support only entry range selection
                {
                    i += 2;  // struct_tag(2B, tag+val)
                    i += 1;  // tag
                    ToH32((U8_16_32*)&entry_from, &appl_selacs[i]);
                    i += 4;
                    i += 1;  // tag
                    ToH32((U8_16_32*)&entry_to, &appl_selacs[i]);
                    i += 4;
                    i += 1;  // tag
                    ToH16((U8_16*)&selcol_from, &appl_selacs[i]);
                    i += 2;
                    i += 1;  // tag
                    ToH16((U8_16*)&selcol_to, &appl_selacs[i]);
                    i += 2;

                    // length
                    t16 = (lpavg_v_index <= LPAVG_SIZE)
                              ? (uint16_t)lpavg_v_index
                              : LPAVG_SIZE;
                    if ((entry_from == 0L) || (entry_from > t16))
                    {
                        getresp_LP_len = 0;
                    }
                    else
                    {
                        getresp_LP_index = lpavg_v_index - entry_from;
                        if (entry_to == 0)
                        {
                            getresp_LP_len = t16;
                        }
                        else
                        {
                            getresp_LP_len =
                                (uint16_t)(entry_to - entry_from + 1);
                        }
                        get_LPavg_selcolmn_info(
                            (uint8_t)selcol_from,
                            (uint8_t)selcol_to);  // selective column
                    }
                }
                else
                {
                    getresp_LP_len = 0;
                }
            }
        }

        if (getresp_LP_len == 0)
        {
            FILL_ARRAY(0);
        }
        else
        {
            approc_fill_get_resp_block(true);
        }
        break;

    case 0x03:
        if (mt_is_onephase())
        {
            memcpy(&pPdu[pPdu_idx], &LPavg_capture_objects[0], LPavg_CAP_SIZE);
            pPdu_idx += LPavg_CAP_SIZE;
        }
        else
        {
            memcpy(&pPdu[pPdu_idx], &LPavg_3PHS_capture_objects[0],
                   LPavg_3PHS_CAP_SIZE);
            pPdu_idx += LPavg_3PHS_CAP_SIZE;
        }
        break;

    case 0x04:

        t32 = lpavg_interval * 60;  // unit = sec
        FILL_U32(t32);
        break;

    case 0x05:
        FILL_ENUM(1);  // sort method(FIFO)
        break;

    case 0x06:
        memcpy(&pPdu[pPdu_idx], &LP_sort_object[0], LP_SORTOBJ_SIZE);
        pPdu_idx += LP_SORTOBJ_SIZE;
        break;

    case 0x07:
        t32 = (lpavg_v_index < LPAVG_SIZE) ? lpavg_v_index : LPAVG_SIZE;
        FILL_U32(t32);
        break;

    case 0x08:
        t32 = LPAVG_SIZE;
        FILL_U32(t32);
        break;
    }
}

static void ob_date_time(void)
{
    int16_t t16;
    prog_dl_type* pdl;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x02:
        comm_dt = cur_rtc;

        fill_clock_obj(&comm_dt);
        break;
    case 0x03:
        t16 = TIME_ZONE_TO_GMT;
        FILL_S16(t16);
        break;
    case 0x04:
        FILL_U08(cap_clk_sts);
        break;
    case 0x05:
        if (act_is_curr_prog_cmd())
        {
            if (mt_conf.dlsinfo.enabled & DLS_ENABLED)
            {
                fill_dst_clock_obj(&mt_conf.dlsinfo.bgn_dt,
                                   mt_conf.dlsinfo.bgn_week);
            }
            else
            {
                fill_unspecified_clock();
            }
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_DLS_BGN, appl_tbuff))
            {
                pdl = (prog_dl_type*)appl_tbuff;
                fill_dst_clock_obj(&pdl->dlsinfo.bgn_dt, pdl->dlsinfo.bgn_week);
            }
            else
            {
                fill_unspecified_clock();
            }
        }
        break;
    case 0x06:
        if (act_is_curr_prog_cmd())
        {
            if (mt_conf.dlsinfo.enabled & DLS_ENABLED)
                fill_dst_clock_obj(&mt_conf.dlsinfo.end_dt,
                                   mt_conf.dlsinfo.end_week);
            else
                fill_unspecified_clock();
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_DLS_END, appl_tbuff))
            {
                pdl = (prog_dl_type*)appl_tbuff;
                fill_dst_clock_obj(&pdl->dlsinfo.end_dt, pdl->dlsinfo.end_week);
            }
            else
            {
                fill_unspecified_clock();
            }
        }
        break;
    case 0x07:
        if (act_is_curr_prog_cmd())
        {
            if (mt_conf.dlsinfo.enabled & DLS_ENABLED)
            {
                FILL_S08(mt_conf.dlsinfo.dev);
            }
            else
            {
                FILL_S08(0);
            }
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_DLS_DEV, appl_tbuff))
            {
                pdl = (prog_dl_type*)appl_tbuff;
                FILL_S08(pdl->dlsinfo.dev);
            }
            else
            {
                FILL_S08(0);
            }
        }
        break;
    case 0x08:
        if (act_is_curr_prog_cmd())
        {
            if (mt_conf.dlsinfo.enabled & DLS_ENABLED)
            {
                FILL_BOOL(1);
            }
            else
            {
                FILL_BOOL(0);
            }
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_DLS_ENA, appl_tbuff))
            {
                pdl = (prog_dl_type*)appl_tbuff;
                FILL_BOOL(pdl->dlsinfo.enabled);
            }
            else
            {
                FILL_BOOL(0);
            }
        }
        break;
    case 0x09:
        FILL_ENUM(1);  // internal crystal
        break;
    }
}

static void ob_lcdset_parm(void)
{
    uint8_t lcdset;
    prog_dl_type* pdl;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (act_is_curr_prog_cmd())
    {
    ob_lcdset_parm1:
        lcdset = 0;

        if (meas_is_basic())
        {
            lcdset |= 0x01;
#if PHASE_NUM != SINGLE_PHASE
        }
        else if (meas_method == E_VECTSUM)
        {
            lcdset |= 0x02;
#endif
        }
        else
        {
            lcdset |= 0x04;
        }
        if (circdsp_is_smode())
            lcdset |= 0x08;

        fill_octet_string_x((uint8_t*)&lcdset, 1);
    }
    else
    {
        if (!fut_pgm_read_set_bits(SETBITS_LCDSET_PARM, appl_tbuff))
            goto ob_lcdset_parm1;

        pdl = (prog_dl_type*)appl_tbuff;
        fill_octet_string_x((uint8_t*)&pdl->lcdsetparm, 1);
    }
}

void ob_device_id(void)
{
    /*
    COSEM 계기 식별자:
        제조사 고유코드 : "XXX", 제조 일자 : “200301”, 제조관리번호 :
    “A” Logical Device(LD) 번호 : 장치 관리용 = 1, 한전 관리용 = 2 규격
    버전 : “3.X”이상 (보안 계기) “2.X”(비보안 계기)

    LDN: Logical Device Name
    */
    device_id_type dev;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_sap)
    {
    case SAP_SEC_UTILITY:
    case SAP_SEC_SITE:
    case SAP_PRIVATE:  ////JP.KIM 24.11.08	1) 생산프로그램 PROTOCOL을 dlms
                       /// 방식으로 변경
        if (!nv_read(I_DEVICE_ID_KEPCO, (U8*)&dev))
        {
            memcpy(dev.devid, &logical_device_name_r_kepco[0], DEVICE_ID_SIZE);
        }
        else
        {
            ////JP.KIM 24.11.08	1) 생산프로그램 PROTOCOL을 dlms 방식으로
            /// 변경
            dev.devid[0] = FLAG_ID1;
            dev.devid[1] = FLAG_ID2;
            dev.devid[2] = FLAG_ID3;
            dev.devid[13] = logical_device_name_r_kepco[13];
            dev.devid[14] = logical_device_name_r_kepco[14];
            dev.devid[15] = logical_device_name_r_kepco[15];
        }
        break;

    default:
        if (!nv_read(I_DEVICE_ID, (U8*)&dev))
        {
            memcpy(dev.devid, &logical_device_name_r[0], DEVICE_ID_SIZE);
        }
        else
        {
            ////JP.KIM 24.11.08	1) 생산프로그램 PROTOCOL을 dlms 방식으로
            /// 변경
            dev.devid[0] = FLAG_ID1;
            dev.devid[1] = FLAG_ID2;
            dev.devid[2] = FLAG_ID3;
            dev.devid[13] = logical_device_name_r[13];
            dev.devid[14] = logical_device_name_r[14];
            dev.devid[15] = logical_device_name_r[15];
        }

        break;
    }
    fill_octet_string_x(dev.devid, DEVICE_ID_SIZE);
    DPRINT_HEX(DBG_TRACE, "LDN", dev.devid, DEVICE_ID_SIZE,
               DUMP_ALWAYS);  // Logical Device Name
}

static void ob_local_time(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_STRING(0x04);

    FILL_V8(cur_hour);
    FILL_V8(cur_min);
    FILL_V8(cur_sec);
    FILL_V8(0xff);
}

static void ob_local_date(void)
{
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_STRING(0x05);

    t16 = BASE_YEAR + (uint16_t)cur_year;
    FILL_V16(t16);
    FILL_V8(cur_month);
    FILL_V8(cur_date);
    FILL_V8(calc_dayofweek(&cur_rtc));
}

static void ob_curr_tariff(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_U08((uint8_t)(cap_cur_rate + 1));
}

static void ob_billing_parm(void)
{
    uint8_t billparm[BILLING_PARM_SIZE];
    prog_dl_type* pdl;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (act_is_curr_prog_cmd())
        goto ob_billing_parm1;

    if (futprog_billparm_available())
    {
        pdl = (prog_dl_type*)appl_tbuff;
        if (!prog_get_fut_dl(pdl))
            goto ob_billing_parm1;

        fill_octet_string_x(pdl->bill_parm, BILLING_PARM_SIZE);
    }
    else
    {
    ob_billing_parm1:
        rtn_billing_parm(billparm);
        fill_octet_string_x(billparm, BILLING_PARM_SIZE);
    }
}

static void ob_selective_act(void)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_STRING(0x01);
    t8 = mt_is_uni_dir() ? 0x00 : 0x01;
    FILL_V8(t8);
}

static void ob_rLOAD_sig(void)
{
    DPRINTF(DBG_TRACE, _D "%s: load[%d]\r\n", __func__, whm_op.ldctrl);

    FILL_BOOL(whm_op.ldctrl);
}

static void ob_rate_pf(energy_dir_type dir)
{
    float fval;
    mr_ch_type* mr;

    DPRINTF(DBG_TRACE, _D "%s: obis_gb[%d]\r\n", __func__, obis_gb);

    if (appl_att_id == 0x02)
    {
        if (mr_prdnprdseason_from_nv(obis_gb, eMrAccm, obis_gc, obis_gf,
                                     energy_group_to_tariff(obis_ge),
                                     appl_tbuff))
        {
            mr = (mr_ch_type*)appl_tbuff;
            fval = mr->pf[dir];
        }
        else
        {
            fval = 0.0;
        }
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 255);  // unit: unitless
    }
}

static void ob_cum_demand(void)
{
    uint32_t t32;
    mr_dm_type* mr;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        if (mr_prdnprdseason_from_nv(obis_gb, eMrDm, obis_gc, obis_gf,
                                     energy_group_to_tariff(obis_ge),
                                     appl_tbuff))
        {
            mr = (mr_dm_type*)appl_tbuff;

            t32 = mr->cum_mxdm[energy_group_to_dmch_type(obis_gc)];
        }
        else
        {
            t32 = 0L;
        }
        FILL_U32(t32);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 255);  // unitless
    }
}

static void ob_max_demand(void)
{
    uint32_t t32;
    date_time_type dt;
    mr_dm_type* mr;

    DPRINTF(DBG_TRACE, _D "%s: obis_gb[%d]\r\n", __func__, obis_gb);

    switch (appl_att_id)
    {
    case 0x02:
        if (mr_prdnprdseason_from_nv(obis_gb, eMrDm, obis_gc, obis_gf,
                                     energy_group_to_tariff(obis_ge),
                                     appl_tbuff))
        {
            mr = (mr_dm_type*)appl_tbuff;

            t32 = mr->mxdm[energy_group_to_dmch_type(obis_gc)].val;
        }
        else
        {
            t32 = 0L;
        }
        FILL_U32(t32);
        break;
    case 0x03:
        fill_register_scale(0, 255);  // unitless
        break;
    case 0x04:  // status
        FILL_U08(0);
        break;
    case 0x05:
        if (mr_prdnprdseason_from_nv(obis_gb, eMrDm, obis_gc, obis_gf,
                                     energy_group_to_tariff(obis_ge),
                                     appl_tbuff))
        {
            mr = (mr_dm_type*)appl_tbuff;

            t32 = mr->mxdm[energy_group_to_dmch_type(obis_gc)].val;
            dt = mr->mxdm[energy_group_to_dmch_type(obis_gc)].dt;
        }
        else
        {
            t32 = 0L;
        }

        if (t32)
        {
            fill_clock_obj(&dt);
        }
        else
        {
            fill_unspecified_clock();
        }
        break;
    }
}

static void ob_rate_energy(void)
{
    uint32_t t32;
    mr_ch_type* mr;

    DPRINTF(DBG_TRACE, _D "%s: obis_gb[%d]\r\n", __func__, obis_gb);

    if (appl_att_id == 0x02)
    {
        if (mr_prdnprdseason_from_nv(obis_gb, eMrAccm, obis_gc, obis_gf,
                                     energy_group_to_tariff(obis_ge),
                                     appl_tbuff))
        {
            mr = (mr_ch_type*)appl_tbuff;
            t32 = mr->ch[energy_group_to_ch_type(obis_gc)];
        }
        else
        {
            t32 = 0L;
        }
        FILL_U32(t32);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 255);  // unitless
    }
}

static void ob_rate_both_react_energy(energy_dir_type dir)
{
    uint32_t t32;
    mr_ch_type* mr;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        if (mr_from_nv(eMrAccm, obis_gc, obis_gf,
                       energy_group_to_tariff(obis_ge), appl_tbuff))
        {
            mr = (mr_ch_type*)appl_tbuff;
            if (dir == eDeliAct)
            {
                t32 = mr->ch[eChDLagReact] + mr->ch[eChDLeadReact];
                if (t32 >= mxdm_dgt_cnt)
                    t32 -= mxdm_dgt_cnt;
            }
            else
            {
                t32 = mr->ch[eChRLagReact] + mr->ch[eChRLeadReact];
                if (t32 >= mxdm_dgt_cnt)
                    t32 -= mxdm_dgt_cnt;
            }
        }
        else
        {
            t32 = 0L;
        }

        FILL_U32(t32);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 255);  // unitless
    }
}

static void ob_working_fault_min(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        FILL_U08(working_fault_min);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 6);  // scaler(0), unit(minute)
    }
}

static void ob_tou_id_change_sts(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        FILL_ENUM(tou_id_change_sts);
    }
}

static void ob_sys_title_server(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        uint8_t sts_buf[SYS_TITLE_LEN], i;

        for (i = 0; i < SYS_TITLE_LEN; i++)
        {
            sts_buf[i] = SYS_TITLE_server[i];
        }

        fill_octet_string_x(sts_buf, SYS_TITLE_LEN);
    }
}

#include "kepco_cert.h"
extern kepco_cert_storage_t kcs;

static void ob_inst_cert(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        int ret = get_kepco_certs(&kcs.kepco_cert);

        if (ret != 0 || kcs.kepco_cert.sign_cert_len == 0 ||
            kcs.kepco_cert.sign_cert_len > sizeof(kcs.kepco_cert.sign_cert))
        {
            DPRINTF(DBG_WARN, _D "%s: cert unavailable ret[%d] len[%lu]\r\n",
                    __func__, ret, (unsigned long)kcs.kepco_cert.sign_cert_len);
            FILL_STRING(0);
            return;
        }

        uint16_t cert_len = kcs.kepco_cert.sign_cert_len;
        FILL_STRING_2(cert_len);
        memcpy(&pPdu[pPdu_idx], kcs.kepco_cert.sign_cert, cert_len);
        pPdu_idx += cert_len;
    }
}

static void ob_inst_key(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        int ret = get_kepco_certs(&kcs.kepco_cert);
        uint8_t key_len = (uint8_t)sizeof(kcs.kepco_cert.sign_prikey);

        if (ret != 0)
        {
            DPRINTF(DBG_WARN, _D "%s: key unavailable ret[%d]\r\n", __func__,
                    ret);
            FILL_STRING(0);
            return;
        }

        FILL_STRING(key_len);
        memcpy(&pPdu[pPdu_idx], kcs.kepco_cert.sign_prikey, key_len);
        pPdu_idx += key_len;
    }
}

static void ob_last_pgm_chg(void)
{
    program_info_type* proginfo;

    proginfo = (program_info_type*)appl_tbuff;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (act_is_curr_prog_cmd())
        nv_sub_info.ch[0] = 0;  // 현재 프로그램
    else
        nv_sub_info.ch[0] = 1;  // 에약 프로그램
    if (nv_read(I_PROG_INFO, (uint8_t*)proginfo))
    {
        if (proginfo->cnt)
        {
            fill_clock_obj(&proginfo->dt);
            return;
        }
    }

    fill_unspecified_clock();
}

static void ob_fut_pgm_chg(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (prog_fut_is_available())
    {
        fill_clock_obj(&fut_prog_work_time);
    }
    else
    {
        fill_unspecified_clock();
    }
}

static void ob_pgm_chg_num(void)
{
    program_info_type* proginfo;

    proginfo = (program_info_type*)appl_tbuff;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    nv_sub_info.ch[0] = 0;  // 현재 프로그램
    if (!nv_read(I_PROG_INFO, (uint8_t*)proginfo))
    {
        memset((uint8_t*)proginfo, 0x00, sizeof(program_info_type));
    }

    FILL_U16(proginfo->cnt);
}

static void ob_last15_pf(energy_dir_type dir)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        FILL_FLOAT(lp_intv_pf_cap[dir]);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 255);  // unit: unitless
    }
}

static void ob_curr_last_demand(void)
{
    uint8_t* tptr;
    uint16_t t16;
    uint32_t t32;
    prog_dl_type* progdl;
    recent_demand_type* rcnt;

    rcnt = (recent_demand_type*)appl_tbuff;
    tptr = (uint8_t*)rcnt + sizeof(recent_demand_type);

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 2 || appl_att_id == 7)
    {
        if (!nv_read(I_CUR_DM_CAP, (uint8_t*)rcnt))
        {
            memset((uint8_t*)rcnt, 0, sizeof(recent_demand_type));
            memset((uint8_t*)&rcnt->dt, 0xff, sizeof(date_time_type));
        }
    }
    else if (appl_att_id == 3 || appl_att_id == 6)
    {
        if (!nv_read(I_RCNT_DM_CAP, (uint8_t*)rcnt))
        {
            memset((uint8_t*)rcnt, 0, sizeof(recent_demand_type));
            memset((uint8_t*)&rcnt->dt, 0xff, sizeof(date_time_type));
        }
    }
    else
    {
        rcnt = (recent_demand_type*)appl_tbuff;
    }

    switch (appl_att_id)
    {
    case 0x02:  // [current_average_value]
    case 0x03:  // [last_average_value]
        t32 = rcnt->dm[energy_group_to_dmch_type(obis_gc)];
        FILL_U32(t32);
        break;
    case 0x04:                        // [scaler_unit]
        fill_register_scale(0, 255);  // unitless
        break;
    case 0x05:  // [status]
        FILL_U08(1);
        break;
    case 0x06:  // [capture_time]
    case 0x07:  // [start_time_current]
        fill_clock_obj(&rcnt->dt);
        break;
    case 0x08:  // [period]
        if (act_is_curr_prog_cmd())
        {
            t32 = dm_sub_interval * 60;
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_DM_PRD, tptr))
            {
                progdl = (prog_dl_type*)tptr;
                t32 = (progdl->dm_prd) * 60;
            }
            else
            {
                t32 = dm_sub_interval * 60;
            }
        }
        FILL_U32(t32);
        break;
    case 0x09:  // [number_of_periods]
        if (act_is_curr_prog_cmd())
        {
            t16 = (uint16_t)dm_period_num;
        }
        else
        {
            if (fut_pgm_read_set_bits(SETBITS_DM_PRD_NUM, tptr))
            {
                progdl = (prog_dl_type*)tptr;
                t16 = (uint16_t)progdl->dm_prd_num;
            }
            else
            {
                t16 = (uint16_t)dm_period_num;
            }
        }
        FILL_U16(t16);
        break;
    }
}

static uint16_t get_log_cnt(elog_kind_type elog, uint8_t* tptr)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (elog == eLogMtInit)
        return get_mtinit_log_cnt(tptr);

    return log_cnt[elog];
}

static void ob_log_cnt(elog_kind_type elog)
{
    /* 통신 규격 3.4.2.6.2 각 이력 기록의 발생 횟수 */
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    t16 = get_log_cnt(elog, appl_tbuff);

    if (elog == eLogSCurrLimit || elog == eLogCoverOpen ||
        elog == eLogTCoverOpen || elog == eLogrLoadCtrl ||
        elog == eLogSCurrNonSel || elog == eLogMtInit ||
        elog == eLogWrongConn || elog == eLogErrDiagonist ||
        elog == eLogSysSwUp || elog == eLogMtrSwUp || elog == eLogInModemUp ||
        elog == eLogExModemUp)
    {
        FILL_U08((uint8_t)t16);
    }
    else
    {
        FILL_U16(t16);
    }
}

static void ob_sCURR_autortn_val(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_U08(scurr_autortn_cnt);
}

static void ob_sCURR_limit(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        FILL_U16(scurr_limit_level);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 27);  // scaler:0  unit: W
    }
}

static void ob_sCURR_limit_2(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        FILL_U16(scurr_limit_level_2);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 27);  // scaler:0  unit: W
    }
}

static void ob_temp_thrshld(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        FILL_S08(temp_thrshld);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale(0, 9);  // unit: temperature
    }
}

static void ob_temp_over(void)
{
    tempover_log_type tover;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x02:
        if (nv_read(I_TOVER_LOG, (uint8_t*)&tover))
        {
            FILL_S08(tover.val);
        }
        else
        {
            FILL_S08(0);  // no meaning => over datetime is unspecified
        }
        break;
    case 0x03:
        fill_register_scale(0, 9);  // unit: temperature
        break;
    case 0x04:  // status
        FILL_U08(0);
        break;
    case 0x05:
        if (nv_read(I_TOVER_LOG, (uint8_t*)&tover))
        {
            fill_clock_obj(&tover.dt);
        }
        else
        {
            fill_unspecified_clock();
        }
        break;
    }
}

static void ob_sag_swell(uint8_t kind)
{
    /*
    Sag: 순간적인 전압 강하
    Swell: 순간적인 전압 상승
    */
    uint8_t val;
    float fval;

    ST_MTP_SAGSWELL st_mtp_sagswell;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (!nv_read(I_MTP_SAG_SWELL, (uint8_t*)&st_mtp_sagswell))
    {
        dsm_mtp_default_sagswell(&st_mtp_sagswell);
    }

    if (appl_att_id == 0x02)
    {
        switch (kind)
        {
        case 1:  // sag level

            ToHFloat((U8_Float*)&fval, &st_mtp_sagswell.val.sag_level[0]);
            val = (uint8_t)(100.0 * fval / 220.0);
            FILL_U08(val);
            break;

        case 2:  // sag time
            val = st_mtp_sagswell.val.sag_time;
            fval = (float)val;
            FILL_FLOAT(fval);
            break;

        case 3:  // swell level
            ToHFloat((U8_Float*)&fval, &st_mtp_sagswell.val.swell_level[0]);
            val = (uint8_t)(100.0 * fval / 220.0);
            FILL_U08(val);
            break;

        case 4:  // swell time
            val = st_mtp_sagswell.val.swell_time;
            fval = (float)val;
            FILL_FLOAT(fval);
            break;
        }
    }
    else if (appl_att_id == 0x03)
    {
        switch (kind)
        {
        case 1:                          // sag level
        case 3:                          // swell level
            fill_register_scale(0, 56);  // unit: percentage
            break;
        case 2:                           // sag time
        case 4:                           // swell time
            fill_register_scale(0, 255);  // unit: unitless
            break;
        }
    }
}

static void ob_inst_profile(void)
{
    uint8_t t8;
    uint32_t t32;
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x02:
        FILL_ARRAY(1);

        if (appl_is_sap_private())
        {
            cal_data_type cal;
            ////JP.KIM 24.11.08	1) 생산프로그램 PROTOCOL을 dlms 방식으로
            /// 변경

            nv_read(I_CAL_DATA, (U8*)&cal);
            DPRINTF(DBG_TRACE,
                    "CAL NV read R: cur_gain[0x%08X], vol_gain[0x%08X], "
                    "phase_gain[0x%08X]\r\n",
                    cal.T_cal_i0, cal.T_cal_v0, cal.T_cal_p0);
            DPRINTF(DBG_TRACE,
                    "CAL NV read S: cur_gain[0x%08X], vol_gain[0x%08X], "
                    "phase_gain[0x%08X]\r\n",
                    cal.T_cal_i1, cal.T_cal_v1, cal.T_cal_p1);
            DPRINTF(DBG_TRACE,
                    "CAL NV read T: cur_gain[0x%08X], vol_gain[0x%08X], "
                    "phase_gain[0x%08X]\r\n",
                    cal.T_cal_i2, cal.T_cal_v2, cal.T_cal_p2);

            ////JP.KIM 24.11.08	1) 생산프로그램 PROTOCOL을 dlms 방식으로
            /// 변경
            FILL_ARRAY(3);
            FILL_STRUCT(0x03);
            FILL_U32(cal.T_cal_v0);
            FILL_U32(cal.T_cal_v1);
            FILL_U32(cal.T_cal_v2);

            FILL_STRUCT(0x03);
            FILL_U32(cal.T_cal_i0);
            FILL_U32(cal.T_cal_i1);
            FILL_U32(cal.T_cal_i2);

            FILL_STRUCT(0x03);
            FILL_U32(cal.T_cal_p0);
            FILL_U32(cal.T_cal_p1);
            FILL_U32(cal.T_cal_p2);
        }
        else
        {
/* bccho, 2024-09-05, 삼상 */
#if PHASE_NUM == SINGLE_PHASE
            FILL_STRUCT(0x09);

            fill_clock_obj(&cur_rtc);  // clock

            fval = get_inst_power(true, 0);
            FILL_FLOAT(fval);  // power
            fval = get_inst_volt(0);
            FILL_FLOAT(fval);             // vrms(0)
            fval = get_inst_volt_THD(0);  // thd(0)
            FILL_FLOAT(fval);
            fval = get_inst_curr(0);
            FILL_FLOAT(fval);  // irms(0)
            fval = get_inst_pf(0);
            FILL_FLOAT(fval);  // pf(0)
            fval = get_inst_phase(0);
            FILL_FLOAT(fval);  // phase(0)

            fval = get_inst_freq();
            FILL_FLOAT(fval);  // frequency
            fval = (float)get_inst_temp();
            FILL_FLOAT(fval);  // temperature
#else
            FILL_STRUCT(27);

            fill_clock_obj(&cur_rtc);  // clock

            for (int i = 0; i < PHASE_NUM; i++)
            {
                fval = get_inst_power(true, i);
                FILL_FLOAT(fval);  // power
                fval = get_inst_volt(i);
                FILL_FLOAT(fval);             // vrms(0)
                fval = get_inst_volt_THD(i);  // thd(0)
                FILL_FLOAT(fval);
                fval = get_inst_curr(i);
                FILL_FLOAT(fval);  // irms(0)
                fval = get_inst_pf(i);
                FILL_FLOAT(fval);  // pf(0)
                fval = get_inst_phase(i);
                FILL_FLOAT(fval);  // phase(0)

                fval = 0.0;
                FILL_FLOAT(fval);  // 비오차율 = 0
            }

            fval = 0.0;
            FILL_FLOAT(fval);  // N상 = 0

            fval = get_inst_vphase(0);  // 선간 위상
            FILL_FLOAT(fval);

            fval = get_inst_vphase(1);  // 선간 위상
            FILL_FLOAT(fval);

            fval = get_inst_freq();
            FILL_FLOAT(fval);  // frequency
            fval = (float)get_inst_temp();
            FILL_FLOAT(fval);  // temperature
#endif
        }
        break;

    case 0x03:
        if (mt_is_onephase())
        {
            memcpy(&pPdu[pPdu_idx], &INST_PROF_capture_objects[0],
                   INSTPROF_CAPOBJ_SIZE);
            pPdu_idx += INSTPROF_CAPOBJ_SIZE;
        }
        else
        {
            memcpy(&pPdu[pPdu_idx], &INST_PROF_3PHS_capture_objects[0],
                   INSTPROF_3PHS_CAPOBJ_SIZE);
            pPdu_idx += INSTPROF_3PHS_CAPOBJ_SIZE;
        }
        break;

    case 0x04:
        t32 = 0;
        FILL_U32(t32);  // 0 means No automatic capture
        break;

    case 0x05:
        t8 = 1;
        FILL_ENUM(t8);  // sort method(FIFO)
        break;

    case 0x06:
        memcpy(&pPdu[pPdu_idx], &LP_sort_object[0], LP_SORTOBJ_SIZE);
        pPdu_idx += LP_SORTOBJ_SIZE;
        break;

    case 0x07:
        t32 = 1;
        FILL_U32(t32);
        break;

    case 0x08:
        t32 = 1;
        FILL_U32(t32);
        break;
    }
}

static void ob_inst_power(U8 line)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        {
            fval = get_inst_power(true, line);
            FILL_FLOAT(fval);
        }
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((S8)0, 27);  // unit: W
    }
}

static void ob_inst_freq(void)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = get_inst_freq();
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 44);  // unit: frequency
    }
}

static void ob_inst_curr(uint8_t line)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = get_inst_curr(line);
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 33);  // unit: current
    }
}

static void ob_inst_volt(uint8_t line)
{
    float fval;

    if (appl_att_id == 0x02)
    {
        fval = get_inst_volt(line);
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 35);  // unit: voltage
    }
}

static void ob_inst_pf(uint8_t line)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = get_inst_pf(line);
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 56);  // unit: percentage
    }
}

static void ob_inst_phase(uint8_t line)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = get_inst_phase(line);
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 8);  // unit: degree
    }
}

static void ob_inst_volt_THD(uint8_t line)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = get_inst_volt_THD(line);
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 56);  // unit: percentage
    }
}

static void ob_inst_vphase(uint8_t sel)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = get_inst_vphase(sel);
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 8);  // unit: degree
    }
}

static void ob_curr_temp(void)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        if (appl_is_sap_private())
        {
            fval = get_inst_temp() + temp_caled;
        }
        else
        {
            fval = (float)get_inst_temp();
        }
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 9);  // unit: temperature
    }
}

static void ob_avg_curr(uint8_t line)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = get_lpavg_i(line);
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 33);  // unit: current
    }
}

static void ob_avg_volt(uint8_t line)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = get_lpavg_v(line);
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 35);  // unit: voltage
    }
}

static void ob_avg_volt_ltol(U8 line)
{
    float fval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        fval = get_lpavg_ltol(line);
        FILL_FLOAT(fval);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((S8)0, 35);  // unit: voltage
    }
}

static void ob_imax_log(uint8_t line)
{
    float fval;
    imax_log_type imax;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 2 || appl_att_id == 5)
    {
        nv_sub_info.ch[0] = line;
        if (!nv_read(I_IMAX_LOG, (uint8_t*)&imax))
        {
            memset((uint8_t*)&imax, 0, sizeof(imax_log_type));
        }
    }

    switch (appl_att_id)
    {
    case 0x02:
        if (imax.dt.month != 0)
        {
            fval = imax.imaxint / 1000;
            fval += (float)((imax.imaxint % 1000) * 0.001);
            FILL_FLOAT(fval);
        }
        else
        {
            fval = 0.0;
            FILL_FLOAT(fval);
        }
        break;
    case 0x03:
        fill_register_scale(0, 33);  // scaler:0  unit: current
        break;
    case 0x04:  // status
        FILL_U08(0);
        break;
    case 0x05:
        if (imax.imaxint && (imax.dt.month != 0))
            fill_clock_obj(&imax.dt);
        else
            fill_unspecified_clock();
        break;
    }
}

static void ob_prepay_remenergy(void)
{
    int32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        t32 = get_prepay_info(2);
        // pulse 수 -> wh
        t32 = (t32 * 1000) / PulseKwh;
        FILL_S32(t32);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)-3, 255);  // unitless
    }
}

static void ob_prepay_buyenergy(void)
{
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        t32 = get_prepay_info(0);
        // pulse 수 -> wh
        t32 = (t32 * 1000) / PulseKwh;
        FILL_U32(t32);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)-3, 255);  // unitless
    }
}

static void ob_prepay_enable(void)
{
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        t32 = get_prepay_info(5);
        FILL_BOOL((bool)t32);
    }
}

static void ob_prepay_loadlimit_cancel(void)
{
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        t32 = get_prepay_info(6);
        FILL_BOOL((bool)t32);
    }
}

static void ob_condensor_inst(void)
{
    uint8_t conds;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    conds = (condensor_en == 1) ? 0 : 1;
    FILL_BOOL(conds);
}

static void ob_ts_conf(void)
{
    uint8_t i, t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        memcpy(&pPdu[pPdu_idx], &ts_conf_ctrl_data[0], TS_CONF_CTRL_LEN);
        pPdu_idx += TS_CONF_CTRL_LEN;

        // ts_conf is not captured ????
        if (ts_conf_available())
        {
            pPdu[pPdu_idx - 1] = ts_conf_ctrl;
        }
    }
    else if (appl_att_id == 0x03)
    {
        FILL_ENUM(5);  // execution time = n(8), all time value may
                       // be different, wildcards in date is allowed
    }
    else if (appl_att_id == 0x04)
    {
        // ts_conf is not captured ????
        if (ts_conf_available())
        {
            t8 = (ts_conf_cnt > TS_ZONE_SIZE)
                     ? TS_ZONE_SIZE
                     : ts_conf_cnt;  // to prevent memory corruption
            FILL_ARRAY(t8);
            for (i = 0; i < t8; i++)
            {
                memcpy(&pPdu[pPdu_idx], &ts_conf_zone_data[0],
                       TS_CONF_ZONE_LEN);

                pPdu[pPdu_idx + 4] = ts_conf_zone[i].hour;
                pPdu[pPdu_idx + 5] = ts_conf_zone[i].min;

                pPdu_idx += TS_CONF_ZONE_LEN;
            }
        }
        else
        {
            FILL_ARRAY(0);
        }
    }
}

static void ob_sel_react(void)
{
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_STRING(4);

    if (sel_react_yr != 0x00 && sel_react_mon != 0x00)
    {
        t16 = sel_react_yr + BASE_YEAR;
        FILL_V16(t16);
        FILL_V8(sel_react_mon);
        FILL_V8(sel_react_sel);
    }
    else
    {
        t16 = 0x0000;
        FILL_V16(t16);
        FILL_V8(0x00);
        FILL_V8(0);
    }
}

static void ob_lp_overlaped_index(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    uint32_t t32;
    t32 = TOTAL_LP_EVENT_CNT;
    FILL_U32(t32);
}

static void ob_comm_enable(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (comm_en_coveropen)
    {
        FILL_BOOL(1);
    }
    else
    {
        FILL_BOOL(0);
    }
}

static void ob_key_value(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_U08(0);  // no meaning
}

static void ob_lcd_map(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    dsp_fill_lcd_dot_comm();

    fill_octet_string_x(lcd_map_comm, LCD_MAP_SIZE);
}

static void ob_scurr_hold(void)
{
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        t32 = (uint32_t)scurr_det_hold;
        FILL_U32(t32);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 7);  // unit: sec
    }
}

static void ob_scurr_rtn_n1(void)
{
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        t32 = (uint32_t)scurr_rtn_dur_1;
        FILL_U32(t32);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 7);  // unit: sec
    }
}

static void ob_scurr_rtn_n2(void)
{
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        t32 = (uint32_t)scurr_rtn_dur_2;
        FILL_U32(t32);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 7);  // unit: sec
    }
}

static void ob_scurr_cnt_n1(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_U08(scurr_cnt_n1);
}

static void ob_latchon_counter(void)
{
    uint16_t _cnt;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    _cnt = get_latchon_cnt();

    FILL_U16(_cnt);
}

static void ob_min_freq(void)
{
    min_max_data_type mval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 2:
        FILL_FLOAT(min_freq);
        break;

    case 3:
        fill_register_scale((int8_t)0, 44);  // unit: Hz
        break;

    case 4:  // status
        FILL_U08(0);
        break;

    case 5:
        if (nv_read(I_MIN_FREQ, (uint8_t*)&mval))
        {
            fill_clock_obj(&mval._dt);
        }
        else
        {
            fill_unspecified_clock();
        }
        break;
    }
}

static void ob_max_freq(void)
{
    min_max_data_type mval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 2:
        FILL_FLOAT(max_freq);
        break;

    case 3:
        fill_register_scale((int8_t)0, 44);  // unit: Hz
        break;

    case 4:  // status
        FILL_U08(0);
        break;

    case 5:
        if (nv_read(I_MAX_FREQ, (uint8_t*)&mval))
        {
            fill_clock_obj(&mval._dt);
        }
        else
        {
            fill_unspecified_clock();
        }
        break;
    }
}

#if 1 /* bccho, 2024-09-05, 삼상 */
static void ob_min_volt(uint8_t idx)
{
    min_max_data_type mval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 2:
        /* bccho, 2024-09-05, 삼상 */
        FILL_FLOAT(min_volt[idx]);
        break;

    case 3:
        fill_register_scale((int8_t)0, 35);  // unit: volt
        break;

    case 4:  // status
        FILL_U08(0);
        break;

    case 5:
        /* bccho, 2024-09-05, 삼상 */
        if (idx == 1)
        {
            if (nv_read(I_MIN_VOLT_L2, (uint8_t*)&mval))
            {
                fill_clock_obj(&mval._dt);
            }
            else
            {
                fill_unspecified_clock();
            }
        }
        else if (idx == 2)
        {
            if (nv_read(I_MIN_VOLT_L3, (uint8_t*)&mval))
            {
                fill_clock_obj(&mval._dt);
            }
            else
            {
                fill_unspecified_clock();
            }
        }
        else
        {
            if (nv_read(I_MIN_VOLT_L1, (uint8_t*)&mval))
            {
                fill_clock_obj(&mval._dt);
            }
            else
            {
                fill_unspecified_clock();
            }
        }
        break;
    }
}

/* bccho, 2024-09-05, 삼상 */
static void ob_max_volt(uint8_t idx)
{
    min_max_data_type mval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 2:
        /* bccho, 2024-09-05, 삼상 */
        FILL_FLOAT(max_volt[idx]);
        break;

    case 3:
        fill_register_scale((int8_t)0, 35);  // unit: volt
        break;

    case 4:  // status
        FILL_U08(0);
        break;

    case 5:
        /* bccho, 2024-09-05, 삼상 */
        if (idx == 1)
        {
            if (nv_read(I_MAX_VOLT_L2, (uint8_t*)&mval))
            {
                fill_clock_obj(&mval._dt);
            }
            else
            {
                fill_unspecified_clock();
            }
        }
        else if (idx == 2)
        {
            if (nv_read(I_MAX_VOLT_L3, (uint8_t*)&mval))
            {
                fill_clock_obj(&mval._dt);
            }
            else
            {
                fill_unspecified_clock();
            }
        }
        else
        {
            if (nv_read(I_MAX_VOLT_L1, (uint8_t*)&mval))
            {
                fill_clock_obj(&mval._dt);
            }
            else
            {
                fill_unspecified_clock();
            }
        }
        break;
    }
}
#else
static void ob_min_volt(void)
{
    min_max_data_type mval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 2:
        FILL_FLOAT(min_volt);
        break;

    case 3:
        fill_register_scale((int8_t)0, 35);  // unit: volt
        break;

    case 4:  // status
        FILL_U08(0);
        break;

    case 5:
        if (nv_read(I_MIN_VOLT, (uint8_t*)&mval))
        {
            fill_clock_obj(&mval._dt);
        }
        else
        {
            fill_unspecified_clock();
        }
        break;
    }
}

static void ob_max_volt(void)
{
    min_max_data_type mval;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 2:
        FILL_FLOAT(max_volt);
        break;

    case 3:
        fill_register_scale((int8_t)0, 35);  // unit: volt
        break;

    case 4:  // status
        FILL_U08(0);
        break;

    case 5:
        if (nv_read(I_MAX_VOLT, (uint8_t*)&mval))
        {
            fill_clock_obj(&mval._dt);
        }
        else
        {
            fill_unspecified_clock();
        }
        break;
    }
}
#endif

static void ob_overcurr_enable(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (overcurr_cut_en)
    {
        FILL_BOOL(0);
    }
    else
    {
        FILL_BOOL(1);
    }
}

static void ob_tou_set_cnt(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    FILL_STRUCT(7);

    // 1. extended tou id
    fill_octet_string_x(tou_set_cnt.ext_tou_id, EXT_TOU_ID_SIZE);
    // 2. cosem object count
    FILL_U16(tou_set_cnt.cosem_cnt);
    // 3. last class id
    FILL_U16(tou_set_cnt.last_classid);
    // 4. last obis code
    fill_octet_string_x(tou_set_cnt.last_obis, OBIS_ID_SIZE);
    // 5. last obis attribute id
    FILL_U08(tou_set_cnt.last_attid);
    // 6. last array entry index
    FILL_U16(tou_set_cnt.last_arrayidx);
    // 7. TOU CRC
    FILL_U16(tou_set_cnt.tou_crc);
}

static void ob_ext_prog_id(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    fill_octet_string_x(tou_set_cnt.ext_tou_id, EXT_TOU_ID_SIZE);
}

static void ob_realtime_p_energy(void)
{
    float w, var;
    float ch_2[4 * 4] = {
        0.0,
    };
    uint8_t i;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:
        if (mt_is_onephase())
        {
            FILL_STRUCT(4);

            w = w0sum_mon;
            if (w >= 0.0)
            {
                ch_2[4] = w;
            }
            else
            {
                ch_2[5] = fabs(w);
            }
            var = var0sum_mon;
            if (var >= 0.0)
            {
                ch_2[6] = var;
            }
            else
            {
                ch_2[7] = fabs(var);
            }

            FILL_FLOAT(ch_2[4]);
            FILL_FLOAT(ch_2[5]);
            FILL_FLOAT(ch_2[6]);
            FILL_FLOAT(ch_2[7]);
        }
        else
        {
            FILL_STRUCT((4 * 4));

            w = w0sum_mon;
            if (w >= 0.0)
            {
                ch_2[4] = w;
            }
            else
            {
                ch_2[5] = fabs(w);
            }
            var = var0sum_mon;
            if (var >= 0.0)
            {
                ch_2[6] = var;
            }
            else
            {
                ch_2[7] = fabs(var);
            }

            w = w1sum_mon;
            if (w >= 0.0)
            {
                ch_2[0 + 4 * 2] = w;
            }
            else
            {
                ch_2[1 + 4 * 2] = fabs(w);
            }
            var = var1sum_mon;
            if (var >= 0.0)
            {
                ch_2[2 + 4 * 2] = var;
            }
            else
            {
                ch_2[3 + 4 * 2] = fabs(var);
            }

            w = w2sum_mon;
            if (w >= 0.0)
            {
                ch_2[0 + 4 * 3] = w;
            }
            else
            {
                ch_2[1 + 4 * 3] = fabs(w);
            }
            var = var2sum_mon;
            if (var >= 0.0)
            {
                ch_2[2 + 4 * 3] = var;
            }
            else
            {
                ch_2[3 + 4 * 3] = fabs(var);
            }

            // 상별 전체 더 하기
            for (i = 0; i < 4; i++)
            {
                ch_2[i] = ch_2[i + 4] + ch_2[i + 4 * 2] + ch_2[i + 4 * 3];
            }

            FILL_FLOAT(ch_2[0]);
            FILL_FLOAT(ch_2[1]);
            FILL_FLOAT(ch_2[2]);
            FILL_FLOAT(ch_2[3]);
            FILL_FLOAT(ch_2[4]);
            FILL_FLOAT(ch_2[5]);
            FILL_FLOAT(ch_2[6]);
            FILL_FLOAT(ch_2[7]);
            FILL_FLOAT(ch_2[8]);
            FILL_FLOAT(ch_2[9]);
            FILL_FLOAT(ch_2[10]);
            FILL_FLOAT(ch_2[11]);
            FILL_FLOAT(ch_2[12]);
            FILL_FLOAT(ch_2[13]);
            FILL_FLOAT(ch_2[14]);
            FILL_FLOAT(ch_2[15]);
        }

        break;
    }
}

static void ob_realtime_p_load_profile(void)
{
    int i;
    uint32_t entry_from, entry_to;
    uint16_t selcol_from, selcol_to;
    uint16_t t16;
    uint32_t t32;

    DPRINTF(DBG_INFO, _D "%s: lprt__index[%d]\r\n", __func__, lprt__index);

    switch (appl_att_id)
    {
    case 0x02:
        if (lprt__index == 0L)
        {
            getresp_LP_len = 0;
        }
        else
        {
            if (appl_selacs_len == 0)
            {
                // non-selective
                getresp_LP_index = lprt__index - 1;

                /* bccho, 2024-09-05, 삼상 */
                // if (mt_is_onephase())
                getresp_LP_len = (lprt__index <= LPRT_SIZE)
                                     ? (uint16_t)lprt__index
                                     : LPRT_SIZE;

                // LP column information
                get_LPrt_selcolmn_info(1,
                                       0);  // full selective column
            }
            else
            {
                // skip appl_selacs_len and tag check
                i = 0;
                if (appl_selacs[i++] == 2)
                {            // support only entry range selection
                    i += 2;  // struct_tag(2B, tag+val)
                    i += 1;  // tag
                    ToH32((U8_16_32*)&entry_from, &appl_selacs[i]);
                    i += 4;
                    i += 1;  // tag
                    ToH32((U8_16_32*)&entry_to, &appl_selacs[i]);
                    i += 4;
                    i += 1;  // tag
                    ToH16((U8_16*)&selcol_from, &appl_selacs[i]);
                    i += 2;
                    i += 1;  // tag
                    ToH16((U8_16*)&selcol_to, &appl_selacs[i]);
                    i += 2;

                    // length
                    t16 = (lprt__index <= LPRT_SIZE) ? (uint16_t)lprt__index
                                                     : LPRT_SIZE;
                    if ((entry_from == 0L) || (entry_from > t16))
                    {
                        getresp_LP_len = 0;
                    }
                    else
                    {
                        getresp_LP_index = lprt__index - entry_from;
                        if (entry_to == 0)
                        {
                            getresp_LP_len = t16;
                        }
                        else
                        {
                            getresp_LP_len =
                                (uint16_t)(entry_to - entry_from + 1);
                        }
                        get_LPrt_selcolmn_info(
                            (uint8_t)selcol_from,
                            (uint8_t)selcol_to);  // selective column
                    }
                }
                else
                {
                    getresp_LP_len = 0;
                }
            }
        }

        if (getresp_LP_len == 0)
        {
            FILL_ARRAY(0);
        }
        else
        {
            approc_fill_get_resp_block(true);
        }
        break;

    case 0x03:
        memcpy(&pPdu[pPdu_idx], &LPrt_capture_objects[0], LPrt_CAP_SIZE);
        pPdu_idx += LPrt_CAP_SIZE;
        break;

    case 0x04:
        t32 = rt_lp_interval;  // unit = sec
        FILL_U32(t32);
        break;

    case 0x05:
        FILL_ENUM(1);  // sort method(FIFO)
        break;

    case 0x06:
        memcpy(&pPdu[pPdu_idx], &LP_sort_object[0], LP_SORTOBJ_SIZE);
        pPdu_idx += LP_SORTOBJ_SIZE;
        break;

    case 0x07:
        t32 = (lprt__index < LPRT_SIZE) ? lprt__index : LPRT_SIZE;
        FILL_U32(t32);
        break;

    case 0x08:
        t32 = LPRT_SIZE;
        FILL_U32(t32);
        break;
    }
}

static void ob_realtime_lp_interval(void)
{
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:
        t16 = rt_lp_interval;
        FILL_U16(t16);

        break;
    case 0x03:
        fill_register_scale(0, 7);  // scaler(0), unit(sec)
        break;
    }
}

static void ob_run_modem_info(void)
{
    uint8_t comm_if = MEDIA_RUN_NONE;
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:
        FILL_BS(8);
        switch (dsm_media_get_fsm_if_hdlc())
        {
        case MEDIA_RUN_RS485:
            comm_if = COMM_IF_RS485;
            break;
        case MEDIA_RUN_CAN:
            comm_if = COMM_IF_CAN;
            break;
        case MEDIA_RUN_SUN:
            comm_if = COMM_IF_INT_SUN;
            break;
        case MEDIA_RUN_EXT:
            switch (dsm_get_bearer())
            {
            case BEARER_LTE:
                comm_if = COMM_IF_LTE;
                break;
            case BEARER_iotPLC:
                comm_if = COMM_IF_IOT_PLC;
                break;
            case BEARER_HPGP:

                break;
            case BEARER_C_SMGW:

                break;
            case BEARER_NONE:
            case BEARER_SUN_MAC:
            case BEARER_SUN_IP:
            default:
                comm_if = COMM_IF_INT_SUN;
                break;
            }

            break;

        default:
            comm_if = COMM_IF_INT_SUN;
            break;
        }
        FILL_V8(comm_if);
        break;
    }
}

static void ob_use_mr_data_num(void)
{
    uint8_t mr_type = 0, t8 = 0;
    DPRINTF(DBG_TRACE, _D "%s: obis_gb[%d]\r\n", __func__, obis_gb);

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:
    {
        mr_type = obis_get_mr_data_type(obis_gb);
        switch (mr_type)
        {
        case eObisGrpB_AmrDataT_PERIOD:
            if (mr_cnt >= MAX_MREADING)
                t8 = MAX_MREADING;
            else
                t8 = (uint8_t)mr_cnt;

            break;
        case eObisGrpB_AmrDataT_nPERIOD:
            if (mrcnt_nprd >= MAX_MREADING_nPRD)
                t8 = MAX_MREADING_nPRD;
            else
                t8 = (uint8_t)mrcnt_nprd;

            break;
        case eObisGrpB_AmrDataT_SEASON_CHG:
            if (mrcnt_season_chg >= MAX_MREADING_SEASON)
                t8 = MAX_MREADING_SEASON;
            else
                t8 = (uint8_t)mrcnt_season_chg;

            break;
        default:

            break;
        }
        DPRINTF(DBG_TRACE, _D "MR_TYPE[%d], mrcnt[%d]\r\n", mr_type, t8);

        FILL_U08(t8);

        break;
    }
    }
}

static void ob_nms_dms_id(void)
{
    /*
    기설 모뎀 연계용 미터 ID(7byte) : byte 4, 5번의 “00”을 ASCII로
    표기 구형 모뎀 연계용 미터 ID
    */
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    get_nms_dms_id(appl_tbuff);
    fill_octet_string_x(appl_tbuff, NMS_DMS_ID_SIZE);
}

static void ob_sap_assignment(void)
{
    uint8_t dev_id[16];
    uint16_t t16 = 0;
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 2:
        FILL_ARRAY(SAP_ASSIGN_MNT_NUM);

        FILL_STRUCT(2);
        t16 = SAP_ASSIGN_DEV_MANAGEMENT;
        FILL_U16(t16);
        FILL_STRING(DEVICE_ID_SIZE);
        if (!nv_read(I_DEVICE_ID, (uint8_t*)&dev_id[0]))
        {
            memcpy(dev_id, &logical_device_name_r[0], DEVICE_ID_SIZE);
        }
        memcpy(&pPdu[pPdu_idx], dev_id, DEVICE_ID_SIZE);
        pPdu_idx += DEVICE_ID_SIZE;

        FILL_STRUCT(2);
        t16 = SAP_ASSIGN_KEPCO_MANAGEMENT;
        FILL_U16(t16);
        FILL_STRING(DEVICE_ID_SIZE);
        if (!nv_read(I_DEVICE_ID_KEPCO, (uint8_t*)&dev_id[0]))
        {
            memcpy(dev_id, &logical_device_name_r_kepco[0], DEVICE_ID_SIZE);
        }
        memcpy(&pPdu[pPdu_idx], dev_id, DEVICE_ID_SIZE);
        pPdu_idx += DEVICE_ID_SIZE;

        break;
    }
}

static void ob_zcrs_sig_out_durtime(void)
{
    if (appl_att_id == 0x02)
    {
        FILL_U32(zcrs_sig_out_dur);  // no meaning => just for CTT tool
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 7);  // unit: second
    }
}

static void ob_zcrs_sig_out_cmpstime(void)
{
    U32 t32 = DEFAULT_zcrs_sig_out_cmps;  // jp.kim 24.11.14
    if (appl_att_id == 0x02)
    {
        FILL_U32(t32);  // jp.kim 24.11.14	상수값 사용
                        // // no meaning => just for CTT tool
    }
    else if (appl_att_id == 0x03)
    {
#if 0  // jp.kim 24.11.12
		fill_register_scale((S8)-3, 7);			// unit: ms
#else
        fill_register_scale((S8)-6, 7);  // unit: us
#endif
    }
}

static void ob_zcrs_sig_out_resulttime(void)
{
    if (appl_att_id == 0x02)
    {
        ST_ZCD_RESULT_TIME st_zcd;
        nv_read(I_ZCD_RESULT_TIME, (uint8_t*)&st_zcd);
        FILL_U32(st_zcd.time);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)-6, 7);  // unit: us
    }
}
static void ob_self_error_ref(void)
{
    if (appl_att_id == 0x02)
    {
        FILL_S16(self_error_ref);  // no meaning => just for CTT tool
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 27);  // unit: w
    }
}

void ob_security_setup(void)
{
    uint8_t t8 = 0;
    uint8_t pt8[SUBJ_MAX_SIZE];

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 2:  // security_policy  :: kepco 문의후 적용 하자
        /*default 0*/
        t8 = 0;
        FILL_ENUM(t8);
        break;

    case 3:  // security_suite
        t8 = dsm_sec_get_security_suite();
        FILL_ENUM(t8);
        break;

    case 4:  // client_system_title
        FILL_STRING(SYS_TITLE_LEN);
        memcpy(&pPdu[pPdu_idx], SYS_TITLE_client, SYS_TITLE_LEN);
        pPdu_idx += SYS_TITLE_LEN;
        break;

    case 5:  // server_system_title
        FILL_STRING(SYS_TITLE_LEN);
        memcpy(&pPdu[pPdu_idx], SYS_TITLE_server, SYS_TITLE_LEN);
        pPdu_idx += SYS_TITLE_LEN;
        break;

    case 6:  // certificates
             ///////////////////////////////////
             // 차후 업데이트 및 관련 코드 및 검증 필요 //
             ///////////////////////////////////
    {
        FILL_ARRAY(4);

        for (int i = 0; i < 4; i++)
        {
            uint8_t entity_idx = dsm_sec_certcon_idx_2_entity_idx(i);

            FILL_STRUCT(0x06);
            FILL_ENUM(entity_idx);
            FILL_ENUM(CERT_TYPE_DS);

#if 0  /* bccho, KEYPAIR, 2023-07-15 */
            dsm_sec_get_cert_info(CERT_SN, i, pt8, &t8);
            FILL_STRING(t8);
            memcpy(&pPdu[pPdu_idx], pt8, t8);
            pPdu_idx += t8;

            dsm_sec_get_cert_info(CERT_ISSUER_DN, i, pt8, &t8);
            FILL_STRING(t8);
            memcpy(&pPdu[pPdu_idx], pt8, t8);
            pPdu_idx += t8;

            dsm_sec_get_cert_info(CERT_SUBJ_DN, i, pt8, &t8);
            FILL_STRING(t8);
            memcpy(&pPdu[pPdu_idx], pt8, t8);
            pPdu_idx += t8;
#endif /* bccho */
            FILL_STRING(0);
        }
    }
    break;
    }
}

static uint16_t get_cert_log_cnt(elog_cert_kind_type elog, uint8_t* tptr)
{
    DPRINTF(DBG_TRACE, _D "%s: elog[%d]\r\n", __func__, elog);

    return log_cert_cnt[elog];
}

#if 1  // jp.kim 25.12.03  보안로그 개별 obis 읽기 fail 종류 추가  //test
       // 필요함.
void ob_cert_log_case(elog_cert_kind_type elog)
{
    U8* tptr;
    tptr = appl_tbuff;

    U8 idx, cnt;
    U8 t8;
    U16 logcertcnt;

    logcertcnt = get_cert_log_cnt(elog, tptr);

    DPRINTF(DBG_TRACE, _D "%s: elog[%d], logcertcnt[%d]\r\n", __func__, elog,
            logcertcnt);

    if (logcertcnt == 0)
    {
        t8 = 0;
        FILL_U08(t8);
        return;
    }
    idx = (logcertcnt - 1) % LOG_CERT_BUFF_SIZE;
    cnt = (logcertcnt < LOG_CERT_RECORD_LEN) ? logcertcnt : LOG_CERT_RECORD_LEN;

    switch (elog)
    {
    default:
        nv_sub_info.ch[0] = elog;
        if (nv_read(I_LOG_CERT_DATA, tptr))
        {
            log_cert_data_type* logcertdata;
            logcertdata = (log_cert_data_type*)tptr;

            FILL_U08(logcertdata->evt[idx].ng_case);
            DPRINTF(DBG_TRACE,
                    _D "%s: elog[%d], logcertdata->evt[idx].ng_case[%d]\r\n",
                    __func__, elog, logcertdata->evt[idx].ng_case);
        }
    }
}
#endif

void ob_cert_log_cnt(elog_cert_kind_type elog)
{
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    t16 = get_cert_log_cnt(elog, appl_tbuff);

#if 1  // jp.kim 25.12.03  보안로그 개별 obis 읽기 fail 횟수
    // 8bit data
    FILL_U08((U8)t16);
#else
    if (elog == eLogCert_NG)
    {
        FILL_U08((U8)t16);
    }
    else
    {
        FILL_U16(t16);
    }
#endif
}

static void fill_cert_log_data(elog_cert_kind_type elog, uint8_t* tptr)
{
    uint8_t i, idx, cnt;
    uint8_t t8;
    uint16_t logcertcnt, t16;

    logcertcnt = get_cert_log_cnt(elog, tptr);

    DPRINTF(DBG_TRACE, _D "%s: elog[%d], logcertcnt[%d]\r\n", __func__, elog,
            logcertcnt);

    if (logcertcnt == 0)
    {
        FILL_ARRAY(0);
        return;
    }
    idx = (logcertcnt - 1) % LOG_CERT_BUFF_SIZE;
    cnt = (logcertcnt < LOG_CERT_RECORD_LEN) ? logcertcnt : LOG_CERT_RECORD_LEN;

    switch (elog)
    {
    default:
        nv_sub_info.ch[0] = elog;
        if (nv_read(I_LOG_CERT_DATA, tptr))
        {
            log_cert_data_type* logcertdata;
            logcertdata = (log_cert_data_type*)tptr;

            FILL_ARRAY(cnt);
            for (i = 0; i < cnt; i++)
            {
                FILL_STRUCT(0x03);
                fill_clock_obj(&logcertdata->evt[idx].dt);

#if 1  // jp.kim 25.12.03  보안로그 data 순서 수정
                /*
                항목구성[capture_objects]	OBIS 코드[logical_name]
                속성[attribute_index] 발생 일자/시간 0.0.1.0.0.255 2 횟수
                0.128.99.98.20.255 			2 보안 이벤트
                종류 				1.128.128.192.99.255 		2
                */

                // 2 	[value] 	보안 로그 발생 횟수 (unsigned: (17))
                t8 = (U8)(logcertcnt - i);
                FILL_U08(t8);

                /*
                [value]  보안 이벤트 종류 (unsigned: (17))
                value 내용
                0xA1 암호 모듈 초기화 오류
                0xA2 암호 모듈 연동 오류
                0xA3 Z 생성 오류
                0xA4 세션키 생성 오류
                0xA7 전자서명 생성 오류
                0xA8 전자서명 검증 오류
                0xA9 보안 항목 누락
                0xAA 상호인증 실패
                0xFF 기타오류
                */
                FILL_U08(logcertdata->evt[idx].ng_case);

#else
                FILL_U08(logcertdata->evt[idx].ng_case);

                if (elog == eLogCert_NG)
                {
                    t8 = (U8)(logcertcnt - i);
                    FILL_U08(t8);
                }
                else
                {
                    t16 = logcertcnt - i;
                    FILL_U16(t16);
                }
#endif
                if (idx == 0)
                    idx = LOG_CERT_BUFF_SIZE - 1;
                else
                    idx--;
            }
        }
        else
        {
            FILL_ARRAY(0);
        }
        break;
    }
}

static void ob_evt_cert_log(elog_cert_kind_type elog)
{
    uint8_t* tptr;
    uint32_t t32;

    tptr = appl_tbuff;

    DPRINTF(DBG_TRACE, _D "%s, elog[%d]\r\n", __func__, elog);

    switch (appl_att_id)
    {
    case 0x02:
        fill_cert_log_data(elog, tptr);
        break;

    case 0x03:
        fill_cert_log_cap(elog);
        break;

    case 0x04:
        t32 = 0L;
        FILL_U32(t32);
        break;

    case 0x05:
        FILL_ENUM(1);
        break;

    case 0x06:
        memcpy(&pPdu[pPdu_idx], &LP_sort_object[0], LP_SORTOBJ_SIZE);
        pPdu_idx += LP_SORTOBJ_SIZE;
        break;

    case 0x07:
        t32 = (log_cert_cnt[elog] < LOG_CERT_RECORD_LEN)
                  ? (uint32_t)log_cert_cnt[elog]
                  : (uint32_t)LOG_CERT_RECORD_LEN;
        FILL_U32(t32);
        break;

    case 0x08:
        t32 = LOG_CERT_RECORD_LEN;
        FILL_U32(t32);
        break;
    }
}

static void ob_evt_tou_imagetransfer(void)
{
    uint32_t t32 = 0;
    // uint32_t blk_size;

    DPRINTF(DBG_TRACE, _D "%s: Attributes %d\r\n", __func__, appl_att_id);

    // Get Req
    /* 통신 규격 3.4.2.8.22 TOU Image Transfer : Attributes */
    switch (appl_att_id)
    {
    case 0x01:
        /* TBD */
        break;

    case 0x02:
        /* [image_block_size] */
        // TODO: (WD)
        t32 = dsm_imgtrfr_get_blk_size(IMG__TOU);
        FILL_U32(t32);
        break;

    case 0x03:
        /* [image_transferred_block_status] ※ 사용 안 함 */
        break;

    case 0x04:
        /* [image_first_not_transferred_block_number] */
        t32 = dsm_imgtrfr_get_blk_num(IMG__TOU);
        FILL_U32(t32);
        break;

    case 0x05:
        /* [image_transfer_enabled] */
        FILL_BOOL(dsm_imgtrfr_get_transfer_enabled(IMG__TOU));
        break;

    case 0x06:
        /* [image_transfer_status] */
        FILL_ENUM(dsm_imgtrfr_get_transfer_status(IMG__TOU));
        break;

    case 0x07:
        /* [image_to_activate_info] */
        // client 에서 get 하여 activate 여부 결정...
        FILL_ARRAY(1);
        FILL_STRUCT(0x03);
        t32 = dsm_imgtrfr_get_image_size(IMG__TOU);
        FILL_U32(t32);
        FILL_STRING(IMAGE_NAME_MAX_SIZE);
        memcpy(&pPdu[pPdu_idx], dsm_imgtrfr_get_name(IMG__TOU),
               IMAGE_NAME_MAX_SIZE);
        pPdu_idx += IMAGE_NAME_MAX_SIZE;
        FILL_STRING(IMAGE_HASH_SIZE);
        memcpy(&pPdu[pPdu_idx], dsm_imgtrfr_get_hash(IMG__TOU),
               IMAGE_HASH_SIZE);
        pPdu_idx += IMAGE_HASH_SIZE;

        break;
    }
}

#if 0
bool g_modem_exist = false;  // jp.kim 25.01.20
#endif

static void ob_evt_fw_info(void)
{
    uint32_t t32;
    ST_FW_INFO fwinfo = {0};
#if 1
    bool modem_exist = true;  // jp.kim 25.01.20
#endif

    DPRINTF(DBG_TRACE, _D "%s: att_id[%d]\r\n", __func__, appl_att_id);

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:
        /*
            Attributes : 2
            - 현재 적용된 소프트웨어 ID
            소프트웨어 ID ::= structure
            {
                기기 형식: (octet string: (9), 2byte)
                소프트웨어 버전: (octet string: (9), 6byte)
                소프트웨어 생성 일자: (octet string: (9), 6byte)
                소프트웨어 Image Hash: (octet string: (9), 32byte)
            } Image Hash 초기값은 00~00 (32byte)
        */
        t32 = obis_get_fw_info_type_group_b(obis_gb);

        DPRINTF(DBG_TRACE, _D "%s: fw_info_type[%d]\r\n", __func__, t32);

#if 1  // jp.kim 25.01.20
        if (t32 == FWINFO_CUR_E_MODEM)
        {
            if (dsm_atcmd_if_is_valid(MEDIA_RUN_EXT, false) != true)
            {
                modem_exist = false;
#if 0
				goto fw_update_info_err;
#endif
            }
            else
            {
                dsm_atcmd_if_is_valid(MEDIA_RUN_EXT, false);
            }
        }

        if (modem_exist)
#else
        if ((t32 != FWINFO_CUR_E_MODEM) || (g_modem_exist))
#endif
        {
            dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, t32);
            FILL_STRUCT(0x04);
            FILL_STRING(2);
            if (t32 == FWINFO_CUR_MODEM)
            {
#if METER_ID == 53  // JP.KIM 24.10.16
                // pPdu[pPdu_idx++] = '7';
                // pPdu[pPdu_idx++] = '1';
                pPdu[pPdu_idx++] = '5';
                pPdu[pPdu_idx++] = '3';
#elif METER_ID == 54  // JP.KIM 25.02.26
                pPdu[pPdu_idx++] = '5';
                pPdu[pPdu_idx++] = '4';
#elif METER_ID == 55
                pPdu[pPdu_idx++] = '5';
                pPdu[pPdu_idx++] = '5';
#endif
            }
            else
            {
                memcpy(&pPdu[pPdu_idx], fwinfo.mt_type, 2);
                pPdu_idx += 2;
            }

            FILL_STRING(FW_VERSION_SIZE);
            memcpy(&pPdu[pPdu_idx], fwinfo.version, FW_VERSION_SIZE);
            pPdu_idx += FW_VERSION_SIZE;

            FILL_STRING(FW_GENERATION_DATE_SIZE);
            memcpy(&pPdu[pPdu_idx], fwinfo.date_time, FW_GENERATION_DATE_SIZE);
            pPdu_idx += FW_GENERATION_DATE_SIZE;

            FILL_STRING(IMAGE_HASH_SIZE);
            memcpy(&pPdu[pPdu_idx], fwinfo.hash, IMAGE_HASH_SIZE);
            pPdu_idx += IMAGE_HASH_SIZE;
        }
        else
        {
            /* already initial to zero */
#if defined(FEATURE_SW_VER_NAME_2_0)  // jp.kim 24.12.13
            FILL_STRUCT(0x04);
            FILL_STRING(2);
#endif
            // mt_type
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;

            // fwinfo.version
            FILL_STRING(FW_VERSION_SIZE);
            pPdu[pPdu_idx++] = 0x2d;
            pPdu[pPdu_idx++] = 0x2d;
            pPdu[pPdu_idx++] = 0x2d;
            pPdu[pPdu_idx++] = 0x2d;
            pPdu[pPdu_idx++] = 0x2d;
            pPdu[pPdu_idx++] = 0x2d;

            // fwinfo.date_time
            FILL_STRING(FW_GENERATION_DATE_SIZE);
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;

            FILL_STRING(IMAGE_HASH_SIZE);

            // 32개
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;

            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;

            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;

            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;

            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;

            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;

            pPdu[pPdu_idx++] = 0x00;
            pPdu[pPdu_idx++] = 0x00;
        }

        break;
    }
}

static void ob_evt_fw_apply_date(void)
{
    uint32_t t32;
    ST_FW_INFO fwinfo;

    DPRINTF(DBG_TRACE, _D "%s: att_id[%d]\r\n", __func__, appl_att_id);

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:
        t32 = obis_get_fw_info_type_group_b(obis_gb);
        DPRINTF(DBG_TRACE, _D "%s: fw_info_type[%d]\r\n", __func__, t32);
        if (t32 == FWINFO_CUR_E_MODEM)
        {
#if 0  // jp.kim 25.01.20
            if (!g_modem_exist)
#else
            if (dsm_atcmd_if_is_valid(MEDIA_RUN_EXT, false) != true)
#endif
            {
                goto fw_update_date_err;
            }
        }

        dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, t32);

        if (fwinfo.dt.year == 0xff && fwinfo.dt.month == 0xff &&
            fwinfo.dt.date == 0xff && fwinfo.dt.hour == 0xff &&
            fwinfo.dt.min == 0xff && fwinfo.dt.sec == 0xff)
        {  // 규격 반영
        fw_update_date_err:
        {
            uint8_t cnt = 0;

            FILL_STRING(0x0c);
            /*
                현재 소프트웨어 적용 날짜 : date-time 초기값은
               {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
               0xff, 0x80, 0x00, 0xff} date-time[12] = {년, 월, 일,
               시, 분, 초, 밀리초/10, 모름, 모름, 모름}
            */
            for (cnt = 0; cnt < 9; cnt++)
            {
                FILL_V8(0xff);
            }
            FILL_V8(0x80);
            FILL_V8(0x00);
            FILL_V8(0xff);

            DPRINTF(DBG_WARN, _D "%s: product_fw_apply_date: all ff\r\n",
                    __func__);
        }
        }
        else
        {
            fill_clock_obj(&fwinfo.dt);
        }

        break;
    }
}

static void ob_evt_fw_imagetransfer(void)
{
    uint32_t t32 = 0;
    uint32_t type = IMG__FW;
    // uint8_t *tptr = appl_tbuff;

    DPRINTF(DBG_TRACE, _D "%s: Attributes %d\r\n", __func__, appl_att_id);

    // Get Req
    /* 통신 규격 3.4.2.11.3 소프트웨어 업데이트 Image transfer :
     * Attributes
     */
    switch (appl_att_id)
    {
    case 0x01:
        /* TBD */
        break;

    case 0x02:
    {
        /* [image_block_size] */
        // 64, 128, 256, 512 bytes (default : 512 bytes)
        // TODO: (WD)
        t32 = dsm_imgtrfr_get_blk_size(type);
        FILL_U32(t32);
    }
    break;

    case 0x03:
        /* [image_transferred_block_status] ※ 사용 안 함 */
        break;

    case 0x04:
        /* [image_first_not_transferred_block_number] */
        t32 = dsm_imgtrfr_get_blk_num(type);
        FILL_U32(t32);
        break;

    case 0x05:
        /* [image_transfer_enabled] */
        FILL_BOOL(dsm_imgtrfr_get_transfer_enabled(type));
        break;

    case 0x06:
        /* [image_transfer_status] */
        FILL_ENUM(dsm_imgtrfr_get_transfer_status(type));
        break;

    case 0x07:
        /* [image_to_activate_info] */
        // client 에서 get 하여 activate 여부 결정...
        FILL_ARRAY(1);
        FILL_STRUCT(0x03);
        t32 = dsm_imgtrfr_get_image_size(type);
        FILL_U32(t32);
        FILL_STRING(IMAGE_FW_NAME_MAX_SIZE);
        memcpy(&pPdu[pPdu_idx], dsm_imgtrfr_get_name(type),
               IMAGE_FW_NAME_MAX_SIZE);
        pPdu_idx += IMAGE_FW_NAME_MAX_SIZE;
        FILL_STRING(IMAGE_HASH_SIZE);
        memcpy(&pPdu[pPdu_idx], dsm_imgtrfr_get_hash(type), IMAGE_HASH_SIZE);
        pPdu_idx += IMAGE_HASH_SIZE;
        break;
    }
}

static void ob_event_info_profile(void)
{
    uint8_t t8;
    uint32_t t32;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x02:
        FILL_ARRAY(1);

        if (mt_is_onephase())
        {
            float fval;

            FILL_STRUCT(0x05);

            ob_err_code_1();
            ob_err_code_2();
            ob_err_code_3();
            ob_err_code_4();

            fval = get_lpavg_v(0);
            FILL_FLOAT(fval);
        }
        else
        {
            float fval;

            FILL_STRUCT(0x07);

            ob_err_code_1();
            ob_err_code_2();
            ob_err_code_3();
            ob_err_code_4();

            fval = get_lpavg_v(0);
            FILL_FLOAT(fval);
            fval = get_lpavg_v(1);
            FILL_FLOAT(fval);
            fval = get_lpavg_v(2);
            FILL_FLOAT(fval);
        }

        break;

    case 0x03:
        if (mt_is_onephase())
        {
            memcpy(&pPdu[pPdu_idx], &EVT_INFO_PROF_capture_objects[0],
                   EVT_INFO_PROF_CAPOBJ_SIZE);
            pPdu_idx += EVT_INFO_PROF_CAPOBJ_SIZE;
        }
        else
        {
            memcpy(&pPdu[pPdu_idx], &EVT_INFO_3phs_PROF_capture_objects[0],
                   EVT_INFO_3PHS_PROF_CAPOBJ_SIZE);
            pPdu_idx += EVT_INFO_3PHS_PROF_CAPOBJ_SIZE;
        }

        break;

    case 0x04:
        t32 = 0;
        FILL_U32(t32);  // 0 means No automatic capture
        break;

    case 0x05:
        t8 = 1;
        FILL_ENUM(t8);  // sort method(FIFO)
        break;

    case 0x06:
        memcpy(&pPdu[pPdu_idx], &LP_sort_object[0], LP_SORTOBJ_SIZE);

        pPdu_idx += LP_SORTOBJ_SIZE;
        break;

    case 0x07:
        t32 = 1;
        FILL_U32(t32);
        break;

    case 0x08:
        t32 = 1;
        FILL_U32(t32);
        break;
    }
}

static void ob_ext_modem_id(void)
{
    /*
    착탈형 모뎀 기기 번호(11byte) : ASCII
    */
    ST_MDM_ID st_ext_modem_id = {0};

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    // memset(&st_ext_modem_id, 0x00, sizeof(ST_MDM_ID));
    nv_read(I_EXT_MODEM_ID, (uint8_t*)&st_ext_modem_id);

    FILL_STRING(MODEM_ID_SIZE);
    memcpy(&pPdu[pPdu_idx], st_ext_modem_id.id, MODEM_ID_SIZE);
    pPdu_idx += MODEM_ID_SIZE;
}

static void ob_modem_atcmd_forSet(uint8_t modem_type)
{
    ST_ATCMD_TMP_BUF* pst_atmcd_from_client =
        dsm_get_atcmd_from_client(modem_type);

    DPRINTF(DBG_TRACE, _D "%s: modem_type[%d]\r\n", __func__, modem_type);

    if (pst_atmcd_from_client->len < CLIENT_ATCMD_STRING_MAX_SIZE)
    {
        DPRINT_HEX(DBG_TRACE, "AT_CLIENT_STRING",
                   &pst_atmcd_from_client->string[0],
                   pst_atmcd_from_client->len, DUMP_ALWAYS);
        fill_octet_string_x(pst_atmcd_from_client->string,
                            pst_atmcd_from_client->len);
    }
}

static void ob_modem_atcmd_forRsp(uint8_t modem_type)
{
    ST_ATCMD_TMP_BUF* pst_atmcd_from_modem =
        dsm_get_atcmd_from_modem(modem_type);

    DPRINTF(DBG_TRACE, _D "%s: modem_type[%d]\r\n", __func__, modem_type);

    if (pst_atmcd_from_modem->len < CLIENT_ATCMD_STRING_MAX_SIZE)
    {
        DPRINT_HEX(DBG_TRACE, "AT_MODEM_STRING",
                   &pst_atmcd_from_modem->string[0], pst_atmcd_from_modem->len,
                   DUMP_ALWAYS);
        fill_octet_string_x(pst_atmcd_from_modem->string,
                            pst_atmcd_from_modem->len);
    }
}

static void ob_stock_op_times(void)
{
#ifdef STOCK_OP /* bccho, 2024-09-26 */
    uint16_t t16 = dsm_stock_op_times_read();

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
#else
    uint16_t t16 = 0;
#endif
    FILL_U16(t16);
}

static void ob_lp_tatal_cnt(void)
{
    uint16_t t32 = TOTAL_LP_EVENT_CNT;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    FILL_U32(t32);
}

static void ob_max_demand_sign(void) {}

static void ob_energy_sign(void) {}

static void ob_adj_factor(void)
{
    int8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    t8 = (int8_t)get_inst_temp();
    FILL_S08(t8);
}

void lp_mtdir_init(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    lpinfo_is_caped = false;
}

bool lp_mtdir_is_chged(void) { return false; }

bool lp_mtdir_is_chged_and_blocked(void) { return false; }

static void ob_tmsync_range(void)
{
    if (appl_att_id == 0x02)
    {
        FILL_U32(rtc_shift_range);
    }
    else if (appl_att_id == 0x03)
    {
        fill_register_scale((int8_t)0, 7);  // scale(0), unit(sec)
    }
}
