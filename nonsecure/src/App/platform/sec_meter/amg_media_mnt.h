#ifndef __AMG_MEDIA_MNT__
#define __AMG_MEDIA_MNT__

/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_dlms_hdlc.h"

/*
******************************************************************************
*    Definition
******************************************************************************
*/

#define MEDIA_PROTOCOL_IF_HDLC 0
#define MEDIA_PROTOCOL_IF_ATCMD 1

/*
******************************************************************************
*    MACRO
******************************************************************************
*/

/*
******************************************************************************
*    DATA TYPE
******************************************************************************
*/
typedef enum
{
    MEDIA_RUN_NONE,
    MEDIA_RUN_RS485,
    MEDIA_RUN_CAN,
    MEDIA_RUN_SUN,
    MEDIA_RUN_EXT,

} EN_RUN_MEDIA;

extern EN_RUN_MEDIA g_en_run_media_HDLC;

typedef enum
{
    NONE_IF_CHG_EVT,
    RS485_IF_CHG_EVT,
    CAN_IF_CHG_EVT,
    SUN_IF_CHG_EVT,
    EXT_IF_CHG_EVT,

} EN_IF_CHG_EVT;

typedef enum
{
    NONE_IF_RX_EVT,
    RS485_IF_RX_EVT,
    CAN_IF_RX_EVT,
    SUN_IF_RX_EVT,
    EXT_IF_RX_EVT,

} EN_IF_RX_EVT;

/*
******************************************************************************
*    GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*    FUNCTIONS
******************************************************************************
*/
void dsm_media_if_init(void);
void dsm_media_set_fsm_if_hdlc(uint32_t ifstate);
uint32_t dsm_media_get_fsm_if_hdlc(void);
void dsm_media_set_fsm_if_ATCMD(uint32_t ifstate);
uint32_t dsm_media_get_fsm_if_ATCMD(void);
char* dsm_media_if_fsm_string(uint32_t fsm);
bool dsm_media_if_dl_is_valid_asso_3_4_svraddr(uint8_t svr_u, uint8_t svr_l);
bool dsm_media_if_dl_is_valid_asso_1_svraddr(uint8_t svr_u, uint8_t svr_l);
bool dsm_media_if_dl_is_valid_asso3_clientaddr(uint8_t client_addr);
bool dsm_media_if_dl_is_valid_asso_1_2_clientaddr(uint8_t client_addr);
bool dsm_media_if_dl_is_valid_asso4_clientaddr(uint8_t client_addr);
bool dsm_media_if_dl_is_disc_or_snrm(uint8_t info);
bool dsm_media_if_dl_is_disc(uint8_t info);
bool dsm_media_if_dl_is_snrm(uint8_t info);
bool dsm_media_if_dl_is_valid_asso_3_4_svraddr(uint8_t svr_u, uint8_t svr_l);
bool dsm_media_if_dl_is_valid_asso3_clientaddr(uint8_t client_addr);
bool dsm_media_if_dl_is_valid_asso4_clientaddr(uint8_t client_addr);
bool dsm_media_if_dl_is_valid_asso_1_2_clientaddr(uint8_t client_addr);
bool dsm_media_if_return_proc(EN_IF_RX_EVT rx_evt, ST_HDLC_COM_PKT* ppkt);
uint32_t dsm_media_if_hdlc_fsm_proc(uint32_t evt);
uint32_t dsm_media_if_send(uint32_t media_protocol_type, uint32_t poll_flag,
                           uint8_t* p_data, uint16_t len);
uint32_t dsm_media_if_hdlc_send(uint32_t if_state, uint32_t poll_flag,
                                uint8_t* p_data, uint16_t len);
uint32_t dsm_media_is_can_power_off_push(void);

#endif /* __AMG_MEDIA_MNT__*/
