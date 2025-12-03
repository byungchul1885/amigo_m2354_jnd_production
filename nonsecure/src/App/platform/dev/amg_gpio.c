/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_gpio.h"
#include "whm.h"
#include "whm_1.h"
#include "amg_modemif_prtl.h"
#include "softimer.h"

#include "amg_stock_op_mode.h"
#include "port.h"

#define _D "[GPIO] "

#ifdef M2354_NEW_HW
#define M_PWR PE13
#else
#define M_PWR PB9
#endif
/*
******************************************************************************
* 	MCARO
******************************************************************************
*/
#define BTN_POLL_INTERVAL_MS (OS_MS_FOR_ONE_TICK)
#define BTN_PROC_INTERVAL_MS (10)
#define BTN_TIME_MS(a) ((a) / BTN_PROC_INTERVAL_MS)

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

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/
BUTTON_CNTX factoryt_btn;
uint8_t g_rs485_zcd_state = 0;

/*
******************************************************************************
*	LOCAL FUNCTION
******************************************************************************
*/
static void dsm_gpio_button_state_change(BUTTON_CNTX* p_cntx, BTN_STATE state)
{
    p_cntx->state = state;
    if (p_cntx->cb)
    {
        (*p_cntx->cb)(state);
    }
}

/*
******************************************************************************
*	GLOBAL FUNCTION
******************************************************************************
*/

void dsm_gpio_MAGNETIC_GPIO_init(void)
{
    MSG07("dsm_gpio_MAGNETIC_GPIO_init()");
    GPIO_SetMode(PD, BIT12, GPIO_MODE_INPUT);

    // GPIO_InitTypeDef GPIO_InitStruct;

    // __HAL_RCC_GPIOF_CLK_ENABLE();

    // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // GPIO_InitStruct.Pin = MAGN_SENS_Pin;
    // HAL_GPIO_Init(MAGN_SENS_GPIO_Port, &GPIO_InitStruct);
}

void dsm_gpio_battery_GPIO_init(void)
{
    MSG07("dsm_gpio_battery_GPIO_init()");
    GPIO_SetMode(PC, BIT13, GPIO_MODE_INPUT);

    // GPIO_InitTypeDef GPIO_InitStruct;

    // __HAL_RCC_GPIOF_CLK_ENABLE();

    // GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    // GPIO_InitStruct.Pull = GPIO_NOPULL;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    // GPIO_InitStruct.Pin = MAGN_SENS_Pin;
    // HAL_GPIO_Init(MAGN_SENS_GPIO_Port, &GPIO_InitStruct);
}

void dsm_gpio_key_port_init(void)
{
#if 1 /* bccho, 2023-08-04 */
    MSG05("dsm_gpio_key_port_init()");
    GPIO_SetMode(PB, BIT2, GPIO_MODE_INPUT); /* 메뉴키 */
#ifdef M2354_NEW_HW
    GPIO_SetMode(PB, BIT3, GPIO_MODE_INPUT); /* 이동키*/
#else
    GPIO_SetMode(PB, BIT12, GPIO_MODE_INPUT); /* 이동키*/
#endif
#else  /* bccho */
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = KEY_MENU_Pin;
    HAL_GPIO_Init(KEY_MENU_GPIO_Port, &GPIO_InitStruct);

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin = KEY_MOVE_Pin;
    HAL_GPIO_Init(KEY_MOVE_GPIO_Port, &GPIO_InitStruct);
#endif /* bccho */
}

void dsm_gpio_eoi_port_init(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG05("dsm_gpio_eoi_port_init()");
    GPIO_SetMode(PH, BIT6, GPIO_MODE_INPUT);
    GPIO_SetMode(PH, BIT4, GPIO_MODE_INPUT);
#endif
#else
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOG_CLK_ENABLE();

    if (LL_PWR_IsEnabledVddIO2() == RESET)
    {
        LL_PWR_EnableVddIO2();
    }

    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = EOI_1_Pin | EOI_2_Pin;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
#endif /* bccho */
}

void dsm_gpio_pulse_ctrl_port_init(void)
{
#if 1 /* bccho, 2023-09-09, Todo */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_gpio_pulse_ctrl_port_init()");
    GPIO_SetMode(PH, BIT7, GPIO_MODE_OUTPUT);
    PH7 = 1;
#endif
#else
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOG_CLK_ENABLE();

    if (LL_PWR_IsEnabledVddIO2() == RESET)
    {
        LL_PWR_EnableVddIO2();
    }

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = PULSE_CTRL_1_Pin;
    HAL_GPIO_Init(PULSE_CTRL_1_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(PULSE_CTRL_1_GPIO_Port, PULSE_CTRL_1_Pin,
                      GPIO_PIN_SET);  // 1
#endif
}

void dsm_gpio_pulse_ctrl_2_port_init(void)
{
#if 1 /* bccho, 2023-09-09, Todo */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_gpio_pulse_ctrl_2_port_init()");
    GPIO_SetMode(PH, BIT5, GPIO_MODE_OUTPUT);
    PH5 = 1;
#endif
#else
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOF_CLK_ENABLE();

    if (LL_PWR_IsEnabledVddIO2() == RESET)
    {
        LL_PWR_EnableVddIO2();
    }

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = PULSE_CTRL_2_Pin;
    HAL_GPIO_Init(PULSE_CTRL_2_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(PULSE_CTRL_2_GPIO_Port, PULSE_CTRL_2_Pin,
                      GPIO_PIN_SET);  // 1//
#endif /* bccho */
}

void dsm_gpio_load_sw_mr_port_init(void) {}

void dsm_gpio_meter_ic_rst_port_init(void) {}

void dsm_gpio_relay_port_init(void)
{
#if 1 /* bccho, 2023-08-24 */
    MSG05("dsm_gpio_relay_port_init()");
#ifdef M2354_NEW_HW
    PA12 = 0;
    PA13 = 0;
    GPIO_SetMode(PA, BIT13, GPIO_MODE_OUTPUT); /* reset */
    GPIO_SetMode(PA, BIT12, GPIO_MODE_OUTPUT); /* set*/
#else
    PF6 = 0;
    PF7 = 0;
    GPIO_SetMode(PF, BIT6, GPIO_MODE_OUTPUT); /* reset */
    GPIO_SetMode(PF, BIT7, GPIO_MODE_OUTPUT); /* set*/
#endif
#else  /* bccho. STM32 */
    HAL_GPIO_WritePin(RLY_RESET_GPIO_Port, RLY_SET_Pin | RLY_RESET_Pin,
                      GPIO_PIN_RESET);

    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = RLY_RESET_Pin | RLY_SET_Pin;
    HAL_GPIO_Init(RLY_RESET_GPIO_Port, &GPIO_InitStruct);
#endif /* bccho */
}

#if 1 /* bccho, 2023-08-04 */
void dsm_gpio_wdt_ext_disable(void)
{
#ifdef M2354_NEW_HW
#else
    MSG01("dsm_gpio_wdt_ext_disable()");
    GPIO_SetMode(PE, BIT11, GPIO_MODE_OUTPUT);
    PE11 = 1;
#endif
}
#endif

void dsm_gpio_sw_cover_port_init(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    MSG05("dsm_gpio_sw_cover_port_init()");
    GPIO_SetMode(PB, BIT14, GPIO_MODE_INPUT);
#else  /* bccho */
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOF_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = SW_COVER1_Pin;
    HAL_GPIO_Init(SW_COVER1_GPIO_Port, &GPIO_InitStruct);
#endif /* bccho */
}

void dsm_gpio_485_sel_port_init(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    MSG05("dsm_gpio_485_sel_port_init()");
    GPIO_SetMode(PD, BIT9, GPIO_MODE_OUTPUT);
    PD9 = 0;
#else  /* bccho */
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOE_CLK_ENABLE();

    if (LL_PWR_IsEnabledVddIO2() == RESET)
    {
        LL_PWR_EnableVddIO2();
    }

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = RS485_SELECT_Pin;
    HAL_GPIO_Init(RS485_SELECT_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(RS485_SELECT_GPIO_Port, RS485_SELECT_Pin,
                      GPIO_PIN_RESET);  // 0
#endif /* bccho */
}

void dsm_gpio_485_en_port_init(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    MSG05("dsm_gpio_485_en_port_init()");
    GPIO_SetMode(PD, BIT8, GPIO_MODE_OUTPUT);

    PD8 = 0; /* Tx disable */
#else        /* bccho */
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = RS485_EN_Pin;
    HAL_GPIO_Init(RS485_EN_GPIO_Port, &GPIO_InitStruct);
#endif       /* bccho */
}

void dsm_gpio_e2p_pwr_en_port_init(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    MSG03("dsm_gpio_e2p_pwr_en_port_init()");
#ifdef M2354_NEW_HW
    GPIO_SetMode(PE, BIT13, GPIO_MODE_OUTPUT);
#else
    GPIO_SetMode(PB, BIT9, GPIO_MODE_OUTPUT);
#endif
#else
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = EEPROM_PWR_EN_Pin;
    HAL_GPIO_Init(EEPROM_PWR_EN_Port, &GPIO_InitStruct);
#endif /* bccho */
}

void dsm_gpio_spi_e2p_pwr_en_port_init(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    MSG06("dsm_gpio_spi_e2p_pwr_en_port_init()");
#ifdef M2354_NEW_HW
    GPIO_SetMode(PE, BIT13, GPIO_MODE_OUTPUT);
#else
    GPIO_SetMode(PB, BIT9, GPIO_MODE_OUTPUT);
#endif
#else
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = EEPROM_PWR_EN_Pin;
    HAL_GPIO_Init(EEPROM_PWR_EN_Port, &GPIO_InitStruct);
#endif
}

/* bccho, 2023-08-09, 사용하지 않음 */
void dsm_gpio_button_poll_proc(BUTTON_CNTX* p_cntx)
{
    uint32_t curstate;

    if (p_cntx->interval)
        p_cntx->interval -= BTN_POLL_INTERVAL_MS;
    if (p_cntx->interval > 0)
        return;
    p_cntx->interval = BTN_PROC_INTERVAL_MS;

#if 1 /* bccho, GPIO, 2023-07-15 */
    curstate = !PB2;
#else
    curstate = HAL_GPIO_ReadPin(KEY_MENU_GPIO_Port, p_cntx->gpio);
#endif /* bccho */

    switch (p_cntx->fsm)
    {
    case BTN_FSM_IDLE:
        if (curstate)
        {
            p_cntx->bnc_cnt++;
            if (p_cntx->bnc_cnt == BTN_TIME_MS(30))
            {
                p_cntx->fsm = BTN_FSM_CHECK_LONG_PRESS;
            }
        }
        else
        {
            p_cntx->bnc_cnt = 0;
        }
        break;

    case BTN_FSM_CHECK_LONG_PRESS:
        if (curstate)
        {
            p_cntx->bnc_cnt++;
            if (p_cntx->bnc_cnt == BTN_TIME_MS(p_cntx->long_key_time_ms))
            {
                p_cntx->fsm = BTN_FSM_WAIT_UNPRESS;
                dsm_gpio_button_state_change(p_cntx, BTN_LONG);
            }
        }
        else
        {
            p_cntx->fsm = BTN_FSM_CHECK_DOUBLE_PRESS_1;
            p_cntx->bnc_cnt = 0;
        }
        break;

    case BTN_FSM_CHECK_DOUBLE_PRESS_1:
        p_cntx->bnc_cnt++;

        if (!curstate)
        {
            if (p_cntx->bnc_cnt == BTN_TIME_MS(150))
            {
                p_cntx->fsm = BTN_FSM_IDLE;
                p_cntx->bnc_cnt = 0;
                dsm_gpio_button_state_change(p_cntx, BTN_SHORT);
            }
        }
        else
        {
            p_cntx->fsm = BTN_FSM_CHECK_DOUBLE_PRESS_2;
            p_cntx->bnc_cnt = 0;
        }
        break;

    case BTN_FSM_CHECK_DOUBLE_PRESS_2:
        if (curstate)
        {
            p_cntx->bnc_cnt++;
            if (p_cntx->bnc_cnt == BTN_TIME_MS(30))
            {
                p_cntx->fsm = BTN_FSM_WAIT_UNPRESS;
                dsm_gpio_button_state_change(p_cntx, BTN_DOUBLE);
            }
        }
        else
        {
            p_cntx->fsm = BTN_FSM_IDLE;
            p_cntx->bnc_cnt = 0;
            dsm_gpio_button_state_change(p_cntx, BTN_SHORT);
        }
        break;

    case BTN_FSM_WAIT_UNPRESS:
        if (!curstate)
        {
            p_cntx->fsm = 0;
            p_cntx->bnc_cnt = 0;
        }
        break;
    }
}

/* bccho, 2023-08-09, 사용하지 않음 */
BTN_STATE dsm_gpio_button_get(BUTTON_CNTX* p_cntx)
{
    BTN_STATE temp = p_cntx->state;

    p_cntx->state = 0;
    return temp;
}

/* bccho, 2023-08-09, 사용하지 않음 */
void dsm_gpio_button_reg_callback(BUTTON_CNTX* p_cntx, void (*func)(BTN_STATE))
{
    p_cntx->cb = func;
}

void dsm_gpio_relay_on(void)
{
    MSG06("dsm_gpio_relay_off()__start");
#if PHASE_NUM == THREE_PHASE
    PA13 = 0;
    PA13 = 1;
    CLK_SysTickLongDelay_S(100000);
    PA13 = 0;
#else
    PA12 = 0;
    PA12 = 1;
    CLK_SysTickLongDelay_S(100000);
    PA12 = 0;
#endif
    MSG06("dsm_gpio_relay_off()__end");
}

void dsm_gpio_relay_off(void)
{
    MSG06("dsm_gpio_relay_on()__start");
#if PHASE_NUM == THREE_PHASE
    PA12 = 0;
    PA12 = 1;
    CLK_SysTickLongDelay_S(100000);
    PA12 = 0;
#else
    PA13 = 0;
    PA13 = 1;
    CLK_SysTickLongDelay_S(100000);
    PA13 = 0;
#endif

    MSG06("dsm_gpio_relay_on()__end");
}

void dsm_rs485_zcd_set_state(uint8_t state) { g_rs485_zcd_state = state; }

uint8_t dsm_rs485_zcd_get_state(void) { return g_rs485_zcd_state; }

void dsm_gpio_analog_to_485_tx_set(void)
{
#if 0 /* bccho, 2023-09-09, Todo */
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
    GPIO_InitStruct.Pin = RS485_TXD_UART3_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
    LL_GPIO_Init(RS485_TXD_UART3_GPIO_Port, &GPIO_InitStruct);
#endif
}

void dsm_485_tx_to_gpio_analog_set(void)
{
#if 0 /* bccho, 2023-09-09, Todo */
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
    GPIO_InitStruct.Pin = RS485_TXD_UART3_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(RS485_TXD_UART3_GPIO_Port, &GPIO_InitStruct);
#endif
}

void dsm_zcd_on_485_off(void)
{
    /* MCU ZCD Pulse Output is Connect to RS-458 Transceiver DI (Data Input) */

#if 1 /* bccho, GPIO, 2023-07-15 */
    MSG05("dsm_zcd_on_485_off()");
    PD9 = 1;
#else
    HAL_GPIO_WritePin(RS485_SELECT_GPIO_Port, RS485_SELECT_Pin, GPIO_PIN_SET);
#endif /* bccho */
    dsm_485_tx_to_gpio_analog_set();

    dsm_rs485_zcd_set_state(RS_485_OFF_ZCD_ON);
}

void dsm_485_on_zcd_off(void)
{
    /* MCU RS485 TXD Output is Connect to RS-458 Transceiver DI (Data Input) */

#if 1 /* bccho, GPIO, 2023-07-15 */
    MSG05("dsm_485_on_zcd_off()");
    PD9 = 0;
#else
    HAL_GPIO_WritePin(RS485_SELECT_GPIO_Port, RS485_SELECT_Pin, GPIO_PIN_RESET);
#endif /* bccho */

    dsm_gpio_analog_to_485_tx_set();

    dsm_rs485_zcd_set_state(RS_485_ON_ZCD_OFF);
}

void dsm_pulse_v_on(void)
{
#if 1 /* bccho, 2023-09-09, Todo */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_pulse_v_on()");
    GPIO_SetMode(PH, BIT7, GPIO_MODE_OUTPUT); /* PULSE_CTRL */
    PH7 = 1;
#endif
#else
    LL_GPIO_SetPinMode(PULSE_CTRL_1_GPIO_Port, PULSE_CTRL_1_Pin,
                       LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetOutputPin(PULSE_CTRL_1_GPIO_Port, PULSE_CTRL_1_Pin);  // pulse
                                                                     // out
#endif
}
void dsm_pulse_w_on(void)
{
#if 1 /* bccho, 2023-09-09, Todo */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_pulse_w_on()");
    GPIO_SetMode(PH, BIT5, GPIO_MODE_OUTPUT); /* PULSE_CTRL2 */
    PH5 = 1;
#endif
#else
    LL_GPIO_SetPinMode(PULSE_CTRL_2_GPIO_Port, PULSE_CTRL_2_Pin,
                       LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetOutputPin(PULSE_CTRL_2_GPIO_Port, PULSE_CTRL_2_Pin);  // pulse
                                                                     // out
#endif
}

void dsm_pulse_v_off(void)
{
#if 1 /* bccho, 2023-09-09, Todo */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_pulse_v_off()");
    GPIO_SetMode(PH, BIT7, GPIO_MODE_OUTPUT); /* PULSE_CTRL */
    PH7 = 0;
#endif
#else
    LL_GPIO_SetPinMode(PULSE_CTRL_1_GPIO_Port, PULSE_CTRL_1_Pin,
                       LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(PULSE_CTRL_1_GPIO_Port,
                           PULSE_CTRL_1_Pin);  // pulse off
#endif
}

void dsm_pulse_w_off(void)
{
#if 1 /* bccho, 2023-09-09, Todo */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_pulse_w_on()");
    GPIO_SetMode(PH, BIT5, GPIO_MODE_OUTPUT); /* PULSE_CTRL2 */
    PH5 = 0;
#endif
#else
    LL_GPIO_SetPinMode(PULSE_CTRL_2_GPIO_Port, PULSE_CTRL_2_Pin,
                       LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(PULSE_CTRL_2_GPIO_Port,
                           PULSE_CTRL_2_Pin);  // pulse off
#endif
}

void dsm_eoi_1_disable(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_eoi_1_disable()");
    GPIO_SetMode(PH, BIT6, GPIO_MODE_INPUT); /* LED_EOI */
#endif
#else
    LL_GPIO_SetPinMode(EOI_1_GPIO_Port, EOI_1_Pin, LL_GPIO_MODE_ANALOG);
#endif /* bccho */
}

void dsm_eoi_1_disable_pulse_out(void)
{
#if 1 /* bccho, 2023-09-09, Todo */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_eoi_1_disable()");
    GPIO_SetMode(PH, BIT6, GPIO_MODE_INPUT); /* LED_EOI */
    dsm_pulse_v_on();
#endif
#else
    /* LED OFF */
    LL_GPIO_SetPinMode(EOI_1_GPIO_Port, EOI_1_Pin,
                       LL_GPIO_MODE_ANALOG);  // eoi disable
    dsm_pulse_w_on();
#endif
}

void dsm_eoi_1_led_on(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_eoi_1_led_on()");
    GPIO_SetMode(PH, BIT6, GPIO_MODE_OUTPUT); /* LED_EOI */
    GPIO_SetMode(PH, BIT7, GPIO_MODE_OUTPUT); /* PULSE_CTRL */

    PH6 = 0; /* LED_EOI */
    PH7 = 0; /* PULSE_CTRL*/
#endif
#else
    LL_GPIO_SetPinMode(EOI_1_GPIO_Port, EOI_1_Pin,
                       LL_GPIO_MODE_OUTPUT);             // eoi enable
    LL_GPIO_ResetOutputPin(EOI_1_GPIO_Port, EOI_1_Pin);  // 0
    LL_GPIO_ResetOutputPin(PULSE_CTRL_1_GPIO_Port,
                           PULSE_CTRL_1_Pin);  ////pulse off
#endif /* bccho */
}

void dsm_eoi_1_led_off(void)
{
    MSG06("dsm_eoi_1_led_off()");

    EOI_VPORT_DEACTIVE;    // LED OFF
    PULSE_VPORT_DEACTIVE;  // v pulse_oFF, EOI VAR SELECT
}

void dsm_eoi_2_disable(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_eoi_2_disable()");
    GPIO_SetMode(PH, BIT4, GPIO_MODE_INPUT); /* LED_EOI2 */
#endif
#else
    LL_GPIO_SetPinMode(EOI_2_GPIO_Port, EOI_2_Pin, LL_GPIO_MODE_ANALOG);
#endif /* bccho */
}

void dsm_eoi_2_disable_pulse_out(void)
{
#if 1 /* bccho, 2023-09-09, Todo */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_eoi_2_disable_pulse_out()");
    GPIO_SetMode(PH, BIT4, GPIO_MODE_INPUT); /* LED_EOI2 */
    dsm_pulse_w_on();
#endif
#else
    /* LED OFF */
    LL_GPIO_SetPinMode(EOI_2_GPIO_Port, EOI_2_Pin,
                       LL_GPIO_MODE_ANALOG);  // eoi disable
    dsm_pulse_v_on();
#endif
}
void dsm_eoi_2_led_on(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#if 0 /* bccho, PH 4,5,6,7 삭제, 2024-06-27 */
    MSG06("dsm_eoi_2_led_on()");

    /* 우측 LED, EOI#2 */
    GPIO_SetMode(PH, BIT4, GPIO_MODE_OUTPUT); /* LED_EOI2 */
    GPIO_SetMode(PH, BIT5, GPIO_MODE_OUTPUT); /* PULSE_CTRL2 */

    PH4 = 0; /* LED_EOI2 */
    PH5 = 0; /* PULSE_CTRL2 */
#endif
#else
    LL_GPIO_SetPinMode(EOI_2_GPIO_Port, EOI_2_Pin,
                       LL_GPIO_MODE_OUTPUT);             // eoi enable
    LL_GPIO_ResetOutputPin(EOI_2_GPIO_Port, EOI_2_Pin);  // 0
    LL_GPIO_ResetOutputPin(PULSE_CTRL_2_GPIO_Port,
                           PULSE_CTRL_2_Pin);  // pulse off
#endif /* bccho */
}

void dsm_eoi_2_led_off(void)
{
    MSG06("dsm_eoi_2_led_off()");

    EOI_WPORT_DEACTIVE;    // LED OFF
    PULSE_WPORT_DEACTIVE;  // v pulse_oFF, EOI VAR SELECT
}

void dsm_gpio_tx_en_enable(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    MSG05("dsm_gpio_tx_en_enable()");
    PD8 = 1;
#else
    HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_SET);
#endif /* bccho */
}

void dsm_gpio_tx_en_disable(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    MSG05("dsm_gpio_tx_en_disable()");
    PD8 = 0;
#else
    HAL_GPIO_WritePin(RS485_EN_GPIO_Port, RS485_EN_Pin, GPIO_PIN_RESET);
#endif /* bccho */
}

uint32_t dsm_gpio_get_keyinput(uint32_t key_name)
{
#if 1 /* bccho, 2023-08-04 */
    switch (key_name)
    {
    case KEY_MENU_0:
        MSG00("get_keyinput(), KEY_MENU_0, %d", !PB2);
        return !PB2;
    case KEY_MOVE_1:
#ifdef M2354_NEW_HW
        MSG00("get_keyinput(), KEY_MOVE_1, %d", !PB3);
        return !PB3;
#else
        MSG00("get_keyinput(), KEY_MOVE_1, %d", !PB12);
        return !PB12;
#endif
    default:
        return 0;
    }
#else
    GPIO_TypeDef* GPIOx = NULL;
    uint32_t pin = 0;

    switch (key_name)
    {
    case KEY_MENU_0:
        GPIOx = KEY_MENU_GPIO_Port;
        pin = KEY_MENU_Pin;

        break;
    case KEY_MOVE_1:
        GPIOx = KEY_MOVE_GPIO_Port;
        pin = KEY_MOVE_Pin;

        break;

    default:
        break;
    }

    return ((uint32_t)HAL_GPIO_ReadPin(GPIOx, pin));
#endif /* bccho */
}

void dsm_gpio_imodem_wdt_pin_init(void) {}

void dsm_gpio_imodem_io_init(uint32_t pf_fail)
{
    dsm_gpio_imodem_reset_low();
    dsm_gpio_imodem_power_disable();

    GPIO_SetMode(PC, BIT7, GPIO_MODE_OUTPUT); /* KSE_PWR */
    GPIO_SetMode(PF, BIT9, GPIO_MODE_OUTPUT); /* LMN_PF */
    GPIO_SetMode(PG, BIT4, GPIO_MODE_OUTPUT); /* LMN_MODEM_RESET */

    if (pf_fail == __IMODEM_PF_STOCK_OP)
    {
        dsm_gpio_imodem_pf_low(); /* PF set */
    }
    else
    {
#if 0 /* bccho, 2024-09-30 */
        dsm_gpio_imodem_pf_low(); /* PF set */
#else
        dsm_gpio_imodem_pf_high(); /* PF unset */
#endif
    }

    dsm_gpio_imodem_power_enable();
    dsm_gpio_imodem_reset_high();
}

void dsm_gpio_imodem_power_enable(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    PC7 = 1;
#else
    HAL_GPIO_WritePin(MODEM_PWR_EN_GPIO_Port, MODEM_PWR_EN_Pin, GPIO_PIN_RESET);
#endif /* bccho */
}

void dsm_gpio_imodem_power_disable(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    PC7 = 0;
#else
    HAL_GPIO_WritePin(MODEM_PWR_EN_GPIO_Port, MODEM_PWR_EN_Pin, GPIO_PIN_SET);
#endif /* bccho */
}

void dsm_gpio_imodem_pf_high(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#ifdef M2354_NEW_HW
    PF9 = 1;
#else
    PF8 = 1;
#endif
#else
    HAL_GPIO_WritePin(MODEM_PF_GPIO_Port, MODEM_PF_Pin, GPIO_PIN_SET);
#endif /* bccho */
}

void dsm_gpio_imodem_pf_low(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#ifdef M2354_NEW_HW
    PF9 = 0;
#else
    PF8 = 0;
#endif
#else
    HAL_GPIO_WritePin(MODEM_PF_GPIO_Port, MODEM_PF_Pin, GPIO_PIN_RESET);
#endif /* bccho */
}

void dsm_gpio_imodem_reset_high(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    PG4 = 1;
#else
    HAL_GPIO_WritePin(MODEM_RESET_GPIO_Port, MODEM_RESET_Pin, GPIO_PIN_SET);
#endif /* bccho */
}

void dsm_gpio_imodem_reset_low(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
    PG4 = 0;
#else
    HAL_GPIO_WritePin(MODEM_RESET_GPIO_Port, MODEM_RESET_Pin, GPIO_PIN_RESET);
#endif /* bccho */
}

void dsm_gpio_metering_ic_reset_high(void) {}

void dsm_gpio_metering_ic_reset_low(void) {}

void dsm_gpio_e_modem_io_init(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#ifdef M2354_NEW_HW
    /* PB1_MODEM_RESET */
    GPIO_SetMode(PB, BIT1, GPIO_MODE_OUTPUT);
#else
    /* PD3_MODEM_RESET */
    GPIO_SetMode(PB, BIT6, GPIO_MODE_OUTPUT);
#endif

    dsm_gpio_e_modem_reset_low();
    vTaskDelay(100);
    dsm_gpio_e_modem_reset_high();
#else
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOG_CLK_ENABLE();
    if (LL_PWR_IsEnabledVddIO2() == RESET)
    {
        LL_PWR_EnableVddIO2();
    }
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Pin = eMODEM_RESET_Pin;
    HAL_GPIO_Init(eMODEM_RESET_GPIO_Port, &GPIO_InitStruct);
    dsm_gpio_e_modem_reset_high();
#endif
}

void dsm_gpio_e_modem_reset_high(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#ifdef M2354_NEW_HW
    PB1 = 1;
#else
    PB6 = 1;
#endif
#else
    HAL_GPIO_WritePin(eMODEM_RESET_GPIO_Port, eMODEM_RESET_Pin, GPIO_PIN_SET);
#endif /* bccho */
}

void dsm_gpio_e_modem_reset_low(void)
{
#if 1 /* bccho, GPIO, 2023-07-15 */
#ifdef M2354_NEW_HW
    PB1 = 0;
#else
    PB6 = 0;
#endif
#else
    HAL_GPIO_WritePin(eMODEM_RESET_GPIO_Port, eMODEM_RESET_Pin, GPIO_PIN_RESET);
#endif /* bccho */
}

void dsm_gpio_spi_eeprom_pwr_enable(void)
{
    MSG00("dsm_gpio_spi_eeprom_pwr_enable()");
    M_PWR = 1;
}
