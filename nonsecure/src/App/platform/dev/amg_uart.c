/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/

#include "main.h"
#include "amg_uart.h"
#include "amg_ansi.h"
#include "amg_gpio.h"
#include "amg_power_mnt.h"
#include "amg_wdt.h"
#include "amg_sec.h"
#include "amg_rtc.h"
#include "options_sel.h"

extern void amr_disc_ind_end_proc(void);

/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/
typedef struct
{
    bool bOpened;
    bool bPollMode;
    bool bRs485TxEnable;
    uint32_t baudRate;
    uint32_t rxPtrHeader;
    uint32_t rxPtrTail;
    uint32_t rxBufSize;
    uint32_t txPtrHeader;
    uint32_t txPtrTail;
    uint32_t txBufSize;

    uint32_t isRxOverrun;
    uint32_t isTxOverrun;

    void (*rxEventHandler)(void);
    void (*rs485TxEnableCb)(bool enable);
    uint32_t rs485_pre_delay_time;
    uint32_t rs485_post_delay_time;

    uint8_t *pRxBuffer;
    uint8_t *pTxBuffer;
    uint16_t bufAllocFlag;
} UART_CNTX;

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/

#if 0 /* bccho, 2023-07-20 */
static USART_TypeDef *COM_UART[COM_NO] = {USART2, LPUART1, USART3, UART4,
                                          UART5};
static const uint32_t irq[] = {USART2_IRQn, LPUART1_IRQn, USART3_IRQn,
                               UART4_IRQn, UART5_IRQn};
#else
#ifdef M2354_NEW_HW
static UART_T *COM_UART[COM_NO] = {UART1, UART0, UART2, UART5, UART3};
static const uint32_t irq[] = {UART1_IRQn, UART0_IRQn, UART2_IRQn, UART5_IRQn,
                               UART3_IRQn};
#else
static UART_T *COM_UART[COM_NO] = {UART1, UART0, UART2, UART4, UART3};
static const uint32_t irq[] = {UART1_IRQn, UART0_IRQn, UART2_IRQn, UART4_IRQn,
                               UART3_IRQn};
#endif

// #define MIF_PORT COM1 --> UART1
// #define DEBUG_COM COM2 --> UART0
// #define RS485_PORT COM3 --> UART2
// #define IMODEM_PORT COM4 --> UART5
// #define EMODEM_PORT COM5 --> UART3

#endif /* bccho */
static UART_CNTX uartCntx[COM_NO];
/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/
void USART_GEN_IRQHandler(uint32_t port)
{
#if 1 /* bccho, 2023-07-20 */
    UART_CNTX *pUartCntx = &uartCntx[port];
    UART_T *pUART = COM_UART[port];

    dsm_pmnt_uart_if_callback(port);

    if (pUART->INTEN & UART_INTEN_RDAIEN_Msk)
    {
        if (UART_IS_RX_READY(pUART))
        {
            do
            {
                uint32_t next_head;
                pUartCntx->pRxBuffer[pUartCntx->rxPtrHeader] =
                    (uint8_t)UART_READ(pUART);
                next_head = (pUartCntx->rxPtrHeader + 1) % pUartCntx->rxBufSize;
                if (next_head != pUartCntx->rxPtrTail)
                {
                    pUartCntx->rxPtrHeader = next_head;
                }
                else
                {
                    pUartCntx->isRxOverrun = 1;
                    break;
                }
            } while (UART_IS_RX_READY(pUART));

            if (pUartCntx->rxEventHandler)
            {
                (*pUartCntx->rxEventHandler)();
            }
        }
    }

    /*
        Break Interrupt Flag
        Framing Error Flag
        Parity Error Flag
        RX Overflow Error Interrupt Flag
        */
    if (pUART->FIFOSTS & (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk |
                          UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk))
    {
        pUART->FIFOSTS = (UART_FIFOSTS_BIF_Msk | UART_FIFOSTS_FEF_Msk |
                          UART_FIFOSTS_PEF_Msk | UART_FIFOSTS_RXOVIF_Msk);
    }
#else
    UART_CNTX *pUartCntx = &uartCntx[port];
    USART_TypeDef *pUART = COM_UART[port];

    OSIntEnter();

    dsm_pmnt_uart_if_callback(port);

    LL_USART_ClearFlag_PE(pUART);
    LL_USART_ClearFlag_FE(pUART);
    LL_USART_ClearFlag_NE(pUART);
    LL_USART_ClearFlag_ORE(pUART);

    if (LL_USART_IsEnabledIT_RXNE(pUART) !=
        RESET) /*!< Receive data register not empty */
    {
        if (LL_USART_IsActiveFlag_RXNE(pUART))
        {
            do
            {
                pUartCntx->pRxBuffer[pUartCntx->rxPtrHeader] =
                    LL_USART_ReceiveData8(pUART);
                pUartCntx->rxPtrHeader++;

                if (pUartCntx->rxPtrHeader >= pUartCntx->rxBufSize)
                {
                    pUartCntx->rxPtrHeader = 0;
                }

                if (pUartCntx->rxPtrHeader == pUartCntx->rxPtrTail)
                {
                    pUartCntx->rxPtrTail++;
                    if (pUartCntx->rxPtrTail >= pUartCntx->rxBufSize)
                    {
                        pUartCntx->rxPtrHeader = 0;
                    }
                    pUartCntx->isRxOverrun = 1;
                    break;
                }
            } while (LL_USART_IsActiveFlag_RXNE(pUART));

            if (pUartCntx->rxEventHandler)
            {
                (*pUartCntx->rxEventHandler)();
            }
        }
    }

    if (LL_USART_IsEnabledIT_TXE(pUART) != RESET)
    {
        if (LL_USART_IsActiveFlag_TXE(pUART) != RESET)
        {
            do
            {
                if (pUartCntx->txPtrHeader != pUartCntx->txPtrTail)
                {
                    LL_USART_TransmitData8(
                        pUART, pUartCntx->pTxBuffer[pUartCntx->txPtrTail]);

                    pUartCntx->txPtrTail++;

                    if (pUartCntx->txPtrTail >= pUartCntx->txBufSize)
                    {
                        pUartCntx->txPtrTail = 0;
                    }
                }
                else
                {
                    break;
                }
            } while ((LL_USART_IsActiveFlag_TC(pUART) != RESET));
        }
    }

    OSIntExit();
#endif /* bccho */
}

#if 0 /* bccho, 2023-07-23 */    
void USART2_IRQHandler(void) { USART_GEN_IRQHandler(MIF_PORT); }
void LPUART1_IRQHandler(void) { LPUART_GEN_IRQHandler(DEBUG_COM); }
void USART3_IRQHandler(void) { USART_GEN_IRQHandler(RS485_PORT); }
void UART4_IRQHandler(void) { USART_GEN_IRQHandler(IMODEM_PORT); }
void UART5_IRQHandler(void) { USART_GEN_IRQHandler(EMODEM_PORT); }
void USART6_IRQHandler(void) { USART_GEN_IRQHandler(COM6); }
#else
void UART0_IRQHandler(void) { USART_GEN_IRQHandler(DEBUG_COM); }   /* COM2 */
void UART1_IRQHandler(void) { USART_GEN_IRQHandler(MIF_PORT); }    /* COM1 */
void UART2_IRQHandler(void) { USART_GEN_IRQHandler(RS485_PORT); }  /* COM3 */
void UART3_IRQHandler(void) { USART_GEN_IRQHandler(EMODEM_PORT); } /* COM5 */
#ifdef M2354_NEW_HW
void UART5_IRQHandler(void) { USART_GEN_IRQHandler(IMODEM_PORT); } /* COM4 */
#else
void UART4_IRQHandler(void) { USART_GEN_IRQHandler(IMODEM_PORT); } /* COM4 */
#endif
#endif

void dsm_uart_reg_rx_callback(COM_PORT COM, void (*rxCallBack)(void))
{
    UART_CNTX *pUartCntx = &uartCntx[COM];

    pUartCntx->rxEventHandler = rxCallBack;
}

void dsm_uart_reg_rs485_tx_enable_callback(COM_PORT port,
                                           void (*rs485TxEnableCb)(bool tx_en))
{
    UART_CNTX *pCntx;

    ASSERT(port < COM_NO);

    pCntx = &uartCntx[port];
    pCntx->rs485TxEnableCb = rs485TxEnableCb;
}

void dsm_uart_set_rs485_tx_enable_delay_time(COM_PORT port, uint32_t pre_delay,
                                             uint32_t post_delay)
{
    UART_CNTX *pCntx;

    ASSERT(port < COM_NO);
    pCntx = &uartCntx[port];

    DPRINTF(DBG_INFO, "Set RS-485 Ctrl Delay: Pre[%d], Post[%d]\r\n", pre_delay,
            post_delay);
    pCntx->rs485_post_delay_time = post_delay;
    pCntx->rs485_pre_delay_time = pre_delay;
}

void dsm_uart_raw_putc(COM_PORT COM, char ch)
{
#if 1 /* bccho, 2023-07-20 */
    UART_T *pUART = COM_UART[COM];
    uint32_t u32delayno = 0ul;
    uint32_t u32Exit = 0ul;

    while (UART_IS_TX_FULL(pUART)) /* Wait Tx not full or Time-out manner */
    {
        u32delayno++;
        if (u32delayno >= 0x40000000ul)
        {
            u32Exit = 1ul;
            break;
        }
    }

    if (u32Exit != 1ul)
    {
        pUART->DAT = ch;

        if (COM == RS485_PORT)
        {
            UART_WAIT_TX_EMPTY(pUART);
        }
    }
#else
    USART_TypeDef *pUART = COM_UART[COM];
    if (COM == DEBUG_COM)
    {
        while (LL_LPUART_IsActiveFlag_TXE(pUART) == RESET) OS_YIELD();
        LL_LPUART_ClearFlag_TC(pUART);
        LL_LPUART_TransmitData8(pUART, ch);
        while (!LL_LPUART_IsActiveFlag_TC(pUART));
    }
    else
    {
        while (LL_USART_IsActiveFlag_TXE(pUART) == RESET) OS_YIELD();
        LL_USART_ClearFlag_TC(pUART);
        LL_USART_TransmitData8(pUART, ch);
        while (!LL_USART_IsActiveFlag_TC(pUART));
    }
#endif /* bccho */
}

void dsm_uart_raw_send(COM_PORT COM, char *str, uint32_t size)
{
#if 1 /* bccho, 2023-07-20 */
    UART_CNTX *pCntx = &uartCntx[COM];

    if (pCntx->rs485TxEnableCb)
    {
        if (pCntx->bRs485TxEnable == FALSE)
        {
            pCntx->bRs485TxEnable = TRUE;
            pCntx->rs485TxEnableCb(TRUE);
            CLK_SysTickLongDelay_S(10000);

            while (size--)
            {
                dsm_uart_raw_putc(COM, *str++);
            }

            pCntx->bRs485TxEnable = FALSE;
            pCntx->rs485TxEnableCb(FALSE);

            amr_disc_ind_end_proc();
        }
    }
    else
    {
        while (size--)
        {
            dsm_uart_raw_putc(COM, *str++);
        }
    }
#else
    UART_CNTX *pCntx = &uartCntx[COM];
    USART_TypeDef *pUART = COM_UART[COM];

    if (pCntx->rs485TxEnableCb)
    {
        ASSERT(OSIntNesting == 0);
        pCntx->rs485TxEnableCb(TRUE);
        OSTimeDly(OS_MS2TICK(pCntx->rs485_pre_delay_time));
    }

    while (size--)
    {
        if (COM == DEBUG_COM)
        {
            while (LL_LPUART_IsActiveFlag_TC(pUART) == RESET) OS_YIELD();
        }
        UART_CNTX *pCntx = &uartCntx[COM];
        UART_T *pUART = COM_UART[COM];

        if (pCntx->rs485TxEnableCb)
        {
            pCntx->rs485TxEnableCb(TRUE);
            OSTimeDly(OS_MS2TICK(pCntx->rs485_pre_delay_time));
        }

        while (size--)
        {
            dsm_uart_raw_putc(COM, *str++);
            dsm_wdt_reset();
        }

        if (pCntx->rs485TxEnableCb)
        {
            OSTimeDly(OS_MS2TICK(pCntx->rs485_post_delay_time));
            pCntx->rs485TxEnableCb(FALSE);
            amr_disc_ind_end_proc();
        }
        else
        {
            while (LL_USART_IsActiveFlag_TC(pUART) == RESET) OS_YIELD();
        }
        dsm_uart_raw_putc(COM, *str++);

        dsm_wdt_reset();
    }

    if (pCntx->rs485TxEnableCb)
    {
        if (COM == DEBUG_COM)
        {
            while (LL_LPUART_IsActiveFlag_TC(pUART) == RESET) OS_YIELD();
        }
        else
        {
            while (LL_USART_IsActiveFlag_TC(pUART) == RESET) OS_YIELD();
        }
        OSTimeDly(OS_MS2TICK(pCntx->rs485_post_delay_time));
        pCntx->rs485TxEnableCb(FALSE);
        amr_disc_ind_end_proc();
    }
#endif /* bccho */
}

void dsm_uart_send(COM_PORT COM, char *buff, uint32_t size)
{
    if (uartCntx[COM].bPollMode)
    {
        dsm_uart_raw_send(COM, buff, size);
    }
    else
    {
        dsm_uart_enq_string(COM, buff, size);
    }
}

#if 0 /* bccho, 2023-11-14, 사용안함 */
bool dsm_uart_kbhit(COM_PORT COM, char *ch)
{
#if 1 /* bccho, 2023-07-20 */
    UART_T *pUART = COM_UART[COM];

    if (!UART_GET_RX_EMPTY(pUART))
    {
        *ch = (uint8_t)UART_READ(pUART);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
#else
    USART_TypeDef *pUART = COM_UART[COM];

    if (COM == DEBUG_COM)
    {
        if (LL_LPUART_IsActiveFlag_RXNE(pUART) != RESET)
        {
            *ch = LL_LPUART_ReceiveData8(pUART);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        if (LL_USART_IsActiveFlag_RXNE(pUART) != RESET)
        {
            *ch = LL_USART_ReceiveData8(pUART);
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
#endif /* bccho */
}
#endif

#if 1 /* bccho, 2023-11-10 */
bool dsm_uart_raw_getc(COM_PORT COM, uint32_t ms, char *ch)
{
    UART_T *pUART = COM_UART[COM];

    uint32_t pre_time = OS_TIME_GET(), timeout = MS2TIMER(ms);

    while (pUART->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk)
    {
        if (OS_TIME_GET() - pre_time >= timeout)
        {
            return false;
        }
    }

    *ch = pUART->DAT;

    return true;
}
#endif

#if 0 /* bccho, 2023-11-14, 사용안함 */
bool dsm_uart_raw_getc_timeout(COM_PORT COM, uint32_t ms, char *ch)
{
    uint32_t pre_time = OS_TIME_GET(), timeout = MS2TIMER(ms);

    while (1)
    {
        if (OS_TIME_GET() - pre_time >= timeout)
            return FALSE;
        if (dsm_uart_kbhit(COM, ch))
            return TRUE;
        OS_YIELD();
    }
}
#endif

uint32_t dsm_uart_gets(COM_PORT COM, char *buff, uint32_t maxLen)
{
    UART_CNTX *pUartCntx = &uartCntx[COM];
    uint32_t cnt = 0;
    cnt = dsm_uart_rx_available(COM);
    if (cnt > maxLen)
    {
        cnt = maxLen;
    }
    for (uint32_t i = 0; i < cnt; i++)
    {
        buff[i] = pUartCntx->pRxBuffer[pUartCntx->rxPtrTail];
        pUartCntx->rxPtrTail =
            (pUartCntx->rxPtrTail + 1) % pUartCntx->rxBufSize;
    }
    return cnt;
}

uint32_t dsm_uart_rx_available(COM_PORT port)
{
    UART_CNTX *pCntx = &uartCntx[port];

    uint32_t avail = 0;

    /* available empty buffer */
    if (pCntx->rxPtrHeader >= pCntx->rxPtrTail)
    {
        avail = pCntx->rxPtrHeader - pCntx->rxPtrTail;
    }
    else
    {
        avail = pCntx->rxBufSize - (pCntx->rxPtrTail - pCntx->rxPtrHeader);
    }
    return avail;
}

void dsm_uart_enable_rx_intr(COM_PORT COM)
{
#if 1 /* bccho, 2023-07-20 */
    NVIC_EnableIRQ(irq[COM]);
    UART_EnableInt(COM_UART[COM], UART_INTEN_RDAIEN_Msk);
#else
    if (COM == DEBUG_COM)
    {
        LL_LPUART_EnableIT_RXNE(COM_UART[COM]);
    }
    else
    {
        LL_USART_EnableIT_RXNE(COM_UART[COM]);
    }

    NVIC_EnableIRQ(irq[COM]);
#endif /* bccho */
}

void dsm_uart_disable_rx_intr(COM_PORT COM)
{
#if 1 /* bccho, 2023-07-20 */
    NVIC_DisableIRQ(irq[COM]);
    UART_DisableInt(COM_UART[COM], UART_INTEN_RDAIEN_Msk);
#else
    if (COM == DEBUG_COM)
    {
        LL_LPUART_DisableIT_RXNE(COM_UART[COM]);
    }
    else
    {
        LL_USART_DisableIT_RXNE(COM_UART[COM]);
    }
    NVIC_DisableIRQ(irq[COM]);
#endif /* bccho */
}

void dsm_uart_peri_rx_poll_init(uint32_t baudrate, uint32_t com_port)
{
#if 1 /* bccho, 2023-07-20 */
    switch (com_port)
    {
    case IMODEM_PORT:
#ifdef M2354_NEW_HW
        SYS_ResetModule_S(UART5_RST);
#else
        SYS_ResetModule_S(UART4_RST);
#endif
        break;
    case EMODEM_PORT:
        SYS_ResetModule_S(UART3_RST);
        break;
    default:
        ASSERT(0);
    }

    UART_T *pUART = COM_UART[com_port];
    UART_Open_S(pUART, baudrate);
#else
    USART_TypeDef *pUART;
    LL_LPUART_InitTypeDef LPUART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_USART_InitTypeDef USART_InitStruct = {0};

    pUART = COM_UART[com_port];

    switch (com_port)
    {
    case MIF_PORT:
        /*
                LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
                LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
                GPIO_InitStruct.Pin = STM_TX_SY_RX_Pin|STM_RX_SY_TX_Pin;
                GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
                GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
                GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
                GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
                GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
                LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        */
        break;

    case DEBUG_COM:
        /* Peripheral clock enable */
        /*
                LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);
                LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOG);
                GPIO_InitStruct.Pin =
           CONSOLE_TX_LPUART_Pin|CONSOLE_RX_LPUART_Pin; GPIO_InitStruct.Mode =
           LL_GPIO_MODE_ALTERNATE; GPIO_InitStruct.Speed =
           LL_GPIO_SPEED_FREQ_VERY_HIGH; GPIO_InitStruct.OutputType =
           LL_GPIO_OUTPUT_PUSHPULL; GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
                GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
                LL_GPIO_Init(GPIOG, &GPIO_InitStruct);
        */
        break;
    case IMODEM_PORT:

        break;
    case EMODEM_PORT:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);
        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
        GPIO_InitStruct.Pin = eMODEM_TX_UART5_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
        LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = eMODEM_RX_UART5_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
        LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        break;

    case RS485_PORT:

        break;

    default:
        ASSERT(0);
    }

    if (DEBUG_COM == com_port)
    {
        LPUART_InitStruct.BaudRate = baudrate;
        LPUART_InitStruct.DataWidth = LL_LPUART_DATAWIDTH_8B;
        LPUART_InitStruct.StopBits = LL_LPUART_STOPBITS_1;
        LPUART_InitStruct.Parity = LL_LPUART_PARITY_NONE;
        LPUART_InitStruct.TransferDirection = LL_LPUART_DIRECTION_TX_RX;
        LPUART_InitStruct.HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
        LL_LPUART_Init(pUART, &LPUART_InitStruct);
        LL_LPUART_Enable(pUART);
    }
    else
    {
        USART_InitStruct.BaudRate = baudrate;
        USART_InitStruct.DataWidth = LL_LPUART_DATAWIDTH_8B;
        USART_InitStruct.StopBits = LL_LPUART_STOPBITS_1;
        USART_InitStruct.Parity = LL_LPUART_PARITY_NONE;
        USART_InitStruct.TransferDirection = LL_LPUART_DIRECTION_TX_RX;
        USART_InitStruct.HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
        LL_USART_Init(pUART, &USART_InitStruct);
        LL_USART_Enable(pUART);
    }
#endif /* bccho */
}

void dsm_uart_peri_init(uint32_t baudrate, uint32_t com_port)
{
#if 1 /* bccho, 2023-07-20 */
    switch (com_port)
    {
    case MIF_PORT:
        MSG01("dsm_uart_peri_init_MIF_PORT, baud:%d", baudrate);
        SYS_ResetModule_S(UART1_RST);
        break;
    case DEBUG_COM:
        MSG01("dsm_uart_peri_init_DEBUG_LP_PORT, baud:%d", baudrate);
        SYS_ResetModule_S(UART0_RST);
        break;
    case IMODEM_PORT:
#ifdef M2354_NEW_HW
        SYS_ResetModule_S(UART5_RST);
#else
        SYS_ResetModule_S(UART4_RST);
#endif
        break;
    case EMODEM_PORT:
        SYS_ResetModule_S(UART3_RST);
        break;
    case RS485_PORT:
        MSG04("dsm_uart_peri_init_RS485_PORT, baud:%d", baudrate);
        SYS_ResetModule_S(UART2_RST);
        break;
    default:
        ASSERT(0);
    }

    UART_T *pUART = COM_UART[com_port];
    UART_Open_S(pUART, baudrate);
#else
    UART_CNTX *pCntx;
    USART_TypeDef *pUART;
    LL_LPUART_InitTypeDef LPUART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_USART_InitTypeDef USART_InitStruct = {0};

    pCntx = &uartCntx[com_port];
    pUART = COM_UART[com_port];

    switch (com_port)
    {
    case EMODEM_PORT:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART5);
        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOC);
        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
        GPIO_InitStruct.Pin = eMODEM_TX_UART5_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
        LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = eMODEM_RX_UART5_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
        LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        NVIC_SetPriority(UART5_IRQn, 0x0B);
        NVIC_EnableIRQ(UART5_IRQn);

        break;

    case DEBUG_COM:
        /* Peripheral clock enable */
        LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);
        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOG);
        GPIO_InitStruct.Pin = CONSOLE_TX_LPUART_Pin | CONSOLE_RX_LPUART_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
        LL_GPIO_Init(GPIOG, &GPIO_InitStruct);
        NVIC_SetPriority(LPUART1_IRQn, 0x0B);

        NVIC_EnableIRQ(LPUART1_IRQn);

        break;
    case RS485_PORT:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
        GPIO_InitStruct.Pin = RS485_TXD_UART3_Pin | RS485_RXD_UART3_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
        LL_GPIO_Init(GPIOD, &GPIO_InitStruct);
        NVIC_SetPriority(USART3_IRQn, 0x0F);

        NVIC_EnableIRQ(USART3_IRQn);

        break;
    case IMODEM_PORT:
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_UART4);
        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

        GPIO_InitStruct.Pin = iMODEM_TX_UART4_Pin | iMODEM_RX_UART4_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
        LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        NVIC_SetPriority(UART4_IRQn, 0x09);
        NVIC_EnableIRQ(UART4_IRQn);
        break;

    case MIF_PORT:
#if 1
        LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
        LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
        GPIO_InitStruct.Pin = STM_RX_SY_TX_Pin | STM_TX_SY_RX_Pin;
        GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
        GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
        GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
        GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
        LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        NVIC_SetPriority(USART2_IRQn, 0x0A);
        NVIC_EnableIRQ(USART2_IRQn);
#endif
        break;

    default:
        ASSERT(0);
    }

    if (DEBUG_COM == com_port)
    {
        LPUART_InitStruct.BaudRate = baudrate;
        LPUART_InitStruct.DataWidth = LL_LPUART_DATAWIDTH_8B;
        LPUART_InitStruct.StopBits = LL_LPUART_STOPBITS_1;
        LPUART_InitStruct.Parity = LL_LPUART_PARITY_NONE;
        LPUART_InitStruct.TransferDirection = LL_LPUART_DIRECTION_TX_RX;
        LPUART_InitStruct.HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
        LL_LPUART_Init(pUART, &LPUART_InitStruct);

        LL_LPUART_Enable(pUART);
    }
    else
    {
        USART_InitStruct.BaudRate = baudrate;
        USART_InitStruct.DataWidth = LL_LPUART_DATAWIDTH_8B;
        USART_InitStruct.StopBits = LL_LPUART_STOPBITS_1;
        USART_InitStruct.Parity = LL_LPUART_PARITY_NONE;
        USART_InitStruct.TransferDirection = LL_LPUART_DIRECTION_TX_RX;
        USART_InitStruct.HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
        LL_USART_Init(pUART, &USART_InitStruct);

        LL_USART_Enable(pUART);
    }
#endif /* bccho */
}

void dsm_uart_init(COM_PORT COM, uint32_t baudRate, bool bPollMode,
                   uint8_t *txBuf, uint32_t txBufSize, uint8_t *rxBuf,
                   uint32_t rxBufSize, bool dma)
{
    MSG05("dsm_uart_init, %d", COM);

    UART_CNTX *pCntx;

    ASSERT(COM < COM_NO);

    pCntx = &uartCntx[COM];

    if (pCntx->bOpened)
    {
        return;
    }
    memset(pCntx, 0x00, sizeof(UART_CNTX));

    pCntx->bOpened = TRUE;
    pCntx->bPollMode = bPollMode;

    pCntx->pRxBuffer = rxBuf ? rxBuf : (uint8_t *)pvPortMalloc(rxBufSize);
    ASSERT(pCntx->pRxBuffer);
    pCntx->bufAllocFlag = rxBuf ? 0 : 0x01;

    pCntx->pTxBuffer = txBuf ? txBuf : (uint8_t *)pvPortMalloc(txBufSize);
    ASSERT(pCntx->pTxBuffer);
    pCntx->bufAllocFlag |= txBuf ? 0 : 0x02;

    pCntx->rxBufSize = rxBufSize;
    pCntx->txBufSize = txBufSize;
    dsm_uart_peri_init(baudRate, COM);
    if (!dma)
    {
        dsm_uart_enable_rx_intr(COM);
    }
}

void dsm_uart_close(COM_PORT port)
{
#if 1 /* bccho, 2023-07-20 */
    UART_CNTX *pCntx;
    UART_T *pUART;

    ASSERT(port < COM_NO);

    pCntx = &uartCntx[port];
    pUART = COM_UART[port];

    if (!pCntx->bOpened)
    {
        return;
    }

    UART_Close(pUART);

    if (pCntx->bufAllocFlag & 0x01)
    {
        vPortFree(pCntx->pRxBuffer);
    }
    if (pCntx->bufAllocFlag & 0x02)
    {
        vPortFree(pCntx->pTxBuffer);
    }
    pCntx->bufAllocFlag = 0;
    pCntx->bOpened = FALSE;
#else
    UART_CNTX *pCntx;
    USART_TypeDef *pUART;

    ASSERT(port < COM_NO);

    pCntx = &uartCntx[port];
    pUART = COM_UART[port];

    if (!pCntx->bOpened)
    {
        return;
    }

    if (DEBUG_COM == port)
    {
        LL_LPUART_Disable(pUART);
        LL_LPUART_DisableIT_RXNE(pUART);
        LL_LPUART_DisableIT_TXE(pUART);
        LL_LPUART_DeInit(pUART);
    }
    else
    {
        LL_USART_Disable(pUART);
        LL_USART_DisableIT_RXNE(pUART);
        LL_USART_DisableIT_TXE(pUART);
        LL_USART_DeInit(pUART);
    }

    if (pCntx->bufAllocFlag & 0x01)
    {
        vPortFree(pCntx->pRxBuffer);
    }
    if (pCntx->bufAllocFlag & 0x02)
    {
        vPortFree(pCntx->pTxBuffer);
    }
    pCntx->bufAllocFlag = 0;
    pCntx->bOpened = FALSE;
#endif /* bccho */
}

void dsm_uart_set_poll_mode(COM_PORT port, bool bPollMode)
{
    if (bPollMode)
    {
        dsm_uart_q_flush(port);
    }
    uartCntx[port].bPollMode = bPollMode;
}

uint32_t dsm_uart_enq_available(COM_PORT port)
{
    UART_CNTX *pCntx = &uartCntx[port];

    uint32_t avail = 0;

    /* available empty buffer */
    if (pCntx->txPtrHeader >= pCntx->txPtrTail)
    {
        avail = pCntx->txBufSize - (pCntx->txPtrHeader - pCntx->txPtrTail);
    }
    else
    {
        avail = pCntx->txPtrTail - pCntx->txPtrHeader;
    }

    return avail;
}

uint32_t dsm_uart_deq_available(COM_PORT port)
{
    UART_CNTX *pCntx = &uartCntx[port];

    uint32_t avail = 0;

    /* available empty buffer */
    if (pCntx->txPtrHeader >= pCntx->txPtrTail)
    {
        avail = pCntx->txPtrHeader - pCntx->txPtrTail;
    }
    else
    {
        avail = pCntx->txBufSize - (pCntx->txPtrTail - pCntx->txPtrHeader);
    }

    return avail;
}

bool dsm_uart_get_poll_mode(COM_PORT port) { return uartCntx[port].bPollMode; }

void dsm_uart_enq_string(COM_PORT port, char *data, uint32_t size)
{
    UART_CNTX *pCntx = &uartCntx[port];

    uint32_t w_avail /*, r_avail*/;
    uint32_t cur_head, remain_hi_addr;

    /* available empty buffer */
    if (pCntx->txPtrHeader >= pCntx->txPtrTail)
    {
        w_avail = pCntx->txBufSize - (pCntx->txPtrHeader - pCntx->txPtrTail);
    }
    else
    {
        w_avail = pCntx->txPtrTail - pCntx->txPtrHeader;
    }
    if (w_avail <= size)
    {
        return;
    }

    remain_hi_addr = pCntx->txBufSize - pCntx->txPtrHeader;
    cur_head = pCntx->txPtrHeader;
    pCntx->txPtrHeader = (pCntx->txPtrHeader + size) % pCntx->txBufSize;

    if (remain_hi_addr >= size)
    {
        memcpy(&pCntx->pTxBuffer[cur_head], data, size);
    }
    else
    {
        memcpy(&pCntx->pTxBuffer[cur_head], data, remain_hi_addr);
        memcpy(pCntx->pTxBuffer, &data[remain_hi_addr],
               (size - remain_hi_addr));
    }
}

void dsm_uart_deq_string(COM_PORT port)
{
#if 1 /* bccho, 2023-07-20 */
    UART_CNTX *pCntx = &uartCntx[port];
    if (pCntx->bOpened == FALSE)
    {
        return;
    }
    if (pCntx->txPtrHeader != pCntx->txPtrTail)
    {
        if (pCntx->rs485TxEnableCb)
        {
            if (pCntx->bRs485TxEnable == FALSE)
            {
                pCntx->bRs485TxEnable = TRUE;
                pCntx->rs485TxEnableCb(TRUE);
                CLK_SysTickLongDelay_S(10000);

                do
                {
                    dsm_uart_raw_putc(port, pCntx->pTxBuffer[pCntx->txPtrTail]);
                    pCntx->txPtrTail =
                        (pCntx->txPtrTail + 1) % pCntx->txBufSize;
                } while (pCntx->txPtrHeader != pCntx->txPtrTail);

                pCntx->bRs485TxEnable = FALSE;
                pCntx->rs485TxEnableCb(FALSE);

                amr_disc_ind_end_proc();
            }
        }
        else
        {
            do
            {
                dsm_uart_raw_putc(port, pCntx->pTxBuffer[pCntx->txPtrTail]);
                pCntx->txPtrTail = (pCntx->txPtrTail + 1) % pCntx->txBufSize;
            } while (pCntx->txPtrHeader != pCntx->txPtrTail);
        }
    }

#else
    uint32_t temp;
    char ch;
    UART_CNTX *pCntx = &uartCntx[port];
    USART_TypeDef *pUART = COM_UART[port];

    temp = pCntx->txPtrTail;

    if (pCntx->txPtrHeader == temp)
    {
        if (DEBUG_COM == port)
        {
            if (pCntx->bRs485TxEnable &&
                LL_LPUART_IsActiveFlag_TC(pUART) == SET)
            {
                pCntx->bRs485TxEnable = FALSE;
                if (pCntx->rs485TxEnableCb)
                {
                    OSTimeDly(OS_MS2TICK(pCntx->rs485_post_delay_time));
                    pCntx->rs485TxEnableCb(FALSE);

                    amr_disc_ind_end_proc();
                }
            }
        }
        else
        {
            if (pCntx->bRs485TxEnable && LL_USART_IsActiveFlag_TC(pUART) == SET)
            {
                pCntx->bRs485TxEnable = FALSE;
                if (pCntx->rs485TxEnableCb)
                {
                    OSTimeDly(OS_MS2TICK(pCntx->rs485_post_delay_time));
                    pCntx->rs485TxEnableCb(FALSE);

                    amr_disc_ind_end_proc();
                }
            }
        }
        return;
    }

    if (!ch_cnt)
    {
        ch_cnt = 1000000;
    }

    if (!pCntx->bRs485TxEnable)
    {
        pCntx->bRs485TxEnable = TRUE;
        if (pCntx->rs485TxEnableCb)
        {
            pCntx->rs485TxEnableCb(TRUE);
            OSTimeDly(OS_MS2TICK(pCntx->rs485_pre_delay_time));
        }
    }

    while (ch_cnt-- && (pCntx->txPtrHeader != temp))
    {
        ch = pCntx->pTxBuffer[temp++];
        temp &= (pCntx->txBufSize - 1);
        dsm_uart_raw_putc(port, ch);
    }
    pCntx->txPtrTail = temp;
#endif /* bccho */
}

void dsm_uart_q_flush(COM_PORT port) { dsm_uart_deq_string(port); }

void dsm_uart_485_txen_control(bool enable)
{
    if (enable)
    {
        dsm_gpio_tx_en_enable();
    }
    else
    {
        dsm_gpio_tx_en_disable();
    }
}

void dsm_all_uart_q_flush(void)
{
    uint8_t port;

    for (port = COM1; port < /*COM_NO*/ COM6; port++)
    {
        UART_CNTX *pCntx = &uartCntx[port];
        if (pCntx->bOpened == FALSE)
        {
            continue;
        }
        if (pCntx->txPtrHeader != pCntx->txPtrTail)
        {
            if (pCntx->rs485TxEnableCb)
            {
                if (pCntx->bRs485TxEnable == FALSE)
                {
                    pCntx->bRs485TxEnable = TRUE;
                    pCntx->rs485TxEnableCb(TRUE);
                    CLK_SysTickLongDelay_S(10000);

                    do
                    {
                        dsm_uart_raw_putc(port,
                                          pCntx->pTxBuffer[pCntx->txPtrTail]);
                        pCntx->txPtrTail =
                            (pCntx->txPtrTail + 1) % pCntx->txBufSize;
                    } while (pCntx->txPtrHeader != pCntx->txPtrTail);

                    pCntx->bRs485TxEnable = FALSE;
                    pCntx->rs485TxEnableCb(FALSE);

                    amr_disc_ind_end_proc();
                }
            }
            else
            {
                do
                {
                    dsm_uart_raw_putc(port, pCntx->pTxBuffer[pCntx->txPtrTail]);
                    pCntx->txPtrTail =
                        (pCntx->txPtrTail + 1) % pCntx->txBufSize;
                } while (pCntx->txPtrHeader != pCntx->txPtrTail);
            }
        }
    }
}