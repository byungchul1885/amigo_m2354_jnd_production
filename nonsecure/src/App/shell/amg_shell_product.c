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

static SHELL_ERR shell_product_default(uint32_t id, char *pParamStr,
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
static SHELL_ERR shell_product_default(uint32_t id, char *pParamStr,
                                       uint32_t size)
{
    char *argv[36];
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
            ser_no_type serno;

            get_cust_id((uint8_t *)&serno);
            DPRINT_HEX(DBG_TRACE, "METER_SERIAL", serno.ser, SERIAL_NO_SIZE,
                       DUMP_ALWAYS);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], /*"devid"*/ "ldn"))
        {
            device_id_type dev;

            nv_read(I_DEVICE_ID_KEPCO, (uint8_t *)&dev);
            DPRINT_HEX(DBG_TRACE, "KEPCO_MGMT", dev.devid, DEVICE_ID_SIZE,
                       DUMP_ALWAYS);

            nv_read(I_DEVICE_ID, (uint8_t *)&dev);
            DPRINT_HEX(DBG_TRACE, "DEVICE_MGMT", dev.devid, DEVICE_ID_SIZE,
                       DUMP_ALWAYS);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "meterid"))
        {
            uint8_t manuid[MANUF_ID_SIZE];

            get_manuf_id(manuid);
            DPRINT_HEX(DBG_TRACE, "METER_ID", manuid, MANUF_ID_SIZE,
                       DUMP_ALWAYS);
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "time"))
        {
            date_time_type dt = cur_rtc;

            DPRINTF(DBG_TRACE, "GET TIME: 20%02d.%02d.%02d %02d:%02d:%02d\r\n",
                    dt.year, dt.month, dt.date, dt.hour, dt.min, dt.sec);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "cert"))
        {
            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "temp"))
        {
            float temp;
            int32_t exponent, mantissa;
            temp = get_inst_temp();
            exponent = temp;
            mantissa = (temp - exponent) * 1000;
            if (mantissa < 0)
            {
                mantissa = (~mantissa) + 1;
            }
            DPRINTF(DBG_INFO, "Get Temp: %d.%03d\r\n", exponent,
                    mantissa);  // fucking user print
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
            if (argc != 8)
            {
                return SHELL_INVALID_PARAM;
            }

            /* prod set custid 30 30 30 30 31 30 35 --> "0000105" <- serial
             * number */
            ser_no_type serno;

            memset((uint8_t *)&serno, 0x00, sizeof(ser_no_type));

            /* 계기 ID 일련번호 */
            str_to_hex_uint8(argv[1], (uint8_t *)&serno.ser[0]);
            str_to_hex_uint8(argv[2], (uint8_t *)&serno.ser[1]);
            str_to_hex_uint8(argv[3], (uint8_t *)&serno.ser[2]);
            str_to_hex_uint8(argv[4], (uint8_t *)&serno.ser[3]);
            str_to_hex_uint8(argv[5], (uint8_t *)&serno.ser[4]);
            str_to_hex_uint8(argv[6], (uint8_t *)&serno.ser[5]);
            str_to_hex_uint8(argv[7], (uint8_t *)&serno.ser[6]);

            set_cust_id(&serno);

            DPRINT_HEX(DBG_TRACE, "METER_SERIAL", serno.ser, SERIAL_NO_SIZE,
                       DUMP_ALWAYS);

            /* software information */
            dsm_sys_fwinfo_initial_set(true);  // external flash info

            return SHELL_OK;
        }
        if (!strcmp(argv[0], "cid2"))
        {
            if (argc != 8)
            {
                return SHELL_INVALID_PARAM;
            }

            ser_no_type serno;

            memset((uint8_t *)&serno, 0x00, sizeof(ser_no_type));

#if 1 /* bccho, 2024-05-17 */
            extern uint8_t SYS_TITLE_server[SYS_TITLE_LEN];
            uint32_t ds_serial_no_32;

            ds_serial_no_32 = SYS_TITLE_server[5];
            ds_serial_no_32 <<= 8;
            ds_serial_no_32 += SYS_TITLE_server[6];
            ds_serial_no_32 <<= 8;
            ds_serial_no_32 += SYS_TITLE_server[7];

            serno.ser[0] = ds_serial_no_32 / 1000000;
            ds_serial_no_32 %= 1000000;
            serno.ser[1] = ds_serial_no_32 / 100000;
            ds_serial_no_32 %= 100000;
            serno.ser[2] = ds_serial_no_32 / 10000;
            ds_serial_no_32 %= 10000;
            serno.ser[3] = ds_serial_no_32 / 1000;
            ds_serial_no_32 %= 1000;
            serno.ser[4] = ds_serial_no_32 / 100;
            ds_serial_no_32 %= 100;
            serno.ser[5] = ds_serial_no_32 / 10;
            ds_serial_no_32 %= 10;
            serno.ser[6] = ds_serial_no_32;
#else
            memcpy((uint8_t *)&serno, (uint8_t *)&cli_buff[4], SERIAL_NO_SIZE);
#endif

            uint8_t byte_to_ascii(unsigned char a);
            for (int idx = 0; idx < SERIAL_NO_SIZE; idx++)
            {
                serno.ser[idx] = byte_to_ascii(serno.ser[idx]);
            }

            set_cust_id(&serno);

            DPRINT_HEX(DBG_TRACE, "METER_SERIAL", serno.ser, SERIAL_NO_SIZE,
                       DUMP_ALWAYS);

            /* software information */
            dsm_sys_fwinfo_initial_set(true);  // external flash info

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], /*"devid"*/ "ldn"))
        {
            if (argc != 10)
            {
                return SHELL_INVALID_PARAM;
            }

            /*
                               y1 y2 m1 m2 d1 d2 A  V1 V2
                prod set devid 32 30 30 39 30 38 41 33 30 --> "200908" 'A' "30"
            */
            /* Ref: whm.c, logical_device_name_r_kepco[], 제조 일자 +
             * 제조관리번호 + 규격 버전, (ex) "220101A30" */
            /* 통신 규격 - 2.3.2 논리적 장치명의 구조 (LDN: Logical Device Name)
             * 및 3.4.2.3.1 COSEM 계기 식별자 참조 */
            device_id_type dev;

            /* 제조사 고유코드 */
            dev.devid[0] = FLAG_ID1;
            dev.devid[1] = FLAG_ID2;
            dev.devid[2] = FLAG_ID3;

            dev.devid[3] = ' ';

            /* 제조 일자 */
            str_to_hex_uint8(argv[1], (uint8_t *)&dev.devid[4]);
            str_to_hex_uint8(argv[2], (uint8_t *)&dev.devid[5]);
            str_to_hex_uint8(argv[3], (uint8_t *)&dev.devid[6]);
            str_to_hex_uint8(argv[4], (uint8_t *)&dev.devid[7]);
            str_to_hex_uint8(argv[5], (uint8_t *)&dev.devid[8]);
            str_to_hex_uint8(argv[6], (uint8_t *)&dev.devid[9]);

            /* 제조 관리번호 */
            // dev.devid[10] = atoi(argv[7]);
            str_to_hex_uint8(argv[7], (uint8_t *)&dev.devid[10]);

            /* 향후 사용 */
            dev.devid[11] = ' ';  // reserved
            dev.devid[12] = ' ';  // reserved

            /* LD(Logical Device) 번호 : 장치 관리용 = 1, 한전 관리용 = 2 */
            dev.devid[13] = '2';  // 0x31: DEVICE_ID, 0x32: DEVICE_ID_KEPCO

            /* 규격 버전 */  // 3.X 이상 : 보안 계기, 2.X : 비 보안 계기
            str_to_hex_uint8(argv[8], (uint8_t *)&dev.devid[14]);
            str_to_hex_uint8(argv[9], (uint8_t *)&dev.devid[15]);

            nv_write(I_DEVICE_ID_KEPCO, (uint8_t *)&dev);  // 한전 관리용
            DPRINT_HEX(DBG_TRACE, "KEPCO_MGMT", dev.devid, DEVICE_ID_SIZE,
                       DUMP_ALWAYS);

            dev.devid[13] = '1';
            nv_write(I_DEVICE_ID, (uint8_t *)&dev);  // 장치 관리용
            DPRINT_HEX(DBG_TRACE, "DEVICE_MGMT", dev.devid, DEVICE_ID_SIZE,
                       DUMP_ALWAYS);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "time"))
        {
            /* prod set time 22 08 30 20 10 59 */

            date_time_type dt;

            if (argc != 7)
            {
                return SHELL_INVALID_PARAM;
            }

            dt.year = atoi(argv[1]);   // YY
            dt.month = atoi(argv[2]);  // MM
            dt.date = atoi(argv[3]);   // DD
            dt.hour = atoi(argv[4]);   // hh
            dt.min = atoi(argv[5]);    // mm
            dt.sec = atoi(argv[6]);    // ss

            DPRINTF(DBG_TRACE, "SET TIME: 20%02d.%02d.%02d %02d:%02d:%02d\r\n",
                    dt.year, dt.month, dt.date, dt.hour, dt.min, dt.sec);

            write_rtc(&dt);
            cur_rtc_update();

            DPRINTF(DBG_TRACE, "CURR TIME: 20%02d.%02d.%02d %02d:%02d:%02d\r\n",
                    cur_rtc.year, cur_rtc.month, cur_rtc.date, cur_rtc.hour,
                    cur_rtc.min, cur_rtc.sec);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "fwinfo"))
        {
            if (argc != 3)
            {
                return SHELL_INVALID_PARAM;
            }
            uint32_t fw_size;
            uint16_t fw_crc;
            fw_size = strtoul(argv[1], 0, 16);
            fw_crc = strtoul(argv[2], 0, 16);
            DPRINTF(DBG_TRACE, "Size : 0x%08lX, CRC : 0x%04X\r\n", fw_size,
                    fw_crc);
            uint8_t temp[/*Page_Offset*/ 64] = {0};
            //				CMD_READ(SFLASH_SYS_INFO_ADDR + Sector_Offset, temp,
            // 6);
            //// size, crc
            CMD_SE(SFLASH_SYS_INFO_ADDR + Sector_Offset);
            *((uint32_t *)&temp[0]) = fw_size;
            *((uint16_t *)&temp[4]) = fw_crc;
            dsm_imgtrfr_fwinfo_read(&temp[6], FWINFO_CUR_SYS);
            CMD_PP(SFLASH_SYS_INFO_ADDR + Sector_Offset, temp,
                   sizeof(ST_FW_INFO) + 6);
            DPRINT_HEX(DBG_TRACE, "FW_INFO_INIT", temp, sizeof(ST_FW_INFO) + 6,
                       DUMP_ALWAYS);

            CMD_READ(SFLASH_SYS_INFO_ADDR, temp, 2);
            DPRINT_HEX(DBG_TRACE, "FW_EXE", temp, 2, DUMP_ALWAYS);
            CMD_SE(SFLASH_SYS_INFO_ADDR);
            temp[0] = 0;  // fw init
            CMD_PP(SFLASH_SYS_INFO_ADDR, temp, 2);
            DPRINT_HEX(DBG_TRACE, "FW_EXE_INIT", temp, 2, DUMP_ALWAYS);
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
        // TODO: (WD) Release Information
        else if (!strcmp(argv[0], "sysinfo"))
        {
            ST_FW_INFO fwinfo = {0};
            uint8_t *rel_name = (uint8_t *)argv[1];
            uint8_t i, len;

            if (argc != 2)
            {
                return SHELL_INVALID_PARAM;
            }
            len = strlen((const char *)rel_name);
            if (len < 8 || len > 8)
            {
                return SHELL_INVALID_PARAM;
            }

            dsm_imgtrfr_fwinfo_read((uint8_t *)&fwinfo, FWINFO_CUR_SYS);

            fwinfo.version[4] = rel_name[0];  // SOFTWARE_VERSION_H;
            fwinfo.version[5] = rel_name[1];  // SOFTWARE_VERSION_L;

            for (i = 0; i < 6; i++)
            {
                if (rel_name[i] < '0' || rel_name[i] > '9')
                {
                    return SHELL_INVALID_PARAM;
                }
                fwinfo.date_time[i] = rel_name[i + 2];  // YYMMDD
            }
            dsm_imgtrfr_fwinfo_write((uint8_t *)&fwinfo, FWINFO_CUR_SYS);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "mtpinfo"))
        {
            ST_FW_INFO fwinfo = {0};
            uint8_t *rel_name = (uint8_t *)argv[1];
            uint8_t i, len;

            if (argc != 2)
            {
                return SHELL_INVALID_PARAM;
            }
            len = strlen((const char *)rel_name);
            if (len < 8 || len > 8)
            {
                return SHELL_INVALID_PARAM;
            }

            dsm_imgtrfr_fwinfo_read((uint8_t *)&fwinfo, FWINFO_CUR_METER);

            fwinfo.version[4] = rel_name[0];  // SOFTWARE_VERSION_H;
            fwinfo.version[5] = rel_name[1];  // SOFTWARE_VERSION_L;

            for (i = 0; i < 6; i++)
            {
                if (rel_name[i] < '0' || rel_name[i] > '9')
                {
                    return SHELL_INVALID_PARAM;
                }
                fwinfo.date_time[i] = rel_name[i + 2];  // YYMMDD
            }
            dsm_imgtrfr_fwinfo_write((uint8_t *)&fwinfo, FWINFO_CUR_METER);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "temp_reset"))
        {
            if (argc != 1)
            {
                return SHELL_INVALID_PARAM;
            }

            float temp;
            int32_t exponent, mantissa;

            temp = get_inst_temp();
            exponent = temp;
            mantissa = (temp - exponent) * 1000;
            if (mantissa < 0)
            {
                mantissa = (~mantissa) + 1;
            }
            DPRINTF(DBG_TRACE, "Before Temp: %d.%03d\r\n", exponent, mantissa);

            temp_adj_data_type temp_adj = {0};
            extern float adj_currtemp;

            temp_adj.T_adj_temp = adj_currtemp = 0;
            nv_write(I_ADJ_TEMP_DATA, (uint8_t *)&temp_adj);

            temp = get_inst_temp();
            exponent = temp;
            mantissa = (temp - exponent) * 1000;
            if (mantissa < 0)
            {
                mantissa = (~mantissa) + 1;
            }
            DPRINTF(DBG_TRACE, "After Temp: %d.%03d\r\n", exponent, mantissa);

            return SHELL_OK;
        }
        else if (!strcmp(argv[0], "temp"))
        {
            if (argc != 2)
            {
                return SHELL_INVALID_PARAM;
            }

            float temp, correction;
            int32_t exponent, mantissa;

            temp = get_inst_temp();
            exponent = temp;
            mantissa = (temp - exponent) * 1000;
            if (mantissa < 0)
            {
                mantissa = (~mantissa) + 1;
            }
            DPRINTF(DBG_TRACE, "Before Temp: %d.%03d\r\n", exponent, mantissa);

            temp_adj_data_type temp_adj = {0};
            extern float adj_currtemp;

            correction = strtof(argv[1], 0);
            /* current temp */
            correction = correction - (temp - adj_currtemp);
            // DPRINTF(DBG_TRACE, "Diff: %d\r\n", (int32_t)(correction*1000));
            temp_adj.T_adj_temp = adj_currtemp = correction;
            nv_write(I_ADJ_TEMP_DATA, (uint8_t *)&temp_adj);

            temp = get_inst_temp();
            exponent = temp;
            mantissa = (temp - exponent) * 1000;
            if (mantissa < 0)
            {
                mantissa = (~mantissa) + 1;
            }
            DPRINTF(DBG_TRACE, "After Temp: %d.%03d\r\n", exponent, mantissa);

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
                cal_begin();
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_CAL_ST);
                dsm_mtp_fsm_send();

                return SHELL_OK;
            }
            else if (!strcmp(argv[0], "parm_sagswell"))
            {
                dsm_mtp_set_op_mode(MTP_OP_NORMAL);
                dsm_mtp_set_fsm(MTP_FSM_PARM_SET);
                dsm_mtp_fsm_send();

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