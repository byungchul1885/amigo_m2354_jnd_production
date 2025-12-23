#include "main.h"
#include "options.h"
#include "nv.h"
#include "dlms_todo.h"
#include "comm.h"
#include "phy.h"
#include "dl.h"
#include "aarq.h"
#include "appl.h"
#include "amg_push_datanoti.h"
#include "amg_power_mnt.h"
#include "amg_sec.h"
#include "get_req.h"
#include "amg_media_mnt.h"
#include "amg_modemif_prtl.h"
#include "amg_meter_main.h"
#include "tmp.h"
#include "amg_imagetransfer.h"
#include "amg_shell_product.h"
#include "mx25r4035f.h"
#include "mx25r4035f_def.h"

#define _D "[DL] "

uint8_t prod_log_in_sts = 0x5a;  // log_off
uint8_t dl_meter_addr;
dl_state_type dl_conn_state;
static uint8_t seg_buff[SEG_BUFF_SIZE];
static bool dl_seg_prev;
static seg_state_type dl_seg_state;
static uint8_t dl_conn_client_addr;
static uint8_t dl_control, prod_dl_control_hi;
static bool dl_poll_final;
static uint8_t dl_seq_Sto;
static uint8_t dl_seq_Rto;
static uint8_t dl_seq_Sfrom;
static uint8_t dl_seq_Rfrom;
uint16_t dl_tx_max_info;
uint16_t dl_rx_max_info;
static int dl_seg_frame_len;
static int dl_seg_frame_offs;
static int dl_seg_frame_offs_back;

bool dl_seg_frame_over;

extern ST_MTP_CAL_DATA g_mtp_caldata;
extern ST_MIF_CAL_START g_mtp_cal_start;

ST_DL_HDLC_EXT gst_dl_tx_hdlc_ext;
ST_DL_HDLC_EXT gst_dl_rx_hdlc_ext;
ST_DL_HDLC_EXT gst_dl_rx_TMP_hdlc_ext;

void dl_frame_tx_for_hdlc_dup(void);
uint16_t dl_appl_get_hdlc_length(void);

static bool dl_frame_parse(void);
static void set_seg_status(bool seg);
static bool dl_addr_parse(void);
static void dl_control_proc(void);
static void dl_ctrl_in_ndm_state(void);
static void dl_ctrl_in_nrm_state(void);
static bool dl_LLC_header_chk(void);
static void dl_send_info_frame(bool seg, uint8_t* buf, uint16_t len);
static void dl_send_FRMR(void);
static void dl_send_SNRM_UA(void);

#if 0
static void prod_dl_ctrl_in_ndm_state(void);
static void prod_dl_control_proc(void);
static void prod_dl_send_SNRM_UA(void);
static void prod_dl_send_DM(void);
static void prod_dl_send_error(U8 sts);
static void prod_dl_frame_tx(uint8_t *buf, uint16_t len);
static void prod_dl_frame_error_tx(uint8_t sts);
static bool prod_dl_frame_parse(void);
void prod_dl_proc(bool frmok);
#endif

static void dl_send_DM(void);
static void dl_send_RR(void);
static void dl_send_UA(void);
static bool dl_control_check(void);
static void dl_appl_msg_proc(void);
static void dl_RR_msg_proc(void);
static bool dl_seg_frame_read(uint8_t* buf, int offs, int len);
static bool dl_seg_frame_write(uint8_t* buf, int offs, int len);
static void dl_stop(bool reason, bool cmdsnrm);
static void dl_seg_info_init(void);
void dsp_comm_is_ing_set(void);
uint8_t byte_to_ascii(uint8_t a);
void prod_control_cal_begin(int idx);
void fw_info_print(fw_info_t* p_fwinfo, uint32_t crc_val);

const uint8_t jnd_ver_fw_id[] = {JND_VERSION_FW_ID};

static const uint8_t nego_dl_parm_asso3[NEGO_PARAM_SIZE_ASSO3] = {
    0x81,
    0x80,  // format id, group id
    0x14,  // group length
    0x05,
    0x02,
    ((TX_MAX_INF_LEN_ASSO3 >> 8) & 0xFF),
    (TX_MAX_INF_LEN_ASSO3 & 0xFF),  // id(max tx info), len, size
    0x06,
    0x02,
    ((RX_MAX_INF_LEN_ASSO3 >> 8) & 0xFF),
    (RX_MAX_INF_LEN_ASSO3 & 0xFF),  // id(max rx info), len, size
    0x07,
    0x04,
    0x00,
    0x00,
    0x00,
    TX_WINDOW_SIZE,  // id(max tx window size), len, size
    0x08,
    0x04,
    0x00,
    0x00,
    0x00,
    RX_WINDOW_SIZE  // id(max rx window size), len, size
};

static const /*__code*/ uint8_t nego_dl_parm[NEGO_PARAM_SIZE] = {
    0x81, 0x80,                  // format id, group id
    0x12,                        // group length
    0x05, 0x01, TX_MAX_INF_LEN,  // id(max tx info), len, size
    0x06, 0x01, RX_MAX_INF_LEN,  // id(max rx info), len, size
    0x07, 0x04, 0x00,
    0x00, 0x00, TX_WINDOW_SIZE,  // id(max tx window size), len, size
    0x08, 0x04, 0x00,
    0x00, 0x00, RX_WINDOW_SIZE  // id(max rx window size), len, size
};

void dl_init(void)
{
    dl_conn_state = DL_NDM_STATE;
    memset(&gst_dl_tx_hdlc_ext, 0x00, sizeof(ST_DL_HDLC_EXT));
    memset(&gst_dl_rx_hdlc_ext, 0x00, sizeof(ST_DL_HDLC_EXT));
}

#if 0
void prod_dl_proc(bool frmok)
{
    bool ok;

    if (frmok)
    {
        // frame data process
        ok = prod_dl_frame_parse();
        if (ok)
        {
            prod_dl_control_proc();
#if defined(FEATURE_JP_COMM_BLINK)
            dsp_comm_is_ing_set();
#endif
        }
    }
    else
    {
        // time out process
        if (dl_is_connected() && comminact_is_timeout())
        {
#if defined(FEATURE_SEC)
            comm_inact_timeset(T120SEC);  // duplicated in amr_rcv_frame()
            DPRINTF(DBG_TRACE, "%s: comm_inact is timeout\r\n",
                    __func__);  // snrm 시작 후, 2분 동안 패킷이 수신 되지 않음.
#else
#if defined(FEATURE_JP_SNRM_DISC_ENABLE)
            dl_stop(false, false);
#else
            dl_stop(false);
#endif
#endif
        }
    }
}
#endif

void dl_proc(bool frmok)
{
    bool ok;

    if (frmok)
    {
        // frame data process
        ok = dl_frame_parse();
        if (ok)
        {
            dl_control_proc();
            dsp_comm_is_ing_set();
        }
    }
    else
    {
        // time out process
        if (dl_is_connected() && comminact_is_timeout())
        {
            comm_inact_timeset(T120SEC);
            DPRINTF(DBG_TRACE, "%s: comm_inact is timeout\r\n",
                    __func__);  // snrm 시작 후, 2분 동안 패킷이 수신 되지 않음.
        }
    }
}

void dl_fill_LLC_header(uint8_t* lptr)
{
    // LLC header encapsulation
    lptr[0] = 0xe6;  // destination LSAP
    lptr[1] = 0xe7;  // source LSAP(means response)
    lptr[2] = 0x00;  // LLC_Quality
}

uint16_t dl_server_macaddr(void)
{
    uint16_t t16;

    if (appl_is_sap_sec_site() || appl_is_sap_sec_utility())
    {
        t16 = SVR_ADDR_SEC_U >> 1;
    }
    else
    {
        t16 = SVR_ADDR_U >> 1;
    }

    return t16;
}

int8_t dl_client_macaddr(void) { return (int8_t)(dl_conn_client_addr >> 1); }

void dl_server_macaddr_set(void)
{
    uint8_t serial_num[sizeof(ser_no_type)];

    get_cust_id(serial_num);

    dl_meter_addr = (serial_num[SERIAL_NO_SIZE - 2] - '0') * 10;
    dl_meter_addr += (serial_num[SERIAL_NO_SIZE - 1] - '0');
    dl_meter_addr += 0x10;

    DPRINTF(DBG_TRACE, "DLMS Server Address Set: 0x%02X(%02d)\r\n",
            dl_meter_addr << 1 | 0x01, dl_meter_addr - 0x10);
}

static void dl_var_init(void)
{
    dl_seg_frame_over = false;  // in case of CTT, overflow is occured but
                                // voerflowed data is no problem
    dl_seg_prev = false;

    dl_seg_info_init();
}

static void dl_stop(bool reason, bool cmdsnrm)
{
    prod_log_in_sts = 0x5a;
    dl_conn_state = DL_NDM_STATE;
    memset(&gst_dl_tx_hdlc_ext, 0x00, sizeof(ST_DL_HDLC_EXT));
    memset(&gst_dl_rx_hdlc_ext, 0x00, sizeof(ST_DL_HDLC_EXT));
    appl_disc_ind(reason, cmdsnrm);
}

void dl_stop_force(void)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    dl_stop(false, false);
}

#if 0
static bool prod_dl_frame_parse(void)
{
    dl_seg_state = SEG_NONE;
    dl_seg_prev = 0;

    prod_dl_control_hi = cli_buff[2] & 0xc0;
    dl_control = cli_buff[3];

    // poll or final
    dl_poll_final = 0;

    return true;
}
#endif

static bool dl_frame_parse(void)
{
    bool seg;

    // check frame type
    if ((frm_type_seg_field & 0xf0) != 0xa0)
        return false;

    // segment bit
    seg = (frm_type_seg_field & FRM_SEG_BIT) ? true : false;
    set_seg_status(seg);

    // control
    dl_control = (uint8_t)(frm_control_field & ~POLL_FINAL_BIT);
    // server and client address
    if (!dl_addr_parse())
        return false;

    // poll or final
    dl_poll_final = (frm_control_field & POLL_FINAL_BIT) ? true : false;

    return true;
}

static void set_seg_status(bool seg)
{
    if (!seg)
    {
        if (!dl_seg_prev)
            dl_seg_state = SEG_NONE;
        else
            dl_seg_state = SEG_LAST;
    }
    else
    {
        if (!dl_seg_prev)
            dl_seg_state = SEG_FIRST;
        else
            dl_seg_state = SEG_MID;
    }

    dl_seg_prev = seg;
}

static bool dl_addr_parse(void)
{
    // server address
    if (frm_svraddrU_field & 0x01)  // 2 byte length
        return false;

    if (frm_svraddrU_field != SVR_ADDR_U &&
        frm_svraddrU_field != SVR_ADDR_SEC_U &&
        frm_svraddrU_field != SVR_ADDR_BROADCASTING_U &&
        DL_DISC != dl_control && DL_SNRM != dl_control)
        return false;

    if ((frm_svraddrL_field & 0x01) == 0)  // 2 byte length
        return false;

    if (frm_svraddrL_field != SVR_ADDR_L &&
        frm_svraddrL_field != SVR_ADDR_BROADCASTING_L)
        return false;

    // client address
    if ((frm_clientaddr_field & 0x01) == 0)  // 1 byte length
        return false;

    if ((DL_DISC != dl_control) && (DL_SNRM != dl_control) &&
        (dl_conn_state == DL_NRM_STATE))  // connection state
        return (bool)(frm_clientaddr_field == dl_conn_client_addr);

    dl_conn_client_addr = frm_clientaddr_field;

    return true;
}

#if 0
static void prod_dl_control_proc(void)
{
    if ((dl_control < PROD_CTRL_LOG_IN) || (dl_control > prod_cmd_num) ||
        (prod_dl_control_hi == 0xc0))
    {
        prod_dl_send_error(PROD_NOT_SUP_CMD);
        return;
    }

    prod_dl_ctrl_in_ndm_state();
}
#endif

static void dl_control_proc(void)
{
    if (!dl_control_check())
    {
        dl_send_FRMR();
        return;
    }

    if (dl_conn_state == DL_NDM_STATE)
    {
        dl_ctrl_in_ndm_state();
    }
    else if (dl_conn_state == DL_NRM_STATE)
    {
        dl_ctrl_in_nrm_state();
    }
    else
    {
        // error => not process
    }
}

static bool dl_control_check(void)
{
    uint8_t cmd;

    cmd = dl_control;

    if (!(cmd & NUM_INF_BIT))
        return true;  // numbered information

    if (cmd == DL_SNRM || cmd == DL_DISC || cmd == DL_UA || cmd == DL_DM ||
        cmd == DL_FRMR || cmd == DL_UI)
    {
        return true;
    }

    cmd &= 0x0f;
    if (cmd == DL_RR || cmd == DL_RNR)
        return true;

    return false;
}

static uint8_t* dl_get_parm_val(uint8_t* cp, U8_16_32* val)
{
    val->l = 0L;

    switch (*cp)
    {
    case 1:
        val->c[LO_LO] = *(cp + 1);
        cp += 2;
        break;
    case 2:
        val->c[LO_HI] = *(cp + 1);
        val->c[LO_LO] = *(cp + 2);
        cp += 3;
        break;
    case 3:
        val->c[HI_LO] = *(cp + 1);
        val->c[LO_HI] = *(cp + 2);
        val->c[LO_LO] = *(cp + 3);
        cp += 4;
        break;
    case 4:
        val->c[HI_HI] = *(cp + 1);
        val->c[HI_LO] = *(cp + 2);
        val->c[LO_HI] = *(cp + 3);
        val->c[LO_LO] = *(cp + 4);
        cp += 5;
        break;
    }

    return cp;
}

// get negotiated HDLC parameter value from the proposed parameter
// fill up nego_param
static bool dl_nego_hdlc_param(void)
{
    bool rslt;
    U8_16_32 parm_val;
    uint8_t info_len, parm_id;
    uint8_t *parm, *frm_end, *parm_end;

#if 0 /* bccho, 2024-04-03 */
    // default parm	(in case of non-nego)
    if (dl_conn_client_addr == CLIENT_ADDR_ASSO3_UTILITY ||
        dl_conn_client_addr == CLIENT_ADDR_ASSO3_SITE)
#endif
    {
        dl_tx_max_info = dl_rx_max_info =
            MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO;
    }
#if 0 /* bccho, 2024-04-03 */
    else
    {
        dl_tx_max_info = dl_rx_max_info =
            MIN_INFORMATION_FIELD_LENGTH_FOR_KEPCO;
    }
#endif
    //    DPRINTF(DBG_TRACE, _D"%s: tx_max[%d], rx_max[%d]\r\n", __func__,
    //    dl_tx_max_info, dl_rx_max_info);

    if (frm_length_field <= (DL_HEADER_LEN + 2))
    {
        return true;  // not negotiate -> next step
    }

    rslt = true;

    info_len = frm_length_field - (DL_HEADER_LEN + 2);  // drop fcs
    frm_end = &cli_buff[frm_length_field - 2];          // drop fcs

    parm = &cli_buff[DL_HEADER_LEN];

    // format id(1), group id(1), param id(1), param len(1), param
    // val(1),.......
    if (info_len >= 6 && info_len <= 0x7f)  // considering non-negative
    {
        if (*parm == 0x81 &&
            *(parm + 1) == 0x80)  // format id(0x81), group id(0x80),
        {
            parm_end = parm + *(parm + 2);  // parm_end = pointer + group len
            parm += 3;

            if (parm_end <= frm_end)
            {
                // param val(1), param len(1), param val, .....
                while (parm <= parm_end)
                {
                    parm_id = *parm++;  // param id
                    parm = dl_get_parm_val(parm, &parm_val);

                    switch (parm_id)
                    {
                    case TX_MAX_INFORMATION_FIELD:
                    {
                        DPRINTF(DBG_TRACE, _D "CLIENT_TX_MAX_INFO[%d]\r\n",
                                parm_val.l);
                        if (parm_val.l > MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO)
                        {
                            dl_rx_max_info =
                                MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO;
                        }
                        else if (parm_val.l <
                                 MIN_INFORMATION_FIELD_LENGTH_FOR_KEPCO)
                        {
                            dl_rx_max_info =
                                MIN_INFORMATION_FIELD_LENGTH_FOR_KEPCO;
                        }
                        else
                        {
                            dl_rx_max_info = parm_val.l;
                        }
                        DPRINTF(DBG_TRACE, _D "SET_RX_MAX_INFO[%d]\r\n",
                                dl_rx_max_info);
                    }
                    break;

                    case RX_MAX_INFORMATION_FIELD:
                    {
                        DPRINTF(DBG_TRACE, _D "CLIENT_RX_MAX_INFO[%d]\r\n",
                                parm_val.l);
                        if (parm_val.l > MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO)
                        {
                            dl_tx_max_info =
                                MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO;
                        }
                        /*
                        else if (parm_val.l <
                        MIN_INFORMATION_FIELD_LENGTH_FOR_KEPCO)
                        {
                            dl_rx_max_info =
                        MIN_INFORMATION_FIELD_LENGTH_FOR_KEPCO;
                        }
                        */
                        else
                        {
                            dl_tx_max_info = parm_val.l;
                        }
                        DPRINTF(DBG_TRACE, _D "SET_TX_MAX_INFO[%d]\r\n",
                                dl_tx_max_info);
                    }
                    break;
                    case 0x07:  // max tx window size (default_negoed = 1)
                    case 0x08:  // max rcv window size (default_negoed = 1)
                        break;
                    default:  // unknown param
                        rslt = false;
                        break;
                    }
                }
            }
            else
            {
                rslt = false;
            }
        }
        else
        {
            rslt = false;
        }
    }
    else
    {
        rslt = false;
    }
    DPRINTF(DBG_TRACE, _D "%s: TX_MAX_INFO[%d], RX_MAX_INFO[%d]\r\n", __func__,
            dl_tx_max_info, dl_rx_max_info);
    return rslt;
}

#if 0
static void prod_dl_snrm_frame_proc(void)
{
    dl_var_init();

    dl_seq_Rto = 0x00;  // expect S_from = 0
    dl_seq_Sto = 0x00;  // initialization

    prod_dl_send_SNRM_UA();

    dl_conn_state = DL_NRM_STATE;  // DL connection !!!
    prod_log_in_sts = 0xa5;

    // transfer to application layer
    // appl_conn_ind(dl_conn_client_addr);
}
#endif

static void dl_snrm_frame_proc(void)
{
    dl_var_init();

    dl_seq_Rto = 0x00;  // expect S_from = 0
    dl_seq_Sto = 0x00;  // initialization

    dl_send_SNRM_UA();

    dl_conn_state = DL_NRM_STATE;  // DL connection !!!

    // transfer to application layer
    appl_conn_ind(dl_conn_client_addr);
}

extern float adj_currtemp;

#if 0
void prod_curr_temp_write(S8 write_temp)
{
    ST_MTP_PUSH_DATA *pushd = dsm_mtp_get_push_data();

    temp_adj_data_type *temp_adj;

    temp_adj = (temp_adj_data_type *)appl_tbuff;

    adj_currtemp = ((float)write_temp - pushd->temp);
    temp_adj->T_adj_temp = adj_currtemp;
    nv_write(I_ADJ_TEMP_DATA, (U8 *)temp_adj);
}
#endif

void prod_control_cal_begin(int idx)
{
    ST_MTP_CAL_POINT st_mtp_cal_point;
    float fval;
    int16_t val16;
    bool error = false;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    DPRINTF(DBG_INFO, "\r\ncal start\r\n");

    if (!nv_read(I_MTP_CAL_POINT,
                 (uint8_t*)&st_mtp_cal_point))  // 우선 최근 사용값 사용
    // if(1)
    {
        dsm_mtp_default_cal_point(&st_mtp_cal_point);  // error 이면 default
                                                       // 설정
    }

    ////JP.KIM 24.11.08	1) 생산프로그램 PROTOCOL을 dlms 방식으로 변경
    idx += 35;
    idx += 2;  // array field skip
    idx += 2;  // struct field skip
    DPRINT_HEX(DBG_TRACE, "appl_msg_data", (uint8_t*)&appl_msg[idx], 30,
               DUMP_ALWAYS);
    ToHFloat((U8_Float*)&fval, (uint8_t*)&appl_msg[idx]);
    ToCommFloat(&st_mtp_cal_point.val.ref_current[0], (U8_Float*)&fval);
    DPRINTF(DBG_TRACE, "current: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    idx += 2;
    idx += 3;

    if ((fval < 2.0) || (fval > 20.0))
        error = true;

    ToHFloat((U8_Float*)&fval, (uint8_t*)&appl_msg[idx]);
    ToCommFloat(&st_mtp_cal_point.val.ref_voltage[0], (U8_Float*)&fval);
    DPRINTF(DBG_TRACE, "voltage: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    idx += 2;
    idx += 3;

    if ((fval < 100.0) || (fval > 300.0))
        error = true;

    ToHFloat((U8_Float*)&fval, (uint8_t*)&appl_msg[idx]);
    idx += 3;

    ToCommFloat(&st_mtp_cal_point.val.ref_phase[0], (U8_Float*)&fval);
    DPRINTF(DBG_TRACE, "phase: %d.%03d\r\n", (uint32_t)(fval),
            (uint32_t)((fval - (uint32_t)(fval)) * 1000));
    idx += 2;

    if ((fval > 90.0))
        error = true;

    st_mtp_cal_point.val.process_time = appl_msg[idx];
    idx++;

    DPRINTF(DBG_TRACE, "process_time: 0x%02X\r\n",
            st_mtp_cal_point.val.process_time);
    idx++;
    if ((st_mtp_cal_point.val.process_time < 2) ||
        (st_mtp_cal_point.val.process_time > 20))
        error = true;

    ToH16((U8_16*)&val16, &appl_msg[idx]);
    fval = ((float)val16) / 10.0;
    idx++;

    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();
    temp_adj_data_type* temp_adj;
    // float fval;
    temp_adj = (temp_adj_data_type*)appl_tbuff;

    if ((fval < -20.0) || (fval > 100.0))
    {
        error = true;
    }
    else
    {
        adj_currtemp = (fval - pushd->temp);
        temp_adj->T_adj_temp = adj_currtemp;
        nv_write(I_ADJ_TEMP_DATA, (U8*)temp_adj);
    }

    if (error)
    {
        dsm_mtp_default_cal_point(
            &st_mtp_cal_point);  // 설정값 error이면 default값 사용
    }
    else
    {
        nv_write(I_MTP_CAL_POINT,
                 (uint8_t*)&st_mtp_cal_point);  // 정상이면 비휘발 메모리 저장
    }

    memcpy((uint8_t*)&g_mtp_cal_start, (uint8_t*)&st_mtp_cal_point.val,
           sizeof(g_mtp_cal_start));

    MSG09("prod_control_cal_begin");
    cal_begin();
    dsm_mtp_set_op_mode(MTP_OP_NORMAL);
    dsm_mtp_set_fsm(MTP_FSM_CAL_ST);
    dsm_mtp_fsm_send();
}

bool factory_addtional_reset(void)
{
    // U8 i =0;
    // ST_FW_INFO fwinfo = {0};
    fw_info_t fw_info_bank1;
    fw_info_t fw_info_bank2;
    uint32_t crc_val;
    uint32_t sf_addr_dest;
    bool error = 0;

    // flash FW_INFO SF_1  erase
    sf_addr_dest = dsm_sflash_fw_get_startaddr(E_SFLASH_SYS_FW_1_T);

    CMD_SE(sf_addr_dest);

    DPRINTF(DBG_TRACE, "FW Info (SF_1)\r\n ");
    crc_val = fw_info_get(1, &fw_info_bank1);
    fw_info_print(&fw_info_bank1, crc_val);

    // flash FW_INFO SF_2  erase
    sf_addr_dest = dsm_sflash_fw_get_startaddr(E_SFLASH_SYS_FW_2_T);

    CMD_SE(sf_addr_dest);

    DPRINTF(DBG_TRACE, "FW Info (SF_2)\r\n ");
    crc_val = fw_info_get(2, &fw_info_bank2);
    fw_info_print(&fw_info_bank2, crc_val);

    // sys fw_info clear
    error = dsm_sys_fwinfo_initial_set(true);  // external flash info

    return error;
}

#if 0
static void prod_dl_ctrl_in_ndm_state(void)
{
    U8 idx = 0, rslt;
#if defined(FEATURE_COMPILE_DM)
    DPRINTF(DBG_TRACE, _D "%s %d, %d\r\n", __func__, dl_is_snrm_frame(),
            dl_is_disc_frame());
#endif
#ifdef SUPPORT_INFORMATION_FIELD_LENGTH
    dl_tx_max_info = dl_rx_max_info = MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO;
#else
    dl_tx_max_info = MAX_HDLC_TX_ASSO3;
    dl_rx_max_info = MAX_HDLC_RX_ASSO3;
#endif

    if ((prod_dl_control_hi == 0x80) &&
        (dl_control == PROD_CTRL_LOG_IN))  // log_in
    {
        rslt = memcmp((uint8_t *)&priv_pwd_default[0], (U8 *)&cli_buff[4],
                      PRIV_PWD_SIZE);
        if (rslt == 0)
        {
            prod_dl_snrm_frame_proc();  // connection
        }
        else
        {
            prod_dl_send_error(PROD_NOT_ACC_USER);  // ERROR PASSWORD
        }
    }
    else if ((prod_dl_control_hi == 0x80) &&
             (dl_control == PROD_CTRL_LOG_OUT))  // log_out
    {
        prod_dl_send_DM();
        dl_stop(false, false);
    }
    else if (dl_is_connected())
    {
        if ((prod_dl_control_hi == 0x80) &&
            (dl_control == PROD_CTRL_FACTORY_RESET))
        {
            whm_clear_all(true);
            prod_dl_frame_tx((U8 *)0, 0);
        }
        else if ((prod_dl_control_hi == 0x80) &&
                 (dl_control == PROD_CTRL_CAL_BEGIN))
        {
            prod_control_cal_begin(0);
        }
        else if ((dl_control == PROD_CTRL_FACTOR))
        {
            if (prod_dl_control_hi == 0x00)  // read
            {
                // only_data
                idx = 0;
                cal_data_type cal;
                nv_read(I_CAL_DATA, (U8 *)&cal);
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
#if 1
                ToComm32(&global_buff[idx], (U8_16_32 *)&cal.T_cal_v0);
                idx += 4;
                ToComm32(&global_buff[idx], (U8_16_32 *)&cal.T_cal_v1);
                idx += 4;
                ToComm32(&global_buff[idx], (U8_16_32 *)&cal.T_cal_v2);
                idx += 4;

                ToComm32(&global_buff[idx], (U8_16_32 *)&cal.T_cal_i0);
                idx += 4;
                ToComm32(&global_buff[idx], (U8_16_32 *)&cal.T_cal_i1);
                idx += 4;
                ToComm32(&global_buff[idx], (U8_16_32 *)&cal.T_cal_i2);
                idx += 4;

                ToComm32(&global_buff[idx], (U8_16_32 *)&cal.T_cal_p0);
                idx += 4;
                ToComm32(&global_buff[idx], (U8_16_32 *)&cal.T_cal_p1);
                idx += 4;
                ToComm32(&global_buff[idx], (U8_16_32 *)&cal.T_cal_p2);
                idx += 4;
#else
                memcpy((U8 *)&global_buff[idx], (U8 *)&cal.T_cal_v0, 4);
                idx += 4;
                memcpy((U8 *)&global_buff[idx], (U8 *)&cal.T_cal_v1, 4);
                idx += 4;
                memcpy((U8 *)&global_buff[idx], (U8 *)&cal.T_cal_v2, 4);
                idx += 4;
                memcpy((U8 *)&global_buff[idx], (U8 *)&cal.T_cal_i0, 4);
                idx += 4;
                memcpy((U8 *)&global_buff[idx], (U8 *)&cal.T_cal_i1, 4);
                idx += 4;
                memcpy((U8 *)&global_buff[idx], (U8 *)&cal.T_cal_i2, 4);
                idx += 4;

                memcpy((U8 *)&global_buff[idx], (U8 *)&cal.T_cal_p0, 4);
                idx += 4;
                memcpy((U8 *)&global_buff[idx], (U8 *)&cal.T_cal_p1, 4);
                idx += 4;
                memcpy((U8 *)&global_buff[idx], (U8 *)&cal.T_cal_p2, 4);
                idx += 4;

#endif
                prod_dl_frame_tx((uint8_t *)&global_buff, (4 * 3 * 3));
            }
        }

        else if ((dl_control == PROD_CTRL_SERIAL_NO))
        {
            /* prod set custid 30 30 30 30 31 30 35 --> "0000105" <- serial
             * number */
            ser_no_type serno;

            if (prod_dl_control_hi == 0x40)  // write
            {
                memset((uint8_t *)&serno, 0x00, sizeof(ser_no_type));

#if 1 /* bccho, 2024-05-17 */
                extern uint8_t SYS_TITLE_server[SYS_TITLE_LEN];
                uint32_t ds_serial_no_32;

                ds_serial_no_32 = SYS_TITLE_server[5];
                ds_serial_no_32 <<= 8;
                ds_serial_no_32 += SYS_TITLE_server[6];
                ds_serial_no_32 <<= 8;
                ds_serial_no_32 += SYS_TITLE_server[7];

                serno.ser[0] = ds_serial_no_32 / 1000000;
                ds_serial_no_32 %= 1000000;
                serno.ser[1] = ds_serial_no_32 / 100000;
                ds_serial_no_32 %= 100000;
                serno.ser[2] = ds_serial_no_32 / 10000;
                ds_serial_no_32 %= 10000;
                serno.ser[3] = ds_serial_no_32 / 1000;
                ds_serial_no_32 %= 1000;
                serno.ser[4] = ds_serial_no_32 / 100;
                ds_serial_no_32 %= 100;
                serno.ser[5] = ds_serial_no_32 / 10;
                ds_serial_no_32 %= 10;
                serno.ser[6] = ds_serial_no_32;
#else
                memcpy((uint8_t *)&serno, (uint8_t *)&cli_buff[4],
                       SERIAL_NO_SIZE);
#endif

                for (idx = 0; idx < SERIAL_NO_SIZE; idx++)
                {
                    serno.ser[idx] = byte_to_ascii(serno.ser[idx]);
                }

                set_cust_id(&serno);

                DPRINT_HEX(DBG_TRACE, "METER_SERIAL", serno.ser, SERIAL_NO_SIZE,
                           DUMP_ALWAYS);

                /* software information */
                dsm_sys_fwinfo_initial_set(true);  // external flash info

                prod_dl_frame_tx((U8 *)0, 0);
            }
            else if (prod_dl_control_hi == 0x00)  // read
            {
                // only_data
                get_cust_id((uint8_t *)&serno);
                DPRINT_HEX(DBG_TRACE, "METER_SERIAL", serno.ser, SERIAL_NO_SIZE,
                           DUMP_ALWAYS);
#if 0
				if(idx =0; idx < SERIAL_NO_SIZE; idx++)
					{
					pPdu[idx] = put_ascii_byte(serno[idx]);
					}			
				prod_dl_frame_tx((uint8_t*)&pPdu, SERIAL_NO_SIZE);
#else
                prod_dl_frame_tx((uint8_t *)&serno, SERIAL_NO_SIZE);
#endif
            }
        }

        else if ((dl_control == PROD_CTRL_DEVICE_NAME))
        {
            /*
                                   y1 y2 m1 m2 d1 d2 A  V1 V2
                prod set devid 32 30 30 39 30 38 41 33 30 --> "200908" 'A' "30"
            */
            /* Ref: whm.c, logical_device_name_r_kepco[], 제조 일자 +
             * 제조관리번호 + 규격 버전, (ex) "220101A30" */
            /* 통신 규격 - 2.3.2 논리적 장치명의 구조 (LDN: Logical Device Name)
             * 및 3.4.2.3.1 COSEM 계기 식별자 참조 */
            device_id_type dev;
            if (prod_dl_control_hi == 0x40)  // write
            {
                /* 제조사 고유코드 */
                dev.devid[0] = FLAG_ID1;
                dev.devid[1] = FLAG_ID2;
                dev.devid[2] = FLAG_ID3;

                dev.devid[3] = ' ';
#if 0
                if(idx =4; idx < DEVICE_ID_SIZE; idx++)
                {
                    dev.devid[idx] = get_byte_of_hex_from_ascii(cli_buff[idx+4]);
				}
#else
                idx = 4;
                memcpy((uint8_t *)&dev.devid[4], (uint8_t *)&cli_buff[idx + 4],
                       12);
#endif
                dev.devid[11] = ' ';
                dev.devid[12] = ' ';

                dev.devid[14] = '3';
#if 1 /* bccho, 2024-09-05, 삼상 */
                dev.devid[15] = logical_device_name_r[15];
#else
                dev.devid[15] = '0';
#endif

                /* LD(Logical Device) 번호 : 장치 관리용 = 1, 한전 관리용 = 2 */
                dev.devid[13] = '2';  // 0x31: DEVICE_ID, 0x32: DEVICE_ID_KEPCO

                nv_write(I_DEVICE_ID_KEPCO, (uint8_t *)&dev);  // 한전 관리용
                DPRINT_HEX(DBG_TRACE, "KEPCO_MGMT", dev.devid, DEVICE_ID_SIZE,
                           DUMP_ALWAYS);

                dev.devid[13] = '1';
                nv_write(I_DEVICE_ID, (uint8_t *)&dev);  // 장치 관리용
                DPRINT_HEX(DBG_TRACE, "DEVICE_MGMT", dev.devid, DEVICE_ID_SIZE,
                           DUMP_ALWAYS);

                prod_dl_frame_tx((U8 *)0, 0);
            }
            else if (prod_dl_control_hi == 0x00)  // read
            {
                // only_data
                nv_read(I_DEVICE_ID_KEPCO, (uint8_t *)&dev);
                DPRINT_HEX(DBG_TRACE, "KEPCO_MGMT", dev.devid, DEVICE_ID_SIZE,
                           DUMP_ALWAYS);
#if 0
				if(idx =0; idx < DEVICE_ID_SIZE; idx++)
                {
                pPdu[idx] = put_ascii_byte(dev.devid[idx]);
                }			
				prod_dl_frame_tx((uint8_t*)&pPdu, DEVICE_ID_SIZE);
#else
                prod_dl_frame_tx((uint8_t *)&dev.devid, DEVICE_ID_SIZE);
#endif
            }
        }
        else if ((dl_control == PROD_CTRL_CURR_TEMP))
        {
            float temp;
            if (prod_dl_control_hi == 0x40)  // write
            {
#if 1 /* bccho, 2024-06-07 패치 포팅 */
                prod_curr_temp_write(cli_buff[4]);
#endif
                prod_dl_frame_tx((U8 *)0, 0);
            }
            else if (prod_dl_control_hi == 0x00)  // read
            {
                // only_data
                temp = get_inst_temp();
                idx = 0;
                pPdu[idx++] = (int8_t)temp;
                prod_dl_frame_tx(&pPdu[0], idx);
            }
        }
        else
        {
            prod_dl_send_error(PROD_NOT_ACC_USER);  // not log_in
        }
    }
    else
    {
        prod_dl_send_error(PROD_NOT_ACC_USER);  // not log_in
    }
}
#endif

static void dl_ctrl_in_ndm_state(void)
{
    DPRINTF(DBG_TRACE, _D "%s %d, %d\r\n", __func__, dl_is_snrm_frame(),
            dl_is_disc_frame());
    if (dl_is_snrm_frame())
    {
        if (dl_nego_hdlc_param())
        {
            if (dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_RS485)
            {
                // 유선 통신으로 SNRM 수신될 경우, 내장 모뎀을 수신 모드로 설정.
                dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);

                // SNRM 수신 -> 내장 모뎀을 수신 모드로
                // 변경, "AT+LISTEN:1\r" 수신 모드
                dsm_atcmd_set_listen(TRUE, AT_LISTEN_ON);
            }

            dl_snrm_frame_proc();
        }
        else
        {
            dl_send_DM();
        }
    }
    else if (dl_is_disc_frame())
    {
#if 1  // jp.kim 25.07.03 // 유선 통신으로 disc 수신될 경우, 내장 모뎀을 수신
       // 모드로 설정.
        if (dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_RS485)
        {
            // 유선 통신으로 disc 수신될 경우, 내장 모뎀을 수신 모드로 설정.
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
            dsm_atcmd_set_listen(
                TRUE, AT_LISTEN_ON);  // SNRM 수신 -> 내장 모뎀을 수신 모드로
                                      // 변경, "AT+LISTEN:1\r" 수신 모드
        }
#endif

        dl_send_DM();
    }
    /*************************************/
    // dsm_push_push_setup_table_default();
    /*************************************/
}

void dsm_dl_seg_frame_offs_subtract(void)
{
    if (dl_seg_frame_offs > 0)
        dl_seg_frame_offs -= 1;
}

static void dl_ctrl_in_nrm_state(void)
{
    if (dl_is_info_frame())
    {
        // frame sequence parse and check
        dl_seq_Sfrom = (dl_control >> 1) & 0x07;  // send seq
        dl_seq_Rfrom = (dl_control >> 5) & 0x07;  // receive seq
        if (dl_seq_Sfrom != dl_seq_Rto)
        {
            if (dl_seq_Sfrom != ((dl_seq_Rto - 1) & 0x07))
            {
                DPRINTF(DBG_ERR, "%s: 1\r\n", __func__);
                dl_seq_Sfrom =
                    (dl_seq_Rto - 1) &
                    0x07;  // refer to dl_seq_Sfrom when RR message is sent
                dl_send_RR();
            }
            else
            {
                dl_frame_tx_for_hdlc_dup();
            }
        }
        else if (dl_seq_Rfrom != dl_seq_Sto)
        {
            if (dl_seq_Rfrom != ((dl_seq_Sto - 1) & 0x07))
            {
                dl_send_FRMR();
            }
            else
            {
                dl_frame_tx_for_hdlc_dup();
            }
        }
        else
        {
            uint16_t hdlc_len, hdlc_total = 0;

            hdlc_len = dl_appl_get_hdlc_length();

            gst_dl_rx_hdlc_ext.data[hdlc_total++] = CHAR_FLAG;
            memcpy(&gst_dl_rx_hdlc_ext.data[hdlc_total], &frm_type_seg_field,
                   hdlc_len);
            hdlc_total += hdlc_len;
            gst_dl_rx_hdlc_ext.data[hdlc_total++] = CHAR_FLAG;
            gst_dl_rx_hdlc_ext.len = hdlc_total;
            gst_dl_rx_hdlc_ext.type = I_FRAME;

            // sequence ok => LLC header check => application msg process
            dl_appl_msg_proc();
        }
    }
    else if (dl_is_disc_frame())
    {
#if 1  // jp.kim 25.07.03 // 유선 통신으로 disc 수신될 경우, 내장 모뎀을 수신
       // 모드로 설정.
        if (dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_RS485)
        {
            // 유선 통신으로 disc 수신될 경우, 내장 모뎀을 수신 모드로 설정.
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
            dsm_atcmd_set_listen(
                TRUE, AT_LISTEN_ON);  // SNRM 수신 -> 내장 모뎀을 수신 모드로
                                      // 변경, "AT+LISTEN:1\r" 수신 모드
        }
#endif
        dl_send_UA();
        dl_stop(true, false);
        dsm_pmnt_disc_o_at_NO_VOLT_SET_OP();
        dsm_push_disable();
    }
    else if (dl_is_rr_frame())
    {
        dl_seq_Rfrom = (dl_control >> 5) & 0x07;

        if (dl_seq_Rfrom == dl_seq_Sto)
        {
            uint16_t hdlc_len, hdlc_total = 0;

            hdlc_len = dl_appl_get_hdlc_length();

            gst_dl_rx_hdlc_ext.data[hdlc_total++] = CHAR_FLAG;
            memcpy(&gst_dl_rx_hdlc_ext.data[1], &frm_type_seg_field, hdlc_len);
            hdlc_total += hdlc_len;
            gst_dl_rx_hdlc_ext.data[hdlc_total++] = CHAR_FLAG;
            gst_dl_rx_hdlc_ext.len = hdlc_total;
            gst_dl_rx_hdlc_ext.type = RR_FRAME;

            dl_RR_msg_proc();
        }
        else if (dl_seq_Rfrom == ((dl_seq_Sto - 1) & 0x07))
        {
            dl_frame_tx_for_hdlc_dup();
        }
        else
        {
            dl_send_FRMR();
        }
    }
    else if (dl_is_snrm_frame())
    {
        if (dl_nego_hdlc_param())
        {
            dl_stop(true, true);
            dl_snrm_frame_proc();
        }
        else
        {
            dl_send_DM();
            dl_stop(false, false);
        }
    }
}

static void dl_RR_msg_proc(void)
{
    uint16_t len;
    bool seg;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);

    if ((dl_seg_frame_len != 0) && (dl_seg_frame_len > dl_seg_frame_offs))
    {
        // if((dl_seg_frame_len - dl_seg_frame_offs) > dl_tx_max_info)
        len = dl_seg_frame_len - dl_seg_frame_offs;
        if (len > dl_tx_max_info)
        {
            len = dl_tx_max_info;
            seg = true;
        }
        else
        {
            // len = dl_seg_frame_len - dl_seg_frame_offs;
            seg = false;
        }

        if (dl_seg_frame_read(global_buff, dl_seg_frame_offs, len))
        {
            dl_send_info_frame(seg, global_buff, len);
            dl_seg_frame_offs_back = dl_seg_frame_offs;
            dl_seg_frame_offs += len;
        }
        else
        {
            // DPRINTF(DBG_WARN, "%s: 2\r\n", __func__);
            dl_send_RR();  // send which command ?? (see DLMS test
                           // case 11.4.3.1)
        }
    }
    else
    {
        dl_seq_Sfrom = 0xFF;  // used in send_RR()
        dl_send_RR();
    }
}

uint16_t dl_appl_get_hdlc_length(void)
{
    uint16_t len;

    len = (uint16_t)((frm_type_seg_field & 0x7) << 8);
    len |= (uint16_t)frm_length_field;

    DPRINTF(DBG_TRACE, "%s: hdlc_len %d\r\n", __func__, len);

    return len;
}

static void dl_appl_msg_proc(void)
{
    int len;
    uint16_t hdlc_len;

    // LLC header check
    if ((dl_seg_state == SEG_NONE) || (dl_seg_state == SEG_FIRST))
    {
        if (!dl_LLC_header_chk())
        {
            return;
        }
    }

    hdlc_len = dl_appl_get_hdlc_length();

    switch (dl_seg_state)
    {
    case SEG_NONE:
        len = hdlc_len - DL_HEADER_LEN_LLC_FCS;
        memcpy(global_buff, &frm_appl_field_llc, len);
        appl_msg_data_ind(global_buff, len);  // appl msg data
        break;
    case SEG_FIRST:
        len = hdlc_len - DL_HEADER_LEN_LLC_FCS;
        if (dl_seg_frame_write(&frm_appl_field_llc, 0, len))
        {
            dl_seg_frame_offs_back = dl_seg_frame_offs;
            dl_seg_frame_offs = len;
        }
        else
        {
            dl_seg_frame_offs_back = 0;
            dl_seg_frame_offs = 0;
        }
        dl_send_RR();
        break;
    case SEG_MID:
        len = hdlc_len - DL_HEADER_LEN_NOLLC_FCS;
        if (dl_seg_frame_write(&frm_appl_field_nollc, dl_seg_frame_offs, len))
        {
            dl_seg_frame_offs_back = dl_seg_frame_offs;
            dl_seg_frame_offs += len;
        }
        dl_send_RR();
        break;
    case SEG_LAST:
        if (dl_seg_frame_read(global_buff, 0, dl_seg_frame_offs))
        {
            len = hdlc_len - DL_HEADER_LEN_NOLLC_FCS;
            if ((dl_seg_frame_offs + len) <= GLOBAL_BUFF_SIZE)
            {
                memcpy(&global_buff[dl_seg_frame_offs], &frm_appl_field_nollc,
                       len);
                dl_seg_frame_offs_back = dl_seg_frame_offs;
                dl_seg_frame_offs += len;
            }

            appl_msg_data_ind(global_buff, dl_seg_frame_offs);
        }
        break;
    }
}

static bool dl_LLC_header_chk(void)
{
    if (frm_llchd0_field != 0xe6 || frm_llchd1_field != 0xe6 ||
        frm_llchd2_field != 0x00)
        return false;

    return true;
}

static uint8_t dl_make_ctrl_seq(bool p_f)
{
    uint8_t rtn;

    dl_seq_Rto = (dl_seq_Sfrom + 1) & 0x07;
    rtn = dl_seq_Rto << 5;

    rtn |= (dl_seq_Sto << 1);
    dl_seq_Sto = (dl_seq_Sto + 1) & 0x07;

    if (p_f)
        rtn |= POLL_FINAL_BIT;

    return rtn;
}

void dl_frame_tx_for_hdlc_dup(void)
{
    uint16_t hdlc_len, hdlc_total = 0;

    if (gst_dl_tx_hdlc_ext.len)
    {
        hdlc_len = dl_appl_get_hdlc_length();
        gst_dl_rx_TMP_hdlc_ext.data[hdlc_total++] = CHAR_FLAG;
        memcpy(&gst_dl_rx_TMP_hdlc_ext.data[hdlc_total], &frm_type_seg_field,
               hdlc_len);
        hdlc_total += hdlc_len;
        gst_dl_rx_TMP_hdlc_ext.data[hdlc_total++] = CHAR_FLAG;
        gst_dl_rx_TMP_hdlc_ext.len = hdlc_total;

        /* duplication check */
        if (gst_dl_rx_TMP_hdlc_ext.len == gst_dl_rx_hdlc_ext.len)
        {
            if (memcmp(gst_dl_rx_TMP_hdlc_ext.data, gst_dl_rx_hdlc_ext.data,
                       gst_dl_rx_hdlc_ext.len) == 0)
            {
                amr_set_send_frame(gst_dl_tx_hdlc_ext.data,
                                   gst_dl_tx_hdlc_ext.len, T20MS);
            }
        }

        memset(&gst_dl_rx_TMP_hdlc_ext, 0x00, sizeof(ST_DL_HDLC_EXT));
    }
}

#if 0
static void prod_dl_frame_error_tx(U8 sts)
{
    /* TODO: (WD) if this function is called to send I-Frame from
     * dl_send_info_frame(), maybe tx_buff[] is smaller then buf[] */
    /* tx_buff[1024+50] */
    int i;

    i = 0;
    tx_buff[i++] = CHAR_FLAG;
    // frame type
    tx_buff[i++] = 0xBC;
    tx_buff[i++] = 0x00;  // length -> below

    tx_buff[i++] = prod_dl_control_hi | (sts << 4);
    tx_buff[i++] = dl_control;

    i += 2;
    tx_buff[2] = i - 1;  // flag + header + fcs + flag

    fcs16(tx_buff + 1, (U16)(i - 1), true);  // Calc FCS

    tx_buff[i++] = CHAR_FLAG;

    amr_set_send_frame(tx_buff, i, T20MS);

#if defined(FEATURE_JP_COMM_BLINK)
    dsp_comm_is_ing_set();
#endif
}

static void prod_dl_frame_tx(U8 *buf, U16 len)
{
    dl_frame_tx_type frm;
    /* TODO: (WD) if this function is called to send I-Frame from
     * dl_send_info_frame(), maybe tx_buff[] is smaller then buf[] */
    /* tx_buff[1024+50] */
    int i;

    i = 0;
    tx_buff[i++] = CHAR_FLAG;
    // frame type
    tx_buff[i++] = 0xBC;
    tx_buff[i++] = 0x00;  // length -> below

    tx_buff[i++] = prod_dl_control_hi;
    tx_buff[i++] = dl_control;

    if (len > 0)
    {
        for (; len > 0; len--)
        {
            tx_buff[i++] = *buf++;
        }
    }
    i += 2;  // FCS Size
             // else
    //{
    tx_buff[2] = i - 1;  // flag + header + fcs + flag
    //}

    fcs16(tx_buff + 1, (U16)(i - 1), true);  // Calc FCS

    tx_buff[i++] = CHAR_FLAG;

#if defined(FEATURE_SPEC_2021_0430_MODIFY_HDLCERR)
    // if(frm == I_FRAME || frm == RR_FRAME)
    if (1)
    //	if(0)
    {
        gst_dl_tx_hdlc_ext.type = I_FRAME;
        gst_dl_tx_hdlc_ext.len = i;
        memcpy(gst_dl_tx_hdlc_ext.data, tx_buff,
               gst_dl_tx_hdlc_ext.len);  // TODO: (WD) gst_dl_tx_hdlc_ext.data[]
                                         // is smaller then tx_buff[]
    }
#endif
    amr_set_send_frame(tx_buff, i, T20MS);

#if defined(FEATURE_JP_COMM_BLINK)
    dsp_comm_is_ing_set();
#endif
}
#endif

static void dl_frame_tx(dl_frame_tx_type frm, bool seg, uint8_t* buf,
                        uint16_t len)
{
    int i;

    i = 0;
    tx_buff[i++] = CHAR_FLAG;
    // frame type
    tx_buff[i++] = seg ? (0xa0 | FRM_SEG_BIT) : 0xa0;
    tx_buff[i++] = 0x00;
    tx_buff[i++] = dl_conn_client_addr;

    if (dl_conn_client_addr == CLIENT_ADDR_ASSO3_UTILITY ||
        dl_conn_client_addr == CLIENT_ADDR_ASSO3_SITE)
        tx_buff[i++] = SVR_ADDR_SEC_U;
    else
        tx_buff[i++] = SVR_ADDR_U;

    tx_buff[i++] = SVR_ADDR_L;

    // control byte
    switch (frm)
    {
    case I_FRAME:
        tx_buff[i++] = dl_make_ctrl_seq(true);
        break;
    case UA_FRAME:
        tx_buff[i++] = (DL_UA | POLL_FINAL_BIT);
        break;
    case DM_FRAME:
        tx_buff[i++] = (DL_DM | POLL_FINAL_BIT);
        break;
    case RR_FRAME:
        dl_seq_Rto = (dl_seq_Sfrom + 1) & 0x07;
        tx_buff[i++] = ((dl_seq_Rto << 5) | POLL_FINAL_BIT | DL_RR);
        break;
    case FRMR_FRAME:
        tx_buff[i++] = (DL_FRMR | POLL_FINAL_BIT);
        break;
    case UI_FRAME:
        tx_buff[i++] = (DL_UI | POLL_FINAL_BIT);
        break;
    case RNR_FRAME:
    case DISC_FRAME:
    default:
        break;
    }

    i += 2;  // HCS Size

    if (len > 0)
    {
#if 0  // JP.KIM 24.10.28
        if(len > 255)
#else
        if (1)
#endif
        {
            tx_buff[1] |= ((i + len + 2 - 1) >> 8) & 0x07;
            tx_buff[2] = (i + len + 2 - 1) & 0xFF;
        }
        else
        {
            tx_buff[2] = i + len + 2 - 1;
        }
        fcs16(tx_buff + 1, (uint16_t)(i - 1), true);  // Calc HCS

        for (; len > 0; len--)
        {
            tx_buff[i++] = *buf++;
        }

        i += 2;  // FCS Size
    }
    else
    {
        tx_buff[2] = i - 1;  // flag + header + fcs + flag
    }

    fcs16(tx_buff + 1, (uint16_t)(i - 1), true);  // Calc FCS

    tx_buff[i++] = CHAR_FLAG;

    if (frm == I_FRAME || frm == RR_FRAME)
    {
        gst_dl_tx_hdlc_ext.type = frm;
        gst_dl_tx_hdlc_ext.len = i;
        memcpy(gst_dl_tx_hdlc_ext.data, tx_buff,
               gst_dl_tx_hdlc_ext.len);  // TODO: (WD) gst_dl_tx_hdlc_ext.data[]
                                         // is smaller then tx_buff[]
    }

    if (dl_poll_final)
    {
        amr_set_send_frame(tx_buff, i, T20MS);

        dsp_comm_is_ing_set();
    }
}

static void dl_send_FRMR(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    dl_frame_tx(FRMR_FRAME, false, (uint8_t*)0, 0);
}

#if 0
static void prod_dl_send_SNRM_UA(void) { prod_dl_frame_tx((U8 *)0, 0); }
#endif

static void dl_send_SNRM_UA(void)
{
    uint8_t tbuf[NEGO_PARAM_SIZE_ASSO3];

    DPRINTF(DBG_WARN, _D "%s: client_addr[0x%02X], tx_max[%d], rx_max[%d]\r\n",
            __func__, dl_conn_client_addr, dl_tx_max_info, dl_rx_max_info);

#if 0 /* bccho, 2024-04-03 */
    if (dl_conn_client_addr == CLIENT_ADDR_ASSO3_UTILITY ||
        dl_conn_client_addr == CLIENT_ADDR_ASSO3_SITE)
#endif
    {
        if (dl_tx_max_info > 255 || dl_rx_max_info > 255)
        {
            memcpy(tbuf, &nego_dl_parm_asso3[0], NEGO_PARAM_SIZE_ASSO3);

            tbuf[5] = ((dl_tx_max_info >> 8) & 0xff);
            tbuf[6] = (dl_tx_max_info & 0xff);
            tbuf[9] = ((dl_rx_max_info >> 8) & 0xff);
            tbuf[10] = (dl_rx_max_info & 0xff);

            dl_frame_tx(UA_FRAME, false, tbuf, NEGO_PARAM_SIZE_ASSO3);
        }
        else
        {
            memcpy(tbuf, &nego_dl_parm[0], NEGO_PARAM_SIZE);

            tbuf[5] = dl_tx_max_info;
            tbuf[8] = dl_rx_max_info;

            dl_frame_tx(UA_FRAME, false, tbuf, NEGO_PARAM_SIZE);
        }
    }
#if 0 /* bccho, 2024-04-03 */
    else
    {
        memcpy(tbuf, &nego_dl_parm[0], NEGO_PARAM_SIZE);

        tbuf[5] = dl_tx_max_info;
        tbuf[8] = dl_rx_max_info;

        dl_frame_tx(UA_FRAME, false, tbuf, NEGO_PARAM_SIZE);
    }
#endif
}

#if 0
static void prod_dl_send_DM(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    prod_dl_frame_tx((U8 *)0, 0);
}

static void prod_dl_send_error(U8 sts)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    prod_dl_frame_error_tx(sts);
}
#endif

static void dl_send_DM(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    dl_frame_tx(DM_FRAME, false, (uint8_t*)0, 0);
}

static void dl_send_RR(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    dl_frame_tx(RR_FRAME, false, (uint8_t*)0, 0);
}

static void dl_send_UA(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    dl_frame_tx(UA_FRAME, false, (uint8_t*)0, 0);
}

static void dl_send_info_frame(bool seg, uint8_t* buf, uint16_t len)
{
    /* buf == pPdu, global_buff[1024+100] */
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    dl_frame_tx(I_FRAME, seg, buf, len);
}

void dl_send_appl_msg(uint8_t* buf, int len)
{
    DPRINTF(DBG_TRACE, _D "%s: len[%d], dl_tx_max_info[%d]\r\n", __func__, len,
            dl_tx_max_info);
    DPRINT_HEX(DBG_TRACE, "DL_SEND", buf, len, /*DUMP_ALWAYS*/ DUMP_DLMS);

    if (len > dl_tx_max_info)
    {
        dl_seg_frame_offs_back = 0;
        dl_seg_frame_offs = 0;
        dl_seg_frame_len = len - dl_tx_max_info;
        dl_seg_frame_write(buf + dl_tx_max_info, 0, dl_seg_frame_len);
        dl_send_info_frame(true, buf, dl_tx_max_info);
    }
    else
    {
        dl_seg_frame_len = 0;
        dl_send_info_frame(false, buf, len);
    }
}

static bool dl_seg_frame_read(uint8_t* buf, int offs, int len)
{
    nv_sub_info.seg.offset = offs;
    nv_sub_info.seg.len = len;
    return nv_read(I_TX_SEG_FRAME, buf);
}

static bool dl_seg_frame_write(uint8_t* buf, int offs, int len)
{
    if ((offs + len) > TX_SEG_FRAME_SIZE)
    {
        dl_seg_frame_over = true;
        return false;
    }

    nv_sub_info.seg.offset = offs;
    nv_sub_info.seg.len = len;
    return nv_write(I_TX_SEG_FRAME, buf);
}

static void dl_seg_info_init(void) { dl_seg_frame_len = 0; }

void dl_set_tx_max_info(uint16_t tx_max_len) { dl_tx_max_info = tx_max_len; }

void dl_send_UI_frame(bool seg, uint8_t* buf, uint16_t len)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    dl_frame_tx(UI_FRAME, seg, buf, len);
}

void dl_send_appl_push_datanoti_msg(uint8_t* buf, int len)
{
    dl_send_UI_frame(false, buf, len);
}
