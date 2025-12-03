#if !defined(__AMG_MIF_PRTL_H__)
#define __AMG_MIF_PRTL_H__
/*
******************************************************************************
* 	FRAME FORMAT
******************************************************************************
-----------------------------------------------------------------------
| ST(1byte)	| LEN(2byte) | CMD(1byte) | BODY | CRC(2byte) | SP(1byte) |
-----------------------------------------------------------------------

    ST	: Start of Frame (0xF7)
    LEN	: CMD(1) + BODY + CRC(2) + SP(1)
          (ST 및 LEN 필드 제외, LEN 필드가 4인경우 BODY가 없는 경우임)
    CMD	: 시스템부와 계기부간 요청(Get Request, Set Request, Action Request)
          응답, Ack, 푸시(Push)의 내용을 정의한다. --> 2. CMD 쉬트 참조
    BODY : CMD에 대응하는 contents
    CRC	 : legacy 계기에서 사용되었던  CRC를 사용한다. CRC 구간 ( LEN CMD BODY)
    SP	 : Stop of Frame (0xFE)
******************************************************************************
*/

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/
#define MIF_MAX_PACKET_NUM 256
#define MIF_MIN_PACKET_NUM 7  // ST(1) LEN(2) CMD(1) CRC(2) SP(1)

#define MIF_ST 0xF7  // start of frame
#define MIF_SP 0xFE  // stop of frame

typedef enum
{
    MIF_TX_CMD,
    MIF_RX_RSP, /* 사용안함 */
    MIF_TX_RSP,
    MIF_TX_END /* 사용안함 */
} EN_MIF_TX_TYPE;

typedef enum
{
    MIF_FSM_START,
    MIF_FSM_LEN_1,
    MIF_FSM_LEN_2,
    MIF_FSM_DATA,
    MIF_FSM_CRC,
    MIF_FSM_END
} EN_MIF_FSM_STATE;

typedef enum
{
    MIF__FRAME_NO_ERR = 0,
    MIF__FRAME_RCV_CONTINUE,
    MIF__FRAME_HEAD_FLAG_ERR,
    MIF__FRAME_TAIL_FLAG_ERR,
    MIF__FRAME_TYPE_ERROR,
    MIF__FRAME_LENGTH_ERROR,
    MIF__FRAME_FRAGMENT_ERROR,
    MIF__FRAME_CRC_ERR,
    MIF__FRAME_GET_RESPONSE_ERR,
    MIF__FRAME_SET_RESPONSE_ERR,
    MIF__FRAME_NORMAL_RESPONSE_ERR,

    MIFF_FRAME_MAX_ERR
} EN_FRM_RESULT_TYPE;

typedef enum
{
    MIF_CMD_GET_REQ = 0x00,
    MIF_CMD_SET_REQ = 0x10,
    MIF_CMD_ACT_REQ = 0x20,
    MIF_CMD_PUSH = 0xB0,
    MIF_CMD_GET_RSP = 0x80,
    MIF_CMD_SET_RSP = 0x90,
    MIF_CMD_ACT_RSP = 0xA0,
    MIF_CMD_PUSH_ACK = 0x70,
    MIF_CMD_PUSH_NACK = 0x71,
    MIF_CMD_SET_ACK = 0xD0,
    MIF_CMD_SET_NACK = 0xD1,
    MIF_CMD_ACT_ACK = 0xE0,
    MIF_CMD_ACT_NACK = 0xE1,
    MIF_CMD_MAX
} EN_MIF_CMD_T;

typedef enum
{
    MIF_BODY_NONE = 0x00,
    MIF_BODY_CAL_DATA = 0x01,
    MIF_BODY_CAL_START = 0x02,
    MIF_BODY_PUSH_MT_DATA = 0x03,
    MIF_BODY_FIRMVER_DATA = 0x04,
    MIF_BODY_SAGSWELL_DATA = 0x05,
    MIF_BODY_SAGSWELL_TIME = 0x06,
    MIF_BODY_SETUP_PARM = 0x07,
    MIF_BODY_FW_DATA = 0x08,
    MIF_BODY_BAUDRATE = 0x09,
    MIF_BODY_EOI_PULSE = 0x0a,
#ifdef MTP_ZCD_ON_OFF
    MIF_BODY_ZCD_PULSE = 0x0b,
    MIF_BODY_ZCD_OFF = 0x0c,
#endif
    MIF_BODY_MAX
} EN_MIF_BODY_T;

#define MIF_GetReq_CalData 0x01
#define MIF_GetRsp_CalData 0x81
#define MIF_SetReq_CalData 0x11
#define MIF_SetRsp_CalData 0x91
#define MIF_ActReq_CalStart 0x22
#define MIF_ActRsp_CalStart 0xA2
#define MIF_Push_MeterData 0xB3
#define MIF_GetReq_MeterData 0x03
#define MIF_GetRsp_MeterData 0x83
#define MIF_SetReq_SagSwell 0x15
#define MIF_SetRsp_SagSwell 0x95
#define MIF_ActReq_SagSwellSpTime 0x26
#define MIF_ActRsp_SagSwellSpTime 0xA6
#define MIF_GetReq_MtSetupParm 0x07
#define MIF_GetRsp_MtSetupDataParm 0x87
#define MIF_SetReq_MtSetupDataParm 0x17
#define MIF_SetARsp_MtSetupDataParm 0x97
#define MIF_ActReq_EoiPulse 0x2a

#define MIF_GetReq_SagSwell_Parm 0x05
#define MIF_GetRsp_SagSwell_Parm 0x85

#define MIF_GetReq_FirmVer 0x04
#define MIF_GetRsp_FirmVer 0x84

#define MIF_SetReq_BaudRate 0x19
#define MIF_SetRsp_BaudRate 0x99
#define MIF_SetAck 0xD0
#define MIF_SetNack 0xD1
#define MIF_ActAck 0xE0
#define MIF_ActNack 0xE1
#define MIF_PushAck 0x70
#define MIF_PushNack 0x71

#define MIF_SetReq_FwData 0x18
#define MIF_GetReq_FwData 0x08

#define MIF_GetRsp_FwData 0x88
#define MIF_SetRsp_FwData 0x98

#define MIF_LEN_POS 1
#define MIF_CMD_POS 3
#define MIF_BODY_POS 4

#define MIF_ST_FIELD_SIZE 1
#define MIF_LEN_FIELD_SIZE 2
#define MIF_CMD_FIELD_SIZE 1
#define MIF_CRC_FIELD_SIZE 2
#define MIF_SP_FIELD_SIZE 1
#define MIF_LEN_ADD_SIZE \
    (MIF_CMD_FIELD_SIZE + MIF_CRC_FIELD_SIZE + MIF_SP_FIELD_SIZE)
#define MIF_EXTRA_SIZE                                             \
    (MIF_ST_FIELD_SIZE + MIF_LEN_FIELD_SIZE + MIF_CMD_FIELD_SIZE + \
     MIF_CRC_FIELD_SIZE + MIF_SP_FIELD_SIZE)

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
typedef struct _cal_data_t
{
    uint32_t r_current_gain;
    uint32_t r_voltage_gain;
    uint32_t r_phase_gain;
    uint32_t s_current_gain;
    uint32_t s_voltage_gain;
    uint32_t s_phase_gain;
    uint32_t t_current_gain;
    uint32_t t_voltage_gain;
    uint32_t t_phase_gain;
    uint8_t cal_ok;
} POST_PACKED ST_MIF_CAL_DATA;

typedef struct _cal_data_arr_t
{
    ST_MIF_CAL_DATA cal_data[2];
} __PACKED ST_MIF_CAL_DATA_ARR;

typedef struct _cal_start_t
{
    uint8_t ref_voltage[4];
    uint8_t ref_current[4];
    uint8_t ref_phase[4];
    uint8_t process_time;
    uint16_t act_const;
    uint16_t react_const;
    uint16_t app_const;
} POST_PACKED ST_MIF_CAL_START;

typedef struct _meter_parm_t
{
    uint8_t cut_voltage_thr[4];
    uint8_t start_current_thr[4];
    uint8_t direct_reverse;
    uint8_t reactive_select;
    uint8_t meter_method;
    uint8_t pulse_select;
} POST_PACKED ST_MIF_METER_PARM;

typedef struct _sagswell_setup_t
{
    uint8_t pf_level[4];
    uint16_t pf_continue_time;
    uint8_t sag_level[4];
    uint8_t sag_time;
    uint8_t swell_level[4];
    uint8_t swell_time;
} POST_PACKED ST_MIF_SAGSWELL_SETUP;

typedef struct _sagswell_stop_t
{
    uint32_t tmp_time;
} POST_PACKED ST_MIF_SAGSWELL_STOP;

/* sjan 20200911 */
typedef struct _zbm_fw_data_t
{
    uint8_t tx_data[8];
} POST_PACKED ST_MIF_ZBM_FW_DATA;

typedef struct _baud_rate_t
{
    uint32_t rate;
} POST_PACKED ST_MIF_BAUD_RATE;

typedef struct _accum_energy_t
{
    uint8_t DeliAct[4];
    uint8_t DLagReact[4];
    uint8_t DLeadReact[4];
    // uint8_t DeliApp[4];
    uint8_t ReceiAct[4];
    uint8_t RLagReact[4];
    uint8_t RLeadReact[4];
    // uint8_t ReceiApp[4];
} POST_PACKED ST_MIF_ACCUM_EG;

typedef struct _mif_accum_rst_energy_t
{
    uint8_t DeliAct[4];
    uint8_t DLagReact[4];
    uint8_t DLeadReact[4];
    uint8_t DeliApp[4];
    uint8_t ReceiAct[4];
    uint8_t RLagReact[4];
    uint8_t RLeadReact[4];
    uint8_t ReceiApp[4];
} POST_PACKED ST_MIF_ACCUM_RST_EG;

typedef struct _metering_data_t
{
    uint8_t r_voltage[4];
    uint8_t r_current[4];
    uint8_t r_phase[4];
    uint8_t r_act[4];
    uint8_t r_react[4];
    uint8_t r_vol_thd[4];

    uint8_t s_voltage[4];
    uint8_t s_current[4];
    uint8_t s_phase[4];
    uint8_t s_act[4];
    uint8_t s_react[4];
    uint8_t s_vol_thd[4];

    uint8_t t_voltage[4];
    uint8_t t_current[4];
    uint8_t t_phase[4];
    uint8_t t_act[4];
    uint8_t t_react[4];
    uint8_t t_vol_thd[4];

    uint8_t freq[4];

    uint8_t rs_phase[4];
    uint8_t rt_phase[4];

    ST_MIF_ACCUM_RST_EG rst_accum;
    ST_MIF_ACCUM_EG r_accum;
    ST_MIF_ACCUM_EG s_accum;
    ST_MIF_ACCUM_EG t_accum;

    uint8_t sag_count;
    uint8_t swell_count;
    uint8_t temp[4];  // float

} POST_PACKED ST_MIF_METERING_DATA;

typedef struct _mif_pkt_
{
    uint8_t fsm;
    uint16_t len;
    uint16_t cnt;
    uint8_t seg;
    uint8_t pkt[MIF_MAX_PACKET_NUM];

} ST_MIF_RX_PKT;

typedef struct _mif_com_pkt_
{
    uint16_t len;
    uint8_t data[MIF_MAX_PACKET_NUM];
} ST_MIF_COM_PKT;

typedef struct _mif_frame_t_
{
    uint8_t buff[MIF_MAX_PACKET_NUM];
    uint16_t length;
    uint16_t idx;
} ST_MIF_FRAME_BUFFER;

/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/
void dsm_mif_init(void);
void dsm_mif_rx_pkt_init(void);
char* dsm_mif_cmd_string(uint32_t cmd);
void dsm_mif_process(EN_MIF_TX_TYPE type, ST_MIF_COM_PKT* p_com_pkt);
uint32_t dsm_mif_rx_parser(uint8_t* buff, uint32_t size, uint32_t* idx,
                           ST_MIF_COM_PKT* p_com_pkt);
void dsm_mif_get_req_cal_data(void);
void dsm_mif_getreq_firmware_ver_data(void);

void dsm_mif_getreq_cal_data(void);
void dsm_mif_getrsp_cal_data(uint8_t* pdata, uint16_t length);
void dsm_mif_setreq_cal_data(uint8_t* pdata, uint16_t length);
void dsm_mif_setrsp_cal_data(uint8_t* pdata, uint16_t length);
void dsm_mif_actreq_cal_start(uint8_t* pdata, uint16_t length);
void dsm_mif_actrsp_cal_start(uint8_t* pdata, uint16_t length);
void dsm_mif_push_data(uint8_t* pdata, uint16_t length);
void dsm_mif_push_ack(void);
void dsm_mif_push_nack(void);
void dsm_mif_set_ack(void);
void dsm_mif_set_nack(void);
void dsm_mif_act_ack(void);
void dsm_mif_act_nack(void);
void dsm_mif_getreq_pushdata(void);
void dsm_mif_getrsp_pushdata(uint8_t* pdata, uint16_t length);
void dsm_mif_setreq_sagswell_data(uint8_t* pdata, uint16_t length);
void dsm_mif_setrsp_sagswell_data(uint8_t* pdata, uint16_t length);
void dsm_mif_actreq_sagswell_time(uint8_t* pdata, uint16_t length);
void dsm_mif_actrsp_sagswell_time(uint8_t* pdata, uint16_t length);
void dsm_mif_getreq_meter_setup_parm(void);
void dsm_mif_getrsp_meter_setup_parm(uint8_t* pdata, uint16_t length);
void dsm_mif_setreq_meter_setup_parm(uint8_t* pdata, uint16_t length);
void dsm_mif_setrsp_meter_setup_parm(uint8_t* pdata, uint16_t length);
void dsm_mif_setreq_baudrate(uint8_t* pdata, uint16_t length);
void dsm_mif_setrsp_baudrate(uint8_t* pdata, uint16_t length);
void dsm_mif_zcd_on(void);
void dsm_mif_zcd_off(void);
void dsm_mif_pwrfail_port_init(void);
void dsm_mif_pwrfail_port_deinit(void);
void dsm_mif_actreq_eoipulse(void);

#endif /* __AMG_MIF_PRTL_H__ */
