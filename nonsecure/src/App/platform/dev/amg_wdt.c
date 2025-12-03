#include "main.h"
#include "amg_gpio.h"
#include "amg_power_mnt.h"
#include "options_sel.h"

#if 1 /* bccho, WDT, 2023-09-27 */
bool metertask_wdkick_done = false;
#endif

void dsm_wdt_ext_toggle_immd(void)
{
#if 1 /* bccho, WDT, 2023-07-15 */
    MSG01("dsm_wdt_ext_toggle_immd()");
    kick_watchdog_S();
#else
    if (dsm_pmnt_get_op_mode() == PMNT_ACTIVE_OP)
    {
        LL_GPIO_TogglePin(WD_DONE_GPIO_Port, WD_DONE_Pin);
    }
#endif /* bccho */
}