#ifndef AMG_PREDEFINES_H_
#define AMG_PREDEFINES_H_

#define M2354_NEW_HW /* bccho, 2023-11-07 */

/* CAN은 삼상 변성기부에 한함 */
// #define M2354_CAN    /* bccho, 2023-11-28 */

#define SPI_FLASH_TESTx     /* bccho, 2023-11-16 */
#define BATTERY_ACTIVE_OPx  /* bccho, 2023-11-26 */
#define RTC_ALARM_INT_TESTx /* bccho, 2023-11-24 */
#define MENU_KEY_RTC_ALARMx /* bccho, 2023-11-26 */
#define STOCK_OP_NO_WAITx   /* bccho, 2023-11-26 */

#define MTP_ZCD_ON_OFF /* bccho, 2025-10-15 */
#define MAIN_TASK_NO_DELAY

#define ADD_DC_LOW_PIN /* bccho, 2024-09-11 */

#if 1 /* -------------bccho, ADD, 2023-07-20 */
#include "NuMicro.h"
#include "partition_M2354.h"
#include "nsclib.h"
#include "hls.h"
#include "hls_error.h"

#define __packed __attribute__((__packed__))
#define __unused __attribute__((__unused__))
#define UNUSED(x) (void)(x)

#define HAL_OK 0x00
#define HAL_ERROR 0x01
#endif /* -----------------------------bccho */

#define FreeRTOS
#define SW_VER 115

#define DISABLE_TASK_BLOCK_RESET
#define FW_INFO_APP_SIGNATURE 0xBEEFCAFE /* 0xDEADC0DE */

//==============================================================================
#define MT_1P2W_60A 0
#define MT_1P2W_120A 1
#define MT_1P2W_TRANS 2
#define MT_3P4W_SINGLE 3
#define MT_3P4W_TRANS 4

#define METER_TYPE MT_1P2W_60A
// #define METER_TYPE MT_1P2W_120A
// #define METER_TYPE MT_3P4W_SINGLE

#define HARDWARE_VERSION 0x32

#define SOFTWARE_SYSTEM_PART 0x30
#define SOFTWARE_INMODEM_PART 0x31
#define SOFTWARE_EXMODEM_PART 0x32
#define SOFTWARE_METER_PART 0x33

#define SOFTWARE_VERSION_H '1'
#define SOFTWARE_VERSION_L '8'

#define SOFTWARE_DATE_YYH '2'
#define SOFTWARE_DATE_YYL '5'
#define SOFTWARE_DATE_MMH '1'
#define SOFTWARE_DATE_MML '1'
#define SOFTWARE_DATE_DDH '1'
#define SOFTWARE_DATE_DDL '3'

#define OUT_OF_PERIOD_DEMAND_RESET

#define USING_ZON_P3SX
#define SY7M213H

#define DISPLAY_MODE_TOU_PROG_ID
#define SUPPORT_INFORMATION_FIELD_LENGTH  // DLMS T/RX Window Size 관련 수정
// #define DO_NOT_ERASE_SW_INFO // 공장 초기화 등 데이터 초기화가 수행될 때,
// 계기의 소프트웨어 정보가 지워지는 현상을 방지함 // 루틴 수정으로 사용안함.
#define ANTI_KEY_PRESSED  // Wake-Up 시, 버튼 키 인식으로 디스플레이 항목 전환이
                          // 일어나는 현상을 방지 (1번이 아닌 2번 항목부터
                          // 표시되는 현상)
#define TOU_HEADER_METER_ID_COMPARE  // TOU Header의 계기 번호를 비교함.

#define SW_UPDATE_IMAGE_TRANSFER_SET_REQ  // 소프트웨어 업데이트 관련 Image
                                          // Transfer 의 Block Size 설정을 지원.
                                          // (Set Req)

//==============================================================================

// #defien TEST_CODE
#ifdef TEST_CODE
// #define STOCK_OP_TEST_CODE_ENABLE // 재고 관리 모드 테스트용 코드 활성화
// #define NON_SECURE_ACCESS /* 보안계기 비보안 액세스 테스트용 */
#endif

#if defined NON_SECURE_ACCESS && !defined NON_SEC_LEGACY_MODE
#define NON_SEC_LEGACY_MODE  // 테스트용 비보안 모드
#endif

//==============================================================================

// CONFIG_RTOS_FREERTOS == y
#define FEATURE_USE_MEMORY_POOL

/*
** Feature - Meter Start =======================================================
*/

/*
** Meter characteristic
*/
#define FEATURE_COMPANY_ID  // 한전 등록 제조사 번호

/*
** HW dependent
*/
#define FEATURE_HW_VERSION4_200826  // hw update 2
#define FEATURE_HW_VERSION3_200529  // hw update 1
// #define FEATURE_HW2_RS485_SELECT_OFF						// 2pin control ->
// 1pin(PA11)
// 로 변경 #define FEATURE_ADC_PA5_PA6									// ADC
// for Vbat, Vdd mon
#define FEATURE_HW_VBAT_MONITOR  // battery monitor pin 으로 vbat 측정
#define FEATURE_EXT_WDT_TOGGLE_IN_ACT_POWER  // active mode 인 경우만 ext_wdt
                                             // toggle - duration 2.5 sec
#define FEATURE_EXT_WDT_TO_8SEC

#define FEATURE_POWER_OPT             // power optimization
#define FEATURE_POWERFAIL_VDDEN_DIFF  // vdd_en 과 power_fail 의 상태가 다른경우
                                      // : 즉 운영부는 main 전원이 공급 (vdd_en
                                      // high), mtp (power_fail) 인 경우
// #define FEATURE_POWERDOWN_EXCEPTION							// power down
// exception 처리
// -> 파워 소스가 vdden_high 50ms 유지, vdden_low 2sec 유지후, vdden_high 상태
// 유지 되는 경우
// #define FEATURE_TP_DEBUG_ENABLE								// tp debug
// enable

/*
** Porting, STM driver, peri
*/
#define FEATURE_INTERNAL_RESET
#define FEATURE_DRIVER_PORT_AT_EXT_SOLUTION  // legacy 부분에 STM driver port ->
                                             // 차후 정리
#define FEATURE_APP_PORT_AT_EXT_SOLUTION     // STM 부분에 legacy 코드 사용시 ->
                                             // 차후 정리
#define FEATURE_IWDT_FREEZE_EN               // first boot 시 iwdt freeze enable
#define FEATURE_WAKEUP_SRC_LIMIT  // wakeup source limit : RTC, MENU key, Vdd
                                  // mon, Vbat mon, Low_power_entry
// #define FEATURE_X_MODEM_USE_INTERNAL_FLASH					// xmodem for
// internal flash and image transfer for fw update
#define FEATURE_X_MODEM_USE_EXT_FLASH  // xmodem for ext.sflash and image
                                       // transfer for fw update  ==> bootloader
                                       // 필히 업데이트
#define FEATURE_X_MODEM_USE_INTERNAL_FLASH_BOOTLOADER  // xmodem for internal
                                                       // flash bootloader
#define FEATURE_TASK_BLOCK_CONTINUE_EXCEPTION  // task block 연속적으로 발생하는
                                               // 경우 exception process
#define FEATURE_STMAPI_GPIO_HAL_APPLY          // gpio init LL -> HAL

/*
** General, security, ACS
*/
#define FEATURE_SEC  // secure meter 개발상 대부분의 내용 묽음  -> 차후 기능별로
                     // 분리 필요..
#define FEATURE_SEC_ACT_OPT_FLAG      // action req시 option flag 적용
#define FEATURE_SEC_SIM_PRINT_ENABLE  // 디버깅시에서 사용
#define FEATURE_ALWAYS_PRINT_OPT      // 항시 dm print opt
#define FEATURE_SIGN_OPT_ENABLE       // signing opt enable
#ifndef NON_SEC_LEGACY_MODE
#define FEATURE_SEC_ACS          // secure meter access process
#define FEATURE_SEC_ACS_DEV_MNT  // secure meter access process -> device
                                 // management, legacy 용 meter 로 사용할 경우
                                 // feature 제거
#ifdef NON_SECURE_ACCESS
#undef FEATURE_SEC_ACS
#undef FEATURE_SEC_ACS_DEV_MNT
#endif
#else  // 비보안 모드 테스트
#warning "[CAUTION] Secure Access is not Enabled"
#endif
#define FEATURE_SEC_TOU_METER_ID  // tou meter id 5 -> 11

/*
** AMR
*/
#define FEATURE_AMR_RT_LP      // secure meter 의 realtime 상별 검침..
#define FEATURE_AMR_LP_STATUS  // LP 상태 정보 변경 ==> 저전압, 과전압 기능 추가
                               // 필요..
#define FEATURE_AMR_INST       // 순시 전압 전류
#define FEATURE_AMR_AVG        // 평균전압 전류
#define FEATURE_SEC_BAUD
#define FEATURE_AMR_CUR_CAPTURE_MODIFY  // assoation 시 관련 변수를 초기화하여
                                        // approc_get_req_capture 를 수행하나,
                                        // AMIGO 는 session 을 유지 하므로 처리
                                        // 안됨.
#define FEATURE_METER_RESET_COMM_TIMER  // meter reset 명령후 disc 수신되지 않는
                                        // 경우 timer 를 이용하여 수행
#define FEATURE_METER_RST_TIME_COMPENSATE   // meter reset 명령후 time 지연되는
                                            // 현상 보정
#define FEATURE_PREPAY_ENABLE               // 선불 기능
#define FEATURE_PREPAY_LOADLIMIT_CANCEL_EN  // 주말 휴일 공휴일 에 부하제한
                                            // 해제를 할지 선택하는 기능
#define FEATURE_PREPAY_WH                   // 선불 기능 단위 ( pulse -> wh )
// #define FEATURE_SEC_SITE_OP									// sec_site 인
// 경우 처리 #define FEATURE_HDLC_RX_RETRY_EXCEPT						//
// client 으로 부터 retry 되는 경우라기 보다는 모뎀에서 duplicate 되어 retry
// 되는 상황임. 기본적으로 처리 불가함.
#define FEATURE_SEC_LP_SIZE  // lp size 가 both 인 경우에서 6240 적용..
// #define FEATURE_TIME_CHG_AMR_CONCEPT						// 1.시간 동기화시
// 3종 이상인 경우만 season 변경되면 검침 수행-> 차후 컨셉 재확인 필요.
#define FEATURE_LOAD_RE_INSERT_CONCEPT  // 통전 컨셉- 규격 반영
// #define FEATURE_IC_VALID_CHECK								// invocation
// counter valid check
#define FEATURE_ENERGY_DM_NON_REPUDIATION  // energy, demand max profile generic
                                           // 시 sign 추가, (부인방지 전자서명
                                           // 기능)
#define FEATURE_FW_VALID_CHECK
#define FEATURE_AMR_RT_LP_SAVE_RAM
#define FEATURE_ASSO_1_LISTEN_ON
#define FEATURE_ASSO_UTILITY_TO_ENABLE
#define FEATURE_ASSO_TO_ENABLE
#define FEATURE_DSP_ON_SUN
#define FEATURE_IMGTRFR_STATE_EN
#define FEATURE_SWRST_BOOT
#define FEATURE_AMR_SAVE_MANUALLY

/*
** FEATURE_COMM_IF
*/
// #define FEATURE_CAN_CNTX
// #define FEATURE_COMM_PLC_SUN_COORD_EN						// 초기에
// iotPLC가 연결될시 sun coord.설정 -> PLC modem이 설정하는 것으로 컨셉 변경 됨.
#define FEATURE_ATCMD_EN_DECRYPT_FIX
// #define FEATURE_DLMS_CLIENT									// DLMS client
// add
#define FEATURE_CM_n_SM_EXTIF  // CM(client mode) 인 경우 EXTif HDLC rx인 경우
                               // SM(server mode) 동작
#define FEATUE_DLMS_CLIENT_HDLC_CONTROL_SEQ_OTHER  // dl sequece control test
// #define FEATUE_DLMS_CLIENT_PGM_RD_TEST						// programe read
// 후 disc 후 다시 asso 후 TOU 전달 #define FEATURE_VIRTUAL_HDLC_ADDR
// // virtual address 생성 및 주소 회피 기능
#define FEATURE_ATCMD_FWUP_EXCEPTION     // 이미지 수신 완료후 atcmd로 모뎀으로
                                         // 이미지 전송시 모뎀 오류 사항에 대한
                                         // exception 처리
#define FEATURE_ATCMD_BAUD_NV_ENABLE     // modem 의 baud rate 을 NV에서 관리
#define FEATURE_ASSO3_REJECT_SNRM_ASSO4  // asso3 session 연결 상태에서 asso4
                                         // SNRM 이 수신되는 경우 처리
#define FEATURE_MODEM_FWUP_DIRECT_SEND   // (WD) F/W Image 저장하지 않고 ByPass
                                         // ??? 기능 검토 필요함

/*
** KTC CERT
*/
/* KTC 계기 인증서를 사용할 수 있도록 KTC 게기 인증서 기능을 활성화 */
// #define FEATURE_KTC_CERT_ADD								// KTC 보안 인증서
// 추가
#ifdef FEATURE_KTC_CERT_ADD
/* KTC Meter 인증서 주입 시 KTC RootCA 인증서를 사용하지 않음 */
// #define FEATURE_KTC_CERT_ADD_NoRootCA						// KTC Root CA
// 인증서 사용하지 않고  동작 가능 하도록 적용 ( 차후 KTC 컨셉에 의해 변경
// 필요.. )
#endif

/*
** CPPcheck
*/
#define FEATURE_CPPcheck  // cpp check

/*
** CODE SIZE OPT
*/
#define FEATURE_COMPILE_SIMU_MODEM

/*
** AMIGO spec change
*/
// #define FEATURE_BAUD_RATE_DEFAULT_AT_RST					// kepco spec 7.20
// 이후
#define FEATURE_METER_ID_FULL        // kepco spec 9.02
#define FEATURE_MAX_DM_ACCUMULATION  // kepco spec 9.02
#define FEATURE_SW_VER_NAME_2_0      // kepco spec 7.20 이후 버전
#define FEATURE_SAGSWELL_TIME_FLOAT  // kepco spec 9.02
// #define FEATURE_OLD_METER_TOU_TRANSFER						// kepco
// spec 9.02
#define FEATURE_OLD_METER_TOU_DEFAULT   // legacy meter tou 1 및 2종 default
                                        // calendar, day_id 변환 처리 적용
#define FEATURE_SW_UPDATE               // kepco spec 7.20 이후 버전
#define FEATURE_PUSH_DISABLE_NV_ACCESS  // push info NV 사용하지 않음.
// #define FEATURE_PUSH_DEFAULT_ENABLE							// push default
// enable 됨.
#define FEATURE_MOVE_KEY_NO_VOLT_SET_OP  // 무전압 검침 모드에서 Move key 누룰시
                                         // NO_VOLT_SET_OP mode 진입및 동작
// #define FEATURE_EXT_MMID_CHECK_ENABLE
#define FEATURE_SPEC_2021_0430_MODIFY
// #define FEATURE_SPEC_2021_0430_MODIFY_RT_LP
#define FEATURE_SPEC_2021_0430_MODIFY_HDLCERR
#define FEATURE_SPEC_2021_0430_INST_PF
#define FEATURE_TOU_INDIVIDUAL_SET_ERR_MSG
#define FEATURE_TOU_FUT_ETC_CONCEPT
#define FEATURE_RECENT_DEMAND_SAVE_CONCEPT
#define FEATURE_MTP_FWVER_MANAGE

/*
** JP
*/
#define FEATURE_JP_AMR
#define FEATURE_JP_AMIGO_LP
#define FEATURE_JP_FREE_DISCONNECTION  // connection state, association state 인
                                       // 경우에도 다른 client 의 disc/snrm 을
                                       // 처리할 수 잇음..
#define FEATURE_JP_SAGSWELL_SIMPLE
#define FEATURE_JP_CAL_LCD_DISP
#define FEATURE_JP_DATA_FULL_MANUAL
#define FEATURE_JP_LCD_PULSE
#define FEATURE_JP_HIVOLT_METER
#define FEATURE_JP_485_PUSH_PROTECT
#define FEATURE_JP_ERRCODE_PUSH
#define FEATURE_JP_AMIGO_KEY
#define FEATURE_JP_SNRM_DISC_ENABLE
#define FEATURE_JP_PUSH_WINDOW_ENABLE
#define FEATURE_JP_PWON_PARA_LOAD_MTP
#define FEATURE_JP_SELF_ERR_DATA_S16
#if 0
#define FEATURE_JP_TCOVER_OPEN_LOGIC_INV  // COVER_OPEN SWITCH LOGIC 변경시
                                          // 적용함.
#ifdef FEATURE_JP_TCOVER_OPEN_LOGIC_INV
#warning "[CAUTION] Cover State is inverted"
#endif
#endif
#define FEATURE_JP_M24C02M  // 2Mbit EEPROM device_address 설정 적용 //
                            // ST-M24M02인데 이름을 잘못쓴듯..
#define FEATURE_JP_NV_SEC
#define FEATURE_JP_INTERN_ERR_SPECIAL_STS
#define FEATURE_JP_DISP_AMIGO_OBIS
#define FEATURE_JP_RTC_ERROR_NO_SET
#define FEATURE_JP_PUSH_ENABLE_TIMING_STS_CONTROL
#define FEATURE_JP_SW_UP_CNT           // SOFTWARE 변경 이력 기록
#define FEATURE_JP_AMIGO_LP_EVENT_SET  // kepco 20.10.21 규격서 , LP 기록 기준
                                       // EVENT 변경
#define FEATURE_JP_MEA_MODE_CHG_SRDR_SET  // METER MODE 변경시 별도의 SR_DR 기준
                                          // 설정값 적용
#define FEATURE_LCD_ERROR_ASCII_DISP_ENABLE  // LCD ERROR 문자 표시기능 활성화
#define FEATURE_JP_TOU_TABLE_201103          // kepco 20.10.21 규격
#define FEATURE_JP_LATCH_CNT_OFF_TO_ON  // latch on 카운터 증가 기준 타이밍(off
                                        // -> on)
// #define FEATURE_JP_MTER_LP_READ_1DAY_COMM_ON				// 기설계기 기능
// 제거
#define FEATURE_JP_COMM_BLINK     // LCD에 통신 인식자 표시(무선/유선 구분)
#define FEATURE_JP_LONG_KEY_CAL   // KEY 길이가 긴 경우에만 CAL 실행
#define FEATURE_JP_EOI_PULSE_OUT  // EOI PULSE OUT ON/OFF 180ms
#define FEATURE_JP_KEY_CONTI_S_T1_5SEC         // 2초 키 SPEED 조금 빠르게
#define FEATURE_JP_IMAGE_TOU_PASSIVE_SET_BODY  // TOU PASSIVE를 BODY 정보에서
                                               // CHECK
#define FEATURE_JP_AMIGO_DEFAULT_ADD           // 추가 설정값들의 디폴트값 지정
#define FEATURE_JP_DM_SUB_INTV_PEROID_NUM_FIXED  // BLOCK INTERVAL =
                                                 // SUB_INTERVAL,  PERIOD_NUM =
                                                 // 1;
#define FEATURE_JP_DM_INTV_SEPERATED_PROG  // 수요전력 연산주기 DM_INTERVAL
                                           // 개별설정 가능.
#define FEATURE_JP_EOI_PULSE_PIN_8         // EOI_1 PORT G6 -> G8로 변경
#define FEATURE_JP_BILLING_PARM_SET        // 검침 파라메타 개별 설정
#define FEATURE_JP_PARTITION_TOU_FUT       // TOU_PROGRAM 부분 예약 기능
#define FEATURE_HOLLIDAY_TOU_ON_OFF_MODE   // 정기 /비정기 공휴일 적용 유무 설정
                                           // 사용
#define FEATURE_JP_EXTENDED_MT_CONFIG  // nv 호환을 위해 별도 mt_config 추가 ->
                                       // 향후 개별 nt parmeter는 mt_config_2에
                                       // 추가 함.
#define FEATURE_JP_RTKIND_WEEK_TABLE_CHECK  // 1종/2종/3종/4종 구분시
                                            // week_table을 먼저 확인.
// #define FEATURE_JP_MLIST_CHECK_TOU_DOWN_LOAD  // TOU 적용시 자신의 METER_ID
// 와 일치할 경우에만TOU 적용
#define FEATURE_JP_ASS3_LN_SUPPORTED_CONFORMANCE
// #define FEATURE_JP_CTT27_TEST_MODE

// HW_TARGET == ES_1
#define FEATURE_JP_KEY_CAL_ENABLE  // H/W Calibration

// HW_TARGET == ES_2
#define FEATURE_JP_NEW_CAL_DISP

#define FEATURE_JP_BIDIR_ANTI_FRAUD_PROTECTION  // 송수전 모드에서는 ANTI FRAUD
                                                // MODE 설정안됨
#define FEATURE_JP_SEASON_CHG_SR_NONSET      // 계절변경시 SR   안함 ( 디폴트 )
#define FEATURE_JP_AUTO_MODE_CHANG_BI_DIR    // 자동 수전 -> 송수전 변환기능
#define FEATURE_JP_DEFAULT_AUTO_MODE_SEL     // 자동 수전 -> 송수전
                                             // 변환(디폴트true)
#define FEATURE_JP_HDLC_SEQ_ERR_NEW_CHK      // hdlc seq. error check 보완
#define FEATURE_JP_RTC_VALID_CHECK_BUG_FIX   // rtc_valid() 함수 bug 수정
#define FEATURE_JP_SW_COVER_PORT_INIT        // cover open s/w port 초기화
#define FEATURE_JP_POWER_OFF_ON_EVENT_CHECK  // POWER 복귀시 처리기능 전체적
                                             // 살려줌.
// #define FEATURE_JP_whm_data_unsave_sag						// 형식승인을
// 위해 기능 제거 2021.01.26
#define FEATURE_JP_TOTAL_LP_CNT_INC_TIMMING_FWD  // 총 LP 갯수 1부터 시작
#define FEATURE_dsp_pwr_fail_CLEAR              // POWER OFF시 LCD에 명확히 표시
#define FEATURE_JP_CONFORMANCE_BLOCK_BUG_2_FIX  // CONFORMANCE_BLOCK_BUG 수정
                                                // 0x04 -> 0x181d
#define FEATURE_JP_AUTO_MODE_BI_DIR_POWER  // 자동송수전 전환 검출값  송전전류
                                           // -> 송전전력
// #define FEATURE_JP_MTP_BIDIR_FIXED  // measuring 부  송수전 방식으로 고정
// 설정
#define FEATURE_JP_4_DAY_TABLE_TOU     // tou default table  규격 변경 20210223
#define FEATURE_JP_circ_mode_freezing  // SHELL CMD 로 LCD 스크롤 고정함.
// #define FEATURE_BATON_POWER_RTN_NON_LP						// BAT ON
// 상태에서 전원복귀시 INTERVAL LP 발생 안함.-> 다른 방법을 사용
#define FEATURE_MAX_DMD_INTERVAL_SKIP_BUG_FIX  // 특수 상황에서 DEMAND_INTV 을
                                               // SKIP하는 과거 버그 수정
#define FEATURE_PARTITION_TOU_CUR_TO_FUT  // TOU_PROGRAM 부분 예약 기능에서 미리
                                          // 현재를 미리버퍼에 copy 안함(예약을
                                          // 버퍼로 이동)
#define FEATURE_PART_FUT_WRITE            // 부분예약에서 일부분 즉시 예약 가능
#define FEATURE_STARTING_CURRENT_DATA_FLOAT  // 잠동 방지 전류설정값 -> float로
                                             // 변경
#define FEATURE_CAL_FLASHDATA_MONITOR    // 계량부 flash 설정정보 직접 monitor
#define FEATURE_MTP_PARAM_RE_CONNECTION  // 계량모드 변경시 계량부 연동 버그
                                         // 수정
#define FEATURE_THD_5_UNDER_TO_0         // thd data 쓰레기 줄이기
#define FEATURE_UNI_DIR_MODE_DEL_POWER_TO_0  // 수전모드에서 송전전력 지우기
// #define FEATURE_RCNT_SEASON_ID_0XFF_FOR_TOU_START			//
// 프로그램변경시 계절id
// -> 0xff (사용안함)
#define FEATURE_JP_DMINTV_CHG_SRDR_SET  // 수요전력 주기변경시 SR DR
#define FEATURE_PWRT_LP_SAVE_DATE_SYNC  // 정전 LP 복전후 소급 적용
// #define FEATURE_FUT_SEASON_CHG_CHK_FROM_1TARRFF_TO_3TARIFF	// tariff 변경시
// season chg 적용
#define FEATURE_POWERFAIL_600MS_DELAY_CHECK  // 600MS 이상인경우에만
                                             // POWER_FAIL로 인식함.
// #define FEATURE_MAGNET_SENSE_LEVEL_DEBUG_PRINT
#define FEATURE_PC_SW_PARA_WRITE
#define FEATURE_PWRUP_THD_STARTING_PROTECTION
#define FEATURE_SUN_RST_CMD_TO_R_SUN_DISP
#define FEATURE_PWFAIL_SAG_MASK
// #define FEATURE_MTP_FWUP_DATA_STARTING_PROTECTION

#define FEATURE_JP_seasonCHG_sr_dr_type_to_SET_SR  // 23.11.09  JP

/*
** MIF & MTP
*/

/* bccho, 2023-09-19, porting, delete */
// #define FEATURE_MTP_CALSET_OFF  // mtp cal/normal mode 에서 cal get 하여 다른
// 경우 cal set 하지 않도록 적용

/*
** ETC
*/
// #define FEATURE_DSP_DEBUG_ENABLE							// LCD debug print
// #define FEATURE_FUTURE_REMOVE_CODE							// secure meter
// 에는 필요 없는 내용
// - 차후 제거
// #define FEATURE_FW_DL_IMODEM_TEST							// ktc client
// 규격 반영 안됨... 기버전으로 test 시 사용, ktc client 규격 반영시 제거..

/*
** Feature - Meter End =========================================================
*/

// #define FEATURE_2017Q4_BUILD_USE							//sjan 20200121 for
// build error #define FEATURE_KEY_TEST #define FEATURE_PULSE_CNT_TEST
// //sjan 20200729
#define FEATURE_ZBM_SDK_USE  // sjan 20200914
// #define FEATURE_SEPERATE_SAVE								//sjan 20200914
#define FEATURE_ZBM_SDK_USE_AUTO_JUMP_ATFER_DL  // 차후 안정화 이후 정리 필요..

/* bccho, 2023-09-19, porting, add */
#define FEATURE_ZBM_DEFAULT_RUNBANK_BANK_LOW

// CONFIG_SMARTCARD_KEYPAIR_M == y
#ifdef USING_KSE100_V231
#define FEATURE_KSE100_V231  // kconf.h, #define SMARTCARD_KEYPAIR_M 1
#endif

//
#if 1  // by WD : Test
#define FEATURE_COMPILE_DM
#define FEATURE_COMPILE_SIMU_PERI
#endif
//

#define SPI 1
#define I2C 0
// #define EEPROM_TYPE I2C
#define EEPROM_TYPE SPI

#endif /* AMG_PREDEFINES_H_ */
