/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_rtc.h"
#include "amg_utc_util.h"
#include "whm.h"
#include "amg_typedef.h"
#include "amg_debug.h"

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

enum
{
    COMMON_YEAR_TOTAL_SECONDS = 31536000,
    LEAP_YEAR_TOTAL_SECONDS = 31622400,
    DAY_TOTAL_SECONDS = 86400,
    HOUR_TOTAL_SECONDS = 3600,
    MIN_TOTAL_SECONDS = 60,
};
const uint8_t days_of_month[13] = {0,  31, 28, 31, 30, 31, 30,
                                   31, 31, 30, 31, 30, 31};

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
*	GLOBAL FUNCTIONS
******************************************************************************
*/


uint32_t dsm_get_utc_time(void) { return dsm_rtc_get_time(); }

static bool dsm_is_leap_year(uint16_t curr_year)
{
    if (curr_year % 4 != 0)
        return FALSE;
    else if (curr_year % 100 != 0)
        return TRUE;
    else if (curr_year % 400 != 0)
        return FALSE;
    else
        return TRUE;
}

uint32_t dsm_convet_time_to_sec(DATE_TIME_T *date_time_p)
{
    int i;
    uint32_t utc_sec;

    if (date_time_p->year < 1970)
        return 0;

    utc_sec = 0;
    // year
    for (i = 1970; i < date_time_p->year; i++)
    {
        if (dsm_is_leap_year(i))
            utc_sec += LEAP_YEAR_TOTAL_SECONDS;
        else
            utc_sec += COMMON_YEAR_TOTAL_SECONDS;
    }
    // month
    for (i = 1; i < date_time_p->month; i++)
    {
        utc_sec += days_of_month[i] * DAY_TOTAL_SECONDS;
        if (i == 2 && dsm_is_leap_year(date_time_p->year))
            utc_sec += DAY_TOTAL_SECONDS;
    }
    // day
    utc_sec += (date_time_p->day - 1) * DAY_TOTAL_SECONDS;
    // hour
    utc_sec += date_time_p->hour * HOUR_TOTAL_SECONDS;
    // min
    utc_sec += date_time_p->min * MIN_TOTAL_SECONDS;
    // sec
    utc_sec += date_time_p->sec;

    return utc_sec;
}

int dsm_convert_utc_to_time(uint32_t utc_sec, DATE_TIME_T *date_time_wp)
{
    int i;

    date_time_wp->year = 1970;
    date_time_wp->month = 1;
    date_time_wp->day = 1;
    date_time_wp->hour = 0;
    date_time_wp->min = 0;
    date_time_wp->sec = 0;

    // year
    for (i = 1970;; i++)
    {
        if (dsm_is_leap_year(i))
        {
            if (utc_sec >= LEAP_YEAR_TOTAL_SECONDS)
                utc_sec -= LEAP_YEAR_TOTAL_SECONDS;
            else
                break;
        }
        else
        {
            if (utc_sec >= COMMON_YEAR_TOTAL_SECONDS)
                utc_sec -= COMMON_YEAR_TOTAL_SECONDS;
            else
                break;
        }
        date_time_wp->year++;
    }

    // month
    for (i = 1;; i++)
    {
        if (utc_sec >= days_of_month[i] * DAY_TOTAL_SECONDS)
            utc_sec -= days_of_month[i] * DAY_TOTAL_SECONDS;
        else
            break;
        if (i == 2 && dsm_is_leap_year(date_time_wp->year))
        {
            if (utc_sec >= DAY_TOTAL_SECONDS)
                utc_sec -= DAY_TOTAL_SECONDS;
            else
            {
                date_time_wp->day = 29;
                break;
            }
        }
        date_time_wp->month++;
    }

    // day
    for (i = 1;; i++)
    {
        if (utc_sec >= DAY_TOTAL_SECONDS)
            utc_sec -= DAY_TOTAL_SECONDS;
        else
            break;
        date_time_wp->day++;
    }

    // hour
    for (i = 0;; i++)
    {
        if (utc_sec >= HOUR_TOTAL_SECONDS)
            utc_sec -= HOUR_TOTAL_SECONDS;
        else
            break;
        date_time_wp->hour++;
    }

    // min
    for (i = 0;; i++)
    {
        if (utc_sec >= MIN_TOTAL_SECONDS)
            utc_sec -= MIN_TOTAL_SECONDS;
        else
            break;
        date_time_wp->min++;
    }

    // sec
    date_time_wp->sec = utc_sec;
    return 0;
}

int dsm_get_time(DATE_TIME_T *date_time_wp)
{
    dsm_convert_utc_to_time(dsm_get_utc_time(), date_time_wp);

    return 0;
}