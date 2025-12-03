#ifndef __SHELL_H__
#define __SHELL_H__

/*
******************************************************************************
*    INCLUDE
******************************************************************************
*/

/*
******************************************************************************
*    Definition
******************************************************************************
*/

#define _H(a) (a)

/*
******************************************************************************
*    MACRO
******************************************************************************
*/
#define SH_PRINTF shell_printf

/*
******************************************************************************
*    DATA TYPE
******************************************************************************
*/
typedef enum
{
    SHELL_NULL,
    SHELL_INVALID_CMD,
    SHELL_INVALID_PARAM,
    SHELL_OK
} SHELL_ERR;

typedef enum
{
    CTRL_NULL = 0,
    CTRL_CODE_BEGIN,
    CTRL_A = CTRL_CODE_BEGIN,
    CTRL_B,
    CTRL_C,
    CTRL_D,
    CTRL_E,
    CTRL_F,
    CTRL_G,
    CTRL_H,
    CTRL_I,
    CTRL_J,
    CTRL_K,
    CTRL_L,
    CTRL_M,
    CTRL_N,
    CTRL_O,
    CTRL_P,
    CTRL_Q,
    CTRL_R,
    CTRL_S,
    CTRL_T,
    CTRL_U,
    CTRL_V,
    CTRL_W,
    CTRL_X,
    CTRL_Y,
    CTRL_Z,
    CTRL_CODE_END
} CTRL_CODE;

typedef struct
{
    uint32_t buffSize;
    uint32_t wrPtr;
    uint8_t *pBuff;
} INPUT_CNTX;

typedef struct _SHELL_CNTX
{
    uint32_t id;
    char *cmdString;
    char *helpString;
    SHELL_ERR (*action)(uint32_t id, char *param, uint32_t size);
    const struct _SHELL_CNTX *subCntx;
} SHELL_CNTX;

typedef struct
{
    CTRL_CODE key;
    char *param;
    SHELL_ERR (*callback)(CTRL_CODE key);
} HOT_KEY;

/*
******************************************************************************
*    GLOBAL VARIABLE
******************************************************************************
*/

/*
******************************************************************************
*    GLOBAL FUNCTIONS
******************************************************************************
*/
void shell_init(void);
void shell_main(char *str, int size);
char *shell_get_token(char *pInStr, char *pOutToken);
void shell_get_atox(char *str, char *seperator, void *pOut);
char *shell_get_token_sep(char *pInStr, char *pOutToken);
char *shell_get_token_ext(char *pInStr, char *pOutToken, char sep);
char *shell_get_token_sep_ext(char *pInStr, char *pOutToken, char sep,
                              char deli);
#if 1 /* bccho, 2023-08-31, hw fault 발생하여 변경 */
void shell_arg_parse(char *pInStr, uint32_t *pArgc, char *pArgv[],
                     uint32_t maxArgc, char seperator);
#else
void shell_arg_parse(char *pInStr, uint32_t *pArgc, int pArgv[],
                     uint32_t maxArgc, char seperator);
#endif
void shell_printf(char *fmt, ...);
void shell_buff_flush(void);
void shell_mem_cget_handler(void);

bool str_to_hex(char *str, uint32_t *value);
#if 1 /* bccho, 2023-08-09, 1byte 변환시 hw fault 발생하여 아래 함수 생성 */
bool str_to_hex_uint8(const char *str, uint8_t *value);
#endif

bool str_to_hex_n(char *str, uint8_t *value, uint32_t cnt);
uint32_t get_hex_array_from_str(char *str, char *sperator, void *pOut,
                                uint32_t outUnitByteSize, uint32_t maxOutCount);

#endif /* __SHELL_H__ */
