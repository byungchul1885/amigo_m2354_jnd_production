
#ifndef OPTIONS_DEF_H
#define OPTIONS_DEF_H 1

/*
[200417_spec]
변성기부 계량기 1P2W 5(2.5)A 51
                3P4W 5(2.5)A 52
단독계량기
                 1P2W 60(5)A 53
               1P2W 120(10)A 54
               3P4W 120(10)A 55

고유정보
-사용자 미터 ID
-제조자 미터 ID
-DMS(NMS) 식별용 ID
-프로그램 ID
-COSEM 계량기 식별자
*/

/* bccho, 2024-09-05, 삼상 */
#define SINGLE_PHASE (1)
#define THREE_PHASE (3)

#if METER_TYPE == MT_1P2W_60A
#define METER_ID \
    53  // 계기 형식 번호 - 1P2W(단독계기:단상2선식), 220V,
        // Maximum:60A(Default:5A).
#define PHASE_NUM SINGLE_PHASE
#elif METER_TYPE == MT_1P2W_120A
#define METER_ID \
    54  // 계기 형식 번호 - 1P2W(단독계기:단상2선식), 220V,
        // Maximum:120A(Default:10A).
#define PHASE_NUM SINGLE_PHASE
#elif METER_TYPE == MT_3P4W_SINGLE
#define METER_ID \
    55  // 계기 형식 번호 - 3P4W(단독계기:3상4선식), 220V,
        // Maximum:120A(Default:10A).
#define PHASE_NUM (THREE_PHASE)
#endif

#define CE_VER_A03 0
#define CE_VER_A05_KEPCO 1   // ce66a05a version
#define CE_VER_A05B_KEPCO 2  // ce66a05b version (14.10.11)
#define CE_VER_A05E_KEPCO 3  // ce66a05e version (14.10.27)
#define CE_VER_TYPE CE_VER_A05E_KEPCO

#if defined(FEATURE_SEC)
#define DEFAULT_LP_INT_VAL 15
#else
#define DEFAULT_LP_INT_VAL 60
#endif

#if defined(FEATURE_SEC)

#if defined(FEATURE_SPEC_2021_0430_MODIFY)  // 2021_0430 spec 반영 1초 default
#define DEFAULT_RT_LP_INT_VAL 1             // unit sec
#else
#define DEFAULT_RT_LP_INT_VAL 60  // unit sec
#endif

#endif

//====
#define CE_A05E_UNITED
#define BATCHK_METHOD BATCHK_DEF
//====
#define FW_V402_SUPPORT
#define SILICON_B2_SUPPORT
#define MAGNET_DET_BY_ADC
#define TEMPCNTL_PER_SET 0x08
#define TEMPCNTL_PER_SET_SLP 0x0F
#define CAL_RANGE_AVAILABLE
#define LP_INTV_PF_SUPPORT
#define ROLLING_DM_SUPPORT
#define TS_CTRL_NOT_WITH_TOU
#define SAP_PRIVATE_SUPPORT
#define LP_SAVE_MANUALLY
#define LPAVG_SAVE_MANUALLY
#define SW_TIMER_DISABLE
#define KEY_READ_IN_TIMER
#define RMS_MON_SUPPORT
// #define RELAY_AGING_TEST
// #define SCURR_TEST
// #define EXTENDED_MEAS_SUPPORT
#define EXTENDED_sCURR_SUPPORT
#define ADVANCE_E_NEW
//====

#endif /* OPTIONS_DEF_H */
