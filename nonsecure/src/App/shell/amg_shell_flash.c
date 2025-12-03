/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_shell.h"
#include "amg_ansi.h"
#include "amg_uart.h"
#include "amg_crc.h"
#include "mx25r4035f.h"
#include "mx25r4035f_def.h"
#include "amg_wdt.h"
#include "flash_if.h"
#include "amg_crc.h"
#include "amg_imagetransfer.h"
#include "amg_spi.h"
/*
******************************************************************************
*     LOCAL CONSTANTS
******************************************************************************
*/

#define FLASH_BUFF_SIZE 4096

#define XMODEM_SOH 0x01
#define XMODEM_STX 0x02
#define XMODEM_EOT 0x04
#define XMODEM_ACK 0x06
#define XMODEM_NAK 0x15
#define XMODEM_CAN 0x18
#define XMODEM_ESC 0x1b
#define XM_PKSZ_128 128
#define XM_PKSZ_1024 1024
#define XN_REQ_CHAR 'C'

/*
******************************************************************************
*    LOCAL DATA TYPES
******************************************************************************
*/

static SHELL_ERR shell_flash_read(uint32_t id, char *pParamStr, uint32_t size);
static SHELL_ERR shell_flash_erase(uint32_t id, char *pParamStr, uint32_t size);
static SHELL_ERR shell_flash_program(uint32_t id, char *pParamStr,
                                     uint32_t size);
static SHELL_ERR shell_flash_select(uint32_t id, char *pParamStr,
                                    uint32_t size);
static SHELL_ERR shell_flash_fwinfo(uint32_t id, char *pParamStr,
                                    uint32_t size);
static SHELL_ERR shell_flash_fwinfo_erase(uint32_t id, char *pParamStr,
                                          uint32_t size);

const SHELL_CNTX shell_flash_cmd_list[] = {
    {2, "read", _H(""), shell_flash_read, NULL},
    {3, "erase", _H(""), shell_flash_erase, NULL},
    {4, "program", _H(""), shell_flash_program, NULL},
    {5, "select", _H(""), shell_flash_select, NULL},
    {6, "fwinfo", _H(""), shell_flash_fwinfo, NULL},
    {10, "fw_erase", _H(""), shell_flash_fwinfo_erase, NULL},

    {0, 0, 0, 0, 0}};

/*
******************************************************************************
*    LOCAL VARIABLES
******************************************************************************
*/
uint32_t g_bank_offset = SFLASH_SYS_FW_1_ADDR;

/*
******************************************************************************
*    LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

bool xmodem_flash_update(uint32_t addr, uint8_t *buf, uint32_t len)
{
    ReturnMsg ret;

    if ((addr & 0xFFF) == 0)
    {
        if (CMD_SE(addr))
        {
            SH_PRINTF("ERROR: flash erase   fail. 0x%08X\r\n", addr);
            return FALSE;
        }
    }

    while (len > 0)
    {
        uint32_t bytes = len > Page_Offset ? Page_Offset : len;
        ret = CMD_PP(addr, buf, bytes);
        if (ret)
        {
            SH_PRINTF("ERROR: flash program fail. 0x%08X\r\n", addr);
            return FALSE;
        }

        addr += bytes;
        buf += bytes;
        len -= bytes;
    }
    return TRUE;
}

#if 1 /* bccho, 2023-09-30 */
static void XMD_putc(uint8_t c)
{
    UART_T *pUART = UART0;

    while (pUART->FIFOSTS & UART_FIFOSTS_TXFULL_Msk);
    pUART->DAT = c;
}

static int32_t XMD_getc()
{
    UART_T *pUART = UART0;

    // SysTick->LOAD = 100000 * CyclesPerUs;
    // SysTick->VAL = (0x00);
    // SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    /* Waiting for down-count to zero */

    uint32_t pre_time = OS_TIME_GET(), timeout = MS2TIMER(200);

    while (pUART->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk)
    {
        if (OS_TIME_GET() - pre_time >= timeout)
            return -1;
        // if ((SysTick->CTRL & (1 << 16)) != 0)
        // {
        //     SysTick->CTRL = 0;
        //     return -1;  // timeout
        // }
    }
    // SysTick->CTRL = 0;

    return ((int32_t)pUART->DAT);
}

static uint16_t crc16_ccitt(const uint8_t *pu8buf, int32_t i32len)
{
    uint16_t crc = 0;

    while (i32len--)
    {
        int32_t i;
        crc ^= *pu8buf++ << 8;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 0x8000)
                crc = (uint16_t)(crc << 1) ^ (uint16_t)0x1021;
            else
                crc = (uint16_t)(crc << 1);
        }
    }

    return crc;
}

static int32_t check(int32_t iscrc, const uint8_t *pu8buf, int32_t i32Size)
{
    if (iscrc)
    {
        uint16_t crc = crc16_ccitt(pu8buf, i32Size);
        uint16_t tcrc =
            (uint16_t)(pu8buf[i32Size] << 8) + (uint16_t)pu8buf[i32Size + 1];
        if (crc == tcrc)
            return 1;
    }
    else
    {
        int32_t i;
        uint8_t tsum = 0;
        for (i = 0; i < i32Size; ++i) tsum += pu8buf[i];
        if (tsum == pu8buf[i32Size])
            return 1;
    }

    return 0;
}

/* Xmodem Standard Commands */
#define XMD_SOH 0x01
#define XMD_STX 0x02
#define XMD_EOT 0x04
#define XMD_ACK 0x06
#define XMD_NAK 0x15
#define XMD_CAN 0x18
#define XMD_CTRLZ 0x1A
#define XMD_MAX_TIMEOUT 0x600

/* Xmodem Status */
#define XMD_STS_SUCCESS 0
#define XMD_STS_USER_CANCEL -1
#define XMD_STS_NAK -2
#define XMD_STS_TIMEOUT -3
#define XMD_STS_PACKET_NUM_ERR -4
#define XMD_STS_WRITE_FAIL -5

#define MAXRETRANS 25
#define XMD_MAX_TRANS_SIZE (1024 * 1024)

/* 1024 for XModem 1k + 3 head chars + 2 crc + nul */
static uint8_t s_au8XmdBuf[1030];
static int xmodem_receive(uint32_t u32DestAddr)
{
    int32_t i32Err = 0;
    uint8_t *p;
    int32_t bufsz, crc = 0;
    uint8_t trychar = 'C';
    uint8_t packetno = 1;
    int32_t i, j;
    int32_t retrans = MAXRETRANS;
    int32_t i32TransBytes = 0;
    int32_t ch;
    uint32_t u32StarAddr, u32Data;

    for (;;)
    {
        for (i = 0; i < XMD_MAX_TIMEOUT; ++i) /* set timeout period */
        {
            if (trychar)
                XMD_putc(trychar);

            ch = XMD_getc();
            if (ch >= 0)
            {
                switch (ch)
                {
                case XMD_SOH:
                    bufsz = 128;
                    goto START_RECEIVE;

                case XMD_STX:
                    bufsz = 1024;
                    goto START_RECEIVE;

                case XMD_EOT:
                    XMD_putc(XMD_ACK);
                    return (i32Err == 0) ? i32TransBytes
                                         : i32Err; /* normal end */

                case XMD_CAN:
                    XMD_putc(XMD_ACK);
                    return XMD_STS_USER_CANCEL; /* canceled by remote */
                default:
                    break;
                }
            }
        }

        if (trychar == 'C')
        {
            XMD_putc(XMD_CAN);
            XMD_putc(XMD_CAN);
            XMD_putc(XMD_CAN);
            return XMD_STS_TIMEOUT; /* too many retry error */
        }
        XMD_putc(XMD_CAN);
        XMD_putc(XMD_CAN);
        XMD_putc(XMD_CAN);
        return XMD_STS_NAK; /* sync error */

    START_RECEIVE:
        if (trychar == 'C')
            crc = 1;
        trychar = 0;
        p = s_au8XmdBuf;
        *p++ = (uint8_t)ch;
        for (i = 0; i < (bufsz + (crc ? 1 : 0) + 3); ++i)
        {
            ch = XMD_getc();

            if (ch < 0)
                goto REJECT_RECEIVE;
            *p++ = (char)ch;
        }

        if (s_au8XmdBuf[1] != packetno)
        {
            XMD_putc(XMD_CAN);
            XMD_putc(XMD_CAN);
            XMD_putc(XMD_CAN);
            return XMD_STS_PACKET_NUM_ERR;
        }
        else
        {
            if (((s_au8XmdBuf[1] + s_au8XmdBuf[2]) == 0xFF) &&
                check(crc, &s_au8XmdBuf[3], bufsz))
            {
                if (s_au8XmdBuf[1] == packetno)
                {
                    volatile int32_t count = XMD_MAX_TRANS_SIZE - i32TransBytes;
                    if (count > bufsz)
                        count = bufsz;
                    if (count > 0)
                    {
                        for (j = 0; j < (bufsz + 3) / 4; j++)
                        {
                            memcpy((uint8_t *)&u32Data,
                                   &s_au8XmdBuf[3 + (j * 0x4)], 4);

                            u32StarAddr = u32DestAddr + (uint32_t)i32TransBytes;

                            i32Err = XMD_Write_S(
                                u32StarAddr + ((uint32_t)j * 0x4), u32Data);

                            if (i32Err < 0)
                                continue;
                        }
                        i32TransBytes += count;
                    }
                    ++packetno;
                    retrans = MAXRETRANS + 1;
                }
                if (--retrans <= 0)
                {
                    XMD_putc(XMD_CAN);
                    XMD_putc(XMD_CAN);
                    XMD_putc(XMD_CAN);
                    return XMD_STS_TIMEOUT; /* too many retry error */
                }
                XMD_putc(XMD_ACK);
                continue;
            }
        }

    REJECT_RECEIVE:
        XMD_putc(XMD_NAK);
    }
}

#else
static int xmodem_receive(uint32_t flash_offset)
{
#define XMODEM_TIMEOUT 1000
#define XMODEM_GETC(port, a)                                              \
    {                                                                     \
        if (!dsm_uart_raw_getc_timeout(port, XMODEM_TIMEOUT, (char *)&a)) \
            goto error;                                                   \
    }

    uint16_t i = 0;
    uint8_t ch = 0, ctrlCode = XN_REQ_CHAR;
    uint16_t calcCrc = 0, crc;
    uint32_t totalSize = 0, pktSize = 0;
    uint8_t *buf;
    uint32_t prev_sector = 0xFFFF;
    uint32_t write_result = FLASHIF_OK;
    uint8_t cnt = 0;
    uint32_t pre_time = OS_TIME_GET();

    buf = pvPortMalloc(1024);
    if (!buf)
    {
        DPRINTF(DBG_ERR, "xmodem buf alloc failed\r\n");
        return FALSE;
    }

    dsm_uart_set_poll_mode(DEBUG_COM, TRUE);

    if (flash_offset == BOOTLOADER_INT_FLASH_ADDRESS)
    {
        FLASH_If_WriteProtectionConfig(FLASHIF_WRP_DISABLE, FLASH_BANK_1);

        if (flash_offset == (flash_offset + totalSize))
        {
            prev_sector = FLASH_If_GetPage((flash_offset + totalSize));

            cnt = 0;
            while (1)
            {
                if (FLASHIF_OK ==
                    FLASH_If_Erase_selected((flash_offset + totalSize), 1))
                    break;
                if (cnt++ >= 3)
                    break;
            }
            if (cnt >= 3)
            {
                DPRINTF(DBG_ERR, "!!!  Erase failed 1[ 0x%08X ]!!!\r\n",
                        (flash_offset + totalSize));
                goto error;
            }
        }
    }
    else
    {
        CMD_SE((flash_offset + totalSize));
    }

    while (1)
    {
        dsm_wdt_ext_toggle();

        dsm_uart_raw_putc(DEBUG_COM, ctrlCode);

        if (ctrlCode == XMODEM_ACK)
        {
            XMODEM_GETC(DEBUG_COM, ch);
        }
        else
        {
            ch = 0;

            dsm_uart_raw_getc_timeout(DEBUG_COM, 200, (char *)&ch);

            if (ctrlCode == XMODEM_NAK &&
                OS_TIME_GET() - pre_time > MS2TIMER(1000))
            {
                DPRINTF(DBG_ERR, "XMODEM Timeout !\r\n");
                goto error;
            }
        }

        if (ch == XMODEM_EOT)
        {
            dsm_uart_raw_putc(DEBUG_COM, XMODEM_ACK);
            break;
        }

        if (ch == XMODEM_CAN)
        {
            DPRINTF(DBG_ERR, "!!!  Canceled !!!\r\n");
            goto error;
        }

        if ((ch == XMODEM_SOH) || (ch == XMODEM_STX))
        {
            if (ch == XMODEM_SOH)
            {
                pktSize = XM_PKSZ_128;
            }
            else if (ch == XMODEM_STX)
            {
                pktSize = XM_PKSZ_1024;
            }

            /* ignore block number */
            XMODEM_GETC(DEBUG_COM, ch);
            XMODEM_GETC(DEBUG_COM, ch);

            for (i = 0; i < pktSize; i++)
            {
                XMODEM_GETC(DEBUG_COM, buf[i]);
            }

            XMODEM_GETC(DEBUG_COM, ch);
            crc = ch << 8;
            XMODEM_GETC(DEBUG_COM, ch) crc |= ch;

            calcCrc = crc16_ccitt_get(buf, pktSize);

            if (calcCrc == crc)
            {
                if (flash_offset == BOOTLOADER_INT_FLASH_ADDRESS)
                {
                    write_result = FLASHIF_OK;

                    if (flash_offset == (flash_offset + totalSize))
                    {
                        prev_sector =
                            FLASH_If_GetPage((flash_offset + totalSize));

                        cnt = 0;
                        while (1)
                        {
                            if (FLASHIF_OK ==
                                FLASH_If_Erase_selected(
                                    (flash_offset + totalSize), 1))
                                break;
                            if (cnt++ >= 3)
                                break;
                        }
                        if (cnt >= 3)
                        {
                            DPRINTF(DBG_ERR,
                                    "!!!  Erase failed 1[ 0x%08X ]!!!\r\n",
                                    (flash_offset + totalSize));
                            goto error;
                        }
                    }

                    if (prev_sector !=
                        FLASH_If_GetPage((flash_offset + totalSize)))
                    {
                        cnt = 0;
                        while (1)
                        {
                            if (FLASHIF_OK ==
                                FLASH_If_Erase_selected(
                                    (flash_offset + totalSize), 1))
                                break;
                            if (cnt++ >= 3)
                                break;
                        }
                        if (cnt >= 3)
                        {
                            DPRINTF(DBG_ERR,
                                    "!!!  Erase failed 2[ 0x%08X ]!!!\r\n",
                                    (flash_offset + totalSize));
                            goto error;
                        }

                        prev_sector =
                            FLASH_If_GetPage((flash_offset + totalSize));
                    }

                    cnt = 0;
                    while (1)
                    {
                        if (FLASHIF_OK ==
                            FLASH_If_Write((flash_offset + totalSize),
                                           (uint32_t *)buf, pktSize / 4))
                            break;
                        if (cnt++ >= 3)
                            break;
                    }
                    if (cnt >= 3)
                    {
                        write_result = FLASHIF_WRITING_ERROR;
                        goto error;
                    }

                    totalSize += pktSize;
                }
                else
                {
                    xmodem_flash_update(flash_offset + totalSize, buf, pktSize);
                    totalSize += pktSize;
                }

                ctrlCode = XMODEM_ACK;
            }
            else
            {
                ctrlCode = XMODEM_NAK;
            }
            calcCrc = 0;
            pre_time = OS_TIME_GET();
        }
    }

    vPortFree(buf);

    DPRINTF(DBG_ERR, "upgrade completed, size=%d\r\n", totalSize);

    return TRUE;

error:
    vPortFree(buf);

    DPRINTF(DBG_ERR, "upgrade failed size=%d, write_result=%d\r\n", totalSize,
            write_result);
    return FALSE;
}
#endif

static SHELL_ERR shell_flash_select(uint32_t id, char *pParamStr, uint32_t size)
{
    char *argv[2];
    uint32_t argc;
    uint32_t dest, offset;

    if (size != 1)
    {
        return SHELL_INVALID_PARAM;
    }

    shell_arg_parse(pParamStr, &argc, argv, 2, ' ');

    dest = atoi(argv[0]);
    if (dest == 1)
    {
        offset = SFLASH_SYS_FW_1_ADDR;
    }
    else if (dest == 2)
    {
        offset = SFLASH_SYS_FW_2_ADDR;
    }
    else if (dest == 3)
    {
        offset = SFLASH_I_MODEM_FW_ADDR;
    }
    else if (dest == 4)
    {
        offset = SFLASH_EXT_MODEM_FW_ADDR;
    }
    else if (dest == 5)
    {
        offset = SFLASH_METER_FW_BLK_ADDR;
    }
    else if (dest == 0)
    {
        offset = BOOTLOADER_INT_FLASH_ADDRESS;
    }
    else
    {
        return SHELL_INVALID_PARAM;
    }

    DPRINTF(DBG_ERR, "Change base from 0x%08X to 0x%08X\r\n", g_bank_offset,
            offset);

    g_bank_offset = offset;

    return SHELL_OK;
}

static SHELL_ERR shell_flash_read(uint32_t id, char *pParamStr, uint32_t size)
{
    char *argv[2];
    uint32_t argc;

    uint32_t addr, len;
    uint8_t *buf;
#if 0 /* bccho, 2023-08-02 */    
    uint8_t buf_2[1024];
#endif

    shell_arg_parse(pParamStr, &argc, argv, 2, ' ');
    if (argc != 2)
        return SHELL_INVALID_PARAM;

    str_to_hex(argv[0], &addr);
    len = atoi(argv[1]);

    if (addr > FlashSize || addr & 0x3)
        return SHELL_INVALID_PARAM;

    if (len > Sector_Offset)
        return SHELL_INVALID_PARAM;

    buf = pvPortMalloc(len);
    memset(buf, 0, len);

    ASSERT(buf);

    CMD_READ(addr, buf, len);

    DPRINT_HEX(DBG_CLEAR, "FLASH READ", buf, len, DUMP_ALWAYS);

    vPortFree(buf);

    return SHELL_OK;
}

static SHELL_ERR shell_flash_erase(uint32_t id, char *pParamStr, uint32_t size)
{
    char *argv[1];
    uint32_t argc;
    uint32_t addr;

    shell_arg_parse(pParamStr, &argc, argv, 1, ' ');
    if (argc != 1)
        return SHELL_INVALID_PARAM;

    str_to_hex(argv[0], &addr);

    if (addr > FlashSize || addr & 0x3)
        return SHELL_INVALID_PARAM;

    SH_PRINTF("FLASH SE: addr(0x%08X)\r\n\r\n", addr);

    CMD_SE(addr);

    return SHELL_OK;
}

static SHELL_ERR shell_flash_program(uint32_t id, char *pParamStr,
                                     uint32_t size)
{
    char *argv[2];
    uint32_t argc;

    uint32_t addr, len;
    uint8_t data[256];
    uint16_t i;
    uint32_t bytes;
    ReturnMsg ret;

    shell_arg_parse(pParamStr, &argc, argv, 2, ' ');
    if (argc != 2)
        return SHELL_INVALID_PARAM;

    str_to_hex(argv[0], &addr);
    len = atoi(argv[1]);

    if (addr > FlashSize || addr & 0x3)
        return SHELL_INVALID_PARAM;

    if (len > Sector_Offset)
        return SHELL_INVALID_PARAM;

    SH_PRINTF("FLASH PP: addr(0x%X), len(%d)\r\n", addr, len);

    for (i = 0; i < sizeof(data); i++) data[i] = (uint8_t)i;

    while (len > 0)
    {
        bytes = len > Page_Offset ? Page_Offset : len;
        ret = CMD_PP(addr, data, bytes);

        SH_PRINTF("addr(0x%X), bytes(%d), len(%d), ret(%d)\r\n", addr, bytes,
                  len, ret);
        addr += bytes;
        len -= bytes;
    }
    return SHELL_OK;
}

uint32_t dsm_get_max_image_size(uint32_t fs_bank)
{
    uint32_t size = 0;

    switch (fs_bank)
    {
    case INT_FLASH_BANK_SELECT_0:
        size = (Block_Offset * SFLASH_SYS_FW_1_BLK_CNT);

        break;
    case E_SFLASH_SYS_FW_1_T:
        size = (Block_Offset * SFLASH_SYS_FW_1_BLK_CNT);

        break;
    case E_SFLASH_SYS_FW_2_T:
        size = (Block_Offset * SFLASH_SYS_FW_2_BLK_CNT);

        break;
    case E_SFLASH_I_MODEM_FW_T:
        size = (Block_Offset * SFLASH_I_MODEM_FW_BLK_CNT);

        break;
    case E_SFLASH_EXT_MODEM_FW_T:
        size = (Block_Offset * SFLASH_EXT_MODEM_FW_BLK_CNT);

        break;
    case E_SFLASH_METER_FW_BLK_T:
        size = (Block_Offset * SFLASH_METER_FW_BLK_CNT);

        break;
    }

    return size;
}

uint32_t fw_info_get(uint32_t fs_bank, fw_info_t *pfwinfo)
{
    uint32_t crc_val = 0;
    uint32_t src_addr = 0;
    uint32_t img_size;
    //    memset(pfwinfo, 0, sizeof(fw_info_t));

    DPRINTF(DBG_TRACE, "\r\n\r\nFW_INFO[%d]\r\n", fs_bank);
    img_size = dsm_get_max_image_size(fs_bank);

    if (fs_bank == INT_FLASH_BANK_SELECT_0)
    { /*int.flash bank 1*/
        src_addr = APPLICATION_BASE_ADDRESS;
        // DPRINTF(DBG_TRACE, "type[0], start_addr[0x%08X]\r\n", src_addr);
    }
    else
    { /*ext.flash*/
        src_addr = dsm_sflash_fw_get_startaddr(fs_bank);
    }

    if (fs_bank == INT_FLASH_BANK_SELECT_0)
    { /*int.flash bank 1*/
        FLASH_If_Read((uint8_t *)(src_addr), (uint8_t *)pfwinfo,
                      sizeof(fw_info_t));
    }
    else
    { /*ext.flash*/
        CMD_READ(src_addr, (uint8_t *)pfwinfo, sizeof(fw_info_t));
    }

    DPRINTF(DBG_TRACE, "img_max_size[%X]\r\n", img_size);
    DPRINTF(DBG_TRACE, "fw_signature[%X], fw_size[%X], fw_crc[%X]\r\n",
            pfwinfo->fw_signature, pfwinfo->fw_size, pfwinfo->fw_crc);
    DPRINT_HEX(DBG_TRACE, "ver_rel", pfwinfo->ver_rel, REL_NAME_SIZE,
               DUMP_ALWAYS);

    if (pfwinfo->fw_size > img_size ||
        pfwinfo->fw_signature != FW_INFO_APP_SIGNATURE)
    {
        memset(pfwinfo, 0, sizeof(fw_info_t));
        return 0xFFFFFFFF;
    }
    if (fs_bank == INT_FLASH_BANK_SELECT_0)
    { /*int.flash bank 1*/
        crc_val = (uint32_t)crc16_ccitt_get(
            (uint8_t *)(src_addr + APPLICATION_FW_HEADROOM), pfwinfo->fw_size);
    }
    else
    {
        crc_val = (uint32_t)extflash_crc16_ccitt(
            (src_addr + APPLICATION_FW_HEADROOM), pfwinfo->fw_size);
    }

    DPRINTF(DBG_TRACE, "\r\n");

    return crc_val;
}

void fw_info_print(fw_info_t *p_fwinfo, uint32_t crc_val)
{
    if (p_fwinfo->fw_size == 0xFFFFFFFF || crc_val == 0xFFFFFFFF)
    {
        DPRINTF(DBG_TRACE, "Flash Empty\r\n");
        return;
    }
    DPRINTF(DBG_TRACE, "Size: %d\r\n", p_fwinfo->fw_size);
    DPRINTF(DBG_TRACE, "Version: %d\r\n", p_fwinfo->fw_ver);
    DPRINTF(DBG_TRACE, "Release: %c%c%c%c%c%c%c%c%c%c%c%c%c%c\r\n",
            p_fwinfo->ver_rel[0], p_fwinfo->ver_rel[1], p_fwinfo->ver_rel[2],
            p_fwinfo->ver_rel[3], p_fwinfo->ver_rel[4], p_fwinfo->ver_rel[5],
            p_fwinfo->ver_rel[6], p_fwinfo->ver_rel[7], p_fwinfo->ver_rel[8],
            p_fwinfo->ver_rel[9], p_fwinfo->ver_rel[10], p_fwinfo->ver_rel[11],
            p_fwinfo->ver_rel[12], p_fwinfo->ver_rel[13]);

    DPRINTF(DBG_TRACE, "CRC: 0x%04X, 0x%04X\r\n", crc_val, p_fwinfo->fw_crc);
}

static SHELL_ERR shell_flash_fwinfo(uint32_t id, char *pParamStr, uint32_t size)
{
    fw_info_t fw_info_int_flash_img, fw_info_sf_img_1, fw_info_sf_img_2;
    uint32_t crc_val;

    switch (id)
    {
    case 6:
        crc_val = fw_info_get(INT_FLASH_BANK_SELECT_0, &fw_info_int_flash_img);
        fw_info_print(&fw_info_int_flash_img, crc_val);

        crc_val = fw_info_get(E_SFLASH_SYS_FW_1_T, &fw_info_sf_img_1);
        fw_info_print(&fw_info_sf_img_1, crc_val);

        crc_val = fw_info_get(E_SFLASH_SYS_FW_2_T, &fw_info_sf_img_2);
        fw_info_print(&fw_info_sf_img_2, crc_val);
        return SHELL_OK;
        break;
    }

    return SHELL_INVALID_PARAM;
}

static SHELL_ERR shell_flash_fwinfo_erase(uint32_t id, char *pParamStr,
                                          uint32_t size)
{
    char *argv[10];
    uint32_t argc;
    fw_info_t fw_info_bank1;
    fw_info_t fw_info_bank2;
    uint32_t crc_val;
    uint32_t sf_addr_dest;

    if (size < 4)
    {
        return SHELL_INVALID_PARAM;
    }

    shell_arg_parse(pParamStr, &argc, argv, 1, ' ');

    switch (id)
    {
    case 10:
    {
        if (!strcmp(argv[0], "sf_1"))
        {
            sf_addr_dest = dsm_sflash_fw_get_startaddr(E_SFLASH_SYS_FW_1_T);

            CMD_SE(sf_addr_dest);

            DPRINTF(DBG_TRACE, "FW Info (SF_1)\r\n ");
            crc_val = fw_info_get(1, &fw_info_bank1);
            fw_info_print(&fw_info_bank1, crc_val);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "sf_2"))
        {
            sf_addr_dest = dsm_sflash_fw_get_startaddr(E_SFLASH_SYS_FW_2_T);

            CMD_SE(sf_addr_dest);

            DPRINTF(DBG_TRACE, "FW Info (SF_2)\r\n ");
            crc_val = fw_info_get(2, &fw_info_bank2);
            fw_info_print(&fw_info_bank2, crc_val);

            return SHELL_OK;
        }
    }
    break;
    }

    return SHELL_INVALID_PARAM;
}
