/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include <stdlib.h>
#include "utils.h"
#include "defines.h"
#include "amg_time.h"
#include "amg_utc_util.h"

/*
******************************************************************************
* 	DEFINES
******************************************************************************
*/
#define _D "[UTI] "

/* Time difference를 나타내기 위한 매크로 */
#define ONE_MINUTE (60)
#define ONE_HOUR (ONE_MINUTE * 60)
#define ONE_DAY (ONE_HOUR * 24)
#define ONE_MONTH (ONE_DAY * 30)  // 1개월은 30일로 설정
#define ONE_YEAR (ONE_DAY * 365)

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
*	LOCAL FUNCTIONS
******************************************************************************
*/

/*
    BCD형태 날짜,시간 Array를 String 형태로 보여주도록 함.
*/
char *util_get_date_time_string(uint8_t *p_bcd, char *buff)
{
    static char bcd_time_str[32];
    char *str = buff ? buff : bcd_time_str;

    memset(bcd_time_str, 0x00, sizeof(bcd_time_str));

    sprintf(str, "20%02X-%02X-%02X %02X:%02X:%02X", p_bcd[T_YEAR], p_bcd[T_MON],
            p_bcd[T_DAY], p_bcd[T_HOUR], p_bcd[T_MIN], p_bcd[T_SEC]);

    return str;
}

/*
    Current System Time을 return하는 함수

PARAMETER
 1'st : struct tm 구조체 형식의 pointer
 2'nd : BCD형식의 time pointer

*/
void util_get_system_time(struct tm *pSysTime, ST_TIME_BCD *pstBCD)
{
    struct timeval tmCurrent;
    ST_TIME_BCD nTime;

    gettimeofday(&tmCurrent, NULL);
    localtime_r(&tmCurrent.tv_sec, pSysTime);

    if (pstBCD != NULL)
    {
        nTime.aucTimeBCD[0] = pSysTime->tm_year - 100;
        nTime.aucTimeBCD[1] = pSysTime->tm_mon + 1;
        nTime.aucTimeBCD[2] = pSysTime->tm_mday;
        nTime.aucTimeBCD[3] = (pSysTime->tm_wday == 0) ? 7 : pSysTime->tm_wday;
        nTime.aucTimeBCD[4] = pSysTime->tm_hour;
        nTime.aucTimeBCD[5] = pSysTime->tm_min;
        nTime.aucTimeBCD[6] = pSysTime->tm_sec;
        pstBCD->aucTimeBCD[0] = DEC2BCD(nTime.aucTimeBCD[0]);
        pstBCD->aucTimeBCD[1] = DEC2BCD(nTime.aucTimeBCD[1]);
        pstBCD->aucTimeBCD[2] = DEC2BCD(nTime.aucTimeBCD[2]);
        pstBCD->aucTimeBCD[3] = DEC2BCD(nTime.aucTimeBCD[3]);
        pstBCD->aucTimeBCD[4] = DEC2BCD(nTime.aucTimeBCD[4]);
        pstBCD->aucTimeBCD[5] = DEC2BCD(nTime.aucTimeBCD[5]);
        pstBCD->aucTimeBCD[6] = DEC2BCD(nTime.aucTimeBCD[6]);
    }
}
