#ifndef __AMG_DLMS_HDLC__
#define __AMG_DLMS_HDLC__

/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"

/*
******************************************************************************
*    Definition
******************************************************************************
*/
#define HDLC_MAX_PACKET_NUM (1024 + 20)
#define HDLC_CTRL_FRAME_LEN_TMP 10
#define HDLC_FLAG_TMP 0x7E

#define HDLC_PKT_CNTX_NUM 4

#define HDLC_PKT_CNTX_485 0
#define HDLC_PKT_CNTX_SUN 1
#define HDLC_PKT_CNTX_CAN 2
#define HDLC_PKT_CNTX_PLC 3

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
typedef enum
{
    HDLC_PASER_FSM_START,
    HDLC_PASER_FSM_FORMAT,
    HDLC_PASER_FSM_LEN,
    HDLC_PASER_FSM_DATA,
    HDLC_PASER_FSM_CRC,
    HDLC_PASER_FSM_END,
} EN_HDLC_PASER_FSM_STATE;

typedef enum
{
    HDLC_FRAME_NO_ERR_TMP = 0,
    HDLC_FRAME_RCV_CONTINUE_TMP, /*HDLC frame이 정상적으나, packet을 추가적으로
                                    수신하여 decoding 필요...*/
    HDLC_FRAME_HEAD_FLAG_ERR_TMP,
    HDLC_FRAME_TAIL_FLAG_ERR_TMP,
    HDLC_FRAME_TYPE_ERROR_TMP,
    HDLC_FRAME_LENGTH_ERROR_TMP,
    HDLC_FRAME_FRAGMENT_ERROR_TMP,
    HDLC_FRAME_CRC_ERR_TMP,
    HDLC_FRAME_LLC_ERR_TMP,
    HDLC_FRAME_GET_RESPONSE_ERR_TMP,
    HDLC_FRAME_SET_RESPONSE_ERR_TMP,
    HDLC_FRAME_NORMAL_RESPONSE_ERR_TMP,
    HDLC_FRAME_INVOKE_ID_RESPONSE_ERR_TMP,

    HDLC_FRAME_MAX_ERR_TMP
} ENUM_FRM_RESULT_TYPE_TMP;

typedef struct _hdlc_pkt_
{
    uint8_t fsm;
    uint16_t len;
    uint16_t cnt;
    uint8_t seg;
    uint8_t pkt[HDLC_MAX_PACKET_NUM];

} ST_HDLC_RX_PKT;

typedef struct _hdlc_com_pkt_
{
    uint16_t len;
    uint8_t data[HDLC_MAX_PACKET_NUM];
} ST_HDLC_COM_PKT;
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

void dsm_hdlc_fsm_rxbuf_reset(uint8_t HDLC_CNTX_TYPE);
void dsm_rs485if_init(uint32_t baud, bool poll);
extern void dsm_hdlc_rx_pkt_init(uint8_t HDLC_CNTX_TYPE);
extern uint8_t dsm_hdlc_rx_parser(uint8_t HDLC_CNTX_TYPE, uint8_t *buff,
                                  uint16_t size, uint16_t *idx,
                                  ST_HDLC_COM_PKT *p_com_pkt);
extern ST_HDLC_RX_PKT *dsm_hdlc_get_parser_pkt(uint8_t HDLC_CNTX_TYPE);

#endif /* __AMG_DLMS_HDLC__*/
