/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "options_sel.h"
#include "main.h"
#include "amg_task.h"
#include "os_task_q.h"
#include "amg_uart.h"
#include "amg_mif_prtl.h"
#include "amg_crc.h"
#include "platform.h"
#if 0 /* bccho, 2023-07-20 */
#include "stm32l4xx.h"
#include "stm32l4xx_hal_gpio.h"
#endif /* bccho */
#include "amg_gpio.h"
#include "amg_meter_main.h"

#ifdef MTP_ZCD_ON_OFF
#include "amg_mtp_process.h"
#endif
/*
******************************************************************************
*   Definition
******************************************************************************
*/
#define _D "[MIF] "

/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/

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

#ifdef MTP_ZCD_ON_OFF
extern U8 g_zcd_on_ing_sts;
#endif

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/

ST_MIF_RX_PKT gst_mif_rx_pkt;
ST_MIF_FRAME_BUFFER g_mif_tx_buf;

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
*   FUNCTIONS - extern
******************************************************************************
*/
extern void dsm_mif_rx_int_cb(void);

void dsm_mif_init(void)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    dsm_gpio_meter_ic_rst_port_init();

    dsm_uart_close(MIF_PORT);
    dsm_uart_init(MIF_PORT, 115200, FALSE, NULL, /*2048*/ 1024, NULL,
                  /*256*/ 1024, FALSE);

    dsm_uart_reg_rx_callback(MIF_PORT, dsm_mif_rx_int_cb);

    dsm_mif_rx_pkt_init();
}

void dsm_mif_rx_pkt_init(void)
{
    memset(&gst_mif_rx_pkt, 0x00, sizeof(ST_MIF_RX_PKT));
}

ST_MIF_RX_PKT* dsm_mif_get_parser_pkt(void) { return &gst_mif_rx_pkt; }

void dsm_mif_fsm_rxbuf_reset(ST_MIF_RX_PKT* p_mif_rx_pkt)
{
    p_mif_rx_pkt->fsm = MIF_FSM_START;
    p_mif_rx_pkt->len = 0;
    p_mif_rx_pkt->cnt = 0;
}

ST_MIF_FRAME_BUFFER* dsm_mif_get_tx_buffer(void) { return &g_mif_tx_buf; }

char* dsm_mif_cmd_string(uint32_t cmd)
{
    switch (cmd)
    {
    case MIF_GetReq_CalData:
        return "GetReq_CalData";
    case MIF_GetRsp_CalData:
        return "GetRsp_CalData";
    case MIF_SetReq_CalData:
        return "SetReq_CalData";
    case MIF_SetRsp_CalData:
        return "SetRsp_CalData";
    case MIF_ActReq_CalStart:
        return "ActReq_CalStart";
    case MIF_ActRsp_CalStart:
        return "ActRsp_CalStart";
    case MIF_Push_MeterData:
        return "Push_MeterData";
    case MIF_GetReq_MeterData:
        return "GetReq_MeterData";
    case MIF_GetRsp_MeterData:
        return "GetRsp_MeterData";
    case MIF_SetReq_SagSwell:
        return "SetReq_SagSwell";
    case MIF_SetRsp_SagSwell:
        return "SetRsp_SagSwell";
    case MIF_GetReq_SagSwell_Parm:
        return "GetReq_SagSwellData";
    case MIF_GetRsp_SagSwell_Parm:
        return "GetRsp_SagSwellData";
    case MIF_ActReq_EoiPulse:
        return "ActReq_EoiPulse";
    case MIF_GetReq_FirmVer:
        return "GetReq_FirmVer";
    case MIF_GetRsp_FirmVer:
        return "GetRsp_FirmVer";
    case MIF_ActReq_SagSwellSpTime:
        return "ActReq_SagSwellSpTime";
    case MIF_ActRsp_SagSwellSpTime:
        return "ActRsp_SagSwellSpTime";
    case MIF_GetReq_MtSetupParm:
        return "GetReq_MtSetupParm";
    case MIF_GetRsp_MtSetupDataParm:
        return "GetRsp_MtSetupDataParm";
    case MIF_SetReq_MtSetupDataParm:
        return "SetReq_MtSetupDataParm";
    case MIF_SetARsp_MtSetupDataParm:
        return "SetARsp_MtSetupDataParm";
    case MIF_SetReq_BaudRate:
        return "SetReq_BaudRate";
    case MIF_SetRsp_BaudRate:
        return "SetRsp_BaudRate";
    case MIF_SetAck:
        return "SetAck";
    case MIF_SetNack:
        return "SetNack";
    case MIF_ActAck:
        return "ActAck";
    case MIF_ActNack:
        return "ActNack";
    case MIF_PushAck:
        return "PushAck";
    case MIF_PushNack:
        return "PushNack";
    case MIF_SetReq_FwData:
        return "SetReq_Fw_Data";
    case MIF_GetReq_FwData:
        return "GetReq_Fw_Data";
    case MIF_SetRsp_FwData:
        return "SetRsp_Fw_Data";
    case MIF_GetRsp_FwData:
        return "GetRsp_Fw_Data";
    default:
        return "Unknown";
    }
}

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/
/*
    MIF command/response 처리
    SYSP 는 Set/Act Request 의 경우 state 관리
*/
void dsm_mif_process(EN_MIF_TX_TYPE type, ST_MIF_COM_PKT* p_com_pkt)
{
    uint8_t rsp = p_com_pkt->data[MIF_CMD_POS];

    DPRINTF(DBG_INFO, "%s: RSP[0x%02X]= %s\r\n", __func__, rsp,
            dsm_mif_cmd_string(rsp));

    if (type == MIF_TX_CMD)
    {
        switch (rsp)
        {
        case MIF_GetReq_CalData:

            break;
        case MIF_SetReq_CalData:

            break;
        case MIF_ActReq_CalStart:

            break;
        case MIF_ActReq_EoiPulse:

            break;
        case MIF_GetReq_MeterData:

            break;
        case MIF_SetReq_SagSwell:

            break;
        case MIF_ActReq_SagSwellSpTime:

            break;
        case MIF_GetReq_MtSetupParm:

            break;
        case MIF_SetReq_MtSetupDataParm:

            break;
        case MIF_SetReq_BaudRate:

            break;
        case MIF_SetAck:

            break;
        case MIF_SetNack:

            break;
        case MIF_ActAck:

            break;
        case MIF_ActNack:

            break;
        default:
            break;
        }
    }
    else
    {
        switch (rsp)
        {
        case MIF_GetRsp_CalData:

            break;
        case MIF_SetRsp_CalData:

            break;
        case MIF_ActRsp_CalStart:

            break;
        case MIF_Push_MeterData:
            dsm_mif_push_ack();

            break;
        case MIF_GetRsp_MeterData:

            break;
        case MIF_SetRsp_SagSwell:

            break;
        case MIF_ActRsp_SagSwellSpTime:

            break;
        case MIF_GetRsp_MtSetupDataParm:

            break;
        case MIF_SetARsp_MtSetupDataParm:

            break;
        case MIF_SetRsp_BaudRate:

            break;
        case MIF_SetAck:

            break;
        case MIF_SetNack:

            break;
        case MIF_ActAck:

            break;
        case MIF_ActNack:

            break;
        default:
            break;
        }
    }
}

uint32_t dsm_mif_rx_parser(uint8_t* buff, uint32_t size, uint32_t* idx,
                           ST_MIF_COM_PKT* p_com_pkt)
{
    uint32_t i;
    uint16_t cal_crc = 0, rx_crc = 0;
    uint32_t result;           // = MIF__FRAME_NO_ERR;
    ST_MIF_RX_PKT* p_mif_pkt;  // ST(1) LEN(2) CMD(1) CRC(2) SP(1)

    result = MIF__FRAME_RCV_CONTINUE;
    p_mif_pkt = dsm_mif_get_parser_pkt();

    for (i = 0; i < size; i++)
    {
        p_mif_pkt->pkt[p_mif_pkt->cnt++] = buff[i];

        switch (p_mif_pkt->fsm)
        {
        case MIF_FSM_START:
        {
            if (buff[i] == MIF_ST)
            {
                p_mif_pkt->fsm = MIF_FSM_LEN_1;
            }
            else
            {
                dsm_mif_fsm_rxbuf_reset(p_mif_pkt);
                result = MIF__FRAME_HEAD_FLAG_ERR;
            }
        }
        break;

        case MIF_FSM_LEN_1:
        {
            p_mif_pkt->len = buff[i] << 8;
            p_mif_pkt->fsm = MIF_FSM_LEN_2;
        }
        break;

        case MIF_FSM_LEN_2:
        {
            p_mif_pkt->len |= buff[i];
            p_mif_pkt->len += 3; /* total length = ST(1) + LEN_FIELD_SIZE(2) +
                                    LEN field value */

            if ((p_mif_pkt->len < MIF_MIN_PACKET_NUM) ||
                (p_mif_pkt->len > MIF_MAX_PACKET_NUM))
            {
                dsm_mif_fsm_rxbuf_reset(p_mif_pkt);
                result = MIF__FRAME_LENGTH_ERROR;
            }
            else
            {
                p_mif_pkt->fsm = MIF_FSM_DATA;
            }
        }
        break;

        case MIF_FSM_DATA:
        {
            if (p_mif_pkt->cnt >= (p_mif_pkt->len))
            {
                if (buff[i] == MIF_SP)
                {
                    rx_crc =
                        (uint16_t)((p_mif_pkt->pkt[p_mif_pkt->cnt - 2] << 8) |
                                   p_mif_pkt->pkt[p_mif_pkt->cnt - 3]);
                    cal_crc = crc16_get((uint8_t*)&p_mif_pkt->pkt[1],
                                        p_mif_pkt->cnt - 4);
                    if (rx_crc != cal_crc)
                    {
                        DPRINTF(DBG_ERR,
                                _D
                                "CRC_ERR: RX_CRC[0x%04X], CAL_CRC[0x%04X], "
                                "LEN[%d]\r\n",
                                rx_crc, cal_crc, p_mif_pkt->cnt);
                        dsm_mif_fsm_rxbuf_reset(p_mif_pkt);
                        result = MIF__FRAME_CRC_ERR;
                    }
                    else
                    {
                        p_com_pkt->len = p_mif_pkt->cnt;
                        memcpy(&p_com_pkt->data[0], &p_mif_pkt->pkt[0],
                               p_mif_pkt->cnt);

                        if (p_mif_pkt->cnt > 256)
                        {
                            DPRINT_HEX(/*DBG_NONE*/ DBG_INFO, "MIF_RX",
                                       &p_mif_pkt->pkt[0], 256, DUMP_MIF);
                        }
                        else
                        {
                            DPRINT_HEX(/*DBG_NONE*/ DBG_INFO, "MIF_RX",
                                       &p_mif_pkt->pkt[0], p_mif_pkt->cnt,
                                       DUMP_MIF);
                        }

                        dsm_mif_fsm_rxbuf_reset(p_mif_pkt);
                        result = MIF__FRAME_NO_ERR;
                    }
                }
                else
                {
                    dsm_mif_fsm_rxbuf_reset(p_mif_pkt);
                    result = MIF__FRAME_TAIL_FLAG_ERR;
                }
            }
        }
        break;
        }

        if (result != MIF__FRAME_RCV_CONTINUE)
        {
            i++;
            break;
        }
    }

    (*idx) += i;
    return result;  // MIF__FRAME_NO_ERR;
}

uint32_t dsm_mif_tx(EN_MIF_TX_TYPE type, uint8_t cmd_data_t, uint8_t* pdata,
                    uint16_t length)
{
    uint16_t crc;
    uint16_t len_field_size = length + MIF_LEN_ADD_SIZE;
    uint16_t crc_size = MIF_LEN_FIELD_SIZE + MIF_CMD_FIELD_SIZE + length;

    memset(&g_mif_tx_buf, 0x00, sizeof(ST_MIF_FRAME_BUFFER));
    g_mif_tx_buf.length =
        MIF_ST_FIELD_SIZE + MIF_LEN_FIELD_SIZE + MIF_LEN_ADD_SIZE + length;

    g_mif_tx_buf.buff[g_mif_tx_buf.idx++] = MIF_ST;
    g_mif_tx_buf.buff[g_mif_tx_buf.idx++] = (len_field_size << 8);
    g_mif_tx_buf.buff[g_mif_tx_buf.idx++] |= (len_field_size);

    g_mif_tx_buf.buff[g_mif_tx_buf.idx++] = cmd_data_t;
    memcpy(&g_mif_tx_buf.buff[g_mif_tx_buf.idx], pdata, length);
    g_mif_tx_buf.idx += length;

    crc = crc16_get(&g_mif_tx_buf.buff[1], crc_size);
    g_mif_tx_buf.buff[g_mif_tx_buf.idx++] = (crc >> 0) & 0xFF;
    g_mif_tx_buf.buff[g_mif_tx_buf.idx++] = (crc >> 8) & 0xFF;

    g_mif_tx_buf.buff[g_mif_tx_buf.idx++] = MIF_SP;

    if (type == MIF_TX_CMD)
    {
        DPRINT_HEX(DBG_INFO, "MIF_TX", g_mif_tx_buf.buff, g_mif_tx_buf.idx,
                   DUMP_MIF);

        if (cmd_data_t != MIF_CMD_PUSH_ACK)
        {
            dsm_meter_sw_timer_start(MT_SW_TIMER_MIF_MTP_TX_TO, TRUE,
                                     MT_TIMEOUT_MIF_MTP_TX_TIME);
        }
        dsm_uart_send(MIF_PORT, (char*)g_mif_tx_buf.buff, g_mif_tx_buf.idx);
    }

    return TRUE;
}

void dsm_mif_getreq_cal_data(void)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_GET_REQ | MIF_BODY_CAL_DATA), NULL, 0);
}

void dsm_mif_getrsp_cal_data(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_GET_RSP | MIF_BODY_CAL_DATA), pdata,
               length);
}

void dsm_mif_setreq_cal_data(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_SET_REQ | MIF_BODY_CAL_DATA), pdata,
               length);
}

void dsm_mif_setrsp_cal_data(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_SET_RSP | MIF_BODY_CAL_DATA), pdata,
               length);
}

void dsm_mif_actreq_eoipulse(void)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_ACT_REQ | MIF_BODY_EOI_PULSE), NULL, 0);
}

#ifdef MTP_ZCD_ON_OFF
void dsm_mif_actreq_zcdpulse(void)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_ACT_REQ | MIF_BODY_ZCD_PULSE), NULL, 0);
}

void dsm_mif_actreq_zcdoff(void)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_ACT_REQ | MIF_BODY_ZCD_OFF), NULL, 0);
}
#endif

void dsm_mif_actreq_cal_start(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_ACT_REQ | MIF_BODY_CAL_START), pdata,
               length);
}

void dsm_mif_actrsp_cal_start(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_ACT_RSP | MIF_BODY_CAL_START), pdata,
               length);
}

void dsm_mif_push_data(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_PUSH | MIF_BODY_PUSH_MT_DATA), pdata,
               length);
}

void dsm_mif_push_ack(void)
{
    dsm_mif_tx(MIF_TX_CMD, MIF_CMD_PUSH_ACK, NULL, 0);
}

void dsm_mif_push_nack(void)
{
    dsm_mif_tx(MIF_TX_CMD, MIF_CMD_PUSH_NACK, NULL, 0);
}

void dsm_mif_set_ack(void) { dsm_mif_tx(MIF_TX_RSP, MIF_CMD_SET_ACK, NULL, 0); }

void dsm_mif_set_nack(void)
{
    dsm_mif_tx(MIF_TX_RSP, MIF_CMD_SET_NACK, NULL, 0);
}

void dsm_mif_act_ack(void) { dsm_mif_tx(MIF_TX_RSP, MIF_CMD_ACT_ACK, NULL, 0); }

void dsm_mif_act_nack(void)
{
    dsm_mif_tx(MIF_TX_RSP, MIF_CMD_ACT_NACK, NULL, 0);
}
void dsm_mif_getreq_pushdata(void)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_GET_REQ | MIF_BODY_PUSH_MT_DATA), NULL, 0);
}

void dsm_mif_getrsp_pushdata(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_GET_RSP | MIF_BODY_PUSH_MT_DATA), pdata,
               length);
}

void dsm_mif_setreq_sagswell_data(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_SET_REQ | MIF_BODY_SAGSWELL_DATA), pdata,
               length);
}

void dsm_mif_getreq_firmware_ver_data(void)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_GET_REQ | MIF_BODY_FIRMVER_DATA), NULL, 0);
}

void dsm_mif_getreq_sagswell_data(void)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_GET_REQ | MIF_BODY_SAGSWELL_DATA), NULL, 0);
}

void dsm_mif_setrsp_sagswell_data(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_SET_RSP | MIF_BODY_SAGSWELL_DATA), pdata,
               length);
}

void dsm_mif_actreq_sagswell_time(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_ACT_REQ | MIF_BODY_SAGSWELL_TIME), pdata,
               length);
}

void dsm_mif_actrsp_sagswell_time(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_ACT_RSP | MIF_BODY_SAGSWELL_TIME), pdata,
               length);
}

void dsm_mif_getreq_meter_setup_parm(void)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_GET_REQ | MIF_BODY_SETUP_PARM), NULL, 0);
}

void dsm_mif_getrsp_meter_setup_parm(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_GET_RSP | MIF_BODY_SETUP_PARM), pdata,
               length);
}

void dsm_mif_setreq_meter_setup_parm(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_SET_REQ | MIF_BODY_SETUP_PARM), pdata,
               length);
}

void dsm_mif_setrsp_meter_setup_parm(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_SET_RSP | MIF_BODY_SETUP_PARM), pdata,
               length);
}

void dsm_mif_setreq_baudrate(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_SET_REQ | MIF_BODY_BAUDRATE), pdata,
               length);
}

void dsm_mif_setrsp_baudrate(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_RSP, (MIF_CMD_SET_RSP | MIF_BODY_BAUDRATE), pdata,
               length);
}

void dsm_mif_getreq_fw_data(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_GET_REQ | MIF_BODY_FW_DATA), pdata, length);
}

void dsm_mif_setreq_fw_data(uint8_t* pdata, uint16_t length)
{
    dsm_mif_tx(MIF_TX_CMD, (MIF_CMD_SET_REQ | MIF_BODY_FW_DATA), pdata, length);
}

#ifdef MTP_ZCD_ON_OFF
void dsm_mif_zcd_on_meter_only(void)
{
    g_zcd_on_ing_sts = 1;
    dsm_mtp_set_op_mode(MTP_OP_NORMAL);
    dsm_mtp_set_fsm(MTP_FSM_ZCD_PULSE_ST);
    dsm_mtp_fsm_send();
}

void dsm_mif_zcd_off_meter_only(void)
{
    g_zcd_on_ing_sts = 0;
    dsm_mtp_set_op_mode(MTP_OP_NORMAL);
    dsm_mtp_set_fsm(MTP_FSM_ZCD_PULSE_END);
    dsm_mtp_fsm_send();
}
#endif

void dsm_mif_zcd_on(void)
{
#if 0 /* bccho, ZCD, 2023-07-15 */    
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOG);

    if (LL_PWR_IsEnabledVddIO2() == RESET)
    {
        LL_PWR_EnableVddIO2();
    }
#if 0
    GPIO_InitStruct.Pin = MCU_ZCD_PULSE_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(MCU_ZCD_PULSE_GPIO_Port, &GPIO_InitStruct);
    LL_GPIO_SetOutputPin(MCU_ZCD_PULSE_GPIO_Port, MCU_ZCD_PULSE_Pin);
#endif

    LL_GPIO_SetPinMode(ZCD_PULSE_P3SX_GPIO_Port, ZCD_PULSE_P3SX_Pin,
                       LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(ZCD_PULSE_P3SX_GPIO_Port, ZCD_PULSE_P3SX_Pin,
                       LL_GPIO_PULL_NO);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE2);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_2);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_2);
    LL_EXTI_EnableFallingTrig_0_31(LL_EXTI_LINE_2);

    NVIC_EnableIRQ(EXTI2_IRQn);
    NVIC_SetPriority(EXTI2_IRQn, 5);
#endif /* bccho */
    MSG05("dsm_mif_zcd_on()");
    dsm_zcd_on_485_off();
    dsm_gpio_tx_en_enable();

#ifdef MTP_ZCD_ON_OFF
    dsm_mif_zcd_on_meter_only();
#endif
}

void dsm_mif_zcd_off(void)
{
#if 1 /* bccho, ZCD, 2023-07-15 */
    MSG05("dsm_mif_zcd_off()");
    dsm_485_on_zcd_off();
    dsm_gpio_tx_en_disable();

#ifdef MTP_ZCD_ON_OFF
    dsm_mif_zcd_off_meter_only();
#endif
#else
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_2);
    LL_EXTI_DisableRisingTrig_0_31(LL_EXTI_LINE_2);
    LL_EXTI_DisableFallingTrig_0_31(LL_EXTI_LINE_2);
    NVIC_DisableIRQ(EXTI2_IRQn);

    dsm_485_on_zcd_off();
    dsm_gpio_tx_en_disable();
#endif /* bccho */
}

uint32_t dsm_mif_zcd_pin_state(void)
{
#if 0 /* bccho, ZCD, 2023-07-15 */
    return ((uint32_t)HAL_GPIO_ReadPin(ZCD_PULSE_P3SX_GPIO_Port,
                                       ZCD_PULSE_P3SX_Pin));
#else /* bccho */
    return 0;
#endif /* bccho */
}

void dsm_mif_pwrfail_port_init(void)
{
#if 0  /* bccho, ZCD, 2023-07-15 */
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);

    if (LL_PWR_IsEnabledVddIO2() == RESET)
    {
        LL_PWR_EnableVddIO2();
    }

    LL_GPIO_SetPinMode(POWER_FAIL_P3SX_GPIO_Port, POWER_FAIL_P3SX_Pin,
                       LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(POWER_FAIL_P3SX_GPIO_Port, POWER_FAIL_P3SX_Pin,
                       LL_GPIO_PULL_NO);
#endif /* bccho */
}

void dsm_mif_pwrfail_port_deinit(void)
{
#if 0  /* bccho, ZCD, 2023-07-15 */
    LL_EXTI_DisableIT_0_31(LL_EXTI_LINE_5);
    LL_EXTI_DisableRisingTrig_0_31(LL_EXTI_LINE_5);
    LL_EXTI_DisableFallingTrig_0_31(LL_EXTI_LINE_5);
    NVIC_DisableIRQ(EXTI9_5_IRQn);
#endif /* bccho */
}
