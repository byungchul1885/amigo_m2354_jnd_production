#ifndef __AMG_RTC_H__
#define __AMG_RTC_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/

#include "options_sel.h"
#include "amg_utc_util.h"

/*
******************************************************************************
* 	DEFINITION
******************************************************************************
*/

typedef enum
{
    E_ALARM_A,
    E_ALARM_B
} EN_ALARM_T;
/*
******************************************************************************
* 	MACRO
******************************************************************************
*/
typedef struct
{
    void (*WakeUpFunc)(void);
} ALARM_CALLBACK;

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/
void dsm_rtc_init(void);
uint32_t dsm_rtc_get_time(void);
void dsm_rtc_set_hw_time(DATE_TIME_T* p_data_time);
uint32_t dsm_rtc_get_hw_time(DATE_TIME_T* date_time_t);
void dsm_rtc_set_alarm_time(uint32_t alarm_type, DATE_TIME_T* ptime,
                            void (*WakeUpFunc)(void));
void dsm_rtc_get_alarm_time(uint32_t alarm_type);
#endif /* __AMG_WDT_H__*/