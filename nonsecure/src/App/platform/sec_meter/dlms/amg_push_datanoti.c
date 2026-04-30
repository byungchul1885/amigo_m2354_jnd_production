/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_task.h"
#include "amg_push_datanoti.h"
#include "meter.h"
#include "appl.h"
#include "nv.h"
#include "dl.h"
#include "lp.h"
#include "get_req.h"
#include "amg_sec.h"
#include "amg_meter_main.h"
#include "amg_media_mnt.h"

/*
******************************************************************************
*   Definition
******************************************************************************
*/
#define _D "[DATA-NOTI] "

/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/
extern float lpavg_volt[4];
extern float lpavg_volt_ltol[4];

ST_PUSH_ACTI_ERRCODE gst_push_acti;
ST_PUSH_SCRIPT_TABLE gst_push_script_table;
ST_PUSH_SETUP_TABLE gst_push_setup_table;
uint8_t g_can_push_flag = 0;

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/

#define PUSH_SCRIPTS_LIST_SIZE (2 + 3 * 26)
static const uint8_t PUSH_SCRIPTS_list[PUSH_SCRIPTS_LIST_SIZE] = {
    0x01,
    0x03,

    0x02,
    0x02,
    0x12,
    0x00,
    PUSH_SCRIPT_ID_ERR_CODE,  // script id
    0x02,
    0x05,
    0x12,
    0x00,
    SCRIPT_TABLE_SVC_ID_EXE_ATT,  // sevice id
    0x12,
    0x00,
    CLS_PushSetUp,  // class id
    0x09,
    0x06,
    OBIS_PUSH_SETUP_ERR_CODE_nobr,  // logical name
    0x0f,
    0x02,  // index
    0x12,
    0x00,
    0x00,  // parameter

    0x02,
    0x02,
    0x12,
    0x00,
    PUSH_SCRIPT_ID_LAST_LP,  // script id
    0x02,
    0x05,
    0x12,
    0x00,
    SCRIPT_TABLE_SVC_ID_EXE_ATT,  // sevice id
    0x12,
    0x00,
    CLS_PushSetUp,  // class id
    0x09,
    0x06,
    OBIS_PUSH_SETUP_LAST_LP_nobr,  // logical name
    0x0f,
    0x02,  // index
    0x12,
    0x00,
    0x00,  // parameter

    0x02,
    0x02,
    0x12,
    0x00,
    PUSH_SCRIPT_ID_LAST_RT_LP,
    0x02,
    0x05,
    0x12,
    0x00,
    SCRIPT_TABLE_SVC_ID_EXE_ATT,
    0x12,
    0x00,
    CLS_PushSetUp,
    0x09,
    0x06,
    OBIS_PUSH_SETUP_LAST_RT_LP_nobr,
    0x0f,
    0x02,
    0x12,
    0x00,
    0x00,
};

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

float get_lpavg_ltol(U8 line);
void magnet_err_mon(void);
void tcover_open_mon(void);

void error_code_event_clear(void);

void dsm_push_err_code_acti_default(ST_PUSH_ACTI_ERRCODE* pst_push_acti)
{
    /* 자기진단 초기값 : 통신 규격 "3.4.2.5.2.2 자기진단 항목 PUSH 활성화
     * 항목구성" 참조 */

#if 0  // JP.KIM 24.10.25 //STT V3.0
	// 자기진단 1 PUSH 활성화 (초기값 : 0b10000000)
	gst_push_acti.code[ERR_CODE_1_IDX] = (PUSHACTI_CODE1_PWR_OFF);
#else  // JP.KIM 24.10.25 //STT V3.2
    // 자기진단 1 PUSH 활성화 (초기값 : 0b10000110)
    gst_push_acti.code[ERR_CODE_1_IDX] =
        (PUSHACTI_CODE1_PWR_OFF | PUSHACTI_CODE1_COVER_OPEN |
         PUSHACTI_CODE1_WORK_BLACKOUT);
#endif

    // 자기진단 2 PUSH 활성화 (초기값 : 0b11110000)
    gst_push_acti.code[ERR_CODE_2_IDX] =
        (PUSHACTI_CODE2_OVERCURR_C | PUSHACTI_CODE2_OVERCURR_B |
         PUSHACTI_CODE2_OVERCURR_A | PUSHACTI_CODE2_WRONGCONN);

    // 자기진단 3 PUSH 활성화 (초기값 : 0b00001111)
    gst_push_acti.code[ERR_CODE_3_IDX] =
        (PUSHACTI_CODE3_NEUT_WRONGCONN | PUSHACTI_CODE3_NO_PHASE_C |
         PUSHACTI_CODE3_NO_PHASE_B | PUSHACTI_CODE3_NO_PHASE_A);

    // 자기진단 4 PUSH 활성화 (초기값 : 0b00000000)
    gst_push_acti.code[ERR_CODE_4_IDX] = 0;

    memcpy((uint8_t*)pst_push_acti, (uint8_t*)&gst_push_acti,
           sizeof(ST_PUSH_ACTI_ERRCODE));

    nv_write(I_PUSH_ACTI_ERR, (uint8_t*)pst_push_acti);
}

#if 1  // jp.kim 24.10.22
void dsm_push_err_code_set(void)
{
    ST_PUSH_ACTI_ERRCODE st_push_acti;
    dsm_push_err_code_acti_default(&st_push_acti);
    memcpy((uint8_t*)&gst_push_acti, (uint8_t*)&st_push_acti,
           sizeof(ST_PUSH_ACTI_ERRCODE));
}
#endif

void dsm_push_err_code_init(void)
{
    ST_PUSH_ACTI_ERRCODE st_push_acti;

    memset((uint8_t*)&gst_push_acti, 0x00, sizeof(ST_PUSH_ACTI_ERRCODE));

    if (!nv_read(I_PUSH_ACTI_ERR, (uint8_t*)&st_push_acti))
    {
        dsm_push_err_code_acti_default(&st_push_acti);
    }
    memcpy((uint8_t*)&gst_push_acti, (uint8_t*)&st_push_acti,
           sizeof(ST_PUSH_ACTI_ERRCODE));
}

uint32_t dsm_push_err_code_nvread(ST_PUSH_ACTI_ERRCODE* pst_push_acti)
{
    if (!nv_read(I_PUSH_ACTI_ERR, (uint8_t*)pst_push_acti))
    {
        dsm_push_err_code_acti_default(pst_push_acti);
    }
    memcpy((uint8_t*)&gst_push_acti, (uint8_t*)pst_push_acti,
           sizeof(ST_PUSH_ACTI_ERRCODE));

    return TRUE;
}

uint32_t dsm_push_err_code_nvwrite(ST_PUSH_ACTI_ERRCODE* pst_push_acti)
{
    if (!nv_write(I_PUSH_ACTI_ERR, (uint8_t*)pst_push_acti))
    {
        dsm_push_err_code_acti_default(pst_push_acti);
    }
    memcpy((uint8_t*)&gst_push_acti, (uint8_t*)pst_push_acti,
           sizeof(ST_PUSH_ACTI_ERRCODE));

    return TRUE;
}

uint32_t dsm_push_err_code_is_acti(uint8_t err_code_idx, uint8_t err_code)
{
    uint8_t err_code_acti = gst_push_acti.code[err_code_idx];

    DPRINTF(DBG_TRACE, "err_code_acti[%d] = 0x%02X, err_code = 0x%02X\r\n",
            err_code_idx, gst_push_acti.code[err_code_idx], err_code);

    if (err_code_acti & err_code)
        return TRUE;
    else
        return FALSE;
}

void dsm_push_script_register(uint8_t id, uint16_t obj_idx)
{
    if (gst_push_script_table.cnt < PUSH_SCRIPT_MAX_NUM)
    {
        gst_push_script_table.script[gst_push_script_table.cnt].identifier = id;
        gst_push_script_table.script[gst_push_script_table.cnt].obj_idx =
            obj_idx;
        DPRINTF(DBG_TRACE, "%s: script[%d] id[%d], obj_idx[%d]\r\n", __func__,
                gst_push_script_table.cnt, id, obj_idx);
        gst_push_script_table.cnt++;
    }
    else
    {
        DPRINTF(DBG_ERR, "%s: overflow!!!\r\n", __func__);
    }
}

void dsm_push_push_script_table_default(void)
{
    memset((uint8_t*)&gst_push_script_table, 0x00,
           sizeof(ST_PUSH_SCRIPT_TABLE));
    dsm_push_script_register(PUSH_SCRIPT_ID_ERR_CODE, OBJ_PUSH_SETUP_ERR_CODE);
    dsm_push_script_register(PUSH_SCRIPT_ID_LAST_LP, OBJ_PUSH_SETUP_LAST_LP);
    dsm_push_script_register(PUSH_SCRIPT_ID_LAST_RT_LP,
                             OBJ_PUSH_SETUP_LAST_RT_LP);
}

void dsm_push_script_table_init(void) { dsm_push_push_script_table_default(); }

uint32_t dsm_push_script_nvread(ST_PUSH_SCRIPT_TABLE* pst_push_script)
{
    if (!nv_read(I_PUSH_SCRIPT_TABLE, (uint8_t*)pst_push_script))
    {
        dsm_push_push_script_table_default();
    }
    else
        memcpy((uint8_t*)&gst_push_script_table, (uint8_t*)pst_push_script,
               sizeof(ST_PUSH_SCRIPT_TABLE));

    return TRUE;
}

uint32_t dsm_push_script_nvwrite(ST_PUSH_SCRIPT_TABLE* pst_push_script)
{
    if (!nv_write(I_PUSH_SCRIPT_TABLE, (uint8_t*)pst_push_script))
    {
        dsm_push_push_script_table_default();
    }
    else
        memcpy((uint8_t*)&gst_push_script_table, (uint8_t*)pst_push_script,
               sizeof(ST_PUSH_SCRIPT_TABLE));

    return TRUE;
}

uint32_t dsm_push_get_encoded_script_table(uint8_t* len, uint8_t* data)
{
    uint8_t cnt = 0;
    cnt = PUSH_SCRIPTS_LIST_SIZE;
    memcpy(data, PUSH_SCRIPTS_list, cnt);
    *len = cnt;

    return TRUE;
}

/*
    default push setup registration
*/
void dsm_push_setup_register(uint8_t id, uint8_t enable,
                             uint16_t random_start_intval,
                             uint16_t repetition_delay)
{
    uint8_t* pdata = NULL;

    if (gst_push_setup_table.cnt < PUSH_SETUP_MAX_NUM)
    {
        gst_push_setup_table.setup[gst_push_setup_table.cnt].identifier = id;
        gst_push_setup_table.setup[gst_push_setup_table.cnt]
            .random_start_intval = random_start_intval;
        gst_push_setup_table.setup[gst_push_setup_table.cnt].repetition_delay =
            repetition_delay;
        gst_push_setup_table.setup[gst_push_setup_table.cnt].window_cnt = 1;
        pdata = &gst_push_setup_table.setup[gst_push_setup_table.cnt]
                     .window[0]
                     .st_dt.year;
        memset(pdata, 0xff, sizeof(date_time_type));
        pdata = &gst_push_setup_table.setup[gst_push_setup_table.cnt]
                     .window[0]
                     .sp_dt.year;
        memset(pdata, 0xff, sizeof(date_time_type));
        DPRINTF(DBG_TRACE,
                "%s: setup[%d] id[%d], enable[%d], random_st_intval[%d], "
                "repeti_delay[%d]\r\n",
                __func__, gst_push_setup_table.cnt, id, enable,
                random_start_intval, repetition_delay);
        gst_push_setup_table.cnt++;
    }
    else
    {
        DPRINTF(DBG_ERR, "%s: overflow!!!\r\n", __func__);
    }
}

void dsm_push_push_setup_table_default(void)
{
    memset((uint8_t*)&gst_push_setup_table, 0x00, sizeof(ST_PUSH_SETUP_TABLE));
    dsm_push_setup_register(PUSH_SCRIPT_ID_ERR_CODE, PUSH_SETUP_DISABLE,
                            PUSH_SETUP_DEFAULT_RANDOM_ST_INTVAL_ERR_CODE, 0);
    dsm_push_setup_register(PUSH_SCRIPT_ID_LAST_LP, PUSH_SETUP_DISABLE,
                            PUSH_SETUP_DEFAULT_RANDOM_ST_INTVAL_LAST_LP, 0);
    dsm_push_setup_register(PUSH_SCRIPT_ID_LAST_RT_LP, PUSH_SETUP_DISABLE,
                            PUSH_SETUP_DEFAULT_RANDOM_ST_INTVAL_LAST_RT_LP, 0);
}

void dsm_push_setup_table_init(void) { dsm_push_push_setup_table_default(); }

ST_PUSH_SETUP_TABLE* dsm_push_setup_get_setup_table(void)
{
    return &gst_push_setup_table;
}

uint32_t dsm_push_setup_nvread(ST_PUSH_SETUP_TABLE* pst_push_setup)
{
    if (!nv_read(I_PUSH_SETUP_TABLE, (uint8_t*)pst_push_setup))
    {
        dsm_push_push_setup_table_default();
    }
    else
        memcpy((uint8_t*)&gst_push_setup_table, (uint8_t*)pst_push_setup,
               sizeof(ST_PUSH_SETUP_TABLE));

    return TRUE;
}

uint32_t dsm_push_setup_nvwrite(ST_PUSH_SETUP_TABLE* pst_push_setup)
{
    if (!nv_write(I_PUSH_SETUP_TABLE, (uint8_t*)pst_push_setup))
    {
        dsm_push_push_setup_table_default();
    }
    else
        memcpy((uint8_t*)&gst_push_setup_table, (uint8_t*)pst_push_setup,
               sizeof(ST_PUSH_SETUP_TABLE));

    return TRUE;
}

uint32_t dsm_push_setup_is_id(uint8_t script_id, uint8_t* pidx)
{
    if (gst_push_setup_table.cnt != 0)
    {
        uint8_t cnt;

        for (cnt = 0; cnt < gst_push_setup_table.cnt; cnt++)
        {
            if (gst_push_setup_table.setup[cnt].identifier == script_id)
            {
                *pidx = cnt;
                return TRUE;
            }
        }
    }

    return FALSE;
}

ST_PUSH_SETUP* dsm_push_setup_get_info(uint8_t id)
{
    if (gst_push_setup_table.cnt != 0)
    {
        return &gst_push_setup_table.setup[id];
    }

    return NULL;
}

uint32_t dsm_push_is_enable(uint8_t script_id)
{
    uint8_t push_idx;
    ST_PUSH_SETUP* p_setup = NULL;
    date_time_type dt_sentinel;
    uint8_t cnt;

    memset(&dt_sentinel, 0xff, sizeof(date_time_type));

    if (dsm_push_setup_is_id(script_id, &push_idx))
    {
        p_setup = dsm_push_setup_get_info(push_idx);

        if (p_setup != NULL)
        {
            /*****************************************************/
            /* window 에 대한 정확한 구현 필요                   */
            /*****************************************************/
            if (p_setup->window_cnt == 0)
            {
                return TRUE;
            }
            if (p_setup->window_cnt >= PUSH_WINDOW_MAX_NUM)
                p_setup->window_cnt = PUSH_WINDOW_MAX_NUM;

            for (cnt = 0; cnt < p_setup->window_cnt; cnt++)
            {
                if (!memcmp(&dt_sentinel, &p_setup->window[cnt].st_dt,
                            sizeof(date_time_type)) &&
                    !memcmp(&dt_sentinel, &p_setup->window[cnt].sp_dt,
                            sizeof(date_time_type)))
                {
                    return FALSE;
                }
                return TRUE;
            }
        }
    }
    return FALSE;
}

static void push_entry_force_disable(uint8_t script_id)
{
    uint8_t push_idx = 0;
    ST_PUSH_SETUP_TABLE* pst_push_setup = dsm_push_setup_get_setup_table();

    if (dsm_push_setup_is_id(script_id, &push_idx))
    {
        pst_push_setup->setup[push_idx].window_cnt = 1;
        memset(&pst_push_setup->setup[push_idx].window[0].st_dt, 0xff,
               sizeof(date_time_type));
        memset(&pst_push_setup->setup[push_idx].window[0].sp_dt, 0xff,
               sizeof(date_time_type));
        DPRINTF(DBG_TRACE, "%s: script_id[%d], push_idx[%d]\r\n", __func__,
                script_id, push_idx);
    }
}

void dsm_push_disable(void)
{
    push_entry_force_disable(PUSH_SCRIPT_ID_ERR_CODE);
    push_entry_force_disable(PUSH_SCRIPT_ID_LAST_LP);
    push_entry_force_disable(PUSH_SCRIPT_ID_LAST_RT_LP);
}

void dsm_can_set_push_flag(uint32_t on_off) { g_can_push_flag = on_off; }

uint32_t dsm_can_get_push_flag(void) { return g_can_push_flag; }

void dsm_data_push_init(void)
{
    dsm_push_err_code_init();
    dsm_push_script_table_init();
    dsm_push_setup_table_init();
}

uint8_t dsm_covert_grp_e_2_errcodeidx(uint8_t grp_e)
{
    switch (grp_e)
    {
    case 5:
        return ERR_CODE_1_IDX;
    case 6:
        return ERR_CODE_2_IDX;
    case 7:
        return ERR_CODE_3_IDX;
    case 8:
        return ERR_CODE_4_IDX;
    default:
        break;
    }
    return ERR_CODE_MAX_IDX;
}

#define PUSH_SEC_BUF_SIZE MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO
uint8_t push_sec_buf[PUSH_SEC_BUF_SIZE];
uint32_t push_data_noti_send(uint32_t EN_AT, uint8_t* pbuff, uint16_t len)
{
    uint16_t tx_len;
    uint8_t idx = 0;

    if (EN_AT)
    {
        if ((uint32_t)len + 30U > (uint32_t)PUSH_SEC_BUF_SIZE)
        {
            DPRINTF(DBG_ERR, "PUSH plain too big: len=%d, max=%d\r\n", len,
                    PUSH_SEC_BUF_SIZE);
            return FALSE;
        }

        dl_fill_LLC_header(&push_sec_buf[idx]);  // rebuild LLC
        len -= LLC_HEADER_LEN;

        dsm_sec_g_glo_ciphering_xDLMS_APDU_build(&push_sec_buf[LLC_HEADER_LEN],
                                                 &tx_len,
                                                 &pbuff[LLC_HEADER_LEN], len);
        tx_len += LLC_HEADER_LEN;

        if (tx_len > dl_get_tx_max_info())
        {
            DPRINTF(DBG_ERR, "PUSH tx_len=%d > dl_tx_max_info=%d\r\n", tx_len,
                    dl_get_tx_max_info());
            return FALSE;
        }

        dsm_can_set_push_flag(TRUE);
        dl_send_appl_push_datanoti_msg(push_sec_buf, tx_len);
    }
    else
    {
        dsm_can_set_push_flag(TRUE);
        dl_send_appl_push_datanoti_msg(pbuff, len);
    }

    return TRUE;
}

/*
Data-Notification ::= SEQUENCE
{
    long-invoke-id-and-priority         Long-Invoke-Id-And-Priority,
    date-time                           OCTET STRING,
    notification-body                    Notification-Body
}
General-Glo-Ciphering ::= SEQUENCE
{
   system-title                       OCTET STRING,
   ciphered-content                  OCTET STRING
}
The Invoke-Id parameter of the GET, SET and ACTION services allows the client
and the server to pair requests and responses. The range of the Invoke_Id is
0…15. In some cases this is not sufficient. To support those cases, the ACCESS
service uses a Long_Invoke_Id parameter. The range of the Long_Invoke_Id is 0…16
777 215. NOTE Description of the circumstances when long Invoke_id-s are useful
is beyond the Scope of this Technical Report.

*/

static const uint8_t long_invokeID_priority[4] = {0xc0, 0x00, 0x00, 0x01};
void appl_push_msg_errcode(void)
{
    uint32_t EN_AT_FLAG = TRUE;
    uint8_t blocked = 0;

    pPdu_idx = 0;
    // LLC header
    dl_fill_LLC_header(&pPdu[pPdu_idx]);
    pPdu_idx += LLC_HEADER_LEN;

    // application header
    pPdu[pPdu_idx++] = TAG_DATA_NOTI;
    memcpy(&pPdu[pPdu_idx], long_invokeID_priority, 4);
    pPdu_idx += 4;
    // pPdu[pPdu_idx++] = 0x00;                 //date-time
    comm_dt = cur_rtc;
    fill_clock_obj_except_tag(&comm_dt);

    // err_code build
    FILL_STRUCT(2);
    memcpy(&pPdu[pPdu_idx], PUSH_SETUP_ERRCODE_capture_objects,
           PUSH_SETUP_ERRCODE_CAPOBJ_SIZE);
    pPdu_idx += PUSH_SETUP_ERRCODE_CAPOBJ_SIZE;

    FILL_ARRAY(1);
    if (mt_is_onephase())
    {
        float fval;

        FILL_STRUCT(0x05);

        ob_err_code_1();
        ob_err_code_2();
        ob_err_code_3();
        ob_err_code_4();

        fval = lpavg_volt[0];
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

        fval = get_lpavg_ltol(0);
        FILL_FLOAT(fval);
        fval = get_lpavg_ltol(1);
        FILL_FLOAT(fval);
        fval = get_lpavg_ltol(2);
        FILL_FLOAT(fval);
    }

    DPRINT_HEX(DBG_TRACE, "ERRCODE_PUSH", &pPdu[0], pPdu_idx, DUMP_ALWAYS);

    if (!dsm_push_is_enable(PUSH_SCRIPT_ID_ERR_CODE))
    {
        DPRINTF(DBG_ERR, "ERRCODE PUSH disabled\r\n");
        blocked = 1;
    }

#if defined(FEATURE_JP_485_PUSH_PROTECT)
    if ((dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_RS485) ||
        (dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_NONE))
    {
        DPRINTF(DBG_ERR, "ERRCODE PUSH blocked: media=%s\r\n",
                dsm_media_if_fsm_string(dsm_media_get_fsm_if_hdlc()));
        blocked = 1;
    }
#endif

    if (appl_get_conn_state() != APPL_ENC_SIGN_STATE)
    {
        DPRINTF(DBG_ERR, "ERRCODE PUSH blocked: conn=%d\r\n",
                appl_get_conn_state());
        blocked = 1;
    }

    if (blocked)
        return;

    if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
    {
        EN_AT_FLAG = TRUE;
    }

    if (push_data_noti_send(EN_AT_FLAG, pPdu, pPdu_idx))
        DPRINTF(DBG_ERR, "ERRCODE PUSH sent\r\n");
    else
        DPRINTF(DBG_ERR, "ERRCODE PUSH drop\r\n");

    error_code_event_clear();
}

void appl_push_msg_lastLP(void)
{
    uint32_t EN_AT_FLAG = TRUE;
    uint8_t blocked = 0;

    pPdu_idx = 0;
    // LLC header
    dl_fill_LLC_header(&pPdu[pPdu_idx]);
    pPdu_idx += LLC_HEADER_LEN;

    // application header
    pPdu[pPdu_idx++] = TAG_DATA_NOTI;
    memcpy(&pPdu[pPdu_idx], long_invokeID_priority, 4);
    pPdu_idx += 4;

    // pPdu[pPdu_idx++] = 0x00;
    comm_dt = cur_rtc;
    fill_clock_obj_except_tag(&comm_dt);

    FILL_STRUCT(2);
    memcpy(&pPdu[pPdu_idx], PUSH_SETUP_lastLP_capture_objects,
           PUSH_SETUP_LAST_LP_CAPOBJ_SIZE);
    pPdu_idx += PUSH_SETUP_LAST_LP_CAPOBJ_SIZE;

    FILL_ARRAY(1);
    fill_last_lp_record();

    DPRINT_HEX(DBG_TRACE, "LP_PUSH", &pPdu[0], pPdu_idx, DUMP_ALWAYS);

    if (!dsm_push_is_enable(PUSH_SCRIPT_ID_LAST_LP))
    {
        DPRINTF(DBG_TRACE, "LastLP PUSH disabled\r\n");
        blocked = 1;
    }

#if defined(FEATURE_JP_485_PUSH_PROTECT)
    if ((dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_RS485) ||
        (dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_NONE))
    {
        DPRINTF(DBG_TRACE, "LastLP PUSH blocked: media=%s\r\n",
                dsm_media_if_fsm_string(dsm_media_get_fsm_if_hdlc()));
        blocked = 1;
    }
#endif

    if (appl_get_conn_state() != APPL_ENC_SIGN_STATE)
    {
        DPRINTF(DBG_TRACE, "LastLP PUSH blocked: conn=%d\r\n",
                appl_get_conn_state());
        blocked = 1;
    }

    if (blocked)
        return;

    if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
    {
        EN_AT_FLAG = TRUE;
    }

    if (push_data_noti_send(EN_AT_FLAG, pPdu, pPdu_idx))
        DPRINTF(DBG_TRACE, "LastLP PUSH sent\r\n");
    else
        DPRINTF(DBG_TRACE, "LastLP PUSH drop\r\n");
}

extern void LPrt_energy_to_pPdu(uint8_t* recbuff);

void appl_push_msg_lastRtLP(void)
{
    uint32_t EN_AT_FLAG = TRUE;
    uint8_t num_records;
    uint8_t i;
    uint8_t capobj_copy[PUSH_SETUP_LAST_RT_LP_CAPOBJ_SIZE];
    uint8_t blocked = 0;

    num_records = (rt_lp_interval == 1) ? 5 : 1;
    if (num_records > lprt__index)
        num_records = (uint8_t)lprt__index;
    if (num_records == 0)
        return;

    pPdu_idx = 0;
    dl_fill_LLC_header(&pPdu[pPdu_idx]);
    pPdu_idx += LLC_HEADER_LEN;

    pPdu[pPdu_idx++] = TAG_DATA_NOTI;
    memcpy(&pPdu[pPdu_idx], long_invokeID_priority, 4);
    pPdu_idx += 4;

    comm_dt = cur_rtc;
    fill_clock_obj_except_tag(&comm_dt);

    FILL_STRUCT(2);

    memcpy(capobj_copy, PUSH_SETUP_lastRtLP_capture_objects,
           PUSH_SETUP_LAST_RT_LP_CAPOBJ_SIZE);
    if (rt_lp_interval == 1)
    {
        capobj_copy[PUSH_SETUP_LAST_RT_LP_CAPOBJ_SIZE - 2] = 0x10;
        capobj_copy[PUSH_SETUP_LAST_RT_LP_CAPOBJ_SIZE - 1] = 0x05;
    }
    memcpy(&pPdu[pPdu_idx], capobj_copy, PUSH_SETUP_LAST_RT_LP_CAPOBJ_SIZE);
    pPdu_idx += PUSH_SETUP_LAST_RT_LP_CAPOBJ_SIZE;

    FILL_ARRAY(num_records);

    for (i = 0; i < num_records; i++)
    {
        date_time_type rdt;

        lprt_record_read(appl_tbuff, lprt__index - 1 - i, 1);
        FILL_STRUCT(2);

        if (mt_is_onephase())
        {
            lprt_record_1phs* rec = (lprt_record_1phs*)appl_tbuff;
            expand_time(&rdt, &rec->dt[0]);
        }
        else
        {
            lprt_record_3phs* rec = (lprt_record_3phs*)appl_tbuff;
            expand_time(&rdt, &rec->dt[0]);
        }

        pPdu[pPdu_idx++] = OCTSTRING_TAG;
        fill_clock_obj_except_tag(&rdt);

        if (mt_is_onephase())
        {
            FILL_STRUCT(8);
        }
        else
        {
            FILL_STRUCT(28);
        }
        LPrt_energy_to_pPdu(appl_tbuff);
    }

    DPRINT_HEX(DBG_TRACE, "RT_LP_PUSH", &pPdu[0], pPdu_idx, DUMP_ALWAYS);

    if (!dsm_push_is_enable(PUSH_SCRIPT_ID_LAST_RT_LP))
    {
        DPRINTF(DBG_TRACE, "RT_LP PUSH disabled\r\n");
        blocked = 1;
    }

#if defined(FEATURE_JP_485_PUSH_PROTECT)
    if ((dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_RS485) ||
        (dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_NONE))
    {
        DPRINTF(DBG_TRACE, "RT_LP PUSH blocked: media=%s\r\n",
                dsm_media_if_fsm_string(dsm_media_get_fsm_if_hdlc()));
        blocked = 1;
    }
#endif

    if (appl_get_conn_state() != APPL_ENC_SIGN_STATE)
    {
        DPRINTF(DBG_TRACE, "RT_LP PUSH blocked: conn=%d\r\n",
                appl_get_conn_state());
        blocked = 1;
    }

    if (blocked)
        return;

    if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
    {
        EN_AT_FLAG = TRUE;
    }

    if (push_data_noti_send(EN_AT_FLAG, pPdu, pPdu_idx))
        DPRINTF(DBG_TRACE, "RT_LP PUSH sent (%d records)\r\n", num_records);
    else
        DPRINTF(DBG_TRACE, "RT_LP PUSH drop (%d records)\r\n", num_records);
}

void dsm_push_data_noti_proc(uint32_t type)
{
    uint8_t push_idx = 0x00, dec;
    ST_PUSH_SETUP* p_push_setup = NULL;
    uint32_t transfer_delay_ms;

    magnet_err_mon();
    tcover_open_mon();
    if (dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_RS485)
    {
        DPRINTF(DBG_ERR, "%s: Can't push: IF(485)\r\n", __func__);
        return;
    }

    if (appl_get_conn_state() == APPL_ENC_SIGN_STATE)
    {
        if (PUSH_SCRIPT_ID_ERR_CODE == type)
        {
            MSG07("dsm_push_data_noti_proc_1");
            /*
                자기 진단 이벤트 발생시 push acti 학인 절차 필요.. 확인 후 push
                차후 자기 진단 이벤트 처리 완료시 적용..
            */
            if (appl_get_conn_state() == APPL_ENC_SIGN_STATE)
            {
                MSG07("dsm_push_data_noti_proc_2");
                if (dsm_push_is_enable(PUSH_SCRIPT_ID_ERR_CODE))
                {
                    MSG07("dsm_push_data_noti_proc_3");
                    appl_push_msg_errcode();
                    DPRINTF(DBG_ERR, "%s -> appl_push_msg_errcode\r\n",
                            __func__);
                }
            }
        }
        else if (PUSH_SCRIPT_ID_LAST_RT_LP == type)
        {
            if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_LAST_RT_LP, &push_idx))
            {
                p_push_setup = dsm_push_setup_get_info(push_idx);
                if (p_push_setup != NULL)
                {
                    dec = get_server_hdlc_addr_to_dec();
                    transfer_delay_ms =
                        ((uint32_t)p_push_setup->random_start_intval * dec) *
                        10;
                    dsm_meter_sw_timer_start(MT_SW_TIMER_PUSH_RT_LP_TO, FALSE,
                                             transfer_delay_ms);

                    DPRINTF(DBG_TRACE,
                            "%s: Push RT LP delay[%d sec, %d ms]\r\n",
                            __func__, p_push_setup->random_start_intval,
                            transfer_delay_ms);
                }
            }
        }
        else if (PUSH_SCRIPT_ID_LAST_LP == type)
        {
            if (appl_get_conn_state() == APPL_ENC_SIGN_STATE)
            {
                if (dsm_push_is_enable(PUSH_SCRIPT_ID_LAST_LP))
                {
                    if (dsm_push_setup_is_id(PUSH_SCRIPT_ID_LAST_LP,
                                             &push_idx))
                    {
                        p_push_setup = dsm_push_setup_get_info(push_idx);
                        if (p_push_setup != NULL)
                        {
                            dec = get_server_hdlc_addr_to_dec();
                            /*
                                동시 PUSH에 대한 충돌을 회피하기 위해 지정하는
                               시간으로 random 알고리즘 에서 얻을 수 있는
                               최대값을 정의함 (기본값은 10초)이며 원격에서
                               설정 가능함, 단위는 초 단위임 예시) 10초
                               설정에 계기 ID가 XXXXXXXXX99이면 지연시간은
                               10*99/100 = 9.9초 이후 PUSH 전송 단, 자기진단의
                               경우 감지 후 즉시 전송한다.
                            */

                            transfer_delay_ms =
                                ((uint32_t)p_push_setup->random_start_intval *
                                 dec) *
                                10;  // milli-sec 라서 x10 함.
                            dsm_meter_sw_timer_start(MT_SW_TIMER_PUSH_LP_TO,
                                                     FALSE, transfer_delay_ms);

                            DPRINTF(DBG_TRACE,
                                    "%s: Push LP delay[%d sec, %d ms]\r\n",
                                    __func__,
                                    p_push_setup->random_start_intval,
                                    transfer_delay_ms);
                        }
                        else
                        {
                            DPRINTF(DBG_ERR, "%s: LastLP get_info NULL\r\n",
                                    __func__);
                        }
                    }
                    else
                    {
                        DPRINTF(DBG_ERR,
                                "%s: LastLP script_id not found\r\n",
                                __func__);
                    }
                }
            }
        }
    }
    else
    {
        DPRINTF(DBG_ERR, "%s: Can't push: not ASSO 3\r\n", __func__);
    }
}
