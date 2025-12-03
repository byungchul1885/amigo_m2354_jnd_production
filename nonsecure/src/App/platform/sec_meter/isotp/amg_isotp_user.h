#ifndef __ISOTP_USER_H__
#define __ISOTP_USER_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "isotp.h"

/*
******************************************************************************
*   Definition
******************************************************************************
*/
/* CAN ID: Mode */
#define NODE_ADVERTISEMENT_MODE_0 0 /* Node Advertisement(Power On) */
#define NODE_ADVERTISEMENT_MODE_1 1 /* Node Advertisement(Solicitation) */
#define NODE_SOLICITATION_MODE 2
#define DLMS_EX_MODE 4
#if 1
#define DLMS_PUSH_MODE 6
#else  // TODO: Check
#define DLMS_PUSH_MODE 7
#endif
/* CAN ID: Reserved */
#define CAN_ID_R0 0
/* CAN ID: Direction */
#define ISO_TP_RX 0
#define ISO_TP_TX 1
/* CAN ID: Meter ID(Meter Serial) */
#define CAN_BROADCAST_SERIAL 0xFFFFFF

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
typedef struct _ST_CAN20B_ID_
{
    uint32_t Serial : 24;
    uint32_t Dir : 1;
    uint32_t r0 : 1;
    uint32_t Mode : 3;
} ST_CAN20B_ID;
/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/
// extern IsoTpLink g_link;
/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/
char* dsm_isotp_user_id_mode_string(uint32_t mode);
/* user implemented, print debug message */
// void isotp_user_debug(const char* message, ...);
/* user implemented, send can message */
int isotp_user_send_can(uint32_t arbitration_id, uint8_t* data, uint8_t size);
/* user implemented, get millisecond */
uint32_t isotp_user_get_ms(void);
void* dsm_isotp_user_get_link(void);
void dsm_isotp_user_init(void);
void dsm_isotp_user_tx(uint32_t mode, uint8_t* ptx, uint16_t len);
void dsm_isotp_user_tx_poll(void);
uint32_t dsm_isotp_user_proc(void);

uint32_t dsm_can_nv_read_serial_number(void);
uint32_t dsm_can_is_valid_ext_id(uint32_t ext_id);
void dsm_can_advertisement_power_on(void);
void dsm_can_advertisement_solicitation(void);
void dsm_can_solicitation(void);

#endif  // __ISOTP_H__
