
#ifndef DISP_H
#define DISP_H 1

#if 0 /* bccho, 2023-07-20 */
#include "stm32l4xx_hal_lcd.h"
#endif /* bccho */
#include "amg_lcd.h"
#include "whm.h"
#include "options_sel.h"

#define DISP_CIRC_PERIOD 5

#define NUM_MODE_DIGIT 8
#define LAST_DIGIT_INDEX (NUM_MODE_DIGIT - 1)
#define NUM_MODE 2
#define NUM_DIGIT (NUM_MODE_DIGIT - NUM_MODE)
#define MAX_LCD_DIGIT_KWH 1000000L
#define LCD_MAP_SIZE 13

#define lcd_w_point 2
#define lcd_w_point 2
#if PHASE_NUM == SINGLE_PHASE
#define lcd_wh_point 1
#else
#define lcd_wh_point 0
#endif
#define lcd_pf_point 2

typedef enum
{
    DISP_CIRC_STATE,
    DISP_INPUT_STATE,
    DISP_TEST_STATE,
    DISP_CAL_STATE,
    DISP_DR_STATE,
    DISP_CALFAIL_STATE,
    DISP_R_SUN_STATE,
    DISP_ON_SUN_STATE,
    DISP_CAL_END_STATE,    // 전류 표시
    DISP_CAL_MEM_BK_STATE  // cal end 표시
} disp_state_type;

typedef enum
{
    DISPINP_DATE,
    DISPINP_TIME,
    DISPINP_REGREAD_DATE,
    DISPINP_SIGSEL,
    DISPINP_sCURR,
    DISPINP_sCURR_aRTN,
    DISPINP_TS,
    DISPINP_RATEKIND,
    DISPINP_TOU,
    DISPINP_DMINTV,
    DISPINP_SN_1,
    DISPINP_SN_2,
    DISPINP_MT_DIR,
    DISPINP_MEAS,
    DISPINP_BAUD,
    DISPINP_TEMP,
    DISPINP_ERR_PLS,
    DISPINP_CONDENSOR,
    DISPINP_COMMEN_COVEROPEN,
    DISPINP_sCURR_HOLD,
    DISPINP_sCURR_RTN1,
    DISPINP_sCURR_RTN_DUR1,
    DISPINP_sCURR_RTN_DUR2,
    DISPINP_sCURR_2,
    DSPINP_CONTRACT_MONTH,
    DISPINP_END
} disp_input_type;

typedef enum
{
    DSPMODE_INIT,
    // 날자/시간
    DSPMODE_DATE,
    DSPMODE_TIME,
    // 현월 (DSPMODE_CU_IMP_ACT_A 은 첫번재,  DSPMODE_CU_EXP_APP_T 는 마지막
    // enum 값) 현월 (DSPMODE_CU_IMP_ACT_MX_A 은 첫번재,
    // DSPMODE_CU_EXP_APP_CUMMX_T 는 마지막 enum 값)
    DSPMODE_CU_IMP_ACT_A,
    DSPMODE_CU_IMP_ACT_B,
    DSPMODE_CU_IMP_ACT_C,
    DSPMODE_CU_IMP_ACT_D,
    DSPMODE_CU_IMP_ACT_T,
    DSPMODE_CU_EXP_ACT_A,
    DSPMODE_CU_EXP_ACT_B,
    DSPMODE_CU_EXP_ACT_C,
    DSPMODE_CU_EXP_ACT_D,
    DSPMODE_CU_EXP_ACT_T,
    DSPMODE_CU_IMP_REACT_A,
    DSPMODE_CU_IMP_REACT_B,
    DSPMODE_CU_IMP_REACT_C,
    DSPMODE_CU_IMP_REACT_D,
    DSPMODE_CU_IMP_REACT_T,
    DSPMODE_CU_IMP_lagREACT_A,
    DSPMODE_CU_IMP_lagREACT_B,
    DSPMODE_CU_IMP_lagREACT_C,
    DSPMODE_CU_IMP_lagREACT_D,
    DSPMODE_CU_IMP_lagREACT_T,
    DSPMODE_CU_IMP_leadREACT_A,
    DSPMODE_CU_IMP_leadREACT_B,
    DSPMODE_CU_IMP_leadREACT_C,
    DSPMODE_CU_IMP_leadREACT_D,
    DSPMODE_CU_IMP_leadREACT_T,
    DSPMODE_CU_EXP_REACT_A,
    DSPMODE_CU_EXP_REACT_B,
    DSPMODE_CU_EXP_REACT_C,
    DSPMODE_CU_EXP_REACT_D,
    DSPMODE_CU_EXP_REACT_T,
    DSPMODE_CU_EXP_lagREACT_A,
    DSPMODE_CU_EXP_lagREACT_B,
    DSPMODE_CU_EXP_lagREACT_C,
    DSPMODE_CU_EXP_lagREACT_D,
    DSPMODE_CU_EXP_lagREACT_T,
    DSPMODE_CU_EXP_leadREACT_A,
    DSPMODE_CU_EXP_leadREACT_B,
    DSPMODE_CU_EXP_leadREACT_C,
    DSPMODE_CU_EXP_leadREACT_D,
    DSPMODE_CU_EXP_leadREACT_T,
    DSPMODE_CU_IMP_APP_A,
    DSPMODE_CU_IMP_APP_B,
    DSPMODE_CU_IMP_APP_C,
    DSPMODE_CU_IMP_APP_D,
    DSPMODE_CU_IMP_APP_T,
    DSPMODE_CU_EXP_APP_A,
    DSPMODE_CU_EXP_APP_B,
    DSPMODE_CU_EXP_APP_C,
    DSPMODE_CU_EXP_APP_D,
    DSPMODE_CU_EXP_APP_T,
    DSPMODE_CU_IMP_ACT_MX_A,
    DSPMODE_CU_IMP_ACT_MX_B,
    DSPMODE_CU_IMP_ACT_MX_C,
    DSPMODE_CU_IMP_ACT_MX_D,
    DSPMODE_CU_IMP_ACT_MX_T,
    DSPMODE_CU_EXP_ACT_MX_A,
    DSPMODE_CU_EXP_ACT_MX_B,
    DSPMODE_CU_EXP_ACT_MX_C,
    DSPMODE_CU_EXP_ACT_MX_D,
    DSPMODE_CU_EXP_ACT_MX_T,
    DSPMODE_CU_IMP_APP_MX_A,
    DSPMODE_CU_IMP_APP_MX_B,
    DSPMODE_CU_IMP_APP_MX_C,
    DSPMODE_CU_IMP_APP_MX_D,
    DSPMODE_CU_IMP_APP_MX_T,
    DSPMODE_CU_EXP_APP_MX_A,
    DSPMODE_CU_EXP_APP_MX_B,
    DSPMODE_CU_EXP_APP_MX_C,
    DSPMODE_CU_EXP_APP_MX_D,
    DSPMODE_CU_EXP_APP_MX_T,
    // DSPMODE_CU_IMP_ACT_MXTIM 배치 (see dsp_mode_is_mxtim())
    DSPMODE_CU_IMP_ACT_MXTIM_A,
    DSPMODE_CU_IMP_ACT_MXTIM_B,
    DSPMODE_CU_IMP_ACT_MXTIM_C,
    DSPMODE_CU_IMP_ACT_MXTIM_D,
    DSPMODE_CU_IMP_ACT_MXTIM_T,
    DSPMODE_CU_EXP_ACT_MXTIM_A,
    DSPMODE_CU_EXP_ACT_MXTIM_B,
    DSPMODE_CU_EXP_ACT_MXTIM_C,
    DSPMODE_CU_EXP_ACT_MXTIM_D,
    DSPMODE_CU_EXP_ACT_MXTIM_T,
    DSPMODE_CU_IMP_APP_MXTIM_A,
    DSPMODE_CU_IMP_APP_MXTIM_B,
    DSPMODE_CU_IMP_APP_MXTIM_C,
    DSPMODE_CU_IMP_APP_MXTIM_D,
    DSPMODE_CU_IMP_APP_MXTIM_T,
    DSPMODE_CU_EXP_APP_MXTIM_A,
    DSPMODE_CU_EXP_APP_MXTIM_B,
    DSPMODE_CU_EXP_APP_MXTIM_C,
    DSPMODE_CU_EXP_APP_MXTIM_D,
    DSPMODE_CU_EXP_APP_MXTIM_T,
    DSPMODE_CU_IMP_ACT_CUMMX_A,
    DSPMODE_CU_IMP_ACT_CUMMX_B,
    DSPMODE_CU_IMP_ACT_CUMMX_C,
    DSPMODE_CU_IMP_ACT_CUMMX_D,
    DSPMODE_CU_IMP_ACT_CUMMX_T,
    DSPMODE_CU_EXP_ACT_CUMMX_A,
    DSPMODE_CU_EXP_ACT_CUMMX_B,
    DSPMODE_CU_EXP_ACT_CUMMX_C,
    DSPMODE_CU_EXP_ACT_CUMMX_D,
    DSPMODE_CU_EXP_ACT_CUMMX_T,
    DSPMODE_CU_IMP_APP_CUMMX_A,
    DSPMODE_CU_IMP_APP_CUMMX_B,
    DSPMODE_CU_IMP_APP_CUMMX_C,
    DSPMODE_CU_IMP_APP_CUMMX_D,
    DSPMODE_CU_IMP_APP_CUMMX_T,
    DSPMODE_CU_EXP_APP_CUMMX_A,
    DSPMODE_CU_EXP_APP_CUMMX_B,
    DSPMODE_CU_EXP_APP_CUMMX_C,
    DSPMODE_CU_EXP_APP_CUMMX_D,
    DSPMODE_CU_EXP_APP_CUMMX_T,
    DSPMODE_CU_IMP_PF_A,
    DSPMODE_CU_IMP_PF_B,
    DSPMODE_CU_IMP_PF_C,
    DSPMODE_CU_IMP_PF_D,
    DSPMODE_CU_IMP_PF_T,
    DSPMODE_CU_EXP_PF_A,
    DSPMODE_CU_EXP_PF_B,
    DSPMODE_CU_EXP_PF_C,
    DSPMODE_CU_EXP_PF_D,
    DSPMODE_CU_EXP_PF_T,
    // 전월 (DSPMODE_BF_IMP_ACT_A 은 첫번재 enum 값)
    DSPMODE_BF_IMP_ACT_A,
    DSPMODE_BF_IMP_ACT_B,
    DSPMODE_BF_IMP_ACT_C,
    DSPMODE_BF_IMP_ACT_D,
    DSPMODE_BF_IMP_ACT_T,
    DSPMODE_BF_EXP_ACT_A,
    DSPMODE_BF_EXP_ACT_B,
    DSPMODE_BF_EXP_ACT_C,
    DSPMODE_BF_EXP_ACT_D,
    DSPMODE_BF_EXP_ACT_T,
    DSPMODE_BF_IMP_REACT_A,
    DSPMODE_BF_IMP_REACT_B,
    DSPMODE_BF_IMP_REACT_C,
    DSPMODE_BF_IMP_REACT_D,
    DSPMODE_BF_IMP_REACT_T,
    DSPMODE_BF_IMP_lagREACT_A,
    DSPMODE_BF_IMP_lagREACT_B,
    DSPMODE_BF_IMP_lagREACT_C,
    DSPMODE_BF_IMP_lagREACT_D,
    DSPMODE_BF_IMP_lagREACT_T,
    DSPMODE_BF_IMP_leadREACT_A,
    DSPMODE_BF_IMP_leadREACT_B,
    DSPMODE_BF_IMP_leadREACT_C,
    DSPMODE_BF_IMP_leadREACT_D,
    DSPMODE_BF_IMP_leadREACT_T,
    DSPMODE_BF_EXP_REACT_A,
    DSPMODE_BF_EXP_REACT_B,
    DSPMODE_BF_EXP_REACT_C,
    DSPMODE_BF_EXP_REACT_D,
    DSPMODE_BF_EXP_REACT_T,
    DSPMODE_BF_EXP_lagREACT_A,
    DSPMODE_BF_EXP_lagREACT_B,
    DSPMODE_BF_EXP_lagREACT_C,
    DSPMODE_BF_EXP_lagREACT_D,
    DSPMODE_BF_EXP_lagREACT_T,
    DSPMODE_BF_EXP_leadREACT_A,
    DSPMODE_BF_EXP_leadREACT_B,
    DSPMODE_BF_EXP_leadREACT_C,
    DSPMODE_BF_EXP_leadREACT_D,
    DSPMODE_BF_EXP_leadREACT_T,
    DSPMODE_BF_IMP_APP_A,
    DSPMODE_BF_IMP_APP_B,
    DSPMODE_BF_IMP_APP_C,
    DSPMODE_BF_IMP_APP_D,
    DSPMODE_BF_IMP_APP_T,
    DSPMODE_BF_EXP_APP_A,
    DSPMODE_BF_EXP_APP_B,
    DSPMODE_BF_EXP_APP_C,
    DSPMODE_BF_EXP_APP_D,
    DSPMODE_BF_EXP_APP_T,
    DSPMODE_BF_IMP_ACT_MX_A,
    DSPMODE_BF_IMP_ACT_MX_B,
    DSPMODE_BF_IMP_ACT_MX_C,
    DSPMODE_BF_IMP_ACT_MX_D,
    DSPMODE_BF_IMP_ACT_MX_T,
    DSPMODE_BF_EXP_ACT_MX_A,
    DSPMODE_BF_EXP_ACT_MX_B,
    DSPMODE_BF_EXP_ACT_MX_C,
    DSPMODE_BF_EXP_ACT_MX_D,
    DSPMODE_BF_EXP_ACT_MX_T,
    DSPMODE_BF_IMP_APP_MX_A,
    DSPMODE_BF_IMP_APP_MX_B,
    DSPMODE_BF_IMP_APP_MX_C,
    DSPMODE_BF_IMP_APP_MX_D,
    DSPMODE_BF_IMP_APP_MX_T,
    DSPMODE_BF_EXP_APP_MX_A,
    DSPMODE_BF_EXP_APP_MX_B,
    DSPMODE_BF_EXP_APP_MX_C,
    DSPMODE_BF_EXP_APP_MX_D,
    DSPMODE_BF_EXP_APP_MX_T,
    // DSPMODE_BF_IMP_ACT_MXTIM 배치 (see dsp_mode_is_mxtim())
    DSPMODE_BF_IMP_ACT_MXTIM_A,
    DSPMODE_BF_IMP_ACT_MXTIM_B,
    DSPMODE_BF_IMP_ACT_MXTIM_C,
    DSPMODE_BF_IMP_ACT_MXTIM_D,
    DSPMODE_BF_IMP_ACT_MXTIM_T,
    DSPMODE_BF_EXP_ACT_MXTIM_A,
    DSPMODE_BF_EXP_ACT_MXTIM_B,
    DSPMODE_BF_EXP_ACT_MXTIM_C,
    DSPMODE_BF_EXP_ACT_MXTIM_D,
    DSPMODE_BF_EXP_ACT_MXTIM_T,
    DSPMODE_BF_IMP_APP_MXTIM_A,
    DSPMODE_BF_IMP_APP_MXTIM_B,
    DSPMODE_BF_IMP_APP_MXTIM_C,
    DSPMODE_BF_IMP_APP_MXTIM_D,
    DSPMODE_BF_IMP_APP_MXTIM_T,
    DSPMODE_BF_EXP_APP_MXTIM_A,
    DSPMODE_BF_EXP_APP_MXTIM_B,
    DSPMODE_BF_EXP_APP_MXTIM_C,
    DSPMODE_BF_EXP_APP_MXTIM_D,
    DSPMODE_BF_EXP_APP_MXTIM_T,
    DSPMODE_BF_IMP_ACT_CUMMX_A,
    DSPMODE_BF_IMP_ACT_CUMMX_B,
    DSPMODE_BF_IMP_ACT_CUMMX_C,
    DSPMODE_BF_IMP_ACT_CUMMX_D,
    DSPMODE_BF_IMP_ACT_CUMMX_T,
    DSPMODE_BF_EXP_ACT_CUMMX_A,
    DSPMODE_BF_EXP_ACT_CUMMX_B,
    DSPMODE_BF_EXP_ACT_CUMMX_C,
    DSPMODE_BF_EXP_ACT_CUMMX_D,
    DSPMODE_BF_EXP_ACT_CUMMX_T,
    DSPMODE_BF_IMP_APP_CUMMX_A,
    DSPMODE_BF_IMP_APP_CUMMX_B,
    DSPMODE_BF_IMP_APP_CUMMX_C,
    DSPMODE_BF_IMP_APP_CUMMX_D,
    DSPMODE_BF_IMP_APP_CUMMX_T,
    DSPMODE_BF_EXP_APP_CUMMX_A,
    DSPMODE_BF_EXP_APP_CUMMX_B,
    DSPMODE_BF_EXP_APP_CUMMX_C,
    DSPMODE_BF_EXP_APP_CUMMX_D,
    DSPMODE_BF_EXP_APP_CUMMX_T,

    DSPMODE_BF_IMP_ACT_A_NPRD,
    DSPMODE_BF_IMP_ACT_B_NPRD,
    DSPMODE_BF_IMP_ACT_C_NPRD,
    DSPMODE_BF_IMP_ACT_D_NPRD,
    DSPMODE_BF_IMP_ACT_T_NPRD,
    DSPMODE_BF_EXP_ACT_A_NPRD,
    DSPMODE_BF_EXP_ACT_B_NPRD,
    DSPMODE_BF_EXP_ACT_C_NPRD,
    DSPMODE_BF_EXP_ACT_D_NPRD,
    DSPMODE_BF_EXP_ACT_T_NPRD,
    DSPMODE_BF_IMP_ACT_CUMMX_A_NPRD,
    DSPMODE_BF_IMP_ACT_CUMMX_B_NPRD,
    DSPMODE_BF_IMP_ACT_CUMMX_C_NPRD,
    DSPMODE_BF_IMP_ACT_CUMMX_D_NPRD,
    DSPMODE_BF_IMP_ACT_CUMMX_T_NPRD,

    DSPMODE_BF_IMP_PF_A,
    DSPMODE_BF_IMP_PF_B,
    DSPMODE_BF_IMP_PF_C,
    DSPMODE_BF_IMP_PF_D,
    DSPMODE_BF_IMP_PF_T,
    DSPMODE_BF_EXP_PF_A,
    DSPMODE_BF_EXP_PF_B,
    DSPMODE_BF_EXP_PF_C,
    DSPMODE_BF_EXP_PF_D,
    DSPMODE_BF_EXP_PF_T,
    // 전전월 (DSPMODE_BBF_EXP_PF_T 은 마지막 enum 값)
    DSPMODE_BBF_IMP_ACT_A,
    DSPMODE_BBF_IMP_ACT_B,
    DSPMODE_BBF_IMP_ACT_C,
    DSPMODE_BBF_IMP_ACT_D,
    DSPMODE_BBF_IMP_ACT_T,
    DSPMODE_BBF_EXP_ACT_A,
    DSPMODE_BBF_EXP_ACT_B,
    DSPMODE_BBF_EXP_ACT_C,
    DSPMODE_BBF_EXP_ACT_D,
    DSPMODE_BBF_EXP_ACT_T,
    DSPMODE_BBF_IMP_REACT_A,
    DSPMODE_BBF_IMP_REACT_B,
    DSPMODE_BBF_IMP_REACT_C,
    DSPMODE_BBF_IMP_REACT_D,
    DSPMODE_BBF_IMP_REACT_T,
    DSPMODE_BBF_IMP_lagREACT_A,
    DSPMODE_BBF_IMP_lagREACT_B,
    DSPMODE_BBF_IMP_lagREACT_C,
    DSPMODE_BBF_IMP_lagREACT_D,
    DSPMODE_BBF_IMP_lagREACT_T,
    DSPMODE_BBF_IMP_leadREACT_A,
    DSPMODE_BBF_IMP_leadREACT_B,
    DSPMODE_BBF_IMP_leadREACT_C,
    DSPMODE_BBF_IMP_leadREACT_D,
    DSPMODE_BBF_IMP_leadREACT_T,
    DSPMODE_BBF_EXP_REACT_A,
    DSPMODE_BBF_EXP_REACT_B,
    DSPMODE_BBF_EXP_REACT_C,
    DSPMODE_BBF_EXP_REACT_D,
    DSPMODE_BBF_EXP_REACT_T,
    DSPMODE_BBF_EXP_lagREACT_A,
    DSPMODE_BBF_EXP_lagREACT_B,
    DSPMODE_BBF_EXP_lagREACT_C,
    DSPMODE_BBF_EXP_lagREACT_D,
    DSPMODE_BBF_EXP_lagREACT_T,
    DSPMODE_BBF_EXP_leadREACT_A,
    DSPMODE_BBF_EXP_leadREACT_B,
    DSPMODE_BBF_EXP_leadREACT_C,
    DSPMODE_BBF_EXP_leadREACT_D,
    DSPMODE_BBF_EXP_leadREACT_T,
    DSPMODE_BBF_IMP_APP_A,
    DSPMODE_BBF_IMP_APP_B,
    DSPMODE_BBF_IMP_APP_C,
    DSPMODE_BBF_IMP_APP_D,
    DSPMODE_BBF_IMP_APP_T,
    DSPMODE_BBF_EXP_APP_A,
    DSPMODE_BBF_EXP_APP_B,
    DSPMODE_BBF_EXP_APP_C,
    DSPMODE_BBF_EXP_APP_D,
    DSPMODE_BBF_EXP_APP_T,
    DSPMODE_BBF_IMP_ACT_MX_A,
    DSPMODE_BBF_IMP_ACT_MX_B,
    DSPMODE_BBF_IMP_ACT_MX_C,
    DSPMODE_BBF_IMP_ACT_MX_D,
    DSPMODE_BBF_IMP_ACT_MX_T,
    DSPMODE_BBF_EXP_ACT_MX_A,
    DSPMODE_BBF_EXP_ACT_MX_B,
    DSPMODE_BBF_EXP_ACT_MX_C,
    DSPMODE_BBF_EXP_ACT_MX_D,
    DSPMODE_BBF_EXP_ACT_MX_T,
    DSPMODE_BBF_IMP_APP_MX_A,
    DSPMODE_BBF_IMP_APP_MX_B,
    DSPMODE_BBF_IMP_APP_MX_C,
    DSPMODE_BBF_IMP_APP_MX_D,
    DSPMODE_BBF_IMP_APP_MX_T,
    DSPMODE_BBF_EXP_APP_MX_A,
    DSPMODE_BBF_EXP_APP_MX_B,
    DSPMODE_BBF_EXP_APP_MX_C,
    DSPMODE_BBF_EXP_APP_MX_D,
    DSPMODE_BBF_EXP_APP_MX_T,
    // DSPMODE_BBF_IMP_ACT_MXTIM 배치 (see dsp_mode_is_mxtim())
    DSPMODE_BBF_IMP_ACT_MXTIM_A,
    DSPMODE_BBF_IMP_ACT_MXTIM_B,
    DSPMODE_BBF_IMP_ACT_MXTIM_C,
    DSPMODE_BBF_IMP_ACT_MXTIM_D,
    DSPMODE_BBF_IMP_ACT_MXTIM_T,
    DSPMODE_BBF_EXP_ACT_MXTIM_A,
    DSPMODE_BBF_EXP_ACT_MXTIM_B,
    DSPMODE_BBF_EXP_ACT_MXTIM_C,
    DSPMODE_BBF_EXP_ACT_MXTIM_D,
    DSPMODE_BBF_EXP_ACT_MXTIM_T,
    DSPMODE_BBF_IMP_APP_MXTIM_A,
    DSPMODE_BBF_IMP_APP_MXTIM_B,
    DSPMODE_BBF_IMP_APP_MXTIM_C,
    DSPMODE_BBF_IMP_APP_MXTIM_D,
    DSPMODE_BBF_IMP_APP_MXTIM_T,
    DSPMODE_BBF_EXP_APP_MXTIM_A,
    DSPMODE_BBF_EXP_APP_MXTIM_B,
    DSPMODE_BBF_EXP_APP_MXTIM_C,
    DSPMODE_BBF_EXP_APP_MXTIM_D,
    DSPMODE_BBF_EXP_APP_MXTIM_T,
    DSPMODE_BBF_IMP_ACT_CUMMX_A,
    DSPMODE_BBF_IMP_ACT_CUMMX_B,
    DSPMODE_BBF_IMP_ACT_CUMMX_C,
    DSPMODE_BBF_IMP_ACT_CUMMX_D,
    DSPMODE_BBF_IMP_ACT_CUMMX_T,
    DSPMODE_BBF_EXP_ACT_CUMMX_A,
    DSPMODE_BBF_EXP_ACT_CUMMX_B,
    DSPMODE_BBF_EXP_ACT_CUMMX_C,
    DSPMODE_BBF_EXP_ACT_CUMMX_D,
    DSPMODE_BBF_EXP_ACT_CUMMX_T,
    DSPMODE_BBF_IMP_APP_CUMMX_A,
    DSPMODE_BBF_IMP_APP_CUMMX_B,
    DSPMODE_BBF_IMP_APP_CUMMX_C,
    DSPMODE_BBF_IMP_APP_CUMMX_D,
    DSPMODE_BBF_IMP_APP_CUMMX_T,
    DSPMODE_BBF_EXP_APP_CUMMX_A,
    DSPMODE_BBF_EXP_APP_CUMMX_B,
    DSPMODE_BBF_EXP_APP_CUMMX_C,
    DSPMODE_BBF_EXP_APP_CUMMX_D,
    DSPMODE_BBF_EXP_APP_CUMMX_T,
    DSPMODE_BBF_IMP_PF_A,
    DSPMODE_BBF_IMP_PF_B,
    DSPMODE_BBF_IMP_PF_C,
    DSPMODE_BBF_IMP_PF_D,
    DSPMODE_BBF_IMP_PF_T,
    DSPMODE_BBF_EXP_PF_A,
    DSPMODE_BBF_EXP_PF_B,
    DSPMODE_BBF_EXP_PF_C,
    DSPMODE_BBF_EXP_PF_D,
    DSPMODE_BBF_EXP_PF_T,

    DSPMODE_BBF_IMP_ACT_A_NPRD,
    DSPMODE_BBF_IMP_ACT_B_NPRD,
    DSPMODE_BBF_IMP_ACT_C_NPRD,
    DSPMODE_BBF_IMP_ACT_D_NPRD,
    DSPMODE_BBF_IMP_ACT_T_NPRD,
    DSPMODE_BBF_EXP_ACT_A_NPRD,
    DSPMODE_BBF_EXP_ACT_B_NPRD,
    DSPMODE_BBF_EXP_ACT_C_NPRD,
    DSPMODE_BBF_EXP_ACT_D_NPRD,
    DSPMODE_BBF_EXP_ACT_T_NPRD,
    DSPMODE_BBF_IMP_ACT_CUMMX_A_NPRD,
    DSPMODE_BBF_IMP_ACT_CUMMX_B_NPRD,
    DSPMODE_BBF_IMP_ACT_CUMMX_C_NPRD,
    DSPMODE_BBF_IMP_ACT_CUMMX_D_NPRD,
    DSPMODE_BBF_IMP_ACT_CUMMX_T_NPRD,

    // 현재/직전
    DSPMODE_CURR_IMP_ACT,
    DSPMODE_CURR_IMP_APP,
    DSPMODE_CURR_EXP_ACT,
    DSPMODE_CURR_EXP_APP,  // next of BBF
    DSPMODE_LAST_IMP_ACT,
    DSPMODE_LAST_IMP_APP,
    DSPMODE_LAST_EXP_ACT,
    DSPMODE_LAST_EXP_APP,  // DSPMODE_LAST_EXP_APP is checked
    DSPMODE_LAST_EXP_PF,
    DSPMODE_LAST_IMP_PF,  // DSPMODE_LAST_IMP_PF is checked

#if 1 /* bccho, 2024-05-17 */
    DSPMODE_TOU_PROG_ID,
#endif
    DSPMODE_REG_DATE, /* 정기 검침일 */

    DSPMODE_mDR_NUM,
    DSPMODE_BUY_ENERGY,
    DSPMODE_BUY_MONEY,
    DSPMODE_REM_ENERGY,
    DSPMODE_REM_MONEY,
    DSPMODE_REM_TIME,
    DSPMODE_ERROR,

    // 시험 모드 (ce update => DSPMODE_V1 ~ DSPMODE_tRaw)  // + DSPMODE_REG_DATE

    DSPMODE_V1,
    DSPMODE_V2,
    DSPMODE_V3,
    DSPMODE_LtoL1,
    DSPMODE_LtoL2,
    DSPMODE_LtoL3,
    DSPMODE_I1,
    DSPMODE_I2,
    DSPMODE_I3,
    DSPMODE_P1,
    DSPMODE_P2,
    DSPMODE_P3,
    DSPMODE_THD_V1,
    DSPMODE_THD_V2,
    DSPMODE_THD_V3,
    DSPMODE_tS,
    DSPMODE_TEMP,
    DSPMODE_SMODE,
    DSPMODE_FREQ,
    DSPMODE_PVT,
    DSPMODE_SYSP_SW_VER,
    DSPMODE_MODEM_VER,
    DSPMODE_MTP_SW_VER,
    DSPMODE_485_BPS,
    DSPMODE_CONDENSOR_EN,
    DSPMODE_ERR_PULSE,
    DSPMODE_AUTO_BI_DIR_MODE,
    DSPMODE_OVERCURR,
    DSPMODE_ERR_RATE1,
    DSPMODE_ERR_RATE2,
    DSPMODE_ERR_RATE3,

    DSPMODE_LATCHON,
    DSPMODE_BILL5DAYS,
    DSPMODE_tRaw,
    DSPMODE_TEST_REG_DATE,
    DSPMODE_TARIFF_RATE,

    NUM_DSPMODE
} disp_mode_type;

/* 3.17.1.2 관리자 순환 표시 모드 : 표시 가능 항목은 40개 이하로 제한되어야
 * 한다. */
// TODO: (WD)
#define MAX_SUPP_DSP_NUM 40
typedef struct
{
    uint8_t mode_cnt;
    uint8_t dsptime;
    uint16_t dsp_mode[MAX_SUPP_DSP_NUM];
    uint16_t CRC_M;
} dsp_supply_type;

#define dsp_is_suppdsp_available() (circ_state_suppdsp_mode.mode_cnt > 0)

#define SEG00 (1 << 0)
#define SEG01 (1 << 1)
#define SEG02 (1 << 2)
#define SEG03 (1 << 3)
#define SEG04 (1 << 4)
#define SEG05 (1 << 5)
#define SEG06 (1 << 6)
#define SEG07 (1 << 7)
#define SEG08 (1 << 8)
#define SEG09 (1 << 9)
#define SEG10 (1 << 10)
#define SEG11 (1 << 11)
#define SEG12 (1 << 12)
#define SEG13 (1 << 13)
#define SEG14 (1 << 14)
#define SEG15 (1 << 15)
#define SEG16 (1 << 16)
#define SEG17 (1 << 17)
#define SEG18 (1 << 18)
#define SEG19 (1 << 19)
#define SEG20 (1 << 20)
#define SEG21 (1 << 21)
#ifdef M2354_NEW_HW
#define SEG22 (1 << 23)
#define SEG23 (1 << 22)
#else
#define SEG22 (1 << 22)
#define SEG23 (1 << 23)
#endif
#define SEG24 (1 << 24)
#define SEG25 (1 << 25)
#define SEG26 (1 << 26)
#define SEG27 (1 << 27)
#define SEG28 (1 << 28)
#define SEG29 (1 << 29)
#define SEG30 (1 << 30)
#define SEG31 (1 << 31)
#define SEG32 (1 << 0)
#define SEG33 (1 << 1)
#define SEG34 (1 << 2)
#define SEG35 (1 << 3)
#define SEG36 (1 << 4)
#define SEG37 (1 << 5)
#define SEG38 (1 << 6)
#define SEG39 (1 << 7)
#define SEG40 (1 << 8)
#define SEG41 (1 << 9)
#define SEG42 (1 << 10)
#define SEG43 (1 << 11)

#define FONT_ABCD 0
#define FONT_EGFx 1

#define DSP_COM1 0
#define DSP_COM2 1
#define DSP_COM3 2
#define DSP_COM4 3

#define LRAM_L_ADDR 0
#define LRAM_H_ADDR 1

typedef enum
{
    DSP_GET_V,
    DSP_GET_ADDR_INFO
} EN_DSP_GET_TYPE;

#if 1 /* bccho, LCD, 2023-08-04 */
#define LCD_DERR_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG05)
#define LCD_DERR_OFF

#define LCD_DRATE_A_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG00)
#define LCD_DRATE_A_OFF (st_lcd_ram.com_xx_s31_00[DSP_COM4] &= ~(SEG00))

#define LCD_DRATE_B_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG00)
#define LCD_DRATE_B_OFF (st_lcd_ram.com_xx_s31_00[DSP_COM3] &= ~(SEG00))

#define LCD_DRATE_C_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG00)
#define LCD_DRATE_C_OFF (st_lcd_ram.com_xx_s31_00[DSP_COM2] &= ~(SEG00))

#define LCD_DRATE_D_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG00)
#define LCD_DRATE_D_OFF (st_lcd_ram.com_xx_s31_00[DSP_COM1] &= ~(SEG00))

/* 둘째 decimal point */
#define LCD_DP2_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG14)
#define LCD_DP2_OFF

/* 넷째 decimal point */
#define LCD_DP4_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG19)
#define LCD_DP4_OFF

/* 셋째 decimal point */
#define LCD_DP3_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG17)
#define LCD_DP3_OFF

/* 첫째 decimal point */
#define LCD_DP1_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG11)
#define LCD_DP1_OFF

#define LCD_DSUN_COMM_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG11)
#define LCD_DSUN_COMM_OFF

#define LCD_DBAT_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG16)
#define LCD_DBAT_OFF

#define LCD_DQUP_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG03)
#define LCD_DQUP_OFF

#define LCD_DQDOWN_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG01)
#define LCD_DQDOWN_OFF

#define LCD_DQRIGHT_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG02)
#define LCD_DQRIGHT_OFF

#define LCD_DQLEFT_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG01)
#define LCD_DQLEFT_OFF

#define LCD_DPLS1_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG02)
#define LCD_DPLS1_OFF

#define LCD_DPLS2_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG02)
#define LCD_DPLS2_OFF

#define LCD_DPLS3_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG01)
#define LCD_DPLS3_OFF

#define LCD_DPLS4_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG01)
#define LCD_DPLS4_OFF

#define LCD_DPLS5_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG02)
#define LCD_DPLS5_OFF

#define LCD_DCL_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG07)
#define LCD_DCL_OFF

#define LCD_DCONN_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG16)
#define LCD_DCONN_OFF

#define LCD_DTEST_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG09)
#define LCD_DTEST_OFF                      \
    (st_lcd_ram.com_xx_s31_00[DSP_COM1] &= \
     ~SEG31)  // jp.kim 24.11.08  LCD창 "TEST" 표시  off

/* bccho, 2024-09-05, 삼상 */
#define LCD_DV1_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG23)
#define LCD_DV1_OFF
#define LCD_DV2_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG23)
#define LCD_DV2_OFF
#define LCD_DV3_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG23)
#define LCD_DV3_OFF

#define LCD_DLOAD_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG11)
#define LCD_DLOAD_OFF

#define LCD_DLOADO_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG12)
#define LCD_DLOADO_OFF

#define LCD_DLOADX_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG23)
#define LCD_DLOADX_OFF

#define LCD_DSECRET_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG16)
#define LCD_DSECRET_OFF

#define LCD_DCOLON1_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG11)
#define LCD_DCOLON1_OFF

#define LCD_DCOLON2_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG16)
#define LCD_DCOLON2_OFF

#define LCD_DUNIT_k_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG21)
#define LCD_DUNIT_k_OFF

#define LCD_DUNIT_V1_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG21)
#define LCD_DUNIT_V1_OFF

#define LCD_DUNIT_V2_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG21)
#define LCD_DUNIT_V2_OFF

#define LCD_DUNIT_A_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG21)
#define LCD_DUNIT_A_OFF

#define LCD_DUNIT_r_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG22)
#define LCD_DUNIT_r_OFF

#define LCD_DUNIT_h_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG22)
#define LCD_DUNIT_h_OFF

#define LCD_DUNIT_z_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG22)
#define LCD_DUNIT_z_OFF

#define LCD_DPERCENT_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG22)
#define LCD_DPERCENT_OFF

///////////////////////////////////////////////////////////////////////
#define LCD_DP1_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG11)
#define LCD_DP2_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG17)
#define LCD_DP3_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG19)
#define LCD_DP4_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG14)

#define LCD_DERR_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG05)
#define LCD_DMEM_IS_ON (false)
#define LCD_DBAT_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG16)
#define LCD_DCL_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG07)
#define LCD_DCONN_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG16)
#define LCD_DSECRET_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG16)
#define LCD_DTEST_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG09)

#define LCD_DV1_IS_ON (false)
#define LCD_DV2_IS_ON (false)
#define LCD_DV3_IS_ON (false)

#define LCD_DLOAD_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG11)
#define LCD_DLOADO_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG12)
#define LCD_DLOADX_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG23)

#define LCD_DCOLON1_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG11)
#define LCD_DCOLON2_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG16)

#define LCD_DUNITK_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG21)
#define LCD_DUNITV1_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG21)
#define LCD_DUNITV2_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG21)
#define LCD_DUNITA_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG21)
#define LCD_DUNITr_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG22)
#define LCD_DUNITh_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG22)
#define LCD_DUNITz_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG22)
#define LCD_DUNITPERC_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG22)

#define LCD_DQUP_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG03)
#define LCD_DQDOWN_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG01)
#define LCD_DQLEFT_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG01)
#define LCD_DQRIGHT_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG02)

#define LCD_DPLS1_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG02)
#define LCD_DPLS2_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG02)
#define LCD_DPLS3_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG01)
#define LCD_DPLS4_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG01)
#define LCD_DPLS5_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG02)

#define LCD_DRATEA_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG00)
#define LCD_DRATEB_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG00)
#define LCD_DRATEC_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG00)
#define LCD_DRATED_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG00)
#else /* bccho, LCD, 2023-08-04 */
#define LCD_DERR_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG13)
#define LCD_DERR_OFF
#define LCD_DRATE_A_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG22)
#define LCD_DRATE_A_OFF (st_lcd_ram.com_xx_s31_00[DSP_COM4] &= ~(SEG22))
#define LCD_DRATE_B_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG22)
#define LCD_DRATE_B_OFF (st_lcd_ram.com_xx_s31_00[DSP_COM3] &= ~(SEG22))
#define LCD_DRATE_C_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG22)
#define LCD_DRATE_C_OFF (st_lcd_ram.com_xx_s31_00[DSP_COM2] &= ~(SEG22))
#define LCD_DRATE_D_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG22)
#define LCD_DRATE_D_OFF (st_lcd_ram.com_xx_s31_00[DSP_COM1] &= ~(SEG22))
#define LCD_DP4_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] |= SEG41)
#define LCD_DP4_OFF
#define LCD_DP3_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG27)
#define LCD_DP3_OFF
#define LCD_DP2_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG24)
#define LCD_DP2_OFF
#define LCD_DP1_ON (st_lcd_ram.com_xx_s43_32[DSP_COM2] |= SEG33)
#define LCD_DP1_OFF

#define LCD_DSUN_COMM_ON (st_lcd_ram.com_xx_s43_32[DSP_COM4] |= SEG33)
#define LCD_DSUN_COMM_OFF

#define LCD_DBAT_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG26)
#define LCD_DBAT_OFF
#define LCD_DQUP_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG11)
#define LCD_DQUP_OFF
#define LCD_DQDOWN_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG05)
#define LCD_DQDOWN_OFF

#define LCD_DQRIGHT_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG10)
#define LCD_DQRIGHT_OFF
#define LCD_DQLEFT_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG05)
#define LCD_DQLEFT_OFF

#define LCD_DPLS1_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG10)
#define LCD_DPLS1_OFF
#define LCD_DPLS2_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG10)
#define LCD_DPLS2_OFF
#define LCD_DPLS3_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG05)
#define LCD_DPLS3_OFF
#define LCD_DPLS4_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG05)
#define LCD_DPLS4_OFF
#define LCD_DPLS5_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] |= SEG10)
#define LCD_DPLS5_OFF

#define LCD_DCL_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG15)
#define LCD_DCL_OFF
#define LCD_DCONN_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] |= SEG26)
#define LCD_DCONN_OFF
#define LCD_DTEST_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG31)
#define LCD_DTEST_OFF
#define LCD_DV1_ON
#define LCD_DV1_OFF
#define LCD_DV2_ON
#define LCD_DV2_OFF
#define LCD_DV3_ON
#define LCD_DV3_OFF
#define LCD_DLOAD_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] |= SEG33)
#define LCD_DLOAD_OFF
#define LCD_DLOADO_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] |= SEG34)
#define LCD_DLOADO_OFF
#define LCD_DLOADX_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] |= SEG39)
#define LCD_DLOADX_OFF
#define LCD_DSECRET_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] |= SEG26)
#define LCD_DSECRET_OFF
#define LCD_DCOLON1_ON (st_lcd_ram.com_xx_s43_32[DSP_COM3] |= SEG33)
#define LCD_DCOLON1_OFF
#define LCD_DCOLON2_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] |= SEG26)
#define LCD_DCOLON2_OFF
#define LCD_DUNIT_k_ON (st_lcd_ram.com_xx_s43_32[DSP_COM4] |= SEG37)
#define LCD_DUNIT_k_OFF
#define LCD_DUNIT_V1_ON (st_lcd_ram.com_xx_s43_32[DSP_COM3] |= SEG37)
#define LCD_DUNIT_V1_OFF
#define LCD_DUNIT_V2_ON (st_lcd_ram.com_xx_s43_32[DSP_COM2] |= SEG37)
#define LCD_DUNIT_V2_OFF
#define LCD_DUNIT_A_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] |= SEG37)
#define LCD_DUNIT_A_OFF
#define LCD_DUNIT_r_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] |= SEG38)
#define LCD_DUNIT_r_OFF
#define LCD_DUNIT_h_ON (st_lcd_ram.com_xx_s43_32[DSP_COM2] |= SEG38)
#define LCD_DUNIT_h_OFF
#define LCD_DUNIT_z_ON (st_lcd_ram.com_xx_s43_32[DSP_COM3] |= SEG38)
#define LCD_DUNIT_z_OFF
#define LCD_DPERCENT_ON (st_lcd_ram.com_xx_s43_32[DSP_COM4] |= SEG38)
#define LCD_DPERCENT_OFF

#define LCD_DP1_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM2] & SEG33)
#define LCD_DP2_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG24)
#define LCD_DP3_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG27)
#define LCD_DP4_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] & SEG41)

#define LCD_DERR_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG13)
#define LCD_DMEM_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM4] & SEG33)
#define LCD_DBAT_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG26)
#define LCD_DCL_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG15)
#define LCD_DCONN_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG26)
#define LCD_DSECRET_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG26)
#define LCD_DTEST_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG31)

#define LCD_DV1_IS_ON (false)
#define LCD_DV2_IS_ON (false)
#define LCD_DV3_IS_ON (false)

#define LCD_DLOAD_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] & SEG33)
#define LCD_DLOADO_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] & SEG34)
#define LCD_DLOADX_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] & SEG39)

#define LCD_DCOLON1_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM3] & SEG33)
#define LCD_DCOLON2_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG26)

#define LCD_DUNITK_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM4] & SEG37)
#define LCD_DUNITV1_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM3] & SEG37)
#define LCD_DUNITV2_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM2] & SEG37)
#define LCD_DUNITA_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] & SEG37)
#define LCD_DUNITr_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM1] & SEG38)
#define LCD_DUNITh_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM2] & SEG38)
#define LCD_DUNITz_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM3] & SEG38)
#define LCD_DUNITPERC_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM4] & SEG38)

#define LCD_DQUP_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG11)
#define LCD_DQDOWN_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG05)
#define LCD_DQLEFT_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG05)
#define LCD_DQRIGHT_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG10)

#define LCD_DPLS1_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG10)
#define LCD_DPLS2_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG10)
#define LCD_DPLS3_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG05)
#define LCD_DPLS4_IS_ON (st_lcd_ram.com_xx_s43_32[DSP_COM3] & SEG05)
#define LCD_DPLS5_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG10)

#define LCD_DRATEA_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM4] & SEG22)
#define LCD_DRATEB_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM3] & SEG22)
#define LCD_DRATEC_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM2] & SEG22)
#define LCD_DRATED_IS_ON (st_lcd_ram.com_xx_s31_00[DSP_COM1] & SEG22)
#endif /* bccho, LCD, 2023-08-04 */

extern uint32_t lcdmem[];
extern dsp_supply_type circ_state_suppdsp_mode;
extern int dsp_pulse_inc;
extern int dsp_pulse_inc_unit;
extern bool b_dsp_pulse_inc_timer;
extern uint8_t accmed_pls_inc;
extern int dsp_pulse_inc_timer;
extern int dsp_pulse_inc_timer_bak;
extern uint8_t lcd_map_comm[];

extern bool b_dsp_bm_finished;

void disp_init(void);
void disp_proc(void);
void dsp_blink_timer_proc(void);
void dsp_pwr_on(void);
void dsp_pwr_fail(void);
void dsp_circ_mode_chg_set(void);
void dsp_dmintv_set(uint8_t idx);
void dsp_test_state_init(void);
void dsp_circ_time_set(U8 bf_sec, U8 ss);
void dsp_cal_state(void);
void dsp_dr_state(void);
void dsp_calfail_state(void);
void dsp_DR_set(void);
void dsp_CalFail_set(void);
void dsp_calfail_state_exit(void);
void dsp_rcntdm_modified_set(void);
void dsp_allon(void);
void dsp_alloff(void);
void dsp_pulse_inc_set(int incr);
void dsp_fill_lcd_dot_comm(void);
bool dsp_is_test_overcurr(void);
void dsp_test_overcurr_toggle(void);
void dsp_test_condensor_toggle(void);
void dsp_test_err_pusle_toggle(void);
void dsp_test_auto_bidir_toggle(void);
bool dsp_test_mode_inc(void);
void dsp_test_smode_toggle(void);
void dsp_test_pvt_toggle(void);
bool dsp_is_test_mode_tS(void);
bool dsp_is_test_smode(void);
bool dsp_is_test_pvt(void);
bool dsp_is_test_trawmode(void);
void dsp_inp_init(date_time_type *pdt);
bool dsp_is_input_state(void);
disp_input_type get_dsp_inputmode(void);
uint8_t dsp_inp_digit_pos(void);
void dsp_inp_digit_inc(uint8_t mx);
void dsp_inp_digit_pos_move(uint8_t limit, uint8_t initval);
void dsp_inp_mtkind_inc(ratekind_type kind);
void dsp_inp_mtdir_toggle(void);
void dsp_inp_tou_rate_time(tou_struct_type *tou);
void dsp_inp_sig_sel(ad_sig_type sg);
void dsp_inp_digit_pos_move_tou(ratekind_type rtkind);
void dsp_inp_ts_toggle(uint8_t pos);
void dsp_inp_tourate_toggle(void);
uint8_t get_dispinp_ts_tourate(void);
void dsp_inp_err_set(void);
void dsp_inp_meas_toggle(void);
void dsp_inp_baud_toggle(void);
void dsp_inp_errpls_toggle(void);
void dsp_inp_condensor_toggle(void);
void dsp_inp_commen_toggle(void);
void dsp_inp_digit_inc_range(uint8_t mn, uint8_t mx);
void dsp_inp_ts_time(ts_struct_type *ts);
uint8_t get_dispinp_ts_ctrl(void);
void dsp_key_touched(void);
disp_state_type get_disp_state(void);
bool dsp_is_inp_end(void);
void dsp_circ_state_init(void);
void dsp_circ_state_mode_init(void);
void dsp_inp_time_init(date_time_type *pdt);
void dsp_inp_regrd_init(uint8_t regrd);
void dsp_inp_sigsel_init(ad_sig_type sg);
void dsp_inp_ts_init(ts_struct_type *ts);
void dsp_inp_scurr_init(uint16_t scurr);
void dsp_inp_scurr_2_init(int16_t scurr);
void dsp_inp_sCurrCnt_init(uint8_t cnt);
void dsp_inp_ratekind_init(ratekind_type mk);
void dsp_inp_tou_rate_init(tou_struct_type *tou);
uint8_t dsp_inp_lpintv_init(uint8_t intv);
void dsp_inp_sn1_init(void);
void dsp_inp_sn2_init(void);
void dsp_inp_mtdir_init(uint8_t dir);
void dsp_inp_meas_init(meas_method_type meas);
void dsp_inp_baud_init(baudrate_type baud);
void dsp_inp_temp_init(int8_t temp);
void dsp_inp_errpls_init(uint8_t pls);
void dsp_inp_condensor_init(uint8_t conds);
void dsp_inp_commen_init(uint8_t commen);
void dsp_inp_scurr_hold_init(uint16_t _hold);
void dsp_inp_scurr_n1_init(uint8_t _cnt);
void dsp_inp_scurr_dur1_init(uint16_t _dur1);
void dsp_inp_scurr_dur2_init(uint16_t _dur2);
void dsp_inp_contract_month_init(uint32_t _contract);
void dsp_inp_end_init(void);
uint8_t get_dispinp_year(void);
uint8_t get_dispinp_month(void);
uint8_t get_dispinp_date(void);
uint8_t get_dispinp_hour(void);
uint8_t get_dispinp_min(void);
uint16_t get_dispinp_sCurr(void);
uint8_t get_dispinp_sCurrCnt(void);
uint8_t get_dispinp_tou_hour(void);
uint8_t get_dispinp_tou_min(void);
uint8_t get_dispinp_ratekind(void);
uint8_t get_dispinp_lpintv(void);
uint8_t get_dispinp_mtdir(void);
meas_method_type get_dispinp_meas(void);
baudrate_type get_dispinp_baud(void);
int8_t get_dispinp_temp(void);
uint8_t get_dispinp_errpls(void);
uint8_t get_dispinp_condensor(void);
uint8_t get_dispinp_commen(void);
uint16_t get_dispinp_scurr_dur(void);
uint8_t get_dispinp_scurr_n1(void);
uint32_t get_dispinp_contract_month(void);
bool dsp_is_suppmode_and_available(void);

extern bool dsp_is_test_reg_mr_date(void);
extern void dsp_test_reg_mr_date_change(void);
extern bool dsp_is_test_tariff_rate(void);
extern void dsp_test_tariff_rate_change(void);
void dsp_test(void);
void dsp_date(date_time_type *dt);
void dsp_time(date_time_type *dt);
void dsp_digit(int32_t val, uint8_t dgt, uint8_t point, bool lead_zero);
void dsm_dsp_digit_update(void);

void dsp_enter_lpm(void);
void dsp_low_pwr_entry_state(uint32_t dsp_idx);
void dsp_debug_state(uint32_t dsp_idx);
void dsp_up_pwr_on_state(void);
void dsp_r_sun_dsp_set(void);
void dsp_on_sun_dsp_set(void);

#endif
