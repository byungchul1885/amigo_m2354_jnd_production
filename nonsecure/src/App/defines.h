#ifndef _DEFINE_HEADER_
#define _DEFINE_HEADER_

#include "main.h"
#include "amg_time.h"

#define BCD2DEC(X) ((((X) >> 4) * 10) + ((X)&0xF))
#define DEC2BCD(X) (((X) % 10) + ((((X) / 10) & 0xF) << 4))

#define BCD_MAX (7)

#define METER_ID_SIZE (11)
#define AMI_METER_ID_LEN (11)

#define T_YEAR 0
#define T_MON 1
#define T_DAY 2
#define T_WEEK 3
#define T_HOUR 4
#define T_MIN 5
#define T_SEC 6

typedef struct _tagST_METER_ID
{
    uint8_t aucMeterId[AMI_METER_ID_LEN];
} __packed ST_METER_ID, *PST_METER_ID;

typedef struct _tagST_TIME_BCD
{
    uint8_t aucTimeBCD[BCD_MAX];
} __packed ST_TIME_BCD, *PST_TIME_BCD;

static inline uint32_t GET_BE32(const uint8_t *a)
{
    return ((uint32_t)a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3];
}

#endif
