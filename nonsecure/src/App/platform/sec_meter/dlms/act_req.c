#include "options.h"
#include "comm.h"
#include "dlms_todo.h"
#include "dl.h"
#include "aarq.h"
#include "appl.h"
#include "get_req.h"
#include "act_req.h"
#include "set_req.h"
#include "nv.h"

#include "main.h"
#include "amg_sec.h"
#include "amg_uart.h"

#if 0 /* bccho, KEYPAIR, 2023-07-15 */
#include "kse100_stm32l4.h"
#endif /* bccho */
#include "amg_imagetransfer.h"
#include "flash_if.h"
#include "amg_push_datanoti.h"
#include "whm.h"
#include "program.h"
#include "amg_meter_main.h"
#include "amg_media_mnt.h"
#include "disp.h"
#include "amg_modemif_prtl.h"
#include "nsclib.h"

#define _D "[AREQ] "

uint8_t prg_tou_type = 0;
uint8_t act_cmd_sel_ap;
device_cmd_type act_devcmd;

extern float VOLT_FWUP_BACK[3];
extern float CURR_FWUP_BACK[3];
extern float PH_FWUP_BACK[3];
extern float PH_LTOL_FWUP_BACK[3];
extern float PULSE_ADD_MODIFY_DATA;
extern float PULSE_ADD_MODIFY_DATA_VA, PULSE_ADD_MODIFY_DATA_VAR;
extern bool METER_FW_UP_ING_STS;
extern bool METER_FW_UP_END_PULSE_MODIFY;
extern U16 METER_FW_UP_ING_CNT;
extern U8 PULSE_DIR_MODIFY_BACK;
extern bool init_mif_task_firm_up;
extern U8 g_meter_fw_chk_sum_err;
extern uint8_t g_get_hash[IMAGE_HASH_SIZE];
extern ST_FW_INFO sun_fw_info;

uint32_t g_pre_sector = 0;

void prog_cur_tou_fut_proc(void);
void prog_cur_dl_proc(prog_dl_type* progdl, uint8_t* tptr);
bool prog_fut_dl_proc(prog_dl_type* progdl, uint8_t* tptr);
static bool check_sys_fw_crc(uint32_t size);

static void approc_act_req_normal(int idx);
static void approc_act_req_proc(int idx);
static void ob_dev_bcmd(uint16_t cmd);
static void ob_rload_cmd(uint8_t cmd);
static void ob_holidays_cmd(uint8_t method, int idx);
static void ob_eob_cmd(uint16_t cmd);
static void approc_fill_act_resp_normal(void);
static void ob_assLN_cmd(uint8_t method, int idx);
static void ob_security_setup_cmd(uint8_t method, int idx);
void ob_act_rsp_ass_LN(void);
void ob_act_rsp_security_setup(void);
static void ob_touimage_transfer_cmd(uint8_t method, int idx);
static void ob_fwimage_transfer_cmd(uint8_t method, int idx);
static void ob_push_setup_cmd(uint8_t setup_type, int idx);
void dsp_r_sun_dsp_set(void);

void approc_act_req(int idx)
{
    appl_act_rsp_option_flag = 0;
    switch (appl_reqchoice)
    {  // apdu[1]
    case ACT_REQ_NORMAL:
        DPRINTF(DBG_TRACE, _D "Normal\r\n");
        approc_act_req_normal(idx);
        break;

    default:
        appl_resp_choice = ACT_RES_NORMAL;
        appl_resp_result = ACT_RESULT_TYPE_UNMAT;
        break;
    }

    appl_act_resp();
    appl_act_rsp_option_flag = 0;
}

static void approc_act_req_normal(int idx)
{
    appl_result_type rslt;

    appl_resp_choice = ACT_RES_NORMAL;
    DPRINTF(DBG_TRACE, _D "%s: idx[%d]\r\n", __func__, idx);
    if (!comm_en_coveropen && IS_MorTCOVER_OPEN)
    {
        appl_resp_result = ACT_RESULT_TYPE_UNMAT;
        return;
    }

    idx = appl_cosem_descript(idx);
    if (idx >= appl_len)
    {
        appl_resp_result = ACT_RESULT_TYPE_UNMAT;
        return;
    }

    rslt = appl_obj_enum_and_acs_check();

    DPRINTF(DBG_TRACE, _D "%s: rslt[%d]\r\n", __func__, rslt);

    if (rslt != RESULT_OK)
    {
        appl_resp_result = ACT_RESULT_TYPE_UNMAT;
        return;
    }

    approc_act_req_proc(idx);

    if (appl_resp_result != RESULT_OK)
    {
        return;
    }

    if (appl_is_sap_sec_site() || appl_is_sap_sec_utility())
        approc_fill_act_resp_normal();
}

static void approc_act_req_proc(int idx)
{
    uint8_t tag;
    uint16_t cmd;

    DPRINTF(DBG_TRACE, _D "%s: obj_id[%d], method[%d]\r\n", __func__,
            appl_obj_id, appl_att_id);

    if (appl_obj_id == OBJ_HOLIDAYS || appl_obj_id == OBJ_ASSOCIATION_LN ||
        appl_obj_id == OBJ_SECURITY_SETUP_3 ||
        appl_obj_id == OBJ_SECURITY_SETUP_4 ||
        appl_obj_id == OBJ_TOU_IMAGE_TRANSFER ||
        appl_obj_id == OBJ_SW_IMAGE_TRANSFER ||
        appl_obj_id == OBJ_PUSH_SETUP_ERR_CODE ||
        appl_obj_id == OBJ_PUSH_SETUP_LAST_LP)
    {
        uint8_t* cp;

        cp = &appl_msg[idx++];
        if (*cp != METHOD_OPT_DATA_FLAG)
        {
            DPRINTF(DBG_ERR, _D "%s: Option Flag Error for part of Object\r\n",
                    __func__);
            appl_resp_result = ACT_RESULT_DATA_NG;
            return;
        }
    }
    else
    {
        if (!appl_is_sap_private())  // JP.KIM 24.11.08
        {
            uint8_t* cp;
            cp = &appl_msg[idx++];
            if (*cp != METHOD_OPT_DATA_FLAG)
            {
                DPRINTF(DBG_ERR, _D "%s: Option Flag Error\r\n", __func__);
                appl_resp_result = ACT_RESULT_DATA_NG;
                return;
            }
        }

        tag = appl_msg[idx++];
        ToH16((U8_16*)&cmd, &appl_msg[idx]);
        idx += 2;

        if (tag != LONGUNSIGNED_TAG)
        {
            DPRINTF(DBG_ERR, _D "%s: Tag Error\r\n", __func__);
            appl_resp_result = ACT_RESULT_DATA_NG;
            return;
        }
    }

    switch (appl_obj_id)
    {
    case OBJ_DEVICE_BCMD:
        /* 계기 운용 기본 명령 */
        if (appl_att_id != 2)
        {
            DPRINTF(DBG_ERR, _D "%s: Attributes is not matched\r\n", __func__);
        }
        ob_dev_bcmd(cmd);
        break;

    case OBJ_DEVICE_CMD:
        /* 계기 운용 주요 명령 */
        if (appl_att_id != 2)
        {
            DPRINTF(DBG_ERR, _D "%s: Attributes is not matched\r\n", __func__);
        }
        ob_dev_cmd(cmd);
        break;

    case OBJ_ENDOF_BILLING:
        ob_eob_cmd(cmd);
        break;

    case OBJ_rLOAD_CTRL:
        ob_rload_cmd((uint8_t)cmd);
        break;

    case OBJ_HOLIDAYS:
        ob_holidays_cmd(appl_att_id, idx);
        break;

    case OBJ_ASSOCIATION_LN:
        ob_assLN_cmd(appl_att_id, idx);
        break;

    case OBJ_SECURITY_SETUP_3:
    case OBJ_SECURITY_SETUP_4:
        ob_security_setup_cmd(appl_att_id, idx);
        break;

    case OBJ_TOU_IMAGE_TRANSFER:
        ob_touimage_transfer_cmd(appl_att_id, idx);
        break;

    case OBJ_SW_IMAGE_TRANSFER:
        ob_fwimage_transfer_cmd(appl_att_id, idx);
        break;

    case OBJ_METERING_TYPE_SEL: /*게량 종별 선택*/
        // 기능 구현 필요..
        break;

    case OBJ_PUSH_SETUP_ERR_CODE:
        ob_push_setup_cmd(PUSH_SCRIPT_ID_ERR_CODE, idx);
        break;

    case OBJ_PUSH_SETUP_LAST_LP:
        ob_push_setup_cmd(PUSH_SCRIPT_ID_LAST_LP, idx);
        break;
    default:
        appl_resp_result = ACT_RESULT_TYPE_UNMAT;
        break;
    }
}

static void approc_fill_act_resp_normal(void)
{
    int pPdu_idx_start;

    appl_act_rsp_option_flag = 1;

    if (appl_act_rsp_option_flag)
        pPdu_idx_start = pPdu_idx = APPL_FILL_RESP_DATA_OPT_IDX_NORMAL;
    else
        pPdu_idx_start = pPdu_idx = APPL_FILL_RESP_DATA_IDX_NORMAL;

    DPRINTF(DBG_TRACE, _D "%s: obj_id[%d], att_id[%d], idx[%d]\r\n", __func__,
            appl_obj_id, appl_att_id, pPdu_idx);

    switch (appl_obj_id)
    {
    case OBJ_ASSOCIATION_LN:
        ob_act_rsp_ass_LN();

        break;
    case OBJ_SECURITY_SETUP_3:
    case OBJ_SECURITY_SETUP_4:
        ob_act_rsp_security_setup();

        if (appl_att_id == 4)
        {
            return;
        }

        break;
    default:
        appl_act_rsp_option_flag = 0;
        pPdu_idx_start = pPdu_idx = APPL_FILL_RESP_DATA_IDX_NORMAL;
        return;
    }

    if (pPdu_idx == pPdu_idx_start)
    {
        appl_resp_result = SET_RESULT_DATA_NG;
        DPRINTF(DBG_ERR, _D "%s ######\r\n", __func__);
    }
}

static void ob_push_setup_cmd(uint8_t setup_type, int idx)
{
    uint8_t* cp;
    uint8_t cp_idx = 0;
    DPRINTF(DBG_TRACE, _D "%s: meth_id[%d], setup_type[%d]\r\n", __func__,
            appl_att_id, setup_type);
    cp = &appl_msg[idx];

    if (appl_att_id == 1)
    {
        if (cp[cp_idx] == INTEGER_TAG && cp[cp_idx + 1] == 0)
        {
            if (setup_type == PUSH_SCRIPT_ID_ERR_CODE ||
                setup_type == PUSH_SCRIPT_ID_LAST_LP)
            {
                // immediately push
                if (setup_type == PUSH_SCRIPT_ID_ERR_CODE)
                    appl_push_msg_errcode();
                else
                    appl_push_msg_lastLP();
            }
            else
            {
                appl_resp_result = ACT_RESULT_DATA_NG;
            }
        }
        else
        {
            appl_resp_result = ACT_RESULT_DATA_NG;
        }
    }
    else
    {
        appl_resp_result = ACT_RESULT_DATA_NG;
    }
}

static void ob_dev_bcmd(uint16_t cmd)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    // cmd is not saved !!!
    switch (cmd) /* script_identifier */
    {
    case DEVICE_BCMD_CURPGM_READ:
        act_cmd_sel_ap = 0;
        break;

    case DEVICE_BCMD_FUTPGM_READ:
        act_cmd_sel_ap = 1;
        break;

    case DEVICE_BCMD_MDM_FACTORY_SETUP:
        dsp_r_sun_dsp_set();
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
        dsm_atcmd_set_reset(FALSE, AT_RST_FACTORY);
        break;

    case DEVICE_BCMD_MDM_RESET:
        dsm_modem_hw_reset(INT_MODEM_RESET);
        break;

    default:
        appl_resp_result = ACT_RESULT_DATA_NG;
        break;
    }
}

void ob_dev_cmd(uint16_t cmd)
{
    DPRINTF(DBG_TRACE, _D "%s: cmd[%d]\r\n", __func__, cmd);

    // cmd is saved !!!

    switch (cmd) /* script_identifier */
    {
    case DEVICE_CMD_INIT:
        act_devcmd = cmd;  // pre_action command
        break;

    case DEVICE_CMD_CURPGM_READ:
        /* Attributes 가 규격에는 2 로 명시되어 있는데 KVMK R4.12.13 기준으로
         * Attributes 가 1 로 내려옴. 현재는 Attributes 값을 판별하지 않아서
         * 정상적으로 처리됨. */
        act_cmd_sel_ap = 0;  // ref. act_is_curr_prog_cmd()
        act_devcmd = cmd;    // pre_action command
        break;

    case DEVICE_CMD_FUTPGM_READ:
        act_cmd_sel_ap = 1;
        act_devcmd = cmd;  // pre_action command
        break;

    case DEVICE_CMD_PGM_DL:
        memset((uint8_t*)&prog_dl, 0, sizeof(prog_dl_type));
        set_prog_dl_idx();
        set_hol_dl_idx();
        tou_set_cnt_reset();
        hol_date_block_init();

        prog_dlcmd_avail = true;

        act_devcmd = cmd;

        prog_cur_tou_fut_proc();
        futprog_partition_tou = 1;
        break;

    case DEVICE_CMD_FUTPGM_WORK:;
        bool rslt;
        rslt = prog_work_is_valid(false);
        tou_set_cnt_reset();
        if (rslt == false)
        {
            appl_resp_result = ACT_RESULT_OTHER;
            break;
        }

        if ((pdl_set_bits & SETBITS_PGM_NAME) &&
            (pdl_set_bits & SETBITS_PAS_TIME) &&
            (pdl_set_bits & SETBITS_SUPPDSP_ITEM))
        {
            DPRINTF(DBG_INFO, "%s: FUT_PROG_OK\r\n", __func__);
        }
        else
        {
            appl_resp_result = ACT_RESULT_OTHER;
            DPRINTF(DBG_ERR, "%s: FUT_PROG_FAIL\r\n", __func__);

            break;
        }

        prog_curr_npbill_date_backup();
        if (prog_fut_dl_proc(&prog_dl, appl_tbuff))
        {
            act_devcmd = cmd;
        }
        else
        {
            appl_resp_result = ACT_RESULT_DATA_NG;
        }
        break;

    case DEVICE_CMD_FUTPGM_DEL:
        prog_fut_delete();
        act_devcmd = cmd;
        break;

    case DEVICE_CMD_sCURR_INIT:
        if (scurr_is_limiting_forever())
        {
            scurr_limit_off();
            scurr_limit_init();
        }
        break;

    case DEVICE_CMD_SELREACT_CANCEL: /*선택 무효전력량 개별예약 삭제*/
        sel_react_cancel();
        break;

    default:
        appl_resp_result = ACT_RESULT_DATA_NG;
        break;
    }
}

static void ob_eob_cmd(uint16_t cmd)
{
    uint8_t eob_type = 0;

    if (mDR_sr_dr_type)
        eob_type = EOB_nPERIOD_FLAG;
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if (cmd == 2)
    {
        sr_dr_proc(eob_type, (mDR_sr_dr_type & MR_SR_BIT), &cur_rtc,
                   appl_tbuff);
    }
    else if (cmd == 3)
    {
        sr_dr_proc(eob_type, (mDR_sr_dr_type), &cur_rtc, appl_tbuff);
    }
    else
    {
        appl_resp_result = ACT_RESULT_DATA_NG;
    }
}

static void ob_rload_cmd(uint8_t cmd)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    rload_ctrl_set(cmd);
}

static void ob_holidays_cmd(uint8_t method, int idx)
{
    bool rslt = false;
    bool fut_act = false;
    uint16_t holidx, yr;
    uint8_t* cp;
    holiday_struct_type hol = {0};

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    cp = &appl_msg[idx];
    /*
    FEP 프로토콜 "계기 TOU 원격 전송 정보 관리" 참조 : Default ASDT의 호환성을
    위하여 휴일(Spacial day)의 day_id는 1로 정의한다. 통신 규격 "3.4.2.4.3 정기,
    비정기 휴일" 및 시험절차서 "4.6.2 TOU(휴일)" 참조할 것.

    정기/비정기 휴일 추가
        OBIS: 0 0 11 0 0 255
        Attributes: 1

    STT 300RRSTA : 정기휴일 index 추가 - "정기휴일 Index 02번", 5월 6일 추가
        DEC_APDU (size=27)
            C3 01 81 00 0B 00 00 0B 00 00 FF
            01 01 // ?, array tag, count 1
            02 03 // structure tag, count 3
            12 // long unsigned tag, 2 bytes
            00 02 // index : 2
            09 05 // octec string tag, 5 bytes
            FF FF // year : wild-card
            05 // month : 5
            06 // day : 6
            FF // ?
            11 // unsigned tag, 1 bytes
            01 // day id

    STT 300RRSTA : 비정기휴일 index 추가 - "비정기휴일 Index 143번", 2027월 2월
    10일 추가 DEC_APDU (size=27) C3 01 81 00 0B 00 00 0B 00 00 FF 01 01 02 03 12
            00 8F // index : 143
            09 05
            07 EB // year : 2027
            02 // month : 2
            0A // day : 10
            03 // ?
            11
            01 // day id

    STT 300RRSTA 정기휴일 index 삭제 - "정기휴일 Index 02번", 5월 5일 삭제
        DEC_APDU (size=16)
            C3 01 81 00 0B 00 00 0B 00 00 FF
            02 01 // structure tag, count 1
            12 // long unsigned tag, 2 bytes
            00 02 // index

    STT 300RRSTA : 비정기휴일 index 삭제 - "비정기휴일 Index 142번", 2027년 2월
    8일 삭제 DEC_APDU (size=16) C3 01 81 00 0B 00 00 0B 00 00 FF 02 01 12 00 8E
    // index : 142
    */
    switch (method)
    {
    case 1:  // insert
        DPRINTF(DBG_TRACE, "%s: Insert of Special Days Table\r\n", __func__);
        if (*cp == STRUCTURE_TAG && *(cp + 1) == 3)
        {
            cp += 2;

            if (*cp++ == LONGUNSIGNED_TAG)
            {
                ToH16((U8_16*)&holidx, cp);
                cp += 2;

                if (*cp == OCTSTRING_TAG && *(cp + 1) == 5)
                {
                    cp += 2;

                    ToH16((U8_16*)&yr, cp);
                    yr -= BASE_YEAR;
                    hol.month = *(cp + 2);
                    hol.date = *(cp + 3);
                    cp += 5;

                    if (*cp++ == UNSIGNED_TAG)
                    {
                        hol.day_id = *cp;
                        edit_holidays(true, fut_act, holidx, yr, &hol,
                                      appl_tbuff);

                        rslt = true;
                    }
                }
            }
        }
        break;

    case 2:  // delete
        DPRINTF(DBG_TRACE, "%s: Delete of Special Days Table\r\n", __func__);
        if (*cp++ == LONGUNSIGNED_TAG)
        {
            ToH16((U8_16*)&holidx, cp);
            edit_holidays(false, fut_act, holidx, /*(uint16_t)NULL*/ 0,
                          /*(holiday_struct_type *)NULL*/ &hol, appl_tbuff);

            rslt = true;
        }
        break;

    default:
        break;
    }

    if (rslt == false)
    {
        DPRINTF(DBG_TRACE, "%s: Error of Special Days Table\r\n", __func__);
        appl_resp_result = ACT_RESULT_DATA_NG;
    }
}

static void ob_assLN_cmd(uint8_t method, int idx)
{
    uint8_t *cp, *pfs2c;
    uint16_t len = 0;

    cp = &appl_msg[idx];

    DPRINTF(DBG_TRACE, _D "%s: cp[%d]\r\n", __func__, *cp);

    switch (method)
    {
    case 1:  // reply_to_HLS_authentication (data)
        if (*cp == METHOD_OPT_DATA_FLAG)
        {
            if (*(cp + 1) == OCTSTRING_TAG)
            {
                len = *(cp + 2);
                if (0x82 == len)
                {
                    len = (*(cp + 3) << 8) | (*(cp + 4));
                    pfs2c = (cp + 5);
                }
                else if (0x81 == len)
                {
                    len = *(cp + 3);
                    pfs2c = (cp + 4);
                }
                else
                    pfs2c = (cp + 3);
            }
        }
        else if (*cp == OCTSTRING_TAG)
        {
            len = *(cp + 1);
            if (0x82 == len)
            {
                len = (*(cp + 2) << 8) | (*(cp + 3));
                pfs2c = (cp + 4);
            }
            else if (0x81 == len)
            {
                len = *(cp + 2);
                pfs2c = (cp + 3);
            }
            else
                pfs2c = (cp + 2);
        }
        if (len > 0)
        {
            memcpy(fXTOX_s2c, pfs2c, len);
            if (dsm_sec_fs2c_verify(fXTOX_s2c, SYS_TITLE_client))
            {
                appl_set_conn_state(APPL_CROSS_AUTH_STATE);
            }
            else
            {
                appl_resp_result = ACT_RESULT_PRE_SEC_MUTUAL_AUTH_NG;
            }
        }

        break;
    case 2:

        break;
    case 3:

        break;
    case 4:

        break;
    }
}

static void parse_key_agreement(key_agree_type* parse)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    if (packed_ptr[packed_idx++] == STRUCTURE_TAG)
    {
        uint8_t member = packed_ptr[packed_idx++];
        if (packed_ptr[packed_idx++] == ENUM_TAG)
        {
            parse->id = packed_ptr[packed_idx++];
            DPRINTF(DBG_TRACE, "%s: id[%d]\r\n", __func__, parse->id);
        }
        else
        {
            DPRINTF(DBG_ERR, "%s: error 1\r\n", __func__);
        }

        if (packed_ptr[packed_idx++] == OCTSTRING_TAG)
        {
            parse->data_len = packed_ptr[packed_idx++];

            if (0x81 == parse->data_len)
            {
                parse->data_len = packed_ptr[packed_idx++];
            }

            if (KEY_DATA_SIZE < parse->data_len)
            {
                DPRINTF(DBG_TRACE, "%s: member %d\r\n", __func__, member);
                parse->data_len = KEY_DATA_SIZE;
            }

            memcpy(&parse->data[0], &packed_ptr[packed_idx], parse->data_len);
            DPRINT_HEX(DBG_TRACE, "RCV_KEY_DATA", &parse->data[0],
                       parse->data_len, /*DUMP_ALWAYS*/ DUMP_DLMS);
            packed_idx += parse->data_len;
        }
        else
        {
            DPRINTF(DBG_ERR, "%s: 3\r\n", __func__);
        }
    }
    DPRINTF(DBG_TRACE, "%s: end\r\n", __func__);
}

bool parse_key_info(uint8_t* cp, key_info_type* parse)
{
    int i;
    bool ret = false;

    packed_ptr = cp;
    packed_idx = 0;
    DPRINTF(DBG_TRACE, "%s: cp[%d]\r\n", __func__, *cp);
    if (packed_ptr[packed_idx] == ARRAY_TAG)
    {
        PARSE_ARRAY(parse->cnt);

        DPRINTF(DBG_TRACE, "%s: array[%d]\r\n", __func__, parse->cnt);

        if (parse->cnt > KEY_INFO_SIZE)
            parse->cnt = KEY_INFO_SIZE;

        for (i = 0; i < parse->cnt; i++)
        {
            parse_key_agreement(&parse->key_agree[i]);
        }

        ret = true;
    }

    return ret;
}

static void ob_security_setup_cmd(uint8_t method, int idx)
{
    uint8_t* cp;
    uint16_t cp_idx = 0;

    cp = &appl_msg[idx];

    DPRINTF(DBG_TRACE, "%s: method[%d], idx[%d], cp[%d]\r\n", __func__, method,
            idx, *cp);

    switch (method)
    {
    case 1:

        break;
    case 2:

        break;
    case 3:  // key_agreement (data)
    {
        key_info_type* pkey_info = (key_info_type*)appl_tbuff;
        bool ret = false;

        ret = parse_key_info(&cp[0], pkey_info);

        if (ret)
        {
            if (dsm_sec_key_agreement_process((uint8_t*)pkey_info,
                                              sizeof(key_info_type)))
            {
                appl_set_conn_state(APPL_KEY_AGREEMENT_STATE);
            }
            else
            {
                appl_resp_result = ACT_RESULT_PRE_SEC_KEYAGREEMENT_NG;
            }
        }
        else
        {
            appl_resp_result = ACT_RESULT_PRE_SEC_KEYAGREEMENT_NG;
        }
    }
    break;
    case 4:  // generate_key_pair
        if (cp[cp_idx] == ENUM_TAG)
        {
            uint8_t dlms_key_pair_type = cp[cp_idx + 1];

            if (dsm_sec_generate_key_pair(dlms_key_pair_type))
            {
                DPRINTF(DBG_TRACE, "%s: Key pair generation OK\r\n", __func__);
            }
            else
            {
                appl_resp_result = ACT_RESULT_OTHER;
            }
        }
        else
        {
            appl_resp_result = ACT_RESULT_OTHER;
        }
        break;
    case 5:  // generate_certificate_request
        if (cp[cp_idx] == ENUM_TAG)
        {
            uint8_t dlms_key_pair_type = cp[cp_idx + 1];

            if (dsm_sec_generate_csr(dlms_key_pair_type))
            {
                DPRINTF(DBG_TRACE, "%s: CSR generation OK\r\n", __func__);
            }
            else
            {
                appl_resp_result = ACT_RESULT_OTHER;
            }
        }
        else
        {
            appl_resp_result = ACT_RESULT_OTHER;
        }
        break;
    case 6:  // import_certificate
        if (cp[cp_idx] == OCTSTRING_TAG)
        {
            uint16_t cert_len;

            cp_idx += 1;

            if (cp[cp_idx] == 0x82)
            {
                cert_len = ((uint16_t)(cp[cp_idx + 1] << 8) |
                            (uint16_t)(cp[cp_idx + 2]));
                cp_idx += 3;
            }
            else if (cp[cp_idx] == 0x81)
            {
                cert_len = cp[cp_idx + 1];
                cp_idx += 2;
            }
            else
            {
                cert_len = cp[cp_idx];
                cp_idx += 1;
            }

            if (dsm_sec_import_certificate(&cp[cp_idx], cert_len))
            {
                DPRINTF(DBG_TRACE, "%s: import_certificate OK\r\n", __func__);
            }
            else
            {
                appl_resp_result = ACT_RESULT_OTHER;
            }
        }
        else
        {
            appl_resp_result = ACT_RESULT_OTHER;
        }
        break;
    case 7:  // export_certificate

        break;
    case 8:  // remove_certificate

        break;
    }
}

void ob_act_rsp_ass_LN(void)
{
    DPRINTF(DBG_TRACE, "%s: appl_att_id[%d], pPdu_idx[%d]\r\n", __func__,
            appl_att_id, pPdu_idx);

    switch (appl_att_id)
    {
    case 1:  // reply_to_HLS_authentication (data)
        FILL_STRING(fXTOX_LEN);

        DPRINT_HEX(DBG_CIPHER, "--> Tx Server Auth", fXTOX, fXTOX_LEN,
                   DUMP_SEC);
        printf("\r\n");

        memcpy(&pPdu[pPdu_idx], fXTOX, fXTOX_LEN);
        pPdu_idx += fXTOX_LEN;

        break;
    }
}

void ob_act_rsp_security_setup(void)
{
    // SECURITY_SETUP act_resp 전송: key_id | KA_PUB_server(64) | sign(64)
    DPRINTF(DBG_TRACE, "%s: method[%d], pPdu_idx[%d]\r\n", __func__,
            appl_att_id, pPdu_idx);

    switch (appl_att_id)
    {
    case 3:  // key_agreement (data)
        FILL_ARRAY(1);
        FILL_STRUCT(0x02);
        FILL_ENUM(server_key_agreedata[0]);
        FILL_STRING_1(128);
        memcpy(&pPdu[pPdu_idx], &server_key_agreedata[1], 128);  // key data
        pPdu_idx += 128;

        DPRINT_HEX(DBG_TRACE, "SERVER_KEY_DATA", server_key_agreedata, 129,
                   /*DUMP_ALWAYS*/ DUMP_DLMS);

        DPRINT_HEX(DBG_CIPHER, "<-- Tx Server Ephemeral Public Key",
                   &server_key_agreedata[1], 64, DUMP_SEC);
        DPRINT_HEX(DBG_CIPHER, "<-- Tx Server Ephemeral Public Key Signature",
                   &server_key_agreedata[65], 64, DUMP_SEC);
        printf("\r\n");
        break;

    case 4:  // generate_key_pair  (data)

        break;

    case 5:  // generate_certificate_request  (data)
        if (gst_csr_info.result)
        {
            if (gst_csr_info.cnt >= 256)
            {
                FILL_STRING_2(gst_csr_info.cnt);
                memcpy(&pPdu[pPdu_idx], gst_csr_info.info, gst_csr_info.cnt);
            }
            else if (gst_csr_info.cnt >= 128)
            {
                FILL_STRING_1(gst_csr_info.cnt);
                memcpy(&pPdu[pPdu_idx], gst_csr_info.info, gst_csr_info.cnt);
            }
            else
            {
                FILL_STRING(gst_csr_info.cnt);
                memcpy(&pPdu[pPdu_idx], gst_csr_info.info, gst_csr_info.cnt);
            }
            pPdu_idx += gst_csr_info.cnt;
            DPRINTF(DBG_TRACE, "CSR_data", gst_csr_info.info, gst_csr_info.cnt,
                    DUMP_ALWAYS);
        }

        break;
    }
}

void dsm_touImage_download_ready(void)
{
    memset((uint8_t*)&prog_dl, 0, sizeof(prog_dl_type));
    set_prog_dl_idx();
    set_hol_dl_idx();
    tou_set_cnt_reset();
    hol_date_block_init();
    prog_dlcmd_avail = true;
    act_devcmd = DEVICE_CMD_PGM_DL;
    DPRINTF(DBG_WARN, "%s: pro_dl_idx[%d]\r\n", __func__, get_prog_dl_idx());
}

#if 1 /* bccho, HASH, 2023-09-01 */
static ctx_handle_t hHandle = {[31] = 0xBB};
#endif
static void ob_touimage_transfer_cmd(uint8_t method, int idx)
{
    uint8_t *cp, *pt8 = NULL;
    uint16_t cp_idx = 0;
    uint32_t t32;
    uint16_t len, offset = 0;
    uint8_t* pimg_blk;
    EN_IMG_TYPE img_type = IMG__TOU;

    cp = &appl_msg[idx];

    DPRINTF(DBG_WARN, "%s: Specific methods[%d], idx[%d], val[%d]\r\n",
            __func__, method, idx, *cp);

    // Act Req
    /* 통신 규격 3.4.2.8.22 TOU Image Transfer : Specific methods */
    switch (method)
    {
    case 1:  // [image_transfer_initiate]
        if (cp[cp_idx] == STRUCTURE_TAG && cp[cp_idx + 1] == 2)
        {
            cp_idx += 2;
            if (cp[cp_idx] == OCTSTRING_TAG &&
                cp[cp_idx + 1] == IMAGE_NAME_MAX_SIZE)
            {
                dsm_imgtrfr_set_name(img_type, IMAGE_NAME_MAX_SIZE,
                                     &cp[cp_idx + 2]);
                cp_idx += (2 + IMAGE_NAME_MAX_SIZE);

                if (cp[cp_idx] == UDLONG_TAG)
                {
                    pt8 = &cp[cp_idx + 1];
                    ToH32((U8_16_32*)&t32, pt8);
                    dsm_imgtrfr_set_image_size(img_type, t32);
                    cp_idx += 5;

                    dsm_imgtrfr_set_transfer_status(img_type,
                                                    IMGTR_S_TRANSFER_INITIATED);
                    dsm_imgtrfr_set_blk_num(img_type, 0);
                    dsm_imgtrfr_set_rcvimage_size(img_type, 0);

                    dsm_imgtrfr_set_int_status_bits(img_type,
                                                    IMG_SBIT_VAL_ACTI_INIT);
                    dsm_imgtrfr_set_int_status_bits(img_type,
                                                    IMG_SBIT_VAL_NAME);

                    dsm_imgtrfr_touimage_info_save();

                    dsm_touImage_download_ready();

                    DPRINTF(DBG_WARN, "%s: initiate OK\r\n", __func__);
                }
                else
                    goto parser_imagetrfr_init_err;
            }
            else
                goto parser_imagetrfr_init_err;
        }
        else
        {
        parser_imagetrfr_init_err:
            appl_resp_result = ACT_RESULT_DATA_NG;
        }

        break;

    case 2:  // [image_block_transfer]
        if (!dsm_imgtrfr_get_int_status_bit(img_type, IMG_SBIT_POS_ACTI_INIT))
        {
            DPRINTF(DBG_ERR, "TOU_not_ready\r\n");
            appl_resp_result = ACT_RESULT_DATA_NG;
        }
        else if (cp[cp_idx] == STRUCTURE_TAG && cp[cp_idx + 1] == 2)
        {
            cp_idx += 2;
            if (cp[cp_idx] == UDLONG_TAG)
            {
                /* Block Number */
                pt8 = &cp[cp_idx + 1];
                ToH32((U8_16_32*)&t32, pt8);
                cp_idx += 5;

                if (cp[cp_idx] == OCTSTRING_TAG)
                {
                    /* Block Size */
                    len = cp[cp_idx + 1];
                    if (0x82 == len)
                    {
                        len = (cp[cp_idx + 2] << 8) | (cp[cp_idx + 3]);
                        pimg_blk = &cp[cp_idx + 4];
                        offset += 3;
                    }
                    else if (0x81 == len)
                    {
                        len = cp[cp_idx + 2];
                        pimg_blk = &cp[cp_idx + 3];
                        offset += 2;
                    }
                    else
                    {
                        pimg_blk = &cp[cp_idx + 2];
                        offset += 1;
                    }

                    if (t32 == 0)  // Block Number is 0
                    {
                        uint8_t* dst_id;
                        uint8_t get_id[MANUF_ID_SIZE];

                        dst_id = pimg_blk + 21;  // TOU Name(8) + 13
                        get_manuf_id(get_id);

                        if (memcmp(dst_id, get_id, MANUF_ID_SIZE) != 0)
                        {
                            DPRINTF(DBG_TRACE,
                                    "Meter ID of TOU does Not Match\r\n");

                            memset(get_id, '0', MANUF_ID_SIZE);
                            if (memcmp(dst_id, get_id, MANUF_ID_SIZE) !=
                                0)  // Check if all values ​​are '0'
                            {
                                DPRINTF(DBG_ERR, "Unknown Meter ID\r\n");
                                goto parser_imagetrfr_blk_err;
                            }
                            else
                            {
                                DPRINTF(DBG_TRACE,
                                        "Applicable to All Targets\r\n");
                            }
                        }
                    }

                    if (dsm_imgtrfr_get_blk_num(img_type) == t32)
                    {
                        int16_t ret = 0;
                        uint8_t* phash = dsm_imgtrfr_get_hash(img_type);
                        uint32_t img_size =
                            dsm_imgtrfr_get_image_size(img_type);
                        uint32_t img_rcv_size =
                            dsm_imgtrfr_get_rcvimage_size(img_type);

                        dsm_imgtrfr_reset_int_status_bits(
                            img_type, IMG_SBIT_VAL_LAST_TRFR);

                        if (img_rcv_size < img_size)
                        {
                            DPRINTF(DBG_TRACE, "Block_Size: %d\r\n", len);
                            dsm_imgtrfr_touimage_write(pimg_blk, img_rcv_size,
                                                       len);
                            dsm_imgtrfr_touimage_buff_update(pimg_blk,
                                                             img_rcv_size, len);
                            dsm_imgtrfr_set_rcvimage_size(img_type,
                                                          (img_rcv_size + len));

                            cp_idx += (1 + offset + len);  // 1 ->tag(1)

                            img_rcv_size =
                                dsm_imgtrfr_get_rcvimage_size(img_type);
                            if (img_rcv_size >= img_size)
                            {
                                DPRINTF(DBG_ERR, "TOU image rcv complete\r\n");
                                dsm_imgtrfr_set_int_status_bits(
                                    img_type, IMG_SBIT_VAL_LAST_TRFR);
                            }
                            if (t32 == 0)
                            {
                                if (dsm_imgtrfr_get_int_status_bit(
                                        img_type, IMG_SBIT_POS_LAST_TRFR))
                                {
#if 1 /* bccho, HASH, 2023-09-01 */
                                    if (axiocrypto_allocate_slot(
                                            hHandle, ASYM_ECDSA_P256, 0) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        MSGERROR(
                                            "axiocrypto_allocate_slot, Fail");
                                    }
                                    CRYPTO_STATUS cret;
                                    if ((cret = axiocrypto_hash_init(
                                             hHandle, HASH_SHA_256)) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        /* final 하기 전에 init을 하는 경우 발생
                                         * --> 펌웨어 다운로드 중지 하고 다시
                                         * 시작하는 경우 */
                                        if (cret == CRYPTO_ERR_HASH_CTX_IN_USE)
                                        {
                                            uint8_t dummy[IMAGE_HASH_SIZE];
                                            axiocrypto_hash_final(
                                                hHandle, dummy,
                                                IMAGE_HASH_SIZE);
                                            axiocrypto_hash_init(hHandle,
                                                                 HASH_SHA_256);
                                        }
                                        else
                                        {
                                            MSGERROR(
                                                "axiocrypto_hash_init, Fail, "
                                                "%d",
                                                cret);
                                        }
                                    }
                                    if (axiocrypto_hash_update(hHandle,
                                                               pimg_blk, len) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        MSGERROR(
                                            "axiocrypto_hash_update, Fail");
                                    }
                                    if (axiocrypto_hash_final(
                                            hHandle, phash, IMAGE_HASH_SIZE) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        MSGERROR("axiocrypto_hash_final, Fail");
                                    }
#else
                                    ret =
                                        _amiFuVerify(AMI_FU_NO_VERI, pimg_blk,
                                                     len, NULL, CERT_DS_CLIENT);
                                    _amiGetFuCodeHash(phash);
#endif

                                    dsm_imgtrfr_set_int_status_bits(
                                        img_type, IMG_SBIT_VAL_HASH_GEN);
                                    DPRINTF(
                                        DBG_TRACE,
                                        "TOU image SHA256: %d, blk_len[%d]\r\n",
                                        ret, len);
                                }
                                else
                                {
#if 1 /* bccho, HASH, 2023-09-01 */
                                    if (axiocrypto_allocate_slot(
                                            hHandle, ASYM_ECDSA_P256, 0) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        MSGERROR(
                                            "axiocrypto_allocate_slot, Fail");
                                    }
                                    CRYPTO_STATUS cret;
                                    if ((cret = axiocrypto_hash_init(
                                             hHandle, HASH_SHA_256)) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        /* final 하기 전에 init을 하는 경우 발생
                                         * --> 펌웨어 다운로드 중지 하고 다시
                                         * 시작하는 경우 */
                                        if (cret == CRYPTO_ERR_HASH_CTX_IN_USE)
                                        {
                                            uint8_t dummy[IMAGE_HASH_SIZE];
                                            axiocrypto_hash_final(
                                                hHandle, dummy,
                                                IMAGE_HASH_SIZE);
                                            axiocrypto_hash_init(hHandle,
                                                                 HASH_SHA_256);
                                        }
                                        else
                                        {
                                            MSGERROR(
                                                "axiocrypto_hash_init, Fail, "
                                                "%d",
                                                cret);
                                        }
                                    }
                                    if (axiocrypto_hash_update(hHandle,
                                                               pimg_blk, len) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        MSGERROR(
                                            "axiocrypto_hash_update, Fail");
                                    }
#else
                                    ret = _amiFuVerifyBegin(AMI_FU_NO_VERI,
                                                            pimg_blk, len, 0);
#endif
                                    DPRINTF(DBG_TRACE,
                                            "TOU image SHA256 Begin: %d, "
                                            "blk_len[%d]\r\n",
                                            ret, len);
                                }
                            }
                            else
                            {
                                if (!dsm_imgtrfr_get_int_status_bit(
                                        img_type, IMG_SBIT_POS_LAST_TRFR))
                                {
#if 1 /* bccho, HASH, 2023-09-01 */
                                    if (axiocrypto_hash_update(hHandle,
                                                               pimg_blk, len) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        MSGERROR(
                                            "axiocrypto_hash_update, Fail");
                                    }
#else
                                    ret = _amiFuVerifyMid(pimg_blk, len);
#endif
                                    DPRINTF(DBG_TRACE,
                                            "TOU image SHA256 Mid: %d, "
                                            "blk_len[%d]\r\n",
                                            ret, len);
                                }
                                else
                                {
#if 1 /* bccho, HASH, 2023-09-01 */
                                    if (axiocrypto_hash_update(hHandle,
                                                               pimg_blk, len) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        MSGERROR(
                                            "axiocrypto_hash_update, Fail");
                                    }
                                    if (axiocrypto_hash_final(
                                            hHandle, phash, IMAGE_HASH_SIZE) !=
                                        CRYPTO_SUCCESS)
                                    {
                                        MSGERROR("axiocrypto_hash_final, Fail");
                                    }
#else
                                    ret = _amiFuVerifyEnd(pimg_blk, len, NULL);
                                    _amiGetFuCodeHash(phash);
#endif
                                    dsm_imgtrfr_set_int_status_bits(
                                        img_type, IMG_SBIT_VAL_HASH_GEN);
                                    DPRINTF(DBG_TRACE,
                                            _D
                                            "TOU image SHA256 End: %d, "
                                            "blk_len[%d]\r\n",
                                            ret, len);
                                    DPRINT_HEX(DBG_TRACE, "TOU_IMG_HASH", phash,
                                               32, DUMP_ALWAYS);
                                }
                            }

                            dsm_imgtrfr_set_blk_num(img_type, (t32 + 1));
                            dsm_imgtrfr_touimage_info_save();
                        }
                    }
                    else
                        goto parser_imagetrfr_blk_err;
                }
                else
                    goto parser_imagetrfr_blk_err;
            }
            else
                goto parser_imagetrfr_blk_err;
        }
        else
        {
        parser_imagetrfr_blk_err:
            appl_resp_result = ACT_RESULT_DATA_NG;
        }
        break;

    case 3:  // [image_verify]
        if (cp[cp_idx] == INTEGER_TAG && cp[cp_idx + 1] == 0)
        {
            if (dsm_imgtrfr_activNverify_is_ready(img_type))
            {
                dsm_imgtrfr_set_transfer_status(img_type,
                                                IMGTR_S_VERIFY_SUCCESSFUL);
                dsm_imgtrfr_touimage_info_save();

                DPRINTF(DBG_ERR, "TOU image verified\r\n");
            }
            else
            {
                dsm_imgtrfr_set_transfer_status(img_type,
                                                IMGTR_S_VERIFY_FAILED);
                dsm_imgtrfr_touimage_info_save();
                DPRINTF(DBG_ERR, "TOU image verify ready fail!!!\r\n");

                goto parser_imagetrfr_verify_err;
            }
        }
        else
        {
        parser_imagetrfr_verify_err:
            appl_resp_result = ACT_RESULT_DATA_NG;
        }

        break;

    case 4:  // [image_activate]
        if (!dsm_imgtrfr_activNverify_is_ready(img_type))
        {
            DPRINTF(DBG_ERR, "TOU image active ready fail !!!\r\n");
            goto parser_imagetrfr_activate_err;
        }
        else if (cp[cp_idx] == INTEGER_TAG && cp[cp_idx + 1] == 0)
        {
            uint32_t crc_rlt;
            uint16_t i, idx = 0;
            ST_TOU_HEADER_INFO tou_hd;
            uint32_t img_size = dsm_imgtrfr_get_image_size(img_type);
            uint8_t* pimg = dsm_imgtrfr_touimage_get_buff();
            prg_tou_type = 0;

            DPRINTF(DBG_WARN, "TOU image activated\r\n");

            /* TOU Header */
            dsm_touHeader_parser(pimg, img_size, &idx, &tou_hd);

            /* TOU Body */
            crc_rlt = dsm_touBody_parserNprocess(&pimg[idx], &tou_hd);
            if (crc_rlt == FALSE)
            {
                goto parser_imagetrfr_activate_err;
            }

            /* bcho, 2023-11-14, undefine */
#if defined(FEATURE_JP_MLIST_CHECK_TOU_DOWN_LOAD)
            if ((tou_hd.meter_num == 0) ||
                (tou_hd.meter_num > TOU_METER_ID_NUM_MAX))
            {
                DPRINTF(DBG_ERR,
                        _D "TOU image parser_imagetrfr_no_activate\r\n");
                goto parser_imagetrfr_no_activate;
            }

            get_manuf_id(appl_tbuff);
            for (i = 0; i < tou_hd.meter_num; i++)
            {
                if (memcmp(appl_tbuff, &tou_hd.mid[i].aucMeterId,
                           AMI_METER_ID_LEN) == 0)
                {
                    goto parser_imagetrfr_activate_go;
                }
            }
            DPRINTF(DBG_ERR, _D "TOU image parser_imagetrfr_no_activate\r\n");
            goto parser_imagetrfr_no_activate;

        parser_imagetrfr_activate_go:
#endif
            tou_set_cnt_reset();

            tou_hd.tou_type = prg_tou_type;
            if (tou_hd.tou_type == 0)  // current TOU
            {
                DPRINTF(DBG_WARN, "TOU image CURRENT\r\n");
                prog_cur_dl_proc(&prog_dl, appl_tbuff);
            }
            else  // reserved TOU
            {
                DPRINTF(DBG_WARN, "TOU image RESERVED\r\n");
                prog_fut_dl_proc(&prog_dl, appl_tbuff);
                futprog_partition_tou = 0;
            }

            dsm_imgtrfr_con_init(img_type);
            dsm_imgtrfr_touimage_info_save();
        }
        else
        {
        parser_imagetrfr_activate_err:
            appl_resp_result = ACT_RESULT_DATA_NG;
        }
/* bcho, 2023-11-14, undefine */
#if defined(FEATURE_JP_MLIST_CHECK_TOU_DOWN_LOAD)
    parser_imagetrfr_no_activate:
#endif
        break;
    }
}

void ob_act_rsp_touimage_transfer(void)
{
    DPRINTF(DBG_TRACE, "%s: method[%d], idx[%d]\r\n", __func__, appl_att_id);

    switch (appl_att_id)
    {
    case 1:  //[image_transfer_initiate]

        break;
    case 2:  //[image_block_transfer]

        break;
    case 3:  //[image_verify]

        break;
    case 4:  //[image_activate]

        break;
    }
}

bool dsm_fw_is_valid(uint8_t fw_type, uint8_t* pname, date_time_type* pst,
                     date_time_type* psp)
{
    bool ret = false;

    if (fw_type == FW_DL_SYS_PART || fw_type == FW_DL_METER_PART)
    {
        if (pname[0] != ((METER_ID / 10) + '0') ||
            pname[1] != ((METER_ID % 10) + '0'))
        {
            return false;
        }

        if (pname[4] != COMPANY_ID_1 || pname[5] != COMPANY_ID_2)
        {
            return false;
        }
    }

    switch (fw_type)
    {
    case FW_DL_SYS_PART:
        if (pname[3] == '0')
            ret = true;

        break;
    case FW_DL_INT_MDM:
#if 0
		if(pname[0] != '7')
			return false;
		if(pname[2] == '1' && pname[3] == '1')
				ret = true;
#else
        if (pname[0] != '5')
            return false;
        if (pname[3] == '1')
            ret = true;
#endif
        break;
    case FW_DL_EXT_MDM:
#if 0 /* bccho, 2024-11-01 */
        if (pname[0] != '8')
            return false;
#endif
        if (pname[3] == '2')
            ret = true;

        break;
    case FW_DL_METER_PART:
        if (pname[3] == '3')
            ret = true;

#if 0  // JP.KIM 24.10.23
        if (cmp_date_time(&cur_rtc, pst) >= 0 &&
            cmp_date_time(psp, &cur_rtc) > 0)
        {
            ret = true;
        }
        else
        {
            ret = false;
            DPRINTF(DBG_TRACE, "TIME FAIL!!!\r\n");
        }
#endif
        break;
    default:
        break;
    }

    return ret;
}

EN_IMG_TYPE g_imggo_img_type = IMG__NUM;
static void ob_fwimage_transfer_cmd(uint8_t method, int idx)
{
    uint8_t *cp, *pt8 = NULL;
    uint16_t cp_idx = 0;
    uint32_t t32;
    uint16_t len, offset = 0;
    uint8_t* pimg_blk;
    EN_IMG_TYPE img_type = IMG__FW;
    // uint32_t flash_addr = 0;
    uint8_t fw_type;
    date_time_type st, sp;

    cp = &appl_msg[idx];

    DPRINTF(DBG_INFO, "%s: Specific methods %d, idx[%d], val[%d]\r\n", __func__,
            method, idx, *cp);

    // Act Req
    /* 통신 규격 3.4.2.11.3 소프트웨어 업데이트 Image transfer : Specific
     * methods ※ methods 사용방법은 DLMS Blue Book Ed 12.2 4.4.6.4 Standard
     * Image Transfer Process를 따른다. */
    switch (method)
    {
    case 1: /* image_transfer_initiate */
        DPRINTF(DBG_TRACE, "[image_transfer_initiate]\r\n");

        /* bccho, 2025-06-13 */
        if (dsm_imgtrfr_fwup_get_fsm() == FWU_FSM_UP_OP)
        {
            appl_resp_result = ACT_RESULT_DATA_NG;
            break;
        }

        if ((cp[cp_idx] == STRUCTURE_TAG) && (cp[cp_idx + 1] == 2))
        {
            cp_idx += 2;
            if ((cp[cp_idx] == OCTSTRING_TAG) &&
                (cp[cp_idx + 1] == IMAGE_FW_NAME_MAX_SIZE))
            {
                uint8_t*
                    img_name;  // 53HTMMSSTTTTTT (H: H/W Ver, T: S/W Type, M:
                // Manufacturer ID, S: S/W Ver, T: Build Date)

                img_name = &cp[cp_idx + 2];

                dsm_imgtrfr_set_name(img_type, IMAGE_FW_NAME_MAX_SIZE,
                                     &cp[cp_idx + 2]);  // SET_IMG_NAME
                cp_idx += (2 + IMAGE_FW_NAME_MAX_SIZE);

                if (cp[cp_idx] == UDLONG_TAG)
                {
                    pt8 = &cp[cp_idx + 1];
                    ToH32((U8_16_32*)&t32, pt8);
                    dsm_imgtrfr_set_image_size(img_type, t32);
                    cp_idx += 5;

                    dsm_imgtrfr_set_transfer_status(img_type,
                                                    IMGTR_S_TRANSFER_INITIATED);
                    dsm_imgtrfr_set_blk_num(img_type, 0);
                    dsm_imgtrfr_set_rcvimage_size(img_type, 0);

                    dsm_imgtrfr_set_int_status_bits(img_type,
                                                    IMG_SBIT_VAL_ACTI_INIT);
                    dsm_imgtrfr_set_int_status_bits(img_type,
                                                    IMG_SBIT_VAL_NAME);

                    // FW_IMG_NAME,
                    // 0: 운영부,
                    // 1: 내장 모뎀,
                    // 2: 착탈형 모뎀,
                    // 3: 계량부
                    fw_type = dsm_imgtrfr_fwimage_dl_get_fw_type();

                    // jp.kim 25.12.06 //계량부 무결성 검증
                    if (fw_type == FW_DL_METER_PART)
                    {
                        g_meter_fw_chk_sum_err = 0;
                    }

                    uint8_t* pfwname = dsm_imgtrfr_get_name(img_type);
                    if (!dsm_fw_is_valid(fw_type, pfwname, &st, &sp))
                    {
                        goto parser_imagetrfr_init_err;
                    }

                    dsm_imgtrfr_set_fw_type(fw_type);

                    dsm_imgtrfr_fwimage_act_init_process(
                        fw_type);  // set addr, first sector erase, save of page
                                   // size and received length
                    dsm_imgtrfr_fwimage_dl_info_save();

                    DPRINTF(DBG_WARN, "%s: initiate OK\r\n", __func__);
                }
                else
                {
                    goto parser_imagetrfr_init_err;
                }
            }
            else
            {
                goto parser_imagetrfr_init_err;
            }
        }
        else
        {
        parser_imagetrfr_init_err:
            appl_resp_result = ACT_RESULT_DATA_NG;
        }
        break;

    case 2: /* image_block_transfer */
        DPRINTF(DBG_TRACE, "[image_block_transfer]\r\n");

        /* bccho, 2025-06-13 */
        if (dsm_imgtrfr_fwup_get_fsm() == FWU_FSM_UP_OP)
        {
            appl_resp_result = ACT_RESULT_DATA_NG;
            break;
        }

        if (!dsm_imgtrfr_get_int_status_bit(img_type, IMG_SBIT_POS_ACTI_INIT))
        {
            DPRINTF(DBG_ERR, _D "FW image act_init is not ready\r\n");
            appl_resp_result = ACT_RESULT_DATA_NG;
        }
        else if (cp[cp_idx] == STRUCTURE_TAG && cp[cp_idx + 1] == 2)
        {
            dsm_imgtrfr_fwup_set_fsm(FWU_FSM_UP);
            cp_idx += 2;
            if (cp[cp_idx] == UDLONG_TAG)
            {
                /* Block Number */
                pt8 = &cp[cp_idx + 1];
                ToH32((U8_16_32*)&t32, pt8);
                cp_idx += 5;

                if (cp[cp_idx] == OCTSTRING_TAG)
                {
                    /* Block Size */
                    len = cp[cp_idx + 1];
                    if (0x82 == len)
                    {
                        len = (cp[cp_idx + 2] << 8) | (cp[cp_idx + 3]);
                        pimg_blk = &cp[cp_idx + 4];
                        offset += 3;
                    }
                    else if (0x81 == len)
                    {
                        len = cp[cp_idx + 2];
                        pimg_blk = &cp[cp_idx + 3];
                        offset += 2;
                    }
                    else
                    {
                        pimg_blk = &cp[cp_idx + 2];
                        offset += 1;
                    }

                    if (dsm_imgtrfr_get_blk_num(img_type) == t32)
                    {
                        bool flash_write_ok = 0;
                        uint8_t mdm_fl_write_retry_cnt = 0;
                        uint32_t img_size =
                            dsm_imgtrfr_get_image_size(img_type);
                        uint32_t img_rcv_size =
                            dsm_imgtrfr_get_rcvimage_size(img_type);

                        dsm_imgtrfr_reset_int_status_bits(
                            img_type, IMG_SBIT_VAL_LAST_TRFR);

                        if (img_rcv_size < img_size)
                        {
                            fw_type = dsm_imgtrfr_fwimage_dl_get_fw_type();
#if 1 /* bccho, 2023-10-02 */
                            /* 첫번째 블락 */
                            if (t32 == 0 && fw_type == FW_DL_SYS_PART)
                            {
                                /* Securityflatform */
                                if (memcmp(pimg_blk + 16,
                                           "\x53\x65\x63\x75\x72\x69\x74\x79"
                                           "\x70\x6C\x61\x74\x66\x6F\x72\x6D",
                                           16) != 0)
                                {
                                    MSGERROR("NOT Our FW");
                                    goto parser_imagetrfr_blk_err;
                                }
                            }
#endif
                            DPRINTF(DBG_TRACE, "Block_Size: %d\r\n", len);

                            if (fw_type == FW_DL_SYS_PART)
                            {
                                flash_write_ok = dsm_imgtrfr_fwimage_update(
                                    fw_type, false, pimg_blk, len);
                                dsm_imgtrfr_set_rcvimage_size(
                                    img_type, (img_rcv_size + len));
                                cp_idx += (1 + offset + len);
                                img_rcv_size =
                                    dsm_imgtrfr_get_rcvimage_size(img_type);
                                if (img_rcv_size >= img_size)
                                {
                                    DPRINTF(DBG_WARN,
                                            _D "FW image rcv complete!!!\r\n");
                                    dsm_imgtrfr_set_int_status_bits(
                                        img_type, IMG_SBIT_VAL_LAST_TRFR);

                                    if (flash_write_ok == false)
                                    {
                                        dsm_imgtrfr_fwimage_update(
                                            fw_type, true, pimg_blk,
                                            len);  // if force is ture, last
                                                   // block
                                    }
                                }

                                dsm_imgtrfr_fwimage_hash(fw_type, pimg_blk,
                                                         len);
                                dsm_imgtrfr_set_blk_num(img_type, (t32 + 1));
                                dsm_imgtrfr_fwimage_dl_info_save();
                            }
                            else
                            {
                                mdm_fl_write_retry_cnt = ATCMD_RETRY_MAX_COUNT;

                                while (mdm_fl_write_retry_cnt--)
                                {
                                    flash_write_ok = dsm_imgtrfr_fwimage_update(
                                        fw_type, false, pimg_blk,
                                        len);  // block write

                                    if (flash_write_ok == true)
                                    {
                                        dsm_imgtrfr_set_rcvimage_size(
                                            img_type, (img_rcv_size + len));
                                        cp_idx +=
                                            (1 + offset + len);  // 1 ->tag(1)

                                        img_rcv_size =
                                            dsm_imgtrfr_get_rcvimage_size(
                                                img_type);
                                        if (img_rcv_size >= img_size)
                                        {
                                            DPRINTF(
                                                DBG_WARN, _D
                                                "FW image rcv complete!!!\r\n");
                                            dsm_imgtrfr_set_int_status_bits(
                                                img_type,
                                                IMG_SBIT_VAL_LAST_TRFR);
                                        }

                                        dsm_imgtrfr_fwimage_hash(fw_type,
                                                                 pimg_blk, len);
                                        dsm_imgtrfr_set_blk_num(img_type,
                                                                (t32 + 1));
                                        dsm_imgtrfr_fwimage_dl_info_save();
                                        break;
                                    }
                                }
                            }

                            if (fw_type == FW_DL_INT_MDM ||
                                fw_type == FW_DL_EXT_MDM)
                            {
                                if (flash_write_ok == false)
                                {
                                    goto parser_imagetrfr_blk_err;
                                }
                            }
                        }
                    }
                    else
                    {
                        goto parser_imagetrfr_blk_err;
                    }
                }
                else
                {
                    goto parser_imagetrfr_blk_err;
                }
            }
            else
            {
                goto parser_imagetrfr_blk_err;
            }
        }
        else
        {
        parser_imagetrfr_blk_err:
            appl_resp_result = ACT_RESULT_DATA_NG;
        }
        break;

    case 3: /* image_verify */
        DPRINTF(DBG_TRACE, "[image_verify]\r\n");
        if (cp[cp_idx] == INTEGER_TAG && cp[cp_idx + 1] == 0)
        {
            if (dsm_imgtrfr_activNverify_is_ready(img_type))
            {
                fw_type = dsm_imgtrfr_fwimage_dl_get_fw_type();
                if (fw_type == FW_DL_SYS_PART)  // 운영부
                {
                    uint32_t size = dsm_imgtrfr_get_rcvimage_size(IMG__FW);
                    if (!check_sys_fw_crc(size))
                    {
                        dsm_imgtrfr_set_transfer_status(img_type,
                                                        IMGTR_S_VERIFY_FAILED);
                        DPRINTF(DBG_ERR,
                                _D "SYS FW image CRC check fail!!!\r\n");
                        appl_resp_result = ACT_RESULT_OTHER;
                    }
                    else
                    {
                        dsm_imgtrfr_set_transfer_status(
                            img_type, IMGTR_S_VERIFY_SUCCESSFUL);
                        DPRINTF(DBG_WARN,
                                _D "SYS FW image verified SUCCESS!!!\r\n");
                    }
                }
                else if (fw_type == FW_DL_METER_PART)  // 계량
                {
                    if (!g_meter_fw_chk_sum_err)
                    {
                        dsm_imgtrfr_set_transfer_status(
                            img_type, IMGTR_S_VERIFY_SUCCESSFUL);
                        DPRINTF(DBG_WARN,
                                _D "METER FW image verified SUCCESS!!!\r\n");
                    }
                    else
                    {
                        dsm_imgtrfr_set_transfer_status(img_type,
                                                        IMGTR_S_VERIFY_FAILED);
                        DPRINTF(
                            DBG_ERR, _D
                            "METER FW image verify check sum cal fail!!! \r\n");
                        appl_resp_result = ACT_RESULT_OTHER;
                    }
                }
                else if (fw_type == FW_DL_INT_MDM)  // SUN
                {
                    uint8_t* phash =
                        dsm_imgtrfr_get_hash(IMG__FW);  // get and set hash

                    dsm_atcmd_if_is_valid(MEDIA_RUN_SUN, true);  // sha256

                    memset(g_get_hash, 0x00, IMAGE_HASH_SIZE);
                    vTaskDelay(100);
                    dsm_atcmd_if_is_valid(MEDIA_RUN_SUN, true);  // sha256
                    // 상호간 비교
                    if (!memcmp(phash, g_get_hash, IMAGE_HASH_SIZE))
                    {
                        dsm_imgtrfr_set_transfer_status(
                            img_type, IMGTR_S_VERIFY_SUCCESSFUL);
                        DPRINTF(DBG_WARN,
                                _D "FW image verified HASH SUCCESS!!!\r\n");
                    }
                    else  // ERROR // 서로 다르면 verify error
                    {
                        dsm_imgtrfr_set_transfer_status(img_type,
                                                        IMGTR_S_VERIFY_FAILED);
                        DPRINTF(DBG_ERR,
                                _D "FW image verify HASH cal fail!!! \r\n");
                        appl_resp_result = ACT_RESULT_OTHER;
                    }

                    DPRINT_HEX(DBG_TRACE, "fw_info.hash(AMIGO)", phash,
                               IMAGE_HASH_SIZE, DUMP_ALWAYS);
                    DPRINT_HEX(DBG_TRACE, "SUN -> g_get_hash", g_get_hash,
                               IMAGE_HASH_SIZE, DUMP_ALWAYS);
                }
                else  // 외장
                {
                    dsm_imgtrfr_set_transfer_status(img_type,
                                                    IMGTR_S_VERIFY_SUCCESSFUL);
                }

                DPRINTF(DBG_WARN, _D "FW image verified!!!\r\n");
            }
            else
            {
                dsm_imgtrfr_set_transfer_status(img_type,
                                                IMGTR_S_VERIFY_FAILED);
                DPRINTF(DBG_ERR, _D "FW image verify ready fail!!! ");
                appl_resp_result = ACT_RESULT_VERIFY_FAILED;
            }
        }
        else
        {
            appl_resp_result = ACT_RESULT_DATA_NG;
        }
        break;

    case 4: /* image_activate */
        DPRINTF(DBG_TRACE, "[image_activate]\r\n");

        /* bccho, 2025-06-13 */
        if (dsm_imgtrfr_fwup_get_fsm() == FWU_FSM_UP_OP)
        {
            appl_resp_result = ACT_RESULT_DATA_NG;
            break;
        }

        if (!dsm_imgtrfr_activNverify_is_ready(img_type))
        {
            DPRINTF(DBG_ERR, _D "FW image active ready fail !!!\r\n");
            goto parser_imagetrfr_activate_err;
        }
        else if (cp[cp_idx] == INTEGER_TAG && cp[cp_idx + 1] == 0)
        {
            uint8_t fw_type;
            fw_type = dsm_imgtrfr_fwimage_dl_get_fw_type();
            dsm_imgtrfr_fwup_set_fsm(FWU_FSM_UP_OP);
            DPRINTF(DBG_ERR, _D "FW image activated!!!\r\n");
            dsm_imgtrfr_set_fw_type(fw_type);
            g_imggo_img_type = img_type;
            M_MT_SW_GENERAL_TIMER_set_image_update_go();
            dsm_meter_sw_timer_start(MT_SW_GENERAL_TO, FALSE,
                                     MT_TIMEOUT_IMG_UPDATE_GO_TIME);
        }
        else
        {
        parser_imagetrfr_activate_err:
            appl_resp_result = ACT_RESULT_DATA_NG;
        }

        break;
    }
}

void dsm_image_update_go_proc(void)
{
    // 0: 운영부, 1: 내장 모뎀, 2: 착탈형 모뎀, 3: 계량부
    uint8_t fw_type = dsm_imgtrfr_get_fw_type();
    ST_FW_IMG_DOWNLOAD_INFO* pimage_dlinfo = NULL;

    // 0: TOU, 1: FW
    uint8_t img_type = g_imggo_img_type;

    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();
    float fval;
    U8 i = 0;

    if (M_MT_SW_GENERAL_TIMER_is_image_update_go())
    {
        DPRINTF(DBG_WARN, "%s: img_type[%d], fw_type[%d]\r\n", __func__,
                img_type, fw_type);

        if (fw_type == FW_DL_METER_PART)
        {
            pushd->rst_accum.DeliAct = 0;
            pushd->rst_accum.DLagReact = 0;
            pushd->rst_accum.DLeadReact = 0;
            pushd->rst_accum.DeliApp = 0;

            pushd->rst_accum.ReceiAct = 0;
            pushd->rst_accum.RLagReact = 0;
            pushd->rst_accum.RLeadReact = 0;
            pushd->rst_accum.ReceiApp = 0;

            for (i = 0; i < 3; i++)
            {
                VOLT_FWUP_BACK[i] = get_inst_volt(i);
                CURR_FWUP_BACK[i] = get_inst_curr(i);
                PH_FWUP_BACK[i] = get_inst_phase(i);
            }

            PH_LTOL_FWUP_BACK[0] = pushd->rs_phase;
            PH_LTOL_FWUP_BACK[1] = pushd->rt_phase;
            PULSE_DIR_MODIFY_BACK = pulse_dir;

            PULSE_ADD_MODIFY_DATA = 0.0;
            PULSE_ADD_MODIFY_DATA_VA = 0.0;
            PULSE_ADD_MODIFY_DATA_VAR = 0.0;
            METER_FW_UP_ING_STS = 1;
            METER_FW_UP_END_PULSE_MODIFY = 0;
            METER_FW_UP_ING_CNT = 0;
        }

        dsm_meter_sw_timer_stop(MT_SW_GENERAL_TO);
        M_MT_SW_GENERAL_TIMER_clear_image_update_go();

        pimage_dlinfo = dsm_imgtrfr_get_fw_image_dlinfo();
        uint8_t fw_name[IMAGE_FW_NAME_MAX_SIZE + 1] = {0};
        memcpy(fw_name, pimage_dlinfo->name, IMAGE_FW_NAME_MAX_SIZE);
        DPRINTF(DBG_WARN, "========================================\r\n");
        DPRINTF(DBG_WARN, "Block Size: %d, Block Count: %d\r\n",
                pimage_dlinfo->blk_size, pimage_dlinfo->blk_number);
        DPRINTF(DBG_WARN, "Addr: 0x%08X, Image Size: %d(0x%x)\r\n",
                pimage_dlinfo->start_dl_addr, pimage_dlinfo->image_size,
                pimage_dlinfo->image_size);
        DPRINTF(DBG_WARN, "FW Type: %d, Image Identifier: %s\r\n",
                pimage_dlinfo->fw_type, fw_name);
        DPRINTF(DBG_WARN, "========================================\r\n");

        dsm_uart_deq_string(DEBUG_COM);

        // if fw_type is FW_DL_SYS_PART, system reset
        dsm_imgtrfr_fwimage_act_run_process(fw_type);

        if (fw_type == FW_DL_SYS_PART)
        {
            dsm_imgtrfr_con_init(img_type);
        }

        // if fw_type == FW_DL_SYS_PART, this code is duplicate
        dsm_imgtrfr_fwup_set_fsm(FWU_FSM_NONE);
        g_imggo_img_type = IMG__NUM;
    }
}

#define LIGHT_LOAD 1
#define MIDDLE_LOAD 2
#define MAXIMUM_LOAD 3

season_struct_type g_default_season[4] = {
    {3, 1, 0}, {6, 1, 1}, {9, 1, 2}, {11, 1, 3}};

// 1종
week_struct_type g_default_week[4] = {{0, {1, 1, 1, 1, 1, 1, 1}},
                                      {1, {1, 1, 1, 1, 1, 1, 1}},
                                      {2, {1, 1, 1, 1, 1, 1, 1}},
                                      {3, {1, 1, 1, 1, 1, 1, 1}}};

// 2종
week_struct_type g_default_tariff_2_type_week[4] = {{0, {2, 2, 2, 2, 2, 2, 2}},
                                                    {1, {2, 2, 2, 2, 2, 2, 2}},
                                                    {2, {2, 2, 2, 2, 2, 2, 2}},
                                                    {3, {2, 2, 2, 2, 2, 2, 2}}};

tou_struct_type g_default_day_0[12] = {
    {8, 0, MIDDLE_LOAD},   {11, 0, MAXIMUM_LOAD}, {12, 0, MIDDLE_LOAD},
    {13, 0, MAXIMUM_LOAD}, {18, 0, MIDDLE_LOAD},  {22, 0, LIGHT_LOAD},
    {0xff, 0xff, 1},       {0xff, 0xff, 1},       {0xff, 0xff, 1},
    {0xff, 0xff, 1},       {0xff, 0xff, 1},       {0xff, 0xff, 1}};
tou_struct_type g_default_day_1[12] = {
    {0, 0, LIGHT_LOAD}, {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1},    {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1},    {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1}};

#if 1 /* bccho, 2025-04-02 */
tou_struct_type g_default_day_2[12] = {
    {8, 0, MIDDLE_LOAD}, {22, 0, LIGHT_LOAD}, {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1},     {0xff, 0xff, 1},     {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1},     {0xff, 0xff, 1},     {0xff, 0xff, 1}, {0xff, 0xff, 1}};
#else
tou_struct_type g_default_day_2[12] = {
    {9, 0, MIDDLE_LOAD}, {23, 0, LIGHT_LOAD}, {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1},     {0xff, 0xff, 1},     {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1},     {0xff, 0xff, 1},     {0xff, 0xff, 1}, {0xff, 0xff, 1}};
#endif

tou_struct_type g_default_day_3[12] = {
    {8, 0, MIDDLE_LOAD},   {9, 0, MAXIMUM_LOAD}, {12, 0, MIDDLE_LOAD},
    {16, 0, MAXIMUM_LOAD}, {19, 0, MIDDLE_LOAD}, {22, 0, LIGHT_LOAD},
    {0xff, 0xff, 1},       {0xff, 0xff, 1},      {0xff, 0xff, 1},
    {0xff, 0xff, 1},       {0xff, 0xff, 1},      {0xff, 0xff, 1}};
tou_struct_type g_default_day_4[12] = {
    {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1}};
tou_struct_type g_default_day_5[12] = {
    {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1},
    {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1}, {0xff, 0xff, 1}};

#define YEAR_OFFSET_BLK_0 63
#define HOLIDAYS_PER_BLOCK_BLK_0 8  // 최대 블럭 20개 : index 0 ~ 19
holiday_struct_type g_holiday_blk_0[HOLIDAYS_PER_BLOCK_BLK_0] = {
    /* 정기 휴일 20개, index 0 ~ 19 */
    {1, 1, 1},  {3, 1, 1},  {5, 5, 1},  {6, 6, 1},
    {8, 15, 1}, {10, 3, 1}, {10, 9, 1}, {12, 25, 1}};

/* bccho, 2024-09-05, 삼상 */
/* 시작 년도 : 23년 */
#define YEAR_OFFSET_BLK_1 23  // 20 //15

// 23
#define HOLIDAYS_PER_BLOCK_BLK_1 9
holiday_struct_type g_holiday_blk_1[HOLIDAYS_PER_BLOCK_BLK_1] = {
    /* 비정기 휴일 20개, index 20 ~ 39 */
    {1, 21, 1}, {1, 22, 1}, {1, 23, 1}, {1, 24, 1}, {5, 27, 1},
    {5, 29, 1}, {9, 28, 1}, {9, 29, 1}, {9, 30, 1}

};
// 24
#define HOLIDAYS_PER_BLOCK_BLK_2 10
holiday_struct_type g_holiday_blk_2[HOLIDAYS_PER_BLOCK_BLK_2] = {
    {2, 9, 1},  {2, 10, 1}, {2, 11, 1}, {2, 12, 1}, {4, 10, 1}, {5, 6, 1},
    {5, 15, 1}, {9, 16, 1}, {9, 17, 1}, {9, 18, 1}

};
// 25
#define HOLIDAYS_PER_BLOCK_BLK_3 10
holiday_struct_type g_holiday_blk_3[HOLIDAYS_PER_BLOCK_BLK_3] = {
    {1, 28, 1}, {1, 29, 1}, {1, 30, 1}, {3, 3, 1}, {5, 5, 1}, {5, 6, 1},
    {10, 5, 1}, {10, 6, 1}, {10, 7, 1}, {10, 8, 1}

};
// 26
#define HOLIDAYS_PER_BLOCK_BLK_4 12
holiday_struct_type g_holiday_blk_4[HOLIDAYS_PER_BLOCK_BLK_4] = {
    {2, 16, 1}, {2, 17, 1}, {2, 18, 1}, {3, 2, 1},  {5, 24, 1}, {5, 25, 1},
    {6, 3, 1},  {8, 17, 1}, {9, 24, 1}, {9, 25, 1}, {9, 26, 1}, {10, 5, 1}};
// 27
#define HOLIDAYS_PER_BLOCK_BLK_5 13
holiday_struct_type g_holiday_blk_5[HOLIDAYS_PER_BLOCK_BLK_5] = {
    {2, 6, 1},  {2, 7, 1},   {2, 8, 1},  {2, 9, 1},  {3, 3, 1},
    {5, 13, 1}, {8, 16, 1},  {9, 14, 1}, {9, 15, 1}, {9, 16, 1},
    {10, 4, 1}, {10, 11, 1}, {12, 27, 1}};
// 28
#define HOLIDAYS_PER_BLOCK_BLK_6 9
holiday_struct_type g_holiday_blk_6[HOLIDAYS_PER_BLOCK_BLK_6] = {
    {1, 26, 1}, {1, 27, 1}, {1, 28, 1}, {4, 12, 1}, {5, 2, 1},
    {10, 2, 1}, {10, 3, 1}, {10, 4, 1}, {10, 5, 1}};
// 29
#define HOLIDAYS_PER_BLOCK_BLK_7 10
holiday_struct_type g_holiday_blk_7[HOLIDAYS_PER_BLOCK_BLK_7] = {
    {2, 12, 1}, {2, 13, 1}, {2, 14, 1}, {5, 7, 1},  {5, 20, 1},
    {5, 21, 1}, {9, 21, 1}, {9, 22, 1}, {9, 23, 1}, {9, 24, 1}};
// 30
#define HOLIDAYS_PER_BLOCK_BLK_8 10
holiday_struct_type g_holiday_blk_8[HOLIDAYS_PER_BLOCK_BLK_8] = {
    {2, 2, 1}, {2, 3, 1},  {2, 4, 1},  {2, 5, 1},  {5, 6, 1},
    {5, 9, 1}, {6, 12, 1}, {9, 11, 1}, {9, 12, 1}, {9, 13, 1}};
// 31
#define HOLIDAYS_PER_BLOCK_BLK_9 8
holiday_struct_type g_holiday_blk_9[HOLIDAYS_PER_BLOCK_BLK_9] = {
    {1, 22, 1}, {1, 23, 1}, {1, 24, 1}, {3, 3, 1},
    {5, 28, 1}, {9, 30, 1}, {10, 1, 1}, {10, 2, 1}};
// 32
#define HOLIDAYS_PER_BLOCK_BLK_10 15
holiday_struct_type g_holiday_blk_10[HOLIDAYS_PER_BLOCK_BLK_10] = {
    {2, 10, 1}, {2, 11, 1}, {2, 12, 1}, {3, 3, 1},   {4, 14, 1},
    {5, 16, 1}, {5, 17, 1}, {8, 16, 1}, {9, 18, 1},  {9, 19, 1},
    {9, 20, 1}, {9, 21, 1}, {10, 4, 1}, {10, 11, 1}, {12, 27, 1}};
// 33
#define HOLIDAYS_PER_BLOCK_BLK_11 10
holiday_struct_type g_holiday_blk_11[HOLIDAYS_PER_BLOCK_BLK_11] = {
    {1, 30, 1}, {1, 31, 1}, {2, 1, 1}, {2, 2, 1},   {5, 6, 1},
    {9, 7, 1},  {9, 8, 1},  {9, 9, 1}, {10, 10, 1}, {12, 26, 1}};
// 34
#define HOLIDAYS_PER_BLOCK_BLK_12 9
holiday_struct_type g_holiday_blk_12[HOLIDAYS_PER_BLOCK_BLK_12] = {
    {2, 18, 1}, {2, 19, 1}, {2, 20, 1}, {2, 21, 1}, {5, 25, 1},
    {5, 31, 1}, {9, 26, 1}, {9, 27, 1}, {9, 28, 1}};
// 35
#define HOLIDAYS_PER_BLOCK_BLK_13 9
holiday_struct_type g_holiday_blk_13[HOLIDAYS_PER_BLOCK_BLK_13] = {
    {2, 7, 1},  {2, 8, 1},  {2, 9, 1},  {5, 7, 1}, {5, 15, 1},
    {9, 15, 1}, {9, 16, 1}, {9, 17, 1}, {9, 18, 1}};
// 36
#define HOLIDAYS_PER_BLOCK_BLK_14 13
holiday_struct_type g_holiday_blk_14[HOLIDAYS_PER_BLOCK_BLK_14] = {
    {1, 27, 1}, {1, 28, 1}, {1, 29, 1}, {1, 30, 1}, {3, 3, 1},
    {4, 9, 1},  {5, 3, 1},  {5, 6, 1},  {10, 3, 1}, {10, 4, 1},
    {10, 5, 1}, {10, 6, 1}, {10, 7, 1}};
// 37
#define HOLIDAYS_PER_BLOCK_BLK_15 12
holiday_struct_type g_holiday_blk_15[HOLIDAYS_PER_BLOCK_BLK_15] = {
    {2, 14, 1}, {2, 15, 1}, {2, 16, 1}, {2, 17, 1}, {3, 2, 1},  {3, 4, 1},
    {5, 22, 1}, {8, 17, 1}, {9, 23, 1}, {9, 24, 1}, {9, 25, 1}, {10, 5, 1}};
// 38
#define HOLIDAYS_PER_BLOCK_BLK_16 0
holiday_struct_type g_holiday_blk_16[HOLIDAYS_PER_BLOCK_BLK_16] = {};
// 39
#define HOLIDAYS_PER_BLOCK_BLK_17 0
holiday_struct_type g_holiday_blk_17[HOLIDAYS_PER_BLOCK_BLK_17] = {};
// 40
#define HOLIDAYS_PER_BLOCK_BLK_18 0
holiday_struct_type g_holiday_blk_18[HOLIDAYS_PER_BLOCK_BLK_18] = {};
// 41
#define HOLIDAYS_PER_BLOCK_BLK_19 0
holiday_struct_type g_holiday_blk_19[HOLIDAYS_PER_BLOCK_BLK_19] = {};
// 42
#define HOLIDAYS_PER_BLOCK_BLK_20 0
holiday_struct_type g_holiday_blk_20[HOLIDAYS_PER_BLOCK_BLK_20] = {};

holiday_struct_type* dsm_hday_blk_pointer(uint8_t blk_num)
{
    holiday_struct_type* p_holiday_blk = NULL;

    switch (blk_num)
    {
    /* 정기 휴일 : 20개 */
    case 0:
        p_holiday_blk = &g_holiday_blk_0[0];
        break;

    /* 비정기 휴일 : 20개 x 20년 = 400개 */
    case 1:
        p_holiday_blk = &g_holiday_blk_1[0];
        break;
    case 2:
        p_holiday_blk = &g_holiday_blk_2[0];
        break;
    case 3:
        p_holiday_blk = &g_holiday_blk_3[0];
        break;
    case 4:
        p_holiday_blk = &g_holiday_blk_4[0];
        break;
    case 5:
        p_holiday_blk = &g_holiday_blk_5[0];
        break;
    case 6:
        p_holiday_blk = &g_holiday_blk_6[0];
        break;
    case 7:
        p_holiday_blk = &g_holiday_blk_7[0];
        break;
    case 8:
        p_holiday_blk = &g_holiday_blk_8[0];
        break;
    case 9:
        p_holiday_blk = &g_holiday_blk_9[0];
        break;
    case 10:
        p_holiday_blk = &g_holiday_blk_10[0];
        break;
    case 11:
        p_holiday_blk = &g_holiday_blk_11[0];
        break;
    case 12:
        p_holiday_blk = &g_holiday_blk_12[0];
        break;
    case 13:
        p_holiday_blk = &g_holiday_blk_13[0];
        break;
    case 14:
        p_holiday_blk = &g_holiday_blk_14[0];
        break;
    case 15:
        p_holiday_blk = &g_holiday_blk_15[0];
        break;
    case 16:
        p_holiday_blk = &g_holiday_blk_16[0];
        break;
    case 17:
        p_holiday_blk = &g_holiday_blk_17[0];
        break;
    case 18:
        p_holiday_blk = &g_holiday_blk_18[0];
        break;
    case 19:
        p_holiday_blk = &g_holiday_blk_19[0];
        break;
    case 20:
        p_holiday_blk = &g_holiday_blk_20[0];
        break;
    }

    return p_holiday_blk;
}

uint16_t dsm_hday_blk_size(uint8_t blk_num)
{
    uint16_t blk_size = HOLIDAYS_PER_BLOCK_BLK_0;

    switch (blk_num)
    {
    case 0:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_0;
        break;
    case 1:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_1;
        break;
    case 2:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_2;
        break;
    case 3:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_3;
        break;
    case 4:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_4;
        break;
    case 5:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_5;
        break;
    case 6:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_6;
        break;
    case 7:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_7;
        break;
    case 8:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_8;
        break;
    case 9:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_9;
        break;
    case 10:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_10;
        break;
    case 11:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_11;
        break;
    case 12:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_12;
        break;
    case 13:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_13;
        break;
    case 14:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_14;
        break;
    case 15:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_15;
        break;
    case 16:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_16;
        break;
    case 17:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_17;
        break;
    case 18:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_18;
        break;
    case 19:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_19;
        break;
    case 20:
        blk_size = HOLIDAYS_PER_BLOCK_BLK_20;
        break;
    }

    return blk_size;
}

void dsm_touimage_default(U8 rate_2)
{
    uint8_t cnt, cnt_2, dp_cnt;
    season_date_type season_info;
    week_date_type week_info;
    dayid_table_type dayid_info;
    holiday_date_type hol_date_block;
    holiday_struct_type* p_holiday_blk = NULL;

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    /****************/
    /* season profile */
    /****************/
    season_info.cnt = 4;

    DPRINTF(DBG_TRACE, "%s: SEASON cnt[%d]\r\n", __func__, season_info.cnt);
    for (cnt = 0; cnt < season_info.cnt; cnt++)
    {
        memcpy(&season_info.season[cnt], &g_default_season[cnt],
               sizeof(season_struct_type));
        DPRINTF(DBG_TRACE, "SP name[%d], month[%d], date[%d], week_id[%d]\r\n",
                cnt, season_info.season[cnt].month,
                season_info.season[cnt].date, season_info.season[cnt].week_id);
    }

    if (nv_write(I_SEASON_PROFILE_DL, (uint8_t*)&season_info))
    {
        pdl_set_bits |= SETBITS_TOU_SEASON;
    }

    /****************/
    /* week profile */
    /****************/
    week_info.cnt = 4;
    DPRINTF(DBG_TRACE, "%s: WEEK cnt[%d]\r\n", __func__, week_info.cnt);
    for (cnt = 0; cnt < week_info.cnt; cnt++)
    {
        if (rate_2 == TWO_RATE_KIND)
        {
            week_info.week[cnt].week_id =
                g_default_tariff_2_type_week[cnt].week_id;
            memcpy(week_info.week[cnt].day_id,
                   g_default_tariff_2_type_week[cnt].day_id, WEEK_LEN);
        }
        else
        {
            week_info.week[cnt].week_id = g_default_week[cnt].week_id;
            memcpy(week_info.week[cnt].day_id, g_default_week[cnt].day_id,
                   WEEK_LEN);
        }
        DPRINTF(DBG_TRACE, "week_id[%d]\r\n", week_info.week[cnt].week_id);
        DPRINT_HEX(DBG_TRACE, "day_id", &week_info.week[cnt].day_id[0],
                   WEEK_LEN, DUMP_ALWAYS);
    }

    if (nv_write(I_WEEK_PROFILE_DL, (uint8_t*)&week_info))
    {
        pdl_set_bits |= SETBITS_TOU_WEEK;
    }

    /****************/
    /* day profile */
    /****************/
    dp_cnt = 5;
    DPRINTF(DBG_TRACE, "%s: DAY cnt[%d]\r\n", __func__, dp_cnt);
    // data_id 0
    dayid_info.day_id = 0;
    dayid_info.tou_conf_cnt = 12;
    DPRINTF(DBG_TRACE, "day_id[%d], %d\r\n", dayid_info.day_id,
            dayid_info.tou_conf_cnt);
    for (cnt = 0; cnt < dayid_info.tou_conf_cnt; cnt++)
    {
        memcpy(&dayid_info.tou_conf[cnt], &g_default_day_0[cnt],
               sizeof(tou_struct_type));
        DPRINTF(DBG_TRACE, "\t\t hour[%d], min[%d], rate[%d]\r\n",
                dayid_info.tou_conf[cnt].hour, dayid_info.tou_conf[cnt].min,
                dayid_info.tou_conf[cnt].rate);
    }
    nv_sub_info.ch[0] = dayid_info.day_id;
    nv_write(I_DAY_PROFILE_DL, (uint8_t*)&dayid_info);

    // data_id 1
    dayid_info.day_id = 1;
    dayid_info.tou_conf_cnt = 12;
    DPRINTF(DBG_TRACE, "day_id[%d], %d\r\n", dayid_info.day_id,
            dayid_info.tou_conf_cnt);
    for (cnt = 0; cnt < dayid_info.tou_conf_cnt; cnt++)
    {
        memcpy(&dayid_info.tou_conf[cnt], &g_default_day_1[cnt],
               sizeof(tou_struct_type));
        DPRINTF(DBG_TRACE, "\t\t hour[%d], min[%d], rate[%d]\r\n",
                dayid_info.tou_conf[cnt].hour, dayid_info.tou_conf[cnt].min,
                dayid_info.tou_conf[cnt].rate);
    }
    nv_sub_info.ch[0] = dayid_info.day_id;
    nv_write(I_DAY_PROFILE_DL, (uint8_t*)&dayid_info);

    // data_id 2
    dayid_info.day_id = 2;
    dayid_info.tou_conf_cnt = 12;
    DPRINTF(DBG_TRACE, "day_id[%d], %d\r\n", dayid_info.day_id,
            dayid_info.tou_conf_cnt);
    for (cnt = 0; cnt < dayid_info.tou_conf_cnt; cnt++)
    {
        memcpy(&dayid_info.tou_conf[cnt], &g_default_day_2[cnt],
               sizeof(tou_struct_type));
        DPRINTF(DBG_TRACE, "\t\t hour[%d], min[%d], rate[%d]\r\n",
                dayid_info.tou_conf[cnt].hour, dayid_info.tou_conf[cnt].min,
                dayid_info.tou_conf[cnt].rate);
    }
    nv_sub_info.ch[0] = dayid_info.day_id;
    nv_write(I_DAY_PROFILE_DL, (uint8_t*)&dayid_info);

    // data_id 3
    dayid_info.day_id = 3;
    dayid_info.tou_conf_cnt = 12;
    DPRINTF(DBG_TRACE, "day_id[%d], %d\r\n", dayid_info.day_id,
            dayid_info.tou_conf_cnt);
    for (cnt = 0; cnt < dayid_info.tou_conf_cnt; cnt++)
    {
        memcpy(&dayid_info.tou_conf[cnt], &g_default_day_3[cnt],
               sizeof(tou_struct_type));
        DPRINTF(DBG_TRACE, "\t\t hour[%d], min[%d], rate[%d]\r\n",
                dayid_info.tou_conf[cnt].hour, dayid_info.tou_conf[cnt].min,
                dayid_info.tou_conf[cnt].rate);
    }
    nv_sub_info.ch[0] = dayid_info.day_id;
    nv_write(I_DAY_PROFILE_DL, (uint8_t*)&dayid_info);

    for (cnt = 4; cnt < DAY_PROF_SIZE; cnt++)
    {
        dayid_info.day_id = cnt;
        dayid_info.tou_conf_cnt = 0;

        DPRINTF(DBG_INFO, "day_id[%d], %d\r\n", dayid_info.day_id,
                dayid_info.tou_conf_cnt);

        for (cnt_2 = 0; cnt_2 < MAX_TOU_DIV_DLMS; cnt_2++)
        {
            dayid_info.tou_conf[cnt_2].hour = 0xff;
            dayid_info.tou_conf[cnt_2].min = 0xff;
            dayid_info.tou_conf[cnt_2].rate = 1;

            DPRINTF(DBG_INFO, "\t\t hour[%d], min[%d], rate[%d]\r\n",
                    dayid_info.tou_conf[cnt_2].hour,
                    dayid_info.tou_conf[cnt_2].min,
                    dayid_info.tou_conf[cnt_2].rate);
        }
        nv_sub_info.ch[0] = dayid_info.day_id;
        nv_write(I_DAY_PROFILE_DL, (uint8_t*)&dayid_info);
    }

    pdl_set_bits |= SETBITS_TOU_DAY;

    DPRINTF(DBG_WARN, _D "%s: DL[%d]\r\n", __func__, get_prog_dl_idx());

    /* 정기, 비정기 휴일 */

    /****************/
    /* special days */
    /****************/
    /* 납품시 한전에서 정보 전달: 한전 규격에 명시되어 잇음. */
    for (cnt_2 = 0; cnt_2 < 21; cnt_2++)
    {
        /* block[0 ~ 20] : block[0] 정기 휴일, block[1 ~ 20] 비정기 휴일 */

        p_holiday_blk = dsm_hday_blk_pointer(cnt_2);  // holidays arrays
        nv_sub_info.ch[0] = cnt_2;

        memset((uint8_t*)&hol_date_block, 0xff, sizeof(holiday_date_type));

        if (cnt_2 == 0)
        {
            hol_date_block.yr = YEAR_OFFSET_BLK_0;  // 정기 휴일
        }
        else
        {
            hol_date_block.yr = YEAR_OFFSET_BLK_1 + (cnt_2 - 1);  // 비정기 휴일
        }

        hol_date_block.arr_len = dsm_hday_blk_size(cnt_2);

        DPRINTF(DBG_TRACE, "BLK_IDX[%d], YEAR[%d], NUM[%d]\r\n",
                nv_sub_info.ch[0], hol_date_block.yr, hol_date_block.arr_len);

        for (cnt = 0; cnt < hol_date_block.arr_len; cnt++)
        {
            memcpy((uint8_t*)&hol_date_block.holiday[cnt], &p_holiday_blk[cnt],
                   sizeof(holiday_struct_type));

            DPRINTF(DBG_TRACE, "\t\tDAY_ID[%d], MONTH[%02d],DATE[%02d]\r\n",
                    hol_date_block.holiday[cnt].day_id,
                    hol_date_block.holiday[cnt].month,
                    hol_date_block.holiday[cnt].date);
        }
        nv_write(I_HOLIDAYS_DL, (uint8_t*)&hol_date_block);
        nv_write(I_HOL_DATE_BLOCK, (uint8_t*)&hol_date_block);
    }

    pdl_set_bits |= SETBITS_HOLIDAYS;

    DPRINTF(DBG_WARN, _D "TOU Default\r\n");

    aprog_area_rcnt = aprog_area;
    // program eeprom area
    aprog_area = get_prog_dl_idx();
    ahol_area = get_hol_dl_idx();

    DPRINTF(DBG_WARN, _D "%s: aprog_area[%d], ahol_area[%d]\r\n", __func__,
            aprog_area, ahol_area);
    currprog_available_bits = prog_setbits_to_availbits(pdl_set_bits, 0);
    prog_cur_add();
    program_default_init();
}

static uint32_t crc32_make_bin_update(uint32_t crc, const uint8_t* p,
                                      uint32_t len)
{
    static uint32_t table[256];
    static uint8_t init_done = 0;
    uint32_t i;

    if (!init_done)
    {
        uint32_t n;
        for (n = 0; n < 256U; n++)
        {
            uint32_t c = n;
            for (i = 0; i < 8U; i++)
            {
                if (c & 1U)
                    c = (c >> 1) ^ 0xEDB88320U;
                else
                    c >>= 1;
            }
            table[n] = c;
        }
        init_done = 1;
    }

    for (i = 0; i < len; i++)
    {
        crc = (crc >> 8) ^ table[(crc ^ p[i]) & 0xFFU];
    }

    return crc;
}

static bool check_sys_fw_crc(uint32_t size)
{
    enum
    {
        CRC_CHUNK_SIZE = 4096U
    };
    bool ok = false;
    uint8_t* buf = NULL;
    uint8_t tail[8];
    uint32_t base_addr;
    uint32_t payload_len;
    uint32_t crc;
    uint32_t offset = 0;
    uint32_t expected = 0;

    if (size < 4U)
        return false;

    base_addr = dsm_imgtrfr_get_start_dl_addr();
    if ((base_addr & 0x3U) != 0U)
        return false;

    payload_len = size - 4U;

    buf = (uint8_t*)pvPortMalloc(CRC_CHUNK_SIZE);
    if (buf == NULL)
        goto cleanup;

    crc = 0xFFFFFFFFU;

    while ((offset + 4U) <= payload_len)
    {
        uint32_t remaining = payload_len - offset;
        uint32_t read_len = remaining;

        if (read_len > CRC_CHUNK_SIZE)
            read_len = CRC_CHUNK_SIZE;

        read_len &= ~0x3U;
        if (read_len == 0U)
            break;

        OTA_ReadNewFW_S(base_addr + offset, buf, read_len);
        crc = crc32_make_bin_update(crc, buf, read_len);
        offset += read_len;

        kick_watchdog_S();
    }

    if (offset < payload_len)
    {
        uint32_t tail_len = payload_len - offset; /* 1..3 */
        OTA_ReadNewFW_S(base_addr + offset, buf, 4U);
        crc = crc32_make_bin_update(crc, buf, tail_len);
    }

    crc ^= 0xFFFFFFFFU;

    {
        uint32_t crc_addr = base_addr + payload_len;
        uint32_t aligned = crc_addr & ~0x3U;
        uint32_t prefix = crc_addr - aligned;

        OTA_ReadNewFW_S(aligned, tail, 8U);

        expected = ((uint32_t)tail[prefix + 0U] << 24) |
                   ((uint32_t)tail[prefix + 1U] << 16) |
                   ((uint32_t)tail[prefix + 2U] << 8) |
                   ((uint32_t)tail[prefix + 3U] << 0);
    }

    DPRINTF(DBG_WARN, _D "SYS FW CRC: calc[0x%08lX] stored[0x%08lX]\r\n", crc,
            expected);
    ok = (crc == expected);

cleanup:
    if (buf != NULL)
        vPortFree(buf);
    return ok;
}
