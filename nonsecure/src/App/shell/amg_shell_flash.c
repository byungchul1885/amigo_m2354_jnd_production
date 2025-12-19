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
#include "nsclib.h"
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

static SHELL_ERR shell_flash_read(uint32_t id, char* pParamStr, uint32_t size);
static SHELL_ERR shell_flash_erase(uint32_t id, char* pParamStr, uint32_t size);
static SHELL_ERR shell_flash_program(uint32_t id, char* pParamStr,
                                     uint32_t size);
static SHELL_ERR shell_flash_select(uint32_t id, char* pParamStr,
                                    uint32_t size);
static SHELL_ERR shell_flash_fwinfo(uint32_t id, char* pParamStr,
                                    uint32_t size);
static SHELL_ERR shell_flash_fwinfo_erase(uint32_t id, char* pParamStr,
                                          uint32_t size);
static SHELL_ERR shell_flash_ota_rw(uint32_t id, char* pParamStr,
                                    uint32_t size);
static SHELL_ERR shell_flash_ota_crc(uint32_t id, char* pParamStr,
                                     uint32_t size);

const SHELL_CNTX shell_flash_cmd_list[] = {
    {2, "read", _H(""), shell_flash_read, NULL},
    {3, "erase", _H(""), shell_flash_erase, NULL},
    {4, "program", _H(""), shell_flash_program, NULL},
    {5, "select", _H(""), shell_flash_select, NULL},
    {6, "fwinfo", _H(""), shell_flash_fwinfo, NULL},
    {10, "fw_erase", _H(""), shell_flash_fwinfo_erase, NULL},
    {11, "ota_rw", _H("ota_rw <addr_hex> <len_dec> [seed_hex]"),
     shell_flash_ota_rw, NULL},
    {12, "ota_crc",
     _H("ota_crc <addr_hex> <len_dec> [seed_hex]  (len includes 4-byte CRC32)"),
     shell_flash_ota_crc, NULL},

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

static SHELL_ERR shell_flash_select(uint32_t id, char* pParamStr, uint32_t size)
{
    return SHELL_OK;
}

static SHELL_ERR shell_flash_read(uint32_t id, char* pParamStr, uint32_t size)
{
    return SHELL_OK;
}

static SHELL_ERR shell_flash_erase(uint32_t id, char* pParamStr, uint32_t size)
{
    return SHELL_OK;
}

static SHELL_ERR shell_flash_program(uint32_t id, char* pParamStr,
                                     uint32_t size)
{
    return SHELL_OK;
}

static SHELL_ERR shell_flash_ota_rw(uint32_t id, char* pParamStr, uint32_t size)
{
    char* argv[3];
    uint32_t argc;
    uint32_t addr;
    uint32_t len;
    uint32_t seed = 0;
    uint8_t status;
    uint32_t offset = 0;
    uint32_t mismatch = 0xFFFFFFFFU;
    uint32_t chunk;
    uint32_t i;

    enum
    {
        OTA_RW_CHUNK_SIZE = 4096U
    };
    static uint8_t buf[OTA_RW_CHUNK_SIZE];

    (void)id;
    (void)size;

    shell_arg_parse(pParamStr, &argc, argv, 3, ' ');
    if ((argc < 2) || (argc > 3))
        return SHELL_INVALID_PARAM;

    str_to_hex(argv[0], &addr);
    len = atoi(argv[1]);
    if (argc == 3)
        str_to_hex(argv[2], &seed);

    if ((addr & 0x3U) != 0U)
    {
        SH_PRINTF("ota_rw: addr must be 4-byte aligned\r\n");
        return SHELL_INVALID_PARAM;
    }

    if (len == 0U)
        return SHELL_INVALID_PARAM;

    if ((addr & (FMC_FLASH_PAGE_SIZE - 1U)) != 0U)
    {
        SH_PRINTF("ota_rw: addr must be 0x%lX-aligned\r\n",
                  (uint32_t)FMC_FLASH_PAGE_SIZE);
        return SHELL_INVALID_PARAM;
    }

    SH_PRINTF(
        "ota_rw: WARNING this writes to the inactive APROM bank and erases "
        "pages\r\n");
    SH_PRINTF("ota_rw: addr=0x%08lX len=%lu seed=0x%08lX\r\n", addr, len, seed);

    while (offset < len)
    {
        chunk = len - offset;
        if (chunk > OTA_RW_CHUNK_SIZE)
            chunk = OTA_RW_CHUNK_SIZE;

        for (i = 0; i < chunk; i++)
        {
            buf[i] = (uint8_t)((seed + offset + i) & 0xFFU);
        }

        status = OTA_WriteNewFW_S(addr + offset, buf, chunk);
        if (status != 0U)
        {
            SH_PRINTF(
                "ota_rw: OTA_WriteNewFW_S failed @0x%08lX (status=%u)\r\n",
                (addr + offset), status);
            return SHELL_INVALID_PARAM;
        }

        memset(buf, 0, chunk);
        OTA_ReadNewFW_S(addr + offset, buf, chunk);

        for (i = 0; i < chunk; i++)
        {
            uint8_t expected = (uint8_t)((seed + offset + i) & 0xFFU);
            if (buf[i] != expected)
            {
                mismatch = offset + i;
                SH_PRINTF(
                    "ota_rw: FAIL mismatch @%lu (addr=0x%08lX) exp=0x%02X "
                    "got=0x%02X\r\n",
                    mismatch, (addr + mismatch), expected, buf[i]);
                return SHELL_OK;
            }
        }

        dsm_wdt_ext_toggle_immd();
        offset += chunk;
    }

    if (mismatch == 0xFFFFFFFFU)
        SH_PRINTF("ota_rw: OK\r\n");

    return SHELL_OK;
}

static uint32_t crc32_make_bin_update(uint32_t crc, const uint8_t* p,
                                      uint32_t len)
{
    static uint32_t table[256];
    static uint8_t init_done = 0;
    uint32_t i;

    if (!init_done)
    {
        uint32_t n;
        for (n = 0; n < 256U; n++)
        {
            uint32_t c = n;
            for (i = 0; i < 8U; i++)
            {
                if (c & 1U)
                    c = (c >> 1) ^ 0xEDB88320U;
                else
                    c >>= 1;
            }
            table[n] = c;
        }
        init_done = 1;
    }

    for (i = 0; i < len; i++)
    {
        crc = (crc >> 8) ^ table[(crc ^ p[i]) & 0xFFU];
    }
    return crc;
}

static void crc32_make_bin_to_be(uint32_t crc, uint8_t out[4])
{
    out[0] = (uint8_t)((crc >> 24) & 0xFFU);
    out[1] = (uint8_t)((crc >> 16) & 0xFFU);
    out[2] = (uint8_t)((crc >> 8) & 0xFFU);
    out[3] = (uint8_t)(crc & 0xFFU);
}

static SHELL_ERR shell_flash_ota_crc(uint32_t id, char* pParamStr,
                                     uint32_t size)
{
    char* argv[3];
    uint32_t argc;
    uint32_t addr;
    uint32_t len;
    uint32_t seed = 0;
    uint32_t payload_len;
    uint8_t status;
    uint32_t offset = 0;
    uint32_t chunk;
    uint32_t i;
    uint32_t crc_w;
    uint32_t crc_r;
    uint8_t crc_be[4];

    enum
    {
        OTA_CRC_CHUNK_SIZE = 4096U
    };
    static uint8_t buf[OTA_CRC_CHUNK_SIZE];
    uint8_t tail[8];

    (void)id;
    (void)size;

    shell_arg_parse(pParamStr, &argc, argv, 3, ' ');
    if ((argc < 2) || (argc > 3))
        return SHELL_INVALID_PARAM;

    str_to_hex(argv[0], &addr);
    len = atoi(argv[1]);
    if (argc == 3)
        str_to_hex(argv[2], &seed);

    if ((addr & 0x3U) != 0U)
    {
        SH_PRINTF("ota_crc: addr must be 4-byte aligned\r\n");
        return SHELL_INVALID_PARAM;
    }

    if (len < 4U)
    {
        SH_PRINTF("ota_crc: len must be >= 4 (includes CRC32)\r\n");
        return SHELL_INVALID_PARAM;
    }

    if ((addr & (FMC_FLASH_PAGE_SIZE - 1U)) != 0U)
    {
        SH_PRINTF("ota_crc: addr must be 0x%lX-aligned\r\n",
                  (uint32_t)FMC_FLASH_PAGE_SIZE);
        return SHELL_INVALID_PARAM;
    }

    payload_len = len - 4U;

    SH_PRINTF(
        "ota_crc: WARNING this writes to the inactive APROM bank and erases "
        "pages\r\n");
    SH_PRINTF("ota_crc: addr=0x%08lX len=%lu payload=%lu seed=0x%08lX\r\n",
              addr, len, payload_len, seed);
    SH_PRINTF(
        "ota_crc: CRC32 algorithm matches tools\\make-bin (poly 0xEDB88320, "
        "init/xor 0xFFFFFFFF, BE stored)\r\n");

    crc_w = 0xFFFFFFFFU;
    offset = 0;
    while (offset < payload_len)
    {
        chunk = payload_len - offset;
        if (chunk > OTA_CRC_CHUNK_SIZE)
            chunk = OTA_CRC_CHUNK_SIZE;

        for (i = 0; i < chunk; i++)
        {
            buf[i] = (uint8_t)((seed + offset + i) & 0xFFU);
        }

        crc_w = crc32_make_bin_update(crc_w, buf, chunk);

        status = OTA_WriteNewFW_S(addr + offset, buf, chunk);
        if (status != 0U)
        {
            SH_PRINTF(
                "ota_crc: OTA_WriteNewFW_S failed @0x%08lX (status=%u)\r\n",
                (addr + offset), status);
            return SHELL_INVALID_PARAM;
        }

        dsm_wdt_ext_toggle_immd();
        offset += chunk;
    }

    crc_w ^= 0xFFFFFFFFU;
    crc32_make_bin_to_be(crc_w, crc_be);

    {
        uint32_t crc_addr = addr + payload_len;
        uint32_t aligned = crc_addr & ~0x3U;
        uint32_t prefix = crc_addr - aligned;
        uint32_t wlen = prefix + 4U;

        memset(tail, 0, sizeof(tail));
        for (i = 0; i < prefix; i++)
        {
            tail[i] = (uint8_t)((seed + (payload_len - prefix) + i) & 0xFFU);
        }
        memcpy(&tail[prefix], crc_be, 4U);

        status = OTA_WriteNewFW_S(aligned, tail, wlen);
        if (status != 0U)
        {
            SH_PRINTF("ota_crc: CRC write failed @0x%08lX (status=%u)\r\n",
                      aligned, status);
            return SHELL_INVALID_PARAM;
        }
    }

    SH_PRINTF("ota_crc: write CRC32=0x%08lX\r\n", crc_w);

    crc_r = 0xFFFFFFFFU;
    offset = 0;
    while (offset < payload_len)
    {
        chunk = payload_len - offset;
        if (chunk > OTA_CRC_CHUNK_SIZE)
            chunk = OTA_CRC_CHUNK_SIZE;

        memset(buf, 0, chunk);
        OTA_ReadNewFW_S(addr + offset, buf, chunk);
        crc_r = crc32_make_bin_update(crc_r, buf, chunk);

        dsm_wdt_ext_toggle_immd();
        offset += chunk;
    }
    crc_r ^= 0xFFFFFFFFU;

    {
        uint32_t crc_addr = addr + payload_len;
        uint32_t aligned = crc_addr & ~0x3U;
        uint32_t prefix = crc_addr - aligned;
        uint32_t rlen = prefix + 4U;

        memset(tail, 0, sizeof(tail));
        OTA_ReadNewFW_S(aligned, tail, rlen);

        crc_be[0] = tail[prefix + 0U];
        crc_be[1] = tail[prefix + 1U];
        crc_be[2] = tail[prefix + 2U];
        crc_be[3] = tail[prefix + 3U];
    }

    {
        uint32_t exp = ((uint32_t)crc_be[0] << 24) |
                       ((uint32_t)crc_be[1] << 16) |
                       ((uint32_t)crc_be[2] << 8) | ((uint32_t)crc_be[3]);

        if (crc_r == exp)
        {
            SH_PRINTF("ota_crc: OK read_crc=0x%08lX expected=0x%08lX\r\n",
                      crc_r, exp);
        }
        else
        {
            SH_PRINTF("ota_crc: FAIL read_crc=0x%08lX expected=0x%08lX\r\n",
                      crc_r, exp);
        }
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

uint32_t fw_info_get(uint32_t fs_bank, fw_info_t* pfwinfo)
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
        FLASH_If_Read((uint8_t*)(src_addr), (uint8_t*)pfwinfo,
                      sizeof(fw_info_t));
    }
    else
    { /*ext.flash*/
        CMD_READ(src_addr, (uint8_t*)pfwinfo, sizeof(fw_info_t));
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
            (uint8_t*)(src_addr + APPLICATION_FW_HEADROOM), pfwinfo->fw_size);
    }
    else
    {
        crc_val = (uint32_t)extflash_crc16_ccitt(
            (src_addr + APPLICATION_FW_HEADROOM), pfwinfo->fw_size);
    }

    DPRINTF(DBG_TRACE, "\r\n");

    return crc_val;
}

void fw_info_print(fw_info_t* p_fwinfo, uint32_t crc_val)
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

static SHELL_ERR shell_flash_fwinfo(uint32_t id, char* pParamStr, uint32_t size)
{
    return SHELL_OK;
}

static SHELL_ERR shell_flash_fwinfo_erase(uint32_t id, char* pParamStr,
                                          uint32_t size)
{
    return SHELL_OK;
}
