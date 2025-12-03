#ifndef _BOOT_RESTORE_H
#define _BOOT_RESTORE_H

/* 4 byte align 되어야 한다 */
typedef struct _boot_restore
{
    uint32_t fsm; /* stock_op fsm */
    uint32_t sec_1;
    uint16_t ms_1;
    uint32_t sec_2;
    uint16_t ms_2;
    uint32_t seed;
    uint8_t bank;
    uint8_t goto_dpd;
    uint8_t reserved[2];
} __attribute__((__packed__)) BOOT_RESTORE;

#define BOOT_RESTORE_BASE (FMC_APROM_END - FMC_FLASH_PAGE_SIZE)
#endif