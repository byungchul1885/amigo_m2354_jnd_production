/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "os_wrap.h"
#include "amg_debug.h"
#include "amg_uart.h"
#include "amg_sprintf.h"
#include "amg_ansi.h"

/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/

/*
******************************************************************************
*    MACRO
******************************************************************************
*/
#define HEX2STR(a) ((a > 9) ? (a - 0xA + 'A') : (a + '0'))

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/
#define PRINTF_BUFF_SIZE 1024
#define FAST_DEBUG_BUFF_SIZE (8192 / 4)
#define PRINTF_FUNC_LIST 8

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
static const char dbg_lvl_char[] = {'0', '1', '2', '3', '4', '5'};

static char debug_print_buff[PRINTF_BUFF_SIZE];

#if 0 /* bccho, 2023-07-10 */
static bool         debug_color_on = FALSE;
#else
static bool debug_color_on = FALSE;
#endif

static DUMP_MASK debug_dump_mask = DUMP_ALWAYS;

uint32_t debug_level = DBG_NONE;
const char *debug_function = NULL;
int32_t debug_prt_func_idx = 0;

void (*PRINTF)(char *fmt, ...) = (void (*)(char *, ...))dsm_printf;

static const uint8_t color_table[][3] = {
    {BLUE, BLACK, 1}, {RED, BLACK, 1},   {MAGENTA, BLACK, 1},
    {CYAN, BLACK, 1}, {GREEN, BLACK, 0}, {YELLOW, BLACK, 0},
};

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/

void dsm_debug_set_level(int level) { debug_level = level; }

int dsm_debug_get_level(void) { return debug_level; }

static void dsm_debug_level_printf(int mode, int level, char *fmt, va_list ap)
{
    int size = 0;
    char *str = (char *)debug_print_buff;

    if (level > debug_level)
    {
        return;
    }

    str[size++] = '[';
    str[size++] = dbg_lvl_char[level];
    str[size++] = ']';
    str[size++] = ' ';

    if (debug_color_on)
    {
        if (color_table[level][2])
        {
            size += ansi_set_bold_fcolor(&str[size], color_table[level][0]);
        }
        else
        {
            size += ansi_set_fcolor(&str[size], color_table[level][0]);
        }

        if (mode & 2)
        {
            size += dsm_sprintf(&str[size], "[%s] ", debug_function);
        }

        size += dsm_vsprintf(&str[size], fmt, ap);
        size += ansi_reset_attr(&str[size]);
    }
    else
    {
        if (mode & 2)
        {
            size += dsm_sprintf(&str[size], "[%s] ", debug_function);
        }

        size += dsm_vsprintf(&str[size], fmt, ap);
    }

    if (mode & 1)
    {
        str[size++] = '\r';
        str[size++] = '\n';
    }

#if 1 /* bccho, 2003-07-11 */
    dsm_uart_send(DEBUG_COM, (char *)debug_print_buff, size);
#else
    debug_print_buff[size] = '\0';
    printf("%s", debug_print_buff);
#endif
}

#if 1 /* bccho, 2003-07-11 */
static void dsm_debug_level_printf2(int mode, int level, char *fmt, va_list ap)
{
    (void)level;
    int size = 0;
    char *str = (char *)debug_print_buff;

    str[size++] = '[';
    str[size++] = dbg_lvl_char[level];
    str[size++] = ']';
    str[size++] = ' ';

    if (debug_color_on)
    {
        if (color_table[level][2])
        {
            size += ansi_set_bold_fcolor(&str[size], color_table[level][0]);
        }
        else
        {
            size += ansi_set_fcolor(&str[size], color_table[level][0]);
        }

        if (mode & 2)
        {
            size += dsm_sprintf(&str[size], "[%s] ", debug_function);
        }

        size += dsm_vsprintf(&str[size], fmt, ap);
        size += ansi_reset_attr(&str[size]);
    }
    else
    {
        if (mode & 2)
        {
            size += dsm_sprintf(&str[size], "[%s] ", debug_function);
        }

        size += dsm_vsprintf(&str[size], fmt, ap);
    }

    if (mode & 1)
    {
        str[size++] = '\r';
        str[size++] = '\n';
    }

    dsm_uart_send(DEBUG_COM, (char *)debug_print_buff, size);
}

static void bccho_debug_level_printf(char *fmt, bool error, va_list ap)
{
    int size = 0;
    char *str = (char *)debug_print_buff;

    uint32_t os_t = OS_TIME_GET();
    uint32_t sec = os_t / 1000;
    uint32_t msec = os_t % 1000;

    if (error)
    {
        size += dsm_sprintf(str, "[%03u.%03u] ERROR!!!", sec, msec);
    }
    else
    {
        size += dsm_sprintf(str, "[%03u.%03u] ", sec, msec);
    }

    size += dsm_vsprintf(&str[size], fmt, ap);

    str[size++] = '\r';
    str[size++] = '\n';

    dsm_uart_send(DEBUG_COM, (char *)debug_print_buff, size);
}
#endif

void dsm_debug_set_color_on(bool on) { debug_color_on = on; }

void dsm_debug_set_dump_mask(DUMP_MASK mask, bool on)
{
    if (on)
    {
        debug_dump_mask |= mask;
    }
    else
    {
        debug_dump_mask &= ~mask;
    }

    debug_dump_mask |= DUMP_ALWAYS;

    DPRINTF(DBG_WARN, "Dump Mask =%08X\r\n", debug_dump_mask);
}

void dsm_printf(char *fmt, ...)
{
    uint32_t size;
    va_list ap;
    va_start(ap, fmt);
    size = dsm_vsprintf(debug_print_buff, fmt, ap);
    va_end(ap);
    dsm_uart_send(DEBUG_COM, debug_print_buff, size);
}

void dsm_debug_printf(int level, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    dsm_debug_level_printf(0, level, fmt, ap);
    va_end(ap);
}

#if 1 /* bccho, 2003-07-11 */
void dsm_debug_printf2(int level, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    dsm_debug_level_printf2(0, level, fmt, ap);
    va_end(ap);
}

void bccho_debug_printf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    bccho_debug_level_printf(fmt, FALSE, ap);
    va_end(ap);
}

void bccho_error_printf(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    bccho_debug_level_printf(fmt, TRUE, ap);
    va_end(ap);
}
#endif

void dsm_print_hexs(int level, char *title, void *buff, uint32_t size,
                    DUMP_MASK type)
{
    uint32_t i = 0;
    uint8_t *data = (uint8_t *)buff, ch;
    char temp[72], *ptr = &temp[4];

    if (level > debug_level)
    {
        return;
    }

    if ((type & debug_dump_mask) == 0)
    {
        return;
    }

    DPRINTF(level, "%s (size=%d)\r\n", title, size);

    temp[69] = '\r';
    temp[70] = '\n';
    temp[71] = 0;

    memset(temp, ' ', sizeof(temp) - 3);

    while (i < size)
    {
        ch = data[i] >> 4;
        ptr[0] = HEX2STR(ch);
        ch = data[i] & 0x0F;
        ptr[1] = HEX2STR(ch);
        ptr += 3;

        temp[53 + (i % 16)] =
            (data[i] >= ' ' && data[i] <= 'z') ? data[i] : '.';

        if (!((i + 1) & 0x0F))
        {
            DPRINTF(level, "%s", temp);
            memset(temp, ' ', sizeof(temp) - 3);
            ptr = &temp[4];
        }

        i++;
    }

    if (i & 0x0F)
    {
        DPRINTF(level, "%s", temp);
    }
}
