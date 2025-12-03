#include <math.h>
#include "options.h"
#include "port.h"
#include "meter.h"
#include "mmath.h"
#include "pulse_src.h"
#include "meter_app.h"
#include "whm.h"
#include "irq.h"
#include "eoi.h"
#include "nv.h"
#include "whm_1.h"
#include "amg_mtp_process.h"
#include "amg_debug.h"
#if 1 /* bccho, 2023-07-20 */
#include "platform.h"
#endif

#define _D "[AREQ] "

extern float PULSE_ADD_MODIFY_DATA, PULSE_ADD_MODIFY_DATA_VA,
    PULSE_ADD_MODIFY_DATA_VAR;
extern bool METER_FW_UP_ING_STS;
extern bool METER_FW_UP_END_PULSE_MODIFY;
extern U16 METER_FW_UP_ING_CNT;
extern U8 PULSE_DIR_MODIFY_BACK;

extern U8 first_pulse_delete_flag;
extern U8 first_pulse_delete_cnt;
extern int8_t no_inst_curr_chk_zon_cnt;

bool pcnt_updated;

int32_t pcnt_ch_1sec[numCHs];
int32_t pcnt_w_cache;  // Real-time count of pulses on WHs led for a second.
int32_t pcnt_x_cache;
int32_t pcnt_y_cache;

uint32_t w_d_fraction;
uint32_t w_r_fraction;
uint32_t r_dlag_fraction;
uint32_t r_dlead_fraction;
uint32_t r_rlag_fraction;
uint32_t r_rlead_fraction;
uint32_t va_d_fraction;
uint32_t va_r_fraction;
uint32_t pls_inc_fraction;

int32_t pcnt_dmover_1sec;
uint32_t dmover_fraction;

bool pulse_dir;
vi_quarter_type pls_vi_quarter_wr;

int32_t wi_sum;      /* 유효 */
int32_t ri_sum;      /* 무효 */
int32_t ri_sum_lag;  /* 지상무효 */
int32_t ri_sum_lead; /* 진상무효 */
int32_t vai_sum;     /* 피상 */
int32_t we_sum;
int32_t re_sum_lag;
int32_t re_sum_lead;
int32_t vae_sum;

bool meter_fwup_pulse_protect(void);
float VAh_WVAR_f(int32_t w0, int32_t v0);

void pcnt_variable_init(void)
{
    pcnt_updated = false;

    pcnt_w_cache = 0;
    pcnt_x_cache = 0;
    pcnt_y_cache = 0;

    w_d_fraction = 0L;
    w_r_fraction = 0L;

    r_rlag_fraction = 0L;
    r_rlead_fraction = 0L;
    r_dlag_fraction = 0L;
    r_dlead_fraction = 0L;

    va_r_fraction = 0L;
    va_d_fraction = 0L;

    pls_inc_fraction = 0L;

    dmover_fraction = 0L;
}

vi_quarter_type decision_quarter(int32_t ws, int32_t rs)
{
    /* ws (w_sum) : 유효, rs (var_sum) : 무효 */
    if (ws >= 0)
    {
        if (rs >= 0)
            return eVIq1;  // 1상한
        else
            return eVIq4;  // 4상한
    }
    else
    {
        if (rs >= 0)
            return eVIq2;  // 2상한
        else
            return eVIq3;  // 3상한
    }
}

void SelectPulses_init(void)
{
    ri_sum = 0L;  // for 1 phase
    wi_sum = ri_sum_lag = ri_sum_lead = vai_sum = 0L;
    we_sum = re_sum_lag = re_sum_lead = vae_sum = 0L;

    for (uint8_t i = 0; i < numCHs; i++)
    {
        pcnt_ch_1sec[i] = 0;
    }
}

float react_err_adj = 0.94;

void pulse_out_quart_dir(void)
{
    int32_t t32_1, t32_2;

    t32_1 = (int32_t)wi_sum;  // 유효 (w)
    t32_2 = (int32_t)ri_sum;  // 무효 (var)

    // 펄스 회전 방향
    if (t32_1 >= 0)
    {
        /* 정방향 : 시계방향 회전 수전 */
        pulse_dir = true;  // forward direction
    }
    else
    {
        /* 역방향 : 반시계방향 회전 송전 */
        pulse_dir = false;
    }

    /* Ref. get_vi_quarter() */
    pls_vi_quarter_wr = decision_quarter(t32_1, t32_2);
#if 1 /* bccho, 2023-07-20 */
    dsp_pulse_inc_set(f2pls_pulse(&pls_inc_fraction, labs2((int32_t)wi_sum)));
#else
    dsp_pulse_inc_set(f2pls_pulse(&pls_inc_fraction, labs((int32_t)wi_sum)));
#endif
}

void SelectPulses(void)
{
    bool meter_fwup_pulse_protect(void);
    ST_MTP_PUSH_DATA* p_pushdata = dsm_mtp_get_push_data();
    SelectPulses_init();

#if PHASE_NUM == SINGLE_PHASE
    wi_sum = (int32_t)w0sum_mon;    // 유효
    ri_sum = (int32_t)var0sum_mon;  // 무효
    vai_sum = (int32_t)va0sum_mon;  // 피상
#else
    wi_sum = (int32_t)(w0sum_mon + w1sum_mon + w2sum_mon);
    ri_sum = (int32_t)(var0sum_mon + var1sum_mon + var2sum_mon);
    vai_sum = (int32_t)(va0sum_mon + va1sum_mon + va2sum_mon);
#endif

    if (!meter_fwup_pulse_protect())
    {
        pcnt_ch_1sec[eChDeliAct] = p_pushdata->rst_accum.DeliAct;
        pcnt_ch_1sec[eChDLagReact] = p_pushdata->rst_accum.DLagReact;
        pcnt_ch_1sec[eChDLeadReact] = p_pushdata->rst_accum.DLeadReact;
        pcnt_ch_1sec[eChDeliApp] = p_pushdata->rst_accum.DeliApp;

        pcnt_ch_1sec[eChReceiAct] = p_pushdata->rst_accum.ReceiAct;
        pcnt_ch_1sec[eChRLagReact] = p_pushdata->rst_accum.RLagReact;
        pcnt_ch_1sec[eChRLeadReact] = p_pushdata->rst_accum.RLeadReact;
        pcnt_ch_1sec[eChReceiApp] = p_pushdata->rst_accum.ReceiApp;

        // 운영부 펌업후 2번째 펄스는 버린다. 대량 펄스 2회중복 사용을 막기
        // 위함.
        if ((first_pulse_delete_flag == 'T') &&
            (first_pulse_delete_cnt))  // JP.KIM 25.03.02
        {
            first_pulse_delete_cnt--;
            if (!first_pulse_delete_cnt)
            {
                first_pulse_delete_flag = 0;
                first_pulse_delete_cnt = 0;

                pcnt_ch_1sec[eChDeliAct] = 0;
                pcnt_ch_1sec[eChDLagReact] = 0;
                pcnt_ch_1sec[eChDLeadReact] = 0;
                pcnt_ch_1sec[eChDeliApp] = 0;

                pcnt_ch_1sec[eChReceiAct] = 0;
                pcnt_ch_1sec[eChRLagReact] = 0;
                pcnt_ch_1sec[eChRLeadReact] = 0;
                pcnt_ch_1sec[eChReceiApp] = 0;
            }
        }

        // 전류값을 0.0으로 막는 구간에는 피상 펄스도 막아야한다.
        if (no_inst_curr_chk_zon_cnt)
        {
#if 0  // jp.kim 25.03.05
			pcnt_ch_1sec[eChDeliApp] = 0;		
			pcnt_ch_1sec[eChReceiApp] =  0;
#else  // 피상 펄스 계산
            pcnt_ch_1sec[eChDeliApp] = (int32_t)VAh_WVAR_f(
                pcnt_ch_1sec[eChDeliAct],
                (pcnt_ch_1sec[eChDLagReact] + pcnt_ch_1sec[eChDLeadReact]));
            pcnt_ch_1sec[eChReceiApp] = (int32_t)VAh_WVAR_f(
                pcnt_ch_1sec[eChReceiAct],
                (pcnt_ch_1sec[eChRLagReact] + pcnt_ch_1sec[eChRLeadReact]));
#endif
        }
    }

    if (METER_FW_UP_END_PULSE_MODIFY)
    {
        METER_FW_UP_END_PULSE_MODIFY = 0;
        METER_FW_UP_ING_STS = 0;
        METER_FW_UP_ING_CNT = 0;

        if (mt_is_uni_dir())
        {
            if (PULSE_DIR_MODIFY_BACK)  // 수전
            {
                pcnt_ch_1sec[eChDeliAct] = (int32_t)fabs(PULSE_ADD_MODIFY_DATA);
                pcnt_ch_1sec[eChDeliApp] =
                    (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VA);
                if (PULSE_ADD_MODIFY_DATA_VAR < 0.0)
                {
                    pcnt_ch_1sec[eChDLeadReact] =
                        (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VAR);
                }
                else
                {
                    pcnt_ch_1sec[eChDLagReact] =
                        (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VAR);
                }
            }
            else  // 역방향 //송전
            {
                if (meas_method == E_SINGLE_DIR)
                {
                    // 수전 단방향(D)
                    pcnt_ch_1sec[eChDeliAct] =
                        (int32_t)fabs(PULSE_ADD_MODIFY_DATA);
                    pcnt_ch_1sec[eChDeliApp] =
                        (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VA);

                    if (PULSE_ADD_MODIFY_DATA_VAR < 0.0)
                    {
                        pcnt_ch_1sec[eChDLagReact] =
                            (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VAR);
                    }
                    else
                    {
                        pcnt_ch_1sec[eChDLeadReact] =
                            (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VAR);
                    }
                }
            }
        }
        else  // 양방향  //송수전
        {
            if (PULSE_DIR_MODIFY_BACK)
            {
                pcnt_ch_1sec[eChDeliAct] = (int32_t)fabs(PULSE_ADD_MODIFY_DATA);
                pcnt_ch_1sec[eChDeliApp] =
                    (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VA);
                if (PULSE_ADD_MODIFY_DATA_VAR < 0.0)
                {
                    pcnt_ch_1sec[eChDLeadReact] =
                        (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VAR);
                }
                else
                {
                    pcnt_ch_1sec[eChDLagReact] =
                        (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VAR);
                }
            }
            else
            {
                pcnt_ch_1sec[eChReceiAct] =
                    (int32_t)fabs(PULSE_ADD_MODIFY_DATA);
                pcnt_ch_1sec[eChReceiApp] =
                    (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VA);
                if (PULSE_ADD_MODIFY_DATA_VAR < 0.0)
                {
                    pcnt_ch_1sec[eChRLagReact] =
                        (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VAR);
                }
                else
                {
                    pcnt_ch_1sec[eChRLeadReact] =
                        (int32_t)fabs(PULSE_ADD_MODIFY_DATA_VAR);
                }
            }
        }
    }

    pulse_out_quart_dir();

    pcnt_updated = true;
}

vi_quarter_type get_vi_quarter(void) { return pls_vi_quarter_wr; }

void pcnt_accumulate(void)
{
    int i, rate, ch;  //,cnt;

    if (pcnt_updated)
    {
        pcnt_updated = false;

        accm_rate = cur_rate;

        // all channels is accumulated
        for (i = 0; i < numCHs; i++)
        {
            ACCM_ENERGY(
                ACC_CH(eTrate, i),
                (uint32_t)pcnt_ch_1sec[i]);  // mxaccm_dgt_cnt is processed
            ACCM_ENERGY(
                ACC_CH(accm_rate, i),
                (uint32_t)pcnt_ch_1sec[i]);  // mxaccm_dgt_cnt is processed
            lp_intv_dm[i] += (uint32_t)pcnt_ch_1sec[i];
        }

        dm_subintv_act_fwd += (uint16_t)pcnt_ch_1sec[eChDeliAct];
        dm_subintv_app_fwd += (uint16_t)pcnt_ch_1sec[eChDeliApp];
        dm_subintv_act_bwd += (uint16_t)pcnt_ch_1sec[eChReceiAct];
        dm_subintv_app_bwd += (uint16_t)pcnt_ch_1sec[eChReceiApp];

        prepay_proc(pcnt_ch_1sec[eChDeliAct]);

        for (i = 0; i < numCHs; i++)
        {
            pcnt_ch_1sec[i] = 0;
        }
    }
}