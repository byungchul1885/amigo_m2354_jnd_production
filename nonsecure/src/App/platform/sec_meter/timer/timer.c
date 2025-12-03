#include "options.h"
#include "port.h"
#include "afe.h"
#include "key.h"
#include "eoi.h"
#include "whm_1.h"
#include "disp.h"
#include "timer.h"
#include "amg_debug.h"
#include "amg_power_mnt.h"

extern uint8_t tmr_tick_cnt;
extern bool METER_FW_UP_ING_STS;
extern bool METER_FW_UP_END_PULSE_MODIFY;

bool test_lcd_on_sts = 0;  // jp.kim 24.11.08  LCD창 "TEST" 표시
U16 job_1sec_timer = 0;

#if 1 //jp.kim 25.06.30
int prev_keyrd = 0;
int keyrd_chat = -1;		// initially different from keyrd
int keychat = 0;
int keycont_s = 0;
int keycont_l = 0;
#endif

uint32_t meter_get_measure_flag(void);
void meter_set_measure_flag(uint32_t flag);
uint32_t meter_get_measure_xdone_timer(void);
void meter_set_measure_xdone_timer(uint32_t value);

bool meter_fwup_pulse_protect(void);

/* bccho, 2023-07-11, 10ms 마다 호출된다 */
void timerB_isr(void)
{
    int keyrd = 0;

    tmr_tick_cnt++;

    job_1sec_timer++;
    if (job_1sec_timer > T1SEC)
    {
        bool meter_fwup_pulse_protect(void);

        job_1sec_timer = 0;
        if (meter_fwup_pulse_protect())
        {
            meter_set_measure_flag(TRUE);
            meter_set_measure_xdone_timer(0);
        }
    }

    xfer_done_timer++;
    if (xfer_done_timer == 0)
    {
        xfer_done_timer -= 1;
    }

    if (relay_onff_req)
    {
        if (relay_onff_timer)
        {
            if ((--relay_onff_timer) == 0)
            {
                relay_onff_req = 0;
                relay_set_release();
            }
        }
    }

    if (b_eoi_deactive)
    {
        if ((--eoi_deactive_timer) == 0)
        {
            b_eoi_deactive = 0;

            EOI_WVPORT_DEACTIVE;
        }
    }

    if (b_sag_exit_timer)
    {
        if ((--sag_exit_timer) == 0)
        {
            b_sag_exit_timer = 0;
        }
    }

    if (b_dsp_pulse_inc_timer)
    {
        if (dsp_pulse_inc_timer)
        {
            if ((--dsp_pulse_inc_timer) == 0)
            {
                dsp_pulse_inc_timer = dsp_pulse_inc_timer_bak;

                if (accmed_pls_inc)
                {
                    accmed_pls_inc -= 1;
                    if ((++dsp_pulse_inc) >= 5)
                        dsp_pulse_inc = 0;
                }
            }
        }
        else
        {
            dsp_pulse_inc_timer = dsp_pulse_inc_timer_bak;
        }
    }

    uint8_t cnt = 0;

    // move key
    while ((cnt < 3) && (KEY0_IN))
    {
        cnt++;
    }
    if (cnt >= 3)
    {
        keyrd |= MENUK_BIT;
    }

    cnt = 0;

    // move key
    while ((cnt < 3) && (KEY1_IN))
    {
        cnt++;
    }
    if (cnt >= 3)
    {
        keyrd |= MOVEK_BIT;
    }

    if (!b_inst_key_cancel)
    {
        if (keyrd == keyrd_chat)
        {
            if (keychat)
            {
                keychat--;
            }
            //
            if (keychat == 0)
            {
                if (keyrd != prev_keyrd)
                {
                    if (keyrd == 0)
                    {
                        // key release
                        if (prev_keyrd)
                        {
                            if (keycont_l != 0)
                            {
                                if (keycont_s != 0)
                                {
                                    // non-cont key
                                    key_pressed = true;
                                    key_code = prev_keyrd;
                                }
                                else
                                {
                                    // short-cont key
                                    key_pressed = true;
                                    key_code = (prev_keyrd | CONTK_S_BIT);
                                }
                            }
                        }
                    }
                    else  // keyrd != 0
                    {
                        // key pressed
                        keycont_s = T1_5SEC;
                        keycont_l = T5SEC;
                    }
                }
                else  // keyrd == prev_keyrd
                {
                    // 연속 키 처리 short
                    if (keycont_s)
                    {
                        --keycont_s;
                        if (!keycont_s)
                        {
                            // LCD_DTEST_ON; //jp.kim 24.11.08  LCD창
                            // "TEST" 표시 on
                            if (keyrd & MOVEK_BIT)  // jp.kim 24.11.13
                            {
                                test_lcd_on_sts = 1;
                                dsm_pmnt_move_key_callback_at_NO_VOLT_OP();
                            }
                        }
                    }

                    // 연속 키 처리 long
                    if (keycont_l)
                    {
                        if ((--keycont_l) == 0)
                        {
                            // LCD_DTEST_OFF;  //jp.kim 24.11.08  LCD창
                            // "TEST" 표시 off
                            test_lcd_on_sts = 0;
                            key_pressed = true;
                            key_code = (keyrd | CONTK_L_BIT);
                        }
                    }
                }
                //
                prev_keyrd = keyrd;
            }
            //
        }
        else  // keyrd != keyrd_chat
        {
            keyrd_chat = keyrd;
            keychat = KEY_CHAT_CNT;
        }
    }
    else
    {
        if (key_read_is_timeout())
        {
            b_inst_key_cancel = false;
        }
        keyrd_chat = prev_keyrd = keyrd;
        keycont_l = keycont_s = keychat = 0;

        key_pressed = false;
    }
}

void timer_init(void) {}
