/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#if 0 /* bccho, 2023-07-20 */
#include "platform.h"
#endif
#include "amg_timer.h"
#include "os_wrap.h"

/*
******************************************************************************
*   Definition
******************************************************************************
*/

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

#if 0 /* bccho, 2023-12-15 */
#if 1 /* bccho, ADD, 2023-07-15 */
volatile uint32_t tmr1_count;

void TMR2_IRQHandler(void)
{
    extern void timerB_isr(void);
    timerB_isr();

    // static uint32_t u32Sec = 1;
    // printf("%u\n", tmr1_count++);

    /* clear timer interrupt flag */
    TIMER_ClearIntFlag(TIMER2);
}

void TMR3_IRQHandler(void)
{
    tmr1_count++;
    TIMER_ClearIntFlag(TIMER3);
}
#endif

void dsm_timer_init(uint8_t hw_timer_no)
{
#if 1 /* bccho, 2023-08-22 */
    static bool init_done1 = FALSE;
    static bool init_done2 = FALSE;
#endif

    switch (hw_timer_no)
    {
#if 1 /* bccho, TIMER, 2023-07-15 */
    case HW_TIM_1:
        if (init_done1)
            break;

        /* Set timer frequency to 100HZ --> 10 ms */
        TIMER_Open_S(TIMER2, TIMER_PERIODIC_MODE, 100);

        TIMER_EnableInt(TIMER2);
        NVIC_EnableIRQ(TMR2_IRQn);

        TIMER_Start(TIMER2);
        init_done1 = TRUE;
        break;

    case HW_TIM_2:
        if (init_done2)
            break;

        TIMER_Open_S(TIMER3, TIMER_PERIODIC_MODE, 1000000);

        TIMER_EnableInt(TIMER3);
        NVIC_EnableIRQ(TMR3_IRQn);

        TIMER_Start(TIMER3);
        init_done2 = TRUE;
        break;
#else
    case HW_TIM_1:
    {
        uint32_t InitialAutoreload = 0;
        LL_TIM_InitTypeDef TIM_InitStruct = {0};

        LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
        NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 0x05);
        NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
        LL_TIM_EnableIT_UPDATE(TIM1);

        TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(APB2_TIMCLK, 10000);
        TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
        LL_TIM_SetPrescaler(TIM1, __LL_TIM_CALC_PSC(SystemCoreClock, 10000));

        InitialAutoreload =
            __LL_TIM_CALC_ARR(APB2_TIMCLK, LL_TIM_GetPrescaler(TIM1), 100);
        TIM_InitStruct.Autoreload = InitialAutoreload;
        TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
        TIM_InitStruct.RepetitionCounter = 0;

        LL_TIM_Init(TIM1, &TIM_InitStruct);
        LL_TIM_EnableARRPreload(TIM1);
        LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
        LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
        LL_TIM_SetTriggerOutput2(TIM1, LL_TIM_TRGO2_RESET);
        LL_TIM_DisableMasterSlaveMode(TIM1);

        /* Enable counter */
        LL_TIM_EnableCounter(TIM1);

        /* Force update generation */
        LL_TIM_GenerateEvent_UPDATE(TIM1);
    }
    break;

    case HW_TIM_2:
    {
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
        LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
        LL_TIM_SetPrescaler(TIM2, __LL_TIM_CALC_PSC(APB1_TIMCLK, 1000000));
        LL_TIM_EnableCounter(TIM2);
        LL_TIM_GenerateEvent_UPDATE(TIM2);
        LL_TIM_SetClockDivision(TIM2, LL_TIM_CLOCKDIVISION_DIV1);
        LL_TIM_SetCounter(TIM2, 0xFFFFFFF);
    }
    break;
#endif /* bccho */
    }
}

void dsm_timer_deinit(uint8_t hw_timer_no)
{
#if 0  /* bccho, TIMER, 2023-07-15 */
	switch(hw_timer_no)
	{
		case HW_TIM_1:
			LL_TIM_DeInit(TIM1);
		break;

		case HW_TIM_2:
			LL_TIM_DeInit(TIM2);
		break;

		default:
		break;
	}
#endif /* bccho */
}
#endif
