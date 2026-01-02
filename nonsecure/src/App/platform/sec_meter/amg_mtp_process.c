/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "options_sel.h"
#include "main.h"
#include "amg_task.h"
#include "os_task_q.h"
#include "amg_uart.h"
#include "amg_mif_prtl.h"
#include "amg_mtp_process.h"
#include "amg_meter_main.h"
#include "eeprom_at24cm02.h"
#include "whm.h"
#include "nv.h"
#include "cal.h"
#include "meter.h"
#include "mx25r4035f_def.h"
#include "mx25r4035f.h"
#include "amg_pwr.h"
#include "amg_spi.h"
#include "amg_imagetransfer.h"
#include "amg_shell_product.h"
#include "softimer.h"
#ifdef M2354_CAN /* bccho, 2023-11-28 */
#include "amg_isotp_user.h"
#endif
/*
1. FEATURE_MTP_CALSET_OFF enable 인 경우
    1) MTP_OP_CAL  <- MTP_OP_NORMAL(cal key push)
    ---------------------------------------
    |     fsm            |   evt           |
    ---------------------------------------
    |MTP_FSM_CAL_ST      |  ACK           ->
    |MTP_FSM_CAL_GET     |  cal_ok        ->
    |MTP_FSM_PARM_SET    |  ACK           ->
    |MTP_FSM_SAG_SWELL   |  ACK           ->
    |MTP_FSM_MESURE_READY|

    2) MTP_OP_NORMAL <- boot 시
    |MTP_FSM_MESURE_READY|

2. FEATURE_MTP_CALSET_OFF disble 인 경우
    1) MTP_OP_CAL  <- boot 시 cal_restore fail, MTP_OP_NORMAL(cal key push)
    ---------------------------------------
    |     fsm            |   evt           |
    ---------------------------------------
    |MTP_FSM_CAL_ST      |  ACK           ->
    |MTP_FSM_CAL_GET     |  cal_ok        ->
    |MTP_FSM_CAL_SET     |  ACK           ->
    |MTP_FSM_PARM_SET    |  ACK           ->
    |MTP_FSM_SAG_SWELL   |  ACK           ->
    |MTP_FSM_MESURE_READY|

    2) MTP_OP_NORMAL <- boot 시 cal_restore OK
    |MTP_FSM_CAL_GET     |  cdata_cmp fail ->
    |MTP_FSM_CAL_SET     |  ACK            ->
    |MTP_FSM_MESURE_READY|
*/

/*
******************************************************************************
*   Definition
******************************************************************************
*/
#define _D "[MTP] "

/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/

#define METER_FW_UP_ING_CNT_ADD 6  // 8  //6초 편차추가계산

#define CAL_NULL_DISP 0
#define CAL_BEGIN_DISP 1
#define CAL_SUCCESS_DISP 2

#define CAL_END_CHECK_TIMES 5
#define MTP_CHECK_TIMES 10

#define MIF_MTP_RETRY_MAX_TO_COUNT 10

uint32_t g_flash_posi;
uint32_t g_fw_length;
uint32_t g_fw_file_end;
uint32_t g_fw_file_start;
uint8_t g_mtp_curr_running_bank = MTP_RUNNING_NOINFO_BANK;

float VOLT_FWUP_BACK[3] = {0};
float CURR_FWUP_BACK[3] = {0};
float PH_FWUP_BACK[3] = {0};
float PH_LTOL_FWUP_BACK[3] = {0};

float PULSE_ADD_MODIFY_DATA = 0.0;
float PULSE_ADD_MODIFY_DATA_VA = 0.0;
float PULSE_ADD_MODIFY_DATA_VAR = 0.0;

bool METER_FW_UP_ING_STS = 0;
bool METER_FW_UP_END_PULSE_MODIFY = 0;
U16 METER_FW_UP_ING_CNT = 0;
U8 PULSE_DIR_MODIFY_BACK = true;
extern bool init_mif_task_firm_up;

int8_t no_inst_curr_chk_zon_cnt =
    10;  // Count of accumulation intervals until data becomes valid

extern bool cal_data_get_success;
extern bool cal_monitor_only;
extern bool dr_dsp, r_sun_dsp, dsp_comm_ing;

// cal 관련
extern bool dsp_cal_st_ing;    // cal 시작 표시, push 에서 설정
extern bool dsp_cal_mode_ing;  // cal 종료, 전류 표시, push 에서 설정
extern bool dsp_cal_mode_end;  // cal get 성공, mtp rx 에서 설정

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/

typedef struct _mtp_fw_wr_
{
    uint32_t record_len;
    uint32_t pre_posi;
    uint8_t clr_flag;
    uint8_t complete_flag;
} ST_MTP_FW_WR;

ST_MTP_FW_WR g_fw;

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/
EN_MTP_OP_MODE g_mtp_op_mode;
EN_MTP_FSM g_mtp_fsm;
ST_MTP_CON g_st_mtp_con;
ST_MTP_CAL_DATA g_mtp_caldata;
ST_MTP_PUSH_DATA g_mtp_pushdata;

ST_MTP_CAL_DATA monitor_mtp_caldata;
ST_MIF_METER_PARM monitor_mtp_meter_parm;
ST_MIF_SAGSWELL_SETUP monitor_mtp_sagswell;

ST_MIF_CAL_START g_mtp_cal_start;
ST_MIF_METER_PARM g_mtp_meter_parm;
ST_MIF_SAGSWELL_SETUP g_mtp_sagswell;
ST_MIF_SAGSWELL_STOP g_mtp_sagswell_stop;

ST_MIF_ZBM_FW_DATA g_mtp_zbm_fw;
TYPE_MTP_FW_IMAGE g_mtp_fw_mode;
uint8_t g_mtp_fw_index;

bool PwOn_1st_parm_set, meter_firmup_delay_ing = 0;

extern void dsm_mtp_fsm_send(void);

void dsp_cal_st_is_ing_set(void);
void dsp_cal_mode_is_ing_set(void);
void dsp_cal_end_state(void);
void dsm_mtp_meter_fw_download_finish(void);
void dsm_mif_setreq_fw_data(uint8_t* pdata, uint16_t length);
void dsm_mif_getreq_fw_data(uint8_t* pdata, uint16_t length);
void afe_init_start(void);
void dsm_mif_getreq_sagswell_data(void);
#ifdef MTP_ZCD_ON_OFF
void dsm_mif_actreq_zcdpulse(void);
void dsm_mif_actreq_zcdoff(void);
#endif
/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

char* dsm_mtp_fw_err_string(int32_t mode)
{
    switch (mode)
    {
    case 0:
        return "No error.";
    case 1:
        return "Command not supported by this version of API.";
    case 2:
        return "The primary image pointer is invalid.";
    case 3:
        return "No image meets requirements.";
    case 4:
        return "The ZBM's table's code pointer is after the end of flash.";
    case 5:
        return "The ZBM's table's code pointer is not zero.";
    case 6:
        return "The image's table's code pointer points after the table.";
    case 7:
        return "The image's table's code size is too small.";
    case 8:
        return "The image's table says code ends after the end of flash.";
    case 9:
        return "Jump at image start is not into image, or no jump found.";
    case 10:
        return "Image hardware ID does not match ZBM's hardware ID.";
    case 11:
        return "Found null pointer to image's verification data.";
    case 12:
        return "Image's checksum does not match table.";
    case 13:
        return "Image's CRC-32 does not match table.";
    case 14:
        return "Image's SHA256 does not match table.";
    case 15:
        return "Image's table's type is invalid.";
    case 16:
        return "Image's table's code pointer is NULL.";
    case 17:
        return "CRC Hardware timed-out in use.";
    default:
        return "Unknown Error";
    }
};

char* dsm_mtp_fw_type_string(uint32_t mode)
{
    switch (mode)
    {
    case MTP_FW_TYPE0:
        return "MTP_FW_TYPE0";
    case MTP_FW_TYPE1:
        return "MTP_FW_TYPE1";
    case MTP_FW_TYPE2:
        return "MTP_FW_TYPE2";
    case MTP_FW_TYPE_START:
        return "MTP_FW_TYPE_START";

    default:
        return "MTP_FW_TYPE_Unknown";
    }
}

char* dsm_mtp_op_mode_string(uint32_t mode)
{
    switch (mode)
    {
    case MTP_OP_NORMAL:
        return "MTP_OP_NORMAL";
    case MTP_OP_CAL:
        return "MTP_OP_CAL";

    default:
        return "MTP_OP_Unknown";
    }
}

char* dsm_mtp_fsm_string(uint32_t fsm)
{
    switch (fsm)
    {
    case MTP_FSM_NONE:
        return "NONE";
    case MTP_FSM_CAL_ST:
        return "CAL_ST";
    case MTP_FSM_CAL_GET:
        return "CAL_GET";
    case MTP_FSM_CAL_SET:
        return "CAL_SET";
    case MTP_FSM_PARM_SET:
        return "PARM_SET";
    case MTP_FSM_PARM_GET:
        return "PARM_GET";  // jp.kim 25.03.13
    case MTP_FSM_SAG_SWELL:
        return "SAG_SWELL_SET";
    case MTP_FSM_SAG_SWELL_GET:
        return "SAG_SWELL_GET";
    case MTP_FSM_MESURE_READY:
        return "MESURE_READY";
    case MTP_FSM_FW_START_SET:
        return "MT_FW_START";
    case MTP_FSM_FW_END_SET:
        return "MT_FW_END";
    case MTP_FSM_FW_SET:
        return "MT_FW_SET";
    case MTP_FSM_FW_GET:
        return "MT_FW_GET";
    default:
        return "Unknown";
    }
}

ST_MIF_CAL_DATA* dsm_mtp_get_cal_data(void)
{
    return (ST_MIF_CAL_DATA*)(&g_mtp_caldata);
}

ST_MTP_PUSH_DATA* dsm_mtp_get_push_data(void) { return &g_mtp_pushdata; }

ST_MIF_CAL_START* dsm_mtp_get_cal_start(void) { return &g_mtp_cal_start; }

ST_MIF_METER_PARM* dsm_mtp_get_meter_parm(void) { return &g_mtp_meter_parm; }

ST_MIF_SAGSWELL_SETUP* dsm_mtp_get_sagswell(void) { return &g_mtp_sagswell; }

ST_MIF_ZBM_FW_DATA* dsm_mtp_get_zbm_fw_data(void)  // sjan 20200911 for zbm
{
    return &g_mtp_zbm_fw;
}

uint8_t dsm_mtp_rx_cal_data(uint8_t length, uint8_t* p_body)
{
    cal_data_type cal;
    uint8_t op_mode = dsm_mtp_get_op_mode();
    ST_MIF_CAL_DATA_ARR* p_caldata = (ST_MIF_CAL_DATA_ARR*)p_body;
    ST_MTP_CAL_DATA st_mtp_caldata;
    ST_MTP_CAL_DATA flash_g_mtp_caldata;

    st_mtp_caldata.r_current_gain =
        dsm_htonl(p_caldata->cal_data[0].r_current_gain);
    st_mtp_caldata.r_voltage_gain =
        dsm_htonl(p_caldata->cal_data[0].r_voltage_gain);
    st_mtp_caldata.r_phase_gain =
        dsm_htonl(p_caldata->cal_data[0].r_phase_gain);
    st_mtp_caldata.s_current_gain =
        dsm_htonl(p_caldata->cal_data[0].s_current_gain);
    st_mtp_caldata.s_voltage_gain =
        dsm_htonl(p_caldata->cal_data[0].s_voltage_gain);
    st_mtp_caldata.s_phase_gain =
        dsm_htonl(p_caldata->cal_data[0].s_phase_gain);
    st_mtp_caldata.t_current_gain =
        dsm_htonl(p_caldata->cal_data[0].t_current_gain);
    st_mtp_caldata.t_voltage_gain =
        dsm_htonl(p_caldata->cal_data[0].t_voltage_gain);
    st_mtp_caldata.t_phase_gain =
        dsm_htonl(p_caldata->cal_data[0].t_phase_gain);

    st_mtp_caldata.ok = p_caldata->cal_data[0].cal_ok;

    DPRINTF(DBG_WARN, "CAL Result[%d]\r\n", p_caldata->cal_data[0].cal_ok);

    MSG09("CAL Result[%d]", p_caldata->cal_data[0].cal_ok);

    if (cal_monitor_only)
    {
        memcpy(&monitor_mtp_caldata, &st_mtp_caldata, sizeof(ST_MTP_CAL_DATA));
    }
    else
    {
        memcpy(&g_mtp_caldata, &st_mtp_caldata, sizeof(ST_MTP_CAL_DATA));

        if (op_mode == MTP_OP_NORMAL)
        {
            if (st_mtp_caldata.ok)
            {
                if (memcmp(&st_mtp_caldata, &g_mtp_caldata,
                           sizeof(ST_MTP_CAL_DATA)))
                {
                    g_mtp_caldata.ok = 0;
                }
                else
                {
                    g_mtp_caldata.ok = 1;
                    memcpy(&g_mtp_caldata, &st_mtp_caldata,
                           sizeof(ST_MTP_CAL_DATA));
                    if (p_caldata->cal_data[0].cal_ok == 1)
                    {
                        cal.T_cal_i0 = g_mtp_caldata.r_current_gain;
                        cal.T_cal_v0 = g_mtp_caldata.r_voltage_gain;
                        cal.T_cal_p0 = g_mtp_caldata.r_phase_gain;
                        cal.T_cal_i1 = g_mtp_caldata.s_current_gain;
                        cal.T_cal_v1 = g_mtp_caldata.s_voltage_gain;
                        cal.T_cal_p1 = g_mtp_caldata.s_phase_gain;
                        cal.T_cal_i2 = g_mtp_caldata.t_current_gain;
                        cal.T_cal_v2 = g_mtp_caldata.t_voltage_gain;
                        cal.T_cal_p2 = g_mtp_caldata.t_phase_gain;

                        nv_write(I_CAL_DATA, (U8*)&cal);

                        // LCD 전류 표시 종료
                        dsp_cal_mode_ing = false;

                        if (!dsp_cal_mode_end)
                        {
                            MSG09("rx_cal_data, ok, end timer set");

                            // LCD cal end 표시 시작
                            dsp_cal_mode_end = true;
                            dsp_cal_end_is_ing_timeset(T30SEC);
                        }
                    }
                }
            }
            else
            {
                g_mtp_caldata.ok = 0;
            }
        }
        else
        {
        }
    }

    return TRUE;
}

uint8_t dsm_mtp_rx_meterParam_data(uint8_t length, uint8_t* p_body)
{
    uint8_t* ptr = p_body;
    float fval;
    ST_MIF_METER_PARM p_prm;

    DPRINT_HEX(DBG_INFO, "Rx Meter Param:", p_body, length, DUMP_ALWAYS);

    memcpy(&p_prm.cut_voltage_thr[0], ptr, 4);
    ptr += 4;
    ToHFloat((U8_Float*)&fval, &p_prm.cut_voltage_thr[0]);
    DPRINTF(DBG_TRACE, _D "cut_voltage: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));

    memcpy(&p_prm.start_current_thr[0], ptr, 4);
    ptr += 4;
    ToHFloat((U8_Float*)&fval, &p_prm.start_current_thr[0]);
    DPRINTF(DBG_TRACE, _D "start_current_thr: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));

    p_prm.direct_reverse = *ptr++;
    p_prm.reactive_select = *ptr++;
    p_prm.meter_method = *ptr++;
    p_prm.pulse_select = *ptr++;

    DPRINT_HEX(DBG_TRACE, "meter_param:", p_body, sizeof(ST_MIF_METER_PARM),
               DUMP_ALWAYS);
    DPRINTF(DBG_TRACE, _D "direct_reverse : [%d]\r\n", p_prm.direct_reverse);
    DPRINTF(DBG_TRACE, _D "reactive_select: [%d]\r\n", p_prm.reactive_select);
    DPRINTF(DBG_TRACE, _D "meter_method   : [%d]\r\n", p_prm.meter_method);
    DPRINTF(DBG_TRACE, _D "pulse_select   : [%d]\r\n", p_prm.pulse_select);

#if 1  // jp.kim 25.03.03
       // direct_reverse_monitor = p_prm.direct_reverse;
    monitor_mtp_meter_parm.direct_reverse = p_prm.direct_reverse;
    monitor_mtp_meter_parm.reactive_select = p_prm.reactive_select;
    monitor_mtp_meter_parm.meter_method = p_prm.meter_method;
    monitor_mtp_meter_parm.pulse_select = p_prm.pulse_select;
#endif

    return TRUE;
}

uint8_t dsm_mtp_rx_sagswell_data(uint8_t length, uint8_t* p_body)
{
    uint8_t* p_strim;
    ST_MIF_SAGSWELL_SETUP l_sagswell;
    float fval;
    uint16_t val;
    p_strim = p_body;

    DPRINTF(DBG_INFO, _D "SagSwell_GetResponse\r\n");
    DPRINT_HEX(DBG_INFO, "SagSwell:", p_body, length, DUMP_ALWAYS);

    memcpy(&l_sagswell.pf_level[0], p_strim, 4);
    ToHFloat((U8_Float*)&fval, &l_sagswell.pf_level[0]);
    p_strim += 4;

    DPRINTF(DBG_TRACE, _D "pf_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));

    ToH16((U8_16_32*)&val, p_strim);
    l_sagswell.pf_continue_time = dsm_htons(val);
    DPRINTF(DBG_INFO, "pf_continue_time:[0x%04X]\r\n",
            l_sagswell.pf_continue_time);
    p_strim += 2;

    memcpy(&l_sagswell.sag_level[0], p_strim, 4);
    ToHFloat((U8_Float*)&fval, &l_sagswell.sag_level[0]);
    DPRINTF(DBG_TRACE, _D "sag_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    p_strim += 4;

    l_sagswell.sag_time = *p_strim;
    p_strim += 1;
    DPRINTF(DBG_TRACE, _D "sag_time: 0x%02X\r\n", l_sagswell.sag_time);

    memcpy(&l_sagswell.swell_level[0], p_strim, 4);
    ToHFloat((U8_Float*)&fval, &l_sagswell.swell_level[0]);
    DPRINTF(DBG_TRACE, _D "swell_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    p_strim += 4;

    l_sagswell.swell_time = *p_strim;
    p_strim += 1;
    DPRINTF(DBG_TRACE, _D "swell_time: 0x%02X\r\n", l_sagswell.swell_time);

#if 1  // jp.kim 25.03.03
    memcpy(&monitor_mtp_sagswell.sag_level[0], &l_sagswell.sag_level[0], 4);
    memcpy(&monitor_mtp_sagswell.swell_level[0], &l_sagswell.swell_level[0], 4);
    monitor_mtp_sagswell.sag_time = l_sagswell.sag_time;
    monitor_mtp_sagswell.swell_time = l_sagswell.swell_time;
#endif

    return TRUE;
}

uint8_t dsm_mtp_meter_fwinfo_update(uint8_t length, uint8_t* p_body)
{
    ST_FW_INFO fwinfo;

    DPRINT_HEX(DBG_INFO, "Rx Meter firm ver:", p_body, length, DUMP_ALWAYS);

    dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_METER);

    if (memcmp(p_body, (uint8_t*)&fwinfo.mt_type[0], IMAGE_FW_NAME_MAX_SIZE))
    {
        memcpy((uint8_t*)&fwinfo.mt_type[0], p_body, IMAGE_FW_NAME_MAX_SIZE);
        dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_METER);
    }
    return TRUE;
}

bool meter_fwup_delay_sag_protect(void)
{
    if (meter_firmup_delay_ing)
    {
        return 1;
    }
    return 0;
}

bool meter_fwup_pulse_protect(void)
{
    if (METER_FW_UP_ING_STS || METER_FW_UP_END_PULSE_MODIFY)
    {
        return 1;
    }
    return 0;
}

bool meter_fwup_ing(void)
{
    if (METER_FW_UP_ING_STS || METER_FW_UP_END_PULSE_MODIFY ||
        init_mif_task_firm_up || meter_firmup_delay_ing)
    {
        return 1;
    }
    return 0;
}

void push_data_user_back_up_data_for_meter_fwup(void)
{
    if (meter_fwup_ing())
    {
        g_mtp_pushdata.r_voltage = VOLT_FWUP_BACK[0];
        g_mtp_pushdata.s_voltage = VOLT_FWUP_BACK[1];
        g_mtp_pushdata.t_voltage = VOLT_FWUP_BACK[2];

        g_mtp_pushdata.r_vol_thd = 0.0;
        g_mtp_pushdata.s_vol_thd = 0.0;
        g_mtp_pushdata.t_vol_thd = 0.0;

        g_mtp_pushdata.r_current = CURR_FWUP_BACK[0];
        g_mtp_pushdata.s_current = CURR_FWUP_BACK[1];
        g_mtp_pushdata.t_current = CURR_FWUP_BACK[2];

        g_mtp_pushdata.r_phase = PH_FWUP_BACK[0];
        g_mtp_pushdata.s_phase = PH_FWUP_BACK[1];
        g_mtp_pushdata.t_phase = PH_FWUP_BACK[2];

        g_mtp_pushdata.rs_phase = PH_LTOL_FWUP_BACK[0];
        g_mtp_pushdata.rt_phase = PH_LTOL_FWUP_BACK[1];
    }
}

uint8_t dsm_mtp_rx_push_measure_data(uint8_t length, uint8_t* p_body)
{
    uint8_t cnt;

    float* pfloat = &g_mtp_pushdata.r_voltage;
    float* fval;  // g_mtp_pushdata
    uint32_t ch_count = dsm_get_dm_out_measure_print_chkcount();

#if 0 /* bccho, 2024-06-24, delete */
    /* bccho, 2024-06-17, 김종필대표 패치 적용 */
    if (starting_cnt)
    {
        // 전원투입 초기에 전압 = 0.0V
        r_voltage_back[0] = 0.0;
        r_voltage_back[1] = 0.0;
        r_voltage_back[2] = 0.0;
        s_voltage_back[0] = 0.0;
        s_voltage_back[1] = 0.0;
        s_voltage_back[2] = 0.0;
        t_voltage_back[0] = 0.0;
        t_voltage_back[1] = 0.0;
        t_voltage_back[2] = 0.0;
    }
    else
    {
        // 순시전압 평균을 위한 벡업 2회
        r_voltage_back[2] = r_voltage_back[1];
        s_voltage_back[2] = s_voltage_back[1];
        t_voltage_back[2] = t_voltage_back[1];
        r_voltage_back[1] = r_voltage_back[0];
        s_voltage_back[1] = s_voltage_back[0];
        t_voltage_back[1] = t_voltage_back[0];
    }
#endif

    for (cnt = 0; cnt < 47; cnt++)  // 47 is float count of measured data
    {
        fval = (pfloat + cnt);
        ToHFloat((U8_Float*)fval, (p_body + cnt * 4));

#if 0 /* bccho, 2024-06-24, delete */
        /* bccho, 2024-06-17, 김종필대표 패치 적용 */
        if (starting_cnt)
        {
            g_mtp_pushdata.r_voltage = 0.0;
            g_mtp_pushdata.s_voltage = 0.0;
            g_mtp_pushdata.t_voltage = 0.0;
        }
        else
        {
            // 순시전압은 3회연속 평균값 적용 , 3상 고려
            if (cnt == 0)
            {
                r_voltage_back[0] = g_mtp_pushdata.r_voltage;
                float_buff = (g_mtp_pushdata.r_voltage + r_voltage_back[1] +
                              r_voltage_back[2]) /
                             3.0;
                g_mtp_pushdata.r_voltage = float_buff;
            }
            if (cnt == (1 * 6))
            {
                s_voltage_back[0] = g_mtp_pushdata.s_voltage;
                float_buff = (g_mtp_pushdata.s_voltage + s_voltage_back[1] +
                              s_voltage_back[2]) /
                             3.0;
                g_mtp_pushdata.s_voltage = float_buff;
            }
            if (cnt == (2 * 6))
            {
                t_voltage_back[0] = g_mtp_pushdata.t_voltage;
                float_buff = (g_mtp_pushdata.t_voltage + t_voltage_back[1] +
                              t_voltage_back[2]) /
                             3.0;
                g_mtp_pushdata.t_voltage = float_buff;
            }
        }
#endif

        if (cnt == 0)
        {
            if (ch_count % 10 == 1)
            {
                MSG07("Voltage: %d.%03d", (uint32_t)(*fval),
                      (uint32_t)((*fval - (uint32_t)(*fval)) * 1000));
            }
        }
        if (cnt == 1)
        {
            if (ch_count % 10 == 1)
            {
                MSG07("Current: %d.%03d", (uint32_t)(*fval),
                      (uint32_t)((*fval - (uint32_t)(*fval)) * 1000));
            }
        }
    }

    g_mtp_pushdata.sag_count = *(p_body + cnt * 4);
    g_mtp_pushdata.swell_count = *(p_body + cnt * 4 + 1);

    pfloat = &g_mtp_pushdata.temp;
    fval = (pfloat);
    ToHFloat((U8_Float*)fval, (p_body + cnt * 4 + 2));

    push_data_user_back_up_data_for_meter_fwup();

    g_mtp_pushdata.cal_disp_sts = *(p_body + cnt * 4 + 6);
    if (CAL_BEGIN_DISP == g_mtp_pushdata.cal_disp_sts)
    {
        dsp_cal_st_is_ing_set();
    }
    else if (CAL_SUCCESS_DISP == g_mtp_pushdata.cal_disp_sts)
    {
        dsp_cal_mode_is_ing_set();
    }

#if PHASE_NUM != SINGLE_PHASE
    //(p_body 읽기) 지우면 안됨 -> 동작안함.
    g_mtp_pushdata.phase_fail_a = *(p_body + cnt * 4 + 7);
    g_mtp_pushdata.phase_fail_b = *(p_body + cnt * 4 + 8);
    g_mtp_pushdata.phase_fail_c = *(p_body + cnt * 4 + 9);

    DPRINTF(
        DBG_INFO,
        _D
        "phase_fail_a[0x%02X], phase_fail_b[0x%02X] phase_fail_c[0x%02X] \r\n",
        g_mtp_pushdata.phase_fail_a, g_mtp_pushdata.phase_fail_b,
        g_mtp_pushdata.phase_fail_c);
#endif

    return TRUE;
}

uint8_t dsm_mtp_get_curr_running_bank(void)
{
    DPRINTF(DBG_INFO, "%s: Bank[%d]\r\n", __func__, g_mtp_curr_running_bank);
    return g_mtp_curr_running_bank;
}

bool dsm_mtp_is_valid_curr_runnning_bank(void)
{
    if (g_mtp_curr_running_bank == MTP_RUNNING_LOW_BANK ||
        g_mtp_curr_running_bank == MTP_RUNNING_HIGH_BANK)
    {
        return true;
    }
    return false;
}

void dsm_mtp_fsm_tx_proc_fw_data_get_currunningbank(void)
{
    DPRINTF(DBG_INFO, "%s\r\n", __func__);
    dsm_mtp_fsm_tx_proc_fw_data_get(0, 0);
}

void dsm_mtp_fsm_tx_proc_fw_data_act_runbank(uint8_t bank)
{
    DPRINTF(DBG_INFO, "%s: Bank[%d]\r\n", __func__, bank);

    dsm_mtp_set_fw_type(1);
    dsm_mtp_set_fw_index(bank);
    dsm_mtp_fsm_tx_proc_fw_data_set(1, bank);
}

uint8_t dsm_mtp_rx_fw_get_data(uint8_t length, uint8_t* p_body, uint8_t type,
                               uint8_t index)
{
    uint8_t* p_strim;
    uint32_t val;
    p_strim = p_body;

    DPRINTF(DBG_INFO, _D "FW Info GetResponse\r\n");
    DPRINT_HEX(DBG_INFO, "Data:", p_body, length, DUMP_ALWAYS);

    if (type == 0)
    {
        if (index == 0)
        {
            g_mtp_curr_running_bank = p_body[3];
            DPRINTF(DBG_INFO, _D "%s: Bank[%d]\r\n", __func__,
                    g_mtp_curr_running_bank);
        }

        ToH32((U8_16_32*)&val, p_strim);
        if ((int32_t)val > ZBM_ERROR_MAX)
        {
            DPRINTF(DBG_INFO, _D "ZBM Result: %ld\r\n", (int32_t)val);
        }
        else
        {
            DPRINTF(DBG_INFO, _D "ZBDI: %ld, %s\r\n", (int32_t)val,
                    dsm_mtp_fw_err_string(-val));
        }
        p_strim += 4;
    }
    else if (type == 1)
    {
        ToH32((U8_16_32*)&val, p_strim);
        if ((int32_t)val > ZBM_ERROR_MAX)
        {
            DPRINTF(DBG_INFO, _D "ZBM Result: %ld\r\n", (int32_t)val);
        }
        else
        {
            DPRINTF(DBG_INFO, _D "ZBDF: %ld, %s\r\n", (int32_t)val,
                    dsm_mtp_fw_err_string(-val));
        }
        p_strim += 4;
    }
    else if (type == 2)
    {
        DPRINTF(DBG_INFO, _D "ZBDT(%d)\r\n", index);

        ToH32((U8_16_32*)&val, p_strim);
        DPRINTF(DBG_INFO, "Table ID  : 0x%08X\r\n", val);
        p_strim += 4;

        ToH32((U8_16_32*)&val, p_strim);
        DPRINTF(DBG_INFO, "Table Type: 0x%08X\r\n", val);
        p_strim += 4;

        ToH32((U8_16_32*)&val, p_strim);
        DPRINTF(DBG_INFO, "Address   : 0x%08X\r\n", val);
        p_strim += 4;

        ToH32((U8_16_32*)&val, p_strim);
        DPRINTF(DBG_INFO, "Length    : 0x%08X\r\n", val);
        p_strim += 4;

        ToH32((U8_16_32*)&val, p_strim);
        DPRINTF(DBG_INFO, "Revision  : 0x%08X\r\n", val);
        p_strim += 4;

        ToH32((U8_16_32*)&val, p_strim);
        DPRINTF(DBG_INFO, "Hardware  : 0x%08X\r\n", val);
        p_strim += 4;

        ToH32((U8_16_32*)&val, p_strim);
        DPRINTF(DBG_INFO, "Checksum  : 0x%08X\r\n", val);
        p_strim += 4;
    }

    return TRUE;
}

uint16_t dsm_mtp_rx_fw_set_data(uint8_t length, uint8_t* p_body)
{
    uint8_t* p_strim;
    uint16_t val;
    p_strim = p_body;

    ToH16((U8_16_32*)&val, p_strim);
    p_strim += 2;

    g_st_mtp_con.TO_retry = 0;
    DPRINTF(DBG_NONE, _D "Response of fw write (%d)\r\n", val);
    return val;
}

void dsm_mtp_set_fsm(uint8_t state)
{
    if (g_mtp_fsm != state)
    {
        DPRINTF(DBG_WARN, _D "FSM[%s -> %s]\r\n", dsm_mtp_fsm_string(g_mtp_fsm),
                dsm_mtp_fsm_string(state));
        g_mtp_fsm = state;
    }
}

uint8_t dsm_mtp_get_fsm(void) { return g_mtp_fsm; }

void dsm_mtp_set_op_mode(uint8_t mode)
{
    if (g_mtp_op_mode != mode)
    {
        DPRINTF(DBG_WARN, _D "MTP_OP[%s -> %s]\r\n",
                dsm_mtp_op_mode_string(g_mtp_op_mode),
                dsm_mtp_op_mode_string(mode));
        g_mtp_op_mode = mode;
    }
}

uint8_t dsm_mtp_get_op_mode(void) { return g_mtp_op_mode; }

uint8_t dsm_mtp_get_fw_type(void) { return g_mtp_fw_mode; }

void dsm_mtp_set_fw_type(uint8_t mode)
{
    DPRINTF(DBG_WARN, _D "MTP_SET FW Type[%d -> %d]\r\n", g_mtp_fw_mode,
            mode);  // enum TYPE_MTP_FW_IMAGE
    g_mtp_fw_mode = mode;
}

uint8_t dsm_mtp_get_fw_index(void) { return g_mtp_fw_index; }

void dsm_mtp_set_fw_index(uint8_t mode)
{
    DPRINTF(DBG_WARN, _D "MTP_SET FW Index[%d]\r\n", mode);
    g_mtp_fw_index = mode;
}

/*
******************************************************************************
*   FUNCTIONS - extern
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/

void dsm_mtp_init(void)
{
    float fval;

    ST_MTP_CAL_POINT st_mtp_cal_point;
    ST_MTP_PARM st_mtp_parm;
    ST_MTP_SAGSWELL st_mtp_sagswell;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    g_mtp_op_mode = 0;
    memset(&g_mtp_fsm, 0x00, sizeof(g_mtp_fsm));
    memset(&g_st_mtp_con, 0x00, sizeof(g_st_mtp_con));
    memset(&g_mtp_caldata, 0x00, sizeof(g_mtp_caldata));
    memset(&g_mtp_pushdata, 0x00, sizeof(g_mtp_pushdata));

    memset(&g_mtp_cal_start, 0x00, sizeof(g_mtp_cal_start));
    memset(&g_mtp_meter_parm, 0x00, sizeof(g_mtp_meter_parm));
    memset(&g_mtp_sagswell, 0x00, sizeof(g_mtp_sagswell));
    memset(&g_mtp_sagswell_stop, 0x00, sizeof(g_mtp_sagswell_stop));

    if (!nv_read(I_MTP_CAL_POINT, (uint8_t*)&st_mtp_cal_point))
    {
        dsm_mtp_default_cal_point(&st_mtp_cal_point);
    }

    memcpy((uint8_t*)&g_mtp_cal_start, (uint8_t*)&st_mtp_cal_point.val,
           sizeof(g_mtp_cal_start));
    if (!nv_read(I_MTP_PARM, (uint8_t*)&st_mtp_parm))
    {
        dsm_mtp_default_parm(&st_mtp_parm);
        PwOn_1st_parm_set = TRUE;
    }

    ToHFloat((U8_Float*)&fval, (uint8_t*)&st_mtp_parm.val.cut_voltage_thr[0]);
    if (fval <= 10.0)
    {
        dsm_mtp_default_parm(&st_mtp_parm);
        PwOn_1st_parm_set = TRUE;
    }
    memcpy((uint8_t*)&g_mtp_meter_parm, (uint8_t*)&st_mtp_parm.val,
           sizeof(g_mtp_meter_parm));
    DPRINTF(DBG_TRACE, _D "cut_Vol hex: %02X:%02X:%02X:%02X\r\n",
            g_mtp_meter_parm.cut_voltage_thr[0],
            g_mtp_meter_parm.cut_voltage_thr[1],
            g_mtp_meter_parm.cut_voltage_thr[2],
            g_mtp_meter_parm.cut_voltage_thr[3]);

    if (!nv_read(I_MTP_SAG_SWELL, (uint8_t*)&st_mtp_sagswell))
    {
        dsm_mtp_default_sagswell(&st_mtp_sagswell);
        PwOn_1st_parm_set = TRUE;
    }

    ToHFloat((U8_Float*)&fval, (uint8_t*)&st_mtp_sagswell.val.pf_level[0]);
    if (fval < 30.0)
    {
        dsm_mtp_default_sagswell(&st_mtp_sagswell);
        PwOn_1st_parm_set = TRUE;
    }
    memcpy((uint8_t*)&g_mtp_sagswell, (uint8_t*)&st_mtp_sagswell.val,
           sizeof(g_mtp_sagswell));
}

void dsm_mtp_rx_process(uint8_t rsp, uint8_t length, uint8_t* p_body)
{
    U8 i = 0;
    float fval = 0;
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();

    uint16_t fw_len;
    uint8_t type = dsm_mtp_get_fw_type();
    uint8_t index = dsm_mtp_get_fw_index();

    switch (rsp)
    {
    case MIF_GetRsp_CalData:
        dsm_mtp_rx_cal_data(length, p_body);

        break;
    case MIF_SetRsp_CalData:

        break;
    case MIF_ActRsp_CalStart:

        break;

    case MIF_Push_MeterData:
        dsm_mtp_rx_push_measure_data(length, p_body);

        if (!METER_FW_UP_ING_STS)
        {
            meter_set_measure_flag(TRUE);
            meter_set_measure_xdone_timer(0);
        }

        dsm_mif_push_ack();
        break;

    case MIF_GetRsp_MeterData:

        break;
    case MIF_SetRsp_SagSwell:

        break;
    case MIF_ActRsp_SagSwellSpTime:

        break;
    case MIF_GetRsp_MtSetupDataParm:
        dsm_mtp_rx_meterParam_data(length, p_body);
        // jp.kim 25.03.13
        dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
        DPRINTF(DBG_TRACE, _D "%s: PARM_GET: RSP => MESURE_READY\r\n",
                __func__);
        break;
    case MIF_GetRsp_SagSwell_Parm:
        dsm_mtp_rx_sagswell_data(length, p_body);
        dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
        DPRINTF(DBG_TRACE,
                _D "%s: MIF_GetRsp_SagSwell_Parm: RSP => MESURE_READY\r\n",
                __func__);
        break;

    case MIF_GetRsp_FirmVer:
        dsm_mtp_meter_fwinfo_update(length, p_body);

        break;

    case MIF_SetARsp_MtSetupDataParm:

        break;
    case MIF_SetRsp_BaudRate:

        break;

    case MIF_SetAck:
        DPRINTF(DBG_INFO, _D "Set ACK\r\n");
        break;
    case MIF_SetNack:
        DPRINTF(DBG_INFO, _D "Set NACK\r\n");
        break;
    case MIF_ActAck:
        DPRINTF(DBG_INFO, _D "Act ACK\r\n");
        break;
    case MIF_ActNack:
        DPRINTF(DBG_INFO, _D "Act NACK\r\n");
        break;

    case MIF_GetRsp_FwData:
        dsm_mtp_rx_fw_get_data(length, p_body, type, index);
        break;

    case MIF_SetRsp_FwData:

        fw_len = dsm_mtp_rx_fw_set_data(length, p_body);
        if (fw_len == 0xCEDE)
        {
            DPRINTF(DBG_WARN, _D "F/W Write Start (0x%X) (%d) OK!! \r\n",
                    fw_len, fw_len);
            dsm_mtp_set_fsm(MTP_FSM_FW_START_SET);
        }
        else if (fw_len == 0xECED)
        {
            DPRINTF(DBG_WARN, _D "F/W Write End  (0x%X) (%d) OK!! \r\n", fw_len,
                    fw_len);
            DPRINTF(DBG_WARN, _D "%s: FSM FW_Download Finished\r\n", __func__);
            dsm_set_meter_fw_down_complete(1);
            if (dsm_get_meter_fw_down_complete())
            {
                DPRINTF(DBG_ERR, _D "F/W Download Complete !!!\r\n");

#if PHASE_NUM == SINGLE_PHASE
                i = 0;
                PULSE_ADD_MODIFY_DATA_VA =
                    VOLT_FWUP_BACK[i] * CURR_FWUP_BACK[i] * PulseKwh *
                    (float)METER_FW_UP_ING_CNT / (1000.0 * 3600.0);
                PULSE_ADD_MODIFY_DATA =
                    PULSE_ADD_MODIFY_DATA_VA *
                    (cos(RADIAN_PER_DEGREES * PH_FWUP_BACK[i]));
                PULSE_ADD_MODIFY_DATA_VAR =
                    PULSE_ADD_MODIFY_DATA_VA *
                    (sin(RADIAN_PER_DEGREES * PH_FWUP_BACK[i]));
#else
                PULSE_ADD_MODIFY_DATA_VA = 0.0;
                PULSE_ADD_MODIFY_DATA = 0.0;
                PULSE_ADD_MODIFY_DATA_VAR = 0.0;
                for (i = 0; i < 3; i++)
                {
                    PULSE_ADD_MODIFY_DATA_VA +=
                        VOLT_FWUP_BACK[i] * CURR_FWUP_BACK[i] * 1.0 * PulseKwh *
                        (float)METER_FW_UP_ING_CNT / (1000.0 * 3600.0);
                    PULSE_ADD_MODIFY_DATA +=
                        VOLT_FWUP_BACK[i] * CURR_FWUP_BACK[i] *
                        (cos(RADIAN_PER_DEGREES * PH_FWUP_BACK[i])) * PulseKwh *
                        (float)METER_FW_UP_ING_CNT / (1000.0 * 3600.0);
                    PULSE_ADD_MODIFY_DATA_VAR +=
                        VOLT_FWUP_BACK[i] * CURR_FWUP_BACK[i] *
                        (sin(RADIAN_PER_DEGREES * PH_FWUP_BACK[i])) * PulseKwh *
                        (float)METER_FW_UP_ING_CNT / (1000.0 * 3600.0);
                }
#endif
                METER_FW_UP_END_PULSE_MODIFY = 1;
                METER_FW_UP_ING_STS = 0;
                METER_FW_UP_ING_CNT = 0;

                log_mtr_sw_up();
            }
            dsm_mtp_meter_fw_download_finish();

            dsm_mtp_set_fsm(MTP_FSM_NONE);

            if (dsm_mtp_get_curr_running_bank() == MTP_RUNNING_LOW_BANK)
            {
                dsm_mtp_fsm_tx_proc_fw_data_act_runbank(MTP_RUNNING_HIGH_BANK);
            }
            else if (dsm_mtp_get_curr_running_bank() == MTP_RUNNING_HIGH_BANK)
            {
                dsm_mtp_fsm_tx_proc_fw_data_act_runbank(MTP_RUNNING_LOW_BANK);
            }
#if defined(FEATURE_ZBM_DEFAULT_RUNBANK_BANK_LOW)
            else
                dsm_mtp_fsm_tx_proc_fw_data_act_runbank(MTP_RUNNING_HIGH_BANK);
#endif
        }
        else
        {
            if (g_fw.record_len == fw_len)
            {
                g_fw.clr_flag = 0x0;
                DPRINTF(DBG_INFO,
                        _D
                        "F/W Write record length g_fw.record(0x%X) (0x%X) OK!! "
                        "\r\n",
                        g_fw.record_len, fw_len);
            }
            else
            {
                g_fw.record_len = fw_len;
            }
        }

        DPRINTF(DBG_INFO,
                _D "g_fw.record_len(%d)(0x%X)   fw_len(%d)(0x%X) \r\n",
                g_fw.record_len, g_fw.record_len, fw_len, fw_len);
        break;
    default:
        break;
    }

    dsm_mtp_fsm_rx_proc(rsp);
}

void dsm_mtp_default_cal_point(ST_MTP_CAL_POINT* pst_mtp_cal_point)
{
    float fval;
    ST_MIF_CAL_START st_mif_cal_start;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    cal_point_init();

    fval = Vcal;
    ToCommFloat(&st_mif_cal_start.ref_voltage[0], (U8_Float*)&fval);
    DPRINTF(DBG_TRACE, _D "Vol hex: %02X:%02X:%02X:%02X\r\n",
            st_mif_cal_start.ref_voltage[0], st_mif_cal_start.ref_voltage[1],
            st_mif_cal_start.ref_voltage[2], st_mif_cal_start.ref_voltage[3]);
    DPRINTF(DBG_TRACE, _D "Vol float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    fval = Ical;
    ToCommFloat(&st_mif_cal_start.ref_current[0], (U8_Float*)&fval);
    DPRINTF(DBG_TRACE, _D "Curr hex: %02X:%02X:%02X:%02X\r\n",
            st_mif_cal_start.ref_current[0], st_mif_cal_start.ref_current[1],
            st_mif_cal_start.ref_current[2], st_mif_cal_start.ref_current[3]);
    DPRINTF(DBG_TRACE, _D "Curr float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    fval = theta_cal;
    ToCommFloat(&st_mif_cal_start.ref_phase[0], (U8_Float*)&fval);
    DPRINTF(DBG_TRACE, _D "ref_phase hex: %02X:%02X:%02X:%02X\r\n",
            st_mif_cal_start.ref_phase[0], st_mif_cal_start.ref_phase[1],
            st_mif_cal_start.ref_phase[2], st_mif_cal_start.ref_phase[3]);
    DPRINTF(DBG_TRACE, _D "ref_phase float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    st_mif_cal_start.process_time = Scal;
#if 0
    st_mif_cal_start.act_const          = dsm_htons(PulseKwh);
    st_mif_cal_start.react_const        = dsm_htons(PulseKvarh);
    st_mif_cal_start.app_const          = dsm_htons(PulseKvah);
#else
    st_mif_cal_start.act_const = (PulseKwh);
    st_mif_cal_start.react_const = (PulseKvarh);
    st_mif_cal_start.app_const = (PulseKvah);
#endif
    DPRINTF(DBG_TRACE, _D "process_time: %02d SEC\r\n",
            st_mif_cal_start.process_time);
    // DPRINTF(DBG_TRACE, _D"CONST: [%08d : %08d : %08d]\r\n",
    // st_mif_cal_start.react_const, st_mif_cal_start.act_const,
    // st_mif_cal_start.app_const);
    DPRINTF(DBG_TRACE, _D "CAL: 0x%04X, 0x%04X, 0x%04X\r\n",
            st_mif_cal_start.react_const, st_mif_cal_start.act_const,
            st_mif_cal_start.app_const);

    memcpy((uint8_t*)&pst_mtp_cal_point->val, (uint8_t*)&st_mif_cal_start,
           sizeof(ST_MIF_CAL_START));
#if 1  // 부팅시 항상 NV에 write 함으로 제거 , metering part 에서 실제 cal point
       // 사용하지 않음.
    nv_write(I_MTP_CAL_POINT, (uint8_t*)pst_mtp_cal_point);
#endif
}

void dsm_mtp_default_parm(ST_MTP_PARM* pst_mtp_parm)
{
    float fval;
    ST_MIF_METER_PARM st_mif_meter_parm;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    fval = (80.0);
    ToCommFloat(&st_mif_meter_parm.cut_voltage_thr[0],
                (U8_Float*)&fval);  // 80%
    DPRINTF(DBG_TRACE, _D "cut_Vol hex: %02X:%02X:%02X:%02X\r\n",
            st_mif_meter_parm.cut_voltage_thr[0],
            st_mif_meter_parm.cut_voltage_thr[1],
            st_mif_meter_parm.cut_voltage_thr[2],
            st_mif_meter_parm.cut_voltage_thr[3]);
    DPRINTF(DBG_TRACE, _D "cut_Vol float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));

    fval = (0.015);
    ToCommFloat(&st_mif_meter_parm.start_current_thr[0],
                (U8_Float*)&fval);  // 15mA
    DPRINTF(DBG_TRACE, _D "cut_Curr hex: %02X:%02X:%02X:%02X\r\n",
            st_mif_meter_parm.start_current_thr[0],
            st_mif_meter_parm.start_current_thr[1],
            st_mif_meter_parm.start_current_thr[2],
            st_mif_meter_parm.start_current_thr[3]);
    DPRINTF(DBG_TRACE, _D "cut_Curr float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));

    st_mif_meter_parm.direct_reverse = 0;   // 0:수전 1:송수전
    st_mif_meter_parm.reactive_select = 1;  // 1:지상 2:진상 4:피상
#if PHASE_NUM == SINGLE_PHASE
    st_mif_meter_parm.meter_method = 0;  // 0:Normal, 1:VectSum, 2:AntiFraud
#else
    st_mif_meter_parm.meter_method = 1;  // 0:Normal, 1:VectSum, 2:AntiFraud
#endif
    st_mif_meter_parm.pulse_select = 1;  // 0:App+Eoi, 1:Act+React 2:Eoi+Act
    DPRINTF(DBG_TRACE, _D "mise hex: %02X:%02X:%02X:%02X\r\n",
            st_mif_meter_parm.direct_reverse, st_mif_meter_parm.reactive_select,
            st_mif_meter_parm.meter_method, st_mif_meter_parm.pulse_select);

    memcpy((uint8_t*)&pst_mtp_parm->val, (uint8_t*)&st_mif_meter_parm,
           sizeof(ST_MIF_METER_PARM));
    nv_write(I_MTP_PARM, (uint8_t*)pst_mtp_parm);
}

void dsm_mtp_default_sagswell(ST_MTP_SAGSWELL* pst_mtp_sagswell)
{
    float fval;
    ST_MIF_SAGSWELL_SETUP st_mif_sagswell;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    fval = (176.0);
    ToCommFloat(&st_mif_sagswell.pf_level[0], (U8_Float*)&fval);  // 80%
    DPRINTF(DBG_TRACE, _D "pf_level hex: %02X:%02X:%02X:%02X\r\n",
            st_mif_sagswell.pf_level[0], st_mif_sagswell.pf_level[1],
            st_mif_sagswell.pf_level[2], st_mif_sagswell.pf_level[3]);
    DPRINTF(DBG_TRACE, _D "pf_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));

    st_mif_sagswell.pf_continue_time = dsm_htons(2000);  // 2
    fval = (220.0 * 90.0 / 100.0);
    ToCommFloat(&st_mif_sagswell.sag_level[0],
                (U8_Float*)&fval);  // 90%(198) ~ 30%(66V)
    DPRINTF(DBG_TRACE, _D "sag_level hex: %02X:%02X:%02X:%02X\r\n",
            st_mif_sagswell.sag_level[0], st_mif_sagswell.sag_level[1],
            st_mif_sagswell.sag_level[2], st_mif_sagswell.sag_level[3]);
    DPRINTF(DBG_TRACE, _D "sag_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    st_mif_sagswell.sag_time = 5;
    fval = (220.0 * 120.0 / 100.0);
    ToCommFloat(&st_mif_sagswell.swell_level[0],
                (U8_Float*)&fval);  // 120%(264V) ~ 110(242V)
    DPRINTF(DBG_TRACE, _D "swell_level hex: %02X:%02X:%02X:%02X\r\n",
            st_mif_sagswell.swell_level[0], st_mif_sagswell.swell_level[1],
            st_mif_sagswell.swell_level[2], st_mif_sagswell.swell_level[3]);
    DPRINTF(DBG_TRACE, _D "swell_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    st_mif_sagswell.swell_time = 5;
    DPRINTF(DBG_TRACE, _D "times hex: %04X:%02X:%02X:\r\n",
            st_mif_sagswell.pf_continue_time, st_mif_sagswell.sag_time,
            st_mif_sagswell.swell_time);

    memcpy((uint8_t*)&pst_mtp_sagswell->val, (uint8_t*)&st_mif_sagswell,
           sizeof(ST_MIF_SAGSWELL_SETUP));
    nv_write(I_MTP_SAG_SWELL, (uint8_t*)pst_mtp_sagswell);
}

void dsm_mtp_fsm_tx_proc_eoipulse(void) { dsm_mif_actreq_eoipulse(); }

#ifdef MTP_ZCD_ON_OFF
void dsm_mtp_fsm_tx_proc_zcdpulse(void) { dsm_mif_actreq_zcdpulse(); }
void dsm_mtp_fsm_tx_proc_zcdoff(void) { dsm_mif_actreq_zcdoff(); }
#endif

void dsm_mtp_fsm_tx_proc_calstart(void)
{
    float fval;
    ST_MIF_CAL_START* pst_mif_cal_start = dsm_mtp_get_cal_start();
    ;

    DPRINTF(DBG_TRACE, _D "CalStart_ActionRequest\r\n");
    ToHFloat((U8_Float*)&fval, &pst_mif_cal_start->ref_voltage[0]);
    DPRINTF(DBG_TRACE, _D "voltage: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    ToHFloat((U8_Float*)&fval, &pst_mif_cal_start->ref_current[0]);
    DPRINTF(DBG_TRACE, _D "current: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    ToHFloat((U8_Float*)&fval, &pst_mif_cal_start->ref_phase[0]);
    DPRINTF(DBG_TRACE, _D "phase: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    DPRINTF(DBG_TRACE, _D "process_time: %02d SEC\r\n",
            pst_mif_cal_start->process_time);
    DPRINTF(DBG_TRACE, _D "CONST: [%08d : %08d : %08d]\r\n",
            pst_mif_cal_start->react_const, pst_mif_cal_start->act_const,
            pst_mif_cal_start->app_const);

    dsm_mif_actreq_cal_start((uint8_t*)pst_mif_cal_start,
                             sizeof(ST_MIF_CAL_START));  // send actreq calstart
}

void dsm_mtp_fsm_tx_proc_caldataset(void)
{
    ST_MIF_CAL_DATA st_mif_cal_data;

    st_mif_cal_data.r_current_gain = dsm_htonl(g_mtp_caldata.r_current_gain);
    st_mif_cal_data.r_voltage_gain = dsm_htonl(g_mtp_caldata.r_voltage_gain);
    st_mif_cal_data.r_phase_gain = dsm_htonl(g_mtp_caldata.r_phase_gain);
    st_mif_cal_data.s_current_gain = dsm_htonl(g_mtp_caldata.s_current_gain);
    st_mif_cal_data.s_voltage_gain = dsm_htonl(g_mtp_caldata.s_voltage_gain);
    st_mif_cal_data.s_phase_gain = dsm_htonl(g_mtp_caldata.s_phase_gain);
    st_mif_cal_data.t_current_gain = dsm_htonl(g_mtp_caldata.t_current_gain);
    st_mif_cal_data.t_voltage_gain = dsm_htonl(g_mtp_caldata.t_voltage_gain);
    st_mif_cal_data.t_phase_gain = dsm_htonl(g_mtp_caldata.t_phase_gain);
    st_mif_cal_data.cal_ok = 1;
    DPRINTF(DBG_TRACE, _D "CalData_SetRequest\r\n");
    DPRINTF(DBG_TRACE,
            _D
            "CAL_DATA R: cur_gain[0x%08X], vol_gain[0x%08X], "
            "phase_gain[0x%08X]\r\n",
            g_mtp_caldata.r_current_gain, g_mtp_caldata.r_voltage_gain,
            g_mtp_caldata.r_phase_gain);
    DPRINTF(DBG_TRACE,
            _D
            "CAL_DATA S: cur_gain[0x%08X], vol_gain[0x%08X], "
            "phase_gain[0x%08X]\r\n",
            g_mtp_caldata.s_current_gain, g_mtp_caldata.s_voltage_gain,
            g_mtp_caldata.s_phase_gain);
    DPRINTF(DBG_TRACE,
            _D
            "CAL_DATA T: cur_gain[0x%08X], vol_gain[0x%08X], "
            "phase_gain[0x%08X]\r\n",
            g_mtp_caldata.t_current_gain, g_mtp_caldata.t_voltage_gain,
            g_mtp_caldata.t_phase_gain);
    dsm_mif_setreq_cal_data((uint8_t*)&st_mif_cal_data,
                            sizeof(ST_MIF_CAL_DATA));

    no_inst_curr_chk_zon_cnt = 7;
}

void dsm_mtp_fsm_tx_proc_parmset(void)
{
    float fval;
    ST_MIF_METER_PARM* pst_mif_meter_parm = dsm_mtp_get_meter_parm();

    DPRINTF(DBG_TRACE, _D "ParmData_SetRequest\r\n");
    ToHFloat((U8_Float*)&fval, &pst_mif_meter_parm->cut_voltage_thr[0]);
    DPRINTF(DBG_TRACE, _D "cut_vol_thres: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    ToHFloat((U8_Float*)&fval, &pst_mif_meter_parm->start_current_thr[0]);
    DPRINTF(DBG_TRACE, _D "start_curr_thres: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    DPRINTF(DBG_TRACE,
            _D
            "DIR[%02X], REATIVE_SEL[%02X], MT_METHOD[%02X], "
            "PULSE_SEL[%02X]\r\n",
            pst_mif_meter_parm->direct_reverse,
            pst_mif_meter_parm->reactive_select,
            pst_mif_meter_parm->meter_method, pst_mif_meter_parm->pulse_select);

    dsm_mif_setreq_meter_setup_parm((uint8_t*)pst_mif_meter_parm,
                                    sizeof(ST_MIF_METER_PARM));

    // jp.kim 25.03.13
    // no_inst_curr_chk_zon_cnt = 3;
}

void dsm_mtp_fsm_tx_proc_sagswellset(void)
{
    float fval;
    ST_MIF_SAGSWELL_SETUP* pst_mif_sagswell = dsm_mtp_get_sagswell();

    DPRINTF(DBG_TRACE, _D "SagSwell_SetRequest\r\n");
    ToHFloat((U8_Float*)&fval, &pst_mif_sagswell->pf_level[0]);
    DPRINTF(DBG_TRACE, _D "pf_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    DPRINTF(DBG_TRACE, _D "pf_contiue_time: 0x%04X\r\n",
            pst_mif_sagswell->pf_continue_time);
    ToHFloat((U8_Float*)&fval, &pst_mif_sagswell->sag_level[0]);
    DPRINTF(DBG_TRACE, _D "sag_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    DPRINTF(DBG_TRACE, _D "sag_time: 0x%02X\r\n", pst_mif_sagswell->sag_time);
    ToHFloat((U8_Float*)&fval, &pst_mif_sagswell->swell_level[0]);
    DPRINTF(DBG_TRACE, _D "swell_level float: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    DPRINTF(DBG_TRACE, _D "swell_time: 0x%02X\r\n",
            pst_mif_sagswell->swell_time);

    dsm_mif_setreq_sagswell_data((uint8_t*)pst_mif_sagswell,
                                 sizeof(ST_MIF_SAGSWELL_SETUP));

    // jp.kim 25.03.13
    no_inst_curr_chk_zon_cnt = 5;
}

void dsm_mtp_meter_fw_down_init(void)
{
    g_flash_posi = 0;
    g_fw_length = 0;
    g_fw_file_end = 0;
    g_fw_file_start = 0;
}

uint8_t dsm_mtp_meter_fw_read_parser(uint8_t* pData, uint32_t len,
                                     uint8_t* dest)
{
    uint8_t *pos, *porg;
    uint8_t sposi = 0;
    uint8_t record_limit = 0;
    uint16_t record_tlen = 0, pkt_tlen = 0, record = 0;
    uint8_t data[5];
    uint8_t data_len, data_type, ret = 0;
    uint8_t err_cnt = 0;

    pos = pData;
    do
    {
        porg = pos;
        data[0] = *pos;
        if (data[0] == ':')
        {
            pos += 1;
            data[1] = *pos++;
            data[2] = *pos++;
            porg += 7;
            data[3] = *porg++;
            data[4] = *porg;

            record++;
            data_len =
                (uint8_t)((AsciiToHEX(data[1]) << 4) | (AsciiToHEX(data[2])));
            data_type =
                (uint8_t)((AsciiToHEX(data[3]) << 4) | (AsciiToHEX(data[4])));
            if (data_len == 0)  // last record:
            {
                if (data_type == 0x01)
                {
                    g_fw_file_end = 0x01;
                }
                record_tlen = ((data_len + 4) * 2) +
                              3;  // 5 (Len(1)addr(2)type(1)chsum(1)) 3(:+0D+0A)
                record_limit = record;
            }
            else
            {
                if (record == 2)
                {
                    if (data_len == 0x10)
                    {
                        record_limit = 5;
                    }
                    else if (data_len == 0x20)
                    {
                        record_limit = 3;
                    }
                }
                record_tlen = ((data_len + 5) * 2) +
                              3;  // 5 (Len(1)addr(2)type(1)chsum(1)) 3(:+0D+0A)
            }

            DPRINTF(DBG_NONE,
                    "MT F/W IHex Record(%d) Len(%d) RTotL(%d) DType(%d)\r\n",
                    record, data_len, record_tlen, data_type);

            if (data_len > 255)
            {
                DPRINTF(DBG_ERR,
                        "INTEL HEX Length Record Parsing Error - (%d)\r\n",
                        data_len);
                return ret;
            }
            memcpy(&dest[sposi + 2], &pData[sposi], record_tlen);
            pos += record_tlen - 3;
            sposi += record_tlen;
            pkt_tlen += record_tlen;
            len -= record_tlen;
            if (data_type == 0x01)
            {
                g_fw_file_end = 0x01;
            }
            if ((record >= record_limit) && (record_limit != 0))
            {
                g_fw.pre_posi = g_flash_posi;
                g_flash_posi += pkt_tlen;
                ret = pkt_tlen;
                len = 0;
                g_fw.record_len += record_limit;
                DPRINTF(DBG_INFO,
                        "Record Cnt(%d) pkt_tlen(%d) g_flash_posi(%d) "
                        "record_limit(%d)\r\n",
                        record, pkt_tlen, g_flash_posi, g_fw.record_len);
                dest[0] = (g_fw.record_len & 0xFF00) >> 8;
                dest[1] = (g_fw.record_len & 0x00FF);
                g_fw.clr_flag = 0x1;

                break;
            }
        }
        else
        {
            len--;
            DPRINTF(DBG_ERR, "MT F/W Down len(%d)\r\n", len);

            if (err_cnt++ > 76)
            {
                err_cnt = 0;
                break;
            }
        }
    } while (len > 0);

    return ret;
}

uint8_t dsm_mtp_meter_fw_download(uint8_t* pTdata, uint8_t opt)
{
    uint8_t buf[256] = {
        0,
    };
    uint32_t flash_addr = SFLASH_METER_FW_BLK_ADDR;
    uint8_t pkt_len = 0;

    dsm_spi_init();

    if (opt == 0)
    {
        g_flash_posi = 0;
    }
    if (g_fw.clr_flag != 0x0)
    {
        g_flash_posi = g_fw.pre_posi;
    }
    DPRINTF(DBG_INFO, _D "g_flash_posi(%d)\r\n", g_flash_posi);
    CMD_READ(flash_addr + g_flash_posi, buf, 256);

    pkt_len = dsm_mtp_meter_fw_read_parser(buf, 256, pTdata);

    if (pkt_len != 0)
    {
        DPRINT_HEX(DBG_TRACE, "FLASH READ & SEND DATA:", &pTdata[2], pkt_len,
                   DUMP_ALWAYS);
    }
    else
    {
        DPRINTF(DBG_ERR, "MT F/W Download Packet is zero \r\n");
    }

    return pkt_len;
}

bool dsm_mtp_fsm_tx_proc_fw_data_set(uint8_t type, uint8_t idx)
{
    uint8_t tx_data[300];
    uint8_t cnt = 0, ret = 0;
    bool ret_val = true;

    if (type == 0)
    {
        tx_data[cnt++] = 'Y';
        ret = 1;
        afe_init_start();
    }
    else if (type == 1)
    {
        tx_data[cnt++] = 'X';
        tx_data[cnt++] = idx;
        ret = 1;
        afe_init_start();

        meter_firmup_delay_ing = true;
        meter_firmup_delay_timeset(T5SEC);
    }
    else if (type == 2)
    {
        tx_data[cnt++] = 'W';
        if (idx == 0x0)
        {
            ret = dsm_mtp_meter_fw_download(&tx_data[cnt], 0);
            if (g_fw_file_start == 0x00)
            {
                g_fw_file_start = 0x01;
            }
        }
        else
        {
            ret = dsm_mtp_meter_fw_download(&tx_data[cnt], 1);
        }
        if (ret != 0)
        {
            cnt += ret;
        }
        else
        {
            DPRINTF(DBG_ERR, "ZBM FW Download Failed!!!\r\n");
            return false;
        }

        DPRINTF(DBG_INFO, _D "ZBM FW Download set Packet Total Len(%d)\r\n",
                cnt);
    }
    else if (type == 3)
    {
        tx_data[cnt++] = 'S';
        ret = 1;
    }
    else if (type == 4)
    {
        tx_data[cnt++] = 'E';
        ret = 1;
    }

    dsm_mif_setreq_fw_data(&tx_data[0], cnt);

    return ret_val;
}

void dsm_mtp_fsm_tx_proc_fw_data_get(uint8_t type, uint8_t idx)
{
    uint8_t tx_data[2] = {0};
    uint8_t cnt = 0;

    if (type == 0)
    {
        tx_data[cnt++] = 'I';
    }
    else if (type == 1)
    {
        tx_data[cnt++] = 'F';
        tx_data[cnt++] = idx;
    }
    else if (type == 2)
    {
        tx_data[cnt++] = 'T';
        tx_data[cnt++] = idx;
    }

    dsm_mtp_set_fw_type(type);
    dsm_mtp_set_fw_index(idx);
    DPRINTF(DBG_TRACE, _D "ZBM FW Data Get CMD(%X) Index(%d)\r\n",
            /*tx_data[0], tx_data[1]*/ type, idx);
    dsm_mif_getreq_fw_data(&tx_data[0], cnt);
}

void dsm_mtp_fw_change(void)
{
    // dsm_mtp_fsm_tx_proc_fw_data_set(0,0);   // select youngest
    // firmware...
    dsm_mtp_fsm_tx_proc_fw_data_set(1, 3);  // select the other firmware...
}

void dsm_mtp_tx_retry_TO_proc(void)
{
    uint8_t fsm = dsm_mtp_get_fsm();

    if (g_st_mtp_con.TO_retry++ < MIF_MTP_RETRY_MAX_TO_COUNT)
    {
        DPRINTF(DBG_TRACE, "%s: TO_retry_cnt[%d], MTP_FSM[%s]\r\n", __func__,
                g_st_mtp_con.TO_retry, dsm_mtp_fsm_string(fsm));
        dsm_mtp_fsm_send();
    }
    else
    {
        DPRINTF(DBG_ERR, "%s: max retry error!!!, MTP_FSM[%s]\r\n", __func__,
                dsm_mtp_fsm_string(fsm));
        g_st_mtp_con.TO_retry = 0;
        dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
        dsm_mtp_fsm_send();
        dsm_meter_sw_timer_stop(MT_SW_TIMER_MIF_MTP_TX_TO);
    }
}

void dsm_mtp_meter_fw_download_proc(void)
{
    dsm_mtp_meter_fw_down_init();
    memset(&g_fw, 0x00, sizeof(ST_MTP_FW_WR));
    dsm_mtp_set_fw_type(3);
    dsm_mtp_fsm_tx_proc_fw_data_set(3, 0);
}

void dsm_mtp_meter_fw_download_finish(void) { dsm_mtp_meter_fw_down_init(); }

void dsm_set_meter_fw_down_complete(uint8_t val) { g_fw.complete_flag = val; }

uint8_t dsm_get_meter_fw_down_complete(void) { return g_fw.complete_flag; }

uint32_t dsm_mtp_fsm_tx_proc(void)
{
    uint8_t fsm = dsm_mtp_get_fsm();
    uint8_t op_mode = dsm_mtp_get_op_mode();
    uint8_t type = dsm_mtp_get_fw_type();
    uint8_t index = dsm_mtp_get_fw_index();

    DPRINTF(DBG_TRACE, "%s: %s[%d], (%d, %d)\r\n", __func__,
            dsm_mtp_op_mode_string(op_mode), fsm, type, index);

    if (op_mode == MTP_OP_NORMAL)
    {
        switch (fsm)
        {
        case MTP_FSM_CAL_ST:
            dsm_mtp_fsm_tx_proc_calstart();
            break;

        case MTP_FSM_CAL_GET:
            DPRINTF(DBG_TRACE, _D "CalData_GetRequest\r\n");
            MSG09("dsm_mif_getreq_cal_data");
            dsm_mif_getreq_cal_data();
            vTaskDelay(1); /* 2025-09-18 */
            break;

        case MTP_FSM_CAL_SET:
            dsm_mtp_fsm_tx_proc_caldataset();
            vTaskDelay(10); /* 2024-06-13, JnD 출장 수정 */
            break;

        case MTP_FSM_PARM_GET:
            dsm_mif_getreq_meter_setup_parm();
            break;

        case MTP_FSM_PARM_SET:
            dsm_mtp_fsm_tx_proc_parmset();
            break;

        case MTP_FSM_SAG_SWELL_GET:
            dsm_mif_getreq_sagswell_data();  // send getreq sagswell
            break;

        case MTP_FSM_SAG_SWELL:
            dsm_mtp_fsm_tx_proc_sagswellset();
            break;

        case MTP_FSM_EOI_PULSE_ST:
            dsm_mtp_fsm_tx_proc_eoipulse();
            break;

#ifdef MTP_ZCD_ON_OFF
        case MTP_FSM_ZCD_PULSE_ST:
            dsm_mtp_fsm_tx_proc_zcdpulse();
            break;

        case MTP_FSM_ZCD_PULSE_END:
            dsm_mtp_fsm_tx_proc_zcdoff();
            break;
#endif

        case MTP_FSM_MESURE_READY:
            break;

        case MTP_FSM_FW_SET: /* sjan 20201006 */
            if (type == 0)
            {
                dsm_mtp_fsm_tx_proc_fw_data_set(0, 0);
            }
            else if (type == 1)
            {
                dsm_mtp_fsm_tx_proc_fw_data_set(1, index);
            }
            else if (type == 2)
            {
                bool blk_valid = true;

                if (g_fw_file_start)
                {
                    blk_valid = dsm_mtp_fsm_tx_proc_fw_data_set(2, 1);
                }
                else
                {
                    blk_valid = dsm_mtp_fsm_tx_proc_fw_data_set(2, 0);
                }

                if (blk_valid == false)
                {
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                    dsm_mtp_fsm_send();
                }
            }
            else if (type == 3)
            {
                dsm_mtp_fsm_tx_proc_fw_data_set(3, 0);
            }
            else if (type == 4)
            {
                dsm_mtp_fsm_tx_proc_fw_data_set(4, 0);
            }
            break;

        case MTP_FSM_FW_GET:
            if (type == 0)
            {
                dsm_mtp_fsm_tx_proc_fw_data_get(0, 0);
            }
            else if (type == 1)
            {
                dsm_mtp_fsm_tx_proc_fw_data_get(1, index);
            }
            else if (type == 2)
            {
                dsm_mtp_fsm_tx_proc_fw_data_get(2, index);
            }
            break;

        default:
            break;
        }
    }
    else
    {
        switch (fsm)
        {
        case MTP_FSM_CAL_ST:
            dsm_mtp_fsm_tx_proc_calstart();
            break;

        case MTP_FSM_EOI_PULSE_ST:
            dsm_mtp_fsm_tx_proc_eoipulse();
            break;

#ifdef MTP_ZCD_ON_OFF
        case MTP_FSM_ZCD_PULSE_ST:
            dsm_mtp_fsm_tx_proc_zcdpulse();
            break;

        case MTP_FSM_ZCD_PULSE_END:
            dsm_mtp_fsm_tx_proc_zcdoff();
            break;
#endif

        case MTP_FSM_CAL_GET:
            dsm_mif_getreq_cal_data();
            break;

        case MTP_FSM_NONE:
            break;

        case MTP_FSM_CAL_SET:
            dsm_mtp_fsm_tx_proc_caldataset();

            break;
        case MTP_FSM_PARM_SET:
            dsm_mtp_fsm_tx_proc_parmset();

            break;
        case MTP_FSM_SAG_SWELL:
            dsm_mtp_fsm_tx_proc_sagswellset();

            break;
        case MTP_FSM_MESURE_READY:

            break;

        case MTP_FSM_FW_SET: /* sjan 20201006 */

            if (type == 0)
            {
                dsm_mtp_fsm_tx_proc_fw_data_set(0, 0);
            }
            else if (type == 1)
            {
                dsm_mtp_fsm_tx_proc_fw_data_set(1, 0);
            }
            else if (type == 2)
            {
                if (g_fw_file_start)
                {
                    dsm_mtp_fsm_tx_proc_fw_data_set(2, 1);
                }
                else
                {
                    dsm_mtp_fsm_tx_proc_fw_data_set(2, 0);
                }
            }
            else if (type == 3)
            {
                dsm_mtp_fsm_tx_proc_fw_data_set(3, 0);
            }
            break;

        case MTP_FSM_FW_GET:
            if (type == 0)
            {
                dsm_mtp_fsm_tx_proc_fw_data_get(0, 0);
            }
            else if (type == 1)
            {
                dsm_mtp_fsm_tx_proc_fw_data_get(1, index);
            }
            else if (type == 2)
            {
                dsm_mtp_fsm_tx_proc_fw_data_get(2, index);
            }
            break;

        default:
            break;
        }
    }

    return TRUE;
}

uint32_t dsm_mtp_fsm_rx_proc(uint32_t evt)
{
    int32_t maglevel;

    uint8_t fsm = dsm_mtp_get_fsm();
    uint8_t op_mode = dsm_mtp_get_op_mode();

    if (op_mode == MTP_OP_NORMAL)
    {
        switch (fsm)
        {
        case MTP_FSM_NONE:
            break;

        case MTP_FSM_CAL_ST:
            if (evt == MIF_ActAck)
            {
                MSG09("CAL_START: ActAck, no nothing");
                DPRINTF(DBG_TRACE,
                        _D "%s: CAL_START: ActAck OK -> timer start\r\n",
                        __func__);
            }
            else if (evt == MIF_ActNack)
            {
                MSG09("CAL_START: ActNac, start again");

                cal_begin();
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_CAL_ST);
                dsm_mtp_fsm_send();
            }

            break;

        case MTP_FSM_EOI_PULSE_ST:
            if (evt == MIF_ActAck)
            {
                DPRINTF(DBG_TRACE,
                        _D "%s: EOI_PULSE_START: ActAck OK -> timer start\r\n",
                        __func__);
            }
            else if (evt == MIF_ActNack)
            {
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_EOI_PULSE_ST);
                dsm_mtp_fsm_send();
            }
            break;

#ifdef MTP_ZCD_ON_OFF
        case MTP_FSM_ZCD_PULSE_ST:
            if (evt == MIF_ActAck)
            {
                DPRINTF(DBG_TRACE,
                        _D "%s: ZCD_PULSE_START: ActAck OK -> timer start\r\n",
                        __func__);
            }
            else if (evt == MIF_ActNack)
            {
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_ZCD_PULSE_ST);
                dsm_mtp_fsm_send();
            }
            break;

        case MTP_FSM_ZCD_PULSE_END:
            if (evt == MIF_ActAck)
            {
                DPRINTF(DBG_TRACE,
                        _D "%s: ZCD_PULSE_END: ActAck OK -> timer start\r\n",
                        __func__);
            }
            else if (evt == MIF_ActNack)
            {
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_ZCD_PULSE_END);
                dsm_mtp_fsm_send();
            }
            break;
#endif

        case MTP_FSM_CAL_GET:
            if (evt == MIF_GetRsp_CalData)
            {
                if (g_mtp_caldata.ok == 1)
                {
                    MSG09("CAL_GET: cal data ok");

                    g_st_mtp_con.retry = 0;
                    DPRINTF(DBG_TRACE, _D "%s: CAL_GET: cal data ok\r\n",
                            __func__);
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                    dsm_mtp_fsm_send();

#ifdef M2354_CAN /* bccho, 2023-11-28 */
                    // cal data ok시 can advertize
                    dsm_can_advertisement_power_on();
#endif
                    cal_data_get_success = 1;
                }
                else
                {
                    MSG09("CAL_GET: cal data nok");

                    if (g_st_mtp_con.retry++ < CAL_END_CHECK_TIMES)
                    {
                        DPRINTF(DBG_TRACE, _D "%s: CAL_GET: Retry[%d]\r\n",
                                __func__, g_st_mtp_con.retry);
                        OSTimeDly(OS_MS2TICK(
                            MT_TIMEOUT_MS_CAL_GETREQ_RETRY_TIME / 2));
                        dsm_mtp_fsm_send();
                    }
                    else
                    {
                        g_st_mtp_con.retry = 0;
                    }
                }
            }
            break;

        case MTP_FSM_CAL_SET:
            if (evt == MIF_SetAck)
            {
                g_st_mtp_con.calset_ok = TRUE;

                DPRINTF(DBG_TRACE, _D "%s: CAL_SET: SetAck OK\r\n", __func__);

                dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                dsm_mtp_fsm_send();

#ifdef M2354_CAN /* bccho, 2023-11-28 */
                // cal data ok시 can advertize
                dsm_can_advertisement_power_on();
#endif
            }
            else if (evt == MIF_SetNack)
            {
                // retry
                OSTimeDly(OS_MS2TICK(MT_TIMEOUT_MS_CAL_GETREQ_RETRY_TIME / 2));
                dsm_mtp_fsm_send();
            }
            break;

        case MTP_FSM_PARM_GET:

            break;

        case MTP_FSM_PARM_SET:
            if (evt == MIF_SetAck)
            {
                g_st_mtp_con.parmset_ok = TRUE;

                DPRINTF(DBG_TRACE, _D "%s: PARM_SET: SetAck OK\r\n", __func__);
                g_st_mtp_con.retry = 0;
#if 0  // defined(FEATURE_JP_PWON_PARA_LOAD_MTP)                
                if (PwOn_1st_parm_set)
                {
                    PwOn_1st_parm_set = false;
                    dsm_mtp_set_fsm(MTP_FSM_SAG_SWELL);
                    dsm_mtp_fsm_send();
                }
                else
                {
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                    dsm_mtp_fsm_send();
                }
#else
                DPRINTF(DBG_TRACE,
                        _D "%s: PARM_SET: SetAck OK => MESURE_READY\r\n",
                        __func__);
                dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                dsm_mtp_fsm_send();
#endif
            }
            else if (evt == MIF_SetNack)
            {
                if (g_st_mtp_con.retry++ < MTP_CHECK_TIMES)
                {
                    dsm_mtp_fsm_send();
                }
                else
                {
                    DPRINTF(DBG_ERR,
                            _D
                            "%s: PARM_SET: fail: retry[%d]!!! => "
                            "MESURE_READY\r\n",
                            __func__, g_st_mtp_con.retry);
                    g_st_mtp_con.retry = 0;
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                }
            }
            break;

        case MTP_FSM_SAG_SWELL_GET:
            break;

        case MTP_FSM_SAG_SWELL:
            if (evt == MIF_SetAck)
            {
                g_st_mtp_con.sagswell_ok = TRUE;

                DPRINTF(DBG_TRACE,
                        _D "%s: SAG_SWELL_SET: SetAck OK => MESURE_READY\r\n",
                        __func__);
                g_st_mtp_con.retry = 0;
                dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                dsm_mtp_fsm_send();
            }
            else if (evt == MIF_SetNack)
            {
                if (g_st_mtp_con.retry++ < MTP_CHECK_TIMES)
                {
                    dsm_mtp_fsm_send();
                }
                else
                {
                    DPRINTF(DBG_ERR,
                            _D
                            "%s: SAG_SWELL_SET: fail: retry[%d]!!! => "
                            "MESURE_READY\r\n",
                            __func__, g_st_mtp_con.retry);
                    g_st_mtp_con.retry = 0;
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                }
            }
            break;

        case MTP_FSM_MESURE_READY:
            break;

        case MTP_FSM_FW_START_SET:
            dsm_mtp_set_fw_type(2);
            DPRINTF(DBG_INFO, _D "%s: FSM FW_START_SET change\r\n", __func__);

            dsm_mtp_set_fsm(MTP_FSM_FW_SET);
            dsm_mtp_fsm_send();

            break;

        case MTP_FSM_FW_SET:
            if (g_fw_file_start && !g_fw_file_end)
            {
                dsm_mtp_set_fsm(MTP_FSM_FW_SET);
                dsm_mtp_fsm_send();
            }
            else if (g_fw_file_start && g_fw_file_end)
            {
                dsm_mtp_set_fw_type(4);
                dsm_mtp_set_fsm(MTP_FSM_FW_SET);
                dsm_mtp_fsm_send();
            }
            break;

        case MTP_FSM_FW_GET:
            DPRINTF(DBG_NONE, "DBG\r\n");
            dsm_uart_deq_string(DEBUG_COM);
            break;

        default:
            break;
        }
    }
    else /* MTP_OP_CAL */
    {
        switch (fsm)
        {
        case MTP_FSM_NONE:
            break;

        case MTP_FSM_CAL_ST:
            if (evt == MIF_ActAck)
            {
                DPRINTF(DBG_TRACE,
                        _D "%s: CAL_START: ActAck OK -> timer start\r\n",
                        __func__);

                g_st_mtp_con.retry = 0;

                dsm_meter_sw_timer_start(
                    MT_SW_TIMER_CAL_GETREQ_TO, FALSE,
                    MT_TIMEOUT_MS_CAL_GETREQ_AFTER_CALST_ACK_TIME);
            }
            else if (evt == MIF_ActNack)
            {
                cal_begin();
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_CAL_ST);
                dsm_mtp_fsm_send();
            }
            break;

        case MTP_FSM_EOI_PULSE_ST:
            if (evt == MIF_ActAck)
            {
                DPRINTF(DBG_TRACE,
                        _D "%s: EOI_PULSE_START: ActAck OK -> timer start\r\n",
                        __func__);
            }
            else if (evt == MIF_ActNack)
            {
                DPRINTF(DBG_TRACE,
                        _D
                        "%s: EOI_PULSE_START: ActNAk RE_TRY -> timer start\r\n",
                        __func__);
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_EOI_PULSE_ST);
                dsm_mtp_fsm_send();
            }
            break;

#ifdef MTP_ZCD_ON_OFF
        case MTP_FSM_ZCD_PULSE_ST:
            if (evt == MIF_ActAck)
            {
                DPRINTF(DBG_TRACE,
                        _D "%s: ZCD_PULSE_START: ActAck OK -> timer start\r\n",
                        __func__);
            }
            else if (evt == MIF_ActNack)
            {
                DPRINTF(DBG_TRACE,
                        _D
                        "%s: ZCD_PULSE_START: ActNAk RE_TRY -> timer start\r\n",
                        __func__);
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_ZCD_PULSE_ST);
                dsm_mtp_fsm_send();
            }
            break;

        case MTP_FSM_ZCD_PULSE_END:
            if (evt == MIF_ActAck)
            {
                DPRINTF(DBG_TRACE,
                        _D "%s: ZCD_PULSE_END: ActAck OK -> timer start\r\n",
                        __func__);
            }
            else if (evt == MIF_ActNack)
            {
                DPRINTF(DBG_TRACE,
                        _D
                        "%s: ZCD_PULSE_END: ActNAk RE_TRY -> timer start\r\n",
                        __func__);
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_ZCD_PULSE_END);
                dsm_mtp_fsm_send();
            }
            break;
#endif

        case MTP_FSM_CAL_GET:
            if (evt == MIF_GetRsp_CalData)
            {
                if (g_mtp_caldata.ok == 1)
                {
                    DPRINTF(DBG_TRACE, _D "%s: CAL_GET: cal success!!!\r\n",
                            __func__);
                    g_st_mtp_con.retry = 0;
#if 0
#if defined(FEATURE_MTP_CALSET_OFF)
					dsm_mtp_set_fsm(MTP_FSM_PARM_SET);
#else
					dsm_mtp_set_fsm(MTP_FSM_CAL_SET);
#endif
					dsm_mtp_fsm_send();
#endif
                }
                else
                {
                    if (g_st_mtp_con.retry++ < CAL_END_CHECK_TIMES)
                    {
                        DPRINTF(DBG_TRACE, _D "%s: CAL_GET: Retry[%d]\r\n",
                                __func__, g_st_mtp_con.retry);
                        OSTimeDly(OS_MS2TICK(
                            MT_TIMEOUT_MS_CAL_GETREQ_RETRY_TIME / 2));
                        dsm_mtp_fsm_send();
                    }
                    else
                    {
                        g_st_mtp_con.retry = 0;
                        DPRINTF(DBG_ERR,
                                _D "%s: CAL_GET: rsp OK: cal failed !!!\r\n",
                                __func__);
#if 0  // defined(FEATURE_MTP_CALSET_OFF)
                        dsm_mtp_set_fsm(MTP_FSM_PARM_SET);
#else
                        dsm_mtp_set_fsm(MTP_FSM_CAL_SET);
#endif
                        dsm_mtp_fsm_send();
                    }
                }
            }
            break;

        case MTP_FSM_CAL_SET:
            if (evt == MIF_SetAck)
            {
                g_st_mtp_con.calset_ok = TRUE;
                g_st_mtp_con.retry = 0;
                DPRINTF(DBG_TRACE, _D "%s: CAL_SET: SetAck OK\r\n", __func__);

                dsm_mtp_set_fsm(MTP_FSM_PARM_SET);
                dsm_mtp_fsm_send();
            }
            else if (evt == MIF_SetNack)
            {
                if (g_st_mtp_con.retry++ < MTP_CHECK_TIMES)
                {
                    dsm_mtp_fsm_send();
                }
                else
                {
                    DPRINTF(DBG_ERR,
                            _D
                            "%s: CAL_SET: fail: retry[%d]!!! => "
                            "MESURE_READY\r\n",
                            __func__, g_st_mtp_con.retry);
                    g_st_mtp_con.retry = 0;
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                }
            }
            break;

        case MTP_FSM_PARM_SET:
            if (evt == MIF_SetAck)
            {
                g_st_mtp_con.parmset_ok = TRUE;

                DPRINTF(DBG_TRACE, _D "%s: PARM_SET: SetAck OK\r\n", __func__);

                dsm_mtp_set_fsm(MTP_FSM_SAG_SWELL);
                dsm_mtp_fsm_send();
            }
            else if (evt == MIF_SetNack)
            {
                if (g_st_mtp_con.retry++ < MTP_CHECK_TIMES)
                {
                    dsm_mtp_fsm_send();
                }
                else
                {
                    DPRINTF(DBG_ERR,
                            _D
                            "%s: PARM_SET: fail: retry[%d]!!! => "
                            "MESURE_READY\r\n",
                            __func__, g_st_mtp_con.retry);
                    g_st_mtp_con.retry = 0;
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                }
            }
            break;

        case MTP_FSM_SAG_SWELL:
            if (evt == MIF_SetAck)
            {
                g_st_mtp_con.sagswell_ok = TRUE;

                DPRINTF(DBG_TRACE,
                        _D "%s: SAG_SWELL_SET: SetAck OK => MESURE_READY\r\n",
                        __func__);
                dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
            }
            else if (evt == MIF_SetNack)
            {
                if (g_st_mtp_con.retry++ < MTP_CHECK_TIMES)
                {
                    dsm_mtp_fsm_send();
                }
                else
                {
                    DPRINTF(DBG_ERR,
                            _D
                            "%s: SAG_SWELL_SET: fail: retry[%d]!!! => "
                            "MESURE_READY\r\n",
                            __func__, g_st_mtp_con.retry);
                    g_st_mtp_con.retry = 0;
                    dsm_mtp_set_fsm(MTP_FSM_MESURE_READY);
                }
            }
            break;

        case MTP_FSM_MESURE_READY:

            break;

        case MTP_FSM_FW_SET:  // sjan 20201006
            if (g_fw_file_start && !g_fw_file_end)
            {
                dsm_mtp_set_fsm(MTP_FSM_FW_SET);
                dsm_mtp_fsm_send();
            }
            else if (g_fw_file_start && g_fw_file_end)
            {
                dsm_mtp_meter_fw_download_finish();
                dsm_mtp_set_fw_type(4);
                dsm_mtp_set_fsm(MTP_FSM_FW_SET);
                dsm_mtp_fsm_send();
            }

            break;

        case MTP_FSM_FW_GET:
            DPRINTF(DBG_NONE, "DBG\r\n");
            dsm_uart_deq_string(DEBUG_COM);
            break;
        default:
            break;
        }
    }

    return TRUE;
}
