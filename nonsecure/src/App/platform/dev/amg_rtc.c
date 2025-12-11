/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_rtc.h"
#include "amg_utc_util.h"
#include "whm.h"
#include "options_sel.h"
/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/
#define _D "[RTC] "

#if 1 /* bccho, RTC, 2023-07-15 */
#define RTC_EXTERNAL
#define BIT(n) (0x1U << (n))

#define RV8803_SEC 0x00
#define RV8803_MIN 0x01
#define RV8803_HOUR 0x02
#define RV8803_WEEK 0x03
#define RV8803_DAY 0x04
#define RV8803_MONTH 0x05
#define RV8803_YEAR 0x06
#define RV8803_RAM 0x07
#define RV8803_ALARM_MIN 0x08
#define RV8803_ALARM_HOUR 0x09
#define RV8803_ALARM_WEEK_OR_DAY 0x0A
#define RV8803_EXT 0x0D
#define RV8803_FLAG 0x0E
#define RV8803_CTRL 0x0F
#define RV8803_OSC_OFFSET 0x2C

/* WADA = 0 for weekday alarm or WADA = 1 for date alarm */
#define RV8803_EXT_WADA BIT(6)

/* Set if the voltage crosses VLOW1 voltage and the temperature compensation is
 * stopped. */
#define RV8803_FLAG_V1F BIT(0) /* Voltage Low Flag 1 */

/* Set if the voltage crosses VLOW2 voltage and the data in the device are no
 * longer valid. All registers must be initialized */
#define RV8803_FLAG_V2F BIT(1) /* Voltage Low Flag 2 */

#define RV8803_FLAG_EVF BIT(2) /* External Event Flag */
#define RV8803_FLAG_AF BIT(3)  /* Alarm Flag */
#define RV8803_FLAG_TF BIT(4)  /* Periodic Countdown Timer Flag */
#define RV8803_FLAG_UF BIT(5)  /* Periodic Time Update Flag */

#define RV8803_CTRL_RESET BIT(0) /* Reset/Stop */
#define RV8803_CTRL_EIE BIT(2)   /* External Event Interrupt Enable */
#define RV8803_CTRL_AIE BIT(3)   /* Alarm Interrupt Enable */
#define RV8803_CTRL_TIE BIT(4)   /* Periodic Countdown Timer Interrupt Enable */
#define RV8803_CTRL_UIE BIT(5)   /* Periodic Time Update Interrupt Enable */

unsigned bcd2bin(unsigned char val) { return (val & 0x0f) + (val >> 4) * 10; }
unsigned char bin2bcd(unsigned val) { return ((val / 10) << 4) + val % 10; }
#define bcd_is_valid(x) (((x) & 0x0f) < 10 && ((x) >> 4) < 10)

uint8_t g_ctrl;
uint8_t alarm_invalid;
uint8_t alrm_enabled;
uint8_t alrm_pending;
#endif

uint32_t rtc_tick;
ALARM_CALLBACK g_alarm_a_callback;
ALARM_CALLBACK g_alarm_b_callback;

#ifdef M2354_NEW_HW
#define RTCI I2C1
#else
#define RTCI I2C2
#endif

#if 0 /* bccho, 2023-11-29 */
#define RTC_CLOCK_SPEED 50000
#else
#define RTC_CLOCK_SPEED 100000
#endif

#if 0 /* bccho, 외부RTC 알람 테스트, 2023-09-26 */
void GPB_IRQHandler(void)
{
    volatile uint32_t u32temp;

    /* To check if PB.10 interrupt occurred */
    if (GPIO_GET_INT_FLAG(PB, BIT12))
    {
        GPIO_CLR_INT_FLAG(PB, BIT12);
        MSGALWAYS("***** PB.12 INT occurred *****");
    }
    else
    {
        /* Un-expected interrupt. Just clear all PB interrupts */
        u32temp = PB->INTSRC;
        PB->INTSRC = u32temp;
        MSGALWAYS("Un-expected interrupts.\n");
    }
}
#endif

void dsm_rtc_config(void)
{
#if 1 /* bccho, RTC, 2023-07-15 */
    MSG06("dsm_rtc_config");

#ifdef RTC_EXTERNAL
#ifdef M2354_NEW_HW
    I2C_Open_S(RTCI, RTC_CLOCK_SPEED);
#else
    I2C_Open_S(RTCI, 400000);
#endif

#if 0 /* bccho, 외부RTC 알람 테스트, 2023-09-26 */
    GPIO_SetMode(PB, BIT12, GPIO_MODE_INPUT);
    GPIO_EnableInt(PB, 12, GPIO_INT_FALLING);
    NVIC_EnableIRQ(GPB_IRQn);

    GPIO_SET_DEBOUNCE_TIME(PB, GPIO_DBCTL_DBCLKSRC_LIRC,
                           GPIO_DBCTL_DBCLKSEL_1024);
    GPIO_ENABLE_DEBOUNCE(PB, BIT12);
#endif
#else
    S_RTC_TIME_DATA_T sInitTime;

    sInitTime.u32Year = 2019;
    sInitTime.u32Month = 12;
    sInitTime.u32Day = 1;
    sInitTime.u32Hour = 12;
    sInitTime.u32Minute = 30;
    sInitTime.u32Second = 0;
    sInitTime.u32DayOfWeek = RTC_MONDAY;
    sInitTime.u32TimeScale = RTC_CLOCK_24;
    if (RTC_Open_S(&sInitTime) != 0)
    {
        printf("\n RTC initial fail!!");
        printf("\n Please check h/w setting!!");
    }
#endif
#else  /* bccho */
    LL_RTC_InitTypeDef RTC_InitStruct = {0};

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    LL_PWR_EnableBkUpAccess();

    if (LL_RCC_LSE_IsReady() == 0)
    {
        LL_RCC_ForceBackupDomainReset();
        LL_RCC_ReleaseBackupDomainReset();
        LL_RCC_LSE_Enable();

        // while (LL_RCC_LSE_IsReady() != 1)
        {
        }
        LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
        LL_RCC_EnableRTC();
    }

    LL_RTC_DisableWriteProtection(RTC);

    RTC_InitStruct.HourFormat = LL_RTC_HOURFORMAT_24HOUR;
    RTC_InitStruct.AsynchPrescaler = RTC_ASYNCH_PREDIV;
    RTC_InitStruct.SynchPrescaler = RTC_SYNCH_PREDIV;
    LL_RTC_Init(RTC, &RTC_InitStruct);

    LL_RTC_WaitForSynchro(RTC);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_18);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_18);
    NVIC_SetPriority(RTC_Alarm_IRQn, 0x0F);
    NVIC_EnableIRQ(RTC_Alarm_IRQn);
#endif /* bccho */
}

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/
void dsm_rtc_init(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    MSG06("dsm_rtc_init()");
    dsm_rtc_config();
}

void RTC_Alarm_IRQHandler(void)
{
#if 0  /* bccho, RTC, 2023-07-15 */
    OSIntEnter ();
    /* Get the Alarm interrupt source enable status */
    if(LL_RTC_IsEnabledIT_ALRA(RTC) != 0)
    {
        /* Get the pending status of the Alarm Interrupt */
        if(LL_RTC_IsActiveFlag_ALRA(RTC) != 0)
        {
            ALARM_CALLBACK* pAlarm = &g_alarm_a_callback;

            if(pAlarm->WakeUpFunc)
            {
                (*pAlarm->WakeUpFunc)();
            }
            /* Clear the Alarm interrupt pending bit */
            LL_RTC_ClearFlag_ALRA(RTC);
        }

        if(LL_RTC_IsActiveFlag_ALRB(RTC) != 0)
        {
            ALARM_CALLBACK* pAlarm = &g_alarm_b_callback;

            if(pAlarm->WakeUpFunc)
            {
                (*pAlarm->WakeUpFunc)();
            }
            /* Clear the Alarm interrupt pending bit */
            LL_RTC_ClearFlag_ALRB(RTC);
        }

    }
    /* Clear the EXTI's Flag for RTC Alarm */
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_18);

    OSIntExit ();
#endif /* bccho */
}

uint32_t dsm_rtc_get_time(void)
{
    uint32_t sec_tick;
    DATE_TIME_T date_time_t;

    dsm_rtc_get_hw_time(&date_time_t);
    sec_tick = dsm_convet_time_to_sec(&date_time_t);

    return sec_tick;
}

void dsm_rtc_set_hw_time(DATE_TIME_T* dt)
{
#if 1 /* bccho, RTC, 2023-07-15 */
#ifdef RTC_EXTERNAL
    MSG06("dsm_rtc_set_hw_time");

    uint8_t date[7];

    int ctrl = I2C_ReadByteOneReg(RTCI, 0x32, RV8803_CTRL);
    if (ctrl == 0 && g_I2C_i32ErrCode != 0)
    {
        MSGERROR("set_hw_time, ReadByteOneReg#1");
        return;
    }

    /* Stop the clock */
    uint8_t ret =
        I2C_WriteByteOneReg(RTCI, 0x32, RV8803_CTRL, ctrl | RV8803_CTRL_RESET);
    if (ret != 0)
    {
        MSGERROR("set_hw_time, WriteByteOneReg#1");
        return;
    }

    date[RV8803_SEC] = bin2bcd(dt->sec);
    date[RV8803_MIN] = bin2bcd(dt->min);
    date[RV8803_HOUR] = bin2bcd(dt->hour);
    date[RV8803_WEEK] = 1 << 1; /* bccho, 2023-07-29, 일단 weekday2로 */
    date[RV8803_DAY] = bin2bcd(dt->day);
    date[RV8803_MONTH] = bin2bcd(dt->month);
    dt->year -= 2000;
    date[RV8803_YEAR] = bin2bcd(dt->year);

    uint32_t write_len =
        I2C_WriteMultiBytesOneReg(RTCI, 0x32, RV8803_SEC, date, 7);
    if (write_len != 7)
    {
        MSGERROR("set_hw_time, WriteMultiBytesOneReg");
        return;
    }

    /* Restart the clock */
    ret =
        I2C_WriteByteOneReg(RTCI, 0x32, RV8803_CTRL, ctrl & ~RV8803_CTRL_RESET);
    if (ret != 0)
    {
        MSGERROR("set_hw_time, WriteByteOneReg#2");
        return;
    }

    uint8_t flags = I2C_ReadByteOneReg(RTCI, 0x32, RV8803_FLAG);
    if (flags == 0 && g_I2C_i32ErrCode != 0)
    {
        MSGERROR("set_hw_time, ReadByteOneReg#2");
        return;
    }
    else if (flags & RV8803_FLAG_V2F)
    {
        MSGERROR("set_hw_time, RV8803_FLAG_V2F");

        I2C_WriteByteOneReg(RTCI, 0x32, RV8803_OSC_OFFSET, 0x00);
        I2C_WriteByteOneReg(RTCI, 0x32, RV8803_CTRL, 0b11000000);
        I2C_WriteMultiBytesOneReg(RTCI, 0x32, RV8803_ALARM_MIN,
                                  (uint8_t[]){0, 0, 0}, 3);
        I2C_WriteByteOneReg(RTCI, 0x32, RV8803_RAM, 0x00);
        I2C_WriteByteOneReg(RTCI, 0x32, RV8803_EXT, RV8803_EXT_WADA);

        return;
    }

    ret = I2C_WriteByteOneReg(RTCI, 0x32, RV8803_FLAG,
                              flags & ~(RV8803_FLAG_V1F | RV8803_FLAG_V2F));
    if (ret != 0)
    {
        MSGERROR("set_hw_time, WriteByteOneReg#3");
    }

#else
    RTC_SetDate_S(dt->year, dt->month, dt->day, 0);
    RTC_SetTime_S(dt->hour, dt->min, dt->sec);
#endif
#else
    RTC_DateTypeDef sdate;
    RTC_TimeTypeDef stime;
    RTC_HandleTypeDef RtcHandle;

    RtcHandle.Instance = RTC;

    sdate.Year = dt->year - BASE_YEAR;
    sdate.Month = dt->month;
    sdate.Date = dt->day;
    sdate.WeekDay = 0;
    stime.Hours = dt->hour;
    stime.Minutes = dt->min;
    stime.Seconds = dt->sec;
    stime.SubSeconds = 0x00;
    stime.TimeFormat = RTC_HOURFORMAT12_AM;
    stime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    stime.StoreOperation = RTC_STOREOPERATION_RESET;

    DPRINTF(DBG_TRACE, _D "%s: %d.%02d.%02d %02d:%02d:%02d\r\n", __func__,
            (sdate.Year + 2000), sdate.Month, sdate.Date, stime.Hours,
            stime.Minutes, stime.Seconds);

    HAL_RTC_SetTime(&RtcHandle, &stime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&RtcHandle, &sdate, RTC_FORMAT_BIN);
#endif /* bccho */
}

uint32_t dsm_rtc_get_hw_time(DATE_TIME_T* dt)
{
#if 1 /* bccho, RTC, 2023-07-15 */
#ifdef RTC_EXTERNAL
    MSG03("dsm_rtc_get_hw_time");

    uint8_t date1[7];
    uint8_t date2[7];
    uint8_t* date = date1;

    static DATE_TIME_T pre_dt = {.month = 1, .day = 1};
    bool use_pre = false;

    int flags = I2C_ReadByteOneReg(RTCI, 0x32, RV8803_FLAG);
    if (flags == 0 && g_I2C_i32ErrCode != 0)
    {
        MSGERROR("get_hw_time. read flags");
        use_pre = true;
        goto exit_use_pre;
    }

    bool clear_flag = false;
    if (flags & RV8803_FLAG_AF)
    {
        MSG06("get_hw_time. ALARM!!!! --> clear");
        clear_flag = true;
        flags &= ~RV8803_FLAG_AF;
    }
    if (flags & RV8803_FLAG_V1F)
    {
        MSG06("get_hw_time. FLAG_V1F --> clear");
        clear_flag = true;
        flags &= ~RV8803_FLAG_V1F;
    }
    if (flags & RV8803_FLAG_V2F)
    {
        MSG06("get_hw_time. FLAG_V2F --> clear");
        clear_flag = true;
        flags &= ~RV8803_FLAG_V2F;
    }

    if (clear_flag)
    {
        vTaskDelay(1);
        uint8_t ret = I2C_WriteByteOneReg(RTCI, 0x32, RV8803_FLAG, flags);
        if (ret != 0)
        {
            MSGERROR("get_hw_time. WriteByteOneReg");
            use_pre = true;
            goto exit_use_pre;
        }
    }

    vTaskDelay(1);
    uint32_t len = I2C_ReadMultiBytesOneReg(RTCI, 0x32, RV8803_SEC, date1, 7);
    if (len != 7)
    {
        MSGERROR("get_hw_time, fail, len:%d, flag:%02X", len, flags);
        use_pre = true;
        goto exit_use_pre;
    }

    if ((date1[RV8803_SEC] & 0x7f) == bin2bcd(59))
    {
        vTaskDelay(1);
        len = I2C_ReadMultiBytesOneReg(RTCI, 0x32, RV8803_SEC, date2, 7);
        if (len != 7)
        {
            use_pre = true;
            goto exit_use_pre;
        }
        if ((date2[RV8803_SEC] & 0x7f) != bin2bcd(59))
        {
            date = date2;
        }
    }

exit_use_pre:
    if (use_pre)
    {
        MSGERROR("get_hw_time. use_pre");

        dt->sec = pre_dt.sec;
        dt->min = pre_dt.min;
        dt->hour = pre_dt.hour;
        dt->day = pre_dt.day;
        dt->month = pre_dt.month;
        dt->year = pre_dt.year;

#ifdef M2354_NEW_HW
        I2C_Close(I2C1);
        SYS_ResetModule_S(I2C1_RST);
        I2C_Open_S(I2C1, RTC_CLOCK_SPEED);
#else
        I2C_Close(RTCI);
        SYS_ResetModule_S(I2C2_RST);
        I2C_Open_S(RTCI, 400000);
#endif
        return FALSE;
    }
    else
    {
        dt->sec = pre_dt.sec = bcd2bin(date[RV8803_SEC] & 0x7f);
        dt->min = pre_dt.min = bcd2bin(date[RV8803_MIN] & 0x7f);
        dt->hour = pre_dt.hour = bcd2bin(date[RV8803_HOUR] & 0x3f);
        dt->day = pre_dt.day = bcd2bin(date[RV8803_DAY] & 0x3f);
        dt->month = pre_dt.month = bcd2bin(date[RV8803_MONTH] & 0x1f);
        dt->year = pre_dt.year = bcd2bin(date[RV8803_YEAR]) + 2000;
    }

    MSG00("dsm_rtc_get_hw_time__year, %X", date[RV8803_YEAR]);
    MSG00("dsm_rtc_get_hw_time, %d", len);
    MSG00("get_hw_time: %d-%d-%d %d:%d:%d", dt->year, dt->month, dt->day,
          dt->hour, dt->min, dt->sec);
#else
    S_RTC_TIME_DATA_T sReadRTC;

    RTC_GetDateAndTime_S(&sReadRTC);
    dt->year = sReadRTC.u32Year;
    dt->month = sReadRTC.u32Month;
    dt->day = sReadRTC.u32Day;
    dt->hour = sReadRTC.u32Hour;
    dt->min = sReadRTC.u32Minute;
    dt->sec = sReadRTC.u32Second;

    MSG00("get_hw_time: %d-%d-%d", dt->year, dt->month, dt->day);
    MSG00("get_hw_time: %d:%d:%d", dt->hour, dt->min, dt->sec);
#endif
#else  /* bccho */
    RTC_DateTypeDef sdate;
    RTC_TimeTypeDef stime;
    RTC_HandleTypeDef RtcHandle;

    RtcHandle.Instance = RTC;

    HAL_RTC_GetTime(&RtcHandle, &stime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RtcHandle, &sdate, RTC_FORMAT_BIN);

    dt->year = sdate.Year + BASE_YEAR;
    dt->month = sdate.Month;
    dt->day = sdate.Date;
    dt->hour = stime.Hours;
    dt->min = stime.Minutes;
    dt->sec = stime.Seconds;
#endif /* bccho */

    return TRUE;
}

void dsm_rtc_set_alarm_time(uint32_t alarm_type, DATE_TIME_T* ptime,
                            void (*WakeUpFunc)(void))
{
#if 1 /* bccho, RTC, 2023-07-15 */
#ifdef RTC_EXTERNAL
    uint8_t alarmvals[3];
    uint8_t ctrl[2];
    uint8_t ret;

    MSGALWAYS("set_alarm_time, %02d:%02d:%02d", ptime->hour, ptime->min,
              ptime->sec);

    /* flag와 ctrl을 읽는다 */
    /* ctrl[0] : flag */
    /* ctrl[1] : ctrl */
    uint32_t len = I2C_ReadMultiBytesOneReg(RTCI, 0x32, RV8803_FLAG, ctrl, 2);
    if (len != 2)
    {
        MSGERROR("set_alarm_time, ReadMultiBytesOneReg");
        return;
    }

    alarmvals[0] = bin2bcd(ptime->min);
    alarmvals[1] = bin2bcd(ptime->hour);
    alarmvals[2] = 0x80;

    /* AIE : Alarm Interrupt Enable */
    /* UIE : Periodic Time Update Interrupt Enable */
    if (ctrl[1] & (RV8803_CTRL_AIE | RV8803_CTRL_UIE))
    {
        ctrl[1] &= ~(RV8803_CTRL_AIE | RV8803_CTRL_UIE);
        ret = I2C_WriteByteOneReg(RTCI, 0x32, RV8803_CTRL, ctrl[1]);
        if (ret != 0)
        {
            MSGERROR("set_alarm_time, WriteByteOneReg#1");
            return;
        }
    }

    /* Alarm Flag 삭제 */
    /* can be cleared by writing a 0 to the bit. */
    ctrl[0] &= ~RV8803_FLAG_AF;
    ret = I2C_WriteByteOneReg(RTCI, 0x32, RV8803_FLAG, ctrl[0]);
    if (ret != 0)
    {
        MSGERROR("set_alarm_time, WriteByteOneReg#2");
        return;
    }

    /* 알람 시간 설정 */
    uint32_t write_len =
        I2C_WriteMultiBytesOneReg(RTCI, 0x32, RV8803_ALARM_MIN, alarmvals, 3);
    if (write_len != 3)
    {
        MSGERROR("set_alarm_time, WriteMultiBytesOneReg");
        return;
    }

    /* Alarm Interrupt 설정 */
    ctrl[1] |= RV8803_CTRL_AIE;
    ret = I2C_WriteByteOneReg(RTCI, 0x32, RV8803_CTRL, ctrl[1]);
    if (ret != 0)
    {
        MSGERROR("set_alarm_time, WriteByteOneReg#3");
        return;
    }
#else /* 내장 RTC */
    MSG06("RTC_SetAlarmTime_S, %02d:%02d:%02d", ptime->hour, ptime->min,
          ptime->sec);
    RTC_SetAlarmTime_S(ptime->hour, ptime->min, ptime->sec);
#endif
#else  /* bccho */
    LL_RTC_AlarmTypeDef rtc_alarm_initstruct;

    DPRINTF(DBG_TRACE, _D "%s: ALARM_T[%d], [%02d:%02d:%02d]\r\n", __func__,
            alarm_type, ptime->hour, ptime->min, ptime->sec);

    dsm_register_alarm_callback(alarm_type, WakeUpFunc);

    if (alarm_type == E_ALARM_A)
    {
        rtc_alarm_initstruct.AlarmTime.TimeFormat = LL_RTC_ALMA_TIME_FORMAT_AM;
        rtc_alarm_initstruct.AlarmTime.Hours = ptime->hour;
        rtc_alarm_initstruct.AlarmTime.Minutes = ptime->min;
        rtc_alarm_initstruct.AlarmTime.Seconds = ptime->sec;

        rtc_alarm_initstruct.AlarmMask = LL_RTC_ALMA_MASK_DATEWEEKDAY;
        rtc_alarm_initstruct.AlarmDateWeekDaySel =
            LL_RTC_ALMA_DATEWEEKDAYSEL_DATE;
        rtc_alarm_initstruct.AlarmDateWeekDay = LL_RTC_WEEKDAY_MONDAY;
    }
    else
    {
        rtc_alarm_initstruct.AlarmTime.TimeFormat = LL_RTC_ALMB_TIME_FORMAT_AM;
        rtc_alarm_initstruct.AlarmTime.Hours = ptime->hour;
        rtc_alarm_initstruct.AlarmTime.Minutes = ptime->min;
        rtc_alarm_initstruct.AlarmTime.Seconds = ptime->sec;

        rtc_alarm_initstruct.AlarmMask = LL_RTC_ALMB_MASK_DATEWEEKDAY;
        rtc_alarm_initstruct.AlarmDateWeekDaySel =
            LL_RTC_ALMB_DATEWEEKDAYSEL_DATE;
        rtc_alarm_initstruct.AlarmDateWeekDay = LL_RTC_WEEKDAY_MONDAY;
    }

    OS_ENTER_CRITICAL();
    LL_RTC_DisableWriteProtection(RTC);

    if (alarm_type == E_ALARM_A)
    {
        LL_RTC_ALMA_Disable(RTC);
        LL_RTC_DisableIT_ALRA(RTC);
        if (LL_RTC_ALMA_Init(RTC, LL_RTC_FORMAT_BIN, &rtc_alarm_initstruct) !=
            SUCCESS)
        {
            DPRINTF(DBG_ERR, "%s: alm_a init error\r\n", __func__);
        }
    }
    else
    {
        LL_RTC_ALMB_Disable(RTC);
        LL_RTC_DisableIT_ALRB(RTC);
        if (LL_RTC_ALMB_Init(RTC, LL_RTC_FORMAT_BIN, &rtc_alarm_initstruct) !=
            SUCCESS)
        {
            DPRINTF(DBG_ERR, "%s: alm_b init error\r\n", __func__);
        }
    }

    LL_RTC_DisableWriteProtection(RTC);

    if (alarm_type == E_ALARM_A)
    {
        LL_RTC_ALMA_Enable(RTC);
        LL_RTC_ClearFlag_ALRA(RTC);
        LL_RTC_EnableIT_ALRA(RTC);
    }
    else
    {
        LL_RTC_ALMB_Enable(RTC);
        LL_RTC_ClearFlag_ALRB(RTC);
        LL_RTC_EnableIT_ALRB(RTC);
    }

    LL_RTC_EnableWriteProtection(RTC);
    OS_EXIT_CRITICAL();
#endif /* bccho */
}

void dsm_rtc_get_alarm_time(uint32_t alarm_type)
{
#if 1 /* bccho, RTC, 2023-07-15 */
#ifdef RTC_EXTERNAL
    uint8_t alarmvals[3];
    uint32_t len =
        I2C_ReadMultiBytesOneReg(RTCI, 0x32, RV8803_ALARM_MIN, alarmvals, 3);
    if (len != 3)
    {
        MSGERROR("get_alarm_time, ReadMultiBytesOneReg, len:%d", len);
        return;
    }

    alarmvals[0] &= 0x7f; /* min */
    alarmvals[1] &= 0x3f; /* hour */
    alarmvals[2] &= 0x3f;

    MSGALWAYS("get_alarm_time, %02X %02X:%02X", alarmvals[2], alarmvals[1],
              alarmvals[0]);
    return;

#else /* 내장 RTC */
    S_RTC_TIME_DATA_T sWriteRTC;
    RTC_GetAlarmDateAndTime_S(&sWriteRTC);

    DPRINTF(DBG_TRACE, _D "%s: %02d:%02d:%02d\r\n", __func__, sWriteRTC.u32Hour,
            sWriteRTC.u32Minute, sWriteRTC.u32Second);
#endif
#else  /* bccho */
    RTC_HandleTypeDef RtcHandle;
    RTC_AlarmTypeDef Alarm;

    RtcHandle.Instance = RTC;

    if (alarm_type == E_ALARM_A)
    {
        HAL_RTC_GetAlarm(&RtcHandle, &Alarm, RTC_ALARM_A, RTC_FORMAT_BIN);
    }
    else
    {
        HAL_RTC_GetAlarm(&RtcHandle, &Alarm, RTC_ALARM_B, RTC_FORMAT_BIN);
    }
    DPRINTF(DBG_TRACE, _D "%s: type[%d] %02d:%02d:%02d\r\n", __func__,
            alarm_type, Alarm.AlarmTime.Hours, Alarm.AlarmTime.Minutes,
            Alarm.AlarmTime.Seconds);
#endif /* bccho */
}
