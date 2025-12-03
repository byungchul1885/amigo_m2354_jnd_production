#ifndef __PLATFORM_MAIN_H
#define __PLATFORM_MAIN_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include <stdint.h>
#include <stdbool.h>

#if 0 /* bccho, 2023-07-20 */
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "stm32l4xx_ll_i2c.h"
#include "stm32l4xx_ll_iwdg.h"
#include "stm32l4xx_ll_lptim.h"
#include "stm32l4xx_ll_lpuart.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_crs.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_exti.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_utils.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_dma.h"
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_tim.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_gpio.h"

#include "stm32l4xx_hal.h"
#if defined(USE_STDPHRIPH_DRIVER)
#include "stm32l4xx_hal_conf.h"
#endif

/*
******************************************************************************
* 	DEFINITION
******************************************************************************
*/
#define UART1_RX_DMA_STREAM DMA1_Channel1
#define UART3_RX_DMA_STREAM DMA1_Channel3
#define UART2_RX_DMA_STREAM DMA1_Channel6
#define UART5_RX_DMA_STREAM DMA2_Channel2
#define UART4_RX_DMA_STREAM DMA2_Channel5
#define LPUART1_RX_DMA_STREAM DMA2_Channel7

#define RLY_SET_Pin LL_GPIO_PIN_4
#define RLY_SET_GPIO_Port GPIOG
#define RLY_RESET_Pin LL_GPIO_PIN_3
#define RLY_RESET_GPIO_Port GPIOG

#define SW_COVER1_Pin LL_GPIO_PIN_5
#define SW_COVER1_GPIO_Port GPIOF

#if 0
#define CAL_ADJ_Pin LL_GPIO_PIN_5
#define CAL_ADJ_GPIO_Port GPIOE
#endif

#define EOI_1_Pin LL_GPIO_PIN_6
#define EOI_1_GPIO_Port GPIOG

#define EOI_2_Pin LL_GPIO_PIN_0
#define EOI_2_GPIO_Port GPIOG

#define POWER_FAIL_Pin LL_GPIO_PIN_5
#define POWER_FAIL_GPIO_Port GPIOC

#if 0
#define TEST_TP_1_pin LL_GPIO_PIN_5
#define TEST_TP_2_pin LL_GPIO_PIN_6
#define TEST_TP_3_pin LL_GPIO_PIN_7
#define TEST_TP_Port GPIOD
#endif

#if 0
#define KEY_MOVE_Pin LL_GPIO_PIN_4
#else
#define KEY_MOVE_Pin LL_GPIO_PIN_5
#endif
#define KEY_MOVE_GPIO_Port GPIOE
#define KEY_MENU_Pin LL_GPIO_PIN_6
#define KEY_MENU_GPIO_Port GPIOE

#define MODEM_RESET_Pin LL_GPIO_PIN_1
#define MODEM_RESET_GPIO_Port GPIOC
#define MODEM_MODE_Pin LL_GPIO_PIN_0
#define MODEM_MODE_GPIO_Port GPIOC
#define MODEM_PF_Pin LL_GPIO_PIN_3
#define MODEM_PF_GPIO_Port GPIOC
#define MODEM_PWR_EN_Pin LL_GPIO_PIN_7
#define MODEM_PWR_EN_GPIO_Port GPIOB

#define iMODEM_RX_UART4_Pin LL_GPIO_PIN_1
#define iMODEM_RX_UART4_GPIO_Port GPIOA
#define iMODEM_TX_UART4_Pin LL_GPIO_PIN_0
#define iMODEM_TX_UART4_GPIO_Port GPIOA

#define MAGN_SENS_Pin LL_GPIO_PIN_3
#define MAGN_SENS_GPIO_Port GPIOF

#define STM_TX_SY_RX_Pin LL_GPIO_PIN_2
#define STM_TX_SY_RX_GPIO_Port GPIOA
#define STM_RX_SY_TX_Pin LL_GPIO_PIN_3
#define STM_RX_SY_TX_GPIO_Port GPIOA

#define WD_DONE_Pin LL_GPIO_PIN_6
#define WD_DONE_GPIO_Port GPIOF

#if 0  // dc ���� ����
#define PWR_SENSE_Pin LL_GPIO_PIN_2
#define PWR_SENSE_GPIO_Port GPIOA
#endif
#define EEPROM_PWR_EN_Pin LL_GPIO_PIN_12
#define EEPROM_PWR_EN_Port GPIOA

#if 0
#define EEPROM_WP_Pin LL_GPIO_PIN_13
#define EEPROM_WP_GPIO_Port GPIOF
#endif
#define EEPROM_SCL_1_Pin LL_GPIO_PIN_14
#define EEPROM_SCL_1_GPIO_Port GPIOG
#define EEPROM_SDA_1_Pin LL_GPIO_PIN_13
#define EEPROM_SDA_1_GPIO_Port GPIOG

#define SPIM_HOLD_Pin LL_GPIO_PIN_4
#define SPIM_HOLD_GPIO_Port GPIOD
#define SPIM_WP_Pin LL_GPIO_PIN_5
#define SPIM_WP_GPIO_Port GPIOD
#define SPIM_1_NSS_Pin LL_GPIO_PIN_12
#define SPIM_1_NSS_GPIO_Port GPIOG
#define SPIM_1_SCLK_Pin LL_GPIO_PIN_9
#define SPIM_1_SCLK_GPIO_Port GPIOG
#define SPIM_1_MISO_Pin LL_GPIO_PIN_10
#define SPIM_1_MISO_GPIO_Port GPIOG
#define SPIM_1_MOSI_Pin LL_GPIO_PIN_11
#define SPIM_1_MOSI_GPIO_Port GPIOG

#define CONSOLE_TX_LPUART_Pin LL_GPIO_PIN_7
#define CONSOLE_TX_LPUART_GPIO_Port GPIOG
#define CONSOLE_RX_LPUART_Pin LL_GPIO_PIN_8
#define CONSOLE_RX_LPUART_GPIO_Port GPIOG

#define ZCD_PULSE_P3SX_Pin LL_GPIO_PIN_2
#define ZCD_PULSE_P3SX_GPIO_Port GPIOC

#define POWER_FAIL_P3SX_Pin POWER_FAIL_Pin
#define POWER_FAIL_P3SX_GPIO_Port POWER_FAIL_GPIO_Port

#if 0
#define STM_RST_P3SX_Pin LL_GPIO_PIN_12
#define STM_RST_P3SX_GPIO_Port GPIOG
#endif
#define RS485_TXD_UART3_Pin LL_GPIO_PIN_8
#define RS485_TXD_UART3_GPIO_Port GPIOD
#define RS485_RXD_UART3_Pin LL_GPIO_PIN_9
#define RS485_RXD_UART3_GPIO_Port GPIOD

#define RS485_EN_Pin LL_GPIO_PIN_1
#define RS485_EN_GPIO_Port GPIOB
#define RS485_SELECT_Pin LL_GPIO_PIN_15
#define RS485_SELECT_GPIO_Port GPIOE
#define CAN_RXD_1_Pin LL_GPIO_PIN_0
#define CAN_RXD_1_GPIO_Port GPIOD
#define CAN_TXD_1_Pin LL_GPIO_PIN_1
#define CAN_TXD_1_GPIO_Port GPIOD

#define eMODEM_TX_UART5_Pin LL_GPIO_PIN_12
#define eMODEM_TX_UART5_GPIO_Port GPIOC
#define eMODEM_RX_UART5_Pin LL_GPIO_PIN_2
#define eMODEM_RX_UART5_GPIO_Port GPIOD

#define eMODEM_RESET_Pin LL_GPIO_PIN_3
#define eMODEM_RESET_GPIO_Port GPIOD
#if 0
#define eMODEM_MODE_Pin LL_GPIO_PIN_9
#define eMODEM_MODE_GPIO_Port GPIOG

#define MCU_ZCD_PULSE_Pin LL_GPIO_PIN_2
#define MCU_ZCD_PULSE_GPIO_Port GPIOC
#endif

#define CRYTO_TX_Pin LL_GPIO_PIN_6
#define CRYTO_TX_GPIO_Port GPIOB
#define CRYTO_RESET_Pin LL_GPIO_PIN_8
#define CRYTO_RESET_GPIO_Port GPIOB
#define CRYTO_CK_Pin LL_GPIO_PIN_5
#define CRYTO_CK_GPIO_Port GPIOB

#define CRYTO_VCC_Pin LL_GPIO_PIN_7
#define CRYTO_VCC_GPIO_Port GPIOB

#define LCD_VLCD_Pin LL_GPIO_PIN_2
#define LCD_VLCD_GPIO_Port GPIOB

#if 0
#define LCD_COM1_Pin LL_GPIO_PIN_9
#define LCD_COM1_GPIO_Port GPIOA
#define LCD_COM2_Pin LL_GPIO_PIN_10
#define LCD_COM2_GPIO_Port GPIOA
#define LCD_COM4_Pin LL_GPIO_PIN_10
#define LCD_COM4_GPIO_Port GPIOC
#define LCD_COM5_Pin LL_GPIO_PIN_11
#define LCD_COM5_GPIO_Port GPIOC
#endif

#define LCD_SEG36_Pin LL_GPIO_PIN_0
#define LCD_SEG36_GPIO_Port GPIOE
#define LCD_SEG37_Pin LL_GPIO_PIN_1
#define LCD_SEG37_GPIO_Port GPIOE

#define LCD_SEG38_Pin LL_GPIO_PIN_2
#define LCD_SEG38_GPIO_Port GPIOE
#define LCD_SEG39_Pin LL_GPIO_PIN_3
#define LCD_SEG39_GPIO_Port GPIOE

    // #define LCD_SEG16_Pin                   LL_GPIO_PIN_8
    // #define LCD_SEG16_GPIO_Port             GPIOB

#define LCD_SEG10_Pin LL_GPIO_PIN_9
#define LCD_SEG10_GPIO_Port GPIOB
#define LCD_SEG11_Pin LL_GPIO_PIN_10
#define LCD_SEG11_GPIO_Port GPIOB

#define LCD_SEG12_Pin LL_GPIO_PIN_12
#define LCD_SEG12_GPIO_Port GPIOB
#define LCD_SEG13_Pin LL_GPIO_PIN_13
#define LCD_SEG13_GPIO_Port GPIOB
#define LCD_SEG14_Pin LL_GPIO_PIN_14
#define LCD_SEG14_GPIO_Port GPIOB
#define LCD_SEG15_Pin LL_GPIO_PIN_15
#define LCD_SEG15_GPIO_Port GPIOB
#define LCD_SEG28_Pin LL_GPIO_PIN_10
#define LCD_SEG28_GPIO_Port GPIOC
#define LCD_SEG29_Pin LL_GPIO_PIN_11
#define LCD_SEG29_GPIO_Port GPIOC
#define LCD_SEG30_Pin LL_GPIO_PIN_10
#define LCD_SEG30_GPIO_Port GPIOD
#define LCD_SEG31_Pin LL_GPIO_PIN_11
#define LCD_SEG31_GPIO_Port GPIOD
#define LCD_SEG32_Pin LL_GPIO_PIN_12
#define LCD_SEG32_GPIO_Port GPIOD
#define LCD_SEG33_Pin LL_GPIO_PIN_13
#define LCD_SEG33_GPIO_Port GPIOD
#define LCD_SEG34_Pin LL_GPIO_PIN_14
#define LCD_SEG34_GPIO_Port GPIOD
#define LCD_SEG35_Pin LL_GPIO_PIN_15
#define LCD_SEG35_GPIO_Port GPIOD

#define LCD_SEG24_Pin LL_GPIO_PIN_6
#define LCD_SEG24_GPIO_Port GPIOC
#define LCD_SEG25_Pin LL_GPIO_PIN_7
#define LCD_SEG25_GPIO_Port GPIOC
#define LCD_SEG26_Pin LL_GPIO_PIN_8
#define LCD_SEG26_GPIO_Port GPIOC
#define LCD_SEG27_Pin LL_GPIO_PIN_9
#define LCD_SEG27_GPIO_Port GPIOC

    // #define LCD_SEG3_Pin                    LL_GPIO_PIN_6
    // #define LCD_SEG3_GPIO_Port              GPIOA

    // #define LCD_SEG4_Pin                    LL_GPIO_PIN_7
    // #define LCD_SEG4_GPIO_Port              GPIOA

#define LCD_SEG22_Pin LL_GPIO_PIN_4
#define LCD_SEG22_GPIO_Port GPIOC

#define LCD_SEG5_Pin LL_GPIO_PIN_0
#define LCD_SEG5_GPIO_Port GPIOB

    // #define LCD_SEG6_Pin                    LL_GPIO_PIN_1
    // #define LCD_SEG6_GPIO_Port              GPIOB

#if 0
#define IMODEM_WDT_PULSE_Pin LL_GPIO_PIN_5
#define IMODEM_WDT_PULSE_Port GPIOF
#endif

#define SYSCLK 80000000
#define HCLK 80000000
#define PCLK1 20000000
#define PCLK2 80000000
#define ADCCLK 20000000
#define APB1_TIMCLK 80000000
#define APB2_TIMCLK 80000000
#endif /* bccho */

    typedef enum
    {
        CAUSE_FWRST = 0x01,   /*!< Firewall reset flag */
        CAUSE_OBLRST = 0x02,  /*!< Option Byte Loader reset flag */
        CAUSE_PINRST = 0x04,  /*!< PIN reset flag */
        CAUSE_BORRST = 0x08,  /*!< BOR reset flag */
        CAUSE_SFTRST = 0x10,  /*!< Software Reset flag */
        CAUSE_IWDGRST = 0x20, /*!< Independent Watchdog reset flag */
        CAUSE_WWDGRST = 0x40, /*!< Window watchdog reset flag */
        CAUSE_LPWRRST = 0x80, /*!< Low-Power reset flag */
    } EN_RESET_CAUSE;

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

#if 0  /* bccho, 2023-07-20 */
    extern LCD_HandleTypeDef hlcd;
#endif /* bccho */

    /*
    ******************************************************************************
    *	GLOBAL FUNCTIONS
    ******************************************************************************
    */
    void SystemClock_Config(void);
    void Error_Handler(void);
    int dsm_platform_pre_init(void);
    int dsm_platform_init(void);
    uint8_t dsm_reset_get_cause(void);
    bool dsm_is_software_reset(uint8_t reset_cause);
    bool dsm_is_bor_reset(uint8_t reset_cause);
    uint32_t dsm_reset_display_cause(uint8_t resetCause);
    void dsm_poll_delay_us(uint32_t us);
    void dsm_poll_delay_ms(uint32_t ms);
#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_MAIN_H */
