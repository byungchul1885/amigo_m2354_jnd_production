#include "options.h"
#include "afe.h"
#include "rtc.h"
#include "tmp.h"
#include "batmode.h"
#include "port.h"
#include "main.h"
#include "amg_pwr.h"
#include "bat.h"
#include "amg_power_mnt.h"

/* bccho, 2023-08-04, 메인전원 OFF 상태이면서 배터리 레벨 정상 */
bool is_dc33_off(void)
{
    if (PA10)
    {
        return false;
    }
    else
    {
        return true;
    }
}
