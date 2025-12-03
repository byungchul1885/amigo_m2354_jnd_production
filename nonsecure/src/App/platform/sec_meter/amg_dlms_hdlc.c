/*
******************************************************************************
*   INCLUDE
******************************************************************************
*/
#include "amg_uart.h"
#include "amg_crc.h"
#include "amg_dlms_hdlc.h"
#include "amg_gpio.h"
#include "dlms_todo.h"

/*
******************************************************************************
*    Definition
******************************************************************************
*/
#define _D "[HDLC] "

#define SEGMENT 1
#define NO_SEGMENT 0
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

/*
******************************************************************************
*    GLOBAL VARIABLE
******************************************************************************
*/
uint8_t prod_frame = 0;
ST_HDLC_RX_PKT gst_hdlc_rx_pkt[HDLC_PKT_CNTX_NUM];

/*
******************************************************************************
*   LOCAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
extern void dsm_485if_uart_rx_int_cb(void);

/*
******************************************************************************
*    LOCAL FUNCTIONS
******************************************************************************
*/
void dsm_rs485if_init(uint32_t baud, bool poll)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    dsm_uart_close(RS485_PORT);
    dsm_uart_init(RS485_PORT, baud, poll, NULL, 2048, NULL, 2048, FALSE);

    dsm_gpio_485_en_port_init();
    dsm_gpio_485_sel_port_init();
    dsm_485_on_zcd_off();
    dsm_uart_reg_rx_callback(RS485_PORT, dsm_485if_uart_rx_int_cb);

    dsm_hdlc_rx_pkt_init(HDLC_PKT_CNTX_485);

    dsm_uart_reg_rs485_tx_enable_callback(RS485_PORT,
                                          dsm_uart_485_txen_control);
    dsm_uart_set_rs485_tx_enable_delay_time(RS485_PORT, 0, 0);
}

void dsm_hdlc_rx_pkt_init(uint8_t HDLC_CNTX_TYPE)
{
    memset(&gst_hdlc_rx_pkt[HDLC_CNTX_TYPE], 0x00, sizeof(ST_HDLC_RX_PKT));
}

ST_HDLC_RX_PKT* dsm_hdlc_get_parser_pkt(uint8_t HDLC_CNTX_TYPE)
{
    return &gst_hdlc_rx_pkt[HDLC_CNTX_TYPE];
}

void dsm_hdlc_parser_FSM_transition(uint8_t HDLC_CNTX_TYPE, uint8_t state)
{
    dsm_hdlc_get_parser_pkt(HDLC_CNTX_TYPE)->fsm = state;
}

uint8_t dsm_hdlc_get_parser_FSM(uint8_t HDLC_CNTX_TYPE)
{
    return dsm_hdlc_get_parser_pkt(HDLC_CNTX_TYPE)->fsm;
}

void dsm_hdlc_fsm_rxbuf_reset(uint8_t HDLC_CNTX_TYPE)
{
    ST_HDLC_RX_PKT* hdlc_pkt = dsm_hdlc_get_parser_pkt(HDLC_CNTX_TYPE);

    dsm_hdlc_parser_FSM_transition(HDLC_CNTX_TYPE, HDLC_PASER_FSM_START);
    hdlc_pkt->cnt = 0;
    hdlc_pkt->len = 0;
    prod_frame = 0;
}

uint8_t dsm_hdlc_rx_parser(uint8_t HDLC_CNTX_TYPE, uint8_t* buff, uint16_t size,
                           uint16_t* idx, ST_HDLC_COM_PKT* p_com_pkt)
{
    uint32_t i;
    uint8_t result = HDLC_FRAME_RCV_CONTINUE_TMP;  // HDLC_FRAME_NO_ERR_TMP;
    ST_HDLC_RX_PKT* hdlc_pkt = dsm_hdlc_get_parser_pkt(HDLC_CNTX_TYPE);

    for (i = 0; i < size; i++)
    {
        hdlc_pkt->pkt[hdlc_pkt->cnt++] = buff[i];
        switch (hdlc_pkt->fsm)
        {
        case HDLC_PASER_FSM_START:
        {
            if (buff[i] == HDLC_FLAG_TMP)
            {
                hdlc_pkt->fsm = HDLC_PASER_FSM_FORMAT;
            }
            else
            {
                dsm_hdlc_fsm_rxbuf_reset(HDLC_CNTX_TYPE);
                result = HDLC_FRAME_HEAD_FLAG_ERR_TMP;
            }
        }
        break;

        case HDLC_PASER_FSM_FORMAT:
        {
            if ((buff[i] & 0xF0) == 0xA0)
            {
                hdlc_pkt->fsm = HDLC_PASER_FSM_LEN;

                hdlc_pkt->len = (buff[i] & 0x07) << 8;
                if (buff[i] & 0x08)
                {
                    hdlc_pkt->seg = 1;
                }
                else
                {
                    hdlc_pkt->seg = 0;
                }
            }
#if 0            
            else if (buff[i] == 0xbc)
            {
                hdlc_pkt->fsm = HDLC_PASER_FSM_LEN;
                prod_frame = 1;
                hdlc_pkt->len = 0;
                hdlc_pkt->seg = 0;
            }
#endif
            else
            {
                dsm_hdlc_fsm_rxbuf_reset(HDLC_CNTX_TYPE);

                result = HDLC_FRAME_TYPE_ERROR_TMP;
            }
        }
        break;

        case HDLC_PASER_FSM_LEN:
        {
#if 0            
            if (prod_frame)
            {
                hdlc_pkt->len = buff[i] + 2;  // Length + Start/End Flag 2 Bytes
            }
            else
#endif
            {
                hdlc_pkt->len = (hdlc_pkt->len | buff[i]) +
                                2;  // Length + Start/End Flag 2 Bytes
            }

#if 0            
            if (prod_frame)
            {
                hdlc_pkt->fsm = HDLC_PASER_FSM_DATA;
            }
            else if ((hdlc_pkt->len < 8) ||
                     (hdlc_pkt->len > HDLC_MAX_PACKET_NUM))
#else
            if ((hdlc_pkt->len < 8) || (hdlc_pkt->len > HDLC_MAX_PACKET_NUM))
#endif
            {
                dsm_hdlc_fsm_rxbuf_reset(HDLC_CNTX_TYPE);
                result = HDLC_FRAME_LENGTH_ERROR_TMP;
            }
            else
            {
                hdlc_pkt->fsm = HDLC_PASER_FSM_DATA;
            }
        }
        break;

        case HDLC_PASER_FSM_DATA:
        {
            if (hdlc_pkt->cnt >= (hdlc_pkt->len))
            {
                /*3. HDLC end flag check */
                if (buff[i] == HDLC_FLAG_TMP)
                {
                    p_com_pkt->len = hdlc_pkt->cnt;
                    memcpy(&p_com_pkt->data[0], &hdlc_pkt->pkt[0],
                           hdlc_pkt->cnt);

                    dsm_hdlc_fsm_rxbuf_reset(HDLC_CNTX_TYPE);
                    result = HDLC_FRAME_NO_ERR_TMP;
                }
                else
                {
                    dsm_hdlc_fsm_rxbuf_reset(HDLC_CNTX_TYPE);

                    result = HDLC_FRAME_TAIL_FLAG_ERR_TMP;
                }
            }
        }
        break;
        }

        if (result != HDLC_FRAME_RCV_CONTINUE_TMP)
        {
            i++;
            break;
        }
    }

    (*idx) += i;

    return result;  // HDLC_FRAME_NO_ERR_TMP;
}
