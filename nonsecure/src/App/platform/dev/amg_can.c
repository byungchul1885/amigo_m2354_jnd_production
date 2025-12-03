#include "amg_can.h"
#include "amg_meter_main.h"
#include "amg_debug.h"
#include "isotp.h"

#define _D "[CAN] "

#ifdef M2354_CAN /* bccho, 2023-11-28 */
static void CAN_ShowMsg(STR_CANMSG_T* Msg)
{
    uint8_t i;
    MSG07("CAN Rx ID=%8X, Type=%s, DLC=%d", Msg->Id,
          Msg->IdType ? "EXT" : "STD", Msg->DLC);
    for (i = 0; i < Msg->DLC; i++)
    {
        printf("%02X,", Msg->Data[i]);
    }
    printf("\n");
}

extern uint32_t dsm_media_is_can_power_off_push(void);
extern void dsm_can_rx2_int_cb(void);

#define MAX_CAN_Q 100
static STR_CANMSG_T rrMsg[MAX_CAN_Q];
static uint8_t wr_idx;
static uint8_t rd_idx;

static void set_wr_idx(void)
{
    wr_idx++;
    if (wr_idx == MAX_CAN_Q)
    {
        wr_idx = 0;
    }
}

static void set_rd_idx(void)
{
    rd_idx++;
    if (rd_idx == MAX_CAN_Q)
    {
        rd_idx = 0;
    }
}

static void CAN_MsgInterrupt(CAN_T* tCAN, uint32_t u32IIDR)
{
    IsoTpLink* plink = (IsoTpLink*)dsm_isotp_user_get_link();
    MSG00("CAN_MsgInterrupt");

    if (u32IIDR == 1 + 5)
    {
        MSG00("Msg-5(solicitaion) INT and Callback");
        CAN_Receive(tCAN, 5, &rrMsg[wr_idx]);
        set_wr_idx();
    }
    else if (u32IIDR == 1 + 6)
    {
        MSG00("Msg-6(dlms_ex) INT and Callback");
        CAN_Receive(tCAN, 6, &rrMsg[wr_idx]);
        set_wr_idx();
    }
    else if (u32IIDR == 1 + 7)
    {
        MSG00("Msg-7(broadcast) INT and Callback");
        CAN_Receive(tCAN, 7, &rrMsg[wr_idx]);
        set_wr_idx();
    }
    else if (u32IIDR == 1 + 8)
    {
        MSG00("Msg-8(dlms_push) INT and Callback");
        CAN_Receive(tCAN, 8, &rrMsg[wr_idx]);
        set_wr_idx();
    }
    else if (u32IIDR == 1 + 10)
    {
        MSG00("Tx");
        return;
    }

    dsm_can_rx2_int_cb();
}

void dsm_can_rx2_proc(void)
{
    MSG00("dsm_can_rx2_proc");

    while (wr_idx != rd_idx)
    {
        if (!dsm_can_is_valid_ext_id(rrMsg[rd_idx].Id))
        {
            set_rd_idx();
            continue;
        }

        IsoTpLink* plink = (IsoTpLink*)dsm_isotp_user_get_link();
        plink->receive_arbitration_id = rrMsg[rd_idx].Id;

        isotp_on_can_message(plink, rrMsg[rd_idx].Data, rrMsg[rd_idx].DLC);
        if (plink->receive_status == ISOTP_RECEIVE_STATUS_FULL)
        {
            if (dsm_media_is_can_power_off_push() == FALSE)
            {
                dsm_can_rx_int_cb();
            }
        }

        set_rd_idx();
    }
}

void CAN0_IRQHandler(void)
{
    uint32_t u8IIDRstatus = CAN0->IIDR;
    MSG00("CAN0_IRQHandler");

    /* Check Status Interrupt Flag (Error status Int and Status change Int)
     */
    if (u8IIDRstatus == 0x00008000)
    {
        /**************************/
        /* Status Change interrupt*/
        /**************************/
        if (CAN0->STATUS & CAN_STATUS_RXOK_Msk)
        {
            CAN0->STATUS &= ~CAN_STATUS_RXOK_Msk; /* Clear Rx Ok status*/
            MSG00("CAN RX OK INT");
        }

        if (CAN0->STATUS & CAN_STATUS_TXOK_Msk)
        {
            CAN0->STATUS &= ~CAN_STATUS_TXOK_Msk; /* Clear Tx Ok status*/
            MSG00("CAN TX OK INT");
        }

        /**************************/
        /* Error Status interrupt */
        /**************************/
        if (CAN0->STATUS & CAN_STATUS_EWARN_Msk)
        {
            MSG00("CAN EWARN INT");
        }

        if (CAN0->STATUS & CAN_STATUS_BOFF_Msk)
        {
            MSG00("CAN BOFF INT");
        }
    }
    else if (u8IIDRstatus != 0)
    {
        CAN_MsgInterrupt(CAN0, u8IIDRstatus);

        /* Clear Interrupt Pending */
        CAN_CLR_INT_PENDING_BIT(CAN0, (uint8_t)((CAN0->IIDR) - 1));
    }
    else if (CAN0->WU_STATUS == 1)
    {
        MSG00("CAN Wake up\n");

        CAN0->WU_STATUS = 0; /* Write '0' to clear */
    }
}

extern uint32_t g_serial_number;
void CAN_NormalMode_SetRxMsg(CAN_T* tCAN)
{
    uint32_t id_solicitaion =
        ((NODE_SOLICITATION_MODE << 26) | (CAN_ID_R0 << 25) |
         (ISO_TP_RX << 24) | g_serial_number);
    if (CAN_SetRxMsg(tCAN, MSG(5), CAN_EXT_ID, id_solicitaion) == FALSE)
    {
        MSGERROR("Set Rx Msg Object failed, id_solicitaion");
        return;
    }

    uint32_t id_dlms_ex = ((DLMS_EX_MODE << 26) | (CAN_ID_R0 << 25) |
                           (ISO_TP_RX << 24) | g_serial_number);
    if (CAN_SetRxMsg(tCAN, MSG(6), CAN_EXT_ID, id_dlms_ex) == FALSE)
    {
        MSGERROR("Set Rx Msg Object failed, dlms_ex");
        return;
    }

    if (CAN_SetRxMsg(tCAN, MSG(7), CAN_EXT_ID, 0x8FFFFFF) == FALSE)
    {
        MSGERROR("Set Rx Msg Object failed, broadcast\n");
        return;
    }

    uint32_t id_dlms_push = ((DLMS_PUSH_MODE << 26) | (CAN_ID_R0 << 25) |
                             (ISO_TP_RX << 24) | g_serial_number);
    if (CAN_SetRxMsg(tCAN, MSG(8), CAN_EXT_ID, id_dlms_push) == FALSE)
    {
        MSGERROR("Set Rx Msg Object failed, dlms_push\n");
        return;
    }

    MSG07("CAN Rx ID, solicitaion:(0x%08X), dlms_ex:(0x%08X)", id_solicitaion,
          id_dlms_ex);
}
#endif

void dsm_can_init(uint32_t baudrate_idx)
{
#ifdef M2354_CAN /* bccho, 2023-11-28 */
    MSG07("dsm_can_init()");
    CAN_Init_S();
    CAN_Open_S(500000); /* 500K */

    CAN_EnableInt(CAN0, CAN_CON_IE_Msk);
    NVIC_SetPriority(CAN0_IRQn, (1 << __NVIC_PRIO_BITS) - 2);
    NVIC_EnableIRQ(CAN0_IRQn);
#endif
}

void dsm_can_deinit(void)
{
#ifdef M2354_CAN /* bccho, 2023-11-28 */
#endif
}

void dsm_can_tx(uint32_t ext_id, uint8_t* ptx, uint8_t len)
{
#ifdef M2354_CAN /* bccho, 2023-11-28 */
    STR_CANMSG_T tMsg;
    MSG00("can_tx(), id:%08X, len:%d", ext_id, len);

    tMsg.FrameType = CAN_DATA_FRAME;
    tMsg.IdType = CAN_EXT_ID;
    tMsg.Id = ext_id;
    tMsg.DLC = len;
    memcpy(tMsg.Data, ptx, len);

    if (CAN_Transmit(CAN0, MSG(10), &tMsg) == FALSE)
    {
        MSGERROR("Set Tx Msg Object failed");
        return;
    }
#else
#endif
}
