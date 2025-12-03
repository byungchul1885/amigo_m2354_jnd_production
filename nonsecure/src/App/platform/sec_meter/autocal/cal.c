#include <math.h>
#include "options.h"
#include "afe.h"
#include "nv.h"
#include "cal.h"
#include "amg_mtp_process.h"
#include "amg_meter_main.h"

#define _D "[CAL] "

uint16_t cal_flag = 0;
bool cal_data_get_success = 0;
extern ST_MIF_CAL_START g_mtp_cal_start;
extern bool cal_monitor_only;

#if EQUATION != EQUATION1
int16_t cnt_down;
static bool cal_error;

typedef struct Data_s
{
    int32_t whi;
    int32_t whi_frac;
    int32_t whe;
    int32_t whe_frac;
    int32_t varhi;
    int32_t varhi_frac;
    int32_t varhe;
    int32_t varhe_frac;
    int32_t v;
    int32_t v_frac;
} Data_t;

typedef struct Element_s
{
    int32_t *pcal_v;
    int32_t *pcal_i;
    int32_t *pphase_adj;
} Element_t;

#define ELEMENT_CNT 1

typedef struct
{
    int32_t T_vcal[ELEMENT_CNT];
    int32_t T_whcal[ELEMENT_CNT];
    int32_t T_vhcal[ELEMENT_CNT];
} CALIBRATION_MEASUREMENTS;

CALIBRATION_MEASUREMENTS CM;

#define vcal CM.T_vcal
#define whcal CM.T_whcal
#define vhcal CM.T_vhcal

#define v0cal CM.T_vcal[0]
#define wh0cal CM.T_whcal[0]
#define vh0cal CM.T_vhcal[0]

int32_t magcal;

float Vcal;
float Ical;
float theta_cal;
int Scal;

void cal_point_init(void)
{
    Vcal = 220.0;
    Ical = 5.0;
    theta_cal = 60.0;
    Scal = 6;
}

// cal 관련
extern bool dsp_cal_st_ing;    // cal 시작 표시, push 에서 설정
extern bool dsp_cal_mode_ing;  // cal 종료, 전류 표시, push 에서 설정
extern bool dsp_cal_mode_end;  // cal get 성공, mtp rx 에서 설정

void cal_begin(void)
{
    Scal = g_mtp_cal_start.process_time;
    cal_error = false;

    cnt_down = (int16_t)(2 + Scal);
    cnt_down += 2;

    cal_flag = YES;
    cal_data_get_success = 0;

    dsp_cal_st_ing = false;    // LCD cal 시작 표시
    dsp_cal_mode_ing = false;  // LCD 전류 표시
    dsp_cal_mode_end = false;  // LCD cal 종료 표시
}

void cal_start_by_key(void) { cal_begin(); }

void calibrate(U8 *tptr)
{
    int8_t i;

    if (cal_flag != YES)
        return;

    if (--cnt_down == 0)
    {
        if (!cal_data_get_success && dsp_cal_mode_ing)
        {
            MSG09("calibrate, get req");

            dsm_mtp_set_op_mode(MTP_OP_NORMAL);
            dsm_mtp_set_fsm(MTP_FSM_CAL_GET);
            dsm_mtp_fsm_send();

            cal_monitor_only = 0;
            cal_data_get_success = 0;
            cnt_down = 2;
        }
        else
        {
            cal_disable(); /* Disable the calibration. */
        }
    }
}

void cal_disable(void)
{
    cnt_down = -1;
    cal_flag = 0;
}

void cal_end(void) { cnt_down = 0; }

#endif

typedef struct
{
    int32_t _cal_i0;
    int32_t _cal_v0;
    int32_t _cal_ph0;
} cal_gain_type;
cal_gain_type cal_gain_mon;

bool cal_restore(uint8_t *tptr)
{
    cal_data_type *cal;
    bool ok;
    ST_MIF_CAL_DATA *p_mtp_caldata = dsm_mtp_get_cal_data();

    if (nv_read(I_CAL_DATA, tptr))
    {
        cal = (cal_data_type *)tptr;

        p_mtp_caldata->r_current_gain = cal->T_cal_i0;
        p_mtp_caldata->r_voltage_gain = cal->T_cal_v0;
        p_mtp_caldata->r_phase_gain = cal->T_cal_p0;
        p_mtp_caldata->s_current_gain = cal->T_cal_i1;
        p_mtp_caldata->s_voltage_gain = cal->T_cal_v1;
        p_mtp_caldata->s_phase_gain = cal->T_cal_p1;
        p_mtp_caldata->t_current_gain = cal->T_cal_i2;
        p_mtp_caldata->t_voltage_gain = cal->T_cal_v2;
        p_mtp_caldata->t_phase_gain = cal->T_cal_p2;
        p_mtp_caldata->cal_ok = 1;
        DPRINTF(DBG_TRACE, _D "%s: OK\r\n", __func__);
        ok = true;
    }
    else
    {
        cal_gain_mon._cal_i0 = -1;
        cal_gain_mon._cal_v0 = -1;
        cal_gain_mon._cal_ph0 = -1;

        DPRINTF(DBG_TRACE, _D "%s: FAIL\r\n", __func__);
        ok = false;
    }
#if 0  // defined(FEATURE_MTP_CALSET_OFF)
    /*MTP_CAL_MODE 는 cal key 를 통해서만 전입한다. */
    ok = true;
#endif
    return ok;
}
