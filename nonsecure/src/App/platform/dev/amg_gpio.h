#ifndef __AMG_GPIO_H__
#define __AMG_GPIO_H__
/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "options_sel.h"
/*
******************************************************************************
*	DEFINES
******************************************************************************
*/

#define SW_OPEN_STATE !PB14 /* 0: Cover Open, 1: Cover Closed */
#define IS_AC_ON PB0        /* 1이면 정상, 0이면 power fail */
#define IS_AC_OFF !PB0
#define MAGN_SENS_STATE PD12

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/
#define KEY_MENU_0 0
#define KEY_MOVE_1 1

#define KEY_CAL_3 3

typedef enum
{
    RS_485_ON_ZCD_OFF,
    RS_485_OFF_ZCD_ON,
} EN_RS485_ZCD_SEL_STATE;

typedef enum
{
    BTN_NONE = 0,
    BTN_SHORT = 1,
    BTN_DOUBLE = 2,
    BTN_LONG = 3
} BTN_STATE;

typedef enum
{
    BTN_FSM_IDLE,
    BTN_FSM_CHECK_LONG_PRESS,
    BTN_FSM_CHECK_DOUBLE_PRESS_1,
    BTN_FSM_CHECK_DOUBLE_PRESS_2,
    BTN_FSM_WAIT_UNPRESS
} BTN_FSM;

typedef struct
{
    int interval;
    BTN_STATE state;
    uint32_t bnc_cnt;
    BTN_FSM fsm;
    uint32_t gpio;
    uint32_t long_key_time_ms;
    void (*cb)(BTN_STATE sts);
} BUTTON_CNTX;

/*
******************************************************************************
*	GLOBAL VARIABLS;
******************************************************************************
*/
extern BUTTON_CNTX factoryt_btn;

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/
void dsm_gpio_key_port_init(void);
void dsm_gpio_MAGNETIC_GPIO_init(void);
void dsm_gpio_battery_GPIO_init(void);
void dsm_gpio_pulse_ctrl_port_init(void);
void dsm_gpio_pulse_ctrl_2_port_init(void);
void dsm_gpio_meter_ic_rst_port_init(void);
void dsm_gpio_relay_port_init(void);
#if 1 /* bccho, 2023-08-04 */
void dsm_gpio_wdt_ext_disable(void);
#endif
void dsm_gpio_sw_cover_port_init(void);
void dsm_gpio_485_sel_port_init(void);
void dsm_gpio_485_en_port_init(void);
void dsm_gpio_e2p_pwr_en_port_init(void);
void dsm_gpio_spi_e2p_pwr_en_port_init(void);
void dsm_gpio_button_poll_proc(BUTTON_CNTX *p_cntx);
BTN_STATE dsm_gpio_button_get(BUTTON_CNTX *p_cntx);
void dsm_gpio_button_reg_callback(BUTTON_CNTX *p_cntx, void (*func)(BTN_STATE));
void dsm_gpio_relay_on(void);
void dsm_gpio_relay_off(void);
uint8_t dsm_rs485_zcd_get_state(void);
void dsm_zcd_on_485_off(void);
void dsm_485_on_zcd_off(void);
void dsm_gpio_tx_en_enable(void);
void dsm_gpio_tx_en_disable(void);
uint32_t dsm_gpio_get_keyinput(uint32_t key_name);
void dsm_gpio_imodem_wdt_pin_init(void);
void dsm_gpio_imodem_io_init(uint32_t pf_fail);
void dsm_gpio_imodem_power_enable(void);
void dsm_gpio_imodem_power_disable(void);
void dsm_gpio_imodem_pf_high(void);
void dsm_gpio_imodem_pf_low(void);
void dsm_gpio_imodem_reset_high(void);
void dsm_gpio_imodem_reset_low(void);
void dsm_gpio_metering_ic_reset_high(void);
void dsm_gpio_metering_ic_reset_low(void);
void dsm_gpio_e_modem_io_init(void);
void dsm_gpio_e_modem_reset_high(void);
void dsm_gpio_e_modem_reset_low(void);
void dsm_gpio_spi_eeprom_pwr_enable(void);
void dsm_gpio_eoi_port_init(void);
void dsm_eoi_1_disable(void);
void dsm_eoi_1_led_on(void);
void dsm_eoi_1_led_off(void);
void dsm_eoi_2_disable(void);
void dsm_eoi_2_led_on(void);
void dsm_eoi_2_led_off(void);
void dsm_eoi_1_disable_pulse_out(void);
void dsm_eoi_2_disable_pulse_out(void);
void dsm_pulse_w_off(void);
void dsm_pulse_v_off(void);

#endif  //__AMG_GPIO_H__
