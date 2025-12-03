#if !defined(__AMG_METER_MAIN_H__)
#define __AMG_METER_MAIN_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/
typedef enum
{
    MT_SW_TIMER_CAL_GETREQ_TO,
    MT_SW_TIMER_FWUP_RESTART_FOR_MODEM_TO,
    MT_SW_TIMER_ATCMD_TX_TO,
    MT_SW_TIMER_MIF_MTP_TX_TO,
    MT_SW_TIMER_PUSH_LP_TO,
    MT_SW_TIMER_I_MODEM_RX_INTER_FRAME_TO,
    MT_SW_TIMER_E_MODEM_RX_INTER_FRAME_TO,
    MT_SW_TIMER_485_MODEM_RX_INTER_FRAME_TO,
    MT_SW_TIMER_CAN_MODEM_RX_INTER_FRAME_TO,
    MT_SW_TIMER_PMNT_NO_VOLT_SET_OP_TO,
    MT_SW_TIMER_ASSO_TO,
    MT_SW_GENERAL_TO,
    MT_SW_GENERAL_two_TO,
    MT_SW_GENERAL_three_TO,
    MT_SW_GENERAL_four_TO, /* bccho, 2023-10-03 */
    MT_SW_TIMER_ASSO_4_485_TO,
    MT_SW_TIMER_MAX
} EN_MT_TASK_SW_TIMER;

#define MT_TIMEOUT_MS_CAL_GETREQ_AFTER_CALST_ACK_TIME (10 * 1000)
#define MT_TIMEOUT_MS_CAL_GETREQ_RETRY_TIME (1000)
#define MT_TIMEOUT_FWUP_RESTART_TIME (60 * 60 * 1000)  // 1 hour
#define MT_TIMEOUT_AT_CMD_TX_TIME (3 * 1000)           // 3 sec
#define MT_TIMEOUT_MIF_MTP_TX_TIME (3 * 1000)          // 3 sec
#define MT_TIMEOUT_RX_INTER_FRAME_TIME (1 * 1000 + 5 * 100)
#define MT_TIMEOUT_PMNT_NO_VOLT_SET_OP_RX_FRAME_TIME (5 * 60 * 1000)  // 5 min
#define MT_TIMEOUT_BOOTUP_SETUP_TIME (3000)
#define MT_TIMEOUT_IMG_UPDATE_GO_TIME (3 * 1000)
#define MT_TIMEOUT_ASSO_TIME (2 * 60 * 60 * 1000)
#define MT_TIMEOUT_ASSO_4_485_TIME (5 * 60 * 1000)  // 5 min

/* General Timer#1 ---------------------------------*/
#define MT_SW_GENERAL_TO__BOOTUP_SETUP (1 << 0)
#define MT_SW_GENERAL_TO__LEGACY_MT_TOU_TRANSFER (1 << 1)
#define MT_SW_GENERAL_TO__LEGACY_MT_TOU_TRANSFER_2 (1 << 3)
#define MT_SW_GENERAL_TO__DLMS_CLIENT_TX_RETRY (1 << 2)
#define MT_SW_GENERAL_TO__VIRTUAL_BACKOFF (1 << 4)
#define MT_SW_GENERAL_TO__IMG_UPDATE_GO (1 << 5)

extern uint32_t g_mt_sw_general_timer_bits;
#define M_MT_SW_GENERAL_TIMER_is_bootup_setup() \
    (bool)(g_mt_sw_general_timer_bits & MT_SW_GENERAL_TO__BOOTUP_SETUP)
#define M_MT_SW_GENERAL_TIMER_is_legacy_mt_tou_transfer() \
    (bool)(g_mt_sw_general_timer_bits &                   \
           MT_SW_GENERAL_TO__LEGACY_MT_TOU_TRANSFER)
#define M_MT_SW_GENERAL_TIMER_is_legacy_mt_tou_transfer_2() \
    (bool)(g_mt_sw_general_timer_bits &                     \
           MT_SW_GENERAL_TO__LEGACY_MT_TOU_TRANSFER_2)
#define M_MT_SW_GENERAL_TIMER_is_dlms_client_tx_retry() \
    (bool)(g_mt_sw_general_timer_bits & MT_SW_GENERAL_TO__DLMS_CLIENT_TX_RETRY)
#define M_MT_SW_GENERAL_TIMER_is_image_update_go() \
    (bool)(g_mt_sw_general_timer_bits & MT_SW_GENERAL_TO__IMG_UPDATE_GO)

#define M_MT_SW_GENERAL_TIMER_set_bootup_setup() \
    (g_mt_sw_general_timer_bits |= MT_SW_GENERAL_TO__BOOTUP_SETUP)
#define M_MT_SW_GENERAL_TIMER_set_legacy_mt_tou_transfer() \
    (g_mt_sw_general_timer_bits |= MT_SW_GENERAL_TO__LEGACY_MT_TOU_TRANSFER)
#define M_MT_SW_GENERAL_TIMER_set_legacy_mt_tou_transfer_2() \
    (g_mt_sw_general_timer_bits |= MT_SW_GENERAL_TO__LEGACY_MT_TOU_TRANSFER_2)
#define M_MT_SW_GENERAL_TIMER_set_dlms_client_tx_retry() \
    (g_mt_sw_general_timer_bits |= MT_SW_GENERAL_TO__DLMS_CLIENT_TX_RETRY)
#define M_MT_SW_GENERAL_TIMER_set_image_update_go() \
    (g_mt_sw_general_timer_bits |= MT_SW_GENERAL_TO__IMG_UPDATE_GO)

#define M_MT_SW_GENERAL_TIMER_clear_bootup_setup() \
    (g_mt_sw_general_timer_bits &= (~MT_SW_GENERAL_TO__BOOTUP_SETUP))
#define M_MT_SW_GENERAL_TIMER_clear_legacy_mt_tou_transfer() \
    (g_mt_sw_general_timer_bits &= (~MT_SW_GENERAL_TO__LEGACY_MT_TOU_TRANSFER))
#define M_MT_SW_GENERAL_TIMER_clear_legacy_mt_tou_transfer_2() \
    (g_mt_sw_general_timer_bits &=                             \
     (~MT_SW_GENERAL_TO__LEGACY_MT_TOU_TRANSFER_2))
#define M_MT_SW_GENERAL_TIMER_clear_dlms_client_tx_retry() \
    (g_mt_sw_general_timer_bits &= (~MT_SW_GENERAL_TO__DLMS_CLIENT_TX_RETRY))
#define M_MT_SW_GENERAL_TIMER_clear_image_update_go() \
    (g_mt_sw_general_timer_bits &= (~MT_SW_GENERAL_TO__IMG_UPDATE_GO))

#define MT_TIMEOUT_METER_RESET_MAX (5 * 1000)

/* General Timer#2 ---------------------------------*/
#define MT_SW_GENERAL_two_TO__VIR_SCAN_MAX_TIME (1 << 0)
#define MT_SW_GENERAL_two_TO__METER_RESET (1 << 1)

extern uint32_t g_mt_sw_general_timer_two_bits;
#define M_MT_SW_GENERAL_two_TIMER_is_meter_reset() \
    (bool)(g_mt_sw_general_timer_two_bits & MT_SW_GENERAL_two_TO__METER_RESET)
#define M_MT_SW_GENERAL_two_TIMER_set_meter_reset() \
    (g_mt_sw_general_timer_two_bits |= MT_SW_GENERAL_two_TO__METER_RESET)
#define M_MT_SW_GENERAL_two_TIMER_clear_meter_reset() \
    (g_mt_sw_general_timer_two_bits &= (~MT_SW_GENERAL_two_TO__METER_RESET))

/* General Timer#3 ---------------------------------*/
#define MT_SW_GENERAL_three_TO__ZCD_OUT (1 << 0)

extern uint32_t g_mt_sw_general_timer_three_bits;
#define M_MT_SW_GENERAL_three_TIMER_is_zcd_out() \
    (bool)(g_mt_sw_general_timer_three_bits & MT_SW_GENERAL_three_TO__ZCD_OUT)
#define M_MT_SW_GENERAL_three_TIMER_set_zcd_out() \
    (g_mt_sw_general_timer_three_bits |= MT_SW_GENERAL_three_TO__ZCD_OUT)
#define M_MT_SW_GENERAL_three_TIMER_clear_zcd_out() \
    (g_mt_sw_general_timer_three_bits &= (~MT_SW_GENERAL_three_TO__ZCD_OUT))

/* bccho, 2023-10-03 */
/* General Timer#4 ---------------------------------*/
#define MT_SW_GENERAL_four_TO__WAIT_STOCK_OP (1 << 0)
#define MT_SW_GENERAL_four_TO__BATT_BOOT_DPD (1 << 1)

extern uint32_t g_mt_sw_general_timer_four_bits;

#define M_MT_SW_GENERAL_four_TIMER_is_wait_stock_op() \
    (bool)(g_mt_sw_general_timer_four_bits &          \
           MT_SW_GENERAL_four_TO__WAIT_STOCK_OP)
#define M_MT_SW_GENERAL_four_TIMER_set_wait_stock_op() \
    (g_mt_sw_general_timer_four_bits |= MT_SW_GENERAL_four_TO__WAIT_STOCK_OP)
#define M_MT_SW_GENERAL_four_TIMER_clear_wait_stock_op() \
    (g_mt_sw_general_timer_four_bits &= (~MT_SW_GENERAL_four_TO__WAIT_STOCK_OP))

#define M_MT_SW_GENERAL_four_TIMER_is_BATT_BOOT_DPD() \
    (bool)(g_mt_sw_general_timer_four_bits &          \
           MT_SW_GENERAL_four_TO__BATT_BOOT_DPD)
#define M_MT_SW_GENERAL_four_TIMER_set_BATT_BOOT_DPD() \
    (g_mt_sw_general_timer_four_bits |= MT_SW_GENERAL_four_TO__BATT_BOOT_DPD)
#define M_MT_SW_GENERAL_four_TIMER_clear_BATT_BOOT_DPD() \
    (g_mt_sw_general_timer_four_bits &= (~MT_SW_GENERAL_four_TO__BATT_BOOT_DPD))

/*
******************************************************************************
*	MACRO
******************************************************************************
*/

/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/
extern uint8_t major_ver;
extern uint8_t minor_ver;

#if 1 /* bccho, 2023-08-25 */
extern uint32_t wakeup_source;
#endif

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/
void dsm_meter_initialize(void);
void dsm_meter_sw_timer_start(uint32_t timerid, uint32_t periodic,
                              uint32_t interval_ms);
void dsm_meter_sw_timer_stop(uint32_t timerid);
void dsm_meter_if_rx_inter_frame_timer_stop(void);
void dsm_meter_int_cb(void);
void dsm_mif_rx_int_cb(void);
void dsm_485if_uart_rx_int_cb(void);
void dsm_can_rx_int_cb(void);
void dsm_imodem_rx_int_cb(void);
void dsm_emodem_rx_int_cb(void);
void dsm_fwup_fsm_send(void);
void dsm_mtp_fsm_send(void);
uint32_t dsm_get_dm_out_measure_print_chkcount(void);
void dsm_data_noti_errcode_evt_send(void);
void dsm_data_noti_lastLP_evt_send(void);
void dsm_pmnt_etc_evt_send(void);
#if 1 /* bccho, 2023-11-30 */
void dsm_sec_module_initialized(void);
#endif
#endif /* __AMG_METER_MAIN_H__ */
