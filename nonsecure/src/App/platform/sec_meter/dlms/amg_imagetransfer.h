#if !defined(__AMG_IMAGE_TRANSFER_H__)
#define __AMG_IMAGE_TRANSFER_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "whm.h"
#include "defines.h"
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/
typedef enum
{
    IMG__TOU,
    IMG__FW,
    IMG__NUM
} EN_IMG_TYPE;

#define TOU_IMAGE_MAX_SIZE 2048
#define IMAGE_NAME_MAX_SIZE 8
#define IMAGE_HASH_SIZE 32

#define IMAGE_TRANSFER_ENABLED 1
#define IMAGE_TRANSFER_DISABLED 0
#define IMAGE_FW_NAME_MAX_SIZE \
    14  // 9 -> 14(형식(2), SWver(6), date(6) 변경 client 업데이트시 반영 필요.
#define FW_VERSION_SIZE 6
#define FW_GENERATION_DATE_SIZE 6

#define FW_BANK_FW_INFO_METER_TYPE_IDX 0
#define FW_BANK_FW_INFO_VER_ST_IDX 2
#define FW_BANK_FW_INFO_VER_FW_TYPE_IDX 3
#define FW_BANK_FW_INFW_VER_BANK_IDX 7
#define FW_BANK_FW_INFO_GEN_DATE 8

typedef enum
{
    IMGTR_S_TRANSFER_NOT_INITIATED,
    IMGTR_S_TRANSFER_INITIATED,
    IMGTR_S_VERIFY_INITIATED,
    IMGTR_S_VERIFY_SUCCESSFUL,
    IMGTR_S_VERIFY_FAILED,
    IMGTR_S_ACTIVATION_INITATED,
    IMGTR_S_ACTIVATION_SUCCESSFUL,
    IMGTR_S_ACTIVATION_IFAILED
} EN_IMGTR_STATUS;

#define IMG_SBIT_POS_ACTI_INIT (0)
#define IMG_SBIT_POS_NAME (1)
#define IMG_SBIT_POS_LAST_TRFR (2)
#define IMG_SBIT_POS_HASH_GEN (3)

#define IMG_SBIT_VAL_ACTI_INIT (1 << IMG_SBIT_POS_ACTI_INIT)
#define IMG_SBIT_VAL_NAME (1 << IMG_SBIT_POS_NAME)
#define IMG_SBIT_VAL_LAST_TRFR (1 << IMG_SBIT_POS_LAST_TRFR)
#define IMG_SBIT_VAL_HASH_GEN (1 << IMG_SBIT_POS_HASH_GEN)

/* FW image*/
typedef enum
{
    FW_DL_SYS_PART,
    FW_DL_INT_MDM,
    FW_DL_EXT_MDM,
    FW_DL_METER_PART,
    FW_DL_TYPE_NONE
} EN_FW_DL_TYPE;

#define FWINFO_UPDATE_T_FIRST 0x00
#define FWINFO_UPDATE_T_DOWNLOAD 0xAA
#define FWINFO_UPDATE_T_NONE 0xFF

/* TOU image*/
#define SDT_B3 0
#define SDT_B2 1
#define SDT_B1 2
#define SDT_B0 3

#define TOU_METER_ID_NUM_MAX 1
#define TOU_METER_ID_5_SIZE 5
#define TOU_METER_ID_11_SIZE 11
#define TOU_IMG_CURRENT 0
#define TOU_IMG_FUTURE 1
#define TOU_IMG_BODY_HEADER_BODY 0
#define TOU_IMG_BODY_HEADER_ONLY 1
#define TOU_IMG_SDT_D_SDT_plus_T_SDT 0
#define TOU_IMG_SDT_D_SDT_IGNORE 1

#define TOU_IMG_BLEN 0
#define TOU_IMG_BNAME 1
#define TOU_IMG_ASP 2
#define TOU_IMG_AWP 3
#define TOU_IMG_ADP 4
#define TOU_IMG_ASDT 5
#define TOU_IMG_AETC 6

/* Flash */
#define FLASH_T_INT 0
#define FLASH_T_EXT 1
#define FLASH_PAGE_MAX_SIZE 0x1000

#define REL_NAME_SIZE 14
#define VER_INDEX 6

#define LEGACY_METER_TOU_TRANSFER_ABILITY_YES 0
#define LEGACY_METER_TOU_TRANSFER_ABILITY_NO 1
#define LEGACY_METER_TOU_TRANSFER_ING LEGACY_METER_TOU_TRANSFER_ABILITY_NO

/*
******************************************************************************
*	MACRO
******************************************************************************
*/

/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/
/*system part firmware header*/
typedef struct
{
    uint32_t fw_signature;
    uint32_t fw_size;
    uint16_t fw_ver;
    uint16_t fw_crc;
    uint8_t ver_rel[REL_NAME_SIZE];
} fw_info_t;

/*TOU*/
typedef struct _st_tou_image_info_
{
    uint8_t transfer_enabled;
    uint8_t int_status_bits;  // bit0[act_init], bit1[name],
                              // bit2[last_transfer], bit3[hash_gen]
    uint32_t blk_size;
    uint32_t blk_number;
    uint8_t transfer_status;
    uint8_t name[IMAGE_NAME_MAX_SIZE];
    uint32_t image_size;
    uint32_t image_rcv_size;
    uint8_t hash[IMAGE_HASH_SIZE];
    uint16_t CRC_M;
} ST_TOU_IMG_INFO;

typedef struct _st_tou_image_data_
{
    uint8_t image[TOU_IMAGE_MAX_SIZE];
    uint16_t CRC_M;
} ST_TOU_IMG_DATA;

typedef struct _st_fw_image_download_info_
{
    uint8_t transfer_enabled;
    uint8_t int_status_bits;  // bit0[act_init], bit1[name],
                              // bit2[last_transfer], bit3[hash_gen]
    uint32_t blk_size;
    uint32_t blk_number;
    uint8_t transfer_status;
    uint8_t name[IMAGE_FW_NAME_MAX_SIZE];
    uint32_t image_size;
    uint32_t image_rcv_size;
    uint8_t hash[IMAGE_HASH_SIZE];
    uint32_t flash_addr;
    uint8_t fw_type;
    uint32_t start_dl_addr;
    uint16_t CRC_M;
} ST_FW_IMG_DOWNLOAD_INFO;

/*FW*/
typedef struct _st_fw_info_
{
    uint8_t update_t; /* FWINFO_UPDATE_T_FIRST 0x00, DOWNLOAD 0xAA, NONE 0xFF */
    date_time_type dt;   // 소프트웨어 적용 날짜
    uint8_t mt_type[2];  // 기기 형식  "53"
    uint8_t version[FW_VERSION_SIZE];
    uint8_t date_time[FW_GENERATION_DATE_SIZE];  // 소프트웨어 생성 일자
    uint8_t hash[IMAGE_HASH_SIZE];
    uint16_t CRC_M;
} ST_FW_INFO;

typedef struct _st_sub_fw_image_dl_info_
{
    uint16_t flash_page_size;
    uint16_t rcv_len;
    uint16_t CRC_M;
} ST_SUB_FW_IMG_DL_INFO;

typedef struct _st_sub_fw_image_dl_
{
#if 1 /* bccho, 2023-09-30, SoC flash에 write */
    uint8_t data[FMC_FLASH_PAGE_SIZE];
#else
    uint8_t data[FLASH_PAGE_MAX_SIZE];
#endif
    uint16_t CRC_M;
} ST_SUB_FW_IMG_DL;

/*tou only*/
typedef struct _itou_info_
{
    uint8_t tou_name[IMAGE_NAME_MAX_SIZE];
    uint8_t tou_type;
    uint8_t body_type;
    uint8_t sdt_type;
    uint8_t adate[2];  // BCD min, sec
    uint16_t meter_num;
    ST_METER_ID mid[TOU_METER_ID_NUM_MAX];
    uint32_t crc32;
} ST_TOU_HEADER_INFO;

typedef struct _tou_body_
{
    uint16_t body_len;
    uint8_t bname[IMAGE_NAME_MAX_SIZE];
    uint16_t CRC_M;
} ST_TOU_BODY_INFO;

typedef struct _ST_IMG_DAY_ID_BACKUP_
{
    uint8_t cnt;
    uint8_t id[DAY_PROF_SIZE];
} ST_IMG_DAY_ID_BACKUP;

typedef struct _st_swrst_boot_flag_
{
    uint32_t magic;
    uint16_t CRC_M;
} ST_SWRST_BOOT_FLAG;
/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/
extern ST_SUB_FW_IMG_DL gst_sub_fw_img;
extern ST_SUB_FW_IMG_DL_INFO gst_sub_fw_info;
/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/
/******************/
/* flash  */
/******************/
uint32_t fw_info_get(uint32_t fs_bank, fw_info_t* pfwinfo);
uint32_t dsm_flash_get_cur_bank(void);
;
uint32_t dsm_flash_set_bank_offset(void);
uint32_t dsm_flash_get_flash_page_size(uint8_t type);

/******************/
/* common  */
/******************/
void dsm_imgtrfr_con_init(EN_IMG_TYPE type);
char* dsm_fw_type_string(uint8_t fw_type);
ST_FW_IMG_DOWNLOAD_INFO* dsm_imgtrfr_get_fw_image_dlinfo(void);
uint8_t dsm_imgtrfr_get_transfer_enabled(EN_IMG_TYPE type);
void dsm_imgtrfr_set_transfer_enabled(EN_IMG_TYPE type, uint8_t val);
uint8_t dsm_imgtrfr_get_int_status_bit(EN_IMG_TYPE type, uint8_t bit_pos);
void dsm_imgtrfr_set_int_status_bits(EN_IMG_TYPE type, uint8_t val);
void dsm_imgtrfr_reset_int_status_bits(EN_IMG_TYPE type, uint8_t val);
uint16_t dsm_convert_dlrxtxmax_to_imgtrfr_blk_size(void);
uint32_t dsm_imgtrfr_get_blk_size(EN_IMG_TYPE type);
void dsm_imgtrfr_set_blk_size(EN_IMG_TYPE type, uint32_t blk_size);
uint32_t dsm_imgtrfr_get_blk_num(EN_IMG_TYPE type);
void dsm_imgtrfr_set_blk_num(EN_IMG_TYPE type, uint32_t blk_number);
void dsm_imgtrfr_touimage_buff_update(uint8_t* pblk, uint32_t pre_rcv_size,
                                      uint16_t blk_size);
uint8_t* dsm_imgtrfr_touimage_get_buff(void);
uint32_t dsm_imgtrfr_get_image_size(EN_IMG_TYPE type);
void dsm_imgtrfr_set_image_size(EN_IMG_TYPE type, uint32_t image_size);
uint32_t dsm_imgtrfr_get_rcvimage_size(EN_IMG_TYPE type);
void dsm_imgtrfr_set_rcvimage_size(EN_IMG_TYPE type, uint32_t image_size);
uint8_t dsm_imgtrfr_get_transfer_status(EN_IMG_TYPE type);
void dsm_imgtrfr_set_transfer_status(EN_IMG_TYPE type, uint8_t transfer_status);
uint8_t* dsm_imgtrfr_get_name(EN_IMG_TYPE type);
void dsm_imgtrfr_set_name(EN_IMG_TYPE type, uint8_t len, uint8_t* name);
uint8_t* dsm_imgtrfr_get_hash(EN_IMG_TYPE type);
void dsm_imgtrfr_set_hash(EN_IMG_TYPE type, uint8_t len, uint8_t* hash);
uint32_t dsm_imgtrfr_activNverify_is_ready(EN_IMG_TYPE type);

/******************/
/* FW image only  */
/******************/
bool dsm_imgtrfr_fwinfo_read(uint8_t* pblk, uint32_t pos);
bool dsm_imgtrfr_fwinfo_write(uint8_t* pblk, uint32_t pos);
bool dsm_sys_fwinfo_initial_set(bool product);
bool dsm_sys_fwinfo_is_initialNset(void);
void dsm_imgtrfr_fwimage_dl_info_save(void);
uint8_t dsm_imgtrfr_fwimage_dl_get_fw_type(void);
bool dsm_imgtrfr_fwimage_act_init_process(uint8_t fw_type);
bool dsm_imgtrfr_fwimage_update(uint8_t fw_type, bool force, uint8_t* pblk,
                                uint16_t blk_size);
bool dsm_imgtrfr_fwimage_hash(uint8_t fw_type, uint8_t* pblk,
                              uint16_t blk_size);
void dsm_imgtrfr_fwimage_act_run_process(uint8_t fw_type);
void dsm_imgtrfr_fwimage_set_sys_flash_addr(uint32_t val);
uint32_t dsm_imgtrfr_fwimage_get_sys_flash_addr(void);
bool dsm_imgtrfr_fw_subimage_buff_update(uint8_t* pblk, uint16_t blk_size);
bool dsm_imgtrfr_fw_subimage_buff_update_force(void);

uint8_t* dsm_imgtrfr_fw_subimage_get_buff(void);
void dsm_imgtrfr_fw_subimage_write(uint8_t* pblk, uint32_t pos,
                                   uint32_t w_size);
bool dsm_imgtrfr_fw_subinfo_read(uint8_t* pinfo);
bool dsm_imgtrfr_fw_subinfo_write(void);
bool dsm_imgtrfr_fw_subimage_restore(void);

uint8_t dsm_imgtrfr_get_fw_type(void);
void dsm_imgtrfr_set_fw_type(uint8_t fw_type);
uint32_t dsm_imgtrfr_get_start_dl_addr(void);
void dsm_imgtrfr_set_start_dl_addr(uint32_t start_dl_addr);
uint32_t dsm_sflash_img_read(uint32_t addr, uint8_t* data, uint16_t len);

void dsm_imgtrfr_fwup_set_fsm(uint8_t state);
uint8_t dsm_imgtrfr_fwup_get_fsm(void);

void dsm_swrst_init_bootflag(void);
bool dsm_swrst_is_bootflag(void);

/******************/
/* TOU image only */
/******************/
void dsm_imgtrfr_touimage_write(uint8_t* pblk, uint32_t pos, uint32_t w_size);
bool dsm_imgtrfr_touimage_read(uint8_t* pblk, uint32_t pos, uint32_t r_size);
void dsm_imgtrfr_touimage_restore(void);
void dsm_imgtrfr_touimage_info_save(void);
uint32_t dsm_touHeader_parser(uint8_t* pimg, uint16_t len, uint16_t* o_idx,
                              ST_TOU_HEADER_INFO* p_o_hd);
uint32_t dsm_touBody_parserNprocess(uint8_t* pimg, ST_TOU_HEADER_INFO* p_hd);
void dsm_touimage_parser(uint8_t* pimg, uint16_t len);

/*********************************/
/* TOU transfer for Legacy meter */
/*********************************/

bool dsm_is_day_id_backup(ST_IMG_DAY_ID_BACKUP* back, uint8_t day_id);
void dsm_set_day_id_backup(ST_IMG_DAY_ID_BACKUP* back, uint8_t day_id);

#endif /* __AMG_IMAGE_TRANSFER_H__ */
