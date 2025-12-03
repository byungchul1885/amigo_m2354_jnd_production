#include "options_sel.h"
#include "typedef.h"
#include "kconf.h"
#include "amg_global.h"
#include "amg_shell.h"
#include "amg_shell_peri.h"
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
#include "appl.h"
#include "whm.h"
#include "amg_mtp_process.h"
#include "amg_lcd.h"
#include "nv.h"
#include "utils.h"
#include "amg_sec.h"
#include "amg_utc_util.h"
#include "amg_power_mnt.h"
#include "amg_rtc.h"
#include "amg_wdt.h"
#include "platform.h"
#include "amg_media_mnt.h"
#include "amg_modemif_prtl.h"
#include "amg_spi.h"
#include "amg_meter_main.h"
#include "key.h"
#include "softimer.h"
#include "eadc_vbat.h"
#include "kepco_cert.h"

/*
******************************************************************************
*    LOCAL DATA TYPES
******************************************************************************
*/
static SHELL_ERR shell_peri_key(UINT32 id, char *pParamStr, UINT32 size);
// static SHELL_ERR shell_peri_sflash (UINT32 id, char *pParamStr, UINT32 size);
#ifdef M2354_CAN /* bccho, 2023-11-28 */
static SHELL_ERR shell_peri_can(UINT32 id, char *pParamStr, UINT32 size);
#endif
static SHELL_ERR shell_peri_low_pwr_mode(UINT32 id, char *pParamStr,
                                         UINT32 size);
static SHELL_ERR shell_peri_nvmt(UINT32 id, char *pParamStr, UINT32 size);
static SHELL_ERR shell_peri_default(UINT32 id, char *pParamStr, UINT32 size);

kepco_cert_storage_t kcs;

/*
******************************************************************************
*    GLOBAL VARIABLES
******************************************************************************
*/
extern ST_SEC_M_POWER_ON_RLT power_on_rlt;

const SHELL_CNTX shell_peri_cmd_list[] = {

    {0, "485", _H(""), shell_peri_default, NULL},
    {1, "flash", _H(""), shell_peri_default, NULL},
    {2, "ext_wdt", _H(""), shell_peri_default, NULL},
    {4, "nv_size", _H(""), shell_peri_nvmt, NULL},
    {5, "nvmt", _H("write[id][sz][data]|read[id]|list"), shell_peri_nvmt, NULL},
    {6, "mcu_info", _H(""), shell_peri_default, NULL},
    {7, "lcd", _H(""), shell_peri_default, NULL},
    {8, "eeprom", _H(""), shell_peri_default, NULL},
    {9, "card", _H(""), shell_peri_default, NULL},
    {10, "load", _H(""), shell_peri_default, NULL},
    {11, "bor", _H(""), shell_peri_default, NULL},
    {12, "mif",
     _H("|caldw|caldg|parm|calst|sagset|sagsp|baud|pushack|pushnack|fsminit|"
        "mifnv|firmreq|"),
     shell_peri_default, NULL},
    {13, "mif2",
     _H("|setack|setnack|actack|actnack|sagrsp|sagsprsp|caldsrsp|caldgrsp|"
        "baudrsp|pushdata|firmrsp|"),
     shell_peri_default, NULL},
    {14, "zcd", _H(""), shell_peri_default, NULL},
    {15, "float", _H(""), shell_peri_default, NULL},
    {16, "adc", _H(""), shell_peri_default, NULL},
    {17, "lowpwr", _H(""), shell_peri_low_pwr_mode, NULL},
    {18, "iwdt", _H(""), shell_peri_default, NULL},
#ifdef M2354_CAN /* bccho, 2023-11-28 */
    {19, "can", _H(""), shell_peri_can, NULL},
#endif
    {20, "key", _H(""), shell_peri_key, NULL},
    {21, "eoi", _H(""), shell_peri_default, NULL},
    {22, "hw_reset", _H(""), shell_peri_default, NULL},

    {0, 0, 0, 0, 0}};

void dsm_mif_getreq_sagswell_data(void);
void dsm_mtp_meter_fw_download_proc(void);
void dsm_mtp_set_fw_index(UINT8 mode);
bool dsm_mtp_fsm_tx_proc_fw_data_set(UINT8 type, UINT8 idx);
void dsm_mtp_fsm_tx_proc_fw_data_get(UINT8 type, UINT8 idx);
void dsm_mtp_set_fw_type(UINT8 mode);
void dsm_mif_getreq_firmware_ver_data(void);

static SHELL_ERR shell_peri_key(UINT32 id, char *pParamStr, UINT32 size)
{
    char *argv[36];
    // UINT8   buff[32];
    UINT32 argc;

    switch (id)
    {
    case 20:
        return SHELL_OK;
        break;

    default:
        break;
    }

    return SHELL_INVALID_PARAM;
}

#ifdef M2354_CAN /* bccho, 2023-11-28 */
static SHELL_ERR shell_peri_can(UINT32 id, char *pParamStr, UINT32 size)
{
    char *argv[36];
    // UINT8   buff[32];
    UINT32 argc;

    switch (id)
    {
    case 19:
    {  // can
        if (size < 4)
        {
            return SHELL_INVALID_PARAM;
        }

        shell_arg_parse(pParamStr, &argc, argv, 1, ' ');

        if (!strcmp(argv[0], "init"))
        {
            dsm_can_init(CAN_BITRATE_500K);
            dsm_isotp_user_init();
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "tptx"))
        {
            UINT8 tx_data[512];
            UINT16 cnt;

            for (cnt = 0; cnt < 512; cnt++)
            {
                tx_data[cnt] = cnt;
            }

            dsm_media_set_fsm_if_hdlc(MEDIA_RUN_CAN);

            // dsm_media_set_rx_evt(CAN_IF_RX_EVT);
            // dsm_media_chg_evt_proc(dsm_media_get_rx_evt(), 0);
            // dsm_media_if_hdlc_fsm_proc(dsm_media_get_chg_evt());

            dsm_isotp_user_tx(DLMS_EX_MODE, tx_data, 512);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "disc"))
        {
            UINT8 disc[10] = {0x7e, 0xa0, 0x08, 0x24, 0xff,
                              0x25, 0x53, 0xb4, 0x2b, 0x7e};

            dsm_media_set_fsm_if_hdlc(MEDIA_RUN_CAN);

            // dsm_media_set_rx_evt(CAN_IF_RX_EVT);
            // dsm_media_chg_evt_proc(dsm_media_get_rx_evt(), 0);
            // dsm_media_if_hdlc_fsm_proc(dsm_media_get_chg_evt());

            // dsm_isotp_user_tx(DLMS_EX_MODE, disc, 10);
            dsm_media_if_send(MEDIA_PROTOCOL_IF_HDLC, FALSE, disc, 10);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "1_advert"))
        {
            dsm_can_advertisement_power_on();

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "2_advert"))
        {
            dsm_can_advertisement_solicitation();

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "solicit"))
        {
            dsm_can_solicitation();

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "push_lp"))
        {
            appl_push_msg_lastLP();

            return SHELL_OK;
        }
    }
    break;

    default:
        break;
    }

    return SHELL_INVALID_PARAM;
}
#endif

bool g_dm_poll_enable = 0;
void dsm_dm_poll_set(bool val)
{
    DPRINTF(DBG_TRACE, "%s: poll_enable[%d]\r\n", __func__, val);
    g_dm_poll_enable = val;
}

bool dsm_dm_poll_get(void) { return g_dm_poll_enable; }

static SHELL_ERR shell_peri_low_pwr_mode(UINT32 id, char *pParamStr,
                                         UINT32 size)
{
    char *argv[36];
    // UINT8   buff[32];
    UINT32 argc;

    switch (id)
    {
    case 17:
    {  // lowpwr
        if (size < 3)
        {
            return SHELL_INVALID_PARAM;
        }

        shell_arg_parse(pParamStr, &argc, argv, 2, ' ');

        if (argc == 1)
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "alarm_get"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "pwr_get"))
        {
            return SHELL_OK;
        }
        else if (argc == 2)
        {
            if (!strcmp(argv[0], "dm_poll"))
            {
                if (!strcmp(argv[1], "enable"))
                {
                    dsm_dm_poll_set(true);
                    return SHELL_OK;
                }
                else if (!strcmp(argv[1], "disable"))
                {
                    dsm_dm_poll_set(false);
                    return SHELL_OK;
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

static SHELL_ERR shell_peri_nvmt(UINT32 id, char *pParamStr, UINT32 size)
{
    char *argv[36];
    // UINT8   buff[32];
    UINT32 argc;

    switch (id)
    {
    case 4:
    {  // nv_size
        return SHELL_OK;
    }
    break;

    case 5:
    {  // nvmt
        ST_MIF_CAL_DATA *p_cal_data = dsm_mtp_get_cal_data();
        cal_data_type cal;

        if (size < 5)
        {
            return SHELL_INVALID_PARAM;
        }

        shell_arg_parse(pParamStr, &argc, argv, 1, ' ');

        if (!strcmp(argv[0], "cal_w"))
        {
            cal.T_cal_i0 = p_cal_data->r_current_gain;
            cal.T_cal_v0 = p_cal_data->r_voltage_gain;
            cal.T_cal_p0 = p_cal_data->r_phase_gain;

            DPRINTF(DBG_TRACE,
                    "CAL NV write: cur_gain[0x%08X], vol_gain[0x%08X], "
                    "phase_gain[0x%08X]\r\n",
                    p_cal_data->r_current_gain, p_cal_data->r_voltage_gain,
                    p_cal_data->r_phase_gain);

            nv_write(I_CAL_DATA, (U8 *)&cal);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "cal_r"))
        {
            nv_read(I_CAL_DATA, (U8 *)&cal);

            DPRINTF(DBG_TRACE,
                    "CAL NV read: cur_gain[0x%08X], vol_gain[0x%08X], "
                    "phase_gain[0x%08X]\r\n",
                    cal.T_cal_i0, cal.T_cal_v0, cal.T_cal_p0);

            return SHELL_OK;
        }
    }
    break;

    default:
        break;
    }

    return SHELL_INVALID_PARAM;
}

static SHELL_ERR shell_peri_default(UINT32 id, char *pParamStr, UINT32 size)
{
    char *argv[36];
    // UINT8   buff[32];
    UINT32 argc;
#if defined(FEATURE_ZBM_SDK_USE)
    UINT8 val;
#endif

    switch (id)
    {
    case 0:
        break;

    case 2:
        break;

    case 6:
        break;

    case 7:
        break;

    case 8:
    {  // eeprom
        if (size < 4)
        {
            return SHELL_INVALID_PARAM;
        }

        shell_arg_parse(pParamStr, &argc, argv, 3, ' ');

        if (argc == 1)
        {
            if (!strcmp(argv[0], "test"))
            {
                UINT8 string_tx[51] =
                    "EEPROM Test - ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
                UINT8 string_rx[50];  //, string_erase[50];

                memset(string_rx, 0x00, 50);
                DPRINTF2(DBG_TRACE, "EEPROM test vector: %s\r\n", string_tx);

                dsm_eeprom_write(FALSE, 0, string_tx, 50);
                OSTimeDly(OS_MS2TICK(10));
                dsm_eeprom_read(0, string_rx, 50);
                if (memcmp((uint8_t *)string_tx, (uint8_t *)string_rx, 50))
                {
                    DPRINTF2(DBG_TRACE,
                             "low addressing of the memory array test : "
                             "fail\r\n");
                    DPRINT_HEX(DBG_TRACE, "E2P_RX", string_rx, 50, DUMP_ALWAYS);
                }
                else
                {
                    DPRINTF2(DBG_TRACE,
                             "low addressing of the memory array test : "
                             "success\r\n");
                    DPRINT_HEX(DBG_TRACE, "E2P_RX", string_rx, 50, DUMP_ALWAYS);
                }

                // array test
                dsm_eeprom_write(FALSE, 0x40000, string_tx, 50);
                OSTimeDly(OS_MS2TICK(10));
                dsm_eeprom_read(0x40000, string_rx, 50);

                if (memcmp((uint8_t *)string_tx, (uint8_t *)string_rx, 50))
                {
                    DPRINTF2(DBG_TRACE,
                             "high addressing of the memory array test : "
                             "fail\r\n");
                    DPRINT_HEX(DBG_TRACE, "E2P_RX", string_rx, 50, DUMP_ALWAYS);
                }
                else
                {
                    DPRINTF2(DBG_TRACE,
                             "high addressing of the memory array test : "
                             "success\r\n");
                    DPRINT_HEX(DBG_TRACE, "E2P_RX", string_rx, 50, DUMP_ALWAYS);
                }

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "init"))
            {
                dsm_eeprom_init();
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "sizeof"))
            {
                DPRINTF2(DBG_TRACE, "DATA_SIZE : %lX\r\n", sizeof(Nv_type));
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pwr_on"))
            {
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pwr_off"))
            {
                return SHELL_OK;
            }
        }
        //
        else if (argc == 2)
        {
            if (!strcmp(argv[0], "erase"))
            {
                if (!strcmp(argv[1], "chip"))
                {
                    DPRINTF2(DBG_TRACE, "FULL_CHIP_ERASE\r\n");
                    dsm_eeprom_erase(0, 0x80000);  // 256KB * 2 EA
                    PRINTF("\r\n!!!!CHIP_ERASE_COMPLETE\r\n");

                    return SHELL_OK;
                }
            }
        }
        //
        else if (argc == 3)
        {
            UINT8 buff[1024];
            UINT32 addr, len, i;

            memset(buff, 0x00, 1024);

            if (!strcmp(argv[0], "read"))
            {
                addr = atoi(argv[1]);
                len = atoi(argv[2]);

                if (len > sizeof(buff))
                {
                    return SHELL_INVALID_PARAM;
                }
                // 256KB * 2 EA
                else if ((addr + len) >
                         /*0x80000*/ EEPROM_LIMIT)
                {
                    return SHELL_INVALID_PARAM;
                }

                dsm_eeprom_read(addr, buff, len);

                DPRINT_HEX(DBG_TRACE, "E2P_READ", buff, len, DUMP_ALWAYS);
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "write"))
            {
                addr = atoi(argv[1]);
                len = atoi(argv[2]);

                if (len > sizeof(buff))
                {
                    return SHELL_INVALID_PARAM;
                }
                // 256KB * 2 EA
                else if ((addr + len) >
                         /*0x80000*/ EEPROM_LIMIT)
                {
                    return SHELL_INVALID_PARAM;
                }

                for (i = 0; i < len; i++)
                {
                    if (i < 256)
                        buff[i] = i;
                    else
                        buff[i] = i - 256;
                }
                dsm_eeprom_write(FALSE, addr, buff, len);

                DPRINT_HEX(DBG_TRACE, "E2P_WRITE", buff, len, DUMP_ALWAYS);
                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "erase"))
            {
                addr = atoi(argv[1]);
                len = atoi(argv[2]);

                dsm_eeprom_erase(addr, len);

                DPRINT_HEX(DBG_TRACE, "E2P_ERASE", buff, len, DUMP_ALWAYS);
                return SHELL_OK;
            }
        }
    }
    break;

    case 9:
    {
        // card
        if (size < 4)
        {
            return SHELL_INVALID_PARAM;
        }

        shell_arg_parse(pParamStr, &argc, argv, 1, ' ');

        if (!strcmp(argv[0], "test"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "test2"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "test3"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "erase"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "read"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "write"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "print"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "sec_pwron"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "sec_pwroff"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "sec_init"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "por_info"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "issue_clear"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "cert_insert_s"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "issue_on"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "cert_rd"))
        {
            return SHELL_OK;
        }
    }
    break;

    case 10:
    {  // load on/off
        if (size < 2)
        {
            return SHELL_INVALID_PARAM;
        }

        shell_arg_parse(pParamStr, &argc, argv, 1, ' ');

        if (!strcmp(argv[0], "on"))
        {
            rload_ctrl_set(1);  // load on and load ctrl state save
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "off"))
        {
            rload_ctrl_set(0);  // load off and load ctrl state save
            return SHELL_OK;
        }
    }
    break;

    case 11:
        break;

    case 12:
    {  // mif
        if (size < 3)
        {
            return SHELL_INVALID_PARAM;
        }

        float fval = 0.0;

        shell_arg_parse(pParamStr, &argc, argv, 4, ' ');

        if (!strcmp(argv[0], "parm"))
        {
            ST_MTP_PARM st_mtp_parm;

            if (argc != 1)
            {
                return SHELL_INVALID_PARAM;
            }

            DPRINTF(DBG_TRACE, "\r\nmeter parm set\r\n");
            if (!nv_read(I_MTP_PARM, (UINT8 *)&st_mtp_parm))
            {
                dsm_mtp_default_parm(&st_mtp_parm);
            }
            dsm_mif_setreq_meter_setup_parm((UINT8 *)&st_mtp_parm.val,
                                            sizeof(ST_MIF_METER_PARM));

            ToHFloat((U8_Float *)&fval, &st_mtp_parm.val.cut_voltage_thr[0]);
            DPRINTF(DBG_TRACE, "cut_vol_thres: %d.%03d\r\n", (UINT32)(fval),
                    (UINT32)((fval - (UINT32)(fval)) * 1000));
            ToHFloat((U8_Float *)&fval, &st_mtp_parm.val.start_current_thr[0]);
            DPRINTF(DBG_TRACE, "start_curr_thres: %d.%03d\r\n", (UINT32)(fval),
                    (UINT32)((fval - (UINT32)(fval)) * 1000));
            DPRINTF(DBG_TRACE,
                    "DIR[%02X], REATIVE_SEL[%02X], MT_METHOD[%02X], "
                    "PULSE_SEL[%02X]\r\n",
                    st_mtp_parm.val.direct_reverse,
                    st_mtp_parm.val.reactive_select,
                    st_mtp_parm.val.meter_method, st_mtp_parm.val.pulse_select);
            return SHELL_OK;
        }
        else if (argc == 1)
        {
            if (!strcmp(argv[0], "calst"))
            {
                ST_MTP_CAL_POINT st_mtp_cal_point;
                dsm_mtp_default_cal_point(&st_mtp_cal_point);

                ToHFloat((U8_Float *)&fval,
                         &st_mtp_cal_point.val.ref_voltage[0]);
                DPRINTF(DBG_TRACE, "voltage: %d.%03d\r\n", (UINT32)(fval),
                        (UINT32)((fval - (UINT32)(fval)) * 1000));
                ToHFloat((U8_Float *)&fval,
                         &st_mtp_cal_point.val.ref_current[0]);
                DPRINTF(DBG_TRACE, "current: %d.%03d\r\n", (UINT32)(fval),
                        (UINT32)((fval - (UINT32)(fval)) * 1000));
                ToHFloat((U8_Float *)&fval, &st_mtp_cal_point.val.ref_phase[0]);
                DPRINTF(DBG_TRACE, "phase: %d.%03d\r\n", (UINT32)(fval),
                        (UINT32)((fval - (UINT32)(fval)) * 1000));
                DPRINTF(DBG_TRACE, "process_time: %02d SEC\r\n",
                        st_mtp_cal_point.val.process_time);
                DPRINTF(DBG_TRACE, "CONST: [%08d : %08d : %08d]\r\n",
                        st_mtp_cal_point.val.react_const,
                        st_mtp_cal_point.val.act_const,
                        st_mtp_cal_point.val.app_const);

                cal_begin();
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_CAL_ST);
                dsm_mtp_fsm_send();

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "sagset"))
            {
                ST_MTP_SAGSWELL st_mtp_sagswell;

                DPRINTF(DBG_TRACE, "\r\nsag/swell set\r\n");
                if (!nv_read(I_MTP_SAG_SWELL, (UINT8 *)&st_mtp_sagswell))
                {
                    dsm_mtp_default_sagswell(&st_mtp_sagswell);
                }
                dsm_mif_setreq_sagswell_data(
                    (UINT8 *)&st_mtp_sagswell.val,
                    sizeof(ST_MIF_SAGSWELL_SETUP));  // send setreq sagswell
                DPRINT_HEX(DBG_INFO,
                           "sag/swell set:", &st_mtp_sagswell.val.pf_level,
                           sizeof(ST_MIF_SAGSWELL_SETUP), DUMP_ALWAYS);
                ToHFloat((U8_Float *)&fval, &st_mtp_sagswell.val.pf_level[0]);
                DPRINTF(DBG_TRACE, "pf_level float: %d.%03d\r\n",
                        (UINT32)(fval),
                        (UINT32)((fval - (UINT32)(fval)) * 1000));
                DPRINTF(DBG_TRACE, "pf_contiue_time: 0x%04X\r\n",
                        st_mtp_sagswell.val.pf_continue_time);
                ToHFloat((U8_Float *)&fval, &st_mtp_sagswell.val.sag_level[0]);
                DPRINTF(DBG_TRACE, "sag_level float: %d.%03d\r\n",
                        (UINT32)(fval),
                        (UINT32)((fval - (UINT32)(fval)) * 1000));
                DPRINTF(DBG_TRACE, "sag_time: 0x%02X\r\n",
                        st_mtp_sagswell.val.sag_time);
                ToHFloat((U8_Float *)&fval,
                         &st_mtp_sagswell.val.swell_level[0]);
                DPRINTF(DBG_TRACE, "swell_level float: %d.%03d\r\n",
                        (UINT32)(fval),
                        (UINT32)((fval - (UINT32)(fval)) * 1000));
                DPRINTF(DBG_TRACE, "swell_time: 0x%02X\r\n",
                        st_mtp_sagswell.val.swell_time);
                return SHELL_OK;
            }
            // #if defined(FEATURE_SEPERATE_SAVE)
            else if (!strcmp(argv[0], "sagget"))
            {
                // ST_MTP_SAGSWELL st_mtp_sagswell;

                DPRINTF(DBG_TRACE, "\r\nsag/swell get\r\n");

                dsm_mif_getreq_sagswell_data();  // send getreq sagswell

                return SHELL_OK;
            }
            // #endif
            else if (!strcmp(argv[0], "default"))  // default NV write
            {
                ST_MTP_CAL_POINT st_mtp_cal_point;
                ST_MTP_PARM st_mtp_parm;
                ST_MTP_SAGSWELL st_mtp_sagswell;

                dsm_mtp_default_cal_point(&st_mtp_cal_point);
                dsm_mtp_default_parm(&st_mtp_parm);
                dsm_mtp_default_sagswell(&st_mtp_sagswell);

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "sagsp"))
            {
                ST_MIF_SAGSWELL_STOP st_mif_sagswell_stop;

                st_mif_sagswell_stop.tmp_time = dsm_htonl(5000);
                DPRINTF(DBG_TRACE, "\r\nsag/swell time stop\r\n");
                dsm_mif_actreq_sagswell_time((UINT8 *)&st_mif_sagswell_stop,
                                             sizeof(ST_MIF_SAGSWELL_STOP));

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "firmreq"))
            {
                DPRINTF(DBG_TRACE, "\r\nmeter firmver get\r\n");

                dsm_mif_getreq_firmware_ver_data();  // send getreq sagswell

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "caldw"))
            {
                ST_MIF_CAL_DATA st_mif_cal_data;

                st_mif_cal_data.r_current_gain =
                    dsm_htonl(g_mtp_caldata.r_current_gain);
                st_mif_cal_data.r_voltage_gain =
                    dsm_htonl(g_mtp_caldata.r_voltage_gain);
                st_mif_cal_data.r_phase_gain =
                    dsm_htonl(g_mtp_caldata.r_phase_gain);
                st_mif_cal_data.s_current_gain =
                    dsm_htonl(g_mtp_caldata.s_current_gain);
                st_mif_cal_data.s_voltage_gain =
                    dsm_htonl(g_mtp_caldata.s_voltage_gain);
                st_mif_cal_data.s_phase_gain =
                    dsm_htonl(g_mtp_caldata.s_phase_gain);
                st_mif_cal_data.t_current_gain =
                    dsm_htonl(g_mtp_caldata.t_current_gain);
                st_mif_cal_data.t_voltage_gain =
                    dsm_htonl(g_mtp_caldata.t_voltage_gain);
                st_mif_cal_data.t_phase_gain =
                    dsm_htonl(g_mtp_caldata.t_phase_gain);
                st_mif_cal_data.cal_ok = 1;
                DPRINTF(DBG_TRACE, "cal data write\r\n");
                dsm_mif_setreq_cal_data(
                    (UINT8 *)&st_mif_cal_data,
                    sizeof(ST_MIF_CAL_DATA));  // send setreq caldata

                DPRINTF(
                    DBG_TRACE,
                    "CAL_GAIN R: cur[0x%08X], vol[0x%08X], phase[0x%08X]\r\n",
                    g_mtp_caldata.r_current_gain, g_mtp_caldata.r_voltage_gain,
                    g_mtp_caldata.r_phase_gain);
                DPRINTF(
                    DBG_TRACE,
                    "CAL_GAIN S: cur[0x%08X], vol[0x%08X], phase[0x%08X]\r\n",
                    g_mtp_caldata.s_current_gain, g_mtp_caldata.s_voltage_gain,
                    g_mtp_caldata.s_phase_gain);
                DPRINTF(
                    DBG_TRACE,
                    "CAL_GAIN T: cur[0x%08X], vol[0x%08X], phase[0x%08X]\r\n",
                    g_mtp_caldata.t_current_gain, g_mtp_caldata.t_voltage_gain,
                    g_mtp_caldata.t_phase_gain);

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "caldg"))
            {
                DPRINTF(DBG_TRACE, "cal data read\r\n");
                dsm_mif_getreq_cal_data();

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pushack"))
            {
                DPRINTF(DBG_TRACE, "\r\nPushAck\r\n");
                dsm_mif_push_ack();

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "pushnack"))
            {
                DPRINTF(DBG_TRACE, "\r\nPushAck\r\n");
                dsm_mif_push_nack();

                return SHELL_OK;
            }
        }
        else if (argc == 2)
        {
            if (!strcmp(argv[0], "baud"))
            {
                ST_MIF_BAUD_RATE st_mif_baudrate;

                DPRINTF(DBG_TRACE, "\r\nbaudrate %d, size %d\r\n",
                        atoi(argv[1]), sizeof(ST_MIF_BAUD_RATE));
                st_mif_baudrate.rate = dsm_htonl(atoi(argv[1]));
                dsm_mif_setreq_baudrate((UINT8 *)&st_mif_baudrate,
                                        sizeof(ST_MIF_BAUD_RATE));

                return SHELL_OK;
            }
        }
        else  // (argc > 2)
        {
            if (!strcmp(argv[0], "zbm"))
            {
                return SHELL_OK;
            }
        }
    }
    break;

    case 13:
        break;

    case 14:
    {  // zcd
        if (size < 2)
        {
            return SHELL_INVALID_PARAM;
        }

        shell_arg_parse(pParamStr, &argc, argv, 1, ' ');

        if (!strcmp(argv[0], "on"))
        {
            // dsm_zcd_on_485_off();
            dsm_mif_zcd_on();
            return SHELL_OK;
        }
        else if (!strncmp(argv[0], "off", 3))
        {
            // dsm_485_on_zcd_off();
            dsm_mif_zcd_off();
            return SHELL_OK;
        }
    }
    break;

    case 15:
    {  // float
        return SHELL_OK;
    }
    break;

    case 16:
        break;

    case 18:
        break;

    case 21:
        break;

    case 22:
    {  // hw_reset
        return SHELL_OK;
    }
    break;

    default:
        break;
    }

    return SHELL_INVALID_PARAM;
}
