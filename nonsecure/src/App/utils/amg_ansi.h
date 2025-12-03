#ifndef __ANSI_CODE_H__
#define __ANSI_CODE_H__
/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/
#include "amg_uart.h"
#include "amg_debug.h"
/*
******************************************************************************
*    Definition
******************************************************************************
*/
typedef enum
{
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    GRAY
}
ANSI_COLOR;

#define ANSI_PUTS(a,b)                  dsm_uart_enq_string (DEBUG_COM, a,b)

#define ANSI_START_CODE                 28
#define ANSI_END_CODE                   29

/*
******************************************************************************
*    MACRO
******************************************************************************
*/
#define ansi_begin(b)                   dsm_sprintf (b, "%c", ANSI_START_CODE);
#define ansi_end(b)                     dsm_sprintf (b, "%c", ANSI_END_CODE);
#define ansi_gotoxy(b, x,y)             dsm_sprintf (b, "\x1B[%d;%dH", y, x)
#define ansi_push_position(b)           dsm_sprintf (b, "\x1B[s")
#define ansi_pop_position(b)            dsm_sprintf (b, "\x1B[u")
#define ansi_clear_screen(b)            dsm_sprintf (b, "\x1B[2J")
#define ansi_clear_line(b)              dsm_sprintf (b, "\x1B[K")
#define ansi_clear_line_fromtoend(b)    dsm_sprintf (b, "\x1B[K")
#define ansi_set_color(b, fc, bc)       dsm_sprintf (b, "\x1B[%d;%dm", fc+30, bc+40)
#define ansi_set_fcolor(b,fc)           dsm_sprintf (b, "\x1B[%dm", fc+30)
#define ansi_set_bold_fcolor(b,fc)      dsm_sprintf (b, "\x1B[1m\x1B[%dm", fc+30)
#define ansi_set_bcolor(b,fc)           dsm_sprintf (b, "\x1B[%dm", fc+40)
#define ansi_set_cursor_visible(b,e)    dsm_sprintf (b, "\x1B[?25%c", e?'h':'I')
#define ansi_reset_color(b)             dsm_sprintf (b, "\x1b[39;49m")
#define ansi_reset_attr(b)              dsm_sprintf (b, "\x1b[0m");

#define ansi_flush(b,s)\
    {\
        if ((int)(sizeof (b)-1) < (int)(s))\
        {\
            DPRINTF (DBG_ERR, "The size of ANSI string is too large (%d<%d)\r\n", sizeof(b)-1, s);\
            ASSERT (0);\
        }\
        ANSI_PUTS (b,(s));\
    }

/*
******************************************************************************
*    GLOBAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*    GLOBAL FUNCTIONS
******************************************************************************
*/

#endif /* __ANSI_CODE_H__ */



