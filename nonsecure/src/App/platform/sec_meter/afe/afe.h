#ifndef AFE_H
#define AFE_H 1

#include <stdio.h>

/* AFE data; literally access to the CE output data */
#define afe_cestatus (CESTATUS_X)                           /* CE status */
#define afe_wsum(_idx_) (*((_idx_) + (&WSUM_X)))            /* Wh */
#define afe_varsum(_idx_) (*((_idx_) + (&VARSUM_X)))        /* VARh */
#define afe_vasum(_idx_) (vasum[(_idx_)])                   /* VAh */
#define afe_vsqsum(_idx_) (*(((_idx_) - 1) + (&V0SQSUM_X))) /* V */
#define afe_isqsum(_idx_) (*(((_idx_) - 1) + (&I0SQSUM_X))) /* I */
#define afe_zc (MAINEDGE_X)    /* edges of zero-crossing */
#define afe_freq (FREQ_X)      /* frequency from AFE */
#define afe_temp (TEMP_X)      /* temperature from AFE */
#define afe_AtoB (PH_AtoB_X)   /* phase A to B angle */
#define afe_AtoC (PH_AtoC_X)   /* phase A to C angle */
#define afe_AtoB2 (PH_AtoB2_X) /* phase A to B angle */
#define afe_AtoC2 (PH_AtoC2_X) /* phase A to C angle */
#define afe_sumpre (CE_SUMPRE) /* number of samples in this accum interval */

#define afe_mag_slot(_idx_) (*((_idx_) + (&MAG_SLOT0))) /* MAG sensor Slots */

//======================================================================================================
//= Harmonics Related Functions
//========================================================================
//======================================================================================================
#define afe_vsqsum_h(_idx_) (*(((_idx_) - 1) + (&V0SQSUM_H_X))) /* V */
#define afe_isqsum_h(_idx_) (*(((_idx_) - 1) + (&I0SQSUM_H_X))) /* I */
#define afe_wsum_h(_idx_) (*(((_idx_) - 1) + (&W0SUM_H_X))) /* Harmonic Wh */

#define AFE_WORD_SIZE 4
#define CE_PLL_OK \
    2  // Accumulation intervals before CE's PLL is stable.
       // CE's PLL is stable after 1 second, so first valid data is after that.
extern int8_t starting_cnt, sag_detect_mask_zon_cnt, no_load_chk_zon_cnt,
    low_volt_detect_mask_zon_cnt;  // Count of accumulation intervals until data
                                   // becomes valid

#if PHASE_NUM == SINGLE_PHASE
#define PULSE_INC_UNIT 4
#else
#define PULSE_INC_UNIT 2
#endif

#define Vmax 10842L
#define Imax 737
#define Wrate 18949
#define CECONFIG_SPEED_SEL 0x01L  // fast(bit1) slow(bit0)

extern int32_t vasum[4];
extern int32_t wrate_mpu;
extern int32_t wrate_mpu_app;
extern int32_t wrate_mpu_pulse;
// extern uint16_t ce_samples;
extern int32_t i_conden_thrshld;  // 10 % of MaxCur for backward decision in
                                  // case of condensor_en
extern int32_t v_min_thrshld;
extern int32_t
    v_over_thrshld;  // Vover threshold (CE value) => wrong neutral connection
extern int32_t i_over_thrshld;  // Iover threshold (CE value)

extern float i_min_thrshld;
extern float i_min_thrshldx2;  // for relay decision

float rtn_inst_irms(float sq);
float rtn_inst_vrms(float sq);
float rtn_inst_power(float _cesum);
float rtn_isqsum(float _irms);
float rtn_vsqsum(float _vrms);
float rtn_wsum(float _watt);
void wrate_mpu_set(void);
void wrate_ce_set(void);
void ceconfig_speed_set(void);
void ceconfig_extpulse_set(void);
// void ce_samples_set(uint16_t _samples);

int32_t afe_read(const int32_t *pSrc);
uint8_t afe_init(void);
int afe_control(int);
void afe_ready_data(void);

float afe_frequency(void);
bool afe_is_up(void);

#if (CE_VER_TYPE == CE_VER_A05_KEPCO) || (CE_VER_TYPE == CE_VER_A05B_KEPCO) || \
    (CE_VER_TYPE == CE_VER_A05E_KEPCO)
//======================================================================================================
//= Utility Functions
//========================================================================
//======================================================================================================
float get_imax(void);           // returns in floating point real world units
float get_vmax(void);           // returns in floating point real world units
float get_pmax(void);           // returns in floating point real world units
float seconds_per_accum(void);  // returns in floating point real world units
#endif

#endif
