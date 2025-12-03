#include "options.h"
#include "rtc.h"
#include "ce.h"
#include "tmp.h"
#include "amg_mtp_process.h"

#define BUSY_WAIT_TIME 98300  // About 1/10s in cycles at 9.8MHz
static int16_t last_stemp;
static float tmprd_val;

bool tmp_busy_wait(void) { return true; }

void tmp_init(void) {}

int16_t tmp_stemp(void)
{
    if (0 == last_stemp)
    {
        tmp_init();
    }
    return last_stemp;
}

void tmp_deinit(void) {}

float tmp(void) { return 0; }

void tmp_meas(void) { tmprd_val = tmp(); }

float get_inst_tmpf(void) { return tmprd_val; }

int16_t get_inst_stemp(void) { return last_stemp; }

int8_t get_inst_temp(void)
{
#if 1 /* bccho, 2024-06-07 패치 포팅 */
    float temperature;
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();

    extern float adj_currtemp;

    if (adj_currtemp != 0)
    {
        temperature = pushd->temp + adj_currtemp;
    }
    else
    {
        temperature = pushd->temp;
    }

#if 0
    double GetTemperature(void);
    double t = GetTemperature();
#else
    double t = 0;
#endif
    MSG00("Temp Sylergy/Nuvoton: %d.%03d / %d.%03d, %d MHz",
          (uint32_t)(temperature),
          (uint32_t)((temperature - (uint32_t)(temperature)) * 1000),
          (uint32_t)(t), (uint32_t)((t - (uint32_t)(t)) * 1000),
          SystemCoreClock / 1000000);

#if 0 /* bccho, 2024-06-25, 삭제 */
    if (temperature > 80 && SystemCoreClock == 96000000)
    {
        Set_CLOCK_24M_S();
        SystemCoreClockUpdate();
    }
    if (temperature < 75 && SystemCoreClock == 24000000)
    {
        Set_CLOCK_96M_S();
        SystemCoreClockUpdate();
    }
#endif
    return temperature;
#else
    ST_MTP_PUSH_DATA* pushd = dsm_mtp_get_push_data();

    return (int8_t)pushd->temp;
#endif
}
