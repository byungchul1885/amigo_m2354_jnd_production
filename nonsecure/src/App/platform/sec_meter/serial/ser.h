#ifndef SER_H
#define SER_H 1

#include <stdint.h>
#include <stdbool.h>

/* Initialize the port and its buffer. */
void ser_init(uint8_t port_idx, int _baud);
/* De-initialize the port and its buffer. */
void ser_deinit(uint8_t port);
/* UART Interrupt Service Routine */
void ser_isr (void);

/* Attach the I/O routines to the port and its buffer. */
void ser_attach(uint8_t port, char *buf, uint8_t len);
/* Detach the I/O routines from the port and its buffer.
 * This is supposed to make the IO routines fail harmlessly. */
void ser_detach(void);

/* get a line without changing the port. */
char *ser_get_line(uint8_t port);

/* Put a character out to the current port's buffer. */
void ser_put_chr (char c);
void ser_put_str(const char *str);  // Put a string out, located in RAM, to the current port.
void ser_put_flash_str(const char *str); // Put a string out, located in Flash, to the current port.
void ser_put_crlf (void);       /* Put <CR><LF> to the UART. */
uint8_t ser_tx_busy(void);
void ser_flush (void);          /* Force buffered data into the I/O system. */
void ser_put_end_of_line(void); /* Waits till line is sent. */

/* Get a character. */
int8_t ser_get_chr (void);      /* Get next character from CLI buffer. */
void ser_unget_chr (void);      /* Safely go back by one character. */
int8_t ser_get_upper (void); /* Get next character. Force alpha to uppercase. */

/* Get next decimal (or hex) digit from CLI buffer. Ignore non numeric chars. */
int8_t ser_get_digit (void);    /* Get next hexadecimal digit. */

int32_t ser_get_32 (void);      /* Get a 32 bit number in either base. */
int16_t ser_get_16 (void);      /* get a 16 bit number in either base. */
int8_t ser_get_8 (void);        /* Get an 8 bit number in either base. */
int32_t ser_get_32b10 (uint8_t c);      /* get a 32 bit number in base 10. */
int16_t ser_get_16b10 (void);   /* get a 32 bit number in base 10. */
int8_t ser_get_8b10 (void);     /* Get an 8 bit number in base 10. */
uint32_t ser_get_32b16 (void);  /* get a 32 bit number in base 16. */
uint8_t ser_get_8b16 (void);    /* Get an 8 bit number in base 16. */

void ser_put_digit (uint8_t c); /* Put a digit. */
void ser_put_32 (int32_t n, int8_t size, uint8_t base); /* put n, size digits, in base */
void ser_put_32b10 (int32_t n); /* Put a 32 bit number in base 10. */
void ser_put_16b10 (int16_t n); /* Put a 16 bit number in base 10. */
void ser_put_8b10 (int8_t n);   /* Put an 8 bit number in base 10. */
void ser_put_32b16 (uint32_t i);/* Put a 32-bit number in base 16. */
void ser_put_16b16 (uint16_t w);/* Put a 16-bit number in base 16. */
void ser_put_8b16 (uint8_t c);  /* Put an 8 bit number in base 16. */

extern bool ser_timeout;        /* 1 = leave I/O wait loops. */

#define BUF_SIZE 128

#ifdef SER_1
extern int8_t ser1_cli_buf [BUF_SIZE+1];  /* from ser1.c; output line buffer */
#endif //SER_1

enum SERIAL_RC { S_EMPTY, S_PENDING, S_FULL, S_PARITY_ERR, S_OVERRUN, S_NACK_ERR };

void comm485_init(void);


#endif
