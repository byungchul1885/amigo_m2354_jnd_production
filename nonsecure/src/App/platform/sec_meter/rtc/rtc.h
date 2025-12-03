#ifndef RTC_H
#define RTC_H 1

#if 1 /* bccho, 2023-07-20, time_t */
#include "amg_time.h"
#endif /* bccho */

/* remove the effects of standard library definitions */
#ifdef tm_t
#undef tm_t
#endif
#ifdef time_t
#undef time_t
#endif

#define RTCA_ADJ (*(vuint8 *)(0x80006B1F))

#ifndef RTC_CALENDAR
#define RTC_CALENDAR 1  // 1 = enable calendar logic.
#endif
#ifndef OPERATING_TIMER
#define OPERATING_TIME 1  // 1 = track hours of operation.
#endif
#ifndef RTC_LINE_LOCKED
#define RTC_LINE_LOCKED 0  // 1 = RTC locked to line frequency.
#endif

// Months
#define JAN 1
#define FEB 2
#define MAR 3
#define APR 4
#define MAY 5
#define JUN 6
#define JUL 7
#define AUG 8
#define SEP 9
#define OCT 10
#define NOV 11
#define DEC 12

struct Tm_s
{
    int16_t tm_year;  // e.g. 2012
    int8_t tm_sec;    // 0 - 59
    int8_t tm_min;    // 0 - 59
    int8_t tm_hour;   // 0 - 23
    int8_t tm_mday;   // 1 - 31
    int8_t tm_mon;    // 1 - 12
    int8_t tm_wday;   // 1 - 7, sun = 1 (US standard numbering)
    int16_t tm_yday;  // 1 - 366 Jan 1 = 1 (US standard numbering)
    int8_t tm_isdst;  // >0 = yes, 0 = no, -1 = unknown (UNIX std)
    bool tm_isvalid;  // true = valid
};

#define __tm_t_defined
typedef struct Tm_s tm_t;

#define __time_t_defined

/* Why does this structure exist?  It includes clock management
 * variables, and is stored in the nonvolatile register area. */
struct Rtc_data_s
{
    tm_t tm_s;  // Most recent read of clock.
};

typedef struct Rtc_data_s rtc_data_t;

extern tm_t rtc_copy;

#define RTC_SEC rtc_copy.tm_sec
#define RTC_MIN rtc_copy.tm_min
#define RTC_HOUR rtc_copy.tm_hour
#define RTC_DAY rtc_copy.tm_wday
#define RTC_DATE rtc_copy.tm_mday
#define RTC_MON rtc_copy.tm_mon
#define RTC_YEAR rtc_copy.tm_year

#define rtc_yday (rtc_copy.tm_yday)        // Year-day, last read ( 1 ~ 365)
#define rtc_isvalid (rtc_copy.tm_isvalid)  // Is RTC valid?
#define rtc_isdst (rtc_copy.tm_isdst)      // Is it DST? (1=yes,-1=?)

// Reset RTC to defaults, if necessary.
extern bool rtc_init(void);

// set the clock
extern void rtc_write(void);

// Read the clock into the global clock data.
extern void rtc_read(void);

// Set the alarm clock
extern void rtc_alarm_set(uint8_t tm_hour, uint8_t tm_min);

// Unset the alarm clock.
extern void rtc_alarm_clear(void);

// Do software temperature compensation of RTC.
// void rtc_compensation (void);

// Set the compensation data into the hardware.
extern void rtc_set_trim(void);

// Get the compensation data from the hardware.
extern void rtc_get_trim(void);

// In this IC, hardware temperature adjustments are stored in TC_A...
// Then, if they are set, rtc_init() turns on the hardware compensation.
// This routine forces the hardware adjustment to a fixed value,
// and disables the hardware compensation.
void rtc_adjust_trim(bool clr_cnt, int32_t ppb);

// seconds since midnight GMT, january 1, 2000
// This and rtc_localtime, below, can implement most calendar functions
extern time_t rtc_mktime(tm_t *tm_ptr);

// Get time and date from a count of seconds since 2000-01-01 00:00 GMT.
extern void rtc_gmtime(tm_t *tm_ptr, time_t sec_cnt);

// Get time and date from a count of seconds since 2000-01-01 00:00 GMT.
extern void rtc_localtime(tm_t *tm_ptr, time_t sec_cnt);

// Get the difference in time between two times.
extern int32_t rtc_delta_seconds(tm_t *start_tm_ptr, tm_t *end_tm_ptr);

// tests the global RAM copy of the RTC reading, rtc_copy.
extern uint8_t rtc_valid(void);

// is the year a leap-year?
extern bool rtc_leap_year(tm_t *tm_ptr);

// calculates which day in the year the date is
extern int16_t rtc_get_yday(tm_t *tm_ptr);

// Used to get access to shadow registers.
bool rtc_shadow_busy_wait(int32_t bit);
void rtcsub_wait(int cnt);

#endif
