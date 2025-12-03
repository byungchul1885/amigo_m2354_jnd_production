#include <stdio.h>
#include <string.h>
#include "amg_typedef.h"
#include "amg_debug.h"
#include "mx25r4035f.h"
#include "mx25r4035f_def.h"
#include "mx25r4035f-interface.h"
#include "main.h"
#include "amg_spi.h"
#include "amg_gpio.h"
#include "options_sel.h"

#define MX25_SPI_READ 0xFF

#if 1 /* bccho, 2023-08-02 */
void wait_SPI_IS_BUSY(SPI_T *spi)
{
    uint32_t u32TimeOutCnt = SystemCoreClock / 10;

    while (SPI_IS_BUSY(spi))
    {
        if (--u32TimeOutCnt == 0)
        {
            printf("Wait for SPI time-out!\n");
            break;
        }
    }
}

__STATIC_INLINE int32_t SpiFlash_WaitReady(void)
{
    uint8_t u8ReturnValue;
    uint32_t u32TimeOutCnt = SystemCoreClock / 10;

    do
    {
        if (--u32TimeOutCnt == 0)
        {
            printf("Wait for SPI time-out!\n");
            return -1;
        }

        CMD_RDSR(&u8ReturnValue);
        u8ReturnValue = u8ReturnValue & 1;
    } while (u8ReturnValue != 0);  // check the BUSY bit

    return 0;
}
#endif

void mx25r4035f_init(void)
{
    MSG05("mx25r4035f_init()");

    mx25r4035f_interface_init();
    CMD_WRDI();

    uint32_t rdid;
    CMD_RDID((uint32_t *)&rdid);
    DPRINTF(DBG_TRACE, "READ RDID 0x%06X\r\n", (unsigned int)rdid);

    uint8_t res;
    CMD_RES((uint8_t *)&res);
    DPRINTF(DBG_TRACE, "READ RES 0x%X\r\n", res);

    uint16_t rems;
    CMD_REMS(&rems);
    DPRINTF(DBG_TRACE, "READ REMS 0x%04X\r\n", rems);

    uint8_t rdsr;
    CMD_RDSR((uint8_t *)&rdsr);
    DPRINTF(DBG_TRACE, "READ RDSR 0x%X\r\n", rdsr);
}

/*
 * Function:       IsFlashBusy
 * Arguments:      None.
 * Description:    Check status register WIP bit.
 *                 If  WIP bit = 1: return TRUE ( Busy )
 *                             = 0: return FALSE ( Ready ).
 * Return Message: TRUE, FALSE
 */
bool IsFlashBusy(void)
{
    MSG05("IsFlashBusy()");

    uint8_t gDataBuffer;
    CMD_RDSR(&gDataBuffer);
    if ((gDataBuffer & FLASH_WIP_MASK) == FLASH_WIP_MASK)
        return TRUE;
    else
        return FALSE;
}

void CMD_WREN(void)
{
    MSG05("CMD_WREN()");
    SPI_SET_SS_LOW(SPI0);
    SPI_WRITE_TX(SPI0, FLASH_CMD_WREN);
    wait_SPI_IS_BUSY(SPI0);
    SPI_SET_SS_HIGH(SPI0);
}

/*
 *   @brief write disable
 */
void CMD_WRDI(void)
{
    MSG05("CMD_WRDI()");
    SPI_SET_SS_LOW(SPI0);
    SPI_WRITE_TX(SPI0, FLASH_CMD_WRDI);
    wait_SPI_IS_BUSY(SPI0);
    SPI_SET_SS_HIGH(SPI0);
}

/*
 * ID Command
 */

/*
 * Function:       CMD_RDID
 * Arguments:      Identification, 32 bit buffer to store id
 * Description:    The RDID instruction is to read the manufacturer ID
 *                 of 1-byte and followed by Device ID of 2-byte.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDID(uint32_t *Identification)
{
    MSG05("CMD_RDID()");

    uint8_t u8RxData[6] = {0}, u8IDCnt = 0;
    uint32_t temp;

    SPI_ClearRxFIFO(SPI0);

    // /CS: active
    SPI_SET_SS_LOW(SPI0);

    SPI_WRITE_TX(SPI0, FLASH_CMD_RDID);

    // send 24-bit '0', dummy
    SPI_WRITE_TX(SPI0, 0x00);
    SPI_WRITE_TX(SPI0, 0x00);
    SPI_WRITE_TX(SPI0, 0x00);

    // wait tx finish
    wait_SPI_IS_BUSY(SPI0);

    // /CS: de-active
    SPI_SET_SS_HIGH(SPI0);

    while (!SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0))
        u8RxData[u8IDCnt++] = (uint8_t)SPI_READ_RX(SPI0);

    // Store identification
    temp = (uint32_t)u8RxData[1];
    temp = (temp << 8) | (uint32_t)u8RxData[2];
    *Identification = (temp << 8) | (uint32_t)u8RxData[3];

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_RES
 * Arguments:      ElectricIdentification, 8 bit buffer to store electric id
 * Description:    The RES instruction is to read the Device
 *                 electric identification of 1-byte.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RES(uint8_t *ElectricIdentification)
{
    MSG05("CMD_RES()");

    uint8_t u8RxData[6] = {0}, u8IDCnt = 0;

    SPI_ClearRxFIFO(SPI0);

    // /CS: active
    SPI_SET_SS_LOW(SPI0);

    SPI_WRITE_TX(SPI0, FLASH_CMD_RES);

    // send 24-bit '0', dummy
    SPI_WRITE_TX(SPI0, 0x00);
    SPI_WRITE_TX(SPI0, 0x00);
    SPI_WRITE_TX(SPI0, 0x00);

    SPI_WRITE_TX(SPI0, 0x00);

    // wait tx finish
    wait_SPI_IS_BUSY(SPI0);

    // /CS: de-active
    SPI_SET_SS_HIGH(SPI0);

    while (!SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0))
        u8RxData[u8IDCnt++] = (uint8_t)SPI_READ_RX(SPI0);

    *ElectricIdentification = u8RxData[4];
    MSG05("count:%d, data:%02X", u8IDCnt, u8RxData[4]);

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_REMS
 * Arguments:      REMS_Identification, 16 bit buffer to store id
 *                 fsptr, pointer of flash status structure
 * Description:    The REMS instruction is to read the Device
 *                 manufacturer ID and electric ID of 1-byte.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_REMS(uint16_t *REMS_Identification)
{
    MSG05("CMD_REMS()");
    uint8_t u8RxData[6] = {0}, u8IDCnt = 0;

    SPI_ClearRxFIFO(SPI0);

    // /CS: active
    SPI_SET_SS_LOW(SPI0);

    // send Command: 0x90, Read Manufacturer/Device ID
    SPI_WRITE_TX(SPI0, FLASH_CMD_REMS);

    // send 24-bit '0', dummy
    SPI_WRITE_TX(SPI0, 0x00);
    SPI_WRITE_TX(SPI0, 0x00);
    SPI_WRITE_TX(SPI0, 0x00);

    // receive 16-bit
    SPI_WRITE_TX(SPI0, 0x00);
    SPI_WRITE_TX(SPI0, 0x00);

    // wait tx finish
    wait_SPI_IS_BUSY(SPI0);

    // /CS: de-active
    SPI_SET_SS_HIGH(SPI0);

    while (!SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0))
        u8RxData[u8IDCnt++] = (uint8_t)SPI_READ_RX(SPI0);

    *REMS_Identification = (uint16_t)(u8RxData[4] << 8) | u8RxData[5];

    return FlashOperationSuccess;
}

/*
 * Register  Command
 */

/*
 * Function:       CMD_RDSR
 * Arguments:      StatusReg, 8 bit buffer to store status register value
 * Description:    The RDSR instruction is for reading Status Register Bits.
 * Return Message: FlashOperationSuccess
 */
ReturnMsg CMD_RDSR(uint8_t *StatusReg)
{
    MSG05("CMD_RDSR()");

    uint8_t u8RxData[6] = {0}, u8IDCnt = 0;

    SPI_ClearRxFIFO(SPI0);

    // /CS: active
    SPI_SET_SS_LOW(SPI0);

    SPI_WRITE_TX(SPI0, FLASH_CMD_RDSR);

    SPI_WRITE_TX(SPI0, 0x00);

    // wait tx finish
    wait_SPI_IS_BUSY(SPI0);

    // /CS: de-active
    SPI_SET_SS_HIGH(SPI0);

    while (!SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0))
        u8RxData[u8IDCnt++] = (uint8_t)SPI_READ_RX(SPI0);

    *StatusReg = u8RxData[1];

    return FlashOperationSuccess;
}

#define BUSY_CHECK()       \
    if (IsFlashBusy())     \
    {                      \
        ret = FlashIsBusy; \
        goto complete;     \
    }

/*
 * Erase Command
 */

/*
 * Function:       CMD_SE
 * Arguments:      flash_address, 32 bit flash memory address
 * Description:    The SE instruction is for erasing the data
 *                 of the chosen sector (4KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
uint8_t CMD_SE(uint32_t flash_address)
{
    ReturnMsg ret;
    MSG05("CMD_SE()");

    // Check flash address
    if (flash_address > FlashSize)
        return FlashAddressInvalid;
    if ((flash_address & 0x00000FFF) != 0)
        return FlashAddressInvalid;

    BUSY_CHECK();

    CMD_WREN();

    SPI_SET_SS_LOW(SPI0);

    SPI_WRITE_TX(SPI0, FLASH_CMD_SE);

    SPI_WRITE_TX(SPI0, (flash_address >> 16) & 0xFF);
    SPI_WRITE_TX(SPI0, (flash_address >> 8) & 0xFF);
    SPI_WRITE_TX(SPI0, flash_address & 0xFF);

    wait_SPI_IS_BUSY(SPI0);

    SPI_SET_SS_HIGH(SPI0);

    if (SpiFlash_WaitReady() < 0)
    {
        ret = FlashTimeOut;
    }
    else
    {
        ret = FlashOperationSuccess;
    }

complete:;
    if (ret != FlashOperationSuccess)
    {
        DPRINTF(DBG_ERR, "Flash Sector Erase Failed ret=%d\r\n", ret);
    }

    return ret;
}

/*
 * Function:       CMD_BE32K
 * Arguments:      flash_address, 32 bit flash memory address
 * Description:    The BE32K instruction is for erasing the data
 *                 of the chosen sector (32KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_BE32K(uint32_t flash_address)
{
    ReturnMsg ret;
    MSG05("CMD_BE32K()");

    // Check flash address
    if (flash_address > FlashSize)
        return FlashAddressInvalid;
    if ((flash_address & 0x00007FFF) != 0)
        return FlashAddressInvalid;

    BUSY_CHECK();

    CMD_WREN();

    SPI_SET_SS_LOW(SPI0);

    SPI_WRITE_TX(SPI0, FLASH_CMD_BE32K);

    SPI_WRITE_TX(SPI0, (flash_address >> 16) & 0xFF);
    SPI_WRITE_TX(SPI0, (flash_address >> 8) & 0xFF);
    SPI_WRITE_TX(SPI0, flash_address & 0xFF);

    wait_SPI_IS_BUSY(SPI0);

    SPI_SET_SS_HIGH(SPI0);

    if (SpiFlash_WaitReady() < 0)
    {
        ret = FlashTimeOut;
    }
    else
    {
        ret = FlashOperationSuccess;
    }

complete:;
    if (ret != FlashOperationSuccess)
    {
        DPRINTF(DBG_ERR, "Flash Block(32K) Erase Failed ret=%d\r\n", ret);
    }

    return ret;
}

/*
 * Function:       CMD_CE
 * Arguments:      None.
 * Description:    The CE instruction is for erasing the data
 *                 of the whole chip to be "1".
 * Return Message: FlashIsBusy, FlashOperationSuccess, FlashTimeOut
 */
ReturnMsg CMD_CE(void)
{
    MSG05("CMD_CE()");
    ReturnMsg ret;

    BUSY_CHECK();

    CMD_WREN();

    SPI_SET_SS_LOW(SPI0);

    SPI_WRITE_TX(SPI0, FLASH_CMD_CE);

    wait_SPI_IS_BUSY(SPI0);

    SPI_SET_SS_HIGH(SPI0);

    if (SpiFlash_WaitReady() < 0)
    {
        ret = FlashTimeOut;
    }
    else
    {
        ret = FlashOperationSuccess;
    }

complete:;
    if (ret != FlashOperationSuccess)
    {
        DPRINTF(DBG_ERR, "Flash Chip Erase Failed ret=%d\r\n", ret);
    }

    return ret;
}

/*
 * Read Command
 */

/*
 * Function:       CMD_READ
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    The READ instruction is for reading data out.
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_READ(uint32_t flash_address, uint8_t *target_address,
                   uint32_t byte_length)
{
    MSG05("CMD_READ()");
    uint32_t index;

    // Check flash address
    if (flash_address > FlashSize)
        return FlashAddressInvalid;

    SPI_SET_SS_LOW(SPI0);

    SPI_WRITE_TX(SPI0, FLASH_CMD_READ);

    // send 24-bit start address
    SPI_WRITE_TX(SPI0, (flash_address >> 16) & 0xFF);
    SPI_WRITE_TX(SPI0, (flash_address >> 8) & 0xFF);
    SPI_WRITE_TX(SPI0, flash_address & 0xFF);

    wait_SPI_IS_BUSY(SPI0);

    SPI_ClearRxFIFO(SPI0);

    for (index = 0; index < byte_length; index++)
    {
        SPI_WRITE_TX(SPI0, 0x00);
        wait_SPI_IS_BUSY(SPI0);
        *(target_address + index) = (uint8_t)SPI_READ_RX(SPI0);
    }

    // wait tx finish
    wait_SPI_IS_BUSY(SPI0);

    // /CS: de-active
    SPI_SET_SS_HIGH(SPI0);

    return FlashOperationSuccess;
}

/*
 * Read Command
 */

/*
 * Function:       CMD_READ
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    The READ instruction is for reading data out.
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_FASTREAD(uint32_t flash_address, uint8_t *target_address,
                       uint32_t byte_length)
{
    MSG05("CMD_FASTREAD()");

    uint32_t index;

    // Check flash address
    if (flash_address > FlashSize)
        return FlashAddressInvalid;

    SPI_SET_SS_LOW(SPI0);

    SPI_WRITE_TX(SPI0, FLASH_CMD_FASTREAD);

    // send 24-bit start address
    SPI_WRITE_TX(SPI0, (flash_address >> 16) & 0xFF);
    SPI_WRITE_TX(SPI0, (flash_address >> 8) & 0xFF);
    SPI_WRITE_TX(SPI0, flash_address & 0xFF);
    SPI_WRITE_TX(SPI0, 0x00);  // dummy byte Send

    wait_SPI_IS_BUSY(SPI0);

    SPI_ClearRxFIFO(SPI0);

    for (index = 0; index < byte_length; index++)
    {
        SPI_WRITE_TX(SPI0, 0x00);
        wait_SPI_IS_BUSY(SPI0);
        *(target_address + index) = (uint8_t)SPI_READ_RX(SPI0);
    }

    // wait tx finish
    wait_SPI_IS_BUSY(SPI0);

    // /CS: de-active
    SPI_SET_SS_HIGH(SPI0);

    return FlashOperationSuccess;
}

/*
 * Function:       CMD_PP
 * Arguments:      flash_address, 32 bit flash memory address
 *                 source_address, buffer address of source data to program
 *                 byte_length, byte length of data to programm
 * Description:    The PP instruction is for programming
 *                 the memory to be "0".
 *                 The device only accept the last 256 byte ( or 32 byte ) to
 * program. If the page address ( flash_address[7:0] ) reach 0xFF, it will
 *                 program next at 0x00 of the same page.
 *                 Some products have smaller page size ( 32 byte )
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_PP(uint32_t flash_address, uint8_t *source_address,
                 uint32_t byte_length)
{
    MSG05("CMD_PP()");
    uint32_t index = 0;
    uint8_t addr_4byte_mode;
    ReturnMsg ret;

    // Check flash address
    if (flash_address > FlashSize)
        return FlashAddressInvalid;

    BUSY_CHECK();

    CMD_WREN();

    SPI_SET_SS_LOW(SPI0);

    SPI_WRITE_TX(SPI0, FLASH_CMD_PP);

    SPI_WRITE_TX(SPI0, (flash_address >> 16) & 0xFF);
    SPI_WRITE_TX(SPI0, (flash_address >> 8) & 0xFF);
    SPI_WRITE_TX(SPI0, flash_address & 0xFF);

    while (1)
    {
        if (!SPI_GET_TX_FIFO_FULL_FLAG(SPI0))
        {
            SPI_WRITE_TX(SPI0, *(source_address + index++));
            if (index > (byte_length - 1))
                break;
        }
    }

    wait_SPI_IS_BUSY(SPI0);

    SPI_SET_SS_HIGH(SPI0);

    if (SpiFlash_WaitReady() < 0)
    {
        ret = FlashTimeOut;
    }
    else
    {
        ret = FlashOperationSuccess;
    }

complete:;
    if (ret != FlashOperationSuccess)
    {
        DPRINTF(DBG_ERR, "Flash Page Program Failed ret=%d\r\n", ret);
    }

    return ret;
}

void dsm_sflash_fw_erase(uint8_t type)
{
    MSG05("dsm_sflash_fw_erase()");

    uint32_t cnt;
    uint32_t start_blk_offset = 0;
    uint32_t blk_cnt = 0;

    switch (type)
    {
    case E_SFLASH_SYS_INFO_T:
        start_blk_offset = SFLASH_SYS_INFO_BLK_OFFSET;
        blk_cnt = SFLASH_SYS_INFO_BLK_CNT;

        break;
    case E_SFLASH_SYS_FW_1_T:
        start_blk_offset = SFLASH_SYS_FW_1_BLK_OFFSET;
        blk_cnt = SFLASH_SYS_FW_1_BLK_CNT;

        break;
    case E_SFLASH_SYS_FW_2_T:
        start_blk_offset = SFLASH_SYS_FW_2_BLK_OFFSET;
        blk_cnt = SFLASH_SYS_FW_2_BLK_CNT;

        break;
    case E_SFLASH_I_MODEM_FW_T:
        start_blk_offset = SFLASH_I_MODEM_FW_BLK_OFFSET;
        blk_cnt = SFLASH_I_MODEM_FW_BLK_CNT;

        break;
    case E_SFLASH_EXT_MODEM_FW_T:
        start_blk_offset = SFLASH_EXT_MODEM_FW_BLK_OFFSET;
        blk_cnt = SFLASH_EXT_MODEM_FW_BLK_CNT;

        break;
    case E_SFLASH_METER_FW_BLK_T:
        start_blk_offset = SFLASH_METER_FW_BLK_OFFSET;
        blk_cnt = SFLASH_METER_FW_BLK_CNT;

        break;
    }

    DPRINTF(DBG_TRACE, "%s: type[%d], start_offset[%d], blk_cnt[%d]\r\n",
            __func__, type, start_blk_offset, blk_cnt);
    for (cnt = 0; cnt < blk_cnt; cnt++)
    {
        CMD_BE32K((cnt + start_blk_offset) * Block_Offset);
        CMD_BE32K(((cnt + start_blk_offset) * Block_Offset) + Block32K_Offset);
    }
}

uint32_t dsm_sflash_fw_get_startaddr(uint8_t type)
{
    MSG05("dsm_sflash_fw_get_startaddr()");

    uint32_t start_addr = 0;

    switch (type)
    {
    case E_SFLASH_SYS_INFO_T:
        start_addr = SFLASH_SYS_INFO_ADDR;
        break;

    case E_SFLASH_SYS_FW_1_T:
        start_addr = SFLASH_SYS_FW_1_ADDR;
        break;

    case E_SFLASH_SYS_FW_2_T:
        start_addr = SFLASH_SYS_FW_2_ADDR;
        break;

    case E_SFLASH_I_MODEM_FW_T:
        start_addr = SFLASH_I_MODEM_FW_ADDR;
        break;

    case E_SFLASH_EXT_MODEM_FW_T:
        start_addr = SFLASH_EXT_MODEM_FW_ADDR;
        break;

    case E_SFLASH_METER_FW_BLK_T:
        start_addr = SFLASH_METER_FW_BLK_ADDR;

        break;
    }

    DPRINTF(DBG_TRACE, "%s: type[%d], start_addr[0x%08X]\r\n", __func__, type,
            start_addr);

    return start_addr;
}
