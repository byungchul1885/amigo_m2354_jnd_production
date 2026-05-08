/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_shell.h"
#include "amg_ansi.h"
#include "amg_uart.h"
#include "defines.h"
#include "amg_crc.h"
#include "eeprom_at24cm02.h"
#include "amg_gpio.h"
#if 0 /* bccho, UNUSED, 2023-07-15 */
#include "amg_usart_7816.h"
#include "platform.h"
#endif /* bccho */
#include "amg_mif_prtl.h"
#include "amg_pwr.h"
#include "appl.h"
#include "whm.h"
#include "amg_mtp_process.h"
#include "amg_lcd.h"
#include "disp.h"
#include "nv.h"
#include "get_req.h"
#include "options_sel.h"
#include "softimer.h"
#include "dl.h"
#include "amg_imagetransfer.h"
#include "set_req.h"
#include "amg_meter_main.h"
#include "eob.h"

#include "mx25r4035f_def.h"
#include "mx25r4035f.h"
#include "tmp.h"
#include "amg_sec.h"
/*
******************************************************************************
*     LOCAL CONSTANTS
******************************************************************************
*/
#define PRODUCT_ENABLE_ID 0xaabbccdd

/*
******************************************************************************
*    LOCAL DATA TYPES
******************************************************************************
*/

static SHELL_ERR shell_product_default(uint32_t id, char* pParamStr,
                                       uint32_t size);
/*
******************************************************************************
*    GLOBAL VARIABLES
******************************************************************************
*/
const SHELL_CNTX shell_cmd_product[] = {
    {0, "prd", _H("ena|dis"), shell_product_default, NULL},
    {1, "get", _H("cid|ldn|meterid|time"), shell_product_default, NULL},
    {2, "set", _H("cid|cid2|ldn|time"), shell_product_default, NULL},
    {3, "act", _H("reset"), shell_product_default, NULL},
    {4, "cal", _H("start|parm_sagswell"), shell_product_default, NULL},

    {0, 0, 0, 0, 0}};

/**********************************************************************************
**  prod product enable globaltopmeter!!! ( 양산 설정 하기전에 항상 먼저 수행
필요*
***********************************************************************************
     prod cal start
     prod cal parm_sagswell

prod product en set

     prod set custid 30 30 30 30 31 30 35 --> "0000105" <- serial number
     prod get custid
     prod get meterid
                    y1 y2 m1 m2 d1 d2 A  V1 V2
     prod set devid 32 30 30 39 30 38 41 33 30 --> "200908"'A'"30"
     prod get devid

     prod act reset factory
     prod act reset comm
     prod act datafull lp / avg / prd / nprd / season

                   y  m d h  m  s
     prod set time 20 9 8 16 59 30 --> 2020.9.8 16:59:30
     prod get time
**********************************************************************************/

/*
******************************************************************************
*    LOCAL VARIABLES
******************************************************************************
*/
extern bool circ_mode_freezing;
extern uint8_t dsp_circ_mode_index;
extern disp_mode_type dsp_mode_in_state;

uint32_t g_product_mode = 0;
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
static SHELL_ERR shell_product_default(uint32_t id, char* pParamStr,
                                       uint32_t size)
{
    char* argv[36];
    uint32_t argc;

    if (size < /*4*/ 3)
    {
        return SHELL_INVALID_PARAM;
    }

    shell_arg_parse(pParamStr, &argc, argv, 10, ' ');

    switch (id)
    {
    case 0:
    { /* prd */
        if (argc == 2)
        {
            if (!strcmp(argv[0], "ena"))
            {
                if (!strcmp(argv[1], "set"))  // by WD
                {
                    g_product_mode = PRODUCT_ENABLE_ID;
                    DPRINTF(DBG_WARN, "Product Enable OK\r\n");

                    return SHELL_OK;
                }
            }
        }
        else if (argc == 1)
        {
            if (!strcmp(argv[0], "dis"))
            {
                g_product_mode = 0;

                return SHELL_OK;
            }
        }
    }
    break;

    /* get */
    case 1:
    {
        if (argc != 1)
        {
            return SHELL_INVALID_PARAM;
        }

        if (!strcmp(argv[0], "cid"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], /*"devid"*/ "ldn"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "meterid"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "time"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "cert"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "temp"))
        {
            return SHELL_OK;
        }
    }
    break;

    /* set */
    case 2:
    {
        if (g_product_mode != PRODUCT_ENABLE_ID)
        {
            DPRINTF(DBG_ERR, "Wrong Product ID\r\n");

            return SHELL_INVALID_CMD;
        }
        if (!strcmp(argv[0], "cid"))
        {
            return SHELL_OK;
        }
        if (!strcmp(argv[0], "cid2"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], /*"devid"*/ "ldn"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "time"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "fwinfo"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "test_cert"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "cert"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "sysinfo"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "mtpinfo"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "temp_reset"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "temp"))
        {
            return SHELL_OK;
        }
    }
    break;

    /* act */
    case 3:
    {
        if (g_product_mode != PRODUCT_ENABLE_ID)
        {
            DPRINTF(DBG_ERR, "Wrong Product ID\r\n");

            return SHELL_INVALID_CMD;
        }

        if (argc == 2)
        {
            if (!strcmp(argv[0], "reset"))
            {
                if (!strcmp(argv[1], "factory"))
                {
                    /* bccho, 2024-01-15, 0111 포팅반영 */
                    dsm_sys_fwinfo_initial_set(true);
                    whm_clear_all(true);
                    return SHELL_OK;
                }
                else if (!strcmp(argv[1], "comm"))
                {
                    whm_clear_all(false);
                    return SHELL_OK;
                }
            }
            else if (!strcmp(argv[0], "datafull"))
            {
                if (!strcmp(argv[1], "lp"))
                {
                    lp_save_manual();
                    return SHELL_OK;
                }
                else if (!strcmp(argv[1], "avg"))
                {
                    lpavg_save_manual();
                    return SHELL_OK;
                }
                else if (!strcmp(argv[1], "prd"))
                {
                    sr_dr_proc(EOB_PERIOD_FLAG, (MR_SR_BIT | MR_DR_BIT),
                               &cur_rtc, appl_tbuff);
                    return SHELL_OK;
                }
                else if (!strcmp(argv[1], "nprd"))
                {
                    sr_dr_proc(EOB_nPERIOD_FLAG, (MR_SR_BIT | MR_DR_BIT),
                               &cur_rtc, appl_tbuff);
                    return SHELL_OK;
                }
                else if (!strcmp(argv[1], "season"))
                {
                    sr_dr_proc(EOB_SEASON_FLAG, (MR_SR_BIT | MR_DR_BIT),
                               &cur_rtc, appl_tbuff);
                    return SHELL_OK;
                }
            }
            else if (!strcmp(argv[0], "lcdstop"))
            {
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "lcdno"))
            {
                return SHELL_OK;
            }
        }
    }
    break;

    /* cal */
    case 4:
    {
        if (g_product_mode != PRODUCT_ENABLE_ID)
        {
            DPRINTF(DBG_ERR, "Wrong Product ID\r\n");

            return SHELL_INVALID_CMD;
        }

        if (argc == 1)
        {
            if (!strcmp(argv[0], "start"))
            {
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "parm_sagswell"))
            {
                return SHELL_OK;
            }
        }
    }
    break;

    default:
        break;
    }

    return SHELL_INVALID_PARAM;
}

uint8_t byte_to_ascii(unsigned char a)
{
    if (a >= 10)
        a += ('A' - 10);
    else
        a += '0';
    return a;
}

uint8_t AsciiToHEX(unsigned char ch)
{
    unsigned char hex;

    if (ch >= 'A' && ch <= 'F')
    {
        hex = (ch - 'A' + 0x0A);
    }
    else if (ch >= 'a' && ch <= 'f')
    {
        hex = (ch - 'a' + 0x0A);
    }
    else
    {
        hex = (ch - '0');
    }

    return (hex & 0x0F);
}