#include "options.h"
#include "rtc.h"
#include "meter.h"
#include "mmath.h"
#include <math.h>
#include "afe.h"


// Add up meter data. This is very important code in a meter.
// Adds watt-hours in DSP counts to pulse counts.  This same logic
// can add 1 to the meter display, by setting mpu_wrate (in Cal_t in main.h)
// to be the DSP counts of the unit that is displayed.
// That is, this can directly produce the meter display if the system
// is arranged to do so.
// This code has a cumulative round-off error of half a DSP LSB per reset.
// This code is easy to understand and modify, fast and compact compared
// to multiprecision arithmetic, BCD counters and other schemes.
// Notice that the code copes with impossible conditions like values
// out of range, etc.
// These sometimes actually occur during ESD, EMC, replacement of
// the EEPROM containing the billing register, or other weird cases.
// (The meters literally have to cope with lightning strikes!)
// Also, the code is loopless because in such weird cases, this code
// is commonly executed in the main loop, and it must finish,
// and have sane register values afterward.
// An early version of this code did the division by a subtraction loop
// and caused the meter to hang when an all-1s "ff" value got into a
// register.
// During ESD, register values may be -corrupted-, but it complains about that.
// The ecord system has to report to the billing system, ultimately.
// Restarting the billing registers should also clear the error system

int f2pls(uint32_t *fract, int32_t w)
{
	int units;				// signed
	uint32_t fraction;

	fraction = *fract;

	fraction += w;

    units = fraction / wrate_mpu;
    fraction -= wrate_mpu * units; 		// fraction %= wh_cnt; // also works

    // When the units and fraction are different signs,
    // the units lies, because it counts a partial unit
    // (the sum of the last unit and an opposite-sign
    // fraction) as a whole unit.
    // Make the fraction positive, so it always adds
    // to a positive units count.  Units will always become positive.
    if ((int32_t)fraction < 0)
    {
        --units; // can make units underflow. See next logic.
        fraction += wrate_mpu;
    }

    // if underflow, remain at zero.
    // E.g. 000000 - 1 = 000000.  So units are always positive.
    if (units < 0)            // range: -big .. -1
    {
        units = 0;
    }

    *fract = fraction;
	return units;
}

int f2pls_pulse(uint32_t *fract, int32_t w)
{
	int units;				// signed
	uint32_t fraction;

	fraction = *fract;
	fraction += w;

	wrate_mpu_pulse = ((uint32_t)(3600.0*1000.0/(float)PulseKwh)) * PULSE_INC_UNIT;
    units = fraction / wrate_mpu_pulse;
    fraction -= wrate_mpu_pulse * units;

    // When the units and fraction are different signs,
    // the units lies, because it counts a partial unit
    // (the sum of the last unit and an opposite-sign
    // fraction) as a whole unit.
    // Make the fraction positive, so it always adds
    // to a positive units count.  Units will always become positive.
    if ((int32_t)fraction < 0)
    {
        --units; // can make units underflow. See next logic.
        fraction += wrate_mpu_pulse;
    }

    // if underflow, remain at zero.
    // E.g. 000000 - 1 = 000000.  So units are always positive.
    if (units < 0)            // range: -big .. -1
    {
        units = 0;
    }

    *fract = fraction;
	return units;
}

// Take a long absolute value.
#if 1 /* bccho, 2023-07-20 */
int32_t labs2(int32_t lval)
#else
int32_t labs(int32_t lval)
#endif
{
    if (0 < lval)
        return lval;
    else
        return -lval;
}

/* GNU Library's sqrtf() fails in some cases
 * and there's no time to isolate them.
 * floating point square root:
 * A fast approximation,
 * followed by four iterations of newton-raphson */
float sqrtf(float f)
{
    int cnt;
    union
    {
        int tmp;
        float f;
    } u;

    if (f < 0)  /* I know.  But... some DSP codes do this. */
        f = -f;

    u.f = f;

    /* To justify the following code, prove that
     * ((((val_int / 2^m) - b) / 2) + b) * 2^m
     *    = ((val_int - 2^m) / 2) + ((b + 1) / 2) * 2^m)
     * where: val_int = u.tmp
     * b = exponent bias
     * m = number of mantissa bits */

    u.tmp -= 1 << 23; /* Subtract 2^m. */
    u.tmp >>= 1; /* Divide by 2. */
    u.tmp += 1 << 29; /* Add ((b + 1) / 2) * 2^m. */

    /* u.f is the starting approximation, usually within 3.5%
     * Perform newton-raphson to refine the value. */
    for(cnt = 4; cnt > 0; --cnt)
    {
        u.f = u.f - ((u.f * u.f) - f)/(2 * u.f);
    }

    return u.f;
}



