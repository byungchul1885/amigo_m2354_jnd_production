#include "main.h"
#include "options.h"
#include "irq.h"
#include "meter.h"
#include "meter_app.h"
#include "cal.h"
#include "comm.h"
#include "eoi.h"
#include "eob.h"
#include "lp.h"
#include "program.h"
#include "nv.h"
#include "phy.h"
#include "key.h"
#include "amg_mtp_process.h"
#include "amg_power_mnt.h"
#include "amg_modemif_prtl.h"
#include "disp.h"
#include "amg_media_mnt.h"
#include "dl.h"
#include "amg_mif_prtl.h"
#include "amg_gpio.h"
#ifdef M2354_CAN /* bccho, 2023-11-28 */
#include "amg_isotp_user.h"
#endif

#define _D "[KEY] "

extern uint8_t mt_kind_key_set_buf;
extern uint8_t mr_date_key_set_buf;

extern bool meter_firmup_delay_ing;
int key_code;
bool key_pressed = false;
bool b_inst_key_cancel = false;
bool key_circ_step_mode;
bool is_comm_key;

void kact_dsp_inp(kaction_type kact, uint8_t *tptr);
void kact_inp_exit(void);
void init_mif_task_init(bool firm_up_sts);

void cal_key_proc(void);
static kaction_type get_key_code(void);
static void kact_circ_dsp(kaction_type kact, uint8_t *tptr);
static void kact_test_dsp(kaction_type kact, U8 *tptr);
static void kcirc_next_dsp_item(void);

static void kcirc_manual_dr(uint8_t *tptr);
static void kcirc_test_dsp_enter(void);
void ktest_next_dsp_item(U8 *tptr);
static void kcirc_load_init(void);

static void ktest_mode_toggle(void);
static void kact_calfail_dsp(kaction_type kact);
bool dsp_is_test_condensor(void);
bool dsp_is_test_auto_bidir(void);
bool dsp_is_test_err_pusle(void);
void dsp_r_sun_dsp_set(void);

void key_init(void)
{
    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    key_pressed = false;
    b_inst_key_cancel = false;
    key_circ_step_mode = false;

    calkey_skip_timeset(T10SEC);
}

bool kact_scurr_is_blocked(void)
{
    if ((get_disp_state() == DISP_INPUT_STATE) && scurr_block_dur_kinp)
        return true;

    return false;
}

void key_proc(void)
{
    kaction_type tkact;
    disp_state_type tds;
    uint8_t *tptr;
    tptr = adjust_tptr(&global_buff[0]);

    tds = get_disp_state();
    tkact = get_key_code();

    if (meter_firmup_delay_ing)
    {
        if (meter_firmup_delay_timeout())
        {
            meter_firmup_delay_ing = 0;

            init_mif_task_init(true);
        }
    }

    if (tkact & COMMK_BIT)
    {
        is_comm_key = true;
        tkact &= ~COMMK_BIT;
        comm_key_timeset(T2SEC);
    }
    else
    {
        is_comm_key = false;
    }

    if ((tkact == KACT_CONT_S_CAL) || (tkact == KACT_CONT_CAL))
    {
        cal_key_proc();
        return;
    }

    if (tkact == KACT_NONE)
    {
#if 0 /* bccho, 2024-01-10, 1227 포팅 */        
        if ((tds == DISP_INPUT_STATE) && comm_is_connected() &&
            comm_key_is_timeout() && !dsp_is_inp_end())
        {
            kact_inp_exit();
        }
#endif
        return;
    }

    /* Work BlackOut */
    else if (run_is_main_power())
    {
#if 1  // JP.KIM 24.10.08
        sagswell_start_timeset(T1SEC * working_fault_min * 60);
#else
        sagswell_start_timeset(T1SEC * 30 * 60);  // 1800 sec == 30 min
#endif
    }

    dsp_key_touched();  // to set state keep timer

    DPRINTF(DBG_TRACE, _D "%s: Disp_state[%d], Kact[0x%02X]\r\n", __func__, tds,
            tkact);

    switch (tds)
    {
    case DISP_CIRC_STATE:  // circulation state
        kact_circ_dsp(tkact, tptr);
        break;

        // input state
    case DISP_INPUT_STATE:
#if 0 /* bccho, 2024-01-10, 1227 포팅 */    
        kact_inp_dsp(tkact, tptr);
#endif
        break;

    case DISP_TEST_STATE:  // debug key state (all key pressed)
        kact_test_dsp(tkact, tptr);
        break;

    case DISP_CALFAIL_STATE:
        kact_calfail_dsp(tkact);
        break;

    default:
        break;
    }
}

void key_circ_step_mode_clear(void) { key_circ_step_mode = FALSE; }

void kact_test_dsp_exit(void) { dsp_circ_state_init(); }

void set_key_code(uint8_t kval)
{
    kaction_type kact;

    kact = 0;
    if ((!(kval & BIT5)) && (kval & BIT4))
    {
        // menu key
        kact |= MENUK_BIT;
    }
    else if (((kval & BIT5)) && (!(kval & BIT4)))
    {
        // menu key short
        kact |= (MENUK_BIT | CONTK_S_BIT);
    }
    else if (((kval & BIT5)) && ((kval & BIT4)))
    {
        // menu key long
        kact |= (MENUK_BIT | CONTK_L_BIT);
    }

    else if ((!(kval & BIT3)) && (kval & BIT2))
    {
        // move key
        kact |= MOVEK_BIT;
    }
    else if (((kval & BIT3)) && (!(kval & BIT2)))
    {
        // move key short
        kact |= (MOVEK_BIT | CONTK_S_BIT);
    }
    else if (((kval & BIT3)) && ((kval & BIT2)))
    {
        // move key long
        kact |= (MOVEK_BIT | CONTK_L_BIT);
    }

    if (kact)
    {
        key_code = kact | COMMK_BIT;
        key_pressed = true;
    }
}

static void kact_circ_dsp(kaction_type kact, uint8_t *tptr)
{
    if (run_is_bat_power())
    {
        if (kact == KACT_MENU)
        {
            kcirc_next_dsp_item();
        }
        else if (kact == KACT_CONT_S_MOVE)  // MOVE 1.5 sec
        {
            dsm_pmnt_move_key_callback_at_NO_VOLT_OP();
        }

        return;
    }

    switch (kact)
    {
    case KACT_MENU:
    {
        kcirc_next_dsp_item();
    }
    break;

        /* MOVE 1.5 sec */
    case KACT_CONT_S_MOVE:
    {
#if defined(FEATURE_MTP_FWVER_MANAGE)
        if (dsm_pmnt_get_op_mode() == PMNT_ACTIVE_OP)
        {
            dsm_mif_getreq_firmware_ver_data();
        }
#endif
        kcirc_test_dsp_enter();
    }
    break;

        /* MENU 5 sec */
    case KACT_CONT_MENU:
    {
        kcirc_manual_dr(tptr);
    }
    break;

        /* MOVE 5 sec */
    case KACT_CONT_MOVE:
    {
        kcirc_load_init();  // 통전
    }
    break;

    default:
        break;
    }
}

static void kcirc_next_dsp_item(void) { dsp_circ_mode_chg_set(); }

extern void dsm_update_forReport(void);

void R_CALL(void)
{
    /* r-cALL 동작 */

    // 0. LCD 표기
    dsp_r_sun_dsp_set();  // r-cALL 3 sec

#ifdef M2354_CAN /* bccho, 2023-11-28 */
    // 1. CAN Mode 0 (Node Advertisement)
    dsm_can_advertisement_power_on();
#endif
    // 2. Association 해제
    dl_stop_force();

    // 3. RS-485 통신 속도 9,600bps로 초기화
    mdm_baud = BAUD_9600;
    phy_init();

    dsm_imgtrfr_fwup_set_fsm(FWU_FSM_NONE);

    // 4. 계기 내장 SUN 팩토리 리셋 (AT+FUN:1 수행)
    dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);

    /* polling 모드 사용 안 해도 될 수도 있음. 사용하지 않을 경우
     * dsm_atcmd_rx_proc() 수정할 것. */
    dsm_atcmd_set_reset(FALSE, AT_RST_FACTORY);

    // 6. 착탈형 모뎀 하드웨어 핀 리셋 (리셋 핀 100msec 이상 Low 후 High로
    // 전환)
    dsm_gpio_e_modem_reset_low();
    vTaskDelay(100);  // 내장 모뎀 통신 시간 확보
    dsm_gpio_e_modem_reset_high();

    // 7. 착탈형 모뎀 팩토리 리셋 (AT+FUN:1)
    dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);

    /* polling 모드 사용 안 해도 될 수도 있음. 사용하지 않을 경우
     * dsm_atcmd_rx_proc() 수정할 것. */
    dsm_atcmd_set_reset(FALSE, AT_RST_FACTORY);
}

/*
    [이동] 버튼 5초 이상 누를 시, 전원 공급 정상 스크롤 상태 및 최초 통전 이전
   상태인 경우, 최초 통전만 수행 최초 통전 이후(재입력 시) * r-cALL 수행(2.2.5.2
   유·무선 통신 사양 표25 참조) 및 * 통전 (부하 제한 및 타임스위치 동작 중에는
   무시)
*/
static void kcirc_load_init(void)
{
    DPRINTF(DBG_TRACE, _D "%s: load initiated[%d]\r\n", __func__, load_inited);

    if (!load_inited)
    {
        /* 최초 통전 */
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
        dsm_atcmd_set_meterid(TRUE);
        load_initialize();
    }
    else
    {
        R_CALL();

        if (mt_is_no_sel() || mt_is_rload_ctrl())
        {
            load_initialize();
        }
    }

    dsm_update_forReport();

    if (dsm_pmnt_get_op_mode() == PMNT_ACTIVE_OP)
    {
        dsm_mif_getreq_firmware_ver_data();
    }
}

static void kcirc_manual_dr(uint8_t *tptr)
{
    if (run_is_main_power())
    {
        if (!DR_limit_timer_sec && load_inited)
        {
            man_sr_dr_proc(&cur_rtc, tptr);
            dsp_DR_set();

            DR_limit_timer_sec = mDR_limit_sec;
        }
    }
}

static void kcirc_test_dsp_enter(void) { dsp_test_state_init(); }

static void kact_test_dsp(kaction_type kact, U8 *tptr)
{
    switch (kact)
    {
    case KACT_CONT_S_MOVE:
        kact_test_dsp_exit();
        break;

    case KACT_MENU:
        ktest_mode_toggle();
        break;

    case KACT_MOVE:
        ktest_next_dsp_item(tptr);
        break;

    default:
        break;
    }
}

void dsp_test_reg_mr_date_change_check(U8 *tptr);
void dsp_test_tariff_rate_change_check(U8 *tptr);
void ktest_next_dsp_item(U8 *tptr)
{
    if (dsp_is_test_reg_mr_date())
    {
        dsp_test_reg_mr_date_change_check(tptr);
    }
    if (dsp_is_test_tariff_rate())
    {
        dsp_test_tariff_rate_change_check(tptr);
    }

    mr_date_key_set_buf = reg_mr_date;
    mt_kind_key_set_buf = mt_rtkind;
    if (dsp_is_test_mode_tS() && mt_is_time_sw() && !ts_is_zone_on())
    {
        if (relay_is_load_on())
        {
            relay_ctrl(0);
            ts_test_off();
        }
    }

    if (!dsp_test_mode_inc())
    {
        kact_test_dsp_exit();
    }
}

static void ktest_ts_toggle(void)
{
    if (dsp_is_test_mode_tS() && mt_is_time_sw() && !ts_is_zone_on())
    {
        if (ts_test_total_used() < 30 * 60 && relay_dly_is_timeout())
        {
            if (relay_is_load_on())
            {
                relay_ctrl(0);
                ts_test_off();
            }
            else
            {
                relay_ctrl(1);
                ts_test_on_start();
            }
        }
    }
    else
    {
        if (dsp_is_test_mode_tS() && mt_is_rload_ctrl())
        {
            if (relay_dly_is_timeout())
            {
                if (relay_is_load_on())
                {
                    relay_ctrl(0);
                }
                else
                {
                    relay_ctrl(1);
                }
            }
        }
    }
}

static void ktest_mode_toggle(void)
{
    if (dsp_is_test_smode())
    {
        dsp_test_smode_toggle();
    }
    else if (dsp_is_test_pvt())
    {
        dsp_test_pvt_toggle();
    }
    else if (dsp_is_test_overcurr())
    {
        dsp_test_overcurr_toggle();
    }
    else if (dsp_is_test_condensor())
    {
        dsp_test_condensor_toggle();
    }
    else if (dsp_is_test_err_pusle())
    {
        dsp_test_err_pusle_toggle();
    }

    else if (dsp_is_test_auto_bidir())
    {
        dsp_test_auto_bidir_toggle();
    }
    else if (dsp_is_test_mode_tS())
    {
        ktest_ts_toggle();
    }
    else if (dsp_is_test_reg_mr_date())
    {
        dsp_test_reg_mr_date_change();
    }
    else if (dsp_is_test_tariff_rate())
    {
        dsp_test_tariff_rate_change();
    }
}

static void kact_calfail_dsp(kaction_type kact)
{
    if (kact == KACT_MENU)
    {
        dsp_calfail_state_exit();
    }
}

extern void dsm_mtp_fsm_send(void);
void cal_key_proc(void)
{
    cal_start_by_key();
    DPRINTF(DBG_TRACE, _D "%s: cal key pressed!!!\r\n");
    dsm_mtp_set_op_mode(MTP_OP_NORMAL);
    dsm_mtp_set_fsm(MTP_FSM_CAL_ST);
    dsm_mtp_fsm_send();
}

static kaction_type get_key_code(void)
{
    int tkey;

    if (key_pressed)
    {
        key_pressed = false;
        tkey = key_code;
        return (kaction_type)tkey;
    }
    return KACT_NONE;
}
