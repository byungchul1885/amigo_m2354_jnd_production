#include "main.h"
#include "options.h"
#if 0 /* bccho, FLASH, 2023-07-15 */
#include "flash.h"
#endif /* bccho */
#include "nv.h"
#include "tmp.h"
#include "key.h"
#include "lp.h"
#include "comm.h"
#include "phy.h"
#include "dl.h"
#include "appl.h"
#include "approc.h"
#include "act_req.h"
#include "set_req.h"
#include "afe.h"
#include "amg_imagetransfer.h"
#include "amg_sec.h"
#include "amg_push_datanoti.h"
#include "amg_mtp_process.h"
#include "amg_mif_prtl.h"
#include "amg_meter_main.h"
#include "amg_media_mnt.h"
#include "amg_wdt.h"
#include "get_req.h"

#define _D "[SREQ] "

#if 1  // jp.kim 25.02.04
extern bool mif_meter_para_set_wait_ing;
extern uint8_t mif_meter_para_set_wait_seq_cnt;
#endif

extern ST_MIF_METER_PARM g_mtp_meter_parm;
void mif_meter_parm_set(void);

extern uint8_t prg_tou_type;

bool MmodeCHG_sr_dr_type_sr_dr_run_pending_sts = 0;
prog_dl_type prog_dl;

int appl_set_save_idx;
uint8_t appl_set_save_result;

uint8_t* packed_ptr;
int packed_idx;

uint16_t touset_array_entry_idx;
int touset_parse_idx;
uint8_t* touset_parse_buf;

holiday_date_type hol_date_block;

void all_error_sts_chk_trigger(uint8_t errcodeidx);

extern void prod_control_cal_begin(int idx);  // jp.kim 24.11.07

static void approc_set_req_normal(int idx);
static void approc_set_req_block_first(int idx);
static void approc_set_req_block(int idx);
static void approc_set_req_proc(int idx);

static void obset_ass_ln(int idx);
static void obset_date_time(int idx);
static void obset_sig_sel(int idx);
static void obset_mdm_baud(int idx);
static void obset_period_billdate(int idx);
static void obset_nperiod_billdate(int idx);
static void obset_billing_parm(int idx);
static void obset_selective_act(int idx);
static void obset_lp_interval(int idx);
static void obset_lpavg_interval(int idx);
static void obset_cur_last_dm(int idx);
static void obset_tou_cal(int idx);
static void obset_holidays(int idx);
static void obset_dsp_supply_mode(int idx);
static void obset_dsp_pvt_mode(int idx);
static void obset_lcdset_parm(int idx);
static void obset_scurr_limit_val(int idx);
static void obset_scurr_limit2_val(int idx);
static void obset_scurr_autortn_val(int idx);
static void obset_prepay(uint8_t kind, int idx);
static void obset_temp_thrshld(int idx);
static void obset_sagswell(uint8_t kind, int idx);
static void obset_ts_conf(int idx);
static void obset_condensor_inst(int idx);
static void obset_sel_react(int idx);
static void obset_comm_enable(int idx);
static void obset_key_value(int idx);
static void obset_scurr_hold(int idx);
static void obset_scurr_rtn_1(int idx);
static void obset_scurr_rtn_2(int idx);
static void obset_scurr_cnt_n1(int idx);
static void obset_overcurr_enable(int idx);
static void obset_ext_prog_id(int idx);
static void hol_date_block_restore(void);
static void hol_date_block_save(void);

static void obset_custom_id(int idx);
static void obset_device_id(int idx);
static void obset_curr_temp(int idx);
static void obset_inst_profile(int idx);

static void obset_working_fault_min(int idx);
static void obset_security_setup(int idx);
static void obset_evt_tou_imagetransfer(int idx);
static void obset_evt_fw_imagetransfer(uint16_t idx);
static void obset_realtime_lp_interval(int idx);
static void obset_evt_push_setup_err_code(uint16_t idx);
static void obset_evt_push_setup_lastLP(uint16_t idx);
static void obset_evt_err_code_activate(uint16_t idx, uint8_t grp_e);

static void obset_modem_atcmd(int idx, uint8_t modem_type);

static void obset_tmsync_range(int idx);
static void obset_ct_pt_ratio(int idx);

static void obset_zcrs_sig_out_durtime(int idx);
#if 0
static void obset_zcrs_sig_out_cmpstime(int idx);
static void obset_zcrs_sig_out_resulttime(int idx);
#endif
static void obset_self_error_ref(int idx);
static void obset_auto_mode_sel(int idx);
static void obset_holiday_sel(int idx);

static void obset_thd_rec_period_sel(int idx);

static bool get_TS_selector(uint8_t* cp, uint8_t* prslt);
static bool dt_is_unspecified(date_time_type* dt);

static bool get_date_time_from_comm(date_time_type* dt, uint8_t* wk,
                                    uint8_t* cp);

extern bool dl_seg_frame_over;
extern void dsm_mtp_fsm_send(void);

static void obset_inst_cert(int idx);
static void obset_inst_key(int idx);

typedef struct
{
    uint8_t id[6];
    disp_mode_type dmode;
} obis_suppdsp_mode_type;

static const obis_suppdsp_mode_type suppdsp_other_obis_table[] = {
    {OBIS_LOCAL_TIME, DSPMODE_TIME},
    {OBIS_LOCAL_DATE, DSPMODE_DATE},

#if 1 /* bccho, 2024-05-17 */
    {OBIS_PGM_ID, DSPMODE_TOU_PROG_ID},
#endif
    {OBIS_PERIOD_BILLDATE, DSPMODE_REG_DATE},

    {OBIS_mDR_NUM, DSPMODE_mDR_NUM},
    {
        {
            0xff,
        },
    }};

void approc_set_req(int idx)
{
    appl_whm_inf_collected = false;

    switch (appl_reqchoice)  // apdu[1]
    {
    case SET_REQ_NORMAL:
        DPRINTF(DBG_TRACE, _D "Normal\r\n");
        approc_set_req_normal(idx);
        break;
    case SET_REQ_BLOCK_FIRST:
        DPRINTF(DBG_TRACE, _D "Block First\r\n");
        approc_set_req_block_first(idx);
        break;
    case SET_REQ_BLOCK:
        DPRINTF(DBG_TRACE, _D "Block\r\n");
        approc_set_req_block(idx);
        break;
    default:
        appl_resp_choice = SET_RES_NORMAL;
        appl_resp_result = SET_RESULT_TYPE_UNMAT;
        break;
    }

    appl_set_resp();
}

static void approc_set_req_normal(int idx)
{
    appl_result_type rslt;

    appl_resp_choice = SET_RES_NORMAL;

    idx = appl_cosem_descript(idx);
    if (idx >= appl_len)
    {
        appl_resp_result = SET_RESULT_TYPE_UNMAT;
        return;
    }

    // selective access option check (idx increment)
    if (appl_msg[idx++] != 0)
    {
        appl_resp_result = SET_RESULT_TYPE_UNMAT;
        return;
    }

    rslt = appl_obj_enum_and_acs_check();
    if (rslt != RESULT_OK)
    {
        appl_resp_result = SET_RESULT_REQ_NG;
        return;
    }

    if (!comm_en_coveropen && IS_MorTCOVER_OPEN &&
        appl_obj_id != OBJ_COMM_ENABLE)
    {
        appl_resp_result = SET_RESULT_REQ_NG;
        return;
    }

    approc_set_req_proc(idx);
}

static void approc_set_req_block_first(int idx)
{
    appl_result_type rslt;

    idx = appl_cosem_descript(idx);
    if (appl_msg[idx++] != 0)
    {
        appl_resp_result = SET_RESULT_TYPE_UNMAT;
        return;
    }
    idx = appl_block_descript(idx);
    if (idx >= appl_len)
    {
        appl_resp_choice = SET_RES_BLOCK_LAST;
        appl_resp_block_num = 1L;
        appl_resp_result = SET_RESULT_TYPE_UNMAT;
        return;
    }

    rslt = appl_obj_enum_and_acs_check();
    if (rslt != RESULT_OK)
    {
        appl_resp_choice = SET_RES_BLOCK_LAST;
        appl_resp_block_num = appl_block_num;
        appl_resp_result = SET_RESULT_REQ_NG;
        return;
    }

    if (!comm_en_coveropen && IS_MorTCOVER_OPEN &&
        appl_obj_id != OBJ_COMM_ENABLE)
    {
        appl_resp_result = SET_RESULT_REQ_NG;
        return;
    }

    // appl_resp_choice = SET_RES_BLOCK;		// 16.11.9 --> set below
    appl_resp_block_num = appl_block_num;
    appl_next_block_num = appl_block_num + 1L;  // checked in next block-req

    // initialize variables
    appl_set_save_idx = 0;
    appl_set_save_result = 0;

    appl_is_first_block = true;

    // 16.11.9 -> first block and last block
    if (!appl_is_last_block)
        appl_resp_choice = SET_RES_BLOCK;
    else
        appl_resp_choice = SET_RES_BLOCK_LAST;

    approc_set_req_proc(idx);
}

static void approc_set_req_block(int idx)
{
    idx = appl_block_descript(idx);
    if (idx >= appl_len)
    {
        appl_resp_choice = SET_RES_BLOCK_LAST;
        appl_resp_block_num = 1L;
        appl_resp_result = SET_RESULT_TYPE_UNMAT;
        return;
    }

    appl_resp_block_num = appl_block_num;
    if (appl_block_num != appl_next_block_num)
    {
        appl_resp_choice = SET_RES_BLOCK_LAST;
        appl_resp_result = SET_RESULT_BLOCK_NO_UNMATCH;
        return;
    }

    appl_next_block_num += 1L;
    appl_set_save_idx += 1;
    appl_is_first_block = false;

    if (!appl_is_last_block)
        appl_resp_choice = SET_RES_BLOCK;
    else
        appl_resp_choice = SET_RES_BLOCK_LAST;

    approc_set_req_proc(idx);
}

static void approc_set_req_proc(int idx)
{
    switch (appl_obj_id)
    {
    case OBJ_ASSOCIATION_LN:
        obset_ass_ln(idx);
        break;

    case OBJ_DATE_TIME:
        obset_date_time(idx);
        break;

    case OBJ_OUT_SIG_SEL: /*부가신호 장치 출력선택*/
        obset_sig_sel(idx);
        break;

    case OBJ_HDLC_SETUP:
        obset_mdm_baud(idx);
        break;

    case OBJ_PERIOD_BILLDATE:
        obset_period_billdate(idx);
        break;

    case OBJ_NPERIOD_BILLDATE:
        obset_nperiod_billdate(idx);
        break;

    case OBJ_BILLING_PARM:
        obset_billing_parm(idx);
        break;

    case OBJ_SELECTIVE_ACT: /*선택 유효전력량 개별설정*/
        obset_selective_act(idx);
        break;

    case OBJ_LP_INTERVAL:
        obset_lp_interval(idx);
        break;

    case OBJ_LPAVG_INTERVAL:
        obset_lpavg_interval(idx);
        break;

    case OBJ_CURR_LAST_DEMAND: /* 현재/직전 수요시한 수요전력 */
        obset_cur_last_dm(idx);
        break;

    case OBJ_TOU_CAL:        /* TOU : Activity Calendar */
        obset_tou_cal(idx);  // TOU Set
        break;

    case OBJ_HOLIDAYS:
        obset_holidays(idx);
        break;

    case OBJ_SUPPLY_DISP_MODE:
        obset_dsp_supply_mode(idx);
        break;

    case OBJ_PVT_DISP_MODE: /*무부하시 부하 동작 표시 설정*/
        obset_dsp_pvt_mode(idx);
        break;

    case OBJ_USER_DISP_MODE:
        // 규격에 잇음..검토후 적용
        break;

    case OBJ_MONTH_SUBLOCKS:
        // 규격에 잇음..검토후 적용
        break;

    case OBJ_LCDSET_PARM: /*계량모드 설정 파라미터*/
        obset_lcdset_parm(idx);
        break;

    case OBJ_sCURR_LIMIT_VAL:
        obset_scurr_limit_val(idx);
        break;

    case OBJ_sCURR_LIMIT2_VAL:
        obset_scurr_limit2_val(idx);
        break;

    case OBJ_sCURR_autoRTN_VAL: /*부하제한 차단 설정횟수*/
        obset_scurr_autortn_val(idx);
        break;

    case OBJ_REM_ENERGY:
        obset_prepay(0, idx);
        break;

    case OBJ_REM_MONEY:
        obset_prepay(1, idx);
        break;

    case OBJ_REM_TIME:
        obset_prepay(2, idx);
        break;

    case OBJ_BUY_ENERGY:
        obset_prepay(3, idx);
        break;

    case OBJ_BUY_MONEY:
        obset_prepay(4, idx);
        break;

    case OBJ_PREPAY_ENABLE:
        obset_prepay(5, idx);
        break;

    case OBJ_PREPAY_LOADLIMIT_CANCEL:
        obset_prepay(6, idx);
        break;

    case OBJ_TEMP_THRSHLD:
        obset_temp_thrshld(idx);
        break;

    case OBJ_SAG_VAL_SET:
        obset_sagswell(1, idx);
        break;

    case OBJ_SAG_TIME_SET:
        obset_sagswell(2, idx);
        break;

    case OBJ_SWELL_VAL_SET:
        obset_sagswell(3, idx);
        break;

    case OBJ_SWELL_TIME_SET:
        obset_sagswell(4, idx);
        break;

    case OBJ_CONDENSOR_INST: /*경 부하 오결선 감지 체크*/
        obset_condensor_inst(idx);
        break;

    case OBJ_TS_CONF: /*타임스위치 개폐 시작 시간 */
        obset_ts_conf(idx);
        break;

    case OBJ_SEL_REACT: /*선택 무효전력량 개별 예약설정*/
        obset_sel_react(idx);
        break;

    case OBJ_COMM_ENABLE:
        obset_comm_enable(idx);
        break;

    case OBJ_KEY_VALUE:
        obset_key_value(idx);
        break;

    case OBJ_sCURR_HOLD: /*부하제한 감지시간*/
        obset_scurr_hold(idx);
        break;

    case OBJ_sCURR_RECOVER_N1: /*부하제한 제 1구간 재복귀시간*/
        obset_scurr_rtn_1(idx);
        break;

    case OBJ_sCURR_RECOVER_N2:
        obset_scurr_rtn_2(idx);
        break;

    case OBJ_sCURR_COUNTER_N1: /*부하제한 제 1구간 차단 설정 횟수*/
        obset_scurr_cnt_n1(idx);
        break;

    case OBJ_OVERCURR_ENABLE: /*과부하전류 차단 설정/해제*/
        obset_overcurr_enable(idx);
        break;

    case OBJ_EXT_PROG_ID:
        obset_ext_prog_id(idx);
        break;

    case OBJ_CUSTOM_ID:
    case OBJ_MANUFACT_ID:  ////JP.KIM 24.11.08	1) 생산프로그램 PROTOCOL을 dlms
                           /// 방식으로 변경
        obset_custom_id(idx);
        break;

    case OBJ_DEVICE_ID:
        obset_device_id(idx);
        break;

    case OBJ_CURR_TEMP:
        obset_curr_temp(idx);
        break;

    case OBJ_INST_PROFILE:
        obset_inst_profile(idx);
        break;

        /// JP.KIM 24.11.08	1) 생산프로그램 PROTOCOL을 dlms 방식으로 변경
    case OBJ_CAL_ADJ_ACT:
        obset_inst_profile(idx);
        break;

    case OBJ_INSTALL_CERT:  // bccho, 2024-12-06
        obset_inst_cert(idx);
        break;

    case OBJ_INSTALL_KEY:  // bccho, 2024-12-06
        obset_inst_key(idx);
        break;

    case OBJ_SECURITY_SETUP_3:
    case OBJ_SECURITY_SETUP_4:
        obset_security_setup(idx);
        break;

    case OBJ_TOU_IMAGE_TRANSFER:
        obset_evt_tou_imagetransfer(idx);
        break;

    case OBJ_SW_IMAGE_TRANSFER:
        obset_evt_fw_imagetransfer(idx);
        break;

    case OBJ_RTIME_P_LP_INTERVAL:
        obset_realtime_lp_interval(idx);
        break;

    case OBJ_PHASE_DET_CONT_TIME:
        //
        obset_zcrs_sig_out_durtime(idx);
        break;

#if 0  // jp.kim 24.11.14  읽기 전용  
    case OBJ_PHASE_DET_CORRECT_VAL:
        // 
        obset_zcrs_sig_out_cmpstime(idx);
        break;

    case OBJ_PHASE_DET_RESULT_VAL:
        obset_zcrs_sig_out_resulttime(idx);
        break;
#endif

    case OBJ_PERMITxx_TIME_LIMIT:
        // 기능 구현 필요..
        obset_tmsync_range(idx);
        break;

    case OBJ_SELF_ERR_REF_VAL:
        // 기능 구현 필요..
        obset_self_error_ref(idx);
        break;

    case OBJ_CT_RATIO:
    case OBJ_PT_RATIO:
        // 기능 구현 필요..
        obset_ct_pt_ratio(idx);
        break;

    case OBJ_METERING_TYPE_SEL:
        obset_auto_mode_sel(idx);
        break;

    case OBJ_INT_PLC_MODEM_ATCMD:  // jp.kim 25.01.20
    {
    }  // jp.kim 25.01.22  //
    break;
    case OBJ_INT_MODEM_ATCMD:
        obset_modem_atcmd(idx, INT_MODEM_TYPE);  // jp.kim 25.01.22  //
        break;
    case OBJ_EXT_MODEM_ATCMD:
        obset_modem_atcmd(idx, EXT_MODEM_TYPE);  // jp.kim 25.01.22  //
        break;

    case OBJ_PUSH_ACT_ERR_CODE_1:
    case OBJ_PUSH_ACT_ERR_CODE_2:
    case OBJ_PUSH_ACT_ERR_CODE_3:
    case OBJ_PUSH_ACT_ERR_CODE_4:
        obset_evt_err_code_activate(idx, obis_ge);
        break;

    case OBJ_PUSH_SETUP_ERR_CODE:
        obset_evt_push_setup_err_code(idx);
        break;

    case OBJ_PUSH_SETUP_LAST_LP:
        obset_evt_push_setup_lastLP(idx);
        break;

    case OBJ_OLD_METER_TOU_TRANSFER:
        break;

    case OBJ_HOLIDAY_SEL:
        obset_holiday_sel(idx);
        break;

    case OBJ_THD_PERIOD_SEL:
        obset_thd_rec_period_sel(idx);
        break;

    case OBJ_WORKING_FAULT_MIN:
        obset_working_fault_min(idx);
        break;

    default:
        appl_resp_result = SET_RESULT_REQ_NG;
        break;
    }
}

static void obset_working_fault_min(int idx)
{
    switch (appl_att_id)
    {
    case 0x02:
        if (appl_msg[idx] == UNSIGNED_TAG)
        {
            idx++;
            working_fault_min = appl_msg[idx];
        }
        break;
    }
}

static void obset_evt_push_setup_err_code(uint16_t idx)
{
    uint8_t* pt8;
    uint16_t t16;
    uint8_t push_idx = 0, array_num = 0;
    ST_PUSH_SETUP_TABLE* pst_push_setup;

    DPRINTF(DBG_TRACE, _D "%s: att_id %d\r\n", __func__, appl_att_id);

    pst_push_setup = dsm_push_setup_get_setup_table();

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:

        break;
    case 0x03:

        break;
    case 0x04:  // window
        if (appl_msg[idx] == ARRAY_TAG)
        {
            array_num = appl_msg[idx + 1];

            if (array_num >= PUSH_WINDOW_MAX_NUM)
                array_num = 1;

            if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_ERR_CODE, &push_idx))
            {
                if (array_num == 0)
                {
                    pst_push_setup->setup[push_idx].window_cnt = array_num;
                }
                else if (appl_msg[idx + 2] == STRUCTURE_TAG &&
                         appl_msg[idx + 3] == 2)
                {
                    get_date_time_from_comm(
                        &pst_push_setup->setup[push_idx].window[0].st_dt, 0,
                        &appl_msg[idx + 4]);
                    get_date_time_from_comm(
                        &pst_push_setup->setup[push_idx].window[0].sp_dt, 0,
                        &appl_msg[idx + 4 + 14]);
                    pst_push_setup->setup[push_idx].window_cnt = array_num;

                    DPRINTF(DBG_TRACE, "%s: window_cnt[%d]\r\n", __func__,
                            array_num);
                    DPRINT_HEX(DBG_TRACE, "ST_DT",
                               &pst_push_setup->setup[push_idx].window[0].st_dt,
                               sizeof(date_time_type), DUMP_ALWAYS);
                    DPRINT_HEX(DBG_TRACE, "SP_DT",
                               &pst_push_setup->setup[push_idx].window[0].sp_dt,
                               sizeof(date_time_type), DUMP_ALWAYS);
                }
                else
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
                all_error_sts_chk_trigger(ERR_CODE_MAX_IDX);
            }
            else
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }

        break;
    case 0x05:  // randomisatin_start_interval
        if (appl_msg[idx] == LONGUNSIGNED_TAG)
        {
            pt8 = &appl_msg[idx + 1];
            ToH16((U8_16*)&t16, pt8);

            if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_ERR_CODE, &push_idx))
            {
                DPRINTF(DBG_TRACE, "idx[%d], random_start_intval[%d -> %d]\r\n",
                        push_idx,
                        pst_push_setup->setup[push_idx].random_start_intval,
                        (uint8_t)t16);
                pst_push_setup->setup[push_idx].random_start_intval =
                    (uint8_t)t16;
            }
            else
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }

        break;
    case 0x06:

        break;
    case 0x07:  // repetion_delay

        break;
    }
}

static void obset_evt_push_setup_lastLP(uint16_t idx)
{
    uint8_t* pt8;
    uint16_t t16;
    uint8_t push_idx = 0, array_num = 0;
    ST_PUSH_SETUP_TABLE* pst_push_setup;

    DPRINTF(DBG_TRACE, _D "%s: att_id %d\r\n", __func__, appl_att_id);

    pst_push_setup = dsm_push_setup_get_setup_table();

    switch (appl_att_id)
    {
    case 0x01:

        break;
    case 0x02:  // push_object_list

        break;
    case 0x03:  // send_destination_and_method

        break;
    case 0x04:  // communication window
        if (appl_msg[idx] == ARRAY_TAG)
        {
            array_num = appl_msg[idx + 1];

            if (array_num >= PUSH_WINDOW_MAX_NUM)
                array_num = 1;
            if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_LAST_LP, &push_idx))
            {
                if (array_num == 0)
                {
                    pst_push_setup->setup[push_idx].window_cnt = array_num;
                }
                else if (appl_msg[idx + 2] == STRUCTURE_TAG &&
                         appl_msg[idx + 3] == 2)
                {
                    get_date_time_from_comm(
                        &pst_push_setup->setup[push_idx].window[0].st_dt, 0,
                        &appl_msg[idx + 4]);
                    get_date_time_from_comm(
                        &pst_push_setup->setup[push_idx].window[0].sp_dt, 0,
                        &appl_msg[idx + 4 + 14]);
                    pst_push_setup->setup[push_idx].window_cnt = array_num;

                    DPRINTF(DBG_TRACE, "%s: window_cnt[%d]\r\n", __func__,
                            array_num);
                    DPRINT_HEX(DBG_TRACE, "ST_DT",
                               &pst_push_setup->setup[push_idx].window[0].st_dt,
                               sizeof(date_time_type), DUMP_ALWAYS);
                    DPRINT_HEX(DBG_TRACE, "SP_DT",
                               &pst_push_setup->setup[push_idx].window[0].sp_dt,
                               sizeof(date_time_type), DUMP_ALWAYS);
                }
                else
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;
    case 0x05:  // randomisatin_start_interval
        if (appl_msg[idx] == LONGUNSIGNED_TAG)
        {
            pt8 = &appl_msg[idx + 1];
            ToH16((U8_16*)&t16, pt8);

            if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_LAST_LP, &push_idx))
            {
                DPRINTF(DBG_TRACE, "idx[%d], random_start_intval[%d -> %d]\r\n",
                        push_idx,
                        pst_push_setup->setup[push_idx].random_start_intval,
                        (uint8_t)t16);
                pst_push_setup->setup[push_idx].random_start_intval =
                    (uint8_t)t16;
            }
            else
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }

        break;
    case 0x06:  // number_of_retries

        break;
    case 0x07:  // repetion_delay

        break;
    }
}

static void obset_evt_err_code_activate(uint16_t idx, uint8_t grp_e)
{
    uint8_t errcodeidx = 0;
    ST_PUSH_ACTI_ERRCODE st_push_acti;
    DPRINTF(DBG_TRACE, _D "%s: err_code[%d]\r\n", __func__, grp_e);

    errcodeidx = dsm_covert_grp_e_2_errcodeidx(grp_e);
    if (errcodeidx >= ERR_CODE_MAX_IDX)
    {
        appl_resp_result = SET_RESULT_DATA_NG;
        return;
    }

    if (appl_msg[idx] == BITSTRING_TAG)
    {
        if (appl_msg[idx + 1] == 8)
        {
            dsm_push_err_code_nvread(&st_push_acti);

            DPRINTF(DBG_TRACE,
                    _D "%s: errcodeidx[%d], errcode[0x%02X -> 0x%02X]\r\n",
                    __func__, st_push_acti.code[errcodeidx], appl_msg[idx + 2]);
            st_push_acti.code[errcodeidx] = appl_msg[idx + 2];

            dsm_push_err_code_nvwrite(&st_push_acti);
            all_error_sts_chk_trigger(errcodeidx);
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

// get date time from communication buffer
// (date_time type in IEC62056-62 4.4)
// : (yy, +1) mm dd day hr mn se hd (t_diff, +1) st
static bool get_date_time_from_comm(date_time_type* dt, uint8_t* wk,
                                    uint8_t* cp)
{
    uint16_t t16;

    if (*cp != OCTSTRING_TAG || *(cp + 1) != 12)
        return false;

    cp += 2;

    ToH16((U8_16*)&t16, cp);
    if (t16 < BASE_YEAR)
        return false;

    if (t16 == 0xffff)
        dt->year = 0xff;
    else
        dt->year = (uint8_t)(t16 - BASE_YEAR);
    cp += 2;

    dt->month = *cp;
    cp += 1;

    dt->date = *cp;
    cp += 1;

    if (wk != 0)
        *wk = *cp;
    cp += 1;

    dt->hour = *cp;
    cp += 1;

    dt->min = *cp;
    cp += 1;

    dt->sec = *cp;

    return rtc_is_valid(dt);
}

static void parse_npbill_time(npbill_date_type* parse, int idx)
{
    uint16_t yr;

    packed_idx += 2;  // skip struct tag

    packed_idx += 2;  // skip octet string of time_tag_type
    parse->npbill[idx].dt.hour = packed_ptr[packed_idx];
    parse->npbill[idx].dt.min = packed_ptr[packed_idx + 1];
    parse->npbill[idx].dt.sec = packed_ptr[packed_idx + 2];
    packed_idx += TIME_TAG_LEN;

    packed_idx += 2;  // skip octet string of date_tag_type
    ToH16((U8_16*)&yr, &packed_ptr[packed_idx]);
    if (yr == 0xffff)
    {
        parse->npbill[idx].dt.year = 0xff;
    }
    else
    {
        parse->npbill[idx].dt.year = yr - BASE_YEAR;
    }
    parse->npbill[idx].dt.month = packed_ptr[packed_idx + 2];
    parse->npbill[idx].dt.date = packed_ptr[packed_idx + 3];
    parse->npbill[idx].day = (enum touDAY)(packed_ptr[packed_idx + 4]);
    packed_idx += DATE_TAG_LEN;
}

static void parse_npBilldate(uint8_t* cp, npbill_date_type* parse)
{
    int i;

    packed_ptr = cp;
    packed_idx = 0;

    PARSE_ARRAY(parse->cnt);
    if (parse->cnt > NP_BILLDATE_LEN)
        parse->cnt = NP_BILLDATE_LEN;

    for (i = 0; i < parse->cnt; i++)
    {
        parse_npbill_time(parse, i);
        if (dt_is_unspecified(&parse->npbill[i].dt))
            break;
    }

    parse->cnt = (uint8_t)i;
}

static bool parse_dayid_struct(tou_struct_type* touconf)
{
    bool ret = true;
    uint16_t t16;

    packed_idx += 2;  // struct of day_struct

    packed_idx += 2;  // octet string of time_tag
    touconf->hour = packed_ptr[packed_idx];
    touconf->min = packed_ptr[packed_idx + 1];
    packed_idx += TIME_TAG_LEN;
    if (touconf->hour > 23 || touconf->min >= 60)
    {
        ret = false;
    }

    packed_idx += 2;  // octet string of obis
    if (packed_ptr[packed_idx] != 0x00 || packed_ptr[packed_idx + 1] != 0x00 ||
        packed_ptr[packed_idx + 2] != 0x0a ||
        packed_ptr[packed_idx + 3] != 0x00 ||
        packed_ptr[packed_idx + 4] != 0x64 ||
        packed_ptr[packed_idx + 5] != 0xff)
    {
        ret = false;
    }
    packed_idx += OBIS_NAME_LEN;

    packed_idx += 1;  // tag
    ToH16((U8_16*)&t16, &packed_ptr[packed_idx]);
    touconf->rate = (uint8_t)(t16 & 0xff);
    packed_idx += 2;  // selector

    return ret;
}

static void parse_dayprof(uint8_t* cp, dayid_table_type* parse)
{
    uint8_t i, t8;
    uint16_t blksize;
    uint8_t conf_cnt;
    int chk_point;

    packed_ptr = cp;
    packed_idx = 0;

    // block size parsing
    packed_idx += parse_ext_len_2(packed_ptr, &blksize);

    if (set_req_is_first_block())
    {
        int lensize;

        lensize =
            size_ext_len_2((packed_ptr + packed_idx + 1));  // 1 means Array_tag
        packed_idx += (1 + lensize);                        // 1 means Array_tag
    }

    while (packed_idx < blksize)
    {
        packed_idx += 2;  // struct of (day_id, day_table)

        PARSE_U8(parse->day_id);

        PARSE_ARRAY(t8);
        if (t8 > MAX_TOU_DIV_DLMS)
            t8 = MAX_TOU_DIV_DLMS;

        chk_point = 0;

        conf_cnt = t8;
        for (i = 0; i < t8; i++)
        {
            if (parse_dayid_struct(&parse->tou_conf[i]) == false)
            {
                if (chk_point == 0)
                {
                    chk_point = 1;
                    conf_cnt = i;
                }
            }
        }

        parse->tou_conf_cnt = conf_cnt;

        nv_sub_info.ch[0] = parse->day_id;
        if (nv_write(I_DAY_PROFILE_DL, (uint8_t*)parse) == false)
        {
            appl_set_save_result = 1;
            break;
        }
    }

    if (appl_is_last_block || appl_set_save_result)
    {
        if (appl_set_save_result == 0)
        {
            i = parse->day_id + 1;
        }
        else
        {
            i = parse->day_id;
        }

        for (; i < DAY_PROF_SIZE; i++)
        {
            parse->day_id = i;
            parse->tou_conf_cnt = 0;

            nv_sub_info.ch[0] = parse->day_id;
            nv_write(I_DAY_PROFILE_DL, (uint8_t*)parse);
        }
    }
}

static void parse_seasonprof_time(season_struct_type* parse)
{
    packed_idx += 2;  // struct tag
    packed_idx += 3;  // season name
    // start time
    packed_idx += 2;  // octet string
    parse->month = packed_ptr[packed_idx + 2];
    parse->date = packed_ptr[packed_idx + 3];
    packed_idx += DATE_TIME_TAG_LEN;
    // week name
    packed_idx += 2;  // octet string
    parse->week_id = packed_ptr[packed_idx];
    packed_idx += 1;
}

static void parse_seasonprof(uint8_t* cp, season_date_type* parse)
{
    int i;

    packed_ptr = cp;
    packed_idx = 0;

    PARSE_ARRAY(parse->cnt);
    if (parse->cnt > SEASON_PROF_SIZE)
        parse->cnt = SEASON_PROF_SIZE;

    for (i = 0; i < parse->cnt; i++)
    {
        parse_seasonprof_time(&parse->season[i]);
    }
}

static void parse_weekprof_time(week_struct_type* parse)
{
    int i;

    packed_idx += 2;  // struct tag
    // week name
    packed_idx += 2;  // octet string
    parse->week_id = packed_ptr[packed_idx];
    packed_idx += 1;
    // start time
    for (i = 0; i < WEEK_LEN; i++)
    {
        packed_idx += 1;                              // tag
        parse->day_id[i] = packed_ptr[packed_idx++];  // MON = 1, TUE = 2....
    }
}

static void parse_weekprof(uint8_t* cp, week_date_type* parse)
{
    int i;

    packed_ptr = cp;
    packed_idx = 0;

    PARSE_ARRAY(parse->cnt);
    if (parse->cnt > WEEK_PROF_SIZE)
        parse->cnt = WEEK_PROF_SIZE;

    for (i = 0; i < parse->cnt; i++)
    {
        parse_weekprof_time(&parse->week[i]);
    }
}

static void parse_holidays(uint8_t* cp, holiday_date_type* parse)
{
    static uint16_t prev_blknum;
    int i;
    bool isfirst;
    int idx;
    uint16_t blksize, blknum, yr, t16;

    ((void)parse);

    isfirst = false;

    packed_ptr = cp;
    packed_idx = 0;

    // block size parsing
    packed_idx += parse_ext_len_2(packed_ptr, &blksize);

    if (set_req_is_first_block())  // first block -> bcz tou continue to
                                   // download
    {
        int lensize;

        lensize = parse_ext_len_2((packed_ptr + packed_idx + 1),
                                  &t16);  // 1 means Array_tag

        hol_date_block.arr_len = t16;
        packed_idx += (1 + lensize);

        ToH16((U8_16*)&t16, &packed_ptr[packed_idx + 3]);
        if ((hol_date_block.arr_len > 0) && ((t16 % HOLIDAYS_PER_BLOCK) != 0))
        {
            hol_date_block_restore();
        }
        else
        {
            memset((uint8_t*)&hol_date_block, 0xff, sizeof(holiday_date_type));
        }

        isfirst = true;
        prev_blknum = 0;
    }

    while (packed_idx < blksize)
    {
        packed_idx += 2;  // struct tag

        // index
        PARSE_U16(t16);
        blknum = t16 / HOLIDAYS_PER_BLOCK;
        idx = t16 % HOLIDAYS_PER_BLOCK;

        if (isfirst)
        {
            isfirst = false;
            prev_blknum = blknum;
        }

        if (blknum != prev_blknum)
        {
            nv_sub_info.ch[0] = prev_blknum;
            if (nv_write(I_HOLIDAYS_DL, (uint8_t*)&hol_date_block))
            {
                memset((uint8_t*)&hol_date_block, 0xff,
                       sizeof(holiday_date_type));
            }
            else
            {
                appl_set_save_result = 1;
                break;  // !!!!!! exit
            }
            prev_blknum = blknum;
        }

        // holiday dates
        packed_idx += 2;                              // octet string tag
        ToH16((U8_16*)&yr, &packed_ptr[packed_idx]);  // year
        if (yr != 0xffff)
        {
            hol_date_block.yr = (uint8_t)(yr - BASE_YEAR);
        }
        hol_date_block.holiday[idx].month =
            packed_ptr[packed_idx + 2];                                 // month
        hol_date_block.holiday[idx].date = packed_ptr[packed_idx + 3];  // date
        packed_idx += DATE_TAG_LEN;
        // day id
        PARSE_U8(hol_date_block.holiday[idx].day_id);
    }

    hol_date_block_save();

    if (appl_is_last_block)
    {
        nv_sub_info.ch[0] = prev_blknum;
        if (nv_write(I_HOLIDAYS_DL, (uint8_t*)&hol_date_block))
        {
            memset((uint8_t*)&hol_date_block, 0xff, sizeof(holiday_date_type));
        }
        else
        {
            appl_set_save_result = 1;
        }
    }

    if (appl_is_last_block || appl_set_save_result)
    {
        if (appl_set_save_result == 0)
        {
            i = prev_blknum + 1;
        }
        else
        {
            i = prev_blknum;
        }

        memset((uint8_t*)&hol_date_block, 0xff, sizeof(holiday_date_type));
        for (; i < HOLIDAY_BLOCK_LEN; i++)
        {
            nv_sub_info.ch[0] = i;
            nv_write(I_HOLIDAYS_DL, (uint8_t*)&hol_date_block);

            dsm_wdt_ext_toggle_immd();
        }
    }
}

static disp_mode_type suppdsp_month_energy_nprd(uint8_t* obis)
{
    rate_type tariff;
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    tariff = energy_group_to_tariff(obis[GROUP_E]);
    switch (obis[GROUP_C])
    {
    case 1:  // 순방향 유효 (수전)
        if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_ACT_A_NPRD; /* 비정기 누적 수전 유효전력량
                                                    (kWh)_경부하 */
        else
            dspmode = DSPMODE_BBF_IMP_ACT_A_NPRD;
        break;
    case 2:  // 역방향 유효 (송전)
        if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_ACT_A_NPRD; /* 비정기 누적 송전
                                                    유효전력량(kWh)_경부하 */
        else
            dspmode = DSPMODE_BBF_EXP_ACT_A_NPRD;
        break;
    default:  // to avoid compile warning
        dspmode = DSPMODE_BF_IMP_ACT_A_NPRD;
        break;
    }

    return (disp_mode_type)(dspmode + tariff);
}

static disp_mode_type suppdsp_month_energy_prd(uint8_t* obis)
{
    rate_type tariff;
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    tariff = energy_group_to_tariff(obis[GROUP_E]);
    switch (obis[GROUP_C])
    {
    case 1:  // 순방향 유효
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_ACT_A; /* 현월 누적 수전 유효전력량
                                               (kWh)_경부하 */
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_ACT_A; /* 전월 누적 수전 유효전력량
                                               (kWh)_경부하 */
        else
            dspmode = DSPMODE_BBF_IMP_ACT_A;
        break;
    case 2:  // 역방향 유효
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_ACT_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_ACT_A;
        else
            dspmode = DSPMODE_BBF_EXP_ACT_A;
        break;
    case 5:
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_lagREACT_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_lagREACT_A;
        else
            dspmode = DSPMODE_BBF_IMP_lagREACT_A;
        break;
    case 8:
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_leadREACT_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_leadREACT_A;
        else
            dspmode = DSPMODE_BBF_IMP_leadREACT_A;
        break;
    case 6:
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_leadREACT_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_leadREACT_A;
        else
            dspmode = DSPMODE_BBF_EXP_leadREACT_A;
        break;
    case 7:
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_lagREACT_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_lagREACT_A;
        else
            dspmode = DSPMODE_BBF_EXP_lagREACT_A;
        break;
    case 9:  // 순방향 피상
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_APP_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_APP_A;
        else
            dspmode = DSPMODE_BBF_IMP_APP_A;
        break;
    case 10:  // 역방향 피상
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_APP_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_APP_A;
        else
            dspmode = DSPMODE_BBF_EXP_APP_A;
        break;
    case 128:  // 순 방향 지상+진상 무효 => 무효는 선택 무효와 반드시 일치 해야
               // 함
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_REACT_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_REACT_A;
        else
            dspmode = DSPMODE_BBF_IMP_REACT_A;
        break;
    case 129:  // 역 방향 지상+진상 무효 => 무효는 선택 무효와 반드시 일치 해야
               // 함
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_REACT_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_REACT_A;
        else
            dspmode = DSPMODE_BBF_EXP_REACT_A;
        break;
    default:  // to avoid compile warning
        dspmode = DSPMODE_CU_IMP_ACT_A;
        break;
    }

    return (disp_mode_type)(dspmode + tariff);
}

static disp_mode_type suppdsp_month_pf(uint8_t* obis)
{
    rate_type tariff;
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    tariff = energy_group_to_tariff(obis[GROUP_E]);
    switch (obis[GROUP_C])
    {
    case 13:  // 순 방향 역률
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_PF_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_PF_A;
        else
            dspmode = DSPMODE_BBF_IMP_PF_A;
        break;
    case 84:  // 역 방향 역률
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_PF_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_PF_A;
        else
            dspmode = DSPMODE_BBF_EXP_PF_A;
        break;
    default:  // to avoid compile warning
        dspmode = DSPMODE_CU_IMP_PF_A;
        break;
    }

    return (disp_mode_type)(dspmode + tariff);
}

static disp_mode_type suppdsp_month_cumdm_nprd(uint8_t* obis)
{
    rate_type tariff;
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    tariff = energy_group_to_tariff(obis[GROUP_E]);
    switch (obis[GROUP_C])
    {
    case 1:
        if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_ACT_CUMMX_A_NPRD;
        else
            dspmode = DSPMODE_BBF_IMP_ACT_CUMMX_A_NPRD;
        break;
    default:
        dspmode = DSPMODE_BF_IMP_ACT_CUMMX_A_NPRD;
        break;
    }

    return (disp_mode_type)(dspmode + tariff);
}

static disp_mode_type suppdsp_month_cumdm_prd(uint8_t* obis)
{
    rate_type tariff;
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    tariff = energy_group_to_tariff(obis[GROUP_E]);
    switch (obis[GROUP_C])
    {
    case 1:  // 순 방향 유효
        if (obis[GROUP_F] == 255)
        {
            dspmode = DSPMODE_CU_IMP_ACT_CUMMX_A;
        }
        else if (obis[GROUP_F] == 101)
        {
            dspmode = DSPMODE_BF_IMP_ACT_CUMMX_A;
        }
        else
        {
            dspmode = DSPMODE_BBF_IMP_ACT_CUMMX_A;
        }
        break;
    case 9:  // 순 방향 피상
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_APP_CUMMX_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_APP_CUMMX_A;
        else
            dspmode = DSPMODE_BBF_IMP_APP_CUMMX_A;
        break;
    case 2:  // 역 방향 유효
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_ACT_CUMMX_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_ACT_CUMMX_A;
        else
            dspmode = DSPMODE_BBF_EXP_ACT_CUMMX_A;
        break;
    case 10:  // 역 방향 피상
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_APP_CUMMX_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_APP_CUMMX_A;
        else
            dspmode = DSPMODE_BBF_EXP_APP_CUMMX_A;
        break;
    default:  // to avoid compile warning
        dspmode =
            DSPMODE_CU_IMP_ACT_A; /* 현월 누적 수전 유효전력량 (kWh)_경부하 */
        break;
    }

    return (disp_mode_type)(dspmode + tariff);
}

static disp_mode_type suppdsp_month_mxdm(uint8_t* obis)
{
    rate_type tariff;
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    tariff = energy_group_to_tariff(obis[GROUP_E]);
    switch (obis[GROUP_C])
    {
    case 1:  // 순 방향 유효
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_ACT_MX_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_ACT_MX_A;
        else
            dspmode = DSPMODE_BBF_IMP_ACT_MX_A;
        break;
    case 9:  // 순 방향 피상
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_APP_MX_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_APP_MX_A;
        else
            dspmode = DSPMODE_BBF_IMP_APP_MX_A;
        break;
    case 2:  // 역 방향 유효
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_ACT_MX_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_ACT_MX_A;
        else
            dspmode = DSPMODE_BBF_EXP_ACT_MX_A;
        break;
    case 10:  // 역 방향 피상
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_APP_MX_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_EXP_APP_MX_A;
        else
            dspmode = DSPMODE_BBF_EXP_APP_MX_A;
        break;
    default:  // to avoid compile warning
        dspmode =
            DSPMODE_CU_IMP_ACT_A; /* 현월 누적 수전 유효전력량 (kWh)_경부하 */
        break;
    }

    return (disp_mode_type)(dspmode + tariff);
}

static disp_mode_type suppdsp_month_mxdm_time(uint8_t* obis)
{
    rate_type tariff;
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    tariff = energy_group_to_tariff(obis[GROUP_E]);
    switch (obis[GROUP_C])
    {
    case 1:  // 순 방향 유효
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_ACT_MXTIM_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_ACT_MXTIM_A;
        else
            dspmode = DSPMODE_BBF_IMP_ACT_MXTIM_A;
        break;
    case 9:  // 순 방향 피상
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_APP_MXTIM_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_APP_MXTIM_A;
        else
            dspmode = DSPMODE_BBF_IMP_APP_MXTIM_A;
        break;
    case 2:  // 역 방향 유효
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_EXP_ACT_MXTIM_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_ACT_MXTIM_A;
        else
            dspmode = DSPMODE_BBF_IMP_ACT_MXTIM_A;
        break;
    case 10:  // 역  방향 피상
        if (obis[GROUP_F] == 255)
            dspmode = DSPMODE_CU_IMP_APP_MXTIM_A;
        else if (obis[GROUP_F] == 101)
            dspmode = DSPMODE_BF_IMP_APP_MXTIM_A;
        else
            dspmode = DSPMODE_BBF_IMP_APP_MXTIM_A;
        break;
    default:  // to avoid compile warning
        dspmode = DSPMODE_CU_IMP_ACT_MXTIM_A;
        break;
    }

    return (disp_mode_type)(dspmode + tariff);
}

static disp_mode_type suppdsp_curr_dm(uint8_t* obis)
{
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (obis[GROUP_C])
    {
    case 1:  // 순 방향 유효
        dspmode = DSPMODE_CURR_IMP_ACT;
        break;
    case 9:  // 순 방향 피상
        dspmode = DSPMODE_CURR_IMP_APP;
        break;
    case 2:  // 역 방향 유효
        dspmode = DSPMODE_CURR_EXP_ACT;
        break;
    case 10:  // 역 방향 피상
        dspmode = DSPMODE_CURR_EXP_APP;
        break;
    default:  // to avoid compile warning
        dspmode =
            DSPMODE_CU_IMP_ACT_A; /* 현월 누적 수전 유효전력량 (kWh)_경부하 */
        break;
    }

    return dspmode;
}

static disp_mode_type suppdsp_rcnt_dm(uint8_t* obis)
{
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (obis[GROUP_C])
    {
    case 1:                             // 순 방향 유효
        dspmode = DSPMODE_LAST_IMP_ACT; /* 직전 수요전력(kW) */
        break;
    case 9:  // 순 방향 피상
        dspmode = DSPMODE_LAST_IMP_APP;
        break;
    case 2:  // 역 방향 유효
        dspmode = DSPMODE_LAST_EXP_ACT;
        break;
    case 10:  // 역 방향 피상
        dspmode = DSPMODE_LAST_EXP_APP;
        break;
    default:  // to avoid compile warning
        dspmode =
            DSPMODE_CU_IMP_ACT_A; /* 현월 누적 수전 유효전력량 (kWh)_경부하 */
        break;
    }

    return dspmode;
}

static disp_mode_type suppdsp_others(uint8_t* obis)
{
    bool ok;
    uint8_t i;
    uint8_t t8;
    uint8_t tbuf[6];
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    ok = false;
    i = 0;
    while (1)
    {
        memcpy(tbuf, &suppdsp_other_obis_table[i], 6);

        if (memcmp(tbuf, obis, 6) == 0)
        {
            ok = true;
            break;
        }

        i += 1;
        t8 = suppdsp_other_obis_table[i].id[0];
        if (t8 == 0xff)
        {
            break;
        }
    }

    if (ok)
    {
        dspmode = suppdsp_other_obis_table[i].dmode;
        return dspmode;
    }

    return DSPMODE_CU_IMP_ACT_A;  // uup !!! /* 현월 누적 수전 유효전력량
                                  // (kWh)_경부하 */
}

uint8_t GROUP_D_MON;
// 비정기 검침 전월
disp_mode_type dspmode_from_suppdsp_set(uint8_t* obis, uint8_t attid)
{
    /*
        디스플레이 관리자 순환표시 모드 관련 아래 통신 규격 참조할 것.
        3.4.1.3.3 계기 정보 (디스플레이용)
        3.4.2.8.2.1 관리자 순환표시 모드 가능 항목
    */
    disp_mode_type dspmode;

    DPRINTF(DBG_TRACE, _D "%s: OBIS %d %d %d %d %d %d, Attr %d\r\n", __func__,
            obis[GROUP_A], obis[GROUP_B], obis[GROUP_C], obis[GROUP_D],
            obis[GROUP_E], obis[GROUP_F], attid);

    if (obis[GROUP_A] == 1)
    {
        // DATE,  TIME
        if ((obis[GROUP_B] == 0) && (obis[GROUP_C] == 0) &&
            (obis[GROUP_D] == 9) && (obis[GROUP_E] == 2) &&
            (obis[GROUP_F] == 0XFF))
        {
            dspmode = DSPMODE_DATE;
        }
        else if ((obis[GROUP_B] == 0) && (obis[GROUP_C] == 0) &&
                 (obis[GROUP_D] == 9) && (obis[GROUP_E] == 1) &&
                 (obis[GROUP_F] == 0XFF))
        {
            dspmode = DSPMODE_TIME;
        }
#if 1 /* bccho, 2024-06-03 */
        else if ((obis[GROUP_B] == 0) && (obis[GROUP_C] == 0) &&
                 (obis[GROUP_D] == 2) && (obis[GROUP_E] == 0) &&
                 (obis[GROUP_F] == 0XFF))
        {
            dspmode = DSPMODE_TOU_PROG_ID;
        }
#endif
        else if ((obis[GROUP_B] == 1) || (obis[GROUP_B] == 0))
        {
            GROUP_D_MON = obis[GROUP_D];
            switch (obis[GROUP_D])
            {
            case 2:  // 월 누적 최대 수요
                dspmode = suppdsp_month_cumdm_prd(obis);
                break;

            case 4:  // 현재/직전 수요
                if (attid == 2)
                    dspmode = suppdsp_curr_dm(obis);
                else
                    dspmode = suppdsp_rcnt_dm(obis);
                break;

            case 6:  // 월 최대 수요
                if (attid == 2)
                    dspmode = suppdsp_month_mxdm(obis);
                else
                    dspmode = suppdsp_month_mxdm_time(obis);
                break;

            case 8:  // 월 순/역 방향 전력량
                dspmode = suppdsp_month_energy_prd(obis);
                break;

            case 9:  // 월 평균 역률
                dspmode = suppdsp_month_pf(obis);
                break;

            case /*0x1d*/ 29: /* LP 주기에 대한 직전 수전/송전 평균 역률 */
                if (obis[GROUP_C] == /*0x0d*/ 13)
                {
                    // 수전
                    dspmode = DSPMODE_LAST_IMP_PF;  // obis[GROUP_C] = 0x0d,
                                                    // obis[GROUP_E] = 0,
                                                    // obis[GROUP_F] = 255
                }
                else
                {
                    // 송전
                    dspmode = DSPMODE_LAST_EXP_PF;  // obis[GROUP_C] = 0x54,
                                                    // obis[GROUP_E] = 0,
                                                    // obis[GROUP_F] = 255
                }
                break;

            default:                            // to avoid compile warning
                dspmode = DSPMODE_CU_IMP_ACT_A; /* 현월 누적 수전 유효전력량
                                                   (kWh)_경부하 */
                break;
            }
        }
        else if ((obis[GROUP_B] == 2) && (obis[GROUP_D] == 8))
        {
            // 월 순/역 방향 전력량
            dspmode = suppdsp_month_energy_nprd(obis);
        }
        else if ((obis[GROUP_B] == 2) && (obis[GROUP_D] == 2))
        {
            // 월 순/역 방향 전력량
            dspmode = suppdsp_month_cumdm_nprd(obis);
        }
        else
        {
            dspmode = suppdsp_others(obis);
        }
    }
    else
    {
        dspmode = suppdsp_others(obis);
    }

    return dspmode;
}

static void touset_parse_bill_drsel(uint8_t* cp)
{
    touset_parse_idx = 0;

    cp += 2;  // skip struct tag

    cp += 2;  // skip octet string tag
    memcpy(&touset_parse_buf[touset_parse_idx], cp, OBIS_NAME_LEN);
    touset_parse_idx += OBIS_NAME_LEN;
    cp += OBIS_NAME_LEN;
    // script selector
    cp += 1;  // U16 tag
    touset_parse_buf[touset_parse_idx] = *cp;
    touset_parse_buf[touset_parse_idx + 1] = *(cp + 1);
    touset_parse_idx += 2;
}

static uint8_t* parse_bill_time_date(uint8_t* cp)
{
    // skip struct
    cp += 2;
    // parse time
    cp += 2;  // octet string tag
    memcpy(&touset_parse_buf[touset_parse_idx], cp, TIME_TAG_LEN);
    touset_parse_idx += TIME_TAG_LEN;
    cp += TIME_TAG_LEN;
    // parse date
    cp += 2;  // octet string tag
    memcpy(&touset_parse_buf[touset_parse_idx], cp, DATE_TAG_LEN);
    touset_parse_idx += DATE_TAG_LEN;
    cp += DATE_TAG_LEN;

    return cp;
}

static void touset_parse_pbilldate(uint8_t* cp)
{
    touset_parse_idx = 0;

    // skip array
    cp += 2;

    parse_bill_time_date(cp);
}

static void touset_parse_npBilldate(uint8_t* cp)
{
    int i;
    uint8_t cnt;

    touset_parse_idx = 0;

    // array
    cnt = *(cp + 1);
    cp += 2;
    if (cnt > NP_BILLDATE_LEN)
        cnt = NP_BILLDATE_LEN;

    for (i = 0; i < cnt; i++)
    {
        cp = parse_bill_time_date(cp);
    }
}

static uint8_t* parse_season_time_date(uint8_t* cp)
{
    uint8_t t8;

    // skip struct
    cp += 2;

    // season profile name
    cp += 1;  // octet string tag
    t8 = *cp++;
    memcpy(&touset_parse_buf[touset_parse_idx], cp, t8);
    touset_parse_idx += t8;
    cp += t8;
    // season start time
    cp += 1;  // octet string tag
    t8 = *cp++;
    memcpy(&touset_parse_buf[touset_parse_idx], cp, t8);
    touset_parse_idx += t8;
    cp += t8;
    // week name
    cp += 1;  // octet string tag
    t8 = *cp++;
    memcpy(&touset_parse_buf[touset_parse_idx], cp, t8);
    touset_parse_idx += t8;
    cp += t8;

    return cp;
}

static void touset_parse_seasonprof(uint8_t* cp)
{
    int i;
    uint8_t cnt;

    touset_parse_idx = 0;

    // array
    cnt = *(cp + 1);
    cp += 2;
    if (cnt > SEASON_PROF_SIZE)
        cnt = SEASON_PROF_SIZE;

    for (i = 0; i < cnt; i++)
    {
        cp = parse_season_time_date(cp);
    }
}

static uint8_t* parse_weekprof_table(uint8_t* cp)
{
    int i;
    uint8_t t8;

    // skip struct
    cp += 2;

    // week profile name
    cp += 1;  // octet string tag
    t8 = *cp++;
    memcpy(&touset_parse_buf[touset_parse_idx], cp, t8);
    touset_parse_idx += t8;
    cp += t8;
    // week dayid
    for (i = 0; i < WEEK_LEN; i++)
    {
        cp += 1;  // U8 tag
        touset_parse_buf[touset_parse_idx++] = *cp++;
    }

    return cp;
}

static void touset_parse_weekprof(uint8_t* cp)
{
    int i;
    uint8_t cnt;

    touset_parse_idx = 0;

    // array
    cnt = *(cp + 1);
    cp += 2;
    if (cnt > WEEK_PROF_SIZE)
        cnt = WEEK_PROF_SIZE;

    for (i = 0; i < cnt; i++)
    {
        cp = parse_weekprof_table(cp);
    }
}

static uint8_t* parse_dayprof_action(uint8_t* cp)
{
    cp += 2;  // skip struct

    // start time
    cp += 2;  // octet string tag
    memcpy(&touset_parse_buf[touset_parse_idx], cp, TIME_TAG_LEN);
    touset_parse_idx += TIME_TAG_LEN;
    cp += TIME_TAG_LEN;
    // script logical name
    cp += 2;  // skip octet string
    memcpy(&touset_parse_buf[touset_parse_idx], cp, OBIS_NAME_LEN);
    touset_parse_idx += OBIS_NAME_LEN;
    cp += OBIS_NAME_LEN;
    // script selector
    cp += 1;  // U16 tag
    touset_parse_buf[touset_parse_idx] = *cp;
    touset_parse_buf[touset_parse_idx + 1] = *(cp + 1);
    touset_parse_idx += 2;
    cp += 2;

    return cp;
}

static void touset_parse_dayprof(uint8_t* cp)
{
    int i;
    uint8_t arrcnt, *pst;
    uint16_t blkidx, blksize;

    touset_parse_idx = 0;
    blkidx = 0;

    // block size parsing
    cp += parse_ext_len_2(cp, &blksize);

    if (set_req_is_first_block())  // first block -> bcz tou continue to
                                   // download
    {
        int lensize;

        // array parsing -> just skip
        cp += 1;  // tag
        blkidx += 1;

        lensize = size_ext_len_2(cp);
        cp += lensize;
        blkidx += lensize;
    }

    while (blkidx < blksize)
    {
        cp += 2;
        blkidx += 2;

        // day id
        cp += 1;  // U8 tag
        touset_parse_buf[touset_parse_idx] = *cp;
        touset_array_entry_idx = *cp;  // !!!! array entry index
        touset_parse_idx += 1;
        cp += 1;
        blkidx += 2;

        pst = cp;

        // day_profile_action

        // array
        arrcnt = *(cp + 1);
        cp += 2;
        if (arrcnt > MAX_TOU_DIV_DLMS)
            arrcnt = MAX_TOU_DIV_DLMS;

        for (i = 0; i < arrcnt; i++)
        {
            cp = parse_dayprof_action(cp);
        }

        blkidx += (cp - pst);
    }
}

static void touset_parse_activate_passive(uint8_t* cp)
{
    uint8_t len;

    touset_parse_idx = 0;

    cp += 1;  // octet string tag
    len = *cp++;

    memcpy(&touset_parse_buf[touset_parse_idx], cp, len);
    touset_parse_idx += len;
}

static void touset_parse_holidays(uint8_t* cp)
{
    uint16_t t16;
    uint16_t blkidx, blksize;

    touset_parse_idx = 0;
    blkidx = 0;

    // block size parsing
    cp += parse_ext_len_2(cp, &blksize);

    // if(set_req_is_first_block() && (tou_set_cnt.last_arrayidx == 0xffff)) //
    // first block -> bcz tou continue to download
    if (set_req_is_first_block())  // first block -> bcz tou continue to
                                   // download
    {
        int lensize;

        // array parsing -> just skip
        cp += 1;  // tag
        blkidx += 1;

        lensize = size_ext_len_2(cp);
        cp += lensize;
        blkidx += lensize;
    }

    while (blkidx < blksize)
    {
        cp += 2;  // struct tag
        blkidx += 2;

        // index
        cp += 1;  // U16 tag
        blkidx += 1;
        touset_parse_buf[touset_parse_idx] = *cp;
        touset_parse_buf[touset_parse_idx + 1] = *(cp + 1);

        ToH16((U8_16*)&t16, cp);
        touset_array_entry_idx = t16;  // !!!! array entry index

        touset_parse_idx += 2;
        cp += 2;
        blkidx += 2;

        // special day
        cp += 2;  // octet string tag
        blkidx += 2;
        memcpy(&touset_parse_buf[touset_parse_idx], cp, DATE_TAG_LEN);
        touset_parse_idx += DATE_TAG_LEN;
        cp += DATE_TAG_LEN;
        blkidx += DATE_TAG_LEN;

        // day id
        cp += 1;  // U8 tag
        blkidx += 1;
        touset_parse_buf[touset_parse_idx] = *cp;
        touset_parse_idx += 1;
        cp += 1;
        blkidx += 1;
    }
}

static void touset_parse_supplydsp_mode(uint8_t* cp)
{
    int i;
    uint8_t cnt;

    touset_parse_idx = 0;

    // array
    cnt = *(cp + 1);
    cp += 2;
    if (cnt > MAX_SUPP_DSP_NUM)
        cnt = MAX_SUPP_DSP_NUM;

    for (i = 0; i < cnt; i++)
    {
        cp += 2;  // struct tag

        // class id
        cp += 1;  // U16 tag
        touset_parse_buf[touset_parse_idx] = *cp;
        touset_parse_buf[touset_parse_idx + 1] = *(cp + 1);

        touset_parse_idx += 2;
        cp += 2;
        // obis
        cp += 2;  // octet string
        memcpy(&touset_parse_buf[touset_parse_idx], cp, OBIS_NAME_LEN);
        touset_parse_idx += OBIS_NAME_LEN;
        cp += OBIS_NAME_LEN;
        // attribute id
        cp += 1;  // tag
        touset_parse_buf[touset_parse_idx] = *cp;
        touset_parse_idx += 1;
        cp += 1;
        // dsp time
        cp += 1;  // tag
        touset_parse_buf[touset_parse_idx] = *cp;
        touset_parse_idx += 1;
        cp += 1;
    }
}

static void touset_parse_tsconf_att2(uint8_t* cp)
{
    DPRINTF(DBG_TRACE, "%s \r\n", __func__);
    touset_parse_idx = 0;

    cp += 2;  // skip struct

    cp += 2;  // skip octet string
    memcpy(&touset_parse_buf[touset_parse_idx], cp, OBIS_NAME_LEN);
    touset_parse_idx += OBIS_NAME_LEN;
    cp += OBIS_NAME_LEN;

    cp += 1;  // U16 tag
    touset_parse_buf[touset_parse_idx] = *cp;
    touset_parse_buf[touset_parse_idx + 1] = *(cp + 1);
    touset_parse_idx += 2;
    cp += 2;
}

static void touset_parse_tsconf_att4(uint8_t* cp)
{
    int i;
    uint8_t cnt;
    DPRINTF(DBG_TRACE, "%s \r\n", __func__);

    touset_parse_idx = 0;

    // array
    cnt = *(cp + 1);
    cp += 2;
    if (cnt > TS_ZONE_SIZE)
        cnt = TS_ZONE_SIZE;

    for (i = 0; i < cnt; i++)
    {
        cp += 2;  // struct tag
        // time
        cp += 2;  // octet string tag
        memcpy(&touset_parse_buf[touset_parse_idx], cp, TIME_TAG_LEN);
        touset_parse_idx += TIME_TAG_LEN;
        cp += TIME_TAG_LEN;
        // parse date
        cp += 2;  // octet string tag
        memcpy(&touset_parse_buf[touset_parse_idx], cp, DATE_TAG_LEN);
        touset_parse_idx += DATE_TAG_LEN;
        cp += DATE_TAG_LEN;
    }
}

static void touset_parse_daily_meas_sched(uint8_t* cp)
{
    int i;
    uint8_t cnt;

    touset_parse_idx = 0;

    // array
    cnt = *(cp + 1);
    cp += 2;
    if (cnt > MAX_TOU_DIV_DLMS)
        cnt = MAX_TOU_DIV_DLMS;

    for (i = 0; i < cnt; i++)
    {
        cp += 2;  // struct tag
        // time
        cp += 2;  // octet string tag
        memcpy(&touset_parse_buf[touset_parse_idx], cp, TIME_TAG_LEN);
        touset_parse_idx += TIME_TAG_LEN;
        cp += TIME_TAG_LEN;

        cp += 2;  // skip octet string
        memcpy(&touset_parse_buf[touset_parse_idx], cp, OBIS_NAME_LEN);
        touset_parse_idx += OBIS_NAME_LEN;
        cp += OBIS_NAME_LEN;

        // selector
        cp += 1;  // U16 tag
        touset_parse_buf[touset_parse_idx] = *cp;
        touset_parse_buf[touset_parse_idx + 1] = *(cp + 1);
        touset_parse_idx += 2;
        cp += 2;
    }
}

static void touset_parse_date_time(uint8_t* cp)
{
    touset_parse_idx = 0;

    // date_time
    cp += 2;  // octet string tag
    memcpy(&touset_parse_buf[touset_parse_idx], cp, DATE_TIME_TAG_LEN);
    touset_parse_idx += DATE_TIME_TAG_LEN;
    cp += DATE_TIME_TAG_LEN;
}

// --------------- each object set ----------------------
static void obset_ass_ln(int idx)
{
    uint8_t len;
    auth_pwd_type auth;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    // id_att (7) ==> set LLS password
    if (appl_msg[idx] == OCTSTRING_TAG)
    {
        len = appl_msg[idx + 1];
        if (len >= 8 && len <= 12)
        {
            if (act_is_progdl_cmd())
            {
                prog_dl.pwd.len = len;
                memcpy(prog_dl.pwd.pwd, &appl_msg[idx + 2], (uint16_t)len);

                pdl_set_bits |= SETBITS_PWD_CHG;

                touset_array_entry_idx = 0xffff;
                touset_parse_idx = len;
                memcpy(touset_parse_buf, prog_dl.pwd.pwd, len);

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                auth.pwd.len = len;
                memcpy(auth.pwd.pwd, &appl_msg[idx + 2], (uint16_t)len);

                if (appl_is_sap_utility())
                {
                    nv_write(I_UTIL_PASSWORD, (uint8_t*)&auth);
                    appl_util_pwd = auth.pwd;
                }
                else if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
                {
                }
                else
                {
                    nv_write(I_485_PASSWORD, (uint8_t*)&auth);
                    appl_485_pwd = auth.pwd;
                }

                lp_event_set(LPE_PROGRAM_CHG);
            }
            return;
        }
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_date_time(int idx)
{
    date_time_type dt;

    DPRINTF(DBG_TRACE, _D "%s: appl_att_id[%d]\r\n", __func__, appl_att_id);

    switch (appl_att_id)
    {
    case 2: /* 통신 규격 3.4.2.8.1 현재 일자/시간 */
        if (get_date_time_from_comm(&dt, 0, &appl_msg[idx]))
        {
            rtc_chg_proc(&dt, &global_buff[0]);

            memcpy((uint8_t*)&pwf_date_time, (uint8_t*)&cur_rtc,
                   sizeof(date_time_type));
            nv_write(I_PWF_DATE_TIME, (uint8_t*)&pwf_date_time);
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;

    case 5:
        if (get_date_time_from_comm(&prog_dl.dlsinfo.bgn_dt,
                                    &prog_dl.dlsinfo.bgn_week, &appl_msg[idx]))
        {
            pdl_set_bits |= SETBITS_DLS_BGN;

            touset_parse_date_time(&appl_msg[idx]);
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;

    case 6:
        if (get_date_time_from_comm(&prog_dl.dlsinfo.end_dt,
                                    &prog_dl.dlsinfo.end_week, &appl_msg[idx]))
        {
            pdl_set_bits |= SETBITS_DLS_END;

            touset_parse_date_time(&appl_msg[idx]);
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;

    case 7:
        if (appl_msg[idx] == INTEGER_TAG)
        {
            prog_dl.dlsinfo.dev = appl_msg[idx + 1];
            pdl_set_bits |= SETBITS_DLS_DEV;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 1;
            touset_parse_buf[0] = prog_dl.dlsinfo.dev;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;

    case 8:
        if (appl_msg[idx] == BOOLEAN_TAG)
        {
            prog_dl.dlsinfo.enabled = appl_msg[idx + 1];
            pdl_set_bits |= SETBITS_DLS_ENA;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 1;
            touset_parse_buf[0] = prog_dl.dlsinfo.enabled;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;
    }
}

static void obset_sig_sel(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == UNSIGNED_TAG)
    {
        if (act_is_progdl_cmd())
        {
            prog_dl.out_sig = appl_msg[idx + 1];
            pdl_set_bits |= SETBITS_SIG_SEL;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 1;
            touset_parse_buf[0] = prog_dl.out_sig;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            sig_sel_proc(appl_msg[idx + 1]);
            lp_event_set(LPE_PROGRAM_CHG);
        }
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_mdm_baud(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == ENUM_TAG)
    {
        prog_dl.baud = (baudrate_type)appl_msg[idx + 1];
        pdl_set_bits |= SETBITS_MODEM_BAUD;
        if (pdl_set_bits & SETBITS_MODEM_BAUD)
        {
            pdl_set_bits &= ~SETBITS_MODEM_BAUD;

            mdm_conf_delay_until_txcomplete = true;
            mdm_baud = prog_dl.baud;
        }
        lp_event_set(LPE_PROGRAM_CHG);
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static bool obis_EOB_is_ok(uint8_t* cp)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    // OBJ_ENDOF_BILLING check
    if (*(cp + 0) == 0x00 && *(cp + 1) == 0x00 && *(cp + 2) == 0x0a &&
        *(cp + 3) == 0x00 && *(cp + 4) == 0x01 && *(cp + 5) == 0xff)
        return true;

    return false;
}

static bool parse_bill_drsel(uint8_t* cp, uint8_t* drsel)
{
    cp += 2;  // skip struct tag

    cp += 2;  // skip octet string tag

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (obis_EOB_is_ok(cp))
    {
        cp += OBIS_ID_SIZE;
        if (*cp == LONGUNSIGNED_TAG)
        {
            *drsel = *(cp + 2) & 0x07;  // just low byte
            return true;
        }
    }

    return false;
}

static void obset_period_billdate(int idx)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x02:  // [executed_script]
        if (parse_bill_drsel(&appl_msg[idx], &t8))
        {
            if (act_is_progdl_cmd())
            {
                t8 = conv_sel_to_DRkind(t8);
                prog_dl.pEOB.dr_sel = t8;  // just get available low byte
                pdl_set_bits |= SETBITS_PBILL_DRSEL;

                touset_parse_bill_drsel(&appl_msg[idx]);
                touset_array_entry_idx = 0xffff;

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                set_pEOB_dr(t8);
                lp_event_set(LPE_PROGRAM_CHG);
            }
            break;
        }
        appl_resp_result = SET_RESULT_DATA_NG;
        break;

    case 0x03:  // [type]
                // tag = ENUM_TAG (type = execution time)
        if (act_is_progdl_cmd())
        {
            prog_dl.pEOB.type = appl_msg[idx + 1];
            pdl_set_bits |= SETBITS_PBILL_DRTYPE;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 1;
            touset_parse_buf[0] = appl_msg[idx + 1];

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            lp_event_set(LPE_PROGRAM_CHG);
        }
        break;

    case 0x04:  // [executed_time]
                // ==> TAG check is ignored
        t8 = appl_msg[idx + 15];
        if (act_is_progdl_cmd())
        {
            prog_dl.regread_date = t8;
            pdl_set_bits |= SETBITS_PBILL_DATE;

            touset_parse_pbilldate(&appl_msg[idx]);
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            if (reg_mr_date != t8)
                tou_id_change_sts = 1;

            reg_mr_date = t8;
            lp_event_set(LPE_PROGRAM_CHG);

            dsm_progname_update_forReport();
        }
        break;
    }
}

static void obset_nperiod_billdate(int idx)
{
    uint8_t t8;
    npbill_date_type* npbill;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x02:  // [executed_script]
        if (parse_bill_drsel(&appl_msg[idx], &t8))
        {
            if (act_is_progdl_cmd())
            {
                t8 = conv_sel_to_DRkind(t8);
                prog_dl.npEOB.dr_sel = t8;
                pdl_set_bits |= SETBITS_NPBILL_DRSEL;

                touset_parse_bill_drsel(&appl_msg[idx]);
                touset_array_entry_idx = 0xffff;

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                set_npEOB_dr(t8);
                lp_event_set(LPE_PROGRAM_CHG);
            }
            break;
        }
        appl_resp_result = SET_RESULT_DATA_NG;
        break;

    case 0x03:  // [type]
        if (act_is_progdl_cmd())
        {
            prog_dl.npEOB.type = appl_msg[idx + 1];
            pdl_set_bits |= SETBITS_NPBILL_DRTYPE;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 1;
            touset_parse_buf[0] = appl_msg[idx + 1];

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            lp_event_set(LPE_PROGRAM_CHG);
        }
        break;

    case 0x04:  // [execution_time]
        npbill = (npbill_date_type*)appl_tbuff;
        parse_npBilldate(&appl_msg[idx], npbill);
        DPRINTF(DBG_TRACE, _D "%s: act_is_progdl_cmd %d\r\n", __func__,
                act_is_progdl_cmd());
        if (act_is_progdl_cmd() == false)
        {
            currprog_available_bits |= PROG_npBILL_DATE_BIT;

            if (nv_write(I_NP_BILLDATE_A, appl_tbuff))
            {
                npBill_date_load();
                lp_event_set(LPE_PROGRAM_CHG);
            }
            else
            {
                appl_resp_result = SET_RESULT_REQ_NG;
            }
        }
        else
        {
            if (nv_write(I_NP_BILLDATE_DL, appl_tbuff))
            {
                pdl_set_bits |= SETBITS_NPBILL_DATE;

                touset_parse_npBilldate(&appl_msg[idx]);
                touset_array_entry_idx = 0xffff;

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                appl_resp_result = SET_RESULT_REQ_NG;
            }
        }
        break;
    }
}

static void obset_billing_parm(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == OCTSTRING_TAG &&
        appl_msg[idx + 1] == BILLING_PARM_SIZE)
    {
        memcpy(prog_dl.bill_parm, &appl_msg[idx + 2], BILLING_PARM_SIZE);
        if (act_is_progdl_cmd())
        {
            pdl_set_bits |= SETBITS_BILLING_PARM;

            memcpy(touset_parse_buf, prog_dl.bill_parm, BILLING_PARM_SIZE);
            touset_parse_idx = BILLING_PARM_SIZE;
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            set_billing_parm(prog_dl.bill_parm);
            lp_event_set(LPE_PROGRAM_CHG);
        }
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

void mif_meter_parm_set(void)
{
    ST_MTP_PARM st_mtp_parm;
    float fval;
    ST_MIF_METER_PARM* pst_mif_meter_parm = dsm_mtp_get_meter_parm();

    ToHFloat((U8_Float*)&fval, &pst_mif_meter_parm->cut_voltage_thr[0]);
    DPRINTF(DBG_TRACE, _D "cut_vol_thres: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));

    DPRINTF(DBG_TRACE, _D "mise hex: %02X:%02X:%02X:%02X\r\n",
            pst_mif_meter_parm->direct_reverse,
            pst_mif_meter_parm->reactive_select,
            pst_mif_meter_parm->meter_method, pst_mif_meter_parm->pulse_select);
    memcpy((uint8_t*)&st_mtp_parm.val, (uint8_t*)pst_mif_meter_parm,
           sizeof(ST_MIF_METER_PARM));

    ToHFloat((U8_Float*)&fval, &pst_mif_meter_parm->cut_voltage_thr[0]);
    DPRINTF(DBG_TRACE, _D "cut_vol_thres: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));

    nv_write(I_MTP_PARM, (uint8_t*)&st_mtp_parm);

    /* bccho, 2024-09-24 */
    if (!nv_read(I_MTP_PARM, (uint8_t*)&st_mtp_parm))
    {
        dsm_mtp_default_parm(&st_mtp_parm);
    }

#if 0
		dsm_mif_setreq_meter_setup_parm((UINT8*)&st_mtp_parm.val, sizeof(ST_MIF_METER_PARM));
#else
    dsm_mtp_set_op_mode(MTP_OP_NORMAL);
    dsm_mtp_set_fsm(MTP_FSM_PARM_SET);
    dsm_mtp_fsm_send();
#endif

#if 1  // jp.kim 25.02.04
    mif_meter_para_set_wait_seq_cnt = 0;
    mif_meter_para_set_wait_ing = 1;
    mif_meter_para_set_wait_timeset(T500MS);
#endif
}

void MmodeCHG_sr_dr_type_sr_dr_is_proc(void)
{
    uint8_t eob_type = 0;
    if (MmodeCHG_sr_dr_type_sr_dr_is_timeout() &&
        MmodeCHG_sr_dr_type_sr_dr_run_pending_sts)
    {
        MmodeCHG_sr_dr_type_sr_dr_run_pending_sts = 0;

        if (MmodeCHG_sr_dr_type)
            eob_type = EOB_nPERIOD_FLAG;

        sr_dr_proc(eob_type, MmodeCHG_sr_dr_type, &cur_rtc, appl_tbuff);

        lp_event_set(LPE_PROGRAM_CHG);
    }
}

static void obset_selective_act(int idx)
{
    uint8_t _mtdir;
    uint8_t eob_type = 0;
    ST_MIF_METER_PARM* pst_mif_meter_parm = dsm_mtp_get_meter_parm();
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == OCTSTRING_TAG && appl_msg[idx + 1] == 1)
    {
        _mtdir = appl_msg[idx + 2] & 0x01;  // 0: 수전, 1: 송수전
        if (mt_dir != _mtdir)
        {
            mt_dir = _mtdir;

            meas_method_adj_dirchg();

            pst_mif_meter_parm->direct_reverse = mt_dir;
            pst_mif_meter_parm->meter_method = meas_method;

            mif_meter_parm_set();

            MmodeCHG_sr_dr_type_sr_dr_time_set(T500MS);
            MmodeCHG_sr_dr_type_sr_dr_run_pending_sts = 1;
#if 0
            if (MmodeCHG_sr_dr_type)
                eob_type = EOB_nPERIOD_FLAG;
            sr_dr_proc(eob_type, MmodeCHG_sr_dr_type, &cur_rtc, appl_tbuff);
#endif
        }
#if 0        
        lp_event_set(LPE_PROGRAM_CHG);
#endif
        dsm_progname_update_forReport();
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_lp_interval(int idx)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == UNSIGNED_TAG)
    {
        t8 = appl_msg[idx + 1];
        if (act_is_progdl_cmd())
        {
            prog_dl.lp_intv = t8;
            pdl_set_bits |= SETBITS_LP_INTV;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 1;
            touset_parse_buf[0] = appl_msg[idx + 1];

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            lp_interval = t8;
            lp_event_set(LPE_PROGRAM_CHG);

            {
                lp_event_set(LPE_IRRG_WR);
                LP_save(&cur_rtc);
                LP_nextdt_set(&cur_rtc);
            }
        }
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_lpavg_interval(int idx)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == UNSIGNED_TAG)
    {
        t8 = appl_msg[idx + 1];
        if (act_is_progdl_cmd())
        {
            prog_dl.lpavg_intv = t8;
            pdl_set_bits |= SETBITS_LPAVG_INTV;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 1;
            touset_parse_buf[0] = appl_msg[idx + 1];

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            LPavg_save(&cur_rtc);
            LPavg_init();

            lpavg_interval = t8;
            lp_event_set(LPE_PROGRAM_CHG);
        }
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_cur_last_dm(int idx)
{
    uint8_t* pt8;
    uint32_t t32 = 0;

    uint16_t t16;
    uint8_t eob_type = 0;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x08)  // [period]
    {
        if (appl_msg[idx] == UDLONG_TAG)
        {
            pt8 = &appl_msg[idx + 1];
            ToH32((U8_16_32*)&t32, pt8);
            if (act_is_progdl_cmd())
            {
                prog_dl.dm_prd = (uint8_t)(t32 / 60);

                pdl_set_bits |= SETBITS_DM_PRD;

                touset_array_entry_idx = 0xffff;
                touset_parse_idx = 4;
                memcpy(&touset_parse_buf[0], &appl_msg[idx + 1], 4);

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                if (dm_sub_interval != (uint8_t)(t32 / 60))
                {
                    if (dmintvCHG_sr_dr_type)
                        eob_type = EOB_nPERIOD_FLAG;
                    sr_dr_proc(eob_type, dmintvCHG_sr_dr_type, &cur_rtc,
                               appl_tbuff);
                    dm_sub_interval = (uint8_t)(t32 / 60);
                    dm_period_num = 1;
                    dm_interval = dm_sub_interval * dm_period_num;
                }
                lp_event_set(LPE_PROGRAM_CHG);
            }

            return;
        }
    }
    else if (appl_att_id == 0x09)  // [number_of_periods]
    {
        if (appl_msg[idx] == LONGUNSIGNED_TAG)
        {
            pt8 = &appl_msg[idx + 1];
            ToH16((U8_16*)&t16, pt8);
            prog_dl.dm_prd_num = (uint8_t)t16;

            pdl_set_bits |= SETBITS_DM_PRD_NUM;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 2;
            touset_parse_buf[0] = appl_msg[idx + 1];
            touset_parse_buf[1] = appl_msg[idx + 2];

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }

            return;
        }
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_tou_cal(int idx)
{
    uint8_t t8;
    season_date_type* season;
    week_date_type* week;
    dayid_table_type* dayid;
    bool curr_prog;
    program_info_type proginfo_data;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    curr_prog = 0;
    switch (appl_att_id)
    {
    case 0x02:  // 현재 미래 동시 설정되지 않음
        /* calendar name active */
        curr_prog = true;
    case 0x06:  // 현재 미래 동시 설정되지 않음
                /* calendar name passive */
        if (appl_msg[idx] == OCTSTRING_TAG)
        {
            memset(prog_dl.pgm_name, ' ', PROG_ID_SIZE);

            t8 = appl_msg[idx + 1];
            t8 = (t8 < PROG_ID_SIZE) ? t8 : PROG_ID_SIZE;
            memcpy(prog_dl.pgm_name, &appl_msg[idx + 2], t8);
            if (!curr_prog && !act_is_progdl_cmd())
            {
                uint8_t prg_name[9] = {0};
                memcpy(prg_name, prog_dl.pgm_name, PROG_ID_SIZE);
                DPRINTF(DBG_INFO, "FUT_PROG_NAME_SET: %s\r\n", prg_name);
                memcpy(proginfo_data.name, prog_dl.pgm_name, PROG_ID_SIZE);
                nv_sub_info.ch[0] = 1;
                nv_write(I_PROG_INFO, (uint8_t*)&proginfo_data);
#ifdef TOU_INDIVIDUAL_RESERVATION_NAME_FIX /* bccho, 2023-11-14, undefine */
                pdl_set_bits |= SETBITS_PGM_NAME;
#endif
            }
            else
            {
                uint8_t prg_name[9] = {0};
                memcpy(prg_name, prog_dl.pgm_name, PROG_ID_SIZE);
                DPRINTF(DBG_INFO, "CUR_PROG_NAME_SET: %s\r\n", prg_name);
                pdl_set_bits |= SETBITS_PGM_NAME;

                touset_parse_idx = t8;
                memcpy(&touset_parse_buf[0], &appl_msg[idx + 2], t8);
                touset_array_entry_idx = 0xffff;

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;

    case 0x03: /* season profile active */
    case 0x07: /* season profile passive */
        season = (season_date_type*)appl_tbuff;
        parse_seasonprof(&appl_msg[idx], season);
        if (nv_write(I_SEASON_PROFILE_DL, appl_tbuff))
        {
            pdl_set_bits |= SETBITS_TOU_SEASON;

            touset_parse_seasonprof(&appl_msg[idx]);
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_REQ_NG;
        }
        break;

    case 0x04: /* week profile table active */
    case 0x08: /* week profile table passive */
        week = (week_date_type*)appl_tbuff;
        parse_weekprof(&appl_msg[idx], week);
        if (nv_write(I_WEEK_PROFILE_DL, appl_tbuff))
        {
            pdl_set_bits |= SETBITS_TOU_WEEK;

            touset_parse_weekprof(&appl_msg[idx]);
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_REQ_NG;
        }
        break;

    case 0x05: /* day profile table active */
    case 0x09: /* day profile table passive */
        dayid = (dayid_table_type*)appl_tbuff;
        parse_dayprof(&appl_msg[idx], dayid);
        if (!appl_set_save_result)
        {
            if (appl_is_last_block)
            {
                pdl_set_bits |= SETBITS_TOU_DAY;
            }

            touset_parse_dayprof(&appl_msg[idx]);
            if (appl_is_last_block)
            {
                touset_array_entry_idx =
                    0xffff;  // overwirte in case of last block
            }

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;

    case 0x0a: /* active passive calendar time */
        if (get_date_time_from_comm(&prog_dl.active_passive_time, 0,
                                    &appl_msg[idx]))
        {
            date_time_type dt;
            if (!act_is_progdl_cmd())
            {
                if (sel_react_mon != 0)
                {
                    dt.year = sel_react_yr;
                    dt.month = sel_react_mon;
                    dt.date = 31;
                    if (cmp_date(&prog_dl.active_passive_time, &dt) <= 0)
                    {
                        break;
                    }
                }

                fut_prog_work_time = prog_dl.active_passive_time;
            }

            else
            {
                pdl_set_bits |= SETBITS_PAS_TIME;

                touset_parse_activate_passive(&appl_msg[idx]);
                touset_array_entry_idx =
                    0xffff;  // overwirte in case of last block

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;
    }
}

static void obset_holidays(int idx)
{
    holiday_date_type* holiday;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    holiday = (holiday_date_type*)appl_tbuff;
    parse_holidays(&appl_msg[idx], holiday);
    if (!appl_set_save_result)
    {
        {
            if (appl_is_last_block)
            {
                pdl_set_bits |= SETBITS_HOLIDAYS;
            }

            touset_parse_holidays(&appl_msg[idx]);
            if (appl_is_last_block)
            {
                touset_array_entry_idx =
                    0xffff;  // overwirte in case of last block
            }

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
    }
    else
    {
        appl_resp_result = SET_RESULT_REQ_NG;
    }
}

static uint8_t tdsptime;
static disp_mode_type parse_suppdsp_mode(void)
{
    uint8_t _attid;
    uint8_t _obis[OBIS_NAME_LEN];

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    packed_idx += 2;  // struct tag
    packed_idx += 3;  // classid(data16_tag_type)
    // obis
    packed_idx += 2;  // octet string
    memcpy(_obis, &packed_ptr[packed_idx], OBIS_NAME_LEN);
    packed_idx += OBIS_NAME_LEN;
    // attribute id
    packed_idx += 1;  // tag
    _attid = packed_ptr[packed_idx];
    packed_idx += 1;
    // dsp time
    packed_idx += 1;  // tag
    tdsptime = packed_ptr[packed_idx];
    packed_idx += 1;

    return dspmode_from_suppdsp_set(_obis, _attid);
}

static void obset_dsp_supply_mode(int idx)
{
    int i;
    dsp_supply_type* suppdsp;

    suppdsp = (dsp_supply_type*)appl_tbuff;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    packed_ptr = &appl_msg[idx];
    packed_idx = 0;

    PARSE_ARRAY(suppdsp->mode_cnt);
    if (suppdsp->mode_cnt > MAX_SUPP_DSP_NUM)
    {
        suppdsp->mode_cnt = MAX_SUPP_DSP_NUM;
    }

    if (suppdsp->mode_cnt == 0)
    {
        return;  // in case of CTT
    }

    for (i = 0; i < suppdsp->mode_cnt; i++)
    {
        suppdsp->dsp_mode[i] = (uint16_t)parse_suppdsp_mode();
    }
    suppdsp->dsptime = tdsptime;

    if (act_is_progdl_cmd())
    {
        if (nv_write(I_DISP_SUPP_DL, (uint8_t*)suppdsp))
        {
            pdl_set_bits |= SETBITS_SUPPDSP_ITEM;

            touset_parse_supplydsp_mode(&appl_msg[idx]);
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_REQ_NG;
        }
    }
    else
    {
        if (nv_write(I_DISP_SUPP_A, (uint8_t*)suppdsp))
        {
            memcpy((uint8_t*)&circ_state_suppdsp_mode, (uint8_t*)suppdsp,
                   sizeof(dsp_supply_type));
            currprog_available_bits |= PROG_SUPP_DISP_BIT;

            lcd_dsp_mode = LCD_SUPP_DSPMODE;

            dsp_circ_state_mode_init();

            lp_event_set(LPE_PROGRAM_CHG);
        }
        else
        {
            appl_resp_result = SET_RESULT_REQ_NG;
        }
    }

    nv_sub_info.seg.offset = 0;
    nv_sub_info.seg.len = appl_len - idx;
    if (act_is_progdl_cmd())
    {
        if (!nv_write(I_SUPPDSP_ITEM_DL, &appl_msg[idx]))
        {
            appl_resp_result = SET_RESULT_REQ_NG;
        }
    }
    else
    {
        if (!nv_write(I_SUPPDSP_ITEM_A, &appl_msg[idx]))
        {
            appl_resp_result = SET_RESULT_REQ_NG;
        }
    }
}

static void obset_dsp_pvt_mode(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == BOOLEAN_TAG)
    {
        if (appl_msg[idx + 1] == 0)
        {
            // 0 : 무부하 시 동작 상한 표시는 꺼지고 부하 동작 표시는 모두 점등
            // 상태. LCD는 정상적인 순환 항목 표시 (default)
            circdsp_pvt_mode_disable();
        }
        else
        {
            // 1 : 무부하 시 동작 상한 표시는 1 상한 방향을 가리키며 12시
            // 방향부터 0.5초 간격으로 회전. LCD는 현재 날짜, 시간만 순환표시
            circdsp_pvt_mode_enable();
        }
        lp_event_set(LPE_PROGRAM_CHG);
        return;
    }
    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_lcdset_parm(int idx)
{
    // measure method
    meas_method_type meas;
    ST_MIF_METER_PARM* pst_mif_meter_parm = dsm_mtp_get_meter_parm();
    uint8_t eob_type = 0;
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == OCTSTRING_TAG && appl_msg[idx + 1] == 1)
    {
        if (act_is_progdl_cmd())
        {
            prog_dl.lcdsetparm = appl_msg[idx + 2];
            pdl_set_bits |= SETBITS_LCDSET_PARM;

            touset_parse_buf[0] = appl_msg[idx + 2];
            touset_parse_idx = 1;
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            // simple mode
            if (appl_msg[idx + 2] & 0x08)
            {
                circdsp_smode_set();
            }
            else
            {
                circdsp_smode_reset();
            }
            /* 내용 검토 및 코딩 필요..*/
            if ((appl_msg[idx + 2] & 0x04) &&
                (appl_msg[idx + 2] &
                 0x02))  // anti-fraud & vector -> 한가지 모드 즉 송/수전 모드
            {
                // add to
            }
            else if (appl_msg[idx + 2] & 0x04)  // anti-fraud
            {
                // add to
            }
            else if (appl_msg[idx + 2] & 0x02)  // vector
            {
                // add to
            }
            else if (appl_msg[idx + 2] & 0x01)  // 수전 모드, 송,수전 모드
            {
                // add to
            }
            else
            {
            }
            meas = parse_meas_method(appl_msg[idx + 2]);

            if ((meas_method != meas) &&
                ((E_SINGLE_DIR != meas) || (mt_is_uni_dir())))
            {
                meas_method = meas;
                pst_mif_meter_parm->meter_method = meas;
                mif_meter_parm_set();
                if (MmodeCHG_sr_dr_type)
                {
                    eob_type = EOB_nPERIOD_FLAG;
                }
                sr_dr_proc(eob_type, (MmodeCHG_sr_dr_type), &cur_rtc,
                           appl_tbuff);
            }
            lp_event_set(LPE_PROGRAM_CHG);
        }
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_scurr_limit_val(int idx)
{
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == LONGUNSIGNED_TAG)
    {
        t16 =
            (uint16_t)(((uint16_t)appl_msg[idx + 1] << 8) + appl_msg[idx + 2]);
        if (sCurr_limit_is_valid(t16))
        {
            if (act_is_progdl_cmd())
            {
                prog_dl.scurr_lmt = t16;
                pdl_set_bits |= SETBITS_sCURR_LMT_VAL;

                touset_parse_buf[0] = appl_msg[idx + 1];
                touset_parse_buf[1] = appl_msg[idx + 2];
                touset_parse_idx = 2;
                touset_array_entry_idx = 0xffff;

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                scurr_limit_level = t16;
            }
            return;
        }
    }
    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_scurr_limit2_val(int idx)
{
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == LONGUNSIGNED_TAG)
    {
        t16 =
            (uint16_t)(((uint16_t)appl_msg[idx + 1] << 8) + appl_msg[idx + 2]);
        if (sCurr_limit_is_valid(t16))
        {
            if (act_is_progdl_cmd())
            {
                prog_dl.scurr_lmt2 = t16;
                pdl_set_bits |= SETBITS_sCURR_LMT2_VAL;

                touset_parse_buf[0] = appl_msg[idx + 1];
                touset_parse_buf[1] = appl_msg[idx + 2];
                touset_parse_idx = 2;
                touset_array_entry_idx = 0xffff;

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                scurr_limit_level_2 = t16;
            }
            return;
        }
    }
    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_scurr_autortn_val(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == UNSIGNED_TAG)
    {
        if (appl_msg[idx + 1] <= 99)
        {
            if (act_is_progdl_cmd())
            {
                prog_dl.scurr_autortn = appl_msg[idx + 1];
                pdl_set_bits |= SETBITS_sCURR_AUTORTN;

                touset_parse_buf[0] = appl_msg[idx + 1];
                touset_parse_idx = 1;
                touset_array_entry_idx = 0xffff;

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                scurr_autortn_cnt = appl_msg[idx + 1];
            }
            return;
        }
    }
    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_prepay(uint8_t kind, int idx)
{
    bool rslt;
    uint8_t* pt8;
    uint32_t t32;
    prepay_info_type* pst_prepay_all = get_prepay_all();

    rslt = 1;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (kind)
    {
    case 3:  // 구매 전력량
        if (appl_msg[idx] == UDLONG_TAG)
        {
            pt8 = &appl_msg[idx + 1];
            ToH32((U8_16_32*)&t32, pt8);
            prepay_info.buyenergy = t32;

            /*=================================================*/
            /* buy energy 설정시 sum 할지, 복사 할지 검토 필요 */
            /*=================================================*/
            int rem_energy = pst_prepay_all->remenergy;
            // 단위 wh -> pulse 수
            prepay_info.buyenergy = (t32 * PulseKwh / 1000);
            pst_prepay_all->remenergy += prepay_info.buyenergy;
            DPRINTF(DBG_TRACE, "%s: Buy[%d] -> Rem[%d = %d  + %d]\r\n",
                    __func__, prepay_info.buyenergy, pst_prepay_all->remenergy,
                    rem_energy, prepay_info.buyenergy);
        }
        else
        {
            rslt = 0;
        }
        break;
    case 5:  // 선불제 요금제 사용 여부
        if (appl_msg[idx] == BOOLEAN_TAG)
        {
            pt8 = &appl_msg[idx + 1];
            prepay_info.enable = *pt8;
            if (prepay_info.enable == 0)
            {
                prepay_load_on();
            }
            DPRINTF(DBG_TRACE, "%s: prepay_enable[%d]\r\n", __func__,
                    prepay_info.enable);
        }
        else
        {
            rslt = 0;
        }
        break;
    case 6:  // 선불형 부하제한 해제 여부
        if (appl_msg[idx] == BOOLEAN_TAG)
        {
            pt8 = &appl_msg[idx + 1];
            prepay_info.cancel_flag = *pt8;

            DPRINTF(DBG_TRACE, "%s: prepay_cancel_flag[%d]\r\n", __func__,
                    prepay_info.cancel_flag);
        }
        else
        {
            rslt = 0;
        }

        break;
    }

    if (!rslt)
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_temp_thrshld(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == INTEGER_TAG)
    {
        temp_thrshld = appl_msg[idx + 1];
        lp_event_set(LPE_PROGRAM_CHG);
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_sagswell(uint8_t kind, int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (kind == 1 || kind == 3)  // sag/swell level
    {
        if (appl_msg[idx] == UNSIGNED_TAG)
        {
            sag_swell_save_set_start(kind, appl_msg[idx + 1], 0);
            lp_event_set(LPE_PROGRAM_CHG);
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
    }
    else  // sag/swell time
    {
        float fval = 0.0;

        if (appl_msg[idx] == FLOAT32_TAG)
        {
            ToHFloat((U8_Float*)&fval, &appl_msg[idx + 1]);

            sag_swell_save_set_start(kind, 0, fval);
            lp_event_set(LPE_PROGRAM_CHG);
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
    }
}

static void obset_condensor_inst(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == BOOLEAN_TAG)
    {
        condensor_en = (appl_msg[idx + 1] == 0) ? 1 : 0;
        lp_event_set(LPE_PROGRAM_CHG);
        return;
    }
    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_auto_mode_sel(int idx)
{
    if (appl_msg[idx] == BOOLEAN_TAG)
    {
        auto_mode_sel = appl_msg[idx + 1];
        lp_event_set(LPE_PROGRAM_CHG);
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_modem_atcmd(int idx, uint8_t modem_type)
{
    ST_ATCMD_TMP_BUF* pst_atcmd_from_modem =
        dsm_get_atcmd_from_modem(modem_type);
    ST_ATCMD_TMP_BUF* pst_atmcd_from_client =
        dsm_get_atcmd_from_client(modem_type);

    DPRINTF(DBG_TRACE, _D "%s: modem_type[%d], idx[%d], len[%d]\r\n", __func__,
            modem_type, idx, appl_len);

    if (appl_msg[idx] == OCTSTRING_TAG && (appl_len > (idx + 1)))
    {
        pst_atmcd_from_client->len = appl_msg[idx + 1];
        memcpy(pst_atmcd_from_client->string, &appl_msg[idx + 2],
               pst_atmcd_from_client->len);

        DPRINT_HEX(DBG_TRACE, "AT_CLIENT", pst_atmcd_from_client->string,
                   pst_atmcd_from_client->len, DUMP_ALWAYS);

        dsm_atcmd_poll_tx_n_rx(modem_type, pst_atmcd_from_client,
                               pst_atcmd_from_modem);
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

#if 1  // 23.11.14  jp

static void obset_holiday_sel(int idx)
{
    U8* tptr;
    tptr = adjust_tptr(&global_buff[0]);
    bool t8;

#if 1  // jp.kim 25.03.15
    if (appl_msg[idx] == BOOLEAN_TAG)
#else
    if (appl_msg[idx] == UNSIGNED_TAG)
#endif
    {
        t8 = appl_msg[idx + 1];
        if (act_is_progdl_cmd())
        {
            prog_dl.hol_sel1 = t8;
            pdl_set_bits_1 |= SETBITS_HOLIDAY_SEL_1;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 1;
            touset_parse_buf[0] = appl_msg[idx + 1];

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            if (holiday_sel1 != t8)
            {
                holiday_sel1 = t8;
                prog_tou_refresh(tptr, false);
                lp_event_set(LPE_PROGRAM_CHG);
            }
        }
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}
#else
static void obset_holiday_sel(int idx)
{
    uint8_t* tptr;
    tptr = adjust_tptr(&global_buff[0]);

    if (appl_msg[idx] == BOOLEAN_TAG)
    {
        if (holiday_sel != appl_msg[idx + 1])
        {
            holiday_sel = appl_msg[idx + 1];
            prog_tou_refresh(tptr);
            prog_dl_type tou_prog;
            nv_read(I_CUR_PROG_DL, (uint8_t*)&tou_prog);
            if (holiday_sel)
            {
                DPRINTF(DBG_TRACE, "%s: Holiday Apply\r\n", __func__);
                tou_prog.set_bits_1 |= SETBITS_HOLIDAY_SEL_1;
            }
            else
            {
                DPRINTF(DBG_TRACE, "%s: Holiday Not Apply\r\n", __func__);
                tou_prog.set_bits_1 &= ~SETBITS_HOLIDAY_SEL_1;
            }
            nv_write(I_CUR_PROG_DL, (uint8_t*)&tou_prog);
        }
        lp_event_set(LPE_PROGRAM_CHG);
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}
#endif

static void obset_thd_rec_period_sel(int idx)
{
    uint16_t t16;

    if (appl_msg[idx] == LONGUNSIGNED_TAG)
    {
        t16 =
            (uint16_t)(((uint16_t)appl_msg[idx + 1] << 8) + appl_msg[idx + 2]);
        thd_rec_period = t16;
        lp_event_set(LPE_PROGRAM_CHG);
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_ct_pt_ratio(int idx)
{
    uint16_t t16;

    if (appl_msg[idx] == LONGUNSIGNED_TAG)
    {
        t16 =
            (uint16_t)(((uint16_t)appl_msg[idx + 1] << 8) + appl_msg[idx + 2]);
        if (obis_ge == 2)
        {
            ct_ratio_prog = t16;
        }
        else
        {
            pt_ratio_prog = t16;
        }
        lp_event_set(LPE_PROGRAM_CHG);
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_ts_conf(int idx)
{
    int i;
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_att_id == 0x02)
    {
        DPRINTF(DBG_TRACE, "%s \r\n", __func__);
        if (get_TS_selector(&appl_msg[idx + 4], &t8))
        {
            prog_dl.ts.ctrl = t8;
            DPRINTF(DBG_TRACE, "%s \r\n", __func__);

            pdl_set_bits |= SETBITS_TS_CTRL;

            if (act_is_progdl_cmd())
            {
                touset_parse_tsconf_att2(&appl_msg[idx]);
                touset_array_entry_idx = 0xffff;

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                if ((pdl_set_bits & (SETBITS_TS_CTRL | SETBITS_TS_ZONE)) ==
                    (SETBITS_TS_CTRL | SETBITS_TS_ZONE))
                {
                    ts_conf = prog_dl.ts;
                    DPRINTF(DBG_TRACE, "%s \r\n", __func__);
                }
            }

            return;
        }
    }
    else if (appl_att_id == 0x03)
    {
        DPRINTF(DBG_TRACE, "%s \r\n", __func__);
        if (act_is_progdl_cmd())
        {
            touset_parse_buf[0] = appl_msg[idx + 1];
            touset_parse_idx = 1;
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }

        return;
    }
    else if (appl_att_id == 0x04)
    {
        DPRINTF(DBG_TRACE, "%s \r\n", __func__);
        packed_ptr = &appl_msg[idx];
        packed_idx = 0;

        PARSE_ARRAY(t8);
        if (t8 > TS_ZONE_SIZE)
            t8 = TS_ZONE_SIZE;

        for (i = 0; i < t8; i++)
        {
            packed_idx += 2;  // struct tag
            // time
            packed_idx += 2;  // octet string tag
            if (packed_ptr[packed_idx] == 0xff ||
                packed_ptr[packed_idx + 1] == 0xff)
                break;
            prog_dl.ts.zone[i].hour = packed_ptr[packed_idx];
            prog_dl.ts.zone[i].min = packed_ptr[packed_idx + 1];
            packed_idx += TIME_TAG_LEN;
            // date
            packed_idx += (2 + DATE_TAG_LEN);  // octet string, date
        }
        prog_dl.ts.cnt = i;

        pdl_set_bits |= SETBITS_TS_ZONE;

        if (act_is_progdl_cmd())
        {
            touset_parse_tsconf_att4(&appl_msg[idx]);
            touset_array_entry_idx = 0xffff;

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            if ((pdl_set_bits & (SETBITS_TS_CTRL | SETBITS_TS_ZONE)) ==
                (SETBITS_TS_CTRL | SETBITS_TS_ZONE))
            {
                ts_conf = prog_dl.ts;
                DPRINTF(DBG_TRACE, "%s \r\n", __func__);
            }
        }
        return;
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_sel_react(int idx)
{
    uint8_t sel;
    uint16_t t16;
    date_time_type dt;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == OCTSTRING_TAG && appl_msg[idx + 1] == 4)
    {
        idx += 2;

        t16 = appl_msg[idx++] << 8;
        t16 |= appl_msg[idx++];

        dt.year = (uint8_t)(t16 - BASE_YEAR);
        dt.month = appl_msg[idx++];
        dt.date = 1;
        sel = appl_msg[idx];
        if (!prog_fut_is_available())
        {
            goto obset_sel_react1;
        }
        else
        {
            if (cmp_date(&dt, &fut_prog_work_time) > 0)
            {
            obset_sel_react1:
                // 월 단위로 해서 보다 커야 함
                sel_react_yr = dt.year;
                sel_react_mon = dt.month;
                sel_react_sel = sel;

                lp_event_set(LPE_PROGRAM_CHG);
                return;
            }
        }
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_comm_enable(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == BOOLEAN_TAG)
    {
        comm_en_coveropen_val = appl_msg[idx + 1];
        comm_en_coveropen_changed = true;

        lp_event_set(LPE_PROGRAM_CHG);
        return;
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_key_value(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == UNSIGNED_TAG)
    {
        set_key_code(appl_msg[idx + 1]);
        return;
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_scurr_hold(int idx)
{
    uint8_t* pt8;
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == UDLONG_TAG)
    {
        pt8 = &appl_msg[idx + 3];
        ToH16((U8_16*)&t16, pt8);
        if (sCurr_dur_is_valid(t16))
        {
            if (act_is_progdl_cmd())
            {
                prog_dl._scurr_det_hold = t16;
                prog_dl.set_bits_1 |= SETBITS_EXT_SCURR_1;

                touset_array_entry_idx = 0xffff;
                touset_parse_idx = 4;
                memcpy(&touset_parse_buf[0], &appl_msg[idx + 1], 4);

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                scurr_det_hold = t16;
            }
            return;
        }
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_scurr_rtn_1(int idx)
{
    uint8_t* pt8;
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == UDLONG_TAG)
    {
        pt8 = &appl_msg[idx + 3];
        ToH16((U8_16*)&t16, pt8);
        if (sCurr_dur_is_valid(t16))
        {
            if (act_is_progdl_cmd())
            {
                prog_dl._scurr_rtn_dur1 = t16;
                prog_dl.set_bits_1 |= SETBITS_EXT_SCURR_1;

                touset_array_entry_idx = 0xffff;
                touset_parse_idx = 4;
                memcpy(&touset_parse_buf[0], &appl_msg[idx + 1], 4);

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                scurr_rtn_dur_1 = t16;
            }
            return;
        }
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_scurr_rtn_2(int idx)
{
    uint8_t* pt8;
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == UDLONG_TAG)
    {
        pt8 = &appl_msg[idx + 3];
        ToH16((U8_16*)&t16, pt8);
        if (sCurr_dur_is_valid(t16))
        {
            if (act_is_progdl_cmd())
            {
                prog_dl._scurr_rtn_dur2 = t16;
                prog_dl.set_bits_1 |= SETBITS_EXT_SCURR_1;

                touset_array_entry_idx = 0xffff;
                touset_parse_idx = 4;
                memcpy(&touset_parse_buf[0], &appl_msg[idx + 1], 4);

                if (touset_last_obj_set(appl_class_id, appl_obis.id,
                                        appl_att_id, touset_array_entry_idx,
                                        touset_parse_buf,
                                        touset_parse_idx) == false)
                {
                    appl_resp_result = SET_RESULT_DATA_NG;
                }
            }
            else
            {
                scurr_rtn_dur_2 = t16;
            }
            return;
        }
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_scurr_cnt_n1(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == UNSIGNED_TAG)
    {
        if (act_is_progdl_cmd())
        {
            prog_dl._scurr_cnt_n1 = appl_msg[idx + 1];
            prog_dl.set_bits_1 |= SETBITS_EXT_SCURR_1;

            touset_array_entry_idx = 0xffff;
            touset_parse_idx = 1;
            touset_parse_buf[0] = appl_msg[idx + 1];

            if (touset_last_obj_set(appl_class_id, appl_obis.id, appl_att_id,
                                    touset_array_entry_idx, touset_parse_buf,
                                    touset_parse_idx) == false)
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            scurr_cnt_n1 = appl_msg[idx + 1];
        }
        return;
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_overcurr_enable(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == BOOLEAN_TAG)
    {
        if (appl_msg[idx + 1] == 0)
        {
            overcurr_cut_en = 1;
        }
        else
        {
            overcurr_cut_en = 0;
        }
        lp_event_set(LPE_PROGRAM_CHG);
        return;
    }
    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_ext_prog_id(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (act_is_progdl_cmd())
    {
        if ((appl_msg[idx] == OCTSTRING_TAG) &&
            (appl_msg[idx + 1] == EXT_TOU_ID_SIZE))
        {
            if (touset_extprogid_set(&appl_msg[idx + 2]))
                return;
        }
    }
    else
    {
        return;
    }

    appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_custom_id(int idx)
{
    ser_no_type* serno;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == OCTSTRING_TAG && appl_msg[idx + 1] == SERIAL_NO_SIZE)
    {
        memcpy(appl_tbuff, &appl_msg[idx + 2], SERIAL_NO_SIZE);
        serno = (ser_no_type*)appl_tbuff;

        set_cust_id(serno);
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}
static void obset_device_id(int idx)
{
    /* COSEM 계기 식별자, LDN: Logical Device Name */
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    if (appl_msg[idx] == OCTSTRING_TAG && appl_msg[idx + 1] == DEVICE_ID_SIZE)
    {
        memcpy(appl_tbuff, &appl_msg[idx + 2], DEVICE_ID_SIZE);
        nv_write(I_DEVICE_ID, appl_tbuff);
        nv_write(I_DEVICE_ID_KEPCO, appl_tbuff);
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_curr_temp(int idx)
{
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();
    temp_adj_data_type* temp_adj;
    float fval = 0;
    temp_adj = (temp_adj_data_type*)appl_tbuff;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (appl_msg[idx] == FLOAT32_TAG)
    {
        ToHFloat((U8_Float*)&fval, &appl_msg[idx + 1]);

        if ((fval > 10.0) && (fval < 40.0))  // jp.kim 24.11.21
        {
            adj_currtemp = (fval - pushd->temp);

            temp_adj->T_adj_temp = adj_currtemp;
            nv_write(I_ADJ_TEMP_DATA, (U8*)temp_adj);
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
    }
    else
    {
        appl_resp_result = SET_RESULT_DATA_NG;
    }
}

static void obset_inst_profile(int idx)
{
    prod_control_cal_begin(idx);  // jp.kim 24.11.07
}

#include "kepco_cert.h"
extern kepco_cert_storage_t kcs;
kepco_cert_storage_t kcs2;

// bccho, 2024-12-06
static void obset_inst_cert(int idx)
{
    // Validate OCTSTRING_TAG
    if (appl_msg[idx] != OCTSTRING_TAG)
    {
        appl_resp_result = SET_RESULT_DATA_NG;
        return;
    }
    idx += 1;

    // Validate extended length format (0x82)
    if (appl_msg[idx] != 0x82)
    {
        appl_resp_result = SET_RESULT_DATA_NG;
        return;
    }

    // Parse certificate length (big-endian 16-bit)
    uint16_t cert_len =
        ((uint16_t)(appl_msg[idx + 1] << 8) | (uint16_t)(appl_msg[idx + 2]));
    idx += 3;

    // Read existing certificate storage
    if (FMC_ReadBytes_S(KEPCO_CERT_ADDRESS, (uint32_t*)&kcs,
                        sizeof(kepco_cert_storage_t)) != 0)
    {
        MSG10("[Cert] Error!!! FMC_ReadBytes_S\r\n");
        appl_resp_result = SET_RESULT_DATA_NG;
        return;
    }

    // Erase flash sector
    if (FMC_Erase_S(KEPCO_CERT_ADDRESS) != 0)
    {
        MSG10("[Cert] Error!!! FMC_Erase_S\r\n");
        appl_resp_result = SET_RESULT_DATA_NG;
        return;
    }

    // Store certificate data
    kcs.kepco_cert.sign_cert_len = cert_len;
    memcpy(kcs.kepco_cert.sign_cert, &appl_msg[idx], cert_len);

    // Extract and store system title (offset 0xAD, 13 bytes)
    kcs.kepco_cert.systemtitle_len = 13;
    memcpy(kcs.kepco_cert.systemtitle, &kcs.kepco_cert.sign_cert[0xAD], 13);

    // Set magic number
    kcs.magic = KEPCO_CERT_STORAGE_MAGIC;
}

// bccho, 2024-12-06
static void obset_inst_key(int idx)
{
    // Validate OCTSTRING_TAG
    if (appl_msg[idx] != OCTSTRING_TAG)
    {
        appl_resp_result = SET_RESULT_DATA_NG;
        return;
    }
    idx += 1;

    // Validate key length (must be 32 bytes)
    uint16_t key_len = (uint16_t)appl_msg[idx];
    if (key_len != 0x20)
    {
        appl_resp_result = SET_RESULT_DATA_NG;
        return;
    }
    idx += 1;

    // Copy private key to certificate storage
    memcpy(kcs.kepco_cert.sign_prikey, &appl_msg[idx], key_len);

    // Write certificate storage to flash with retry mechanism
    uint8_t* addr = (uint8_t*)&kcs;
    const uint32_t MAX_RETRIES = 5;
    bool write_success = false;

    for (uint32_t retry = 0; retry < MAX_RETRIES && !write_success; retry++)
    {
        if (retry > 0)
        {
            MSG10("[KEY] Retry attempt %d/%d\r\n", retry, MAX_RETRIES - 1);

            // Erase flash sector before retry
            if (FMC_Erase_S(KEPCO_CERT_ADDRESS) != 0)
            {
                MSG10("[KEY] Error!!! FMC_Erase_S on retry %d\r\n", retry);
                continue;
            }
        }

        // Write data in 512-byte chunks
        bool write_failed = false;
        for (uint32_t i = 0; i < 2; i++)
        {
            uint32_t offset = i * 512;
            if (FMC_WriteMultiple_S(KEPCO_CERT_ADDRESS + offset,
                                    (uint32_t*)(addr + offset), 512) <= 0)
            {
                MSG10("[KEY] Error!!! FMC_WriteMultiple_S_%d (attempt %d)\r\n",
                      i + 1, retry + 1);
                write_failed = true;
                break;
            }
        }

        if (write_failed)
        {
            continue;
        }

        // Verify written data
        if (FMC_ReadBytes_S(KEPCO_CERT_ADDRESS, (uint32_t*)&kcs2,
                            sizeof(kepco_cert_storage_t)) != 0)
        {
            MSG10("[KEY] Error!!! FMC_ReadBytes_S (attempt %d)\r\n", retry + 1);
            continue;
        }

        // Compare written and read data
        if (memcmp(&kcs, &kcs2, sizeof(kepco_cert_storage_t)) == 0)
        {
            write_success = true;
            MSG10("[KEY] Write verified successfully on attempt %d\r\n",
                  retry + 1);
        }
        else
        {
            MSG10("[KEY] Verification failed on attempt %d\r\n", retry + 1);
        }
    }

    if (!write_success)
    {
        MSG10("[KEY] Error!!! Failed after %d attempts\r\n", MAX_RETRIES);
        appl_resp_result = SET_RESULT_DATA_NG;
        return;
    }

    MSG10("[KEY] insert OK\r\n");
}

static bool check_obj_TSscript(uint8_t* cp)
{
    if (*(cp + 0) == 0x01 && *(cp + 1) == 0x00 && *(cp + 2) == 0x80 &&
        *(cp + 3) == 0x00 && *(cp + 4) == 0x05 && *(cp + 5) == 0xff)
        return true;

    return false;
}

static bool get_TS_selector(uint8_t* cp, uint8_t* prslt)
{
    if (check_obj_TSscript(cp))
    {
        cp += OBIS_ID_SIZE;
        if (*cp == LONGUNSIGNED_TAG)
        {
            *prslt = *(cp + 2);  // just get available low byte
            return true;
        }
    }
    return false;
}

static bool dt_is_unspecified(date_time_type* dt)
{
    if (dt->year == UNSPECIFIED_CHAR && dt->month == UNSPECIFIED_CHAR &&
        dt->date == UNSPECIFIED_CHAR && dt->hour == UNSPECIFIED_CHAR &&
        dt->min == UNSPECIFIED_CHAR && dt->sec == UNSPECIFIED_CHAR)
    {
        return true;
    }

    return false;
}

int parse_ext_len_2(uint8_t* cp, uint16_t* extlen)
{
    int ret;

    if (*cp != 0x82)
    {
        *extlen = *cp;
        ret = 1;
    }
    else
    {
        cp += 1;  // next of 0x82
        ToH16((U8_16*)extlen, cp);
        ret = 3;
    }

    return ret;
}

int size_ext_len_2(uint8_t* cp)
{
    if (*cp == 0x82)
        return 3;

    return 1;
}

void hol_date_block_init(void)
{
    memset((uint8_t*)&hol_date_block, 0xff, sizeof(holiday_date_type));
    nv_write(I_HOL_DATE_BLOCK, (uint8_t*)&hol_date_block);
}

static void hol_date_block_restore(void)
{
    if (!nv_read(I_HOL_DATE_BLOCK, (uint8_t*)&hol_date_block))
    {
        memset((uint8_t*)&hol_date_block, 0xff, sizeof(holiday_date_type));
    }
}

static void hol_date_block_save(void)
{
    nv_write(I_HOL_DATE_BLOCK, (uint8_t*)&hol_date_block);
}

#define ZCD_DURTIME_MAX 100            // sec
#define ZCD_ACTION_START_DELAY_ms 300  // ms
uint8_t g_zcd_action_start_flag = 0;

uint8_t dsm_zcd_action_get_start_flag(void) { return g_zcd_action_start_flag; }

void dsm_zcd_action_set_start_flag(uint8_t val)
{
    g_zcd_action_start_flag = val;
}

static void obset_zcrs_sig_out_durtime(int idx)
{
    if (appl_msg[idx] == UDLONG_TAG)
    {
        uint8_t* pt8;
        uint32_t t32 = 0;

        pt8 = &appl_msg[idx + 1];
        ToH32((U8_16_32*)&t32, pt8);
        zcrs_sig_out_dur = t32;

        if (t32 > ZCD_DURTIME_MAX)
        {
            zcrs_sig_out_dur = ZCD_DURTIME_MAX;
        }

        DPRINTF(DBG_WARN, "%s: durtime[%d]\r\n", __func__, zcrs_sig_out_dur);

        M_MT_SW_GENERAL_three_TIMER_set_zcd_out();
        dsm_meter_sw_timer_start(MT_SW_GENERAL_three_TO, FALSE,
                                 (ZCD_ACTION_START_DELAY_ms));

        g_zcd_action_start_flag = TRUE;
    }
    else
        appl_resp_result = SET_RESULT_DATA_NG;
}

#if 0  // jp.kim 24.11.14
static void obset_zcrs_sig_out_cmpstime(int idx)
{
	   U8 *pt8;
	U32 t32;

	if(appl_msg[idx] == UDLONG_TAG) {
                pt8 = &appl_msg[idx+1];
		ToH32((U8_16_32 *)&t32, pt8);
		zcrs_sig_out_cmps = t32;
		}
        else	appl_resp_result = SET_RESULT_DATA_NG;
}
static void obset_zcrs_sig_out_resulttime(int idx)
{
    U8 *pt8;
    U32 t32;
    ST_ZCD_RESULT_TIME st_zcd;

    if(appl_msg[idx] == UDLONG_TAG)
    {
        pt8 = &appl_msg[idx+1];
        ToH32((U8_16_32 *)&t32, pt8);
        st_zcd.time = t32;
        nv_write(I_ZCD_RESULT_TIME, (UINT8 *)&st_zcd);
    }
    else
        appl_resp_result = SET_RESULT_DATA_NG;
}
#endif

static void obset_self_error_ref(int idx)
{
    uint8_t* pt8;
    int16_t t16;

    if (appl_msg[idx] == LONG_TAG)
    {
        pt8 = &appl_msg[idx + 1];
        ToH16((U8_16*)&t16, pt8);
        self_error_ref = t16;
    }
    else
        appl_resp_result = SET_RESULT_DATA_NG;
}

static void obset_security_setup(int idx)
{
    uint8_t t8;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x03:  // security_suite
        if (appl_msg[idx] == ENUM_TAG)
        {
            t8 = appl_msg[idx + 1];

            if (t8 == SECURITY_SUITE_ARIA_128GCM_ECDSAP256SIGN ||
                t8 == SECURITY_SUITE_AES_128GCM_ECDSAP256SIGN)
            {
                dsm_sec_set_security_suite(t8);
            }
            else
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
        break;
    }
}

uint32_t dsm_touETC_suply_disp(uint8_t* p_val, uint16_t val_len)
{
    uint8_t st, cnt, t8;
    uint16_t idx = 0;
    uint16_t lclass_id;
    uint8_t obis[6];
    uint8_t att_id = 0, dsp_time = 0;
    dsp_supply_type suppdsp;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    DPRINT_HEX(DBG_TRACE, "suply_disp", p_val, val_len, DUMP_DLMS);

    if (ARRAY_TAG == p_val[idx++])
    {
        suppdsp.mode_cnt = p_val[idx++];

        for (cnt = 0; cnt < suppdsp.mode_cnt; cnt++)
        {
            if (STRUCTURE_TAG == p_val[idx++])
            {
                st = p_val[idx++];
                if (LONGUNSIGNED_TAG == p_val[idx++])
                {
                    ToH16((U8_16_32*)&lclass_id, &p_val[idx]);
                    idx += 2;
                }
                else
                    goto touETC_err;
                if (OCTSTRING_TAG == p_val[idx++])
                {
                    t8 = p_val[idx++];
                    memcpy(obis, &p_val[idx], t8);
                    idx += t8;
                }
                else
                    goto touETC_err;
                if (UNSIGNED_TAG == p_val[idx++])
                {
                    att_id = p_val[idx++];
                }
                else
                    goto touETC_err;
                if (UNSIGNED_TAG == p_val[idx++])
                {
                    dsp_time = p_val[idx++];
                }
                else
                    goto touETC_err;
                DPRINTF(DBG_TRACE,
                        "  %d: ST_NUM[%d], CLASS_ID[0x%04d], ATT_ID[%d], "
                        "DSP_T[%d]\r\n",
                        cnt, st, lclass_id, att_id, dsp_time);
                DPRINT_HEX(DBG_TRACE, "OBIS", obis, t8, DUMP_ALWAYS);

                suppdsp.dsp_mode[cnt] =
                    (uint16_t)dspmode_from_suppdsp_set(obis, att_id);
            }
            else
                goto touETC_err;
        }
        suppdsp.dsptime = dsp_time;

        if (act_is_progdl_cmd())
        {
            if (nv_write(I_DISP_SUPP_DL, (uint8_t*)&suppdsp))
            {
                pdl_set_bits |= SETBITS_SUPPDSP_ITEM;
            }
        }
        else
        {
            if (nv_write(I_DISP_SUPP_A, (uint8_t*)&suppdsp))
            {
                memcpy((uint8_t*)&circ_state_suppdsp_mode, (uint8_t*)&suppdsp,
                       sizeof(dsp_supply_type));
                currprog_available_bits |= PROG_SUPP_DISP_BIT;
                lcd_dsp_mode = LCD_SUPP_DSPMODE;
                dsp_circ_state_mode_init();
                lp_event_set(LPE_PROGRAM_CHG);
            }
        }
        nv_sub_info.seg.offset = 0;
        nv_sub_info.seg.len = val_len;
        DPRINT_HEX(DBG_TRACE, "SUPPDSP_ITEM_DL_IMG", &p_val[0],
                   nv_sub_info.seg.len, DUMP_DLMS);

        if (act_is_progdl_cmd())
        {
            if (!nv_write(I_SUPPDSP_ITEM_DL, &p_val[0]))
            {
                appl_resp_result = SET_RESULT_REQ_NG;
            }
        }
        else
        {
            if (!nv_write(I_SUPPDSP_ITEM_A, &p_val[0]))
            {
                appl_resp_result = SET_RESULT_REQ_NG;
            }
        }
    }
    else
    {
    touETC_err:
        DPRINTF(DBG_ERR, "touETC_err!!!\r\n");
        return FALSE;
    }

    return TRUE;
}

uint32_t dsm_touETC_nperiod_billdate(uint16_t att_id, uint8_t* p_val,
                                     uint16_t val_len)
{
    uint8_t t8;
    uint16_t idx = 0;
    npbill_date_type* npbill;

    DPRINTF(DBG_TRACE, "%s: ATT_id[%d]\r\n", __func__, att_id);
    DPRINT_HEX(DBG_TRACE, "nperiod_data", p_val, val_len, DUMP_DLMS);

    switch (att_id)
    {
    case 2:  // [executed_script]
        if (parse_bill_drsel(p_val, &t8))
        {
            DPRINTF(DBG_TRACE, "%s: dr_sel[%d]\r\n", __func__, t8);
            if (act_is_progdl_cmd())
            {
                t8 = conv_sel_to_DRkind(t8);
                prog_dl.npEOB.dr_sel = t8;
                pdl_set_bits |= SETBITS_NPBILL_DRSEL;
            }
            else
            {
                set_npEOB_dr(t8);
                lp_event_set(LPE_PROGRAM_CHG);
            }
        }

        break;
    case 3:  // []type]

        break;
    case 4:  // [execution_time]
        npbill = (npbill_date_type*)appl_tbuff;
        parse_npBilldate(&p_val[idx], npbill);

        DPRINTF(DBG_TRACE, "%s: npbill->cnt[%d], act_is_progdl_cmd[%d]\r\n",
                __func__, npbill->cnt, act_is_progdl_cmd());

        if (act_is_progdl_cmd() == false)
        {
            currprog_available_bits |= PROG_npBILL_DATE_BIT;

            if (nv_write(I_NP_BILLDATE_A, appl_tbuff))
            {
                npBill_date_load();
                lp_event_set(LPE_PROGRAM_CHG);
            }
        }
        else
        {
            if (nv_write(I_NP_BILLDATE_DL, appl_tbuff))
            {
                pdl_set_bits |= SETBITS_NPBILL_DATE;
            }
        }

        break;
    }

    return TRUE;
}

uint32_t dsm_touETC_daylight_saving_enabled(uint16_t att_id, uint8_t* p_val,
                                            uint16_t val_len)
{
    uint16_t idx = 0;

    DPRINTF(DBG_TRACE, "%s: ATT_id[%d]\r\n", __func__, att_id);
    DPRINT_HEX(DBG_TRACE, "daylightsave", p_val, val_len, DUMP_DLMS);

    switch (att_id)
    {
    case 8:
        if (p_val[idx] == BOOLEAN_TAG)
        {
            prog_dl.dlsinfo.enabled = p_val[idx + 1];
            pdl_set_bits |= SETBITS_DLS_ENA;
        }

        break;
    }

    return TRUE;
}

uint32_t dsm_touETC_billing_parm(uint16_t att_id, uint8_t* p_val,
                                 uint16_t val_len)
{
    uint16_t idx = 0;

    DPRINTF(DBG_TRACE, "%s: ATT_id[%d]\r\n", __func__, att_id);
    DPRINT_HEX(DBG_TRACE, "billingparm", p_val, val_len, DUMP_DLMS);

    switch (att_id)
    {
    case 2:
        if (p_val[idx] == OCTSTRING_TAG && p_val[idx + 1] == BILLING_PARM_SIZE)
        {
            memcpy(prog_dl.bill_parm, &p_val[idx + 2], BILLING_PARM_SIZE);
            pdl_set_bits |= SETBITS_BILLING_PARM;
        }
        break;
    }

    return TRUE;
}

uint32_t dsm_touETC_lcdset_parm(uint16_t att_id, uint8_t* p_val,
                                uint16_t val_len)
{
    ST_MIF_METER_PARM* pst_mif_meter_parm = dsm_mtp_get_meter_parm();

    uint16_t idx = 0;
    uint8_t eob_type = 0;
    DPRINTF(DBG_TRACE, "%s: ATT_id[%d]\r\n", __func__, att_id);
    DPRINT_HEX(DBG_TRACE, "lcdset_parm", p_val, val_len, DUMP_DLMS);

    switch (att_id)
    {
    case 2:
        if (p_val[idx] == OCTSTRING_TAG && p_val[idx + 1] == 1)
        {
            meas_method_type meas;

            if (act_is_progdl_cmd())
            {
                prog_dl.lcdsetparm = p_val[idx + 2];
                pdl_set_bits |= SETBITS_LCDSET_PARM;
            }
            else
            {
                // simple mode
                if (p_val[idx + 2] & 0x08)
                {
                    circdsp_smode_set();
                }
                else
                {
                    circdsp_smode_reset();
                }

                meas = parse_meas_method(p_val[idx + 2]);
                if (meas_method != meas)
                {
                    meas_method = meas;
                    if (MmodeCHG_sr_dr_type)
                        eob_type = EOB_nPERIOD_FLAG;
                    sr_dr_proc(eob_type, (MmodeCHG_sr_dr_type), &cur_rtc,
                               appl_tbuff);

                    pst_mif_meter_parm->direct_reverse = mt_dir;
                    pst_mif_meter_parm->meter_method = meas_method;
                    mif_meter_parm_set();
                }
                lp_event_set(LPE_PROGRAM_CHG);
            }
        }

        break;
    }

    return TRUE;
}

uint32_t dsm_touETC_period_billdate(uint16_t att_id, uint8_t* p_val,
                                    uint16_t val_len)
{
    uint8_t t8;
    uint16_t idx = 0;

    DPRINTF(DBG_TRACE, "%s: ATT_id[%d]\r\n", __func__, att_id);
    DPRINT_HEX(DBG_TRACE, "period_data", p_val, val_len, DUMP_DLMS);

    switch (att_id)
    {
    case 2:  // [executed_script]
        if (parse_bill_drsel(&p_val[idx], &t8))
        {
            if (act_is_progdl_cmd())
            {
                t8 = conv_sel_to_DRkind(t8);
                prog_dl.pEOB.dr_sel = t8;
                pdl_set_bits |= SETBITS_PBILL_DRSEL;
            }
            else
            {
                set_pEOB_dr(t8);
                lp_event_set(LPE_PROGRAM_CHG);
            }
            break;
        }

        break;
    case 3:  // [type]

        break;
    case 4:  // [execution_time]
        // ==> TAG check is ignored
        t8 = p_val[idx + 15];
        if (act_is_progdl_cmd())
        {
            prog_dl.regread_date = t8;
            pdl_set_bits |= SETBITS_PBILL_DATE;
        }
        else
        {
            if (reg_mr_date != t8)
                tou_id_change_sts = 1;

            reg_mr_date = t8;
            lp_event_set(LPE_PROGRAM_CHG);
        }

        break;
    }

    return TRUE;
}

uint32_t dsm_touETC_lp_interval(uint16_t att_id, uint8_t* p_val,
                                uint16_t val_len)
{
    uint8_t t8;
    uint16_t idx = 0;

    DPRINTF(DBG_TRACE, "%s: ATT_id[%d]\r\n", __func__, att_id);
    DPRINT_HEX(DBG_TRACE, "lp_interval", p_val, val_len, DUMP_DLMS);

    switch (att_id)
    {
    case 2:
        if (p_val[idx] == UNSIGNED_TAG)
        {
            t8 = p_val[idx + 1];
            if (act_is_progdl_cmd())
            {
                prog_dl.lp_intv = t8;
                pdl_set_bits |= SETBITS_LP_INTV;
            }
            else
            {
                lp_interval = t8;
                lp_event_set(LPE_PROGRAM_CHG);
            }
        }

        break;
    }

    return TRUE;
}

uint32_t dsm_touETC_curr_last_demand(uint16_t att_id, uint8_t* p_val,
                                     uint16_t val_len)
{
    uint8_t* pt8;
    uint32_t t32;
    uint16_t t16;
    uint16_t idx = 0;

    DPRINTF(DBG_TRACE, "%s: ATT_id[%d]\r\n", __func__, att_id);
    DPRINT_HEX(DBG_TRACE, "curr_last_demand", p_val, val_len, DUMP_DLMS);

    switch (att_id)
    {
    case 8:  // [period]
        if (p_val[idx] == UDLONG_TAG)
        {
            pt8 = &p_val[idx + 1];
            ToH32((U8_16_32*)&t32, pt8);
            prog_dl.dm_prd = (uint8_t)(t32 / 60);
            pdl_set_bits |= SETBITS_DM_PRD;
        }

        break;
    case 9:  // [number_of_periods]
        if (p_val[idx] == LONGUNSIGNED_TAG)
        {
            pt8 = &p_val[idx + 1];
            ToH16((U8_16*)&t16, pt8);
            prog_dl.dm_prd_num = (uint8_t)t16;
            pdl_set_bits |= SETBITS_DM_PRD_NUM;
        }

        break;
    }

    return TRUE;
}

uint32_t dsm_touETC_active_passive_time(uint16_t att_id, uint8_t* p_val,
                                        uint16_t val_len)
{
    uint16_t idx = 0;

    DPRINTF(DBG_TRACE, "%s: ATT_id[%d]\r\n", __func__, att_id);
    DPRINT_HEX(DBG_TRACE, "active_passive_time", p_val, val_len, DUMP_ALWAYS);

    switch (att_id)
    {
    case 10:
        /* [active_passive_calendar_time] : 예약 프로그램 적용 일자/시간 */
        if (get_date_time_from_comm(&prog_dl.active_passive_time, 0,
                                    &p_val[idx]))
        {
            prg_tou_type = 1;
            pdl_set_bits |= SETBITS_PAS_TIME;
        }

        break;
    }

    return TRUE;
}

void dsm_touETC_holiday_sel(uint16_t att_id, uint8_t* p_val, uint16_t val_len)
{
    /* 통신 규격 3.4.2.8.27 정기/비정기 휴일 적용 */

    // 테스트 필요함. obset_holiday_sel() 참조
    DPRINTF(DBG_TRACE, "%s: AttrID[%d]\r\n", __func__, att_id);
    DPRINT_HEX(DBG_ERR, "holiday_sel", p_val, val_len, DUMP_ALWAYS);

    switch (att_id)
    {
    case 2:
    {
        uint8_t* tptr;
        tptr = adjust_tptr(&global_buff[0]);  // 버퍼로 인하여 문제가 발생할지도
                                              // 모름. 확인 필요함.

        if (p_val[0] == BOOLEAN_TAG)
        {
#if 0
            if (p_val[1])
            {
                // 적용
                DPRINTF(DBG_TRACE, "%s: TOU AETC Holiday Apply\r\n", __func__);
                pdl_set_bits_1 |= SETBITS_HOLIDAY_SEL_1;
            }
            else
            {
                // 미적용
                DPRINTF(DBG_TRACE, "%s: TOU AETC Holiday Not Apply\r\n", __func__);
                pdl_set_bits_1 &= ~SETBITS_HOLIDAY_SEL_1;
            }
#else  // 23.11.13 jp
            if (act_is_progdl_cmd())
            {
                prog_dl.hol_sel1 = p_val[1];
                pdl_set_bits_1 |= SETBITS_HOLIDAY_SEL_1;
            }
            else
            {
                if (holiday_sel1 != p_val[1])
                {
                    holiday_sel1 = p_val[1];
                    prog_tou_refresh(tptr, false);
                    lp_event_set(LPE_PROGRAM_CHG);
                }
            }
#endif
        }
    }
    break;
    }
}
uint8_t dsm_imgtrfr_parser_image_to_activate_info(EN_IMG_TYPE type,
                                                  uint8_t* pdata)
{
    // TODO: (WD) Check

    uint8_t t8, *pt8;
    uint32_t t32;
    uint8_t result = 0;
    uint16_t idx = 0;

    if ((pdata[idx] == ARRAY_TAG) && (pdata[idx + 1] == 1))
    {
        idx += 2;
        if ((pdata[idx] == STRUCTURE_TAG) && (pdata[idx + 1] == 3))
        {
            idx += 2;
            if (pdata[idx] == UDLONG_TAG)
            {
                pt8 = &pdata[idx + 1];
                ToH32((U8_16_32*)&t32, pt8);
                dsm_imgtrfr_set_image_size(type, t32);
                idx += 5;
                if (pdata[idx] == OCTSTRING_TAG)
                {
                    t8 = pdata[idx + 1];
                    if (IMAGE_NAME_MAX_SIZE > t8)
                    {
                        dsm_imgtrfr_set_name(type, t8, &pdata[idx + 2]);
                        idx += (2 + t8);
                        if (pdata[idx] == OCTSTRING_TAG)
                        {
                            t8 = pdata[idx + 1];
                            if (IMAGE_HASH_SIZE > t8)
                            {
                                dsm_imgtrfr_set_hash(type, t8, &pdata[idx + 2]);
                            }
                            idx += (2 + t8);
                        }
                        else
                            goto parser_image_to_activate_info_err;
                    }
                    else
                        goto parser_image_to_activate_info_err;
                }
                else
                    goto parser_image_to_activate_info_err;
            }
            else
                goto parser_image_to_activate_info_err;
        }
        else
            goto parser_image_to_activate_info_err;
    }
    else
    {
    parser_image_to_activate_info_err:
        return SET_RESULT_DATA_NG;
    }
    return result;
}

static void obset_evt_tou_imagetransfer(int idx)
{
    DPRINTF(DBG_TRACE, _D "%s: Attributes %d\r\n", __func__, appl_att_id);

    // Set Req
    /* 통신 규격 3.4.2.8.22 TOU Image Transfer : Attributes */
    switch (appl_att_id)
    {
    case 0x02:
    {
        /* [image_block_size] */
        if (appl_msg[idx] == UDLONG_TAG)
        {
            uint32_t dword = 0;
            idx++;
            ToH32((U8_16_32*)&dword, &appl_msg[idx]);
            dsm_imgtrfr_set_blk_size(IMG__TOU, dword);
        }
    }
    break;
#if 0  // jp.kim 25.12.06 //규격에 없는 attr. write //운영부 계량부 무결성
       // 검증
        case 0x03:
    {
        /* [image_transferred_block_status] ※ 사용 안 함 */
    }
    break;

    case 0x04:
    {
        /* [image_first_not_transferred_block_number] */
        if (appl_msg[idx] == UDLONG_TAG)
        {
            uint32_t dword = 0;
            idx++;
            ToH32((U8_16_32*)&dword, &appl_msg[idx]);
            dsm_imgtrfr_set_blk_num(IMG__TOU, dword);
        }
    }

    case 0x05:
    {
        /* [image_transfer_enabled] */
        if (appl_msg[idx] == BOOLEAN_TAG)
        {
            uint8_t byte = appl_msg[++idx];
            if ((byte == IMAGE_TRANSFER_ENABLED) ||
                (byte == IMAGE_TRANSFER_DISABLED))
            {
                dsm_imgtrfr_set_transfer_enabled(IMG__TOU, byte);
            }
            else
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
    }
    break;

    case 0x06:
    {
        /* [image_transfer_status] */
        if (appl_msg[idx] == ENUM_TAG)
        {
            uint8_t byte = appl_msg[++idx];
            dsm_imgtrfr_set_transfer_status(IMG__TOU, byte);
        }
    }
    break;

    case 0x07:
    {
        /* [image_to_activate_info] */
        appl_resp_result =
            dsm_imgtrfr_parser_image_to_activate_info(IMG__TOU, &appl_msg[idx]);
    }
    break;
#endif
    }
}
// TODO: (WD)
static void obset_evt_fw_imagetransfer(uint16_t idx)
{
    uint8_t byte = 0;
    uint32_t dword = 0;

    DPRINTF(DBG_TRACE, _D "%s: Attributes %d\r\n", __func__, appl_att_id);

    // Set Req
    /* 통신 규격 3.4.2.11.3 소프트웨어 업데이트 Image transfer : Attributes */
    switch (appl_att_id)
    {
    case 0x02:
    {
        /* [image_block_size] */
        /*
            DEC_APDU (size=18):
                C1 01 81 // APDU[0]: req type, APDU[1]: choice, APDU[2]: invoke
                00 12 00 00 2C 00 00 FF 02  // class 2bytes, obis 6bytes,
           attributes 1byte, 00 // APDU[12]: selective access option 06 // idx
           == 13, APDU[13]: UDLONG_TAG 00 00 01 00 // value
        */
        // BlockSize: 512(default), 256, 128, 64
        if (appl_msg[idx] == UDLONG_TAG)
        {
            idx++;
            ToH32((U8_16_32*)&dword, &appl_msg[idx]);
            dsm_imgtrfr_set_blk_size(IMG__FW, dword);
        }
    }
    break;
#if 0  // jp.kim 25.12.06 //규격에 없는 attr. write //운영부 계량부 무결성 검증
    case 0x03:
    {
        /* [image_transferred_block_status] */
    }
    break;

    case 0x04:
    {
        /* [image_first_not_transferred_block_number] */
        if (appl_msg[idx] == UDLONG_TAG)
        {
            idx++;
            ToH32((U8_16_32*)&dword, &appl_msg[idx]);
            dsm_imgtrfr_set_blk_num(IMG__FW, dword);
        }
    }

    case 0x05:
    {
        /* [image_transfer_enabled] */
        if (appl_msg[idx] == BOOLEAN_TAG)
        {
            byte = appl_msg[++idx];
            if ((byte == IMAGE_TRANSFER_ENABLED) ||
                (byte == IMAGE_TRANSFER_DISABLED))
            {
                dsm_imgtrfr_set_transfer_enabled(IMG__FW, byte);
            }
            else
            {
                appl_resp_result = SET_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = SET_RESULT_DATA_NG;
        }
    }
    break;

    case 0x06:
    {
        /* [image_transfer_status] */
        if (appl_msg[idx] == ENUM_TAG)
        {
            byte = appl_msg[++idx];
            dsm_imgtrfr_set_transfer_status(IMG__FW, byte);
        }
    }
    break;

    case 0x07:
    {
        /* [image_to_activate_info] */
        appl_resp_result =
            dsm_imgtrfr_parser_image_to_activate_info(IMG__FW, &appl_msg[idx]);
    }
    break;
#endif
    }
}

static void obset_realtime_lp_interval(int idx)
{
    uint8_t* pt8;
    uint16_t t16;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    switch (appl_att_id)
    {
    case 0x02:
        if (appl_msg[idx] == LONGUNSIGNED_TAG)
        {
            pt8 = &appl_msg[idx + 1];
            ToH16((U8_16*)&t16, pt8);
            if (t16 > 60)
                appl_resp_result = SET_RESULT_DATA_NG;
            else
                rt_lp_interval = t16;
        }
        else
            appl_resp_result = SET_RESULT_DATA_NG;

        break;
    }
}

static void obset_tmsync_range(int idx)
{
    uint8_t* pt8;
    uint32_t t32;

    if (appl_msg[idx] == UDLONG_TAG)
    {
        pt8 = &appl_msg[idx + 1];
        ToH32((U8_16_32*)&t32, pt8);
        rtc_shift_range = t32;
    }
    else
        appl_resp_result = SET_RESULT_DATA_NG;
}
