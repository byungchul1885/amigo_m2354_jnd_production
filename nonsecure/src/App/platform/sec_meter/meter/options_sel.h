
#ifndef OPTIONS_SEL_H
#define OPTIONS_SEL_H 1

#define ES2 2
#define ES1 1

#define H_CRYSTAL 8
#define HW_VER ES2

#define JND 79
#define BPW 99

#define MAKER_DEFINE JND

#if MAKER_DEFINE == JND
#define COMPANY_ID_1 '7'
#define COMPANY_ID_2 '9'

#define FLAG_ID1 'J'
#define FLAG_ID2 'N'
#define FLAG_ID3 'D'

#define PWD_CHR1 'J'
#define PWD_CHR2 'N'
#define PWD_CHR3 'D'
#define PWD_CHR4 'A'
#define PWD_CHR5 'U'
#define PWD_CHR6 'T'
#define PWD_CHR7 'O'
#define PWD_CHR8 'S'

// 1.23.17_01.
#define JND_VER_FW_ID_SIZE 10  // 11
#define JND_VERSION_FW_ID "1.23.17_01"

#else
#define COMPANY_ID_1 '4' /* 계기 제조사 번호 1 */
#define COMPANY_ID_2 '8' /* 계기 제조사 번호 2 */

#define FLAG_ID1 'S' /* 제조사 고유 코드 1 */
#define FLAG_ID2 'D' /* 제조사 고유 코드 2 */
#define FLAG_ID3 'E' /* 제조사 고유 코드 3 */

#define PWD_CHR1 'S'
#define PWD_CHR2 'D'
#define PWD_CHR3 'E'
#define PWD_CHR4 'L'
#define PWD_CHR5 'E'
#define PWD_CHR6 'C'
#define PWD_CHR7 'T'
#define PWD_CHR8 '0'
#endif

#define BATCHK_BY_PORT 0
#define BATCHK_BY_LEVEL 1

/* get_ver_fw_nv(), get_nv_version() */
#define VERSION_FW_NV "1.01.01_01"

#define LCDMAPSEL0 0xfffc0000L
#define LCDMAPSEL1 0x000003ffL

#if 1 /* bccho, POWER, 2023-08-04, 배터리전압 감지-->포트 */
#define BATCHK_DEF BATCHK_BY_PORT
#else
#define BATCHK_DEF BATCHK_BY_LEVEL
#endif

#define RTC_TCAB_VAL 0xfe4ce82bL
#define RTC_TCCD_VAL 0x0205ff0eL
#define RTC_RTCCAL_VAL 0x00C80000L

#define SAP_ADDR_PRIVATE 0xCF

#endif /* OPTIONS_SEL_H */
