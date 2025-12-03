#include "options.h"
#include <math.h>
#include "mmath.h"
#include "cal.h"
#include "ce.h"
#include "afe.h"
#include "amg_mtp_process.h"

// uint16_t ce_samples;
int32_t wrate_mpu;
int32_t wrate_mpu_app;
int32_t wrate_mpu_pulse;

/* How many phases have creep detection? */
#if EQUATION != EQUATION5
#define LAST_PHASE 2
#else
#define LAST_PHASE 3
#endif

/* used to determine if the analog front end has valid data. */
#define MAGIC_VALUE 0x5A

/* cached configuration; only written after start */
int32_t cached_ce_config;
Cal_t *cal_ptr;
int32_t vasum[4];

/* information about the AFE */
/* Number of accumulation intervals before data is valid */
uint8_t afe_up;
int8_t starting_cnt, sag_detect_mask_zon_cnt, no_load_chk_zon_cnt,
    low_volt_detect_mask_zon_cnt; /* bccho, 2024-06-17, 김종필대표 패치 적용 */

#if defined(FEATURE_STARTING_CURRENT_DATA_FLOAT)
float i_min_thrshld;
float i_min_thrshldx2;  // for relay decision
#else
int32_t i_min_thrshld;
int32_t i_min_thrshldx2;  // for relay decision
#endif

int32_t i_conden_thrshld;  // 10 % of MaxCur for backward decision in case of
                           // condensor_en
int32_t v_min_thrshld;
int32_t
    v_over_thrshld;  // Vover threshold (CE value) => wrong neutral connection
int32_t i_over_thrshld;
extern int8_t no_inst_curr_chk_zon_cnt;
;
bool b_first_afe_up = true;

#if 0 /* bccho, 2024-06-24, delete */
/* bccho, 2024-06-17, 김종필대표 패치 적용 */
extern float r_voltage_back[3];
extern float s_voltage_back[3];
extern float t_voltage_back[3];
#endif

/****************************************************************************
 * Description:
 *       Read one 32-bit word from the analog front end.
 *
 * Parameters:
 *   Input:
 *         The pointer to the source.
 *   Output:
 *         the value read from the source.
 ****************************************************************************/
int32_t afe_read(const int32_t *src_ptr) { return *src_ptr; }

extern bool xfer_done;

/****************************************************************************
 * Description:
 *       read and write afe operational state.
 *
 * Parameters:
 *   Input:
 *       1 = enable AFE, 0 read whether enabled, -1 disable AFE
 * Return Status:
 *       0 = disabled, 1 = enabled
 ****************************************************************************/
int afe_control(int wanted) { return 1; }

/****************************************************************************
 * Description:
 *       Starts the AFE, with assigned code, data and interpretation values
 *
 * Parameters:
 *   Input:
 *       A pointer to a calibration structure.
 * Return Status:
 *       Returns nonzero if it failed.
 ****************************************************************************/
uint8_t afe_init(void)
{
#if 0
#if METER_ID == 53
        i_min_thrshld = rtn_isqsum(0.015);						// 15 mA	(starting current = 5A * 0.004)
        i_min_thrshldx2 = 4 * i_min_thrshld;
#else
        i_min_thrshld = rtn_isqsum(0.030);						// 30 mA	(starting current = 10A * 0.004)
        i_min_thrshldx2 = 4 * i_min_thrshld;
#endif
#else
    // jp.kim 25.04.03
    i_min_thrshld =
        rtn_isqsum(StartCur);  // 15 mA , 30 mA	(starting current = 10A * 0.004)
    i_min_thrshldx2 = 4 * i_min_thrshld;
#endif

    // 6 A (10 % of MaxCur)
    i_conden_thrshld = rtn_isqsum((MaxCur * 0.1));
    i_over_thrshld = rtn_isqsum(IoverThrsld);

    // 80 V    //결상
    v_min_thrshld = rtn_vsqsum(80.0);

    // 300 V = 176 * sqrt(3) for wrong neutral connection
    v_over_thrshld = rtn_vsqsum(300.0);

    b_first_afe_up = true;

    return 0;
}

/****************************************************************************
 * Description:
 *       Gets data from the AFE, and massages it.
 *       Should be called as soon as possible when new data is ready.
 *       Calculates VA,
 *       Performs creep: The purpose of creep mode is so that the meter
 *       does not bill customers for noise, which is illegal.
 *       On multiphase the phase to read frequency is automatically
 *       switched to a phase with voltage. the switch only occurs if a phase
 *       measuring it does not have voltage.  Once switched, it stays.
 *       On single-phase meters, the neutral current is translated to have
 *       the same units as the main phase. (done before creep, of course.)
 *       It also adds up the watt-hours and sets the pulse outputs.
 *
 * Parameters:
 *   Input:
 *         None.
 *   Output:
 *         None.
 * Return Status:
 *         None
 ****************************************************************************/
void afe_ready_data(void)
{
    if (no_inst_curr_chk_zon_cnt)
    {
        --no_inst_curr_chk_zon_cnt;
    }

    if (no_load_chk_zon_cnt)
    {
        --no_load_chk_zon_cnt;
    }

    if (sag_detect_mask_zon_cnt)
    {
        --sag_detect_mask_zon_cnt;
    }

    /* bccho, 2024-06-17, 김종필대표 패치 적용 */
    if (low_volt_detect_mask_zon_cnt)
    {
        --low_volt_detect_mask_zon_cnt;
    }

    /* wait for PLL in the afe to settle */
    if (starting_cnt)
    {
        --starting_cnt;

#if 0 /* bccho, 2024-06-24, delete */
        /* bccho, 2024-06-17, 김종필대표 패치 적용 */
        // 전원투입 초기에 전압 = 0.0V
        r_voltage_back[0] = 0.0;
        r_voltage_back[1] = 0.0;
        r_voltage_back[2] = 0.0;
        s_voltage_back[0] = 0.0;
        s_voltage_back[1] = 0.0;
        s_voltage_back[2] = 0.0;
        t_voltage_back[0] = 0.0;
        t_voltage_back[1] = 0.0;
        t_voltage_back[2] = 0.0;
#endif

        afe_up = MAGIC_VALUE;
    }
}

/****************************************************************************
 * Description:
 *       Read the frequency of AC mains.
 *       The frequency is read at a particular phase.  On multiphase
 *       systems, the phase to read can be set.  It's also automatically
 *       switched to phase with voltage. the switch only occurs if a phase
 *       measuring it does not have voltage.  Once switched, it stays.
 *
 * Parameters:
 *   Input:
 *         None.
 *   Output:
 *         Frequency of ac mains at a particular phase.
 ****************************************************************************/
float afe_frequency(void) /* mains frequency, in Hz */
{
    ST_MTP_PUSH_DATA *pushd = dsm_mtp_get_push_data();

    return pushd->freq;
}

float rtn_inst_irms(float sq) { return sq; }

float rtn_inst_vrms(float sq) { return sq; }

float rtn_isqsum(float _irms) { return _irms; }

float rtn_vsqsum(float _vrms) { return _vrms; }

float rtn_inst_power(float _cesum) { return _cesum; }

float rtn_wsum(float _watt) { return _watt; }

#define MARGIN_FOR_APP 0

void wrate_mpu_set(void)
{
    float f;

    /* calculate the CE counts per pulse; It's a pretty big number,
     * and is a constant in a real meter. */
    f = (float)WHPPNUM;
    f /= (float)WHPPDEN;
#if CE_VER_TYPE == CE_VER_A03
    f /= WH_PER_CE;
#else
    f /= CE_WH_PER_CE;
#endif
    f /= ((float)Vmax) / 10.0;
    f /= ((float)Imax) / 10.0;

    wrate_mpu = lroundf(f);
    wrate_mpu_app = wrate_mpu - MARGIN_FOR_APP;
    wrate_mpu_pulse = wrate_mpu * PULSE_INC_UNIT;
}

bool afe_is_up(void) { return (MAGIC_VALUE == afe_up); }

#if (CE_VER_TYPE == CE_VER_A05_KEPCO) || (CE_VER_TYPE == CE_VER_A05B_KEPCO) || \
    (CE_VER_TYPE == CE_VER_A05E_KEPCO)

/****************************************************************************
 * Description:
 *       return i_max in amps.
 *
 * Parameters:
 *   Input:
 *         None.
 *   Output:
 *         floating point current in amps
 ****************************************************************************/
float get_imax(void) { return (((float)Imax) / 10.0); }

/****************************************************************************
 * Description:
 *       return v_max in volts.
 *
 * Parameters:
 *   Input:
 *         None.
 *   Output:
 *         floating point voltage in volts
 ****************************************************************************/
float get_vmax(void) { return (((float)Vmax) / 10.0); }

/****************************************************************************
 * Description:
 *       return p_max in W/VARs/VAs.
 *
 * Parameters:
 *   Input:
 *         None.
 *   Output:
 *         floating point voltage in W/VARs/VAs.
 ****************************************************************************/
float get_pmax(void) { return (float)((Vmax * Imax) / 100.0); }

/****************************************************************************
 * Description:
 *       return number of seconds per accum interval.
 *
 * Parameters:
 *   Input:
 *         None.
 *   Output:
 *         floating point.
 ****************************************************************************/
// returns in floating point real world units
float seconds_per_accum(void) { return ((float)afe_sumpre) / FS_FLOAT; }

#endif

void set_ppmc(void) {}
