/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "options_sel.h"
#include "main.h"
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
#include "defines.h"
#include "amg_utc_util.h"
#include "disp.h"
#include "amg_power_mnt.h"
#include "platform.h"
#include "batmode.h"
#include "amg_wdt.h"
#include "amg_media_mnt.h"
#include "amg_modemif_prtl.h"
#include "mx25r4035f.h"
#include "amg_stock_op_mode.h"
#include "amg_push_datanoti.h"
#include "amg_imagetransfer.h"
#include "eeprom_at24cm02.h"
#include "comm.h"
#include "amg_spi.h"
#include "program.h"
#include "disp.h"
#include "boot_restore.h"
#include "rtc.h"
#include "amg_secu_main.h"
#ifdef M2354_CAN /* bccho, 2023-11-28 */
#include "amg_can.h"
#include "amg_isotp_user.h"
#endif
/*
******************************************************************************
*   Definition
******************************************************************************
*/
#define _D "[PMNT] "

#if 0
#define GPIO_A_PINS (0xFFFF & (~PWR_SENSE_Pin))
#define GPIO_B_PINS (0xFFFF)
#define GPIO_C_PINS (0xFFFF & (~(KEY_MENU_Pin | LCD_VLCD_Pin)))
#define GPIO_D_PINS (0xFFFF)
#define GPIO_E_PINS (0xFFFF & (~(RLY_SET_Pin | RLY_RESET_Pin)))
#define GPIO_F_PINS (0xFFFF)
#define GPIO_G_PINS (0xFFFF)

#else
#define GPIO_A_PINS (0xFFFF)
#define GPIO_B_PINS (0xFFFF & (~(LCD_VLCD_Pin)))
#define GPIO_C_PINS (0xFFFF)
#define GPIO_D_PINS (0xFFFF)
#define GPIO_E_PINS (0xFFFF & (~(KEY_MENU_Pin)))
#define GPIO_F_PINS (0xFFFF)
#define GPIO_G_PINS (0xFFFF & (~(RLY_SET_Pin | RLY_RESET_Pin)))
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

uint32_t g_pmnt_op_mode = PMNT_POWER_OFF;
uint32_t g_pmnt_wakeup_dn_evt = 0;

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
*   FUNCTIONS - extern
******************************************************************************
*/
extern void dsm_dm_debug_uart_rx_cb(void);

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/

char* dsm_pmnt_op_mode_string(uint32_t idx)
{
    switch (idx)
    {
    case PMNT_ACTIVE_OP:
        return "ACTIVE_OP";
    case PMNT_NO_VOLT_SET_OP:
        return "NO_VOLT_SET_OP";
    case PMNT_NO_VOLT_OP:
        return "NO_VOLT_OP";
    case PMNT_STOCK_OP:
        return "STOCK_OP";
    case PMNT_LOW_POWER_OP:
        return "LOW_POWER_OP";
    case PMNT_POWER_OFF:
        return "POWER_OFF";

    default:
        return "PMNT_Unknown";
    }
}

char* dsm_pmnt_wakeup_dn_string(uint32_t wakeup_src)
{
    switch (wakeup_src)
    {
    case WUP_S_NONE:
        return "WUP_NONE";
    case WUP_S_RTC_ALA:
        return "WUP_RTC_ALA";
    case WUP_S_RTC_ALB:
        return "WUP_RTC_ALB";
    case WUP_S_MENU_KEY:
        return "WUP_MENU_KEY";
    case WUP_S_MOVE_KEY:
        return "WUP_MOVE_KEY";
    case WUP_S_CAN_IF:
        return "WUP_CAN_IF";
    case WUP_S_I_MDM_IF:
        return "WUP_I_MDM_IF";
    case WUP_S_E_MDM_IF:
        return "WUP_E_MDM_IF";
    case WUP_S_MIF_IF:
        return "WUP_MIF_IF";
    case WUP_S_SYSTICK:
        return "WUP_SYSTICK";
    case WUP_S_RS485IF:
        return "WUP_RS485IF";
    case WUP_S_OS_HW_TIMER:
        return "WUP_OS_HW_TIMER";
    case WUP_S_METER_TIMER:
        return "WUP_METER_TIMER";
    case WUP_S_MAIN_POWER:
        return "WUP_MAIN_POWER";
    case WUP_S_BAT_POWER:
        return "WUP_BAT_POWER";
    case WUP_S_DM_IF:
        return "WUP_DM_IF";
    case WUP_S_ALL:
        return "WUP_ALL";
    case WDN_S_NOVOLTSET_COMM_x:
        return "WDN_S_NOVOLTSET_COMM_x";
    case WDN_S_NOVOLTSET_DISC:
        return "WDN_S_NOVOLTSET_DISC";
    case WDN_S_LOW_PWR_ENTRY:
        return "WDN_LOW_PWR_ENTRY";

    default:
        return "WUPDN_S_Unknown";
    }
}

void dsm_pmnt_set_op_mode(uint32_t op)
{
    if (g_pmnt_op_mode != op)
    {
        DPRINTF(DBG_WARN, _D "_FSM: Change OP [%s] -> [%s]\r\n",
                dsm_pmnt_op_mode_string(g_pmnt_op_mode),
                dsm_pmnt_op_mode_string(op));
        g_pmnt_op_mode = op;
    }
}

uint32_t dsm_pmnt_get_op_mode(void) { return g_pmnt_op_mode; }

void dsm_pmnt_set_wakeup_dn_evt(uint32_t wakeup_dn_evt)
{
    bool is_not_lpm = true;
    if (dsm_pmnt_get_op_mode() == PMNT_LOW_POWER_OP)
    {
        is_not_lpm = false;
    }

    if (wakeup_dn_evt == WUP_S_RTC_ALA ||
        /* wakeup_dn_evt == WUP_S_RTC_ALB ||*/ wakeup_dn_evt ==
            WUP_S_MAIN_POWER ||
        wakeup_dn_evt == WUP_S_MENU_KEY || wakeup_dn_evt == WUP_S_BAT_POWER ||
        wakeup_dn_evt == WDN_S_LOW_PWR_ENTRY)
    {
        g_pmnt_wakeup_dn_evt |= wakeup_dn_evt;
        if (is_not_lpm)
        {
            if (WUP_S_DM_IF != wakeup_dn_evt)
            {
                if (wakeup_dn_evt == WDN_S_LOW_PWR_ENTRY)
                {
                    DPRINTF(DBG_WARN,
                            _D
                            "_W_DN EVT occur: [%s] -> remain_evt[0x%08X]\r\n",
                            dsm_pmnt_wakeup_dn_string(wakeup_dn_evt),
                            g_pmnt_wakeup_dn_evt);
                }
                else
                {
                    DPRINTF(DBG_WARN,
                            _D
                            "_W_UP EVT occur: [%s] -> remain_evt[0x%08X]\r\n",
                            dsm_pmnt_wakeup_dn_string(wakeup_dn_evt),
                            g_pmnt_wakeup_dn_evt);
                }
            }
        }
        return;
    }
    else if (PMNT_NO_VOLT_OP == dsm_pmnt_get_op_mode())
    {
        if (wakeup_dn_evt == WUP_S_MOVE_KEY)
        {
            g_pmnt_wakeup_dn_evt |= wakeup_dn_evt;
            if (is_not_lpm)
            {
                DPRINTF(DBG_TRACE,
                        _D "_W_CHG EVT occur: [%s] -> remain_evt[0x%08X]\r\n",
                        dsm_pmnt_wakeup_dn_string(wakeup_dn_evt),
                        g_pmnt_wakeup_dn_evt);
            }
        }
        return;
    }
    else if (PMNT_NO_VOLT_SET_OP == dsm_pmnt_get_op_mode())
    {
        if (wakeup_dn_evt == WDN_S_NOVOLTSET_COMM_x ||
            wakeup_dn_evt == WDN_S_NOVOLTSET_DISC)
        {
            g_pmnt_wakeup_dn_evt |= wakeup_dn_evt;
            if (is_not_lpm)
            {
                DPRINTF(DBG_TRACE,
                        _D "_W_DN EVT occur: [%s] -> remain_evt[0x%08X]\r\n",
                        dsm_pmnt_wakeup_dn_string(wakeup_dn_evt),
                        g_pmnt_wakeup_dn_evt);
            }
        }
        return;
    }
}

void dsm_pmnt_clear_wakeup_dn_evt(uint32_t wakeup_dn_evt)
{
    g_pmnt_wakeup_dn_evt &= !(wakeup_dn_evt);
    DPRINTF(DBG_TRACE, _D "_W_UP_DN EVT clear: [%s] -> remain_evt[0x%08X]\r\n",
            dsm_pmnt_wakeup_dn_string(wakeup_dn_evt), g_pmnt_wakeup_dn_evt);
}

uint32_t dsm_pmnt_get_wakeup_dn_evt(void)
{
    /* enum EN_WAKEUP_DN_SOURCE */
    return g_pmnt_wakeup_dn_evt;
}

void dsm_pmnt_alarm_a_callback(void)
{
    if (PMNT_LOW_POWER_OP == dsm_pmnt_get_op_mode())
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_RTC_ALA);
}

void dsm_pmnt_alarm_b_callback(void)
{
    if (PMNT_LOW_POWER_OP == dsm_pmnt_get_op_mode())
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_RTC_ALB);
}

void dsm_pmnt_dm_if_callback(void)
{
    if (PMNT_LOW_POWER_OP == dsm_pmnt_get_op_mode())
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_DM_IF);
}

void dsm_pmnt_485_if_callback(void)
{
    if (PMNT_LOW_POWER_OP == dsm_pmnt_get_op_mode())
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_RS485IF);
}

void dsm_pmnt_imdm_if_callback(void)
{
    if (PMNT_LOW_POWER_OP == dsm_pmnt_get_op_mode())
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_I_MDM_IF);
}

void dsm_pmnt_emdm_if_callback(void)
{
    if (PMNT_LOW_POWER_OP == dsm_pmnt_get_op_mode())
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_E_MDM_IF);
}

void dsm_pmnt_mif_callback(void)
{
    if (PMNT_LOW_POWER_OP == dsm_pmnt_get_op_mode())
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_MIF_IF);
}

void dsm_pmnt_menu_key_callback(void)
{
    if (PMNT_LOW_POWER_OP == dsm_pmnt_get_op_mode())
    {
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_MENU_KEY);
    }
}

void dsm_pmnt_main_power_callback(void)
{
    if (PMNT_LOW_POWER_OP == dsm_pmnt_get_op_mode())
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_MAIN_POWER);
}

void dsm_pmnt_uart_if_callback(uint32_t port)
{
    switch (port)
    {
    case EMODEM_PORT:
        dsm_pmnt_emdm_if_callback();
        break;
    case IMODEM_PORT:
        dsm_pmnt_imdm_if_callback();
        break;
    case DEBUG_COM:
        dsm_pmnt_dm_if_callback();
        break;
    case MIF_PORT:
        dsm_pmnt_mif_callback();
        break;
    case RS485_PORT:
        dsm_pmnt_485_if_callback();
        break;
    default:

        break;
    }
}

void dsm_pmnt_move_key_callback_at_NO_VOLT_OP(void)
{
    if (PMNT_NO_VOLT_OP == dsm_pmnt_get_op_mode())
    {
        dsm_pmnt_set_wakeup_dn_evt(WUP_S_MOVE_KEY);
        dsm_pmnt_etc_evt_send();
    }
}

void dsm_pmnt_comm_rx_x_at_NO_VOLT_SET_OP(void)
{
    if (PMNT_NO_VOLT_SET_OP == dsm_pmnt_get_op_mode())
    {
        DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
        dsm_pmnt_set_wakeup_dn_evt(WDN_S_NOVOLTSET_COMM_x);
        dsm_pmnt_etc_evt_send();
    }
}

void dsm_pmnt_disc_o_at_NO_VOLT_SET_OP(void)
{
    if (PMNT_NO_VOLT_SET_OP == dsm_pmnt_get_op_mode())
    {
        DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
        OSTimeDly(OS_MS2TICK(300));
        dsm_pmnt_set_wakeup_dn_evt(WDN_S_NOVOLTSET_DISC);
        dsm_pmnt_etc_evt_send();
    }
}

uint32_t g_sec_timer_start_for_novoltsetop = 0;

void novoltsetop_update_sec_for_timestart(uint32_t sec)
{
    DPRINTF(DBG_TRACE, _D "%s sec[%d]\r\n", __func__, sec);
    g_sec_timer_start_for_novoltsetop = sec;
}

uint32_t novoltsetop_get_sec_for_timestart(void)
{
    return g_sec_timer_start_for_novoltsetop;
}

uint32_t novoltsetop_get_sec_timeout(void)
{
    return (MT_TIMEOUT_PMNT_NO_VOLT_SET_OP_RX_FRAME_TIME / 1000);
}

void dsm_pmnt_batpwr_noVolt_noVoltSet_proc(void)
{
    uint32_t evt = 0;
    uint32_t op = dsm_pmnt_get_op_mode();

    switch (op)
    {
    case PMNT_NO_VOLT_OP:
        evt = dsm_pmnt_get_wakeup_dn_evt();
        if (evt & WUP_S_MOVE_KEY)
        {
            uint32_t chg_op = dsm_pmnt_fsm(evt);

            if (PMNT_NO_VOLT_SET_OP == chg_op)
            {
                dsm_pmnt_peri_initialize(chg_op);

                dsm_meter_sw_timer_stop(MT_SW_TIMER_PMNT_NO_VOLT_SET_OP_TO);
                dsm_meter_sw_timer_start(
                    MT_SW_TIMER_PMNT_NO_VOLT_SET_OP_TO, FALSE,
                    MT_TIMEOUT_PMNT_NO_VOLT_SET_OP_RX_FRAME_TIME);  // 5 Min

                novoltsetop_update_sec_for_timestart(dsm_rtc_get_time());
                DPRINTF(DBG_TRACE,
                        _D
                        "NoVoltProc: TIMER START - No-Voltage Setup Mode %d "
                        "sec\r\n",
                        novoltsetop_get_sec_for_timestart());
            }
        }

        break;
    case PMNT_NO_VOLT_SET_OP:
        evt = dsm_pmnt_get_wakeup_dn_evt();
        if ((evt & WDN_S_NOVOLTSET_COMM_x) || (evt & WDN_S_NOVOLTSET_DISC))
        {
            DPRINTF(DBG_TRACE,
                    _D "NoVoltProc: Exit from No-Voltage Setup Mode\r\n");
        }
        break;
    }
}

uint32_t dsm_pmnt_fsm(uint32_t wakeup_dn_evt)
{
    uint32_t op = dsm_pmnt_get_op_mode();
    uint32_t wake_evt = dsm_pmnt_get_wakeup_dn_evt();

    DPRINTF(DBG_WARN, _D "_FSM: OP[%s], EVT[%s]\r\n",
            dsm_pmnt_op_mode_string(op),
            dsm_pmnt_wakeup_dn_string(wakeup_dn_evt));

    switch (op)
    {
    case PMNT_ACTIVE_OP:
        if ((wake_evt & WDN_S_LOW_PWR_ENTRY))
        {
            dsm_pmnt_set_op_mode(PMNT_LOW_POWER_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WUP_S_BAT_POWER))
        {
            dsm_pmnt_set_op_mode(PMNT_NO_VOLT_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else
        {
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }

        break;
    case PMNT_NO_VOLT_SET_OP:
        if ((wake_evt & WDN_S_LOW_PWR_ENTRY))
        {
            dsm_pmnt_set_op_mode(PMNT_LOW_POWER_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WDN_S_NOVOLTSET_COMM_x))
        {
            dsm_pmnt_set_op_mode(PMNT_LOW_POWER_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WDN_S_NOVOLTSET_DISC))
        {
            dsm_pmnt_set_op_mode(PMNT_LOW_POWER_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WUP_S_MAIN_POWER))
        {
            dsm_pmnt_set_op_mode(PMNT_ACTIVE_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else
        {
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }

        break;
    case PMNT_NO_VOLT_OP:
        if ((wake_evt & WDN_S_LOW_PWR_ENTRY))
        {
            dsm_pmnt_set_op_mode(PMNT_LOW_POWER_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WUP_S_MAIN_POWER))
        {
            dsm_pmnt_set_op_mode(PMNT_ACTIVE_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WUP_S_BAT_POWER))
        {
            dsm_pmnt_set_op_mode(PMNT_NO_VOLT_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WUP_S_MOVE_KEY))
        {
            dsm_pmnt_set_op_mode(PMNT_NO_VOLT_SET_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else
        {
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }

        break;
    case PMNT_STOCK_OP:
        if ((wake_evt & WDN_S_LOW_PWR_ENTRY))
        {
            dsm_pmnt_set_op_mode(PMNT_LOW_POWER_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else
        {
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }

        break;
    case PMNT_LOW_POWER_OP:
        if (wake_evt & WUP_S_RTC_ALA)
        {
            dsm_pmnt_set_op_mode(PMNT_STOCK_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if (wake_evt & WUP_S_RTC_ALB)
        {
            dsm_pmnt_set_op_mode(PMNT_STOCK_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WUP_S_MAIN_POWER))
        {
            dsm_pmnt_set_op_mode(PMNT_ACTIVE_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WUP_S_BAT_POWER))
        {
            dsm_pmnt_set_op_mode(PMNT_NO_VOLT_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WUP_S_MENU_KEY))
        {
            dsm_pmnt_set_op_mode(PMNT_NO_VOLT_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WDN_S_LOW_PWR_ENTRY))
        {
            dsm_pmnt_set_op_mode(PMNT_LOW_POWER_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else
        {
            dsm_pmnt_set_op_mode(PMNT_STOCK_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }

        break;
    case PMNT_POWER_OFF:
        if ((wake_evt & WUP_S_MAIN_POWER))
        {
            dsm_pmnt_set_op_mode(PMNT_ACTIVE_OP);
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else if ((wake_evt & WUP_S_BAT_POWER))
        {
#if 1 /* bccho, 2023-09-15 */
            dsm_pmnt_set_op_mode(PMNT_NO_VOLT_OP);
#else
            dsm_pmnt_set_op_mode(PMNT_STOCK_OP);
#endif
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }
        else
        {
            dsm_pmnt_clear_wakeup_dn_evt(wakeup_dn_evt);
        }

        break;
    default:
        break;
    }

    op = dsm_pmnt_get_op_mode();

    return op;
}

void dsm_pmnt_get_wakup_datetime(DATE_TIME_T* p_w_dt)
{
    cur_rtc_update();

    p_w_dt->year = cur_rtc.year + BASE_YEAR;
    p_w_dt->month = cur_rtc.month;
    p_w_dt->day = cur_rtc.date;
    p_w_dt->hour = cur_rtc.hour;
    p_w_dt->min = cur_rtc.min;
    p_w_dt->sec = cur_rtc.sec;

    DPRINTF(DBG_WARN, _D "%s: %d.%02d.%02d  %02d:%02d:%02d\r\n", __func__,
            p_w_dt->year, p_w_dt->month, p_w_dt->day, p_w_dt->hour, p_w_dt->min,
            p_w_dt->sec);
}

void dsm_pmnt_disable_wakeup_int(void)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
#if 0
    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_2);
    NVIC_DisableIRQ(EXTI2_IRQn);
#endif

#if 0  /* bccho, POWER, 2023-07-15 */
    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_6);
    NVIC_DisableIRQ(EXTI9_5_IRQn);
#endif /* bccho */
}

#if 0
void EXTI2_IRQHandler(void)
{
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);

        dsm_pmnt_main_power_callback();
    }
}
#endif

#if 0
void EXTI15_10_IRQHandler(void)
{
    if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_13);
        dsm_pmnt_menu_key_callback();
    }
}
#endif

void dsm_peri_io_deinit(void)
{
#if 0  /* bccho, POWER, 2023-07-15 */    
    HAL_GPIO_DeInit(GPIOA, GPIO_A_PINS);
    HAL_GPIO_DeInit(GPIOB, GPIO_B_PINS);
    HAL_GPIO_DeInit(GPIOC, GPIO_C_PINS);
    HAL_GPIO_DeInit(GPIOD, GPIO_D_PINS);
    HAL_GPIO_DeInit(GPIOE, GPIO_E_PINS);
    HAL_GPIO_DeInit(GPIOF, GPIO_F_PINS);
    HAL_GPIO_DeInit(GPIOG, GPIO_G_PINS);
#endif /* bccho */
}

bool dsm_pmnt_power_save_func(uint32_t lowpwr, uint32_t w_evt)
{
#if 1 /* bccho, POWER, 2023-07-15 */
    dsm_printf("\r\n\r\n\r\n");
    DPRINTF(DBG_WARN, _D "[[[[[[[[[[  LOW_POWER_ENTER_SETUP  ]]]]]]]]]]\r\n");

    dsm_gpio_imodem_pf_low(); /* PF set */
    dsm_gpio_imodem_power_disable();

#ifdef STOCK_OP /* bccho, 2024-09-26 */
    /* wakeup alarm 등록 --> 염병, 상태를 저장해야 한다*/
    dsm_stock_op_proc_for_lowpower_entry(w_evt);
#endif

#if 1 /* bccho, 2023-08-29, FMC */
    BOOT_RESTORE br;
#ifdef STOCK_OP /* bccho, 2024-09-26 */
    ST_RAND_TX_INFO* p_rand_txinfo = dsm_stock_op_get_rand_txinfo();
    br.fsm = dsm_stock_op_get_fsm();
    br.sec_1 = p_rand_txinfo->sec_1;
    br.ms_1 = p_rand_txinfo->ms_1;
    br.sec_2 = p_rand_txinfo->sec_2;
    br.ms_2 = p_rand_txinfo->ms_2;
    br.seed = p_rand_txinfo->seed;
#endif
    br.bank = get_current_bank_S();
    br.goto_dpd = true;

    if (FMC_Erase_S(BOOT_RESTORE_BASE) != 0)
    {
        MSGERROR("FMC_Erase Data Flash\n");
        return FALSE;
    }
    if (FMC_WriteMultiple_S(BOOT_RESTORE_BASE, (uint32_t*)&br,
                            sizeof(BOOT_RESTORE)) <= 0)
    {
        MSGERROR("FMC_WriteMultiple_S\n");
        return FALSE;
    }
    MSG06("W__%d %d %d %d %d %d", br.fsm, br.sec_1, br.ms_1, br.sec_2, br.ms_2,
          br.seed);
#endif

    DPRINTF(DBG_WARN, _D "[[[[[[[[[[  ENTER LOW_POWER_MODE  ]]]]]]]]]]\r\n");
    MSG07("Enter DPD");

    dsm_pwr_enter_low_pwrmode(lowpwr);

    return TRUE;
#endif /* bccho */
}

void dsm_pmnt_power_restore_func(void)
{
#if 0  /* bccho, POWER, 2023-07-15 */
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_LPUART1_CLK_ENABLE();
    __HAL_RCC_UART5_CLK_ENABLE();
    __HAL_RCC_UART4_CLK_ENABLE();
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_LCD_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();
    //__HAL_RCC_I2C4_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_ADC_CLK_ENABLE();
#endif /* bccho */
}

uint32_t dsm_pmnt_EntryLowPwr_n_waitingForwakeup(uint32_t lowpwr,
                                                 uint32_t w_evt)
{
#if 1 /* bccho, 2023-08-22 */
    m_ac_power_set_high();

    if (dsm_pmnt_power_save_func(lowpwr, w_evt) == FALSE)
    {
        return PMNT_ACTIVE_OP;
    }

    return PMNT_ACTIVE_OP;
#endif /* bccho */
}

void dsm_pmnt_peri_initialize(uint32_t op)
{
    DPRINTF(DBG_WARN, "%s: %s\r\n", __func__, dsm_pmnt_op_mode_string(op));
    MSG07("%s: %s", __func__, dsm_pmnt_op_mode_string(op));

    uint32_t reset_cause = get_reset_cause_S();
    if (reset_cause & SYS_RSTSTS_BODRF_Msk)
    {
        MSG06("___BODRF");
    }
    if (reset_cause & SYS_RSTSTS_CPURF_Msk)
    {
        MSG06("___CPURF");
    }
    if (reset_cause & SYS_RSTSTS_LVRF_Msk)
    {
        MSG06("___LVRF");
    }
    if (reset_cause & SYS_RSTSTS_PORF_Msk)
    {
        MSG06("___PORF");
    }
    if (reset_cause & SYS_RSTSTS_PINRF_Msk)
    {
        MSG06("___PINRF");
    }
    if (reset_cause & SYS_RSTSTS_SYSRF_Msk)
    {
        MSG06("___SYSRF");
    }
    if (reset_cause & SYS_RSTSTS_WDTRF_Msk)
    {
        MSG06("___WDTRF");
    }

    /*
        전원리셋           : PORF | PINRF | SYSRF
        NuLink reset       : PORF | SYSRF
        SYS_ResetCPU_S     : CPURF
        NVIC_SystemReset_S : SYSRF
        SYS_ResetChip_S    : PORF | SYSRF
    */

    if (op == PMNT_ACTIVE_OP)
    {
        dsm_mif_init();
        dsm_eeprom_init();
        mx25r4035f_init();
        dsm_modemif_baud_road();
        dsm_imodemif_init(TRUE);
        dsm_gpio_imodem_io_init(__IMODEM_PF_NORMAL_OP);
        dsm_emodemif_init();
        dsm_lcd_init();
        dsm_gpio_sw_cover_port_init();
        dsm_gpio_key_port_init();
        dsm_gpio_MAGNETIC_GPIO_init();
        dsm_gpio_battery_GPIO_init();
        dsm_gpio_eoi_port_init();
        dsm_gpio_pulse_ctrl_port_init();
        dsm_gpio_pulse_ctrl_2_port_init();
        dsm_gpio_e_modem_io_init();
        whm_init();
        dsm_rtc_init();
        cur_rtc_update();
        cur_rtc_restore_batmode();
        dsm_mtp_init();

        /* bccho, 2024-09-05, 삼상 */
        volt_drop_thrd_set();

        dsm_mif_pwrfail_port_init();
#ifdef STOCK_OP /* bccho, 2024-09-26 */
        dsm_stock_op_init();
#endif
        meter_task_init();
        dsm_media_if_init();
        dsm_data_push_init();
        pwr_rtn_proc();
        dsm_sec_initialize();

        M_MT_SW_GENERAL_TIMER_set_bootup_setup();
        dsm_meter_sw_timer_start(MT_SW_GENERAL_TO, FALSE,
                                 MT_TIMEOUT_BOOTUP_SETUP_TIME);

        dsm_mif_getreq_firmware_ver_data();
    }
    /* 무전압설정: 무전압검침 --> 메뉴키 2초 */
    else if (op == PMNT_NO_VOLT_SET_OP)
    {
        dsm_modemif_baud_road();
        dsm_imodemif_init(FALSE); /* FALSE: sun listen */
        dsm_media_if_init();
        dsm_gpio_imodem_io_init(__IMODEM_PF_NORMAL_OP);
        dsp_on_sun_dsp_set();

#if 0 /* bccho, 2025-04-09 */
        dsm_secu_module_init(); /* bccho, 2023-12-01 */
#else
        hls_sec_init();
#endif
    }
    /* 무전압검침:
       1. DPD에서 메뉴버튼  2. 배터리로 부팅  */
    else if (op == PMNT_NO_VOLT_OP)
    {
#if 0 /* bccho, 2023-12-15 */
        dsm_timer_init(HW_METER_TIMER);
#endif
        dsm_lcd_init();
        dsm_eeprom_init();
        whm_init();
        dsm_rtc_init();
        cur_rtc_update();
        cur_rtc_restore_batmode();
        dsm_mtp_init();

        /* bccho, 2024-09-05, 삼상 */
        volt_drop_thrd_set();

        meter_task_init();
#ifdef STOCK_OP /* bccho, 2024-09-26 */
        dsm_stock_op_init();
#endif
        dsm_data_push_init();
        dsm_sec_initialize();
    }
#ifdef STOCK_OP /* bccho, 2024-09-26 */
    /* 재고관리. DPD에서 RTC 알람으로 부팅 */
    else if (op == PMNT_STOCK_OP)
    {
        MSG06("############   PMNT_STOCK_OP");

#if 0 /* bccho, 2023-12-15 */
        dsm_timer_init(HW_METER_TIMER);
#endif
        dsm_eeprom_init();
#if 0 /* bccho, 2023-11-25, 아래로 대체 */
        dsm_gpio_imodem_io_init(__IMODEM_PF_NORMAL_OP);
#else
        GPIO_SetMode(PC, BIT7, GPIO_MODE_OUTPUT); /* KSE_PWR */
        GPIO_SetMode(PF, BIT9, GPIO_MODE_OUTPUT); /* LMN_PF */
        GPIO_SetMode(PG, BIT4, GPIO_MODE_OUTPUT); /* LMN_MODEM_RESET */

        PF9 = 0; /* pf_low */
        PG4 = 0; /* reset_low */
        PC7 = 0; /* power_disable */
#endif
        dsm_rtc_init();
        cur_rtc_update();
        meter_task_init();
        whm_init();

#if 1 /* bccho, 2023-08-29, FMC */
        BOOT_RESTORE br;
        if (FMC_ReadBytes_S(BOOT_RESTORE_BASE, (uint32_t*)&br,
                            sizeof(BOOT_RESTORE)) != 0)
        {
            MSGERROR("FMC_ReadBytes_S from Data Flash\n");
        }
        MSG06("R__%d %d %d %d %d %d", br.fsm, br.sec_1, br.ms_1, br.sec_2,
              br.ms_2, br.seed);

        if (br.fsm == 0xFFFFFFFF)
        {
            MSGERROR("Boot in PMNT_STOCK_OP, but no data in DATA FLASH\n");
            dsm_stock_op_init();

            memset((uint8_t*)&br, 0x00, sizeof(BOOT_RESTORE));
            if (FMC_Erase_S(BOOT_RESTORE_BASE) != 0)
            {
                MSGERROR("FMC_Erase Data Flash\n");
            }
            if (FMC_WriteMultiple_S(BOOT_RESTORE_BASE, (uint32_t*)&br,
                                    sizeof(BOOT_RESTORE)) <= 0)
            {
                MSGERROR("FMC_WriteMultiple_S\n");
            }
        }
        else
        {
            dsm_stock_set_fsm(br.fsm);
            ST_RAND_TX_INFO* p_rand_txinfo = dsm_stock_op_get_rand_txinfo();
            p_rand_txinfo->sec_1 = br.sec_1;
            p_rand_txinfo->ms_1 = br.ms_1;
            p_rand_txinfo->sec_2 = br.sec_2;
            p_rand_txinfo->ms_2 = br.ms_2;
            p_rand_txinfo->seed = br.seed;
        }

        uint32_t fsm = dsm_stock_op_get_fsm();
        uint32_t wait_time = 0;
        switch (fsm)
        {
        case STOCK_FSM_NONE:
            MSGERROR("why!!!!__1");
            break;
        case STOCK_FSM_WAKE_DEFAULT: /* 20:00에 깨어남 */
            wait_time = 1;
            MSG07("Defaut Alarm--> wait time: %d msec", wait_time);
            break;
        case STOCK_FSM_WAKE_ALA_1: /* 첫번째 알람 */
            wait_time = (br.sec_1 % 60) * 1000 + br.ms_1;
            MSG07("1st Alarm--> wait time: %d msec", wait_time);
            break;
        case STOCK_FSM_WAKE_ALA_2: /* 두번째 알람 */
            wait_time = (br.sec_2 % 60) * 1000 + br.ms_2;
            MSG07("2nd Alarm--> wait time: %d msec", wait_time);
            break;
        default:
            MSGERROR("why!!!!__2");
            break;
        }

        M_MT_SW_GENERAL_four_TIMER_set_wait_stock_op();
#ifdef STOCK_OP_NO_WAIT
        dsm_meter_sw_timer_start(MT_SW_GENERAL_four_TO, FALSE, 1);
#else
        dsm_meter_sw_timer_start(MT_SW_GENERAL_four_TO, FALSE, wait_time);
#endif

#endif
    }
#endif

    if (op == PMNT_ACTIVE_OP)
    {
        phol_nv_read(PERIODIC_HOL);
        phol_nv_read(nPERIODIC_HOL);
    }
}
