#if !defined(__AMG_DEBUG_H__)
#define __AMG_DEBUG_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "amg_typedef.h"
#include "amg_sprintf.h"
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/

#define DBG_CLEAR (0)
#define DBG_ERR (1)
#define DBG_WARN (2)
#define DBG_TRACE (3)
#define DBG_INFO (4)
#define DBG_NONE (5)

#define DBG_CIPHER (1) /* bccho, 2023-10-18, NET 2차심사 시연 */

#define RUN_DBG_LVL DBG_CLEAR

typedef enum
{
    DUMP_AMI = (1 << 1),
    DUMP_DLMS = (1 << 2),
    DUMP_MIF = (1 << 3),
    DUMP_MDM = (1 << 4),
    DUMP_EMDM = (1 << 5),
    DUMP_TOU = (1 << 6),
    DUMP_SEC = (1 << 7),
    DUMP_SF = (1 << 8),

    DUMP_ALWAYS = (1 << 31)
} DUMP_MASK;
/*
******************************************************************************
*	MACRO
******************************************************************************
*/
#define DPRINTF dsm_debug_printf
#define DPRINT_HEX dsm_print_hexs

#if 1 /* bccho, 2003-07-11 */
#define MSG00(...)
#define MSG01(...)
#define MSG02(...)
#define MSG03(...)
#define MSG04(...)
#define MSG05(...)
#define MSG06(...)
// #define MSG07 bccho_debug_printf
#define MSG07(...)
#define MSG08(...)
#define MSG09(...)
// #define MSG09 bccho_debug_printf
#define MSG10 bccho_debug_printf

// #define MSGALWAYS bccho_debug_printf
#define MSGALWAYS(...)
// #define MSGERROR bccho_error_printf
#define MSGERROR(...)
// #define DPRINTF2 dsm_debug_printf2
#define DPRINTF2(...)
#endif

#define DSM_SPRINTF dsm_sprintf
#define DSM_VSPRINTF dsm_vsprintf

/*
******************************************************************************
*	DATA TYPE
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLE
******************************************************************************
*/
extern uint32_t debug_level;
extern const char *debug_function;
extern void (*PRINTF)(char *fmt, ...);
/*
******************************************************************************
*	FUNCTIONS
******************************************************************************
*/
void dsm_debug_set_level(int level);
int dsm_debug_get_level(void);
void dsm_debug_set_color_on(bool on);
void dsm_debug_printf(int level, char *fmt, ...);
#if 1 /* bccho, 2003-07-11 */
void dsm_debug_printf2(int level, char *fmt, ...);
void bccho_debug_printf(char *fmt, ...);
void bccho_error_printf(char *fmt, ...);
#endif
void dsm_printf(char *fmt, ...);
void dsm_print_hexs(int level, char *title, void *buff, uint32_t size,
                    DUMP_MASK type);
void dsm_debug_set_dump_mask(DUMP_MASK mask, bool on);

#endif /*__AMG_DEBUG_H__*/
