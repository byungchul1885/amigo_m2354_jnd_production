/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/

#ifndef __TIME_H__
#define __TIME_H__

#define DATE_TIME_BUFF_SIZE 20

#if 1 /* bccho, ADD, 2023-07-20 */
typedef unsigned int time_t;

struct timeval
{
    time_t tv_sec;
    time_t tv_usec;
};
#endif

struct tm
{
    int tm_sec;   /* Seconds */
    int tm_min;   /* Minutes */
    int tm_hour;  /* Hour (0--23) */
    int tm_mday;  /* Day of month (1--31) */
    int tm_mon;   /* Month (0--11) */
    int tm_year;  /* Year (calendar year minus 1900) */
    int tm_wday;  /* Weekday (0--6; Sunday = 0) */
    int tm_yday;  /* Day of year (0--365) */
    int tm_isdst; /* 0 if daylight savings time is not in effect) */
};

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

typedef struct _tagST_TIME
{
    uint16_t usYear;
    uint8_t ucMonth;
    uint8_t ucDay;
    uint8_t ucWeek;
    uint8_t ucHour;
    uint8_t ucMin;
    uint8_t ucSec;
    uint8_t ucHundr;
    uint16_t usDevia;
    uint8_t ucState;
} __packed ST_TIME;

typedef struct _tagST_DATE
{
    uint16_t usYear;
    uint8_t ucMonth;
    uint8_t ucDay;
    uint8_t ucWeek;
} __packed ST_DATE;

time_t time(time_t *timeptr);
char *asctime(struct tm *timeptr);
char *ctime(time_t *timep);
struct tm *gmtime_r(time_t *timep, struct tm *p_time);
struct tm *gmtime(time_t *timep);
struct tm *localtime(time_t *timep);
struct tm *localtime_r(time_t *timep, struct tm *p_time);
time_t mktime(struct tm *timeptr);
int gettimeofday(struct timeval *tv, struct timezone *tz);
char *get_date_time_string(char *buff);
uint32_t dsm_get_utc_time(void);

#endif
