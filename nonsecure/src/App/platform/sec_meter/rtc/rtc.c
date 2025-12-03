#include "options.h"
#include "rtc.h"
#include "amg_time.h"
#include "utils.h"
#include "amg_rtc.h"
#include "amg_utc_util.h"

#define _D "[RTC] "

#define DEF_RTC_CAL 0xC80000
#define DEF_RTC_CAL_FLOAT 13107200.0

tm_t rtc_copy;
tm_t starting_tm = {2000, 0, 0, 0, 1, 1, _SAT, 0, -1, false};

bool rtc_init(void)
{
    rtc_read();

    if (!rtc_valid())
    {
        rtc_copy = starting_tm;
        rtc_alarm_clear();
        rtc_write();

        return false;
    }
    return true;
}

bool rtc_shadow_busy_wait(int32_t bit) { return true; }

void rtc_write(void)
{
    DATE_TIME_T curr_date_time;

    curr_date_time.year = rtc_copy.tm_year;
    curr_date_time.sec = rtc_copy.tm_sec;
    curr_date_time.min = rtc_copy.tm_min;
    curr_date_time.hour = rtc_copy.tm_hour;
    curr_date_time.day = rtc_copy.tm_mday;
    curr_date_time.month = rtc_copy.tm_mon;

    dsm_rtc_set_hw_time(&curr_date_time);

    rtc_isvalid = true;
}

void rtc_read(void)
{
    struct tm SystemTime;
    ST_TIME_BCD stITime;

    util_get_system_time(&SystemTime, &stITime);

    rtc_copy.tm_year = SystemTime.tm_year;
    rtc_copy.tm_sec = SystemTime.tm_sec;
    rtc_copy.tm_min = SystemTime.tm_min;
    rtc_copy.tm_hour = SystemTime.tm_hour;
    rtc_copy.tm_mday = SystemTime.tm_mday;
    rtc_copy.tm_mon = SystemTime.tm_mon;
    rtc_copy.tm_wday = SystemTime.tm_wday;
    rtc_copy.tm_yday = SystemTime.tm_yday;
    rtc_copy.tm_isdst = SystemTime.tm_isdst;

    rtc_copy.tm_isvalid = true;

    // cur_rtc.year = SystemTime.tm_year - 100;
    // cur_rtc.month = SystemTime.tm_mon + 1;
    // cur_rtc.date = SystemTime.tm_mday;
    // cur_rtc.hour = SystemTime.tm_hour;
    // cur_rtc.min = SystemTime.tm_min;
    // cur_rtc.sec = SystemTime.tm_sec;
}

#define SECONDS_PER_DAY ((time_t)(86400L))
void rtc_alarm_set(uint8_t hour, uint8_t min) {}

void rtc_alarm_clear(void) {}

void rtc_set_trim(void) {}

void rtc_get_trim(void) {}

void rtc_adjust_trim(bool clr_cnt, int32_t ppb) {}

time_t rtc_mktime(tm_t *tm_ptr)
{
    int32_t a, y, m, j;

    a = (14 - tm_ptr->tm_mon) / 12L;
    y = (int32_t)((uint32_t)((tm_ptr->tm_year) + 4800L) - a);
    m = (int32_t)(((uint32_t)tm_ptr->tm_mon + (12 * a)) - 3);

    /* julian days since Jan 1, 2000; 5.8e6 years range in 32-bit signed int */
    j = (int32_t)((uint32_t)tm_ptr->tm_mday + (((153 * m) + 2) / 5) +
                  (365 * y) + (y / 4) - (y / 100) + (y / 400) - 2483590);

    /* calculate julian seconds since Jan 1, 2000; valid until j wraps
     * around in 2068 */
    j = (int32_t)((j * 86400) + ((((uint32_t)tm_ptr->tm_hour) - 12L) * 3600) +
                  ((uint32_t)tm_ptr->tm_min * 60L) + (uint32_t)tm_ptr->tm_sec +
                  43200L);

    return (time_t)j;
}

void rtc_localtime(tm_t *tm_ptr, time_t j)
{
    uint32_t jd, w, x, a, b, c, d, e, f, month;

    tm_ptr->tm_sec = (uint8_t)(j % 60);
    tm_ptr->tm_min = (uint8_t)((j / 60) % 60);
    tm_ptr->tm_hour = (uint8_t)((j / 3600) % 24);
    jd = (uint32_t)((j / 86400L) + 2451545L); /* jd = julian days */

    /* Calculate day of week: 1 = Sunday; January 1, 2000 was Saturday */
    tm_ptr->tm_wday = (int8_t)(((jd + 1) % 7) + 1);

    /* This obscure logic is the standard arithmetic for calculating a date
     * from a julian day, taken from a web site and tested. */

    /* In standard calculation w = int((jd - 1867216.25)/36524.25); */
    w = ((4L * jd) - 7468865L) / 146097L;
    x = w / 4L;
    a = (jd + w + 1L) - x;
    b = a + 1524L;

    /* In standard calculation c = int((b - 122.1) / 365.25); */
    c = ((20L * b) - 2442L) / 7305L;
    /* In standard calculation d = int(c * 365.25); */
    d = (c * 1461L) / 4;
    /* In standard calculation e = int((b - d)/30.6001); */
    e = ((b - d) * 10000L) /
        306001L;                /* b - d is a few hundred, so no overflow */
    f = (306001L * e) / 10000L; /* e is less than 25, so no overflow */

    tm_ptr->tm_mday = (uint8_t)((b - d) - f);
    tm_ptr->tm_mon = (int8_t)(month = (e < 14) ? (e - 1) : (e - 13));
    tm_ptr->tm_year = (uint16_t)((month < 3) ? (c - 4715) : (c - 4716));
    tm_ptr->tm_yday = rtc_get_yday(tm_ptr);
    tm_ptr->tm_isdst = rtc_isdst;
    tm_ptr->tm_isvalid = rtc_isvalid;
}

int32_t rtc_delta_seconds(tm_t *start_tm_ptr, tm_t *end_tm_ptr)
{
    return ((int32_t)rtc_mktime(end_tm_ptr) -
            (int32_t)rtc_mktime(start_tm_ptr)); /* delta time. */
}

uint8_t rtc_valid(void)
{
    time_t julian_secs;
    tm_t test;
    tm_t test_2;

    test_2.tm_year = rtc_copy.tm_year - 100 + 2000;
    test_2.tm_mon = rtc_copy.tm_mon + 1;
    test_2.tm_mday = rtc_copy.tm_mday;
    test_2.tm_wday = (rtc_copy.tm_wday == 0) ? 7 : rtc_copy.tm_wday;
    test_2.tm_hour = rtc_copy.tm_hour;
    test_2.tm_min = rtc_copy.tm_min;
    test_2.tm_sec = rtc_copy.tm_sec;

    /* Test the result.  Bad dates make different dates. */
    julian_secs = rtc_mktime(&test_2); /* make seconds since Jan 1, 2000 */

    rtc_localtime(&test, julian_secs); /* convert to time and date */

    rtc_copy.tm_yday = test.tm_yday; /* Use calculated day of year */

    if (test.tm_year != 2000 && test.tm_sec == test_2.tm_sec &&
        test.tm_min == test_2.tm_min && test.tm_hour == test_2.tm_hour &&
        test.tm_mday == test_2.tm_mday && test.tm_mon == test_2.tm_mon &&
        test.tm_year == test_2.tm_year)
    {
        return true;
    }
    rtc_copy.tm_isvalid = false;
    return false;
}

bool rtc_leap_year(tm_t *tm_ptr)
{
    int year = tm_ptr->tm_year;
    if (0 == (year & 3))
    {
        if (0 == (year % 100))
        {
            if (0 == (year % 400))
                return true;
            else
                return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

const int16_t yday_by_mon[13] = {0, /* unused, months are 1..12 */
                                 0,   31,  59,  90,  120, 151,
                                 181, 212, 243, 273, 304, 334};
int16_t rtc_get_yday(tm_t *tm_ptr) /* 1 - 366 */
{
    uint16_t mon, doy;  // Month, doy = Day Of Year

    mon = tm_ptr->tm_mon;

    /* Day Of Year is the start of the month,
     * plus the day of the month. */
    doy = yday_by_mon[mon] + tm_ptr->tm_mday;

    /* Past February, add 1 on leap years. */
    if (mon > (uint16_t)FEB)
    {
        doy += (rtc_leap_year(tm_ptr)) ? 1 : 0;
    }
    return doy;
}

void rtcsub_wait(int cnt)
{
    uint32_t old_tck_cnt, tck_cnt, rtcsub;

    old_tck_cnt = RTCSUB;
    old_tck_cnt = (old_tck_cnt >> 24) & 0xFF;
    // RTCSUB is accurate in all clock speeds and battery modes.
    while (0 != cnt)
    {
        // Has the time changed? (usually not)
        rtcsub = RTCSUB;
        rtcsub = (rtcsub >> 24) & 0xFF;
        if (old_tck_cnt != rtcsub)
        {
            tck_cnt = 0xff & (rtcsub - old_tck_cnt);
            if (tck_cnt < cnt)  // Timer expired?
            {
                // count timer down.
                cnt = cnt - (uint16_t)tck_cnt;
            }
            else
            {
                // The count expired, so stop the timer.
                cnt = 0;
            }
            old_tck_cnt = rtcsub;
        }
    }
}
