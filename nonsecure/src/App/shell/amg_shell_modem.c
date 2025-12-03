/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_shell.h"
#include "amg_ansi.h"
#include "amg_uart.h"
#include "mx25r4035f.h"
#include "defines.h"
#include "amg_crc.h"
#include "mx25r4035f_def.h"
#include "disp.h"
#include "eeprom_at24cm02.h"
#include "amg_gpio.h"
#include "amg_mif_prtl.h"
#include "amg_pwr.h"
#include "whm.h"
#include "amg_mtp_process.h"
#include "amg_lcd.h"
#include "disp.h"
#include "nv.h"
#include "utils.h"
#include "amg_sec.h"
#include "amg_utc_util.h"
#include "amg_power_mnt.h"
#include "amg_rtc.h"
#include "amg_wdt.h"
#include "amg_media_mnt.h"
#include "amg_modemif_prtl.h"
#include "amg_spi.h"
#include "amg_meter_main.h"
#include "amg_stock_op_mode.h"

/*
******************************************************************************
*     LOCAL CONSTANTS
******************************************************************************
*/

/*
******************************************************************************
*    LOCAL DATA TYPES
******************************************************************************
*/

static SHELL_ERR shell_peri_i_modem(uint32_t id, char *pParamStr,
                                    uint32_t size);
static SHELL_ERR shell_peri_e_modem(uint32_t id, char *pParamStr,
                                    uint32_t size);
/*
******************************************************************************
*    GLOBAL VARIABLES
******************************************************************************
*/
const SHELL_CNTX shell_modem_cmd_list[] = {
    {0, "imodem", _H("pwr_on/off|pwr_high/low|reset|atcmd"), shell_peri_i_modem,
     NULL},
    {1, "emodem", _H("reset|atcmd"), shell_peri_e_modem, NULL},
    {0, 0, 0, 0, 0}};

/*
******************************************************************************
*    LOCAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*    LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
*    FUNCTIONS
******************************************************************************
*/

extern const uint16_t crc16_ccitt_tab[256];
uint16_t g_t_decrypt_len = 0;
uint8_t g_t_decrypt[64];
uint8_t modem_type = INT_MODEM_TYPE;
static SHELL_ERR shell_peri_i_modem(uint32_t id, char *pParamStr, uint32_t size)
{
    char *argv[36];
    uint32_t argc;
    uint8_t IF_STATE = MEDIA_RUN_SUN;

    if (size < 5)
    {
        return SHELL_INVALID_PARAM;
    }

    switch (id)
    {
    case 0:
    {
        shell_arg_parse(pParamStr, &argc, argv, 4, ' ');

        if (argc == 1)
        {
            if (!strcmp(argv[0], "pwr_on"))
            {
                dsm_gpio_imodem_power_enable();

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pwr_off"))
            {
                dsm_gpio_imodem_power_disable();

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pf_high"))
            {
                dsm_gpio_imodem_pf_high(); /* PF unset */

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pf_low"))
            {
                dsm_gpio_imodem_pf_low(); /* PF set */

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "reset"))
            {
                dsm_modem_hw_reset(INT_MODEM_RESET);

                return SHELL_OK;
            }
#ifdef STOCK_OP /* bccho, 2024-09-26 */
            else if (!strcmp(argv[0], "on_stock"))
            {
                dsm_pmnt_set_op_mode(PMNT_STOCK_OP);
                dsm_stock_action();

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "off_stock"))
            {
                dsm_pmnt_set_op_mode(PMNT_ACTIVE_OP);

                return SHELL_OK;
            }
#endif
        }
        else if (argc == 2)
        {
            if (!strcmp(argv[0], "baud"))
            {
                ST_AT_BAUD *at_baud = dsm_modemif_get_atbaud_info();
                at_baud->imodem = atoi(argv[1]);
                DPRINTF(DBG_WARN, "at_baud %d\r\n", at_baud->imodem);

                dsm_modemif_baud_nvwrite(at_baud);
                dsm_imodemif_init(FALSE);

                return SHELL_OK;
            }
            if (!strcmp(argv[0], "pwr_tunning"))
            {
                uint32_t delay = atoi(argv[1]);

                DPRINTF(DBG_WARN, "power_reset tuning: delay %d ms\r\n", delay);
                dsm_gpio_imodem_power_disable();
                dsm_gpio_imodem_pf_low(); /* PF set */
                dsm_gpio_imodem_reset_low();
                OSTimeDly(OS_MS2TICK(delay));
                dsm_gpio_imodem_power_enable();
                OSTimeDly(OS_MS2TICK(10));
                dsm_gpio_imodem_reset_high();

                return SHELL_OK;
            }
        }
        else
        {
            if (!strcmp(argv[0], "atcmd"))
            {
                if (!strcmp(argv[1], "get"))
                {
                    if (argc == 3)
                    {
                        ST_AT_CMD_TX_PKT *tx_pkt = dsm_atcmd_get_txpkt();

                        if (!strcmp(argv[2], "mac"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_MAC;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "meterid"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_METERID;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mfdate"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_MFDATE;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "hwver"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_HWVER;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "fwver"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_FWVER;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "fwupsta"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_FWUPSTA;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "hash"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_SHA256;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "listen"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_LISTEN;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mode"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_MODE;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mmid"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_get_modem_id(FALSE);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "rns"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "zcd"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_get_zcd(FALSE);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "zcd_time"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_get_zcd_time(FALSE);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "baud"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_get_baud(FALSE);

                            return SHELL_OK;
                        }
                    }
                }
                else if (!strcmp(argv[1], "set"))
                {
                    if (argc == 3)
                    {
                        if (!strcmp(argv[2], "mac"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "meterid"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_meterid(FALSE);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mfdate"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_mfdate(FALSE);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "rns"))
                        {
                            return SHELL_OK;
                        }
                    }
                    else if (argc == 4)
                    {
                        if (!strcmp(argv[2], "reset"))
                        {
                            uint32_t reset_type = atoi(argv[3]);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);

                            if (reset_type == 0)
                            {
                                dsm_atcmd_set_reset(FALSE, AT_RST_NORMAL);
                            }
                            else
                            {
                                dsm_atcmd_set_reset(FALSE, AT_RST_FACTORY);
                            }

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "listen"))
                        {
                            uint32_t listen = atoi(argv[3]);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_listen(FALSE, listen);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mode"))
                        {
                            uint32_t router_n_coodi = atoi(argv[3]);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_mode(FALSE, router_n_coodi);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "zcd"))
                        {
                            uint32_t enable = atoi(argv[3]);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_zcd(FALSE, enable);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "baud"))
                        {
                            uint32_t baud = atoi(argv[3]);

                            DPRINTF(DBG_WARN, "baud %d\r\n", baud);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_baud(FALSE, baud);

                            return SHELL_OK;
                        }
                    }
                }
                else if (!strcmp(argv[1], "poll"))
                {
                    if (argc == 3)
                    {
                        ST_ATCMD_TMP_BUF st_atmcd_from_modem;
                        ST_AT_CMD_RX_PKT atcmd_com_pkt;

                        if (!strcmp(argv[2], "meterid"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_meterid(TRUE);
                            // DPRINTF
                            dsm_atcmd_tx_pkt_init();
                            dsm_atcmd_if_rx_polling(modem_type,
                                                    &st_atmcd_from_modem);
                            // DPRINT_HEX
                            if (dsm_atcmd_rx_parser(
                                    (uint8_t *)st_atmcd_from_modem.string,
                                    st_atmcd_from_modem.len,
                                    &atcmd_com_pkt) == AT_ERR_NONE_PARSER_OK)
                            {
                                dsm_atcmd_rx_proc(&atcmd_com_pkt);
                                dsm_atcmd_rx_pkt_init();
                                atcmd_com_pkt.len = 0;
                            }

                            return SHELL_OK;
                        }
                    }
                }
            }
        }
    }
    break;

    default:
        break;
    }

    return SHELL_INVALID_PARAM;
}

static SHELL_ERR shell_peri_e_modem(uint32_t id, char *pParamStr, uint32_t size)
{
    char *argv[36];
    uint32_t argc;
    uint8_t IF_STATE = MEDIA_RUN_EXT;

    if (size < 3)
    {
        return SHELL_INVALID_PARAM;
    }

    switch (id)
    {
    case 1:
    {
        shell_arg_parse(pParamStr, &argc, argv, 4, ' ');

        if (argc == 1)
        {
            if (!strcmp(argv[0], "reset"))
            {
                dsm_modem_hw_reset(EXT_MODEM_RESET);

                return SHELL_OK;
            }
        }
        else if (argc == 2)
        {
            if (!strcmp(argv[0], "mode"))
            {
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "baud"))
            {
                dsm_modemif_get_atbaud_info()->emodem = atoi(argv[1]);
                DPRINTF(DBG_WARN, "at_baud %d\r\n",
                        dsm_modemif_get_atbaud_info()->emodem);

                dsm_modemif_baud_nvwrite(dsm_modemif_get_atbaud_info());
                dsm_emodemif_init();

                return SHELL_OK;
            }
        }
        else
        {
            if (!strcmp(argv[0], "atcmd"))
            {
                if (!strcmp(argv[1], "get"))
                {
                    if (argc == 3)
                    {
                        ST_AT_CMD_TX_PKT *tx_pkt = dsm_atcmd_get_txpkt();

                        if (!strcmp(argv[2], "mac"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_MAC;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "meterid"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_METERID;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mfdate"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_MFDATE;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "hwver"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_HWVER;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "fwver"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_FWVER;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "fwupsta"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_FWUPSTA;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "hash"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_SHA256;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "listen"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_LISTEN;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mode"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            tx_pkt->cmd_id = ATCMD_MODE;
                            tx_pkt->cmd_type = AT_GET_REQ_DELIMETER;
                            tx_pkt->data_cnt = 0;
                            dsm_atcmd_tx(FALSE, tx_pkt);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "id"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_get_modem_id(FALSE);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "485"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_get_485(FALSE);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "zcd"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_get_zcd(FALSE);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "time_zcd"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_get_zcd_time(FALSE);

                            return SHELL_OK;
                        }
                    }
                }
                else if (!strcmp(argv[1], "set"))
                {
                    if (argc == 3)
                    {
                        if (!strcmp(argv[2], "mac"))
                        {
                            // dsm_media_set_fsm_if_hdlc(MEDIA_RUN_SUN);
                            // dsm_atcmd_tx(AT_SET_REQ_DELIMETER, ATCMD_MAC,
                            // NULL, 0);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "meterid"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_meterid(FALSE);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mfdate"))
                        {
                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_mfdate(FALSE);

                            return SHELL_OK;
                        }
                    }
                    else if (argc == 4)
                    {
                        if (!strcmp(argv[2], "reset"))
                        {
                            uint32_t reset_type = atoi(argv[3]);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);

                            if (reset_type == 0)
                            {
                                dsm_atcmd_set_reset(FALSE, AT_RST_NORMAL);
                            }
                            else
                            {
                                dsm_atcmd_set_reset(FALSE, AT_RST_FACTORY);
                            }

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "listen"))
                        {
                            uint32_t listen = atoi(argv[3]);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_listen(FALSE, listen);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mode"))
                        {
                            uint32_t router_n_coodi = atoi(argv[3]);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_mode(FALSE, router_n_coodi);

                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "485"))
                        {
                            uint32_t enable = atoi(argv[3]);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_485(FALSE, enable);

                            return SHELL_OK;
                        }
                        else if (!strncmp(argv[2], "zcd", 3))
                        {
                            uint32_t enable = atoi(argv[3]);

                            dsm_media_set_fsm_if_ATCMD(IF_STATE);
                            dsm_atcmd_set_zcd(FALSE, enable);

                            return SHELL_OK;
                        }
                    }
                }
            }
        }
    }
    break;

    default:
        break;
    }

    return SHELL_INVALID_PARAM;
}
