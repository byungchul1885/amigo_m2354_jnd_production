#include "options.h"
#if 0 /* bccho, FLASH, 2023-07-15 */
#include "flash.h"
#endif /* bccho */
#include "comm.h"
#include "dl.h"
#include "aarq.h"
#include "appl.h"

#include "main.h"
#include "amg_sec.h"

#define _D "[AARQ] "

uint8_t app_context_name_asso3[APP_CONTEXT_LEN] = {TAG_APP_CONTEXT,
                                                   0x09,
                                                   0x06,
                                                   0x07,
                                                   0x60,
                                                   0x85,
                                                   0x74,
                                                   0x05,
                                                   0x08,
                                                   0x01,
                                                   APPCTXT_LN_NO_CIPHERING};
static uint8_t app_context_name[APP_CONTEXT_LEN] = {TAG_APP_CONTEXT,
                                                    0x09,
                                                    0x06,
                                                    0x07,
                                                    0x60,
                                                    0x85,
                                                    0x74,
                                                    0x05,
                                                    0x08,
                                                    0x01,
                                                    0x01};

uint8_t mecha_name_asso3[MECHA_NAME_SIZE] = {
    0x60, 0x85, 0x74, 0x05, 0x08, 0x02, AUTHMECH_HIGH_ECDSA};
static const uint8_t mecha_name[MECHA_NAME_SIZE] = {0x60, 0x85, 0x74, 0x05,
                                                    0x08, 0x02, 0x01};

static const uint8_t mecha_name_noauth[MECHA_NAME_SIZE] = {
    0x60, 0x85, 0x74, 0x05, 0x08, 0x02, 0x00};

uint8_t calling_ae_qualifier[AARQ_AE_CALLING_QUALIFIER_LEN] = {
    ASSO_CALLING_AE_QUALIFIER, 0x82, 0x01, 0xF8,
    BER_TYPE_OCTET_STRING,     0x82, 0x01, 0xF4,
};

static int skip_calling_data(int idx);
static int check_auth(int idx, uint8_t *rslt);
static int acse_require(int idx, appl_auth_type *auth);
static int get_auth_val(int idx, auth_val_type *auth_p);
static bool check_get_conformance_block(uint8_t *pdata);
static user_info_rslt_type proc_user_info(int idx);
static user_info_rslt_type proc_user_info_asso3(uint8_t *pdata,
                                                uint16_t *p_o_idx);
static aarq_rslt_type uinf_rslt_to_aarq_rslt(user_info_rslt_type rslt);

aarq_rslt_type appl_AARQ_proc(int idx)
{
    uint8_t t8;
    uint8_t tbuf[APP_CONTEXT_LEN];
    user_info_rslt_type rslt;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    if (appl_msg[idx++] != TAG_AARQ)
    {
        DPRINTF(DBG_ERR, _D "%s: Tag is not AARQ\r\n", __func__);
        return AARQ_SERVICE_NOT;
    }
    // tag+length+info(length)
    // length
    t8 = appl_msg[idx++];
    DPRINTF(DBG_TRACE, _D "%s: AARQ Length %d\r\n", __func__, t8);

    if ((idx + t8) > appl_len)
    {
        DPRINTF(DBG_ERR, _D "%s: AARQ Length Exceeded\r\n", __func__);
        return AARQ_SERVICE_NOT;
    }
    else if (t8 == 0)  // by WD
    {
        DPRINTF(DBG_ERR, _D "%s: AARQ Length Error\r\n", __func__);
        return AARQ_SERVICE_NOT;
    }
    // ctt (ex, 0x80 0x02, 0x02, 0x84 => protocol version ('100001') + tailing
    // bits ('00')) ACSE protocol version check: none or default (bit0 = 1)
    if (appl_msg[idx] == TAG_BITSTRING)
    {
        if ((appl_msg[idx + 3] & 0x80) == 0)
        {
            // not default protocol version
            return AARQ_PROTOCOL_VER_ERROR;
        }
        idx += 4;  // tag, length, unused_bits, bit_string byte(including
                   // tailing bits(0))
    }

    // application context name check
    memcpy(tbuf, &app_context_name[0], APP_CONTEXT_LEN);

    if (memcmp(&appl_msg[idx], tbuf, APP_CONTEXT_LEN) != 0)
    {
        return AARQ_APP_CONTEXT_ERROR;
    }

    idx += APP_CONTEXT_LEN;
    // skip the called and calling AP/AE title, qualifier, invocation id
    idx = skip_calling_data(idx);
    // authentication check (only low level auth)
    idx = check_auth(idx, &t8);

    if (t8 != 0)
    {
        return AARQ_AUTH_ERROR;
    }

    // implementation information processing ==> DLMS test case (12.4.2.6)
    if (appl_msg[idx] == TAG_IMPLEMENT_INFO)
    {
        return AARQ_IMPL_INF_NOT_SUPPORTED;
    }

    // process xDLMS-initiate.request
    rslt = proc_user_info(idx);

    DPRINTF(DBG_TRACE, _D "%s: result[%d]\r\n", __func__, rslt);
    return uinf_rslt_to_aarq_rslt(rslt);
}

uint8_t mecha_name_rslt = false;
aarq_rslt_type aarq_get_n_check_param(uint8_t tag, uint8_t *pdata,
                                      uint16_t *p_o_idx)
{
    uint16_t idx = 0, param_len = 0;
    // auth_val_asso3_type auth;
    appl_auth_type_asso3 auth;
    user_info_rslt_type rslt;

    DPRINTF(DBG_TRACE, _D "%s: AARQ TAG[0x%02X], idx[%d]\r\n", __func__, tag,
            *p_o_idx);

    switch (tag)
    {
        // protocol-version TLV: 80 02 07 80
        // Tag: 80 (Context-specific, Primitive, Tag 0)
        // Length: 02
        // Value: 07 80
        // 07 → 7 unused bits in the last byte
        // 80 → 10000000, ignoring the unused bits → actual version: 0000001 →
        // version = 1
    case ASSO_PROTO_VER:  // 0x80
    {
        if (pdata[idx] == TAG_BITSTRING)
        {
            if ((pdata[idx + 3] & 0x80) == 0)
            {
                return AARQ_PROTOCOL_VER_ERROR;
            }
            idx += 4;  // tag, length, unused_bits, bit_string byte(including
                       // tailing bits(0))
            *p_o_idx += idx;
        }

        break;
    }
    case ASSO_APP_CONTEXT_NAME:  // 0xA1
    {
        uint8_t *p_context_name;

        p_context_name = &app_context_name_asso3[0];
        DPRINT_HEX(DBG_TRACE, "APP_CONTEXT_NAME", &pdata[idx], APP_CONTEXT_LEN,
                   DUMP_DLMS);
        if (memcmp(&pdata[idx], p_context_name, APP_CONTEXT_LEN) != 0)
        {
            *p_o_idx += APP_CONTEXT_LEN;

            if (pdata[idx + APP_CONTEXT_LEN - 1] == APPCTXT_LN_CIPHERING)
            {
            }
            else
            {
                DPRINTF(DBG_ERR, _D "%s: AARQ_CONTEXT_ERROR\r\n", __func__);
                return AARQ_APP_CONTEXT_ERROR;
            }
        }
        else
            *p_o_idx += APP_CONTEXT_LEN;

        break;
    }
    case ASSO_CALLED_AP_TITLE:  // 0xA2

        break;
    case ASSO_CALLED_AE_QUALIFIER:  // 0xA3

        break;
    case ASSO_CALLED_AP_INVOC_ID:  // 0xA4

        break;
    case ASSO_CALLED_AE_INVOC_ID:  // 0xA5

        break;
    case ASSO_CALLING_AP_TITLE:  // 0xA6
    {
        uint8_t sys_title_len = 0;

        if (pdata[idx + 1] == 0x0A && pdata[idx + 2] == 0x04 &&
            pdata[idx + 3] == 0x08)
        {
            sys_title_len = pdata[idx + 3];
            memcpy(SYS_TITLE_client, &pdata[idx + 4], sys_title_len);
            DPRINT_HEX(DBG_CIPHER, "<-- Rx Client SystemTitle", &pdata[idx + 4],
                       sys_title_len, DUMP_SEC);
#if 1 /* bccho, HLS, 2023-08-14 */
            int rtn = hls_set_other_system_title(hls_ctx, SYS_TITLE_client,
                                                 sys_title_len);
            if (rtn != HLS_SUCCESS)
            {
                DPRINTF(DBG_ERR,
                        "hls_set_other_system_title() error!!!, %d\r\n", rtn);
                return AARQ_ERROR;
            }
#endif
            *p_o_idx += sys_title_len + 4;  // 4 -> tag, len, type, len
        }
        else
        {
            DPRINTF(DBG_WARN, _D "%s: AARQ_ERROR\r\n", __func__);
            return AARQ_ERROR;
        }

        break;
    }
    case ASSO_CALLING_AE_QUALIFIER:  // 0xA7
    {
        uint16_t cnt = 0, pos = 0;
        uint8_t *p_ae_qualifier = &calling_ae_qualifier[0];

        idx += 1;
        if (pdata[idx] == 0x82)
        {
            param_len =
                ((uint16_t)(pdata[idx + 1] << 8) | (uint16_t)(pdata[idx + 2]));

            p_ae_qualifier[cnt++] = tag;
            p_ae_qualifier[cnt++] = pdata[idx];
            p_ae_qualifier[cnt++] = pdata[idx + 1];
            p_ae_qualifier[cnt++] = pdata[idx + 2];
            pos = idx + 3;
        }
        else if (pdata[idx] == 0x81)
        {
            param_len = pdata[idx + 1];

            p_ae_qualifier[cnt++] = tag;
            p_ae_qualifier[cnt++] = pdata[idx];
            p_ae_qualifier[cnt++] = pdata[idx + 1];
            pos = idx + 2;
        }
        else
        {
            param_len = pdata[idx];

            p_ae_qualifier[cnt++] = tag;
            p_ae_qualifier[cnt++] = pdata[idx];
            pos = idx + 1;
        }

        memcpy(&p_ae_qualifier[cnt], &pdata[pos], param_len);
        cnt += param_len;
        *p_o_idx += cnt;

        DPRINT_HEX(DBG_WARN, "AE Qualifier", &calling_ae_qualifier[0], cnt,
                   DUMP_DLMS);
        OSTimeDly(OS_MS2TICK(1));

        DPRINTF(DBG_INFO, "%s: 0xA7 len(%d), pos_idx[%d], next_idx[%d]\r\n",
                __func__, param_len, pos, *p_o_idx);

        break;
    }
    case ASSO_CALLING_AP_INVOC_ID:  // 0xA8
        *p_o_idx += idx + 1;
        break;
    case ASSO_CALLING_AE_INVOC_ID:  // 0xA9
        *p_o_idx += idx + 1;
        break;
    case ASSO_SENDER_ACSE_REQU:  // 0x8A
    {
        uint8_t len;

        auth.acse_req = 0;
        if (pdata[idx + 1] == 2 && pdata[idx + 2] == 7 &&
            pdata[idx + 3] == 0x80)
        {
            auth.acse_req = 1;
            DPRINTF(DBG_TRACE, "acse_req[%d]\r\n", auth.acse_req);
        }

        len = pdata[idx + 1];
        idx += (len + 2);  // tag, len, data...
        *p_o_idx += idx;

        DPRINT_HEX(DBG_WARN, "ACSE_REQU", &pdata[0], idx, DUMP_DLMS);

        break;
    }
    case ASSO_REQ_MECHANISM_NAME:  // 0x8B
    {
        uint8_t len;
        uint8_t *p_mema_name_asso3 = &mecha_name_asso3[0];

        len = pdata[idx + 1];
        DPRINTF(DBG_TRACE, "len[%d]\r\n", len);
        if (len == MECHA_NAME_SIZE)
        {
            DPRINT_HEX(DBG_TRACE, "MECHA_NAME", &pdata[idx + 2],
                       MECHA_NAME_SIZE, DUMP_DLMS);
            if (memcmp(&pdata[idx + 2], p_mema_name_asso3, len) == 0)
            {
                mecha_name_rslt = true;
                DPRINTF(DBG_TRACE, "mecha_name_rslt[%d]\r\n", mecha_name_rslt);
            }
        }
        idx += (len + 2);  // tag, len, data
        *p_o_idx += idx;

        break;
    }
    case ASSO_CALLING_AUTH_VALUE:  // 0xAC
    {
        uint8_t len;

        len = pdata[idx + 1];
        DPRINTF(DBG_TRACE, "len[%d], mecha_name_rslt[%d]\r\n", len,
                mecha_name_rslt);
        if (mecha_name_rslt)
        {
            auth.auth_val.len = pdata[idx + 3];  // auth value is OK -> copy
            DPRINTF(DBG_TRACE, "auth.auth_val.len[%d]\r\n", auth.auth_val.len);
            memcpy(auth.auth_val.random, &pdata[idx + 4], auth.auth_val.len);
            memcpy(XTOX_c, &pdata[idx + 4], auth.auth_val.len);
            DPRINT_HEX(DBG_TRACE, "AARQ_RANDOM", auth.auth_val.random,
                       auth.auth_val.len, DUMP_DLMS);
        }
        idx += (len + 2);  // tag, len, data
        *p_o_idx += idx;

        break;
    }
    case ASSO_IMPLEMENTATION_INFO:  // 0xBD
    {
        *p_o_idx += idx + 1;
        return AARQ_IMPL_INF_NOT_SUPPORTED;
    }
    case ASSO_USER_INFORMATION:  // 0xBE
    {
        rslt = proc_user_info_asso3(&pdata[idx], p_o_idx);
        *p_o_idx += idx + 1;
        DPRINTF(DBG_INFO, _D "%s: ASSO_USER_INFORMATION rslt[%d]\r\n", __func__,
                rslt);
        return uinf_rslt_to_aarq_rslt(rslt);
    }
    default:
        *p_o_idx += idx + 1;
        break;
    }

    return AARQ_OK;
}

#if 1 /* bccho, 2024-08-12, 인증서 검증 */
#include "x509_crt.h"
#include "nv.h"

/* 아래 두 개는 전역으로 해야함. 스택에 정의하면 hardware fault 발생 */
static mbedtls_x509_crt amica_crt;
static mbedtls_x509_crt sign_crt;

static bool verify_certificate(uint8_t *pcert)
{
    uint8_t hash1[32] = {0};
    uint8_t hash2[32] = {0};

    bool ret = false;

    /* hash1 */
    nv_read(I_CERTI_HASH, hash1);
    /* hash2 */
    axiocrypto_hash(HASH_SHA_256, pcert, 500, hash2, 32);

    /* 인증서 검증 생략 */
    if (!memcmp(hash1, hash2, 32))
    {
        MSG00("skip--");
        return true;
    }

    extern uint8_t SubCa_cert[500];

    mbedtls_x509_crt_init(&amica_crt);
    mbedtls_x509_crt_init(&sign_crt);

    if (mbedtls_x509_crt_parse(&sign_crt, (unsigned char *)pcert, 500) != 0)
    {
        DPRINTF(DBG_ERR, _D "%s: AARQ DS parse Error\r\n", __func__);
        goto clear;
    }

    if (mbedtls_x509_crt_parse(&amica_crt, (unsigned char *)SubCa_cert, 500) !=
        0)
    {
        DPRINTF(DBG_ERR, _D "%s: AARQ SubCa parse Error\r\n", __func__);
        goto clear;
    }

    uint32_t flags;
    if (mbedtls_x509_crt_verify(&sign_crt, &amica_crt, NULL, NULL, &flags, NULL,
                                NULL) != 0)
    {
        DPRINTF(DBG_ERR, _D "%s: AARQ Certificate DS invalid\r\n", __func__);
        goto clear;
    }

    MSG00("hash write");
    nv_write(I_CERTI_HASH, hash2);

    ret = true;

clear:
    mbedtls_x509_crt_free(&amica_crt);
    mbedtls_x509_crt_free(&sign_crt);

    return ret;
}
#endif

aarq_rslt_type appl_AARQ_asso3_proc(int idx)
{
    uint16_t aarq_len;
    uint8_t len_ext, tag;
    aarq_rslt_type rslt = AARQ_SERVICE_NOT;

    DPRINTF(DBG_TRACE, _D "%s\r\n", __func__);
    if (appl_msg[idx++] != TAG_AARQ)
    {
        DPRINTF(DBG_ERR, _D "%s: Tag is not AARQ\r\n", __func__);
        return rslt;
    }
    // tag+length+info(length)
    // length
    len_ext = appl_msg[idx];
    if (len_ext == 0x82)
    {
        aarq_len = ((uint16_t)(appl_msg[idx + 1] << 8) |
                    (uint16_t)(appl_msg[idx + 2]));
        idx += 3;
    }
    else if (len_ext == 0x81)
    {
        aarq_len = appl_msg[idx + 1];
        idx += 2;
    }
    else
    {
        aarq_len = appl_msg[idx++];
    }
    DPRINTF(DBG_TRACE, _D "%s: AARQ Length %d\r\n", __func__, aarq_len);

    if ((idx + aarq_len) > appl_len)
    {
        DPRINTF(DBG_ERR, _D "%s: AARQ Length Exceeded\r\n", __func__);
        return rslt;
    }
    else if (aarq_len == 0)  // by WD
    {
        DPRINTF(DBG_ERR, _D "%s: AARQ Length Error\r\n", __func__);
        return rslt;
    }

    while (idx < aarq_len)
    {
        tag = appl_msg[idx];

        if (tag == ASSO_USER_INFORMATION)
        {
            rslt =
                aarq_get_n_check_param(tag, &appl_msg[idx], (uint16_t *)&idx);

            break;
        }
        else
        {
            rslt =
                aarq_get_n_check_param(tag, &appl_msg[idx], (uint16_t *)&idx);
            if (rslt != AARQ_OK)
            {
                return rslt;
            }
        }
    }
    DPRINTF(DBG_TRACE, _D "%s: result[%d] %s\r\n", __func__, rslt,
            ((rslt == AARQ_OK) ? "Success" : "Fail"));

#if 1 /* bccho, 2024-08-12, 인증서 검증 */
    if (rslt == AARQ_OK)
    {
        if (!verify_certificate(&calling_ae_qualifier[8]))
        {
            return AARQ_UINF_DK_UNSUPPORT;
        }

        dsm_sec_cert_put_client_ds(&calling_ae_qualifier[8]);
    }
#endif

    return rslt;
}

int appl_fill_context_name(uint8_t *buf)
{
    if (appl_is_sap_sec_site() || appl_is_sap_sec_utility())
    {
        memcpy(buf, &app_context_name_asso3[4], 7);
    }
    else
    {
        memcpy(buf, &app_context_name[4], 7);
    }
    return 7;
}

int appl_fill_mecha_name(uint8_t *buf)
{
    if (appl_is_sap_sec_site() || appl_is_sap_sec_utility())
    {
        memcpy(buf, &mecha_name_asso3[0], MECHA_NAME_SIZE);
    }
    else
    {
        memcpy(buf, &mecha_name[0], MECHA_NAME_SIZE);
    }
    return MECHA_NAME_SIZE;
}

int appl_fill_mecha_name_noauth(uint8_t *buf)
{
    memcpy(buf, &mecha_name_noauth[0], MECHA_NAME_SIZE);

    return MECHA_NAME_SIZE;
}

static aarq_rslt_type uinf_rslt_to_aarq_rslt(user_info_rslt_type rslt)
{
    aarq_rslt_type rtn;

    switch (rslt)
    {
    case UINF_OK:
        rtn = AARQ_OK;
        break;
    case UINF_DLMS_VER_LOW:
        rtn = AARQ_UINF_DLMS_VER_LOW;
        break;
    case UINF_CONFORMANCE_NOT_SUPPORTED:
        rtn = AARQ_UINF_CONFORMANCE_NOT;
        break;
    case UINF_CLIENT_RCV_LOW:
        rtn = AARQ_UINF_CLIENT_RCV_LOW;
        break;
    case UINF_DK_NOT_SUPPORTED:  // ctt 3.1
        rtn = AARQ_UINF_DK_UNSUPPORT;
        break;
    default:
        rtn = AARQ_UINF_ERROR;
        break;
    }

    return rtn;
}

static int skip_calling_data(int idx)
{
    uint8_t len;

    while (idx < appl_len)
    {
        if (appl_msg[idx] < TAG_CALLED_AP_TITLE ||
            appl_msg[idx] > TAG_CALLING_AE_INVOC)
            break;
        len = appl_msg[idx + 1];
        idx += len + 2;  // 2 means tag+len
    }

    return idx;
}

static bool cmp_pwd(appl_sap_type sap, auth_val_type *pwd)
{
    int rslt;

    rslt = -1;

    switch (sap)
    {
    case SAP_UTILITY:
        if (appl_util_pwd.len == pwd->len)
        {
            rslt = memcmp(appl_util_pwd.pwd, pwd->pwd, pwd->len);
        }
        break;
    case SAP_485COMM:
        if (appl_485_pwd.len == pwd->len)
        {
            rslt = memcmp(appl_485_pwd.pwd, pwd->pwd, pwd->len);
        }
        break;
    case SAP_PRIVATE:
        if (appl_priv_pwd.len == pwd->len)
        {
            rslt = memcmp(appl_priv_pwd.pwd, pwd->pwd, pwd->len);
        }
        break;
    default:
        rslt = -1;
        break;
    }
    DPRINTF(DBG_TRACE, _D "%s sap[%d], rslt[%d]\r\n", __func__, sap, rslt);
    return rslt == 0;
}

static int check_auth(int idx, uint8_t *rslt)
{
    appl_auth_type auth;

    *rslt = 1;  // NG

    idx = acse_require(idx, &auth);  // acse_require = 0 but auth_val exist in
                                     // DLMS test case (12.4.2.4.4)
    idx = get_auth_val(idx, &auth.auth_val);
    DPRINTF(DBG_TRACE, _D "%s: acse_req[%d], auth_val_len[%d]\r\n", __func__,
            auth.acse_req, auth.auth_val.len);
    // password compare
    if (auth.acse_req && auth.auth_val.len)
    {
        {
            if (appl_is_sap_utility())
            {
                if (cmp_pwd(SAP_UTILITY, &auth.auth_val))
                    *rslt = 0;
            }
            else if (appl_is_sap_485comm())
            {
                if (cmp_pwd(SAP_485COMM, &auth.auth_val))
                    *rslt = 0;
            }
            else if (appl_is_sap_private())
            {
                if (cmp_pwd(SAP_PRIVATE, &auth.auth_val))
                    *rslt = 0;
            }
            else if (appl_is_sap_sec_utility())
            {
            }
            else if (appl_is_sap_sec_site())
            {
            }
            DPRINT_HEX(DBG_TRACE, "SAP_AUTH", &auth.auth_val,
                       sizeof(auth_val_type), DUMP_ALWAYS);
        }
    }
    else
    {
        if (appl_is_sap_public())
            *rslt = 0;
    }

    return idx;
}

static int acse_require(int idx, appl_auth_type *auth)
{
    uint8_t len;

    auth->acse_req = 0;

    if (appl_msg[idx] == TAG_ACSE_REQUIRE)
    {
        if (appl_msg[idx + 1] == 2 && appl_msg[idx + 2] == 7 &&
            appl_msg[idx + 3] == 0x80)
            auth->acse_req = 1;

        len = appl_msg[idx + 1];
        idx += (len + 2);  // tag, len, data...
    }

    return idx;
}

static int get_auth_val(int idx, auth_val_type *auth_p)
{
    bool rslt;
    uint8_t len;
    uint8_t tbuf[MECHA_NAME_SIZE];

    rslt = false;

    auth_p->len = 0;

    if (appl_msg[idx] == TAG_MECHA_NAME)
    {
        len = appl_msg[idx + 1];
        if (len == MECHA_NAME_SIZE)
        {
            memcpy(tbuf, &mecha_name[0], len);
            if (memcmp(&appl_msg[idx + 2], tbuf, len) == 0)
                rslt = true;
        }
        idx += (len + 2);  // tag, len, data
    }

    if (appl_msg[idx] == TAG_MECHA_VAL)
    {
        len = appl_msg[idx + 1];
        if (rslt)
        {
            auth_p->len = appl_msg[idx + 3];  // auth value is OK -> copy
            memcpy(auth_p->pwd, &appl_msg[idx + 4], auth_p->len);
        }
        idx += (len + 2);  // tag, len, data
    }

    return idx;
}

static user_info_rslt_type proc_user_info(int idx)
{
    uint16_t len;

    if (appl_msg[idx++] != TAG_USER_INFO)
        return UINF_ERROR;

    len = (uint16_t)appl_msg[idx++];
    if ((idx + len) != appl_len)  // length check
        return UINF_ERROR;

    idx += 2;  // skip choice(OCTET STRING), length

    // DLMS initiate request
    if (appl_msg[idx++] != TAG_DLMS_INITREQ)
        return UINF_ERROR;

    // dedicated key(OPTIONAL) for ciphering assumes no ciphering
    if (appl_msg[idx++] != 0)
        return UINF_DK_NOT_SUPPORTED;

    // response allowed parameter shall always be set to TRUE, so test is
    // skipped (test case 12.4.2.7.2)
    idx += 1;

    // proposed quality of service can be omitted or present (test
    // case 12.4.2.7.3) => unsolved
    if (appl_msg[idx++] != 0)
        idx += 1;

    // proposed dlms version number(must be >= 6)
    if (appl_msg[idx++] < 0x06)
        return UINF_DLMS_VER_LOW;

    if (appl_msg[idx] == 0x5f && appl_msg[idx + 1] == 0x1f &&
        appl_msg[idx + 2] == 0x04 && appl_msg[idx + 3] == 0x00)
        idx += 4;
    else if (appl_msg[idx] == 0x5f && appl_msg[idx + 1] == 0x04 &&
             appl_msg[idx + 2] ==
                 0x00)  // for compliance with existing implementations
        idx += 3;
    else
        return UINF_ERROR;
    if (check_get_conformance_block(&appl_msg[idx]) == false)
        return UINF_CONFORMANCE_NOT_SUPPORTED;
    idx += 3;

    // client max receive pdu size
    ToH16((U8_16 *)&len, &appl_msg[idx]);
    if (len != 0 && len < 12)
        return UINF_CLIENT_RCV_LOW;  // 0 means no limit
    // client max receive pdu size -> not supported -> skip

    return UINF_OK;
}

static user_info_rslt_type proc_user_info_asso3(uint8_t *pdata,
                                                uint16_t *p_o_idx)
{
    uint16_t len, idx = 0;

    if (pdata[idx++] != TAG_USER_INFO)
        return UINF_ERROR;

    len = (uint16_t)pdata[idx++];
    DPRINTF(DBG_TRACE, "len[%d], *p_o_idx[%d], appl_len[%d]\r\n", len, *p_o_idx,
            appl_len);

    if ((*p_o_idx + len + idx) != appl_len)
        return UINF_ERROR;

    idx += 2;  // skip choice(OCTET STRING), length

    // DLMS initiate request
    if (pdata[idx++] != TAG_DLMS_INITREQ)
        return UINF_ERROR;

    // dedicated key(OPTIONAL) for ciphering assumes no ciphering
    if (pdata[idx++] != 0)
        return UINF_DK_NOT_SUPPORTED;

    // response allowed parameter shall always be set to TRUE, so test is
    // skipped (test case 12.4.2.7.2)
    idx += 1;

    // proposed quality of service can be omitted or present (test
    // case 12.4.2.7.3) => unsolved
    if (pdata[idx++] != 0)
        idx += 1;

    // proposed dlms version number(must be >= 6)
    if (pdata[idx++] < 0x06)
        return UINF_DLMS_VER_LOW;

    if (pdata[idx] == 0x5f && pdata[idx + 1] == 0x1f &&
        pdata[idx + 2] == 0x04 && pdata[idx + 3] == 0x00)
        idx += 4;
    else if (pdata[idx] == 0x5f && pdata[idx + 1] == 0x04 &&
             pdata[idx + 2] ==
                 0x00)  // for compliance with existing implementations
        idx += 3;
    else
        return UINF_ERROR;
    if (check_get_conformance_block(&pdata[idx]) == false)
        return UINF_CONFORMANCE_NOT_SUPPORTED;
    idx += 3;

    // client max receive pdu size
    ToH16((U8_16 *)&len, &pdata[idx]);
    if (len != 0 && len < 12)
        return UINF_CLIENT_RCV_LOW;  // 0 means no limit
    // client max receive pdu size -> not supported -> skip

    *p_o_idx += idx;

    return UINF_OK;
}

static bool check_get_conformance_block(uint8_t *pdata)
{
    int idx = 0;
    if (appl_is_sap_assign_kepco_mnt())
    {
        appl_conformance[0] = pdata[idx++] & ASS3_LN_SUPPORTED_CONFORMANCE_1;
        appl_conformance[1] = pdata[idx++] & ASS3_LN_SUPPORTED_CONFORMANCE_2;
        appl_conformance[2] = pdata[idx++] & ASS3_LN_SUPPORTED_CONFORMANCE_3;

        if ((appl_conformance[0] == 0) && (appl_conformance[1] == 0) &&
            (appl_conformance[2] == 0))
            return false;
    }
    else
    {
        appl_conformance[0] = 0;  // to avoid compile warning bcz of
                                  // (LN_SUPPORTED_CONFORMANCE_1 == 0)
        idx++;
        appl_conformance[1] = pdata[idx++] & LN_SUPPORTED_CONFORMANCE_2;
        appl_conformance[2] = pdata[idx++] & LN_SUPPORTED_CONFORMANCE_3;

        if ((appl_conformance[0] == 0) && (appl_conformance[1] == 0) &&
            (appl_conformance[2] == 0))
            return false;
    }

    return true;
}
