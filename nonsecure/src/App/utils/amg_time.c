/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_time.h"
#include "amg_utc_util.h"

static const int g_days_in_month[] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
static const char *__month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
static const char *__day[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char ascTimeBuffer[32];
static struct tm lastTime;

time_t time(time_t *timeptr)
{
    time_t t = -1;

    t = (time_t)dsm_get_utc_time();

    if (timeptr)
    {
        *timeptr = t;
    }
    return t;
}

static void check_time(struct tm *timeptr)
{
    if (timeptr->tm_sec < 0)
        timeptr->tm_sec = 0;
    if (timeptr->tm_min < 0)
        timeptr->tm_min = 0;
    if (timeptr->tm_hour < 0)
        timeptr->tm_hour = 0;
    if (timeptr->tm_wday < 0)
        timeptr->tm_wday = 0;
    if (timeptr->tm_mon < 0)
        timeptr->tm_mon = 0;

    if (timeptr->tm_sec > 59)
        timeptr->tm_sec = 59;
    if (timeptr->tm_min > 59)
        timeptr->tm_min = 59;
    if (timeptr->tm_hour > 23)
        timeptr->tm_hour = 23;
    if (timeptr->tm_wday > 6)
        timeptr->tm_wday = 6;
    if (timeptr->tm_mday < 1)
        timeptr->tm_mday = 1;
    else if (timeptr->tm_mday > 31)
        timeptr->tm_mday = 31;
    if (timeptr->tm_mon > 11)
        timeptr->tm_mon = 11;
    if (timeptr->tm_year < 0)
        timeptr->tm_year = 0;
}

char *asctime(struct tm *timeptr)
{
    check_time(timeptr);
    dsm_sprintf(ascTimeBuffer, "%s %s %2d %02d:%02d:%02d %04d\r\n",
                __day[timeptr->tm_wday], __month[timeptr->tm_mon],
                timeptr->tm_mday, timeptr->tm_hour, timeptr->tm_min,
                timeptr->tm_sec, timeptr->tm_year + 1900);
    return ascTimeBuffer;
}

char *ctime(time_t *timep) { return asctime(localtime(timep)); }

#define LEAP_YEAR(year) ((year % 4) == 0)

struct tm *gmtime_r(time_t *timep, struct tm *p_time)
{
    unsigned long epoch = *timep;
    unsigned int year;
    unsigned char month, monthLength;
    unsigned long days;

    p_time->tm_sec = epoch % 60;
    epoch /= 60;
    p_time->tm_min = epoch % 60;
    epoch /= 60;
    p_time->tm_hour = epoch % 24;
    epoch /= 24;
    p_time->tm_wday = (epoch + 4) % 7;

    year = 1970;
    days = 0;

    while ((days += (LEAP_YEAR(year) ? 366 : 365)) <= epoch)
    {
        year++;
    }
    p_time->tm_year = year - 1900;

    days -= LEAP_YEAR(year) ? 366 : 365;
    epoch -= days;

    p_time->tm_yday = epoch;

    days = 0;
    month = 0;
    monthLength = 0;

    for (month = 0; month < 12; month++)
    {
        if (month == 1)
        {
            if (LEAP_YEAR(year))
            {
                monthLength = 29;
            }
            else
            {
                monthLength = 28;
            }
        }
        else
        {
            monthLength = g_days_in_month[month];
        }

        if (epoch >= monthLength)
        {
            epoch -= monthLength;
        }
        else
        {
            break;
        }
    }
    p_time->tm_mon = month;
    p_time->tm_mday = epoch + 1;
    p_time->tm_isdst = 0;

    return p_time;
}
struct tm *gmtime(time_t *timep) { return gmtime_r(timep, &lastTime); }

struct tm *localtime(time_t *timep) { return gmtime(timep); }

struct tm *localtime_r(time_t *timep, struct tm *p_time)
{
    return gmtime_r(timep, p_time);
}

time_t mktime(struct tm *timeptr)
{
    int year = timeptr->tm_year + 1900;
    int month = timeptr->tm_mon, i;
    long seconds;

    check_time(timeptr);

    seconds = (year - 1970) * (60 * 60 * 24L * 365);

    for (i = 1970; i < year; i++)
    {
        if (LEAP_YEAR(i))
        {
            seconds += 60 * 60 * 24L;
        }
    }

    for (i = 0; i < month; i++)
    {
        if (i == 1 && LEAP_YEAR(year))
        {
            seconds += 60 * 60 * 24L * 29;
        }
        else
        {
            seconds += 60 * 60 * 24L * g_days_in_month[i];
        }
    }

    seconds += (timeptr->tm_mday - 1) * 60 * 60 * 24L;
    seconds += timeptr->tm_hour * 60 * 60;
    seconds += timeptr->tm_min * 60;
    seconds += timeptr->tm_sec;

    return seconds;
}

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    ASSERT(tz == NULL);

    tv->tv_usec = 0;
    if (time(&tv->tv_sec) == (time_t)-1)
    {
        return -1;
    }
    return 0;
}

char *get_date_time_string(char *buff)
{
    time_t ti;
    struct tm tm;

    if (!buff)
    {
        static char date_time_buff[DATE_TIME_BUFF_SIZE];
        buff = date_time_buff;
    }
    time(&ti);

    localtime_r(&ti, &tm);
    dsm_sprintf(buff, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
                tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    return buff;
}
