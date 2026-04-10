#ifndef __XMD_WRITE_HELPER_H__
#define __XMD_WRITE_HELPER_H__

#include <stdint.h>
#include <string.h>

#include "nsclib.h"

/*
 * Writes a 4-byte aligned buffer using XMD_Write_S.
 * The caller must provide a 4-byte aligned destination address.
 * If the destination starts at a page boundary, XMD_Write_S will erase it.
 */
static inline uint8_t dsm_xmd_write_words(uint32_t addr, const void *src,
                                          uint32_t len)
{
    const uint8_t *bytes = (const uint8_t *)src;
    uint32_t offset;

    if ((src == NULL) || ((addr & 0x3U) != 0U) || ((len & 0x3U) != 0U))
        return 1U;

    for (offset = 0; offset < len; offset += 4U)
    {
        uint32_t word = 0U;

        memcpy(&word, &bytes[offset], sizeof(word));
        if (XMD_Write_S(addr + offset, word) != 0U)
            return 1U;
    }

    return 0U;
}

#endif
