#ifndef _SER1_H
#define _SER1_H 1

extern int8_t ser1_cli_buf[];

extern volatile uint8_t ser1_tx_busy; /* transmit in progress */

/*
 * Get a input CLI buffer
 */
char *ser1_get_line(void);

/*
 * Sends a zero-terminated line located in Data RAM
 */
void ser1_put_str(const char *);

/*
 * Sends a zero-terminated line located in Flash
 */
void ser1_put_flash_str (const char *str);
/*
 * Initializes serial module's variables.
 */
void ser1_init(int _baud);

/*
* Serial interrupt, fill in str buffer, edit line, xmit output queue
*/
void ser1_isr(void);

void send_ser1_one(void);

void ser_rx_buf_inQ(uint8_t* rdata, uint16_t len);



#endif // _OPTIONS_H
