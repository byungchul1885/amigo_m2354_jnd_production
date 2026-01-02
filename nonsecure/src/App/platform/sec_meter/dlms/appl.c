#include "main.h"
#include "options.h"
#include "nv.h"
#if 0 /* bccho, FLASH, 2023-07-15 */
#include "flash.h"
#endif /* bccho */
#include "comm.h"
#include "dlms_todo.h"
#include "dl.h"
#include "aarq.h"
#include "appl.h"
#include "get_req.h"
#include "act_req.h"
#include "set_req.h"
#include "amg_sec.h"
#include "defines.h"
#include "amg_meter_main.h"
#include "amg_rtc.h"
#include "amg_media_mnt.h"
#include "amg_uart.h"

#define _D "[APPL] "

uint8_t* appl_msg;
int appl_len;

uint8_t* pPdu;
int pPdu_idx;
uint8_t* appl_tbuff;

/* bccho, 2024-09-05, 삼상 */
bool G_SIGNING_OK = 0;

extern U16 getresp_LP_len;
extern U32 getresp_LP_index;

U8_16* pu8_16;
U8_16_32* pu8_16_32;

appl_sap_type appl_sap;
uint8_t g_sap_assin_run;

auth_val_type appl_util_pwd;
auth_val_type appl_485_pwd;
auth_val_type appl_priv_pwd;

uint8_t appl_conformance[3];
bool appl_whm_inf_collected;
bool appl_mdm_baud_changed;

uint16_t appl_class_id;
obis_type appl_obis;
uint8_t appl_att_id;
obj_id_enum_type appl_obj_id;
appl_req_type appl_reqtype;
uint8_t appl_reqchoice;
uint8_t appl_invokeid_pri;
bool appl_is_first_block;
uint8_t appl_is_last_block;
uint32_t appl_block_num;
uint32_t appl_next_block_num;
uint8_t appl_act_rsp_option_flag;
uint8_t appl_acs_type;

static appl_state_type appl_conn_state;

#define SELECTIVE_ACS_BUFSIZE 20
uint8_t appl_selacs[SELECTIVE_ACS_BUFSIZE];
uint8_t appl_selacs_len;

uint8_t appl_resp_result;
uint8_t appl_resp_choice;
uint8_t appl_resp_last_block;
uint32_t appl_resp_block_num;
int appl_idx1_for_block;
int appl_idx2_for_block;
extern int8_t no_inst_curr_chk_zon_cnt;

static void appl_sap_proc(uint8_t client);
static void appl_var_init(void);
static bool appl_AARE(aarq_rslt_type rslt);
static bool appl_AARE_asso3(aarq_rslt_type rslt);
static bool appl_reqtype_act_check(appl_req_type req);
static bool appl_reqtype_sec_check(appl_req_type req);
static bool appl_reqtype_sec_ds_check(appl_req_type req);
bool obis_is_asso_range(uint8_t grp);
void appl_msg_process(uint8_t* pbuff);
static void appl_exception_resp(uint8_t err_state, uint8_t err_enum);
static bool obis_is_fw_ch_groupb(uint8_t grpb);
static bool obis_is_fw_ch_groupe(uint8_t grpe);
static void appl_msg_proc(int idx);
static void appl_confirmed_svcerr_resp(uint8_t err_choice, uint8_t err_enum);
static bool obis_is_enercy_ch_groupc(uint8_t grpc, uint8_t grpd);
static uint8_t appl_get_result_reason(get_req_result_type rslt);
static uint8_t appl_set_result_reason(set_req_result_type rslt);
static uint8_t appl_act_result_reason(act_req_result_type rslt);
static bool appl_reqtype_check(appl_req_type req);
static void appl_util_pwd_retore(void);
static void appl_priv_pwd_retore(void);
static void appl_485comm_pwd_retore(void);
void lcd_ASSOCIATED_disp(void);
void lcd_ASSOCIATED_disp_all_off(void);
extern bool factory_addtional_reset(void);

#define UTIL_PWD_SIZE 8
const uint8_t util_pwd_default[UTIL_PWD_SIZE] = {'1', 'A', '2', 'B',
                                                 '3', 'C', '4', 'D'};

#define COMM485_PWD_SIZE 8
const /*__code*/ uint8_t comm485_pwd_default[COMM485_PWD_SIZE] = {
    '5', 'E', '6', 'F', '7', 'G', '8', 'H'};

#define PRIV_PWD_SIZE 8
const /*__code*/ uint8_t priv_pwd_default[PRIV_PWD_SIZE] = {
    PWD_CHR1, PWD_CHR2, PWD_CHR3, PWD_CHR4,
    PWD_CHR5, PWD_CHR6, PWD_CHR7, PWD_CHR8};

typedef struct
{
    uint8_t grpm;
    uint8_t grpt;
} groupce_mt_type;

#define NUM_GRPMT 40
static const groupce_mt_type groupce_mt[NUM_GRPMT] = {
    {1, 0}, {1, 1}, {1, 2}, {1, 3},  {1, 4},  {2, 0},  {2, 1},  {2, 2},
    {2, 3}, {2, 4}, {5, 0}, {5, 1},  {5, 2},  {5, 3},  {5, 4},  {6, 0},
    {6, 1}, {6, 2}, {6, 3}, {6, 4},  {7, 0},  {7, 1},  {7, 2},  {7, 3},
    {7, 4}, {8, 0}, {8, 1}, {8, 2},  {8, 3},  {8, 4},  {9, 0},  {9, 1},
    {9, 2}, {9, 3}, {9, 4}, {10, 0}, {10, 1}, {10, 2}, {10, 3}, {10, 4}};

#define NUM_GRPCM 8
static const uint8_t groupc_m[NUM_GRPCM] = {1, 2, 5, 6, 7, 8, 9, 10};

#define NUM_GRPMT1 20
static const groupce_mt_type groupce_mt1[NUM_GRPMT1] = {
    {1, 0}, {1, 1},  {1, 2},  {1, 3},  {1, 4},  {2, 0},  {2, 1},
    {2, 2}, {2, 3},  {2, 4},  {9, 0},  {9, 1},  {9, 2},  {9, 3},
    {9, 4}, {10, 0}, {10, 1}, {10, 2}, {10, 3}, {10, 4},
};

#define NUM_GRPCM1 4
static const uint8_t groupc_m1[NUM_GRPCM1] = {1, 2, 9, 10};

// jp.kim 24.10.28
#define NUM_GRPEN 17  // 15
static const /*__code*/ uint8_t groupe_n[NUM_GRPEN] = {
    1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 14, 15, 16, 17, 18, 19, 26};
#define NUM_ASSO_GRPEN 5
static const /*__code*/ uint8_t groupe_asso_n[NUM_ASSO_GRPEN] = {0, 1, 2, 3, 4};

#define NUM_FW_GRPBB 4
static const /*__code*/ uint8_t groupb_fw_b[NUM_FW_GRPBB] = {0, 2, 4, 6};

#define NUM_FW_GRPE 4
static const uint8_t groupe_fw_e[NUM_FW_GRPE] = {0, 3, 4, 6};

#define NUM_AMR_DATA_GRPEB 3
static const uint8_t groupb_amr_data_b[NUM_AMR_DATA_GRPEB] = {
    eObisGrpB_AmrDataT_PERIOD, eObisGrpB_AmrDataT_nPERIOD,
    eObisGrpB_AmrDataT_SEASON_CHG};

#define NUM_ENERGY_T_GRP_C 8
static const uint8_t groupc_energy_c[NUM_ENERGY_T_GRP_C] = {
    eObisGrpC_EngyT_ReceiAct,       eObisGrpC_EngyT_DeliAct,
    eObisGrpC_EngyT_ReceiApp,       eObisGrpC_EngyT_DeliApp,
    eObisGrpC_EngyT_ReceiLagReact,  eObisGrpC_EngyT_DeliLeadReact,
    eObisGrpC_EngyT_ReceiLeadReact, eObisGrpC_EngyT_DeliLagReact};

#define NUM_TARIFF_T_GRP_E 5
static const uint8_t groupe_tariff_e[NUM_TARIFF_T_GRP_E] = {
    eObisGrpE_TariffT_Total, eObisGrpE_TariffT_1, eObisGrpE_TariffT_2,
    eObisGrpE_TariffT_3, eObisGrpE_TariffT_4};

#define NUM_ENERGY_T_GRP_E 2
static const uint8_t groupe_energy_e[NUM_ENERGY_T_GRP_E] = {
    eObisGrpE_EngyT_Recei, eObisGrpE_EngyT_Deli};

#define AARE_RESULT_POS 17
#define AARE_SVC_CHOICE_POS 20
#define AARE_DIAGNOSTICS_POS 24
#define AARE_CONFORM_BLK_POS 36
#define AARE_SIZE 43
#define AARE_CONFORM_BLK_POS_for_SEC_USRINFO 11
#define ACSE_RESP_REQ_LEN 4
#define APP_RESP_MECHANISM_NAME_LEN 9
#define APP_RESP_RESULT_LEN 5
#define APP_RESP_DIAGONOSTIC_SRC_RESULT_LEN 7
#define APP_RESP_AP_TITLE_LEN 12
#define APP_RESP_USR_INFORMATION_LEN 18
#define APP_RESP_AE_CALLING_QUALIFIER_LEN 508
static const /*__code*/ uint8_t aare[AARE_SIZE] = {
    0x61,  // 0: AARE tag
    0x29,  // 1: length from next to end
    0xa1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01,
    0x01,                    // 2: application context name
    0xa2, 0x03, 0x02, 0x01,  // 13: tag and length .... of the result
    0x00,                    // 17: result = accept
    // result source diagnostic
    0xa3, 0x05,  // 18: tag and length
    0xa1, 0x03,  // 20: tag(acse-service-user CHOICE(1)) and length
    0x02, 0x01,  // 22: tag of INTEGER and length
    0x00,        // 24: result source diagnostic (0 means no diagnostics)
    0xbe, 0x10, 0x04,
    0x0e,  // 25: user information field(xDLMS initiate.response)
    // xDLMS initiate.response
    0x08,  // 29: tag of initiate.response
    0x00,  // 30: usage flag of quality of service
    0x06,  // 31: dlms version
    0x5f, 0x1f, 0x04,
    0x00,              // 32: tag and length ... of negotiated conformance block
    0x00, 0x00, 0x00,  // 36: conformance block (filled in the processing)
    0x3a, 0xb3,  // 39: server-max-receive size (calendar size =  15027 byte)
    0x00, 0x07   // 41: vaa-name
};

uint8_t app_resp_acse_req[ACSE_RESP_REQ_LEN] = {ASSO_RESPONDER_ACSE_REQ, 0x02,
                                                0x07, 0x80};

uint8_t app_resp_mecah_name[APP_RESP_MECHANISM_NAME_LEN] = {
    ASSO_RESP_MECHANISM_NAME, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x07};
uint8_t app_resp_result[APP_RESP_RESULT_LEN] = {ASSO_RESULT_FIELD, 0x03, 0x02,
                                                0x01, 0x00};
uint8_t app_resp_diagnostic_src_result[APP_RESP_DIAGONOSTIC_SRC_RESULT_LEN] = {
    ASSO_RESULT_SRC_DIAG, 0x05, 0xA1, 0x03, 0x02, 0x01, 0x0E};
uint8_t app_resp_ap_title[APP_RESP_AP_TITLE_LEN] = {ASSO_RESP_AP_TITLE,
                                                    0x0A,
                                                    BER_TYPE_OCTET_STRING,
                                                    0x08,
                                                    0x31,
                                                    0x32,
                                                    0x33,
                                                    0x34,
                                                    0x35,
                                                    0x36,
                                                    0x37,
                                                    0x38};
uint8_t app_resp_ae_qualifier[AARQ_AE_CALLING_QUALIFIER_LEN] = {
    ASSO_RESP_AE_QUALIFIER, 0x82, 0x01, 0xF8,
    BER_TYPE_OCTET_STRING,  0x82, 0x01, 0xF4,
};

uint8_t app_resp_usr_info[APP_RESP_USR_INFORMATION_LEN] = {
    ASSO_USER_INFORMATION, 0x10, 0x04,
    0x0e,  // 25: user information field(xDLMS initiate.response)
    // xDLMS initiate.response
    0x08,  // 29: tag of initiate.response
    0x00,  // 30: usage flag of quality of service
    0x06,  // 31: dlms version
    0x5f, 0x1f, 0x04,
    0x00,              // 32: tag and length ... of negotiated conformance block
    0x00, 0x00, 0x00,  // 36: conformance block (filled in the processing)
    0x3a, 0xb3,  // 39: server-max-receive size (calendar size =  15027 byte)
    0x00, 0x07   // 41: vaa-name
};

#define AARE_SVCERR_POS 32
#define AARE_SERR_SIZE 33
static const /*__code*/ uint8_t aare_svcerr[AARE_SERR_SIZE] = {
    0x61,  // 0: AARE tag
    0x1f,  // 1: length from next to end
    0xa1, 0x09, 0x06, 0x07, 0x60, 0x85, 0x74, 0x05, 0x08, 0x01,
    0x01,                    // 2: application context name
    0xa2, 0x03, 0x02, 0x01,  // 13: tag and length .... of the result
    0x01,                    // 17: result = permanent rejected
    // result source diagnostic
    0xa3, 0x05,  // 18: tag and length
    0xa1, 0x03,  // 20: tag(acse-service-user CHOICE(1)) and length
    0x02, 0x01,  // 22: tag of INTEGER and length
    0x01,        // 24: result source diagnostic (1 means no reason given)
    0xbe, 0x06, 0x04,
    0x04,  // 25: user information field(xDLMS ConfirmedServiceError)
    // xDLMS ConfirmedServiceError Pdu
    0x0e,  // 29: tag of ConfirmedServiceError
    0x01,  // 30: CHOICE (1: InitiateError)
    0x06,  // 31: CHOICE (6: initiate)
    0x01   // 32: 1-> DLMS version too low
};

//-----------------------------------------------------------------------------
// All Attributes

const uint8_t all_r[] = {read_only, read_only, read_only, read_only,
                         read_only, read_only, read_only, read_only,
                         read_only, read_only, read_only};

const uint8_t r_rw_r_r_r_r_r_r_r_r[] = {
    read_only, read_and_write, read_only, read_only, read_only,
    read_only, read_only,      read_only, read_only, read_only};

const uint8_t r_r_r_r_r_r_r_rw_r_r[] = {
    read_only, read_only, read_only,      read_only, read_only,
    read_only, read_only, read_and_write, read_only, read_only};

const uint8_t r_r_r_r_r_r_r_rw_rw_r[] = {
    read_only, read_only, read_only,      read_only,      read_only,
    read_only, read_only, read_and_write, read_and_write, read_only};

const uint8_t r_r_r_r_r_r_rw_r_r_r[] = {
    read_only, read_only,      read_only, read_only, read_only,
    read_only, read_and_write, read_only, read_only, read_only};

const uint8_t r_r_r_rw_r_r_r_r_r_r[] = {
    read_only, read_only, read_only, read_and_write, read_only,
    read_only, read_only, read_only, read_only,      read_only};

const uint8_t r_r_r_r_r_r_n_r_r_r[] = {
    read_only, read_only, read_only, read_only, read_only,
    read_only, no_access, read_only, read_only, read_only};

const uint8_t r_rw_r_r_rw_rw_rw_rw_r_r[] = {
    read_only,      read_and_write, read_only,      read_only, read_and_write,
    read_and_write, read_and_write, read_and_write, read_only, read_only};

const uint8_t r_rw_rw_rw_rw_rw_rw_rw_rw_rw[] = {
    read_only,      read_and_write, read_and_write, read_and_write,
    read_and_write, read_and_write, read_and_write, read_and_write,
    read_and_write, read_and_write};

const /*__code*/ uint8_t all_n[] = {no_access, no_access, no_access, no_access,
                                    no_access, no_access, no_access, no_access,
                                    no_access, no_access};

const uint8_t all_rC[] = {auth_enc_read, auth_enc_read, auth_enc_read,
                          auth_enc_read, auth_enc_read, auth_enc_read,
                          auth_enc_read, auth_enc_read, auth_enc_read,
                          auth_enc_read, auth_enc_read};

const uint8_t rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC[] = {
    auth_enc_read, auth_enc_read_write, auth_enc_read, auth_enc_read,
    auth_enc_read, auth_enc_read,       auth_enc_read, auth_enc_read,
    auth_enc_read, auth_enc_read,       auth_enc_read};

const uint8_t rC_rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC[] =  // security setup
    {auth_enc_read, auth_enc_read, auth_enc_read_write, auth_enc_read,
     auth_enc_read, auth_enc_read, auth_enc_read,       auth_enc_read,
     auth_enc_read, auth_enc_read, auth_enc_read};

const uint8_t rC_rwC_rwC_rwC_rC_rC_rC_rC_rC_rC_rC[] = {
    auth_enc_read,       auth_enc_read_write, auth_enc_read_write,
    auth_enc_read_write, auth_enc_read,       auth_enc_read,
    auth_enc_read,       auth_enc_read,       auth_enc_read,
    auth_enc_read,       auth_enc_read};

const uint8_t rC_rwC_rC_rwC_rC_rC_rC_rC_rC_rC_rC[] = {
    auth_enc_read, auth_enc_read_write, auth_enc_read, auth_enc_read_write,
    auth_enc_read, auth_enc_read,       auth_enc_read, auth_enc_read,
    auth_enc_read, auth_enc_read,       auth_enc_read};

const uint8_t rC_rC_rC_rwC_rwC_rC_rC_rC_rC_rC_rC[] = {
    auth_enc_read,       auth_enc_read, auth_enc_read, auth_enc_read_write,
    auth_enc_read_write, auth_enc_read, auth_enc_read, auth_enc_read,
    auth_enc_read,       auth_enc_read, auth_enc_read};

const uint8_t rC_rwC_rC_rC_rwC_rwC_rwC_rwC_rC_rC_rC[] = {
    auth_enc_read,       auth_enc_read_write, auth_enc_read,
    auth_enc_read,       auth_enc_read_write, auth_enc_read_write,
    auth_enc_read_write, auth_enc_read_write, auth_enc_read,
    auth_enc_read,       auth_enc_read};

const uint8_t rC_rC_rC_rC_rC_rC_rC_rwC_rwC_rC_rC[] = {
    auth_enc_read,       auth_enc_read, auth_enc_read, auth_enc_read,
    auth_enc_read,       auth_enc_read, auth_enc_read, auth_enc_read_write,
    auth_enc_read_write, auth_enc_read, auth_enc_read};

const uint8_t rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC[] = {
    auth_enc_read, auth_enc_sign_read, auth_enc_read, auth_enc_read,
    auth_enc_read, auth_enc_read,      auth_enc_read, auth_enc_read,
    auth_enc_read, auth_enc_read,      auth_enc_read};

const uint8_t rC_rC_n_rC_rwCS_rC_rwCS_rC_rC_rC_rC[] = {auth_enc_read,
                                                       auth_enc_read,
                                                       no_access,
                                                       auth_enc_read,
                                                       auth_enc_sign_read_write,
                                                       auth_enc_read,
                                                       auth_enc_sign_read_write,
                                                       auth_enc_read,
                                                       auth_enc_read,
                                                       auth_enc_read,
                                                       auth_enc_read};

const uint8_t rC_rC_rC_rC_rwC_rC_rwC_rC_rC_rC_rC[] = {
    auth_enc_read,       auth_enc_read, auth_enc_read,       auth_enc_read,
    auth_enc_read_write, auth_enc_read, auth_enc_read_write, auth_enc_read,
    auth_enc_read,       auth_enc_read, auth_enc_read};

const uint8_t rC_rC_rC_rC_rC_rwC_rwC_rwC_rwC_rwC_rC[] = {
    auth_enc_read,       auth_enc_read,       auth_enc_read,
    auth_enc_read,       auth_enc_read,       auth_enc_read_write,
    auth_enc_read_write, auth_enc_read_write, auth_enc_read_write,
    auth_enc_read_write, auth_enc_read};

const uint8_t rC_rwCS_rC_rC_rC_rC_rC_rC_rC_rC_rC[] = {
    auth_enc_read, auth_enc_sign_read_write,
    auth_enc_read, auth_enc_read,
    auth_enc_read, auth_enc_read,
    auth_enc_read, auth_enc_read,
    auth_enc_read, auth_enc_read,
    auth_enc_read};

const uint8_t rC_rwC_n_rC_rC_rC_rC_rC_rC_rC_rC[] = {
    auth_enc_read, auth_enc_read_write, no_access,     auth_enc_read,
    auth_enc_read, auth_enc_read,       auth_enc_read, auth_enc_read,
    auth_enc_read, auth_enc_read,       auth_enc_read};

const uint8_t rC_rC_rC_rC_rwC_rC_rC_rC_rC_rC_rC[] = {
    auth_enc_read,       auth_enc_read, auth_enc_read, auth_enc_read,
    auth_enc_read_write, auth_enc_read, auth_enc_read, auth_enc_read,
    auth_enc_read,       auth_enc_read, auth_enc_read};

// class version
#define VER_0 0x00
#define VER_1 0x01  // Profile Generic, Association SN,
#define VER_2 0x02
#define VER_3 0x03

static const myobj_struct_type myobj_list[NUM_MYOBJ_SEC] = {
    {OBJ_ASSOCIATION_LN, CLS_AssLN, OBIS_ASSOCIATION_LN, VER_3, 11, all_r,
     all_r, all_rC, all_rC},
    {OBJ_MANUFACT_ID, CLS_DATA, OBIS_MANUFACT_ID, VER_0, 2, all_n, all_r,
     all_rC, all_rC},
    {OBJ_CUSTOM_ID, CLS_DATA, OBIS_CUSTOM_ID, VER_0, 2, all_n, all_r, all_rC,
     all_rC},
    {OBJ_COUNTER_BILLING, CLS_DATA, OBIS_COUNTER_BILLING, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_NUM_AVAIL_BILLING, CLS_DATA, OBIS_NUM_AVAIL_BILLING, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_PGM_ID, CLS_DATA, OBIS_PGM_ID, VER_0, 2, all_n, all_n, all_rC, all_rC},
    {OBJ_TIME_BILLING, CLS_DATA, OBIS_TIME_BILLING, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_LOCAL_DATE, CLS_DATA, OBIS_LOCAL_DATE, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_LOCAL_TIME, CLS_DATA, OBIS_LOCAL_TIME, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_rLOAD_CTRL, CLS_ScptT, OBIS_rLOAD_CTRL, VER_0, 1, all_n, all_n, all_rC,
     all_rC},
    {OBJ_BILLING_PARM, CLS_DATA, OBIS_BILLING_PARM, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_SELECTIVE_ACT, CLS_DATA, OBIS_SELECTIVE_ACT, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_rLOAD_SIG, CLS_DATA, OBIS_rLOAD_SIG, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_CURR_TARIFF, CLS_DATA, OBIS_CURR_TARIFF, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_MAGNET_DURTIME, CLS_Reg, OBIS_MAGNET_DURTIME, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_MTCONST_ACTIVE, CLS_Reg, OBIS_MTCONST_ACTIVE, VER_0, 3, all_n, all_r,
     all_rC, all_rC},
    {OBJ_MTCONST_REACTIVE, CLS_Reg, OBIS_MTCONST_REACTIVE, VER_0, 3, all_n,
     all_r, all_rC, all_rC},
    {OBJ_MTCONST_APP, CLS_Reg, OBIS_MTCONST_APP, VER_0, 3, all_n, all_r, all_rC,
     all_rC},
    {OBJ_CUM_DEMAND, CLS_Reg, OBIS_CUM_DEMAND, VER_0, 3, all_n, all_n,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_LAST15_PF_DELI, CLS_Reg, OBIS_LAST15_PF_DELI, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_LAST15_PF_RECEI, CLS_Reg, OBIS_LAST15_PF_RECEI, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_AVGPF_DELI, CLS_Reg, OBIS_AVGPF_DELI, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_AVGPF_RECEI, CLS_Reg, OBIS_AVGPF_RECEI, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_ENERGY, CLS_Reg, OBIS_ENERGY, VER_0, 3, all_n, all_n,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_ENERGY_FWD_BOTH_REACT, CLS_Reg, OBIS_ENERGY_FWD_BOTH_REACT, VER_0, 3,
     all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_ENERGY_BWD_BOTH_REACT, CLS_Reg, OBIS_ENERGY_BWD_BOTH_REACT, VER_0, 3,
     all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MAX_DEMAND, CLS_XReg, OBIS_MAX_DEMAND, VER_0, 5, all_n, all_n,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_LCDSET_PARM, CLS_DATA, OBIS_LCDSET_PARM, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_LP_INTERVAL, CLS_Reg, OBIS_LP_INTERVAL, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_LPAVG_INTERVAL, CLS_Reg, OBIS_LPAVG_INTERVAL, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_SUBLOCKS, CLS_DATA, OBIS_MONTH_SUBLOCKS, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_CURR_LAST_DEMAND, CLS_DReg, OBIS_CURR_LAST_DEMAND, VER_0, 9, all_n,
     all_n, rC_rC_rC_rC_rC_rC_rC_rwC_rwC_rC_rC,
     rC_rC_rC_rC_rC_rC_rC_rwC_rwC_rC_rC},
    {OBJ_USER_DISP_MODE, CLS_DATA, OBIS_USER_DISP_MODE, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_SUPPLY_DISP_MODE, CLS_DATA, OBIS_SUPPLY_DISP_MODE, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PVT_DISP_MODE, CLS_DATA, OBIS_PVT_DISP_MODE, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_SAGEVT_LOG, CLS_ProfG, OBIS_SAGEVT_LOG, VER_1, 8, all_n, all_n, all_rC,
     all_rC},
    {OBJ_SWELLEVT_LOG, CLS_ProfG, OBIS_SWELLEVT_LOG, VER_1, 8, all_n, all_n,
     all_rC, all_rC},
    {OBJ_EVT_LOG, CLS_ProfG, OBIS_EVT_LOG, VER_1, 8, all_n, all_n, all_rC,
     all_rC},
    {OBJ_MTINIT_NUM, CLS_DATA, OBIS_MTINIT_NUM, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_WRONG_CONN_NUM, CLS_DATA, OBIS_WRONG_CONN_NUM, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_LOAD_PROFILE, CLS_ProfG, OBIS_LOAD_PROFILE, VER_1, 8, all_n, all_n,
     all_rC, all_rC},  // ==> association 0 에서 LP attribute=2 를 read 함
    {OBJ_LPAVG, CLS_ProfG, OBIS_LPAVG, VER_1, 8, all_n, all_n, all_rC, all_rC},
    {OBJ_sCURR_LIMIT_NUM, CLS_DATA, OBIS_sCURR_LIMIT_NUM, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_sCURR_LIMIT_VAL, CLS_Reg, OBIS_sCURR_LIMIT_VAL, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_sCURR_LIMIT2_VAL, CLS_Reg, OBIS_sCURR_LIMIT2_VAL, VER_0, 3, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_SAG_VAL_SET, CLS_Reg, OBIS_SAG_VAL_SET, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_SAG_CNT, CLS_DATA, OBIS_SAG_CNT, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_SAG_TIME_SET, CLS_Reg, OBIS_SAG_TIME_SET, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_SWELL_VAL_SET, CLS_Reg, OBIS_SWELL_VAL_SET, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_SWELL_CNT, CLS_DATA, OBIS_SWELL_CNT, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_SWELL_TIME_SET, CLS_Reg, OBIS_SWELL_TIME_SET, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_TCOVER_OPEN_NUM, CLS_DATA, OBIS_TCOVER_OPEN_NUM, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_MAGNET_DET_NUM, CLS_DATA, OBIS_MAGNET_DET_NUM, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_TEMP_THRSHLD, CLS_Reg, OBIS_TEMP_THRSHLD, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_TEMP_OVER, CLS_XReg, OBIS_TEMP_OVER, VER_0, 5, all_n, all_n, all_rC,
     all_rC},
    {OBJ_rLOAD_NUM, CLS_DATA, OBIS_rLOAD_NUM, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_INST_POWER, CLS_Reg, OBIS_INST_POWER, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_INST_POWER_L1, CLS_Reg, OBIS_INST_POWER_L1, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_POWER_L2, CLS_Reg, OBIS_INST_POWER_L2, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_POWER_L3, CLS_Reg, OBIS_INST_POWER_L3, VER_0, 3, all_n, all_n,
     all_rC, all_rC},

    {OBJ_INST_PHASE_U12, CLS_Reg, OBIS_INST_PHASE_U12, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_PHASE_U13, CLS_Reg, OBIS_INST_PHASE_U13, VER_0, 3, all_n, all_n,
     all_rC, all_rC},

    {OBJ_AVG_VOLT_L1_L2, CLS_Reg, OBIS_AVG_VOLT_L1_L2, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_AVG_VOLT_L2_L3, CLS_Reg, OBIS_AVG_VOLT_L2_L3, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_AVG_VOLT_L3_L1, CLS_Reg, OBIS_AVG_VOLT_L3_L1, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_FREQ, CLS_Reg, OBIS_INST_FREQ, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_INST_PROFILE, CLS_ProfG, OBIS_INST_PROFILE, VER_1, 8, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_CURR_L1, CLS_Reg, OBIS_INST_CURR_L1, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_voltTHD_L1, CLS_Reg, OBIS_INST_voltTHD_L1, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_VOLT_L1, CLS_Reg, OBIS_INST_VOLT_L1, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_PF_L1, CLS_Reg, OBIS_INST_PF_L1, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_AVG_CURR_L1, CLS_Reg, OBIS_AVG_CURR_L1, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_AVG_VOLT_L1, CLS_Reg, OBIS_AVG_VOLT_L1, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_IMAXLOAD_L1, CLS_XReg, OBIS_IMAXLOAD_L1, VER_0, 5, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_PHASE_L1, CLS_Reg, OBIS_INST_PHASE_L1, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
#if 1 /* bccho, 2024-09-05, 삼상 */
    {OBJ_INST_CURR_L2, CLS_Reg, OBIS_INST_CURR_L2, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_voltTHD_L2, CLS_Reg, OBIS_INST_voltTHD_L2, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_VOLT_L2, CLS_Reg, OBIS_INST_VOLT_L2, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_PF_L2, CLS_Reg, OBIS_INST_PF_L2, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_AVG_CURR_L2, CLS_Reg, OBIS_AVG_CURR_L2, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_AVG_VOLT_L2, CLS_Reg, OBIS_AVG_VOLT_L2, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_IMAXLOAD_L2, CLS_XReg, OBIS_IMAXLOAD_L2, VER_0, 5, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_PHASE_L2, CLS_Reg, OBIS_INST_PHASE_L2, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_CURR_L3, CLS_Reg, OBIS_INST_CURR_L3, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_voltTHD_L3, CLS_Reg, OBIS_INST_voltTHD_L3, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_VOLT_L3, CLS_Reg, OBIS_INST_VOLT_L3, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_PF_L3, CLS_Reg, OBIS_INST_PF_L3, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_AVG_CURR_L3, CLS_Reg, OBIS_AVG_CURR_L3, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_AVG_VOLT_L3, CLS_Reg, OBIS_AVG_VOLT_L3, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_IMAXLOAD_L3, CLS_XReg, OBIS_IMAXLOAD_L3, VER_0, 5, all_n, all_n,
     all_rC, all_rC},
    {OBJ_INST_PHASE_L3, CLS_Reg, OBIS_INST_PHASE_L3, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
#endif
    {OBJ_BAT_INSTALL_TIME, CLS_DATA, OBIS_BAT_INSTALL_TIME, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_DEVICE_ID, CLS_DATA, OBIS_DEVICE_ID, VER_0, 2, all_r, all_r, all_rC,
     all_rC},
    {OBJ_DATE_TIME, CLS_CLOCK, OBIS_DATE_TIME, VER_0, 9, all_n, all_n,
     rC_rwC_rC_rC_rwC_rwC_rwC_rwC_rC_rC_rC,
     rC_rwC_rC_rC_rwC_rwC_rwC_rwC_rC_rC_rC},
    {OBJ_DEVICE_CMD, CLS_ScptT, OBIS_DEVICE_CMD, VER_0, 1, all_n, all_n, all_rC,
     all_rC},
    {OBJ_DEVICE_BCMD, CLS_ScptT, OBIS_DEVICE_BCMD, VER_0, 1, all_n, all_n,
     all_rC, all_rC},
    {OBJ_HOLIDAYS, CLS_SpDayT, OBIS_HOLIDAYS, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_TOU_CAL, CLS_ActCal, OBIS_TOU_CAL, VER_0, 10, all_n, all_n,
     rC_rC_rC_rC_rC_rwC_rwC_rwC_rwC_rwC_rC,
     rC_rC_rC_rC_rC_rwC_rwC_rwC_rwC_rwC_rC},
    {OBJ_PERIOD_BILLDATE, CLS_SglActS, OBIS_PERIOD_BILLDATE, VER_0, 4, all_n,
     all_r, rC_rwC_rC_rwC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rwC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_NPERIOD_BILLDATE, CLS_SglActS, OBIS_NPERIOD_BILLDATE, VER_0, 4, all_n,
     all_n, rC_rwC_rC_rwC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rwC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_HDLC_SETUP, CLS_HdlcS, OBIS_HDLC_SETUP, VER_0, 9, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PGM_CHG_NUM, CLS_DATA, OBIS_PGM_CHG_NUM, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_LAST_PGM_CHG, CLS_DATA, OBIS_LAST_PGM_CHG, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_FUTURE_PGM_CHG, CLS_DATA, OBIS_FUTURE_PGM_CHG, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_OUT_SIG_SEL, CLS_DATA, OBIS_OUT_SIG_SEL, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PWR_FAIL_NUM, CLS_DATA, OBIS_PWR_FAIL_NUM, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_BAT_USE_TIME, CLS_DATA, OBIS_BAT_USE_TIME, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_ERR_CODE_1, CLS_DATA, OBIS_ERR_CODE_1, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_ERR_CODE_2, CLS_DATA, OBIS_ERR_CODE_2, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_ERR_CODE_3, CLS_DATA, OBIS_ERR_CODE_3, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_LP_STATUS, CLS_DATA, OBIS_LP_STATUS, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_MONTH_ENERGY_DELI, CLS_ProfG, OBIS_MONTH_ENERGY_DELI, VER_1, 8, all_n,
     all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_MAXDM_DELI, CLS_ProfG, OBIS_MONTH_MAXDM_DELI, VER_1, 8, all_n,
     all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_ENERGY_RECEI, CLS_ProfG, OBIS_MONTH_ENERGY_RECEI, VER_1, 8,
     all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_MAXDM_RECEI, CLS_ProfG, OBIS_MONTH_MAXDM_RECEI, VER_1, 8, all_n,
     all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_ENDOF_BILLING, CLS_ScptT, OBIS_ENDOF_BILLING, VER_0, 1, all_n, all_n,
     all_rC, all_rC},
    {OBJ_TARIFF_SCRIPT, CLS_ScptT, OBIS_TARIFF_SCRIPT, VER_0, 1, all_n, all_n,
     all_rC, all_rC},
    {OBJ_TS_SCRIPT_TABLE, CLS_ScptT, OBIS_TS_SCRIPT_TABLE, VER_0, 1, all_n,
     all_n, all_rC, all_rC},
    {OBJ_RTC_CHG_NUM, CLS_DATA, OBIS_RTC_CHG_NUM, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_aDR_NUM, CLS_DATA, OBIS_aDR_NUM, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_mDR_NUM, CLS_DATA, OBIS_mDR_NUM, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_SR_NUM, CLS_DATA, OBIS_SR_NUM, VER_0, 2, all_n, all_n, all_rC, all_rC},
    {OBJ_sCURR_nonSEL_NUM, CLS_DATA, OBIS_sCURR_nonSEL_NUM, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_sCURR_autoRTN_VAL, CLS_DATA, OBIS_sCURR_autoRTN_VAL, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_TS_CONF, CLS_SglActS, OBIS_TS_CONF, VER_0, 4, all_n, all_n,
     rC_rwC_rC_rwC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rwC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_CONDENSOR_INST, CLS_DATA, OBIS_CONDENSOR_INST, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_REM_ENERGY, CLS_Reg, OBIS_REM_ENERGY, VER_0, 3, all_n, all_n,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_REM_MONEY, CLS_Reg, OBIS_REM_MONEY, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_REM_TIME, CLS_DATA, OBIS_REM_TIME, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_BUY_ENERGY, CLS_Reg, OBIS_BUY_ENERGY, VER_0, 3, all_n, all_n,
     rC_rwCS_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_BUY_MONEY, CLS_Reg, OBIS_BUY_MONEY, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_CURR_TEMP, CLS_Reg, OBIS_CURR_TEMP, VER_0, 3, all_n, all_n, all_rC,
     all_rC},
    {OBJ_SEL_REACT, CLS_DATA, OBIS_SEL_REACT, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_LP_OVERLAPED_INDEX, CLS_DATA, OBIS_LP_OVERLAPED_INDEX, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_KEY_VALUE, CLS_DATA, OBIS_KEY_VALUE, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_LCD_MAP, CLS_DATA, OBIS_LCD_MAP, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_sCURR_HOLD, CLS_Reg, OBIS_sCURR_HOLD, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_sCURR_RECOVER_N1, CLS_Reg, OBIS_sCURR_RECOVER_N1, VER_0, 3, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_sCURR_RECOVER_N2, CLS_Reg, OBIS_sCURR_RECOVER_N2, VER_0, 3, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_sCURR_COUNTER_N1, CLS_DATA, OBIS_sCURR_COUNTER_N1, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_LATCHON_COUNTER, CLS_DATA, OBIS_LATCHON_COUNTER, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_MIN_INST_FREQ, CLS_XReg, OBIS_MIN_INST_FREQ, VER_0, 5, all_n, all_n,
     all_rC, all_rC},
    {OBJ_MAX_INST_FREQ, CLS_XReg, OBIS_MAX_INST_FREQ, VER_0, 5, all_n, all_n,
     all_rC, all_rC},
#if 1 /* bccho, 2024-09-05, 삼상 */
    {OBJ_MIN_INST_VOLT_L1, CLS_XReg, OBIS_MIN_INST_VOLT_L1, VER_0, 5, all_n,
     all_n, all_rC, all_rC},
    {OBJ_MAX_INST_VOLT_L1, CLS_XReg, OBIS_MAX_INST_VOLT_L1, VER_0, 5, all_n,
     all_n, all_rC, all_rC},
    {OBJ_MIN_INST_VOLT_L2, CLS_XReg, OBIS_MIN_INST_VOLT_L2, VER_0, 5, all_n,
     all_n, all_rC, all_rC},
    {OBJ_MAX_INST_VOLT_L2, CLS_XReg, OBIS_MAX_INST_VOLT_L2, VER_0, 5, all_n,
     all_n, all_rC, all_rC},
    {OBJ_MIN_INST_VOLT_L3, CLS_XReg, OBIS_MIN_INST_VOLT_L3, VER_0, 5, all_n,
     all_n, all_rC, all_rC},
    {OBJ_MAX_INST_VOLT_L3, CLS_XReg, OBIS_MAX_INST_VOLT_L3, VER_0, 5, all_n,
     all_n, all_rC, all_rC},
#else
    {OBJ_MIN_INST_VOLT, CLS_XReg, OBIS_MIN_INST_VOLT, VER_0, 5, all_n, all_n,
     all_rC, all_rC},
    {OBJ_MAX_INST_VOLT, CLS_XReg, OBIS_MAX_INST_VOLT, VER_0, 5, all_n, all_n,
     all_rC, all_rC},
#endif
    {OBJ_OVERCURR_ENABLE, CLS_DATA, OBIS_OVERCURR_ENABLE, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC}

    ,
    {OBJ_TOU_SET_CNT, CLS_DATA, OBIS_TOU_SET_CNT, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_EXT_PROG_ID, CLS_DATA, OBIS_EXT_PROG_ID, VER_0, 2, all_n, all_n,
     all_rC, all_rC}

    ,
    {OBJ_SECURITY_SETUP_3, CLS_SecSetUp, OBIS_SECURITY_SETUP_3, VER_1, 6, all_n,
     all_n, rC_rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_SECURITY_SETUP_4, CLS_SecSetUp, OBIS_SECURITY_SETUP_4, VER_1, 6, all_n,
     all_n, rC_rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC}

    ,
    {OBJ_SEC_EVT_LOG_HISTORY, CLS_ProfG, OBIS_CERT_LOG_HISTORY, VER_0, 8, all_n,
     all_n, all_rC, all_rC},
    {OBJ_SEC_EVT_LOG_NUM, CLS_DATA, OBIS_CERT_LOG_NUM, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_SEC_EVT_LOG_CASE, CLS_DATA, OBIS_CERT_LOG_CASE, VER_0, 3, all_n, all_n,
     all_rC, all_rC}

    ,
    {OBJ_SAP_ASSIGNM, CLS_SapAsg, OBIS_SAP_ASSIGNM, VER_0, 2, all_r, all_n,
     all_rC, all_rC},
    {OBJ_RTIME_P_ENERGY, CLS_DATA, OBIS_RTIME_P_ENERGY, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_RTIME_P_LP, CLS_ProfG, OBIS_RTIME_P_LP, VER_0, 8, all_n, all_n, all_rC,
     all_rC},
    {OBJ_RTIME_P_LP_INTERVAL, CLS_Reg, OBIS_RTIME_P_LP_INTERVAL, VER_0, 3,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_TOU_IMAGE_TRANSFER, CLS_ImgTr, OBIS_TOU_IMAGE_TRANSFER, VER_0, 7,
     all_n, all_n, rC_rwC_n_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_n_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_SELF_ERR_REF_VAL, CLS_Reg, OBIS_SELF_ERR_REF_VAL, VER_0, 3, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_CT_RATIO, CLS_DATA, OBIS_CT_RATIO, VER_0, 2, all_r, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PT_RATIO, CLS_DATA, OBIS_PT_RATIO, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PUL_SEL_REACT_APP, CLS_DATA, OBIS_PUL_SEL_REACT_APP, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_METERING_TYPE_SEL, CLS_DATA, OBIS_METERING_TYPE_SEL, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_EVENT_INFO, CLS_ProfG, OBIS_EVENT_INFO, VER_0, 7, all_n, all_n, all_rC,
     all_rC},
    {OBJ_SW_INFO, CLS_DATA, OBIS_SW_INFO, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_SW_APPLY_DATE, CLS_DATA, OBIS_SW_APPLY_DATE, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_SW_IMAGE_TRANSFER, CLS_ImgTr, OBIS_SW_IMAGE_TRANSFER, VER_0, 7, all_n,
     all_n, rC_rwC_n_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_n_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_RUN_MODEM_INFO, CLS_DATA, OBIS_RUN_MODEM_INFO, VER_0, 2, all_r, all_n,
     all_rC, all_rC},
    {OBJ_NMS_DMS_ID, CLS_DATA, OBIS_NMS_ID, VER_0, 2, all_n, all_r, all_rC,
     all_rC}

    ,
    {OBJ_USE_AMR_DATA_NUM, CLS_DATA, OBIS_USE_AMR_DATA_NUM, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_MONTH_ENERGY_DELI_nPRD, CLS_ProfG, OBIS_MONTH_ENERGY_DELI_nPRD, VER_1,
     8, all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_ENERGY_RECEI_nPRD, CLS_ProfG, OBIS_MONTH_ENERGY_RECEI_nPRD,
     VER_1, 8, all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_MAXDM_DELI_nPRD, CLS_ProfG, OBIS_MONTH_MAXDM_DELI_nPRD, VER_1, 8,
     all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_MAXDM_RECEI_nPRD, CLS_ProfG, OBIS_MONTH_MAXDM_RECEI_nPRD, VER_1,
     8, all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_ENERGY_DELI_SEASON, CLS_ProfG, OBIS_MONTH_ENERGY_DELI_SEASON,
     VER_1, 8, all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_ENERGY_RECEI_SEASON, CLS_ProfG, OBIS_MONTH_ENERGY_RECEI_SEASON,
     VER_1, 8, all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_MAXDM_DELI_SEASON, CLS_ProfG, OBIS_MONTH_MAXDM_DELI_SEASON,
     VER_1, 8, all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MONTH_MAXDM_RECEI_SEASON, CLS_ProfG, OBIS_MONTH_MAXDM_RECEI_SEASON,
     VER_1, 8, all_n, all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC}

    ,
    {OBJ_ENERGY_nPRD, CLS_Reg, OBIS_ENERGY_nPRD, VER_0, 3, all_n, all_n,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_ENERGY_SEASON, CLS_Reg, OBIS_ENERGY_SEASON, VER_0, 3, all_n, all_n,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MAX_DEMAND_nPRD, CLS_XReg, OBIS_MAX_DEMAND_nPRD, VER_0, 5, all_n,
     all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MAX_DEMAND_SEASON, CLS_XReg, OBIS_MAX_DEMAND_SEASON, VER_0, 5, all_n,
     all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_CUM_DEMAND_nPRD, CLS_Reg, OBIS_CUM_DEMAND_nPRD, VER_0, 3, all_n, all_n,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_CUM_DEMAND_SEASON, CLS_Reg, OBIS_CUM_DEMAND_SEASON, VER_0, 3, all_n,
     all_n, rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rCS_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_AVGPF_DELI_nPRD, CLS_Reg, OBIS_AVGPF_DELI_nPRD, VER_0, 3, all_n, all_n,
     all_rC, all_rC},
    {OBJ_AVGPF_RECEI_nPRD, CLS_Reg, OBIS_AVGPF_RECEI_nPRD, VER_0, 3, all_n,
     all_n, all_rC, all_rC},
    {OBJ_AVGPF_DELI_SEASON, CLS_Reg, OBIS_AVGPF_DELI_SEASON, VER_0, 3, all_n,
     all_n, all_rC, all_rC},
    {OBJ_AVGPF_RECEI_SEASON, CLS_Reg, OBIS_AVGPF_RECEI_SEASON, VER_0, 3, all_n,
     all_n, all_rC, all_rC}

    ,
    {OBJ_ERR_CODE_4, CLS_DATA, OBIS_ERR_CODE_4, VER_0, 2, all_n, all_n, all_rC,
     all_rC},
    {OBJ_ERR_DIAGONIST_NUM, CLS_DATA, OBIS_ERR_DIAGONIST_NUM, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_PREPAY_ENABLE, CLS_DATA, OBIS_PREPAY_ENABLE, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC}

    ,
    {OBJ_PHASE_DET_CONT_TIME, CLS_Reg, OBIS_PHASE_DET_CONT_TIME, VER_0, 3,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
#if 1  // 읽기 전용  //jp.kim 24.11.14
    {OBJ_PHASE_DET_CORRECT_VAL, CLS_Reg, OBIS_PHASE_DET_CORRECT_VAL, VER_0, 3,
     all_n, all_n, all_rC, all_rC},
    {OBJ_PHASE_DET_RESULT_VAL, CLS_Reg, OBIS_PHASE_DET_RESULT_VAL, VER_0, 3,
     all_n, all_n, all_rC, all_rC},
#else
    {OBJ_PHASE_DET_CORRECT_VAL, CLS_Reg, OBIS_PHASE_DET_CORRECT_VAL, VER_0, 3,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PHASE_DET_RESULT_VAL, CLS_Reg, OBIS_PHASE_DET_RESULT_VAL, VER_0, 3,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC}
#endif
    {OBJ_PERMITxx_TIME_LIMIT, CLS_Reg, OBIS_PERMITxx_TIME_LIMIT, VER_0, 3,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_INT_PLC_MODEM_ATCMD, CLS_DATA, OBIS_INT_PLC_MODEM_ATCMD, VER_0, 2,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_INT_MODEM_ATCMD, CLS_DATA, OBIS_INT_MODEM_ATCMD, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_EXT_MODEM_ATCMD, CLS_DATA, OBIS_EXT_MODEM_ATCMD, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_INT_PLC_MODEM_ATCMD_RSP, CLS_DATA, OBIS_INT_PLC_MODEM_ATCMD_RSP, VER_0,
     2, all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_INT_MODEM_ATCMD_RSP, CLS_DATA, OBIS_INT_MODEM_ATCMD_RSP, VER_0, 2,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_EXT_MODEM_ATCMD_RSP, CLS_DATA, OBIS_EXT_MODEM_ATCMD_RSP, VER_0, 2,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC}

    ,
    {OBJ_PUSH_ACT_ERR_CODE_1, CLS_DATA, OBIS_PUSH_ACT_ERR_CODE_1, VER_0, 2,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PUSH_ACT_ERR_CODE_2, CLS_DATA, OBIS_PUSH_ACT_ERR_CODE_2, VER_0, 2,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PUSH_ACT_ERR_CODE_3, CLS_DATA, OBIS_PUSH_ACT_ERR_CODE_3, VER_0, 2,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PUSH_ACT_ERR_CODE_4, CLS_DATA, OBIS_PUSH_ACT_ERR_CODE_4, VER_0, 2,
     all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC}

    ,
    {OBJ_PUSH_SCRIPT_TABLE, CLS_ScptT, OBIS_PUSH_SCRIPT_TABLE, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_PUSH_SETUP_ERR_CODE, CLS_PushSetUp, OBIS_PUSH_SETUP_ERR_CODE, VER_0, 7,
     all_n, all_n, rC_rC_rC_rwC_rwC_rC_rC_rC_rC_rC_rC,
     rC_rC_rC_rwC_rwC_rC_rC_rC_rC_rC_rC},
    {OBJ_PUSH_SETUP_LAST_LP, CLS_PushSetUp, OBIS_PUSH_SETUP_LAST_LP, VER_0, 7,
     all_n, all_n, rC_rC_rC_rwC_rwC_rC_rC_rC_rC_rC_rC,
     rC_rC_rC_rwC_rwC_rC_rC_rC_rC_rC_rC},
    {OBJ_EXT_MODEM_ID, CLS_DATA, OBIS_EXT_MODEM_ID, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_STOCK_OP_TIMES, CLS_DATA, OBIS_STOCK_OP_TIMES, VER_0, 2, all_n, all_n,
     all_rC, all_rC}
    // #if defined(FEATURE_OLD_METER_TOU_TRANSFER)
    ,
    {OBJ_OLD_METER_TOU_TRANSFER, CLS_DATA, OBIS_OLD_METER_TOU_TRANSFER, VER_0,
     2, all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC}
    // #endif
    ,
    {OBJ_LP_TOTAL_CNT, CLS_DATA, OBIS_LP_TOTAL_CNT, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_HOLIDAY_SEL, CLS_DATA, OBIS_HOLIDAY_SEL, VER_0, 2, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_THD_PERIOD_SEL, CLS_Reg, OBIS_THD_PERIOD_SEL, VER_0, 3, all_n, all_n,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC}

    ,
    {OBJ_SW_UP_CNT_0, CLS_DATA, OBIS_SW_UP_CNT_0, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_SW_UP_CNT_1, CLS_DATA, OBIS_SW_UP_CNT_1, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_SW_UP_CNT_2, CLS_DATA, OBIS_SW_UP_CNT_2, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_SW_UP_CNT_3, CLS_DATA, OBIS_SW_UP_CNT_3, VER_0, 2, all_n, all_n,
     all_rC, all_rC}

    ,
    {OBJ_SW_UP_LOG_0, CLS_ProfG, OBIS_SW_UP_LOG_0, VER_0, 7, all_n, all_n,
     all_rC, all_rC},
    {OBJ_SW_UP_LOG_1, CLS_ProfG, OBIS_SW_UP_LOG_1, VER_0, 7, all_n, all_n,
     all_rC, all_rC},
    {OBJ_SW_UP_LOG_2, CLS_ProfG, OBIS_SW_UP_LOG_2, VER_0, 7, all_n, all_n,
     all_rC, all_rC},
    {OBJ_SW_UP_LOG_3, CLS_ProfG, OBIS_SW_UP_LOG_3, VER_0, 7, all_n, all_n,
     all_rC, all_rC}

    ,
    {OBJ_HDLC_OVERLAP_AVOID, CLS_DATA, OBIS_HDLC_OVERLAP_AVOID, VER_0, 2, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_PREPAY_LOADLIMIT_CANCEL, CLS_DATA, OBIS_PREPAY_LOADLIMIT_CANCEL, VER_0,
     2, all_n, all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_MAX_DEMAND__SIGN, CLS_DATA, OBIS_MAX_DEMAND__SIGN, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_MAX_DEMAND_nPRD__SIGN, CLS_DATA, OBIS_MAX_DEMAND_nPRD__SIGN, VER_0, 2,
     all_n, all_n, all_rC, all_rC},
    {OBJ_MAX_DEMAND_SEASON__SIGN, CLS_DATA, OBIS_MAX_DEMAND_SEASON__SIGN, VER_0,
     2, all_n, all_n, all_rC, all_rC},
    {OBJ_ENERGY__SIGN, CLS_DATA, OBIS_ENERGY__SIGN, VER_0, 2, all_n, all_n,
     all_rC, all_rC},
    {OBJ_ENERGY_nPRD__SIGN, CLS_DATA, OBIS_ENERGY_nPRD__SIGN, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
    {OBJ_ENERGY_SEASON__SIGN, CLS_DATA, OBIS_ENERGY_SEASON__SIGN, VER_0, 2,
     all_n, all_n, all_rC, all_rC},
    {OBJ_WORKING_FAULT_MIN, CLS_Reg, OBIS_WORKING_FAULT_MIN, VER_0, 3, all_n,
     all_n, rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC,
     rC_rwC_rC_rC_rC_rC_rC_rC_rC_rC_rC},
    {OBJ_TOU_ID_CHANGE_STS, CLS_DATA, OBIS_TOU_ID_CHANGE_STS, VER_0, 2, all_n,
     all_n, all_rC, all_rC},
#if 1  // jp.kim 24.10.28
    {OBJ_Working_PWR_FAIL_NUM, CLS_DATA, OBIS_Working_PWR_FAIL_NUM, VER_0, 2,
     all_n, all_n, all_rC, all_rC},
    {OBJ_Working_PWR_FAIL_LOG, CLS_ProfG, OBIS_Working_PWR_FAIL_LOG, VER_0, 7,
     all_n, all_n, all_rC, all_rC}
#endif
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
static const /*__code*/ myobj_struct_type
    myobj_list_485comm[NUM_MYOBJ_485COMM] = {
        {OBJ_ASSOCIATION_LN, CLS_AssLN, OBIS_ASSOCIATION_LN, VER_0, 8,
         r_r_r_r_r_r_rw_r_r_r, r_r_r_r_r_r_rw_r_r_r, r_r_r_r_r_r_rw_r_r_r},
        {OBJ_DEVICE_ID, CLS_DATA, OBIS_DEVICE_ID, VER_0, 2, all_r, all_r,
         all_r},
        {OBJ_COMM_ENABLE, CLS_DATA, OBIS_COMM_ENABLE, VER_0, 2,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r}};

static const /*__code*/ myobj_struct_type
    myobj_list_private[NUM_MYOBJ_PRIVATE] = {
        {OBJ_ASSOCIATION_LN, CLS_AssLN, OBIS_ASSOCIATION_LN, VER_0, 8,
         r_r_r_r_r_r_rw_r_r_r, r_r_r_r_r_r_rw_r_r_r, r_r_r_r_r_r_rw_r_r_r},
        {OBJ_MANUFACT_ID, CLS_DATA, OBIS_MANUFACT_ID, VER_0, 2,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r},
        {OBJ_CUSTOM_ID, CLS_DATA, OBIS_CUSTOM_ID, VER_0, 2,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r},
        {OBJ_DEVICE_ID, CLS_DATA, OBIS_DEVICE_ID, VER_0, 2,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r},
        {OBJ_DEVICE_CMD, CLS_ScptT, OBIS_DEVICE_CMD, VER_0, 1, all_r, all_r,
         all_r},
        {OBJ_CURR_TEMP, CLS_Reg, OBIS_CURR_TEMP, VER_0, 3, r_rw_r_r_r_r_r_r_r_r,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r},
        {OBJ_INST_PROFILE, CLS_ProfG, OBIS_INST_PROFILE, VER_1, 8,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r},
        {OBJ_CAL_ADJ_ACT, CLS_DATA, OBIS_CAL_ADJ_ACT, VER_0, 2,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r},
        {OBJ_DATE_TIME, CLS_CLOCK, OBIS_DATE_TIME, VER_0, 9,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r},
        {OBJ_SYS_TITLE, CLS_DATA, OBIS_SYS_TITLE, VER_0, 2, all_r, all_r},
        {OBJ_INSTALL_CERT, CLS_DATA, OBIS_INSTALL_CERT, VER_0, 2,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r},
        {OBJ_INSTALL_KEY, CLS_DATA, OBIS_INSTALL_KEY, VER_0, 2,
         r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r, r_rw_r_r_r_r_r_r_r_r},
};

/* 장치 관리자용 (Management) Logical Device */
static const myobj_struct_type
    myobj_list_dev_management[NUM_MYOBJ_DEV_MANAGEMENT] = {
        {OBJ_DEVICE_ID, CLS_DATA, OBIS_DEVICE_ID, VER_0, 2, all_r, all_r, all_n,
         all_n},
        {OBJ_ASSOCIATION_LN, CLS_AssLN, OBIS_ASSOCIATION_LN, VER_3, 11, all_r,
         all_r, all_n, all_n},
        {OBJ_SAP_ASSIGNM, CLS_SapAsg, OBIS_SAP_ASSIGNM, VER_0, 2, all_r, all_n,
         all_n, all_n},
        {OBJ_CUSTOM_ID, CLS_DATA, OBIS_CUSTOM_ID, VER_0, 2, all_n, all_r, all_n,
         all_n},
        {OBJ_NMS_DMS_ID, CLS_DATA, OBIS_NMS_ID, VER_0, 2, all_n, all_r, all_n,
         all_n},
        {OBJ_MANUFACT_ID, CLS_DATA, OBIS_MANUFACT_ID, VER_0, 2, all_n, all_r,
         all_n, all_n},
        {OBJ_MTCONST_ACTIVE, CLS_Reg, OBIS_MTCONST_ACTIVE, VER_0, 3, all_n,
         all_r, all_n, all_n},
        {OBJ_MTCONST_REACTIVE, CLS_Reg, OBIS_MTCONST_REACTIVE, VER_0, 3, all_n,
         all_r, all_n, all_n},
        {OBJ_MTCONST_APP, CLS_Reg, OBIS_MTCONST_APP, VER_0, 3, all_n, all_r,
         all_n, all_n},
        {OBJ_PERIOD_BILLDATE, CLS_SglActS, OBIS_PERIOD_BILLDATE, VER_0, 4,
         all_n, all_r, all_n, all_n},
};

extern uint8_t* touset_parse_buf;

uint8_t* appl_get_msg_pointer(void) { return appl_msg; }

void appl_set_msg_pointer(uint8_t* pmsg) { appl_msg = pmsg; }

int appl_get_appl_len(void) { return appl_len; }

void appl_set_appl_len(int len) { appl_len = len; }

void appl_set_sap(appl_sap_type sap) { appl_sap = sap; }

char* dsm_appl_sap_string(uint32_t sap)
{
    switch (sap)
    {
    case SAP_PUBLIC:
        return "public ";
    case SAP_UTILITY:
        return "utility ";
    case SAP_485COMM:
        return "485comm ";
    case SAP_PRIVATE:
        return "private ";
    case SAP_SEC_UTILITY:
        return "sec_util";
    case SAP_SEC_SITE:
        return "sec_site";
    default:
        return "Unknown ";
    }
}

char* dsm_appl_conn_string(uint32_t conn)
{
    switch (conn)
    {
    case APPL_IDLE_STATE:
        return "appl_idle    ";
    case APPL_ASSOCIATED_STATE:
        return "appl_asso    ";
    case APPL_CROSS_AUTH_STATE:
        return "appl_auth    ";
    case APPL_KEY_AGREEMENT_STATE:
        return "appl_keyagree";
    case APPL_ENC_SIGN_STATE:
        return "appl_encsign ";
    default:
        return "Unknown      ";
    }
}

uint32_t appl_get_sap(void) { return appl_sap; }

appl_state_type appl_get_conn_state(void) { return appl_conn_state; }

void appl_set_conn_state(appl_state_type state) { appl_conn_state = state; }

bool appl_is_asso_for_sec_site(void)
{
    bool ret = false;

    if ((appl_get_sap() == SAP_SEC_SITE) &&
        (appl_get_conn_state() >= APPL_KEY_AGREEMENT_STATE))
    {
        ret = true;
    }

    return ret;
}

bool appl_is_asso_for_sec_utility(void)
{
    bool ret = false;

    if ((appl_get_sap() == SAP_SEC_UTILITY) &&
        (appl_get_conn_state() >= APPL_KEY_AGREEMENT_STATE))
    {
        ret = true;
    }

    return ret;
}

bool appl_is_asso_utility(void)
{
    bool ret = false;

    if ((appl_get_sap() == SAP_UTILITY) &&
        (appl_get_conn_state() >= APPL_ASSOCIATED_STATE))
    {
        ret = true;
    }

    return ret;
}

uint32_t g_sec_timer_start_asso_for_sec_utility = 0;
bool g_asso_timer_start_flag = 0;

uint32_t g_sec_timer_start_asso_4_485_for_sec_utility = 0;
bool g_asso_4_485_timer_start_flag = 0;

void appl_update_sec_for_timestart_sec_utility(uint32_t sec)
{
    DPRINTF(DBG_TRACE, "%s sec[%d]\r\n", __func__, sec);
    g_sec_timer_start_asso_for_sec_utility = sec;
}

uint32_t appl_get_sec_for_timestart_sec_utility(void)
{
    return g_sec_timer_start_asso_for_sec_utility;
}

uint32_t appl_get_sec_timeout_for_sec_utility(void)
{
    return (MT_TIMEOUT_ASSO_TIME / 1000);
}

void appl_set_asso_timestart_flag(bool val) { g_asso_timer_start_flag = val; }

bool appl_get_asso_timestart_flag(void) { return g_asso_timer_start_flag; }

void appl_update_sec_4_485_for_timestart_sec_utility(uint32_t sec)
{
    DPRINTF(DBG_TRACE, "%s sec[%d]\r\n", __func__, sec);
    g_sec_timer_start_asso_4_485_for_sec_utility = sec;
}

uint32_t appl_get_sec_4_485_for_timestart_sec_utility(void)
{
    return g_sec_timer_start_asso_4_485_for_sec_utility;
}

uint32_t appl_get_sec_4_485_timeout_for_sec_utility(void)
{
    return (MT_TIMEOUT_ASSO_4_485_TIME / 1000);  // sec 단위
}

void appl_set_asso_4_485_timestart_flag(bool val)
{
    g_asso_4_485_timer_start_flag = val;
}

bool appl_get_asso_4_485_timestart_flag(void)
{
    return g_asso_4_485_timer_start_flag;
}

void appl_init(void)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    appl_conn_state = APPL_IDLE_STATE;

    pPdu = adjust_tptr(&global_buff[0]);
    appl_tbuff = adjust_tptr(&cli_buff[0]);

    appl_var_init();

    appl_util_pwd_retore();
    appl_485comm_pwd_retore();
    appl_priv_pwd_retore();

    touset_parse_buf = cli_buff;
}

void appl_conn_ind(uint8_t client)
{
    appl_conn_state = APPL_IDLE_STATE;

    appl_sap_proc(client);
}

void appl_disc_ind(bool reason, bool cmdsnrm)
{
    if (reason)
    {
        if ((appl_conn_state >= APPL_ASSOCIATED_STATE) || (cmdsnrm))
        {
            if (pdl_set_bits & SETBITS_MODEM_BAUD)
            {
                pdl_set_bits &= ~SETBITS_MODEM_BAUD;

                mdm_conf_delay_until_txcomplete = true;
                mdm_baud = prog_dl.baud;
            }

            if (act_devcmd == DEVICE_CMD_INIT)
            {
                mt_init_delay_until_txcomplete = true;

                if (appl_is_sap_public() ||
                    appl_is_sap_utility())  // jp.kim 24.11.07
                {
                    sap_is_utility_when_mt_init = true;
                }
                else if (appl_is_sap_sec_utility())
                {
                    sap_is_utility_when_mt_init = true;
                }
                else if (appl_is_sap_sec_site())
                {
                    sap_is_utility_when_mt_init = true;
                }

                else
                {
                    sap_is_utility_when_mt_init = false;
                }
                DPRINTF(DBG_WARN,
                        "%s: mt_init_delay_until_txcomplete[%d], "
                        "sap_is_utility_when_mt_init[%d]\r\n",
                        __func__, mt_init_delay_until_txcomplete,
                        sap_is_utility_when_mt_init);
            }

            if (comm_en_coveropen_changed)
            {
                comm_en_coveropen_changed = false;
                comm_en_coveropen = comm_en_coveropen_val;
            }

            if (lp_mtdir_is_chged())
            {
                lp_mtdir_init();
                lp_record_move_while_blocked();
            }
        }
    }

    prog_dl_backup_save();
    tou_set_cnt_save();
    prog_dlcmd_avail = false;
    memset(SYS_TITLE_client, 0x00, SYS_TITLE_LEN);
    appl_conn_state = APPL_IDLE_STATE;
}

void appl_msg_data_ind(uint8_t* buf, int len)
{
    int idx;
    aarq_rslt_type rslt;

    idx = 0;

    /* bccho, 2024-09-05, 삼상 */
    G_SIGNING_OK = 0;

    appl_msg = buf;
    appl_len = len;

    if (appl_conn_state == APPL_IDLE_STATE)
    {
        if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
        {
            DPRINT_HEX(DBG_TRACE, "AARQ_SEC", appl_msg, appl_len, DUMP_DLMS);
            rslt = appl_AARQ_asso3_proc(idx);
        }
        else
        {
            rslt = appl_AARQ_proc(idx);
        }

        DPRINTF(DBG_TRACE, _D "AARQ: result[%d]\r\n", rslt);
        if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
        {
            if (rslt != AARQ_SERVICE_NOT)
            {
                if (appl_AARE_asso3(rslt))
                {
                    appl_conn_state = APPL_ASSOCIATED_STATE;
                    appl_var_init();
                }
            }
            else
            {
                appl_confirmed_svcerr_resp(3, 2);
            }
        }
        else
        {
            if (rslt != AARQ_SERVICE_NOT)
            {
                if (appl_AARE(rslt))
                {
                    appl_conn_state = APPL_ASSOCIATED_STATE;
                    appl_var_init();
                }
            }
            else
            {
                appl_confirmed_svcerr_resp(3, 2);
            }
        }
    }
    else
    {
#if 1 /* bccho, HLS, 2023-08-14, hw fault 발생 */
        T_DLMS_SC sc = {
            0,
        };
        dsm_sec_sc_field_set(&sc);
#else
        dsm_sec_sc_field_set((T_DLMS_SC*)NULL);
#endif

        if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
        {
            DPRINTF(DBG_TRACE, "%s: appl_conn_state[%d]\r\n", __func__,
                    appl_conn_state);
            DPRINT_HEX(DBG_TRACE, "APDU_SEC", appl_msg, appl_len, DUMP_DLMS);

            if (appl_conn_state == APPL_ASSOCIATED_STATE || appl_conn_state ==
                                                                APPL_CROSS_AUTH_STATE /*|| appl_conn_state == APPL_KEY_AGREEMENT_STATE*/)
            {
                appl_reqtype = (appl_req_type)appl_msg[idx++];
                appl_reqchoice = appl_msg[idx++];
                appl_invokeid_pri = appl_msg[idx++];

                DPRINTF(
                    DBG_TRACE,
                    _D
                    "%s: reqtype[0x%02X], choice[0x%02X], invoke[0x%02X]\r\n",
                    __func__, appl_reqtype, appl_reqchoice, appl_invokeid_pri);

                if (appl_reqtype_act_check(appl_reqtype))
                {
                    appl_msg_proc(idx);
                    dsm_sec_invocation_count_reset();
                }
                else if (appl_reqtype_sec_check(appl_reqtype))
                {
                    appl_exception_resp(SER_STATE_NOT_ALLOWED,
                                        SER_ERR_OTHER_REASON);
                }
                else
                {
                    appl_confirmed_svcerr_resp(
                        3, 2);  // choice(3): service, enum(2): service
                                // unsupported as described in confirmance block
                }
            }
            else if (appl_conn_state == APPL_KEY_AGREEMENT_STATE ||
                     appl_conn_state == APPL_ENC_SIGN_STATE)
            {
                uint16_t dec_len;
                appl_reqtype = (appl_req_type)appl_msg[idx];

                if (appl_conn_state == APPL_KEY_AGREEMENT_STATE)
                {
                    if (appl_reqtype_sec_check(appl_reqtype))
                        appl_conn_state = APPL_ENC_SIGN_STATE;
                }

                dsm_meter_sw_timer_stop(MT_SW_TIMER_ASSO_TO);
                dsm_meter_sw_timer_start(MT_SW_TIMER_ASSO_TO, FALSE,
                                         MT_TIMEOUT_ASSO_TIME);
                appl_set_asso_timestart_flag(true);

                appl_update_sec_for_timestart_sec_utility(dsm_rtc_get_time());
                DPRINTF(DBG_TRACE, "%s: TIMER START - asso_3_4 %d sec\r\n",
                        __func__, appl_get_sec_for_timestart_sec_utility());

                if (appl_is_asso_for_sec_site())
                {
                    // ASSO4 인 경우
                    // 5분간 현장(asso_4) 485 통신이 안되면-> asso_4
                    // disconnection  ===============================
                    // JP.KIM 24.10.08
                    dsm_meter_sw_timer_stop(MT_SW_TIMER_ASSO_4_485_TO);
                    dsm_meter_sw_timer_start(MT_SW_TIMER_ASSO_4_485_TO, FALSE,
                                             MT_TIMEOUT_ASSO_4_485_TIME);
                    appl_set_asso_4_485_timestart_flag(true);
                    appl_update_sec_4_485_for_timestart_sec_utility(
                        dsm_rtc_get_time());
                    DPRINTF(DBG_TRACE, "%s: TIMER START - asso_4 %d sec\r\n",
                            __func__,
                            appl_get_sec_4_485_for_timestart_sec_utility());
                }

                if (appl_reqtype_sec_ds_check(appl_reqtype))
                {
                    int16_t sign_ok = 0;
                    uint8_t ciphered_idx = 0;
                    uint16_t cipher_len = 0;

                    sign_ok = dsm_sec_g_signing_xDLMS_APDU_parser(
                        &appl_msg[idx], 0, &ciphered_idx, &cipher_len);

                    if (sign_ok)
                    {
                        // DPRINT_HEX(DBG_TRACE, "RCV_CIPHERED_MSG",
                        // &appl_msg[ciphered_idx], cipher_len, DUMP_ALWAYS);
                        DPRINT_HEX(DBG_TRACE, "Cipher MSG",
                                   &appl_msg[ciphered_idx], cipher_len,
                                   DUMP_ALWAYS);  // Encrypted APDU
                        if (appl_reqtype_sec_check(appl_msg[ciphered_idx]))
                        {
                            int16_t dec_ok = 0;

                            dec_ok = dsm_sec_ciphering_xDLMS_APDU_parser(
                                &appl_msg[ciphered_idx], cipher_len,
                                &appl_msg[0], &dec_len);

                            /* bccho, 2024-09-05, 삼상 */
                            G_SIGNING_OK = 1;
                            if (dec_ok)
                            {
                                dsm_sec_invocation_count_add();
                                idx = 0;
                                appl_len = dec_len;
                                appl_msg_process(&appl_msg[idx]);
                            }
                            else
                            {
                                appl_exception_resp(SER_STATE_UNKNOWN,
                                                    SER_ERR_DECIPER_ERR);
                            }
                        }
                    }
                    else
                    {
                        appl_exception_resp(SER_STATE_UNKNOWN,
                                            SER_ERR_DECIPER_ERR);
                    }
                }
                else if (appl_reqtype_sec_check(appl_reqtype))
                {
                    int16_t dec_ok = 0;

                    dec_ok = dsm_sec_ciphering_xDLMS_APDU_parser(
                        &appl_msg[idx], 0, &appl_msg[0], &dec_len);

                    if (dec_ok)
                    {
                        dsm_sec_invocation_count_add();
                        idx = 0;
                        appl_len = dec_len;
                        appl_msg_process(&appl_msg[idx]);
                    }
                    else
                    {
                        appl_exception_resp(SER_STATE_UNKNOWN,
                                            SER_ERR_DECIPER_ERR);
                    }
                }
                else
                {
                    appl_exception_resp(SER_STATE_NOT_ALLOWED,
                                        SER_ERR_OTHER_REASON);
                }
            }
        }
        else
        {
            dsm_meter_sw_timer_stop(MT_SW_TIMER_ASSO_TO);
            dsm_meter_sw_timer_start(MT_SW_TIMER_ASSO_TO, FALSE,
                                     MT_TIMEOUT_ASSO_TIME);
            appl_set_asso_timestart_flag(true);

            appl_update_sec_for_timestart_sec_utility(dsm_rtc_get_time());
            DPRINTF(DBG_TRACE, "%s: TIMER START - asso_1_2 %d sec\r\n",
                    __func__, appl_get_sec_for_timestart_sec_utility());

            if (appl_is_asso_for_sec_site())
            {
                // ASSO4 인 경우
                // 5분간 현장(asso_4) 485 통신이 안되면-> asso_4 disconnection
                // ===============================	JP.KIM 24.10.08
                dsm_meter_sw_timer_stop(MT_SW_TIMER_ASSO_4_485_TO);
                dsm_meter_sw_timer_start(MT_SW_TIMER_ASSO_4_485_TO, FALSE,
                                         MT_TIMEOUT_ASSO_4_485_TIME);  // 5 min
                appl_set_asso_4_485_timestart_flag(true);
                appl_update_sec_4_485_for_timestart_sec_utility(
                    dsm_rtc_get_time());
                DPRINTF(DBG_TRACE, "%s: TIMER START - asso_4 %d sec\r\n",
                        __func__,
                        appl_get_sec_4_485_for_timestart_sec_utility());
            }

            appl_msg_process(&appl_msg[idx]);
        }
    }

    dsm_sec_set_operation(false);
}

void appl_msg_process(uint8_t* pbuff)
{
    uint16_t idx = 0;

    appl_reqtype = (appl_req_type)pbuff[idx++];
    appl_reqchoice = pbuff[idx++];
    appl_invokeid_pri = pbuff[idx++];

    DPRINTF(DBG_TRACE,
            _D "%s: type[0x%02X], choice[0x%02X], invoke[0x%02X]\r\n", __func__,
            appl_reqtype, appl_reqchoice, appl_invokeid_pri);

    if (appl_reqtype_check(appl_reqtype))
    {
        appl_msg_proc(idx);
    }
    else
    {
        appl_confirmed_svcerr_resp(
            3, 2);  // choice(3): service, enum(2): service unsupported as
                    // described in confirmance block
    }
}

myobj_struct_type* appl_get_object(uint16_t obj_idx)
{
    DPRINTF(DBG_TRACE, "%s: obj_idx[%d]\r\n", __func__, obj_idx);
    return (myobj_struct_type*)&myobj_list[obj_idx];
}

appl_result_type appl_obj_enum_and_acs_check(void)
{
    /* 오브젝트 액세스 권한 확인 */

    int i, j;
    const /*__code*/ uint8_t* aso_att;
    const /*__code*/ myobj_struct_type* object;

    if (appl_is_sap_public() ||
        appl_is_sap_utility()) /* CLIENT_ADDR_PUBLIC or CLIENT_ADDR_UTILITY */
    {
        /* Association 1, 2 */
        j = NUM_MYOBJ_DEV_MANAGEMENT;
        object = &myobj_list_dev_management[0];
    }
    else if (appl_is_sap_sec_utility() ||
             appl_is_sap_sec_site()) /* CLIENT_ADDR_ASSO3_UTILITY or
                                        CLIENT_ADDR_ASSO3_SITE */
    {
        /* Association 3, 4 */
        j = NUM_MYOBJ_SEC;
        object = &myobj_list[0];
    }
    else if (appl_is_sap_485comm()) /* CLIENT_ADDR_485COMM */
    {
        j = NUM_MYOBJ_485COMM;
        object = &myobj_list_485comm[0];
    }
    else /* CLIENT_ADDR_PRIVATE (SAP_ADDR_PRIVATE) */
    {
        j = NUM_MYOBJ_PRIVATE;
        object = &myobj_list_private[0];
    }
    char* str_sap = 0;
    switch (appl_sap)
    {
    case SAP_PUBLIC:
        str_sap = "Public";
        break;
    case SAP_UTILITY:
        str_sap = "Utility";
        break;
    case SAP_485COMM:
        str_sap = "RS-485";
        break;
    case SAP_PRIVATE:
        str_sap = "Private";
        break;
    case SAP_SEC_UTILITY:
        str_sap = "SecUtil";
        break;
    case SAP_SEC_SITE:
        str_sap = "SecSite";
        break;
    }
    DPRINTF(DBG_TRACE, "SAP[%d]:%s, OBJ[%d]\r\n", appl_sap, str_sap, j);

    for (i = 0; i < j; i++, object++)
    {
        if (appl_class_id == object->class_id &&
            obis_ga == object->instance_id[GROUP_A] &&
            obis_gb == object->instance_id[GROUP_B] &&
            obis_gd == object->instance_id[GROUP_D])
        {
            if ((obis_gc == object->instance_id[GROUP_C]) ||
                (object->instance_id[GROUP_C] == 'm' &&
                 obis_is_enercy_ch_groupc(obis_gc, obis_gd)))
            {
                if ((obis_ge == object->instance_id[GROUP_E]) ||
                    ((object->instance_id[GROUP_E] == 't') &&
                     (obis_ge < numRates)) ||
                    ((object->instance_id[GROUP_E] == 'n') &&
                     obis_is_evtlog_range(obis_ge)) ||
                    (object->instance_id[GROUP_E] == 'e' &&
                     obis_is_asso_range(obis_ge))  // 2020_0417 apply 'c'->'e'
                    || (object->instance_id[GROUP_E] == 'e' &&
                        obis_is_fw_ch_groupe(obis_ge)))
                {
                    if ((obis_gf == object->instance_id[GROUP_F]) ||
                        (object->instance_id[GROUP_F] == 'Z'))
                        break;
                }
            }
        }
        else if (appl_class_id == object->class_id &&
                 obis_gc == object->instance_id[GROUP_C] &&
                 obis_gd == object->instance_id[GROUP_D] &&
                 obis_ge == object->instance_id[GROUP_E] &&
                 obis_gf == object->instance_id[GROUP_F])
        {
            if (obis_ga == 0x00 && object->instance_id[GROUP_B] == 'b' &&
                obis_gc == 0x00 && obis_gd == 0x02 && obis_ge == 0x00)
            {  // OBIS_SW_INFO
                if (obis_is_fw_ch_groupb(obis_gb))
                {
                    break;
                }
            }
            else if (obis_ga == 0x00 && object->instance_id[GROUP_B] == 'b' &&
                     obis_gc == 0x60 && obis_gd == 0x02 && obis_ge == 0x0d)
            {  // OBIS_SW_INFO
                if (obis_is_fw_ch_groupb(obis_gb))
                {
                    break;
                }
            }
            else if (obis_ga == 0x00 && obis_gb == 0x80 && obis_gc == 0x63 &&
                     obis_gd == 0x62 && obis_ge == 0x1A)
            {  // OBIS_SW_INFO
                DPRINTF(DBG_TRACE, "log cnt obis[%d]: %d.%d.%d.%d.%d.%d\r\n", i,
                        obis_ga, obis_gb, obis_gc, obis_gd, obis_ge, obis_gf);
                break;
            }
            else if (obis_ga == 0x01 && object->instance_id[GROUP_B] == 'b' &&
                     obis_gc == 0x00 && obis_gd == 0x01 && obis_ge == 0x01)
            {  // OBIS_USE_AMR_DATA_NUM
                if (obis_is_mr_data_type_groupb(obis_gb))
                {
                    break;
                }
            }
        }
        else if (appl_class_id == object->class_id &&
                 obis_ga == object->instance_id[GROUP_A] &&
                 obis_gd == object->instance_id[GROUP_D])
        {
            if (object->instance_id[GROUP_B] == 'b' &&
                object->instance_id[GROUP_E] == 'e' &&
                object->instance_id[GROUP_F] == 'f')
            {
                if (object->instance_id[GROUP_C] == 'c')  // energy (item data)
                {
                    break;
                }
                else if (object->instance_id[GROUP_C] ==
                         obis_gc)  // energy(month profile data)
                {
                    break;
                }
            }
            else if (object->instance_id[GROUP_B] == 'b' &&
                     object->instance_id[GROUP_F] == 'Z' &&
                     object->instance_id[GROUP_C] == obis_gc &&
                     object->instance_id[GROUP_E] == obis_ge)
            {
                if (obis_is_mr_data_type_groupb(obis_gb))
                {
                    break;
                }
            }
        }
    }

    if (i == j)
    {
        return APPL_RESULT_OBJ_UNDEF;
    }
    else
    {
        DPRINTF(DBG_INFO, "OBJ_LIST[%d], OBJ_INFO[%d]\r\n", i, object->obj);
    }

    appl_obj_id = object->obj;

    // access right check
    if (appl_att_id == 0 || appl_att_id > object->attributes_cnt)
        return APPL_RESULT_NO_ACS;

    if (appl_is_sap_public())
        aso_att = object->as0_attr;
    else if (appl_is_sap_utility())
        aso_att = object->as1_attr;
    else if (appl_is_sap_sec_utility())
        aso_att = object->as3_attr;
    else if (appl_is_sap_sec_site())
        aso_att = object->as4_attr;
    else
        aso_att = object->as0_attr;

    if (appl_reqtype != APPL_ACT_REQ)
    {
        if (*(aso_att + appl_att_id - 1) == no_access)
        {
            DPRINTF(DBG_ERR, "NO_ACCESS[%d]\r\n", i);
            return APPL_RESULT_NO_ACS;
        }
    }

    if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
    {
        appl_acs_type = *(aso_att + appl_att_id - 1);
        if (appl_reqtype == APPL_SET_REQ)
        {
            if (appl_acs_type != auth_enc_read_write &&
                appl_acs_type != auth_enc_sign_read_write)
                return APPL_RESULT_NO_ACS;
        }
    }
    else
    {
        if ((appl_reqtype == APPL_SET_REQ) &&
            *(aso_att + appl_att_id - 1) != read_and_write)
            return APPL_RESULT_NO_ACS;
    }

    return APPL_RESULT_OK;
}

myobj_struct_type* dsm_touETC_get_object(uint8_t class_id, uint8_t* obis)
{
    int i, j;
    myobj_struct_type* object;

    //    DPRINTF(DBG_NONE, "%s: [ IN_CLASS_ID[%04d] ]\r\n", __func__,
    //    class_id); DPRINT_HEX(DBG_NONE, "IN_OBIS", obis, 6, DUMP_DLMS);

    if (appl_is_sap_public() || appl_is_sap_utility())
    {
        j = NUM_MYOBJ_DEV_MANAGEMENT;
        object = (myobj_struct_type*)&myobj_list_dev_management[0];
    }
    else if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
    {
        j = NUM_MYOBJ_SEC;
        object = (myobj_struct_type*)&myobj_list[0];
    }
    else if (appl_is_sap_485comm())
    {
        j = NUM_MYOBJ_485COMM;
        object = (myobj_struct_type*)&myobj_list_485comm[0];
    }
    else
    {
        j = NUM_MYOBJ_PRIVATE;
        object = (myobj_struct_type*)&myobj_list_private[0];
    }

    for (i = 0; i < j; i++, object++)
    {
        if (class_id == object->class_id &&
            obis[GROUP_A] == object->instance_id[GROUP_A] &&
            obis[GROUP_B] == object->instance_id[GROUP_B] &&
            obis[GROUP_D] == object->instance_id[GROUP_D])
        {
            if ((obis[GROUP_C] == object->instance_id[GROUP_C]) ||
                (object->instance_id[GROUP_C] == 'm' &&
                 obis_is_enercy_ch_groupc(obis[GROUP_C], obis[GROUP_D])))
            {
                if ((obis[GROUP_E] == object->instance_id[GROUP_E]) ||
                    ((object->instance_id[GROUP_E] == 't') &&
                     (obis[GROUP_E] < numRates)) ||
                    ((object->instance_id[GROUP_E] == 'n') &&
                     obis_is_evtlog_range(obis[GROUP_E])) ||
                    (object->instance_id[GROUP_E] == 'c' &&
                     obis_is_asso_range(obis[GROUP_E])))
                {
                    if ((obis[GROUP_F] == object->instance_id[GROUP_F]) ||
                        (object->instance_id[GROUP_F] == 'Z'))
                        break;
                }
            }
        }
    }

    return object;
}

int appl_cosem_descript(int idx)
{
    // class id
    ToH16((U8_16*)&appl_class_id, &appl_msg[idx]);
    idx += 2;
    // obis code
    memcpy(appl_obis.id, &appl_msg[idx], OBIS_ID_SIZE);
    idx += OBIS_ID_SIZE;
    // attribute id
    appl_att_id = appl_msg[idx];
    idx += 1;
    DPRINTF(DBG_TRACE, "COSEM Objects:\r\n");
    DPRINTF(DBG_TRACE, "- CLASS: %d\r\n", appl_class_id);
    // DPRINTF(DBG_TRACE, "- OBIS: A[%d] B[%d] C[%d] D[%d] E[%d] F[%d]\r\n",
    // appl_obis.id[GROUP_A], appl_obis.id[GROUP_B], appl_obis.id[GROUP_C],
    // appl_obis.id[GROUP_D], appl_obis.id[GROUP_E], appl_obis.id[GROUP_F]);
    DPRINTF(DBG_TRACE, "- OBIS: %d %d %d %d %d %d\r\n", appl_obis.id[GROUP_A],
            appl_obis.id[GROUP_B], appl_obis.id[GROUP_C], appl_obis.id[GROUP_D],
            appl_obis.id[GROUP_E], appl_obis.id[GROUP_F]);
    DPRINTF(DBG_TRACE, "- Attributes: %d\r\n", appl_att_id);

    return idx;
}

void appl_selective_acs_descript(int idx)
{
    if (appl_msg[idx++] == 0)
    {
        appl_selacs_len = 0;
    }
    else
    {
        appl_selacs_len = (appl_len > idx) ? (uint8_t)(appl_len - idx) : 0;
        if (appl_selacs_len > SELECTIVE_ACS_BUFSIZE)
            appl_selacs_len = SELECTIVE_ACS_BUFSIZE;
        memcpy(&appl_selacs[0], &appl_msg[idx], appl_selacs_len);
        idx += appl_selacs_len;
    }
    DPRINTF(DBG_TRACE, "appl_selacs_len[%d]\r\n", appl_selacs_len);
    if (appl_selacs_len)
    {
        DPRINT_HEX(DBG_TRACE, "SELACS", appl_selacs, appl_selacs_len,
                   DUMP_ALWAYS);
    }
}

int appl_block_descript(int idx)
{
    // last block ?
    appl_is_last_block = appl_msg[idx];
    idx += 1;
    // block number
    ToH32((U8_16_32*)&appl_block_num, &appl_msg[idx]);
    idx += 4;

    return idx;
}

/*
    CTT 에서만 사용되는 경우 입니다.
    CTT 에서 처음 요구하는 obis 로써 응답하는 obis list 에 대해서만 obis request
   가 이루어 짐

    현재 계량기가 지원하는 obis list 를
    응답데이테 구조체에 맞줘 block response 형식으로 응답 함.
    (전체 데이터를 segment 형식으로 보내기에는 상대방도 고려해야 하기 때문에
   곤란)

    block number 는 appl_resp_block_num 변수 사용.
    block response 마다 1개의 obis 를 데이터 구조화 해서 보냄.
    (class id, version, obis code, sap 에 적용 되는 attribute 표현,)
*/
void object_list_element_proc(void)
{
    uint8_t t8;
    const uint8_t* attr;
    uint8_t attr_num;
    const myobj_struct_type* object;
    uint8_t idx_max;
    uint8_t inst_id[6];
    uint16_t clsid;

    idx_max = 1;

    if (appl_is_sap_public() || appl_is_sap_utility())
    {
        object = &myobj_list_dev_management[appl_idx1_for_block];
    }
    else if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
    {
        object = &myobj_list[appl_idx1_for_block];
    }

    else if (appl_is_sap_485comm())
    {
        object = &myobj_list_485comm[appl_idx1_for_block];
    }
    else
    {
        object = &myobj_list_private[appl_idx1_for_block];
    }

    inst_id[GROUP_A] = object->instance_id[GROUP_A];
    inst_id[GROUP_B] = object->instance_id[GROUP_B];
    inst_id[GROUP_C] = object->instance_id[GROUP_C];
    inst_id[GROUP_D] = object->instance_id[GROUP_D];
    inst_id[GROUP_E] = object->instance_id[GROUP_E];
    inst_id[GROUP_F] = object->instance_id[GROUP_F];
    if (inst_id[GROUP_C] == 'm')
    {
        if (inst_id[GROUP_E] == 't')
        {
            // 채널 과 tariff
            if (inst_id[GROUP_D] == 0x02 || inst_id[GROUP_D] == 0x06 ||
                inst_id[GROUP_D] == 0x80)
            {  // OBIS_CUM_DEMAND, OBIS_MAX_DEMAND, OBIS_MONTH_SUBLOCKS
                idx_max = NUM_GRPMT1;
                inst_id[GROUP_C] = groupce_mt1[appl_idx2_for_block].grpm;
                inst_id[GROUP_E] = groupce_mt1[appl_idx2_for_block].grpt;
            }
            else
            {
                idx_max = NUM_GRPMT;
                inst_id[GROUP_C] = groupce_mt[appl_idx2_for_block].grpm;
                inst_id[GROUP_E] = groupce_mt[appl_idx2_for_block].grpt;
            }
        }
        else
        {
            // 채널 (OBIS_CURR_LAST_DEMAND)
            idx_max = NUM_GRPCM1;
            inst_id[GROUP_C] = groupc_m1[appl_idx2_for_block];
        }
    }
    else
    {
        if (inst_id[GROUP_E] == 'n')
        {
            // 이벤트 그룹
            idx_max = NUM_GRPEN;
            inst_id[GROUP_E] = groupe_n[appl_idx2_for_block];
        }
        else if (inst_id[GROUP_E] == 't')
        {
            // tariff
            idx_max = numRates;
            inst_id[GROUP_E] = (uint8_t)appl_idx2_for_block;
        }
        else if (inst_id[GROUP_D] == 44 && inst_id[GROUP_E] == 'e')
        {
            // “Image transfe type
            idx_max = NUM_FW_GRPE;
            inst_id[GROUP_E] = groupe_fw_e[appl_idx2_for_block];
        }
        else if (inst_id[GROUP_D] == 0 && inst_id[GROUP_E] == 'e')
        {
            // association type
            // idx_max = NUM_ASSO_GRPEN;  //1,2,3,4
            if (appl_is_sap_utility())
            {
                inst_id[GROUP_E] = 2;
            }
            else if (appl_is_sap_sec_utility())
            {
                inst_id[GROUP_E] = 3;
            }
            else if (appl_is_sap_sec_site())
            {
                inst_id[GROUP_E] = 4;
            }
            else
            {
                inst_id[GROUP_E] = 1;
            }
        }
    }
    if (inst_id[GROUP_F] == 'Z')
        inst_id[GROUP_F] = 0xff;

    FILL_STRUCT(0x04);
    clsid = object->class_id;
    FILL_U16(clsid);
    FILL_U08(object->version);
    FILL_STRING(6);
    pPdu[pPdu_idx++] = inst_id[GROUP_A];
    pPdu[pPdu_idx++] = inst_id[GROUP_B];
    pPdu[pPdu_idx++] = inst_id[GROUP_C];
    pPdu[pPdu_idx++] = inst_id[GROUP_D];
    pPdu[pPdu_idx++] = inst_id[GROUP_E];
    pPdu[pPdu_idx++] = inst_id[GROUP_F];

    FILL_STRUCT(0x02);
    FILL_ARRAY(object->attributes_cnt);
    if (appl_is_sap_public())
        attr = object->as0_attr;
    else if (appl_is_sap_utility())
        attr = object->as1_attr;
    else if (appl_is_sap_sec_utility())
    {
        attr = object->as3_attr;
    }
    else if (appl_is_sap_sec_site())
    {
        attr = object->as4_attr;
    }
    else
        attr = object->as0_attr;  // other object list

    attr_num = 1;
    do
    {
        FILL_STRUCT(0x03);
        FILL_S08(attr_num);
        FILL_ENUM(*attr++);
        if ((object->obj == OBJ_LOAD_PROFILE && attr_num == 2) ||
            (object->obj == OBJ_LPAVG && attr_num == 2))
        {
            FILL_ARRAY(0x01);
            FILL_S08(0x02);
        }
        else
        {
            pPdu[pPdu_idx++] = 0;
        }
    } while (++attr_num <= object->attributes_cnt);

    FILL_ARRAY(0x00);

    if ((++appl_idx2_for_block) >= idx_max)
    {
        appl_idx1_for_block++;
        appl_idx2_for_block = 0;
    }

    if (appl_is_sap_public() || appl_is_sap_utility())
    {
        t8 = NUM_MYOBJ_DEV_MANAGEMENT;
    }
    else if (appl_is_sap_sec_site() || appl_is_sap_sec_utility())
    {
        t8 = NUM_MYOBJ_SEC;
    }
    else if (appl_is_sap_485comm())
    {
        t8 = NUM_MYOBJ_485COMM;
    }
    else
    {
        t8 = NUM_MYOBJ_PRIVATE;
    }

    if (appl_idx1_for_block < t8)
        appl_resp_last_block = 0;
    else
        appl_resp_last_block = 1;
}

#define ATCMD_RSP_BLOCK_UNIT 1024

#if 1  // jp.kim 25.01.22
void at_cmd_rsp_list_element_proc(void)
{
    U16 reclen;
    ST_ATCMD_TMP_BUF* pst_atmcd_from_modem =
        dsm_get_atcmd_from_modem(EXT_MODEM_TYPE);
    DPRINTF(DBG_TRACE, _D "%s: modem_type[%d]\r\n", __func__, EXT_MODEM_TYPE);

    reclen =
        (U16)((getresp_LP_len >= ATCMD_RSP_BLOCK_UNIT) ? ATCMD_RSP_BLOCK_UNIT
                                                       : getresp_LP_len);

    // encode to packet data unit
    // LPavg_record_to_pPdu(appl_tbuff, reclen);
    if (pst_atmcd_from_modem->len < CLIENT_ATCMD_STRING_MAX_SIZE)
    {
        DPRINTF(DBG_TRACE, _D "%s 1 pPdu_idx[%d] reclen[%d]  \r\n", __func__,
                pPdu_idx, reclen);
        memcpy(&pPdu[pPdu_idx], &pst_atmcd_from_modem->string[getresp_LP_index],
               reclen);
        DPRINT_HEX(DBG_TRACE, "AT_MODEM_STRING", &pPdu[pPdu_idx], reclen,
                   DUMP_ALWAYS);
        pPdu_idx += reclen;
        DPRINTF(DBG_TRACE, _D "%s 2 pPdu_idx[%d] reclen[%d]  \r\n", __func__,
                pPdu_idx, reclen);
        // fill_octet_string_x(&pst_atmcd_from_modem->string[getresp_LP_index],
        // reclen);
    }

    getresp_LP_index += (U32)reclen;
    getresp_LP_len -= (U16)reclen;

    DPRINTF(DBG_TRACE, _D "%s getresp_LP_len[%d] getresp_LP_index[%d]  \r\n",
            __func__, getresp_LP_len, getresp_LP_index);

    if (getresp_LP_len <= ATCMD_RSP_BLOCK_UNIT)
        appl_resp_last_block = 1;  // last block
    else
        appl_resp_last_block = 0;  // non-last block
}
#endif

static void appl_msg_proc(int idx)
{
    appl_resp_result = RESULT_OK;

    switch (appl_reqtype)
    {  // apdu[0]
    case APPL_GET_REQ:
        DPRINTF(DBG_TRACE, _D "Get Req\r\n");
        approc_get_req(idx);
        break;
    case APPL_SET_REQ:
        DPRINTF(DBG_TRACE, _D "Set Req\r\n");
        approc_set_req(idx);
        break;
    case APPL_ACT_REQ:
        DPRINTF(DBG_TRACE, _D "Act Req\r\n");
        approc_act_req(idx);
        break;
    default:  // filtered in appl_reqtype_check()
        break;
    }
}

static void appl_sap_proc(uint8_t client)
{
    switch (client)
    {
    case CLIENT_ADDR_PUBLIC:
        appl_sap = SAP_PUBLIC;
        g_sap_assin_run = SAP_ASSIGN_DEV_MANAGEMENT;
        break;
    case CLIENT_ADDR_UTILITY:
        appl_sap = SAP_UTILITY;
        g_sap_assin_run = SAP_ASSIGN_DEV_MANAGEMENT;
        break;
    case CLIENT_ADDR_PRIVATE:
        appl_sap = SAP_PRIVATE;
        break;
    case CLIENT_ADDR_485COMM:
        appl_sap = SAP_485COMM;
        g_sap_assin_run = SAP_ASSIGN_DEV_MANAGEMENT;
        break;
    case CLIENT_ADDR_ASSO3_UTILITY:
        appl_sap = SAP_SEC_UTILITY;
        g_sap_assin_run = SAP_ASSIGN_KEPCO_MANAGEMENT;
        break;
    case CLIENT_ADDR_ASSO3_SITE:
        appl_sap = SAP_SEC_SITE;
        g_sap_assin_run = SAP_ASSIGN_KEPCO_MANAGEMENT;
        break;
    }
    if (appl_sap == SAP_SEC_UTILITY || appl_sap == SAP_SEC_SITE)
        dsm_sec_random_generate();
}

static void appl_var_init(void)
{
    appl_whm_inf_collected = false;
    appl_mdm_baud_changed = false;
    act_cmd_sel_ap = 0;  // default = select active program
    act_devcmd = DEVICE_CMD_NULL;
    lpinfo_is_caped = false;

    pdl_set_bits_clr();

    prog_dlcmd_avail = false;
}

static bool appl_reqtype_check(appl_req_type req)
{
    if (req == APPL_GET_REQ || req == APPL_SET_REQ || req == APPL_ACT_REQ)
        return true;

    return false;
}

static bool appl_reqtype_act_check(appl_req_type req)
{
    if (req == APPL_ACT_REQ)
        return true;

    return false;
}

static bool appl_reqtype_sec_check(appl_req_type req)
{
    if (req == APPL_GLO_GET_REQ || req == APPL_GLO_SET_REQ ||
        req == APPL_GLO_ACT_REQ || req == APPL_GENERAL_SIGNING)
        return true;

    return false;
}

static bool appl_reqtype_sec_ds_check(appl_req_type req)
{
    if (req == APPL_GENERAL_SIGNING)
    {
        return true;
    }

    return false;
}

static bool obis_is_fw_ch_groupb(uint8_t grpb)
{
    int i;

    for (i = 0; i < NUM_FW_GRPBB; i++)
    {
        if (grpb == groupb_fw_b[i])
            break;
    }
    return (i < NUM_FW_GRPBB) ? true : false;
}

uint32_t obis_get_fw_info_type_group_b(uint8_t grpb)
{
    int i;
    uint32_t fwinfo_type = (uint32_t)FWINFO_MAX_NUM;

    for (i = 0; i < NUM_FW_GRPBB; i++)
    {
        if (grpb == groupb_fw_b[i])
        {
            switch (grpb)
            {
            case 0:
                fwinfo_type = FWINFO_CUR_SYS;
                break;
            case 2:
                fwinfo_type = FWINFO_CUR_METER;
                break;
            case 4:
                fwinfo_type = FWINFO_CUR_MODEM;
                break;
            case 6:
                fwinfo_type = FWINFO_CUR_E_MODEM;
                break;
            }
            break;
        }
    }

    return fwinfo_type;
}

// grpe 0,3, 4, 6
static bool obis_is_fw_ch_groupe(uint8_t grpe)
{
    int i;

    for (i = 0; i < NUM_FW_GRPE; i++)
    {
        if (grpe == groupe_fw_e[i])
            break;
    }
    return (i < NUM_FW_GRPE) ? true : false;
}

uint32_t obis_get_fw_info_imgtr_groupe(void)
{
    int i;
    uint32_t imgtr_fw = (uint32_t)FW_DL_TYPE_NONE;

    for (i = 0; i < NUM_FW_GRPE; i++)
    {
        if (obis_ge == groupe_fw_e[i])
        {
            switch (obis_ge)
            {
            case 0:
                imgtr_fw = FW_DL_SYS_PART;
                break;
            case 3:
                imgtr_fw = FW_DL_METER_PART;
                break;
            case 4:
                imgtr_fw = FW_DL_INT_MDM;
                break;
            case 6:
                imgtr_fw = FW_DL_EXT_MDM;
                break;
            }
        }
    }

    return imgtr_fw;
}

// grpb 1,2,3
bool obis_is_mr_data_type_groupb(uint8_t grpb)
{
    int i;

    for (i = 0; i < NUM_AMR_DATA_GRPEB; i++)
    {
        if (grpb == groupb_amr_data_b[i])
            break;
    }
    return (i < NUM_AMR_DATA_GRPEB) ? true : false;
}

uint8_t obis_get_mr_data_type(uint8_t grpb)
{
    int i;

    for (i = 0; i < NUM_AMR_DATA_GRPEB; i++)
    {
        if (grpb == groupb_amr_data_b[i])
            break;
    }
    return groupb_amr_data_b[i];
}

static bool obis_is_enercy_ch_groupc(uint8_t grpc, uint8_t grpd)
{
    int i;

    if (grpd == 0x08)
    {
        // OBIS_ENERGY
        for (i = 0; i < NUM_GRPCM; i++)
        {
            if (grpc == groupc_m[i])
                break;
        }
        return (i < NUM_GRPCM) ? true : false;
    }
    else
    {
        // 최대 수요 관련 obis
        for (i = 0; i < NUM_GRPCM1; i++)
        {
            if (grpc == groupc_m1[i])
                break;
        }
        return (i < NUM_GRPCM1) ? true : false;
    }
}

static void appl_util_pwd_retore(void)
{
    auth_pwd_type auth;

    if (nv_read(I_UTIL_PASSWORD, (uint8_t*)&auth))
    {
        appl_util_pwd = auth.pwd;
    }
    else
    {
        appl_util_pwd.len = UTIL_PWD_SIZE;
        memcpy(appl_util_pwd.pwd, &util_pwd_default[0], UTIL_PWD_SIZE);
    }
}

static void appl_485comm_pwd_retore(void)
{
    auth_pwd_type auth;

    if (nv_read(I_485_PASSWORD, (uint8_t*)&auth))
    {
        appl_485_pwd = auth.pwd;
    }
    else
    {
        appl_485_pwd.len = COMM485_PWD_SIZE;
        memcpy(appl_485_pwd.pwd, &comm485_pwd_default[0], COMM485_PWD_SIZE);
    }
}

static void appl_priv_pwd_retore(void)
{
    appl_priv_pwd.len = PRIV_PWD_SIZE;
    memcpy(appl_priv_pwd.pwd, &priv_pwd_default[0], PRIV_PWD_SIZE);
}

bool obis_is_evtlog_range(uint8_t grp)
{
    int i;

    for (i = 0; i < NUM_GRPEN; i++)
    {
        if (grp == groupe_n[i])
            break;
    }

    return (bool)(i < NUM_GRPEN);
}

bool obis_is_asso_range(uint8_t grp)
{
    int i;

    for (i = 0; i < NUM_ASSO_GRPEN; i++)
    {
        if (grp == groupe_asso_n[i])
            break;
    }

    return (bool)(i < NUM_ASSO_GRPEN);
}

elog_kind_type conv_elog_from_grpe(uint8_t grp)
{
    if (grp == 17)
        return eLogSCurrLimit;

    if (grp == 18)
        return eLogSCurrNonSel;

    if (grp == 19)
        return eLogErrDiagonist;

#if 1  // jp.kim 24.10.28
    if (grp == 26)
        return eLogWorkPwrF;
#endif

    return (elog_kind_type)(grp - 1);
}

// ------------ response -------------------------------
// uint8_t sec_buf[1024 + 270];
uint8_t sec_buf[1500 + 100];  // jp.kim 25.01.22
#define OPT_DS_SIGN_NEED 0
void appl_send_msg(uint32_t EN_AT, uint8_t* pbuff, uint16_t len)
{
    uint16_t tx_len = 0;
    uint8_t idx = 0;
    /* Application Access Type, Encryption Attribute, Application Message Length
     * ? */
    DPRINTF(DBG_INFO, "%s: ACS[%d], EN_AT[%d], tx_len[%d]\r\n", __func__,
            appl_acs_type, EN_AT, len);

    if (EN_AT)
    {
        dl_fill_LLC_header(&sec_buf[idx]);  // rebuild LLC
        len -= LLC_HEADER_LEN;

        dsm_sec_ciphering_xDLMS_APDU_build(&sec_buf[LLC_HEADER_LEN], &tx_len,
                                           &pbuff[LLC_HEADER_LEN], len);
        tx_len += LLC_HEADER_LEN;

        if (appl_acs_type == auth_enc_sign_read_write ||
            appl_acs_type == auth_enc_sign_read)
        {
#if 1 /* bccho, 2024-09-05, 삼상 */
            if (G_SIGNING_OK)
            {
                G_SIGNING_OK = 0;
                dl_fill_LLC_header(&pbuff[idx]);  // rebuild LLC
                tx_len -= LLC_HEADER_LEN;
                dsm_sec_g_signing_xDLMS_APDU_build(
                    &pbuff[LLC_HEADER_LEN], &tx_len, &sec_buf[LLC_HEADER_LEN],
                    tx_len);
                tx_len += LLC_HEADER_LEN;
                dl_send_appl_msg(pbuff, tx_len);
            }
            else
            {
                appl_exception_resp(SER_STATE_NOT_ALLOWED,
                                    SER_ERR_OTHER_REASON);
                return;
            }
#else
            dl_fill_LLC_header(&pbuff[idx]);  // rebuild LLC
            tx_len -= LLC_HEADER_LEN;
            dsm_sec_g_signing_xDLMS_APDU_build(&pbuff[LLC_HEADER_LEN], &tx_len,
                                               &sec_buf[LLC_HEADER_LEN],
                                               tx_len);

            DPRINT_HEX(DBG_CIPHER, "--> Tx Cipher + Sign",
                       &pbuff[LLC_HEADER_LEN], tx_len, DUMP_SEC);

            tx_len += LLC_HEADER_LEN;
            dl_send_appl_msg(pbuff, tx_len);
#endif
        }
        else
        {
            dl_send_appl_msg(sec_buf, tx_len);
        }
    }
    else
    {
        dl_send_appl_msg(pbuff, len);
    }
}

void appl_get_resp(void)
{
    int idx;
    int pktlen;
    uint32_t EN_AT_FLAG = 0;
    T_DLMS_SC* p_sc = dsm_sec_sc_field_get();

    DPRINTF(DBG_TRACE, "%s: appl_resp_result[%d], RX_ENC[%d]\r\n", __func__,
            appl_resp_result, p_sc->enc);

    idx = 0;
    // LLC header
    dl_fill_LLC_header(&pPdu[idx]);
    idx += LLC_HEADER_LEN;

    // application header
    pPdu[idx++] = TAG_GET_RES;
    pPdu[idx++] = appl_resp_choice;
    pPdu[idx++] = appl_invokeid_pri;

    // block description (in case of GET_RES_BLOCK)
    if (appl_resp_choice == GET_RES_BLOCK)
    {
        pPdu[idx++] = appl_resp_last_block;
        FILL_U32_pV(idx, appl_resp_block_num);
    }

    // result option
    if (appl_resp_result == APPL_RESULT_OK)
    {
        pPdu[idx++] = 0;
        // response data => already filled by (pPdu, pPdu_idx)
        pktlen = pPdu_idx;
    }
    else
    {
        pPdu[idx++] = 1;  // result option
        pPdu[idx++] = appl_get_result_reason(appl_resp_result);  // error code
        pktlen = idx;
    }
    if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
    {
        if (p_sc->enc == 1)
            EN_AT_FLAG = TRUE;
        else if (appl_conn_state == APPL_ENC_SIGN_STATE)
        {
            if (appl_acs_type == auth_enc_read ||
                appl_acs_type == auth_enc_read_write ||
                appl_acs_type == auth_enc_sign_read ||
                appl_acs_type == auth_enc_sign_read_write)
            {
                EN_AT_FLAG = TRUE;
            }
        }
    }
    appl_send_msg(EN_AT_FLAG, pPdu, pktlen);
}

void appl_set_resp(void)
{
    int idx;
    uint32_t EN_AT_FLAG = 0;
    T_DLMS_SC* p_sc = dsm_sec_sc_field_get();

    DPRINTF(DBG_TRACE, "%s: appl_resp_result[%d], RX_ENC[%d]\r\n", __func__,
            appl_resp_result, p_sc->enc);

    idx = 0;
    // LLC header
    dl_fill_LLC_header(&pPdu[idx]);
    idx += LLC_HEADER_LEN;

    // application header
    pPdu[idx++] = TAG_SET_RES;
    pPdu[idx++] = appl_resp_choice;
    pPdu[idx++] = appl_invokeid_pri;

    switch (appl_resp_choice)
    {
    case SET_RES_NORMAL:
        pPdu[idx++] = appl_set_result_reason(appl_resp_result);
        break;

    case SET_RES_BLOCK_LAST:
        pPdu[idx++] = appl_set_result_reason(appl_resp_result);
    case SET_RES_BLOCK:
        FILL_U32_pV(idx, appl_resp_block_num);
        break;
    }

    if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
    {
        if (p_sc->enc == 1)
            EN_AT_FLAG = TRUE;
        else if (appl_conn_state == APPL_ENC_SIGN_STATE)
        {
            if (appl_acs_type == auth_enc_read ||
                appl_acs_type == auth_enc_read_write ||
                appl_acs_type == auth_enc_sign_read ||
                appl_acs_type == auth_enc_sign_read_write)
            {
                EN_AT_FLAG = TRUE;
            }
        }
    }
    appl_send_msg(EN_AT_FLAG, pPdu, idx);
}

void appl_act_resp(void)  // action
{
    int idx;
    int pktlen;
    uint32_t EN_AT_FLAG = 0;
    T_DLMS_SC* p_sc = dsm_sec_sc_field_get();

    DPRINTF(DBG_TRACE,
            "%s: appl_resp_result[%d], RX_ENC[%d], option_flag[%d]\r\n",
            __func__, appl_resp_result, p_sc->enc, appl_act_rsp_option_flag);

    idx = 0;
    // LLC header
    dl_fill_LLC_header(&pPdu[idx]);
    idx += LLC_HEADER_LEN;

    // application header
    pPdu[idx++] = TAG_ACT_RES;
    pPdu[idx++] = appl_resp_choice;
    pPdu[idx++] = appl_invokeid_pri;

    pPdu[idx++] = appl_act_result_reason(appl_resp_result);
    if (appl_is_sap_sec_utility() || appl_is_sap_sec_site())
    {
        if (p_sc->enc == 1)
            EN_AT_FLAG = TRUE;
        else if (appl_conn_state == APPL_ENC_SIGN_STATE)
        {
            if (appl_acs_type == auth_enc_read ||
                appl_acs_type == auth_enc_read_write ||
                appl_acs_type == auth_enc_sign_read ||
                appl_acs_type == auth_enc_sign_read_write)
            {
                EN_AT_FLAG = TRUE;
            }
        }
    }

    if (appl_act_rsp_option_flag)
    {
        pPdu[idx++] = 1;  // option enable
        pPdu[idx++] = 0;  // data choice

        pktlen = pPdu_idx;
        appl_send_msg(EN_AT_FLAG, pPdu, pktlen);
    }
    else
    {
        pPdu[idx++] = 0;
        appl_send_msg(EN_AT_FLAG, pPdu, idx);
    }
}

// Description: send confirmed service error in case of error after processing
// client message
//		RESPONSE CHOICE TAG is 1 temporarily
// Parameters:
// 		service error choice
//		service error enum
static void appl_confirmed_svcerr_resp(uint8_t err_choice, uint8_t err_enum)
{
    // DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    DPRINTF(DBG_TRACE, _D "Service Error\r\n");
    pPdu_idx = 0;

    // LLC header
    dl_fill_LLC_header(&pPdu[pPdu_idx]);
    pPdu_idx += LLC_HEADER_LEN;

    // application header
    pPdu[pPdu_idx++] = TAG_CONFIRMED_SVCERR;
    pPdu[pPdu_idx++] = 1;  // choice (1) =>  a temporary(unspecified) value
    // application data
    pPdu[pPdu_idx++] = err_choice;  // result option
    pPdu[pPdu_idx++] = err_enum;

    dl_send_appl_msg(pPdu, pPdu_idx);
}

static void appl_exception_resp(uint8_t err_state, uint8_t err_enum)
{
    DPRINTF(DBG_ERR, "%s: err_state[%d], err_num[%d]\r\n", __func__, err_state,
            err_enum);
    pPdu_idx = 0;

    // LLC header
    dl_fill_LLC_header(&pPdu[pPdu_idx]);
    pPdu_idx += LLC_HEADER_LEN;

    // application header
    pPdu[pPdu_idx++] = APPL_EXEPTION_RSP;
    pPdu[pPdu_idx++] = err_state;
    pPdu[pPdu_idx++] = err_enum;

    dl_send_appl_msg(pPdu, pPdu_idx);
}

static bool appl_AARE(aarq_rslt_type rslt)
{
    bool ok;
    uint8_t svcerr;

    ok = false;
    pPdu_idx = 0;

    DPRINTF(DBG_TRACE, "%s, result[%d]\r\n", __func__, rslt);
    // LLC header
    dl_fill_LLC_header(&pPdu[pPdu_idx]);
    pPdu_idx += LLC_HEADER_LEN;

    if (rslt == AARQ_UINF_DLMS_VER_LOW || rslt == AARQ_UINF_CONFORMANCE_NOT ||
        rslt == AARQ_UINF_DK_UNSUPPORT ||  // ctt 3.1
        rslt == AARQ_UINF_CLIENT_RCV_LOW)
    {
        // user information is ConfirmedServiceError
        memcpy(&pPdu[pPdu_idx], &aare_svcerr[0], AARE_SERR_SIZE);
        switch (rslt)
        {
        case AARQ_UINF_DLMS_VER_LOW:
            svcerr = SVCERR_DLMS_VER_TOOLOW;
            break;
        case AARQ_UINF_CONFORMANCE_NOT:
            svcerr = SVCERR_INCOMPATIBLE_CONFORMANCE;
            break;
        case AARQ_UINF_CLIENT_RCV_LOW:
            svcerr = SVCERR_PDU_TOOSHORT;
            break;
        case AARQ_UINF_DK_UNSUPPORT:
            svcerr = SVCERR_OTHER;
            break;
        default:
            svcerr = SVCERR_OTHER;
            break;
        }

        pPdu[pPdu_idx + AARE_SVCERR_POS] = svcerr;
        pPdu_idx += AARE_SERR_SIZE;

        dl_send_appl_msg(pPdu, pPdu_idx);
    }
    else
    {
        memcpy(&pPdu[pPdu_idx], &aare[0], AARE_SIZE);
        if (rslt == AARQ_OK)
        {
            ok = true;
        }
        else
        {
            pPdu[pPdu_idx + AARE_RESULT_POS] = REJECTED_PERMANENT;

            switch (rslt)
            {
            case AARQ_PROTOCOL_VER_ERROR:
                pPdu[pPdu_idx + AARE_SVC_CHOICE_POS] =
                    0xa2;  // unsolved: acse_service_provider(0xa2 (???))
                pPdu[pPdu_idx + AARE_DIAGNOSTICS_POS] =
                    ACSE_NO_COMMON_ACSE_VERSION;
                break;
            case AARQ_APP_CONTEXT_ERROR:
                pPdu[pPdu_idx + AARE_DIAGNOSTICS_POS] =
                    ACSE_APP_CONTEXT_NAME_NOT_SUPPORTED;
                break;
            case AARQ_AUTH_ERROR:
                pPdu[pPdu_idx + AARE_DIAGNOSTICS_POS] = ACSE_AUTH_FAILURE;
                break;
            case AARQ_IMPL_INF_NOT_SUPPORTED:
            case AARQ_ERROR:
            case AARQ_UINF_ERROR:
            default:
                pPdu[pPdu_idx + AARE_DIAGNOSTICS_POS] =
                    ACSE_NO_REASON_GIVEN_PROVIDER;
                break;
            }
        }

        pPdu[pPdu_idx + AARE_CONFORM_BLK_POS] = appl_conformance[0];
        pPdu[pPdu_idx + AARE_CONFORM_BLK_POS + 1] = appl_conformance[1];
        pPdu[pPdu_idx + AARE_CONFORM_BLK_POS + 2] = appl_conformance[2];

        pPdu_idx += AARE_SIZE;

        dl_send_appl_msg(pPdu, pPdu_idx);
    }

    return ok;
}

uint16_t appl_aare_set_param(uint32_t sel_param, uint8_t* ptxbuff)
{
    uint16_t idx = 0, ret_size = 0;

    DPRINTF(DBG_TRACE, _D "%s: AARE[0x%02X]\r\n", __func__, sel_param);

    switch (sel_param)
    {
    case ASSO_RESPONDER_ACSE_REQ:  // 88
        memcpy(ptxbuff, app_resp_acse_req, ACSE_RESP_REQ_LEN);
        ret_size = ACSE_RESP_REQ_LEN;
        DPRINT_HEX(DBG_WARN, "RESP_ACSE_REQ", ptxbuff, ACSE_RESP_REQ_LEN,
                   DUMP_DLMS);

        break;
    case ASSO_RESP_MECHANISM_NAME:  // 89
        memcpy(ptxbuff, app_resp_mecah_name, APP_RESP_MECHANISM_NAME_LEN);
        ret_size = APP_RESP_MECHANISM_NAME_LEN;
        DPRINT_HEX(DBG_WARN, "RESP_MECHANISM_NAME", ptxbuff,
                   APP_RESP_MECHANISM_NAME_LEN, DUMP_DLMS);

        break;
    case ASSO_APP_CONTEXT_NAME:  // A1
        memcpy(ptxbuff, app_context_name_asso3, APP_CONTEXT_LEN);
        ptxbuff[APP_CONTEXT_LEN - 1] = APPCTXT_LN_NO_CIPHERING;

        ret_size = APP_CONTEXT_LEN;
        DPRINT_HEX(DBG_WARN, "RESP_APP_CONTEXT_NAME", ptxbuff, APP_CONTEXT_LEN,
                   DUMP_DLMS);

        break;
    case ASSO_RESULT_FIELD:  // A2
        memcpy(ptxbuff, app_resp_result, APP_RESP_RESULT_LEN);
        ret_size = APP_RESP_RESULT_LEN;
        DPRINT_HEX(DBG_WARN, "RESP_RESULT_FIELD", ptxbuff, APP_RESP_RESULT_LEN,
                   DUMP_DLMS);

        break;
    case ASSO_RESULT_SRC_DIAG:  // A3
        memcpy(ptxbuff, app_resp_diagnostic_src_result,
               APP_RESP_DIAGONOSTIC_SRC_RESULT_LEN);
        ret_size = APP_RESP_DIAGONOSTIC_SRC_RESULT_LEN;
        DPRINT_HEX(DBG_WARN, "RESP_RESULT_SRC_DIAG", ptxbuff,
                   APP_RESP_DIAGONOSTIC_SRC_RESULT_LEN, DUMP_DLMS);

        break;
    case ASSO_RESP_AP_TITLE:  // A4
        memcpy(&app_resp_ap_title[4], SYS_TITLE_server, SYS_TITLE_LEN);
        memcpy(ptxbuff, app_resp_ap_title, APP_RESP_AP_TITLE_LEN);
        DPRINT_HEX(DBG_CIPHER, "--> Tx Server System Title", ptxbuff,
                   APP_RESP_AP_TITLE_LEN,
                   /*DUMP_ALWAYS*/ DUMP_SEC);  // 인증서 시스템 타이틀
        ret_size = APP_RESP_AP_TITLE_LEN;

        break;
    case ASSO_RESP_AE_QUALIFIER:  // A5
    {
        uint16_t cert_len;
        uint8_t* p_cert;

        p_cert = dsm_sec_get_cert_buff();
        if (!p_cert)
        {
            DPRINTF(DBG_INFO, _D "RESP_AE_QUALIFIER Error\r\n");
            return 0;
        }
        if (p_cert[1] == 0x82)
        {
            cert_len =
                (((uint16_t)p_cert[2] << 8) | ((uint16_t)(p_cert[3]))) + 4;
        }
        else if (p_cert[1] == 0x81)
        {
            cert_len = p_cert[2] + 3;
        }
        else
        {
            cert_len = p_cert[1] + 2;
        }
        app_resp_ae_qualifier[2] = (cert_len + 4) >> 8;
        app_resp_ae_qualifier[3] = (cert_len + 4);
        app_resp_ae_qualifier[6] = (cert_len) >> 8;
        app_resp_ae_qualifier[7] = (cert_len);

        memcpy(&app_resp_ae_qualifier[8], &p_cert[0], cert_len);

        memcpy(ptxbuff, app_resp_ae_qualifier, (cert_len + 8));

        ret_size = cert_len + 8;
        DPRINT_HEX(DBG_CIPHER, "--> Tx Server Certificate", ptxbuff,
                   (cert_len + 8),
                   DUMP_DLMS | DUMP_SEC);  // 인증서 데이터

        return ret_size;
    }
    case ASSO_RESP_AUTH_VALUE:  // AA
        ptxbuff[idx++] = ASSO_RESP_AUTH_VALUE;
        ptxbuff[idx++] = 0x22;
        ptxbuff[idx++] = 0x80;
        ptxbuff[idx++] = XTOX_LEN;
        memcpy(&ptxbuff[idx], XTOX, XTOX_LEN);
        idx += XTOX_LEN;
        ret_size = idx;

        DPRINT_HEX(DBG_CIPHER, "--> Tx Server RAND", XTOX, XTOX_LEN,
                   /*DUMP_ALWAYS*/ DUMP_SEC);

        break;
    case ASSO_USER_INFORMATION:  // BE
        memcpy(ptxbuff, app_resp_usr_info, APP_RESP_USR_INFORMATION_LEN);
        ptxbuff[AARE_CONFORM_BLK_POS_for_SEC_USRINFO] = appl_conformance[0];
        ptxbuff[AARE_CONFORM_BLK_POS_for_SEC_USRINFO + 1] = appl_conformance[1];
        ptxbuff[AARE_CONFORM_BLK_POS_for_SEC_USRINFO + 2] = appl_conformance[2];
        ret_size = APP_RESP_USR_INFORMATION_LEN;
        DPRINT_HEX(DBG_WARN, "RESP_USER_INFORMATION", ptxbuff, ret_size,
                   DUMP_DLMS);
        break;
    default:

        break;
    }
    DPRINTF(DBG_TRACE, "%s: AARE: Next idx[%d]\r\n", __func__, ret_size);

    return ret_size;
}
static bool appl_AARE_asso3(aarq_rslt_type rslt)
{
    bool ok;
    uint8_t svcerr = SVCERR_OTHER;

    ok = false;
    pPdu_idx = 0;

    DPRINTF(DBG_TRACE, "%s, result[%d]\r\n", __func__, rslt);
    // LLC header
    dl_fill_LLC_header(&pPdu[pPdu_idx]);
    pPdu_idx += LLC_HEADER_LEN;

    if (rslt == AARQ_UINF_DLMS_VER_LOW || rslt == AARQ_UINF_CONFORMANCE_NOT ||
        rslt == AARQ_UINF_DK_UNSUPPORT ||  // ctt 3.1
        rslt == AARQ_UINF_CLIENT_RCV_LOW)
    {
        // user information is ConfirmedServiceError
        memcpy(&pPdu[pPdu_idx], &aare_svcerr[0], AARE_SERR_SIZE);

        switch (rslt)
        {
        case AARQ_UINF_DLMS_VER_LOW:
            svcerr = SVCERR_DLMS_VER_TOOLOW;
            break;
        case AARQ_UINF_CONFORMANCE_NOT:
            svcerr = SVCERR_INCOMPATIBLE_CONFORMANCE;
            break;
        case AARQ_UINF_CLIENT_RCV_LOW:
            svcerr = SVCERR_PDU_TOOSHORT;
            break;
        case AARQ_UINF_DK_UNSUPPORT:
            svcerr = SVCERR_OTHER;
            break;
        default:

            break;
        }

        pPdu[pPdu_idx + AARE_SVCERR_POS] = svcerr;
        pPdu_idx += AARE_SERR_SIZE;

        dl_send_appl_msg(pPdu, pPdu_idx);
    }
    else
    {
        uint8_t tmpbuff[600], POS_ADD = 0;
        uint16_t offset = 0;

        offset += appl_aare_set_param(ASSO_APP_CONTEXT_NAME, &tmpbuff[offset]);
        offset += appl_aare_set_param(ASSO_RESULT_FIELD, &tmpbuff[offset]);
        offset += appl_aare_set_param(ASSO_RESULT_SRC_DIAG, &tmpbuff[offset]);
        offset += appl_aare_set_param(ASSO_RESP_AP_TITLE, &tmpbuff[offset]);
        offset += appl_aare_set_param(ASSO_RESP_AE_QUALIFIER, &tmpbuff[offset]);
        offset += appl_aare_set_param(
            ASSO_RESPONDER_ACSE_REQ,
            &tmpbuff[offset]);  // responder-acse-requirements
        offset += appl_aare_set_param(ASSO_RESP_MECHANISM_NAME,
                                      &tmpbuff[offset]);  // mechanism-name
        offset += appl_aare_set_param(
            ASSO_RESP_AUTH_VALUE,
            &tmpbuff[offset]);  // responding-authentication-value(StoC)
        offset += appl_aare_set_param(ASSO_USER_INFORMATION,
                                      &tmpbuff[offset]);  // user-information

        pPdu[pPdu_idx++] = ASSO_AARE;
        if (offset > 255)
        {
            pPdu[pPdu_idx++] = 0x82;
            pPdu[pPdu_idx++] = offset >> 8;
            pPdu[pPdu_idx++] = offset;
            POS_ADD = 2;
        }
        else if (offset > 128)
        {
            pPdu[pPdu_idx++] = 0x81;
            pPdu[pPdu_idx++] = offset;
            POS_ADD = 1;
        }
        else
        {
            pPdu[pPdu_idx++] = offset;
        }

        memcpy(&pPdu[pPdu_idx], &tmpbuff[0], offset);
        pPdu_idx += offset;

        if (rslt == AARQ_OK)
        {
            ok = true;
        }
        else
        {
            pPdu[LLC_HEADER_LEN + AARE_RESULT_POS + POS_ADD] =
                REJECTED_PERMANENT;

            switch (rslt)
            {
            case AARQ_PROTOCOL_VER_ERROR:
                pPdu[LLC_HEADER_LEN + AARE_SVC_CHOICE_POS + POS_ADD] =
                    0xa2;  // unsolved: acse_service_provider(0xa2 (???))
                pPdu[LLC_HEADER_LEN + AARE_DIAGNOSTICS_POS + POS_ADD] =
                    ACSE_NO_COMMON_ACSE_VERSION;
                break;
            case AARQ_APP_CONTEXT_ERROR:
                pPdu[LLC_HEADER_LEN + AARE_DIAGNOSTICS_POS + POS_ADD] =
                    ACSE_APP_CONTEXT_NAME_NOT_SUPPORTED;
                break;
            case AARQ_AUTH_ERROR:
                pPdu[LLC_HEADER_LEN + AARE_DIAGNOSTICS_POS + POS_ADD] =
                    ACSE_AUTH_FAILURE;
                break;
            case AARQ_IMPL_INF_NOT_SUPPORTED:
            case AARQ_ERROR:
            case AARQ_UINF_ERROR:
            default:
                pPdu[LLC_HEADER_LEN + AARE_DIAGNOSTICS_POS + POS_ADD] =
                    ACSE_NO_REASON_GIVEN_PROVIDER;
                break;
            }
        }
        OSTimeDly(OS_MS2TICK(1));
        DPRINT_HEX(DBG_WARN, "AARE", pPdu, pPdu_idx, DUMP_DLMS);
        dl_send_appl_msg(pPdu, pPdu_idx);
    }

    return ok;
}

static uint8_t appl_get_result_reason(get_req_result_type rslt)
{
    uint8_t reason;

    switch (rslt)
    {
    case GET_RESULT_OK:
        reason = DATA_ACS_SUCCESS;
    case GET_RESULT_OBJ_UNDEF:
        reason = DATA_ACS_OBJ_UNDEF;
        break;

    case GET_RESULT_OBJ_UNAVAIL:
        reason = DATA_ACS_OBJ_UNAVAIL;
        break;

    case GET_RESULT_TYPE_UNMAT:
        reason = DATA_ACS_TYPE_UNMAT;
        break;

    case GET_RESULT_LONG_GET_ABORT:
        reason = DATA_ACS_LONG_GET_ABORT;
        break;

    case GET_RESULT_NO_LONG_GET:
        reason = DATA_ACS_NO_LONG_GET;
        break;

    case GET_RESULT_OTHER_REASON:
        reason = DATA_ACS_OTHER_REASON;
        break;

    case GET_RESULT_BLOCK_APDU_ERR:
        reason = DATA_ACS_LONG_GET_ABORT;
        break;

    case GET_RESULT_BLOCK_NEXT_ERR:
        reason = DATA_ACS_NO_LONG_GET;
        break;

    default:
        reason = DATA_ACS_OTHER_REASON;
        break;
    }

    DPRINTF(DBG_TRACE, _D "%s: %s[%d], Result[%d]\r\n", __func__,
            (rslt ? "Error" : "Success"), rslt, reason);
    return reason;
}

static uint8_t appl_set_result_reason(set_req_result_type rslt)
{
    uint8_t reason;

    switch (rslt)
    {
    case SET_RESULT_OK:
        reason = DATA_ACS_SUCCESS;
        break;

    case SET_RESULT_DATA_NG:
    case SET_RESULT_REQ_NG:
        reason = DATA_ACS_TEMP_FAIL;
        break;

    case SET_RESULT_BLOCK_NO_UNMATCH:
        reason = DATA_ACS_NO_LONG_SET_INPROGRESS;
        break;

    case SET_RESULT_TYPE_UNMAT:
        reason = DATA_ACS_TYPE_UNMAT;
        break;

    default:
        reason = DATA_ACS_OTHER_REASON;
        break;
    }

    DPRINTF(DBG_TRACE, _D "%s: %s[%d], Result[%d]\r\n", __func__,
            (rslt ? "Error" : "Success"), rslt, reason);
    return reason;
}

static uint8_t appl_act_result_reason(act_req_result_type rslt)
{
    uint8_t reason;

    switch (rslt)
    {
    case ACT_RESULT_OK:
        reason = DATA_ACS_SUCCESS;
        break;

    case ACT_RESULT_DATA_NG:
    case ACT_RESULT_TYPE_UNMAT:
        reason = DATA_ACS_TYPE_UNMAT;
        break;
#if 1  // jp.kim 25.12.06 //운영부 계량부 무결성 검증
    case ACT_RESULT_VERIFY_FAILED:
        reason = RLT_ACT_TEMP_FAIL;
        break;

    case ACT_RESULT_OTHER:
#endif
    case ACT_RESULT_PRE_SEC_MUTUAL_AUTH_NG:
    case ACT_RESULT_PRE_SEC_KEYAGREEMENT_NG:
        reason = RLT_ACT_OTHER_REASON;
        break;
    default:
        reason = DATA_ACS_OTHER_REASON;
        break;
    }
    DPRINTF(DBG_TRACE, _D "%s: %s[%d], Result[%d]\r\n", __func__,
            (rslt ? "Error" : "Success"), rslt, reason);

    if (act_devcmd == DEVICE_CMD_INIT)
    {
        M_MT_SW_GENERAL_two_TIMER_set_meter_reset();
        dsm_meter_sw_timer_start(MT_SW_GENERAL_two_TO, FALSE,
                                 MT_TIMEOUT_METER_RESET_MAX);

        DPRINTF(DBG_WARN, _D "%s: meter reset TO[%d ms] \r\n", __func__,
                MT_TIMEOUT_METER_RESET_MAX);
    }
    return reason;
}

void dsm_meter_reset_timer_proc(void)
{
    if (M_MT_SW_GENERAL_two_TIMER_is_meter_reset())
    {
        M_MT_SW_GENERAL_two_TIMER_clear_meter_reset();
        dsm_meter_sw_timer_stop(MT_SW_GENERAL_two_TO);

        if (act_devcmd == DEVICE_CMD_INIT)
        {
            mt_init_delay_until_txcomplete = true;

            if (appl_is_sap_public() ||
                appl_is_sap_utility())  // jp.kim 24.11.07
            {
                sap_is_utility_when_mt_init = true;
            }
            else if (appl_is_sap_sec_utility())
            {
                sap_is_utility_when_mt_init = true;
            }
            else if (appl_is_sap_sec_site())
            {
                sap_is_utility_when_mt_init = true;
            }
            else
            {
                sap_is_utility_when_mt_init = false;
            }

            amr_disc_ind_end_proc();
        }
    }
}

void whm_clear_all(bool is_factory)
{
    DPRINTF(DBG_TRACE, "%s: factory[%d]\r\n", __func__, is_factory);

    bool factory_addtional_reset_err = 0;
    if (is_factory)  // product   //jp.kim 24.11.07
    {
        factory_addtional_reset_err =
            factory_addtional_reset();  // jp.kim 24.11.07
    }
    else
    {
        no_inst_curr_chk_zon_cnt = 10;
    }

    ST_FW_INFO fwinfo = {0};
    if (is_factory)  // product //jp.kim 25.03.30
    {
        // jp.kim 25.03.30
        dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_MODEM);
        memset(&fwinfo.dt, 0xFF, sizeof(date_time_type));
        dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_MODEM);
    }
    // jp.kim 25.03.30
    dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_E_MODEM);
    memset(&fwinfo.dt, 0xFF, sizeof(date_time_type));
    dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_E_MODEM);

    // 착탈형 모뎀 ID 정보 초기화
    ST_MDM_ID ext_mdm_id = {0};
    nv_write(I_EXT_MODEM_ID, (uint8_t*)&ext_mdm_id);

    ST_MTP_CAL_POINT st_mtp_cal_point;
    ST_MTP_PARM st_mtp_parm;
    ST_MTP_SAGSWELL st_mtp_sagswell;

    if (is_factory)  // product
    {
        dsm_mtp_default_cal_point(&st_mtp_cal_point);
        dsm_mtp_default_parm(&st_mtp_parm);
        dsm_mtp_default_sagswell(&st_mtp_sagswell);

        log_mt_init_clear(appl_tbuff);
        latchon_cnt_clear();
        bat_used_time_clear();
        bat_inst_completely_clear();
        nv_header_set(NV_PROD_INITED);
    }
    else
    {
#if 1  // jp.kim 25.03.15
        dsm_mtp_default_cal_point(&st_mtp_cal_point);
        dsm_mtp_default_parm(&st_mtp_parm);
        dsm_mtp_default_sagswell(&st_mtp_sagswell);
#if 0  // jp.kim 25.03.30 계기초기화 횟수 계속 누적
        log_mt_init_clear(appl_tbuff);
#endif
#endif

        ST_MT_RST_TIME st_mt_rst_time;
        memcpy(&st_mt_rst_time.dt, &cur_rtc, sizeof(date_time_type));
        DPRINTF(DBG_TRACE, _D "%s: %d.%d.%d %d:%d:%d\r\n", __func__,
                cur_rtc.year, cur_rtc.month, cur_rtc.date, cur_rtc.hour,
                cur_rtc.min, cur_rtc.sec);
        nv_write(I_MT_RST_TIME, (uint8_t*)&st_mt_rst_time);
        log_mt_initialization(appl_tbuff);
        nv_header_set(NV_COMM_INITED);
    }

    lp_clear();

    dsm_uart_deq_string(DEBUG_COM);

    if (factory_addtional_reset_err)
        return;

#if 0 /* bccho, 2023-11-16, 계기 초기화 시 리붓하지 않는다.  --> 다시 리붓 \
         적용 (2023-11-26) */
    whm_init();
#else
#if 1 /* bccho, NVIC_SystemReset, 2023-07-15 */
    goto_loader_S();
#else
    NVIC_SystemReset();
    while (1);
#endif
#endif
}
