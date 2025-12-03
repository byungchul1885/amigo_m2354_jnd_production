
#include "options.h"
#include "port.h"
#include "delay.h"
#include "nv.h"
#include "eeprom_at24cm02.h"
#include "whm.h"

#define _D "[NV] "

pwf_date_type pwf_date_time;
nv_sub_info_type nv_sub_info;
static uint8_t prog_dl_idx;
static uint8_t hol_dl_idx;

static uint8_t idx_cap_wear = 0;

ram_type RAM_data;
nv_sub_info_type ram_sub_info;

#define NVACS_DELAY_MS 100
#define NVRETRY_DELAY_MS 500

static void delay_for_nvreacs(void);

extern uint32_t dsm_get_dm_out_measure_print_chkcount(void);

void inc_idx_cap_wear(void)
{
    idx_cap_wear += 1;
    if (idx_cap_wear >= NUM_CAP_WEAR)
        idx_cap_wear = 0;
}

void inc_rcntdm_wear_idx(void)
{
    rcntdm_wear_idx += 1;
    if (rcntdm_wear_idx >= NUM_CAP_WEAR)
        rcntdm_wear_idx = 0;
}

char *dsm_nv_item_string(nv_item_type nv_item)
{
    switch (nv_item)
    {
    case I_VERSION:
        return "VERSION";
    case I_VERSION1:
        return "VERSION1";
    case I_ALL_CLEAR:
        return "ALL_CLEAR";
    case I_MTINIT_LOG:
        return "MTINIT_LOG";
    case I_SAG_FLAG:
        return "SAG_FLAG";
    case I_MT_CONFIG:
        return "MT_CONFIG";
    case I_WHM_OP:
        return "WHM_OP";
    case I_MT_ACCM:
        return "MT_ACCM";
    case I_MT_READ_DATA:
        return "MT_READ_DATA";
    case I_MAX_DEMAND:
        return "MAX_DEMAND";
    case I_LOG_DATA:
        return "LOG_DATA";
    case I_LOG_DATA1:
        return "LOG_DATA1";
    case I_LOG_SCURR:
        return "LOG_SCURR";
    case I_PROG_INFO:
        return "PROG_INFO";
    case I_LP_DATA:
        return "LP_DATA";
    case I_LPAVG_DATA:
        return "LPAVG_DATA";
    case I_SERIAL_NO:
        return "SERIAL_NO";
    case I_DEVICE_ID:
        return "DEVICE_ID";
    case I_CAL_NAME_ACTIVE:
        return "CAL_NAME_ACTIVE";
    case I_SEASON_PROFILE_ACTIVE:
        return "SEASON_PROFILE_ACTIVE";
    case I_WEEK_PROFILE_ACTIVE:
        return "WEEK_PROFILE_ACTIVE";
    case I_DAY_PROFILE_ACTIVE:
        return "DAY_PROFILE_ACTIVE";
    case I_CAL_NAME_PASSIVE:
        return "CAL_NAME_PASSIVE";
    case I_SEASON_PROFILE_PASSIVE:
        return "SEASON_PROFILE_PASSIVE";
    case I_WEEK_PROFILE_PASSIVE:
        return "WEEK_PROFILE_PASSIVE";
    case I_DAY_PROFILE_PASSIVE:
        return "DAY_PROFILE_PASSIVE";
    case I_ACT_PAS_TIME:
        return "ACT_PAS_TIME";
    case I_CUST_NO:
        return "CUST_NO";
    case I_NP_BILLDATE:
        return "NP_BILLDATE";
    case I_HOLIDAYS_A:
        return "HOLIDAYS_A";
    case I_HOLIDAYS_P:
        return "HOLIDAYS_P";
    case I_TMP_HOLIDAYS_A:
        return "TMP_HOLIDAYS_A";
    case I_TMP_HOLIDAYS_P:
        return "TMP_HOLIDAYS_P";
    case I_NP_BILLDATE_A:
        return "NP_BILLDATE_A";
    case I_NP_BILLDATE_P:
        return "NP_BILLDATE_P";
    case I_DISP_SUPP_A:
        return "DISP_SUPP_A";
    case I_DISP_SUPP_P:
        return "DISP_SUPP_P";
    case I_SUPPDSP_ITEM_A:
        return "SUPPDSP_ITEM_A";
    case I_SUPPDSP_ITEM_P:
        return "SUPPDSP_ITEM_P";
    case I_CUR_PROG_DL:
        return "CUR_PROG_DL";
    case I_FUT_PROG_DL:
        return "FUT_PROG_DL";
    case I_TX_SEG_FRAME:
        return "TX_SEG_FRAME";
    case I_WHM_OP_CLR:
        return "WHM_OP_CLR";
    case I_SEASON_PROFILE_DL:
        return "SEASON_PROFILE_DL";
    case I_WEEK_PROFILE_DL:
        return "WEEK_PROFILE_DL";
    case I_DAY_PROFILE_DL:
        return "DAY_PROFILE_DL";
    case I_HOLIDAYS_DL:
        return "HOLIDAYS_DL";
    case I_NP_BILLDATE_DL:
        return "NP_BILLDATE_DL";
    case I_DISP_SUPP_DL:
        return "DISP_SUPP_DL";
    case I_SUPPDSP_ITEM_DL:
        return "SUPPDSP_ITEM_DL";
    case I_TMP_HOLIDAYS_DL:
        return "TMP_HOLIDAYS_DL";
    case I_WHM_LOG_LEN:
        return "WHM_LOG_LEN";
    case I_WHM_LOG:
        return "WHM_LOG";
    case I_ABS_ACCESS:
        return "ABS_ACCESS";
    case I_BM_INP_QUE:
        return "BM_INP_QUE";
    case I_RTC_COMP:
        return "RTC_COMP";
    case I_BAT_USE:
        return "BAT_USE";
    case I_DR_LIMIT:
        return "DR_LIMIT";
    case I_MAG_CAL_DATA:
        return "MAG_CAL_DATA";
    case I_TEMP_CAL_DATA:
        return "TEMP_CAL_DATA";
    case I_CAL_DATA:
        return "CAL_DATA";
    case I_PFINF_PACCM:
        return "PFINF_PACCM";
    case I_RCNT_DEMAND:
        return "RCNT_DEMAND";
    case I_CUR_PF_DATA:
        return "CUR_PF_DATA";
    case I_CUR_PF_DATA_CLEAR:
        return "CUR_PF_DATA_CLEAR";
    case I_RCNT_DM_CAP:
        return "RCNT_DM_CAP";
    case I_CUR_DM_CAP:
        return "CUR_DM_CAP";
    case I_CUR_DATA_CAP:
        return "CUR_DATA_CAP";
    case I_MT_RATE_CAP:
        return "MT_RATE_CAP";
    case I_LOG_CNT_CAP:
        return "LOG_CNT_CAP";
    case I_UTIL_PASSWORD:
        return "UTIL_PASSWORD";
    case I_485_PASSWORD:
        return "485_PASSWORD";
    case I_BAT_INST:
        return "BAT_INST";
    case I_IMAX_LOG:
        return "IMAX_LOG";
    case I_TOVER_LOG:
        return "TOVER_LOG";
    case I_PREPAY:
        return "PREPAY";
    case I_SAG_SWELL:
        return "SAG_SWELL";
    case I_DM_INTV_DATA:
        return "DM_INTV_DATA";
    case I_DM_SUBLOCKS_DATA:
        return "DM_SUBLOCKS_DATA";
    case I_JIG_PASSWORD:
        return "JIG_PASSWORD";
    case I_PWF_DATE_TIME:
        return "PWF_DATE_TIME";
    case I_WORKPWR_FLAG:
        return "WORKPWR_FLAG";
    case I_UNINIT_DATA:
        return "UNINIT_DATA";
    case I_MIN_FREQ:
        return "MIN_FREQ";
    case I_MAX_FREQ:
        return "MAX_FREQ";
#if 1 /* bccho, 2024-09-05, 삼상 */
    case I_MIN_VOLT_L1:
        return "MIN_VOLT_L1";
    case I_MAX_VOLT_L1:
        return "MAX_VOLT_L1";
    case I_MIN_VOLT_L2:
        return "MIN_VOLT_L2";
    case I_MAX_VOLT_L2:
        return "MAX_VOLT_L2";
    case I_MIN_VOLT_L3:
        return "MIN_VOLT_L3";
    case I_MAX_VOLT_L3:
        return "MAX_VOLT_L3";
#else
    case I_MIN_VOLT:
        return "MIN_VOLT";
    case I_MAX_VOLT:
        return "MAX_VOLT";
#endif
    case I_LP_DATA_BLKED:
        return "LP_DATA_BLKED";
    case I_DISP_SUPP_RCNT:
        return "DISP_SUPP_RCNT";
    case I_SUPPDSP_ITEM_RCNT:
        return "SUPPDSP_ITEM_RCNT";
    case I_ADJ_TEMP_DATA:
        return "ADJ_TEMP_DATA";
    case I_MANUFACT_ID:
        return "MANUFACT_ID";
    case I_TOU_SET_CNT:
        return "TOU_SET_CNT";
    case I_BACKUP_PROG_DL:
        return "BACKUP_PROG_DL";
    case I_HOL_DATE_BLOCK:
        return "HOL_DATE_BLOCK";
    case I_DEVICE_ID_KEPCO:
        return "DEVICE_ID_KEPCO";
    case I_LOG_CERT_DATA:
        return "LOG_CERT_DATA";
    case I_TOU_IMG_INFO:
        return "TOU_IMG_INFO";
    case I_TOU_IMG_DATA:
        return "TOU_IMG_DATA";
    case I_FW_IMG_DOWNLOAD_INFO:
        return "FW_IMG_DOWNLOAD_INFO";
    case I_FW_IMG_INFO:
        return "FW_IMG_INFO";
    case I_SUB_FW_IMG_DOWNLOAD_INFO:
        return "SUB_FW_IMG_DOWNLOAD_INFO";
    case I_SUB_FW_IMG_DATA:
        return "SUB_FW_IMG_DATA";
    case I_MT_READ_DATA_nPRD:
        return "MT_READ_DATA_nPRD";
    case I_MT_READ_DATA_SEASON:
        return "MT_READ_DATA_SEASON";
    case I_LPRT_DATA:
        return "LPRT_DATA";
    case I_RANDOM_SEED:
        return "RANDOM_SEED";
    case I_MTP_CAL_POINT:
        return "MTP_CAL_POINT";
    case I_MTP_PARM:
        return "MTP_PARM";
    case I_MTP_SAG_SWELL:
        return "MTP_SAG_SWELL";
    case I_PUSH_ACTI_ERR:
        return "PUSH_ACTI_ERR";
    case I_PUSH_SCRIPT_TABLE:
        return "PUSH_SCRIPT_TABLE";
    case I_PUSH_SETUP_TABLE:
        return "PUSH_SETUP_TABLE";
    case I_MT_RST_TIME:
        return "MT_RST_TIME";
    case I_EXT_MODEM_ID:
        return "EXT_MODEM_ID";
    case I_STOCK_OP_TIMES:
        return "STOCK_OP_TIMES";
    case I_EXTN_CLEAR:
        return "EXTN CLEAR";
    case I_MT_CONFIG_2:
        return "MT_CONFIG_2";
    case I_ZCD_RESULT_TIME:
        return "ZCD_RESULT_TIME";
    case I_MODEM_BAUD:
        return "MODEM_BAUD";
    case I_BOOT_AFTER_SWRST:
        return "BOOT_AFTER_SWRST";
    case I_CERTI_HASH: /* bccho, 2024-09-04 */
        return "I_CERTI_HASH";
    default:
        return "Unknown item";
    }
}

bool dsm_ram_write(uint32_t ram_addr, uint8_t *pwdata, uint32_t length)
{
    uint8_t *ptr = (uint8_t *)RAM_data.lprt_record;
    uint32_t ch_count = dsm_get_dm_out_measure_print_chkcount();

    if (ch_count % 20 == 5 && (rt_lp_interval < 5))
    {
        DPRINTF(DBG_NONE, "%s: addr[%d], len[%d]\r\n", __func__, ram_addr,
                length);
    }
    memcpy(&ptr[ram_addr], pwdata, length);

    DPRINT_HEX(DBG_NONE, "RAM_W", &ptr[ram_addr], length, DUMP_AMI);

    return true;
}

bool dsm_ram_read(uint32_t ram_addr, uint8_t *prdata, uint32_t length)
{
    uint8_t *ptr = (uint8_t *)RAM_data.lprt_record;
    {
        DPRINTF(DBG_NONE, "%s: addr[%d], len[%d]\r\n", __func__, ram_addr,
                length);
    }
    memcpy(prdata, &ptr[ram_addr], length);

    DPRINT_HEX(DBG_NONE, "RAM_R", &ptr[ram_addr], length, DUMP_AMI);

    return true;
}

bool ram_proc(nv_item_type ram_item, uint8_t *ram_data, bool wr)
{
    ram_type *ram_type_ptr;
    uint32_t ram_pos = 0;
    uint16_t ram_size = 0;

    uint32_t ch_count = dsm_get_dm_out_measure_print_chkcount();

    if ((wr == 0) ||
        ((wr == 1) && (ch_count % 20 == 5 && (rt_lp_interval < 5))))
    {
        DPRINTF(DBG_NONE, "%s: %s [%s]\r\n", __func__, (wr ? "write" : "read"),
                dsm_nv_item_string(ram_item));
    }

    ram_type_ptr = (ram_type *)0;

    switch (ram_item)
    {
    case I_LPRT_DATA:

        ram_pos = (uint32_t)&(ram_type_ptr->lprt_record[0]);

        if (mt_is_onephase())
        {
            ram_pos += (uint32_t)ram_sub_info.rel.idx *
                       (uint32_t)sizeof(lprt_record_1phs);
            ram_size =
                (uint16_t)ram_sub_info.rel.len * sizeof(lprt_record_1phs);
        }
        else
        {
            ram_pos += (uint32_t)ram_sub_info.rel.idx *
                       (uint32_t)sizeof(lprt_record_3phs);
            ram_size =
                (uint16_t)ram_sub_info.rel.len * sizeof(lprt_record_3phs);
        }
        break;

    default:

        break;
    }

    if (wr)
    {
        dsm_ram_write(ram_pos, ram_data, ram_size);
    }
    else
    {
        dsm_ram_read(ram_pos, ram_data, ram_size);
    }

    return true;
}

bool nv_proc(nv_item_type nv_item, uint8_t *nv_data, bool wr)
{
    error_nv_type nv_err = NV_OK;
#if 0 /* bccho, HAL_OK, 2023-07-15 */    
    HAL_StatusTypeDef pstatus = HAL_ERROR;
#else /* bccho */
    uint32_t pstatus = HAL_ERROR;
#endif
    Nv_type *nv_type_ptr = (Nv_type *)0;
    uint32_t nv_pos = 0;
    uint16_t nv_size = 0;
    bool b_CRC_chk = true;
    int t1, t2, t3;

    DPRINTF(DBG_INFO, _D "%s: %s [%s]\r\n", __func__, (wr ? "write" : "read"),
            dsm_nv_item_string(nv_item));

    //	nv_type_ptr = (Nv_type *) 0;
    //	b_CRC_chk = true;

    switch (nv_item)
    {
    case I_CAL_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->cal_data);
        nv_size = sizeof(cal_data_type);
        break;

    case I_VERSION:
        nv_pos = (uint32_t)&(nv_type_ptr->nver);
        nv_size = sizeof(nv_version_type);
        break;

    case I_VERSION1:
        break;

    case I_ABS_ACCESS:
        nv_pos = nv_sub_info.abs.addr;
        nv_size = nv_sub_info.abs.len;

        b_CRC_chk = FALSE;
        break;

    case I_SERIAL_NO:
        nv_pos = (uint32_t)&(nv_type_ptr->serno);
        nv_size = sizeof(ser_no_type);
        break;

    case I_DEVICE_ID:
        nv_pos = (uint32_t)&(nv_type_ptr->devid);
        nv_size = sizeof(device_id_type);
        break;

    case I_DEVICE_ID_KEPCO:
        nv_pos = (uint32_t)&(nv_type_ptr->devid_kepco);
        nv_size = sizeof(device_id_type);
        break;

    case I_MANUFACT_ID:
        nv_pos = (uint32_t)&(nv_type_ptr->manfid);
        nv_size = sizeof(manufact_id_type);
        break;

    case I_MAG_CAL_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->mag_cal);
        nv_size = sizeof(mag_cal_data_type);
        break;

    case I_TEMP_CAL_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->temp_cal);
        nv_size = sizeof(temp_cal_data_type);
        break;

    case I_MTINIT_LOG:
        nv_pos = (uint32_t)&(nv_type_ptr->mtinit_log);
        nv_size = sizeof(mtinit_log_data_type);
        break;

    case I_UNINIT_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->uninit_data);
        nv_size = sizeof(latchon_data_type);
        break;

    case I_ALL_CLEAR:
        nv_pos = (uint32_t)&(nv_type_ptr->init_start);
        nv_size = (uint16_t)(((uint32_t)&nv_type_ptr->init_stop) - nv_pos);
        DPRINTF(DBG_TRACE, _D "%s I_ALL_CLEAR[%d]\r\n", __func__, nv_size);
        break;

    case I_EXTN_CLEAR:
        nv_pos = (uint32_t)&(nv_type_ptr->init_extn_start);
        nv_size = (uint16_t)(((uint32_t)&nv_type_ptr->init_extn_stop) - nv_pos);
        DPRINTF(DBG_TRACE, _D "%s I_EXTN_CLEAR[%d]\r\n", __func__, nv_size);
        break;

    case I_SAG_FLAG:
        nv_pos = (uint32_t)&(nv_type_ptr->sag_flag);
        nv_size = SAG_FLAG_LENGTH;

        b_CRC_chk = FALSE;
        break;

    case I_BAT_USE:
        nv_pos = (uint32_t)&(nv_type_ptr->bat_use);
        nv_size = sizeof(bat_used_time_type);
        break;

    case I_BAT_INST:
        nv_pos = (uint32_t)&(nv_type_ptr->bat_inst);
        nv_size = sizeof(bat_install_type);
        break;

    case I_TX_SEG_FRAME:
        nv_pos = nv_sub_info.seg.offset;
        nv_size = nv_sub_info.seg.len;
        nv_pos += (uint32_t)&(nv_type_ptr->tx_seg[idx_cap_wear]);

        b_CRC_chk = FALSE;
        break;

    case I_MT_CONFIG:
        nv_pos = (uint32_t)&(nv_type_ptr->mtconf);
        nv_size = sizeof(mt_conf_type);
        break;

    case I_MT_CONFIG_2:
        nv_pos = (uint32_t)&(nv_type_ptr->mtconf_2);
        nv_size = sizeof(mt_conf_2_type);
        break;

    case I_WHM_OP:
        nv_pos = (uint32_t)&(nv_type_ptr->whmop);
        nv_size = sizeof(whm_op_type);
        break;

    case I_MT_ACCM:
        nv_pos = (uint32_t)&(nv_type_ptr->mtacc);
        nv_size = sizeof(mt_acc_type);
        break;

    case I_MT_READ_DATA:
        if (nv_sub_info.mr.sel == eMrAccm)
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr->mrinfo[nv_sub_info.mr.mrcnt % NUM_MREADING_SAVE]
                    .accm[nv_sub_info.mr.rt]);
            nv_size = sizeof(mr_ch_type);
        }
        else if (nv_sub_info.mr.sel == eMrDm)
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr->mrinfo[nv_sub_info.mr.mrcnt % NUM_MREADING_SAVE]
                    .dm[nv_sub_info.mr.rt]);
            nv_size = sizeof(mr_dm_type);
        }
        else if (nv_sub_info.mr.sel == eMrInfo)
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr->mrinfo[nv_sub_info.mr.mrcnt % NUM_MREADING_SAVE]
                    .info);
            nv_size = sizeof(mr_data_info_type);
        }
#if 0 /* bccho, 2024-09-24, 삼상, delete  */
#if 1 /* bccho, 2024-09-05, 삼상 */
        else if (nv_sub_info.mr.sel == eMrAccmEcdsa)
        {
            nv_pos =
                (uint32_t) &
                (nv_type_ptr->mrinfo[nv_sub_info.mr.mrcnt % NUM_MREADING_SAVE]
                     .acc_ecdsa[nv_sub_info.mr.dir]);
            nv_size = sizeof(mr_ecdsa_type);
        }
        else if (nv_sub_info.mr.sel == eMrDmEcdsa)
        {
            nv_pos =
                (uint32_t) &
                (nv_type_ptr->mrinfo[nv_sub_info.mr.mrcnt % NUM_MREADING_SAVE]
                     .dm_ecdsa[nv_sub_info.mr.dir]);
            nv_size = sizeof(mr_ecdsa_type);
        }
#endif
#endif
        else
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr->mr_rolldm[nv_sub_info.mr.mrcnt % NUM_MREADING_SAVE]
                    .roll[nv_sub_info.mr.rt]
                    .ch[nv_sub_info.mr.chsel]);
            nv_size = sizeof(rolling_dm_ch_type);
        }
        break;

    case I_MT_READ_DATA_nPRD:
        if (nv_sub_info.mr.sel == eMrAccm)
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr
                    ->mrinfo_nprd[nv_sub_info.mr.mrcnt % NUM_MREADING_nPRD_SAVE]
                    .accm[nv_sub_info.mr.rt]);
            nv_size = sizeof(mr_ch_type);
        }
        else if (nv_sub_info.mr.sel == eMrDm)
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr
                    ->mrinfo_nprd[nv_sub_info.mr.mrcnt % NUM_MREADING_nPRD_SAVE]
                    .dm[nv_sub_info.mr.rt]);
            nv_size = sizeof(mr_dm_type);
        }
        else if (nv_sub_info.mr.sel == eMrInfo)
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr
                    ->mrinfo_nprd[nv_sub_info.mr.mrcnt % NUM_MREADING_nPRD_SAVE]
                    .info);
            nv_size = sizeof(mr_data_info_type);
        }
#if 0 /* bccho, 2024-09-24, 삼상, delete  */
#if 1 /* bccho, 2024-09-05, 삼상 */
        else if (nv_sub_info.mr.sel == eMrAccmEcdsa)
        {
            nv_pos = (uint32_t) & (nv_type_ptr
                                       ->mrinfo_nprd[nv_sub_info.mr.mrcnt %
                                                     NUM_MREADING_nPRD_SAVE]
                                       .acc_ecdsa[nv_sub_info.mr.dir]);
            nv_size = sizeof(mr_ecdsa_type);
        }
        else if (nv_sub_info.mr.sel == eMrDmEcdsa)
        {
            nv_pos = (uint32_t) & (nv_type_ptr
                                       ->mrinfo_nprd[nv_sub_info.mr.mrcnt %
                                                     NUM_MREADING_nPRD_SAVE]
                                       .dm_ecdsa[nv_sub_info.mr.dir]);
            nv_size = sizeof(mr_ecdsa_type);
        }
#endif
#endif
        else
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr
                    ->mr_rolldm[nv_sub_info.mr.mrcnt % NUM_MREADING_nPRD_SAVE]
                    .roll[nv_sub_info.mr.rt]
                    .ch[nv_sub_info.mr.chsel]);
            nv_size = sizeof(rolling_dm_ch_type);
        }
        break;

    case I_MT_READ_DATA_SEASON:
        if (nv_sub_info.mr.sel == eMrAccm)
        {
            nv_pos = (uint32_t)&(nv_type_ptr
                                     ->mrinfo_season[nv_sub_info.mr.mrcnt %
                                                     NUM_MREADING_SEASON_SAVE]
                                     .accm[nv_sub_info.mr.rt]);
            nv_size = sizeof(mr_ch_type);
        }
        else if (nv_sub_info.mr.sel == eMrDm)
        {
            nv_pos = (uint32_t)&(nv_type_ptr
                                     ->mrinfo_season[nv_sub_info.mr.mrcnt %
                                                     NUM_MREADING_SEASON_SAVE]
                                     .dm[nv_sub_info.mr.rt]);
            nv_size = sizeof(mr_dm_type);
        }
        else if (nv_sub_info.mr.sel == eMrInfo)
        {
            nv_pos = (uint32_t)&(nv_type_ptr
                                     ->mrinfo_season[nv_sub_info.mr.mrcnt %
                                                     NUM_MREADING_SEASON_SAVE]
                                     .info);
            nv_size = sizeof(mr_data_info_type);
        }
#if 0 /* bccho, 2024-09-24, 삼상, delete  */
#if 1 /* bccho, 2024-09-05, 삼상 */
        else if (nv_sub_info.mr.sel == eMrAccmEcdsa)
        {
            nv_pos = (U32) & (nv_type_ptr
                                  ->mrinfo_season[nv_sub_info.mr.mrcnt %
                                                  NUM_MREADING_SEASON_SAVE]
                                  .acc_ecdsa[nv_sub_info.mr.dir]);
            nv_size = sizeof(mr_ecdsa_type);
        }
        else if (nv_sub_info.mr.sel == eMrDmEcdsa)
        {
            nv_pos = (U32) & (nv_type_ptr
                                  ->mrinfo_season[nv_sub_info.mr.mrcnt %
                                                  NUM_MREADING_SEASON_SAVE]
                                  .dm_ecdsa[nv_sub_info.mr.dir]);
            nv_size = sizeof(mr_ecdsa_type);
        }
#endif
#endif
        else
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr
                    ->mr_rolldm[nv_sub_info.mr.mrcnt % NUM_MREADING_SEASON_SAVE]
                    .roll[nv_sub_info.mr.rt]
                    .ch[nv_sub_info.mr.chsel]);
            nv_size = sizeof(rolling_dm_ch_type);
        }
        break;

    case I_MAX_DEMAND:
        nv_pos = (uint32_t)&(nv_type_ptr->mxdm);
        nv_size = sizeof(max_demand_type);
        break;

    case I_RCNT_DEMAND:
        if (rcntdm_wear_idx > NUM_CAP_WEAR)
        {
            DPRINTF(DBG_NONE, "not yet recent demand\r\n");
            nv_err = NV_RANGE_ERROR;
        }
        else
        {
            nv_pos = (uint32_t)&(nv_type_ptr->rcntdm[rcntdm_wear_idx]);
            nv_size = sizeof(recent_demand_type);
        }
        break;

    case I_LP_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->lp_recstart);
        nv_pos +=
            (uint32_t)nv_sub_info.rel.idx * (uint32_t)nv_sub_info.rel.rec_siz;
        nv_size = (uint16_t)nv_sub_info.rel.len * nv_sub_info.rel.rec_siz;

        b_CRC_chk = FALSE;
        break;

    case I_LPAVG_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->lpavg_record[0]);
        nv_pos +=
            (uint32_t)nv_sub_info.rel.idx * (uint32_t)sizeof(lpavg_record_type);
        nv_size = (uint16_t)nv_sub_info.rel.len * sizeof(lpavg_record_type);

        b_CRC_chk = FALSE;
        break;

    case I_LPRT_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->lprt_record[0]);

        if (mt_is_onephase())
        {
            nv_pos += (uint32_t)nv_sub_info.rel.idx *
                      (uint32_t)sizeof(lprt_record_1phs);
            nv_size = (uint16_t)nv_sub_info.rel.len * sizeof(lprt_record_1phs);
        }
        else
        {
            nv_pos += (uint32_t)nv_sub_info.rel.idx *
                      (uint32_t)sizeof(lprt_record_3phs);
            nv_size = (uint16_t)nv_sub_info.rel.len * sizeof(lprt_record_3phs);
        }
        b_CRC_chk = FALSE;
        break;

    case I_LP_DATA_BLKED:
        nv_pos = (uint32_t)(nv_type_ptr->lprec_blked);
        nv_pos +=
            (uint32_t)nv_sub_info.rel.idx * (uint32_t)nv_sub_info.rel.rec_siz;
        nv_size = nv_sub_info.rel.rec_siz;

        b_CRC_chk = FALSE;
        break;

    case I_NP_BILLDATE_A:
        nv_pos = (uint32_t)&(nv_type_ptr->pgm[aprog_area].np_billdate);
        nv_size = sizeof(npbill_date_type);
        break;

    case I_NP_BILLDATE_P:
        nv_pos = (uint32_t)&(nv_type_ptr->pgm[pprog_area].np_billdate);
        nv_size = sizeof(npbill_date_type);
        break;

    case I_NP_BILLDATE_DL:
        if (prog_dl_idx < NUM_PGM_AREA)
        {
            nv_pos = (uint32_t)&(nv_type_ptr->pgm[prog_dl_idx].np_billdate);
            nv_size = sizeof(npbill_date_type);
        }
        else
        {
            nv_err = NV_RANGE_ERROR;
        }
        break;

    case I_SEASON_PROFILE_ACTIVE:
        nv_pos = (uint32_t)&(nv_type_ptr->pgm[aprog_area].season_profile);
        nv_size = sizeof(season_date_type);
        break;

    case I_SEASON_PROFILE_PASSIVE:
        nv_pos = (uint32_t)&(nv_type_ptr->pgm[pprog_area].season_profile);
        nv_size = sizeof(season_date_type);
        break;

    case I_SEASON_PROFILE_DL:
        if (prog_dl_idx < NUM_PGM_AREA)
        {
            nv_pos = (uint32_t)&(nv_type_ptr->pgm[prog_dl_idx].season_profile);
            nv_size = sizeof(season_date_type);
        }
        else
        {
            nv_err = NV_RANGE_ERROR;
        }
        break;

    case I_WEEK_PROFILE_ACTIVE:
        nv_pos = (uint32_t)&(nv_type_ptr->pgm[aprog_area].week_profile);
        nv_size = sizeof(week_date_type);
        break;

    case I_WEEK_PROFILE_PASSIVE:
        nv_pos = (uint32_t)&(nv_type_ptr->pgm[pprog_area].week_profile);
        nv_size = sizeof(week_date_type);
        break;

    case I_WEEK_PROFILE_DL:
        if (prog_dl_idx < NUM_PGM_AREA)
        {
            nv_pos = (uint32_t)&(nv_type_ptr->pgm[prog_dl_idx].week_profile);
            nv_size = sizeof(week_date_type);
        }
        else
        {
            nv_err = NV_RANGE_ERROR;
        }
        break;

    case I_DAY_PROFILE_ACTIVE:
        nv_pos = (uint32_t)&(
            nv_type_ptr->pgm[aprog_area].day_profile[nv_sub_info.ch[0]]);
        nv_size = sizeof(dayid_table_type);
        break;

    case I_DAY_PROFILE_PASSIVE:
        nv_pos = (uint32_t)&(
            nv_type_ptr->pgm[pprog_area].day_profile[nv_sub_info.ch[0]]);
        nv_size = sizeof(dayid_table_type);
        break;

    case I_DAY_PROFILE_DL:
        if (prog_dl_idx < NUM_PGM_AREA)
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr->pgm[prog_dl_idx].day_profile[nv_sub_info.ch[0]]);
            nv_size = sizeof(dayid_table_type);
        }
        else
        {
            nv_err = NV_RANGE_ERROR;
        }
        break;

    case I_HOLIDAYS_DL:
    case I_HOLIDAYS_A:
    case I_HOLIDAYS_P:
        if (nv_item == I_HOLIDAYS_A)
        {
            t1 = ahol_area;
        }
        else if (nv_item == I_HOLIDAYS_P)
        {
            t1 = phol_area;
        }
        else
        {
            if (hol_dl_idx < NUM_PGM_AREA)
            {
                t1 = hol_dl_idx;
            }
            else
            {
                nv_err = NV_RANGE_ERROR;
                break;
            }
        }

        if (nv_item == I_HOLIDAYS_A && nv_sub_info.ch[1] == HEAD_OF_BLOCK)
        {
            DPRINTF(DBG_WARN,
                    "%s: I_HOLIDAYS_A  case 1 t1[%d], nv_sub_info.ch[0]=%d\r\n",
                    __func__, t1, nv_sub_info.ch[0]);
            nv_pos =
                (uint32_t)&(nv_type_ptr->pgm[t1].holidays[nv_sub_info.ch[0]]);
            nv_size = 3;  // array size and year

            DPRINTF(DBG_WARN, "%s: I_HOLIDAYS_A  case 1 t1[%d], nv_pos=%d\r\n",
                    __func__, t1, nv_pos);

            b_CRC_chk = FALSE;
        }
        else
        {
            DPRINTF(
                DBG_WARN,
                "%s: I_HOLIDAYS else case 2 t1[%d], nv_sub_info.ch[0]=%d\r\n",
                __func__, t1, nv_sub_info.ch[0]);
            nv_pos =
                (uint32_t)&(nv_type_ptr->pgm[t1].holidays[nv_sub_info.ch[0]]);
            nv_size = sizeof(holiday_date_type);

            DPRINTF(DBG_WARN,
                    "%s: I_HOLIDAYS else case 2 t1[%d], nv_pos=%d\r\n",
                    __func__, t1, nv_pos);
        }
        break;

    case I_DISP_SUPP_A:
    case I_DISP_SUPP_P:
    case I_DISP_SUPP_DL:
    case I_DISP_SUPP_RCNT:
        if (nv_item == I_DISP_SUPP_A)
        {
            t1 = aprog_area;
        }
        else if (nv_item == I_DISP_SUPP_P)
        {
            t1 = pprog_area;
        }
        else if (nv_item == I_DISP_SUPP_DL)
        {
            if (prog_dl_idx < NUM_PGM_AREA)
            {
                t1 = prog_dl_idx;
            }
            else
            {
                nv_err = NV_RANGE_ERROR;
                break;
            }
        }
        else
        {
            t1 = nv_sub_info.ch[0];
        }
        nv_pos = (uint32_t)&(nv_type_ptr->pgm[t1].dsp_supp);
        nv_size = sizeof(dsp_supply_type);
        break;

    case I_SUPPDSP_ITEM_DL:
    case I_SUPPDSP_ITEM_A:
    case I_SUPPDSP_ITEM_P:
        if (nv_item == I_SUPPDSP_ITEM_A)
        {
            t1 = aprog_area;
        }
        else if (nv_item == I_SUPPDSP_ITEM_P)
        {
            t1 = pprog_area;
        }
        else
        {
            if (prog_dl_idx < NUM_PGM_AREA)
            {
                t1 = prog_dl_idx;
            }
            else
            {
                nv_err = NV_RANGE_ERROR;
                break;
            }
        }

        if (wr)
        {
            if ((nv_sub_info.seg.offset + nv_sub_info.seg.len) <=
                sizeof(dsp_supply_set_type))
            {
                nv_pos = (uint32_t)&(nv_type_ptr->pgm[t1].suppitem);
                nv_pos += (uint32_t)nv_sub_info.seg.offset;
                nv_size = nv_sub_info.seg.len;
            }
            else
            {
                nv_err = NV_RANGE_ERROR;
            }
        }
        else
        {
            nv_pos = (uint32_t)&(nv_type_ptr->pgm[t1].suppitem);
            nv_size = sizeof(dsp_supply_set_type);
        }

        b_CRC_chk = FALSE;
        break;

    case I_SUPPDSP_ITEM_RCNT:
        if ((nv_sub_info.seg.offset + nv_sub_info.seg.len) <=
            sizeof(dsp_supply_set_type))
        {
            nv_pos = (uint32_t)&(nv_type_ptr->pgm[aprog_area_rcnt].suppitem);
            nv_pos += (uint32_t)nv_sub_info.seg.offset;
            nv_size = nv_sub_info.seg.len;
        }
        else
        {
            nv_err = NV_RANGE_ERROR;
        }

        b_CRC_chk = FALSE;
        break;

    case I_PROG_INFO:
        nv_pos = (uint32_t)&(nv_type_ptr->proginfo[nv_sub_info.ch[0]]);
        nv_size = sizeof(program_info_type);
        break;

    case I_CUR_PROG_DL:
        nv_pos = (uint32_t)&(nv_type_ptr->pgm_dl_a);
        nv_size = sizeof(prog_dl_type);
        break;

    case I_FUT_PROG_DL:
        nv_pos = (uint32_t)&(nv_type_ptr->pgm_dl_p);
        nv_size = sizeof(prog_dl_type);
        break;

    case I_LOG_DATA:
        if (wr)
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr->log_data[nv_sub_info.ch[0]].dt[nv_sub_info.ch[1]]);
            nv_size = (uint8_t)sizeof(date_time_type);
        }
        else
        {
            nv_pos = (uint32_t)&(nv_type_ptr->log_data[nv_sub_info.ch[0]]);
            nv_size = (uint8_t)sizeof(log_data_type);
        }
        b_CRC_chk = FALSE;
        break;

    case I_LOG_DATA1:
        if (wr)
        {
            nv_pos = (uint32_t)&(nv_type_ptr->log_data1.evt[nv_sub_info.ch[0]]);
            nv_size = (uint8_t)sizeof(evt_durtime_type);
        }
        else
        {
            nv_pos = (uint32_t)&(nv_type_ptr->log_data1);
            nv_size = (uint8_t)sizeof(log_data1_type);
        }
        b_CRC_chk = FALSE;
        break;

    case I_LOG_SCURR:
        if (wr)
        {
            nv_pos =
                (uint32_t)&(nv_type_ptr->log_data_scur.evt[nv_sub_info.ch[0]]);
            nv_size = sizeof(scurr_log_info_type);
        }
        else
        {
            nv_pos = (uint32_t)&(nv_type_ptr->log_data_scur);
            nv_size = sizeof(scurr_log_data_type);
        }
        b_CRC_chk = FALSE;
        break;

    case I_PREPAY:
        nv_pos = (uint32_t)&(nv_type_ptr->prepay);
        nv_size = sizeof(prepay_info_type);
        break;

    case I_IMAX_LOG:
        nv_pos = (uint32_t)&(nv_type_ptr->imaxlog[nv_sub_info.ch[0]]);
        nv_size = sizeof(imax_log_type);
        break;

    case I_TOVER_LOG:
        nv_pos = (uint32_t)&(nv_type_ptr->toverlog);
        nv_size = sizeof(tempover_log_type);
        break;

    case I_SAG_SWELL:
        nv_pos = (uint32_t)&(nv_type_ptr->sag_swell);
        nv_size = sizeof(sag_swell_info_type);
        break;

    case I_UTIL_PASSWORD:
        nv_pos = (uint32_t)&(nv_type_ptr->utilpwd);
        nv_size = sizeof(auth_pwd_type);
        break;

    case I_485_PASSWORD:
        nv_pos = (uint32_t)&(nv_type_ptr->comm485pwd);
        nv_size = sizeof(auth_pwd_type);
        break;

    case I_CUR_DATA_CAP:
        if (nv_sub_info.cur.sel == eMrAccm)
        {
            nv_pos = (uint32_t)&(nv_type_ptr->curr_mr_cap[idx_cap_wear]
                                     .accm[nv_sub_info.cur.rt]);
            nv_size = sizeof(mr_ch_type);
        }
        else if (nv_sub_info.cur.sel == eMrDm)
        {
            nv_pos = (uint32_t)&(
                nv_type_ptr->curr_mr_cap[idx_cap_wear].dm[nv_sub_info.cur.rt]);
            nv_size = sizeof(mr_dm_type);
        }
        else if (nv_sub_info.cur.sel == eMrInfo)
        {
            nv_pos = (uint32_t)&(nv_type_ptr->curr_mr_cap[idx_cap_wear].info);
            nv_size = sizeof(mr_data_info_type);
        }
        else
        {
            nv_pos = (uint32_t)&(nv_type_ptr->sublock_cap[idx_cap_wear]
                                     .roll[nv_sub_info.cur.rt]
                                     .ch[nv_sub_info.cur.chsel]);
            nv_size = sizeof(rolling_dm_ch_type);
        }
        break;

    case I_CUR_DM_CAP:
        nv_pos = (uint32_t)&(nv_type_ptr->curr_dm_cap[idx_cap_wear]);
        nv_size = sizeof(recent_demand_type);
        break;

    case I_RCNT_DM_CAP:
        nv_pos = (uint32_t)&(nv_type_ptr->rcnt_dm_cap[idx_cap_wear]);
        nv_size = sizeof(recent_demand_type);
        break;

    case I_DM_INTV_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->rolldm);
        nv_size = sizeof(rolling_dm_type);
        break;

    case I_DM_SUBLOCKS_DATA:
        if (nv_sub_info.cur.rt == numRates)
        {
            nv_pos = (uint32_t)&(nv_type_ptr->sublock.roll[0]);
            nv_size = sizeof(rolling_dm_rate_type);
        }
        else
        {
            nv_pos = (uint32_t)&(nv_type_ptr->sublock.roll[nv_sub_info.cur.rt]
                                     .ch[nv_sub_info.cur.chsel]);
            nv_size = sizeof(rolling_dm_ch_type);
        }
        break;

    case I_PWF_DATE_TIME:
        nv_pos = (uint32_t)&(nv_type_ptr->pwf_date_time);
        nv_size = sizeof(pwf_date_type);
        break;

    case I_WORKPWR_FLAG:
        nv_pos = (uint32_t)&(nv_type_ptr->_work_pwr_flag);
        nv_size = sizeof(Work_PwrF_data_type);
        break;

    case I_MIN_FREQ:
        nv_pos = (uint32_t)&(nv_type_ptr->_min_freq);
        nv_size = sizeof(min_max_data_type);
        break;

    case I_MAX_FREQ:
        nv_pos = (uint32_t)&(nv_type_ptr->_max_freq);
        nv_size = sizeof(min_max_data_type);
        break;

#if 1 /* bccho, 2024-09-05, 삼상 */
    case I_MIN_VOLT_L1:
        nv_pos = (U32) & (nv_type_ptr->_min_volt[0]);
        nv_size = sizeof(min_max_data_type);
        break;

    case I_MAX_VOLT_L1:
        nv_pos = (U32) & (nv_type_ptr->_max_volt[0]);
        nv_size = sizeof(min_max_data_type);
        break;

    case I_MIN_VOLT_L2:
        nv_pos = (U32) & (nv_type_ptr->_min_volt[1]);
        nv_size = sizeof(min_max_data_type);
        break;

    case I_MAX_VOLT_L2:
        nv_pos = (U32) & (nv_type_ptr->_max_volt[1]);
        nv_size = sizeof(min_max_data_type);
        break;

    case I_MIN_VOLT_L3:
        nv_pos = (U32) & (nv_type_ptr->_min_volt[2]);
        nv_size = sizeof(min_max_data_type);
        break;

    case I_MAX_VOLT_L3:
        nv_pos = (U32) & (nv_type_ptr->_max_volt[2]);
        nv_size = sizeof(min_max_data_type);
        break;
#else
    case I_MIN_VOLT:
        nv_pos = (uint32_t)&(nv_type_ptr->_min_volt);
        nv_size = sizeof(min_max_data_type);
        break;

    case I_MAX_VOLT:
        nv_pos = (uint32_t)&(nv_type_ptr->_max_volt);
        nv_size = sizeof(min_max_data_type);
        break;
#endif
    case I_ADJ_TEMP_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->temp_adj);
        nv_size = sizeof(temp_adj_data_type);
        break;

    case I_TOU_SET_CNT:
        nv_pos = (uint32_t)&(nv_type_ptr->tousetcnt);
        nv_size = sizeof(tou_set_cnt_type);
        break;

    case I_BACKUP_PROG_DL:
        nv_pos = (uint32_t)&(nv_type_ptr->pgm_dl_b);
        nv_size = sizeof(prog_dl_type);
        break;

    case I_HOL_DATE_BLOCK:
        nv_pos = (uint32_t)&(nv_type_ptr->holdateblock);
        nv_size = sizeof(holiday_date_type);
        break;

    case I_LOG_CERT_DATA:
        if (wr)
        {
            nv_pos = (uint32_t)&(nv_type_ptr->log_cert_data[nv_sub_info.ch[0]]
                                     .evt[nv_sub_info.ch[1]]);
            nv_size = (uint8_t)sizeof(evt_cert_time_type);
        }
        else
        {
            nv_pos = (uint32_t)&(nv_type_ptr->log_cert_data[nv_sub_info.ch[0]]);
            nv_size = (uint8_t)sizeof(log_cert_data_type);
        }
        b_CRC_chk = FALSE;
        break;
    case I_TOU_IMG_INFO:
        nv_pos = (uint32_t)&(nv_type_ptr->tou_image_info);
        nv_size = sizeof(ST_TOU_IMG_INFO);

        break;
    case I_TOU_IMG_DATA:
        nv_pos =
            (uint32_t)&(nv_type_ptr->tou_image_data.image[nv_sub_info.i16[0]]);
        nv_size = nv_sub_info.i16[1];
        b_CRC_chk = FALSE;

        break;
    case I_FW_IMG_DOWNLOAD_INFO:
        nv_pos = (uint32_t)&(nv_type_ptr->fw_image_dlinfo);
        nv_size = sizeof(ST_FW_IMG_DOWNLOAD_INFO);

        break;
    case I_FW_IMG_INFO:
        nv_pos = (uint32_t)&(nv_type_ptr->fw_info[nv_sub_info.ch[0]]);
        nv_size = (uint8_t)sizeof(ST_FW_INFO);
        break;

    case I_SUB_FW_IMG_DOWNLOAD_INFO:
        nv_pos = (uint32_t)&(nv_type_ptr->sub_fw_info);
        nv_size = sizeof(ST_SUB_FW_IMG_DL_INFO);
        break;

    case I_SUB_FW_IMG_DATA:
        nv_pos = (uint32_t)&(nv_type_ptr->sub_fw_img.data[nv_sub_info.i16[0]]);
        nv_size = nv_sub_info.i16[1];
        b_CRC_chk = FALSE;
        break;

    case I_RANDOM_SEED:
        nv_pos = (uint32_t)&(nv_type_ptr->random);
        nv_size = sizeof(ST_RANDOM);
        break;

    case I_CERTI_HASH: /* bccho, 2024-09-04 */
        nv_pos = (uint32_t)&(nv_type_ptr->nv_dummy_31);
        nv_size = 32;
        b_CRC_chk = FALSE;
        break;

    case I_MTP_CAL_POINT:
        nv_pos = (uint32_t)&(nv_type_ptr->mtp_calpoint);
        nv_size = sizeof(ST_MTP_CAL_POINT);
        break;

    case I_MTP_PARM:
        nv_pos = (uint32_t)&(nv_type_ptr->mtp_parm);
        nv_size = sizeof(ST_MTP_PARM);
        break;

    case I_MTP_SAG_SWELL:
        nv_pos = (uint32_t)&(nv_type_ptr->mtp_sagswell);
        nv_size = sizeof(ST_MTP_SAGSWELL);
        break;

    case I_PUSH_ACTI_ERR:
        nv_pos = (uint32_t)&(nv_type_ptr->push_act_err);
        nv_size = sizeof(ST_PUSH_ACTI_ERRCODE);

        break;

    case I_PUSH_SCRIPT_TABLE:
        nv_pos = (uint32_t)&(nv_type_ptr->push_script_table);
        nv_size = sizeof(ST_PUSH_SCRIPT_TABLE);

        break;

    case I_PUSH_SETUP_TABLE:
        nv_pos = (uint32_t)&(nv_type_ptr->push_setup_table);
        nv_size = sizeof(ST_PUSH_SETUP_TABLE);

        break;

    case I_MT_RST_TIME:
        nv_pos = (uint32_t)&(nv_type_ptr->mt_rst_time);
        nv_size = sizeof(ST_MT_RST_TIME);

        break;

    case I_EXT_MODEM_ID:
        nv_pos = (uint32_t)&(nv_type_ptr->ext_modem_id);
        nv_size = sizeof(ST_MDM_ID);

        break;

    case I_STOCK_OP_TIMES:
        nv_pos = (uint32_t)&(nv_type_ptr->stock_op_times);
        nv_size = sizeof(ST_STOCK_OP_TIMES);

        break;

    case I_ZCD_RESULT_TIME:
        nv_pos = (uint32_t)&(nv_type_ptr->zcd_result_time);
        nv_size = sizeof(ST_ZCD_RESULT_TIME);

        break;

    case I_MODEM_BAUD:
        nv_pos = (uint32_t)&(nv_type_ptr->baud);
        nv_size = sizeof(ST_AT_BAUD);

        break;
    case I_BOOT_AFTER_SWRST:
        nv_pos = (uint32_t)&(nv_type_ptr->swrst_boot_flag);
        nv_size = sizeof(ST_SWRST_BOOT_FLAG);

        break;
    default:
        nv_err = NV_UNDEF_ITEM_ERROR;
        break;
    }

    if (nv_err != NV_OK)
    {
        return false;
    }

    eep_wp_disable();

    if (nv_item == I_ALL_CLEAR || nv_item == I_EXTN_CLEAR)
    {
        if (WMStatus_intern & (GE_MEM | GE_MEM1))
        {
            t3 = 1;
        }
        else
        {
            t3 = 3;
        }
        DPRINTF(DBG_WARN, "%s: %s: t3[%d], nv_pos[%d], nv_size[%d]\r\n",
                __func__, dsm_nv_item_string(nv_item), t3, nv_pos, nv_size);

        for (t1 = 0; t1 < t3; t1++)
        {
            pstatus = dsm_eeprom_erase(nv_pos, nv_size);
            if (pstatus == HAL_OK)
            {
                break;
            }
            delay_for_nvreacs();
        }
    }
    else
    {
        if (wr) /* true: write */
        {
            if (b_CRC_chk)
            {
                crc16_chk(nv_data, nv_size, true);
            }

            if (WMStatus_intern & (GE_MEM | GE_MEM1))
            {
                t3 = 1;
            }
            else
            {
                t3 = 3;
            }

            for (t1 = 0; t1 < t3; t1++)
            {
                pstatus = dsm_eeprom_write(false, nv_pos, nv_data, nv_size);
                if (pstatus == HAL_OK)
                {
                    break;
                }
                delay_for_nvreacs();
            }
        }
        else /* false: read */
        {
            if (WMStatus_intern & (GE_MEM | GE_MEM1))
            {
                t3 = 1;
            }
            else
            {
                t3 = 3;
            }

            for (t2 = 0; t2 < t3; t2++)
            {
                pstatus = dsm_eeprom_read(nv_pos, nv_data, nv_size);
                if (pstatus == HAL_OK)
                {
                    break;
                }
                delay_for_nvreacs();
            }

            if ((pstatus == HAL_OK) && b_CRC_chk)
            {
                if (!crc16_chk(nv_data, nv_size, false))
                {
                    MSG00("EEPROM__Read CRC, %s", dsm_nv_item_string(nv_item));
                    nv_err = NV_CRC_ERROR;
                }
            }
        }
    }

    eep_wp_enable();

    if (pstatus != HAL_OK)
    {
        WMStatus_intern |= GE_MEM;
        DPRINTF(DBG_ERR, _D "%s: WMStatus |= GE_MEM\r\n", __func__);

        return false;
    }

    WMStatus_intern &= ~GE_MEM;

    return (nv_err == NV_OK);
}

void set_prog_dl_idx(void)
{
    uint8_t i, tarea;

    tarea = 0;

    DPRINTF(DBG_WARN, _D "%s: available[cur %d, fut %d]\r\n", __func__,
            prog_curr_is_available(), prog_fut_is_available());

    if (prog_curr_is_available())
        tarea |= 0x01 << aprog_area;

    if (prog_fut_is_available())
        tarea |= 0x01 << pprog_area;

    for (i = 0; i < NUM_PGM_AREA; i++)
    {
        if (!(tarea & 0x01))
            break;
        tarea >>= 1;
    }

    prog_dl_idx = i;
    DPRINTF(DBG_TRACE, _D "%s: prog_dl_idx[%d]\r\n", __func__, i);
}

uint8_t get_prog_dl_idx(void) { return prog_dl_idx; }

void set_hol_dl_idx(void)
{
    uint8_t i, tarea;

    tarea = 0;

    DPRINTF(DBG_WARN, _D "%s: available[cur %d, fut %d]\r\n", __func__,
            prog_hol_available(), futprog_hol_available());

    if (prog_curr_is_available() && prog_hol_available())
        tarea |= 0x01 << ahol_area;

    if (prog_fut_is_available() && futprog_hol_available())
        tarea |= 0x01 << phol_area;

    for (i = 0; i < NUM_PGM_AREA; i++)
    {
        if (!(tarea & 0x01))
            break;
        tarea >>= 1;
    }

    hol_dl_idx = i;
    DPRINTF(DBG_TRACE, _D "%s: hol_dl_idx[%d]\r\n", __func__, hol_dl_idx);
}

uint8_t get_hol_dl_idx(void) { return hol_dl_idx; }

static void delay_for_nvreacs(void)
{
#if 1 /* bccho, 2023-11-17 */
    vTaskDelay(10);
#endif
    // why this code used?, Delay time already
    // applied in eeprom function.
#if 0 
    delay(DELAY_MS(NVACS_DELAY_MS));
#endif
}
