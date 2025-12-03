#if !defined(__AMG_SEC_H__)
#define __AMG_SEC_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "whm.h"
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/
#define CERT_MAX_NUM 6

#define CERT_CON_IDX_ROOTCA_S 0
#define CERT_CON_IDX_SUBCA_S 1
#define CERT_CON_IDX_DS_S 2
#define CERT_CON_IDX_DS_C 3
#define CERT_CON_IDX_KTC_CA 4
#define CERT_CON_IDX_KTC_DS 5

typedef enum
{
    CERT_ENTITY_SERVER,
    CERT_ENTITY_CLIENT,
    CERT_ENTITY_AUTHORITY,
    CERT_ENTITY_OTHER,
} EN_CERT_ENTITY;

typedef enum
{
    CERT_TYPE_DS,
    CERT_TYPE_KA,
    CERT_TYPE_TLS,
    CERT_TYPE_OTHER,
} EN_CERT_TYPE;

#define SN_MAX_SIZE 20
#define ISSUER_MAX_SIZE 64
#define SUBJ_MAX_SIZE 96
#define SUBJ_ALT_NAME_SIZE 20

#define SYS_TITLE_LEN 8
#define CERT_LEN 500
#define XTOX_LEN 32
#define fXTOX_LEN 64

#define CERT_ROOTCA 0
#define CERT_SUBCA 1
#define CERT_DS 2
#define CERT_KA 3

#define CERT_DS_CLIENT CERT_DS + 4
#define CERT_KA_CLIENT CERT_KA + 4

#define CERT_KTC_ROOTCA 10
#define CERT_KTC_MT_CERT_DS 11
#define CERT_KTC_MT_CERT_TMP_DS 12

#define CERT_GEN_KEYPAIR 20

#define SEC_SERVER 0
#define SEC_CLIENT 1
#define SEC_KTC 2
#define SEC_M_BUFFER_SIZE 500

#define SEC_ISSUE_OK 0
#define SEC_ISSUE_NG 1

#define SEC_INIT_OK 0
#define SEC_INIT_NG_POR 1
#define SEC_INIT_NG_CERT 2
#define SEC_INIT_NG_ISSUE 3
#define SEC_INIT_NG_ISSUE_TERMINATE 4
#define SEC_INIT_NG_DRNG 5

#define DLMS_SC_FIELD_LEN 1
#define DLMS_IC_FIELD_LEN 4
#define DLMS_SEC_HDR_LEN (DLMS_SC_FIELD_LEN + DLMS_IC_FIELD_LEN)
#define DLMS_AUTH_TAG_LEN 12
#define DLMS_DS_LEN 64
#define DLMS_DS_ADDITION_FIELD 32

#define DLMS_KEY_PAIR_DS 0
#define DLMS_KEY_PAIR_KA 1
#define DLMS_KEY_PAIR_TLS 2

enum
{
    DLMS_SC_ENC = 0x01,
    DLMS_SC_AT = 0x02,
    DLMS_SC_DS = 0x04
};

#define WRITE_WORD(addr, data)               \
    {                                        \
        *(addr + 3) = (data & 0xff);         \
        *(addr + 2) = ((data >> 8) & 0xff);  \
        *(addr + 1) = ((data >> 16) & 0xff); \
        *(addr + 0) = ((data >> 24) & 0xff); \
    }

// sec log
#define SEC_LOG_M_INIT_MIS 0xA1
#define SEC_LOG_M_INTER_MIS 0xA2
#define SEC_LOG_Z_GEN_MIS 0xA3
#define SEC_LOG_SESSION_KEY_MIS 0xA4
#define SEC_LOG_ENC_MIS 0xA5
#define SEC_LOG_DEC_MIS 0xA6
#define SEC_LOG_DS_GEN_MIS 0xA7
#define SEC_LOG_DS_VERIFY_MIS 0xA8
#define SEC_LOG_SEC_ITEM_OMISSION_MIS 0xA9
#define SEC_LOG_CROSS_CERT_FAIL 0xAA
#define SEC_LOG_ETC_MIS 0xFF

#define SECURITY_SUITE_AES_128GCM_ECDSAP256SIGN 1
#define SECURITY_SUITE_ARIA_128GCM_ECDSAP256SIGN 15

/*
******************************************************************************
*	MACRO
******************************************************************************
*/

/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/

typedef struct _st_sec_m_power_on_rlt_
{
    uint8_t abVer[3];
    uint8_t bLifeCycle;
    uint8_t chipSerial[8];
    uint8_t abSystemTitle[8];
    uint8_t bVcType, bMaxVcRetryCount;
    uint16_t usMaxKcmvpKeyCount;
    uint16_t usMaxCertKeyCount;
    uint16_t usMaxIoDataSize;
    uint16_t usInfoFileSize;

} ST_SEC_M_POWER_ON_RLT;

typedef struct
{
    uint8_t suite_id : 4;
    uint8_t auth : 1;
    uint8_t enc : 1;
    uint8_t ks : 1;
    uint8_t c : 1;
} T_DLMS_SC;

typedef struct
{
    uint8_t sn_size;
    uint8_t sn[SN_MAX_SIZE];
    uint8_t issuer_size;
    uint8_t issuer[ISSUER_MAX_SIZE];
    uint8_t subj_size;
    uint8_t subj[SUBJ_MAX_SIZE];
    uint8_t subj_alt_name_size;
    uint8_t subj_alt_name[SUBJ_ALT_NAME_SIZE];
} ST_CERT_INFO;

typedef struct
{
    uint8_t cnt;
    ST_CERT_INFO cert_info[CERT_MAX_NUM];
} ST_CERT_CON;

typedef struct
{
    uint16_t cnt;
    uint16_t result;
    uint8_t info[500];
} ST_CSR_INFO;

extern uint8_t SYS_TITLE_server[SYS_TITLE_LEN];
extern uint8_t SYS_TITLE_client[SYS_TITLE_LEN];
extern uint8_t CERT_DS_BUFF[CERT_LEN];
extern uint8_t DS_privatekey[32];
extern uint8_t XTOX[XTOX_LEN];
extern uint8_t XTOX_c[XTOX_LEN];
extern uint8_t fXTOX[fXTOX_LEN];
extern uint8_t fXTOX_s2c[fXTOX_LEN];

extern uint8_t server_key_agreedata[129];
extern uint32_t g_sec_invocation_count;
extern ST_CSR_INFO gst_csr_info;

#if 1 /* bccho, HLS, 2023-08-14 */
extern hls_context_t *hls_ctx;
#endif

void dsm_sec_set_operation(bool val);
bool dsm_sec_get_operation(void);
int16_t dsm_sec_initialize(void);
int16_t dsm_sec_deinitialize(void);
int16_t dsm_sec_issue_clear(void);
int16_t dsm_sec_issue(uint32_t power_on_force);

uint8_t dsm_sec_get_security_suite(void);
void dsm_sec_set_security_suite(uint8_t suite);

ST_SEC_M_POWER_ON_RLT *dsm_sec_m_get_por_result_info(void);
void dsm_sec_por_print(ST_SEC_M_POWER_ON_RLT *p_por_rlt);
ST_SEC_M_POWER_ON_RLT *dsm_sec_m_get_por_result_info(void);

uint8_t dsm_sec_certcon_idx_2_cert_idx(uint8_t certcon_idx);
uint8_t dsm_sec_certcon_idx_2_entity_idx(uint8_t certcon_idx);
uint8_t dsm_sec_set_cert_info(uint8_t cert_field_t, uint8_t cert_idx,
                              uint8_t *pdata, uint8_t len);
uint8_t dsm_sec_get_cert_info(uint8_t cert_field_t, uint8_t cert_con_idx,
                              uint8_t *pdata, uint8_t *len);

uint8_t *dsm_sec_get_cert_buff(void);
void dsm_sec_invocation_count_reset(void);
void dsm_sec_invocation_count_add(void);
void dsm_sec_invocation_count_subtract(void);
uint32_t dsm_sec_invocation_count_get(void);
void dsm_sec_sc_field_set(T_DLMS_SC *p_sc);
T_DLMS_SC *dsm_sec_sc_field_get(void);

#if 1 /* bccho, 2023-07-20 */
int16_t dsm_sec_cert_get(uint32_t party, uint32_t cert_idx,
                         uint32_t sys_t_backup_flag);
#endif
int16_t dsm_sec_cert_put_server(void);
int16_t dsm_sec_cert_put_client_ds(uint8_t *pcert);
int16_t dsm_sec_cert_put_ktc_meter_ca(void);
int16_t dsm_sec_cert_erase_ktc_meter_ca(void);
int16_t dsm_sec_cert_put_ktc_meter_ds(uint8_t *pcert, uint8_t *p_fwname,
                                      uint8_t *phash, date_time_type *pst,
                                      date_time_type *psp);
int16_t dsm_sec_prikey_put_server(void);
int16_t dsm_sec_random_generate(void);
int16_t dsm_sec_fs2c_verify(uint8_t *p_fs2c, uint8_t *client_sys_t);
int16_t dsm_sec_fc2s_generate(uint8_t *client_sys_t);
int16_t dsm_sec_key_agreement_process(uint8_t *pdata, uint16_t len);

int16_t dsm_sec_ciphering_xDLMS_APDU_build(uint8_t *p_xdlms_apdu,
                                           uint16_t *xdlms_len,
                                           uint8_t *p_plain,
                                           uint16_t plain_len);
int16_t dsm_sec_ciphering_xDLMS_APDU_parser(uint8_t *p_i_apdu,
                                            uint16_t xdlms_len,
                                            uint8_t *p_decipher,
                                            uint16_t *decipher_len);
int16_t dsm_sec_signing_for_month_profile(uint8_t *p_o_sign, uint8_t *p_msg,
                                          uint16_t msg_len);
int16_t dsm_sec_g_signing_xDLMS_APDU_build(uint8_t *p_o_apdu,
                                           uint16_t *p_o_apdu_len,
                                           uint8_t *p_indata,
                                           uint16_t indata_len);
int16_t dsm_sec_g_signing_xDLMS_APDU_parser(uint8_t *p_i_apdu,
                                            uint16_t xdlms_len,
                                            uint8_t *p_ciphered_idx,
                                            uint16_t *p_ciphered_len);
int16_t dsm_sec_g_glo_ciphering_xDLMS_APDU_build(uint8_t *p_xdlms_apdu,
                                                 uint16_t *xdlms_len,
                                                 uint8_t *p_plain,
                                                 uint16_t plain_len);
int16_t dsm_sec_g_glo_ciphering_xDLMS_APDU_parser(uint8_t *p_i_apdu,
                                                  uint16_t xdlms_len,
                                                  uint8_t *p_decipher,
                                                  uint16_t *decipher_len);
int16_t dsm_sec_generate_key_pair(uint8_t dlms_key_pair_type);
int16_t dsm_sec_generate_csr(uint8_t dlms_key_pair_type);
int16_t dsm_sec_import_certificate(uint8_t *p_cert, uint16_t len);

#if 1 /* bccho, HLS, 2023-08-14 */
bool hls_sec_init(void);
#endif
#endif /* __AMG_SEC_H__ */
