/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "amg_uart.h"
#include "platform.h"
#include "amg_debug.h"
#include "main.h"
#include "amg_wdt.h"

/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/
#define USR_NVIC_PRIO_BITS \
    3  // 4bits for preemptive priority, 0bit for sub-priority
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
extern void dsm_dm_debug_uart_rx_cb(void);
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
void SystemClock_Config(void);

uint8_t dsm_reset_get_cause(void)
{
    uint8_t ret = 0;
#if 0  /* bccho, 2023-07-20 */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_FWRST) != RESET)
    {
        DPRINTF(DBG_WARN, " - Firewell Reset\r\n");
        ret |= CAUSE_FWRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST) != RESET)
    {
        DPRINTF(DBG_WARN, " - OBL Reset\r\n");
        ret |= CAUSE_OBLRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
    {
        DPRINTF(DBG_WARN, " - PIN Reset\r\n");
        ret |= CAUSE_PINRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST) != RESET)
    {
        DPRINTF(DBG_WARN, " - Brown out Reset\r\n");
        ret |= CAUSE_BORRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET)
    {
        DPRINTF(DBG_WARN, " - Software Reset\r\n");
        ret |= CAUSE_SFTRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET)
    {
        DPRINTF(DBG_WARN, " - iWatchdog Reset\r\n");
        ret |= CAUSE_IWDGRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET)
    {
        DPRINTF(DBG_WARN, " - wWatchdog Reset\r\n");
        ret |= CAUSE_WWDGRST;
    }
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST) != RESET)
    {
        DPRINTF(DBG_WARN, " - Low Power Reset\r\n");
        ret |= CAUSE_LPWRRST;
    }

    __HAL_RCC_CLEAR_RESET_FLAGS();
#endif /* bccho */
    return ret;
}

bool dsm_is_software_reset(uint8_t reset_cause)
{
    bool ret = false;

    if (reset_cause & CAUSE_SFTRST)
    {
        ret = true;
    }

    return ret;
}

bool dsm_is_bor_reset(uint8_t reset_cause)
{
    bool ret = false;

    if (reset_cause & CAUSE_BORRST)
    {
        ret = true;
    }

    return ret;
}

uint32_t dsm_reset_display_cause(uint8_t resetCause)
{
    uint32_t iwdt_freezee = 0;

    DPRINTF(DBG_WARN, "[Reset Reason]\r\n");

    if (resetCause & CAUSE_FWRST)
        DPRINTF(DBG_WARN, " - Firewell Reset\r\n");
    if (resetCause & CAUSE_OBLRST)
    {
        DPRINTF(DBG_WARN, " - PIN Reset\r\n");
    }
    if (resetCause & CAUSE_PINRST)
    {
        DPRINTF(DBG_WARN, " - Power On/Down Reset\r\n");
    }
    if (resetCause & CAUSE_BORRST)
    {
        DPRINTF(DBG_WARN, " - Brown out Reset\r\n");
        iwdt_freezee = 1;
    }
    if (resetCause & CAUSE_SFTRST)
    {
        DPRINTF(DBG_WARN, " - Software Reset\r\n");
    }
    if (resetCause & CAUSE_IWDGRST)
    {
        DPRINTF(DBG_WARN, " - Independent Watchdog Reset\r\n");
        iwdt_freezee = 1;
    }
    if (resetCause & CAUSE_WWDGRST)
    {
        DPRINTF(DBG_WARN, " - Window Watchdog Reset\r\n");
    }
    if (resetCause & CAUSE_LPWRRST)
    {
        DPRINTF(DBG_WARN, " - Low Power Reset\r\n");
    }
    return iwdt_freezee;
}

void dsm_poll_delay_us(uint32_t us)
{
    while (us-- != 0)
    {
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
        __NOP();
    };
}

void dsm_poll_delay_ms(uint32_t ms)
{
    while (ms-- != 0)
    {
        dsm_poll_delay_us(1000);
    };
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int dsm_platform_pre_init(void)
{
#if 0  /* bccho, 2023-07-20 */    
    /* Reset of all peripherals, Initializes the Flash interface and the
     * Systick. */
    HAL_Init();
#endif /* bccho */


#if 1 /* bccho, 2023-07-11 */
    dsm_uart_init(DEBUG_COM, 115200, TRUE, NULL, 2048, NULL, 1024, FALSE);
#else
    /* Configure the system clock */
    SystemClock_Config();
    dsm_uart_close(DEBUG_COM);
    dsm_uart_init(DEBUG_COM, 921600, TRUE, NULL, 8192, NULL, 1024, FALSE);
#endif

    return 1;
}

int dsm_platform_init(void)
{
#if 0  /* bccho, 2023-07-20 */
    /* Configure preemption priority groups. */
    /* only 3~7 is allowed for STM32 (4bits priority levels), 7:sub uses 4bits,
     * 3:group uses 4bits */
    /* Configure preemption priority groups.*/
    NVIC_SetPriorityGrouping(USR_NVIC_PRIO_BITS);

    /* Exceptions */
    /* Critical Error, Group 0 */
    NVIC_SetPriority(MemoryManagement_IRQn,
                     NVIC_EncodePriority(USR_NVIC_PRIO_BITS, 0, 0));
    NVIC_SetPriority(BusFault_IRQn,
                     NVIC_EncodePriority(USR_NVIC_PRIO_BITS, 0, 0));
    NVIC_SetPriority(UsageFault_IRQn,
                     NVIC_EncodePriority(USR_NVIC_PRIO_BITS, 0, 0));
#endif /* bccho */
    return 1;
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
#if 1 /* bccho, 2023-07-20 */
#if 0 /* bccho, 2023-07-25, secure에서 호출한다 */
    /* Enable HIRC clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);

    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

    /* Set core clock to 96MHz */
    CLK_SetCoreClock(96000000);
#endif
#else
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);

    if (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
    {
        Error_Handler();
    }
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while (LL_RCC_HSE_IsReady() != 1)
    {
    }

    LL_PWR_EnableBkUpAccess();
    LL_RCC_LSE_EnableBypass();
    LL_RCC_LSE_Enable();

    /* Wait till LSE is ready */
    // while(LL_RCC_LSE_IsReady() != 1)
    {
    }
    LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
    LL_RCC_EnableRTC();

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 40,
                                LL_RCC_PLLR_DIV_2);
    LL_RCC_PLL_EnableDomain_SYS();
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1)
    {
    }
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {
    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_SetSystemCoreClock(80000000);

    /* Update the time base */
    if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
    {
        Error_Handler();
    }

    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
    LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_PCLK1);
    LL_RCC_SetUSARTClockSource(LL_RCC_USART2_CLKSOURCE_PCLK1);
    LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_SYSCLK);
    LL_RCC_SetUARTClockSource(LL_RCC_UART4_CLKSOURCE_PCLK1);
    LL_RCC_SetUARTClockSource(LL_RCC_UART5_CLKSOURCE_PCLK1);
    LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);
    // LL_RCC_SetI2CClockSource(LL_RCC_I2C4_CLKSOURCE_PCLK1);
    LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM1_CLKSOURCE_PCLK1);
    LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSOURCE_SYSCLK);
#endif /* bccho */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state
     */

    /* USER CODE END Error_Handler_Debug */
}