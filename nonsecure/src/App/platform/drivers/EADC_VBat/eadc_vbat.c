#include <stdio.h>
#include "NuMicro.h"
#include "amg_debug.h"
#include "FreeRTOS.h"
#include "task.h"

// #define TEST_PB14

static volatile uint32_t g_u32AdcIntFlag;
static bool inited = false;

void EADC0_IRQHandler(void)
{
    g_u32AdcIntFlag = 1;

    /* Clear the A/D ADINT0 interrupt flag */
    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);
}

void init_vbat_adc(void)
{
#if 0 /* bccho, 2024-1-2, 사용안함 */   
    MSG07("init_vbat_adc()");

    /* Set input mode as single-end and enable the A/D converter */
    EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);

#if VBAT_ADC
    /* Set sample module 18 external sampling time to 0xFF */
    EADC_SetExtendSampleTime(EADC, 18, 0xFF);
#endif

    inited = true;
#endif
}

float get_vbat_adc(void)
{
    int32_t i32ConversionData;
    uint32_t u32TimeOutCnt;

    if (!inited)
    {
        return 3.6F;
    }

    MSG00("get_vbat_adc()");

#if VBAT_ADC /* bccho, 2023-11-15, EADC 사용안함 */

    /* Clear the A/D ADINT0 interrupt flag for safe */
    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);

    /* Enable sample module A/D ADINT0 interrupt. */
    EADC_ENABLE_INT(EADC, BIT0);
    /* Enable sample module 18 interrupt. */
    EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, BIT18);
    NVIC_EnableIRQ(EADC0_IRQn);

    /* Reset the ADC interrupt indicator and trigger sample module 18 to start
     * A/D conversion */
    g_u32AdcIntFlag = 0;
    EADC_START_CONV(EADC, BIT18);

    /* Wait EADC conversion done */
    u32TimeOutCnt = SystemCoreClock; /* 1 second time-out */
    while (g_u32AdcIntFlag == 0)
    {
        if (--u32TimeOutCnt == 0)
        {
            printf("Wait for EADC conversion done time-out!\n");
            return 0.0F;
        }
    }

    /* Disable the ADINT0 interrupt */
    EADC_DISABLE_INT(EADC, BIT0);

    /* Get the conversion result of the sample module 18 */
    i32ConversionData = EADC_GET_CONV_DATA(EADC, 18);
    // printf("-----%d\n", i32ConversionData);

    float V_00 = 720.0F;
    float V_36 = 1130.0F;
    float one_step = 3.6F / (V_36 - V_00);
    float V_m = one_step * (i32ConversionData - V_00);

    if (V_m > 3.7F)
    {
        V_m = 0.0F;
    }

    return V_m;
#else

#ifdef TEST_PB14
    EADC_ConfigSampleModule(EADC, 7, EADC_ADINT0_TRIGGER, 14);
#else
    /* Configure the sample 7 module for analog input channel 15 and
     * enable ADINT0 trigger source */
    EADC_ConfigSampleModule(EADC, 7, EADC_ADINT0_TRIGGER, 15);
#endif

    /* Clear the A/D ADINT0 interrupt flag for safe */
    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);

    // Enable sample module A/D ADINT0 interrupt.
    EADC_ENABLE_INT(EADC, BIT0);

#ifdef TEST_PB14
    EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, BIT6);
#else
    // Enable sample module 7 interrupt.
    EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, BIT7);
#endif
    NVIC_EnableIRQ(EADC0_IRQn);

    /* Reset the ADC indicator and trigger sample module 7 to start A/D
     * conversion */
    g_u32AdcIntFlag = 0;

#ifdef TEST_PB14
    EADC_START_CONV(EADC, BIT6);
#else
    EADC_START_CONV(EADC, BIT7);
#endif

#if 0
    __WFI();
#else
    /* Wait EADC conversion done */
    u32TimeOutCnt = SystemCoreClock; /* 1 second time-out */
    while (g_u32AdcIntFlag == 0)
    {
        if (--u32TimeOutCnt == 0)
        {
            MSGERROR("[0] Wait for EADC conversion done time-out!");
            return 0.0F;
        }
    }
#endif

#ifdef TEST_PB14
    EADC_DISABLE_SAMPLE_MODULE_INT(EADC, 0, BIT6);
#else
    /* Disable the sample module 7 interrupt */
    EADC_DISABLE_SAMPLE_MODULE_INT(EADC, 0, BIT7);
#endif

    /* Wait conversion done */
    u32TimeOutCnt = SystemCoreClock; /* 1 second time-out */
#ifdef TEST_PB14
    while (EADC_GET_DATA_VALID_FLAG(EADC, BIT6) != BIT6)
#else
    while (EADC_GET_DATA_VALID_FLAG(EADC, BIT7) != BIT7)
#endif
    {
        if (--u32TimeOutCnt == 0)
        {
            MSGERROR("[1] Wait for EADC conversion done time-out!");
            return 0.0F;
        }
    }

    i32ConversionData = EADC_GET_CONV_DATA(EADC, 7);
#ifdef TEST_PB14
    MSG07("PB.14 ADC: %d", i32ConversionData);
#else
    MSG00("PB.15 ADC: %d", i32ConversionData);
#endif

    /* Reset the sample module 4, 5, 6, 7 for analog input channel and disable
     * ADINT0 trigger source */
    EADC_ConfigSampleModule(EADC, 7, EADC_SOFTWARE_TRIGGER, 0);

    return 3.6; /* bccho, 2023-11-23, 임시 */
#endif
}

uint32_t GetTemperatureCodeFromADC(void)
{
    uint32_t u32TimeOutCnt = EADC_TIMEOUT;

    /* Set input mode as single-end and enable the A/D converter */
    EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);

    /* Set sample module 17 external sampling time to 0xFF */
    EADC_SetExtendSampleTime(EADC, 17, 0xFF);

    /* Clear the A/D ADINT0 interrupt flag for safe */
    EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF0_Msk);

    /* Enable the sample module 17 interrupt.  */
    EADC_ENABLE_INT(EADC, BIT0);  // Enable sample module A/D ADINT0 interrupt.
    EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0,
                                  BIT17);  // Enable sample module 17 interrupt.
    NVIC_EnableIRQ(EADC0_IRQn);

    /* Reset the ADC interrupt indicator and trigger sample module 17 to start
     * A/D conversion */
    g_u32AdcIntFlag = 0;
    EADC_START_CONV(EADC, BIT17);

    /* Wait EADC conversion done */
    while (g_u32AdcIntFlag == 0)
    {
        if (--u32TimeOutCnt == 0)
        {
            printf("Wait for EADC conversion done time-out!\n");
            break;
        }
    }

    /* Disable the ADINT0 interrupt */
    EADC_DISABLE_INT(EADC, BIT0);

    /* Return the conversion result of the sample module 17 */
    return EADC_GET_CONV_DATA(EADC, 17);
}

#define VREF_VOLTAGE (3.3)
// #define VREF_VOLTAGE (6.0)
double GetTemperature(void)
{
    double dmVoffset = 708.58154;

    double dAvgTemperatureCode = 0;
    double dmVT = 0;
    double dT;

    dAvgTemperatureCode = 0;
    dmVT = 0;

    /* Get ADC code of temperature sensor */
    dAvgTemperatureCode = GetTemperatureCodeFromADC();

    /* ADC code to voltage conversion formula: */
    dmVT = (dAvgTemperatureCode) * (VREF_VOLTAGE) * 1000 / 4096;

    /* Get temperature with temperature sensor */
    /* Temperature sensor formula: Tx = (VT - Voffset)/(-1.8118) */
    dT = (dmVT - dmVoffset) / (-1.8118);

    return (dT);
}
