#include "main.h"
#include "options.h"
#include "delay.h"
#include "irq.h"
#include "softimer.h"

uint8_t tmr_tick_cnt;

int sag_exit_timer;
uint8_t b_sag_exit_timer;

int timer_table[NUM_TMR];
uint32_t scurr_mon_timer;
uint32_t scurr_limit_timer;

static void timer_proc(uint8_t tick_cnt);

void init_softtmr(void)
{
    uint16_t i;

    tmr_tick_cnt = 0;

    for (i = 0; i < NUM_TMR; i++) timer_table[i] = 0;

    scurr_mon_timer = 0L;
    scurr_limit_timer = 0L;

    sag_exit_timer = 0;
    b_sag_exit_timer = 0;
}

/* bccho, 2023-07-11, tmr_tick_cnt는 timerB_isr()에서 증가. 10 ms 단위 */
void timer_process(void)
{
    uint8_t _cnt;

    if (tmr_tick_cnt != 0)
    {
        _cnt = tmr_tick_cnt;
        tmr_tick_cnt = 0;

        timer_proc(_cnt);
    }
}

static void timer_proc(uint8_t tick_cnt)
{
    int i;

    MSG00("timer_proc: %d", tick_cnt);

    for (i = 0; i < NUM_TMR; i++)
    {
        if (timer_table[i] <= tick_cnt)
            timer_table[i] = 0;
        else
            timer_table[i] -= tick_cnt;
    }

    if (scurr_mon_timer >= tick_cnt)
    {
        scurr_mon_timer -= tick_cnt;
    }
    else
    {
        scurr_mon_timer = 0;
    }
    if (scurr_limit_timer >= tick_cnt)
    {
        scurr_limit_timer -= tick_cnt;
    }
    else
    {
        scurr_limit_timer = 0;
    }
}

void softtimer_set(uint32_t timer_id, uint32_t tick_ms)
{
    timer_table[timer_id] = tick_ms / TICK_PERIOD;
}