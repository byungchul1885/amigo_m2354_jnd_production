#ifndef __AMG_PWR_H__
#define __AMG_PWR_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#if 0 /* bccho, POWER, 2023-07-15 */
#include "stm32l4xx_ll_pwr.h"
#endif /* bccho */

/*
******************************************************************************
* 	DEFINITION
******************************************************************************
*/
typedef enum
{
    MT_BOR_LEVEL_IDX0_1_7v,
    MT_BOR_LEVEL_IDX1_2_0v,
    MT_BOR_LEVEL_IDX2_2_2v,
    MT_BOR_LEVEL_IDX3_2_5v,
    MT_BOR_LEVEL_IDX4_2_8v,
} EN_BOR_LEVEL_IDX;

typedef enum
{
    PWR_SHUTDOWN,
    PWR_STANDBY,
    PWR_STOP_2,
    PWR_STOP_1,
    PWR_LP_SLEEP,
    PWR_SLEEP_RANGE_2,
    PWR_SLEEP_RANGE_1,
    PWR_LP_RUN,
    PWR_MP_RUN,
    PWR_HP_RUN

} EN_PWR_MODE;

/*
******************************************************************************
* 	MACRO
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/
char* dsm_pwr_lowpwr_string(uint32_t idx);
void dsm_sag_port_init(void);
float dsm_bat_volts(void);
void dsm_pwr_enter_low_pwrmode(uint32_t type);

#endif /* __AMG_PWR_H__*/
