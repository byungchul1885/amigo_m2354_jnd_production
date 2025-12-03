#if !defined(__AMG_TYPEDEF_H__)
#define __AMG_TYPEDEF_H__

/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include <stdint.h>
#include <stdbool.h>
/*
******************************************************************************
*	DEFINITION
******************************************************************************
*/

#if defined(ADS120)
#define ADS_PACKED __packed
#define GCC_PACKED
#elif defined(__ICCARM__)
#define ADS_PACKED __packed
#define GCC_PACKED
#elif defined(__CS_COMPILER__)
#define ADS_PACKED
#define GCC_PACKED __attribute__((packed))
#else  // Other compilers
#define ADS_PACKED
#define GCC_PACKED __attribute__((packed))
#endif

#if defined(__GNUC__)
#define DWALIGN
#define WDALIGN
#define _512ALIGN __attribute__((aligned(512)))  // BULK_MEMORY_BLOCK_SIZE
#define _32ALIGN __attribute__((aligned(32)))
#define _DWALIGN __attribute__((aligned(8)))
#define _WALIGN __attribute__((aligned(4)))
#elif defined(__ICCARM__)
#define DWALIGN
#define _DWALIGN
#endif

#if defined(__GNUC__)
#define PRE_PACKED
#define POST_PACKED __attribute__((packed))
#elif defined(__ICCARM__)
#define PRE_PACKED __packed
#define POST_PACKED
#endif

#if 0 /* bccho, 2023-07-20. "NuMicro.h"와 겹침 */
#define NO_BITS 0x00
#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80
#define BIT8 0x0100
#define BIT9 0x0200
#define BIT10 0x0400
#define BIT11 0x0800
#define BIT12 0x1000
#define BIT13 0x2000
#define BIT14 0x4000
#define BIT15 0x8000
#define BIT16 0x010000UL
#define BIT17 0x020000UL
#define BIT18 0x040000UL
#define BIT19 0x080000UL
#define BIT20 0x100000UL
#define BIT21 0x200000UL
#define BIT22 0x400000UL
#define BIT23 0x800000UL
#define BIT24 0x01000000UL
#define BIT25 0x02000000UL
#define BIT26 0x04000000UL
#define BIT27 0x08000000UL
#define BIT28 0x10000000UL
#define BIT29 0x20000000UL
#define BIT30 0x40000000UL
#define BIT31 0x80000000UL
#endif

#define MASK_1BIT 0x00000001
#define MASK_2BITS 0x00000003
#define MASK_3BITS 0x00000007
#define MASK_4BITS 0x0000000F
#define MASK_5BITS 0x0000001F
#define MASK_6BITS 0x0000003F
#define MASK_7BITS 0x0000007F
#define MASK_8BITS 0x000000FF
#define MASK_9BITS 0x000001FF
#define MASK_10BITS 0x000003FF
#define MASK_11BITS 0x000007FF
#define MASK_12BITS 0x00000FFF
#define MASK_13BITS 0x00001FFF
#define MASK_14BITS 0x00003FFF
#define MASK_15BITS 0x00007FFF
#define MASK_16BITS 0x0000FFFF
#define MASK_17BITS 0x0001FFFF
#define MASK_18BITS 0x0003FFFF
#define MASK_19BITS 0x0007FFFF
#define MASK_20BITS 0x000FFFFF
#define MASK_21BITS 0x001FFFFF
#define MASK_22BITS 0x003FFFFF
#define MASK_23BITS 0x007FFFFF
#define MASK_24BITS 0x00FFFFFF
#define MASK_25BITS 0x01FFFFFF
#define MASK_26BITS 0x03FFFFFF
#define MASK_27BITS 0x07FFFFFF
#define MASK_28BITS 0x0FFFFFFF
#define MASK_29BITS 0x1FFFFFFF
#define MASK_30BITS 0x3FFFFFFF
#define MASK_31BITS 0x7FFFFFFF
#define MASK_32BITS 0xFFFFFFFF

#define HI 1
#define LO 0

#define HI_HI 3
#define HI_LO 2
#define LO_HI 1
#define LO_LO 0

typedef signed char S8;
typedef signed short S16;
typedef signed long S32;
typedef unsigned char U8;
typedef unsigned char u8_t;

typedef unsigned short U16;
typedef unsigned short u16_t;

typedef unsigned long U32;
typedef unsigned long u32_t;

typedef signed long long S64;
typedef unsigned long long U64;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#ifndef PNULL
#define PNULL 0
#endif

#ifndef DSM_TRUE
#define DSM_TRUE 1
#endif

#ifndef DSM_FALSE
#define DSM_FALSE 0
#endif

#ifndef DSM_NULL
#define DSM_NULL 0
#endif

#ifndef DSM_OK
#define DSM_OK 0
#endif

#ifndef DSM_NO_ERR
#define DSM_NO_ERR 0
#endif

#ifndef DSM_ERR
#define DSM_ERR (-1)
#endif

typedef union U8_16_t
{
    uint8_t c[2];
    uint16_t i;
} U8_16;

typedef union U8_16_32_t
{
    uint8_t c[4];
    uint16_t i[2];
    uint32_t l;
} U8_16_32;

/* bccho, 2023-10-04 */
typedef union U8_16_32_packet_t
{
    uint8_t c[4];
    uint16_t i[2];
    uint32_t l;
} __attribute__((packed)) U8_16_32_PACKED;

typedef union U8_Float_t
{
    uint8_t c[4];
    float f;
} U8_Float;

typedef union U8_S16_t
{
    uint8_t c[2];
    int16_t i;
} U8_S16;

typedef union U8_16_S32_t
{
    uint8_t c[4];
    uint16_t i[2];
    uint32_t l;
} U8_16_S32;

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef BOOL
#define BOOL bool
#endif

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#define MAXIMUM(A, B) MAX(A, B)
#define MINIMUM(A, B) MIN(A, B)

#define ntohl(x)                                                \
    ((unsigned long)((((unsigned long)(x)&0x000000ffU) << 24) | \
                     (((unsigned long)(x)&0x0000ff00U) << 8) |  \
                     (((unsigned long)(x)&0x00ff0000U) >> 8) |  \
                     (((unsigned long)(x)&0xff000000U) >> 24)))

#define htonl(x)                                                \
    ((unsigned long)((((unsigned long)(x)&0x000000ffU) << 24) | \
                     (((unsigned long)(x)&0x0000ff00U) << 8) |  \
                     (((unsigned long)(x)&0x00ff0000U) >> 8) |  \
                     (((unsigned long)(x)&0xff000000U) >> 24)))

#define dsm_htonl(x)                                            \
    ((unsigned long)((((unsigned long)(x)&0x000000ffU) << 24) | \
                     (((unsigned long)(x)&0x0000ff00U) << 8) |  \
                     (((unsigned long)(x)&0x00ff0000U) >> 8) |  \
                     (((unsigned long)(x)&0xff000000U) >> 24)))

#define shtonl(x)                                                            \
    ((long)((((long)(x)&0x000000ff) << 24) | (((long)(x)&0x0000ff00) << 8) | \
            (((long)(x)&0x00ff0000) >> 8) | (((long)(x)&0xff000000) >> 24)))

#define ntohs(x)                                            \
    ((unsigned short)((((unsigned short)(x)&0x00ff) << 8) | \
                      (((unsigned short)(x)&0xff00) >> 8)))

#define htons(x)                                            \
    ((unsigned short)((((unsigned short)(x)&0x00ff) << 8) | \
                      (((unsigned short)(x)&0xff00) >> 8)))

#define dsm_ntohl(x)                                            \
    ((unsigned long)((((unsigned long)(x)&0x000000ffU) << 24) | \
                     (((unsigned long)(x)&0x0000ff00U) << 8) |  \
                     (((unsigned long)(x)&0x00ff0000U) >> 8) |  \
                     (((unsigned long)(x)&0xff000000U) >> 24)))

#define dsm_shtonl(x)                                                        \
    ((long)((((long)(x)&0x000000ff) << 24) | (((long)(x)&0x0000ff00) << 8) | \
            (((long)(x)&0x00ff0000) >> 8) | (((long)(x)&0xff000000) >> 24)))

#define dsm_ntohs(x)                                        \
    ((unsigned short)((((unsigned short)(x)&0x00ff) << 8) | \
                      (((unsigned short)(x)&0xff00) >> 8)))

#define dsm_htons(x)                                        \
    ((unsigned short)((((unsigned short)(x)&0x00ff) << 8) | \
                      (((unsigned short)(x)&0xff00) >> 8)))

#endif /*__AMG_TYPEDEF_H__*/
