/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "options_sel.h"
#include "amg_task.h"
#include "os_task_q.h"
#include "amg_sw_timer.h"
#include "amg_uart.h"
#include "amg_timer.h"
#include "amg_meter_main.h"
#include "amg_mif_prtl.h"
#include "amg_mtp_process.h"
#include "amg_dlms_hdlc.h"
#include "amg_gpio.h"
#include "amg_rtc.h"
#include "amg_utc_util.h"
#include "defines.h"
#include "utils.h"
#include "amg_lcd.h"
#include "disp.h"
#include "eeprom_at24cm02.h"
#include "meter.h"
#include "ser1.h"
#include "meter_app.h"
#include "amg_sec.h"
#include "amg_pwr.h"
#include "amg_power_mnt.h"
#include "batmode.h"
#include "amg_imagetransfer.h"
#include "flash_if.h"
#include "amg_media_mnt.h"
#include "amg_modemif_prtl.h"
#include "amg_push_datanoti.h"
#include "dl.h"
#include "amg_dlms_hdlc.h"
#include "set_req.h"
#include "appl.h"
#include "act_req.h"
#include "amg_wdt.h"
#include "get_req.h"
#include "eadc_vbat.h"
#include "amg_stock_op_mode.h"
#include "amg_secu_main.h"
#ifdef M2354_CAN /* bccho, 2023-11-28 */
#include "isotp_defines.h"
#include "isotp.h"
#include "amg_isotp_user.h"
#include "amg_can.h"
#endif
#include "key.h"

extern uint8_t prod_frame;
extern uint8_t prod_log_in_sts;

extern bool PwOn_1st_parm_set;
ST_MIF_SAGSWELL_SETUP *dsm_mtp_get_sagswell(void);
void cal_key_process(void);
void init_mif_task_init(bool firm_up_sts);
void dsm_atcmd_set_trap_power_notify(uint32_t poll_flag, uint8_t sel);
/*
******************************************************************************
*   Definition
******************************************************************************
*/
#define _D "[MT_MAIN] "
#define METER_TASK_STACK_SIZE ((4096 + 1024 + 1024 + 512) / 4)

#define METER_MSGQ_SIZE 32
#define EVENT_MASK_METER_INT (1UL << 0)
#define EVENT_MASK_METER_IC_RX (1UL << 1)
#define EVENT_MASK_RESERVED_1 (1UL << 2)
#define EVENT_MASK_RESERVED_2 (1UL << 3)
#define EVENT_MASK_RESERVED_3 (1UL << 4)
#define EVENT_MASK_RESERVED_4 (1UL << 5)
#define EVENT_MASK_RESERVED_5 (1UL << 6)
#define EVENT_MASK_MTP_FSM (1UL << 7)
#define EVENT_MASK_RS485_RX (1UL << 8)
#define EVENT_MASK_CAN_RX (1UL << 9)
#define EVENT_MASK_IMODEM_RX (1UL << 10)
#define EVENT_MASK_FWUP_FSM (1UL << 11)
#define EVENT_MASK_DATA_NOTI_ERRCODE (1UL << 12)
#define EVENT_MASK_DATA_NOTI_LASTLP (1UL << 13)
#define EVENT_MASK_EMODEM_RX (1UL << 14)
#define EVENT_MASK_PMNT_ETC (1UL << 15)
#if 0 /* bccho, 2025-04-09 */
#define EVENT_MASK_SECU_INITED (1UL << 16)
#define EVENT_MASK_CAN_RX2 (1UL << 17)
#endif

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

extern ST_MIF_SAGSWELL_SETUP g_mtp_sagswell;

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/
static OS_FLAG_GRP *meter_event;
SW_TIMER_CNTX meter_task_timer_cntx;
SW_TIMER meter_task_timer_list[MT_SW_TIMER_MAX];

uint32_t g_mt_sw_general_timer_bits = 0;
uint32_t g_mt_sw_general_timer_two_bits = 0;
uint32_t g_mt_sw_general_timer_three_bits = 0;

#if 1 /* bccho, 2023-08-25 */
uint32_t g_mt_sw_general_timer_four_bits = 0;
uint32_t wakeup_source;
#endif

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
static void meter_timer_event_handler(void *pTmrCntx, uint32_t timerId);

/*
******************************************************************************
*   FUNCTIONS - extern
******************************************************************************
*/
void dsm_mif_rx_int_cb(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_METER_IC_RX, OS_FLAG_SET, &err);
}

void dsm_485if_uart_rx_int_cb(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_RS485_RX, OS_FLAG_SET, &err);
}

void dsm_mtp_fsm_send(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_MTP_FSM, OS_FLAG_SET, &err);
}

void dsm_can_rx_int_cb(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_CAN_RX, OS_FLAG_SET, &err);
}

#if 0 /* bccho, 2025-04-09 */
void dsm_can_rx2_int_cb(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_CAN_RX2, OS_FLAG_SET, &err);
}
#endif

void dsm_imodem_rx_int_cb(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_IMODEM_RX, OS_FLAG_SET, &err);
}

void dsm_emodem_rx_int_cb(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_EMODEM_RX, OS_FLAG_SET, &err);
}

void dsm_fwup_fsm_send(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_FWUP_FSM, OS_FLAG_SET, &err);
}

void dsm_data_noti_errcode_evt_send(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_DATA_NOTI_ERRCODE, OS_FLAG_SET, &err);
}

void dsm_data_noti_lastLP_evt_send(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_DATA_NOTI_LASTLP, OS_FLAG_SET, &err);
}

void dsm_pmnt_etc_evt_send(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_PMNT_ETC, OS_FLAG_SET, &err);
}

#if 0 /* bccho, 2025-04-09 */
void dsm_sec_module_initialized(void)
{
    uint8_t err;
    OSFlagPost(meter_event, EVENT_MASK_SECU_INITED, OS_FLAG_SET, &err);
}
#endif

void dsm_meter_sw_timer_start(uint32_t timerid, uint32_t periodic,
                              uint32_t interval_ms)
{
    dsm_sw_timer_start(&meter_task_timer_cntx, timerid, periodic, interval_ms,
                       meter_timer_event_handler);
}

void dsm_meter_sw_timer_stop(uint32_t timerid)
{
    if (dsm_sw_timer_get_status(&meter_task_timer_cntx, timerid))
    {
        dsm_sw_timer_stop(&meter_task_timer_cntx, timerid);
    }
}

void dsm_meter_if_rx_inter_frame_timer_stop(void)
{
    dsm_meter_sw_timer_stop(MT_SW_TIMER_I_MODEM_RX_INTER_FRAME_TO);
    dsm_meter_sw_timer_stop(MT_SW_TIMER_E_MODEM_RX_INTER_FRAME_TO);
    dsm_meter_sw_timer_stop(MT_SW_TIMER_485_MODEM_RX_INTER_FRAME_TO);
    dsm_meter_sw_timer_stop(MT_SW_TIMER_CAN_MODEM_RX_INTER_FRAME_TO);
}

static void meter_timer_event_handler(void *pTmrCntx, uint32_t timerId)
{
    DPRINTF(DBG_TRACE, _D "TimerHandler - ID:%lu\r\n", timerId);

    switch (timerId)
    {
    case MT_SW_TIMER_CAL_GETREQ_TO:
        dsm_mtp_set_fsm(MTP_FSM_CAL_GET);
        dsm_mtp_fsm_send();
        break;

    case MT_SW_TIMER_FWUP_RESTART_FOR_MODEM_TO:
        dsm_mdm_mic_fwup_retry_TO_proc();
        break;

    case MT_SW_TIMER_ATCMD_TX_TO:
        dsm_atcmd_tx_retry_proc();
        break;

    case MT_SW_TIMER_MIF_MTP_TX_TO:
        dsm_mtp_tx_retry_TO_proc();
        break;

    case MT_SW_TIMER_PUSH_LP_TO:
        appl_push_msg_lastLP();
        break;

    case MT_SW_TIMER_I_MODEM_RX_INTER_FRAME_TO:
    case MT_SW_TIMER_E_MODEM_RX_INTER_FRAME_TO:
        if (timerId == MT_SW_TIMER_I_MODEM_RX_INTER_FRAME_TO)
        {
            DPRINTF(DBG_TRACE, "I_MDM_RX_INTER_TO\r\n");
        }
        else
        {
            DPRINTF(DBG_TRACE, "E_MDM_RX_INTER_TO\r\n");
        }
        dsm_atcmd_fsm_rxbuf_reset();
        if (timerId == MT_SW_TIMER_I_MODEM_RX_INTER_FRAME_TO)
            dsm_hdlc_fsm_rxbuf_reset(HDLC_PKT_CNTX_SUN);
        else
            dsm_hdlc_fsm_rxbuf_reset(HDLC_PKT_CNTX_PLC);
        break;

    case MT_SW_TIMER_485_MODEM_RX_INTER_FRAME_TO:
    case MT_SW_TIMER_CAN_MODEM_RX_INTER_FRAME_TO:
        if (timerId == MT_SW_TIMER_485_MODEM_RX_INTER_FRAME_TO)
        {
            DPRINTF(DBG_TRACE, "485_RX_INTER_FRAME_TO\r\n");
        }
        else
        {
            DPRINTF(DBG_TRACE, "CAN_RX_INTER_FRAME_TO\r\n");
        }
        if (timerId == MT_SW_TIMER_485_MODEM_RX_INTER_FRAME_TO)
            dsm_hdlc_fsm_rxbuf_reset(HDLC_PKT_CNTX_485);
        else
            dsm_hdlc_fsm_rxbuf_reset(HDLC_PKT_CNTX_CAN);
        break;

    case MT_SW_GENERAL_three_TO:
        if (M_MT_SW_GENERAL_three_TIMER_is_zcd_out())
        {
            dsm_meter_sw_timer_stop(MT_SW_GENERAL_three_TO);
            M_MT_SW_GENERAL_three_TIMER_clear_zcd_out();
            if (dsm_zcd_action_get_start_flag() == TRUE)
            {
                dsm_zcd_action_set_start_flag(FALSE);
                DPRINTF(DBG_TRACE, "duration %d sec\r\n", zcrs_sig_out_dur);

                dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
                dsm_atcmd_set_zcd(FALSE, 1);
                dsm_mif_zcd_on();

                M_MT_SW_GENERAL_three_TIMER_set_zcd_out();
                dsm_meter_sw_timer_start(MT_SW_GENERAL_three_TO, FALSE,
                                         (zcrs_sig_out_dur * 1000));
            }
            else
            {
                zcrs_sig_out_dur = 0;
                dsm_mif_zcd_off();
                dsm_atcmd_get_zcd_time(FALSE);
                OSTimeDly(OS_MS2TICK(150));

                dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
                dsm_atcmd_set_zcd(FALSE, 0);
            }
        }
        break;

    case MT_SW_GENERAL_four_TO: /* bccho, 2023-10-03 */
#ifdef STOCK_OP                 /* bccho, 2024-09-26 */
        if (M_MT_SW_GENERAL_four_TIMER_is_wait_stock_op())
        {
            extern uint16_t g_stock_op_times;
            bool send = false;

            MSG07("<M_MT_SW_GENERAL_four_TIMER_is_wait_stock_op>");
            dsm_meter_sw_timer_stop(MT_SW_GENERAL_four_TO);
            M_MT_SW_GENERAL_four_TIMER_clear_wait_stock_op();

            uint32_t fsm = dsm_stock_op_get_fsm();
            switch (fsm)
            {
            case STOCK_FSM_NONE:
                MSG07("  -->STOCK_FSM_NONE");
                break;

            case STOCK_FSM_WAKE_DEFAULT: /* 20:00에 깨어남*/
                MSG07("  -->STOCK_FSM_WAKE_DEFAULT");
#ifdef RTC_ALARM_INT_TEST
                send = true;
#endif
                break;

            case STOCK_FSM_WAKE_ALA_1:
                MSG07("  -->STOCK_FSM_WAKE_ALA_1");
                send = true;
                break;

            case STOCK_FSM_WAKE_ALA_2:
                MSG07("  -->STOCK_FSM_WAKE_ALA_2");
                send = true;
                break;

            default:
                break;
            }

            if (send)
            {
                /* 재고 관리 동작 횟수 */
                g_stock_op_times++;
                dsm_stock_op_times_write();
                dsm_stock_action();
            }

            b_dsp_bm_finished = TRUE;
        }
        else
#endif
            /* bccho, 2024-01-10, 배터리로 부팅 시 2초 후에 DPD 진입 */
            if (M_MT_SW_GENERAL_four_TIMER_is_BATT_BOOT_DPD())
            {
                MSGALWAYS("Timeout!!!, four_TIMER_is_BATT_BOOT_DPD");
                b_dsp_bm_finished = TRUE;
            }
        break;

    case MT_SW_TIMER_PMNT_NO_VOLT_SET_OP_TO:
        dsm_meter_sw_timer_stop(MT_SW_TIMER_PMNT_NO_VOLT_SET_OP_TO);
        dsm_pmnt_comm_rx_x_at_NO_VOLT_SET_OP();
        break;

    case MT_SW_GENERAL_TO:
        DPRINTF(DBG_WARN, "GENERAL_TO [0x%08X]\r\n",
                g_mt_sw_general_timer_bits);
        if (M_MT_SW_GENERAL_TIMER_is_bootup_setup())
        {
            M_MT_SW_GENERAL_TIMER_clear_bootup_setup();
            dsm_meter_sw_timer_stop(MT_SW_GENERAL_TO);
            dsm_update_forReport();
            dsm_mtp_fsm_tx_proc_fw_data_get_currunningbank();
        }
        dsm_image_update_go_proc();
        break;

    case MT_SW_GENERAL_two_TO:
        DPRINTF(DBG_WARN, "GENERAL_two_TO [0x%08X]\r\n",
                g_mt_sw_general_timer_two_bits);
        dsm_meter_reset_timer_proc();
        break;

        /* 계기 자신에게 DLMS Polling이 2시간 동안 수신되지 않는 경우 */
    case MT_SW_TIMER_ASSO_TO:  // ID : 10
        dsm_meter_sw_timer_stop(MT_SW_TIMER_ASSO_TO);
        appl_set_asso_timestart_flag(false);
#if 0        
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
        dsm_atcmd_set_meterid(TRUE);
        OSTimeDly(OS_MS2TICK(100));
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
        dsm_imgtrfr_fwup_set_fsm(FWU_FSM_NONE);
        dsm_atcmd_set_reset(TRUE, AT_RST_FACTORY);
        // 2. aa 해제
        dl_stop_force();
        // 3. 착탈형 모뎀 리셋
        dsm_modem_hw_reset(EXT_MODEM_RESET);
#else
        R_CALL();
#endif
        break;

    case MT_SW_TIMER_ASSO_4_485_TO:  // ID : 10
        dsm_meter_sw_timer_stop(MT_SW_TIMER_ASSO_4_485_TO);
        appl_set_asso_4_485_timestart_flag(false);

#if 0  // JP.KIM 24.10.08
       //  asso_4 485 connection 후 5분간  통신 없으면 disconnection
       //=======================================================================
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
        dsm_atcmd_set_meterid(TRUE);
        OSTimeDly(OS_MS2TICK(100));
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);

        dsm_imgtrfr_fwup_set_fsm(FWU_FSM_NONE);

#if defined(FEATURE_SUN_MODEM_AT_FUN_1_MT_SW_TIMER_ASSO_TO)  // 24.09.04
                                                             // (AT+FUN:1 제거)
                                                             // -> SUN_MODEM
        dsm_atcmd_set_reset(TRUE, AT_RST_FACTORY);
#endif
#endif
        // 2. aa 해제
        dl_stop_force();
#if 0        
        // 3. 착탈형 모뎀 리셋
        dsm_modem_hw_reset(EXT_MODEM_RESET);
#endif
        //=======================================================================
        break;

    default:
        break;
    }
}

uint32_t g_dm_out_measure_print_chkcount = 0;
uint32_t dsm_get_dm_out_measure_print_chkcount(void)
{
    return g_dm_out_measure_print_chkcount;
}

void dsm_add_dm_out_measure_print_chkcount(void)
{
    g_dm_out_measure_print_chkcount++;
}

static void dsm_mif_rx_proc(void)
{
    //==============================================================================
    uint8_t buff[256];
    ST_MIF_COM_PKT mif_com_pkt;
    uint32_t size = 0;
    uint32_t tmp, result, idx = 0;
    struct tm SystemTime;
    ST_TIME_BCD stITime;
    size = dsm_uart_gets(MIF_PORT, (char *)buff, sizeof(buff));

    DPRINT_HEX(DBG_NONE, "EVENT_MASK_METER_IC_RX", buff, size, DUMP_MIF);

    while (size)
    {
        tmp = idx;
        result = dsm_mif_rx_parser(&buff[idx], size, &idx, &mif_com_pkt);
        size -= (idx - tmp);

        if (result == MIF__FRAME_NO_ERR)
        {
            dsm_meter_sw_timer_stop(MT_SW_TIMER_MIF_MTP_TX_TO);

            uint8_t rsp, len;
            uint32_t ch_count = 0;

            dsm_add_dm_out_measure_print_chkcount();
            ch_count = dsm_get_dm_out_measure_print_chkcount();

            rsp = mif_com_pkt.data[MIF_CMD_POS];
            len = mif_com_pkt.len - MIF_EXTRA_SIZE;
            dsm_mtp_rx_process(rsp, len, &mif_com_pkt.data[MIF_BODY_POS]);
            mif_com_pkt.len = 0;

            if (ch_count % 10 == 0)
            {
                util_get_system_time(&SystemTime, &stITime);
                DPRINTF(DBG_NONE, _D "TIME: %s\r\n",
                        util_get_date_time_string((uint8_t *)&stITime,
                                                  NULL));  // [MT_MAIN] TIME:
            }
        }
    }
    //==============================================================================
}

static void dsm_485_rx_proc(void)
{
    char buff[1024 + 20];  // HDLC_MAX_PACKET_NUM
    ST_HDLC_COM_PKT hdlc_com_pkt;
    uint16_t size;
    uint8_t HDLC_CNTX_TYPE = HDLC_PKT_CNTX_485;
    uint16_t tmp, idx = 0;
    uint8_t result = HDLC_FRAME_RCV_CONTINUE_TMP;
    size = dsm_uart_gets(RS485_PORT, buff, sizeof(buff));

    DPRINT_HEX(DBG_NONE, "EVENT_MASK_RS485_RX", buff, size, DUMP_DLMS);
    while (size)
    {
        tmp = idx;
        result = dsm_hdlc_rx_parser(HDLC_CNTX_TYPE, (uint8_t *)&buff[idx], size,
                                    &idx, &hdlc_com_pkt);
        size -= (idx - tmp);
        if (result == HDLC_FRAME_NO_ERR_TMP)
        {
            dsm_meter_sw_timer_stop(MT_SW_TIMER_485_MODEM_RX_INTER_FRAME_TO);

            if (prod_log_in_sts != 0xa5)
            {
                if (dsm_media_if_return_proc(RS485_IF_RX_EVT, &hdlc_com_pkt))
                {
                    continue;
                }
            }

            if (hdlc_com_pkt.len > MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO)
            {
                DPRINTF(DBG_ERR, "DLMS Length Error: %ld\r\n",
                        hdlc_com_pkt.len);
            }
            else
            {
                DPRINT_HEX(DBG_TRACE, "RS485_RX", hdlc_com_pkt.data,
                           hdlc_com_pkt.len, DUMP_SEC);

                dsm_media_set_fsm_if_hdlc(MEDIA_RUN_RS485);
                if (dsm_media_if_dl_is_disc(hdlc_com_pkt.data[6]))
                {
                    if ((dsm_media_if_dl_is_valid_asso4_clientaddr(
                             hdlc_com_pkt.data[5]) &&
                         dsm_media_if_dl_is_valid_asso_3_4_svraddr(
                             hdlc_com_pkt.data[3], hdlc_com_pkt.data[4])) ||
                        (dsm_media_if_dl_is_valid_asso3_clientaddr(
                             hdlc_com_pkt.data[5]) &&
                         dsm_media_if_dl_is_valid_asso_3_4_svraddr(
                             hdlc_com_pkt.data[3], hdlc_com_pkt.data[4])) ||
                        (dsm_media_if_dl_is_valid_asso_1_2_clientaddr(
                             hdlc_com_pkt.data[5]) &&
                         dsm_media_if_dl_is_valid_asso_1_svraddr(
                             hdlc_com_pkt.data[3], hdlc_com_pkt.data[4])))
                    {
#if 0 /* bccho, 2024-09-30, 삭제 */                     
                        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
                        // DISC 수신 -> 내장 모뎀을 일반 모드로 변경,
                        // "AT+LISTEN:0\r" 일반 모드
                        dsm_atcmd_set_listen(TRUE, AT_LISTEN_OFF);
#endif
                        dsm_meter_sw_timer_stop(MT_SW_TIMER_ASSO_TO);
                        dsm_meter_sw_timer_start(MT_SW_TIMER_ASSO_TO, FALSE,
                                                 MT_TIMEOUT_ASSO_TIME);
                        appl_set_asso_timestart_flag(true);

                        appl_update_sec_for_timestart_sec_utility(
                            dsm_rtc_get_time());

                        if (dsm_media_if_dl_is_valid_asso4_clientaddr(
                                hdlc_com_pkt.data[5]) &&
                            dsm_media_if_dl_is_valid_asso_3_4_svraddr(
                                hdlc_com_pkt.data[3], hdlc_com_pkt.data[4]))
                        {
                            // 5분간 현장(asso_4) 485 통신이 안되면-> asso_4
                            // disconnection  ===============================
                            // JP.KIM 24.10.07
                            dsm_meter_sw_timer_stop(MT_SW_TIMER_ASSO_4_485_TO);
                            dsm_meter_sw_timer_start(
                                MT_SW_TIMER_ASSO_4_485_TO, FALSE,
                                MT_TIMEOUT_ASSO_4_485_TIME);
                            appl_set_asso_4_485_timestart_flag(true);
                            appl_update_sec_4_485_for_timestart_sec_utility(
                                dsm_rtc_get_time());
                            DPRINTF(
                                DBG_TRACE,
                                "%s: TIMER START - asso_4 %d sec\r\n", __func__,
                                appl_get_sec_4_485_for_timestart_sec_utility());
                        }
                    }
                }
                // amr_rcv_frame() 함수에서 데이터를 읽어서 사용함
                ser_rx_buf_inQ(hdlc_com_pkt.data, hdlc_com_pkt.len);
            }

            hdlc_com_pkt.len = 0;
        }  // HDLC_FRAME_NO_ERR_TMP
        else if (result != HDLC_FRAME_RCV_CONTINUE_TMP)
        {
            DPRINTF(DBG_TRACE, "DLMS Parsing Error %d: s:%d\r\n", result,
                    /*size*/ idx + size);
            DPRINT_HEX(DBG_TRACE, "RX Data", buff, /*size*/ idx + size,
                       DUMP_ALWAYS);
            dsm_hdlc_rx_pkt_init(
                HDLC_CNTX_TYPE);  // dsm_hdlc_fsm_rxbuf_reset(HDLC_CNTX_TYPE);
            return;
        }
        //
    }
    //
    if (result != HDLC_FRAME_NO_ERR_TMP)
    {
        dsm_meter_sw_timer_start(MT_SW_TIMER_485_MODEM_RX_INTER_FRAME_TO, FALSE,
                                 MT_TIMEOUT_RX_INTER_FRAME_TIME);
    }
    //
}

#ifdef M2354_CAN /* bccho, 2023-11-28 */
static void dsm_can_rx_proc(void)
{
    IsoTpLink *plink = NULL;
    ST_HDLC_COM_PKT hdlc_com_pkt;
    uint32_t mode;
    uint8_t buff[1024 + 20];
    uint16_t size;
    uint8_t HDLC_CNTX_TYPE = HDLC_PKT_CNTX_CAN;
    int ret;
    uint16_t tmp, idx = 0;
    uint8_t result = HDLC_FRAME_RCV_CONTINUE_TMP;

    plink = (IsoTpLink *)dsm_isotp_user_get_link();
    ret = isotp_receive(plink, buff, 1024, &size);
    mode = dsm_isotp_user_proc();

    if (mode != DLMS_EX_MODE || ret != ISOTP_RET_OK)
    {
        return;
    }

    DPRINT_HEX(DBG_NONE, "EVENT_MASK_CAN_RX", buff, size, DUMP_DLMS);
    while (size)
    {
        tmp = idx;
        result = dsm_hdlc_rx_parser(HDLC_CNTX_TYPE, (uint8_t *)&buff[idx], size,
                                    &idx, &hdlc_com_pkt);
        size -= (idx - tmp);

        if (result == HDLC_FRAME_NO_ERR_TMP)
        {
            dsm_meter_sw_timer_stop(MT_SW_TIMER_CAN_MODEM_RX_INTER_FRAME_TO);

            if (dsm_media_if_return_proc(CAN_IF_RX_EVT, &hdlc_com_pkt))
            {
                continue;
            }

            if (hdlc_com_pkt.len > MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO)
            {
                DPRINTF(DBG_ERR, "CAN Length Error: %ld\r\n", hdlc_com_pkt.len);
            }
            else
            {
                DPRINT_HEX(DBG_NONE, "CAN_RX", hdlc_com_pkt.data,
                           hdlc_com_pkt.len, DUMP_DLMS);

                dsm_media_set_fsm_if_hdlc(MEDIA_RUN_CAN);
                if (dsm_media_if_dl_is_disc(hdlc_com_pkt.data[6]))
                {
                    if ((dsm_media_if_dl_is_valid_asso4_clientaddr(
                             hdlc_com_pkt.data[5]) &&
                         dsm_media_if_dl_is_valid_asso_3_4_svraddr(
                             hdlc_com_pkt.data[3], hdlc_com_pkt.data[4])) ||
                        (dsm_media_if_dl_is_valid_asso3_clientaddr(
                             hdlc_com_pkt.data[5]) &&
                         dsm_media_if_dl_is_valid_asso_3_4_svraddr(
                             hdlc_com_pkt.data[3], hdlc_com_pkt.data[4])) ||
                        (dsm_media_if_dl_is_valid_asso_1_2_clientaddr(
                             hdlc_com_pkt.data[5]) &&
                         dsm_media_if_dl_is_valid_asso_1_svraddr(
                             hdlc_com_pkt.data[3], hdlc_com_pkt.data[4])))
                    {
                        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
                        // DISC 수신 -> 내장 모뎀을 일반 모드로 변경,
                        // "AT+LISTEN:0\r" 일반 모드
                        dsm_atcmd_set_listen(TRUE, AT_LISTEN_OFF);
                        dsm_meter_sw_timer_stop(MT_SW_TIMER_ASSO_TO);
                        dsm_meter_sw_timer_start(
                            MT_SW_TIMER_ASSO_TO, FALSE,
                            MT_TIMEOUT_ASSO_TIME);  // 2 Hour
                        appl_set_asso_timestart_flag(true);
                        appl_update_sec_for_timestart_sec_utility(
                            dsm_rtc_get_time());
                    }
                }

                ser_rx_buf_inQ(hdlc_com_pkt.data, hdlc_com_pkt.len);
            }

            hdlc_com_pkt.len = 0;
        } /* result == HDLC_FRAME_NO_ERR_TMP */
        else if (result != HDLC_FRAME_RCV_CONTINUE_TMP)
        {
            DPRINTF(DBG_TRACE, "DLMS Parsing Error %d: s:%d\r\n", result,
                    /*size*/ idx + size);
            DPRINT_HEX(DBG_TRACE, "RX Data", buff, /*size*/ idx + size,
                       DUMP_ALWAYS);
            dsm_hdlc_rx_pkt_init(HDLC_CNTX_TYPE);
            return;
        }
    }

    if (result != HDLC_FRAME_NO_ERR_TMP)
    {
        dsm_meter_sw_timer_start(MT_SW_TIMER_CAN_MODEM_RX_INTER_FRAME_TO, FALSE,
                                 MT_TIMEOUT_RX_INTER_FRAME_TIME);
    }
}
#endif

static void dsm_imodem_rx_proc(void)
{
    //==============================================================================
    char buff[1024 + 20];
    uint16_t size;
    ST_HDLC_COM_PKT hdlc_com_pkt;
    ST_AT_CMD_RX_PKT atcmd_com_pkt;
    uint8_t HDLC_CNTX_TYPE = HDLC_PKT_CNTX_SUN;
    uint16_t tmp, idx = 0;
    uint8_t result = 0;

    size = dsm_uart_gets(IMODEM_PORT, buff, sizeof(buff));

    DPRINT_HEX(DBG_INFO, "I-MODEM_RX", buff, size, DUMP_MDM);

    /* AT+CMD Parsing */
    while (size)
    {
        result =
            dsm_atcmd_rx_parser((uint8_t *)&buff[idx], size, &atcmd_com_pkt);

        if (result == AT_ERR_NONE)
        {
            return;
        }
        else if (result == AT_ERR_NONE_PARSER_OK)
        {
            idx += atcmd_com_pkt.idx;
            size -= atcmd_com_pkt.idx;

            DPRINT_HEX(DBG_NONE, "SUN_AT_RX", atcmd_com_pkt.pkt,
                       atcmd_com_pkt.cnt, DUMP_ALWAYS);
            dsm_meter_sw_timer_stop(MT_SW_TIMER_ATCMD_TX_TO);

            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);

            dsm_atcmd_rx_proc(&atcmd_com_pkt);
            dsm_atcmd_rx_pkt_init();
            atcmd_com_pkt.len = 0;
        }
        else
        {
            break;
        }
    }

    /* DLMS Parsing */
    result = HDLC_FRAME_RCV_CONTINUE_TMP;
    while (size)
    {
        tmp = idx;
        result = dsm_hdlc_rx_parser(HDLC_CNTX_TYPE, (uint8_t *)&buff[idx], size,
                                    &idx, &hdlc_com_pkt);
        size -= (idx - tmp);

        if (result == HDLC_FRAME_NO_ERR_TMP)
        {
            dsm_meter_sw_timer_stop(MT_SW_TIMER_I_MODEM_RX_INTER_FRAME_TO);

            // dsm_wdt_ext_toggle_immd();
            if (dsm_media_if_return_proc(SUN_IF_RX_EVT, &hdlc_com_pkt))
            {
                continue;
            }

            if (PMNT_NO_VOLT_SET_OP == dsm_pmnt_get_op_mode())
            {
                dsm_meter_sw_timer_stop(MT_SW_TIMER_PMNT_NO_VOLT_SET_OP_TO);
                dsm_meter_sw_timer_start(
                    MT_SW_TIMER_PMNT_NO_VOLT_SET_OP_TO, FALSE,
                    MT_TIMEOUT_PMNT_NO_VOLT_SET_OP_RX_FRAME_TIME);  // 5 Min
                novoltsetop_update_sec_for_timestart(dsm_rtc_get_time());
                DPRINTF(DBG_TRACE,
                        _D "%s: TIMER START - No-Voltage Setup Mode %d sec\r\n",
                        __func__, novoltsetop_get_sec_for_timestart());
            }

            if (hdlc_com_pkt.len > MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO)
            {
                DPRINT_HEX(DBG_ERR, "SUN_LEN_ERR", hdlc_com_pkt.data, 512,
                           DUMP_ALWAYS);
            }
            else
            {
                DPRINT_HEX(DBG_NONE, "SUN_HDLC_RX", hdlc_com_pkt.data,
                           hdlc_com_pkt.len, DUMP_ALWAYS);

                dsm_media_set_fsm_if_hdlc(MEDIA_RUN_SUN);

                ser_rx_buf_inQ(hdlc_com_pkt.data, hdlc_com_pkt.len);
            }

            // dsm_hdlc_rx_pkt_init(HDLC_CNTX_TYPE);
            hdlc_com_pkt.len = 0;
        }
    }

    if (result != HDLC_FRAME_NO_ERR_TMP)
    {
        dsm_meter_sw_timer_start(MT_SW_TIMER_I_MODEM_RX_INTER_FRAME_TO, FALSE,
                                 MT_TIMEOUT_RX_INTER_FRAME_TIME);
    }
    //==============================================================================
}

static void dsm_emodem_rx_proc(void)
{
    char buff[1024 + 20];
    uint16_t size;
    ST_HDLC_COM_PKT hdlc_com_pkt;
    ST_AT_CMD_RX_PKT atcmd_com_pkt;
    uint8_t HDLC_CNTX_TYPE = HDLC_PKT_CNTX_PLC;
    uint16_t tmp, idx = 0;
    uint8_t result = 0;

    size = dsm_uart_gets(EMODEM_PORT, buff, sizeof(buff));

    DPRINT_HEX(DBG_INFO, "E-MODEM_RX", buff, size, DUMP_EMDM);

    //======================================================================
    // TODO: (WD) Parsing of the LMN header is required.
    //======================================================================

    /* AT+CMD Parsing */
    while (size)
    {
        result =
            dsm_atcmd_rx_parser((uint8_t *)&buff[idx], size, &atcmd_com_pkt);

        if (result == AT_ERR_NONE)
        {
            return;
        }
        else if (result == AT_ERR_NONE_PARSER_OK)
        {
            idx += atcmd_com_pkt.idx;
            size -= atcmd_com_pkt.idx;

            DPRINT_HEX(DBG_NONE, "EXT_AT_RX", atcmd_com_pkt.pkt,
                       atcmd_com_pkt.cnt, DUMP_ALWAYS);

            dsm_meter_sw_timer_stop(MT_SW_TIMER_ATCMD_TX_TO);

            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);

            dsm_atcmd_rx_proc(&atcmd_com_pkt);
            dsm_atcmd_rx_pkt_init();
            atcmd_com_pkt.len = 0;
        }
        else
        {
            break;
        }
    }

    /* DLMS Parsing */
    result = HDLC_FRAME_RCV_CONTINUE_TMP;
    while (size)
    {
        tmp = idx;
        result = dsm_hdlc_rx_parser(HDLC_CNTX_TYPE, (uint8_t *)&buff[idx], size,
                                    &idx, &hdlc_com_pkt);
        size -= (idx - tmp);

        if (result == HDLC_FRAME_NO_ERR_TMP)
        {
            dsm_meter_sw_timer_stop(MT_SW_TIMER_E_MODEM_RX_INTER_FRAME_TO);

            if (dsm_media_if_return_proc(EXT_IF_RX_EVT, &hdlc_com_pkt))
            {
                continue;
            }

            if (hdlc_com_pkt.len > MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO)
            {
                DPRINT_HEX(DBG_ERR, "EXT_HDLC_RX_LEN_ERR", hdlc_com_pkt.data,
                           512, DUMP_ALWAYS);
            }
            else
            {
                DPRINT_HEX(DBG_NONE, "EXT_HDLC_RX", hdlc_com_pkt.data,
                           hdlc_com_pkt.len, DUMP_ALWAYS);

                if (dsm_media_get_fsm_if_hdlc() != MEDIA_RUN_EXT)
                {
                    dsm_media_set_fsm_if_hdlc(MEDIA_RUN_EXT);
                }

                ser_rx_buf_inQ(hdlc_com_pkt.data, hdlc_com_pkt.len);
            }

            // dsm_hdlc_rx_pkt_init(HDLC_CNTX_TYPE);
            hdlc_com_pkt.len = 0;
        }
    }

    if (result != HDLC_FRAME_NO_ERR_TMP)
    {
        dsm_meter_sw_timer_start(MT_SW_TIMER_E_MODEM_RX_INTER_FRAME_TO, FALSE,
                                 MT_TIMEOUT_RX_INTER_FRAME_TIME);
    }
}

#define METER_TASK_BLOCK_ms_MAX (15 * 1000)
#define METER_TASK_BLOCK_DURATION_MS (BLOCK_DURATION_MS + 2000)
#define METER_TASK_BLOCK_CONTINUE_COUNT 50

uint32_t g_task_block_cnt = 0;
void dsm_os_check_task_block(uint8_t *name, uint32_t pre_time)
{
    static uint32_t max_interval;
    uint32_t dt = TIMER2MS(OS_TIME_GET() - pre_time);
    if (max_interval < dt)
    {
        max_interval = dt;
        DPRINTF(DBG_WARN, "%s Task maximum duration is updated %d\r\n", name,
                dt);

        if (max_interval > METER_TASK_BLOCK_ms_MAX)
        {
            max_interval = 0;
            g_task_block_cnt = 0;
        }
    }
    if (dt > METER_TASK_BLOCK_DURATION_MS)
    {
        DPRINTF(DBG_ERR, "%s Task blocked during %dms\r\n", name, dt);

        if (g_task_block_cnt++ > METER_TASK_BLOCK_CONTINUE_COUNT)
        {
            max_interval = 0;
            g_task_block_cnt = 0;
        }
    }
    else
    {
        g_task_block_cnt = 0;
    }
}

static void dsm_meter_task_main(void *pdata)
{
#if 1 /* bccho, 2023-09-14 */
    portALLOCATE_SECURE_CONTEXT(configMINIMAL_SECURE_STACK_SIZE);
#endif

    uint8_t err;
    OS_FLAGS masked_event;
    OS_FLAGS waiting_event;
    bool bat_boot_mdoe = false;
    PwOn_1st_parm_set = false;
    UNUSED(pdata);
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    waiting_event =
        EVENT_MASK_PRIMITIVE | EVENT_MASK_METER_INT | EVENT_MASK_TIMER |
        EVENT_MASK_RS485_RX | EVENT_MASK_METER_IC_RX | EVENT_MASK_MTP_FSM |
        EVENT_MASK_CAN_RX | EVENT_MASK_IMODEM_RX | EVENT_MASK_EMODEM_RX |
        EVENT_MASK_FWUP_FSM | EVENT_MASK_DATA_NOTI_ERRCODE |
        EVENT_MASK_DATA_NOTI_LASTLP | EVENT_MASK_PMNT_ETC,
#if 0 /* bccho, 2025-04-09 */
        | EVENT_MASK_SECU_INITED | EVENT_MASK_CAN_RX2;
#endif

    dsm_imgtrfr_con_init(IMG__TOU);
    dsm_imgtrfr_con_init(IMG__FW);

    dsm_sag_port_init();
    init_vbat_adc(); /* bccho, 2023-11-30 */

#ifdef BATTERY_ACTIVE_OP /* bccho, 2023-11-26, 배터리로 부팅해도 ACTIVE_OP로 \
                            동작하도록. 테스트용 */
    run_set_main_power();
    dsm_pmnt_set_wakeup_dn_evt(WUP_S_MAIN_POWER);
    DPRINTF(DBG_WARN, _D "BOOT  - MAIN PWR\r\n");
#else
    if (IS_AC_ON)
    {
        run_set_main_power();
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_MAIN_POWER);
        MSGALWAYS("BOOT  - MAIN PWR, PB.0:%d, PA.10:%d", PB0, PA10);

#if 0 /* bccho, 2025-04-09 */
        dsm_secu_module_init(); /* bccho, 2023-12-01 */
#else
        hls_sec_init();
#endif
    }
    else
    {
        run_set_bat_power();
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_BAT_POWER);
        MSGALWAYS("BOOT  - BAT PWR, PB.0:%d, PA.10:%d", PB0, PA10);
    }
#endif
    /* --> 여기는 op == POWER_OFF */

    dsm_pmnt_fsm(dsm_pmnt_get_wakeup_dn_evt());
    /* --> 부팅시 전원에 따라 PMNT_ACTIVE_OP 또는 PMNT_NO_VOLT_OP */

    /* 배터리로 부팅한 경우 */
    if (dsm_pmnt_get_op_mode() == PMNT_NO_VOLT_OP)
    {
        /* PMNT_LOW_POWER_OP 전환 */
        dsm_pmnt_set_wakeup_dn_evt(WDN_S_LOW_PWR_ENTRY);
        dsm_pmnt_fsm(dsm_pmnt_get_wakeup_dn_evt());

        /* 메뉴키 wakeup, PB.2 --> 무전압검침 */
        if ((wakeup_source & CLK_PMUSTS_PINWK2_Msk) != 0)
        {
            MSG07("Wake-up source: PB.2 (Menu Key)");
            dsm_pmnt_set_wakeup_dn_evt(WUP_S_MENU_KEY);
            dsm_pmnt_fsm(dsm_pmnt_get_wakeup_dn_evt());
        }
#ifdef STOCK_OP /* bccho, 2024-09-26 */
        /* RTC wakeup, PB.12 --> 재고관리 */
        else if ((wakeup_source & CLK_PMUSTS_PINWK3_Msk) != 0)
        {
            MSG07("Wake-up source: PB.12 (RTC Alarm)");
            dsm_pmnt_set_wakeup_dn_evt(WUP_S_RTC_ALA);
            dsm_pmnt_fsm(dsm_pmnt_get_wakeup_dn_evt());
        }
#endif
        /* 배터리로 POR */
        else
        {
            MSG07("battery && Normal booting");
/* bccho, 20240-07-05, JnD 출장. 아래 코드 안씀 */
#ifdef DPD_ENTER_NEW_LOGICx /* bccho, 2024-06-13 */
            run_set_main_power();
            dsm_pmnt_set_wakeup_dn_evt(WUP_S_MAIN_POWER);
            dsm_pmnt_fsm(dsm_pmnt_get_wakeup_dn_evt());
#else
            dsm_pmnt_set_wakeup_dn_evt(WUP_S_BAT_POWER);
            dsm_pmnt_fsm(dsm_pmnt_get_wakeup_dn_evt());
            bat_boot_mdoe = true;
#endif
        }
    }

    dsm_pmnt_peri_initialize(dsm_pmnt_get_op_mode());

    MSG06("meter_task_main, --> %s",
          dsm_pmnt_op_mode_string(dsm_pmnt_get_op_mode()));

    init_mif_task_init(0);

#if 0
    dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
    dsm_atcmd_set_trap_power_notify(FALSE, '1');  // ac 복전
#endif

#if 0 /* bccho, 2025-04-09 */
    bool secu_inited = false;
#endif

    while (1)
    {
#if 1 /* bccho, WDT, 2023-09-27 */

        taskENTER_CRITICAL();
        metertask_wdkick_done = true;
        taskEXIT_CRITICAL();
#endif
        // clang-format off
        masked_event = OSFlagPend(
            meter_event, 
            waiting_event,
            (OS_FLAG_WAIT_SET_ANY + OS_FLAG_CONSUME), /* 사용안함 */
            OS_MS2TICK(2), 
            &err                                      /* 사용안함 */
        );
        // clang-format on

        if (masked_event & EVENT_MASK_TIMER)
        {
            MSG03("main_EVENT_MASK_TIMER");
            dsm_sw_timer_task_proc(&meter_task_timer_cntx);
        }
        if (masked_event & EVENT_MASK_DATA_NOTI_ERRCODE)
        {
            MSG03("EVENT_MASK_DATA_NOTI_ERRCODE");
            dsm_push_data_noti_proc(PUSH_SCRIPT_ID_ERR_CODE);
        }
        if (masked_event & EVENT_MASK_DATA_NOTI_LASTLP)
        {
            MSG03("EVENT_MASK_DATA_NOTI_LASTLP");
            dsm_push_data_noti_proc(PUSH_SCRIPT_ID_LAST_LP);
        }
        if (masked_event & EVENT_MASK_MTP_FSM)
        {
            MSG03("EVENT_MASK_MTP_FSM");
            dsm_mtp_fsm_tx_proc();
        }
        if (masked_event & EVENT_MASK_METER_IC_RX)
        {
            MSG00("EVENT_MASK_METER_IC_RX");
            dsm_mif_rx_proc();
        }
        if (masked_event & EVENT_MASK_RS485_RX)
        {
            MSG03("EVENT_MASK_RS485_RX");
            dsm_485_rx_proc();
        }
        if (masked_event & EVENT_MASK_CAN_RX)
        {
            MSG00("EVENT_MASK_CAN_RX");
#ifdef M2354_CAN /* bccho, 2023-11-28 */
            dsm_can_rx_proc();
#endif
        }
#ifdef M2354_CAN /* bccho, 2023-11-28 */
        if (masked_event & EVENT_MASK_CAN_RX2)
        {
            MSG00("EVENT_MASK_CAN_RX2");
            void dsm_can_rx2_proc(void);
            dsm_can_rx2_proc();
        }
#endif
        if (masked_event & EVENT_MASK_IMODEM_RX)
        {
            MSG03("EVENT_MASK_IMODEM_RX");
            dsm_imodem_rx_proc();
        }
        if (masked_event & EVENT_MASK_EMODEM_RX)
        {
            MSG03("EVENT_MASK_EMODEM_RX");
            dsm_emodem_rx_proc();
        }
        if (masked_event & EVENT_MASK_FWUP_FSM)
        {
            MSG03("EVENT_MASK_FWUP_FSM");
            dsm_mdm_mic_fwup_fsm_tx_proc();
        }
        if (masked_event & EVENT_MASK_PMNT_ETC)
        {
            MSG03("EVENT_MASK_PMNT_ETC");
            dsm_pmnt_batpwr_noVolt_noVoltSet_proc();
        }

#if 0            /* bccho, 2025-04-09 */

        if (masked_event & EVENT_MASK_SECU_INITED)
        {
            MSG06("EVENT_MASK_SECU_INITED");

            // 아래는 보안칩이 초기화된 이후에 호출되어야 함.
#ifdef M2354_CAN /* bccho, 2023-11-28 */
            dsm_can_init(CAN_BITRATE_500K);
            dsm_isotp_user_init();
            CAN_NormalMode_SetRxMsg(CAN0);
            if (dsm_mtp_get_op_mode() == MTP_OP_NORMAL)
            {
                dsm_can_advertisement_power_on();
            }
#endif
#ifdef STOCK_OP /* bccho, 2024-09-26 */
            dsm_stock_op_random_seed_init();
#endif
            secu_inited = true;
        }
#endif

        dsm_sec_set_operation(false);

        background();

        foreground();

#ifdef M2354_CAN /* bccho, 2023-11-28 */
        dsm_isotp_user_tx_poll();
#endif
        dsm_all_uart_q_flush(); /* All UART Send */

#if 0 /* bccho, 2025-04-09 */
        if (secu_inited)
        {
#ifdef MAIN_TASK_NO_DELAY
            /* do nothing */
#else
            vTaskDelay(1);
#endif
        }
        else
        {
            vTaskDelay(10);
        }
#endif

        /* bccho, 2024-01-10, 배터리로 부팅 시 2초 후에 DPD 진입 */
        /* bccho, 2024-06-07, 사용 안함 */
        if (bat_boot_mdoe)
        {
            bat_boot_mdoe = false;
            M_MT_SW_GENERAL_four_TIMER_set_BATT_BOOT_DPD();
            dsm_meter_sw_timer_start(MT_SW_GENERAL_four_TO, FALSE, 2000);
        }
    } /* while(1) */
}

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/

// clang-format off
void dsm_meter_initialize(void)
{
    uint8_t err;

    meter_event = OSFlagCreate(0x00, &err);
    ASSERT(meter_event);

#if 0 /* bccho, 2023-07-07 */
    OSTaskCreateExt(
        "meter", 
        dsm_meter_task_main, 
        (void *)0,
        &meter_task_stack[METER_TASK_STACK_SIZE - 1],   /* 사용안함 */
        TASK_PRI_METER, 
        TASK_METER,                                     /* 사용안함 */
        meter_task_stack,                               /* 사용안함 */
        METER_TASK_STACK_SIZE, 
        (void *)0,                                      /* 사용안함 */
        0                                               /* 사용안함 */
    );
#else
    OSTaskCreateExt(
        "meter", 
        dsm_meter_task_main, 
        (void *)0,
        0,   /* 사용안함 */
        TASK_PRI_METER, 
        TASK_METER,                                     /* 사용안함 */
        0,                                              /* 사용안함 */
        METER_TASK_STACK_SIZE + 1048, /* bccho, 2023-08-17, 스택 늘림 */ 
        (void *)0,                                      /* 사용안함 */
        0                                               /* 사용안함 */
    );
#endif    

    dsm_sw_timer_init(
        &meter_task_timer_cntx, 
        meter_task_timer_list,
        MT_SW_TIMER_MAX, 
        10, 
        meter_event);
}
// clang-format on