/*
******************************************************************************
*   INCLUDE
******************************************************************************
*/
#include "amg_crc.h"
#include "amg_media_mnt.h"
#include "amg_uart.h"
#include "amg_push_datanoti.h"
#include "dl.h"
#include "appl.h"
#include "comm.h"
#include "amg_modemif_prtl.h"
#include "amg_wdt.h"
#ifdef M2354_CAN /* bccho, 2023-11-28 */
#include "amg_isotp_user.h"
#endif

/*
******************************************************************************
*    Definition
******************************************************************************
*/
#define _D "[IF_MNT] "

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
EN_RUN_MEDIA g_en_run_media_HDLC;
EN_IF_CHG_EVT g_en_if_chg_evt;
EN_IF_RX_EVT g_en_if_rx_evt;
uint32_t g_media_can_power_off_push = 0;

EN_RUN_MEDIA g_en_run_media_ATCMD;

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

/*
******************************************************************************
*    LOCAL FUNCTIONS
******************************************************************************
*/

void dsm_media_if_init(void)
{
    g_en_run_media_HDLC = MEDIA_RUN_NONE;
    g_en_if_chg_evt = NONE_IF_CHG_EVT;
    g_en_if_rx_evt = NONE_IF_RX_EVT;
    g_en_run_media_ATCMD = MEDIA_RUN_NONE;
}

void dsm_media_set_fsm_if_hdlc(uint32_t ifstate)
{
    if (g_en_run_media_HDLC != ifstate)
    {
        DPRINTF(DBG_TRACE, "media_if HDLC fsm[%s -> %s]\r\n",
                dsm_media_if_fsm_string(g_en_run_media_HDLC),
                dsm_media_if_fsm_string(ifstate));
        g_en_run_media_HDLC = ifstate;
    }
}

uint32_t dsm_media_get_fsm_if_hdlc(void) { return g_en_run_media_HDLC; }

void dsm_media_set_fsm_if_ATCMD(uint32_t ifstate)
{
    if (g_en_run_media_ATCMD != ifstate)
    {
        DPRINTF(DBG_TRACE, "media_if ATCMD fsm[%s -> %s]\r\n",
                dsm_media_if_fsm_string(g_en_run_media_ATCMD),
                dsm_media_if_fsm_string(ifstate));
        g_en_run_media_ATCMD = ifstate;
    }
}

uint32_t dsm_media_get_fsm_if_ATCMD(void) { return g_en_run_media_ATCMD; }

char* dsm_media_if_fsm_string(uint32_t fsm)
{
    switch (fsm)
    {
    case MEDIA_RUN_NONE:
        return "IF_NONE";
    case MEDIA_RUN_RS485:
        return "IF_485";
    case MEDIA_RUN_CAN:
        return "IF_CAN";
    case MEDIA_RUN_SUN:
        return "IF_SUN";
    case MEDIA_RUN_EXT:
        return "IF_EXT";

    default:
        return "IF_Unknown";
    }
}

char* dsm_media_if_protocol_string(uint32_t type)
{
    switch (type)
    {
    case MEDIA_PROTOCOL_IF_HDLC:
        return "HDLC_IF";
    case MEDIA_PROTOCOL_IF_ATCMD:
        return "ATCMD_IF";

    default:
        return "HDLC_IF";
    }
}

char* dsm_media_rx_evt_string(uint32_t rx_evt)
{
    switch (rx_evt)
    {
    case NONE_IF_RX_EVT:
        return "NONE";
    case RS485_IF_RX_EVT:
        return "RS485";
    case CAN_IF_RX_EVT:
        return "CAN";
    case SUN_IF_RX_EVT:
        return "SUN";
    case EXT_IF_RX_EVT:
        return "EXT";

    default:
        return "EVT_Unknown";
    }
}

bool dsm_media_if_dl_is_valid_asso_3_4_svraddr(uint8_t svr_u, uint8_t svr_l)
{
    bool ret = false;

    if (svr_u == SVR_ADDR_SEC_U && svr_l == SVR_ADDR_L)
    {
        ret = true;
    }
    else if (svr_u == SVR_ADDR_SEC_U && svr_l == SVR_ADDR_BROADCASTING_L)
    {
        ret = true;
    }

    return ret;
}

bool dsm_media_if_dl_is_valid_asso_1_svraddr(uint8_t svr_u, uint8_t svr_l)
{
    bool ret = false;

    if (svr_u == SVR_ADDR_U && svr_l == SVR_ADDR_L)
    {
        ret = true;
    }
    else if (svr_u == SVR_ADDR_U && svr_l == SVR_ADDR_BROADCASTING_L)
    {
        ret = true;
    }

    return ret;
}

bool dsm_media_if_dl_is_valid_asso3_clientaddr(uint8_t client_addr)
{
    bool ret = false;

    if (client_addr == CLIENT_ADDR_ASSO3_UTILITY)
    {
        ret = true;
    }
    return ret;
}

bool dsm_media_if_dl_is_valid_asso_1_2_clientaddr(uint8_t client_addr)
{
    bool ret = false;

    if (client_addr == CLIENT_ADDR_UTILITY || client_addr == CLIENT_ADDR_PUBLIC)
    {
        ret = true;
    }
    return ret;
}

bool dsm_media_if_dl_is_valid_asso4_clientaddr(uint8_t client_addr)
{
    bool ret = false;

    if (client_addr == CLIENT_ADDR_ASSO3_SITE)
    {
        ret = true;
    }
    return ret;
}

#define MEDIA_DL_SNRM 0x93  // set normal respond mode
#define MEDIA_DL_DISC 0x53  // disconnect command
bool dsm_media_if_dl_is_disc_or_snrm(uint8_t info)
{
    bool ret = false;

    if (info == MEDIA_DL_SNRM || info == MEDIA_DL_DISC)
    {
        ret = true;
    }
    return ret;
}

bool dsm_media_if_dl_is_disc(uint8_t info)
{
    bool ret = false;

    if (info == MEDIA_DL_DISC)
    {
        ret = true;
    }
    return ret;
}

bool dsm_media_if_dl_is_snrm(uint8_t info)
{
    bool ret = false;

    if (info == MEDIA_DL_SNRM)
    {
        ret = true;
    }
    return ret;
}

void dsm_media_dl_dm_frame_tx(uint32_t if_state, uint8_t client_addr,
                              uint8_t svr_addl_u, uint8_t svr_addr_l)
{
    int i;
    uint8_t disc[10];

    i = 0;
    disc[i++] = CHAR_FLAG;
    // frame type
    disc[i++] = 0xa0;
    disc[i++] = 0x00;  // length -> below

    disc[i++] = client_addr;
    disc[i++] = svr_addl_u;
    disc[i++] = svr_addr_l;

    // control byte
    disc[i++] = (DL_DM | POLL_FINAL_BIT);

    i += 2;  // next point of ( HCS or FCS )

    disc[2] = i - 1;  // flag + header +  fcs + flag

    fcs16(disc + 1, (uint16_t)(i - 1), true);  // HCS

    disc[i++] = CHAR_FLAG;
    dsm_media_if_hdlc_send(if_state, FALSE, disc, 10);
}

void dsm_media_DM_check_DM_send(uint32_t if_state, ST_HDLC_COM_PKT* ppkt)
{
    if (dsm_media_if_dl_is_valid_asso3_clientaddr(ppkt->data[5]) &&
        dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                  ppkt->data[4]) &&
        dsm_media_if_dl_is_disc_or_snrm(ppkt->data[6]))
    {
        DPRINTF(DBG_WARN, "%s \r\n", __func__);
        dsm_media_dl_dm_frame_tx(if_state, ppkt->data[5], ppkt->data[3],
                                 ppkt->data[4]);
    }
}

bool dsm_media_if_return_proc(EN_IF_RX_EVT rx_evt, ST_HDLC_COM_PKT* ppkt)
{
    bool ret = false;
    uint32_t pre_run = dsm_media_get_fsm_if_hdlc();

    DPRINTF(DBG_INFO, "%s: rx_evt[%s]: pre_run[%s]\r\n", __func__,
            dsm_media_rx_evt_string(rx_evt), dsm_media_if_fsm_string(pre_run));
    DPRINTF(
        DBG_INFO,
        "%s: ClientAddr[0x%02X], ServerAddr[U:0x%02X, L:0x%02X(%02d)], "
        "MyAddr[0x%02X(%02d)]\r\n",
        __func__, ppkt->data[5], ppkt->data[3], ppkt->data[4],
        ((ppkt->data[4] == 0xFF) ? ppkt->data[4] : (ppkt->data[4] >> 1) - 0x10),
        SVR_ADDR_L, dl_meter_addr - 0x10);

    switch (rx_evt)
    {
    case NONE_IF_RX_EVT:
        DPRINTF(DBG_ERR, "NONE_IF_RX_EVT \r\n");
        ret = true;

        break;

    case SUN_IF_RX_EVT:
        if ((pre_run != MEDIA_RUN_SUN) &&
            (appl_get_conn_state() > APPL_ASSOCIATED_STATE ||
             dl_is_connected()))
        {
            if (appl_is_sap_sec_site())
            {
#if 1  // jp.kim 25.06.05 defined(FEATURE_SEC_SITE_OP)
                if (dsm_media_if_dl_is_valid_asso3_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    dsm_media_DM_check_DM_send(MEDIA_RUN_SUN, ppkt);
                    ret = true;
                }
                else if (dsm_media_if_dl_is_valid_asso_1_2_clientaddr(
                             ppkt->data[5]) &&
                         dsm_media_if_dl_is_valid_asso_1_svraddr(ppkt->data[3],
                                                                 ppkt->data[4]))
                {  // 보안  우선권
#if defined(FEATURE_COMPILE_DM)
                    DPRINTF(DBG_WARN,
                            "2_2_1. ASSO 3: not return - received client_addr "
                            "for ASSO1\r\n");
#endif
                    ret = true;
                }
#else
                ret = true;
#endif
            }
            else if (appl_is_sap_sec_utility())
            {
                if (dsm_media_if_dl_is_valid_asso4_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    if (dsm_media_if_dl_is_snrm(ppkt->data[6]))
                    {
                        DPRINTF(DBG_ERR,
                                "2_1_1. ASSO 3:  return - SNRM received "
                                "client_addr for ASSO4\r\n");
                        ret = true;
                    }
                    else
                    {
                        DPRINTF(DBG_WARN,
                                "2_1_2. ASSO 3: not return - received "
                                "client_addr for ASSO4\r\n");
                    }
                }
                else
                {  // 485,can,ext
                    DPRINTF(DBG_ERR,
                            "2_2. ASSO 3 -> received client_addr for ASSO3, "
                            "Priority miss\r\n");
                    ret = true;
                }
            }
            else
            {
                DPRINTF(DBG_ERR, "4. None: return \r\n");
                ret = true;
            }
        }
        else if (appl_get_conn_state() > APPL_ASSOCIATED_STATE ||
                 dl_is_connected())
        {
            if (appl_is_sap_sec_site())
            {  // SUN ASSO4 인 경우
                if (dsm_media_if_dl_is_valid_asso3_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    DPRINTF(DBG_ERR,
                            "5. ASSO 4: return - received client_addr for "
                            "ASSO3 \r\n");
                    dsm_media_DM_check_DM_send(MEDIA_RUN_SUN, ppkt);
                    ret = true;
                }
            }
        }

        break;

    case RS485_IF_RX_EVT:
        if (pre_run != MEDIA_RUN_RS485 &&
            (appl_get_conn_state() > APPL_ASSOCIATED_STATE ||
             dl_is_connected()))
        {
            if (appl_is_sap_sec_site())
            {
#if 1  // jp.kim 25.06.05 defined(FEATURE_SEC_SITE_OP)
                if (dsm_media_if_dl_is_valid_asso3_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    dsm_media_DM_check_DM_send(MEDIA_RUN_RS485, ppkt);
                    ret = true;
                }
                else if (dsm_media_if_dl_is_valid_asso_1_2_clientaddr(
                             ppkt->data[5]) &&
                         dsm_media_if_dl_is_valid_asso_1_svraddr(ppkt->data[3],
                                                                 ppkt->data[4]))
                {  //  보안  우선권
#if defined(FEATURE_COMPILE_DM)
                    DPRINTF(DBG_WARN,
                            "2_2_1. ASSO 3: not return - received client_addr "
                            "for ASSO1\r\n");
#endif
                    ret = true;
                }
#else
                ret = true;
#endif
            }
            else if (appl_is_sap_sec_utility())
            {
                if (dsm_media_if_dl_is_valid_asso4_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    if (dsm_media_if_dl_is_snrm(ppkt->data[6]))
                    {
                        DPRINTF(DBG_ERR,
                                "2_1_1. ASSO 3:  return - SNRM received "
                                "client_addr for ASSO4\r\n");
                        ret = true;
                    }
                    else
                    {
                        DPRINTF(DBG_WARN,
                                "2_1_2. ASSO 3: not return - received "
                                "client_addr for ASSO4\r\n");
                    }
                }
                else if (dsm_media_if_dl_is_valid_asso3_clientaddr(
                             ppkt->data[5]) &&
                         dsm_media_if_dl_is_valid_asso_3_4_svraddr(
                             ppkt->data[3], ppkt->data[4]))
                {  // 485 우선권
                    DPRINTF(DBG_WARN,
                            "2_2. ASSO 3: not return - received client_addr "
                            "for ASSO3\r\n");
                }
                else if (dsm_media_if_dl_is_valid_asso_1_2_clientaddr(
                             ppkt->data[5]) &&
                         dsm_media_if_dl_is_valid_asso_1_svraddr(ppkt->data[3],
                                                                 ppkt->data[4]))
                {  // 485 우선권
                    DPRINTF(DBG_WARN,
                            "2_2_1. ASSO 3: not return - received client_addr "
                            "for ASSO1\r\n");
                }
                else
                {
                    DPRINTF(DBG_ERR, "2_3. ASSO 3 None: return \r\n");
                    ret = true;
                }
            }
            else
            {
                DPRINTF(DBG_ERR, "3. None: return \r\n");
                ret = true;
            }
        }
        else if (appl_get_conn_state() > APPL_ASSOCIATED_STATE ||
                 dl_is_connected())
        {
#if 1  // jp.kim 25.06.02
            if (appl_is_sap_sec_site())
            {  // 485 ASSO4 -> ASSO3
                if (dsm_media_if_dl_is_valid_asso3_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    DPRINTF(DBG_ERR,
                            "5. ASSO 4: return - received client_addr for "
                            "ASSO3 \r\n");
                    dsm_media_DM_check_DM_send(MEDIA_RUN_RS485, ppkt);
                    ret = true;
                }
            }
#endif
        }

        break;

    case CAN_IF_RX_EVT:
        if (pre_run != MEDIA_RUN_CAN &&
            (appl_get_conn_state() > APPL_ASSOCIATED_STATE ||
             dl_is_connected()))
        {
            if (appl_is_sap_sec_site())
            {
#if 1  // jp.kim 25.06.05 defined(FEATURE_SEC_SITE_OP)
                if (dsm_media_if_dl_is_valid_asso3_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    dsm_media_DM_check_DM_send(MEDIA_RUN_CAN, ppkt);
                    ret = true;
                }
                else if (dsm_media_if_dl_is_valid_asso_1_2_clientaddr(
                             ppkt->data[5]) &&
                         dsm_media_if_dl_is_valid_asso_1_svraddr(ppkt->data[3],
                                                                 ppkt->data[4]))
                {  //  보안  우선권
#if defined(FEATURE_COMPILE_DM)
                    DPRINTF(DBG_WARN,
                            "2_2_1. ASSO 3: not return - received client_addr "
                            "for ASSO1\r\n");
#endif
                    ret = true;
                }
#else
                ret = true;
#endif
            }
            else if (appl_is_sap_sec_utility())
            {
                if (dsm_media_if_dl_is_valid_asso4_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    if (dsm_media_if_dl_is_snrm(ppkt->data[6]))
                    {
                        DPRINTF(DBG_ERR,
                                "2_1_1. ASSO 3:  return - SNRM received "
                                "client_addr for ASSO4\r\n");
                        ret = true;
                    }
                    else
                    {
                        DPRINTF(DBG_WARN,
                                "2_1_2. ASSO 3: not return - received "
                                "client_addr for ASSO4\r\n");
                    }
                }
                else if (dsm_media_if_dl_is_valid_asso3_clientaddr(
                             ppkt->data[5]) &&
                         dsm_media_if_dl_is_valid_asso_3_4_svraddr(
                             ppkt->data[3], ppkt->data[4]))
                {  // can 우선권
                    DPRINTF(DBG_WARN,
                            "2_2. ASSO 3: not return - received client_addr "
                            "for ASSO3, Priority OK\r\n");
                }
#if 0
                else if (dsm_media_if_dl_is_valid_asso_1_2_clientaddr(
                             ppkt->data[5]) &&
                         dsm_media_if_dl_is_valid_asso_1_svraddr(ppkt->data[3],
                                                                 ppkt->data[4]))
                {  // 485 우선권
                    DPRINTF(DBG_WARN,
                            "2_2_1. ASSO 3: not return - received client_addr "
                            "for ASSO1\r\n");
                }
#endif
                else
                {
                    DPRINTF(DBG_ERR, "2_3. ASSO 3 None: return \r\n");
                    ret = true;
                }
            }
            else
            {
                DPRINTF(DBG_ERR, "3. None: return \r\n");
                ret = true;
            }
        }
#if 1  // jp.kim 25.06.02
        else if (appl_get_conn_state() > APPL_ASSOCIATED_STATE ||
                 dl_is_connected())
        {
            if (appl_is_sap_sec_site())
            {  // CAN ASSO4 -> ASSO3
                if (dsm_media_if_dl_is_valid_asso3_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    DPRINTF(DBG_ERR,
                            "5. ASSO 4: return - received client_addr for "
                            "ASSO3 \r\n");
                    dsm_media_DM_check_DM_send(MEDIA_RUN_CAN, ppkt);
                    ret = true;
                }
            }
        }
#endif
        break;

    case EXT_IF_RX_EVT:
        if (pre_run != MEDIA_RUN_EXT &&
            (appl_get_conn_state() > APPL_ASSOCIATED_STATE ||
             dl_is_connected()))
        {
            if (appl_is_sap_sec_site())
            {
                DPRINTF(DBG_ERR, "1. ASSO 4: return \r\n");
#if 1  // jp.kim 25.06.05 defined(FEATURE_SEC_SITE_OP)
                if (dsm_media_if_dl_is_valid_asso3_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    dsm_media_DM_check_DM_send(MEDIA_RUN_EXT, ppkt);
                    ret = true;
                }
                else if (dsm_media_if_dl_is_valid_asso_1_2_clientaddr(
                             ppkt->data[5]) &&
                         dsm_media_if_dl_is_valid_asso_1_svraddr(ppkt->data[3],
                                                                 ppkt->data[4]))
                {  //  보안  우선권
#if defined(FEATURE_COMPILE_DM)
                    DPRINTF(DBG_WARN,
                            "2_2_1. ASSO 3: not return - received client_addr "
                            "for ASSO1\r\n");
#endif
                    ret = true;
                }
#else
                ret = true;
#endif
            }
            else if (appl_is_sap_sec_utility())
            {
                if (dsm_media_if_dl_is_valid_asso4_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    if (dsm_media_if_dl_is_snrm(ppkt->data[6]))
                    {
                        DPRINTF(DBG_ERR,
                                "2_1_1. ASSO 3:  return - SNRM received "
                                "client_addr for ASSO4\r\n");
                        ret = true;
                    }
                    else
                    {
                        DPRINTF(DBG_WARN,
                                "2_1_2. ASSO 3: not return - received "
                                "client_addr for ASSO4\r\n");
                    }
                }
                else if (dsm_media_if_dl_is_valid_asso3_clientaddr(
                             ppkt->data[5]) &&
                         dsm_media_if_dl_is_valid_asso_3_4_svraddr(
                             ppkt->data[3], ppkt->data[4]))
                {
                    if (pre_run == MEDIA_RUN_RS485 || pre_run == MEDIA_RUN_CAN)
                    {
                        // 유선 인 경우 EXT 가 오면
                        DPRINTF(DBG_ERR,
                                "2_2. ASSO 3: return - received client_addr "
                                "for ASSO3, Priority miss\r\n");
                        ret = true;
                    }
                    else
                    {
                        DPRINTF(DBG_WARN,
                                "2_3. ASSO 3: not return - received "
                                "client_addr for ASSO3, Priority OK\r\n");
                    }
                }
                else
                {
                    DPRINTF(DBG_ERR, "3. None: return \r\n");
                    ret = true;
                }
            }
            else
            {
                DPRINTF(DBG_ERR, "4. None: return \r\n");
                ret = true;
            }
        }
#if 1  // jp.kim 25.06.02
        else if (appl_get_conn_state() > APPL_ASSOCIATED_STATE ||
                 dl_is_connected())
        {
            if (appl_is_sap_sec_site())
            {  // EXT_MODEM ASSO4 -> ASSO3
                if (dsm_media_if_dl_is_valid_asso3_clientaddr(ppkt->data[5]) &&
                    dsm_media_if_dl_is_valid_asso_3_4_svraddr(ppkt->data[3],
                                                              ppkt->data[4]))
                {
                    DPRINTF(DBG_ERR,
                            "5. EXT_MODEM ASSO 4: return - received "
                            "client_addr for ASSO3 \r\n");
                    dsm_media_DM_check_DM_send(MEDIA_RUN_EXT, ppkt);
                    ret = true;
                }
            }
        }
#endif
        break;
    }
    return ret;
}

uint32_t dsm_media_is_can_power_off_push(void)
{
    return g_media_can_power_off_push;
}

uint32_t dsm_media_if_send(uint32_t media_protocol_type, uint32_t poll_flag,
                           uint8_t* p_data, uint16_t len)
{
    uint32_t if_state = MEDIA_PROTOCOL_IF_HDLC;

    DPRINTF(DBG_INFO, "%s: %s\r\n", __func__,
            dsm_media_if_protocol_string(media_protocol_type));

    if (MEDIA_PROTOCOL_IF_HDLC == media_protocol_type)
    {
        if_state = dsm_media_get_fsm_if_hdlc();
    }
    else
    {
        if_state = dsm_media_get_fsm_if_ATCMD();
    }

    if (poll_flag)
    {
    }

    switch (if_state)
    {
    case MEDIA_RUN_NONE:
        break;

    case MEDIA_RUN_RS485:
        DPRINT_HEX(DBG_NONE, "RS485_TX", p_data, len, DUMP_ALWAYS);

        if (poll_flag)
        {
            dsm_uart_set_poll_mode(RS485_PORT, TRUE);
            dsm_uart_send(RS485_PORT, (char*)p_data, len);
            dsm_uart_set_poll_mode(RS485_PORT, FALSE);
        }
        else
        {
            dsm_uart_send(RS485_PORT, (char*)p_data, len);
        }
        break;

    case MEDIA_RUN_CAN:
        DPRINT_HEX(DBG_NONE, "CAN_TX", p_data, len, DUMP_DLMS);
#ifdef M2354_CAN /* bccho, 2023-11-28 */
        if (dsm_can_get_push_flag())
        {
            if (poll_flag)
            {
                MSG00("poll");
                uint16_t cnt = 500;
                IsoTpLink* plink = (IsoTpLink*)dsm_isotp_user_get_link();
                dsm_isotp_user_tx(DLMS_PUSH_MODE, p_data, len);

                g_media_can_power_off_push = TRUE;

                while (cnt--)
                {
                    OSTimeDly(OS_MS2TICK(2));
                    isotp_poll(plink);
                }

                g_media_can_power_off_push = FALSE;
            }
            else
            {
                g_media_can_power_off_push = FALSE;
                dsm_isotp_user_tx(DLMS_PUSH_MODE, p_data, len);
            }
            dsm_can_set_push_flag(FALSE);
        }
        else
        {
            dsm_isotp_user_tx(DLMS_EX_MODE, p_data, len);
        }
#endif
        break;

    case MEDIA_RUN_SUN:
        if (poll_flag)
        {
            DPRINT_HEX(DBG_TRACE, "SUN_TX", p_data, len, DUMP_ALWAYS);
            dsm_uart_set_poll_mode(IMODEM_PORT, TRUE);
            dsm_uart_send(IMODEM_PORT, (char*)p_data, len);
            dsm_uart_set_poll_mode(IMODEM_PORT, FALSE);
        }
        else
        {
            DPRINT_HEX(/*DBG_NONE*/ DBG_TRACE, "SUN_TX", p_data, len,
                       DUMP_ALWAYS);
            dsm_uart_send(IMODEM_PORT, (char*)p_data, len);
        }

        break;
    case MEDIA_RUN_EXT:
        DPRINT_HEX(/*DBG_NONE*/ DBG_TRACE, "EXT_TX", p_data, len, DUMP_ALWAYS);
        if (poll_flag)
        {
            dsm_uart_set_poll_mode(EMODEM_PORT, TRUE);
            dsm_uart_send(EMODEM_PORT, (char*)p_data, len);
            dsm_uart_set_poll_mode(EMODEM_PORT, FALSE);
        }
        else
        {
            dsm_uart_send(EMODEM_PORT, (char*)p_data, len);
        }

        break;
    }

    if (poll_flag)
    {
        dsm_wdt_ext_toggle_immd();
    }
    return TRUE;
}

uint32_t dsm_media_if_hdlc_send(uint32_t if_state, uint32_t poll_flag,
                                uint8_t* p_data, uint16_t len)
{
    DPRINTF(DBG_WARN, "%s: %d\r\n", __func__, if_state);

    switch (if_state)
    {
    case MEDIA_RUN_NONE:

        break;
    case MEDIA_RUN_RS485:
        DPRINT_HEX(DBG_NONE, "RS485_TX", p_data, len, DUMP_ALWAYS);

        if (poll_flag)
        {
            dsm_uart_set_poll_mode(RS485_PORT, TRUE);
            dsm_uart_send(RS485_PORT, (char*)p_data, len);
            dsm_uart_set_poll_mode(RS485_PORT, FALSE);
        }
        else
        {
            dsm_uart_send(RS485_PORT, (char*)p_data, len);
        }

        break;
    case MEDIA_RUN_CAN:
        DPRINT_HEX(DBG_NONE, "CAN_TX", p_data, len, DUMP_ALWAYS);
#ifdef M2354_CAN /* bccho, 2023-11-28 */
        dsm_isotp_user_tx(DLMS_EX_MODE, p_data, len);
#endif
        break;

    case MEDIA_RUN_SUN:
        DPRINT_HEX(DBG_NONE, "SUN_TX", p_data, len, DUMP_ALWAYS);
        if (poll_flag)
        {
            dsm_uart_set_poll_mode(IMODEM_PORT, TRUE);
            dsm_uart_send(IMODEM_PORT, (char*)p_data, len);
            dsm_uart_set_poll_mode(IMODEM_PORT, FALSE);
        }
        else
        {
            dsm_uart_send(IMODEM_PORT, (char*)p_data, len);
        }

        break;
    case MEDIA_RUN_EXT:
        DPRINT_HEX(DBG_NONE, "EXT_TX", p_data, len, DUMP_ALWAYS);
        if (poll_flag)
        {
            dsm_uart_set_poll_mode(EMODEM_PORT, TRUE);
            dsm_uart_send(EMODEM_PORT, (char*)p_data, len);
            dsm_uart_set_poll_mode(EMODEM_PORT, FALSE);
        }
        else
        {
            dsm_uart_send(EMODEM_PORT, (char*)p_data, len);
        }

        break;
    }

    return TRUE;
}
