/*
******************************************************************************
*   INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_uart.h"
#include "defines.h"
#include "amg_crc.h"
#include "amg_modemif_prtl.h"
#include "amg_gpio.h"
#include "amg_media_mnt.h"
#include "whm.h"
#include "nv.h"
#include "amg_meter_main.h"
#include "amg_shell.h"
#include "amg_sec.h"
#include "amg_rtc.h"
#include "amg_power_mnt.h"
#include "dl.h"
#include "amg_wdt.h"
#include "options_sel.h"
#include "amg_shell_product.h"

/*
******************************************************************************
*    Definition
******************************************************************************
*/
#define _D "[modem_IF] "

#define AT_CMD_POS 3
#define AT_DELIMETER_SIZE 1

#if 1 /* bccho, 2023-11-17 */
#define FWUP_MAX_RETRY_COUNT 15
#else
#define FWUP_MAX_RETRY_COUNT 5
#endif
#define ATCMD_MMID_RETRY_MAX_COUNT 2

#define ATCMD_ANSWER_AFTER_TX 1
#define ATCMD__NO_ANSWER_AFTER_TX 0

#define AT_EN_DE_NONE 0
#define AT_EN_DE_START 1
#define AT_EN_DE_PARM_OK 2

#define IMODEM_DEFAULT_BAUD 115200
#define EMODEM_DEFAULT_BAUD 460800

#define MODEM_RX_POLL_TIME 2000    // ms
#define MODEM_RX_POLL_TIME_2 2000  // ms
#define FWUP_DIRECT_RETRY_NUM 1
#define MODEM_RX_POLL_RETRY_TIME 500

#define EXT_MODEM_RX_POLL_TIME 2500   // ms
#define EXT_MODEM_RX_POLL_TIME_2 500  // ms
#define EXT_MODEM_RX_POLL_TIME_3 500  // ms

/*
******************************************************************************
*    MACRO
******************************************************************************
*/

/*
******************************************************************************
*    DATA TYPE
******************************************************************************
*/
typedef struct _int_modem_wdt_info_
{
    uint32_t low_cnt;
    uint32_t high_cnt;
    uint32_t sec_tick;
} ST_IMOMEM_WDT_INFO;

/*
******************************************************************************
*    GLOBAL VARIABLE
******************************************************************************
*/
ST_AT_CMD_RX_PKT gst_atcmd_rx_pkt;
ST_AT_CMD_TX_PKT gst_atcmd_tx_pkt;

ST_MDM_MIC_FWUP_DL gst_mdm_mic_fwupdl;
EN_FWU_FSM g_mdm_mic_fwup_fsm = FWU_FSM_NONE;
uint8_t g_bearer = BEARER_NONE;

/* bccho, 2024-09-05, 삼상 */
uint8_t g_mdlink_mode = SUN_DEVICE_OFF_ASSO;
bool power_notify_tx_set = 0;

#ifdef MTP_ZCD_ON_OFF
U8 g_zcd_on_ing_sts = 0;
#endif

#if 0
extern bool g_modem_exist;  // jp.kim 25.01.20
#endif

extern ST_FW_IMG_DOWNLOAD_INFO gst_fw_image_dlinfo;

// clang-format off
/*
모뎀 제어 AT 명령어
+============================================================+==============================================================================
| AT Cmommand                                                | Description
+============================================================+==============================================================================
| AT+BEARER?                                                 | Network Bearer 정보 조회
| AT+MAC?                                                    | MAC 어드레스 조회
| AT+MAC<address>                                            | MAC 어드레스 설정
| AT+MMID?                                                   | 모뎀 기기 ID 조회
| AT+HWVER?                                                  | 모뎀 HW 버전 조회
| AT+FWVER?                                                  | 모뎀 FW 버전 조회
| AT+FUN<fun>                                                | 모뎀 기능 설정 0: modemreset, 1: modemfactoryreset
| AT+TXPWR?                                                  | 모뎀 TX power 조회
| AT+TXPWR<power>                                            | 모뎀 TX power 설정
| AT+PMAC?                                                   | 부모의 MAC 어드레스 조회
| AT+PANID?                                                  | 네트워크 PAN ID 조회
| AT+PANID<panid>                                            | 네트워크 PAN ID 설정
| AT+ASSOPAN?                                                | 네트워크 조인 여부 조회 0: 미 조인 상태, 1: 조인 완료
| AT+ASSOPER?                                                | 네트워크 조인 허가 조회 0: permit 불허 (L2 MESH 불허), 1: permit 허용 (L2 MESH 허용)
| AT+ASSOPER<permit>                                         | 네트워크 조인 허가 설정
| AT+NWKKEY?                                                 | AES-128용 대칭키 조회
| AT+NWKKEY<network_key>                                     | AES-128용 대칭키 설정
| AT+FWUPINIT0                                               | 모뎀 F/W 업데이트 설정 초기화
| AT+FWUPSTA?                                                | 모뎀 F/W 업데이트 상태 조회
| AT+FWUP<FW_version,length,more flag,offset,image_data,crc> | 모뎀 F/W 이미지 전송
| AT+FWUPOP<fwupdate>                                        | 모뎀 F/W 이미지 업데이트 적용 설정
| AT+PANDLST?                                                | 모뎀의 PAN 노드 MAC 어드레스 List조회
| AT+NEWPAND=<macaddress-계기ID>                               | 모뎀의 신규 PAN 노드 MAC 어드레스 Trap
| AT+METERID?                                                | 계기 ID 조회
| AT+METERID<계기ID>                                           | 계기 ID 설정
| AT+MFDATE?                                                 | 계기 제조일자 조회
| AT+MFDATE<YY.MM>                                           | 계기 제조일자 설정
| AT+MODE?                                                   | 모뎀의 동작 모드 확인 0: 코디네이터 동작, 1: 라우터 동작
| AT+MODE<mode>                                              | 모뎀의 동작 모드 설정
| AT+BAUD?                                                   | 모뎀 시리얼 Baud rate 확인
| AT+BAUD<baud rate>                                         | 모뎀 시리얼 Baud rate 설정
| AT+SHA256?                                                 | 모뎀 펌웨어 이미지의 SHA256 확인
| AT+NWK?                                                    | Network Connectivity 정보 조회
+============================================================+==============================================================================
*/
// clang-format on
static const ATCMD_CNTX atcmd_mt_list[] = {{ATCMD_BEARER, "BEARER", 0},
                                           {ATCMD_MAC, "MAC", 1},
                                           {ATCMD_PPDU, "PPDU", 1},
                                           {ATCMD_MMID, "MMID", 0},
                                           {ATCMD_HWVER, "HWVER", 0},
                                           {ATCMD_FWVER, "FWVER", 0},
                                           {ATCMD_FUN, "FUN", 1},
                                           {ATCMD_TXPWR, "TXPWR", 1},
                                           {ATCMD_NWKDR, "NWKDR", 1},
                                           {ATCMD_PMAC, "PMAC", 1},
                                           {ATCMD_PANID, "PANID", 1},
                                           {ATCMD_CHAN, "CHAN", 1},
                                           {ATCMD_ASSOPAN, "ASSOPAN", 0},
                                           {ATCMD_ASSOPER, "ASSOPER", 1},
                                           {ATCMD_MAXBE, "MAXBE", 1},
                                           {ATCMD_MAXRTY, "MAXRTY", 1},
                                           {ATCMD_RNS, "RNS", 1},
                                           {ATCMD_NWKKEY, "NWKKEY", 1},
                                           {ATCMD_FWUPSTA, "FWUPSTA", 0},
                                           {ATCMD_FWUP, "FWUP", 6},
                                           {ATCMD_FWUPOP, "FWUPOP", 1},
                                           {ATCMD_FWUPINIT, "FWUPINIT", 1},
                                           {ATCMD_PANDLST, "PANDLST", 1},
                                           {ATCMD_NEWPAND, "NEWPAND", 1},
                                           {ATCMD_METERID, "METERID", 1},
                                           {ATCMD_MFDATE, "MFDATE", 1},
                                           {ATCMD_EBR, "EBR", 1},
                                           {ATCMD_EBLIST, "EBLIST", 1},
                                           {ATCMD_MODE, "MODE", 1},
                                           {ATCMD_BAUD, "BAUD", 1},
                                           {ATCMD_SHA256, "SHA256", 0},
                                           {ATCMD_LISTEN, "LISTEN", 1},
                                           {ATCMD_POLL, "POLL", 0},
                                           {ATCMD_ENCRYPT, "ENCRYPT", 3},
                                           {ATCMD_DECRYPT, "DECRYPT", 3},
                                           {ATCMD_485, "485", 1},
                                           {ATCMD_ZCD, "ZCD", 1},
                                           {ATCMD_ZCDT, "ZCDT", 1},
                                           {ATCMD_VFREE, "VFREE", 1},
                                           /* bccho, 2024-09-05, 삼상 */
                                           {ATCMD_MDLINK, "MDLINK", 0},
                                           {ATCMD_PWRNOTIFY, "PWRNOTIFY", 0},
                                           {ATCMD_STDVER, "STDVER", 0},
                                           {ATCMD_ZCDOFFSET, "ZCDOFFSET", 0},
                                           {ATCMD_ZCDEN, "ZCDEN", 0},
                                           {0, 0, 0}};

/*
******************************************************************************
*   LOCAL VARIABLES
******************************************************************************
*/

ST_MDM_ID gst_int_modem_id;
uint8_t g_sun_listen = AT_LISTEN_OFF;
ST_ATCMD_TMP_BUF gst_atcmd_from_client[MAX_MODEM_TYPE];
ST_ATCMD_TMP_BUF gst_atcmd_from_modem[MAX_MODEM_TYPE];
uint8_t g_atcmd_modem_fwup_retry_cnt = 0;
ST_AT_BAUD gst_mdm_baud;
EN_MODEM_BAUD_RATE_VAL gen_imodem_baud_rate = E_MODEM_BAUD_RATE_v115200;
EN_MODEM_BAUD_RATE_VAL gen_emodem_baud_rate = E_MODEM_BAUD_RATE_v460800;

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
int dsm_atcmd_get_param(uint8_t* instr, uint8_t* outstr, char delimiter);
void dsm_imodem_rx_int_cb(void);
void dsm_emodem_rx_int_cb(void);
void dsm_atcmd_get_mdlink(uint32_t poll_flag, bool retry_enable);
uint8_t str_to_bin32_n(char* str, uint32_t* value, uint32_t cnt);
void dsm_atcmd_get_rsp_stdver(uint32_t poll_flag, uint32_t err, uint8_t* pdata,
                              uint8_t data_len);
void dsm_atcmd_get_rsp_zcdoffset(uint32_t poll_flag, uint32_t err,
                                 uint8_t* pdata, uint8_t data_len);
void dsm_atcmd_get_rsp_zcden(uint32_t poll_flag, uint32_t err, uint8_t* pdata,
                             uint8_t data_len);
bool is_dc33_off(void);
void dsm_atcmd_set_trap_power_notify(uint32_t poll_flag, uint8_t data);
void dsm_mif_zcd_on_meter_only(void);
void dsm_mif_zcd_off_meter_only(void);

/*
******************************************************************************
*    LOCAL FUNCTIONS
******************************************************************************
*/

ST_AT_BAUD* dsm_modemif_get_atbaud_info(void) { return &gst_mdm_baud; }

EN_MODEM_BAUD_RATE_VAL dsm_modemif_atbaud_2_baudvalue(EN_AT_BAUD at_baud)
{
    EN_MODEM_BAUD_RATE_VAL ret_val = E_MODEM_BAUD_RATE_v115200;
    switch (at_baud)
    {
    case AT_BAUD_230400:
        ret_val = E_MODEM_BAUD_RATE_v230400;

        break;
    case AT_BAUD_460800:
        ret_val = E_MODEM_BAUD_RATE_v460800;

        break;
    default:

        break;
    }

    return ret_val;
}

EN_AT_BAUD dsm_modemif_baudvalue_2_atbaud(uint32_t baud_rate)
{
    EN_AT_BAUD ret = AT_BAUD_115200;

    switch (baud_rate)
    {
    case E_MODEM_BAUD_RATE_v115200:
        ret = AT_BAUD_115200;

        break;
    case E_MODEM_BAUD_RATE_v230400:
        ret = AT_BAUD_230400;

        break;
    case E_MODEM_BAUD_RATE_v460800:
        ret = AT_BAUD_460800;
        break;
    default:

        break;
    }

    return ret;
}

void dsm_modemif_default_baudrate(void)
{
    DPRINTF(DBG_WARN, "%s\r\n", __func__);
    gst_mdm_baud.imodem = AT_BAUD_115200;
    gst_mdm_baud.emodem = AT_BAUD_460800;
    nv_write(I_MODEM_BAUD, (uint8_t*)&gst_mdm_baud);
}

void dsm_modemif_baud_road(void)
{
    if (!nv_read(I_MODEM_BAUD, (uint8_t*)&gst_mdm_baud))
    {
        dsm_modemif_default_baudrate();
    }
    else
    {
        if (gst_mdm_baud.imodem > AT_BAUD_460800 ||
            gst_mdm_baud.emodem > AT_BAUD_460800)
        {
            dsm_modemif_default_baudrate();
        }
    }

    gen_imodem_baud_rate = dsm_modemif_atbaud_2_baudvalue(gst_mdm_baud.imodem);
    gen_emodem_baud_rate = dsm_modemif_atbaud_2_baudvalue(gst_mdm_baud.emodem);

    DPRINTF(DBG_WARN, "%s imodem %d, emodem %d\r\n", __func__,
            gen_imodem_baud_rate, gen_emodem_baud_rate);
}

void dsm_modemif_baud_nvwrite(ST_AT_BAUD* pst_at_baud)
{
    if (!nv_write(I_MODEM_BAUD, (uint8_t*)pst_at_baud))
    {
        dsm_modemif_default_baudrate();
    }

    gen_imodem_baud_rate = dsm_modemif_atbaud_2_baudvalue(pst_at_baud->imodem);
    gen_emodem_baud_rate = dsm_modemif_atbaud_2_baudvalue(pst_at_baud->emodem);

    DPRINTF(DBG_WARN, "%s imodem %d, emodem %d\r\n", __func__,
            gen_imodem_baud_rate, gen_emodem_baud_rate);
}

void dsm_imodemif_init(uint8_t first)
{
    dsm_uart_close(IMODEM_PORT);

    dsm_uart_init(IMODEM_PORT, gen_imodem_baud_rate, FALSE, NULL, 2048, NULL,
                  2048, FALSE);

    DPRINTF(DBG_TRACE, "%s: baud %d\r\n", __func__, gen_imodem_baud_rate);

    dsm_uart_reg_rx_callback(IMODEM_PORT, dsm_imodem_rx_int_cb);
    dsm_atcmd_rx_pkt_init();

    if (first)
        sun_set_no_listen();
}

void dsm_imodemif_deinit(void)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    dsm_gpio_imodem_pf_low();     /* PF set */
    dsm_gpio_imodem_reset_low();  // reset_low
    dsm_gpio_imodem_power_disable();
    dsm_uart_close(IMODEM_PORT);
}

void dsm_emodemif_init(void)
{
    dsm_uart_close(EMODEM_PORT);
    dsm_uart_init(EMODEM_PORT, gen_emodem_baud_rate, FALSE, NULL, 2048, NULL,
                  2048, FALSE);

    DPRINTF(DBG_TRACE, "%s: baud %d\r\n", __func__, gen_emodem_baud_rate);
    dsm_uart_reg_rx_callback(EMODEM_PORT, dsm_emodem_rx_int_cb);
    dsm_atcmd_rx_pkt_init();
}

char* dsm_bearer_string(uint8_t bearer)
{
    switch (bearer)
    {
    case BEARER_NONE:
        return "NONE";
    case BEARER_SUN_MAC:
        return "SUN_MAC";
    case BEARER_SUN_IP:
        return "SUN_IP";
    case BEARER_LTE:
        return "LTE";
    case BEARER_iotPLC:
        return "iotPLC";
    case BEARER_HPGP:
        return "HPGP";
    case BEARER_C_SMGW:
        return "C_SMGW";

    default:
        return "Unknown";
    }
}

char* dsm_baud_string(uint8_t baud)
{
    switch (baud)
    {
    case AT_BAUD_115200:
        return "Baud_115200";
    case AT_BAUD_230400:
        return "Baud_230400";
    case AT_BAUD_460800:
        return "Baud_460800";

    default:
        return "Baud_Unknown";
    }
}

uint8_t dsm_get_bearer(void) { return g_bearer; }

void dsm_set_bearer(uint8_t bearer)
{
    if (g_bearer != bearer)
    {
        DPRINTF(DBG_WARN, _D "BEARER[%s -> %s]\r\n",
                dsm_bearer_string(g_bearer), dsm_bearer_string(bearer));
        g_bearer = bearer;
    }
}

void dsm_atcmd_client_modem_buff_init(uint8_t modem_type)
{
    memset(&gst_atcmd_from_client[modem_type], 0x00, sizeof(ST_ATCMD_TMP_BUF));
    memset(&gst_atcmd_from_modem[modem_type], 0x00, sizeof(ST_ATCMD_TMP_BUF));
}

ST_ATCMD_TMP_BUF* dsm_get_atcmd_from_client(uint8_t modem_type)
{
    return &gst_atcmd_from_client[modem_type];
}

ST_ATCMD_TMP_BUF* dsm_get_atcmd_from_modem(uint8_t modem_type)
{
    return &gst_atcmd_from_modem[modem_type];
}

uint8_t at_error_test_sts = 0;
bool dsm_atcmd_rx_poll(COM_PORT COM, uint32_t ms,
                       ST_ATCMD_TMP_BUF* pst_atmcd_from_modem)
{
    char ch;
    uint32_t cnt = 0, pre_time = OS_TIME_GET(), timeout = MS2TIMER(ms);
#if 1 /* bccho, 2023-11-17 */
    uint8_t timeout_raw_ms = 10, cr_cnt = 0;
#else
    uint8_t timeout_raw_ms = 1, cr_cnt = 0;
#endif
    bool result = false;
    int16_t k;

    dsm_uart_disable_rx_intr(COM);

    while (1)
    {
#if 1 /* bccho, 2023-11-10 */
        if (dsm_uart_raw_getc(COM, timeout_raw_ms, &ch))
#else
        if (dsm_uart_raw_getc_timeout(COM, timeout_raw_ms, &ch))
#endif
        {
            pst_atmcd_from_modem->string[cnt++] = ch;
            if (ch == '\r')
            {
                if (++cr_cnt > 1)
                {
                    pst_atmcd_from_modem->len = cnt;
                    DPRINT_HEX(DBG_TRACE, "AT_RX_1",
                               pst_atmcd_from_modem->string, cnt, DUMP_ALWAYS);

#if 1  // at_cmd fail recovery //jp.kim 25.03.15
                    if ((pst_atmcd_from_modem->string[0] == AT_SECOND_T) &&
                        (pst_atmcd_from_modem->string[1] == AT_plus))
                    {
                        for (k = cnt; k > 0; k--)
                        {
                            pst_atmcd_from_modem->string[k] =
                                pst_atmcd_from_modem->string[k - 1];
                        }
                        pst_atmcd_from_modem->string[0] = AT_FIRST_A;
                        cnt++;
                        pst_atmcd_from_modem->len = cnt;
                        DPRINT_HEX(DBG_TRACE, "AT_RX_3",
                                   pst_atmcd_from_modem->string, cnt,
                                   DUMP_ALWAYS);
                    }
#endif
                    result = true;
                    break;
                }
            }
        }
        else if ((OS_TIME_GET() - pre_time) >= timeout)
        {
            DPRINTF(DBG_TRACE, "%s: Timeout[%lu]\r\n", __func__, cnt);
            if (cnt)
            {
                DPRINT_HEX(DBG_TRACE, "GET", pst_atmcd_from_modem->string, cnt,
                           DUMP_MDM | DUMP_EMDM);
            }
            break;
        }
    }

    dsm_uart_enable_rx_intr(COM);

    dsm_wdt_ext_toggle_immd();

    return result;
}

void dsm_atcmd_if_poll_rx_init(uint8_t modem_type)
{
    if (INT_MODEM_TYPE == modem_type)
    {
        dsm_uart_close(IMODEM_PORT);
        dsm_uart_peri_rx_poll_init(gen_imodem_baud_rate, IMODEM_PORT);
    }
    else
    {
        dsm_uart_close(EMODEM_PORT);
        dsm_uart_peri_rx_poll_init(gen_emodem_baud_rate, EMODEM_PORT);
    }
}

void dsm_atcmd_if_default_init(uint8_t modem_type)
{
    if (INT_MODEM_TYPE == modem_type)
    {
        dsm_imodemif_init(FALSE);
    }
    else
    {
        dsm_emodemif_init();
    }
}

bool dsm_atcmd_if_rx_polling(uint8_t modem_type,
                             ST_ATCMD_TMP_BUF* pst_atmcd_from_modem)
{
    bool ret = false;

    if (INT_MODEM_TYPE == modem_type)
    {
        ret = dsm_atcmd_rx_poll(IMODEM_PORT, MODEM_RX_POLL_TIME,
                                pst_atmcd_from_modem);
    }
    else
    {
        ret = dsm_atcmd_rx_poll(EMODEM_PORT, EXT_MODEM_RX_POLL_TIME,
                                pst_atmcd_from_modem);
    }
    return ret;
}

bool dsm_atcmd_if_rx_polling_valid_check(uint8_t modem_type,
                                         ST_ATCMD_TMP_BUF* pst_atmcd_from_modem)
{
    bool ret = false;

    if (INT_MODEM_TYPE == modem_type)
    {
        ret = dsm_atcmd_rx_poll(IMODEM_PORT, MODEM_RX_POLL_TIME,
                                pst_atmcd_from_modem);
    }
    else
    {
        ret = dsm_atcmd_rx_poll(EMODEM_PORT, EXT_MODEM_RX_POLL_TIME_3,
                                pst_atmcd_from_modem);
    }
    return ret;
}

void dsm_atcmd_poll_tx_n_rx(uint8_t modem_type,
                            ST_ATCMD_TMP_BUF* pst_atmcd_client,
                            ST_ATCMD_TMP_BUF* pst_atmcd_from_modem)
{
    if (INT_MODEM_TYPE == modem_type)
    {
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
        dsm_media_if_send(MEDIA_PROTOCOL_IF_ATCMD, TRUE,
                          pst_atmcd_client->string, pst_atmcd_client->len);

        dsm_atcmd_rx_poll(IMODEM_PORT, MODEM_RX_POLL_TIME_2,
                          pst_atmcd_from_modem);

        if (pst_atmcd_from_modem->len > CLIENT_ATCMD_STRING_MAX_SIZE)
            pst_atmcd_from_modem->len = CLIENT_ATCMD_STRING_MAX_SIZE;
        DPRINT_HEX(DBG_TRACE, "AT_I", &pst_atmcd_from_modem->string[0],
                   pst_atmcd_from_modem->len, DUMP_ALWAYS);
    }
    else if (EXT_MODEM_TYPE == modem_type)  // jp.kim 25.01.21
    {
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
        dsm_media_if_send(MEDIA_PROTOCOL_IF_ATCMD, TRUE,
                          pst_atmcd_client->string, pst_atmcd_client->len);

        dsm_atcmd_rx_poll(EMODEM_PORT, EXT_MODEM_RX_POLL_TIME_2,
                          pst_atmcd_from_modem);

        if (pst_atmcd_from_modem->len > CLIENT_ATCMD_STRING_MAX_SIZE)
            pst_atmcd_from_modem->len = CLIENT_ATCMD_STRING_MAX_SIZE;
        DPRINT_HEX(DBG_TRACE, "AT_E", pst_atmcd_from_modem->string,
                   pst_atmcd_from_modem->len, DUMP_ALWAYS);
    }
    else  // jp.kim 25.01.21
    {
    }
}

char* dsm_mdm_mic_fwup_fsm_string(uint32_t fsm)
{
    switch (fsm)
    {
    case FWU_FSM_NONE:
        return "FWU_none";
    case FWU_FSM_INIT:
        return "FWU_init";
    case FWU_FSM_UP:
        return "FWU_ing";
    case FWU_FSM_UP_OP:
        return "FWU_op";
    case FWU_FSM_SUCCESS:
        return "FWU_success";
    case FWU_FSM_FAIL:
        return "FWU_fail";

    default:
        return "FWU_Unknown";
    }
}

void dsm_mdm_mic_fwup_set_fsm(uint8_t state)
{
    if (g_mdm_mic_fwup_fsm != state)
    {
        DPRINTF(DBG_WARN, _D "FSM[%s -> %s]\r\n",
                dsm_mdm_mic_fwup_fsm_string(g_mdm_mic_fwup_fsm),
                dsm_mdm_mic_fwup_fsm_string(state));
        g_mdm_mic_fwup_fsm = state;
    }
}

uint8_t dsm_mdm_mic_fwup_get_fsm(void) { return g_mdm_mic_fwup_fsm; }

ST_MDM_MIC_FWUP_DL* dsm_mdm_mic_get_fwupdlinfo(void)
{
    return &gst_mdm_mic_fwupdl;
}

ATCMD_CNTX* dsm_atcmd_get_mt_cntx(uint32_t cmdid)
{
    return (ATCMD_CNTX*)&atcmd_mt_list[cmdid];
}

bool dsm_mdm_mic_fwup_init_txproc(void)
{
    ST_FW_IMG_DOWNLOAD_INFO* pimage_dlinfo = dsm_imgtrfr_get_fw_image_dlinfo();
    ST_MDM_MIC_FWUP_DL* pmdm_mic_fwdl = dsm_mdm_mic_get_fwupdlinfo();
    uint8_t modem_type;
    ST_ATCMD_TMP_BUF st_atmcd_from_modem;
    ST_AT_CMD_RX_PKT atcmd_com_pkt;
    uint8_t cnt = 0;
    bool ret = false;
    DPRINTF(DBG_TRACE, "%s: FW_TYPE[%s]\r\n", __func__,
            dsm_fw_type_string(pimage_dlinfo->fw_type));

    switch (pimage_dlinfo->fw_type)
    {
    case FW_DL_SYS_PART:

        break;

    case FW_DL_INT_MDM:
    case FW_DL_EXT_MDM:
    {
        if (FW_DL_INT_MDM == pimage_dlinfo->fw_type)
        {
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
        }
        else
        {
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
        }

        if (FW_DL_INT_MDM == pimage_dlinfo->fw_type)
        {
            modem_type = INT_MODEM_TYPE;
        }
        else
        {
            modem_type = EXT_MODEM_TYPE;
        }

        while (cnt < FWUP_DIRECT_RETRY_NUM)
        {
            ret = false;
            dsm_atcmd_fwup_set_init(TRUE);

            dsm_atcmd_tx_pkt_init();
            dsm_atcmd_if_rx_polling(modem_type, &st_atmcd_from_modem);
            //
            // DPRINT_HEX
            if (dsm_atcmd_rx_parser((uint8_t*)st_atmcd_from_modem.string,
                                    st_atmcd_from_modem.len,
                                    &atcmd_com_pkt) == AT_ERR_NONE_PARSER_OK)
            {
                ret = dsm_atcmd_poll_rx_proc(&atcmd_com_pkt);
                if (ret)
                {
                    break;
                }
            }
            dsm_atcmd_rx_pkt_init();
            atcmd_com_pkt.len = 0;
            cnt++;
        }

        if (!ret)
        {
            return false;
        }

        memcpy(pmdm_mic_fwdl->name, pimage_dlinfo->name,
               IMAGE_FW_NAME_MAX_SIZE);
        pmdm_mic_fwdl->image_size = pimage_dlinfo->image_size;
        pmdm_mic_fwdl->fw_type = pimage_dlinfo->fw_type;
        pmdm_mic_fwdl->start_dl_addr = 0;
        pmdm_mic_fwdl->blk_size = pimage_dlinfo->blk_size;
        pmdm_mic_fwdl->snd_cnt = 0;
        pmdm_mic_fwdl->retry_cnt = 0;
        pmdm_mic_fwdl->blk_num =
            (pmdm_mic_fwdl->image_size / pimage_dlinfo->blk_size);
        if (pmdm_mic_fwdl->image_size % pimage_dlinfo->blk_size)
        {
            pmdm_mic_fwdl->blk_num += 1;
        }

        uint8_t fw_name[IMAGE_FW_NAME_MAX_SIZE + 1] = {0};
        memcpy(fw_name, pimage_dlinfo->name, IMAGE_FW_NAME_MAX_SIZE);
        DPRINTF(DBG_WARN, "========================================\r\n");
        DPRINTF(DBG_WARN, "Block Size: %d, Block Count: %d\r\n",
                pimage_dlinfo->blk_size, pimage_dlinfo->blk_number);
        DPRINTF(DBG_WARN, "Addr: 0x%08X, Image Size: %d\r\n",
                pimage_dlinfo->start_dl_addr, pimage_dlinfo->image_size);
        DPRINTF(DBG_WARN, "FW Type: %d, Image Identifier: %s\r\n",
                pimage_dlinfo->fw_type, fw_name);
        DPRINTF(DBG_WARN, "========================================\r\n");
    }
    break;

    case FW_DL_METER_PART:

        break;
    case FW_DL_TYPE_NONE:
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);

        break;
    }

    return true;
}

void dsm_mdm_mic_fwup_up_txproc(void)
{
    uint8_t fw_type = dsm_imgtrfr_get_fw_type();
    ST_MDM_MIC_FWUP_DL* pmdm_mic_fwdl = dsm_mdm_mic_get_fwupdlinfo();

    DPRINTF(DBG_TRACE, _D "fwup_up: FW_T: %s\r\n", dsm_fw_type_string(fw_type));

    switch (fw_type)
    {
    case FW_DL_SYS_PART:

        break;
    case FW_DL_INT_MDM:
    case FW_DL_EXT_MDM:
    {
        uint32_t offset = pmdm_mic_fwdl->blk_size * pmdm_mic_fwdl->snd_cnt;
        uint32_t addr = pmdm_mic_fwdl->start_dl_addr + offset;
        uint32_t addr_max =
            pmdm_mic_fwdl->start_dl_addr + pmdm_mic_fwdl->image_size;

        DPRINTF(DBG_TRACE, "%s: addr[0x%08X] cnt[%d], offset[%d]\r\n", __func__,
                addr, pmdm_mic_fwdl->snd_cnt, offset);
        if (FW_DL_INT_MDM == fw_type)
        {
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
        }
        else
        {
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
        }

        if (addr < addr_max)
        {
            if (pmdm_mic_fwdl->blk_size >= (addr_max - addr))
                dsm_atcmd_fwup_set_imgdata(FALSE, pmdm_mic_fwdl->name, addr,
                                           pmdm_mic_fwdl->blk_size, 0, offset);
            else
                dsm_atcmd_fwup_set_imgdata(FALSE, pmdm_mic_fwdl->name, addr,
                                           pmdm_mic_fwdl->blk_size, 1, offset);
        }
    }
    break;
    case FW_DL_METER_PART:

        break;
    case FW_DL_TYPE_NONE:
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);

        break;
    }
}

bool dsm_mdm_mic_fwup_direct_up_txproc(uint8_t* pimg_seg, uint16_t len)
{
    uint8_t fw_type = dsm_imgtrfr_get_fw_type();
    ST_MDM_MIC_FWUP_DL* pmdm_mic_fwdl = dsm_mdm_mic_get_fwupdlinfo();
    uint8_t modem_type;
    ST_ATCMD_TMP_BUF st_atmcd_from_modem;
    ST_AT_CMD_RX_PKT atcmd_com_pkt;
    uint8_t cnt = 0;
    bool ret = false;

    DPRINTF(DBG_TRACE, "FW_TYPE: %s\r\n", dsm_fw_type_string(fw_type));

    switch (fw_type)
    {
    case FW_DL_SYS_PART:

        break;
    case FW_DL_INT_MDM:
    case FW_DL_EXT_MDM:
    {
        uint32_t offset = pmdm_mic_fwdl->blk_size * pmdm_mic_fwdl->snd_cnt;
        uint32_t addr = pmdm_mic_fwdl->start_dl_addr + offset;
        uint32_t addr_max =
            pmdm_mic_fwdl->start_dl_addr + pmdm_mic_fwdl->image_size;

        DPRINTF(DBG_TRACE, "FW_IMG: cnt[%d], offset[%d], pktlen[%d]\r\n",
                pmdm_mic_fwdl->snd_cnt, offset, len);

        if (FW_DL_INT_MDM == fw_type)
        {
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
            modem_type = INT_MODEM_TYPE;
        }
        else
        {
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
            modem_type = EXT_MODEM_TYPE;
        }

        while (cnt < FWUP_DIRECT_RETRY_NUM)
        {
            ret = false;

            if (addr < addr_max)
            {
                if ((addr_max - addr) <= pmdm_mic_fwdl->blk_size)

                    DPRINTF(
                        DBG_TRACE,
                        "%s: addr[0x%08X] gst_fw_image_dlinfo.image_size[%d]   "
                        "pmdm_mic_fwdl->image_size[%d]\r\n",
                        __func__, addr, gst_fw_image_dlinfo.image_size,
                        pmdm_mic_fwdl->image_size);

#if 1  // jp.kim 25.03.13
                if (((addr_max - addr) <= pmdm_mic_fwdl->blk_size) ||
                    (len < pmdm_mic_fwdl->blk_size))
#else
                if ((addr_max - addr) <= pmdm_mic_fwdl->blk_size)
#endif
                {
                    dsm_atcmd_fwup_direct_set_imgdata(TRUE, pmdm_mic_fwdl->name,
                                                      pimg_seg, len, 0,
                                                      offset);  // last packet
                }
                else
                {
                    dsm_atcmd_fwup_direct_set_imgdata(
                        TRUE, pmdm_mic_fwdl->name, pimg_seg,
                        pmdm_mic_fwdl->blk_size, 1, offset);
                }
            }

            dsm_atcmd_tx_pkt_init();
            memset(&st_atmcd_from_modem, 0x00, sizeof(st_atmcd_from_modem));

            if (dsm_atcmd_if_rx_polling(modem_type, &st_atmcd_from_modem) &&
                st_atmcd_from_modem.len)
            {
                if (dsm_atcmd_rx_parser((uint8_t*)st_atmcd_from_modem.string,
                                        st_atmcd_from_modem.len,
                                        &atcmd_com_pkt) ==
                    AT_ERR_NONE_PARSER_OK)
                {
                    ret = dsm_atcmd_poll_rx_proc(&atcmd_com_pkt);
                    if (ret)
                        break;
                }
                OSTimeDly(OS_MS2TICK(MODEM_RX_POLL_RETRY_TIME));
            }

            dsm_atcmd_rx_pkt_init();
            atcmd_com_pkt.len = 0;
            cnt++;
        }

        if (!ret)
        {
            // DPRINTF
            ////dsm_debug_set_level(DBG_INFO);

            return false;
        }
    }

    break;
    case FW_DL_METER_PART:

        break;
    case FW_DL_TYPE_NONE:
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);

        break;
    }

    return true;
}

bool dsm_mdm_mic_fwup_op_txproc(void)
{
    uint8_t fw_type = dsm_imgtrfr_get_fw_type();
    ST_MDM_MIC_FWUP_DL* pmdm_mic_fwdl = dsm_mdm_mic_get_fwupdlinfo();
    uint8_t modem_type;
    ST_ATCMD_TMP_BUF st_atmcd_from_modem;
    ST_AT_CMD_RX_PKT atcmd_com_pkt;
    uint8_t cnt = 0;
    bool ret = false;

    DPRINTF(DBG_TRACE, _D "%s: FW_TYPE: %s\r\n", __func__,
            dsm_fw_type_string(fw_type));

    switch (fw_type)
    {
    case FW_DL_SYS_PART:

        break;
    case FW_DL_INT_MDM:
    case FW_DL_EXT_MDM:
        if (FW_DL_INT_MDM == fw_type)
        {
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
            modem_type = INT_MODEM_TYPE;
        }
        else
        {
            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
            modem_type = EXT_MODEM_TYPE;
        }

        while (cnt < FWUP_DIRECT_RETRY_NUM)
        {
            ret = false;
            dsm_atcmd_fwup_set_action(TRUE, pmdm_mic_fwdl->name);

            dsm_atcmd_tx_pkt_init();
            dsm_atcmd_if_rx_polling(modem_type, &st_atmcd_from_modem);

            if (dsm_atcmd_rx_parser((uint8_t*)st_atmcd_from_modem.string,
                                    st_atmcd_from_modem.len,
                                    &atcmd_com_pkt) == AT_ERR_NONE_PARSER_OK)
            {
                ret = dsm_atcmd_poll_rx_proc(&atcmd_com_pkt);
                if (ret)
                    break;
            }
            OSTimeDly(OS_MS2TICK(MODEM_RX_POLL_RETRY_TIME));
            dsm_atcmd_rx_pkt_init();
            atcmd_com_pkt.len = 0;
            cnt++;
        }

        if (!ret)
        {
            return false;
        }
        break;
    case FW_DL_METER_PART:

        break;
    case FW_DL_TYPE_NONE:
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);

        break;
    }

    return true;
}

void dsm_mdm_mic_fwup_success_txproc(void)
{
    uint8_t fw_type = dsm_imgtrfr_get_fw_type();
    ST_MDM_MIC_FWUP_DL* pmdm_mic_fwdl = dsm_mdm_mic_get_fwupdlinfo();

    DPRINTF(DBG_TRACE, _D "%s: FW_TYPE: %s\r\n", __func__,
            dsm_fw_type_string(fw_type));

    switch (fw_type)
    {
    case FW_DL_SYS_PART:

        break;

    case FW_DL_INT_MDM:
    case FW_DL_EXT_MDM:
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);
        memset((uint8_t*)pmdm_mic_fwdl, 0x00, sizeof(ST_MDM_MIC_FWUP_DL));

        break;
    case FW_DL_METER_PART:

        break;
    case FW_DL_TYPE_NONE:
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);

        break;
    }
}

void dsm_mdm_mic_fwup_fail_txproc(void)
{
    uint8_t fw_type = dsm_imgtrfr_get_fw_type();
    ST_MDM_MIC_FWUP_DL* pmdm_mic_fwdl = dsm_mdm_mic_get_fwupdlinfo();

    DPRINTF(DBG_TRACE, _D "%s: FW_TYPE: %s\r\n", __func__,
            dsm_fw_type_string(fw_type));

    switch (fw_type)
    {
    case FW_DL_SYS_PART:

        break;
    case FW_DL_INT_MDM:
    case FW_DL_EXT_MDM:
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);
        dsm_meter_sw_timer_start(MT_SW_TIMER_FWUP_RESTART_FOR_MODEM_TO, FALSE,
                                 MT_TIMEOUT_FWUP_RESTART_TIME);
        pmdm_mic_fwdl->retry_cnt = 0;

        break;
    case FW_DL_METER_PART:

        break;
    case FW_DL_TYPE_NONE:
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);

        break;
    }
}

void dsm_mdm_mic_fwup_retry_TO_proc(void)
{
    ST_MDM_MIC_FWUP_DL* pmdm_mic_fwdl = dsm_mdm_mic_get_fwupdlinfo();

    DPRINTF(DBG_TRACE, _D "%s: FW_TYPE: %s\r\n", __func__,
            dsm_fw_type_string(pmdm_mic_fwdl->fw_type));
    DPRINTF(DBG_WARN, _D "%s: fail_FSM: %s\r\n", __func__,
            dsm_mdm_mic_fwup_fsm_string(pmdm_mic_fwdl->fail_fsm));

    if (pmdm_mic_fwdl->fail_fsm == FWU_FSM_UP_OP)
    {
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_UP_OP);
    }
    else
    {
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_INIT);
    }
    dsm_fwup_fsm_send();
}

uint32_t dsm_mdm_mic_fwup_fsm_tx_proc(void)
{
    uint8_t fsm = dsm_mdm_mic_fwup_get_fsm();

    switch (fsm)
    {
    case FWU_FSM_INIT:
        dsm_mdm_mic_fwup_init_txproc();

        break;
    case FWU_FSM_UP:
        dsm_mdm_mic_fwup_up_txproc();

        break;
    case FWU_FSM_UP_OP:
        dsm_mdm_mic_fwup_op_txproc();

        break;
    case FWU_FSM_SUCCESS:
        dsm_mdm_mic_fwup_success_txproc();

        break;
    case FWU_FSM_FAIL:
        dsm_mdm_mic_fwup_fail_txproc();

        break;
    default:
        break;
    }

    return TRUE;
}

bool dsm_mdm_mic_fwup_direct_fsm_tx_proc(uint8_t* pdata, uint16_t len)
{
    bool ret = false;
    uint8_t fsm = dsm_mdm_mic_fwup_get_fsm();
    switch (fsm)
    {
    case FWU_FSM_INIT:
        ret = dsm_mdm_mic_fwup_init_txproc();

        return ret;
    case FWU_FSM_UP:
        ret = dsm_mdm_mic_fwup_direct_up_txproc(pdata, len);

        return ret;
    case FWU_FSM_UP_OP:
        ret = dsm_mdm_mic_fwup_op_txproc();

        return ret;
    case FWU_FSM_SUCCESS:
        dsm_mdm_mic_fwup_success_txproc();

        break;
    case FWU_FSM_FAIL:
        dsm_mdm_mic_fwup_fail_txproc();

        break;
    default:
        break;
    }
    return true;
}

void dsm_atcmd_rx_pkt_init(void)
{
    memset(&gst_atcmd_rx_pkt, 0x00, sizeof(ST_AT_CMD_RX_PKT));
}

void dsm_atcmd_tx_pkt_init(void)
{
    memset(&gst_atcmd_tx_pkt, 0x00, sizeof(ST_AT_CMD_TX_PKT));
}

ST_AT_CMD_RX_PKT* dsm_atcmd_get_parser_pkt(void) { return &gst_atcmd_rx_pkt; }

ST_AT_CMD_TX_PKT* dsm_atcmd_get_txpkt(void) { return &gst_atcmd_tx_pkt; }

void dsm_atcmd_parser_FSM_transition(uint8_t state)
{
    dsm_atcmd_get_parser_pkt()->fsm = state;
}

uint8_t dsm_atcmd_get_parser_FSM(void)
{
    return dsm_atcmd_get_parser_pkt()->fsm;
}

void dsm_atcmd_fsm_rxbuf_reset(void)
{
    ST_AT_CMD_RX_PKT* atcmd_pkt = dsm_atcmd_get_parser_pkt();

    dsm_atcmd_parser_FSM_transition(AT_FSM_A);
    atcmd_pkt->cnt = 0;
    atcmd_pkt->cmd_cnt = 0;
    atcmd_pkt->cmd_type = 0;
    atcmd_pkt->data_cnt = 0;
    atcmd_pkt->data_ok_cnt = 0;
    atcmd_pkt->len = 0;

    atcmd_pkt->en_de_state = 0;
    atcmd_pkt->en_de_parm_cnt = 0;
    atcmd_pkt->en_de_hd_len = 0;
    atcmd_pkt->en_de_data_len = 0;
}

void dsm_modem_hw_reset(uint8_t type)
{
    DPRINTF(DBG_WARN, "%s: %d\r\n", __func__, type);
    if (type == INT_MODEM_RESET)
    {
        dsm_gpio_imodem_reset_high();
        OSTimeDly(OS_MS2TICK(10));
        dsm_gpio_imodem_reset_low();
        OSTimeDly(OS_MS2TICK(10));
        dsm_gpio_imodem_reset_high();
    }
    else
    {
        dsm_gpio_e_modem_reset_high();
        OSTimeDly(OS_MS2TICK(10));
        dsm_gpio_e_modem_reset_low();
        OSTimeDly(OS_MS2TICK(100));  // 규격 반영
        dsm_gpio_e_modem_reset_high();
    }
}

uint8_t dsm_atcmd_is_en_decrpyt(ST_AT_CMD_RX_PKT* atcmd_pkt)
{
    uint8_t cmd_len, ret = FALSE;
    uint8_t* cmd_ptr = NULL;
    ATCMD_CNTX* cmd_cntx = NULL;

    cmd_ptr = &atcmd_pkt->pkt[AT_CMD_POS];
    cmd_cntx = (ATCMD_CNTX*)&atcmd_mt_list[ATCMD_ENCRYPT];
    cmd_len = strlen(cmd_cntx->cmdStr);

    if (cmd_len == atcmd_pkt->cmd_cnt && atcmd_pkt->cmd_type == AT_Colon)
    {
        if (strncmp((char*)cmd_ptr, cmd_cntx->cmdStr, cmd_len) == 0)
        {
            DPRINTF(DBG_WARN, "_________encrypt_is_ok\r\n");
            return TRUE;
        }
    }

    cmd_cntx = (ATCMD_CNTX*)&atcmd_mt_list[ATCMD_DECRYPT];
    cmd_len = strlen(cmd_cntx->cmdStr);

    if (cmd_len == atcmd_pkt->cmd_cnt && atcmd_pkt->cmd_type == AT_Colon)
    {
        if (strncmp((char*)cmd_ptr, cmd_cntx->cmdStr, cmd_len) == 0)
        {
            DPRINTF(DBG_WARN, "_________decrypt_is_ok\r\n");
            return TRUE;
        }
    }
    return ret;
}

uint8_t dsm_atcmd_rx_parser(uint8_t* buff, uint16_t size,
                            ST_AT_CMD_RX_PKT* p_com_pkt)
{
    uint32_t i;
    uint8_t result = AT_ERR_NONE;
    ST_AT_CMD_RX_PKT* atcmd_pkt = dsm_atcmd_get_parser_pkt();

    for (i = 0; i < size; i++)
    {
        atcmd_pkt->pkt[atcmd_pkt->cnt++] = buff[i];
        switch (dsm_atcmd_get_parser_FSM())
        {
        case AT_FSM_A:
            if (buff[i] == AT_FIRST_A)
            {
                dsm_atcmd_parser_FSM_transition(AT_FSM_T);
            }
            else
            {
                result = AT_ERR_FISRT_A;
                dsm_atcmd_fsm_rxbuf_reset();
                DPRINT_HEX(DBG_ERR, "ERROR_STX_A", &buff[i], 1, DUMP_MDM);
            }
            break;
        case AT_FSM_T:
            if (buff[i] == AT_SECOND_T)
            {
                dsm_atcmd_parser_FSM_transition(AT_FSM_plus);
            }
            else
            {
                result = AT_ERR_SECOND_T;
                dsm_atcmd_fsm_rxbuf_reset();
                DPRINT_HEX(DBG_ERR, "ERROR_STX_T", &buff[i], 1, DUMP_MDM);
            }
            break;
        case AT_FSM_plus:
            if (buff[i] == AT_plus)
            {
                dsm_atcmd_parser_FSM_transition(AT_FSM_CMD);
            }
            else
            {
                result = AT_ERR_PLUS;
                dsm_atcmd_fsm_rxbuf_reset();
                DPRINT_HEX(DBG_ERR, "ERROR_STX_+", &buff[i], 1, DUMP_MDM);
            }
            break;
        case AT_FSM_CMD:
            if (buff[i] == AT_Question || buff[i] == AT_Colon ||
                buff[i] == AT_Equal)
            {
                dsm_atcmd_parser_FSM_transition(AT_FSM_TYPE);

                if (buff[i] == AT_Question)
                {
                    dsm_atcmd_parser_FSM_transition(AT_FSM_GET_END);
                }

                atcmd_pkt->cmd_type = buff[i];
            }
            else
            {
                if (buff[i] >= '0' && buff[i] <= 'z')
                {
                    atcmd_pkt->cmd_cnt++;
                    if (atcmd_pkt->cmd_cnt > AT_CMD_MAX_SIZE)
                    {
                        result = AT_ERR_CMD_LEN;
                        dsm_atcmd_fsm_rxbuf_reset();
                        DPRINT_HEX(DBG_ERR, "ERROR_CMD_LEN", &buff[i], 1,
                                   DUMP_MDM);
                    }
                }
                else
                {
                    result = AT_ERR_CMD_VALIDITY;
                    dsm_atcmd_fsm_rxbuf_reset();
                    DPRINT_HEX(DBG_ERR, "ERROR_CMD", &buff[i], 1, DUMP_MDM);
                }
            }

            break;
        case AT_FSM_TYPE:
            if (atcmd_pkt->cmd_type == AT_Question)
                dsm_atcmd_parser_FSM_transition(AT_FSM_GET_END);
            else if (atcmd_pkt->cmd_type == AT_Colon)
            {
                dsm_atcmd_parser_FSM_transition(AT_FSM_GET_RSP_DATA);
                atcmd_pkt->data_cnt++;
                if (dsm_atcmd_is_en_decrpyt(atcmd_pkt))
                {
                    atcmd_pkt->en_de_state = AT_EN_DE_START;
                }
            }
            else if (atcmd_pkt->cmd_type == AT_Equal)
            {
                atcmd_pkt->data_cnt++;
                dsm_atcmd_parser_FSM_transition(AT_FSM_SET_DATA);
            }
            else
            {
                result = AT_ERR_TYPE;
                dsm_atcmd_fsm_rxbuf_reset();
                DPRINT_HEX(DBG_ERR, "ERROR_TYPE", &buff[i], 1, DUMP_MDM);
            }

            break;
        case AT_FSM_GET_END:
            if (buff[i] == 0x0d)
            {
                result = AT_ERR_NONE_PARSER_OK;
                memcpy(p_com_pkt, atcmd_pkt, sizeof(ST_AT_CMD_RX_PKT));
                dsm_atcmd_fsm_rxbuf_reset();

                // return result;
            }
            else
            {
                result = AT_ERR_GET_END;
                dsm_atcmd_fsm_rxbuf_reset();
                DPRINT_HEX(DBG_ERR, "ERROR_ETX", &buff[i], 1, DUMP_MDM);
            }

            break;
        case AT_FSM_GET_RSP_DATA:
            if (atcmd_pkt->en_de_state == AT_EN_DE_START)
            {
                if (buff[i] == ',')
                {
                    atcmd_pkt->en_de_parm_cnt++;

                    if (atcmd_pkt->en_de_parm_cnt == 2)
                    {
                        int8_t outstr[32];
                        uint8_t data_pos;
                        uint8_t* pdata;

                        data_pos =
                            AT_CMD_POS + AT_DELIMETER_SIZE + atcmd_pkt->cmd_cnt;
                        pdata = &atcmd_pkt->pkt[data_pos];

                        atcmd_pkt->data_cnt++;

                        memset(outstr, 0, sizeof(outstr));
                        dsm_atcmd_get_param(pdata, (uint8_t*)outstr, ',');
                        atcmd_pkt->en_de_data_len = atoi((char*)outstr);

                        atcmd_pkt->en_de_hd_len = atcmd_pkt->data_cnt;
                        atcmd_pkt->en_de_state = AT_EN_DE_PARM_OK;

                        DPRINT_HEX(DBG_WARN, "EN_DE_HDR",
                                   &atcmd_pkt->pkt[data_pos],
                                   atcmd_pkt->en_de_hd_len, DUMP_ALWAYS);
                    }
                    else
                    {
                        atcmd_pkt->data_cnt++;
                    }
                    if (atcmd_pkt->data_cnt > AT_GET_RSP_DATA_MAX_SIZE)
                    {
                        result = AT_ERR_GET_RSP_DATA_LEN;
                        dsm_atcmd_fsm_rxbuf_reset();
                        DPRINT_HEX(DBG_ERR, "ERROR_GET_LEN", &buff[i], 1,
                                   DUMP_MDM);
                    }
                }
                else
                {
                    atcmd_pkt->data_cnt++;
                    if (atcmd_pkt->data_cnt > AT_GET_RSP_DATA_MAX_SIZE)
                    {
                        result = AT_ERR_GET_RSP_DATA_LEN;
                        dsm_atcmd_fsm_rxbuf_reset();
                        DPRINT_HEX(DBG_ERR, "ERROR_GET_LEN", &buff[i], 1,
                                   DUMP_MDM);
                    }
                }
            }
            else if (atcmd_pkt->en_de_state == AT_EN_DE_PARM_OK)
            {
                if (atcmd_pkt->data_cnt ==
                        (atcmd_pkt->en_de_hd_len + atcmd_pkt->en_de_data_len) &&
                    (buff[i] == 0x0d))
                {
                    result = AT_ERR_NONE_PARSER_OK;

                    DPRINTF(DBG_WARN, "EN_DE_DATA_OK[%d]\r\n",
                            atcmd_pkt->data_cnt);
                    memcpy(p_com_pkt, atcmd_pkt, sizeof(ST_AT_CMD_RX_PKT));
                    dsm_atcmd_fsm_rxbuf_reset();

                    // return result;
                }
                else
                {
                    atcmd_pkt->data_cnt++;
                    if (atcmd_pkt->data_cnt > AT_GET_RSP_DATA_MAX_SIZE)
                    {
                        result = AT_ERR_GET_RSP_DATA_LEN;
                        dsm_atcmd_fsm_rxbuf_reset();
                        DPRINT_HEX(DBG_ERR, "ERROR_GET_LEN", &buff[i], 1,
                                   DUMP_MDM);
                    }
                }
            }
            else
            {
                if (buff[i] == 0x0d)
                {
                    result = AT_ERR_NONE_PARSER_OK;
                    memcpy(p_com_pkt, atcmd_pkt, sizeof(ST_AT_CMD_RX_PKT));
                    dsm_atcmd_fsm_rxbuf_reset();

                    // return result;
                }
                else
                {
                    atcmd_pkt->data_cnt++;
                    if (atcmd_pkt->data_cnt > AT_GET_RSP_DATA_MAX_SIZE)
                    {
                        result = AT_ERR_GET_RSP_DATA_LEN;
                        dsm_atcmd_fsm_rxbuf_reset();
                        DPRINT_HEX(DBG_ERR, "ERROR_GET_LEN", &buff[i], 1,
                                   DUMP_MDM);
                    }
                }
            }
            break;
        case AT_FSM_SET_DATA:
            if (buff[i] == 0x0d)
                dsm_atcmd_parser_FSM_transition(AT_FSM_SET_DATA_END);
            else
            {
                atcmd_pkt->data_cnt++;

                if (atcmd_pkt->data_cnt > AT_SET_DATA_MAX_SIZE)
                {
                    result = AT_ERR_SET_DATA_LEN;
                    dsm_atcmd_fsm_rxbuf_reset();
                    DPRINT_HEX(DBG_ERR, "ERROR_SET_LEN", &buff[i], 1, DUMP_MDM);
                }
            }

            break;
        case AT_FSM_SET_DATA_END:
            if (buff[i] == 0x0d)
            {
                result = AT_ERR_NONE_PARSER_OK;
                memcpy(p_com_pkt, atcmd_pkt, sizeof(ST_AT_CMD_RX_PKT));
                dsm_atcmd_fsm_rxbuf_reset();

                // return result;
            }
            else
            {
                atcmd_pkt->data_ok_cnt++;
                if (atcmd_pkt->data_ok_cnt > AT_SET_DATA_OK_MAX_SIZE)
                {
                    result = AT_ERR_SET_DATAOK_LEN;
                    dsm_atcmd_fsm_rxbuf_reset();
                    DPRINT_HEX(DBG_ERR, "ERROR_SET_LEN", &buff[i], 1, DUMP_MDM);
                }
            }

            break;
        }

        if (result == AT_ERR_NONE_PARSER_OK)
        {
            i++;
            break;
        }
    }

    p_com_pkt->idx = i;

    return result;
}

uint32_t dsm_atcmd_is_valid_fwup_rsp(uint8_t* pdata, uint8_t* ok_pos)
{
    uint8_t cnt = 0;

    while (1)
    {
        if (pdata[cnt] == 0x0d)
        {
            if (pdata[cnt - 1] == '0')
            {
                *ok_pos = cnt + 1;
                DPRINTF(DBG_TRACE, "%s: crc ok, ok_pos[%d, %c%c]\r\n", __func__,
                        *ok_pos, pdata[cnt + 1], pdata[cnt + 2]);

                if (pdata[cnt + 1] == 'O' && pdata[cnt + 2] == 'K')
                {
                    return TRUE;
                }
                else
                    return FALSE;
            }
            else
            {
                DPRINTF(DBG_ERR, "%s: crc fail\r\n", __func__);
                return FALSE;
            }
        }
        if (cnt > 50)
        {
            break;
        }
        cnt++;
    }
    return FALSE;
}

// void dsm_atcmd_fwup_rx_proc(uint8_t cmd_id, uint8_t* pdata, uint16_t
// data_len)
// {
//     uint8_t fsm = dsm_mdm_mic_fwup_get_fsm();
//     ST_MDM_MIC_FWUP_DL* pmdm_mic_fwdl = dsm_mdm_mic_get_fwupdlinfo();

//     DPRINTF(DBG_TRACE, "%s: fsm %s\r\n", __func__,
//             dsm_mdm_mic_fwup_fsm_string(fsm));

//     switch (fsm)
//     {
//     case FWU_FSM_INIT:
//         if (cmd_id == ATCMD_FWUPINIT)
//         {
//             if (pdata[0] == '0' && pdata[2] == 'O' && pdata[3] == 'K')
//             {
//                 pmdm_mic_fwdl->retry_cnt = 0;
//                 DPRINTF(DBG_TRACE, "%s: ATCMD_FWUPINIT rxok\r\n", __func__);
//                 dsm_mdm_mic_fwup_set_fsm(FWU_FSM_UP);
//             }
//             else
//                 pmdm_mic_fwdl->retry_cnt++;
//         }
//         else
//             pmdm_mic_fwdl->retry_cnt++;

//         if (pmdm_mic_fwdl->retry_cnt > FWUP_MAX_RETRY_COUNT)
//         {
//             pmdm_mic_fwdl->fail_fsm = ATCMD_FWUPINIT;
//             DPRINTF(DBG_ERR, "%s: ATCMD rx fail: fail_fsm[%s]\r\n", __func__,
//                     dsm_mdm_mic_fwup_fsm_string(pmdm_mic_fwdl->fail_fsm));
//             dsm_mdm_mic_fwup_set_fsm(FWU_FSM_FAIL);
//         }

//         dsm_fwup_fsm_send();

//         break;
//     case FWU_FSM_UP:
//         if (cmd_id == ATCMD_FWUP)
//         {
//             uint8_t ok_pos;
//             uint32_t offset;
//             uint32_t addr;
//             uint32_t addr_max;

//             if (dsm_atcmd_is_valid_fwup_rsp(pdata, &ok_pos))
//             {
//                 DPRINTF(DBG_TRACE,
//                         "%s: ATCMD_FWUP rx ok: snd_cnt[%d],
//                         retry_cnt[%d]\r\n",
//                         __func__, pmdm_mic_fwdl->snd_cnt,
//                         pmdm_mic_fwdl->retry_cnt);
//                 pmdm_mic_fwdl->snd_cnt++;
//                 pmdm_mic_fwdl->retry_cnt = 0;
//                 g_atcmd_modem_fwup_retry_cnt = 0;
//             }
//             else
//             {
//                 pmdm_mic_fwdl->retry_cnt++;
//             }

//             offset = pmdm_mic_fwdl->blk_size * pmdm_mic_fwdl->snd_cnt;
//             addr = pmdm_mic_fwdl->start_dl_addr + offset;
//             addr_max = pmdm_mic_fwdl->start_dl_addr +
//             pmdm_mic_fwdl->image_size;

//             if (addr >= addr_max)
//             {
//                 dsm_mdm_mic_fwup_set_fsm(FWU_FSM_UP_OP);
//             }
//         }
//         else
//         {
//             pmdm_mic_fwdl->retry_cnt++;
//         }

//         if (pmdm_mic_fwdl->retry_cnt > 0)
//         {
//             DPRINTF(DBG_WARN,
//                     "%s: ATCMD_FWUP rx fail: snd_cnt[%d], retry_cnt[%d]\r\n",
//                     __func__, pmdm_mic_fwdl->snd_cnt,
//                     pmdm_mic_fwdl->retry_cnt);
//         }

//         if (pmdm_mic_fwdl->retry_cnt > FWUP_MAX_RETRY_COUNT)
//         {
//             pmdm_mic_fwdl->fail_fsm = FWU_FSM_UP;
//             DPRINTF(DBG_ERR, "%s: ATCMD rx fail: fail_fsm[%s]\r\n", __func__,
//                     dsm_mdm_mic_fwup_fsm_string(pmdm_mic_fwdl->fail_fsm));
//             dsm_mdm_mic_fwup_set_fsm(FWU_FSM_FAIL);
//         }
//         dsm_fwup_fsm_send();

//         break;
//     case FWU_FSM_UP_OP:
//         if (cmd_id == ATCMD_FWUPOP)
//         {
//             if (pdata[0] == '0' && pdata[2] == 'O' && pdata[3] == 'K')
//             {
//                 pmdm_mic_fwdl->retry_cnt = 0;
//                 dsm_mdm_mic_fwup_set_fsm(FWU_FSM_SUCCESS);
//                 DPRINTF(DBG_TRACE, "%s: FWU_FSM_UP_OP :ATCMD_FWUPOP
//                 rxok\r\n",
//                         __func__);
//             }
//             else
//             {
//                 pmdm_mic_fwdl->retry_cnt++;
//             }
//         }
//         else
//             pmdm_mic_fwdl->retry_cnt++;

//         if (pmdm_mic_fwdl->retry_cnt > FWUP_MAX_RETRY_COUNT)
//         {
//             pmdm_mic_fwdl->fail_fsm = FWU_FSM_UP_OP;
//             DPRINTF(DBG_ERR, "%s: ATCMD rx fail: fail_fsm[%s]\r\n", __func__,
//                     dsm_mdm_mic_fwup_fsm_string(pmdm_mic_fwdl->fail_fsm));
//             dsm_mdm_mic_fwup_set_fsm(FWU_FSM_FAIL);
//         }

//         dsm_fwup_fsm_send();

//         break;
//     default:
//         break;
//     }
// }

bool dsm_atcmd_poll_fwup_rx_proc(uint8_t cmd_id, uint8_t* pdata,
                                 uint16_t data_len)
{
    bool ret = false;
    uint8_t fsm = dsm_mdm_mic_fwup_get_fsm();
    ST_MDM_MIC_FWUP_DL* pmdm_mic_fwdl = dsm_mdm_mic_get_fwupdlinfo();

    DPRINTF(DBG_TRACE, "%s: fsm %s\r\n", __func__,
            dsm_mdm_mic_fwup_fsm_string(fsm));

    switch (fsm)
    {
    case FWU_FSM_INIT:
        if (cmd_id == ATCMD_FWUPINIT)
        {
            if (pdata[0] == '0' && pdata[2] == 'O' && pdata[3] == 'K')
            {
                pmdm_mic_fwdl->retry_cnt = 0;
                DPRINTF(DBG_TRACE, "%s: ATCMD_FWUPINIT rxok\r\n", __func__);
                dsm_mdm_mic_fwup_set_fsm(FWU_FSM_UP);

                ret = true;
            }
        }

        break;
    case FWU_FSM_UP:
        if (cmd_id == ATCMD_FWUP)
        {
            uint8_t ok_pos;
            uint32_t offset;
            uint32_t addr;
            uint32_t addr_max;

            if (dsm_atcmd_is_valid_fwup_rsp(pdata, &ok_pos))
            {
                DPRINTF(DBG_TRACE,
                        "%s: ATCMD_FWUP rx ok: snd_cnt[%d], retry_cnt[%d]\r\n",
                        __func__, pmdm_mic_fwdl->snd_cnt,
                        pmdm_mic_fwdl->retry_cnt);
#if 0
                        pmdm_mic_fwdl->snd_cnt++;
#else
#endif
                pmdm_mic_fwdl->retry_cnt = 0;
                g_atcmd_modem_fwup_retry_cnt = 0;
                ret = true;
            }
            else
            {
                MSGERROR("not valid_fwup_rsp");
                return ret;
            }

            offset = pmdm_mic_fwdl->blk_size * pmdm_mic_fwdl->snd_cnt;
            addr = pmdm_mic_fwdl->start_dl_addr + offset;
            addr_max = pmdm_mic_fwdl->start_dl_addr + pmdm_mic_fwdl->image_size;

            if (addr >= addr_max)
            {
                dsm_mdm_mic_fwup_set_fsm(FWU_FSM_UP_OP);
            }
        }

        break;
    case FWU_FSM_UP_OP:
        if (cmd_id == ATCMD_FWUPOP)
        {
            if (pdata[0] == '0' && pdata[2] == 'O' && pdata[3] == 'K')
            {
                pmdm_mic_fwdl->retry_cnt = 0;
                dsm_mdm_mic_fwup_set_fsm(FWU_FSM_SUCCESS);
                DPRINTF(DBG_TRACE, "%s: FWU_FSM_UP_OP :ATCMD_FWUPOP rxok\r\n",
                        __func__);

                ret = true;
            }
        }

        break;
    default:
        break;
    }

    return ret;
}

int dsm_atcmd_get_param(uint8_t* instr, uint8_t* outstr, char delimiter)
{
    int len = 0;

    while (*instr != '\0' && *instr != delimiter && *instr != '\r')
    {
        *outstr++ = *instr++;
        len++;
    }
    outstr = outstr - len;

    return len + 1;
}

uint32_t dsm_atcmd_en_de_crypt_rx_proc(uint8_t cmd_id, uint8_t* pdata,
                                       uint16_t data_len, uint8_t* o_pdata,
                                       uint16_t* o_plen)
{
#if 0 /* bccho, 2023-08-17 */
    int32_t offset = 0;
    uint32_t value = 0;
    int8_t outstr[32];
    uint32_t en_de_crypt_len = 0;
    uint16_t crc_val = 0, crc_calc = 0;
    uint32_t ret = AT_DATA_RX_OK;

    DPRINTF(DBG_TRACE, "%s: %d\r\n", __func__, cmd_id);
    DPRINT_HEX(DBG_TRACE, "I_DATA", pdata, data_len, DUMP_ALWAYS);

    if (cmd_id == ATCMD_ENCRYPT || cmd_id == ATCMD_DECRYPT)
    {
        memset(outstr, 0, sizeof(outstr));
        offset = dsm_atcmd_get_param(pdata, (uint8_t*)outstr, ',');
        en_de_crypt_len = atoi((char*)outstr);
        DPRINTF(DBG_TRACE, "offset[%d], value[0x%08X]\r\n", offset,
                en_de_crypt_len);
        pdata += offset;

        memset(outstr, 0, sizeof(outstr));
        offset = dsm_atcmd_get_param(pdata, (uint8_t*)outstr, ',');
        str_to_hex((char*)outstr, &value);
        DPRINTF(DBG_TRACE, "offset[%d], value[0x%08X]\r\n", offset, value);
        crc_val = (uint16_t)value;
        pdata += offset;

        DPRINT_HEX(DBG_TRACE, "EN_DE_DATA", pdata, en_de_crypt_len,
                   DUMP_ALWAYS);

        crc_calc = crc16_get(pdata, en_de_crypt_len);

        DPRINTF(
            DBG_TRACE,
            _D "%s: en_de_crypt_len = %d,  crc_val 0x%04X, crc_calc 0x%04X\r\n",
            __func__, en_de_crypt_len, crc_val, crc_calc);

        if (crc_val == crc_calc)
        {
            if (cmd_id == ATCMD_DECRYPT)
            {
                if (dsm_sec_legacy_decrypt(pdata, en_de_crypt_len, o_pdata,
                                           o_plen))
                {
                    dsm_sec_invocation_count_add();
                }
                else
                {
                    ret = AT_DATA_EN_DE_CRYPT_ERR;
                }
            }
            else
            {
                if (dsm_sec_legacy_encrypt(o_pdata, o_plen, pdata,
                                           en_de_crypt_len))
                    ;
                else
                {
                    ret = AT_DATA_EN_DE_CRYPT_ERR;
                }
            }
        }
        else
        {
            ret = AT_DATA_CRC_ERR;
        }
    }

    return ret;
#else /* bccho */
    return AT_DATA_RX_OK;
#endif
}

uint32_t dsm_mdm_id_is_valid(uint8_t* pid, uint8_t len)
{
    uint8_t cnt = 0;
    uint32_t ret = TRUE;

    if (len == MODEM_ID_SIZE)
    {
        for (cnt = 0; cnt < MODEM_ID_SIZE; cnt++)
        {
            if ((pid[cnt] < '0') || (pid[cnt] > 'z'))
            {
                return FALSE;
            }
        }
    }
    else
        return FALSE;

    return ret;
}

void dsm_atcmd_zcd_time_rx_proc(uint8_t* pdata, uint8_t size)
{
    uint32_t dec_us = 0;
    uint32_t val[50];
    U8 cnt = 0, k = 0, val_cnt = 0;
    ST_ZCD_RESULT_TIME st_zcd;

    memset((U8*)val, 0x00, (50 * 4));
    val_cnt = str_to_bin32_n((char*)pdata, val, (uint32_t)size);
    // DPRINT_HEX(DBG_TRACE, "ATCMD_ZCDT_DATA", (U8 *)val, val_cnt*4,
    // DUMP_ALWAYS);
    for (k = 0; k < val_cnt; k += 4)
    {
        DPRINTF(DBG_WARN,
                _D " 1 ATCMD_ZCDT_DATA: val[k]=[%d] [%d] [%d] [%d] \r\n",
                val[k], val[k + 1], val[k + 2], val[k + 3]);
    }

    if (!val_cnt)  // all 0xffff
    {
        st_zcd.time = 0xffffffff;

        nv_write(I_ZCD_RESULT_TIME, (uint8_t*)&st_zcd);
        DPRINTF(DBG_ERR, _D "ZCD computation Fail: val_cnt[%d]\r\n", val_cnt);
    }
    else
    {
        dec_us = 0;
        cnt = val_cnt;
        while (cnt)
        {
            cnt--;
            dec_us += val[cnt];
            DPRINTF(DBG_WARN,
                    _D
                    " 1 dec_us result: dec_us[%d]  val_cnt[%d] val[cnt][%d] "
                    "cnt[%d]\r\n",
                    dec_us, val_cnt, val[cnt], cnt);
        };

        DPRINTF(DBG_WARN, _D " 2 dec_us result: dec_us[%d]  val_cnt[%d]\r\n",
                dec_us, val_cnt);
        dec_us /= val_cnt;

        for (k = 0; k < val_cnt; k++)
        {
            if ((val[k] < dec_us / 2) || (val[k] > dec_us * 1.5))
                val[k] = dec_us;
            // DPRINTF(DBG_WARN, _D"2 ATCMD_ZCDT_DATA: val[k]=[%d] [%d] [%d]
            // [%d] \r\n", val[k]  ,val[k+1]  ,val[k+2],  val[k+3]);
        }
        for (k = 0; k < val_cnt; k += 4)
        {
            DPRINTF(DBG_WARN,
                    _D " 2 ATCMD_ZCDT_DATA: val[k]=[%d] [%d] [%d] [%d] \r\n",
                    val[k], val[k + 1], val[k + 2], val[k + 3]);
        }

        dec_us = 0;
        cnt = val_cnt;
        while (cnt)
        {
            cnt--;
            dec_us += val[cnt];
        };

        DPRINTF(DBG_WARN, _D " dec_us result: dec_us[%d]  val_cnt[%d]\r\n",
                dec_us, val_cnt);
        dec_us /= val_cnt;

        ST_ZCD_RESULT_TIME st_zcd;

        // memcpy(hex, pdata, size);
        // dec_us = atoi((char*)pdata);

#if 0  // jp.kim 24.11.12
		st_zcd.time = dec_us - zcrs_sig_out_cmps*1000;
		DPRINTF(DBG_WARN, _D"ZCD result: %d[zcd_us]  = %d[cal] - %d[cmps]\r\n", st_zcd.time, dec_us, zcrs_sig_out_cmps*1000);
#else
#if 1  // 상수값 사용  //jp.kim 24.11.14
        if (dec_us > DEFAULT_zcrs_sig_out_cmps)
            st_zcd.time = dec_us - DEFAULT_zcrs_sig_out_cmps;
        else
            st_zcd.time = 0;
#else
        st_zcd.time = dec_us - zcrs_sig_out_cmps;
        DPRINTF(DBG_WARN,
                _D "ZCD result: %d[zcd_us]  = %d[cal] - %d[cmps]  size[%d]\r\n",
                st_zcd.time, dec_us, zcrs_sig_out_cmps, size);
#endif
#endif

        nv_write(I_ZCD_RESULT_TIME, (uint8_t*)&st_zcd);
    }
}

void dsm_atcmd_baud_rx_proc(uint8_t* pdata, uint8_t size)
{
    uint32_t baud_rate;
    uint8_t hex[50];

    memset(hex, 0x00, 50);
    str_to_hex_n((char*)pdata, hex, (uint32_t)size);

    memcpy(hex, pdata, size);
    baud_rate = atoi((char*)pdata);

    DPRINTF(DBG_TRACE, "%s: baud_rate[%d]\r\n", __func__, baud_rate);
    if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_EXT)
    {
        EN_AT_BAUD at_baud = dsm_modemif_baudvalue_2_atbaud(baud_rate);

        gst_mdm_baud.emodem = at_baud;
        dsm_modemif_baud_nvwrite(&gst_mdm_baud);
        dsm_emodemif_init();
    }
    else if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_SUN)
    {
        EN_AT_BAUD at_baud = dsm_modemif_baudvalue_2_atbaud(baud_rate);

        gst_mdm_baud.imodem = at_baud;
        dsm_modemif_baud_nvwrite(&gst_mdm_baud);
        dsm_imodemif_init(FALSE);
    }
}

uint32_t dsm_atcmd_rx_proc(ST_AT_CMD_RX_PKT* p_com_pkt)
{
    uint8_t cmd_len, data_pos = 0;
    uint8_t cmd_idx = 0;
    ATCMD_CNTX* cmd_cntx;
    uint8_t* cmd_ptr = &p_com_pkt->pkt[AT_CMD_POS];
    uint8_t* ptr;
    uint32_t ret = AT_DATA_RX_OK;
    ST_FW_INFO fwinfo;

    if (p_com_pkt->cmd_type == AT_RSP_DELIMETER)
    {
        cmd_cntx = (ATCMD_CNTX*)atcmd_mt_list;
        //        DPRINTF(DBG_TRACE, "AT_RSP\r\n");

        while (*cmd_cntx->cmdStr != '\0')
        {
            cmd_len = strlen(cmd_cntx->cmdStr);

            if (cmd_len == p_com_pkt->cmd_cnt)
            {
                if (strncmp((char*)cmd_ptr, cmd_cntx->cmdStr, cmd_len) == 0)
                {
                    data_pos =
                        AT_CMD_POS + AT_DELIMETER_SIZE + p_com_pkt->cmd_cnt;
                    ptr = &p_com_pkt->pkt[data_pos];
                    DPRINT_HEX(DBG_TRACE, "AT_CMD", cmd_ptr, p_com_pkt->cmd_cnt,
                               DUMP_MDM | DUMP_EMDM);
                    DPRINT_HEX(DBG_TRACE, "AT_RXD", ptr, p_com_pkt->data_cnt,
                               DUMP_MDM | DUMP_EMDM);
                    switch (cmd_idx)
                    {
                    case ATCMD_FWUPSTA:

                        break;
                    case ATCMD_FWUP:
                    case ATCMD_FWUPOP:
                    case ATCMD_FWUPINIT:
                        // dsm_atcmd_fwup_rx_proc(cmd_idx, ptr,
                        //                        p_com_pkt->data_cnt);
                        // break;
                    case ATCMD_ENCRYPT:
                    case ATCMD_DECRYPT:

                        break;
                    case ATCMD_METERID:
                        DPRINT_HEX(DBG_TRACE, "METERID", ptr,
                                   p_com_pkt->data_cnt, DUMP_ALWAYS);
                        break;

                    case ATCMD_MFDATE:
                        DPRINT_HEX(DBG_TRACE, "MFDATE", ptr,
                                   p_com_pkt->data_cnt, DUMP_ALWAYS);

                        break;
                    case ATCMD_BAUD:
                        DPRINT_HEX(DBG_TRACE, "ATCMD_BAUD", ptr,
                                   p_com_pkt->data_cnt, DUMP_ALWAYS);
                        dsm_atcmd_baud_rx_proc(ptr, p_com_pkt->data_cnt);
                        break;
                    case ATCMD_SHA256:
                        if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_SUN)
                        {
                            uint8_t i, result = 0;

                            memset((uint8_t*)&fwinfo, 0x00, sizeof(ST_FW_INFO));
                            dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo,
                                                    FWINFO_CUR_MODEM);

                            for (i = 0; i < p_com_pkt->data_cnt; i++)
                            {
                                uint8_t hexa, ascii;

                                hexa = ((i % 2 == 0)
                                            ? ((fwinfo.hash[i / 2] & 0xF0) >> 4)
                                            : (fwinfo.hash[i / 2] & 0x0F));
                                ascii = ((hexa > 9) ? (hexa - 10 + 'A')
                                                    : (hexa + '0'));

                                if (ascii != ptr[i])
                                {
                                    result = 1;
                                    break;
                                }
                            }

                            if (result)
                            {
                                uint8_t get_hash[IMAGE_HASH_SIZE] = {0};
                                uint8_t byte_data;
                                for (i = 0; i < p_com_pkt->data_cnt; i++)
                                {
                                    byte_data =
                                        ((ptr[i] > '9') ? (ptr[i] - 'A' + 10)
                                                        : (ptr[i] - '0'));
                                    byte_data =
                                        (byte_data << 4) |
                                        ((ptr[++i] > '9') ? (ptr[i] - 'A' + 10)
                                                          : (ptr[i] - '0'));
                                    get_hash[i / 2] = byte_data;
                                }

                                memcpy(fwinfo.hash, get_hash, IMAGE_HASH_SIZE);
                                dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo,
                                                         FWINFO_CUR_MODEM);
                            }
                        }
                        else if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_EXT)
                        {
                            uint8_t i, result = 0;

                            memset((uint8_t*)&fwinfo, 0x00, sizeof(ST_FW_INFO));
                            dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo,
                                                    FWINFO_CUR_E_MODEM);

                            for (i = 0; i < p_com_pkt->data_cnt; i++)
                            {
                                uint8_t hexa, ascii;

                                hexa = ((i % 2 == 0)
                                            ? ((fwinfo.hash[i / 2] & 0xF0) >> 4)
                                            : (fwinfo.hash[i / 2] & 0x0F));
                                ascii = ((hexa > 9) ? (hexa - 10 + 'A')
                                                    : (hexa + '0'));

                                if (ascii != ptr[i])
                                {
                                    result = 1;
                                    break;
                                }
                            }

                            if (result)
                            {
                                uint8_t get_hash[IMAGE_HASH_SIZE] = {0};
                                uint8_t byte_data = 0;
                                for (i = 0; i < p_com_pkt->data_cnt; i++)
                                {
                                    byte_data =
                                        ((ptr[i] > '9') ? (ptr[i] - 'A' + 10)
                                                        : (ptr[i] - '0'));
                                    byte_data =
                                        (byte_data << 4) |
                                        ((ptr[++i] > '9') ? (ptr[i] - 'A' + 10)
                                                          : (ptr[i] - '0'));
                                    get_hash[i / 2] = byte_data;

                                    DPRINTF(DBG_TRACE, "hex : %02X\r\n",
                                            byte_data);
                                }

                                DPRINT_HEX(DBG_TRACE, "NV_HASH", fwinfo.hash,
                                           IMAGE_HASH_SIZE, DUMP_ALWAYS);
                                memcpy(fwinfo.hash, get_hash, IMAGE_HASH_SIZE);
                                dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo,
                                                         FWINFO_CUR_E_MODEM);
                            }
                        }

                        break;
                    case ATCMD_BEARER:  // "AT+BEARER=1\r"
                        /* 구매 규격 "2.2.5.2 유/무선 통신 사양" : ※ 계기는
                         * "AT+BEARER" 수신 시 r-cALL 동작 가이드라인의 5항.
                         * 8항. 항목 수행 */
                        DPRINTF(DBG_INFO, "ATCMD TRAP = %s\r\n",
                                dsm_media_if_fsm_string(
                                    dsm_media_get_fsm_if_ATCMD()));

                        if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_SUN)
                        {
                            MSG07("Rx SUN ATCMD_BEARER");
                            DPRINT_HEX(DBG_TRACE, "SUN_BEARER", ptr,
                                       p_com_pkt->data_cnt, DUMP_ALWAYS);

                            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);

                            // 5-1. 계기 ID 주입
                            dsm_atcmd_set_meterid(FALSE);
                            vTaskDelay(100);

#if 0 /* JPKIM, 2024-10-08 */
                            // 5-2. 계기 제조일자 주입
                            dsm_atcmd_set_mfdate(FALSE);
                            vTaskDelay(100);
#endif
#if 0 /* bccho, 2024-09-30, 삭제 */
                            // 5-3. MAC 주소 주입 (AT+MAC:ADDRESS)
                            dsm_atcmd_set_macaddress(FALSE);
                            vTaskDelay(100);
#endif
                            dsm_atcmd_get_mode(FALSE, FALSE);
                            vTaskDelay(100);

                            dsm_atcmd_get_modem_id(FALSE);
                            vTaskDelay(100);

                            dsm_atcmd_get_fwver(FALSE, FALSE);
                            vTaskDelay(100);

                            dsm_atcmd_get_hash(FALSE, FALSE);
                            vTaskDelay(100);

#if 0 /* bccho, 2024-09-30, 삭제 */
                            dsm_atcmd_set_mode(FALSE, 1); /* 라우터 동작 */
                            vTaskDelay(100);

                            dsm_atcmd_set_listen(FALSE, AT_LISTEN_OFF);
                            vTaskDelay(100);
#endif
                            /* bccho, 2024-09-05, 삼상 */
                            dsm_atcmd_get_mdlink(FALSE, FALSE);
                            vTaskDelay(100);
                        }
                        else if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_EXT)
                        {
                            MSG07("Rx EXT ATCMD_BEARER");
                            DPRINT_HEX(DBG_TRACE, "EXT_BEARER", ptr,
                                       p_com_pkt->data_cnt, DUMP_ALWAYS);

#if 1  //"EXT_BEARER" 후 ac 복전
                            if (!is_dc33_off())
                            {
                                if (!power_notify_tx_set)
                                {
                                    dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
                                    dsm_atcmd_set_trap_power_notify(
                                        FALSE,
                                        '1');  // ac 복전
                                    power_notify_tx_set = 1;
                                }
                            }
#endif
                            // 8-1. 계기 ID 주입
                            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
                            dsm_atcmd_set_meterid(FALSE);
                            vTaskDelay(100);

#if 1  // jp.kim 25.12.03  외부모뎀 기기 정보 읽기 추가
                            dsm_atcmd_get_modem_id(FALSE);
                            vTaskDelay(100);
#endif

#if 1  // jp.kim 25.01.17
                            dsm_atcmd_get_fwver(FALSE, FALSE);
                            vTaskDelay(100);
#endif

#if 1  // jp.kim 25.03.30
                            dsm_atcmd_get_hash(FALSE, FALSE);
                            vTaskDelay(100);
#endif

#if 0  // jp.kim 24.12.30

                            // 8-2. 계기 내장 SUN의 모드전환 (AT+MODE:0)
                            // (*8-1이후에 5. 계기 내장 SUN Bearer 수신이 되는
                            // 경우, 계기는 5-1 ~ 5-3을 수행 후에 8-2를 수행해야
                            // 함.)
                            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
#endif

#if 0 /* bccho, 2024-09-30, 삭제 */
                            dsm_atcmd_set_mode(FALSE, 0); /* 코디네이터 동작 */
                            vTaskDelay(100);
                            // 8-3. 계기 내장 SUN을 리슨모드로 전환(AT+LISTEN:1
                            // 수신 모드)
                            dsm_atcmd_set_listen(FALSE, AT_LISTEN_ON);
#endif

                            dsm_set_bearer(*ptr);
                        }

                        break;
                    case ATCMD_MAC:
                        DPRINT_HEX(DBG_TRACE, "MAC", ptr, p_com_pkt->data_cnt,
                                   DUMP_ALWAYS);
                        break;
                    case ATCMD_PPDU:
                        break;
                    case ATCMD_MMID:
                        if (dsm_mdm_id_is_valid(ptr, p_com_pkt->data_cnt))
                        {
                            if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_SUN)
                            {
                                memcpy(&gst_int_modem_id.id[0], ptr,
                                       p_com_pkt->data_cnt);
                                DPRINT_HEX(DBG_TRACE, "INT_MDM_ID", ptr,
                                           p_com_pkt->data_cnt, DUMP_ALWAYS);
                            }
                            else if (dsm_media_get_fsm_if_ATCMD() ==
                                     MEDIA_RUN_EXT)
                            {
                                ST_MDM_ID st_modem_id;

#if 1  // jp.kim 25.12.03  외부모뎀 기기 정보 읽기 추가
                                memcpy((uint8_t*)&st_modem_id, ptr,
                                       p_com_pkt->data_cnt);
                                DPRINT_HEX(DBG_TRACE, "EXT_MDM_ID", ptr,
                                           p_com_pkt->data_cnt, DUMP_ALWAYS);

                                nv_write(I_EXT_MODEM_ID,
                                         (uint8_t*)&st_modem_id);
                                nv_read(I_EXT_MODEM_ID, (uint8_t*)&st_modem_id);
                                DPRINT_HEX(DBG_TRACE, "EXT_MDM_ID_NV",
                                           &st_modem_id.id[0],
                                           p_com_pkt->data_cnt, DUMP_ALWAYS);
#else
                                nv_read(I_EXT_MODEM_ID, (uint8_t*)&st_modem_id);

                                if (memcmp(&st_modem_id.id[0], ptr,
                                           p_com_pkt->data_cnt))
                                {
                                    memcpy(&st_modem_id.id[0], ptr,
                                           p_com_pkt->data_cnt);
                                    nv_write(I_EXT_MODEM_ID,
                                             (uint8_t*)&st_modem_id);
                                }
#endif
                            }
                        }

                        break;
                    case ATCMD_HWVER:
                        DPRINT_HEX(DBG_TRACE, "HWVER", ptr, p_com_pkt->data_cnt,
                                   DUMP_ALWAYS);
                        break;
                    case ATCMD_FWVER:
                    {
                        uint8_t version[IMAGE_FW_NAME_MAX_SIZE];

                        if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_SUN)
                        {
                            DPRINT_HEX(DBG_TRACE, "SUN_FWVER", ptr,
                                       p_com_pkt->data_cnt, DUMP_ALWAYS);
                            memset((uint8_t*)&fwinfo, 0x00, sizeof(ST_FW_INFO));
                            dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo,
                                                    FWINFO_CUR_MODEM);
                            memcpy(&version[0], &ptr[0], 14);

                            DPRINT_HEX(DBG_TRACE, "VERSION", &version[0],
                                       IMAGE_FW_NAME_MAX_SIZE, DUMP_ALWAYS);

                            if (memcmp(fwinfo.mt_type, version,
                                       IMAGE_FW_NAME_MAX_SIZE))
                            {
                                DPRINTF(DBG_ERR, "F/W Ver Info Updated\r\n");
                                memcpy(fwinfo.mt_type, version,
                                       IMAGE_FW_NAME_MAX_SIZE);
                                dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo,
                                                         FWINFO_CUR_MODEM);
                            }
                            else
                            {
                                DPRINTF(DBG_INFO, "F/W Ver Info Same\r\n");
                            }
                        }
                        else if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_EXT)
                        {
#if 0
                            g_modem_exist = true;  // jp.kim 25.01.20
#endif

                            DPRINT_HEX(DBG_TRACE, "EXT_MODEM FWVER", ptr,
                                       p_com_pkt->data_cnt, DUMP_ALWAYS);

                            memset((uint8_t*)&fwinfo, 0x00, sizeof(ST_FW_INFO));
                            dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo,
                                                    FWINFO_CUR_E_MODEM);

                            memcpy(&version[0], &ptr[0], 2);
                            memcpy(&version[2], &ptr[2], 12);

                            DPRINT_HEX(DBG_TRACE, "VERSION", &version[0],
                                       IMAGE_FW_NAME_MAX_SIZE, DUMP_ALWAYS);

                            if (memcmp(fwinfo.mt_type, version,
                                       IMAGE_FW_NAME_MAX_SIZE))
                            {
                                DPRINTF(DBG_ERR, "F/W Ver Info Updated\r\n");

                                memcpy(fwinfo.mt_type, version,
                                       IMAGE_FW_NAME_MAX_SIZE);

                                dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo,
                                                         FWINFO_CUR_E_MODEM);
                            }
                            else
                            {
                                DPRINTF(DBG_INFO, "F/W Ver Info Same\r\n");
                            }
                        }
                    }
                    break;
                    case ATCMD_FUN:
                        break;
                    case ATCMD_TXPWR:
                        break;
                    case ATCMD_NWKDR:
                        break;
                    case ATCMD_PMAC:
                        break;
                    case ATCMD_PANID:
                        break;
                    case ATCMD_CHAN:
                        break;
                    case ATCMD_ASSOPAN:
                        break;
                    case ATCMD_ASSOPER:
                        break;
                    case ATCMD_MAXBE:
                        break;
                    case ATCMD_MAXRTY:
                        break;
                    case ATCMD_RNS:
                        break;
                    case ATCMD_NWKKEY:
                        break;
                    case ATCMD_PANDLST:
                        break;
                    case ATCMD_NEWPAND:
                        break;
                    case ATCMD_EBR:
                        break;
                    case ATCMD_EBLIST:
                        break;
                    case ATCMD_MODE:
                        if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_SUN)
                        {
                            DPRINT_HEX(DBG_TRACE, "MODE", ptr,
                                       p_com_pkt->data_cnt, DUMP_ALWAYS);
                        }

                        break;

                    case ATCMD_485:
                        break;
                    case ATCMD_ZCD:
                        break;
                    case ATCMD_ZCDT:
                        dsm_atcmd_zcd_time_rx_proc(ptr, p_com_pkt->data_cnt);
                        break;
                    case ATCMD_VFREE:
                        break;
                    case ATCMD_PWRNOTIFY:
                        break;
                    case ATCMD_STDVER:
                        break;
                    case ATCMD_ZCDOFFSET:
                        break;
                    case ATCMD_ZCDEN:
                        break;

                        /* bccho, 2024-09-05, 삼상 */
                    case ATCMD_MDLINK:
                        MSG07("MDLINK--RSP");
                        DPRINTF(DBG_INFO, "ATCMD TRAP = %s\r\n",
                                dsm_media_if_fsm_string(
                                    dsm_media_get_fsm_if_ATCMD()));
                        if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_SUN)
                        {
                            DPRINT_HEX(DBG_TRACE, "ATCMD_MDLINK", ptr,
                                       p_com_pkt->data_cnt, DUMP_ALWAYS);
                            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);

                            g_mdlink_mode = ptr[0];
                            if (g_mdlink_mode == '1')
                            {
                                if (p_com_pkt->data_cnt > 16)
                                {
                                    g_mdlink_mode = SUN_DEVICE_ON_ASSO;
                                }
                                else
                                {
                                    g_mdlink_mode = SUN_DEVICE_OFF_ASSO;
                                }
                            }
                            else
                                g_mdlink_mode = SUN_CORDI;

                            DPRINTF(DBG_INFO, "g_mdlink_mode = [%d]\r\n",
                                    g_mdlink_mode);
                        }
                        break;
                    }
                    return TRUE;
                }
            }
            cmd_cntx++;
            cmd_idx++;
        }
    }
    else if (p_com_pkt->cmd_type == AT_GET_REQ_DELIMETER)
    {
        cmd_cntx = (ATCMD_CNTX*)atcmd_mt_list;
        //        DPRINTF(DBG_WARN, "AT_GET_REQ\r\n");

        while (*cmd_cntx->cmdStr != '\0')
        {
            cmd_len = strlen(cmd_cntx->cmdStr);

            if (cmd_len == p_com_pkt->cmd_cnt)
            {
                if (strncmp((char*)cmd_ptr, cmd_cntx->cmdStr, cmd_len) == 0)
                {
                    data_pos =
                        AT_CMD_POS + AT_DELIMETER_SIZE + p_com_pkt->cmd_cnt;
                    ptr = &p_com_pkt->pkt[data_pos];
                    DPRINT_HEX(DBG_TRACE, "AT_CMD", cmd_ptr, p_com_pkt->cmd_cnt,
                               DUMP_MDM | DUMP_EMDM);
                    DPRINT_HEX(DBG_TRACE, "AT_RXD", ptr, p_com_pkt->data_cnt,
                               DUMP_MDM | DUMP_EMDM);
                    switch (cmd_idx)
                    {
                    case ATCMD_FWUPSTA:

                        break;
                    case ATCMD_FWUP:
                    case ATCMD_FWUPOP:
                    case ATCMD_FWUPINIT:

                        break;
                    case ATCMD_ENCRYPT:
                    case ATCMD_DECRYPT:

                        break;
                    case ATCMD_METERID:
                        if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_EXT)
                        {
                            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
                            dsm_atcmd_set_meterid(TRUE);
                        }

                        break;
                    case ATCMD_MFDATE:
                        DPRINT_HEX(DBG_TRACE, "MFDATE", ptr,
                                   p_com_pkt->data_cnt, DUMP_ALWAYS);

                        break;
                    case ATCMD_BAUD:

                        break;
                    case ATCMD_SHA256:

                        break;
                    case ATCMD_BEARER:
                        break;
                    case ATCMD_MAC:

                        break;
                    case ATCMD_PPDU:
                        break;
                    case ATCMD_MMID:

                        break;
                    case ATCMD_HWVER:

                        break;
                    case ATCMD_FWVER:

                        break;
                    case ATCMD_FUN:
                        break;
                    case ATCMD_TXPWR:
                        break;
                    case ATCMD_NWKDR:
                        break;
                    case ATCMD_PMAC:
                        break;
                    case ATCMD_PANID:
                        break;
                    case ATCMD_CHAN:
                        break;
                    case ATCMD_ASSOPAN:
                        break;
                    case ATCMD_ASSOPER:
                        break;
                    case ATCMD_MAXBE:
                        break;
                    case ATCMD_MAXRTY:
                        break;
                    case ATCMD_RNS:
                        break;
                    case ATCMD_NWKKEY:
                        break;
                    case ATCMD_PANDLST:
                        break;
                    case ATCMD_NEWPAND:
                        break;
                    case ATCMD_EBR:
                        break;
                    case ATCMD_EBLIST:
                        break;
                    case ATCMD_MODE:
                        break;
                    case ATCMD_485:
                        break;
                    case ATCMD_ZCD:
                        break;
                    case ATCMD_ZCDT:
                        break;
                    case ATCMD_VFREE:
                        break;

                    case ATCMD_PWRNOTIFY:
                        // 정복전시 출력 전용
                        break;
                    case ATCMD_STDVER:
                        //? 응답 구현 AT+STDVER=32,0000\rOK\r
                        ptr[0] = COSEM_METER_ID_VER[1];
                        ptr[1] = COSEM_METER_ID_VER[2];
                        ptr[2] = ',';
                        ptr[3] = '0';
                        ptr[4] = '0';
                        ptr[5] = '0';
#ifdef MTP_ZCD_ON_OFF
                        ptr[6] = '0';
#else
                        ptr[6] = '1';
#endif
                        dsm_atcmd_get_rsp_stdver(FALSE, 0, ptr, 7);
                        break;
                    case ATCMD_ZCDOFFSET:
                        //? 응답 구현AT+ZCDOFFSET=offset_usec\rOK\r
                        // AT+ZCDOFFSET="DEFAULT_zcrs_sig_out_cmps"\rOK\r  1000
                        // uint32_t t32 = DEFAULT_zcrs_sig_out_cmps;
                        ptr[0] = zcrs_sig[0] + '0';
                        ptr[1] = zcrs_sig[1] + '0';
                        ptr[2] = zcrs_sig[2] + '0';
                        ptr[3] = zcrs_sig[3] + '0';
                        dsm_atcmd_get_rsp_zcdoffset(FALSE, 0, ptr, 4);
                        break;
                    case ATCMD_ZCDEN:
                        //? 응답 구현AT+ZCDEN=enb\rOK\r
                        // AT+ZCDEN=1\rOK\r
#ifdef MTP_ZCD_ON_OFF
                        if (g_zcd_on_ing_sts)
                            ptr[0] = '1';  //  "on" 으로 응답 '1': ZCD pulse on
                        else
                            ptr[0] = '0';  //  "off" 로 응답 '0': ZCD pulse off
#else
                        ptr[0] = '1';  //  "on" 으로 응답 '1': ZCD pulse on
#endif

                        dsm_atcmd_get_rsp_zcden(FALSE, 0, ptr, 1);
                        break;

                        /* bccho, 2024-09-05, 삼상 */
                    case ATCMD_MDLINK:
#if 0  // 해당 안됨. jp.kim 24.11.22                    
                        MSG07("MDLINK--REQ");

                        if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_SUN)
                        {
                            DPRINT_HEX(DBG_TRACE,
                                       "AT_GET_REQ_DELIMETER  ATCMD_MDLINK",
                                       ptr, p_com_pkt->data_cnt, DUMP_ALWAYS);
                            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);

                            g_mdlink_mode = ptr[0];
                            if (g_mdlink_mode == '1')
                            {
                                if (p_com_pkt->data_cnt > 16)
                                {
                                    g_mdlink_mode = SUN_DEVICE_ON_ASSO;
                                }
                                else
                                {
                                    g_mdlink_mode = SUN_DEVICE_OFF_ASSO;
                                }
                            }
                            else
                                g_mdlink_mode = SUN_CORDI;

                            DPRINTF(DBG_INFO, "g_mdlink_mode = [%d]\r\n",
                                    g_mdlink_mode);
                        }
#endif
                        break;
                    }
                    return TRUE;
                }
            }
            cmd_cntx++;
            cmd_idx++;
        }
    }
    else if (p_com_pkt->cmd_type == AT_SET_REQ_DELIMETER)
    {
        cmd_cntx = (ATCMD_CNTX*)atcmd_mt_list;
        //        DPRINTF(DBG_WARN, "AT_SET_REQ\r\n");

        while (*cmd_cntx->cmdStr != '\0')
        {
            cmd_len = strlen(cmd_cntx->cmdStr);
            if (cmd_len == p_com_pkt->cmd_cnt)
            {
                if (strncmp((char*)cmd_ptr, cmd_cntx->cmdStr, cmd_len) == 0)
                {
                    data_pos =
                        AT_CMD_POS + AT_DELIMETER_SIZE + p_com_pkt->cmd_cnt;
                    ptr = &p_com_pkt->pkt[data_pos];
                    DPRINT_HEX(DBG_TRACE, "AT_CMD", cmd_ptr, p_com_pkt->cmd_cnt,
                               DUMP_MDM | DUMP_EMDM);
                    DPRINT_HEX(DBG_TRACE, "AT_RXD", ptr, p_com_pkt->data_cnt,
                               DUMP_MDM | DUMP_EMDM);

                    switch (cmd_idx)
                    {
                    case ATCMD_ENCRYPT:
                    case ATCMD_DECRYPT:
#if 0 /* bccho, 2023-08-17 */
                        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
                        ret = dsm_atcmd_en_de_crypt_rx_proc(
                            cmd_idx, ptr, p_com_pkt->data_cnt, p_com_pkt->pkt,
                            &p_com_pkt->data_cnt);
                        dsm_atcmd_set_rsp_en_de_crypt(FALSE, cmd_cntx, ret,
                                                      p_com_pkt->pkt,
                                                      p_com_pkt->data_cnt);
#endif
                        break;
                    case ATCMD_BEARER:
                        if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_EXT)
                        {
                            DPRINT_HEX(DBG_TRACE, "EXT_BEARER", ptr,
                                       p_com_pkt->data_cnt, DUMP_ALWAYS);
                            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
                            dsm_atcmd_set_rsp_bearer(FALSE, 0, ptr,
                                                     p_com_pkt->data_cnt);
                            OSTimeDly(OS_MS2TICK(150));
                            dsm_atcmd_set_meterid(FALSE);
                            dsm_set_bearer(*ptr);
                        }
                        else if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_SUN)
                        {
                            DPRINT_HEX(DBG_TRACE, "SUN_BEARER", ptr,
                                       p_com_pkt->data_cnt, DUMP_ALWAYS);
                            dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
                            dsm_atcmd_set_rsp_bearer(FALSE, 0, ptr,
                                                     p_com_pkt->data_cnt);
                            OSTimeDly(OS_MS2TICK(150));
                        }
                        break;

                    case ATCMD_ZCDEN:
                        // 설정AT+ZCDEN:enb\r
                        //: 응답 구현AT+ZCDEN=enb\rOK\r
                        // AT+ZCDEN=1\rOK\r

                        if (dsm_media_get_fsm_if_ATCMD() == MEDIA_RUN_EXT)
                        {
#ifdef MTP_ZCD_ON_OFF
                            if (ptr[0] == '1')
                            {
                                dsm_mif_zcd_on_meter_only();
                                // "on" 으로 응답 '1': ZCD pulse on
                                ptr[0] = '1';
                            }
                            else if (ptr[0] == '0')
                            {
                                dsm_mif_zcd_off_meter_only();
                                // "off" 로 응답 '0': ZCD pulse off
                                ptr[0] = '0';
                            }
                            else
                            {
                                if (g_zcd_on_ing_sts)
                                    ptr[0] = '1';  // "on" 으로 응답 '1': ZCD
                                                   // pulse on
                                else
                                    ptr[0] = '0';  // "off" 로 응답 '0': ZCD
                                                   // pulse off
                            }
#else
                            ptr[0] = '1';  //  "on" 으로 응답 '1': ZCD pulse on
#endif

                            dsm_atcmd_get_rsp_zcden(FALSE, 0, ptr, 1);
                        }
                        break;
                    }
                    return TRUE;
                }
            }
            cmd_cntx++;
            cmd_idx++;
        } /* while */
    }

    return FALSE;
}

bool dsm_atcmd_poll_rx_proc(ST_AT_CMD_RX_PKT* p_com_pkt)
{
    uint8_t cmd_len, data_pos = 0;
    uint8_t cmd_idx = 0;
    ATCMD_CNTX* cmd_cntx;
    uint8_t* cmd_ptr = &p_com_pkt->pkt[AT_CMD_POS];
    uint8_t* ptr;

    if (p_com_pkt->cmd_type == AT_RSP_DELIMETER)
    {
        cmd_cntx = (ATCMD_CNTX*)atcmd_mt_list;
        //        DPRINTF(DBG_TRACE, "AT_RSP\r\n");

        while (*cmd_cntx->cmdStr != '\0')
        {
            cmd_len = strlen(cmd_cntx->cmdStr);

            if (cmd_len == p_com_pkt->cmd_cnt)
            {
                if (strncmp((char*)cmd_ptr, cmd_cntx->cmdStr, cmd_len) == 0)
                {
                    data_pos =
                        AT_CMD_POS + AT_DELIMETER_SIZE + p_com_pkt->cmd_cnt;
                    ptr = &p_com_pkt->pkt[data_pos];
                    DPRINT_HEX(DBG_TRACE, "AT_CMD", cmd_ptr, p_com_pkt->cmd_cnt,
                               DUMP_MDM | DUMP_EMDM);
                    DPRINT_HEX(DBG_TRACE, "AT_RXD", ptr, p_com_pkt->data_cnt,
                               DUMP_MDM | DUMP_EMDM);

                    switch (cmd_idx)
                    {
                    case ATCMD_FWUPSTA:
                        break;
                    case ATCMD_FWUP:
                    case ATCMD_FWUPOP:
                    case ATCMD_FWUPINIT:
                        if (dsm_atcmd_poll_fwup_rx_proc(cmd_idx, ptr,
                                                        p_com_pkt->data_cnt))
                        {
#if 1 /* bccho, 2023-11-17 */
                            vTaskDelay(10);
#endif
                            return true;
                        }
#if 1 /* bccho, 2023-11-17 */
                        else
                        {
                            MSGERROR("atcmd_poll_fwup_rx_proc-->exit");
                            goto exit;
                        }
#endif
                        break;
                    }
                }
            }
            cmd_cntx++;
            cmd_idx++;
        }
    }
#if 1 /* bccho, 2023-11-17 */
exit:
#endif
    return false;
}

void dsm_atcmd_tx_getreq(uint32_t poll_flag, uint8_t cmd)
{
    uint16_t cnt = 0;
    uint8_t tx_data[1024];
    const ATCMD_CNTX* cmd_cntx = &atcmd_mt_list[cmd];
    uint8_t cmd_len = strlen(cmd_cntx->cmdStr);

    tx_data[cnt++] = AT_FIRST_A;
    tx_data[cnt++] = AT_SECOND_T;
    tx_data[cnt++] = AT_plus;
    memcpy(&tx_data[cnt], cmd_cntx->cmdStr, cmd_len);
    cnt += cmd_len;
    tx_data[cnt++] = AT_GET_REQ_DELIMETER;
    tx_data[cnt++] = 0x0d;

    dsm_media_if_send(MEDIA_PROTOCOL_IF_ATCMD, poll_flag, tx_data, cnt);
}

void dsm_atcmd_tx_setreq(uint32_t poll_flag, uint8_t cmd, uint8_t* pdata,
                         uint16_t len)
{
    uint16_t cnt = 0;
    uint8_t tx_data[1024];
    const ATCMD_CNTX* cmd_cntx = &atcmd_mt_list[cmd];
    uint8_t cmd_len = strlen(cmd_cntx->cmdStr);

    tx_data[cnt++] = AT_FIRST_A;
    tx_data[cnt++] = AT_SECOND_T;
    tx_data[cnt++] = AT_plus;
    memcpy(&tx_data[cnt], cmd_cntx->cmdStr, cmd_len);
    cnt += cmd_len;
    tx_data[cnt++] = AT_SET_REQ_DELIMETER;
    memcpy(&tx_data[cnt], pdata, len);
    cnt += len;
    tx_data[cnt++] = 0x0d;

    dsm_media_if_send(MEDIA_PROTOCOL_IF_ATCMD, poll_flag, tx_data, cnt);
}

void dsm_atcmd_tx_setrsp(uint32_t poll_flag, uint8_t cmd, uint8_t* pdata,
                         uint16_t len, uint32_t err)
{
    uint16_t cnt = 0;
    uint8_t tx_data[1024];
    const ATCMD_CNTX* cmd_cntx = &atcmd_mt_list[cmd];
    uint8_t cmd_len = strlen(cmd_cntx->cmdStr);

    tx_data[cnt++] = AT_FIRST_A;
    tx_data[cnt++] = AT_SECOND_T;
    tx_data[cnt++] = AT_plus;
    memcpy(&tx_data[cnt], cmd_cntx->cmdStr, cmd_len);
    cnt += cmd_len;
    tx_data[cnt++] = AT_RSP_DELIMETER;
    memcpy(&tx_data[cnt], pdata, len);
    cnt += len;
    tx_data[cnt++] = 0x0d;

    if (err == AT_DATA_RX_OK)
    {
        tx_data[cnt++] = 'O';
        tx_data[cnt++] = 'K';
    }
    else
    {
        tx_data[cnt++] = 'E';
        tx_data[cnt++] = 'R';
        tx_data[cnt++] = 'R';
        tx_data[cnt++] = 'O';
        tx_data[cnt++] = 'R';
    }
    tx_data[cnt++] = 0x0d;

    DPRINT_HEX(DBG_TRACE, "AT_SET_RSP_TX", tx_data, cnt, DUMP_ALWAYS);
    dsm_media_if_send(MEDIA_PROTOCOL_IF_ATCMD, poll_flag, tx_data, cnt);
}

void dsm_atcmd_tx(uint32_t poll_flag, ST_AT_CMD_TX_PKT* tx_pkt)
{
    if (tx_pkt->cmd_type == AT_GET_REQ_DELIMETER)
    {
        dsm_atcmd_tx_getreq(poll_flag, tx_pkt->cmd_id);
    }
    else if (tx_pkt->cmd_type == AT_SET_REQ_DELIMETER)
    {
        dsm_atcmd_tx_setreq(poll_flag, tx_pkt->cmd_id, tx_pkt->data,
                            tx_pkt->data_cnt);
    }
    else if (tx_pkt->cmd_type == AT_RSP_DELIMETER)
    {
        dsm_atcmd_tx_setrsp(poll_flag, tx_pkt->cmd_id, tx_pkt->data,
                            tx_pkt->data_cnt, tx_pkt->rsp_err);
    }
    if (gst_atcmd_tx_pkt.rsp_need_after_tx)
    {
        dsm_meter_sw_timer_stop(MT_SW_TIMER_ATCMD_TX_TO);
        dsm_meter_sw_timer_start(MT_SW_TIMER_ATCMD_TX_TO, FALSE,
                                 MT_TIMEOUT_AT_CMD_TX_TIME);
    }
}

void dsm_atcmd_tx_retry_proc(void)
{
    uint8_t retry_max = ATCMD_RETRY_MAX_COUNT;

    dsm_meter_sw_timer_stop(MT_SW_TIMER_ATCMD_TX_TO);

    if (gst_atcmd_tx_pkt.cmd_id == ATCMD_MMID)
        retry_max = ATCMD_MMID_RETRY_MAX_COUNT;

    if (gst_atcmd_tx_pkt.retry_cnt++ < retry_max)
    {
        DPRINTF(DBG_TRACE, "%s: retry_cnt[%d]\r\n", __func__,
                gst_atcmd_tx_pkt.retry_cnt);

        dsm_atcmd_tx(FALSE, &gst_atcmd_tx_pkt);
    }
    else
    {
        DPRINTF(DBG_WARN, "%s: max retry error!!!\r\n", __func__);
        gst_atcmd_tx_pkt.retry_cnt = 0;

        if (dsm_mdm_mic_fwup_get_fsm() != FWU_FSM_NONE)
        {
            uint8_t comm_if = dsm_media_get_fsm_if_ATCMD();
            if ((comm_if == MEDIA_RUN_SUN) || (comm_if == MEDIA_RUN_EXT))
            {
                if (g_atcmd_modem_fwup_retry_cnt++ < FWUP_MAX_RETRY_COUNT)
                {
                    dsm_atcmd_tx(FALSE, &gst_atcmd_tx_pkt);
                }
                else
                {
                    if (comm_if == MEDIA_RUN_SUN)
                    {
                        dsm_modem_hw_reset(INT_MODEM_RESET);
                    }
                    else if (comm_if == MEDIA_RUN_EXT)
                    {
#if 0  // JP.KIM 24.10.08                        
                        dsm_modem_hw_reset(EXT_MODEM_RESET);
#endif
                    }
                    g_atcmd_modem_fwup_retry_cnt = 0;
                    dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);
                }
            }
            else
            {
                dsm_mdm_mic_fwup_set_fsm(FWU_FSM_NONE);
            }
        }
    }
}

void dsm_atcmd_set_trap_power_notify(uint32_t poll_flag, uint8_t data)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    gst_atcmd_tx_pkt.cmd_id = ATCMD_PWRNOTIFY;
    gst_atcmd_tx_pkt.cmd_type = AT_Equal;
    gst_atcmd_tx_pkt.data_cnt = 1;
    // gst_atcmd_tx_pkt.data[0] = sel;
    memcpy(gst_atcmd_tx_pkt.data, &data, 1);
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;
    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_set_meterid(uint32_t poll_flag)
{
    uint8_t manuf_id[MANUF_ID_SIZE /*+2*/];

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    get_manuf_id(manuf_id);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_METERID;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = METER_ID_SIZE;
    memcpy(gst_atcmd_tx_pkt.data, &manuf_id[0], METER_ID_SIZE);
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

#if 1 /* bccho, 2023-10-23, 제이엔디전자에서 추가 */
void dsm_atcmd_set_macaddress(uint32_t poll_flag)
{
    uint8_t manuf_id[MANUF_ID_SIZE /*+2*/];
    uint8_t mac[16];

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    get_manuf_id(manuf_id);

    /*  0 ~ 3  : "79"--> 0x3739
        4 ~ 6  : "FFF"
        7 ~ 17 : meterID 11 자리 */
    mac[0] = '3';
    mac[1] = '7';
    mac[2] = '3';
    mac[3] = '9';
    mac[4] = 'F';
    mac[5] = 'F';
    mac[6] = 'F';
    memcpy(&mac[7], &manuf_id[2], 9);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_MAC;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 16;
    memcpy(gst_atcmd_tx_pkt.data, &mac[0], 16);
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}
#endif

void dsm_atcmd_set_mfdate(uint32_t poll_flag)
{
    device_id_type dev;

    if (!nv_read(I_DEVICE_ID_KEPCO, (uint8_t*)&dev))
    {
        memcpy(dev.devid, &logical_device_name_r_kepco[0], DEVICE_ID_SIZE);
    }
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_MFDATE;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;

    gst_atcmd_tx_pkt.data_cnt = 4;
    memcpy(gst_atcmd_tx_pkt.data, &dev.devid[4], gst_atcmd_tx_pkt.data_cnt);

    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    DPRINT_HEX(DBG_TRACE, "MFDATE", dev.devid, DEVICE_ID_SIZE, DUMP_ALWAYS);

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_set_reset(uint32_t poll_flag, uint8_t reset_type)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_FUN;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 1;
    if (reset_type == AT_RST_NORMAL)  // reset
        gst_atcmd_tx_pkt.data[0] = '0';
    else  // factory reset
        gst_atcmd_tx_pkt.data[0] = '1';
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_baud(uint32_t poll_flag)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_BAUD;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_set_baud(uint32_t poll_flag, uint8_t baud_type)
{
    gst_atcmd_tx_pkt.cmd_id = ATCMD_BAUD;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 6;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    DPRINTF(DBG_TRACE, "%s, %s\r\n", __func__, dsm_baud_string(baud_type));

    switch (baud_type)
    {
    case AT_BAUD_115200:
        memcpy(gst_atcmd_tx_pkt.data, "115200", 6);

        break;
    case AT_BAUD_230400:
        memcpy(gst_atcmd_tx_pkt.data, "230400", 6);

        break;
    case AT_BAUD_460800:
        memcpy(gst_atcmd_tx_pkt.data, "460800", 6);

        break;
    }

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_modem_id(uint32_t poll_flag)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_MMID;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_fwup_set_init(uint32_t poll_flag)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    gst_atcmd_tx_pkt.cmd_id = ATCMD_FWUPINIT;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 1;
    gst_atcmd_tx_pkt.data[0] = '0';
    gst_atcmd_tx_pkt.retry_cnt = 0;
    if (poll_flag)
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;
    else
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_fwup_get_status(uint32_t poll_flag)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    gst_atcmd_tx_pkt.cmd_id = ATCMD_FWUPSTA;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

// JP.KIM 25.01.17
void ami_atcmd_get_bearer(uint32_t poll_flag)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    gst_atcmd_tx_pkt.cmd_id = ATCMD_BEARER;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_fwup_set_imgdata(uint32_t poll_flag, uint8_t* p_name,
                                uint32_t img_addr, uint16_t img_len,
                                uint8_t flag, uint32_t offset)
{
    uint16_t crc;
    int8_t ascill[8];
    int32_t size = 0;
    ST_AT_CMD_TX_PKT* txpkt = dsm_atcmd_get_txpkt();

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    txpkt->cmd_id = ATCMD_FWUP;
    txpkt->cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;
    txpkt->data_cnt = 0;

    memcpy(txpkt->data, p_name, 2);
    txpkt->data[2] = 'V';
    memcpy(&txpkt->data[3], &p_name[2], 12);

    txpkt->data_cnt += 15;
    txpkt->data[txpkt->data_cnt++] = ',';

    size = dsm_sprintf((char*)&ascill[0], "%d", img_len);
    memcpy(&txpkt->data[txpkt->data_cnt], ascill, size);
    txpkt->data_cnt += size;
    txpkt->data[txpkt->data_cnt++] = ',';

    if (flag == 1)
        txpkt->data[txpkt->data_cnt++] = '1';
    else
        txpkt->data[txpkt->data_cnt++] = '0';
    txpkt->data[txpkt->data_cnt++] = ',';

    size = dsm_sprintf((char*)&ascill[0], "%d", offset);
    memcpy(&txpkt->data[txpkt->data_cnt], ascill, size);
    txpkt->data_cnt += size;
    txpkt->data[txpkt->data_cnt++] = ',';

    dsm_sflash_img_read(img_addr, &txpkt->data[txpkt->data_cnt + 5], img_len);
    crc = crc16_get(&txpkt->data[txpkt->data_cnt + 5], img_len);

    size = dsm_sprintf((char*)&ascill[0], "%04X", crc);
    memcpy(&txpkt->data[txpkt->data_cnt], ascill, size);
    txpkt->data_cnt += 4;
    txpkt->data[txpkt->data_cnt++] = ',';
    txpkt->data_cnt += img_len;

    txpkt->retry_cnt = 0;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_fwup_direct_set_imgdata(uint32_t poll_flag, uint8_t* p_name,
                                       uint8_t* pimg_seg, uint16_t img_len,
                                       uint8_t flag, uint32_t offset)
{
    uint16_t crc;
    int8_t ascill[8];
    int32_t size = 0;
    ST_AT_CMD_TX_PKT* txpkt = dsm_atcmd_get_txpkt();

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    txpkt->cmd_id = ATCMD_FWUP;
    txpkt->cmd_type = AT_SET_REQ_DELIMETER;

    if (poll_flag)
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;
    else
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    txpkt->data_cnt = 0;

#if 0 /* bccho, 2024-06-24, delete */
    memcpy(txpkt->data, p_name, 2);
    txpkt->data[2] = 'V';
    memcpy(&txpkt->data[3], &p_name[2], 12);
    txpkt->data_cnt += 15;
#else
    memcpy(txpkt->data, p_name, 14);
    txpkt->data_cnt += 14;
#endif

    txpkt->data[txpkt->data_cnt++] = ',';

    size = dsm_sprintf((char*)&ascill[0], "%d", img_len);
    memcpy(&txpkt->data[txpkt->data_cnt], ascill, size);
    txpkt->data_cnt += size;
    txpkt->data[txpkt->data_cnt++] = ',';

    if (flag == 1)
        txpkt->data[txpkt->data_cnt++] = '1';
    else
        txpkt->data[txpkt->data_cnt++] = '0';
    txpkt->data[txpkt->data_cnt++] = ',';

    size = dsm_sprintf((char*)&ascill[0], "%d", offset);
    memcpy(&txpkt->data[txpkt->data_cnt], ascill, size);
    txpkt->data_cnt += size;
    txpkt->data[txpkt->data_cnt++] = ',';

    memcpy(&txpkt->data[txpkt->data_cnt + 5], pimg_seg, img_len);
    crc = crc16_get(&txpkt->data[txpkt->data_cnt + 5], img_len);

    size = dsm_sprintf((char*)&ascill[0], "%04X", crc);
    memcpy(&txpkt->data[txpkt->data_cnt], ascill, size);
    txpkt->data_cnt += 4;
    txpkt->data[txpkt->data_cnt++] = ',';
    txpkt->data_cnt += img_len;

    txpkt->retry_cnt = 0;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_fwup_set_action(uint32_t poll_flag, uint8_t* pfwinfo)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    gst_atcmd_tx_pkt.cmd_id = ATCMD_FWUPOP;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;

    if (poll_flag)
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;
    else
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

#if 0 /* bccho, 2024-06-24, delete */
    gst_atcmd_tx_pkt.data_cnt = 15;
    memcpy(gst_atcmd_tx_pkt.data, pfwinfo, 2);
    gst_atcmd_tx_pkt.data[2] = 'V';
    memcpy(&gst_atcmd_tx_pkt.data[3], &pfwinfo[2], 12);
#else
    gst_atcmd_tx_pkt.data_cnt = 14;
    memcpy(gst_atcmd_tx_pkt.data, pfwinfo, 14);
#endif

    gst_atcmd_tx_pkt.retry_cnt = 0;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_listen(uint32_t poll_flag)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_LISTEN;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_set_listen(uint32_t poll_flag, uint8_t on)
{
    DPRINTF(DBG_TRACE, "%s %d\r\n", __func__, on);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_LISTEN;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 1;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    /*
    "AT+LISTEN:1\r" 모뎀 동작 모드 상태 설정, listen 모드 설정 요청을 받으면
    SUN은 기존 SUN 연결 정보들을 초기화한다. 0 : listen 모드 동작 꺼짐 (일반
    모드 동작 수행) 1 : listen 모드 동작 켜짐 (수신전용 모드 동작)
    */
    if (on)
    {
        gst_atcmd_tx_pkt.data[0] = '1';
        sun_set_listen();  // 수신 모드
    }
    else
    {
        gst_atcmd_tx_pkt.data[0] = '0';
        sun_set_no_listen();  // 일반 모드
    }

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_set_mode(uint32_t poll_flag, uint8_t router_n_coordi)
{
    DPRINTF(DBG_TRACE, "%s %d\r\n", __func__, router_n_coordi);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_MODE;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 1;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    /* 0: 코디네이터 동작, 1: 라우터 동작 */
    if (router_n_coordi)  // router
        gst_atcmd_tx_pkt.data[0] = '1';
    else  // coordi
        gst_atcmd_tx_pkt.data[0] = '0';

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_mode(uint32_t poll_flag, bool retry_enable)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_MODE;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    if (retry_enable)
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;
    else
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_set_rsp_bearer(uint32_t poll_flag, uint32_t err, uint8_t* pdata,
                              uint8_t data_len)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_BEARER;
    gst_atcmd_tx_pkt.cmd_type = AT_RSP_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = data_len;
    gst_atcmd_tx_pkt.rsp_err = err;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;

    memcpy(gst_atcmd_tx_pkt.data, pdata, data_len);

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_rsp_stdver(uint32_t poll_flag, uint32_t err, uint8_t* pdata,
                              uint8_t data_len)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_STDVER;
    gst_atcmd_tx_pkt.cmd_type = AT_RSP_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = data_len;
    gst_atcmd_tx_pkt.rsp_err = err;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;

    memcpy(gst_atcmd_tx_pkt.data, pdata, data_len);

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

// AT+ZCDOFFSET=offset_usec\rOK\r
void dsm_atcmd_get_rsp_zcdoffset(uint32_t poll_flag, uint32_t err,
                                 uint8_t* pdata, uint8_t data_len)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_ZCDOFFSET;
    gst_atcmd_tx_pkt.cmd_type = AT_RSP_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = data_len;
    gst_atcmd_tx_pkt.rsp_err = err;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;

    memcpy(gst_atcmd_tx_pkt.data, pdata, data_len);

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_rsp_zcden(uint32_t poll_flag, uint32_t err, uint8_t* pdata,
                             uint8_t data_len)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_ZCDEN;
    gst_atcmd_tx_pkt.cmd_type = AT_RSP_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = data_len;
    gst_atcmd_tx_pkt.rsp_err = err;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;

    memcpy(gst_atcmd_tx_pkt.data, pdata, data_len);

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_set_rsp_en_de_crypt(uint32_t poll_flag, ATCMD_CNTX* p_cmd_cntx,
                                   uint32_t err, uint8_t* pdata,
                                   uint16_t data_len)
{
#if 0 /* bccho, 2023-08-17 */    
    uint16_t crc;
    int8_t ascill[8];
    int32_t size = 0;
    ST_AT_CMD_TX_PKT* txpkt = dsm_atcmd_get_txpkt();

    DPRINTF(DBG_TRACE, "%s: [ %s ]\r\n", __func__, p_cmd_cntx->cmdStr);

    txpkt->cmd_id = p_cmd_cntx->id;
    txpkt->cmd_type = AT_RSP_DELIMETER;
    txpkt->data_cnt = 0;
    txpkt->retry_cnt = 0;
    txpkt->rsp_err = err;
    txpkt->rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;

    switch (err)
    {
    case AT_DATA_RX_OK:
        size = dsm_sprintf((char*)&ascill[0], "%d", data_len);
        memcpy(&txpkt->data[txpkt->data_cnt], ascill, size);
        txpkt->data_cnt += size;
        txpkt->data[txpkt->data_cnt++] = ',';

        crc = crc16_get(pdata, data_len);

        size = dsm_sprintf((char*)&ascill[0], "%04X", crc);
        memcpy(&txpkt->data[txpkt->data_cnt], ascill, size);
        txpkt->data_cnt += 4;
        txpkt->data[txpkt->data_cnt++] = ',';

        memcpy(&txpkt->data[txpkt->data_cnt], pdata, data_len);
        txpkt->data_cnt += data_len;

        break;
    case AT_DATA_CRC_ERR:
        txpkt->data[txpkt->data_cnt++] = '0';

        break;
    case AT_DATA_EN_DE_CRYPT_ERR:
        txpkt->data[txpkt->data_cnt++] = '1';

        break;
    default:

        break;
    }

    dsm_atcmd_tx(FALSE, txpkt);
#endif
}

void dsm_atcmd_set_485(uint32_t poll_flag, uint8_t enable)
{
    DPRINTF(DBG_TRACE, "%s: enable[%d]\r\n", __func__, enable);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_485;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 1;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    if (enable)
        gst_atcmd_tx_pkt.data[0] = '1';
    else
        gst_atcmd_tx_pkt.data[0] = '0';

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_485(uint32_t poll_flag)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_485;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_set_zcd(uint32_t poll_flag, uint8_t enable)
{
    DPRINTF(DBG_TRACE, "%s: enable[%d]\r\n", __func__, enable);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_ZCD;
    gst_atcmd_tx_pkt.cmd_type = AT_SET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 1;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    if (enable)
        gst_atcmd_tx_pkt.data[0] = '1';
    else
        gst_atcmd_tx_pkt.data[0] = '0';

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_zcd(uint32_t poll_flag)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_ZCD;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_zcd_time(uint32_t poll_flag)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_ZCDT;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_hash(uint32_t poll_flag, bool retry_enable)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_SHA256;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;

    if (retry_enable)
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;
    else
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

void dsm_atcmd_get_fwver(uint32_t poll_flag, bool retry_enable)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_FWVER;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    if (retry_enable)
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;
    else
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

/* bccho, 2024-09-05, 삼상 */
void dsm_atcmd_get_mdlink(uint32_t poll_flag, bool retry_enable)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    gst_atcmd_tx_pkt.cmd_id = ATCMD_MDLINK;
    gst_atcmd_tx_pkt.cmd_type = AT_GET_REQ_DELIMETER;
    gst_atcmd_tx_pkt.data_cnt = 0;
    gst_atcmd_tx_pkt.retry_cnt = 0;
    if (retry_enable)
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD_ANSWER_AFTER_TX;
    else
        gst_atcmd_tx_pkt.rsp_need_after_tx = ATCMD__NO_ANSWER_AFTER_TX;

    dsm_atcmd_tx(poll_flag, &gst_atcmd_tx_pkt);
}

bool dsm_atcmd_if_is_valid(uint32_t ifstate, uint8_t get_sha256)
{
    uint8_t modem_type;
    ST_ATCMD_TMP_BUF st_atmcd_from_modem;
    ST_AT_CMD_RX_PKT atcmd_com_pkt;
    uint8_t cnt = 0;
    bool ret = false;

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    if (ifstate == MEDIA_RUN_SUN)
    {
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_SUN);
        modem_type = INT_MODEM_TYPE;
    }
    else
    {
        dsm_media_set_fsm_if_ATCMD(MEDIA_RUN_EXT);
        modem_type = EXT_MODEM_TYPE;
    }

    while (cnt < 2)
    {
        ret = false;

        if (get_sha256)
            dsm_atcmd_get_hash(TRUE, TRUE);
        else
            dsm_atcmd_get_fwver(TRUE, TRUE);

        dsm_atcmd_tx_pkt_init();
        memset(&st_atmcd_from_modem, 0x00, sizeof(ST_ATCMD_TMP_BUF));

        if (dsm_atcmd_if_rx_polling_valid_check(modem_type,
                                                &st_atmcd_from_modem) &&
            st_atmcd_from_modem.len)
        {
            DPRINTF(DBG_TRACE, "%s\r\n", __func__);
            //
            if (dsm_atcmd_rx_parser((uint8_t*)st_atmcd_from_modem.string,
                                    st_atmcd_from_modem.len,
                                    &atcmd_com_pkt) == AT_ERR_NONE_PARSER_OK)
            {
                if (st_atmcd_from_modem.len > 5)
                {
                    dsm_atcmd_rx_proc(&atcmd_com_pkt);
                    ret = true;
                    dsm_atcmd_rx_pkt_init();
                    atcmd_com_pkt.len = 0;
                    break;
                }
            }
            OSTimeDly(OS_MS2TICK(MODEM_RX_POLL_RETRY_TIME));
        }
        dsm_atcmd_rx_pkt_init();
        atcmd_com_pkt.len = 0;
        cnt++;
    }

    DPRINTF(DBG_TRACE, "%s ret[%d]\r\n", __func__, ret);
    return ret;
}
