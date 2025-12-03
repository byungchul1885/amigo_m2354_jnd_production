#if !defined(__AMG_UART_H__)
#define __AMG_UART_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/

#include "amg_typedef.h"
#include "options_sel.h"
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/

/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/
#define TX_TIMEOUT ((uint32_t)100)
#define RX_TIMEOUT ((uint32_t)50)
#define RX_POLL_TIMEOUT ((uint32_t)50 * 1000)

typedef enum
{
    COM1 = 0,
    COM2 = 1,
    COM3 = 2,
    COM4 = 3,
    COM5 = 4,
    COM6 = 5,
    COM_NO
} COM_PORT;

/*
******************************************************************************
*	MACRO
******************************************************************************
*/
#define MIF_PORT COM1
#define DEBUG_COM COM2
#define IMODEM_PORT COM4
#define RS485_PORT COM3
#define EMODEM_PORT COM5

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
void dsm_uart_enable_rx_intr(COM_PORT COM);
void dsm_uart_disable_rx_intr(COM_PORT COM);
void dsm_uart_peri_rx_poll_init(uint32_t baudrate, uint32_t com_port);
void dsm_uart_init(COM_PORT COM, uint32_t baudRate, bool bPollMode,
                   uint8_t *txBuf, uint32_t txBufSize, uint8_t *rxBuf,
                   uint32_t rxBufSize, bool dma);
void dsm_uart_close(COM_PORT port);
void dsm_uart_reg_rx_callback(COM_PORT COM, void (*rxCallBack)(void));
void dsm_uart_reg_rs485_tx_enable_callback(COM_PORT port,
                                           void (*rs485TxEnableCb)(bool tx_en));
void dsm_uart_set_rs485_tx_enable_delay_time(COM_PORT port, uint32_t pre_delay,
                                             uint32_t post_delay);
void dsm_uart_send(COM_PORT COM, char *buff, uint32_t size);
void dsm_uart_raw_putc(COM_PORT COM, char ch);
void dsm_uart_raw_send(COM_PORT COM, char *str, uint32_t size);
bool dsm_uart_kbhit(COM_PORT COM, char *ch);
#if 1 /* bccho, 2023-11-10 */
bool dsm_uart_raw_getc(COM_PORT COM, uint32_t ms, char *ch);
#endif
bool dsm_uart_raw_getc_timeout(COM_PORT COM, uint32_t ms, char *ch);
uint32_t dsm_uart_gets(COM_PORT COM, char *buff, uint32_t maxLen);
uint32_t dsm_uart_rx_available(COM_PORT port);
void dsm_uart_q_flush(COM_PORT port);
void dsm_uart_enq_string(COM_PORT port, char *data, uint32_t size);
void dsm_uart_deq_string(COM_PORT port);
void dsm_uart_set_poll_mode(COM_PORT port, bool bPollMode);
bool dsm_uart_get_poll_mode(COM_PORT port);

void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void USART6_IRQHandler(void);
void dsm_uart_485_txen_control(bool enable);
void dsm_all_uart_q_flush(void);
void dsm_uart_all_ports_deq_string_line(void);

#endif /*__AMG_UART_H__*/
