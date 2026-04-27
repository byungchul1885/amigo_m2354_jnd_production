#include "options.h"
#include "delay.h"
#include "afe.h"
#include "ser.h"
#include "tmp.h"
#include "timer.h"
#include "eeprom_at24cm02.h"
#if 0 /* bccho, LCD, 2023-07-15 */
#include "lcd.h"
#endif /* bccho */
#include "irq.h"
#include "meter.h"
#include "comm.h"
#include "key.h"
#include "disp.h"
#include "lp.h"
#include "port.h"
#include "bat.h"
#include "batmode.h"
#if 0 /* bccho, FLASH, 2023-07-15 */
#include "flash.h"
#endif /* bccho */

volatile bool b_flash_test = false;

volatile bool reset_flag;  // true = a reset has occurred.
void swna_test_init(void);
void foreground(void);

extern bool xfer_done;
void afe_ready_data(void);

char* dsm_rate_string(rate_type rate)
{
    switch (rate)
    {
    case eArate:
        return "A";
    case eBrate:
        return "B";
    case eCrate:
        return "C";
    case eDrate:
        return "D";
#if defined(FEATURE_TOU_8RATE)
    case eErate:
        return "E";
    case eFrate:
        return "F";
    case eGrate:
        return "G";
    case eHrate:
        return "H";
#endif
    default:
        return "T";
    }
}

char* dsm_rate_mask_string(uint8_t mask)
{
    static char buf[24];
    char* p = buf;
    int i;
    int first = 1;

    if (mask == 0)
    {
        buf[0] = 'T';
        buf[1] = 0;
        return buf;
    }

    for (i = 0; i < 8; i++)
    {
        if (mask & (1 << i))
        {
            if (!first)
                *p++ = '+';

            *p++ = (char)('A' + i);
            first = 0;
        }
    }

    *p = 0;
    return buf;
}

void background(void)
{
    meter_task();
    timer_process();
}

void MmodeCHG_sr_dr_type_sr_dr_is_proc(void);
void foreground(void)
{
    MmodeCHG_sr_dr_type_sr_dr_is_proc();
    whm_proc();
    key_proc();
    disp_proc();
    LP_proc();
    whm_bakup_crc_calc();
}

void error_software(void) {}

void lcd_seg_for_dio_init(void) {}

void port_inmode(void)
{
    tx485_inmode();
    relay_port_inmode();
    eep_wp_inmode();
    EOI_WPORT_inmode();
    EOI_VPORT_inmode();
    EOI_selWPORT_inmode();
    EOI_selVPORT_inmode();
    pwrfail_ind_inmode();
    BATDET_V3P3DISABLE_inmode();
}
