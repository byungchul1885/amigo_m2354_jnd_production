
#ifndef GET_REQ_H
#define GET_REQ_H 1

#define BLOCK_LP_UNIT_UNI 14  // 14.7.30 LP records to send in one time
#define BLOCK_LP_UNIT_BOTH 14

#define BLOCK_LPAVG_UNIT 3  // LPavg records to send in one time

#define BLOCK_LPRT_UNIT_1PHS 2  // LPrt records to send in one time
#define BLOCK_LPRT_UNIT_3PHS 3  // LPrt records to send in one time

#define TIME_ZONE_TO_GMT ((int16_t) - 9 * 60)

#define UNSP_TIME_SIZE 14
#define ZERO_TIME_SIZE 14

#define PUSH_SETUP_ERRCODE_CAPOBJ_SIZE (2 + 2 * 18)
#define PUSH_SETUP_LAST_LP_CAPOBJ_SIZE (2 + 2 * 18)
extern uint8_t
    PUSH_SETUP_ERRCODE_capture_objects[PUSH_SETUP_ERRCODE_CAPOBJ_SIZE];
extern uint8_t
    PUSH_SETUP_lastLP_capture_objects[PUSH_SETUP_LAST_LP_CAPOBJ_SIZE];

typedef struct
{
    date_time_type dt;
    uint8_t clksts;
    rate_type s_cur_rate;
    uint32_t lpevt;
} whm_info_cap_type;

// whm_info capture 정의
#define comm_dt whm_info_cap.dt
#define cap_clk_sts whm_info_cap.clksts
#define cap_cur_rate whm_info_cap.s_cur_rate
#define cap_LP_event whm_info_cap.lpevt

typedef struct
{
    /*
    통신 규격 3.4.2.8.2 디스플레이
        display_item ::= array display_item_element
        {
            class_id: long-unsigned,
            logical_name: octet-string, [6byte]
            attribute_number: unsigned,
            display_time: unsigned [second]
            (최대 40개 이하 항목에 대한 시간 지정은 동일하게 지정)
        }
        ※ 현월, 전월, 전전월의 전력(량), 역률에 대하여 logical_name의 마지막
    바이트는 각각 255, 101, 102, 103 으로 지정
    */
    uint16_t classid;
    uint8_t obis[6];
    uint8_t attid;
    uint8_t dur;
} dsp_item_info_type;

extern whm_info_cap_type whm_info_cap;
extern float lp_intv_pf_cap[numDirChs];
extern bool lpinfo_is_caped;
extern uint8_t recsize_for_lp;
extern uint16_t buffsize_for_lp;

void approc_get_req(int idx);
void lp_mtdir_init(void);
bool lp_mtdir_is_chged(void);
bool lp_mtdir_is_chged_and_blocked(void);
void ob_device_id(void);

uint16_t get_mr_idx(uint8_t grp_f);
uint16_t get_mr_idx_nperiod(uint8_t grp_f);
uint16_t get_mr_idx_season(uint8_t grp_f);

bool Is_push_PWR_OFF_act_mask(void);
void PWR_OFF_trigger(void);
bool Is_push_Work_BlackOut_act_mask(void);
void Work_BlackOut_trigger(void);
bool Is_push_ENERGY_REMAINING_act_mask(void);
void ENERGY_REMAINING_trigger(void);
void error_code_event_clear_for_prepay(void);
bool is_WMStatus_prepay_remaining_0(void);
bool Is_push_SECURITY_ERR_act_mask(void);
void SECURITY_ERR_trigger(void);
bool Is_push_MAGNET_SENS_act_mask(void);
void MAGNET_SENS_trigger(void);
bool Is_push_COVER_OPEN_act_mask(void);
void tCOVER_OPEN_trigger(void);
void mCOVER_OPEN_trigger(void);
bool Is_push_NO_BATTERY_act_mask(void);
void NO_BATTERY_trigger(void);

bool Is_push_OVERCURR_a_act_mask(void);
bool Is_push_OVERCURR_b_act_mask(void);
bool Is_push_OVERCURR_c_act_mask(void);
int OVERCURR_trigger(int i);
bool Is_push_WRONGCONN_act_mask(void);
void WRONGCONN_trigger(void);
bool Is_push_temp_act_mask(void);
void TEMPOVER_trigger(void);

bool Is_push_NO_PHASE_act_mask(int i);
void NO_PHASE_set_trigger(int i);
void NO_PHASE_rcv_trigger(int i);

bool Is_push_LATCH_ERR_A_act_mask(void);
bool Is_push_LATCH_ERR_B_act_mask(void);
bool Is_push_LATCH_ERR_C_act_mask(void);
void LATCH_ERR_A_trigger(void);
void LATCH_ERR_B_trigger(void);
void LATCH_ERR_C_trigger(void);
bool Is_push_SELF_ERR_act_mask(void);
void SELF_ERR_trigger(void);
bool Is_push_NEUT_WRONGCONN_act_mask(void);
void NEUT_WRONGCONN_trigger(void);
bool Is_push_NO_PHASE_A_act_mask(void);
bool Is_push_NO_PHASE_B_act_mask(void);
bool Is_push_NO_PHASE_C_act_mask(void);
void NO_PHASE_A_trigger(void);
void NO_PHASE_B_trigger(void);
void NO_PHASE_C_trigger(void);
bool Is_push_OVERVOT_A_act_mask(void);
bool Is_push_OVERVOT_B_act_mask(void);
bool Is_push_OVERVOT_C_act_mask(void);
void OVERVOT_A_trigger(void);
void OVERVOT_B_trigger(void);
void OVERVOT_C_trigger(void);
bool Is_push_LOWVOT_A_act_mask(void);
bool Is_push_LOWVOT_B_act_mask(void);
bool Is_push_LOWVOT_C_act_mask(void);

void LOWVOT_A_trigger(void);
void LOWVOT_B_trigger(void);
void LOWVOT_C_trigger(void);
bool Is_push_SAG_act_mask(void);
void SAG_trigger(void);
bool Is_push_SWELL_act_mask(void);
void SWELL_trigger(void);

void ob_err_code_1(void);
void ob_err_code_2(void);
void ob_err_code_3(void);
void ob_err_code_4(void);
void LP_last_record_to_pPdu(uint8_t* recbuff);
void fill_clock_obj_except_tag(date_time_type* dt);
void dsm_update_forReport(void);
uint8_t* dsm_get_progname_forReport(bool curprog);
uint8_t* dsm_get_meterid_forReport(void);

extern void dsm_progname_update_forReport(void);

#endif
