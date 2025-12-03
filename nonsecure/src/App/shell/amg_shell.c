/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "defines.h"
#include "main.h"
#include "amg_debug.h"
#include "amg_shell.h"
#include "amg_ansi.h"
#include "amg_uart.h"
#include "amg_time.h"
#include "utils.h"
#include "amg_mtp_process.h"
#include "amg_push_datanoti.h"
#include "appl.h"
#include "amg_rtc.h"
#include "amg_modemif_prtl.h"
#include "amg_media_mnt.h"
#include "amg_power_mnt.h"
#include "amg_sec.h"
#include "phy.h"
#include "whm.h"
#include "meter_app.h"
#include "get_req.h"
#include "FreeRTOS.h"

/*
******************************************************************************
*    Definition
******************************************************************************
*/
#define SHELL_BUFF_SIZE 128
#define SHELL_PRINT_SIZE 256
#define SHELL_PROMPT "$ "

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
static SHELL_ERR shell_dbg_level(uint32_t id, char *pParamStr, uint32_t size);
static SHELL_ERR shell_cls(uint32_t id, char *pParamStr, uint32_t size);
static SHELL_ERR shell_handler(char *pHelpTitle, const SHELL_CNTX *pCmdList,
                               char *pParamStr, uint32_t size);
static SHELL_ERR shell_uart(uint32_t id, char *pParamStr, uint32_t size);

/*
******************************************************************************
*    GLOBAL VARIABLE
******************************************************************************
*/
extern const SHELL_CNTX shell_peri_cmd_list[];
extern const SHELL_CNTX shell_flash_cmd_list[];
extern const SHELL_CNTX shell_cmd_product[];
extern const SHELL_CNTX shell_modem_cmd_list[];
extern uint8_t major_ver;
extern uint8_t minor_ver;

/*
******************************************************************************
*   LOCAL VARIABLES
******************************************************************************
*/
static uint8_t shell_in_buff[SHELL_BUFF_SIZE];
static uint8_t shell_in_bk_buff[SHELL_BUFF_SIZE];
static INPUT_CNTX shell_cntx;
static char shell_out_buff[SHELL_PRINT_SIZE];
static uint32_t shell_out_cnt;

static const SHELL_CNTX shell_cmd_list[] = {
    {0, "debug",
     _H("?|0-5|color|reset|dump[on|off][ami|dlms|mif|mdm|emdm|tou|sec]"),
     shell_dbg_level, NULL},
    {3, "cls", _H(""), shell_cls, NULL},
    {6, "modem", _H("imodem|emodem"), NULL, shell_modem_cmd_list},
    {7, "uart", _H("poll[on|off][port]"), shell_uart, NULL},
    {8, "flash", _H(""), NULL, shell_flash_cmd_list},
    {10, "prod", _H("get|set|act"), NULL, shell_cmd_product},
    {11, "peri", _H(""), NULL, shell_peri_cmd_list},
    {0, 0, 0, 0, 0}};

static const HOT_KEY shell_hot_key_list[] = {{CTRL_NULL, 0, 0}};

/*
******************************************************************************
*    LOCAL FUNCTIONS
******************************************************************************
*/
void shell_printf(char *fmt, ...)
{
    va_list ap;
    char *str;
    int size;

    va_start(ap, fmt);

    str = (char *)(shell_out_buff + shell_out_cnt);
    size = dsm_vsprintf(str, fmt, ap);

    shell_out_cnt += size;

    if (shell_out_cnt + 128 > SHELL_PRINT_SIZE)
    {
        dsm_uart_send(DEBUG_COM, shell_out_buff, shell_out_cnt);
        shell_out_cnt = 0;
    }

    va_end(ap);
}

static void shell_in_buff_init(INPUT_CNTX *pInBuff, uint8_t *pBuff,
                               uint32_t size)
{
    pInBuff->buffSize = size;
    pInBuff->pBuff = pBuff;
    pInBuff->wrPtr = 0;
}

char *shell_get_token_ext(char *pInStr, char *pOutToken, char sep)
{
    uint32_t i, j;
    char seperator = ' ';

    i = 0;
    j = 0;

    if (sep)
        seperator = sep;

    if (!pInStr)
    {
        if (pOutToken)
        {
            pOutToken[0] = 0;
        }
        return 0;
    }

    while (((pInStr[i] == seperator) || (pInStr[i] == ',')) && (pInStr[i] != 0))
    {
        i++;
    }

    while (((pInStr[i] != 0) && (pInStr[i] != ',')) && (pInStr[i] != seperator))
    {
        pOutToken[j] = pInStr[i];
        j++;
        i++;

        /* token size must be under 32 bytes */
        if (j >= 64)
        {
            goto ext;
        }
    }

    while (((pInStr[i] == seperator) || (pInStr[i] == ',')) && pInStr[i] != 0)
    {
        i++;
    }

    if (pInStr[i] == ' ')
        i++;

ext:
    pOutToken[j] = 0;

    return (&pInStr[i]);
}

char *shell_get_token_sep_ext(char *pInStr, char *pOutToken, char sep,
                              char deli)
{
    uint32_t i, j;
    uint8_t seperator = ' ';
    uint8_t delimiter = '/';
    uint8_t b_bypass = FALSE;

    i = 0;
    j = 0;

    if (sep)
        seperator = sep;
    if (deli)
        delimiter = deli;

    if (!pInStr)
    {
        if (pOutToken)
        {
            pOutToken[0] = 0;
        }
        return 0;
    }

    while ((pInStr[i] == seperator) && (pInStr[i] != 0))
    {
        i++;
    }

    while ((pInStr[i] != 0) && (pInStr[i] != seperator))
    {
        if (deli && pInStr[i] == delimiter)
            b_bypass = TRUE;
        else
            b_bypass = FALSE;

        if (b_bypass)
        {
            i++;
            pOutToken[j] = pInStr[i];
            j++;
            i++;
        }
        else
        {
            pOutToken[j] = pInStr[i];
            j++;
            i++;
        }

        /* token size must be under 32 bytes */
        if (j >= 512)
        {
            goto ext;
        }
    }

    while ((pInStr[i] == seperator) && pInStr[i] != 0)
    {
        i++;
    }

    if (pInStr[i] == ' ')
        i++;

ext:
    pOutToken[j] = 0;

    return (&pInStr[i]);
}

char *shell_get_token(char *pInStr, char *pOutToken)
{
    return (shell_get_token_ext(pInStr, pOutToken, ' '));
}

char *shell_get_token_sep(char *pInStr, char *pOutToken)
{
    return (shell_get_token_sep_ext(pInStr, pOutToken, ' ', FALSE));
}

void shell_get_atox(char *str, char *seperator, void *pOut)
{
    char hex_token[8];
    char *tok;
    uint32_t idx = 0, out;
    uint8_t *pOutB = (uint8_t *)pOut;

    if (!str || !pOut)
    {
        return;
    }

    tok = str;
    tok = shell_get_token_sep_ext(tok, hex_token, *seperator, FALSE);

    while (hex_token[0] != 0)
    {
        if (!str_to_hex(hex_token, &out))
            break;

        pOutB[idx] = (uint8_t)out;
        tok = shell_get_token_sep_ext(tok, hex_token, *seperator, FALSE);
        idx++;
    }
}

#if 1 /* bccho, 2023-08-31, pArgv 타입 수정, hardware fault 발생 */
void shell_arg_parse(char *pInStr, uint32_t *pArgc, char *pArgv[],
                     uint32_t maxArgc, char seperator)
#else
void shell_arg_parse(char *pInStr, uint32_t *pArgc, int pArgv[],
                     uint32_t maxArgc, char seperator)
#endif
{
    char *pStPtr = pInStr;
    uint32_t argc = 0;
    uint32_t ccnt = 0;

    while (1)
    {
        if (*pInStr == seperator || *pInStr == 0)
        {
            if (ccnt)
            {
#if 1 /* bccho, 2023-08-31 */
                pArgv[argc++] = pStPtr;
#else
                pArgv[argc++] = (int)pStPtr;
#endif
                if (argc == maxArgc)
                {
                    break;
                }
            }
            if (*pInStr == 0)
            {
                break;
            }
            *pInStr = 0;
            pStPtr = pInStr + 1;
            ccnt = 0;
        }
        else
        {
            ccnt++;
        }

        pInStr++;
    }

    *pArgc = argc;
}

static void shell_disp_rsp(SHELL_ERR err)
{
    switch ((int)err)
    {
    case SHELL_INVALID_CMD:
        SH_PRINTF("Invalid Command\r\n\r\n");
        break;

    case SHELL_INVALID_PARAM:
        SH_PRINTF("Invalid Parameter\r\n\r\n");
        break;

    case SHELL_OK:
        SH_PRINTF("OK\r\n\r\n");
        break;
    }

    SH_PRINTF(SHELL_PROMPT);
}

static void shell_handler_proc(char *buff, uint32_t size)
{
    char help_title[32];
    SHELL_ERR err = SHELL_OK;

    help_title[0] = 0;

    if (!strcmp(buff, "a/")) /* repeat command */
    {
        if (shell_in_bk_buff[0])
        {
            err = shell_handler(help_title, shell_cmd_list,
                                (char *)shell_in_bk_buff,
                                strlen((char *)shell_in_bk_buff));
        }
    }
    else
    {
        err = shell_handler(help_title, shell_cmd_list, buff, size);

        if (err == SHELL_OK)
        {
            if (strcmp(buff, "a/"))
            {
                strcpy((char *)shell_in_bk_buff, buff);
            }
        }
    }

    shell_disp_rsp(err);
}

static int shell_get_line(INPUT_CNTX *pInpBuff, char *buff, uint32_t size)
{
    uint32_t i;
    uint8_t data;
    bool bEcho;
    int ret = -1;

    for (i = 0; i < size; i++)
    {
        data = buff[i];
        bEcho = TRUE;

        if (data != 0x08 && data != 0x0d && data >= CTRL_CODE_BEGIN &&
            data <= CTRL_CODE_END)
        {
            const HOT_KEY *hKeyList = shell_hot_key_list;

            while (hKeyList->key != CTRL_NULL)
            {
                if (hKeyList->key == data)
                {
                    if (hKeyList->callback)
                    {
                        shell_disp_rsp(hKeyList->callback((CTRL_CODE)data));
                    }
                    else if (hKeyList->param)
                    {
                        shell_handler_proc(hKeyList->param,
                                           strlen(hKeyList->param));
                    }
                    break;
                }
                hKeyList++;
            }
        }
        else if (data == 0x08) /* backspace */
        {
            if (pInpBuff->wrPtr > 0)
            {
                pInpBuff->wrPtr--;
                SH_PRINTF("\x08 ");
            }
            else
            {
                bEcho = FALSE;
            }
        }
        else if (data == 0x0d) /* newline */
        {
            pInpBuff->pBuff[pInpBuff->wrPtr] = 0x00;
            i = pInpBuff->wrPtr;
            pInpBuff->wrPtr = 0;
            ret = i;
            data = '\n';
        }
        else if (data >= ' ' && data < 128) /* valid character */
        {
            pInpBuff->pBuff[pInpBuff->wrPtr] = data;
            if (pInpBuff->wrPtr < pInpBuff->buffSize - 2)
            {
                pInpBuff->wrPtr++;
            }
            else
            {
                SH_PRINTF("Too large input string..\r\n");
            }
        }

        if (bEcho)
        {
            SH_PRINTF("%c", data); /* echo */
            shell_buff_flush();
        }
    }
    return ret;
}

static SHELL_ERR shell_help(char *pCmdName, const SHELL_CNTX *pCmdList)
{
    static const char *line = "---------------------------------";

    SH_PRINTF("\r\n%s\r\n", line);

    if (pCmdName[0])
    {
        SH_PRINTF("Command [%s]\r\n", pCmdName);
    }
    SH_PRINTF("%-10s: %s\r\n", "COMMAND", "PARAM/OPTIONS");
    SH_PRINTF("%s\r\n", line);

    while (pCmdList->cmdString)
    {
        SH_PRINTF("%-10s\r\n", pCmdList->cmdString);
        pCmdList++;
    }

    SH_PRINTF("%s\r\n", line);

    return SHELL_NULL;
}

static SHELL_ERR shell_handler(char *pHelpTitle, const SHELL_CNTX *pCmdList,
                               char *pParamStr, uint32_t size)
{
    char *pTemp = pParamStr;
    char pCmdStr[32];

    pTemp = shell_get_token((char *)pTemp, pCmdStr);

    if (pCmdStr[0] == 0)
    {
        return SHELL_NULL;
    }

    if (!strcmp(pCmdStr, "help"))
    {
        return shell_help(pHelpTitle, pCmdList);
    }

    while (pCmdList->cmdString)
    {
        if (!strcmp(pCmdList->cmdString, pCmdStr))
        {
            if (pCmdList->action)
            {
                return pCmdList->action(
                    pCmdList->id, (char *)pTemp,
                    size - ((uint32_t)pTemp - (uint32_t)pParamStr));
            }
            else if (pCmdList->subCntx)
            {
                strcat(pHelpTitle, pCmdList->cmdString);
                strcat(pHelpTitle, " ");

                return shell_handler(
                    pHelpTitle, pCmdList->subCntx, (char *)pTemp,
                    size - ((uint32_t)pTemp - (uint32_t)pParamStr));
            }
        }
        pCmdList++;
    }

    return SHELL_INVALID_CMD;
}
extern void whm_data_save_sag(void);
static SHELL_ERR shell_dbg_level(uint32_t id, char *pParamStr, uint32_t size)
{
    char argv[32];
    int level;

    pParamStr = shell_get_token(pParamStr, argv);

    do
    {
        if (argv[0] == 0)
        {
            break;
        }

        if (!strcmp(argv, "?"))
        {
            pParamStr = shell_get_token(pParamStr, argv);
            if (argv[0])
            {
                break;
            }

            SH_PRINTF("\r\n%d\r\n", debug_level);
            return SHELL_OK;
        }
        else if (!strcmp(argv, "color"))
        {
            int32_t result;
            pParamStr = shell_get_token(pParamStr, argv);
            if (argv[0] == 0)
            {
                break;
            }
            result = !strcmp(argv, "on");

            pParamStr = shell_get_token(pParamStr, argv);
            if (argv[0])
            {
                break;
            }

            dsm_debug_set_color_on(result);

            return SHELL_OK;
        }
        else if (argv[0] >= '0' + DBG_CLEAR && argv[0] <= '0' + DBG_NONE)
        {
            level = atoi(argv);

            pParamStr = shell_get_token(pParamStr, argv);
            if (argv[0])
            {
                break;
            }

            if (level >= DBG_CLEAR && level <= DBG_NONE)
            {
                debug_level = level;
                return SHELL_OK;
            }
        }
        else if (!strcmp(argv, "cmp"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv, "ver"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv, "reset"))
        {
            pParamStr = shell_get_token(pParamStr, argv);
            if (argv[0])
            {
                break;
            }

            whm_data_save_sag();
#if 1 /* bccho, NVIC_SystemReset, 2023-07-15 */
            SYS_ResetChip_S();
#else
            NVIC_SystemReset();
            while (1);
#endif
        }
#if 1 /* bccho, NVIC_SystemReset, 2023-08-25 */
        else if (!strcmp(argv, "reset_cpu"))
        {
            pParamStr = shell_get_token(pParamStr, argv);
            if (argv[0])
            {
                break;
            }

            whm_data_save_sag();
            SYS_ResetCPU_S();
        }
        else if (!strcmp(argv, "goto_loader"))
        {
            pParamStr = shell_get_token(pParamStr, argv);
            if (argv[0])
            {
                break;
            }

            whm_data_save_sag();
            goto_loader_S();
        }
#endif
        else if (!strcmp(argv, "dump"))
        {
            int on, mask = 0;
            pParamStr = shell_get_token(pParamStr, argv);
            if (argv[0] == 0)
            {
                break;
            }
            if (!strcmp(argv, "on"))
            {
                on = 1;
            }
            else if (!strcmp(argv, "off"))
            {
                on = 0;
            }
            else
            {
                return SHELL_INVALID_PARAM;
            }
            do
            {
                pParamStr = shell_get_token(pParamStr, argv);

                if (!argv[0])
                {
                    break;
                }

                if (!strcmp(argv, "ami"))
                {
                    pParamStr = shell_get_token(pParamStr, argv);
                    if (argv[0])
                    {
                        break;
                    }

                    mask |= DUMP_AMI;
                }
                else if (!strcmp(argv, "dlms"))
                {
                    pParamStr = shell_get_token(pParamStr, argv);
                    if (argv[0])
                    {
                        break;
                    }

                    mask |= DUMP_DLMS;
                }
                else if (!strcmp(argv, "mif"))
                {
                    pParamStr = shell_get_token(pParamStr, argv);
                    if (argv[0])
                    {
                        break;
                    }

                    mask |= DUMP_MIF;
                }
                else if (!strcmp(argv, "mdm"))
                {
                    pParamStr = shell_get_token(pParamStr, argv);
                    if (argv[0])
                    {
                        break;
                    }

                    mask |= DUMP_MDM;
                }
                else if (!strcmp(argv, "emdm"))
                {
                    pParamStr = shell_get_token(pParamStr, argv);
                    if (argv[0])
                    {
                        break;
                    }

                    mask |= DUMP_EMDM;
                }
                else if (!strcmp(argv, "tou"))
                {
                    pParamStr = shell_get_token(pParamStr, argv);
                    if (argv[0])
                    {
                        break;
                    }

                    mask |= DUMP_TOU;
                }
                else if (!strcmp(argv, "sec"))
                {
                    pParamStr = shell_get_token(pParamStr, argv);
                    if (argv[0])
                    {
                        break;
                    }

                    mask |= DUMP_SEC;
                }
                else if (!strcmp(argv, "sf"))
                {
                    pParamStr = shell_get_token(pParamStr, argv);
                    if (argv[0])
                    {
                        break;
                    }

                    mask |= DUMP_SF;
                }
                else if (!strcmp(argv, "all"))
                {
                    pParamStr = shell_get_token(pParamStr, argv);
                    if (argv[0])
                    {
                        break;
                    }

                    mask = 0xFFFFFFFF;
                }
            } while (1);

            dsm_debug_set_dump_mask(mask, on);

            return SHELL_OK;
        }
    } while (0);

    return SHELL_INVALID_PARAM;
}

static SHELL_ERR shell_cls(uint32_t id, char *pParamStr, uint32_t size)
{
    char buff[32], *ptr = buff;

    ptr += ansi_begin(ptr);
    ptr += ansi_clear_screen(ptr);
    ptr += ansi_end(ptr);
    ansi_flush(buff, ptr - buff);

    return SHELL_OK;
}

static SHELL_ERR shell_uart(uint32_t id, char *pParamStr, uint32_t size)
{
    char *argv[8];
    uint32_t argc;
    uint32_t port = DEBUG_COM;

    if (size < 4)
    {
        return SHELL_INVALID_PARAM;
    }

    shell_arg_parse(pParamStr, &argc, argv, 5, ' ');

    if (argc < 2)
    {
        return SHELL_INVALID_PARAM;
    }

    if (!strcmp(argv[0], "poll"))
    {
        return SHELL_OK;
    }

    return SHELL_INVALID_PARAM;
}

/*
******************************************************************************
*    GLOBAL FUNCTIONS
******************************************************************************
*/
bool str_to_hex(char *str, uint32_t *value)
{
    uint32_t i = 0;
    uint32_t ret = 0;

    while (str[i] && i < 8)
    {
        ret <<= 4;

        if (str[i] >= '0' && str[i] <= '9')
        {
            ret |= str[i] - '0';
        }
        else if (str[i] >= 'A' && str[i] <= 'F')
        {
            ret |= str[i] - 'A' + 10;
        }
        else if (str[i] >= 'a' && str[i] <= 'f')
        {
            ret |= str[i] - 'a' + 10;
        }
        else
        {
            return FALSE;
        }
        i++;
    }
    *value = ret;
    return TRUE;
}

#if 1 /* bccho, 2023-08-09, 1byte 변환시 hw fault 발생하여 아래 함수 생성 */
bool str_to_hex_uint8(const char *str, uint8_t *value)
{
    uint8_t i = 0;
    uint8_t ret = 0;

    while (i < 2 && str[i])
    {
        ret <<= 4;

        if (str[i] >= '0' && str[i] <= '9')
        {
            ret |= str[i] - '0';
        }
        else if (str[i] >= 'A' && str[i] <= 'F')
        {
            ret |= str[i] - 'A' + 10;
        }
        else if (str[i] >= 'a' && str[i] <= 'f')
        {
            ret |= str[i] - 'a' + 10;
        }
        else
        {
            return FALSE;
        }
        i++;
    }

    *value = ret;

    return TRUE;
}
#endif

bool str_to_hex_n(char *str, uint8_t *value, uint32_t cnt)
{
    uint32_t i = 0;
    uint8_t ret = 0;

    while (str[i] && i < cnt)
    {
        ret <<= 4;

        if (str[i] >= '0' && str[i] <= '9')
        {
            ret |= str[i] - '0';
        }
        else if (str[i] >= 'A' && str[i] <= 'F')
        {
            ret |= str[i] - 'A' + 10;
        }
        else if (str[i] >= 'a' && str[i] <= 'f')
        {
            ret |= str[i] - 'a' + 10;
        }
        else
        {
            return FALSE;
        }

        i++;

        if (i % 2 == 0)
        {
            value[(i >> 1) - 1] = ret;
            ret = 0;
        }
    }

    return TRUE;
}

#if 1  // jp.kim 24.11.13
uint8_t str_to_bin32_n(char *str, uint32_t *value, uint32_t cnt)
{
    uint32_t i = 0;
    uint32_t k = 0;
    uint32_t ret = 0;
    bool f_data_in = 0;

    DPRINTF(DBG_ERR, " 1 str_to_bin32_n: cnt[%d]\r\n", cnt);

    while (str[i] && i < cnt)
    {
        if (str[i] >= '0' && str[i] <= '9')
        {
            ret += str[i] - '0';
            ret *= 10;
            DPRINTF(DBG_ERR, " 2 str_to_bin32_n: ret[%d] k[%d]\r\n", ret, k);
        }
        else if (str[i] == 'F')
        {
            f_data_in = 1;
            ret |= str[i] - 'A' + 10;
            ret <<= 4;
            DPRINTF(DBG_ERR, " 3 str_to_bin32_n: ret[%d] k[%d]\r\n", ret, k);
        }
        else if (str[i] == 'f')
        {
            f_data_in = 1;
            ret |= str[i] - 'a' + 10;
            ret <<= 4;
            DPRINTF(DBG_ERR, " 7 str_to_bin32_n: ret[%d] k[%d]\r\n", ret, k);
        }
        else if ((str[i] == ',') || (str[i] == '.'))
        {
            if (f_data_in)
                ret >>= 4;
            else
                ret /= 10;
            value[k] = ret;
            ret = 0;
            DPRINTF(DBG_ERR,
                    " 4 str_to_bin32_n: ret[%d] k[%d]  value[k][%d]\r\n", ret,
                    k, value[k]);
            if ((value[k] != 0xffff) && (value[k] != 0xffffffff))
                k++;

            f_data_in = 0;
        }
        else
        {
            DPRINTF(DBG_ERR,
                    " 5 str_to_bin32_n: ret[%d] k[%d]  value[k][%d]\r\n", ret,
                    k, value[k]);
            return k;
        }

        i++;

        DPRINTF(DBG_ERR,
                " 6 str_to_bin32_n: ret[%d] k[%d]  value[k][%d] i[%d]\r\n", ret,
                k, value[k], i);
    }

    DPRINTF(DBG_ERR, " 7 str_to_bin32_n: ret[%d] k[%d]  value[k][%d]\r\n", ret,
            k, value[k]);

    return k;
}
#endif

uint32_t get_hex_array_from_str(char *str, char *sperator, void *pOut,
                                uint32_t outUnitByteSize, uint32_t maxOutCount)
{
    char *tok;
    uint32_t idx = 0, out;
    uint8_t *pOutB = (uint8_t *)pOut;

    if (!str || !pOut)
    {
        return 0;
    }

    tok = strtok(str, sperator);

    while (maxOutCount-- && tok && *tok != 0)
    {
        if (!str_to_hex(tok, &out))
        {
            return 0;
        }

        switch (outUnitByteSize)
        {
        case 1:
            pOutB[idx] = (uint8_t)out;
            break;
        case 2:
            *(uint16_t *)&pOutB[idx << 1] = (uint16_t)out;
            break;

        case 4:
            *(uint32_t *)&pOutB[idx << 2] = out;
            break;

        default:
            DPRINTF(DBG_ERR, "Invalid Size = %d\r\n", outUnitByteSize);
            ASSERT(0);
        }

        tok = strtok(NULL, sperator);
        idx++;
    }

    return idx;
}

void shell_buff_flush(void)
{
    if (shell_out_cnt)
    {
        dsm_uart_send(DEBUG_COM, shell_out_buff, shell_out_cnt);
        shell_out_cnt = 0;
    }
}

void shell_init(void)
{
    shell_in_buff_init(&shell_cntx, shell_in_buff, SHELL_BUFF_SIZE);
}

void shell_main(char *str, int size)
{
    int i, retSize;

    shell_out_cnt = 0;

    if (!size)
    {
        return;
    }

    for (i = 0; i < size; i++)
    {
        retSize = shell_get_line(&shell_cntx, &str[i], 1);

        if (retSize >= 0)
        {
            shell_handler_proc((char *)shell_cntx.pBuff, retSize);
            shell_buff_flush();
        }
    }

    shell_buff_flush();
}
