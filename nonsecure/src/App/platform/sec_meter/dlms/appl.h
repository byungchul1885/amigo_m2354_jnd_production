
#ifndef APPL_H
#define APPL_H 1

#include "whm.h"
#include "options_sel.h"

extern uint8_t g_sap_assin_run;

typedef enum
{
    SAP_PUBLIC,
    SAP_UTILITY,
    SAP_485COMM,
    SAP_PRIVATE,
    SAP_SEC_UTILITY,
    SAP_SEC_SITE
} appl_sap_type;

#define SAP_ASSIGN_MNT_NUM 2
typedef enum
{
    SAP_ASSIGN_DEV_MANAGEMENT = 0x01,
    SAP_ASSIGN_KEPCO_MANAGEMENT = 0x12
} appl_sap_assign_type;

typedef enum
{
    APPL_IDLE_STATE,
    APPL_ASSOCIATED_STATE,
    APPL_CROSS_AUTH_STATE,
    APPL_KEY_AGREEMENT_STATE,
    APPL_ENC_SIGN_STATE
} appl_state_type;

#define PRIV_PWD_SIZE 8
extern const /*__code*/ U8 priv_pwd_default[PRIV_PWD_SIZE];

#define APPL_RESP_HEADER_IDX 3
#define APPL_RESP_DATA_IDX (APPL_RESP_HEADER_IDX + 3)
#define APPL_FILL_RESP_DATA_IDX_NORMAL \
    (APPL_RESP_DATA_IDX + 1)  // next of result option field
#define APPL_FILL_RESP_DATA_IDX_BLOCK \
    (APPL_FILL_RESP_DATA_IDX_NORMAL + 5)  // last(1) + block_num(4)
#define APPL_RESP_OPTION_FLAG_IDX 2
#define APPL_FILL_RESP_DATA_OPT_IDX_NORMAL \
    (APPL_FILL_RESP_DATA_IDX_NORMAL + APPL_RESP_OPTION_FLAG_IDX)

#define TAG_BITSTRING 0x80
#define TAG_APP_CONTEXT 0xa1
#define TAG_CALLED_AP_TITLE 0xa2
#define TAG_CALLED_AE_QUAL 0xa3
#define TAG_CALLED_AP_INVOC 0xa4
#define TAG_CALLED_AE_INVOC 0xa5
#define TAG_CALLING_AP_TITLE 0xa6
#define TAG_CALLING_AE_QUAL 0xa7
#define TAG_CALLING_AP_INVOC 0xa8
#define TAG_CALLING_AE_INVOC 0xa9
#define TAG_ACSE_REQUIRE 0x8a
#define TAG_MECHA_NAME 0x8b
#define TAG_MECHA_VAL 0xac
#define TAG_IMPLEMENT_INFO 0xbd  // unsolved : need exact value
#define TAG_USER_INFO 0xbe
#define TAG_DLMS_INITREQ 0x01

#define TAG_AARQ 0x60
#define TAG_AARE 0x61
#define TAG_GET_REQ 192  // 0xc0
#define TAG_SET_REQ 193
#define TAG_EVT_REQ 194  // 0xc2
#define TAG_ACT_REQ 195
#define TAG_GET_RES 196  // 0xc4
#define TAG_SET_RES 197
#define TAG_ACT_RES 199                   // 0xc7
#define TAG_GLO_GET_REQ_with_CIPHER 0xC8  // glo GET-Request with ciphering
#define TAG_GLO_SET_REQ_with_CIPHER 0xC9  // glo SET-Request with ciphering
#define TAG_GLO_EVENT_NOTI_REQ_with_CIPHER \
    0xCA  // glo EVENT-NOTIFICATION-Request with ciphering
#define TAG_GLO_ACTION_REQ_with_CIPHER \
    0xCB                                  // glo ACTION-Request with ciphering
#define TAG_GLO_GET_RSP_with_CIPHER 0xCC  // glo GET-Response with ciphering
#define TAG_GLO_SET_RSP_with_CIPHER 0xCD  // glo SET-Response with ciphering
#define TAG_GLO_ACTION_RSP_with_CIPHER \
    0xCF                             // glo ACTION-Response with ciphering
#define TAG_EXCEPTION_RSP 0xD8       //
#define TAG_GENERAL_GLO_CIPHER 0xDB  // general glo-ciphering
#define TAG_GENERAL_SIGNING 0xDF     // general-signing
#define TAG_DATA_NOTI 0x0F

#define TAG_CONFIRMED_SVCERR 14

// CONFORMANCE_1 => reserved or for SN
#define GEN_PRT 0x40  // GENERAL PROTECTION service

// CONFORMANCE_2 for LN
#define ATT0_SET 0x80  // attribute_0 referencing with SET
#define PRI_MGM 0x40   // priority management
#define ATT0_GET 0x20  // attribute_0 referencing with GET
#define BLK_GET 0x10   // block transfer with GET
#define BLK_SET 0x08   // block transfer with SET
#define BLK_ACT 0x04   // block transfer with ACTION
#define MUL_REF 0x02   // multiple reference
// CONFORMANCE_3 for LN
#define DAT_NOT 0x80  // DATA NOTIFICATION service
#define GET_SVC 0x10  // GET service
#define SET_SVC 0x08  // SET service
#define SEL_SVC 0x04  // SELective access service
#define EVT_NOT 0x02  // EVENT NOTification service
#define ACT_SVC 0x01  // ACTion service

#define LN_SUPPORTED_CONFORMANCE_1 0x00
#define LN_SUPPORTED_CONFORMANCE_2 0x00  //(BLK_GET)
#define LN_SUPPORTED_CONFORMANCE_3 (GET_SVC)

#define ASS3_LN_SUPPORTED_CONFORMANCE_1 (GEN_PRT)  // 0x00
#define ASS3_LN_SUPPORTED_CONFORMANCE_2 (BLK_GET | BLK_SET)
#define ASS3_LN_SUPPORTED_CONFORMANCE_3 \
    (DAT_NOT | GET_SVC | SET_SVC | ACT_SVC | SEL_SVC)

// CONFORMANCE_1/2/3 for only SN
#define SN_SUPPORTED_CONFORMANCE_1 0x1c  // read, write, unconfirmed-write
#define SN_SUPPORTED_CONFORMANCE_2 0x01  // information-report
#define SN_SUPPORTED_CONFORMANCE_3 0x20  // parameterized-access

// ctt 3.1
#define MAX_RXPDU_SIZE \
    0x3ab3  // sync with static const /*__code*/ uint8_t aare[AARE_SIZE]
#define MAX_TXPDU_SIZE 0x3ab3

// ---------- request and response and result -----------------

#define RESULT_OK 0

typedef enum
{
    APPL_RESULT_OK = RESULT_OK,
    APPL_RESULT_REQ_TYPE_UNMAT = 100,
    APPL_RESULT_OBJ_UNDEF = 101,
    APPL_RESULT_NO_ACS = 102
} appl_result_type;

typedef enum
{
    GET_RESULT_OK = RESULT_OK,
    GET_RESULT_OBJ_UNDEF,
    GET_RESULT_OBJ_UNAVAIL,
    GET_RESULT_TYPE_UNMAT,
    GET_RESULT_LONG_GET_ABORT,
    GET_RESULT_NO_LONG_GET,
    GET_RESULT_BLOCK_NEXT_ERR,
    GET_RESULT_BLOCK_APDU_ERR,
    GET_RESULT_OTHER_REASON
} get_req_result_type;

typedef enum
{
    SET_RESULT_OK = RESULT_OK,
    SET_RESULT_DATA_NG,
    SET_RESULT_REQ_NG,
    SET_RESULT_BLOCK_NO_UNMATCH,
    SET_RESULT_TYPE_UNMAT
} set_req_result_type;

typedef enum
{
    ACT_RESULT_OK = RESULT_OK,
    ACT_RESULT_DATA_NG,
    ACT_RESULT_TYPE_UNMAT,
    ACT_RESULT_OTHER,
    ACT_RESULT_PRE_SEC_MUTUAL_AUTH_NG,
    ACT_RESULT_PRE_SEC_KEYAGREEMENT_NG
} act_req_result_type;

typedef enum
{
    APPL_NULL_REQ = 0,
    APPL_AA_REQ = TAG_AARQ,
    APPL_GET_REQ = TAG_GET_REQ,
    APPL_SET_REQ = TAG_SET_REQ,
    APPL_ACT_REQ = TAG_ACT_REQ,
    APPL_EVT_REQ = TAG_EVT_REQ,
    APPL_GLO_GET_REQ = TAG_GLO_GET_REQ_with_CIPHER,
    APPL_GLO_SET_REQ = TAG_GLO_SET_REQ_with_CIPHER,
    APPL_GLO_EVTNOTI_REQ = TAG_GLO_EVENT_NOTI_REQ_with_CIPHER,
    APPL_GLO_ACT_REQ = TAG_GLO_ACTION_REQ_with_CIPHER,
    APPL_GLO_GET_RSP = TAG_GLO_GET_RSP_with_CIPHER,
    APPL_GLO_SET_RSP = TAG_GLO_SET_RSP_with_CIPHER,
    APPL_GLO_ACT_RSP = TAG_GLO_ACTION_RSP_with_CIPHER,
    APPL_EXEPTION_RSP = TAG_EXCEPTION_RSP,
    APPL_GENERAL_GLO_DATANOTI = TAG_GENERAL_GLO_CIPHER,
    APPL_GENERAL_SIGNING = TAG_GENERAL_SIGNING,
} appl_req_type;

// get request choice
#define GET_REQ_NORMAL 1
#define GET_REQ_NEXT 2
#define GET_REQ_LIST 3
// get response choice
#define GET_RES_NORMAL 1
#define GET_RES_BLOCK 2
#define GET_RES_LIST 3
// set request choice
#define SET_REQ_NORMAL 1
#define SET_REQ_BLOCK_FIRST 2
#define SET_REQ_BLOCK 3
#define SET_REQ_LIST 4
#define SET_REQ_LIST_FIRST_BLOCK 5
// set response choice
#define SET_RES_NORMAL 1
#define SET_RES_BLOCK 2
#define SET_RES_BLOCK_LAST 3
#define SET_RES_BLOCK_LAST_LIST 4
#define SET_RES_LIST 5
// set request choice
#define ACT_REQ_NORMAL 1
#define ACT_REQ_NEXT_PBLOCK 2
#define ACT_REQ_LIST 3
#define ACT_REQ_FIRST_PBLOCK 4
#define ACT_REQ_LIST_FIRST_PBLOCK 5
#define ACT_REQ_PBLOCK 6
// action response choice
#define ACT_RES_NORMAL 1
#define ACT_RES_PBLOCK 2
#define ACT_RES_LIST 3
#define ACT_RES_NEXT_PBLOCK 4

typedef struct
{
    uint8_t acse_req;
    auth_val_type auth_val;
} appl_auth_type;
typedef struct
{
    uint8_t acse_req;
    auth_val_asso3_type auth_val;
} appl_auth_type_asso3;

#define ToH16(a, b)      \
    (a)->c[HI] = (b)[0]; \
    (a)->c[LO] = (b)[1]
#define ToH32(a, b)         \
    (a)->c[HI_HI] = (b)[0]; \
    (a)->c[HI_LO] = (b)[1]; \
    (a)->c[LO_HI] = (b)[2]; \
    (a)->c[LO_LO] = (b)[3]
#define ToHFloat(a, b)      \
    (a)->c[HI_HI] = (b)[0]; \
    (a)->c[HI_LO] = (b)[1]; \
    (a)->c[LO_HI] = (b)[2]; \
    (a)->c[LO_LO] = (b)[3]

#define ToComm16(a, b)   \
    (a)[0] = (b)->c[HI]; \
    (a)[1] = (b)->c[LO]
#define ToComm32(a, b)      \
    (a)[0] = (b)->c[HI_HI]; \
    (a)[1] = (b)->c[HI_LO]; \
    (a)[2] = (b)->c[LO_HI]; \
    (a)[3] = (b)->c[LO_LO]
#define ToCommFloat(a, b)   \
    (a)[0] = (b)->c[HI_HI]; \
    (a)[1] = (b)->c[HI_LO]; \
    (a)[2] = (b)->c[LO_HI]; \
    (a)[3] = (b)->c[LO_LO]

#define UNSPECIFIED_CHAR 0xff  // used in date/time

// ------------- obis-related definition -----------------

// class id
#define CLS_DATA 0x01     // Data class
#define CLS_Reg 0x03      // Register class
#define CLS_XReg 0x04     // Extended Register class
#define CLS_DReg 0x05     // Demand Register class
#define CLS_RegAct 0x06   // Register Activation class (Not Used)
#define CLS_ProfG 0x07    // Profile Generic class
#define CLS_CLOCK 0x08    // Clock class
#define CLS_ScptT 0x09    // Script Table class
#define CLS_Schdl 0x0a    // Schedule class (Not Used)
#define CLS_SpDayT 0x0b   // Special Days Table class
#define CLS_AssSN 0x0c    // Association SN class (Not Used)
#define CLS_AssLN 0x0f    // Association LN class
#define CLS_SapAsg 0x11   // SAP Assignment class
#define CLS_ImgTr 0x12    // Image Transfer class
#define CLS_LoPort 0x13   // IEC local port setup (OPTIC) class
#define CLS_ActCal 0x14   // Activity Calendar class
#define CLS_RegM 0x15     // Register Monitor class (Not Used)
#define CLS_SglActS 0x16  // Single action schedule class
#define CLS_HdlcS 0x17    // IEC HDLC setup class
#define CLS_UtlT 0x1a     // Utility tables class (Not Used)
#define CLS_PstnM 0x1b    // PSTN modem configuration class
#define CLS_PstnA 0x1c    // PSTN auto answer class
#define CLS_PushSetUp 0x28
#define CLS_SecSetUp 0x40

typedef enum OBIS_GROUP_t
{
    GROUP_A,
    GROUP_B,
    GROUP_C,
    GROUP_D,
    GROUP_E,
    GROUP_F,
    OBIS_ID_SIZE
} obis_group_type;

typedef struct
{
    uint8_t id[OBIS_ID_SIZE];
} obis_type;

#define SCRIPT_TABLE_SVC_WRITE_ATT 1
#define SCRIPT_TABLE_SVC_ID_EXE_ATT 2

#define PUSH_SND_TP_SVC_HDLC 5
#define PUSH_SND_DESTINATION_ASSO_3 0x12
#define PUSH_TP_MSG_A_XDR_EN_xDLMS_APDU 0

#define obis_ga appl_obis.id[GROUP_A]
#define obis_gb appl_obis.id[GROUP_B]
#define obis_gc appl_obis.id[GROUP_C]
#define obis_gd appl_obis.id[GROUP_D]
#define obis_ge appl_obis.id[GROUP_E]
#define obis_gf appl_obis.id[GROUP_F]

#define NUM_MYOBJ_G 128
#define NUM_TOTAL_MYOBJ_G 226

#define _BWD_n 1
#define _BWD_n1 \
    32  // OBIS_CUM_DEMAND, OBIS_MAX_DEMAND, OBIS_MONTH_SUBLOCKS,
        // OBIS_CURR_LAST_DEMAND
#define _MEAS_n 0
#define _MEAS_n1 0
#define _sCURR_n 6
#define _MIN_MAX_n 4
#define _ETC_n 2
#define _Tou_n 2

#define NUM_MYOBJ                                                           \
    (NUM_MYOBJ_G + _BWD_n + _MEAS_n + _sCURR_n + _MIN_MAX_n + _ETC_n - 19 + \
     _Tou_n + 2) /* 128 + 1 + 10 + 6 + 4 + 2 - 19 + 2 = 134 */

/***********************************/
/*
security setup 3/4(2)       //2
sec_log(3)                  //5
sap_assign(1)               //6
realtime LP(2->3)           //9
////prepaid_rate_system(1)      //9
////PHASE_DETECT_SIG(1)         //10
////TIMECHG_PERMIT_BOUND(1)     //11
TOU_IMAGE_TRANSFER(1)       //10
SELF_ERR_REF_VAL(1)         //11
CT_RATIO(1)                 //12
PT_RATIO(1)                 //13
PUL_SEL_REACT_APP(1)        //14
METERING_TYPE_SEL(1)        //15
EVENT_INFO(1)               //16
SOFTWARE_INFO(1)            //17
SOFTWARE_APPLY_DATE(1)      //18
SOFTWARE_IMAGE_TRANSFER(1)  //19
MODEM_INFO(1)               //20
NMS_DMS_ID(1)               //21
OBJ_AMR_DATA_NUM(1)         //22
OBJ_MONTH_ENERGY_DELI_nPRD  //23
OBJ_MONTH_ENERGY_RECEI_nPRD //24
OBJ_MONTH_MAXDM_DELI_nPRD   //25
OBJ_MONTH_MAXDM_RECEI_nPRDN //26
OBJ_MONTH_ENERGY_DELI_SEASON //27
OBJ_MONTH_ENERGY_RECEI_SEASON//28
OBJ_MONTH_MAXDM_DELI_SEASON  //29
OBJ_MONTH_MAXDM_RECEI_SEASON //30
OBJ_ENERGY_nPRD              //31
OBJ_ENERGY_SEASON            //32
OBJ_MAX_DEMAND_nPRD          //33
OBJ_MAX_DEMAND_SEASON        //34
OBJ_AVGPF_DELI_nPRD          //35
OBJ_AVGPF_RECEI_nPRD         //36
OBJ_AVGPF_DELI_SEASON        //37
OBJ_AVGPF_RECEI_SEASON       //38
OBJ_ERR_CODE_4               //39
OBJ_ERR_DIAGONIST_NUM        //40
OBJ_PREPAY_ENABLE            //41
OBJ_PHASE_DET(2)             //43
OBJ_PHASE_DET_RESULT_VAL
OBJ_PERMITxx_TIME_LIMIT      //44

OBJ_INT_MODEM_ATCMD          //45
OBJ_EXT_MODEM_ATCMD          //46
OBJ_INT_MODEM_ATCMD_RSP      //47
OBJ_EXT_MODEM_ATCMD_RSP      //48

OBJ_PUSH_ACT_ERR_CODE_1~4(4) //50
OBJ_PUSH_SCRIPT_TABLE        //51
OBJ_PUSH_SETUP_ERR_CODE      //52
OBJ_PUSH_SETUP_LAST_LP       //53
OBJ_EXT_MODEM_ID             //54
OBJ_STOCK_OP_TIMES           //55
OBJ_OLD_METER_TOU_TRANSFER   //56
OBJ_LP_TOTAL_CNT             //57
*/

/*
#if defined(FEATURE_JP_AMR)
OBJ_HOLIDAY_SEL			//1
    #if defined(FEATURE_JP_HIVOLT_METER)
OBJ_THD_PERIOD_SEL			//2
    #endif
#endif

#if defined(FEATURE_JP_SW_UP_CNT)
,OBJ_SW_UP_CNT_0
,OBJ_SW_UP_CNT_1
,OBJ_SW_UP_CNT_2
,OBJ_SW_UP_CNT_3



#endif

OBJ_HDLC_OVERLAP_AVOID  //58
OBJ_PREPAY_LOADLIMIT_CANCEL //62
#if defined(FEATURE_ENERGY_DM_NON_REPUDIATION)
    ,OBJ_MAX_DEMAND__SIGN
    ,OBJ_MAX_DEMAND_nPRD__SIGN
    ,OBJ_MAX_DEMAND_SEASON__SIGN
    ,OBJ_ENERGY__SIGN
    ,OBJ_ENERGY_nPRD__SIGN
    ,OBJ_ENERGY_SEASON__SIGN
#endif

*/

#define JP_AMR_ADD 10  // 2

/* bccho, 2024-09-05, 삼상 */
#define PHASE3_DATA_ADD 20
#define EXTENDED_MEAS_SUB 10

#define NUM_ADD_SEC_OBJ_NUM (62 + 6)

/************************************/
#if 1 /* bccho, 2024-09-05, 삼상 */
#define NUM_MYOBJ_SEC                                                       \
    (NUM_MYOBJ_G + _BWD_n + _MEAS_n + _sCURR_n + _MIN_MAX_n + _ETC_n - 19 + \
     _Tou_n + NUM_ADD_SEC_OBJ_NUM + JP_AMR_ADD + PHASE3_DATA_ADD + 4 +      \
     2 + 10) /* 128+1+10+6+4+2-19+2+68+10 = 212 */
#else
#define NUM_MYOBJ_SEC                                                       \
    (NUM_MYOBJ_G + _BWD_n + _MEAS_n + _sCURR_n + _MIN_MAX_n + _ETC_n - 19 + \
     _Tou_n + NUM_ADD_SEC_OBJ_NUM +                                         \
     JP_AMR_ADD) /* 128+1+10+6+4+2-19+2+68+10 = 212 */
#endif

#define NUM_TOTAL_MYOBJ (0x132 - 19 + _Tou_n)
#define NUM_TOTAL_MYOBJ_SEC \
    (0x132 - 19 + _Tou_n + NUM_ADD_SEC_OBJ_NUM + JP_AMR_ADD)

#define NUM_MYOBJ_485COMM 3
#define NUM_MYOBJ_PRIVATE 12

#define NUM_MYOBJ_DEV_MANAGEMENT 10

typedef enum
{
    OBJ_NULL,
    OBJ_ASSOCIATION_LN, /* 현재 association */
    OBJ_MTINIT_LOG,
    OBJ_MTINIT_NUM,
    OBJ_DATE_TIME,     /* 일자/시간 설정 및 읽기 */
    OBJ_DEVICE_CMD,    /* 계기 운용 주요 명령 */
    OBJ_DEVICE_BCMD,   /* 계기 운용 기본 명령 */
    OBJ_ENDOF_BILLING, /* 검침 (end of billing period) */
    OBJ_TARIFF_SCRIPT, /* TOU : Tariffication Script Table */
    OBJ_HOLIDAYS,      /* 정기, 비정기 휴일 */
    OBJ_TOU_CAL,
    /* TOU : Activity Calendar */  // 10
    OBJ_PERIOD_BILLDATE,           /* 정기 검침일 */
    OBJ_NPERIOD_BILLDATE,          /* 비정기 검침일 */
    OBJ_HDLC_SETUP,
    OBJ_DEVICE_ID, /* COSEM 계기 식별자, LDN */
    OBJ_PGM_CHG_NUM,
    OBJ_LAST_PGM_CHG,
    OBJ_FUTURE_PGM_CHG,
    OBJ_OUT_SIG_SEL, /* 부가신호 장치 출력 선택 */
    OBJ_PWR_FAIL_NUM,
#if 1  // jp.kim 24.10.28
    OBJ_Working_PWR_FAIL_NUM,
    OBJ_Working_PWR_FAIL_LOG,
#endif
    OBJ_BAT_USE_TIME,  // 20
    OBJ_BAT_INSTALL_TIME,
    OBJ_USER_DISP_MODE,   /* 디스플레이 남품 시 순환표시 모드 */
    OBJ_SUPPLY_DISP_MODE, /* 디스플레이 관리자 순환표시 모드 */
    OBJ_PVT_DISP_MODE,    /* 무부하 시 부하 동작 표시 설정 */
    OBJ_MONTH_ENERGY_DELI, /* 전력량 (월별) - 정기 검침 자료, 수전 전력량, 검침
                              종류 */
    OBJ_MONTH_MAXDM_DELI, /* 최대수요전력 (월별) - 정기 검침 자료, 수전
                             최대수요전력, 검침 종류 */
    OBJ_MONTH_ENERGY_RECEI, /* 전력량 (월별) - 정기 검침 자료, 송전 전력량, 검침
                               종류 */
    OBJ_MONTH_MAXDM_RECEI, /* 최대수요전력 (월별) - 정기 검침 자료, 송전
                              최대수요전력, 검침 종류 */
    OBJ_MONTH_SUBLOCKS,
    OBJ_CUSTOM_ID,
    /* 계기 시리얼 번호 */  // 30
    OBJ_MANUFACT_ID,        /* 계기 제조사 번호 */
    OBJ_COUNTER_BILLING,
    OBJ_NUM_AVAIL_BILLING,
    OBJ_TIME_BILLING,
    OBJ_PGM_ID,     /* TOU : 프로그램 ID */
    OBJ_LOCAL_TIME, /* 디스플레이용 일자/시간 : 시간 time */
    OBJ_LOCAL_DATE, /* 디스플레이용 일자/시간 : 일자 date */
    OBJ_MAGNET_DURTIME,
    OBJ_CURR_TARIFF,
    OBJ_LCD_PARM,      // 40
    OBJ_LCDSET_PARM,   /* 계량 모드 설정 파라미터 */
    OBJ_BILLING_PARM,  /* 검침 파라미터 */
    OBJ_SELECTIVE_ACT, /* 선택 유효전력량 개별설정 */
    OBJ_rLOAD_SIG,     /* 원격부하 개폐 상태 */
    OBJ_rLOAD_CTRL,    /* 원격부하 개폐 실행 */
    OBJ_LOAD_PROFILE,  /* Load Profile */
    OBJ_LPAVG,
    OBJ_MTCONST_ACTIVE,   /* 계기 정수 : 유효전력량 */
    OBJ_MTCONST_REACTIVE, /* 계기 정수 : 무효전력량 */
    OBJ_MTCONST_APP,
    /* 계기 정수 : 피상전력량 */  // 50
    OBJ_LAST15_PF_DELI,
    OBJ_LAST15_PF_RECEI,
    OBJ_CURR_LAST_DEMAND, /* 현재/직전 수요 시한 수요전력 */
    OBJ_AVGPF_DELI,
    OBJ_AVGPF_RECEI,
    OBJ_ERR_CODE_1, /* 자기진단 항목 : 자기진단 1 */
    OBJ_ERR_CODE_2, /* 자기진단 항목 : 자기진단 2 */
    OBJ_ERR_CODE_3, /* 자기진단 항목 : 자기진단 3 */
    OBJ_LP_STATUS,
    OBJ_SAGEVT_LOG,
    /* Sag/Swell(이력) : Sag 발생 */  // 60
    OBJ_SWELLEVT_LOG,                 /* Sag/Swell(이력) : Swell 발생 */
    OBJ_EVT_LOG,                      /* 이력 기록 */
    OBJ_LP_INTERVAL,                  /* LP 기록 주기 */
    OBJ_LPAVG_INTERVAL,
    OBJ_CUM_DEMAND,
    OBJ_MAX_DEMAND,
    OBJ_ENERGY,
    OBJ_ENERGY_FWD_BOTH_REACT,
    OBJ_ENERGY_BWD_BOTH_REACT,
    OBJ_RTC_CHG_NUM,  // 70
    OBJ_aDR_NUM,
    OBJ_mDR_NUM,
    OBJ_SR_NUM,
    OBJ_sCURR_nonSEL_NUM,
    OBJ_sCURR_autoRTN_VAL,
    OBJ_sCURR_LIMIT_NUM,
    OBJ_sCURR_LIMIT_VAL,
    OBJ_SAG_VAL_SET,
    OBJ_SAG_CNT,
    OBJ_SAG_TIME_SET,  // 80
    OBJ_SWELL_VAL_SET,
    OBJ_SWELL_CNT,
    OBJ_SWELL_TIME_SET,
    OBJ_COVER_OPEN_NUM,
    OBJ_TCOVER_OPEN_NUM,
    OBJ_MAGNET_DET_NUM,
    OBJ_TEMP_THRSHLD, /* 온도 Threshold 설정 */
    OBJ_TEMP_OVER,    /* 온도 초과 */
    OBJ_rLOAD_NUM,
    OBJ_INST_FREQ,  // 90
    OBJ_INST_PROFILE,
    OBJ_INST_CURR_L1,
    OBJ_INST_voltTHD_L1, /* 순시 전압 THD : A상 */
    OBJ_INST_VOLT_L1,    /* 순시 전압 : A상 */
    OBJ_INST_PF_L1,
    OBJ_INST_CURR_L2,
    OBJ_INST_voltTHD_L2, /* 순시 전압 THD : B상 */
    OBJ_INST_VOLT_L2,    /* 순시 전압 : B상 */
    OBJ_INST_PF_L2,
    OBJ_INST_CURR_L3,    // 100
    OBJ_INST_voltTHD_L3, /* 순시 전압 THD : C상 */
    OBJ_INST_VOLT_L3,    /* 순시 전압 : C상 */
    OBJ_INST_PF_L3,
    OBJ_INST_PHASE_U12,
    OBJ_INST_PHASE_U13,
    OBJ_INST_PHASE_L1,
    OBJ_INST_PHASE_L2,
    OBJ_INST_PHASE_L3,
    OBJ_INST_POWER,
    OBJ_AVG_CURR_L1,  // 110
    OBJ_AVG_CURR_L2,
    OBJ_AVG_CURR_L3,
    OBJ_AVG_VOLT_L1,
    OBJ_AVG_VOLT_L2,
    OBJ_AVG_VOLT_L3,
    OBJ_IMAXLOAD_L1, /* 최대부하전류 : A상 */
    OBJ_IMAXLOAD_L2, /* 최대부하전류 : B상 */
    OBJ_IMAXLOAD_L3, /* 최대부하전류 : C상 */
    OBJ_REM_ENERGY,
    OBJ_REM_MONEY,  // 120
    OBJ_REM_TIME,
    OBJ_BUY_ENERGY,
    OBJ_BUY_MONEY,
    OBJ_TS_CONF,
    OBJ_TMP_HOLIDAYS,
    OBJ_CURR_TEMP, /* 현재 온도 */
    OBJ_SEL_REACT,
    OBJ_LP_OVERLAPED_INDEX, /* 총 LP 발생 횟수 */
    OBJ_CONDENSOR_INST,
    OBJ_TS_SCRIPT_TABLE,  // 130
    OBJ_COMM_ENABLE,
    OBJ_WRONG_CONN_NUM,
    OBJ_KEY_VALUE,
    OBJ_LCD_MAP,
    OBJ_sCURR_LIMIT2_VAL,
    OBJ_sCURR_HOLD,
    OBJ_sCURR_RECOVER_N1,
    OBJ_sCURR_RECOVER_N2,
    OBJ_sCURR_COUNTER_N1,
    OBJ_LATCHON_COUNTER  // 150
    ,
    OBJ_MIN_INST_FREQ,
    OBJ_MAX_INST_FREQ,
#if 1 /* bccho, 2024-09-05, 삼상 */
    OBJ_MIN_INST_VOLT_L1,
    OBJ_MAX_INST_VOLT_L1,
    OBJ_MIN_INST_VOLT_L2,
    OBJ_MAX_INST_VOLT_L2,
    OBJ_MIN_INST_VOLT_L3,
    OBJ_MAX_INST_VOLT_L3,
#else
    OBJ_MIN_INST_VOLT,
    OBJ_MAX_INST_VOLT,
#endif
    OBJ_OVERCURR_ENABLE,
    OBJ_ADJ_FACTOR,
    OBJ_TOU_SET_CNT,
    OBJ_EXT_PROG_ID,
    OBJ_SECURITY_SETUP_3,
    OBJ_SECURITY_SETUP_4  // 160
    ,
    OBJ_SEC_EVT_LOG_HISTORY,
    OBJ_SEC_EVT_LOG_NUM,
    OBJ_SEC_EVT_LOG_CASE,
    OBJ_SAP_ASSIGNM /* SAP Assignment */
    ,
    OBJ_RTIME_P_ENERGY,
    OBJ_RTIME_P_LP,
    OBJ_RTIME_P_LP_INTERVAL,
    OBJ_TOU_IMAGE_TRANSFER /* TOU : Image Transfer */
    ,
    OBJ_SELF_ERR_REF_VAL,
    OBJ_CT_RATIO  // 170
    ,
    OBJ_PT_RATIO,
    OBJ_PUL_SEL_REACT_APP,
    OBJ_METERING_TYPE_SEL,
    OBJ_EVENT_INFO,
    OBJ_SW_INFO /* 현재 소프트웨어 정보 */
    ,
    OBJ_SW_APPLY_DATE,
    OBJ_SW_IMAGE_TRANSFER /* S/W Update : Image Transfer */
    ,
    OBJ_RUN_MODEM_INFO,
    OBJ_NMS_DMS_ID,
    OBJ_USE_AMR_DATA_NUM /* 사용 가능한 검침 자료 개수 */  // 180

    ,
    OBJ_MONTH_ENERGY_DELI_nPRD /* 전력량 (월별) - 비정기 검침 자료, 수전 전력량,
                                  검침 종류 */
    ,
    OBJ_MONTH_ENERGY_RECEI_nPRD /* 전력량 (월별) - 비정기 검침 자료, 송전
                                   전력량, 검침 종류 */
    ,
    OBJ_MONTH_MAXDM_DELI_nPRD /* 최대수요전력 (월별) - 비정기 검침 자료, 수전
                                 최대수요전력, 검침 종류 */
    ,
    OBJ_MONTH_MAXDM_RECEI_nPRD /* 최대수요전력 (월별) - 비정기 검침 자료, 송전
                                  최대수요전력, 검침 종류 */
    ,
    OBJ_MONTH_ENERGY_DELI_SEASON /* 전력량 (월별) - 계절 변경 검침 자료, 수전
                                    전력량, 검침 종류 */
    ,
    OBJ_MONTH_ENERGY_RECEI_SEASON /* 전력량 (월별) - 계절 변경 검침 자료, 송전
                                     전력량, 검침 종류 */
    ,
    OBJ_MONTH_MAXDM_DELI_SEASON /* 최대수요전력 (월별) - 계절 변경 검침 자료,
                                   수전 최대수요전력, 검침 종류 */
    ,
    OBJ_MONTH_MAXDM_RECEI_SEASON /* 최대수요전력 (월별) - 계절 변경 검침 자료,
                                    송전 최대수요전력, 검침 종류 */

    ,
    OBJ_ENERGY_nPRD,
    OBJ_ENERGY_SEASON  // 190
    ,
    OBJ_MAX_DEMAND_nPRD,
    OBJ_MAX_DEMAND_SEASON

    ,
    OBJ_AVGPF_DELI_nPRD,
    OBJ_AVGPF_RECEI_nPRD,
    OBJ_AVGPF_DELI_SEASON,
    OBJ_AVGPF_RECEI_SEASON

    ,
    OBJ_ERR_CODE_4 /* 자기진단 항목 : 자기진단 4 */
    ,
    OBJ_ERR_DIAGONIST_NUM,
    OBJ_PREPAY_ENABLE

    ,
    OBJ_PHASE_DET_CONT_TIME  // 200
    ,
    OBJ_PHASE_DET_CORRECT_VAL,
    OBJ_PHASE_DET_RESULT_VAL,
    OBJ_PERMITxx_TIME_LIMIT

    ,
    OBJ_INT_PLC_MODEM_ATCMD  // jp.kim 25.01.20
    ,
    OBJ_INT_MODEM_ATCMD,
    OBJ_EXT_MODEM_ATCMD,
    OBJ_INT_PLC_MODEM_ATCMD_RSP,  // jp.kim 25.01.20
    OBJ_INT_MODEM_ATCMD_RSP,
    OBJ_EXT_MODEM_ATCMD_RSP

    ,
    OBJ_PUSH_ACT_ERR_CODE_1 /* 자기진단 항목 PUSH 활성화 : 자기진단 1 PUSH
                               활성화 */
    ,
    OBJ_PUSH_ACT_ERR_CODE_2 /* 자기진단 항목 PUSH 활성화 : 자기진단 2 PUSH
                               활성화 */
    ,
    OBJ_PUSH_ACT_ERR_CODE_3 /* 자기진단 항목 PUSH 활성화 : 자기진단 3 PUSH
                               활성화 */
    ,
    OBJ_PUSH_ACT_ERR_CODE_4 /* 자기진단 항목 PUSH 활성화 : 자기진단 4 PUSH
                               활성화 */
    ,
    OBJ_PUSH_SCRIPT_TABLE /* PUSH script table 정보 */
    ,
    OBJ_PUSH_SETUP_ERR_CODE /* PUSH Setup OBIS 코드 : 자기진단 이벤트 정보 */
    ,
    OBJ_PUSH_SETUP_LAST_LP /* PUSH Setup OBIS 코드 : 최근 발생 LP */

    ,
    OBJ_EXT_MODEM_ID,
    OBJ_STOCK_OP_TIMES,
    OBJ_OLD_METER_TOU_TRANSFER,
    OBJ_LP_TOTAL_CNT,
    OBJ_HOLIDAY_SEL /* 정기/비정기 휴일 적용 */
    ,
    OBJ_THD_PERIOD_SEL

    ,
    OBJ_SW_UP_CNT_0,
    OBJ_SW_UP_CNT_1,
    OBJ_SW_UP_CNT_2,
    OBJ_SW_UP_CNT_3,
    OBJ_SW_UP_LOG_0,
    OBJ_SW_UP_LOG_1,
    OBJ_SW_UP_LOG_2,
    OBJ_SW_UP_LOG_3

    // #if defined(FEATURE_VIRTUAL_HDLC_ADDR)
    ,
    OBJ_HDLC_OVERLAP_AVOID
    // #endif
    ,
    OBJ_PREPAY_LOADLIMIT_CANCEL,
    OBJ_MAX_DEMAND__SIGN,
    OBJ_MAX_DEMAND_nPRD__SIGN,
    OBJ_MAX_DEMAND_SEASON__SIGN,
    OBJ_ENERGY__SIGN,
    OBJ_ENERGY_nPRD__SIGN,
    OBJ_ENERGY_SEASON__SIGN,
    OBJ_WORKING_FAULT_MIN,
    OBJ_TOU_ID_CHANGE_STS,
    OBJ_CAL_ADJ_ACT,   // JP.KIM 24.11.08
    OBJ_SYS_TITLE,     // JP.KIM 24.12.05
    OBJ_INSTALL_CERT,  // bccho, 2024-12-06
    OBJ_INSTALL_KEY    // bccho, 2024-12-06
    ,
    OBJ_AVG_VOLT_L1_L2,
    OBJ_AVG_VOLT_L2_L3,
    OBJ_AVG_VOLT_L3_L1,
    OBJ_INST_POWER_L1,
    OBJ_INST_POWER_L2,
    OBJ_INST_POWER_L3,
    OBJ_CUM_DEMAND_nPRD,
    OBJ_CUM_DEMAND_SEASON,
} obj_id_enum_type;

#define OBIS_ASSOCIATION_LN \
    {0x00, 0x00, 0x28,      \
     0x00, 'e',  0xff} /* 현재 association */  // 2020_0417 apply
#define OBIS_DATE_TIME \
    {0x00, 0x00, 0x01, 0x00, 0x00, 0xff} /* 일자/시간 설정 및 읽기 */
#define OBIS_DATE_TIME_nobr 0x00, 0x00, 0x01, 0x00, 0x00, 0xff
#define OBIS_DEVICE_CMD \
    {0x00, 0x00, 0x0a, 0x00, 0x00, 0xff} /* 계기 운용 주요 명령 */
#define OBIS_DEVICE_BCMD \
    {0x00, 0x00, 0x0a, 0x01, 0x00, 0xff} /* 계기 운용 기본 명령 */
#define OBIS_ENDOF_BILLING \
    {0x00, 0x00, 0x0a, 0x00, 0x01, 0xff} /* 검침 (end of billing period) */
#define OBIS_TARIFF_SCRIPT               \
    {0x00, 0x00, 0x0a, 0x00, 0x64, 0xff} \
    /* TOU : Tariffication Script Table (요금율) */
#define OBIS_rLOAD_CTRL \
    {0x00, 0x00, 0x0a, 0x00, 0x67, 0xff} /* 원격부하 개폐 실행 */
#define OBIS_HOLIDAYS \
    {0x00, 0x00, 0x0b, 0x00, 0x00, 0xff} /* 정기/비정기 휴일 */
#define OBIS_TOU_CAL \
    {0x00, 0x00, 0x0d, 0x00, 0x00, 0xff} /* TOU : Activity Calendar */
#define OBIS_TOU_CAL_nobr 0x00, 0x00, 0x0d, 0x00, 0x00, 0xff
#define OBIS_PERIOD_BILLDATE \
    {0x00, 0x00, 0x0f, 0x00, 0x00, 0xff} /* 정기 검침일 */
#define OBIS_NPERIOD_BILLDATE \
    {0x00, 0x00, 0x0f, 0x01, 0x00, 0xff} /* 비정기 검침일 */
#define OBIS_HDLC_SETUP {0x00, 0x00, 0x16, 0x00, 0x00, 0xff}
#define OBIS_DEVICE_ID \
    {0x00, 0x00, 0x2a, 0x00, 0x00, 0xff} /* COSEM 계기 식별자 (LDN) */
#define OBIS_PGM_CHG_NUM {0x00, 0x00, 0x60, 0x02, 0x00, 0xff}
#define OBIS_PGM_CHG_NUM_nobr 0x00, 0x00, 0x60, 0x02, 0x00, 0xff
#define OBIS_LAST_PGM_CHG {0x00, 0x00, 0x60, 0x02, 0x01, 0xff}
#define OBIS_FUTURE_PGM_CHG {0x00, 0x00, 0x60, 0x02, 0x06, 0xff}
#define OBIS_OUT_SIG_SEL \
    {0x00, 0x00, 0x60, 0x03, 0x02, 0xff} /* 부가신호 장치 출력 선택 */
#define OBIS_BAT_USE_TIME {0x00, 0x00, 0x60, 0x06, 0x00, 0xff}
#define OBIS_BAT_INSTALL_TIME {0x00, 0x00, 0x60, 0x06, 0x05, 0xff}
#define OBIS_PWR_FAIL_NUM {0x00, 0x00, 0x60, 0x07, 0x00, 0xff}
#define OBIS_PWR_FAIL_NUM_nobr 0x00, 0x00, 0x60, 0x07, 0x00, 0xff
#define OBIS_CURR_TEMP {0x00, 0x00, 0x60, 0x09, 0x00, 0xff} /* 현재 온도 */
#define OBIS_CURR_TEMP_nobr 0x00, 0x00, 0x60, 0x09, 0x00, 0xff
#define OBIS_USER_DISP_MODE                                                                \
    {0x00, 0x00, 0x60, 0x32, 0x32, 0xff} /* 디스플레이 남품 시 순환표시 모드 \
                                          */
#define OBIS_SUPPLY_DISP_MODE \
    {0x00, 0x00, 0x60, 0x32, 0x33, 0xff} /* 디스플레이 관리자 순환표시 모드 */
#define OBIS_PVT_DISP_MODE \
    {0x00, 0x00, 0x60, 0x32, 0x34, 0xff} /* 무부하 시 부하 동작 표시 설정 */
#define OBIS_ERR_CODE_1 \
    {0x00, 0x00, 0x61, 0x61, 0x00, 0xff} /* 자기진단 항목 : 자기진단 1 */
#define OBIS_ERR_CODE_1_nobr 0x00, 0x00, 0x61, 0x61, 0x00, 0xff
#define OBIS_ERR_CODE_2 \
    {0x00, 0x00, 0x61, 0x61, 0x01, 0xff} /* 자기진단 항목 : 자기진단 2 */
#define OBIS_ERR_CODE_2_nobr 0x00, 0x00, 0x61, 0x61, 0x01, 0xff
#define OBIS_ERR_CODE_3 \
    {0x00, 0x00, 0x61, 0x61, 0x02, 0xff} /* 자기진단 항목 : 자기진단 3 */
#define OBIS_ERR_CODE_3_nobr 0x00, 0x00, 0x61, 0x61, 0x02, 0xff
#define OBIS_ERR_CODE_4 \
    {0x00, 0x00, 0x61, 0x61, 0x03, 0xff} /* 자기진단 항목 : 자기진단 4 */
#define OBIS_ERR_CODE_4_nobr 0x00, 0x00, 0x61, 0x61, 0x03, 0xff

#define OBIS_PUSH_ACT_ERR_CODE_1         \
    {0x00, 0x00, 0x61, 0x61, 0x05, 0xff} \
    /* 자기진단 항목 PUSH 활성화 : 자기진단 1 PUSH 활성화 */
#define OBIS_PUSH_ACT_ERR_CODE_1_nobr 0x00, 0x00, 0x61, 0x61, 0x05, 0xff
#define OBIS_PUSH_ACT_ERR_CODE_2         \
    {0x00, 0x00, 0x61, 0x61, 0x06, 0xff} \
    /* 자기진단 항목 PUSH 활성화 : 자기진단 2 PUSH 활성화 */
#define OBIS_PUSH_ACT_ERR_CODE_2_nobr 0x00, 0x00, 0x61, 0x61, 0x06, 0xff
#define OBIS_PUSH_ACT_ERR_CODE_3         \
    {0x00, 0x00, 0x61, 0x61, 0x07, 0xff} \
    /* 자기진단 항목 PUSH 활성화 : 자기진단 3 PUSH 활성화 */
#define OBIS_PUSH_ACT_ERR_CODE_3_nobr 0x00, 0x00, 0x61, 0x61, 0x07, 0xff
#define OBIS_PUSH_ACT_ERR_CODE_4         \
    {0x00, 0x00, 0x61, 0x61, 0x08, 0xff} \
    /* 자기진단 항목 PUSH 활성화 : 자기진단 4 PUSH 활성화 */
#define OBIS_PUSH_ACT_ERR_CODE_4_nobr 0x00, 0x00, 0x61, 0x61, 0x08, 0xff
#define OBIS_LP_STATUS {0x00, 0x00, 0x61, 0x61, 0x04, 0xff}
#define OBIS_LP_STATUS_nobr 0x00, 0x00, 0x61, 0x61, 0x04, 0xff
// OBIS : 0 b 98 1 e f
#define OBIS_MONTH_ENERGY_DELI          \
    {0x00, 0x01, 0x62, 0x01, 0x01, 'Z'} \
    /* 전력량 (월별) - 정기 검침 자료, 수전 전력량, 검침 종류 */
#define OBIS_MONTH_ENERGY_DELI_nPRD     \
    {0x00, 0x02, 0x62, 0x01, 0x01, 'Z'} \
    /* 전력량 (월별) - 비정기 검침 자료, 수전 전력량, 검침 종류 */
#define OBIS_MONTH_ENERGY_DELI_SEASON   \
    {0x00, 0x03, 0x62, 0x01, 0x01, 'Z'} \
    /* 전력량 (월별) - 계절 변경 검침 자료, 수전 전력량, 검침 종류 */

#define OBIS_MONTH_MAXDM_DELI           \
    {0x00, 0x01, 0x62, 0x01, 0x02, 'Z'} \
    /* 최대수요전력 (월별) - 정기 검침 자료, 수전 최대수요전력, 검침 종류 */
#define OBIS_MONTH_MAXDM_DELI_nPRD                                                                     \
    {0x00, 0x02, 0x62, 0x01, 0x02, 'Z'}                                                                \
    /* 최대수요전력 (월별) - 비정기 검침 자료, 수전 최대수요전력, 검침 종류 \
     */
#define OBIS_MONTH_MAXDM_DELI_SEASON                                                                \
    {0x00, 0x03, 0x62, 0x01, 0x02, 'Z'}                                                             \
    /* 최대수요전력 (월별) - 계절 변경 검침 자료, 수전 최대수요전력, 검침 \
       종류 */

#define OBIS_MONTH_ENERGY_RECEI         \
    {0x00, 0x01, 0x62, 0x01, 0x03, 'Z'} \
    /* 전력량 (월별) - 정기 검침 자료, 송전 전력량, 검침 종류 */
#define OBIS_MONTH_ENERGY_RECEI_nPRD    \
    {0x00, 0x02, 0x62, 0x01, 0x03, 'Z'} \
    /* 전력량 (월별) - 비정기 검침 자료, 송전 전력량, 검침 종류 */
#define OBIS_MONTH_ENERGY_RECEI_SEASON  \
    {0x00, 0x03, 0x62, 0x01, 0x03, 'Z'} \
    /* 전력량 (월별) - 계절 변경 검침 자료, 송전 전력량, 검침 종류 */

#define OBIS_MONTH_MAXDM_RECEI          \
    {0x00, 0x01, 0x62, 0x01, 0x04, 'Z'} \
    /* 최대수요전력 (월별) - 정기 검침 자료, 송전 최대수요전력, 검침 종류 */
#define OBIS_MONTH_MAXDM_RECEI_nPRD                                                                    \
    {0x00, 0x02, 0x62, 0x01, 0x04, 'Z'}                                                                \
    /* 최대수요전력 (월별) - 비정기 검침 자료, 송전 최대수요전력, 검침 종류 \
     */
#define OBIS_MONTH_MAXDM_RECEI_SEASON                                                               \
    {0x00, 0x03, 0x62, 0x01, 0x04, 'Z'}                                                             \
    /* 최대수요전력 (월별) - 계절 변경 검침 자료, 송전 최대수요전력, 검침 \
       종류 */

#define OBIS_RTC_CHG_NUM {0x00, 0x80, 0x63, 0x62, 0x03, 0xff}
#define OBIS_RTC_CHG_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x03, 0xff
#define OBIS_aDR_NUM {0x00, 0x80, 0x63, 0x62, 0x05, 0xff}
#define OBIS_aDR_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x05, 0xff
#define OBIS_mDR_NUM {0x00, 0x80, 0x63, 0x62, 0x06, 0xff}
#define OBIS_mDR_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x06, 0xff
#define OBIS_SR_NUM {0x00, 0x80, 0x63, 0x62, 0x07, 0xff}
#define OBIS_SR_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x07, 0xff
#define OBIS_COVER_OPEN_NUM {0x00, 0x80, 0x63, 0x62, 0x0a, 0xff}
#define OBIS_COVER_OPEN_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x0a, 0xff
#define OBIS_rLOAD_NUM {0x00, 0x80, 0x63, 0x62, 0x0b, 0xff}
#define OBIS_rLOAD_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x0b, 0xff
#define OBIS_TCOVER_OPEN_NUM {0x00, 0x80, 0x63, 0x62, 0x0e, 0xff}
#define OBIS_TCOVER_OPEN_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x0e, 0xff
#define OBIS_MTINIT_NUM {0x00, 0x80, 0x63, 0x62, 0x0f, 0xff}
#define OBIS_MTINIT_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x0f, 0xff
#define OBIS_WRONG_CONN_NUM {0x00, 0x80, 0x63, 0x62, 0x10, 0xff}
#define OBIS_WRONG_CONN_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x10, 0xff
#define OBIS_ERR_DIAGONIST_NUM {0x00, 0x80, 0x63, 0x62, 0x13, 0xff}
#define OBIS_ERR_DIAGONIST_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x13, 0xff

#define OBIS_CUSTOM_ID \
    {0x01, 0x00, 0x00, 0x00, 'e', 0xff} /* 계기 시리얼 번호 */
#define OBIS_MANUFACT_ID \
    {0x01, 0x00, 0x00, 0x00, 'e', 0xff} /* 계기 제조사 번호 */

#define OBIS_COUNTER_BILLING {0x01, 0x00, 0x00, 0x01, 0x00, 0xff}
#define OBIS_NUM_AVAIL_BILLING {0x01, 0x00, 0x00, 0x01, 0x01, 0xff}
#define OBIS_TIME_BILLING {0x01, 'b', 0x00, 0x01, 0x02, 'Z'}
#define OBIS_PGM_ID                                               \
    {0x01, 0x00, 0x00, 0x02, 0x00, 0xff} /* TOU : 프로그램 ID \
                                          */
#define OBIS_LOCAL_TIME                  \
    {0x01, 0x00, 0x00, 0x09, 0x01, 0xff} \
    /* 디스플레이용 일자/시간 : 시간 time */
#define OBIS_LOCAL_DATE                  \
    {0x01, 0x00, 0x00, 0x09, 0x02, 0xff} \
    /* 디스플레이용 일자/시간 : 일자 date */
#define OBIS_CURR_TARIFF {0x01, 0x00, 0x00, 0xf0, 0x00, 0xff}
#define OBIS_LCD_PARM {0x01, 0x00, 0x00, 0xf1, 0x00, 0xff}
#define OBIS_BILLING_PARM \
    {0x01, 0x00, 0x00, 0xf2, 0x00, 0xff} /* 검침 파라미터 */
#define OBIS_SELECTIVE_ACT \
    {0x01, 0x00, 0x00, 0xf2, 0x01, 0xff} /* 선택 유효전력량 개별설정 */
#define OBIS_LCDSET_PARM \
    {0x01, 0x00, 0x00, 0xf3, 0x00, 0xff} /* 계량 모드 설정 파라미터 */
#define OBIS_rLOAD_SIG \
    {0x01, 0x00, 0x00, 0xf4, 0x00, 0xff} /* 원격부하 개폐 상태 */
#define OBIS_rLOAD_SIG_nobr 0x01, 0x00, 0x00, 0xf4, 0x00, 0xff
#define OBIS_SAG_VAL_SET {0x01, 0x00, 0x0c, 0x1f, 0x00, 0xff}
#define OBIS_SAG_CNT {0x01, 0x00, 0x0c, 0x20, 0x00, 0xff}
#define OBIS_SAG_CNT_nobr 0x01, 0x00, 0x0c, 0x20, 0x00, 0xff
#define OBIS_SAG_TIME_SET {0x01, 0x00, 0x0c, 0x21, 0x00, 0xff}
#define OBIS_SWELL_VAL_SET {0x01, 0x00, 0x0c, 0x23, 0x00, 0xff}
#define OBIS_SWELL_CNT {0x01, 0x00, 0x0c, 0x24, 0x00, 0xff}
#define OBIS_SWELL_CNT_nobr 0x01, 0x00, 0x0c, 0x24, 0x00, 0xff
#define OBIS_SWELL_TIME_SET {0x01, 0x00, 0x0c, 0x25, 0x00, 0xff}
#define OBIS_MIN_INST_FREQ {0x01, 0x00, 0x0e, 0x03, 0x00, 0xff}
#define OBIS_MAX_INST_FREQ {0x01, 0x00, 0x0e, 0x06, 0x00, 0xff}

#define OBIS_INST_POWER {0x01, 0x00, 16, 0x07, 0x00, 0xff}
#define OBIS_INST_POWER_nobr 0x01, 0x00, 16, 0x07, 0x00, 0xff
#define OBIS_INST_POWER_L1 {0x01, 0x00, 36, 0x07, 0x00, 0xff}
#define OBIS_INST_POWER_L1_nobr 0x01, 0x00, 36, 0x07, 0x00, 0xff
#define OBIS_INST_POWER_L2 {0x01, 0x00, 56, 0x07, 0x00, 0xff}
#define OBIS_INST_POWER_L2_nobr 0x01, 0x00, 56, 0x07, 0x00, 0xff
#define OBIS_INST_POWER_L3 {0x01, 0x00, 76, 0x07, 0x00, 0xff}
#define OBIS_INST_POWER_L3_nobr 0x01, 0x00, 76, 0x07, 0x00, 0xff

#define OBIS_INST_FREQ {0x01, 0x00, 0x0e, 0x07, 0x00, 0xff} /* 순시 주파수 */
#define OBIS_INST_FREQ_nobr 0x01, 0x00, 0x0e, 0x07, 0x00, 0xff

#if 1 /* bccho, 2024-09-05, 삼상 */
#define OBIS_MIN_INST_VOLT_L1 \
    {0x01, 0x00, 0x20, 0x03, 0x00, 0xff} /* 순시 전압 최소값 : A상 */
#define OBIS_MAX_INST_VOLT_L1 \
    {0x01, 0x00, 0x20, 0x06, 0x00, 0xff} /* 순시 전압 최대값 : A상 */
#else
#define OBIS_MIN_INST_VOLT \
    {0x01, 0x00, 0x20, 0x03, 0x00, 0xff} /* 순시 전압 최소값 : A상 */
#define OBIS_MAX_INST_VOLT \
    {0x01, 0x00, 0x20, 0x06, 0x00, 0xff} /* 순시 전압 최대값 : A상 */
#endif

#define OBIS_INST_CURR_L1 \
    {0x01, 0x00, 0x1f, 0x07, 0x00, 0xff} /* 순시 전류 : A상 */
#define OBIS_INST_CURR_L1_nobr 0x01, 0x00, 0x1f, 0x07, 0x00, 0xff
#define OBIS_AVG_CURR_L1 \
    {0x01, 0x00, 0x1f, 0x05, 0x00, 0xff} /* 평균 전류 : A상 */
#define OBIS_AVG_CURR_L1_nobr 0x01, 0x00, 0x1f, 0x05, 0x00, 0xff
#define OBIS_AVG_VOLT_L1 \
    {0x01, 0x00, 0x20, 0x80, 0x00, 0xff} /* 평균 전압 : A상 */
#define OBIS_AVG_VOLT_L1_nobr 0x01, 0x00, 0x20, 0x80, 0x00, 0xff
#define OBIS_INST_voltTHD_L1 \
    {0x01, 0x00, 0x20, 0x07, 0x7c, 0xff} /* 순시 전압 THD : A상 */
#define OBIS_INST_voltTHD_L1_nobr 0x01, 0x00, 0x20, 0x07, 0x7c, 0xff
#define OBIS_IMAXLOAD_L1 \
    {0x01, 0x00, 0x1f, 0x26, 0x00, 0xff} /* 최대부하전류 : A상 */
#define OBIS_INST_VOLT_L1 \
    {0x01, 0x00, 0x20, 0x07, 0x00, 0xff} /* 순시 전압 : A상 */
#define OBIS_INST_VOLT_L1_nobr 0x01, 0x00, 0x20, 0x07, 0x00, 0xff
#define OBIS_INST_PF_L1 \
    {0x01, 0x00, 0x21, 0x07, 0x00, 0xff} /* 순시 역률 : A상 */
#define OBIS_INST_PF_L1_nobr 0x01, 0x00, 0x21, 0x07, 0x00, 0xff

#if 1 /* bccho, 2024-09-05, 삼상 */
#define OBIS_MIN_INST_VOLT_L2 \
    {0x01, 0x00, 0x34, 0x03, 0x00, 0xff} /* 순시 전압 최소값 : B상 */
#define OBIS_MAX_INST_VOLT_L2 \
    {0x01, 0x00, 0x34, 0x06, 0x00, 0xff} /* 순시 전압 최대값 : B상 */
#endif

#define OBIS_INST_CURR_L2 \
    {0x01, 0x00, 0x33, 0x07, 0x00, 0xff} /* 순시 전류 : B상 */
#define OBIS_INST_CURR_L2_nobr 0x01, 0x00, 0x33, 0x07, 0x00, 0xff
#define OBIS_AVG_CURR_L2 \
    {0x01, 0x00, 0x33, 0x05, 0x00, 0xff} /* 평균 전류 : B상 */
#define OBIS_AVG_CURR_L2_nobr 0x01, 0x00, 0x33, 0x05, 0x00, 0xff
#define OBIS_AVG_VOLT_L2 \
    {0x01, 0x00, 0x34, 0x80, 0x00, 0xff} /* 평균 전압 : B상 */
#define OBIS_AVG_VOLT_L2_nobr 0x01, 0x00, 0x34, 0x80, 0x00, 0xff
#define OBIS_INST_voltTHD_L2 \
    {0x01, 0x00, 0x34, 0x07, 0x7c, 0xff} /* 순시 전압 THD : B상 */
#define OBIS_INST_voltTHD_L2_nobr 0x01, 0x00, 0x34, 0x07, 0x7c, 0xff
#define OBIS_IMAXLOAD_L2 \
    {0x01, 0x00, 0x33, 0x26, 0x00, 0xff} /* 최대부하전류 : B상 */
#define OBIS_INST_VOLT_L2 \
    {0x01, 0x00, 0x34, 0x07, 0x00, 0xff} /* 순시 전압 : B상 */
#define OBIS_INST_VOLT_L2_nobr 0x01, 0x00, 0x34, 0x07, 0x00, 0xff
#define OBIS_INST_PF_L2 \
    {0x01, 0x00, 0x35, 0x07, 0x00, 0xff} /* 순시 역률 : B상 */
#define OBIS_INST_PF_L2_nobr 0x01, 0x00, 0x35, 0x07, 0x00, 0xff

#if 1 /* bccho, 2024-09-05, 삼상 */
#define OBIS_MIN_INST_VOLT_L3 \
    {0x01, 0x00, 0x48, 0x03, 0x00, 0xff} /* 순시 전압 최소값 : C상 */
#define OBIS_MAX_INST_VOLT_L3 \
    {0x01, 0x00, 0x48, 0x06, 0x00, 0xff} /* 순시 전압 최대값 : C상 */
#endif

#define OBIS_INST_CURR_L3 \
    {0x01, 0x00, 0x47, 0x07, 0x00, 0xff} /* 순시 전류 : C상 */
#define OBIS_INST_CURR_L3_nobr 0x01, 0x00, 0x47, 0x07, 0x00, 0xff
#define OBIS_AVG_CURR_L3 \
    {0x01, 0x00, 0x47, 0x05, 0x00, 0xff} /* 평균 전류 : C상 */
#define OBIS_AVG_CURR_L3_nobr 0x01, 0x00, 0x47, 0x05, 0x00, 0xff
#define OBIS_AVG_VOLT_L3 \
    {0x01, 0x00, 0x48, 0x80, 0x00, 0xff} /* 평균 전압 : C상 */
#define OBIS_AVG_VOLT_L3_nobr 0x01, 0x00, 0x48, 0x80, 0x00, 0xff
#define OBIS_INST_voltTHD_L3 \
    {0x01, 0x00, 0x48, 0x07, 0x7c, 0xff} /* 순시 전압 THD : C상 */
#define OBIS_INST_voltTHD_L3_nobr 0x01, 0x00, 0x48, 0x07, 0x7c, 0xff
#define OBIS_IMAXLOAD_L3 \
    {0x01, 0x00, 0x47, 0x26, 0x00, 0xff} /* 최대부하전류 : C상 */
#define OBIS_INST_VOLT_L3 \
    {0x01, 0x00, 0x48, 0x07, 0x00, 0xff} /* 순시 전압 : C상 */
#define OBIS_INST_VOLT_L3_nobr 0x01, 0x00, 0x48, 0x07, 0x00, 0xff
#define OBIS_INST_PF_L3 \
    {0x01, 0x00, 0x49, 0x07, 0x00, 0xff} /* 순시 역률 : C상 */
#define OBIS_INST_PF_L3_nobr 0x01, 0x00, 0x49, 0x07, 0x00, 0xff

#define OBIS_INST_PHASE_U12 \
    {0x01, 0x00, 0x51, 0x07, 0x0a, 0xff}  // 선간 전압 위상 각 (A-B)
#define OBIS_INST_PHASE_U12_nobr 0x01, 0x00, 0x51, 0x07, 0x0a, 0xff
#define OBIS_INST_PHASE_U13 \
    {0x01, 0x00, 0x51, 0x07, 0x14, 0xff}  // 선간 전압 위상 각 (A-C)
#define OBIS_INST_PHASE_U13_nobr 0x01, 0x00, 0x51, 0x07, 0x14, 0xff

#define OBIS_INST_PHASE_L1 \
    {0x01, 0x00, 0x51, 0x07, 0x28, 0xff} /* 전압/전류 위상 각 : A상 */
#define OBIS_INST_PHASE_L1_nobr 0x01, 0x00, 0x51, 0x07, 0x28, 0xff
#define OBIS_INST_PHASE_L2 \
    {0x01, 0x00, 0x51, 0x07, 0x33, 0xff} /* 전압/전류 위상 각 : B상 */
#define OBIS_INST_PHASE_L2_nobr 0x01, 0x00, 0x51, 0x07, 0x33, 0xff
#define OBIS_INST_PHASE_L3 \
    {0x01, 0x00, 0x51, 0x07, 0x3e, 0xff} /* 전압/전류 위상 각 : C상 */
#define OBIS_INST_PHASE_L3_nobr 0x01, 0x00, 0x51, 0x07, 0x3e, 0xff

#if 1 /* bccho, 2024-09-05, 삼상 */
#define OBIS_ERROR_RATE_L1 \
    {0x01, 0x00, 0x97, 0x07, 0x00, 0xff} /* 비오차율  : A상 */
#define OBIS_ERROR_RATE_L1_nobr 0x01, 0x00, 0x97, 0x07, 0x00, 0xff

#define OBIS_ERROR_RATE_L2 \
    {0x01, 0x00, 0xAB, 0x07, 0x00, 0xff} /* 비오차율  : B상 */
#define OBIS_ERROR_RATE_L2_nobr 0x01, 0x00, 0xAB, 0x07, 0x00, 0xff

#define OBIS_ERROR_RATE_L3 \
    {0x01, 0x00, 0xBF, 0x07, 0x00, 0xff} /* 비오차율  : C상 */
#define OBIS_ERROR_RATE_L3_nobr 0x01, 0x00, 0xBF, 0x07, 0x00, 0xff
#endif

#define OBIS_INST_CURR_N \
    {0x01, 0x00, 0x5b, 0x07, 0x00, 0xff} /* N상 순시 전류 */
#define OBIS_INST_CURR_N_nobr 0x01, 0x00, 0x5b, 0x07, 0x00, 0xff

#define OBIS_PHASE_L1_2 \
    {0x01, 0x00, 0x51, 0x07, 0x0a, 0xff} /* 선간 전압 위상 각 (A-B) */
#define OBIS_PHASE_L1_2_nobr 0x01, 0x00, 0x51, 0x07, 0x0a, 0xff

#define OBIS_PHASE_L1_3 \
    {0x01, 0x00, 0x51, 0x07, 0x14, 0xff} /* 선간 전압 위상 각 (A-C) */
#define OBIS_PHASE_L1_3_nobr 0x01, 0x00, 0x51, 0x07, 0x14, 0xff

#define OBIS_AVG_VOLT_L1_L2 \
    {0x01, 0x00, 0x7c, 0x80, 0x00, 0xff} /* 평균 선간 전압(A-B) */
#define OBIS_AVG_VOLT_L1_L2_nobr 0x01, 0x00, 0x7c, 0x80, 0x00, 0xff

#define OBIS_AVG_VOLT_L2_L3 \
    {0x01, 0x00, 0x7d, 0x80, 0x00, 0xff} /* 평균 선간 전압(B-C) */
#define OBIS_AVG_VOLT_L2_L3_nobr 0x01, 0x00, 0x7d, 0x80, 0x00, 0xff

#define OBIS_AVG_VOLT_L3_L1 \
    {0x01, 0x00, 0x7e, 0x80, 0x00, 0xff} /* 평균 선간 전압(C-A) */
#define OBIS_AVG_VOLT_L3_L1_nobr 0x01, 0x00, 0x7e, 0x80, 0x00, 0xff

#define OBIS_LOAD_PROFILE \
    {0x01, 0x00, 0x63, 0x01, 0x00, 0xff} /* Load Profile */
#define OBIS_LOAD_PROFILE_nobr 0x01, 0x00, 0x63, 0x01, 0x00, 0xff
#define OBIS_SAGEVT_LOG \
    {0x01, 0x00, 0x63, 0x0a, 0x01, 0xff} /* Sag/Swell(이력) : Sag 발생 */
#define OBIS_SWELLEVT_LOG \
    {0x01, 0x00, 0x63, 0x0a, 0x02, 0xff} /* Sag/Swell(이력) : Swell 발생 */
#define OBIS_LPAVG {0x01, 0x00, 0x63, 0x0d, 0x00, 0xff}
#define OBIS_EVT_LOG {0x01, 0x00, 0x63, 0x62, 'n', 0xff} /* 이력 기록 */
#define OBIS_MAGNET_DURTIME {0x01, 0x00, 0x80, 0x00, 0x03, 0xff}
#define OBIS_MAGNET_DURTIME_nobr 0x01, 0x00, 0x80, 0x00, 0x03, 0xff
#define OBIS_TS_CONF {0x01, 0x00, 0x80, 0x00, 0x04, 0xff}
#define OBIS_TS_SCRIPT_TABLE \
    {0x01, 0x00, 0x80, 0x00, 0x05, 0xff}  // 단지 object 로만 사용하기 위함
#define OBIS_TMP_HOLIDAYS {0x01, 0x00, 0x80, 0x00, 0x07, 0xff}
#define OBIS_CONDENSOR_INST {0x01, 0x00, 0x80, 0x00, 0x08, 0xff}
#define OBIS_LCD_MAP {0x01, 0x00, 0x80, 0x00, 0x09, 0xff}
#define OBIS_KEY_VALUE {0x01, 0x00, 0x80, 0x00, 0x0a, 0xff}
#define OBIS_LP_OVERLAPED_INDEX \
    {0x01, 0x00, 0x80, 0x00, 0x0b, 0xff} /* 총 LP 발생 횟수 */
#define OBIS_LP_OVERLAPED_INDEX_nobr 0x01, 0x00, 0x80, 0x00, 0x0b, 0xff

#define OBIS_LPAVG_INTERVAL {0x01, 0x00, 0x80, 0x00, 0x0c, 0xff}
#define OBIS_SEL_REACT {0x01, 0x00, 0x80, 0x00, 0x0d, 0xff}
#define OBIS_COMM_ENABLE {0x01, 0x00, 0x80, 0x00, 0x0e, 0xff}
#define OBIS_OVERCURR_ENABLE {0x01, 0x00, 0x80, 0x00, 0x0f, 0xff}
#define OBIS_MAGNET_DET_NUM {0x01, 0x00, 0x80, 0x0a, 0x00, 0xff}
#define OBIS_MAGNET_DET_NUM_nobr 0x01, 0x00, 0x80, 0x0a, 0x00, 0xff
#define OBIS_TEMP_THRSHLD \
    {0x01, 0x00, 0x80, 0x23, 0x00, 0xff} /* 온도 Threshold 설정 */
#define OBIS_TEMP_OVER {0x01, 0x00, 0x81, 0x1a, 0x00, 0xff} /* 온도 초과 */
#define OBIS_MTCONST_ACTIVE \
    {0x01, 0x01, 0x00, 0x03, 0x00, 0xff} /* 계기 정수 : 유효전력량 */
#define OBIS_MTCONST_REACTIVE \
    {0x01, 0x01, 0x00, 0x03, 0x01, 0xff} /* 계기 정수 : 무효전력량 */
#define OBIS_MTCONST_APP \
    {0x01, 0x01, 0x00, 0x03, 0x02, 0xff} /* 계기 정수 : 피상전력량 */
#define OBIS_LP_INTERVAL                                     \
    {0x01, 0x01, 0x00, 0x08, 0x04, 0xff} /* LP 기록 주기 \
                                          */
#define OBIS_CUM_DEMAND {0x01, 0x01, 'm', 0x02, 't', 'Z'}
#define OBIS_CUM_DEMAND_nPRD {0x01, 0x02, 'm', 0x02, 't', 'Z'}
#define OBIS_CUM_DEMAND_SEASON {0x01, 0x03, 'm', 0x02, 't', 'Z'}
#define OBIS_CURR_LAST_DEMAND                                                                        \
    {0x01, 0x01, 'm', 0x04, 0x00, 0xff}                                                              \
    /* 현재/직전 수요 시한 수요전력 : 1 수전 유효, 2 송전 유효, 9 수전 피상, \
       10 송전 피상 */
#define OBIS_MAX_DEMAND {0x01, 0x01, 'm', 0x06, 't', 'Z'}
#define OBIS_MAX_DEMAND_nPRD {0x01, 0x02, 'm', 0x06, 't', 'Z'}
#define OBIS_MAX_DEMAND_SEASON {0x01, 0x03, 'm', 0x06, 't', 'Z'}
#define OBIS_MAX_DEMAND__SIGN {0x01, 0x01, 'c', 0x06, 0x80, 'Z'}
#define OBIS_MAX_DEMAND_nPRD__SIGN {0x01, 0x02, 'c', 0x06, 0x80, 'Z'}
#define OBIS_MAX_DEMAND_SEASON__SIGN {0x01, 0x03, 'c', 0x06, 0x80, 'Z'}
#define OBIS_ENERGY {0x01, 0x01, 'm', 0x08, 't', 'Z'}
#define OBIS_ENERGY_nPRD {0x01, 0x02, 'm', 0x08, 't', 'Z'}
#define OBIS_ENERGY_SEASON {0x01, 0x03, 'm', 0x08, 't', 'Z'}
#define OBIS_ENERGY__SIGN {0x01, 0x01, 'c', 0x08, 0x80, 'Z'}
#define OBIS_ENERGY_nPRD__SIGN {0x01, 0x02, 'c', 0x08, 0x80, 'Z'}
#define OBIS_ENERGY_SEASON__SIGN {0x01, 0x03, 'c', 0x08, 0x80, 'Z'}
#define OBIS_ENERGY_FWD_BOTH_REACT {0x01, 0x01, 0x80, 0x08, 't', 'Z'}
#define OBIS_ENERGY_BWD_BOTH_REACT {0x01, 0x01, 0x81, 0x08, 't', 'Z'}
#define OBIS_MONTH_SUBLOCKS {0x01, 0x01, 'm', 0x80, 't', 'Z'}
#define OBIS_ENERGY_FWD_ACT              \
    {0x01, 0x01, 0x01, 0x08, 0x00, 0xff} \
    /* 전체 시간대 - 수전 유효전력량[Q1+Q4] : 정기 검침 자료, 현재 */
#define OBIS_ENERGY_FWD_ACT_nobr 0x01, 0x01, 0x01, 0x08, 0x00, 0xff
#define OBIS_ENERGY_BWD_ACT              \
    {0x01, 0x01, 0x02, 0x08, 0x00, 0xff} \
    /* 전체 시간대 - 송전 유효전력량[Q2+Q3] : 정기 검침 자료, 현재 */
#define OBIS_ENERGY_BWD_ACT_nobr 0x01, 0x01, 0x02, 0x08, 0x00, 0xff
#define OBIS_ENERGY_FWD_LAG_REACT        \
    {0x01, 0x01, 0x05, 0x08, 0x00, 0xff} \
    /* 전체 시간대 - 수전 지상 무효전력량[Q1] : 정기 검침 자료, 현재 */
#define OBIS_ENERGY_FWD_LAG_REACT_nobr 0x01, 0x01, 0x05, 0x08, 0x00, 0xff
#define OBIS_ENERGY_BWD_LEAD_REACT       \
    {0x01, 0x01, 0x06, 0x08, 0x00, 0xff} \
    /* 전체 시간대 - 송전 진상 무효전력량[Q2] : 정기 검침 자료, 현재 */
#define OBIS_ENERGY_BWD_LEAD_REACT_nobr 0x01, 0x01, 0x06, 0x08, 0x00, 0xff
#define OBIS_ENERGY_BWD_LAG_REACT        \
    {0x01, 0x01, 0x07, 0x08, 0x00, 0xff} \
    /* 전체 시간대 - 송전 지상 무효전력량[Q3] : 정기 검침 자료, 현재 */
#define OBIS_ENERGY_BWD_LAG_REACT_nobr 0x01, 0x01, 0x07, 0x08, 0x00, 0xff
#define OBIS_ENERGY_FWD_LEAD_REACT       \
    {0x01, 0x01, 0x08, 0x08, 0x00, 0xff} \
    /* 전체 시간대 - 수전 진상 무효전력량[Q4] : 정기 검침 자료, 현재 */
#define OBIS_ENERGY_FWD_LEAD_REACT_nobr 0x01, 0x01, 0x08, 0x08, 0x00, 0xff
#define OBIS_ENERGY_FWD_APP              \
    {0x01, 0x01, 0x09, 0x08, 0x00, 0xff} \
    /* 전체 시간대 - 수전 피상전력량[Q1+Q4] : 정기 검침 자료, 현재 */
#define OBIS_ENERGY_FWD_APP_nobr 0x01, 0x01, 0x09, 0x08, 0x00, 0xff
#define OBIS_ENERGY_BWD_APP              \
    {0x01, 0x01, 0x0a, 0x08, 0x00, 0xff} \
    /* 전체 시간대 - 송전 피상전력량[Q2+Q3] : 정기 검침 자료, 현재 */
#define OBIS_ENERGY_BWD_APP_nobr 0x01, 0x01, 0x0a, 0x08, 0x00, 0xff
#define OBIS_LAST15_PF_DELI {0x01, 0x01, 0x0d, 0x1d, 0x00, 0xff}
#define OBIS_LAST15_PF_RECEI {0x01, 0x01, 0x54, 0x1d, 0x00, 0xff}
#define OBIS_AVGPF_DELI {0x01, 0x01, 0x0d, 0x09, 't', 'Z'}
#define OBIS_AVGPF_RECEI {0x01, 0x01, 0x54, 0x09, 't', 'Z'}
#define OBIS_AVGPF_DELI_nPRD {0x01, 0x02, 0x0d, 0x09, 't', 'Z'}
#define OBIS_AVGPF_RECEI_nPRD {0x01, 0x02, 0x54, 0x09, 't', 'Z'}
#define OBIS_AVGPF_DELI_SEASON {0x01, 0x03, 0x0d, 0x09, 't', 'Z'}
#define OBIS_AVGPF_RECEI_SEASON {0x01, 0x03, 0x54, 0x09, 't', 'Z'}
#define OBIS_INST_PROFILE {0x01, 0x01, 0x62, 0x80, 0x00, 0xff}
#define OBIS_REM_ENERGY {0x01, 0x80, 0x80, 0xbd, 0x01, 0xff}
#define OBIS_REM_ENERGY_nobr 0x01, 0x80, 0x80, 0xbd, 0x01, 0xff
#define OBIS_REM_MONEY {0x01, 0x80, 0x80, 0xbd, 0x02, 0xff}
#define OBIS_REM_TIME {0x01, 0x80, 0x80, 0xbd, 0x03, 0xff}
#define OBIS_BUY_ENERGY {0x01, 0x80, 0x80, 0xbd, 0x04, 0xff}
#define OBIS_BUY_ENERGY_nobr 0x01, 0x80, 0x80, 0xbd, 0x04, 0xff
#define OBIS_BUY_MONEY {0x01, 0x80, 0x80, 0xbd, 0x05, 0xff}
#define OBIS_PREPAY_ENABLE {0x01, 0x80, 0x80, 0xbd, 0x06, 0xff}
#define OBIS_PREPAY_ENABLE_nobr 0x01, 0x80, 0x80, 0xbd, 0x06, 0xff
#define OBIS_PREPAY_LOADLIMIT_CANCEL \
    {0x01, 0x80, 0x80, 0xbe, 0x06, 0xff}  // 선불형 특정일 부하 제한 해제
#define OBIS_PREPAY_LOADLIMIT_CANCEL_nobr 0x01, 0x80, 0x80, 0xbe, 0x06, 0xff
#define OBIS_sCURR_nonSEL_NUM {0x01, 0x80, 0x80, 0xc0, 0x01, 0xff}
#define OBIS_sCURR_autoRTN_VAL {0x01, 0x80, 0x80, 0xc0, 0x02, 0xff}
#define OBIS_sCURR_autoRTN_VAL_nobr 0x01, 0x80, 0x80, 0xc0, 0x02, 0xff
#define OBIS_sCURR_COUNTER_N1 {0x01, 0x80, 0x80, 0xc0, 0x03, 0xff}
#define OBIS_sCURR_COUNTER_N1_nobr 0x01, 0x80, 0x80, 0xc0, 0x03, 0xff
#define OBIS_sCURR_LIMIT_NUM {0x01, 0x80, 0x80, 0xc0, 0x04, 0xff}
#define OBIS_sCURR_LIMIT_NUM_nobr 0x01, 0x80, 0x80, 0xc0, 0x04, 0xff
#define OBIS_sCURR_LIMIT_VAL {0x01, 0x80, 0x80, 0xc0, 0x05, 0xff}
#define OBIS_sCURR_LIMIT_VAL_nobr 0x01, 0x80, 0x80, 0xc0, 0x05, 0xff
#define OBIS_sCURR_LIMIT2_VAL {0x01, 0x80, 0x80, 0xc0, 0x06, 0xff}
#define OBIS_sCURR_LIMIT2_VAL_nobr 0x01, 0x80, 0x80, 0xc0, 0x06, 0xff
#define OBIS_sCURR_HOLD {0x01, 0x80, 0x80, 0xc0, 0x07, 0xff}
#define OBIS_sCURR_RECOVER_N1 {0x01, 0x80, 0x80, 0xc0, 0x08, 0xff}
#define OBIS_sCURR_RECOVER_N2 {0x01, 0x80, 0x80, 0xc0, 0x09, 0xff}
#define OBIS_LATCHON_COUNTER {0x01, 0x80, 0x80, 0xbd, 0x0a, 0xff}
#define OBIS_TOU_SET_CNT {0x00, 0x41, 0x00, 0x00, 0x01, 0xFF}
#define OBIS_EXT_PROG_ID {0x00, 0x41, 0x00, 0x00, 0x02, 0xFF}

#define OBIS_ADJ_FACTOR \
    {0x01, 0x80, 0x80, 0x80, 0x01, 0xff}  // Private OBIS : 순시 온도 ?

#define OBIS_SECURITY_SETUP_3 {0x00, 0x00, 0x2B, 0x00, 0x03, 0xff}
#define OBIS_SECURITY_SETUP_3_nobr 0x00, 0x00, 0x2B, 0x00, 0x03, 0xff
#define OBIS_SECURITY_SETUP_4 {0x00, 0x00, 0x2B, 0x00, 0x04, 0xff}
#define OBIS_SECURITY_SETUP_4_nobr 0x00, 0x00, 0x2B, 0x00, 0x04, 0xff
#define OBIS_CERT_LOG_HISTORY {0x01, 0x00, 0x63, 0x63, 0x00, 0xff}
#define OBIS_CERT_LOG_HISTORY_nobr 0x01, 0x00, 0x63, 0x63, 0x00, 0xff
#define OBIS_CERT_LOG_NUM {0x00, 0x80, 0x63, 0x62, 0x14, 0xff}
#define OBIS_CERT_LOG_NUM_nobr 0x00, 0x80, 0x63, 0x62, 0x14, 0xff
#define OBIS_CERT_LOG_CASE {0x01, 0x80, 0x80, 0xC0, 0x63, 0xff}
#define OBIS_CERT_LOG_CASE_nobr 0x01, 0x80, 0x80, 0xC0, 0x63, 0xff
#define OBIS_SAP_ASSIGNM \
    {0x00, 0x00, 0x29, 0x00, 0x00, 0xff} /* SAP Assignment */
#define OBIS_SAP_ASSIGNM_nobr 0x00, 0x00, 0x29, 0x00, 0x00, 0xff
#define OBIS_RTIME_P_ENERGY {0x01, 0x02, 0x00, 0x08, 0x00, 0xff}
#define OBIS_RTIME_P_ENERGY_nobr 0x01, 0x02, 0x00, 0x08, 0x00, 0xff
#define OBIS_RTIME_P_LP {0x01, 0x00, 0x63, 0x02, 0x00, 0xff}
#define OBIS_RTIME_P_LP_nobr 0x01, 0x00, 0x63, 0x02, 0x00, 0xff
#define OBIS_RTIME_P_LP_INTERVAL {0x01, 0x01, 0x00, 0x08, 0x05, 0xff}
#define OBIS_RTIME_P_LP_INTERVAL_nobr 0x01, 0x01, 0x00, 0x08, 0x05, 0xff

#define OBIS_TOU_IMAGE_TRANSFER          \
    {0x00, 0x00, 0x2c, 0x00, 0x02, 0xff} \
    // 0x02 가 다른 값일 경우 OBIS_SW_IMAGE_TRANSFER 임.
#define OBIS_TOU_IMAGE_TRANSFER_nobr 0x00, 0x00, 0x2c, 0x00, 0x02, 0xff
#define OBIS_SELF_ERR_REF_VAL {0x01, 0x80, 0x80, 0xbe, 0x11, 0xff}
#define OBIS_SELF_ERR_REF_VAL_nobr 0x01, 0x80, 0x80, 0xbe, 0x11, 0xff
#define OBIS_CT_RATIO {0x01, 0x00, 0x00, 0x04, 0x02, 0xff}
#define OBIS_CT_RATIO_nobr 0x01, 0x00, 0x00, 0x04, 0x02, 0xff
#define OBIS_PT_RATIO {0x01, 0x00, 0x00, 0x04, 0x03, 0xff}
#define OBIS_PT_RATIO_nobr 0x01, 0x00, 0x00, 0x04, 0x03, 0xff
#define OBIS_PUL_SEL_REACT_APP {0x00, 0x00, 0x60, 0x03, 0x03, 0xff}
#define OBIS_PUL_SEL_REACT_APP_nobr 0x00, 0x00, 0x60, 0x03, 0x03, 0xff
#define OBIS_METERING_TYPE_SEL {0x01, 0x00, 0x80, 0x00, 0x10, 0xff}
#define OBIS_METERING_TYPE_SEL_nobr 0x01, 0x00, 0x80, 0x00, 0x10, 0xff
#define OBIS_EVENT_INFO \
    {0x01, 0x00, 0x63, 0x80, 0x00, 0xff} /* 자기진단 이벤트 정보 */
#define OBIS_EVENT_INFO_nobr 0x01, 0x00, 0x63, 0x80, 0x00, 0xff

// 0080636216FF
#define OBIS_SW_UP_CNT_0 {0x00, 0x80, 0x63, 0x62, 0x16, 0xff}     //
#define OBIS_SW_UP_CNT_0_nobr 0x00, 0x80, 0x63, 0x62, 0x16, 0xff  //
#define OBIS_SW_UP_CNT_1 {0x00, 0x80, 0x63, 0x62, 0x17, 0xff}     //
#define OBIS_SW_UP_CNT_1_nobr 0x00, 0x80, 0x63, 0x62, 0x17, 0xff  //
#define OBIS_SW_UP_CNT_2 {0x00, 0x80, 0x63, 0x62, 0x18, 0xff}     //
#define OBIS_SW_UP_CNT_2_nobr 0x00, 0x80, 0x63, 0x62, 0x18, 0xff  //
#define OBIS_SW_UP_CNT_3 {0x00, 0x80, 0x63, 0x62, 0x19, 0xff}     //
#define OBIS_SW_UP_CNT_3_nobr 0x00, 0x80, 0x63, 0x62, 0x19, 0xff  //
#if 1  // jp.kim 24.10.28
#define OBIS_Working_PWR_FAIL_NUM {0x01, 0x80, 0x63, 0x62, 0x1A, 0xff}
#define OBIS_Working_PWR_FAIL_NUM_nobr 0x01, 0x80, 0x63, 0x62, 0x1A, 0xff
#endif

#define OBIS_SW_UP_LOG_0 {0x01, 0x00, 0x63, 0x62, 0x16, 0xff}     //
#define OBIS_SW_UP_LOG_0_nobr 0x01, 0x00, 0x63, 0x62, 0x16, 0xff  //
#define OBIS_SW_UP_LOG_1 {0x01, 0x00, 0x63, 0x62, 0x17, 0xff}     //
#define OBIS_SW_UP_LOG_1_nobr 0x01, 0x00, 0x63, 0x62, 0x17, 0xff  //
#define OBIS_SW_UP_LOG_2 {0x01, 0x00, 0x63, 0x62, 0x18, 0xff}     //
#define OBIS_SW_UP_LOG_2_nobr 0x01, 0x00, 0x63, 0x62, 0x18, 0xff  //
#define OBIS_SW_UP_LOG_3 {0x01, 0x00, 0x63, 0x62, 0x19, 0xff}     //
#define OBIS_SW_UP_LOG_3_nobr 0x01, 0x00, 0x63, 0x62, 0x19, 0xff  //
#if 1  // jp.kim 24.10.28
#define OBIS_Working_PWR_FAIL_LOG {0x01, 0x00, 0x63, 0x62, 0x1A, 0xff}     //
#define OBIS_Working_PWR_FAIL_LOG_nobr 0x01, 0x00, 0x63, 0x62, 0x1A, 0xff  //
#endif

#define OBIS_SW_INFO                         \
    {0x00, 'b', 0x00, 0x02, 0 /*'e'*/, 0xff} \
    //'a' -> 0, 'e' -> 0 /* 현재 소프트웨어 정보 */
#define OBIS_SW_INFO_nobr \
    0x00, 'b', 0x00, 0x02, 0 /*'e'*/, 0xff  //'a' -> 0, 'e' -> 0
#define OBIS_SW_APPLY_DATE \
    {0x00, 'b', 0x60, 0x02, 0x0d, 0xff}  //'a' -> 0, 0->'b'
#define OBIS_SW_APPLY_DATE_nobr \
    0x00, 'b', 0x60, 0x02, 0x0d, 0xff  //'a' -> 0, 0->'b'
#define OBIS_SW_IMAGE_TRANSFER          \
    {0x00, 0x00, 0x2c, 0x00, 'e', 0xff} \
    // 'e' 가 2 일경우 OBIS_TOU_IMAGE_TRANSFER 임.
#define OBIS_SW_IMAGE_TRANSFER_nobr 0x00, 0x00, 0x2c, 0x00, 'e', 0xff
#define OBIS_RUN_MODEM_INFO {0x00, 0x00, 0x60, 0x0c, 0, 0xff}
#define OBIS_RUN_MODEM_INFO_nobr 0x00, 0x00, 0x60, 0x0c, 0, 0xff

#define OBIS_NMS_ID {0x01, 0x00, 0x00, 0x00, 0x01, 0xff}
#define OBIS_NMS_ID_nobr 0x01, 0x00, 0x00, 0x00, 0x01, 0xff

#define OBIS_USE_AMR_DATA_NUM \
    {0x01, 'b', 0x00, 0x01, 0x01, 0xff} /* 사용 가능한 검침 자료 개수 */
#define OBIS_USE_AMR_DATA_NUM_nobr 0x01, 'b', 0x00, 0x01, 0x01, 0xff

#define OBIS_PHASE_DET_CONT_TIME {0x01, 0x00, 0x00, 0x08, 0x03, 0xff}
#define OBIS_PHASE_DET_CONT_TIME_nobr 0x01, 0x00, 0x00, 0x08, 0x03, 0xff
#define OBIS_PHASE_DET_CORRECT_VAL {0x01, 0x00, 0x00, 0x08, 0x80, 0xff}
#define OBIS_PHASE_DET_RESULT_VAL {0x01, 0x00, 0x00, 0x08, 0x81, 0xff}
#define OBIS_PHASE_DET_RESULT_VAL_nobr 0x01, 0x00, 0x00, 0x08, 0x81, 0xff
#define OBIS_PERMITxx_TIME_LIMIT {0x01, 0x00, 0x00, 0x09, 0x0b, 0xff}

#define OBIS_INT_PLC_MODEM_ATCMD {0x00, 0x05, 0x60, 0x0c, 0x03, 0xff}
#define OBIS_INT_PLC_MODEM_ATCMD_nobr 0x00, 0x05, 0x60, 0x0c, 0x03, 0xff
#define OBIS_INT_MODEM_ATCMD {0x00, 0x04, 0x60, 0x0c, 0x03, 0xff}
#define OBIS_INT_MODEM_ATCMD_nobr 0x00, 0x04, 0x60, 0x0c, 0x03, 0xff
#define OBIS_EXT_MODEM_ATCMD {0x00, 0x06, 0x60, 0x0c, 0x03, 0xff}
#define OBIS_EXT_MODEM_ATCMD_nobr 0x00, 0x06, 0x60, 0x0c, 0x03, 0xff
// lhh_comment 210126 규격 반영
#define OBIS_INT_MODEM_ATCMD_RSP {0x00, 0x07, 0x60, 0x0c, 0x03, 0xff}
#define OBIS_INT_MODEM_ATCMD_RSP_nobr 0x00, 0x07, 0x60, 0x0c, 0x03, 0xff
#define OBIS_EXT_MODEM_ATCMD_RSP {0x00, 0x08, 0x60, 0x0c, 0x03, 0xff}
#define OBIS_EXT_MODEM_ATCMD_RSP_nobr 0x00, 0x08, 0x60, 0x0c, 0x03, 0xff
#define OBIS_INT_PLC_MODEM_ATCMD_RSP {0x00, 0x09, 0x60, 0x0c, 0x03, 0xff}
#define OBIS_INT_PLC_MODEM_ATCMD_RSP_nobr 0x00, 0x09, 0x60, 0x0c, 0x03, 0xff

#define OBIS_EXT_MODEM_ID {0x00, 0x06, 0x60, 0x0c, 0x05, 0xff}
#define OBIS_EXT_MODEM_ID_nobr 0x00, 0x06, 0x60, 0x0c, 0x05, 0xff

#define OBIS_PUSH_SCRIPT_TABLE \
    {0x00, 0x00, 0x0a, 0x00, 0x6c, 0xff} /* PUSH script table 정보 */
#define OBIS_PUSH_SETUP_ERR_CODE         \
    {0x00, 0x01, 0x19, 0x09, 0x00, 0xff} \
    /* PUSH Setup OBIS 코드 : 자기진단 이벤트 정보 */
#define OBIS_PUSH_SETUP_ERR_CODE_nobr 0x00, 0x01, 0x19, 0x09, 0x00, 0xff
#define OBIS_PUSH_SETUP_LAST_LP          \
    {0x00, 0x02, 0x19, 0x09, 0x00, 0xff} \
    /* PUSH Setup OBIS 코드 : 최근 발생 LP */
#define OBIS_PUSH_SETUP_LAST_LP_nobr 0x00, 0x02, 0x19, 0x09, 0x00, 0xff

#define OBIS_STOCK_OP_TIMES {0x00, 0x80, 0x63, 0x62, 0x15, 0xff}

#define OBIS_OLD_METER_TOU_TRANSFER {0x00, 0x00, 0x0a, 0x80, 0x00, 0xff}

#define OBIS_LP_TOTAL_CNT {0x01, 0x00, 0x80, 0x00, 0x0b, 0xff}
#define OBIS_HOLIDAY_SEL \
    {0x01, 0x00, 0x80, 0x00, 0x12, 0xff} /* 정기/비정기 휴일 적용 */
#define OBIS_HOLIDAY_SEL_nobr 0x01, 0x00, 0x80, 0x00, 0x12, 0xff
#define OBIS_THD_PERIOD_SEL {0x01, 0x01, 0x00, 0x08, 0x06, 0xff}
// #if defined(FEATURE_VIRTUAL_HDLC_ADDR)
#define OBIS_HDLC_OVERLAP_AVOID {0x00, 0x04, 0x60, 0x0c, 0x81, 0xff}
// #endif

#define OBIS_WORKING_FAULT_MIN \
    {0x01, 0x00, 0x80, 0x00, 0x13, 0xff} /* 작업 정전 분 설정 */
#define OBIS_WORKING_FAULT_MIN_nobr 0x01, 0x00, 0x80, 0x00, 0x13, 0xff

#define OBIS_TOU_ID_CHANGE_STS           \
    {0x01, 0x00, 0x80, 0x00, 0x14, 0xff} \
    /* 최근 계량종별 및 정기검침일 변경 방법 */
#define OBIS_TOU_ID_CHANGE_STS_nobr 0x01, 0x00, 0x80, 0x00, 0x14, 0xff

#define OBIS_CAL_ADJ_ACT \
    {0x01, 0x00, 0x60, 0x34, 0xF0, 0xff}  // Private OBIS : //JP.KIM 24.11.08
#define OBIS_SYS_TITLE \
    {0x00, 0x01, 0x01, 0x06, 0X00, 0x06}  // Private OBIS : //JP.KIM 24.12.05
#define OBIS_INSTALL_CERT \
    {0x00, 0x01, 0x01, 0x06, 0X00, 0x04}  // Private OBIS : bccho, 2024-12-06
#define OBIS_INSTALL_KEY \
    {0x00, 0x01, 0x01, 0x06, 0X00, 0x05}  // Private OBIS : bccho, 2024-12-06

#define MAX_BILLING_COUNTER 100  // VZ (max billing counter)

//-----------------------------------------------------------------------------
// Access Mode
//-----------------------------------------------------------------------------
enum access_mode_e
{
    no_access = 0,
    read_only,
    write_only,
    read_and_write,
    authenticated_read_only,
    authenticated_write_only,
    authenticated_read_and_write,
    enc_read,
    auth_enc_read,
    auth_enc_read_write,
    auth_enc_sign_read,
    auth_enc_sign_read_write

};

typedef struct
{
    obj_id_enum_type obj;
    uint16_t class_id;
    uint8_t instance_id[6];
    uint8_t version;
    uint8_t attributes_cnt;
    const /*__code*/ uint8_t *as0_attr;
    const /*__code*/ uint8_t *as1_attr;
    const uint8_t *as3_attr;
    const uint8_t *as4_attr;
} myobj_struct_type;

typedef enum
{
    DATA_ACS_SUCCESS,
    DATA_ACS_HW_FAULT,
    DATA_ACS_TEMP_FAIL,
    DATA_ACS_RDWR_DENIED,
    DATA_ACS_OBJ_UNDEF,
    DATA_ACS_CLS_INCONS = 9,
    DATA_ACS_OBJ_UNAVAIL = 11,
    DATA_ACS_TYPE_UNMAT,
    DATA_ACS_SCOPE_VIOLATED,
    DATA_ACS_BLK_UNAVAIL,
    DATA_ACS_LONG_GET_ABORT,
    DATA_ACS_NO_LONG_GET,
    DATA_ACS_LONG_SET_ABORT,
    DATA_ACS_NO_LONG_SET_INPROGRESS,
    DATA_ACS_OTHER_REASON = 250
} data_access_result_type;

typedef enum
{
    RLT_ACT_SUCCESS,
    RLT_ACT_HW_FAULT,
    RLT_ACT_TEMP_FAIL,
    RLT_ACT_RDWR_DENIED,
    RLT_ACT_OBJ_UNDEF,
    RLT_ACT_CLS_INCONS = 9,
    RLT_ACT_OBJ_UNAVAIL = 11,
    RLT_ACT_TYPE_UNMAT,
    RLT_ACT_SCOPE_VIOLATED,
    RLT_ACT_BLK_UNAVAIL,
    RLT_ACT_LONG_ACT_ABORT,
    RLT_ACT_NO_LONG_ACT_INPROGRESS,
    RLT_ACT_OTHER_REASON = 250
} action_result_type;

/*state-error [0] IMPLICIT ENUMERATED*/
typedef enum
{
    SER_STATE_OK = 0,
    SER_STATE_NOT_ALLOWED,
    SER_STATE_UNKNOWN
} state_err_type;

/*service-error [1] CHOICE*/
typedef enum
{
    SER_ERR_OK = 0,
    SER_ERR_OP_NOT_POSSIBLE,
    SER_ERR_SER_NOT_SUPPORTED,
    SER_ERR_OTHER_REASON,
    SER_ERR_PDU_TOO_LONG,
    SER_ERR_DECIPER_ERR,
    SER_ERR_INVOC_CNT_ERR  // uint32_t
} sevice_err_type;

typedef enum
{
    eObisGrpB_AmrDataT_PERIOD = 1,
    eObisGrpB_AmrDataT_nPERIOD = 2,
    eObisGrpB_AmrDataT_SEASON_CHG = 3
} EN_OBIS_GRPB_AMR_DATA_TYPE;

typedef enum
{
    eObisGrpC_EngyT_ReceiAct = 1,  // 수전 유효
    eObisGrpC_EngyT_DeliAct = 2,
    eObisGrpC_EngyT_ReceiApp = 9,
    eObisGrpC_EngyT_DeliApp = 10,
    eObisGrpC_EngyT_ReceiLagReact = 5,
    eObisGrpC_EngyT_DeliLeadReact = 6,
    eObisGrpC_EngyT_ReceiLeadReact = 8,
    eObisGrpC_EngyT_DeliLagReact = 7
} EN_OBIS_GRPC_ENERGY_TYPE;

typedef enum
{
    eObisGrpE_TariffT_Total,
    eObisGrpE_TariffT_1,
    eObisGrpE_TariffT_2,
    eObisGrpE_TariffT_3,
    eObisGrpE_TariffT_4
} EN_OBIS_GRPE_TARIFF_TYPE;

typedef enum
{
    eObisGrpE_EngyT_Recei = 1,  // 수전
    eObisGrpE_EngyT_Deli = 3
} EN_OBIS_GRPE_ENERGY_TYPE;

typedef enum
{
    DEVICE_CMD_NULL,
    DEVICE_CMD_INIT = 1,
    DEVICE_CMD_PGM_DL = 2,
    DEVICE_CMD_CURPGM_WORK = 3,
    DEVICE_CMD_CURPGM_READ = 4,
    DEVICE_CMD_FUTPGM_WORK = 5,
    DEVICE_CMD_FUTPGM_READ = 6,
    DEVICE_CMD_FUTPGM_DEL = 7,
    DEVICE_CMD_sCURR_INIT = 8,
    DEVICE_CMD_SELREACT_CANCEL = 10
} device_cmd_type;

typedef enum
{
    DEVICE_BCMD_NULL,
    DEVICE_BCMD_CURPGM_READ,
    DEVICE_BCMD_FUTPGM_READ,
    DEVICE_BCMD_MDM_FACTORY_SETUP,
    DEVICE_BCMD_MDM_RESET
} device_bcmd_type;

typedef struct _st_mt_rst_time_
{
    date_time_type dt;
    uint16_t CRC_M;
} ST_MT_RST_TIME;

#define appl_is_sap_public() (appl_sap == SAP_PUBLIC)
#define appl_is_sap_utility() (appl_sap == SAP_UTILITY)
#define appl_is_sap_485comm() (appl_sap == SAP_485COMM)
#define appl_is_sap_private() (appl_sap == SAP_PRIVATE)
#define appl_is_sap_sec_utility() (appl_sap == SAP_SEC_UTILITY)
#define appl_is_sap_sec_site() (appl_sap == SAP_SEC_SITE)

#define appl_is_sap_assign_dev_mnt() \
    (g_sap_assin_run == SAP_ASSIGN_DEV_MANAGEMENT)
#define appl_is_sap_assign_kepco_mnt() \
    (g_sap_assin_run == SAP_ASSIGN_KEPCO_MANAGEMENT)

extern uint8_t *appl_msg;
extern int appl_len;
extern uint8_t *pPdu;
extern int pPdu_idx;
extern uint8_t *appl_tbuff;
extern uint16_t appl_class_id;
extern obis_type appl_obis;
extern uint8_t appl_att_id;
extern obj_id_enum_type appl_obj_id;
extern appl_req_type appl_reqtype;
extern uint8_t appl_reqchoice;
extern uint8_t appl_invokeid_pri;
extern bool appl_is_first_block;
extern uint8_t appl_is_last_block;
extern uint32_t appl_block_num;
extern uint32_t appl_next_block_num;
extern appl_result_type appl_result;
extern U8_16 *pu8_16;
extern U8_16_32 *pu8_16_32;
extern U8_16 appl_fill_var16;
extern U8_16_32 appl_fill_var32;
extern U8_Float appl_fill_varFloat;
extern U8_S16 appl_fill_varS16;
extern U8_16_S32 appl_fill_varS32;
extern appl_sap_type appl_sap;
extern auth_val_type appl_util_pwd;
extern auth_val_type appl_485_pwd;
extern auth_val_type appl_priv_pwd;
extern uint8_t appl_conformance[3];
extern bool appl_whm_inf_collected;
extern bool appl_mdm_baud_changed;
extern uint8_t appl_resp_result;
extern uint8_t appl_resp_choice;
extern uint8_t appl_resp_last_block;
extern uint32_t appl_resp_block_num;
extern uint8_t appl_selacs_len;
extern uint8_t appl_selacs[];
extern int appl_idx1_for_block;
extern int appl_idx2_for_block;
extern uint8_t appl_act_rsp_option_flag;

appl_state_type appl_get_conn_state(void);
void appl_set_conn_state(appl_state_type state);
void appl_init(void);
void appl_conn_ind(uint8_t client);
void appl_disc_ind(bool reason, bool cmdsnrm);
void appl_msg_data_ind(uint8_t *buf, int len);
appl_result_type appl_obj_enum_and_acs_check(void);
void appl_get_resp(void);
void appl_set_resp(void);
void appl_act_resp(void);
int appl_cosem_descript(int idx);
void appl_selective_acs_descript(int idx);
int appl_block_descript(int idx);
void object_list_element_proc(void);
bool obis_is_evtlog_range(uint8_t grp);
elog_kind_type conv_elog_from_grpe(uint8_t grp);
void whm_clear_all(bool is_factory);

// DLMS data-encoding ==> consider endian-type !!!!

#define NULL_DATA 0
#define ARRAY_TAG 1
#define STRUCTURE_TAG 2
#define BOOLEAN_TAG 3
#define BITSTRING_TAG 4
#define DLONG_TAG 5
#define UDLONG_TAG 6
#define FLOATPOINT_TAG 7
#define OCTSTRING_TAG 9
#define VISIBSTRING_TAG 10
#define BCD_TAG 13
#define INTEGER_TAG 15
#define LONG_TAG 16
#define UNSIGNED_TAG 17
#define LONGUNSIGNED_TAG 18
#define CPARRAY_TAG 19
#define LONG64_TAG 20
#define ULONG64_TAG 21
#define ENUM_TAG 22
#define FLOAT32_TAG 23
#define FLOAT64_TAG 24
#define DONTCARE_TAG 255

#define METHOD_OPT_DATA_FLAG 1  // method-invocation-parameters Data OPTIONAL

#define FILL_V8(v) pPdu[pPdu_idx++] = (v)

// (PACKED)alignment 문제 대문에 포인터를 사용 함
#define FILL_V16(v)                   \
    pu8_16 = (U8_16 *)&(v);           \
    pPdu[pPdu_idx++] = pu8_16->c[HI]; \
    pPdu[pPdu_idx++] = pu8_16->c[LO]
#define FILL_V32(v)                         \
    pu8_16_32 = (U8_16_32 *)&(v);           \
    pPdu[pPdu_idx++] = pu8_16_32->c[HI_HI]; \
    pPdu[pPdu_idx++] = pu8_16_32->c[HI_LO]; \
    pPdu[pPdu_idx++] = pu8_16_32->c[LO_HI]; \
    pPdu[pPdu_idx++] = pu8_16_32->c[LO_LO]

#define FILL_U16_pV(a, v)        \
    pu8_16 = (U8_16 *)&(v);      \
    pPdu[(a)++] = pu8_16->c[HI]; \
    pPdu[(a)++] = pu8_16->c[LO]
#define FILL_U32_pV(a, v)              \
    pu8_16_32 = (U8_16_32 *)&(v);      \
    pPdu[(a)++] = pu8_16_32->c[HI_HI]; \
    pPdu[(a)++] = pu8_16_32->c[HI_LO]; \
    pPdu[(a)++] = pu8_16_32->c[LO_HI]; \
    pPdu[(a)++] = pu8_16_32->c[LO_LO]

#define FILL_NULL FILL_V8(NULL)
#define FILL_BOOL(v)      \
    FILL_V8(BOOLEAN_TAG); \
    FILL_V8(v)
#define FILL_BS(b)          \
    FILL_V8(BITSTRING_TAG); \
    FILL_V8(b)
#define FILL_S08(v)       \
    FILL_V8(INTEGER_TAG); \
    FILL_V8(v)
#define FILL_S16(v)    \
    FILL_V8(LONG_TAG); \
    FILL_V16(v)
#define FILL_U08(v)        \
    FILL_V8(UNSIGNED_TAG); \
    FILL_V8(v)
#define FILL_U16(v)            \
    FILL_V8(LONGUNSIGNED_TAG); \
    FILL_V16(v)
#define FILL_S32(v)     \
    FILL_V8(DLONG_TAG); \
    FILL_V32(v)
#define FILL_U32(v)      \
    FILL_V8(UDLONG_TAG); \
    FILL_V32(v)
#define FILL_ENUM(v)   \
    FILL_V8(ENUM_TAG); \
    FILL_V8(v)
#define FILL_FLOAT(v)     \
    FILL_V8(FLOAT32_TAG); \
    FILL_V32(v)
#define FILL_STRING(v)      \
    FILL_V8(OCTSTRING_TAG); \
    FILL_V8(v)
#define FILL_STRING_1(v)    \
    FILL_V8(OCTSTRING_TAG); \
    FILL_V8(0x81);          \
    FILL_V8(v)
#define FILL_STRING_2(v)    \
    FILL_V8(OCTSTRING_TAG); \
    FILL_V8(0x82);          \
    FILL_V16(v)
#define FILL_ARRAY(v)   \
    FILL_V8(ARRAY_TAG); \
    FILL_V8(v)
#define FILL_ARRAY_2(v) \
    FILL_V8(ARRAY_TAG); \
    FILL_V8(0x82);      \
    FILL_V16(v)
#define FILL_STRUCT(v)      \
    FILL_V8(STRUCTURE_TAG); \
    FILL_V8(v)
#define FILL_LEN_2(v) \
    FILL_V8(0x82);    \
    FILL_V16(v)

typedef enum
{
    FWINFO_CUR_SYS,
    FWINFO_CUR_METER,
    FWINFO_CUR_MODEM,
    FWINFO_CUR_E_MODEM,
    FWINFO_FUT_SYS,
    FWINFO_FUT_METER,
    FWINFO_FUT_MODEM,
    FWINFO_FUT_E_MODEM,
    FWINFO_MAX_NUM
} EN_FWINFO_TYPE;

#define COMM_IF_WM_BUS (1 << 6)
#define COMM_IF_LTE (1 << 5)
#define COMM_IF_IOT_PLC (1 << 4)
#define COMM_IF_EXT_SUN (1 << 3)
#define COMM_IF_INT_SUN (1 << 2)
#define COMM_IF_CAN (1 << 1)
#define COMM_IF_RS485 (1 << 0)

uint8_t *appl_get_msg_pointer(void);
void appl_set_msg_pointer(uint8_t *pmsg);
int appl_get_appl_len(void);
void appl_set_appl_len(int len);
void appl_set_sap(appl_sap_type sap);
char *dsm_appl_sap_string(uint32_t sap);
char *dsm_appl_conn_string(uint32_t conn);
bool appl_is_asso_for_sec_site(void);
bool appl_is_asso_for_sec_utility(void);
bool appl_is_asso_utility(void);
uint32_t appl_get_sap(void);

void appl_update_sec_for_timestart_sec_utility(uint32_t sec);
uint32_t appl_get_sec_for_timestart_sec_utility(void);
uint32_t appl_get_sec_timeout_for_sec_utility(void);
void appl_set_asso_timestart_flag(bool val);
bool appl_get_asso_timestart_flag(void);

void appl_update_sec_4_485_for_timestart_sec_utility(uint32_t sec);
uint32_t appl_get_sec_4_485_for_timestart_sec_utility(void);
uint32_t appl_get_sec_4_485_timeout_for_sec_utility(void);
void appl_set_asso_4_485_timestart_flag(bool val);
bool appl_get_asso_4_485_timestart_flag(void);

void appl_msg_process(uint8_t *pbuff);
myobj_struct_type *dsm_touETC_get_object(uint8_t class_id, uint8_t *obis);
// uint32_t obis_get_fw_info_type_groupa_b(uint8_t grpb);
uint32_t obis_get_fw_info_type_group_b(uint8_t grpb);
uint32_t obis_get_fw_info_imgtr_groupe(void);
bool obis_is_mr_data_type_groupb(uint8_t grpb);
uint8_t obis_get_mr_data_type(uint8_t grpb);
myobj_struct_type *appl_get_object(uint16_t obj_idx);
void whm_clear_all(bool is_factory);
void dsm_meter_reset_timer_proc(void);

#endif
