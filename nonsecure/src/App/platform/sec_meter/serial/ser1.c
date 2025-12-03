
#include "main.h"
#include "options.h" /* typedef Integer Definition */
#include "irq.h"     /* interrupt controls */
#include "ser.h"     /* test the prototypes */
#include "ser1.h"    /* test the prototypes */

#if 0 /* bccho, FLASH, 2023-07-15 */
#include "flash.h"
#endif /* bccho */

#include <ctype.h>

#ifdef SER_1

#define B_SIZE (1024 + 20)
#define SER1_ECHO 0 /* Set echo mode on/off */

#define XON 0x11  /* DC1 */
#define XOFF 0x13 /* DC3 */

/* The acknowledge macros compile to "move SCON.TI,#0" and "move SCON.RI,#0"
 * In full-duplex operation, simple bit-masking will cause hangs by clearing
 * the other interrupt bit if it is set. */
#define ACK_TI() ACK_TI1() /* Acknowledge transmit interrupt. */
#define ACK_RI() ACK_RI1() /* Acknowledge receive interrupt. */

#define SERIAL_BAUD 9600
#define SERIAL_ECHO 0
#define SERIAL_SMOD \
    SMD1_SMOD0  // Serial port Baud rate select, SMOD=1 (16 times the baud clock
                // for mode 1)

/* queues */
static uint8_t rx_buf[B_SIZE];               /* Input RX circular buffer
                                              * This buffer will be mostly copied
                                              * to CLI for processing */
static volatile uint16_t rx_buf_extract_idx; /* Starting RX buffer index */
static volatile uint16_t rx_buf_insert_idx;  /* Ending RX buffer index */
static volatile uint16_t rx_cnt;             /* count of characters in rx buf */

static const int8_t null_str[] = ""; /* NULL character '\0'  */

static volatile uint16_t tx_q_extract_idx; /* Queue index to extract strings */
static volatile uint16_t tx_q_insert_idx;  /* Queue index to insert strings */
static const int8_t *tx_str = null_str;    /* Char pointer to output string. */

static uint8_t *xmit_q;
static int xmit_q_out_idx;
static int xmit_q_char_cnt;
static enum SERIAL_RC xmit_status;

/* timers */
volatile uint16_t ser1_xon_timer; /* Timer to resend XON */

/* state flags */
static volatile uint8_t xon_sent;       /* Other side can send. */
static volatile uint8_t xon_recvd;      /* Transmit ok flag */
volatile uint8_t ser1_tx_busy;          /* Transmitting flag; port in use */
static volatile uint8_t send_xoff_flag; /* Need XOFF sent */
static volatile uint8_t send_xon_flag;  /* Need XON sent */

/* Sends a zero-terminated line located in Flash */
void ser1_put_flash_str(const char *str) {}

/* Get a CLI input buffer
 * This function will return NULL if the line is not ready
 * for processing (no CR key received yet) or is being
 * used as an output buffer. */
char *ser1_get_line(void) { return (NULL); }

void send_ser1_one(void) {}

/* Serial interrupt, fill in str buffer, edit line, xmit output queue */
void ser1_isr(void) {}

void ser_rx_buf_inQ(uint8_t *rdata, uint16_t len)
{
    uint16_t cnt;

    for (cnt = 0; cnt < len; cnt++)
    {
        if (rx_buf_insert_idx >= B_SIZE)
            rx_buf_insert_idx = 0;

        rx_buf[rx_buf_insert_idx++] = rdata[cnt];
        ++rx_cnt;
    }
}

/* Initializes serial module's variables */
void ser1_init(int _baud)
{


    xon_sent = true;        /* Queue is empty, so receive is OK */
    rx_buf_extract_idx = 0; /* Reset RX buffer starting index */
    rx_buf_insert_idx = 0;  /* Reset RX buffer ending index */
    rx_cnt = 0;             /* No characters received */
    xon_recvd = true;       /* Able to send at start. */
    ser1_tx_busy = false;   /* Not transmitting at start. */
    tx_q_extract_idx = 0;   /* Reset queue starting index */
    tx_q_insert_idx = 0;    /* Reset queue ending index */
    tx_str = null_str;      /* Make the string pointer empty. */
    send_xoff_flag = false; /* No XOFF needed */
    send_xon_flag = false;  /* No XON needed */

    xmit_status = S_EMPTY;

}

int dlms_rx(uint8_t *buf, int mxlen)
{
    uint16_t i, my_rx_cnt;

    /* No data received? */
    my_rx_cnt = rx_cnt;

    if (my_rx_cnt == 0)
    {
        return 0;
    }
    if (my_rx_cnt > mxlen)
        my_rx_cnt = mxlen;

    rx_cnt -= my_rx_cnt;

    if (rx_buf_extract_idx >= B_SIZE)
        rx_buf_extract_idx = 0;

    for (i = 0; i < my_rx_cnt; i++)
    {
        buf[i] = rx_buf[rx_buf_extract_idx++];

        if (rx_buf_extract_idx >= B_SIZE)
            rx_buf_extract_idx = 0;
    }

    return (int)my_rx_cnt;
}

enum SERIAL_RC *dlms_tx(uint8_t *txptr, int txlen)
{

    xmit_q = txptr;
    xmit_q_out_idx = 0;
    xmit_q_char_cnt = txlen;

    if (S_EMPTY == xmit_status)
    {
        send_ser1_one();
        xmit_status = S_PENDING;
    }


    return &xmit_status;
}

#endif
