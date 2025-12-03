
#include <stdio.h>
#include "options.h"
#include "FreeRTOS.h"
#include "task.h"
#include "amg_uart.h"
#include "amg_dm_main.h"
#include "amg_meter_main.h"
#include "amg_wdt.h"
#include "amg_pwr.h"
#include "amg_gpio.h"
#include "os_task_q.h"
#include "platform.h"
#include "amg_timer.h"
#include "kepco_cert.h"
#include "hexdump.h"
#include "options.h"
#if 1 /* bccho, 2023-09-15 */
#include "amg_sec.h"
#include "amg_secu_main.h"
#include "amg_stock_op_mode.h"
#include "amg_mtp_process.h"
#include "eadc_vbat.h"
#include "amg_power_mnt.h"
#endif
#ifdef M2354_CAN /* bccho, 2023-11-28 */
#include "amg_can.h"
#include "amg_isotp_user.h"
#endif
/*-----------------------------------------------------------*/

uint8_t major_ver = 0;
uint8_t minor_ver = 0;

void dsm_hw_info_display(void)
{
    major_ver = SW_VER / 100;
    minor_ver = SW_VER % 100;

    dsm_uart_set_poll_mode(DEBUG_COM, TRUE);
    dsm_printf("\r\n");
    DPRINTF(DBG_INFO, "DSM F/W v%d.%02d\t%s %s\r\n", major_ver, minor_ver,
            __TIME__, __DATE__);
    dsm_printf("\r\n");
}

static void dsm_hw_initialize(void)
{
    dsm_hw_info_display();

    dsm_wdt_ext_toggle_immd();
}

#if 1 /* bccho, 2023-09-15 */
static void wdt_task_main(void *arg)
{
    portALLOCATE_SECURE_CONTEXT(512);
    static uint16_t cnt;

    static int main_wdt_fail_consec_count;
    while (1)
    {
        vTaskDelay(10);

        void timerB_isr(void);
        timerB_isr();

#if 0 /* bccho, 2023-12-22, 나중에 필요할 지 모른다 */
        if (cnt % 10 == 0)
        {
            void disp_proc(void);
            disp_proc();
        }
#endif

        if (cnt++ < 5) /* 2024-06-13, 50 -> 5, JnD 출장 수정 */
        {
            continue;
        }
        cnt = 0;

#ifdef SPI_FLASH_TEST /* bccho, 2023-11-13 */
        kick_watchdog_S();
#else
        taskENTER_CRITICAL();
        if (metertask_wdkick_done)
        {
            kick_watchdog_S();
            metertask_wdkick_done = false;
            main_wdt_fail_consec_count = 0;
        }
        else
        {
            main_wdt_fail_consec_count++;

            /* 2024-06-13, 10 -> 1000, JnD 출장 수정 */
            if (main_wdt_fail_consec_count < 1000)
                kick_watchdog_S();
        }
        taskEXIT_CRITICAL();
#endif
    }
}
#endif

static void dsm_task_initialize(void)
{
    dsm_dm_initialize();

    dsm_meter_initialize();

    OSTaskCreateExt("wdt", wdt_task_main, (void *)0, 0, /* 사용안함 */
                    TASK_PRI_DM + 1,                    /* 가장 높게 */
                    TASK_METER,                         /* 사용안함 */
                    0,                                  /* 사용안함 */
                    512,                                /* 스택 크기 */
                    (void *)0,                          /* 사용안함 */
                    0                                   /* 사용안함 */
    );
}

int main(void)
{
#if 1 /* bccho, 2023-07-28 */
    SystemCoreClockUpdate();
    wakeup_source = CheckPowerSource_S();
#endif

    dsm_platform_pre_init();
    printf("\n\nNon-Secure : VECMAP = 0x%x, bank = %d, wakeup = 0x%x\n",
           FMC_GetVECMAP_S(), get_current_bank_S(), wakeup_source);
    printf("Non-Secure CPU @ %d Hz\n", SystemCoreClock);

    static kepco_cert_t kepco_cert = {0};
    int ret;

    printf("Get Kepco certs...");
    ret = get_kepco_certs(&kepco_cert);
    if (ret == 0)
    {
        printf("success\n");

        uint8_t *t = kepco_cert.systemtitle;
        uint8_t sys_t[8];
        sys_t[0] = t[0];
        sys_t[1] = t[1];
        sys_t[2] = t[2];

        uint8_t AsciiToHEX(unsigned char ch);
        for (int i = 3; i < 13; i++)
        {
            t[i] = AsciiToHEX(t[i]);
        }

        sys_t[3] = (t[3] << 4) + t[4];
        sys_t[4] = (t[5] << 4) + t[6];
        sys_t[5] = (t[7] << 4) + t[8];
        sys_t[6] = (t[9] << 4) + t[10];
        sys_t[7] = (t[11] << 4) + t[12];

        printf("SYS_T: %02X %02X %02X %02X %02X %02X %02X %02X", sys_t[0],
               sys_t[1], sys_t[2], sys_t[3], sys_t[4], sys_t[5], sys_t[6],
               sys_t[7]);

        memcpy(SYS_TITLE_server, sys_t, SYS_TITLE_LEN);
        memcpy(DS_privatekey, kepco_cert.sign_prikey, 32);
        memcpy(CERT_DS_BUFF, kepco_cert.sign_cert, 500);
#if 0
        hexlog("Sign cert", kepco_cert.sign_cert, kepco_cert.sign_cert_len);
        hexlog("Km cert", kepco_cert.km_cert, kepco_cert.km_cert_len);
        hexlog("Sign prikey", kepco_cert.sign_prikey,
               sizeof(kepco_cert.sign_prikey));
        hexlog("Km prikey", kepco_cert.km_prikey, sizeof(kepco_cert.km_prikey));
#endif
    }
    else
    {
        printf("Failed\n");
    }

    dsm_hw_initialize();
    dsm_platform_init();
    dsm_task_initialize();

    debug_level = RUN_DBG_LVL;
    // debug_level = DBG_TRACE;
    // debug_level = DBG_NONE;
    // debug_level = DBG_WARN;
    // debug_level = DBG_ERR;

    dsm_uart_set_poll_mode(DEBUG_COM, TRUE);

    vTaskStartScheduler();

    for (;;);

    return 0;
}
