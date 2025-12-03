/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "options_sel.h"
#include "main.h"
#include "amg_task.h"
#include "appl.h"
#include "whm.h"
#include "nv.h"
#include "amg_rtc.h"
#include "amg_power_mnt.h"
#include "amg_stock_op_mode.h"
#include "amg_utc_util.h"
#include "amg_gpio.h"

/*
******************************************************************************
*   Definition
******************************************************************************
*/
#define _D "[STOCK] "

#define STOCK_WINDOW_SIZE 36000     // second ( 10 hour )
#define STOCK_SUB_WINDOW_SIZE 1000  // mili-second ( 1 sec )
#define STOCK_MIN_INTERVAL 1        // second
#define STOCK_SUB_MIN_INTERVAL 1    // mili-second

#define STOCK_SEED_CREATE_HOUR 20
#define STOCK_SEED_CREATE_MIN 0
#define STOCK_SEED_CREATE_SEC 0

#if 1 /* bccho, 2023-11-23, 100 ms로 원복 */
#define STOCK_INT_MODEM_PWR_KEEP_TIME 100
#else
#define STOCK_INT_MODEM_PWR_KEEP_TIME \
    100  // hw reset time(10ms) + imodem sf boot min(40) = 50ms -> *2 = 100ms
#endif

#define STOCK_OP_START_1_TIME_SEC (20 * 60 * 60)
#define STOCK_OP_END_1_TIME_SEC (24 * 60 * 60)
#define STOCK_OP_START_2_TIME_SEC (0)
#define STOCK_OP_END_2_TIME_SEC (6 * 60 * 60)
#define STOCK_nonOP_START_TIME_SEC (6 * 60 * 60)
#define STOCK_nonOP_END_TIME_SEC (20 * 60 * 60)

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

ST_RAND_TX_INFO gst_rand_txinfo;
uint32_t g_stock_fsm = STOCK_FSM_NONE;
uint32_t g_stock_evt = 0;
uint16_t g_stock_op_times;

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

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/
#ifdef STOCK_OP /* bccho, 2024-09-26 */
void dsm_stock_op_init(void)
{
    memset((uint8_t *)&gst_rand_txinfo, 0x00, sizeof(ST_RAND_TX_INFO));
    g_stock_fsm = STOCK_FSM_NONE;
}

char *dsm_stock_op_fsm_string(uint32_t fsm)
{
    switch (fsm)
    {
    case STOCK_FSM_NONE:
        return "STOCK_FSM_NONE";
    case STOCK_FSM_WAKE_DEFAULT:
        return "STOCK_FSM_WAKE_DEFAULT";
    case STOCK_FSM_WAKE_ALA_1:
        return "STOCK_FSM_WAKE_ALA_1";
    case STOCK_FSM_WAKE_ALA_2:
        return "STOCK_FSM_WAKE_ALA_2";

    default:
        return "STOCK_FSM_Unknown";
    }
}

void dsm_stock_set_fsm(uint8_t state)
{
    if (g_stock_fsm != state)
    {
        DPRINTF(DBG_WARN, _D "FSM[%s -> %s]\r\n",
                dsm_stock_op_fsm_string(g_stock_fsm),
                dsm_stock_op_fsm_string(state));
        g_stock_fsm = state;
    }
}

uint8_t dsm_stock_op_get_fsm(void) { return g_stock_fsm; }

ST_RAND_TX_INFO *dsm_stock_op_get_rand_txinfo(void) { return &gst_rand_txinfo; }

void dsm_stock_op_times_reset(void)
{
    ST_STOCK_OP_TIMES st_stock_op_times;

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    memset(&st_stock_op_times, 0x00, sizeof(ST_STOCK_OP_TIMES));
    nv_write(I_STOCK_OP_TIMES, (uint8_t *)&st_stock_op_times);
    g_stock_op_times = 0;
}

uint16_t dsm_stock_op_times_read(void)
{
    ST_STOCK_OP_TIMES st_stock_op_times;

    nv_read(I_STOCK_OP_TIMES, (uint8_t *)&st_stock_op_times);
    g_stock_op_times = st_stock_op_times.cnt;

    DPRINTF(DBG_TRACE, "%s: %d\r\n", __func__, g_stock_op_times);

    return st_stock_op_times.cnt;
}

void dsm_stock_op_times_write(void)
{
    ST_STOCK_OP_TIMES st_stock_op_times;

    st_stock_op_times.cnt = g_stock_op_times;
    nv_write(I_STOCK_OP_TIMES, (uint8_t *)&st_stock_op_times);

    DPRINTF(DBG_TRACE, "%s: %d\r\n", __func__, g_stock_op_times);
}

void dsm_stock_op_random_seed_init(void)
{
    uint8_t seed[4] = {0};
    uint32_t tick, seed_32 = 0;
    int16_t ret = 0;
    uint8_t manuf_id[MANUF_ID_SIZE];
    uint8_t cust_id[SERIAL_NO_SIZE];
    uint16_t sec_1, sec_2, ms_1, ms_2;
    ST_RAND_TX_INFO *p_rand_txinfo = dsm_stock_op_get_rand_txinfo();

    get_manuf_id(manuf_id);
    get_cust_id(cust_id);
    tick = dsm_rtc_get_time();

#if 1 /* bccho, KEYPAIR, 2023-07-15 */
    CRYPTO_STATUS ac_ret = axiocrypto_random(seed, 4);
    if (ac_ret == CRYPTO_SUCCESS)
    {
        ToH32((U8_16_32 *)&seed_32, seed);
        DPRINT_HEX(DBG_TRACE, "TRNG", seed, 4, DUMP_ALWAYS);
        seed_32 += (tick + manuf_id[0] + manuf_id[1] + manuf_id[3] +
                    cust_id[0] + cust_id[1] + cust_id[2] + cust_id[3] +
                    cust_id[4] + cust_id[5] + cust_id[6]);
        ToComm32(&seed[0], (U8_16_32 *)&seed_32);
        DPRINT_HEX(DBG_TRACE, "SEED_KSE", seed, 4, DUMP_ALWAYS);
        nv_write(I_RANDOM_SEED, (uint8_t *)&seed);
    }
    else
    {
        MSGERROR("axiocrypto_random error: %d", ac_ret);
        seed_32 += (tick + manuf_id[0] + manuf_id[1] + manuf_id[3] +
                    cust_id[0] + cust_id[1] + cust_id[2] + cust_id[3] +
                    cust_id[4] + cust_id[5] + cust_id[6]);
        ToComm32(&seed[0], (U8_16_32 *)&seed_32);
        nv_write(I_RANDOM_SEED, (uint8_t *)&seed);
        DPRINT_HEX(DBG_TRACE, "SEED_2", seed, 4, DUMP_ALWAYS);
    }
#else
    ret = _kcmvpDrbg(seed, 4);

    if (ret == /*KSE_SUCCESS*/ 0)
    {
        ToH32((U8_16_32 *)&seed_32, seed);
        DPRINT_HEX(DBG_TRACE, "TRNG", seed, 4, DUMP_ALWAYS);
        seed_32 += (tick + manuf_id[0] + manuf_id[1] + manuf_id[3] +
                    cust_id[0] + cust_id[1] + cust_id[2] + cust_id[3] +
                    cust_id[4] + cust_id[5] + cust_id[6]);
        ToComm32(&seed[0], (U8_16_32 *)&seed_32);
        DPRINT_HEX(DBG_TRACE, "SEED", seed, 4, DUMP_ALWAYS);
        nv_write(I_RANDOM_SEED, (uint8_t *)&seed);
    }
    else
    {
        seed_32 += (tick + manuf_id[0] + manuf_id[1] + manuf_id[3] +
                    cust_id[0] + cust_id[1] + cust_id[2] + cust_id[3] +
                    cust_id[4] + cust_id[5] + cust_id[6]);
        ToComm32(&seed[0], (U8_16_32 *)&seed_32);
        nv_write(I_RANDOM_SEED, (uint8_t *)&seed);
        DPRINT_HEX(DBG_TRACE, "SEED_2nd", seed, 4, DUMP_ALWAYS);
    }
#endif /* bccho */

    DPRINTF(DBG_WARN, "SEED_VAL: 0x%08X\r\n", seed_32);

    // power up 시 rand time 선출
    srand(seed_32);
    sec_1 = rand() % ((STOCK_WINDOW_SIZE - 1) + 1);
    sec_2 = rand() % ((STOCK_WINDOW_SIZE - 1) + 1);
    ms_1 = rand() % ((STOCK_SUB_WINDOW_SIZE - 1));
    ms_2 = rand() % ((STOCK_SUB_WINDOW_SIZE - 1));
    if (sec_1 < sec_2)
    {
        p_rand_txinfo->sec_1 = sec_1;
        p_rand_txinfo->ms_1 = ms_1;
        p_rand_txinfo->sec_2 = sec_2;
        p_rand_txinfo->ms_2 = ms_2;
    }
    else
    {
        p_rand_txinfo->sec_1 = sec_2;
        p_rand_txinfo->ms_1 = ms_2;
        p_rand_txinfo->sec_2 = sec_1;
        p_rand_txinfo->ms_2 = ms_1;
    }

    uint8_t day_hour, wakeup_time[20] = {0};
    day_hour = p_rand_txinfo->sec_1 / 60 / 60 + 20;
    sprintf((char *)wakeup_time, "[%02d:%02d:%02d.%03d]",
            (day_hour >= 24) ? (day_hour - 24) : day_hour,
            (uint8_t)(p_rand_txinfo->sec_1 / 60 % 60),
            (uint8_t)(p_rand_txinfo->sec_1 % 60), p_rand_txinfo->ms_1);
    DPRINTF(DBG_INFO, _D "Init RANDOM_1: %s, sec[%d], ms[%d]\r\n", wakeup_time,
            p_rand_txinfo->sec_1, p_rand_txinfo->ms_1);

    day_hour = p_rand_txinfo->sec_2 / 60 / 60 + 20;
    sprintf((char *)wakeup_time, "[%02d:%02d:%02d.%03d]",
            (day_hour >= 24) ? (day_hour - 24) : day_hour,
            (uint8_t)(p_rand_txinfo->sec_2 / 60 % 60),
            (uint8_t)(p_rand_txinfo->sec_2 % 60), p_rand_txinfo->ms_2);
    DPRINTF(DBG_INFO, _D "Init RANDOM_2: %s, sec[%d], ms[%d]\r\n", wakeup_time,
            p_rand_txinfo->sec_2, p_rand_txinfo->ms_2);

    p_rand_txinfo->seed = seed_32;
}

void dsm_stock_op_create_random_time(ST_RAND_TX_INFO *p_rand_txinfo)
{
    uint8_t seed[4];
    uint32_t seed_32 = 0;
    uint16_t sec_1, sec_2, ms_1, ms_2;

    memset(seed, 0x00, 4);
    if (!nv_read(I_RANDOM_SEED, (uint8_t *)&seed))
    {
        uint32_t tick;
        uint8_t manuf_id[MANUF_ID_SIZE];
        uint8_t cust_id[SERIAL_NO_SIZE];
        DPRINTF(DBG_ERR, "%s: nv read error\r\n", __func__);
        DPRINTF(DBG_WARN, "SEED_2nd REGEN\r\n");
        get_manuf_id(manuf_id);
        get_cust_id(cust_id);
        tick = dsm_rtc_get_time();
        seed_32 += (tick + manuf_id[0] + manuf_id[1] + manuf_id[3] +
                    cust_id[0] + cust_id[1] + cust_id[2] + cust_id[3] +
                    cust_id[4] + cust_id[5] + cust_id[6]);
        ToComm32(&seed[0], (U8_16_32 *)&seed_32);
    }

    ToH32((U8_16_32 *)&seed_32, seed);
    DPRINTF(DBG_TRACE, "SEED_VAL: 0x%08X\r\n", seed_32);

    srand(seed_32);

    sec_1 = rand() % ((STOCK_WINDOW_SIZE - 1) + 1);
    sec_2 = rand() % ((STOCK_WINDOW_SIZE - 1) + 1);
    ms_1 = rand() % ((STOCK_SUB_WINDOW_SIZE - 1));
    ms_2 = rand() % ((STOCK_SUB_WINDOW_SIZE - 1));
    if (sec_1 < sec_2)
    {
        p_rand_txinfo->sec_1 = sec_1;
        p_rand_txinfo->ms_1 = ms_1;
        p_rand_txinfo->sec_2 = sec_2;
        p_rand_txinfo->ms_2 = ms_2;
    }
    else
    {
        p_rand_txinfo->sec_1 = sec_2;
        p_rand_txinfo->ms_1 = ms_2;
        p_rand_txinfo->sec_2 = sec_1;
        p_rand_txinfo->ms_2 = ms_1;
    }

    uint8_t day_hour, wakeup_time[20] = {0};
    day_hour = p_rand_txinfo->sec_1 / 60 / 60 + 20;
    sprintf((char *)wakeup_time, "[%02d:%02d:%02d.%03d]",
            (day_hour >= 24) ? (day_hour - 24) : day_hour,
            (uint8_t)(p_rand_txinfo->sec_1 / 60 % 60),
            (uint8_t)(p_rand_txinfo->sec_1 % 60), p_rand_txinfo->ms_1);
    DPRINTF(DBG_ERR, _D "RANDOM_1: %s, sec[%d], ms[%d]\r\n", wakeup_time,
            p_rand_txinfo->sec_1, p_rand_txinfo->ms_1);
    day_hour = p_rand_txinfo->sec_2 / 60 / 60 + 20;
    sprintf((char *)wakeup_time, "[%02d:%02d:%02d.%03d]",
            (day_hour >= 24) ? (day_hour - 24) : day_hour,
            (uint8_t)(p_rand_txinfo->sec_2 / 60 % 60),
            (uint8_t)(p_rand_txinfo->sec_2 % 60), p_rand_txinfo->ms_2);
    DPRINTF(DBG_ERR, _D "RANDOM_2: %s, sec[%d], ms[%d]\r\n", wakeup_time,
            p_rand_txinfo->sec_2, p_rand_txinfo->ms_2);

    seed_32 += (p_rand_txinfo->sec_1 + p_rand_txinfo->sec_2 +
                p_rand_txinfo->ms_1 + p_rand_txinfo->ms_2);
    memset(seed, 0x00, 4);
    ToComm32(&seed[0], (U8_16_32 *)&seed_32);
    nv_write(I_RANDOM_SEED, (uint8_t *)&seed);
    DPRINTF(DBG_WARN, _D "SEED_NEW: 0x%08X\r\n", seed_32);

    p_rand_txinfo->seed = seed_32;
}

uint32_t dsm_stock_op_get_seed_create_time(void)
{
    uint32_t sec_tick;
    DATE_TIME_T date_time_t;

    dsm_rtc_get_hw_time(&date_time_t);
    date_time_t.hour = STOCK_SEED_CREATE_HOUR;
    date_time_t.min = STOCK_SEED_CREATE_MIN;
    date_time_t.sec = STOCK_SEED_CREATE_SEC;
    sec_tick = dsm_convet_time_to_sec(&date_time_t);

    return sec_tick;
}

void dsm_stock_op_alarm_set_default(void)
{
    DATE_TIME_T ptime;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    dsm_rtc_get_hw_time(&ptime);
#ifdef RTC_ALARM_INT_TEST
    if (ptime.min == 59)
    {
        ptime.min = 0;
    }
    else
    {
        ptime.min = ptime.min + 1;
    }
#else
    ptime.hour = STOCK_SEED_CREATE_HOUR;
    ptime.min = STOCK_SEED_CREATE_MIN;
#endif
    ptime.sec = STOCK_SEED_CREATE_SEC;
    dsm_rtc_set_alarm_time(E_ALARM_A, &ptime, dsm_pmnt_alarm_a_callback);

    dsm_stock_set_fsm(STOCK_FSM_WAKE_DEFAULT);
}

void dsm_stock_op_alarm_set_rtime_1(void)
{
    DATE_TIME_T ptime;

#ifdef RTC_ALARM_INT_TEST
    dsm_rtc_get_hw_time(&ptime);
    if (ptime.min == 59)
    {
        ptime.min = 0;
    }
    else
    {
        ptime.min = ptime.min + 1;
    }
#else
    ST_RAND_TX_INFO *p_rand_txinfo = dsm_stock_op_get_rand_txinfo();
    uint32_t sec_tick = dsm_stock_op_get_seed_create_time();

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    sec_tick += p_rand_txinfo->sec_1;
    dsm_convert_utc_to_time(sec_tick, &ptime);
#endif
    dsm_rtc_set_alarm_time(E_ALARM_A, &ptime, dsm_pmnt_alarm_a_callback);

    dsm_stock_set_fsm(STOCK_FSM_WAKE_ALA_1);
}

void dsm_stock_op_alarm_set_rtime_2(void)
{
    DATE_TIME_T ptime;

#ifdef RTC_ALARM_INT_TEST
    dsm_rtc_get_hw_time(&ptime);
    if (ptime.min == 59)
    {
        ptime.min = 0;
    }
    else
    {
        ptime.min = ptime.min + 1;
    }
#else
    ST_RAND_TX_INFO *p_rand_txinfo = dsm_stock_op_get_rand_txinfo();
    uint32_t sec_tick = dsm_stock_op_get_seed_create_time();

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    sec_tick += p_rand_txinfo->sec_2;
    dsm_convert_utc_to_time(sec_tick, &ptime);
#endif
    dsm_rtc_set_alarm_time(E_ALARM_A, &ptime, dsm_pmnt_alarm_a_callback);

    dsm_stock_set_fsm(STOCK_FSM_WAKE_ALA_2);
}

bool dsm_is_ctime_stock_no_op_time(uint32_t *ctime_sec)
{
    bool ret = FALSE;
    DATE_TIME_T ctime;
    uint32_t sec;

    dsm_rtc_get_hw_time(&ctime);

    sec = (ctime.hour * 60 * 60) + (ctime.min * 60) + ctime.sec;
    *ctime_sec = sec;

    if (sec > STOCK_nonOP_START_TIME_SEC && sec < STOCK_nonOP_END_TIME_SEC)
    {
        ret = TRUE;
    }
    return ret;
}
/*
    20:00 ~ 23:59:59
*/
bool dsm_is_ctime_stock_op_1_time(void)
{
    bool ret = FALSE;
    DATE_TIME_T ctime;
    uint32_t sec;

    dsm_rtc_get_hw_time(&ctime);

    sec = (ctime.hour * 60 * 60) + (ctime.min * 60) + ctime.sec;

    if (sec >= STOCK_OP_START_1_TIME_SEC && sec < STOCK_OP_END_1_TIME_SEC)
    {
        ret = TRUE;
    }
    return ret;
}
/*
    00:00 ~ 05:59:59
*/
bool dsm_is_rtime_stock_op_1_time(uint32_t sec)
{
    bool ret = FALSE;

    if (sec >= STOCK_OP_START_1_TIME_SEC && sec < STOCK_OP_END_1_TIME_SEC)
    {
        ret = TRUE;
    }
    return ret;
}

void dsm_stock_op_alarm_setup_at_STOCK_FSM_NONE(void)
{
    uint32_t ctime_sec;
    int32_t sec_1, sec_2;
    ST_RAND_TX_INFO *p_rand_txinfo = dsm_stock_op_get_rand_txinfo();
    bool no_op = dsm_is_ctime_stock_no_op_time(&ctime_sec);

    sec_1 = p_rand_txinfo->sec_1 + STOCK_OP_START_1_TIME_SEC;
    if (sec_1 >= STOCK_OP_END_1_TIME_SEC)
        sec_1 -= STOCK_OP_END_1_TIME_SEC;

    sec_2 = p_rand_txinfo->sec_2 + STOCK_OP_START_1_TIME_SEC;
    if (sec_2 >= STOCK_OP_END_1_TIME_SEC)
        sec_2 -= STOCK_OP_END_1_TIME_SEC;

    DPRINTF(DBG_TRACE, _D "%s: ctime_sec %d, sec_1 %d, sec_2 %d\r\n", __func__,
            ctime_sec, sec_1, sec_2);

    if (no_op)
    {
        /*==================================*/
        /************************************/
        /*ctime domain 0 :: 06:00 ~ 19:59:59*/
        /************************************/
        /*==================================*/

        dsm_stock_op_alarm_set_default();
    }
    else
    {
        if (dsm_is_ctime_stock_op_1_time())
        {
            /*==================================*/
            /************************************/
            /*ctime domain 1 :: 20:00 ~ 23:59:59*/
            /************************************/
            /*==================================*/
            if (dsm_is_rtime_stock_op_1_time(sec_1))
            {
                /************************************/
                /*sec_1 domain 1 :: 20:00 ~ 23:59:59*/
                /************************************/
                if (ctime_sec < sec_1)
                {
                    dsm_stock_op_alarm_set_rtime_1();
                }
                else /*ctime_sec >= p_rand_txinfo->sec_1 : sec_1 은 고려 하지
                        않는다. */
                {
                    if (dsm_is_rtime_stock_op_1_time(sec_2))
                    {
                        /************************************/
                        /*sec_2 domain 1 :: 20:00 ~ 23:59:59*/
                        /************************************/
                        if (ctime_sec < sec_2)
                        {
                            dsm_stock_op_alarm_set_rtime_2();
                        }
                        else
                        {  // ctime, sec_2 domain 1 이나 ctime 이 더 큰 경우
                           // default setup
                            dsm_stock_op_alarm_set_default();
                        }
                    }
                    else
                    {
                        /************************************/
                        /*sec_2 domain 2 :: 00:00 ~ 05:59:59*/
                        /************************************/
                        // ctime doman 1 < sec_2 dmain 2 : sec_2 이 큰
                        // 시간으므로 rt_time 1 setup
                        dsm_stock_op_alarm_set_rtime_2();
                    }
                }
            }
            else
            {
                /************************************/
                /*sec_1 domain 2 :: 00:00 ~ 05:59:59*/
                /************************************/
                // ctime doman 1 < sec_1 dmain 2 : sec_1 이 큰 시간으므로
                // rt_time 1 setup
                dsm_stock_op_alarm_set_rtime_1();
            }
        }
        else
        {
            /*==================================*/
            /************************************/
            /*ctime domain 2 :: 00:00 ~ 05:59:59*/
            /************************************/
            /*==================================*/
            if (dsm_is_rtime_stock_op_1_time(sec_1))
            {
                /************************************/
                /*sec_1 domain 1 :: 20:00 ~ 23:59:59*/
                /************************************/
                // ctime doman 2 > sec_1 dmain 1 : sec_1 보다ctime 이 크므로
                // sec_1 고려 하지 않는다.

                if (dsm_is_rtime_stock_op_1_time(sec_2))
                {
                    /************************************/
                    /*sec_2 domain 1 :: 20:00 ~ 23:59:59*/
                    /************************************/
                    // ctime doman 2 > sec_2 dmain 1 : sec_2 보다ctime 이 크므로
                    // sec_1 고려 하지 않는다. 이경우 default setup
                    dsm_stock_op_alarm_set_default();
                }
                else
                {
                    /************************************/
                    /*sec_2 domain 2 :: 00:00 ~ 05:59:59*/
                    /************************************/
                    if (ctime_sec < sec_2)
                    {
                        dsm_stock_op_alarm_set_rtime_2();
                    }
                    else
                    {  // ctime, sec_2 domain 2 이나 ctime 이 더 큰 경우 default
                       // setup
                        dsm_stock_op_alarm_set_default();
                    }
                }
            }
            else
            {
                /************************************/
                /*ctime domain 2                    */
                /*sec_1 domain 2 :: 00:00 ~ 05:59:59*/
                /************************************/
                if (ctime_sec < sec_1)
                {
                    dsm_stock_op_alarm_set_rtime_1();
                }
                else
                {  // ctime, sec_1 domain 2 이나 ctime 이 더 큰 경우이므로 sec_1
                   // 고려 하지 않는다.
                    if (ctime_sec < sec_2)
                    {
                        dsm_stock_op_alarm_set_rtime_2();
                    }
                    else
                    {  // ctime, sec_2 domain 2 이나 ctime 이 더 큰 경우 default
                       // setup
                        dsm_stock_op_alarm_set_default();
                    }
                }
            }
        }
    }
}

/* 계기 시 기준 20:00 - 06:00에 2회 패킷 발송 */
/* 계기는 재고 관리 동작 시 매일 20:00시에 Wakeup 하여 당일분 2회 발송 시간을
 * 계산하여 테이블로 구성하여야 한다*/
void dsm_stock_op_proc_for_lowpower_entry(uint32_t w_evt)
{
    uint32_t fsm = dsm_stock_op_get_fsm();
    ST_RAND_TX_INFO *p_rand_txinfo = dsm_stock_op_get_rand_txinfo();

    DPRINTF(DBG_TRACE, _D "%s: C_FSM[%s], Register Evt[%08X]\r\n", __func__,
            dsm_stock_op_fsm_string(fsm), w_evt);

    /* Alarm A */
    switch (fsm)
    {
    case STOCK_FSM_NONE:
        /* Default : Daily Alarm Set 20:00 */
        if (WUP_S_RTC_ALA & w_evt)
        {
            dsm_stock_op_alarm_setup_at_STOCK_FSM_NONE();
        }
        break;

    case STOCK_FSM_WAKE_DEFAULT: /* 20:00에 깨어남*/
                                 /* Daily Alarm -> 1st Alarm Set */
        if (WUP_S_RTC_ALA & w_evt)
        {
            dsm_stock_op_create_random_time(p_rand_txinfo);
            dsm_stock_op_alarm_set_rtime_1();
        }
        break;

    case STOCK_FSM_WAKE_ALA_1:
        /* 1st Alarm -> 2nd Alarm Set */
        if (WUP_S_RTC_ALA & w_evt)
        {
            dsm_stock_op_alarm_set_rtime_2();
        }
        break;

    case STOCK_FSM_WAKE_ALA_2:
        /* 2nd Alarm -> Daily Alarm Set */
        if (WUP_S_RTC_ALA & w_evt)
        {
            /* 20:00으로 알람 설정 */
            dsm_stock_op_alarm_set_default();
        }
        break;

    default:
        break;
    }
}

void dsm_stock_action(void)
{
    DPRINTF(DBG_TRACE, "\tI-modem power on\r\n");
#if 0 /* bccho, 2023-11-25, 아래로 대체 */
    dsm_gpio_imodem_io_init(__IMODEM_PF_STOCK_OP);

    OSTimeDly(OS_MS2TICK(STOCK_INT_MODEM_PWR_KEEP_TIME));

    dsm_gpio_imodem_power_disable();
#else
    PC7 = 1; /* power_enable */
    PG4 = 1; /* reset high */
    vTaskDelay(100);
    PC7 = 0; /* power_disable */
#endif
    DPRINTF(DBG_TRACE, "\tI-modem power off\r\n");
}
#endif
