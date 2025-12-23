/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_imagetransfer.h"
#include "nv.h"
#include "whm.h"
#include "appl.h"
#include "act_req.h"
#include "amg_crc.h"
#include "options_def.h"
#include "flash_if.h"
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
#include "kse100_stm32l4.h"
#endif /* bccho */
#include "mx25r4035f.h"
#include "mx25r4035f_def.h"
#include "amg_meter_main.h"
#include "amg_modemif_prtl.h"
#include "dl.h"
#include "amg_wdt.h"
#include "amg_uart.h"
#include "amg_sec.h"
#include "amg_media_mnt.h"
#include "mx25r4035f.h"

/* bccho, 2024-09-05, 삼상 */
#include "amg_gpio.h"
#include <ctype.h>
#include <string.h>

#include "Softimer.h"
/*
******************************************************************************
*   Definition
******************************************************************************
*/
#define _D "[IMG] "

typedef enum
{
    IMG_TRF_NOT_INITIATED,
    IMG_TRF_INITIATED,
    IMG_VERIF_INITIATED,
    IMG_VERIF_SUCCESSFUL,
    IMG_VERIF_FAILED,
    IMG_ACTIVATION_INITIATED,
    IMG_ACTIVATION_SUCCESSFUL,
    IMG_ACTIVATION_FAILED,

} EN_IMG_TRF_STATUS;

typedef enum
{
    IMG_TRF_RESULT_SUCCESS,
    IMG_TRF_RESULT_TEMP_FAIL = 2,
    IMG_TRF_RESULT_OTHER_REASON = 250
} EN_IMG_TRF_RESULT;

#define SFLAG 0xE6

/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/
extern uint32_t g_bank_offset;
extern uint32_t g_pre_sector;
extern ST_MDM_MIC_FWUP_DL gst_mdm_mic_fwupdl;
extern ST_FW_INFO sun_fw_info;
extern bool dr_dsp, r_sun_dsp, dsp_comm_ing, dsp_cal_mode_ing, dsp_cal_st_ing,
    dsp_cal_mode_end, dsp_sun_ver_err_ing;

ST_TOU_IMG_INFO gst_tou_image_info;
ST_TOU_IMG_DATA gst_tou_image_data;
ST_FW_IMG_DOWNLOAD_INFO gst_fw_image_dlinfo;
DWALIGN ST_SUB_FW_IMG_DL gst_sub_fw_img _DWALIGN;
ST_SUB_FW_IMG_DL_INFO gst_sub_fw_info;

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
*   FUNCTIONS - extern
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/
void dsp_sun_version_error(void);
void log_sys_sw_up(void);

#if 1  // jp.kim 25.12.06 //  무결성 검증//intel-HEX 무결성 검증 test 프로그램
U8 g_meter_fw_chk_sum_err = 0;
int g_eof_found = 0;
U32 g_total_data_len = 0;
char g_linebuf[256];  // 현재 라인만 저장
int g_linepos = 0;

// HEX 문자열에서 특정 위치의 2자리 필드를 읽어 정수로 변환
int read_hex_field(const char* line, int offset)
{
    char buf[3];  // 2자리 + 널 종료
    memcpy(buf, line + offset, 2);
    buf[2] = '\0';
    return (int)strtol(buf, NULL, 16);
}

// 한 줄 검증 및 데이터 길이 누적
static int hex_nibble(char c)
{
    if ((c >= '0') && (c <= '9'))
        return (int)(c - '0');

    c = (char)toupper((unsigned char)c);
    if ((c >= 'A') && (c <= 'F'))
        return (int)(10 + (c - 'A'));

    return -1;
}

static int read_hex_field_checked(const char* line, unsigned int line_len,
                                  unsigned int offset, int* out)
{
    if ((line == NULL) || (out == NULL))
        return -1;

    if ((offset + 2U) > line_len)
        return -1;

    if (!isxdigit((unsigned char)line[offset]) ||
        !isxdigit((unsigned char)line[offset + 1U]))
        return -1;

    int hi = hex_nibble(line[offset]);
    int lo = hex_nibble(line[offset + 1U]);
    if ((hi < 0) || (lo < 0))
        return -1;

    *out = (hi << 4) | lo;
    return 0;
}

int verify_hex_line(const char* line)
{
    if (line == NULL)
        return -1;

    unsigned int line_len = (unsigned int)strlen(line);
    while ((line_len > 0U) &&
           ((line[line_len - 1U] == '\n') || (line[line_len - 1U] == '\r')))
    {
        line_len--;
    }

    // Minimum: ":LLAAAATTCC" => 11 chars
    if (line_len < 11U)
        return -1;

    if (line[0] != ':')
        return -1;

    int len = 0;
    if (read_hex_field_checked(line, line_len, 1U, &len) != 0)
        return -1;

    unsigned int expected_len = 11U + (2U * (unsigned int)len);
    if (line_len != expected_len)
        return -1;

    unsigned int sum = (unsigned int)len;

    int addr_hi = 0;
    int addr_lo = 0;
    if (read_hex_field_checked(line, line_len, 3U, &addr_hi) != 0)
        return -1;
    if (read_hex_field_checked(line, line_len, 5U, &addr_lo) != 0)
        return -1;
    sum += (unsigned int)addr_hi + (unsigned int)addr_lo;

    int rectype = 0;
    if (read_hex_field_checked(line, line_len, 7U, &rectype) != 0)
        return -1;
    sum += (unsigned int)rectype;

    unsigned int pos = 9U;
    for (int i = 0; i < len; i++)
    {
        int val = 0;
        if (read_hex_field_checked(line, line_len, pos, &val) != 0)
            return -1;
        sum += (unsigned int)val;
        pos += 2U;
    }

    int checksum = 0;
    if (read_hex_field_checked(line, line_len, pos, &checksum) != 0)
        return -1;
    sum += (unsigned int)checksum;

    if ((sum & 0xFF) != 0)
    {
        DPRINTF(DBG_ERR,
                "%s: Checksum error in line:  if ((sum & 0xFF) != 0) return "
                "-1; \r\n",
                __func__);
        return -1;  // 체크섬 오류
    }

    // if (rectype == 0x00) { // 데이터 레코드일 경우 길이 누적
    //     g_total_data_len += len;
    // }

    return rectype;
}

// 패킷 단위 처리 (라인 버퍼만 유지)
// jp.kim 25.12.06 //운영부  무결성 검증

int verify_packets(unsigned char* pkt, unsigned int len)
{
    U8 error_sum = 0;
    g_total_data_len += len;

    while (len--)
    {
        char c = *pkt++;
        if ((g_linepos < 0) ||
            ((unsigned int)g_linepos >= (sizeof(g_linebuf) - 1U)))
        {
            g_linebuf[sizeof(g_linebuf) - 1U] = '\0';
            DPRINTF(DBG_ERR, "Checksum buffer over error in line: %s\r\n",
                    g_linebuf);
            g_linepos = 0;  // 버퍼 초기화
            error_sum = 1;
            return -1;
        }
        g_linebuf[g_linepos] = c;
        g_linepos++;

        if (c == '\n')
        {  // 라인 완성
            DPRINTF(DBG_ERR, "lin ok: c[0x%x] len[0x%x] g_linepos[%d]\r\n", c,
                    len, g_linepos);
            g_linebuf[g_linepos] = '\0';  // string end 표시

            int result = verify_hex_line(g_linebuf);
            if (result < 0)
            {
                DPRINTF(DBG_ERR, "Checksum error in line: %s\r\n", g_linebuf);

                error_sum = 1;
                // g_linepos = 0; // 버퍼 초기화
                // return -1;
            }
            if (result == 0x01)
            {  // EOF 레코드 확인
                g_eof_found = 1;
            }

            g_linepos = 0;  // 버퍼 초기화
        }
    }

    if (error_sum)
    {
        return -1;
    }

    return 0;
}

#endif

uint32_t dsm_flash_get_flash_page_size(uint8_t type)
{
    uint32_t ret = 0;

    if (type == FLASH_T_INT)
    {
        ret = FLASH_PAGE_SIZE;
    }
    else
    {
        /*************************************/
        // flash page size 에 맞게 수정 필요...
        /*************************************/
        ret = Sector_Offset;  // TODO: (WD) Check, 어짜피 섹터 단위로 지워야함.
    }

    DPRINTF(DBG_TRACE, "flash_t[%d], page_size[%d]\r\n", type,
            ret);  // 0: Internal, 1: External
    return ret;
}

uint16_t dsm_convert_dlrxtxmax_to_imgtrfr_blk_size(void)
{
    uint16_t block_size = 0;

    if (MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO > 512)
    {
        block_size = 512;
    }
    else if (MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO > 256)
    {
        block_size = 256;
    }
    else if (MAX_INFORMATION_FIELD_LENGTH_FOR_KEPCO > 128)
    {
        block_size = 128;
    }
    else
    {
        block_size = 64;
    }

    DPRINTF(DBG_WARN, _D "ImageBlockSize %d\r\n", block_size);

    return block_size;
}

void dsm_imgtrfr_con_init(EN_IMG_TYPE type)
{
    /* config init */

    uint16_t blk_size = dsm_convert_dlrxtxmax_to_imgtrfr_blk_size();

    // DPRINTF(DBG_WARN, _D"%s: type[%d]\r\n", __func__, type);
    DPRINTF(DBG_WARN, _D "%s: %s\r\n", __func__, (type ? "F/W" : "TOU"));

    if (IMG__TOU == type)
    {
        memset(&gst_tou_image_info, 0x00, sizeof(ST_TOU_IMG_INFO));
        memset(&gst_tou_image_data, 0x00, sizeof(ST_TOU_IMG_DATA));
        gst_tou_image_info.transfer_enabled = IMAGE_TRANSFER_ENABLED;
        /**************************************************************************/
        /**************************************************************************/
        dsm_imgtrfr_set_blk_size(IMG__TOU,
                                 blk_size);  // block size 나중에 검토 필요...
        /**************************************************************************/
        /**************************************************************************/
    }
    else
    {
        memset(&gst_fw_image_dlinfo, 0x00, sizeof(ST_FW_IMG_DOWNLOAD_INFO));
        gst_fw_image_dlinfo.transfer_enabled = IMAGE_TRANSFER_ENABLED;
        /**************************************************************************/
        /**************************************************************************/
        dsm_imgtrfr_set_blk_size(IMG__FW,
                                 blk_size);  // block size 나중에 검토 필요...
        /**************************************************************************/
        /**************************************************************************/
        dsm_imgtrfr_fwup_set_fsm(FWU_FSM_NONE);
    }
}

ST_FW_IMG_DOWNLOAD_INFO* dsm_imgtrfr_get_fw_image_dlinfo(void)
{
    return &gst_fw_image_dlinfo;
}

uint8_t dsm_imgtrfr_get_transfer_enabled(EN_IMG_TYPE type)
{
    uint8_t transfer_enabled = FALSE;
    if (IMG__TOU == type)
    {
        transfer_enabled = gst_tou_image_info.transfer_enabled;
    }
    else
    {
        transfer_enabled = gst_fw_image_dlinfo.transfer_enabled;
    }
    // DPRINTF(DBG_TRACE, _D"get: transfer_enabled[%d], type[%d]\r\n",
    // transfer_enabled, type);
    DPRINTF(DBG_TRACE, _D "Get: %s transfer_enabled[%d]\r\n",
            (type ? "F/W" : "TOU"), transfer_enabled);
    return transfer_enabled;
}

void dsm_imgtrfr_set_transfer_enabled(EN_IMG_TYPE type, uint8_t val)
{
    if (IMG__TOU == type)
    {
        gst_tou_image_info.transfer_enabled = val;
    }
    else
    {
        gst_fw_image_dlinfo.transfer_enabled = val;
    }
    // DPRINTF(DBG_TRACE, _D"set: transfer_enabled[%d], type[%d]\r\n", val,
    DPRINTF(DBG_TRACE, _D "Set: %s transfer_enabled[%d]\r\n",
            (type ? "F/W" : "TOU"), val);
}

uint8_t dsm_imgtrfr_get_int_status_bit(EN_IMG_TYPE type, uint8_t bit_pos)
{
    uint8_t int_status_bit = FALSE;
    if (IMG__TOU == type)
    {
        int_status_bit = (gst_tou_image_info.int_status_bits >> bit_pos) & 0x01;
    }
    else
    {
        int_status_bit =
            (gst_fw_image_dlinfo.int_status_bits >> bit_pos) & 0x01;
    }
    DPRINTF(DBG_TRACE, _D "Get: %s status_bit[%d], bit_pos[%d]\r\n",
            (type ? "F/W" : "TOU"), int_status_bit, bit_pos);
    return int_status_bit;
}

void dsm_imgtrfr_set_int_status_bits(EN_IMG_TYPE type, uint8_t val)
{
    if (IMG__TOU == type)
    {
        gst_tou_image_info.int_status_bits |= val;
    }
    else
    {
        gst_fw_image_dlinfo.int_status_bits |= val;
    }
    // DPRINTF(DBG_TRACE, _D"set: int_status_bits [0x%02X], type[%d]\r\n", val,
    // type);
    DPRINTF(DBG_TRACE, _D "Set: %s status_bits[0x%02X]\r\n",
            (type ? "F/W" : "TOU"), val);
}

void dsm_imgtrfr_reset_int_status_bits(EN_IMG_TYPE type, uint8_t val)
{
    if (IMG__TOU == type)
    {
        gst_tou_image_info.int_status_bits &= (~val);
    }
    else
    {
        gst_fw_image_dlinfo.int_status_bits &= (~val);
    }
    // DPRINTF(DBG_TRACE, _D"re_set: int_status_bits [0x%02X], type[%d]\r\n",
    // val, type);
    DPRINTF(DBG_TRACE, _D "Set: %s Reset status_bits[0x%02X]\r\n",
            (type ? "F/W" : "TOU"), val);
}

uint32_t dsm_imgtrfr_get_blk_size(EN_IMG_TYPE type)
{
    uint32_t blk_size;

    if (IMG__TOU == type)
        blk_size = gst_tou_image_info.blk_size;
    else
        blk_size = gst_fw_image_dlinfo.blk_size;

    // DPRINTF(DBG_TRACE, _D"get: BLK_SIZE[%d], type[%d]\r\n", blk_size, type);
    DPRINTF(DBG_TRACE, _D "Get: %s BlockSize %d\r\n", (type ? "F/W" : "TOU"),
            blk_size);
    return blk_size;
}

void dsm_imgtrfr_set_blk_size(EN_IMG_TYPE type, uint32_t blk_size)
{
    // DPRINTF(DBG_TRACE, _D"set: BLK_NUM[%d], type[%d]\r\n", blk_size, type);
    DPRINTF(DBG_TRACE, _D "Set: %s BlockSize %d\r\n", (type ? "F/W" : "TOU"),
            blk_size);

    if (IMG__TOU == type)
        gst_tou_image_info.blk_size = blk_size;
    else
        gst_fw_image_dlinfo.blk_size = blk_size;
}

uint32_t dsm_imgtrfr_get_blk_num(EN_IMG_TYPE type)
{
    uint32_t blk_number;

    if (IMG__TOU == type)
        blk_number = gst_tou_image_info.blk_number;
    else
        blk_number = gst_fw_image_dlinfo.blk_number;

    // DPRINTF(DBG_TRACE, _D"get: BLK_NUM[%d], type[%d]\r\n", blk_number, type);
    DPRINTF(DBG_TRACE, _D "Get: %s BLK[%d]\r\n", (type ? "F/W" : "TOU"),
            blk_number);
    return blk_number;
}

void dsm_imgtrfr_set_blk_num(EN_IMG_TYPE type, uint32_t blk_number)
{
    // DPRINTF(DBG_TRACE, _D"set: BLK_NUM[%d], type[%d]\r\n", blk_number, type);

    gst_mdm_mic_fwupdl.snd_cnt = blk_number;
    DPRINTF(DBG_TRACE, _D "Set: %s BLK[%d]\r\n", (type ? "F/W" : "TOU"),
            blk_number);

    if (IMG__TOU == type)
        gst_tou_image_info.blk_number = blk_number;
    else
        gst_fw_image_dlinfo.blk_number = blk_number;
}

uint8_t dsm_imgtrfr_get_transfer_status(EN_IMG_TYPE type)
{
    uint8_t transfer_status;

    if (IMG__TOU == type)
        transfer_status = gst_tou_image_info.transfer_status;
    else
        transfer_status = gst_fw_image_dlinfo.transfer_status;

    // DPRINTF(DBG_TRACE, _D"get: transfer_status[%d], type[%d]\r\n",
    // transfer_status, type);
    DPRINTF(DBG_TRACE, _D "Get: %s transfer_status[%d]\r\n",
            (type ? "F/W" : "TOU"), transfer_status);
    return transfer_status;
}

void dsm_imgtrfr_set_transfer_status(EN_IMG_TYPE type, uint8_t transfer_status)
{
    if (IMG__TOU == type)
        gst_tou_image_info.transfer_status = transfer_status;
    else
        gst_fw_image_dlinfo.transfer_status = transfer_status;

    // DPRINTF(DBG_TRACE, _D"set: transfer_status[%d], type[%d]\r\n",
    // transfer_status, type);
    DPRINTF(DBG_TRACE, _D "Set: %s transfer_status[%d]\r\n",
            (type ? "F/W" : "TOU"), transfer_status);
}

uint32_t dsm_imgtrfr_get_image_size(EN_IMG_TYPE type)
{
    uint32_t image_size;
    if (IMG__TOU == type)
        image_size = gst_tou_image_info.image_size;
    else
        image_size = gst_fw_image_dlinfo.image_size;

    // DPRINTF(DBG_TRACE, _D"get: IMG_SIZE[%d], type[%d]\r\n", image_size,
    DPRINTF(DBG_TRACE, _D "Get: %s IMG_SIZE[%d]\r\n", (type ? "F/W" : "TOU"),
            image_size);
    return image_size;
}

void dsm_imgtrfr_set_image_size(EN_IMG_TYPE type, uint32_t image_size)
{
    // DPRINTF(DBG_TRACE, _D"set: IMG_SIZE[%d], type[%d]\r\n", image_size,
    DPRINTF(DBG_TRACE, _D "Set: %s IMG_SIZE[%d]\r\n", (type ? "F/W" : "TOU"),
            image_size);

    if (IMG__TOU == type)
        gst_tou_image_info.image_size = image_size;
    else
        gst_fw_image_dlinfo.image_size = image_size;
}

uint32_t dsm_imgtrfr_get_rcvimage_size(EN_IMG_TYPE type)
{
    uint32_t image_size;
    if (IMG__TOU == type)
        image_size = gst_tou_image_info.image_rcv_size;
    else
        image_size = gst_fw_image_dlinfo.image_rcv_size;

    // DPRINTF(DBG_TRACE, _D"get: IMG_RCV_SIZE[%d], type[%d]\r\n", image_size,
    // type);
    DPRINTF(DBG_TRACE, _D "Get: %s RECV_SIZE[%d]\r\n", (type ? "F/W" : "TOU"),
            image_size);
    return image_size;
}

void dsm_imgtrfr_set_rcvimage_size(EN_IMG_TYPE type, uint32_t image_size)
{
    // DPRINTF(DBG_TRACE, _D"set: IMG_RCV_SIZE set[%d], type[%d]\r\n",
    DPRINTF(DBG_TRACE, _D "Set: %s RECV_SIZE[%d]\r\n", (type ? "F/W" : "TOU"),
            image_size);

    if (IMG__TOU == type)
        gst_tou_image_info.image_rcv_size = image_size;
    else
        gst_fw_image_dlinfo.image_rcv_size = image_size;
}

char* dsm_fw_type_string(uint8_t fw_type)
{
    switch (fw_type)
    {
    case FW_DL_SYS_PART:
        return "SYS_PART";
    case FW_DL_INT_MDM:
        return "INT_MDM";
    case FW_DL_EXT_MDM:
        return "EXT_MDM";
    case FW_DL_METER_PART:
        return "METER_PART";

    default:
        return "FW_Unknown";
    }
}

uint8_t dsm_imgtrfr_get_fw_type(void) { return gst_fw_image_dlinfo.fw_type; }

void dsm_imgtrfr_set_fw_type(uint8_t fw_type)
{
    gst_fw_image_dlinfo.fw_type = fw_type;
    // DPRINTF(DBG_WARN, _D"FW_TYPE set: %s\r\n", dsm_fw_type_string(fw_type));
    DPRINTF(DBG_WARN, _D "Set F/W Type: %s\r\n", dsm_fw_type_string(fw_type));
}

uint32_t dsm_imgtrfr_get_start_dl_addr(void)
{
    // DPRINTF(DBG_TRACE, _D"FW dl_start_addr get[0x%08X]\r\n",
    DPRINTF(DBG_TRACE, _D "Get F/W Addr[0x%08X]\r\n",
            gst_fw_image_dlinfo.start_dl_addr);
    return gst_fw_image_dlinfo.start_dl_addr;
}

void dsm_imgtrfr_set_start_dl_addr(uint32_t start_dl_addr)
{
    gst_fw_image_dlinfo.start_dl_addr = start_dl_addr;
    // DPRINTF(DBG_TRACE, _D"FW dl_start_addr set[0x%08X]\r\n", start_dl_addr);
    DPRINTF(DBG_TRACE, _D "Set F/W Addr[0x%08X]\r\n", start_dl_addr);
}

uint8_t* dsm_imgtrfr_get_name(EN_IMG_TYPE type)
{
    uint8_t* pname;
    uint8_t len;

    if (IMG__TOU == type)
    {
        pname = gst_tou_image_info.name;
        len = IMAGE_NAME_MAX_SIZE;
    }
    else
    {
        pname = gst_fw_image_dlinfo.name;
        len = IMAGE_FW_NAME_MAX_SIZE;
    }

    DPRINT_HEX(DBG_TRACE, "GET_IMG_NAME", pname, len, DUMP_ALWAYS);
    return pname;
}

void dsm_imgtrfr_set_name(EN_IMG_TYPE type, uint8_t len, uint8_t* name)
{
    DPRINT_HEX(DBG_TRACE, "SET_IMG_NAME", name, len, DUMP_ALWAYS);
    if (IMG__TOU == type)
        memcpy(gst_tou_image_info.name, name, len);
    else
        memcpy(gst_fw_image_dlinfo.name, name, len);
}

uint8_t* dsm_imgtrfr_get_hash(EN_IMG_TYPE type)
{
    if (IMG__TOU == type)
        return gst_tou_image_info.hash;
    else
        return gst_fw_image_dlinfo.hash;
}

void dsm_imgtrfr_set_hash(EN_IMG_TYPE type, uint8_t len, uint8_t* hash)
{
    if (IMG__TOU == type)
        memcpy(gst_tou_image_info.hash, hash, len);
    else
        memcpy(gst_fw_image_dlinfo.hash, hash, len);
}

uint32_t dsm_imgtrfr_activNverify_is_ready(EN_IMG_TYPE type)
{
    if (IMG__TOU == type)
    {
        if (!gst_tou_image_info.transfer_enabled)
            return FALSE;
        else if (!dsm_imgtrfr_get_int_status_bit(type, IMG_SBIT_POS_ACTI_INIT))
            return FALSE;
        else if (!dsm_imgtrfr_get_int_status_bit(type, IMG_SBIT_POS_LAST_TRFR))
            return FALSE;
        else if (!dsm_imgtrfr_get_int_status_bit(type, IMG_SBIT_POS_HASH_GEN))
            return FALSE;
    }
    else
    {
        if (!gst_fw_image_dlinfo.transfer_enabled)
            return FALSE;
        else if (!dsm_imgtrfr_get_int_status_bit(type, IMG_SBIT_POS_ACTI_INIT))
            return FALSE;
        else if (!dsm_imgtrfr_get_int_status_bit(type, IMG_SBIT_POS_LAST_TRFR))
            return FALSE;
        else if (!dsm_imgtrfr_get_int_status_bit(type, IMG_SBIT_POS_HASH_GEN))
            return FALSE;
    }
    return TRUE;
}

/******************/
/* FW image only  */
/******************/
char* dsm_fw_info_string(uint32_t fw_pos)
{
    switch (fw_pos)
    {
    case FWINFO_CUR_SYS:
        return "SYSTEM";

    case FWINFO_CUR_METER:
        return "METER";

    case FWINFO_CUR_MODEM:
        return "I-MODEM";

    case FWINFO_CUR_E_MODEM:
        return "E-MODEM";

    default:
        return "UNKNOWN";
    }
}

bool dsm_imgtrfr_fwinfo_read(uint8_t* pblk, uint32_t pos)
{
    bool ret = 0;
    ST_FW_INFO fwinfo;
    nv_sub_info.ch[0] = pos;

    ret = nv_read(I_FW_IMG_INFO, pblk);

#if 1  // JP.KIM 24.10.29
    if (pos == FWINFO_CUR_SYS)
    {
        memcpy((uint8_t*)&fwinfo, pblk, sizeof(ST_FW_INFO));

        fwinfo.mt_type[0] = (METER_ID / 10) + '0';
        fwinfo.mt_type[1] = (METER_ID % 10) + '0';
        fwinfo.version[0] = HARDWARE_VERSION;
        fwinfo.version[1] = SOFTWARE_SYSTEM_PART;
        fwinfo.version[2] = COMPANY_ID_1;  //
        fwinfo.version[3] = COMPANY_ID_2;  //
        fwinfo.version[4] = SOFTWARE_VERSION_H;
        fwinfo.version[5] = SOFTWARE_VERSION_L;
        fwinfo.date_time[0] = SOFTWARE_DATE_YYH;  // Y
        fwinfo.date_time[1] = SOFTWARE_DATE_YYL;  // Y
        fwinfo.date_time[2] = SOFTWARE_DATE_MMH;  // M
        fwinfo.date_time[3] = SOFTWARE_DATE_MML;  // M
        fwinfo.date_time[4] = SOFTWARE_DATE_DDH;  // D
        fwinfo.date_time[5] = SOFTWARE_DATE_DDL;  // D

        memcpy(pblk, (uint8_t*)&fwinfo, sizeof(ST_FW_INFO));
    }
#endif

    DPRINTF(DBG_TRACE, "%s: %s[%d]\r\n", __func__, dsm_fw_info_string(pos),
            ret);
    DPRINT_HEX(DBG_TRACE, "FW_INFO", pblk, sizeof(ST_FW_INFO), DUMP_ALWAYS);

    return ret;
}

bool dsm_imgtrfr_fwinfo_write(uint8_t* pblk, uint32_t pos)
{
    bool ret = 0;

    nv_sub_info.ch[0] = pos;

    ret = nv_write(I_FW_IMG_INFO, pblk);

    DPRINTF(DBG_TRACE, "%s: %s[%d]\r\n", __func__, dsm_fw_info_string(pos),
            ret);
    DPRINT_HEX(DBG_TRACE, "FW_INFO", pblk, sizeof(ST_FW_INFO), DUMP_ALWAYS);

    return ret;
}

// #define VERSION_NORMAL      0x30
// #define VERSION_ABNORMAL    0x31
// #define VERSION_METER_PART  0x33
bool dsm_sys_fwinfo_initial_set(bool product)
{
    bool error = 0;
    ST_FW_INFO fwinfo = {0};
    uint8_t manuid[MANUF_ID_SIZE] = {0};

    /* 소프트웨어 적용 날짜 : 6 Bytes */
    if (product == true)
    {
        fwinfo.update_t = FWINFO_UPDATE_T_FIRST;
        get_manuf_id(manuid);
        DPRINT_HEX(DBG_TRACE, "manuid", manuid, MANUF_ID_SIZE, DUMP_ALWAYS);

        fwinfo.mt_type[0] = manuid[2];  // '5'
        fwinfo.mt_type[1] = manuid[3];  // '3'

        // 규격 반영 - 양산시 fw apply date
        // TODO : (WD) 공장 초기화 시 소프트웨어 정보를 형식에 맞게 정상적으로
        // 초기화 시키려면 아래 코드를 항상 적용되도록 영역 밖으로 이동 및 수정
        // 해야함.
        /* 소프트웨어 Image Hash : 32 Bytes */
        memset(fwinfo.hash, 0, IMAGE_HASH_SIZE);  // ref. ob_evt_fw_info()
        //
        fwinfo.version[0] = HARDWARE_VERSION;
        fwinfo.version[1] = SOFTWARE_SYSTEM_PART;
        fwinfo.version[2] = manuid[0];  // '4'
        fwinfo.version[3] = manuid[1];  // '8'
        fwinfo.version[4] = SOFTWARE_VERSION_H;
        fwinfo.version[5] = SOFTWARE_VERSION_L;

        fwinfo.date_time[0] = SOFTWARE_DATE_YYH;  // Y
        fwinfo.date_time[1] = SOFTWARE_DATE_YYL;  // Y
        fwinfo.date_time[2] = SOFTWARE_DATE_MMH;  // M
        fwinfo.date_time[3] = SOFTWARE_DATE_MML;  // M
        fwinfo.date_time[4] = SOFTWARE_DATE_DDH;  // D
        fwinfo.date_time[5] = SOFTWARE_DATE_DDL;  // D
        /*
            현재 소프트웨어 적용 날짜
            OBIS : 0 0 96 2 13 255, Attributes : 2
            date-time 초기값은 {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
           0xff, 0x80, 0x00, 0xff}                              date-time[12] =
           {year, month, day, hour,                              minute, second,
           sub_sec(msec/10), xx, xx,                              xx}
        */
        memset(&fwinfo.dt, 0xff,
               sizeof(date_time_type));  // ref. ob_evt_fw_apply_date()

        fwinfo.version[0] = METER_HW_VERSION;
        fwinfo.version[1] = SOFTWARE_METER_PART;
        dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_SYS);

        uint8_t temp[/*Page_Offset*/ 64] = {0};
        CMD_READ(SFLASH_SYS_INFO_ADDR + Sector_Offset, temp, 6);  // size, crc
        CMD_SE(SFLASH_SYS_INFO_ADDR + Sector_Offset);
        memcpy(&temp[6], (uint8_t*)&fwinfo, sizeof(ST_FW_INFO));
        CMD_PP(SFLASH_SYS_INFO_ADDR + Sector_Offset, temp,
               sizeof(ST_FW_INFO) + 6);
    }

    /****************************/
    /*system part fwinfo default*/
    /****************************/
    // memset((uint8_t*)&fwinfo, 0x00, sizeof(ST_FW_INFO));
    dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_SYS);

    DPRINTF(DBG_WARN, "SYSTEM FWINFO default\r\n");
    DPRINTF(DBG_TRACE, "APPLY_TIME %02d.%02d.%02d %02d:%02d:%02d\r\n",
            fwinfo.dt.year, fwinfo.dt.month, fwinfo.dt.date, fwinfo.dt.hour,
            fwinfo.dt.min, fwinfo.dt.sec);
    DPRINTF(DBG_TRACE, "UPDATE_T (0x%X), METER_T[%c%c]\r\n", fwinfo.update_t,
            fwinfo.mt_type[0], fwinfo.mt_type[1]);
    DPRINTF(DBG_TRACE, "FW_SYS_VER: %c%c%c%c%c%c\r\n", fwinfo.version[0],
            fwinfo.version[1], fwinfo.version[2], fwinfo.version[3],
            fwinfo.version[4], fwinfo.version[5]);

    /****************************/
    /*meter part fwinfo default**/
    /****************************/

    if (product == true)
    {  // 규격 반영 - 양산시 fw apply date

        fwinfo.version[1] = SOFTWARE_METER_PART;
        dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_METER);
    }

    dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_METER);
    DPRINTF(DBG_WARN, "METER FWINFO default\r\n");
    DPRINTF(DBG_TRACE, "APPLY_TIME %02d.%02d.%02d %02d:%02d:%02d\r\n",
            fwinfo.dt.year, fwinfo.dt.month, fwinfo.dt.date, fwinfo.dt.hour,
            fwinfo.dt.min, fwinfo.dt.sec);
    DPRINTF(DBG_TRACE, "UPDATE_T (0x%X), METER_T[%c%c]\r\n", fwinfo.update_t,
            fwinfo.mt_type[0], fwinfo.mt_type[1]);
    DPRINTF(DBG_TRACE, "FW_METER_VER: %c%c%c%c%c%c\r\n", fwinfo.version[0],
            fwinfo.version[1], fwinfo.version[2], fwinfo.version[3],
            fwinfo.version[4], fwinfo.version[5]);

    /****************************/
    /*IN_MODEM part fwinfo default**/
    /****************************/
    if (product == true)
    {
        fwinfo.mt_type[0] = (METER_ID / 10) + '0';
        fwinfo.mt_type[1] = (METER_ID % 10) + '0';
        fwinfo.version[0] = INMODEM_HW_VERSION;
        fwinfo.version[1] = SOFTWARE_INMODEM_PART;
        fwinfo.version[2] = COMPANY_ID_1;  //
        fwinfo.version[3] = COMPANY_ID_2;  //
        fwinfo.version[4] = INMODEM_VERSION_H;
        fwinfo.version[5] = INMODEM_VERSION_L;
        fwinfo.date_time[0] = INMODEM_DATE_YYH;  // Y
        fwinfo.date_time[1] = INMODEM_DATE_YYL;  // Y
        fwinfo.date_time[2] = INMODEM_DATE_MMH;  // M
        fwinfo.date_time[3] = INMODEM_DATE_MML;  // M
        fwinfo.date_time[4] = INMODEM_DATE_DDH;  // D
        fwinfo.date_time[5] = INMODEM_DATE_DDL;  // D

        // SUN FWINFO 읽기 sun_fw_info
        memset(sun_fw_info.date_time, 0x00, FW_GENERATION_DATE_SIZE);
        dsm_atcmd_if_is_valid(MEDIA_RUN_SUN, false);  // fw_info
        vTaskDelay(100);
        DPRINT_HEX(DBG_TRACE, "1 sun_fw_info.date_time", sun_fw_info.date_time,
                   FW_GENERATION_DATE_SIZE, DUMP_ALWAYS);

        memset(sun_fw_info.date_time, 0x00, FW_GENERATION_DATE_SIZE);
        dsm_atcmd_if_is_valid(MEDIA_RUN_SUN, false);  // fw_info
        vTaskDelay(100);
        DPRINT_HEX(DBG_TRACE, "2 sun_fw_info.date_time", sun_fw_info.date_time,
                   FW_GENERATION_DATE_SIZE, DUMP_ALWAYS);

        // 상호간 비교  ( 운영부 <-> SUN )
        if ((!memcmp(fwinfo.date_time, sun_fw_info.date_time,
                     FW_GENERATION_DATE_SIZE)))
        {
            DPRINTF(DBG_WARN, _D "FW image version compare SUCCESS!!!\r\n");
        }
        else  // ERROR // 서로 다르면 verify error
        {
            DPRINTF(DBG_ERR, _D "FW image verify FW.DATE fail!!! \r\n");
            DPRINT_HEX(DBG_TRACE, "fw_info.date_tim(AMIGO)", fwinfo.date_time,
                       FW_GENERATION_DATE_SIZE, DUMP_ALWAYS);
            DPRINT_HEX(DBG_TRACE, "sun_fw_info.date_time",
                       sun_fw_info.date_time, FW_GENERATION_DATE_SIZE,
                       DUMP_ALWAYS);
            dsp_sun_ver_err_ing = true;
            dsp_sun_ver_err_ing_timeset(T300SEC);
            error = 1;
        }

        dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_MODEM);
    }

    dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_MODEM);
    DPRINTF(DBG_WARN, "INMODEM FWINFO default\r\n");
    DPRINTF(DBG_TRACE, "APPLY_TIME %02d.%02d.%02d %02d:%02d:%02d\r\n",
            fwinfo.dt.year, fwinfo.dt.month, fwinfo.dt.date, fwinfo.dt.hour,
            fwinfo.dt.min, fwinfo.dt.sec);
    DPRINTF(DBG_TRACE, "UPDATE_T (0x%X), INMODEM_T[%c%c]\r\n", fwinfo.update_t,
            fwinfo.mt_type[0], fwinfo.mt_type[1]);
    DPRINTF(DBG_TRACE, "FW_METER_VER: %c%c%c%c%c%c\r\n", fwinfo.version[0],
            fwinfo.version[1], fwinfo.version[2], fwinfo.version[3],
            fwinfo.version[4], fwinfo.version[5]);

    return error;
}

bool dsm_sys_fwinfo_is_initialNset(void)
{
    uint32_t ret = FALSE;
    ST_FW_INFO fwinfo;
    uint8_t manuid[MANUF_ID_SIZE];

    get_manuf_id(manuid);
    dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_SYS);

    if (fwinfo.update_t == FWINFO_UPDATE_T_FIRST ||
        fwinfo.update_t == FWINFO_UPDATE_T_DOWNLOAD)
    {
        if (!memcmp(&manuid[2], &fwinfo.mt_type[0], 2))
            ret = TRUE;
    }

    return ret;
}

void dsm_imgtrfr_fwimage_dl_info_save(void)
{
    nv_write(I_FW_IMG_DOWNLOAD_INFO, (uint8_t*)&gst_fw_image_dlinfo);
}

uint8_t dsm_imgtrfr_fwimage_dl_get_fw_type(void)
{
    uint8_t fw_type = FW_DL_TYPE_NONE;

    DPRINT_HEX(DBG_TRACE, "FW_IMG_NAME", gst_fw_image_dlinfo.name,
               IMAGE_FW_NAME_MAX_SIZE, DUMP_ALWAYS);

    switch (gst_fw_image_dlinfo
                .name[FW_BANK_FW_INFO_VER_FW_TYPE_IDX])  // get type from
                                                         // image_identifier
    {
    case '0':  // system
        fw_type = FW_DL_SYS_PART;

        break;
    case '1':  // int modem
        fw_type = FW_DL_INT_MDM;

        break;
    case '2':  // ext modem
        fw_type = FW_DL_EXT_MDM;

        break;
    case '3':  // meter
        fw_type = FW_DL_METER_PART;

        break;
    }
    // DPRINTF(DBG_TRACE, "FW_TYPE[%c, %d]\r\n", gst_fw_image_dlinfo.name[1],
    // fw_type);
    DPRINTF(DBG_TRACE, "SW_TYPE[%c]\r\n",
            gst_fw_image_dlinfo.name[FW_BANK_FW_INFO_VER_FW_TYPE_IDX]);

    return fw_type;
}

void dsm_imgtrfr_fwimage_set_sys_flash_addr(uint32_t val)
{
    gst_fw_image_dlinfo.flash_addr = val;
}

uint32_t dsm_imgtrfr_fwimage_get_sys_flash_addr(void)
{
    return gst_fw_image_dlinfo.flash_addr;
}

EN_FWU_FSM g_imgtrfr_fwup_fsm = FWU_FSM_NONE;
void dsm_imgtrfr_fwup_set_fsm(uint8_t state)
{
    if (g_imgtrfr_fwup_fsm != state)
    {
        g_imgtrfr_fwup_fsm = state;
    }
}

uint8_t dsm_imgtrfr_fwup_get_fsm(void) { return g_imgtrfr_fwup_fsm; }

/*act init*/
bool dsm_imgtrfr_fwimage_act_init_process(uint8_t fw_type)
{
    uint32_t flash_addr;
    uint32_t flash_type;

    switch (fw_type)
    {
    case FW_DL_SYS_PART:
        flash_addr = SFLASH_SYS_FW_1_ADDR;
        dsm_imgtrfr_fwimage_set_sys_flash_addr(flash_addr);
        dsm_imgtrfr_set_start_dl_addr(flash_addr);

#if 1 /* bccho, 2023-09-30, SoC flash에 write */
        flash_type = FLASH_T_INT;
#else
        flash_type = FLASH_T_EXT;
#endif
        gst_sub_fw_info.flash_page_size =
            dsm_flash_get_flash_page_size(flash_type);
        gst_sub_fw_info.rcv_len = 0;

        dsm_imgtrfr_fw_subinfo_write();
        CMD_SE(flash_addr);
        DPRINTF(
            DBG_WARN,
            "SYSTEM FW: act_init - g_pre_sector[%d], flash_addr [0x%08X]\r\n",
            g_pre_sector, flash_addr);
        break;

    case FW_DL_INT_MDM:
        DPRINTF(DBG_WARN, "I-MDM FW: action init - direct\r\n");

        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_INIT);
        if (dsm_mdm_mic_fwup_direct_fsm_tx_proc(NULL, 0) == false)
            return false;
        break;

    case FW_DL_EXT_MDM:
        DPRINTF(DBG_WARN, "E-MDM FW: action init - direct\r\n");
        dsm_mdm_mic_fwup_set_fsm(FWU_FSM_INIT);
        if (dsm_mdm_mic_fwup_direct_fsm_tx_proc(NULL, 0) == false)
            return false;
        break;

    case FW_DL_METER_PART:
        DPRINTF(DBG_WARN, "METER FW: act init\r\n");
        dsm_sflash_fw_erase(E_SFLASH_METER_FW_BLK_T);
        flash_addr = dsm_sflash_fw_get_startaddr(E_SFLASH_METER_FW_BLK_T);
        dsm_imgtrfr_fwimage_set_sys_flash_addr(flash_addr);
        dsm_imgtrfr_set_start_dl_addr(flash_addr);

        break;
    }

    return true;
}

#define FEATURE_SF_WRITE_CHECK
uint32_t dsm_sflash_img_write(uint32_t addr, uint8_t* data, uint16_t len)
{
    ReturnMsg ret = 0;
    uint32_t bytes;
    uint32_t w_pos = 0;
#if defined(FEATURE_SF_WRITE_CHECK)
    uint32_t addr_backup = addr;
    uint32_t len_backup = len;
#endif

    if (addr > FlashSize || addr & 0x3)
        return false;

    if (len > Sector_Offset)
        return false;

    // DPRINTF("FLASH PP: addr(0x%X), len(%d)\r\n", addr, len);
    DPRINTF(DBG_TRACE, "%s: addr[0x%08X], len[%d]\r\n", __func__, addr, len);

    while (len > 0)
    {
        bytes = len > Page_Offset ? Page_Offset : len;
        ret = CMD_PP(addr, &data[w_pos], bytes);

        DPRINTF(DBG_TRACE, "addr(0x%X), bytes(%d), len(%d), ret(%d)\r\n", addr,
                bytes, len, ret);
        // DPRINT_HEX(DBG_TRACE, "SF_WRITE", &data[w_pos], 16, DUMP_SF);
        OSTimeDly(OS_MS2TICK(2));
        addr += bytes;
        len -= bytes;
        w_pos += bytes;
    }
#if defined(FEATURE_SF_WRITE_CHECK)
    {
        uint8_t rdata[512];
        if (len_backup > 512)
            len_backup = 512;
        memset(rdata, 0x00, len_backup);
        dsm_sflash_img_read(addr_backup, rdata, len_backup);
        if (strncmp((char*)data, (char*)rdata, len_backup))
        {
            DPRINTF(DBG_ERR, "%s: write error :: addr[0x%08X], len[%d]\r\n",
                    __func__, addr, len);
            OSTimeDly(OS_MS2TICK(100));
            ret = FlashWriteRegFailed;
        }
    }
#endif

    return ret;
}

uint32_t dsm_sflash_img_read(uint32_t addr, uint8_t* data, uint16_t len)
{
    if (addr > FlashSize || addr & 0x3)
        return false;

    if (len > Sector_Offset)
        return false;

    DPRINTF(DBG_TRACE, "%s: addr[0x%08X], len[%d]\r\n", __func__, addr, len);

    CMD_READ(addr, data, len);

    DPRINT_HEX(DBG_TRACE, "SF_READ", data, len, DUMP_SF);

    return true;
}

bool dsm_imgtrfr_fwimage_update(uint8_t fw_type, bool force, uint8_t* pblk,
                                uint16_t blk_size)
{
    bool ret = 0;
    uint32_t flash_addr = 0;

    switch (fw_type)
    {
    case FW_DL_SYS_PART:
        if (force == false)
        {
            ret = dsm_imgtrfr_fw_subimage_buff_update(pblk, blk_size);
        }
        else
        {
            // page size에 관계 없이 write
            ret = dsm_imgtrfr_fw_subimage_buff_update_force();
        }
        break;
    case FW_DL_INT_MDM:
        if (dsm_mdm_mic_fwup_direct_fsm_tx_proc(pblk, blk_size))
        {
            return true;
        }
        // jp.kim 25.03.15
        return false;
        break;

    case FW_DL_EXT_MDM:

        if (dsm_mdm_mic_fwup_direct_fsm_tx_proc(pblk, blk_size))
        {
            return true;
        }
        break;

    case FW_DL_METER_PART:
        flash_addr = dsm_imgtrfr_fwimage_get_sys_flash_addr();
        ret = dsm_sflash_img_write(flash_addr, pblk, blk_size);
        dsm_imgtrfr_fwimage_set_sys_flash_addr((flash_addr + blk_size));

        if (ret == 0)
            ret = true;

        break;
    }

    return ret;
}

#if 1 /* bccho, HASH, 2023-09-01 */
static ctx_handle_t hHandle = {[31] = 0xAA};
#endif
bool dsm_imgtrfr_fwimage_hash(uint8_t fw_type, uint8_t* pblk, uint16_t blk_size)
{
    int16_t ret = 0;
    uint8_t* phash = dsm_imgtrfr_get_hash(IMG__FW);  // get and set hash
    //    EN_IMG_TYPE img_type = IMG__FW;
    uint32_t blk_number;
    static uint16_t ImageBlockSize = 512;

    switch (fw_type)
    {
    case FW_DL_INT_MDM:
    case FW_DL_EXT_MDM:
    case FW_DL_SYS_PART:
        blk_number = dsm_imgtrfr_get_blk_num(IMG__FW);
        if (blk_number == 0)
        {
            ImageBlockSize = blk_size;
#if 1 /* bccho, HASH, 2023-09-01 */
            if (axiocrypto_allocate_slot(hHandle, ASYM_ECDSA_P256, 0) !=
                CRYPTO_SUCCESS)
            {
                MSGERROR("axiocrypto_allocate_slot, Fail");
                return FALSE;
            }
            CRYPTO_STATUS cret;
            if ((cret = axiocrypto_hash_init(hHandle, HASH_SHA_256)) !=
                CRYPTO_SUCCESS)
            {
                /* final 하기 전에 init을 하는 경우 발생 --> 펌웨어 다운로드
                 * 중지 하고 다시 시작하는 경우 */
                if (cret == CRYPTO_ERR_HASH_CTX_IN_USE)
                {
                    uint8_t dummy[IMAGE_HASH_SIZE];
                    axiocrypto_hash_final(hHandle, dummy, IMAGE_HASH_SIZE);
                    axiocrypto_hash_init(hHandle, HASH_SHA_256);
                }
                else
                {
                    MSGERROR("axiocrypto_hash_init, Fail, %d", cret);
                    return FALSE;
                }
            }
            if (axiocrypto_hash_update(hHandle, pblk, blk_size) !=
                CRYPTO_SUCCESS)
            {
                MSGERROR("axiocrypto_hash_update, Fail");
                return FALSE;
            }
#else
            ret = _amiFuVerifyBegin(AMI_FU_NO_VERI, pblk, blk_size, 0);
#endif /* bccho */

            DPRINTF(DBG_TRACE,
                    _D
                    "FW IMG Hash Begin: blk_num[%d], blk_len[%d], ret[%d]\r\n",
                    blk_number, blk_size, ret);
        }
        else
        {
            if (!dsm_imgtrfr_get_int_status_bit(IMG__FW,
                                                IMG_SBIT_POS_LAST_TRFR))
            {
                lp_event_set(LPE_PROGRAM_CHG);  // jp.kim 25.01.24

#if 1 /* bccho, HASH, 2023-09-01 */
                if (axiocrypto_hash_update(hHandle, pblk, blk_size) !=
                    CRYPTO_SUCCESS)
                {
                    MSGERROR("axiocrypto_hash_update, Fail");
                    return FALSE;
                }
#else
                ret = _amiFuVerifyMid(pblk, blk_size);
#endif /* bccho */
                DPRINTF(
                    DBG_TRACE,
                    _D "FW IMG Hash Mid: blk_num[%d], blk_len[%d], ret[%d]\r\n",
                    blk_number, blk_size, ret);
            }
            else
            {
#if 1 /* bccho, HASH, 2023-09-01 */
                if (axiocrypto_hash_update(hHandle, pblk, blk_size) !=
                    CRYPTO_SUCCESS)
                {
                    MSGERROR("axiocrypto_hash_update, Fail");
                    return FALSE;
                }
                if (axiocrypto_hash_final(hHandle, phash, IMAGE_HASH_SIZE) !=
                    CRYPTO_SUCCESS)
                {
                    MSGERROR("axiocrypto_hash_final, Fail");
                    return FALSE;
                }
#else
                ret = _amiFuVerifyEnd(pblk, blk_size, NULL);
                _amiGetFuCodeHash(phash);
#endif
                DPRINTF(
                    DBG_TRACE,
                    _D "FW IMG Hash End: blk_num[%d], blk_len[%d], ret[%d]\r\n",
                    blk_number, blk_size, ret);
                //------------------------------------------------------------------
                //------------------------------------------------------------------
                dsm_imgtrfr_set_int_status_bits(
                    IMG__FW, IMG_SBIT_VAL_HASH_GEN);  // hash gen complete flag
                DPRINT_HEX(DBG_TRACE, "FW_IMG_HASH", phash, 32, DUMP_ALWAYS);
            }
        }
        break;
    case FW_DL_METER_PART:
        blk_number = dsm_imgtrfr_get_blk_num(IMG__FW);

        if (blk_number == 0)
        {
#if 1  // jp.kim 25.12.06 // 계량부 무결성 검증
            g_eof_found = 0;
            g_total_data_len = 0;
            g_linepos = 0;
            g_meter_fw_chk_sum_err = 0;
#endif
            if (axiocrypto_allocate_slot(hHandle, ASYM_ECDSA_P256, 0) !=
                CRYPTO_SUCCESS)
            {
                MSGERROR("axiocrypto_allocate_slot, Fail");
                return FALSE;
            }
            if (axiocrypto_hash_init(hHandle, HASH_SHA_256) != CRYPTO_SUCCESS)
            {
                MSGERROR("axiocrypto_hash_init, Fail");
                return FALSE;
            }
            if (axiocrypto_hash_update(hHandle, pblk, blk_size) !=
                CRYPTO_SUCCESS)
            {
                MSGERROR("axiocrypto_hash_update, Fail");
                return FALSE;
            }
            DPRINTF(DBG_TRACE,
                    _D
                    "FW IMG Hash Begin: blk_num[%d], blk_len[%d], ret[%d]\r\n",
                    blk_number, blk_size, ret);

#if 1  // jp.kim 25.12.06 // 계량부 무결성 검증
            if (verify_packets(pblk, blk_size) != 0)
            {
                g_meter_fw_chk_sum_err = 1;
                DPRINTF(DBG_ERR, "Integrity check failed!  pk1 \r\n");
            }
#endif
        }
        else
        {
            if (!dsm_imgtrfr_get_int_status_bit(IMG__FW,
                                                IMG_SBIT_POS_LAST_TRFR))
            {
                if (axiocrypto_hash_update(hHandle, pblk, blk_size) !=
                    CRYPTO_SUCCESS)
                {
                    MSGERROR("axiocrypto_hash_update, Fail");
                    return FALSE;
                }

                DPRINTF(
                    DBG_TRACE,
                    _D "FW IMG Hash Mid: blk_num[%d], blk_len[%d], ret[%d]\r\n",
                    blk_number, blk_size, ret);

#if 1  // jp.kim 25.12.06 // 계량부 무결성 검증
                if (verify_packets(pblk, blk_size))
                {
                    g_meter_fw_chk_sum_err = 1;
                    DPRINTF(DBG_ERR, "Integrity check failed!  pk2 \r\n");
                }
#endif
            }
            else
            {
                uint8_t* phash = dsm_imgtrfr_get_hash(IMG__FW);

                if (axiocrypto_hash_update(hHandle, pblk, blk_size) !=
                    CRYPTO_SUCCESS)
                {
                    MSGERROR("axiocrypto_hash_update, Fail");
                    return FALSE;
                }
#if 1  // jp.kim 25.12.06 // 계량부 무결성 검증
       // 최종 check sum
                if (verify_packets(pblk, blk_size))
                {
                    g_meter_fw_chk_sum_err = 1;
                    DPRINTF(DBG_ERR, "Integrity check failed!  pk3 \r\n");
                }
                else if (g_meter_fw_chk_sum_err)
                    DPRINTF(DBG_ERR, "Integrity check failed!  all \r\n");
                else if (!g_eof_found)
                {
                    g_meter_fw_chk_sum_err = 1;
                    DPRINTF(DBG_ERR, "EOF record (:00000001FF) not found!\r\n");
                }
                else
                {
                    DPRINTF(DBG_ERR, "All packets verified successfully!\r\n");
                    DPRINTF(DBG_ERR, "Total data length = %d bytes\r\n",
                            g_total_data_len);
                }
#endif
                if (axiocrypto_hash_final(hHandle, phash, IMAGE_HASH_SIZE) !=
                    CRYPTO_SUCCESS)
                {
                    MSGERROR("axiocrypto_hash_final, Fail");
                    return FALSE;
                }

                dsm_imgtrfr_set_int_status_bits(IMG__FW, IMG_SBIT_VAL_HASH_GEN);
                DPRINTF(
                    DBG_TRACE,
                    _D "FW IMG Hash End: blk_num[%d], blk_len[%d], ret[%d]\r\n",
                    blk_number, blk_size, ret);
                DPRINT_HEX(DBG_TRACE, "FW_IMG_HASH", phash, 32, DUMP_ALWAYS);
            }
        }
        break;
    }

    return (ret ? TRUE : FALSE);
}

void dsm_swrst_init_bootflag(void)
{
    ST_SWRST_BOOT_FLAG swrst_boot_flag;

    DPRINTF(DBG_WARN, "%s\r\n", __func__);

    memset((uint8_t*)&swrst_boot_flag, 0x00, sizeof(ST_SWRST_BOOT_FLAG));

    nv_write(I_BOOT_AFTER_SWRST, (uint8_t*)&swrst_boot_flag);
}

void dsm_swrst_set_bootflag(void)
{
    ST_SWRST_BOOT_FLAG swrst_boot_flag;
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    memset((uint8_t*)&swrst_boot_flag, 0x00, sizeof(ST_SWRST_BOOT_FLAG));
    swrst_boot_flag.magic = 0xaabbcc33;

    nv_write(I_BOOT_AFTER_SWRST, (uint8_t*)&swrst_boot_flag);
}

bool dsm_swrst_is_bootflag(void)
{
    bool ret = false;
    ST_SWRST_BOOT_FLAG swrst_boot_flag;

    memset((uint8_t*)&swrst_boot_flag, 0x00, sizeof(ST_SWRST_BOOT_FLAG));

    if (nv_read(I_BOOT_AFTER_SWRST, (uint8_t*)&swrst_boot_flag))
    {
        if (swrst_boot_flag.magic == 0xaabbcc33)
        {
            ret = true;
        }
    }
    DPRINTF(DBG_TRACE, "%s: 0x%08X, ret[%d]\r\n", __func__,
            swrst_boot_flag.magic, ret);
    return ret;
}

void dsm_imgtrfr_fwimage_act_run_process(uint8_t fw_type)
{
    ST_FW_INFO fwinfo;
    uint8_t* p_name = &gst_fw_image_dlinfo.name[0];
    uint8_t* p_hash = &gst_fw_image_dlinfo.hash[0];

    memset((uint8_t*)&fwinfo, 0x00, sizeof(ST_FW_INFO));

    switch (fw_type)
    {
    case FW_DL_SYS_PART:
        dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_SYS);

        /* bccho, 2024-09-05, 삼상 */
        dsm_gpio_imodem_pf_low(); /* PF set */

        uint16_t crc;
        uint8_t temp[/*256*/ 64];

        crc = extflash_crc16_ccitt(gst_fw_image_dlinfo.start_dl_addr,
                                   gst_fw_image_dlinfo.image_size);
        CMD_SE(0x2000);
        CMD_PP(0x2000, (uint8_t*)&gst_fw_image_dlinfo.image_size, 4);
        CMD_PP(0x2004, (uint8_t*)&crc, 2);

        /* Update F/W Information */
        memcpy(fwinfo.version, &p_name[FW_BANK_FW_INFO_VER_ST_IDX],
               FW_VERSION_SIZE);
        memcpy(fwinfo.date_time, &p_name[FW_BANK_FW_INFO_GEN_DATE],
               FW_GENERATION_DATE_SIZE);
        memcpy(&fwinfo.dt, (uint8_t*)&cur_rtc, sizeof(date_time_type));
        memcpy(fwinfo.hash, p_hash, IMAGE_HASH_SIZE);
        memcpy(fwinfo.mt_type, &p_name[FW_BANK_FW_INFO_METER_TYPE_IDX], 2);
        fwinfo.update_t = FWINFO_UPDATE_T_DOWNLOAD;

        // update fw info write
        CMD_PP(0x2006, (uint8_t*)&fwinfo, sizeof(ST_FW_INFO));

        CMD_READ(0, temp, 2);
        temp[1] = 1;  // fw update req
        CMD_SE(0);
        CMD_PP(0, temp, 2);

        CMD_READ(0x2000, temp, sizeof(ST_FW_INFO) + 6);
        DPRINT_HEX(4, "FW_UP_INFO", temp, sizeof(ST_FW_INFO) + 6, DUMP_ALWAYS);
        dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_SYS);

        log_sys_sw_up();
        dsm_swrst_set_bootflag();

        whm_data_save_sag();

        dsm_uart_deq_string(DEBUG_COM);
#if 1 /* bccho, NVIC_SystemReset, 2023-07-15 */
#include "boot_restore.h"
        BOOT_RESTORE br;
#ifdef STOCK_OP /* bccho, 2024-09-26 */
        ST_RAND_TX_INFO* p_rand_txinfo = dsm_stock_op_get_rand_txinfo();
        br.fsm = dsm_stock_op_get_fsm();
        br.sec_1 = p_rand_txinfo->sec_1;
        br.ms_1 = p_rand_txinfo->ms_1;
        br.sec_2 = p_rand_txinfo->sec_2;
        br.ms_2 = p_rand_txinfo->ms_2;
        br.seed = p_rand_txinfo->seed;
#endif
        br.bank = get_current_bank_S() ? 0 : 1;

        if (FMC_Erase_S(BOOT_RESTORE_BASE) != 0)
        {
            MSGERROR("FMC_Erase Data Flash\n");
        }
        if (FMC_WriteMultiple_S(BOOT_RESTORE_BASE, (uint32_t*)&br,
                                sizeof(BOOT_RESTORE)) <= 0)
        {
            MSGERROR("FMC_WriteMultiple_S\n");
        }

        goto_loader_S();
#else
        NVIC_SystemReset();
        while (1);
#endif
        break;
    case FW_DL_INT_MDM:
    case FW_DL_EXT_MDM:
    case FW_DL_METER_PART:
        if (FW_DL_INT_MDM == fw_type)
        {
            dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_MODEM);
        }
        else if (FW_DL_EXT_MDM == fw_type)
        {
            dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_E_MODEM);
        }
        else
        {
            dsm_imgtrfr_fwinfo_read((uint8_t*)&fwinfo, FWINFO_CUR_METER);
        }

        /* Update F/W Information */
        memcpy(fwinfo.version, &p_name[FW_BANK_FW_INFO_VER_ST_IDX],
               FW_VERSION_SIZE);
        memcpy(fwinfo.date_time, &p_name[FW_BANK_FW_INFO_GEN_DATE],
               FW_GENERATION_DATE_SIZE);
        memcpy(&fwinfo.dt, (uint8_t*)&cur_rtc, sizeof(date_time_type));
        memcpy(fwinfo.hash, p_hash, IMAGE_HASH_SIZE);
        memcpy(fwinfo.mt_type, &p_name[FW_BANK_FW_INFO_METER_TYPE_IDX], 2);
        fwinfo.update_t = FWINFO_UPDATE_T_DOWNLOAD;

        if (FW_DL_INT_MDM == fw_type)
        {
            dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_MODEM);
            DPRINTF(DBG_TRACE, "I-modem fw transfer via AT command\r\n");
        }
        else if (FW_DL_EXT_MDM == fw_type)
        {
#if 0            
            g_modem_exist = true;  // jp.kim 25.01.20
#endif
            dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_E_MODEM);
            DPRINTF(DBG_TRACE, "EXT-modem fw transfer via AT command\r\n");
        }
        else
        {
            dsm_imgtrfr_fwinfo_write((uint8_t*)&fwinfo, FWINFO_CUR_METER);
            DPRINTF(DBG_TRACE, "MeterIC fw transfer via MIF\r\n");
        }

        if ((FW_DL_INT_MDM == fw_type) || (FW_DL_EXT_MDM == fw_type))
        {
            dsm_mdm_mic_fwup_set_fsm(FWU_FSM_UP_OP);
            if (dsm_mdm_mic_fwup_direct_fsm_tx_proc(NULL, 0))
            {
                if (FW_DL_INT_MDM == fw_type)
                    log_int_modem_up();
                else
                    log_ext_modem_up();
            }
        }
        else
        {
            dsm_mtp_set_fw_type(/*3*/ MTP_FW_TYPE_START);
            dsm_mtp_fsm_tx_proc_fw_data_set(/*3*/ MTP_FW_TYPE_START, 0);
            // dsm_mtp_fsm_send();
        }

        break;
    }
}

/********************/
/*sub fw image*/
/********************/
bool dsm_imgtrfr_fw_subimage_buff_update(uint8_t* pblk, uint16_t blk_size)
{
    uint32_t flash_addr;
    uint16_t pos = gst_sub_fw_info.rcv_len;

    DPRINTF(DBG_TRACE, _D "%s: pos[%d], size[%d]\r\n", __func__, pos, blk_size);

    memcpy(&gst_sub_fw_img.data[pos], pblk, blk_size);
    DPRINT_HEX(DBG_TRACE, "BLK_SUB", &gst_sub_fw_img.data[pos], blk_size,
               DUMP_DLMS);

    dsm_imgtrfr_fw_subimage_write(pblk, gst_sub_fw_info.rcv_len, blk_size);

    gst_sub_fw_info.rcv_len += blk_size; /* blk_size: 512 bytes */

    if (gst_sub_fw_info.rcv_len >=
        gst_sub_fw_info.flash_page_size) /* flash_page_size: 2048 bytes */
    {
        flash_addr = dsm_imgtrfr_fwimage_get_sys_flash_addr();

        DPRINTF(DBG_WARN,
                _D "%s: f_addr[0x%08X], len/4[%d], sector[%d, %d]\r\n",
                __func__, flash_addr, gst_sub_fw_info.rcv_len / 4, g_pre_sector,
                FLASH_If_GetPage(flash_addr));
#if 1 /* bccho, 2023-09-30, SoC flash에 write */
        OTA_WriteNewFW_S(flash_addr, &gst_sub_fw_img.data[0],
                         (uint32_t)gst_sub_fw_info.rcv_len);
#else
        xmodem_flash_update(flash_addr, &gst_sub_fw_img.data[0],
                            (uint32_t)gst_sub_fw_info.rcv_len);
#endif
        gst_sub_fw_info.rcv_len = 0;
        dsm_imgtrfr_fwimage_set_sys_flash_addr(
            (flash_addr + gst_sub_fw_info.flash_page_size));

        dsm_imgtrfr_fw_subinfo_write();

        return true;
    }
    else
    {
        dsm_imgtrfr_fw_subinfo_write();
    }

    return false;
}

bool dsm_imgtrfr_fw_subimage_buff_update_force(void)
{
    uint32_t flash_addr;

    flash_addr = dsm_imgtrfr_fwimage_get_sys_flash_addr();

    DPRINTF(DBG_WARN, _D "%s: flash_addr[0x%08X], len/4[%d]\r\n", __func__,
            flash_addr, gst_sub_fw_info.rcv_len / 4);

#if 1 /* bccho, 2023-09-30, SoC flash에 write */
    OTA_WriteNewFW_S(flash_addr, &gst_sub_fw_img.data[0],
                     (uint32_t)gst_sub_fw_info.flash_page_size);
#else
    xmodem_flash_update(flash_addr, &gst_sub_fw_img.data[0],
                        (uint32_t)gst_sub_fw_info.flash_page_size);
#endif
    gst_sub_fw_info.rcv_len = 0;

    dsm_imgtrfr_fwimage_set_sys_flash_addr(
        (flash_addr + gst_sub_fw_info.rcv_len));

    return true;
}

uint8_t* dsm_imgtrfr_fw_subimage_get_buff(void)
{
    return &gst_sub_fw_img.data[0];
}

void dsm_imgtrfr_fw_subimage_write(uint8_t* pblk, uint32_t pos, uint32_t w_size)
{
    nv_sub_info.i16[0] = pos;
    nv_sub_info.i16[1] = w_size;

    DPRINTF(DBG_TRACE, _D "%s: pos[%d], size[%d]\r\n", __func__, pos, w_size);

    nv_write(I_SUB_FW_IMG_DATA, pblk);
}

bool dsm_imgtrfr_fw_subinfo_read(uint8_t* pinfo)
{
    bool ret = 0;

    ret = nv_read(I_SUB_FW_IMG_DOWNLOAD_INFO, pinfo);

    DPRINTF(DBG_TRACE, _D "%s: ret[%d]\r\n", __func__, ret);
    DPRINT_HEX(DBG_TRACE, "SUB_FW_INFO", pinfo, sizeof(ST_SUB_FW_IMG_DL_INFO),
               DUMP_ALWAYS);

    return ret;
}

bool dsm_imgtrfr_fw_subinfo_write(void)
{
    bool ret = 0;

    DPRINTF(DBG_TRACE, _D "%s: flash_page_size[%d], rcv_len[%d]\r\n", __func__,
            gst_sub_fw_info.flash_page_size, gst_sub_fw_info.rcv_len);

    nv_write(I_SUB_FW_IMG_DOWNLOAD_INFO, (uint8_t*)&gst_sub_fw_info);

    return ret;
}

bool dsm_imgtrfr_fw_subimage_restore(void)
{
    if (!nv_read(I_SUB_FW_IMG_DOWNLOAD_INFO, (uint8_t*)&gst_sub_fw_info))
    {
        DPRINTF(DBG_ERR, _D "%s: SUB fw img download restore fail !!!\r\n",
                __func__);
        memset(&gst_sub_fw_info, 0x00, sizeof(ST_SUB_FW_IMG_DL_INFO));
    }

    DPRINTF(DBG_TRACE, _D "%s: flash_page_size[%d], rcv_len[%d]\r\n", __func__,
            gst_sub_fw_info.flash_page_size, gst_sub_fw_info.rcv_len);
    if (gst_sub_fw_info.flash_page_size == 0)
        return FALSE;

    if (gst_sub_fw_info.rcv_len != 0)
    {
        nv_sub_info.i16[0] = 0;
        nv_sub_info.i16[1] = gst_sub_fw_info.rcv_len;

        nv_read(I_SUB_FW_IMG_DATA, &gst_sub_fw_img.data[0]);
        DPRINT_HEX(DBG_TRACE, "SUB_IMG", &gst_sub_fw_img.data[0],
                   gst_sub_fw_info.rcv_len, DUMP_DLMS);

        return TRUE;
    }

    return FALSE;
}

/******************/
/* TOU image only */
/******************/
void dsm_imgtrfr_touimage_buff_update(uint8_t* pblk, uint32_t pre_rcv_size,
                                      uint16_t blk_size)
{
    DPRINTF(DBG_TRACE, _D "%s: pos[%d], size[%d]\r\n", __func__, pre_rcv_size,
            blk_size);
    memcpy(&gst_tou_image_data.image[pre_rcv_size], pblk, blk_size);
    DPRINT_HEX(DBG_TRACE, "BLK", &gst_tou_image_data.image[pre_rcv_size],
               blk_size, DUMP_ALWAYS);
}

uint8_t* dsm_imgtrfr_touimage_get_buff(void)
{
    return &gst_tou_image_data.image[0];
}

void dsm_imgtrfr_touimage_write(uint8_t* pblk, uint32_t pos, uint32_t w_size)
{
    nv_sub_info.i16[0] = pos;
    nv_sub_info.i16[1] = w_size;

    DPRINTF(DBG_TRACE, _D "%s: BLK_NUM[%d], pos[%d], size[%d]\r\n", __func__,
            gst_tou_image_info.blk_number, pos, w_size);
    DPRINT_HEX(DBG_TRACE, "BLK[W]", pblk, w_size, DUMP_AMI);

    nv_write(I_TOU_IMG_DATA, pblk);
}

bool dsm_imgtrfr_touimage_read(uint8_t* pblk, uint32_t pos, uint32_t r_size)
{
    bool ret = 0;

    nv_sub_info.i16[0] = pos;
    nv_sub_info.i16[1] = r_size;

    memset(pblk, 0x00, r_size);

    ret = nv_read(I_TOU_IMG_DATA, pblk);

    DPRINTF(DBG_TRACE, _D "%s: BLK_NUM[%d], pos[%d], size[%d], ret[%d]\r\n",
            __func__, gst_tou_image_info.blk_number, pos, r_size, ret);
    DPRINT_HEX(DBG_TRACE, "BLK[R]", pblk, r_size, DUMP_DLMS);

    return ret;
}

void dsm_imgtrfr_touimage_restore(void)
{
    if (!nv_read(I_TOU_IMG_INFO, (uint8_t*)&gst_tou_image_info))
    {
        DPRINTF(DBG_ERR, _D "%s: default 1\r\n", __func__);
        dsm_imgtrfr_con_init(IMG__TOU);
    }

    DPRINTF(DBG_TRACE,
            "%s: transfer_enabled[%d], int_status_bits[0x%02X], "
            "transfer_status[%d]\r\n",
            __func__, gst_tou_image_info.transfer_enabled,
            gst_tou_image_info.int_status_bits,
            gst_tou_image_info.transfer_status);
    DPRINTF(DBG_TRACE, "%s: img_size[%ld], rcv_size[%ld]\r\n", __func__,
            gst_tou_image_info.image_size, gst_tou_image_info.image_rcv_size);
    DPRINTF(DBG_TRACE, "%s: blk_number[%ld], blk_size[%ld]\r\n", __func__,
            gst_tou_image_info.blk_number, gst_tou_image_info.blk_size);

    if (gst_tou_image_info.transfer_enabled)
    {
        uint32_t pos = 0;
        uint32_t r_size =
            (gst_tou_image_info.blk_number) * gst_tou_image_info.blk_size;

        r_size = TOU_IMAGE_MAX_SIZE;

        if (dsm_imgtrfr_touimage_read((uint8_t*)&gst_tou_image_data, pos,
                                      r_size))
        {
            DPRINT_HEX(DBG_TRACE, "T_IMG", &gst_tou_image_data, r_size,
                       DUMP_TOU);
        }
    }
}

void dsm_imgtrfr_touimage_info_save(void)
{
    nv_write(I_TOU_IMG_INFO, (uint8_t*)&gst_tou_image_info);
}

void DecToAscii(uint8_t* result, uint8_t dig, uint32_t dat)
{
    uint8_t i, divout;
    uint32_t modout, modb = 0, sumdigit[2] = {0};

    if (dig == 4)
        modb = 1000;
    else if (dig == 7)
        modb = 1000000;

    for (i = 0; i < dig; i++)
    {
        modout = dat % modb;
        sumdigit[1] = dat - sumdigit[0] - modout;
        divout = (uint8_t)(sumdigit[1] / modb);
        result[i] = divout + 0x30;
        modb /= 10;
        sumdigit[0] += sumdigit[1];
    }
}

void genMid11(uint8_t* dst_buf, uint16_t first, uint32_t second)
{
    DecToAscii(&dst_buf[0], 4, first);
    DecToAscii(&dst_buf[4], 7, second);
}

uint32_t dsm_touSP_parserNprocess(uint8_t* pimg, uint16_t* o_idx,
                                  ST_TOU_HEADER_INFO* p_hd)
{
    uint8_t cnt, name;
    uint16_t idx = 0;

    season_date_type season_info;

    season_info.cnt = pimg[idx++];

    DPRINTF(DBG_TRACE, "%s: SP cnt[%d]\r\n", __func__, season_info.cnt);

    if (season_info.cnt > SEASON_PROF_SIZE)
        season_info.cnt = SEASON_PROF_SIZE;

    for (cnt = 0; cnt < season_info.cnt; cnt++)
    {
        name = pimg[idx++];
        season_info.season[name].month = pimg[idx++];
        season_info.season[name].date = pimg[idx++];
        season_info.season[name].week_id = pimg[idx++];

        DPRINTF(DBG_TRACE, "SP name[%d], month[%d], data[%d], week_id[%d]\r\n",
                name, season_info.season[name].month,
                season_info.season[name].date,
                season_info.season[name].week_id);
    }

    if (nv_write(I_SEASON_PROFILE_DL, (uint8_t*)&season_info))
    {
        pdl_set_bits |= SETBITS_TOU_SEASON;
    }

    *o_idx = idx;

    return TRUE;
}

uint32_t dsm_touWP_parserNprocess(uint8_t* pimg, uint16_t* o_idx,
                                  ST_TOU_HEADER_INFO* p_hd)
{
    uint8_t cnt;
    uint16_t idx = 0;

    week_date_type week_info;

    week_info.cnt = pimg[idx++];

    DPRINTF(DBG_TRACE, "%s: WP cnt[%d]\r\n", __func__, week_info.cnt);

    for (cnt = 0; cnt < week_info.cnt; cnt++)
    {
        week_info.week[cnt].week_id = pimg[idx++];
        memcpy(week_info.week[cnt].day_id, &pimg[idx], WEEK_LEN);
        idx += WEEK_LEN;

        DPRINTF(DBG_TRACE, "week[%d], week_id[%d]\r\n", cnt,
                week_info.week[cnt].week_id);
        DPRINT_HEX(DBG_TRACE, "day_id", &week_info.week[cnt].day_id, WEEK_LEN,
                   DUMP_ALWAYS);
    }

    if (nv_write(I_WEEK_PROFILE_DL, (uint8_t*)&week_info))
    {
        pdl_set_bits |= SETBITS_TOU_WEEK;
    }

    *o_idx = idx;

    return TRUE;
}

bool dsm_is_day_id_backup(ST_IMG_DAY_ID_BACKUP* back, uint8_t day_id)
{
    uint8_t cnt;

    for (cnt = 0; cnt < back->cnt; cnt++)
    {
        if (back->id[cnt] == day_id)
        {
            return true;
        }
    }
    return false;
}

void dsm_set_day_id_backup(ST_IMG_DAY_ID_BACKUP* back, uint8_t day_id)
{
    back->id[back->cnt] = day_id;
    back->cnt++;
}

uint32_t dsm_touDP_parserNprocess(uint8_t* pimg, uint16_t* o_idx,
                                  ST_TOU_HEADER_INFO* p_hd)
{
    uint8_t cnt, cnt2, dp_cnt;
    uint16_t idx = 0;

    dayid_table_type dayid_info;
    ST_IMG_DAY_ID_BACKUP st_backup;

    memset(&dayid_info, 0x00, sizeof(dayid_table_type));
    memset(&st_backup, 0x00, sizeof(ST_IMG_DAY_ID_BACKUP));

    dp_cnt = pimg[idx++];

    DPRINTF(DBG_TRACE, _D "%s: DP cnt[%d]\r\n", __func__, dp_cnt);

    if (dp_cnt > DAY_PROF_SIZE)
        dp_cnt = DAY_PROF_SIZE;

    for (cnt = 0; cnt < dp_cnt; cnt++)
    {
        dayid_info.day_id = pimg[idx++];
        dayid_info.tou_conf_cnt = pimg[idx++];

        dsm_set_day_id_backup(&st_backup, dayid_info.day_id);

        DPRINTF(DBG_TRACE, "day_id[%d], %d\r\n", dayid_info.day_id,
                dayid_info.tou_conf_cnt);

        if (dayid_info.tou_conf_cnt > MAX_TOU_DIV_DLMS)
            dayid_info.tou_conf_cnt = MAX_TOU_DIV_DLMS;

        for (cnt2 = 0; cnt2 < dayid_info.tou_conf_cnt; cnt2++)
        {
            dayid_info.tou_conf[cnt2].hour = pimg[idx++];
            dayid_info.tou_conf[cnt2].min = pimg[idx++];
            dayid_info.tou_conf[cnt2].rate = pimg[idx++];

            DPRINTF(DBG_TRACE, "\thour[%d], min[%d], rate[%d]\r\n",
                    dayid_info.tou_conf[cnt2].hour,
                    dayid_info.tou_conf[cnt2].min,
                    dayid_info.tou_conf[cnt2].rate);
        }

        nv_sub_info.ch[0] = dayid_info.day_id;
        nv_write(I_DAY_PROFILE_DL, (uint8_t*)&dayid_info);
    }

    for (cnt = 0; cnt < DAY_PROF_SIZE; cnt++)
    {
        if (dsm_is_day_id_backup(&st_backup, cnt) == false)
        {
            dayid_info.day_id = cnt;
            dayid_info.tou_conf_cnt = 0;

            DPRINTF(DBG_INFO, "day_id[%d], %d\r\n", dayid_info.day_id,
                    dayid_info.tou_conf_cnt);

            for (cnt2 = 0; cnt2 < MAX_TOU_DIV_DLMS; cnt2++)
            {
                dayid_info.tou_conf[cnt2].hour = 0xff;
                dayid_info.tou_conf[cnt2].min = 0xff;
                dayid_info.tou_conf[cnt2].rate = 1;

                DPRINTF(DBG_INFO, "\thour[%d], min[%d], rate[%d]\r\n",
                        dayid_info.tou_conf[cnt2].hour,
                        dayid_info.tou_conf[cnt2].min,
                        dayid_info.tou_conf[cnt2].rate);
            }
            nv_sub_info.ch[0] = dayid_info.day_id;
            nv_write(I_DAY_PROFILE_DL, (uint8_t*)&dayid_info);
        }
    }

    pdl_set_bits |= SETBITS_TOU_DAY;

    *o_idx = idx;

    return TRUE;
}

#define SDT_ZERO_YEAR_OFFSET_BLK_0 63
#define SDT_ZERO_HOLIDAYS_PER_BLOCK_BLK_0 8
#define SDT_ZERO_YEAR_OFFSET_BLK_1 21
#define SDT_ZERO_HOLIDAYS_PER_BLOCK_BLK 0
holiday_struct_type g_holiday_blk_zero[SDT_ZERO_HOLIDAYS_PER_BLOCK_BLK] = {

};

uint32_t dsm_touSDT_parserNprocess(uint8_t* pimg, uint16_t* o_idx,
                                   ST_TOU_HEADER_INFO* p_hd)
{
    uint8_t cnt, *pt8;
    uint16_t idx = 0, base_year, sdt_cnt;
    uint8_t date, month, year_off, day_id, pre_year_off = 0;
    uint16_t index, pre_index = 0;

    holiday_date_type hol_date_block;

    memset((uint8_t*)&hol_date_block, 0xff, sizeof(holiday_date_type));
    hol_date_block.arr_len = 0;

    /* Base Year */
    ToH16((U8_16_32*)&base_year, &pimg[idx]);
    idx += 2;

    /* SDT 개수 */
    ToH16((U8_16_32*)&sdt_cnt, &pimg[idx]);
    idx += 2;

    /*
    ○ Base Year (기준년도) : 기준년도 + year-off = 비정기 휴일의 실제 년도
    ○ index는 오름차순으로 정렬하여 구성한다(불연속적일 수 있음)
    ○ Default ASDT의 호환성을 위하여 휴일(Spacial day)의 day_id는 1로 정의한다.
    ○ Index의 값이 0 ~ 19인 경우, year off 와일드카드 값 채움
    ○ DLMS표준 datetime의 와일드카드 처리의 경우, 각 필드별 채움 값, year-off :
    63 /  month : 15 /  date : 63

        SDT 4 Bytes :
            bits[31:28] day_id
            bits[27:25] Reserved (0 으로 채움)
            bits[24:16] index
            bits[15:10] year-off
            bits[9:6] month
            bits[5:0] date

        ex) MSB     LSB
            10 00 FC 41 : day_id 1, index 0, 63년(와일드카드) 1월 1일

        와일드 카드는 정기적으로 반복되는 휴일인 듯. (매년, 매월, 매일)
    */
    DPRINTF(DBG_TRACE, _D "%s: BASE_YEAR[%d], SDT_NUM[%d]\r\n", __func__,
            base_year, sdt_cnt);

    for (cnt = 0; cnt < 21; cnt++)
    {
        nv_sub_info.ch[0] = cnt;

        memset((uint8_t*)&hol_date_block, 0xff, sizeof(holiday_date_type));

        if (cnt == 0)
            hol_date_block.yr = SDT_ZERO_YEAR_OFFSET_BLK_0;
        else
            hol_date_block.yr = SDT_ZERO_YEAR_OFFSET_BLK_1 + (cnt - 1);

        hol_date_block.arr_len = SDT_ZERO_HOLIDAYS_PER_BLOCK_BLK;

        DPRINTF(DBG_TRACE, "BLK_IDX[%d], YEAR[%d], NUM[%d]\r\n",
                nv_sub_info.ch[0], hol_date_block.yr, hol_date_block.arr_len);
        nv_write(I_HOLIDAYS_DL, (uint8_t*)&hol_date_block);
        nv_write(I_HOL_DATE_BLOCK, (uint8_t*)&hol_date_block);
    }

    for (cnt = 0; cnt < sdt_cnt; cnt++)
    {
        pt8 = &pimg[idx];
        date = pt8[SDT_B0] & MASK_6BITS;
        month =
            (((pt8[SDT_B0] >> 6) & MASK_2BITS) | ((pt8[SDT_B1] << 2) & 0xC));
        year_off = ((pt8[SDT_B1] >> 2) & MASK_6BITS);
        index = (pt8[SDT_B2]) | ((pt8[SDT_B3] & MASK_1BIT) << 8);
        day_id = (pt8[SDT_B3] >> 4) & MASK_4BITS;

        idx += 4;

        if (cnt == 0)
        {
            pre_year_off = year_off;
        }

        if (year_off == pre_year_off)
        {
            hol_date_block.yr = year_off;
            hol_date_block.holiday[hol_date_block.arr_len].date = date;
            hol_date_block.holiday[hol_date_block.arr_len].month = month;
            hol_date_block.holiday[hol_date_block.arr_len].day_id = day_id;
            hol_date_block.arr_len++;

            pre_index = index;
            pre_year_off = year_off;

            if ((cnt + 1) >= sdt_cnt)
            {
                if (pre_index < 20)
                    nv_sub_info.ch[0] = 0;
                else
                    nv_sub_info.ch[0] = pre_index / 20;

                {
                    int i = 0;

                    DPRINTF(DBG_TRACE, "BLK_IDX[%d], YEAR[%d], NUM[%d]\r\n",
                            nv_sub_info.ch[0], hol_date_block.yr,
                            hol_date_block.arr_len);

                    for (i = 0; i < hol_date_block.arr_len; i++)
                    {
                        DPRINTF(DBG_TRACE,
                                "\tDAY_ID[%d], MONTH[%02d], DATE[%02d]\r\n",
                                hol_date_block.holiday[i].day_id,
                                hol_date_block.holiday[i].month,
                                hol_date_block.holiday[i].date);
                    }
                }
                if (nv_write(I_HOLIDAYS_DL, (uint8_t*)&hol_date_block))
                {
                    memset((uint8_t*)&hol_date_block, 0xff,
                           sizeof(holiday_date_type));
                }
                nv_write(I_HOL_DATE_BLOCK, (uint8_t*)&hol_date_block);
            }
        }
        else
        {
            if (pre_index < HOLIDAYS_PER_BLOCK)
                nv_sub_info.ch[0] = 0;
            else
                nv_sub_info.ch[0] = (uint8_t)(pre_index / HOLIDAYS_PER_BLOCK);

            {
                int i = 0;

                DPRINTF(DBG_TRACE, "BLK_IDX[%d], YEAR[%d], NUM[%d]\r\n",
                        nv_sub_info.ch[0], hol_date_block.yr,
                        hol_date_block.arr_len);

                for (i = 0; i < hol_date_block.arr_len; i++)
                {
                    DPRINTF(DBG_TRACE,
                            "\tDAY_ID[%d], MONTH[%02d], DATE[%02d]\r\n",
                            hol_date_block.holiday[i].day_id,
                            hol_date_block.holiday[i].month,
                            hol_date_block.holiday[i].date);
                }
            }

            if (nv_write(I_HOLIDAYS_DL, (uint8_t*)&hol_date_block))
            {
                memset((uint8_t*)&hol_date_block, 0xff,
                       sizeof(holiday_date_type));
                hol_date_block.arr_len = 0;
            }
            nv_write(I_HOL_DATE_BLOCK, (uint8_t*)&hol_date_block);

            hol_date_block.yr = year_off;
            hol_date_block.holiday[hol_date_block.arr_len].date = date;
            hol_date_block.holiday[hol_date_block.arr_len].month = month;
            hol_date_block.holiday[hol_date_block.arr_len].day_id = day_id;
            hol_date_block.arr_len++;

            pre_index = index;
            pre_year_off = year_off;
        }
    }

    pdl_set_bits |= SETBITS_HOLIDAYS;

    *o_idx = idx;

    return TRUE;
}

uint32_t dsm_touETC_decoder_n_obset(uint8_t class_id, uint8_t* p_obis,
                                    uint8_t att_id, uint8_t* p_val,
                                    uint16_t val_len)
{
    myobj_struct_type* object =
        dsm_touETC_get_object((uint16_t)class_id, p_obis);

    // DPRINTF(DBG_NONE, _D"OBJ_IDX[%d], CLASS_ID[%d]\r\n", object->obj,
    // object->class_id);
    //    DPRINT_HEX(DBG_NONE, "OBJ_OBIS", object->instance_id, 6, DUMP_DLMS);

    switch ((uint16_t)object->obj)
    {
    case OBJ_SUPPLY_DISP_MODE: /* 디스플레이 */
        dsm_touETC_suply_disp(p_val, val_len);

        break;
    case OBJ_NPERIOD_BILLDATE: /* 비정기 검침일 */
        dsm_touETC_nperiod_billdate(att_id, p_val, val_len);

        break;
    case OBJ_DATE_TIME: /* 일광절약 주기 */
        dsm_touETC_daylight_saving_enabled(att_id, p_val, val_len);

        break;
    case OBJ_BILLING_PARM:
        dsm_touETC_billing_parm(att_id, p_val, val_len);

        break;
    case OBJ_LCDSET_PARM: /*계량모드 설정 파라미터*/
        dsm_touETC_lcdset_parm(att_id, p_val, val_len);

        break;
    case OBJ_PERIOD_BILLDATE: /* 정기 검침일 */
        dsm_touETC_period_billdate(att_id, p_val, val_len);

        break;
    case OBJ_LP_INTERVAL: /* LP 기록 주기 */
        dsm_touETC_lp_interval(att_id, p_val, val_len);

        break;
    case OBJ_CURR_LAST_DEMAND: /* 현재/직전 수요시한 수요전력 */
        dsm_touETC_curr_last_demand(att_id, p_val, val_len);

        break;
    case OBJ_TOU_CAL: /* TOU Activity Calendar */
        /* [active_passive_calendar_time] : 예약 프로그램 적용 일자/시간 */
        dsm_touETC_active_passive_time(att_id, p_val, val_len);
        break;
        // Ref. touset_parse_holidays() & obset_holiday_sel()
        /*
        typedef struct _SpecialDaysTable
        {
            uint32_t index; // 0 ~ 19 : 정기 휴일, 20 ~ 419 : 비정기 휴일
            uint8_t special_day_date[5];
            uint8_t day_id; // 요일별 적용 단계 (0 ~ 9)
        } SDT_Entries_t;
        */
    case OBJ_HOLIDAY_SEL: /* 정기/비정기 휴일 적용 */
        // OBIS : 1 0 128 0 18 255 (0x01, 0x00, 0x80, 0x00, 0x12, 0xFF)
        dsm_touETC_holiday_sel(att_id, p_val, val_len);  // 테스트 필요함.
        break;

    default:
        DPRINTF(DBG_NONE, "Object Not Found\r\n");
        break;
    }

    return TRUE;
}

uint32_t dsm_touETC_parserNprocess(uint8_t* pimg, uint16_t* o_idx,
                                   ST_TOU_HEADER_INFO* p_hd)
{
    uint8_t cnt, att_id, *p_val;
    uint16_t idx = 0, etc_num, slen;
    uint8_t* p_obis;
    uint16_t class_id;

    etc_num = pimg[idx++];

    DPRINTF(DBG_TRACE, _D "ETC_NUM[%d]\r\n", etc_num);

    for (cnt = 0; cnt < etc_num; cnt++)
    {
        if (SFLAG == pimg[idx++])
        {
            ToH16((U8_16_32*)&slen, &pimg[idx]);
            idx += 2;
            ToH16((U8_16_32*)&class_id, &pimg[idx]);
            idx += 2;
            p_obis = &pimg[idx];
            idx += 6;  // obis(6)
            att_id = pimg[idx++];
            p_val = &pimg[idx];

            DPRINTF(DBG_TRACE, "ETC[%d]: LEN[%d], Class[%d], Attribute[%d]\r\n",
                    cnt, slen, class_id, att_id);
            DPRINT_HEX(DBG_TRACE, "OBIS", p_obis, 6, DUMP_ALWAYS);
            DPRINT_HEX(DBG_TRACE, "VAL", p_val, slen - 9, DUMP_DLMS | DUMP_TOU);

            dsm_touETC_decoder_n_obset(class_id, p_obis, att_id, p_val,
                                       slen - 9);

            idx += (slen - 9);
        }
    }

    *o_idx = idx;

    return TRUE;
}

uint32_t dsm_touHeader_parser(uint8_t* pimg, uint16_t len, uint16_t* o_idx,
                              ST_TOU_HEADER_INFO* p_o_hd)
{
    uint8_t t8, cnt, *pt8;
    uint16_t idx = 0;

    DPRINTF(DBG_WARN, "%s: img_len[%d]\r\n", __func__, len);
    // 1. tou name
    memcpy(p_o_hd->tou_name, &pimg[idx], IMAGE_NAME_MAX_SIZE);
    idx += IMAGE_NAME_MAX_SIZE;

    DPRINT_HEX(DBG_TRACE, "TOU_NAME", p_o_hd->tou_name, IMAGE_NAME_MAX_SIZE,
               DUMP_ALWAYS);

    /*
    TODO: (WD) TOU Image, 통신 규격 "3.4.2.8.22 TOU Image Transfer"
        ※ 참조한 FEP 프로토콜이 옛날 규격인가봄. 시발 무시하셈. ITOU 및 Mlist
    항목은 AMIGO 계기에서 N/A 임. 의미없음.

        ITOU[11](Information of TOU, TOU 정보의 제어 및 요약정보) : 11 Bytes
            ITOU Control[1] : 1 Byte,  TOU 정보 제어 및 구성 정보 (0b0001x10x :
    1C, 1D, 14, 15) Control bits[0] TOU 모드 - 0: 현재 TOU 설정 수행, 1: 예약
    TOU 설정 수행 Control bits[1] TOU 구성 - 0: Header + Body 구성, 1: Header
    Only 구성 Control bits[2] 휴일 설정 - 1: 전체 휴일 대체 (Special Days Table)
                Control bits[3] 예약전환일 제어 - 0: 예약 전환일 사용 안함, 1:
    예약 전환일 사용 Control bits[4] Meter ID 길이 - 0: 5 Bytes(G/AE Type), 1:
    11 Bytes (AMIGO) Control bits[5:7] Reserved, 0 으로 채움 ITOU A-Date[2] :
    계기 TOU 설정 시작 일시 (BCD 일/시, 0000인 경우 즉시 전송) ITOU R-Date[2] :
    TOU 적용 예약 전환일 (BCD 월/일) ITOU Rev[6] : Reserved

        SMGW가 운영하는 TOU Info는 다음과 같다.
            항   목               A-SMGW    B-SMGW    C-SMGW
            TOU Info 최대 운용 개수    20        7         7
            Mlist의 MID 최대 개수     200       32        32
            Body 최대 크기           3KB       3KB       3KB
    */

    // 2. ITOU 11
    // 2.1 ITOU CON
    t8 = pimg[idx++];

    p_o_hd->body_type = (t8 >> 1) & 0x01;
    p_o_hd->sdt_type = (t8 >> 2) & 0x01;
    p_o_hd->adate[0] = pimg[idx++];
    p_o_hd->adate[1] = pimg[idx++];
    idx += 8;

    // DPRINTF(DBG_TRACE, "tou_t[%d], body_t[%d], sdt_t[%d], adata[%d, %d]\r\n",
    // p_o_hd->tou_type, p_o_hd->body_type, p_o_hd->sdt_type, p_o_hd->adate[0],
    // p_o_hd->adate[1]);

    // 2.2 ITOU Meterlist
    ToH16((U8_16_32*)&p_o_hd->meter_num, &pimg[idx]);
    idx += 2;

    DPRINTF(DBG_TRACE, "meter_cnt[%d]\r\n", p_o_hd->meter_num);

    for (cnt = 0; cnt < p_o_hd->meter_num; cnt++)
    {
        pt8 = &pimg[idx];
        memcpy(&p_o_hd->mid[cnt].aucMeterId[0], pt8, 11);
        idx += 11;
        DPRINT_HEX(DBG_TRACE, "METER_ID", &p_o_hd->mid[cnt], 11, DUMP_ALWAYS);
    }
    // 3. CRC
    ToH32((U8_16_32*)&p_o_hd->crc32, &pimg[idx]);
    idx += 4;

    *o_idx = idx;

    return TRUE;
}

uint32_t dsm_touBody_parserNprocess(uint8_t* pimg, ST_TOU_HEADER_INFO* p_hd)
{
    /* TOU (Time of Use) 계시별 요금제 */
    uint16_t idx = 0, o_idx = 0;
    ST_TOU_BODY_INFO tou_body;
    uint8_t item = 0;
    uint8_t delimeter[3] = {0xCA, 0xCA, 0xCA};
    uint32_t crc32_calculator;

    // 1. body length
    ToH16((U8_16_32*)&tou_body.body_len, &pimg[idx]);
    idx += 2;

    // DPRINTF(DBG_TRACE, "Body_len[%d]\r\n", tou_body.body_len);
    DPRINTF(DBG_TRACE, "BLEN[%d]\r\n", tou_body.body_len);

    // crc check
    crc32_calculator = crc32_get(&pimg[idx], tou_body.body_len);

    if (p_hd->crc32 != crc32_calculator)
    {
        // DPRINTF(DBG_ERR, _D"CRC FAIL: CRC32_cal[0x%08X], CRC32[0x%08X]\r\n",
        // crc32_calculator, p_hd->crc32);
        DPRINTF(DBG_ERR, _D "B-CRC Err: B-CRC 0x%08X, CALC 0x%08X\r\n",
                p_hd->crc32, crc32_calculator);
        return FALSE;
    }
    /*******************************************************************************/
    /*1.  C3 01 81 00 00 09 00 00 0A 00 00 FF 01 12 00 02 */
    /*******************************************************************************/

    memcpy(tou_body.bname, &pimg[idx], PROG_ID_SIZE);
    idx += PROG_ID_SIZE;

    // DPRINT_HEX(DBG_WARN, "TOU_BD_NAME", tou_body.bname, IMAGE_NAME_MAX_SIZE,
    // DUMP_ALWAYS);
    DPRINT_HEX(DBG_TRACE, "B-NAME", tou_body.bname, IMAGE_NAME_MAX_SIZE,
               DUMP_ALWAYS);

    /*******************************************************************************/
    /*2.  C1 01 81 00 14 00 00 0D 00 00 FF 02 00 09 08 32 30 31 53 52 53 39 39*/
    memset(prog_dl.pgm_name, ' ', PROG_ID_SIZE);
    memcpy(prog_dl.pgm_name, tou_body.bname, PROG_ID_SIZE);
    pdl_set_bits |= SETBITS_PGM_NAME;
    /*******************************************************************************/

    while (1)
    {
        if ((idx - 2) >= tou_body.body_len)
        {
            break;
        }

        if (!memcmp(&pimg[idx], delimeter, 3))
        {
            idx += 3;

            item = pimg[idx++];

            switch (item)
            {
            case TOU_IMG_ASP:
                /* Season Profile */
                dsm_touSP_parserNprocess(&pimg[idx], &o_idx, p_hd);
                idx += o_idx;

                break;
            case TOU_IMG_AWP:
                /* Week Profile */
                dsm_touWP_parserNprocess(&pimg[idx], &o_idx, p_hd);
                idx += o_idx;

                break;
            case TOU_IMG_ADP:
                /* Day Profile */
                dsm_touDP_parserNprocess(&pimg[idx], &o_idx, p_hd);
                idx += o_idx;

                break;
            case TOU_IMG_ASDT:
                /* Special Days Table : 정기/비정기 휴일 */
                dsm_touSDT_parserNprocess(&pimg[idx], &o_idx, p_hd);
                idx += o_idx;

                break;
            case TOU_IMG_AETC:
                dsm_touETC_parserNprocess(&pimg[idx], &o_idx, p_hd);
                idx += o_idx;

                break;
            }
            if (TOU_IMG_AETC == item)
            {
                break;
            }
        }
    }

    return TRUE;
}

void dsm_touimage_parser(uint8_t* pimg, uint16_t len)
{
    uint16_t idx = 0;
    ST_TOU_HEADER_INFO tou_header;

    dsm_touHeader_parser(pimg, len, &idx, &tou_header);
    dsm_touBody_parserNprocess(&pimg[idx], &tou_header);
}
