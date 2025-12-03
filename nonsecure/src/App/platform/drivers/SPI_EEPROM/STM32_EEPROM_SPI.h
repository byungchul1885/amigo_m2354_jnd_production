#ifndef _STM32_EEPROM_SPI_H
#define _STM32_EEPROM_SPI_H

#include "options.h"
#include "options_sel.h"

/* M95040 SPI EEPROM defines */
#define EEPROM_WREN 0x06  /*!< Write Enable */
#define EEPROM_WRDI 0x04  /*!< Write Disable */
#define EEPROM_RDSR 0x05  /*!< Read Status Register */
#define EEPROM_WRSR 0x01  /*!< Write Status Register */
#define EEPROM_READ 0x03  /*!< Read from Memory Array */
#define EEPROM_WRITE 0x02 /*!< Write to Memory Array */

#define EEPROM_WIP_FLAG 0x01 /*!< Write In Progress (WIP) flag */

#if HARDWARE_VERSION == 0x32
#define EEPROM_PAGESIZE 512
#elif HARDWARE_VERSION == 0x34
#define EEPROM_PAGESIZE 256
#else
#define EEPROM_PAGESIZE 512
#endif

#define EEPROM_BUFFER_SIZE EEPROM_PAGESIZE
#define _PAGE_SIZE EEPROM_PAGESIZE
#define EEPROM_LIMIT 0x80000

typedef enum
{
    EEPROM_STATUS_PENDING,
    EEPROM_STATUS_COMPLETE,
    EEPROM_STATUS_ERROR
} EepromOperations;

EepromOperations EEPROM_SPI_WriteBuffer(uint8_t _erase, uint8_t *pBuffer,
                                        uint32_t WriteAddr,
                                        uint32_t NumByteToWrite);
EepromOperations EEPROM_SPI_ReadBuffer(uint8_t *pBuffer, uint32_t ReadAddr,
                                       uint32_t NumByteToRead);
uint32_t dsm_spi_eep_write(uint8_t _erase, uint32_t eeprom_addr,
                           uint8_t *pwdata, uint32_t length);
uint32_t dsm_spi_eep_read(uint32_t eeprom_addr, uint8_t *prdata,
                          uint32_t length);
uint32_t dsm_spi_eep_erase(uint32_t eeprom_addr, uint32_t length);
void dsm_spi_eeprom_init(void);

#endif  // _STM32_EEPROM_SPI_H
