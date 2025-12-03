#ifdef M2354_CAN /* bccho, 2023-11-28 */
#include <stdint.h>
#include "isotp.h"
#include "amg_debug.h"

///////////////////////////////////////////////////////
///                 STATIC FUNCTIONS                ///
///////////////////////////////////////////////////////

/* st_min to microsecond */
static uint8_t isotp_ms_to_st_min(uint8_t ms)
{
    uint8_t st_min;

    st_min = ms;
    if (st_min > 0x7F)
    {
        st_min = 0x7F;
    }

    return st_min;
}

/* st_min to msec  */
static uint8_t isotp_st_min_to_ms(uint8_t st_min)
{
    uint8_t ms;

    if (st_min >= 0xF1 && st_min <= 0xF9)
    {
        ms = 1;
    }
    else if (st_min <= 0x7F)
    {
        ms = st_min;
    }
    else
    {
        ms = 0;
    }

    return ms;
}

static int isotp_send_flow_control(IsoTpLink *link, uint8_t flow_status,
                                   uint8_t block_size, uint8_t st_min_ms)
{
    IsoTpCanMessage message;
    int ret;
#if 0
    DPRINTF(DBG_INFO, "%s: send_id[0x%08X], block_size[%d], st_min_ms[%d]\r\n", __func__, link->send_arbitration_id, block_size, st_min_ms);
#endif
    MSG00("%s: send_id[0x%08X], block_size[%d], st_min_ms[%d]", __func__,
          link->send_arbitration_id, block_size, st_min_ms);
    MSG00("send_flow()");

    /* setup message  */
    message.as.flow_control.type = ISOTP_PCI_TYPE_FLOW_CONTROL_FRAME;
    message.as.flow_control.FS = flow_status;
    message.as.flow_control.BS = block_size;
    message.as.flow_control.STmin = isotp_ms_to_st_min(st_min_ms);

    /* send message : 3 Bytes */
#ifdef ISO_TP_FRAME_PADDING
#if 0
    (void) memset(message.as.flow_control.reserve, 0, sizeof(message.as.flow_control.reserve));
#else
    (void)memset(message.as.flow_control.reserve, 0xCC,
                 sizeof(message.as.flow_control.reserve));
#endif

#if 1 /* bccho, 2023-12-22 */
    uint32_t dsm_isotp_user_set_can20B_id(uint32_t mode);
    uint32_t can_id = dsm_isotp_user_set_can20B_id(DLMS_EX_MODE);
    ret =
        isotp_user_send_can(can_id, message.as.data_array.ptr, sizeof(message));
#else
    ret = isotp_user_send_can(link->send_arbitration_id,
                              message.as.data_array.ptr, sizeof(message));
#endif
#else
    ret = isotp_user_send_can(link->send_arbitration_id,
                              message.as.data_array.ptr, 3);
#endif

    return ret;
}

static int isotp_send_single_frame(IsoTpLink *link, uint32_t id)
{
    IsoTpCanMessage message;
    int ret;
    MSG00("send_single()");

#if 0
    DPRINTF(DBG_INFO, "%s: send_id[0x%08X], len[%d]\r\n", __func__, link->send_arbitration_id, link->send_size);
#endif
#if 0
    /* multi frame message length must greater than 7  */
    ASSERT(link->send_size <= 7);
#else
    if (link->send_size > 7)
    {
        return ISOTP_RET_LENGTH;
    }
#endif
    /* setup message  */
    message.as.single_frame.type = ISOTP_PCI_TYPE_SINGLE;
    message.as.single_frame.SF_DL = (uint8_t)link->send_size;
    (void)memcpy(message.as.single_frame.data, link->send_buffer,
                 link->send_size);

    /* send message : 0 ~ 7 Bytes */
#ifdef ISO_TP_FRAME_PADDING
#if 0
    (void) memset(message.as.single_frame.data + link->send_size, 0, sizeof(message.as.single_frame.data) - link->send_size);
#else
    (void)memset(message.as.single_frame.data + link->send_size, 0xCC,
                 sizeof(message.as.single_frame.data) - link->send_size);
#endif
    ret = isotp_user_send_can(id, message.as.data_array.ptr, sizeof(message));
#else
    ret =
        isotp_user_send_can(id, message.as.data_array.ptr, link->send_size + 1);
#endif

    return ret;
}

static int isotp_send_first_frame(IsoTpLink *link, uint32_t id)
{
    IsoTpCanMessage message;
    int ret;
    MSG00("send_frist()");
#if 0
    DPRINTF(DBG_INFO, "%s: send_id[0x%08X]\r\n", __func__, link->send_arbitration_id);
#endif
#if 0
    /* multi frame message length must greater than 7  */
    ASSERT(link->send_size > 7);
#else
    if (link->send_size < 8)
    {
        return ISOTP_RET_LENGTH;
    }
#endif
    /* setup message  */
    message.as.first_frame.type = ISOTP_PCI_TYPE_FIRST_FRAME;
    message.as.first_frame.FF_DL_low = (uint8_t)link->send_size;
    message.as.first_frame.FF_DL_high =
        (uint8_t)(0x0F & (link->send_size >> 8));
    (void)memcpy(message.as.first_frame.data, link->send_buffer,
                 sizeof(message.as.first_frame.data));

    /* send message : 6 Bytes */
    ret = isotp_user_send_can(id, message.as.data_array.ptr, sizeof(message));
    if (ISOTP_RET_OK == ret)
    {
        link->send_offset += sizeof(message.as.first_frame.data);
        link->send_sn = 1;
#if 0  // defined(FEATURE_CAN_DEBUG_ENABLE)
        DPRINTF(DBG_NONE, "%s: ISOTP_RET_OK sn[%d], offset[%d]\r\n", __func__, link->send_sn, link->send_offset);
#endif
    }

    return ret;
}

static int isotp_send_consecutive_frame(IsoTpLink *link)
{
    IsoTpCanMessage message;
    uint16_t data_length;
    int ret;
    MSG00("send_consecutive()");

#if 0  // defined(FEATURE_CAN_DEBUG_ENABLE)
    DPRINTF(DBG_NONE, "%s\r\n", __func__);
#endif

//    DPRINTF(DBG_INFO, "%s: SN[%d]\r\n", __func__, link->send_sn); // sequence
//    number
#if 0
    /* multi frame message length must greater than 7  */
    ASSERT(link->send_size > 7);
#else
    if (link->send_size < 8)
    {
        return ISOTP_RET_LENGTH;
    }
#endif
    /* setup message  */
    message.as.consecutive_frame.type = TSOTP_PCI_TYPE_CONSECUTIVE_FRAME;
    message.as.consecutive_frame.SN = link->send_sn;
    data_length = link->send_size - link->send_offset;
    if (data_length > sizeof(message.as.consecutive_frame.data))
    {
        data_length = sizeof(message.as.consecutive_frame.data);
    }
    (void)memcpy(message.as.consecutive_frame.data,
                 link->send_buffer + link->send_offset, data_length);

    /* send message : 0 ~ 7 Bytes */
#ifdef ISO_TP_FRAME_PADDING
#if 0
    (void) memset(message.as.consecutive_frame.data + data_length, 0, sizeof(message.as.consecutive_frame.data) - data_length);
#else
    (void)memset(message.as.consecutive_frame.data + data_length, 0xCC,
                 sizeof(message.as.consecutive_frame.data) - data_length);
#endif
    ret = isotp_user_send_can(link->send_arbitration_id,
                              message.as.data_array.ptr, sizeof(message));
#else
    ret = isotp_user_send_can(link->send_arbitration_id,
                              message.as.data_array.ptr, data_length + 1);
#endif
    if (ISOTP_RET_OK == ret)
    {
        link->send_offset += data_length;
        if (++(link->send_sn) > 0x0F)
        {
            link->send_sn = 0;
        }
    }

    return ret;
}

static int isotp_receive_single_frame(IsoTpLink *link, IsoTpCanMessage *message,
                                      uint8_t len)
{
#if 0
    DPRINTF(DBG_INFO, "%s\r\n", __func__);
#endif
    /* check data length */
#if 0
    if ((0 == message->as.single_frame.SF_DL) || (message->as.single_frame.SF_DL > (len - 1)))
#else
    // single frame data bytes is 0 ~ 7
    if (message->as.single_frame.SF_DL > (len - 1))
#endif
    {
#if 0
        DPRINTF(DBG_INFO, "Single-frame length too small.\r\n");
#endif

        return ISOTP_RET_LENGTH;
    }

    /* copying data */
    (void)memcpy(link->receive_buffer, message->as.single_frame.data,
                 message->as.single_frame.SF_DL);
    link->receive_size = message->as.single_frame.SF_DL;

    return ISOTP_RET_OK;
}

static int isotp_receive_first_frame(IsoTpLink *link, IsoTpCanMessage *message,
                                     uint8_t len)
{
    uint16_t payload_length;
#if 0
    DPRINTF(DBG_INFO, "%s\r\n", __func__);
#endif
    if (8 != len)
    {
#if 0
        DPRINTF(DBG_WARN, "First frame should be 8 bytes in length.\r\n");
#endif
        return ISOTP_RET_LENGTH;
    }

    /* check data length */
    payload_length = message->as.first_frame.FF_DL_high;
    payload_length = (payload_length << 8) + message->as.first_frame.FF_DL_low;

    //	DPRINTF(DBG_TRACE, "%s: LEN[%d]\r\n", __func__, payload_length);

    /* should not use multiple frame transmition */
    if (payload_length <= 7)
    {
#if 1
        MSG07("Should not use multiple frame transmission");
#endif
        return ISOTP_RET_LENGTH;
    }

    if (payload_length > link->receive_buf_size)
    {
#if 1
        MSG07("Multi-frame response too large for receiving buffer");
#endif
        return ISOTP_RET_OVERFLOW;
    }

    /* copying data */
    (void)memcpy(link->receive_buffer, message->as.first_frame.data,
                 sizeof(message->as.first_frame.data));
    link->receive_size = payload_length;
    link->receive_offset = sizeof(message->as.first_frame.data);
    link->receive_sn = 1;

    return ISOTP_RET_OK;
}

static int isotp_receive_consecutive_frame(IsoTpLink *link,
                                           IsoTpCanMessage *message,
                                           uint8_t len)
{
    uint16_t remaining_bytes;

#if 0  // defined(FEATURE_CAN_DEBUG_ENABLE)
	//DPRINTF(DBG_NONE, "%s\n", __func__);
    DPRINTF(DBG_NONE, "%s: SN[%d]\r\n", __func__, link->receive_sn);
#endif
    /* check sn */
    if (link->receive_sn != message->as.consecutive_frame.SN)
    {
        return ISOTP_RET_WRONG_SN;
    }

    /* check data length */
    remaining_bytes = link->receive_size - link->receive_offset;
    if (remaining_bytes > sizeof(message->as.consecutive_frame.data))
    {
        remaining_bytes = sizeof(message->as.consecutive_frame.data);
    }
    if (remaining_bytes > len - 1)
    {
#if 0
        DPRINTF(DBG_INFO, "Consecutive frame too short.\r\n");
#endif
        return ISOTP_RET_LENGTH;
    }

    /* copying data */
    (void)memcpy(link->receive_buffer + link->receive_offset,
                 message->as.consecutive_frame.data, remaining_bytes);

    link->receive_offset += remaining_bytes;
    if (++(link->receive_sn) > 0x0F)
    {
        link->receive_sn = 0;
    }

    return ISOTP_RET_OK;
}

static int isotp_receive_flow_control_frame(IsoTpLink *link,
                                            IsoTpCanMessage *message,
                                            uint8_t len)
{
#if 0
    DPRINTF(DBG_INFO, "%s: type[0x%X], FS[0x%X], BS[0x%02X], STmin[0x%02X]\r\n", __func__, message->as.flow_control.type, message->as.flow_control.FS,\
        message->as.flow_control.BS, message->as.flow_control.STmin);
#endif
    /* check message length */
    if (len < 3)
    {
#if 0
        DPRINTF(DBG_INFO, "Flow control frame too short.\r\n");
#endif
        return ISOTP_RET_LENGTH;
    }

    return ISOTP_RET_OK;
}

///////////////////////////////////////////////////////
///                 PUBLIC FUNCTIONS                ///
///////////////////////////////////////////////////////

int isotp_send(IsoTpLink *link, const uint8_t payload[], uint16_t size)
{
    return isotp_send_with_id(link, link->send_arbitration_id, payload, size);
}

int isotp_send_with_id(IsoTpLink *link, uint32_t id, const uint8_t payload[],
                       uint16_t size)
{
    int ret;

    // DPRINTF(DBG_TRACE, "%s: tx_len[%d]\r\n", __func__, size);
    if (link == 0x0)
    {
        DPRINTF(DBG_INFO, "Link is null!\r\n");
        return ISOTP_RET_ERROR;
    }

    if (size > link->send_buf_size)
    {
        DPRINTF(DBG_INFO,
                "Message size too large. Increase ISO_TP_MAX_MESSAGE_SIZE to "
                "set a larger buffer\r\n");
#if defined(FEATURE_COMPILE_DM)
        char message[128];
        sprintf(&message[0], "Attempted to send %d bytes; max size is %d!\r\n",
                size, link->send_buf_size);
#endif
        return ISOTP_RET_OVERFLOW;
    }

    if (ISOTP_SEND_STATUS_INPROGRESS == link->send_status)
    {
        DPRINTF(DBG_INFO,
                "Abort previous message, transmission in progress.\r\n");
        return ISOTP_RET_INPROGRESS;
    }

    /* copy into local buffer */
    link->send_size = size;
    link->send_offset = 0;
    (void)memcpy(link->send_buffer, payload, size);

    if (link->send_size < 8)
    {
        /* send single frame */
        ret = isotp_send_single_frame(link, id);
    }
    else
    {
        /* send multi-frame */
        ret = isotp_send_first_frame(link, id);

        /* init multi-frame control flags */
        if (ISOTP_RET_OK == ret)
        {
            link->send_bs_remain = 0;
            // link->send_bs_remain = 10;
            link->send_st_min = 0;
            link->send_wtf_count = 0;
            link->send_timer_st = isotp_user_get_ms();
            link->send_timer_bs =
                isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;
            link->send_protocol_result = ISOTP_PROTOCOL_RESULT_OK;
            link->send_status = ISOTP_SEND_STATUS_INPROGRESS;

            // DPRINTF(DBG_TRACE, "%s: ISOTP_SEND_STATUS_INPROGRESS
            // send_timer_st[%d], send_timer_bs[%d]\r\n", __func__,
            // link->send_timer_st, link->send_timer_bs);
        }
    }

    return ret;
}

void isotp_on_can_message(IsoTpLink *link, uint8_t *data, uint8_t len)
{
    IsoTpCanMessage message;
    int ret;

    if (len < 2 || len > 8)
    {
        return;
    }

    memcpy(message.as.data_array.ptr, data, len);
    memset(message.as.data_array.ptr + len, 0,
           sizeof(message.as.data_array.ptr) - len);

    switch (message.as.common.type)
    {
    case ISOTP_PCI_TYPE_SINGLE:
    {
        MSG00("on_can, SINGLE");
        /* update protocol result */
        if (ISOTP_RECEIVE_STATUS_INPROGRESS == link->receive_status)
        {
            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_UNEXP_PDU;
        }
        else
        {
            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_OK;
        }

        /* handle message */
        ret = isotp_receive_single_frame(link, &message, len);

        if (ISOTP_RET_OK == ret)
        {
            /* change status */
            link->receive_status = ISOTP_RECEIVE_STATUS_FULL;
        }
        break;
    }
    case ISOTP_PCI_TYPE_FIRST_FRAME:
    {
        MSG00("on_can, FIRST");

        /* update protocol result */
        if (ISOTP_RECEIVE_STATUS_INPROGRESS == link->receive_status)
        {
            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_UNEXP_PDU;
        }
        else
        {
            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_OK;
        }

        /* handle message */
        ret = isotp_receive_first_frame(link, &message, len);

        /* if overflow happened */
        if (ISOTP_RET_OVERFLOW == ret)
        {
            MSG00("__OVERFLOW");
            /* update protocol result */
            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_BUFFER_OVFLW;
            /* change status */
            link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;
            /* send error message */
            isotp_send_flow_control(link, PCI_FLOW_STATUS_OVERFLOW, 0, 0);
            break;
        }

        /* if receive successful */
        if (ISOTP_RET_OK == ret)
        {
            MSG00("__RET_OK");

            /* change status */
            link->receive_status = ISOTP_RECEIVE_STATUS_INPROGRESS;
            /* send fc frame */
            link->receive_bs_count = ISO_TP_DEFAULT_BLOCK_SIZE;
            isotp_send_flow_control(link, PCI_FLOW_STATUS_CONTINUE,
                                    link->receive_bs_count,
                                    ISO_TP_DEFAULT_ST_MIN);
            /* refresh timer cs */
            link->receive_timer_cr =
                isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;
        }

        break;
    }
    case TSOTP_PCI_TYPE_CONSECUTIVE_FRAME:
    {
        MSG00("on_can, CONSECUTIVE");

        /* check if in receiving status */
        if (ISOTP_RECEIVE_STATUS_INPROGRESS != link->receive_status)
        {
            MSG07("__Not INPROGRESS");
            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_UNEXP_PDU;
            break;
        }

        /* handle message */
        ret = isotp_receive_consecutive_frame(link, &message, len);

        /* if wrong sn */
        if (ISOTP_RET_WRONG_SN == ret)
        {
            MSG00("__WRONG_SN, %d %d", link->receive_sn,
                  message.as.consecutive_frame.SN);

            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_WRONG_SN;
            link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;
            break;
        }
        /* if success */
        else if (ISOTP_RET_OK == ret)
        {
            MSG00("__RET_OK");

            /* refresh timer cs */
            link->receive_timer_cr =
                isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;

            /* receive finished */
            if (link->receive_offset >= link->receive_size)
            {
                MSG00("__STATUS_FULL");
                link->receive_status = ISOTP_RECEIVE_STATUS_FULL;
            }
            else
            {
                /* send fc when bs reaches limit */
                if (0 == --link->receive_bs_count)
                {
                    MSG00("__???");
                    link->receive_bs_count = ISO_TP_DEFAULT_BLOCK_SIZE;
                    isotp_send_flow_control(link, PCI_FLOW_STATUS_CONTINUE,
                                            link->receive_bs_count,
                                            ISO_TP_DEFAULT_ST_MIN);
                }
            }
        }

        break;
    }
    case ISOTP_PCI_TYPE_FLOW_CONTROL_FRAME:
        MSG00("on_can, FLOW_CONTROL");

        /* handle fc frame only when sending in progress  */
        if (ISOTP_SEND_STATUS_INPROGRESS != link->send_status)
        {
            MSG00("__Not INPROGRESS");
            break;
        }

        /* handle message */
        ret = isotp_receive_flow_control_frame(link, &message, len);
        if (ISOTP_RET_OK == ret)
        {
            /* refresh bs timer */
            link->send_timer_bs =
                isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;

            /* overflow */
            if (PCI_FLOW_STATUS_OVERFLOW == message.as.flow_control.FS)
            {
                link->send_protocol_result = ISOTP_PROTOCOL_RESULT_BUFFER_OVFLW;
                link->send_status = ISOTP_SEND_STATUS_ERROR;
            }
            /* wait */
            else if (PCI_FLOW_STATUS_WAIT == message.as.flow_control.FS)
            {
                link->send_wtf_count += 1;
                /* wait exceed allowed count */
                if (link->send_wtf_count > ISO_TP_MAX_WFT_NUMBER)
                {
                    link->send_protocol_result = ISOTP_PROTOCOL_RESULT_WFT_OVRN;
                    link->send_status = ISOTP_SEND_STATUS_ERROR;
                }
            }
            /* permit send */
            else if (PCI_FLOW_STATUS_CONTINUE == message.as.flow_control.FS)
            {
                if (0 == message.as.flow_control.BS)
                {
                    link->send_bs_remain = ISOTP_INVALID_BS;
                }
                else
                {
                    link->send_bs_remain = message.as.flow_control.BS;
                }
                link->send_st_min =
                    isotp_st_min_to_ms(message.as.flow_control.STmin);
                link->send_wtf_count = 0;
            }
        }
        break;
    default:
        break;
    };

    return;
}

int isotp_receive(IsoTpLink *link, uint8_t *payload,
                  const uint16_t payload_size, uint16_t *out_size)
{
    uint16_t copylen;

    if (ISOTP_RECEIVE_STATUS_FULL != link->receive_status)
    {
        return ISOTP_RET_NO_DATA;
    }

    copylen = link->receive_size;
    if (copylen > payload_size)
    {
        copylen = payload_size;
    }

    memcpy(payload, link->receive_buffer, copylen);
    *out_size = copylen;

    link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;

    return ISOTP_RET_OK;
}

void isotp_init_link(IsoTpLink *link, uint32_t sendid, uint8_t *sendbuf,
                     uint16_t sendbufsize, uint8_t *recvbuf,
                     uint16_t recvbufsize)
{
    memset(link, 0, sizeof(*link));
    link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;
    link->send_status = ISOTP_SEND_STATUS_IDLE;
    link->send_arbitration_id = sendid;
    link->send_buffer = sendbuf;
    link->send_buf_size = sendbufsize;
    link->receive_buffer = recvbuf;
    link->receive_buf_size = recvbufsize;

    return;
}

void isotp_poll(IsoTpLink *link)
{
    int ret;

    /* only polling when operation in progress */
    if (ISOTP_SEND_STATUS_INPROGRESS == link->send_status)
    {
#if defined(FEATURE_CAN_DEBUG_ENABLE)
        DPRINTF(DBG_TRACE, "%s: send_bs_remain %d, send_st_min %d\r\n",
                __func__, link->send_bs_remain, link->send_st_min);
#endif
        /* continue send data */
        if (/* send data if bs_remain is invalid or bs_remain large than zero */
            (ISOTP_INVALID_BS == link->send_bs_remain ||
             link->send_bs_remain > 0) &&
            /* and if st_min is zero or go beyond interval time */
            (0 == link->send_st_min ||
             (0 != link->send_st_min &&
              IsoTpTimeAfter(isotp_user_get_ms(), link->send_timer_st))))
        {
            ret = isotp_send_consecutive_frame(link);
            if (ISOTP_RET_OK == ret)
            {
                if (ISOTP_INVALID_BS != link->send_bs_remain)
                {
                    link->send_bs_remain -= 1;
                }
                link->send_timer_bs =
                    isotp_user_get_ms() + ISO_TP_DEFAULT_RESPONSE_TIMEOUT;
                link->send_timer_st = isotp_user_get_ms() + link->send_st_min;

                /* check if send finish */
                if (link->send_offset >= link->send_size)
                {
                    link->send_status = ISOTP_SEND_STATUS_IDLE;
                }
            }
            else
            {
                link->send_status = ISOTP_SEND_STATUS_ERROR;
            }
        }

        /* check timeout */
        if (IsoTpTimeAfter(isotp_user_get_ms(), link->send_timer_bs))
        {
            link->send_protocol_result = ISOTP_PROTOCOL_RESULT_TIMEOUT_BS;
            link->send_status = ISOTP_SEND_STATUS_ERROR;
        }
    }

    /* only polling when operation in progress */
    if (ISOTP_RECEIVE_STATUS_INPROGRESS == link->receive_status)
    {
        /* check timeout */
        if (IsoTpTimeAfter(isotp_user_get_ms(), link->receive_timer_cr))
        {
            link->receive_protocol_result = ISOTP_PROTOCOL_RESULT_TIMEOUT_CR;
            link->receive_status = ISOTP_RECEIVE_STATUS_IDLE;
        }
    }

    return;
}
#endif