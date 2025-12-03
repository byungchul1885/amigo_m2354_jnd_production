#include "options.h"
#include <ctype.h>
#include <string.h>
#if 0 /* bccho, FLASH, 2023-07-15 */
#include "flash.h"
#endif /* bccho */
#ifdef SER_1
#include "ser1.h"
#endif //SER_1
#include "ser.h"


/*** Public variables used within this module ***/
#define OFFSET 30  /* Characters saved in input buffer. */
bool ser_timeout;


/*** Private functions declared within this module ***/


/*** Private variables used within this module ***/
static char * ser_get_str;
static char * ser_get_str_prev;
static char * ser_put_buf;
static char * ser_put_max;
static char * ser_out_ptr;
static const char * ser_out_str;
static char * ser_str_num;
static uint8_t ser_port_idx;


// Initialize a serial port.
void ser_init(uint8_t port_idx, int _baud)
{
    // Initialize a port.
    switch(port_idx)
    {
        #ifdef SER_1
        case 1: ser1_init(_baud); break;
        #endif //SER_1

        default: error_software(); break;
    }
}

/* UART Interrupt Service Routine */
void ser_isr (void)
{
   #ifdef SER_1
     ser1_isr();
   #endif //SER_1
}

// Put a string out ,located in RAM, to the serial driver.
// Always called from the meter loop.
static void put_str(const char *str)
{
    // Put the buffered string out to the current port.
    switch(ser_port_idx)
    {
        default: error_software(); break;
    }
}


// Put a string out,,located in Flash, to the serial driver.
// Always called from the meter loop.
static void put_flash_str(const char *str)
{
    // Put the buffered string out to the current port.
    switch(ser_port_idx)
    {
        #ifdef SER_1
        case 1: ser1_put_flash_str(str); break;
        #endif //SER_1

        default: error_software(); break;
    }
}



// Is the serial driver's transmit busy?
uint8_t ser_tx_busy(void)
{
    // Put the buffered string out to the current port.
    switch(ser_port_idx)
    {
        #ifdef SER_1
        case 1: return (uint8_t)ser1_tx_busy;
        #endif //SER_1

        default: error_software(); return 0;
    }
}

// Get a string from the serial driver.
// Always called from the meter loop.
char *ser_get_line(uint8_t port_idx)
{
	char *p = NULL;

    // Put the buffered string out to the current port.
    switch(port_idx)
    {
    #ifdef SER_1
    case 1: p = ser1_get_line(); break;
    #endif //SER_1

    default: error_software(); break;
    }

    if (NULL != p && port_idx == ser_port_idx)
    {
        ser_get_str = p;
        ser_get_str_prev = p;
        ser_out_str = p;
        ser_put_buf = p;
        ser_put_max = p + strlen(p);
        ser_out_ptr = p;
    }
    return p;
}



// Put out any buffered data.
void ser_flush (void)
{
    if (ser_out_ptr > ser_out_str)
    {
        ser_put_chr('\0');
        put_str(ser_out_str);
        ser_out_str = ser_out_ptr;
    }
}

// Attach the I/O routines to the input and output buffer.
void ser_attach(uint8_t port_idx, char *buf, uint8_t len)
{
    uint8_t cmd_len = (uint8_t)strlen(buf);

    ser_port_idx = port_idx;
    ser_get_str = buf;
    ser_get_str_prev = buf;

    // Save short commands for ','.
    if(cmd_len < OFFSET)
    {
        buf += cmd_len;
        len -= cmd_len;
    }
    *buf++ = '\0';
    --len;

    // Two cases: Old command short and preserved, or too long and gone.
    // If gone, then the "old command" becomes an empty string.
    ser_out_str = buf;
    ser_put_buf = buf;
    ser_put_max = buf + len;
    *ser_put_max = '\0'; /* terminate it, if ever needed */
    ser_out_ptr = buf;
}

// Detach the I/O routines from the input and output buffer.
// In this mode, the IO routines fail harmlessly.
void ser_detach(void)
{
    ser_flush(); // put out any buffered data.
    ser_port_idx = 0xff;
    ser_get_str = NULL;
    ser_get_str_prev = NULL;
    ser_put_buf = NULL;
    ser_put_max = NULL;
    ser_out_ptr = NULL;
    ser_out_str = NULL;
    ser_str_num = NULL;
}

// Put a string out, located in RAM, to the current port.
void ser_put_str(const char *str)
{
    ser_flush(); // put out any buffered data.
    put_str(str);
}

// Put a string out,, located in Flash, to the current port.
void ser_put_flash_str(const char *str)
{
    ser_flush();          // put out any buffered data.
    put_flash_str(str);
}

static const char cr_lf[] = "\r\n";
void ser_put_crlf (void)                   // Send <CR><LF> to UART.
{
    ser_put_flash_str(cr_lf);
}

void ser_put_end_of_line(void)
{
}

// Put a character out (into the buffer).
void ser_put_chr (char c)
{
    if (ser_out_ptr == NULL
        || ser_out_ptr < ser_put_buf
        || ser_out_ptr >= ser_put_max)
    {
        error_software();       // Buffer full!  Tell somebody!
        return;
    }
    *ser_out_ptr++ = c;
}

int8_t ser_get_chr (void)    // Get next character from CLI buffer.
{
    int8_t chr;
    ser_get_str_prev = ser_get_str;  // Provide a safe way to go backward.
    if (NULL == ser_get_str)
        return '\0';
    else
    {
        chr = *ser_get_str++;
        // Stay inside the data.
        if ('\0' == chr)
        {
            --ser_get_str;
        }
        return chr;
    }
}

void ser_unget_chr (void)    // Safely go back by one character.
{
    ser_get_str = ser_get_str_prev;
}

int8_t ser_get_upper (void)
{
    return (toupper ( ser_get_chr () ) );
}

// Get next decimal (or hex) digit from CLI buffer.
int8_t ser_get_digit (void)
{
    int8_t c;

    if (isxdigit (c = ser_get_upper ()))
    {
       c -= '0';                        // '0' mapped to  0;

       if (c >= 10)
       {
          c -= ('A' - '0') - 10;          // 'A' mapped to 10.
       }
    }
    else
       c = 0x7f;

    return (c);
}

int32_t ser_get_32b10 (uint8_t c)
{
    bool sign;
    uint8_t i;
    int32_t n;

    i = 10;                             // Maximum number of digits allowed for decimal input.
    n = 0;                              // Number to be returned.

    sign = '-' == c;

    while (0 < i && (c = ser_get_digit ()) < 10)
    {                                   // Convert ASCII decimal number to binary number.
       n = (n * 10) + c;
       i--;
    }

    if (0 != i) ser_unget_chr();                // Unget last character, it wasn't a digit.

    if (sign)
       n = -n;

    return (n);
}

static uint32_t ser_get_b16 (uint8_t i)     // Convert ASCII hexadecimal number to binary number.
{
    uint8_t c;
    uint32_t n;

    n = 0;                              // Number to be returned.
    while (i > 0 && (c = ser_get_digit ()) < 0x10)
    {                                   // Convert ASCII hexadecimal number to binary number.
       n = (n << 4) + c;
       --i;
    }

    if (i) ser_unget_chr();                 // Unget last character, it wasn't a digit.
    return (n);
}
uint32_t ser_get_32b16 (void)               // Convert ASCII hexadecimal number to binary number.
{
    return ser_get_b16(8);
}

// Convert ascii decimal (or hex) long to binary number.
int32_t ser_get_32 (void)
{
    uint8_t c;

    c = ser_get_chr ();

    if ('+' == c || '-' == c)
       return (ser_get_32b10 (c));
    else
    {
       ser_unget_chr();                         // Unget last character.
       return ((int32_t) ser_get_b16 (8));  // Default to hexadecimal input.
    }
}

// Convert ascii decimal (or hex) short to binary number.
// Used by flash commands, and the 6510's CLI commands to read the CE code space
int16_t ser_get_16 (void)
{
    return ((int16_t) ser_get_32 ());
}

// Convert ascii decimal (or hex) number to binary number.
int8_t ser_get_8 (void)
{
    return ((int8_t) ser_get_32 ());
}

// Convert ascii decimal number to binary number.
int16_t ser_get_16b10 (void)
{
    return ((int16_t) ser_get_32b10 ('+'));
}

// Convert ascii decimal number to binary number.
int8_t ser_get_8b10 (void)
{
    return ((int8_t) ser_get_32b10 ('+'));
}

uint8_t ser_get_8b16 (void)    // Convert ascii hexdecimal byte to binary
{
    return ((uint8_t) ser_get_b16(2));
}

// Numeric output functions.
static uint8_t htoc (uint8_t c)
{
    if (c > 9)
       c += 'A' - 10;
    else
       c += '0';

    return (c);
}

// Send single ASCII hex or decimal digit to DTE.
void ser_put_digit (uint8_t c)
{
    ser_put_chr (htoc (c));
}

// Reverse a string. p1 < p2!  No effect when pointers are NULL.
static void strrev(char *p1, char *p2)
{
    char chr;

    while (p1 < p2)
    {
        chr = *p2;
        *p2-- = *p1;
        *p1++ = chr;
    }
}

// Reverse and print digits.
static void flush_num(void)
{
    // Reverse the numeric string. (No effect when pointers are NULLs).
    strrev(ser_str_num, (ser_out_ptr - 1));
}

void ser_put_32 (int32_t n, int8_t size, uint8_t base)
{
    uint8_t minus,m;

    if (base > 16)
        base = 16;

    ser_str_num = ser_out_ptr;

    minus = (n < 0);

    // Compute digits in reverse order..LSD to MSD.
    for (; 0 < size || n != 0; --size)
    {
        m = n % base;
        if (0 != (0x80 & m))
            m = -m;
        ser_put_digit(m);
        n /= base;
    }

    if (minus)
    {
        ser_put_chr ('-');
    }

    // Reverse digits, add space.
    flush_num();
}

static void ser_put_32_16 (int32_t n, int8_t size)
{
    ser_str_num = ser_out_ptr;

    // Compute digits in reverse order..LSD to MSD.
    for (; 0 < size; --size)
    {
        ser_put_digit((uint8_t)(n & 0x0F));
        n >>= 4;
    }

    // Reverse digits, add space.
    flush_num();
}

void ser_put_32b10 (int32_t n)                  // Send a [0, 9,999,999,999] value to DTE.
{
    ser_put_32 (n, 1, 10);
}

void ser_put_16b10 (int16_t n)                  // Send a [0, 65,536] value to DTE.
{
    ser_put_32 ((int32_t)n, 1, 10);
}

void ser_put_8b10 (int8_t n)                  // Send a [0, 255] value to DTE.
{
    ser_put_32 ((int32_t)n, 1, 10);
}

void ser_put_32b16 (uint32_t i)
{
    ser_put_32_16 ((int32_t)i, 8);
}

void ser_put_16b16 (uint16_t w)
{
    ser_put_32_16 ((int32_t)w, 4);
}

void ser_put_8b16 (uint8_t c)
{
    ser_put_32_16 ((int32_t)c, 2);
}



