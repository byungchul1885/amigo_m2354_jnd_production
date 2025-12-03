#ifndef DL_H
#define DL_H 1

#define CLIENT_ADDR_PUBLIC 0x21               // 0x10 : association 1
#define CLIENT_ADDR_UTILITY 0x23              // 0x11 : association 2
#define CLIENT_ADDR_PRIVATE SAP_ADDR_PRIVATE  // 0x15
#define CLIENT_ADDR_485COMM 0x45              // 0x22
#define CLIENT_ADDR_ASSO3_UTILITY 0x25        // 0x12 : association 3
#define CLIENT_ADDR_ASSO3_SITE 0x27           // 0x13 : association 4

#define SVR_ADDR_BROADCASTING_U (0x7f << 1)           // 0xFE
#define SVR_ADDR_BROADCASTING_L ((0x7f << 1) | 0x01)  // 0xFF
#define SVR_ADDR_U (0x01 << 1)                        // 0x02
#define SVR_ADDR_SEC_U (0x12 << 1)                    // 0x24
#define SVR_ADDR_L ((dl_meter_addr << 1) | 0x01)      // 0x10 + serial number

#define frm_type_seg_field cli_buff[0]
#define frm_length_field cli_buff[1]
#define frm_svraddrU_field cli_buff[2]
#define frm_svraddrL_field cli_buff[3]
#define frm_clientaddr_field cli_buff[4]
#define frm_control_field cli_buff[5]
#define frm_hcs_field cli_buff[6]  // 2 byte
#define frm_llchd0_field cli_buff[8]
#define frm_llchd1_field cli_buff[9]
#define frm_llchd2_field cli_buff[10]
#define frm_appl_field_llc cli_buff[11]   // none-seg, first-seg
#define frm_appl_field_nollc cli_buff[8]  // mid-seg, last-seg

#define DL_HEADER_LEN 8  // total 8 byte
#define DL_HEADER_LEN_NOLLC DL_HEADER_LEN
#define DL_HEADER_LEN_LLC (DL_HEADER_LEN + 3)  // + 3 (LLC)
#define DL_HEADER_LEN_NOLLC_FCS (DL_HEADER_LEN_NOLLC + 2)
#define DL_HEADER_LEN_LLC_FCS (DL_HEADER_LEN_LLC + 2)

#define FRM_SEG_BIT 0x08     // frame segment bit (1: segment)
#define POLL_FINAL_BIT 0x10  // poll and final bit
#define NUM_INF_BIT 0x01     // 0 -> numbered information, 1->command
#define DL_RR 0x01           // RR message (low 4 bit)
#define DL_RNR 0x05          // RNR message (low 4 bit)
#define DL_SNRM 0x83         // set normal respond mode
#define DL_DISC 0x43         // disconnect command
#define DL_UA 0x63           // Unnumbered acknowledge
#define DL_DM 0x0f           // disconnected mode response
#define DL_FRMR 0x87         // frame reject command
#define DL_UI 0x03           // Unnumbered Information command/response

#define MAX_INFORMATION_FIELD_LENGTH 0x07FF
#define MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO 0x0400 /* 1024 */
#define MIN_INFORMATION_FIELD_LENGTH_FOR_KEPCO 0x80

#define TX_MAX_INFORMATION_FIELD 0x05
#define RX_MAX_INFORMATION_FIELD 0x06

#define NEGO_PARAM_SIZE_ASSO3 (21 + 2)
#define TX_MAX_INF_LEN_ASSO3 1024
#define RX_MAX_INF_LEN_ASSO3 1024
#define NEGO_PARAM_SIZE 21
#define TX_MAX_INF_LEN 128
#define RX_MAX_INF_LEN 128
#define TX_WINDOW_SIZE 1  // 4 bytes type (long)
#define RX_WINDOW_SIZE 1  // 4 bytes type (long)

#define LLC_HEADER_LEN 3

#if 0
typedef enum
{
    PROD_SUCCESS,
    PROD_NOT_SUP_CMD,
    PROD_NOT_ACC_USER,
    PROD_FAIL
} dl_prod_sts_type;

typedef enum
{
    PROD_CTRL_LOG_IN = 1,
    PROD_CTRL_LOG_OUT,
    PROD_CTRL_FACTORY_RESET,
    PROD_CTRL_CAL_BEGIN,
    PROD_CTRL_CAL_END,
    PROD_CTRL_FACTOR,
    PROD_CTRL_SERIAL_NO,
    PROD_CTRL_DEVICE_NAME,
    PROD_CTRL_CURR_TEMP,
    PROD_CTRL_FIRM_VER,
    prod_cmd_num
} dl_prod_cmd_type;
#endif

typedef enum
{
    DL_NDM_STATE,
    DL_NRM_STATE
} dl_state_type;

typedef enum
{
    I_FRAME,
    RR_FRAME,
    RNR_FRAME,
    SNRM_FRAME,
    DISC_FRAME,
    UA_FRAME,
    DM_FRAME,
    FRMR_FRAME,
    UI_FRAME
} dl_frame_tx_type;

typedef enum
{
    SEG_NONE,
    SEG_FIRST,
    SEG_MID,
    SEG_LAST
} seg_state_type;

typedef struct _st_dl_hdlc_ext_
{
    uint16_t type;
    uint16_t len;
    uint8_t data[TX_BUFF_SIZE];
} ST_DL_HDLC_EXT;

#define FLAH_SEG_BUFF_ADDR (0x40000 - FLASH_PAGE_SIZE_1)

#define dl_is_connected() (bool)(dl_conn_state == DL_NRM_STATE)
#define dl_is_info_frame() (bool)((dl_control & NUM_INF_BIT) == 0)
#define dl_is_snrm_frame() (bool)(dl_control == DL_SNRM)
#define dl_is_disc_frame() (bool)(dl_control == DL_DISC)
#define dl_is_rr_frame() (bool)((dl_control & 0x0f) == DL_RR)

extern uint16_t dl_tx_max_info;
extern uint16_t dl_rx_max_info;
#define dl_get_tx_max_info() dl_tx_max_info
#define dl_get_rx_max_info() dl_rx_max_info

extern uint8_t dl_meter_addr;
extern dl_state_type dl_conn_state;

void dl_init(void);
void dl_proc(bool frmok);
void prod_dl_proc(bool frmok);
void dl_stop_force(void);
void dl_fill_LLC_header(uint8_t *lptr);
void dl_send_appl_msg(uint8_t *buf, int len);
uint16_t dl_server_macaddr(void);
int8_t dl_client_macaddr(void);
void dl_server_macaddr_set(void);
void dl_set_tx_max_info(uint16_t tx_max_len);
void dl_send_UI_frame(bool seg, uint8_t *buf, uint16_t len);
void dl_send_appl_push_datanoti_msg(uint8_t *buf, int len);

#endif
