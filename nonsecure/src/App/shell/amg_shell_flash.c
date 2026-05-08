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
    {11, "ota_rw", _H(""), shell_flash_ota_rw, NULL},
    {12, "ota_crc", _H(""), shell_flash_ota_crc, NULL},

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
    return SHELL_OK;
}

static SHELL_ERR shell_flash_ota_crc(uint32_t id, char* pParamStr,
                                     uint32_t size)
{
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
