#ifndef MMATH_H
#define MMATH_H 1

// Add watts to pulse counts.  Nearly identical logic
// will add 1 to the meter display, by setting cnt per pulse
// to the unit that is displayed.
void add(int32_t *cnt_ptr, int32_t *frac_ptr, int32_t frac);

// Convert pulse counts and fraction to a float.
// This is only accurate for ratios.  It drops 7 bits of
// precision and can't be used for billing registers.
// However, it is very convenient in the autocalibration
// logic, which needs to take ratios of energy measurements.
float p2f(int32_t const *cnt_ptr, int32_t const *fract_ptr);

int f2pls(uint32_t *fract, int32_t w);
int f2pls_app(uint32_t *fract, int32_t w);
int f2pls_pulse(uint32_t *fract, int32_t w);

/* milliwatt-hours (good for a demo, bad for a meter) */
/* takes a pointer to a count of pulses */
int32_t mwh (const int32_t* wh_ptr);

/* centiwatt-hours (good for a demo, bad for a meter) */
/* takes a pointer to a count of pulses */
int32_t cwh (const int32_t* wh_ptr);

/* kilowatt-hours (good for a meter, bad for a demo) */
/* takes a pointer to a count of pulses */
int32_t kwh (const int32_t* wh_ptr);

/* utility functions, often unimplemented. */
#if 1 /* bccho, 2023-07-20 */
int32_t labs2(int32_t lval);
#else
int32_t labs(int32_t lval);
#endif
int32_t lmax(int32_t a, int32_t b);
int32_t lmin(int32_t a, int32_t b);

long int lroundf (float f0);

// standard C99 library function not provided by most compilers.
// converts a float to the nearest long integer.
extern long int lroundf (float f0);

// Standard square root.
float sqrtf(float);


#endif
