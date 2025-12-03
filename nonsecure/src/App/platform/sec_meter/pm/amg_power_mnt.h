#if !defined(__AMG_POWER_MNT_H__)
#define __AMG_POWER_MNT_H__

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
    PMNT_ACTIVE_OP,      /* AC ON */
    PMNT_NO_VOLT_SET_OP, /* 무전압 설정 */
    PMNT_NO_VOLT_OP,     /* 무전압 검침 */
    PMNT_STOCK_OP,       /* 재고 관리 */
    PMNT_LOW_POWER_OP,   /* STOP2 모드 수행중 */
    PMNT_POWER_OFF,      /* 부팅 시 초기값 */
} EN_PMNT_OP_MODE;

typedef enum
{
    WUP_S_NONE = 0,
    WUP_S_RTC_ALA = (1 << 0),
    WUP_S_RTC_ALB = (1 << 1),
    WUP_S_MENU_KEY = (1 << 2),
    WUP_S_MOVE_KEY = (1 << 3),
    WUP_S_CAN_IF = (1 << 4),
    WUP_S_I_MDM_IF = (1 << 5),
    WUP_S_E_MDM_IF = (1 << 6),
    WUP_S_MIF_IF = (1 << 7),
    WUP_S_SYSTICK = (1 << 8),
    WUP_S_RS485IF = (1 << 9),
    WUP_S_OS_HW_TIMER = (1 << 10),
    WUP_S_METER_TIMER = (1 << 11),
    WUP_S_MAIN_POWER = (1 << 12),
    WUP_S_BAT_POWER = (1 << 13),
    WUP_S_DM_IF = (1 << 14),
    WUP_S_ALL = 0x0FFFFFFF,
    WDN_S_NOVOLTSET_COMM_x = (1 << 28),
    WDN_S_NOVOLTSET_DISC = (1 << 29),
    WDN_S_LOW_PWR_ENTRY = 0x80000000

} EN_WAKEUP_DN_SOURCE;

#define WUP_S_LOWPWR (WUP_S_RTC_ALA | WUP_S_RTC_ALB | WUP_S_MENU_KEY)

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

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/
char* dsm_pmnt_op_mode_string(uint32_t idx);
char* dsm_pmnt_wakeup_dn_string(uint32_t wakeup_src);
void dsm_pmnt_set_op_mode(uint32_t op);
uint32_t dsm_pmnt_get_op_mode(void);
void dsm_pmnt_set_wakeup_dn_evt(uint32_t wakeup_dn_evt);
void dsm_pmnt_clear_wakeup_dn_evt(uint32_t wakeup_dn_evt);
uint32_t dsm_pmnt_get_wakeup_dn_evt(void);
void dsm_pmnt_alarm_a_callback(void);
void dsm_pmnt_alarm_b_callback(void);
void dsm_pmnt_uart_if_callback(uint32_t port);
void dsm_pmnt_move_key_callback_at_NO_VOLT_OP(void);
void dsm_pmnt_disc_o_at_NO_VOLT_SET_OP(void);
void dsm_pmnt_comm_rx_x_at_NO_VOLT_SET_OP(void);
void novoltsetop_update_sec_for_timestart(uint32_t sec);
uint32_t novoltsetop_get_sec_for_timestart(void);
uint32_t novoltsetop_get_sec_timeout(void);
void dsm_pmnt_batpwr_noVolt_noVoltSet_proc(void);
uint32_t dsm_pmnt_fsm(uint32_t wakeup_dn_evt);
void dsm_pmnt_peri_initialize(uint32_t op);
uint32_t dsm_pmnt_EntryLowPwr_n_waitingForwakeup(uint32_t lowpwr,
                                                 uint32_t w_evt);
bool dsm_pmnt_power_save_func(uint32_t lowpwr, uint32_t w_evt);

#endif /* __AMG_POWER_MNT_H__ */
