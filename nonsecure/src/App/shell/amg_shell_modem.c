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

static SHELL_ERR shell_peri_i_modem(uint32_t id, char* pParamStr,
                                    uint32_t size);
static SHELL_ERR shell_peri_e_modem(uint32_t id, char* pParamStr,
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
static SHELL_ERR shell_peri_i_modem(uint32_t id, char* pParamStr, uint32_t size)
{
    char* argv[36];
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
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pwr_off"))
            {
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pf_high"))
            {
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pf_low"))
            {
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "reset"))
            {
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
                return SHELL_OK;
            }
            if (!strcmp(argv[0], "pwr_tunning"))
            {
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
                        if (!strcmp(argv[2], "mac"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "meterid"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mfdate"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "hwver"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "fwver"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "fwupsta"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "hash"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "listen"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mode"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mmid"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "rns"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "zcd"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "zcd_time"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "baud"))
                        {
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
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mfdate"))
                        {
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
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "listen"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mode"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "zcd"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "baud"))
                        {
                            return SHELL_OK;
                        }
                    }
                }
                else if (!strcmp(argv[1], "poll"))
                {
                    if (argc == 3)
                    {
                        if (!strcmp(argv[2], "meterid"))
                        {
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

static SHELL_ERR shell_peri_e_modem(uint32_t id, char* pParamStr, uint32_t size)
{
    char* argv[36];
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
                        ST_AT_CMD_TX_PKT* tx_pkt = dsm_atcmd_get_txpkt();

                        if (!strcmp(argv[2], "mac"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "meterid"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mfdate"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "hwver"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "fwver"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "fwupsta"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "hash"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "listen"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mode"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "id"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "485"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "zcd"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "time_zcd"))
                        {
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
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mfdate"))
                        {
                            return SHELL_OK;
                        }
                    }
                    else if (argc == 4)
                    {
                        if (!strcmp(argv[2], "reset"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "listen"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "mode"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strcmp(argv[2], "485"))
                        {
                            return SHELL_OK;
                        }
                        else if (!strncmp(argv[2], "zcd", 3))
                        {
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
