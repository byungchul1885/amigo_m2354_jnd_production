#ifdef M2354_CAN /* bccho, 2023-11-28 */
/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "options_sel.h"
#include "defines.h"
#include "isotp.h"
#include "amg_isotp_user.h"
#include "amg_can.h"
#include "whm.h"
#include "amg_utc_util.h"
#include "amg_meter_main.h"
#include "amg_media_mnt.h"

/*
******************************************************************************
*   Definition
******************************************************************************
*/
#define _D "[CAN-TP] "

/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/
/***********************************************
 Description                 CMD/DIR   SERIAL
************************************************
GW solicitation        01000    08 | FF FF FF
GW dlms exchange       10000    10 | 00 00 09
Meter advert           00001    01 | 00 00 09
Meter advert_1         00100    04 | FF FF FF
Meter dlms excahge     10001    11 | 00 00 09
Meter push             11001    19 | 00 00 09
***********************************************/

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/

#if defined(__ICCARM__)
#pragma data_alignment = 8
#endif

#define ISOTP_BUFSIZE (1024 + 20)

static uint8_t g_isotpRecvBuf[ISOTP_BUFSIZE];
static uint8_t g_isotpSendBuf[ISOTP_BUFSIZE];
IsoTpLink g_link;
uint32_t g_serial_number;
/*
******************************************************************************
*   FUNCTIONS - extern
******************************************************************************
*/
char* dsm_isotp_user_id_mode_string(uint32_t mode)
{
    switch (mode)
    {
    case NODE_ADVERTISEMENT_MODE_0:
        return "NODE_ADVERTISEMENT_MODE_0";
    case NODE_ADVERTISEMENT_MODE_1:
        return "NODE_ADVERTISEMENT_MODE_1";
    case NODE_SOLICITATION_MODE:
        return "NODE_SOLICITATION_MODE";
    case DLMS_EX_MODE:
        return "DLMS_EX_MODE";
    case DLMS_PUSH_MODE:
        return "DLMS_PUSH_MODE";

    default:
        return "CAN_ID_Unknown";
    }
}

#if 0 /* bccho, 2023-12-22 */
uint32_t isotp_user_get_ms(void) { return dsm_get_utc_time(); }
#else
uint32_t isotp_user_get_ms(void) { return xTaskGetTickCount(); }
#endif

uint32_t dsm_can_nv_read_serial_number(void) { return Generate_CAN_ID(); }

uint32_t dsm_isotp_user_set_can20B_id(uint32_t mode)
{
    // clang-format off
    /*
        CAN ID:
            +------------------------------------+
            | MSB                             LSB|
            +--------------+----+-----+----------+
            | 28 | 27 | 26 | 25 | 24  | 23 ... 0 |
            +----+----+----+----+-----+----------+
            | Mode         | R0 | Dir | 계기 ID    |
            +--------------+----+-----+----------+
            - Mode : bit[28:26]
                0b000 Node Advertisement(Power On시)
                0b001 Node Advertisement(Node Soliciation 요청 시)
                0b010 Node Solicitication
                0b100 DLMS Exchange
                0b111 DLMS Push
            - R0 : bit[25]
                Reserved Bit
            - Dir : bit[24]
                계기 -> SMGW : 1
                SMGW -> 계기 : 0
            - 계기 ID : bit[23:0]
                (1) Broadcast ID : 150,994,943 (0x8FFFFFF)
                (2) 계기 CAN-ID(10진수 8자리) : 제조사 번호 3자리 + 시리얼 번호 6자리 중, 하위 5자리 (0-99,999,999) 
                (3) Gateway CAN-ID : 100,000,000  (0x5F5E100) 
                (4) 예약ID : 100,000,001 (0x5F5E101) - 150,994,942 (0x8FFFFFE)
    */
    // clang-format on

    uint32_t can_id = 0;

    switch (mode)
    {
        /* Node Advertisement(Power On) : 0b000
           계기 Power On 시 또는 설치 후 계기 ID 전송 */
    case NODE_ADVERTISEMENT_MODE_0:

        /* Node Advertisement(Solicitation) : 0b001
           계기 ID 수집을 위한 Broadcast의 응답으로 Node Advertisement 전송 */
    case NODE_ADVERTISEMENT_MODE_1:

        /* DLMS Exchange : 0b100
           DLMS 통신을 위한 ID 모드  */
    case DLMS_EX_MODE:

        /* DLMS Push : 0b110
           DLMS Push 통신을 위한 ID 모드 */
    case DLMS_PUSH_MODE:
        can_id = ((mode << 26) | (CAN_ID_R0 << 25) | (ISO_TP_TX << 24) |
                  g_serial_number);
        break;

        /* Node Solicitation : 0b010
           SMGW 계기 CAN ID 수집을 위해 Broadcast로 전송 */
    case NODE_SOLICITATION_MODE:
        can_id = ((mode << 26) | (CAN_ID_R0 << 25) | (ISO_TP_RX << 24) |
                  CAN_BROADCAST_SERIAL);
        break;
    }

    return can_id;
}

void dsm_isotp_user_init(void)
{
    uint32_t can_id;

    // generate can id
    g_serial_number = dsm_can_nv_read_serial_number();
    MSG07("can_id (24bit): 0x%06X", g_serial_number);

    can_id = dsm_isotp_user_set_can20B_id(DLMS_EX_MODE);

    isotp_init_link(&g_link, can_id, g_isotpSendBuf, sizeof(g_isotpSendBuf),
                    g_isotpRecvBuf, sizeof(g_isotpRecvBuf));
}

void* dsm_isotp_user_get_link(void) { return &g_link; }

void dsm_isotp_user_tx(uint32_t mode, uint8_t* ptx, uint16_t len)
{
    uint32_t can_id = dsm_isotp_user_set_can20B_id(mode);

    // DPRINTF(DBG_TRACE, "can_id [0x%08X]\r\n", can_id);
    MSG00("dsm_isotp_user_tx(), mode: %d, can_id (29bit): 0x%08X", mode,
          can_id);

    isotp_init_link(&g_link, can_id, g_isotpSendBuf, sizeof(g_isotpSendBuf),
                    g_isotpRecvBuf, sizeof(g_isotpRecvBuf));

    isotp_send(&g_link, ptx, len);
}

void dsm_isotp_user_tx_poll(void) { isotp_poll(&g_link); }

int isotp_user_send_can(uint32_t arbitration_id, uint8_t* data, uint8_t size)
{
    dsm_can_tx(arbitration_id, data, size);

    return 0;
}

uint32_t dsm_isotp_user_proc(void)
{
    uint32_t mode;
    IsoTpLink* plink = (IsoTpLink*)dsm_isotp_user_get_link();

    mode = (plink->receive_arbitration_id >> 26) & 0x07;

    // DPRINTF(DBG_NONE, _D"%s: Mode[%s], ID[0x%08X], receive_status[%d]\r\n",
    // __func__, dsm_isotp_user_id_mode_string(mode),
    // plink->receive_arbitration_id, plink->receive_status);
    // DPRINT_HEX(DBG_TRACE, "CAN_RX_T", plink->receive_buffer,
    // plink->receive_offset, DUMP_ALWAYS);

    /* receive_status is IsoTpReceiveStatusTypes, 0: IDLE, 1: ING, 2: FULL */
    DPRINTF(DBG_NONE, _D "Mode[%s], ID[0x%08X], Status[%d]\r\n",
            dsm_isotp_user_id_mode_string(mode), plink->receive_arbitration_id,
            plink->receive_status);

    switch (mode)
    {
    case NODE_ADVERTISEMENT_MODE_0:
    case NODE_ADVERTISEMENT_MODE_1:
        break;

    case NODE_SOLICITATION_MODE:
        dsm_isotp_user_tx(NODE_ADVERTISEMENT_MODE_1, NULL, 0);
        break;

    case DLMS_EX_MODE:
        break;

    case DLMS_PUSH_MODE:

        break;
    }

    return mode;
}

uint32_t dsm_can_is_valid_ext_id(uint32_t ext_id)
{
    uint32_t mode, dir;
    uint32_t ret = FALSE;
    uint32_t serial_num;

    mode = (ext_id >> 26) & 0x07;
    dir = (ext_id >> 24) & 0x01;
    serial_num = ext_id & 0xFFFFFF;

    if (dir == ISO_TP_RX)
    {
        if (mode == NODE_SOLICITATION_MODE &&
            serial_num == CAN_BROADCAST_SERIAL)
        {
            MSG00("valid_ext_id, solicit, broad");
            ret = TRUE;
        }
        else if (mode == DLMS_EX_MODE && serial_num == g_serial_number)
        {
            MSG00("valid_ext_id, dlms ex");
            ret = TRUE;
        }
    }
    // DPRINTF(DBG_TRACE, "%s: mode[%s], dir[%d], serial[0x%08X], ret[%d]\r\n",
    // __func__, dsm_isotp_user_id_mode_string(mode), dir, serial_num, ret);

    MSG00("valid_ext_id, %d", ret);

    return ret;
}

void dsm_can_advertisement_power_on(void)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    // dsm_media_set_rx_evt(CAN_IF_RX_EVT);
    // dsm_media_chg_evt_proc(dsm_media_get_rx_evt(), 0);
    // dsm_media_if_hdlc_fsm_proc(dsm_media_get_chg_evt());

    dsm_isotp_user_tx(NODE_ADVERTISEMENT_MODE_0, NULL, 0);

    // 부팅시 처음 tx 이므로 ifstate 초기화 한다.
    dsm_media_set_fsm_if_hdlc(MEDIA_RUN_NONE);
    dsm_meter_if_rx_inter_frame_timer_stop();
}

void dsm_can_advertisement_solicitation(void)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    dsm_media_set_fsm_if_hdlc(MEDIA_RUN_CAN);

    // dsm_media_set_rx_evt(CAN_IF_RX_EVT);
    // dsm_media_chg_evt_proc(dsm_media_get_rx_evt(), 0);
    // dsm_media_if_hdlc_fsm_proc(dsm_media_get_chg_evt());

    dsm_isotp_user_tx(NODE_ADVERTISEMENT_MODE_1, NULL, 0);
}

void dsm_can_solicitation(void)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    dsm_media_set_fsm_if_hdlc(MEDIA_RUN_CAN);

    // dsm_media_set_rx_evt(CAN_IF_RX_EVT);
    // dsm_media_chg_evt_proc(dsm_media_get_rx_evt(), 0);
    // dsm_media_if_hdlc_fsm_proc(dsm_media_get_chg_evt());

    dsm_isotp_user_tx(NODE_SOLICITATION_MODE, NULL, 0);
}
#endif