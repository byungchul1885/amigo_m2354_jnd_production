#ifndef __EEPROM_AT24CM02_H__
#define __EEPROM_AT24CM02_H__

#if EEPROM_TYPE == SPI
#include "STM32_EEPROM_SPI.h"
#else /* I2C */
#define EEPROM_SIZE 0x40000 /* bccho, 2023-07-31, 0x20000 --> 0x40000 */
#define _PAGE_SIZE 256
#define EEPROM_LIMIT 0x80000
#endif

#define I2C_ADDRESS \
    0xA0 /* bccho, 2023-07-26, 0x50 << 1, 01010000 ==> 10100000 */
#define I2C_ADDRESS2 \
    0xB0 /* bccho, 2023-07-26, 0x58 << 1, 01011000 ==> 10110000 */

void dsm_eeprom_init(void);
uint32_t dsm_eeprom_write(uint8_t _erase, uint32_t eeprom_addr, uint8_t* pwdata,
                          uint32_t length);
uint32_t dsm_eeprom_read(uint32_t eeprom_addr, uint8_t* prdata,
                         uint32_t length);
uint32_t dsm_eeprom_erase(uint32_t eeprom_addr, uint32_t length);

#endif
