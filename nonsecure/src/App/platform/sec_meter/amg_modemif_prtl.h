#ifndef __AMG_MODEMIF_PRTL__
#define __AMG_MODEMIF_PRTL__

/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_imagetransfer.h"

/*
******************************************************************************
*    Definition
******************************************************************************
*/
#define ATCMD_RETRY_MAX_COUNT 3

typedef enum
{
    INT_MODEM_TYPE = 0,
    EXT_MODEM_TYPE,
    INT_PLC_MODEM_TYPE,  // jp.kim 25.01.21
    MAX_MODEM_TYPE
} EN_MODEM_T;

#define INT_MODEM_RESET 0
#define EXT_MODEM_RESET 1

#define CLIENT_ATCMD_STRING_MAX_SIZE 2048

typedef enum
{
    AT_FSM_A,
    AT_FSM_T,
    AT_FSM_plus,
    AT_FSM_CMD,
    AT_FSM_TYPE,
    AT_FSM_GET_END,
    AT_FSM_GET_RSP_DATA,
    AT_FSM_SET_DATA,
    AT_FSM_SET_DATA_END

} EN_AT_PASER_FSM_STATE;

typedef enum
{
    AT_ERR_NONE = 0,
    AT_ERR_FISRT_A,
    AT_ERR_SECOND_T,
    AT_ERR_PLUS,
    AT_ERR_CMD_VALIDITY,
    AT_ERR_CMD_LEN,
    AT_ERR_GET_RSP_DATA_LEN,
    AT_ERR_SET_DATA_LEN,
    AT_ERR_SET_DATAOK_LEN,
    AT_ERR_TYPE,
    AT_ERR_GET_END,
    AT_ERR_DATA_END,
    AT_ERR_SET_DATA_END,

    AT_ERR_NONE_PARSER_OK

} EN_AT_RESULT_TYPE;

typedef enum
{
    FWU_FSM_NONE,
    FWU_FSM_INIT,
    FWU_FSM_UP,
    FWU_FSM_UP_OP,
    FWU_FSM_SUCCESS,
    FWU_FSM_FAIL,
} EN_FWU_FSM;

#define AT_DATA_RX_OK 0
#define AT_DATA_CRC_ERR 1
#define AT_DATA_EN_DE_CRYPT_ERR 2

#define AT_FIRST_A 'A'
#define AT_SECOND_T 'T'
#define AT_plus '+'
#define AT_Question '?'  // Get_Req
#define AT_Colon ':'     // SET
#define AT_Equal '='     // Get_RSP

#define AT_GET_REQ_DELIMETER AT_Question
#define AT_RSP_DELIMETER AT_Equal
#define AT_SET_REQ_DELIMETER AT_Colon

#define AT_CMD_MAX_SIZE 10
#define AT_GET_RSP_DATA_MAX_SIZE 512
#define AT_SET_DATA_MAX_SIZE 512
#define AT_SET_DATA_OK_MAX_SIZE 5

typedef enum
{
    ATCMD_BEARER = 0,
    ATCMD_MAC,
    ATCMD_PPDU,
    ATCMD_MMID,
    ATCMD_HWVER,
    ATCMD_FWVER,
    ATCMD_FUN,
    ATCMD_TXPWR,
    ATCMD_NWKDR,
    ATCMD_PMAC,
    ATCMD_PANID,
    ATCMD_CHAN,
    ATCMD_ASSOPAN,
    ATCMD_ASSOPER,
    ATCMD_MAXBE,
    ATCMD_MAXRTY,
    ATCMD_RNS,
    ATCMD_NWKKEY,
    ATCMD_FWUPSTA,
    ATCMD_FWUP,
    ATCMD_FWUPOP,
    ATCMD_FWUPINIT,
    ATCMD_PANDLST,
    ATCMD_NEWPAND,
    ATCMD_METERID,
    ATCMD_MFDATE,
    ATCMD_EBR,
    ATCMD_EBLIST,
    ATCMD_MODE,
    ATCMD_BAUD,
    ATCMD_SHA256,
    ATCMD_LISTEN,
    ATCMD_POLL,
    ATCMD_ENCRYPT,
    ATCMD_DECRYPT,
    ATCMD_485,
    ATCMD_ZCD,
    ATCMD_ZCDT,
    ATCMD_VFREE,
    ATCMD_MDLINK,
    ATCMD_PWRNOTIFY,
    ATCMD_STDVER,
    ATCMD_ZCDOFFSET,
    ATCMD_ZCDEN,
} enATCMD_ID;

/* bccho, 2024-09-05, 삼상 */
typedef enum
{
    SUN_CORDI = 0,
    SUN_DEVICE_ON_ASSO,
    SUN_DEVICE_OFF_ASSO
} enSUN_MODE_TYPE;

#define AT_RST_NORMAL 0
#define AT_RST_FACTORY 1

#define AT_LISTEN_OFF 0
#define AT_LISTEN_ON 1

typedef enum
{
    AT_BAUD_115200 = 0,
    AT_BAUD_230400 = 1,
    AT_BAUD_460800 = 2
} EN_AT_BAUD;

typedef enum
{
    E_MODEM_BAUD_RATE_v115200 = 115200,
    E_MODEM_BAUD_RATE_v230400 = 230400,
    E_MODEM_BAUD_RATE_v460800 = 460800
} EN_MODEM_BAUD_RATE_VAL;

#define MODEM_ID_SIZE 11

typedef enum
{
    E_EXT_PLC_MODEM_OP_NONE,
    E_EXT_PLC_MODEM_SUN_COORD_OP,
    E_EXT_PLC_MODEM_MODEM_ID,
    E_EXT_PLC_MODEM_MODEM_ID_WAIT,
    E_EXT_PLC_MODEM_OP_OK
} EN_EXT_PLC_MODEM_STATE;

typedef enum
{
    BEARER_NONE = 0,
    BEARER_SUN_MAC = 1,
    BEARER_SUN_IP = 2,
    BEARER_LTE = 3,
    BEARER_iotPLC = 4,
    BEARER_HPGP = 5,
    BEARER_C_SMGW = 99
} EN_BEARER;
/*
******************************************************************************
*    MACRO
******************************************************************************
*/
extern uint8_t g_sun_listen;
#define sun_set_listen() (g_sun_listen = AT_LISTEN_ON)
#define sun_set_no_listen() (g_sun_listen = AT_LISTEN_OFF)
#define sun_is_listen() (g_sun_listen == AT_LISTEN_ON)
#define sun_is_not_listen() (g_sun_listen == AT_LISTEN_OFF)

typedef enum
{
    EMDM_FSM_IDLE,
    EMDM_FSM_ID_CHECK_TIME
} EN_EMDM_FSM;
/*
******************************************************************************
*    DATA TYPE
******************************************************************************
*/

typedef struct _atcmd_rx_pkt_
{
    uint8_t fsm;
    uint16_t len;
    uint16_t cnt;
    uint8_t cmd_type;
    uint8_t cmd_cnt;
    uint16_t data_cnt;
    uint8_t data_ok_cnt;
    uint8_t pkt[1024];
    uint8_t en_de_state;
    uint8_t en_de_parm_cnt;
    uint8_t en_de_hd_len;
    uint32_t en_de_data_len;
    uint16_t idx;
} ST_AT_CMD_RX_PKT;

typedef struct _atcmd_tx_data_pkt_
{
    uint8_t cmd_id;
    uint8_t cmd_type;
    uint16_t data_cnt;
    uint8_t data[1024];
    uint8_t retry_cnt;
    uint32_t rsp_err;
    uint8_t rsp_need_after_tx;

} ST_AT_CMD_TX_PKT;

typedef struct _ATCMD_CNTX
{
    uint32_t id;
    char* cmdStr;
    int paramCnt;
} ATCMD_CNTX;

typedef struct _st_mdm_mic_fwupdl_
{
    uint8_t name[IMAGE_FW_NAME_MAX_SIZE];
    uint32_t image_size;
    uint8_t fw_type;
    uint32_t blk_size;
    uint32_t blk_num;
    uint32_t snd_cnt;
    uint32_t start_dl_addr;
    uint8_t retry_cnt;
    uint8_t fail_fsm;

} ST_MDM_MIC_FWUP_DL;

typedef struct _st_mdm_id_
{
    uint8_t id[MODEM_ID_SIZE];
    uint16_t CRC_M;
} ST_MDM_ID;

typedef struct _st_atcmd_tmp_buf_
{
    uint16_t len;
    uint8_t string[CLIENT_ATCMD_STRING_MAX_SIZE];
} ST_ATCMD_TMP_BUF;

typedef struct _st_at_baud_
{
    EN_AT_BAUD imodem;
    EN_AT_BAUD emodem;
    uint16_t CRC_M;
} ST_AT_BAUD;

/*
******************************************************************************
*    GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*    FUNCTIONS
******************************************************************************
*/
ST_AT_BAUD* dsm_modemif_get_atbaud_info(void);
EN_MODEM_BAUD_RATE_VAL dsm_modemif_atbaud_2_baudvalue(EN_AT_BAUD at_baud);
EN_AT_BAUD dsm_modemif_baudvalue_2_atbaud(uint32_t baud_rate);
void dsm_modemif_baud_road(void);
void dsm_modemif_baud_nvwrite(ST_AT_BAUD* pst_at_baud);
void dsm_modemif_default_baudrate(void);
void dsm_imodemif_init(uint8_t first);
void dsm_imodemif_deinit(void);
char* dsm_mdm_mic_fwup_fsm_string(uint32_t fsm);
void dsm_emodemif_init(void);
char* dsm_bearer_string(uint8_t bearer);
uint8_t dsm_get_bearer(void);
void dsm_set_bearer(uint8_t bearer);

void dsm_atcmd_client_modem_buff_init(uint8_t modem_type);
ST_ATCMD_TMP_BUF* dsm_get_atcmd_from_client(uint8_t modem_type);
ST_ATCMD_TMP_BUF* dsm_get_atcmd_from_modem(uint8_t modem_type);

void dsm_atcmd_if_poll_rx_init(uint8_t modem_type);
void dsm_atcmd_if_default_init(uint8_t modem_type);
bool dsm_atcmd_if_rx_polling(uint8_t modem_type,
                             ST_ATCMD_TMP_BUF* pst_atmcd_from_modem);
void dsm_atcmd_poll_tx_n_rx(uint8_t modem_type,
                            ST_ATCMD_TMP_BUF* pst_atmcd_client,
                            ST_ATCMD_TMP_BUF* pst_atmcd_from_modem);
void dsm_modem_hw_reset(uint8_t type);

void dsm_mdm_mic_fwup_set_fsm(uint8_t state);
uint8_t dsm_mdm_mic_fwup_get_fsm(void);
uint32_t dsm_mdm_mic_fwup_fsm_tx_proc(void);
bool dsm_mdm_mic_fwup_direct_up_txproc(uint8_t* pimg_seg, uint16_t len);
bool dsm_mdm_mic_fwup_direct_fsm_tx_proc(uint8_t* pdata, uint16_t len);
ATCMD_CNTX* dsm_atcmd_get_mt_cntx(uint32_t cmdid);
void dsm_mdm_mic_fwup_retry_TO_proc(void);
ST_MDM_MIC_FWUP_DL* dsm_mdm_mic_get_fwupdlinfo(void);
uint32_t dsm_mdm_id_is_valid(uint8_t* pid, uint8_t len);

void dsm_atcmd_fsm_rxbuf_reset(void);
void dsm_atcmd_rx_pkt_init(void);
void dsm_atcmd_tx_pkt_init(void);
uint8_t dsm_atcmd_rx_parser(uint8_t* buff, uint16_t size,
                            ST_AT_CMD_RX_PKT* p_com_pkt);
ST_AT_CMD_TX_PKT* dsm_atcmd_get_txpkt(void);
uint32_t dsm_atcmd_en_de_crypt_rx_proc(uint8_t cmd_id, uint8_t* pdata,
                                       uint16_t data_len, uint8_t* o_pdata,
                                       uint16_t* o_plen);
uint32_t dsm_atcmd_rx_proc(ST_AT_CMD_RX_PKT* p_com_pkt);
bool dsm_atcmd_poll_rx_proc(ST_AT_CMD_RX_PKT* p_com_pkt);
void dsm_atcmd_tx(uint32_t poll_flag, ST_AT_CMD_TX_PKT* tx_pkt);
void dsm_atcmd_tx_retry_proc(void);
void dsm_atcmd_set_meterid(uint32_t poll_flag);
#if 1 /* bccho, 2023-10-23, 제이엔디전자에서 추가 */
void dsm_atcmd_set_macaddress(uint32_t poll_flag);
#endif
void dsm_atcmd_set_mfdate(uint32_t poll_flag);
void dsm_atcmd_set_reset(uint32_t poll_flag, uint8_t reset_type);
void dsm_atcmd_get_baud(uint32_t poll_flag);
void dsm_atcmd_set_baud(uint32_t poll_flag, uint8_t baud_type);
void dsm_atcmd_get_modem_id(uint32_t poll_flag);
void dsm_atcmd_fwup_set_init(uint32_t poll_flag);
void dsm_atcmd_fwup_get_status(uint32_t poll_flag);
void dsm_atcmd_fwup_set_imgdata(uint32_t poll_flag, uint8_t* p_name,
                                uint32_t img_addr, uint16_t img_len,
                                uint8_t flag, uint32_t offset);
void dsm_atcmd_fwup_direct_set_imgdata(uint32_t poll_flag, uint8_t* p_name,
                                       uint8_t* pimg_seg, uint16_t img_len,
                                       uint8_t flag, uint32_t offset);
void dsm_atcmd_fwup_set_action(uint32_t poll_flag, uint8_t* pfwinfo);
void dsm_atcmd_get_listen(uint32_t poll_flag);
void dsm_atcmd_set_listen(uint32_t poll_flag, uint8_t on);
void dsm_atcmd_set_mode(uint32_t poll_flag, uint8_t router_n_coordi);
void dsm_atcmd_set_rsp_bearer(uint32_t poll_flag, uint32_t err, uint8_t* pdata,
                              uint8_t data_len);
void dsm_atcmd_set_rsp_en_de_crypt(uint32_t poll_flag, ATCMD_CNTX* p_cmd_cntx,
                                   uint32_t err, uint8_t* pdata,
                                   uint16_t data_len);
void dsm_atcmd_set_485(uint32_t poll_flag, uint8_t enable);
void dsm_atcmd_get_485(uint32_t poll_flag);
void dsm_atcmd_set_zcd(uint32_t poll_flag, uint8_t enable);
void dsm_atcmd_get_zcd(uint32_t poll_flag);
void dsm_atcmd_get_zcd_time(uint32_t poll_flag);
void dsm_atcmd_get_hash(uint32_t poll_flag, bool retry_enable);
void dsm_atcmd_get_fwver(uint32_t poll_flag, bool retry_enable);
bool dsm_atcmd_if_is_valid(uint32_t ifstate, uint8_t get_sha256);
void log_sys_sw_up(void);
void log_mtr_sw_up(void);
void log_int_modem_up(void);
void log_ext_modem_up(void);
void dsm_atcmd_get_mode(uint32_t poll_flag, bool retry_enable);

#endif /* __AMG_MODEMIF_PRTL__*/
