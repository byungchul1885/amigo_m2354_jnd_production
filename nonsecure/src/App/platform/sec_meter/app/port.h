
#ifndef PORT_H
#define PORT_H 1
#include "amg_gpio.h"
#include "options_sel.h"

#define trip_led_outmode()
#define trip_led_on()
#define trip_led_off()
#define trip_led_toggle()

#define tamper_port_init()     /*PD1 &= ~BIT2*/
#define MCOVER_OPEN_IN (true)  // actually should be true

#if METER_TYPE == MT_1P2W_60A /* bccho, 2024-09-24 */
#define TCOVER_OPEN_IN (!SW_OPEN_STATE)
#else /* 삼상 */
#define TCOVER_OPEN_IN (SW_OPEN_STATE)
#endif

#define batdet_port_init() /*PD1 &= ~BIT16*/

#if 1 /* bccho, 2023-08-04 */
#define BAT_DET_IN PC13
#else
#define BAT_DET_IN /*(PI1 & BIT16)*/
#endif

// 485 comm
#define tx485_port_init() /*PD0 |= BIT12*/
#define tx485_inmode()    /*PD0 &= ~BIT12*/
#define TX485_ENABLE      /*PO0 |= BIT12*/
#define TX485_DISABLE     /*PO0 &= ~BIT12*/
// relay control port
#define relay_port_init()                    /*PD0 |= BIT29; \
                                                                 PD0 |= BIT31*/
#define relay_port_inmode()                  /*PD0 &= ~BIT29; \
                                                                         PD0 &= ~BIT31*/
#define relay_set_on() dsm_gpio_relay_on()   /*PO0 &= ~BIT31*/
#define relay_set_off() dsm_gpio_relay_off() /*PO0 &= ~BIT29*/
#define relay_set_release()                  /*PO0 |= BIT29; \
                                                                         PO0 |= BIT31*/
// key read
#define key_port_init() /*PD1 &= ~BIT21;                  \
                                            PD1 &= ~BIT3; \
                                            PD1 &= ~BIT20*/
// menu
#define KEY0_IN (dsm_gpio_get_keyinput(KEY_MENU_0)) /*(!(PI1 & BIT21))*/
// move
#define KEY1_IN (dsm_gpio_get_keyinput(KEY_MOVE_1)) /*(PI1 & BIT3)*/
#define KEY2_IN                                     /*(!(PI1 & BIT20))*/

#define dbg_port_outmode()
#define dbg_port_on()
#define dbg_port_off()
#define dbg_port_toggle()

#define cal_key_inmode()                                /*PD1 &= ~BIT14;*/
#define cal_key_in() (dsm_gpio_get_keyinput(KEY_CAL_3)) /*(!(PI1 & BIT14))*/

#define EOI_VPORT_inmode() \
    (dsm_eoi_1_disable_pulse_out())  // eoi disable, pulse out
#define EOI_WPORT_inmode() \
    (dsm_eoi_2_disable_pulse_out())  // eoi disable, pulse out
#define EOI_selWPORT_inmode()        /*PD0 &= ~BIT25*/
#define EOI_selVPORT_inmode()        /*PD1 &= ~BIT31*/

#define EOI_VPORT_DISABLE \
    (dsm_eoi_1_disable_pulse_out())            // eoi disable, pulse out
#define EOI_VPORT_ACTIVE (dsm_eoi_1_led_on())  // eoi enable, pulse off, led on
#define EOI_VPORT_DEACTIVE \
    (dsm_eoi_1_disable())  // eoi disable, pulse  상태 유지

#define EOI_WPORT_DISABLE \
    (dsm_eoi_2_disable_pulse_out())            // eoi disable, pulse out
#define EOI_WPORT_ACTIVE (dsm_eoi_2_led_on())  // eoi enable, pulse off, led on
#define EOI_WPORT_DEACTIVE \
    (dsm_eoi_2_disable())  // eoi disable, pulse  상태 유지

#define PULSE_WPORT_DEACTIVE (dsm_pulse_w_off())  // pulse off
#define PULSE_VPORT_DEACTIVE (dsm_pulse_v_off())  // pulse off

#define EOI_WVPORT_DEACTIVE \
    EOI_WPORT_DEACTIVE;     \
    EOI_VPORT_DEACTIVE  // eoi enable, pulse 상태 유지

#define EOI_selWPORT_INIT /*PD0 |= BIT25*/
#define EOI_selVPORT_INIT /*PD1 |= BIT31*/

#define EOI_selWPORT_EOI   /*PO0 |= BIT25*/
#define EOI_selVPORT_EOI   /*PO1 |= BIT31*/
#define EOI_selWPORT_PULSE /*PO0 &= ~BIT25*/
#define EOI_selVPORT_PULSE /*PO1 &= ~BIT31*/

// eeprom WP
#define eep_wp_init()
#define eep_wp_inmode()
#define eep_wp_enable()
#define eep_wp_disable()

#define pwrfail_ind_inmode()
#define pwrfail_ind_init()
#define PWRFAIL_IND_PWRON
#define PWRFAIL_IND_PWROFF

#define MT_MON_TOGGLE_1
#define MT_MON_TOGGLE_2
#define MT_MON_TOGGLE_3
#define MT_MON_TOGGLE_4

#define BATDET_V3P3DISABLE          /*PD1 |= BIT15; \
                                                                PO1 &= ~BIT15*/
#define BATDET_V3P3ENABLE           /*PD1 &= ~BIT15*/
#define BATDET_V3P3DISABLE_inmode() /*PD1 &= ~BIT15*/

void port_inmode(void);

#endif
