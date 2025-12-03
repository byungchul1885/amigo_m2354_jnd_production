#include "main.h"
#include "amg_pwr.h"
#include "eadc_vbat.h"

char* dsm_pwr_lowpwr_string(uint32_t idx)
{
    switch (idx)
    {
    case PWR_SHUTDOWN:
        return "SHUTDOWN";
    case PWR_STANDBY:
        return "STANDBY";
    case PWR_STOP_2:
        return "STOP_2";
    case PWR_STOP_1:
        return "STOP_1";
    case PWR_LP_SLEEP:
        return "LP_SLEEP";
    case PWR_SLEEP_RANGE_2:
        return "SLEEP_RANGE_2";
    case PWR_SLEEP_RANGE_1:
        return "SLEEP_RANGE_1";

    default:
        return "LowPwr_Unknown";
    }
}

uint32_t dsm_bor_idx2level(uint32_t level_idx)
{
    uint32_t set_bor_level = 0;
#if 0 /* bccho, 2023-07-20 */
    switch (level_idx)
    {
    case MT_BOR_LEVEL_IDX0_1_7v:
        set_bor_level = OB_BOR_LEVEL_0;
        break;
    case MT_BOR_LEVEL_IDX1_2_0v:
        set_bor_level = OB_BOR_LEVEL_1;
        break;
    case MT_BOR_LEVEL_IDX2_2_2v:
        set_bor_level = OB_BOR_LEVEL_2;
        break;
    case MT_BOR_LEVEL_IDX3_2_5v:
        set_bor_level = OB_BOR_LEVEL_3;
        break;
    case MT_BOR_LEVEL_IDX4_2_8v:
        set_bor_level = OB_BOR_LEVEL_4;
        break;
    default:
        set_bor_level = OB_BOR_LEVEL_3;
        break;
    }
#endif

    return set_bor_level;
}

/* BOR will hold a PIC® MCU in reset when the Vdd drops below a brown out
 * threshold voltage. Not all devices have BOR, but most do, and some have
 * multiple voltage thresholds to select from. Between a BOR and Power On Reset
 * the whole range of startup voltages can be covered to protect for proper
 * operation after a power drop at the Vdd line. It is also recommended that the
 * Power-Up Timer (PWRT) be enabled to increase the delay in returning from a
 * BOR event. */
uint32_t dsm_bor_level2idx(uint32_t level)
{
    uint32_t set_bor_level = 0;

#if 0 /* bccho, 2023-07-20 */
    switch (level)
    {
    case OB_BOR_LEVEL_0:
        set_bor_level = MT_BOR_LEVEL_IDX0_1_7v;
        break;
    case OB_BOR_LEVEL_1:
        set_bor_level = MT_BOR_LEVEL_IDX1_2_0v;
        break;
    case OB_BOR_LEVEL_2:
        set_bor_level = MT_BOR_LEVEL_IDX2_2_2v;
        break;
    case OB_BOR_LEVEL_3:
        set_bor_level = MT_BOR_LEVEL_IDX3_2_5v;
        break;
    case OB_BOR_LEVEL_4:
        set_bor_level = MT_BOR_LEVEL_IDX4_2_8v;
        break;
    default:
        set_bor_level = MT_BOR_LEVEL_IDX3_2_5v;
        break;
    }
#endif

    return set_bor_level;
}

void dsm_sag_port_init(void)
{
    GPIO_SetMode(PB, BIT0, GPIO_MODE_INPUT);

#ifdef ADD_DC_LOW_PIN /* bccho, 2024-09-11 */
    GPIO_SetMode(PA, BIT10, GPIO_MODE_INPUT);
#endif
}

float dsm_bat_volts(void)
{
#if 1 /* bccho, POWER, 2023-07-15 */
    float vbat_float = get_vbat_adc();
    // printf("vat: %f\n", vbat_float);
#else
    uint32_t uwConvertedValue;
    uint32_t uwInputVoltage;
    ADC_HandleTypeDef* p_adc = &Adc_PwrBat_Handle;
    ADC_ChannelConfTypeDef config;
    uint32_t vbat_u32 = 0;

    dsm_sag_port_init();

    config.Channel = ADC_CHANNEL_VBAT;
    config.Rank = ADC_REGULAR_RANK_1;
    config.SamplingTime = ADC_SAMPLETIME_24CYCLES_5;
    config.SingleDiff = ADC_SINGLE_ENDED;
    config.OffsetNumber = ADC_OFFSET_NONE;
    config.Offset = 0;
    if (HAL_ADC_ConfigChannel(p_adc, &config) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADC_Start(p_adc) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_ADC_PollForConversion(p_adc, 10) != HAL_OK)
    {
        Error_Handler();
    }

    if ((HAL_ADC_GetState(p_adc) & HAL_ADC_STATE_REG_EOC) ==
        HAL_ADC_STATE_REG_EOC)
    {
        uwConvertedValue = HAL_ADC_GetValue(p_adc);
        uwInputVoltage = uwConvertedValue * 3300;
        uwInputVoltage = uwInputVoltage / 4095;
        vbat_u32 = uwInputVoltage;
        vbat_float = ((float)(vbat_u32 * 3) / 1000);
    }
    dsm_pwr_bat_adc_deinit();
#endif /* bccho */

    return vbat_float;
}

void dsm_pwr_enter_low_pwrmode(uint32_t type)
{
#if 1 /* bccho, POWER, 2023-07-15 */
    goto_loader_S();
#else
    switch (type)
    {
    case PWR_SHUTDOWN:
        HAL_PWREx_EnterSHUTDOWNMode();

        break;
    case PWR_STANDBY:
        HAL_PWR_EnterSTANDBYMode();

        break;
    case PWR_STOP_2:
        HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);

        break;
    case PWR_STOP_1:
        HAL_PWREx_EnterSTOP1Mode(PWR_STOPENTRY_WFI);

        break;
    case PWR_LP_SLEEP:
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

        break;
    case PWR_SLEEP_RANGE_2:
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

        break;
    case PWR_SLEEP_RANGE_1:
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        break;
    default:

        break;
    }
#endif /* bccho */
}

void dsm_pwr_run_mode_entry(uint32_t type, uint32_t wakeup_src)
{
    switch (type)
    {
    case PWR_LP_RUN:

        break;
    case PWR_MP_RUN:

        break;
    case PWR_HP_RUN:
    default:

        break;
    }
}
