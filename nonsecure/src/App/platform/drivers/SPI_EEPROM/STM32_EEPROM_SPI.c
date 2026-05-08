#if EEPROM_TYPE == SPI
#include "STM32_EEPROM_SPI.h"
#include "amg_gpio.h"
#include "amg_spi.h"
#include "amg_debug.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "string.h"

#ifdef M2354_NEW_HW
#define CS_M PA5
#else
#define CS_M PB1
#endif

#define BUSY_CHECK()                 \
    if (IsFlashBusy())               \
    {                                \
        ret = EEPROM_STATUS_PENDING; \
        goto complete;               \
    }

static EepromOperations wait_SPI_IS_BUSY(SPI_T* spi)
{
    uint32_t u32TimeOutCnt = SystemCoreClock / 10;

    while (SPI_IS_BUSY(spi))
    {
        if (--u32TimeOutCnt == 0)
        {
            MSGERROR("Wait for SPI time-out!\n");
            return EEPROM_STATUS_ERROR;
        }
    }

    return EEPROM_STATUS_COMPLETE;
}

static EepromOperations CMD_WREN(void)
{
    MSG00("SPI-EEPROM, CMD_WREN()");
    CS_M = 0;
    SPI_WRITE_TX(SPI0, EEPROM_WREN);
    if (wait_SPI_IS_BUSY(SPI0) != EEPROM_STATUS_COMPLETE)
    {
        CS_M = 1;
        DPRINTF(DBG_ERR, "SPI-EEPROM CMD_WREN timeout\r\n");
        return EEPROM_STATUS_ERROR;
    }
    CS_M = 1;

    return EEPROM_STATUS_COMPLETE;
}

static EepromOperations CMD_WRDI(void)
{
    MSG00("SPI-EEPROM, CMD_WRDI()");
    CS_M = 0;
    SPI_WRITE_TX(SPI0, EEPROM_WRDI);
    if (wait_SPI_IS_BUSY(SPI0) != EEPROM_STATUS_COMPLETE)
    {
        CS_M = 1;
        DPRINTF(DBG_ERR, "SPI-EEPROM CMD_WRDI timeout\r\n");
        return EEPROM_STATUS_ERROR;
    }
    CS_M = 1;

    return EEPROM_STATUS_COMPLETE;
}

static EepromOperations CMD_RDSR(uint8_t* StatusReg)
{
    MSG00("SPI-EEPROM, CMD_RDSR()");

    uint8_t u8RxData[6] = {0}, u8IDCnt = 0;

    SPI_ClearRxFIFO(SPI0);

    // CS: active
    CS_M = 0;

    SPI_WRITE_TX(SPI0, EEPROM_RDSR);

    SPI_WRITE_TX(SPI0, 0x00);

    // wait tx finish
    if (wait_SPI_IS_BUSY(SPI0) != EEPROM_STATUS_COMPLETE)
    {
        CS_M = 1;
        DPRINTF(DBG_ERR, "SPI-EEPROM CMD_RDSR timeout\r\n");
        return EEPROM_STATUS_ERROR;
    }

    // CS: de-active
    CS_M = 1;

    while (!SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0))
        u8RxData[u8IDCnt++] = (uint8_t)SPI_READ_RX(SPI0);

    *StatusReg = u8RxData[1];

    return EEPROM_STATUS_COMPLETE;
}

static int32_t SpiFlash_WaitReady(void)
{
    uint8_t u8ReturnValue;
    uint32_t u32TimeOutCnt = SystemCoreClock / 10;

    do
    {
        if (--u32TimeOutCnt == 0)
        {
            MSGERROR("Wait for SPI time-out!\n");
            return -1;
        }

        if (CMD_RDSR(&u8ReturnValue) != EEPROM_STATUS_COMPLETE)
        {
            return -1;
        }
        u8ReturnValue = u8ReturnValue & 1;
    } while (u8ReturnValue != 0);  // check the BUSY bit

    return 0;
}

static bool IsFlashBusy(void)
{
    MSG00("SPI-EEPROM, IsFlashBusy()");

    uint8_t gDataBuffer;
    if (CMD_RDSR(&gDataBuffer) != EEPROM_STATUS_COMPLETE)
    {
        DPRINTF(DBG_ERR, "SPI-EEPROM IsFlashBusy: CMD_RDSR failed\r\n");
        return TRUE;
    }
    if ((gDataBuffer & EEPROM_WIP_FLAG) == EEPROM_WIP_FLAG)
        return TRUE;
    else
        return FALSE;
}

void dsm_spi_eeprom_init(void)
{
    MSG06("dsm_spi_eeprom_init()");

    dsm_spi_init();

    dsm_gpio_spi_e2p_pwr_en_port_init();

    dsm_gpio_spi_eeprom_pwr_enable();
    vTaskDelay(10);

    // CS: de-active
#ifdef M2354_NEW_HW
    GPIO_SetMode(PA, BIT5, GPIO_MODE_OUTPUT);
#else
    GPIO_SetMode(PB, BIT1, GPIO_MODE_OUTPUT);
#endif
    CS_M = 1;
}

EepromOperations EEPROM_SPI_WritePage(uint8_t _erase, uint8_t* pBuffer,
                                      uint32_t WriteAddr,
                                      uint16_t NumByteToWrite)
{
    MSG00("EEPROM_SPI_WritePage()__%d", NumByteToWrite);
    uint32_t index = 0;
    EepromOperations ret;

    if (NumByteToWrite == 0)
    {
        return EEPROM_STATUS_COMPLETE;
    }

    BUSY_CHECK();

    if (CMD_WREN() != EEPROM_STATUS_COMPLETE)
    {
        ret = EEPROM_STATUS_ERROR;
        goto complete;
    }

    CS_M = 0;

    SPI_WRITE_TX(SPI0, EEPROM_WRITE);
    SPI_WRITE_TX(SPI0, (WriteAddr >> 16) & 0xFF);
    SPI_WRITE_TX(SPI0, (WriteAddr >> 8) & 0xFF);
    SPI_WRITE_TX(SPI0, WriteAddr & 0xFF);

    uint32_t tx_fifo_timeout = SystemCoreClock / 10;
    while (1)
    {
        if (!SPI_GET_TX_FIFO_FULL_FLAG(SPI0))
        {
            SPI_WRITE_TX(SPI0, *(pBuffer + index++));
            tx_fifo_timeout = SystemCoreClock / 10;
            if (index > (NumByteToWrite - 1))
                break;
        }
        else if (--tx_fifo_timeout == 0)
        {
            CS_M = 1;
            (void)CMD_WRDI();
            DPRINTF(DBG_ERR,
                    "SPI-EEPROM WritePage TX FIFO timeout addr=0x%lX\r\n",
                    (unsigned long)WriteAddr);
            ret = EEPROM_STATUS_ERROR;
            goto complete;
        }
    }

    if (wait_SPI_IS_BUSY(SPI0) != EEPROM_STATUS_COMPLETE)
    {
        CS_M = 1;
        (void)CMD_WRDI();
        ret = EEPROM_STATUS_ERROR;
        goto complete;
    }

    CS_M = 1;

    if (SpiFlash_WaitReady() < 0)
    {
        ret = EEPROM_STATUS_ERROR;
    }
    else
    {
        ret = EEPROM_STATUS_COMPLETE;
    }

    (void)CMD_WRDI();

complete:;
    if (ret != EEPROM_STATUS_COMPLETE)
    {
        DPRINTF(DBG_ERR, "EEPROM_SPI, Flash Page Program Failed ret=%d\r\n",
                ret);
    }

    return ret;
#if 0
    while (EEPROM_SPI->State != HAL_SPI_STATE_READY)
    {
        osDelay(1);
    }

    HAL_StatusTypeDef spiTransmitStatus;

    sEE_WriteEnable();

    /*
        We gonna send commands in one packet of 3 bytes
     */
    uint8_t header[4];

    header[0] = EEPROM_WRITE;     // Send "Write to Memory" instruction
    header[1] = WriteAddr >> 16;  // Send 24-bit address
    header[2] = WriteAddr >> 8;   // Send 16-bit address
    header[3] = WriteAddr;

    // Select the EEPROM: Chip Select low
    EEPROM_CS_LOW();

    if (_erase)
    {
        memset(pBuffer, 0xff, NumByteToWrite);
    }

    EEPROM_SPI_SendInstruction((uint8_t*)header, 4);

    // Make 5 attemtps to write the data
    for (uint8_t i = 0; i < 5; i++)
    {
        spiTransmitStatus =
            HAL_SPI_Transmit(EEPROM_SPI, pBuffer, NumByteToWrite, 100);

        if (spiTransmitStatus == HAL_BUSY)
        {
            osDelay(5);
        }
        else
        {
            break;
        }
    }

    // Deselect the EEPROM: Chip Select high
    EEPROM_CS_HIGH();

    // Wait the end of EEPROM writing
    EEPROM_SPI_WaitStandbyState();

    // Disable the write access to the EEPROM
    sEE_WriteDisable();

    if (spiTransmitStatus == HAL_ERROR)
    {
        return EEPROM_STATUS_ERROR;
    }
    else
    {
        return EEPROM_STATUS_COMPLETE;
    }
#endif
}

EepromOperations EEPROM_SPI_WriteBuffer(uint8_t _erase, uint8_t* pBuffer,
                                        uint32_t WriteAddr,
                                        uint32_t NumByteToWrite)
{
    uint32_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    uint16_t sEE_DataNum = 0;

    EepromOperations pageWriteStatus = EEPROM_STATUS_PENDING;

    Addr = WriteAddr % EEPROM_PAGESIZE;
    count = EEPROM_PAGESIZE - Addr;
    NumOfPage = NumByteToWrite / EEPROM_PAGESIZE;
    NumOfSingle = NumByteToWrite % EEPROM_PAGESIZE;

    if (Addr == 0)
    { /* WriteAddr is EEPROM_PAGESIZE aligned  */
        if (NumOfPage == 0)
        { /* NumByteToWrite < EEPROM_PAGESIZE */
            sEE_DataNum = NumByteToWrite;
            //   DPRINTF(DBG_NONE, "%s: addr[%d], length[%d],
            //   lenInPage[%d]\r\n", __func__, WriteAddr, NumByteToWrite,
            //   sEE_DataNum);
            pageWriteStatus =
                EEPROM_SPI_WritePage(_erase, pBuffer, WriteAddr, sEE_DataNum);

            if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
            {
                return pageWriteStatus;
            }
        }
        else
        { /* NumByteToWrite > EEPROM_PAGESIZE */
            while (NumOfPage--)
            {
                sEE_DataNum = EEPROM_PAGESIZE;
                //	   DPRINTF(DBG_NONE, "%s: addr[%d], length[%d],
                // lenInPage[%d]\r\n", __func__, WriteAddr, NumByteToWrite,
                // sEE_DataNum);
                pageWriteStatus = EEPROM_SPI_WritePage(_erase, pBuffer,
                                                       WriteAddr, sEE_DataNum);

                if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
                {
                    return pageWriteStatus;
                }

                WriteAddr += EEPROM_PAGESIZE;
                if (_erase == FALSE)
                    pBuffer += EEPROM_PAGESIZE;
            }

            if (NumOfSingle != 0)
            {
                sEE_DataNum = NumOfSingle;
                // DPRINTF(DBG_NONE, "%s: addr[%d], length[%d], lenInPage[%d]\r\n",
                // __func__, WriteAddr, NumByteToWrite, sEE_DataNum);
                pageWriteStatus = EEPROM_SPI_WritePage(_erase, pBuffer,
                                                       WriteAddr, sEE_DataNum);

                if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
                {
                    return pageWriteStatus;
                }
            }
        }
    }
    else
    { /* WriteAddr is not EEPROM_PAGESIZE aligned  */
        if (NumOfPage == 0)
        { /* NumByteToWrite < EEPROM_PAGESIZE */
            if (NumOfSingle > count)
            { /* (NumByteToWrite + WriteAddr) > EEPROM_PAGESIZE */
                temp = NumOfSingle - count;
                sEE_DataNum = count;
                // DPRINTF(DBG_NONE, "%s: addr[%d], length[%d],
                // lenInPage[%d]\r\n", __func__, WriteAddr, NumByteToWrite,
                // sEE_DataNum);
                pageWriteStatus = EEPROM_SPI_WritePage(_erase, pBuffer,
                                                       WriteAddr, sEE_DataNum);

                if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
                {
                    return pageWriteStatus;
                }

                WriteAddr += count;
                if (_erase == FALSE)
                    pBuffer += count;

                sEE_DataNum = temp;
                //   DPRINTF(DBG_NONE, "%s: addr[%d], length[%d],
                //   lenInPage[%d]\r\n", __func__, WriteAddr, NumByteToWrite,
                //   sEE_DataNum);
                pageWriteStatus = EEPROM_SPI_WritePage(_erase, pBuffer,
                                                       WriteAddr, sEE_DataNum);
            }
            else
            {
                sEE_DataNum = NumByteToWrite;
                //   DPRINTF(DBG_NONE, "%s: addr[%d], length[%d],
                //   lenInPage[%d]\r\n", __func__, WriteAddr, NumByteToWrite,
                //   sEE_DataNum);
                pageWriteStatus = EEPROM_SPI_WritePage(_erase, pBuffer,
                                                       WriteAddr, sEE_DataNum);
            }

            if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
            {
                return pageWriteStatus;
            }
        }
        else
        { /* NumByteToWrite > EEPROM_PAGESIZE */
            NumByteToWrite -= count;
            NumOfPage = NumByteToWrite / EEPROM_PAGESIZE;
            NumOfSingle = NumByteToWrite % EEPROM_PAGESIZE;

            sEE_DataNum = count;
            //  DPRINTF(DBG_NONE, "%s: addr[%d], length[%d], lenInPage[%d]\r\n",
            //  __func__, WriteAddr, NumByteToWrite, sEE_DataNum);
            pageWriteStatus =
                EEPROM_SPI_WritePage(_erase, pBuffer, WriteAddr, sEE_DataNum);

            if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
            {
                return pageWriteStatus;
            }

            WriteAddr += count;
            if (_erase == FALSE)
                pBuffer += count;

            while (NumOfPage--)
            {
                sEE_DataNum = EEPROM_PAGESIZE;
                //   DPRINTF(DBG_NONE, "%s: addr[%d], length[%d],
                //   lenInPage[%d]\r\n", __func__, WriteAddr, NumByteToWrite,
                //   sEE_DataNum);
                pageWriteStatus = EEPROM_SPI_WritePage(_erase, pBuffer,
                                                       WriteAddr, sEE_DataNum);

                if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
                {
                    return pageWriteStatus;
                }

                WriteAddr += EEPROM_PAGESIZE;
                if (_erase == FALSE)
                    pBuffer += EEPROM_PAGESIZE;
            }

            if (NumOfSingle != 0)
            {
                sEE_DataNum = NumOfSingle;
                //   DPRINTF(DBG_NONE, "%s: addr[%d], length[%d],
                //   lenInPage[%d]\r\n", __func__, WriteAddr, NumByteToWrite,
                //   sEE_DataNum);
                pageWriteStatus = EEPROM_SPI_WritePage(_erase, pBuffer,
                                                       WriteAddr, sEE_DataNum);

                if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
                {
                    return pageWriteStatus;
                }
            }
        }
    }

    return EEPROM_STATUS_COMPLETE;
}

EepromOperations EEPROM_SPI_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr,
                                       uint32_t NumByteToRead)
{
    MSG00("EEPROM_SPI_ReadBuffer()");
    uint32_t index;

    CS_M = 0;

    SPI_WRITE_TX(SPI0, EEPROM_READ);

    // send 24-bit start address
    SPI_WRITE_TX(SPI0, (ReadAddr >> 16) & 0xFF);
    SPI_WRITE_TX(SPI0, (ReadAddr >> 8) & 0xFF);
    SPI_WRITE_TX(SPI0, ReadAddr & 0xFF);

    if (wait_SPI_IS_BUSY(SPI0) != EEPROM_STATUS_COMPLETE)
    {
        CS_M = 1;
        DPRINTF(DBG_ERR, "SPI-EEPROM ReadBuffer command timeout\r\n");
        return EEPROM_STATUS_ERROR;
    }

    SPI_ClearRxFIFO(SPI0);

    for (index = 0; index < NumByteToRead; index++)
    {
        SPI_WRITE_TX(SPI0, 0x00);
        if (wait_SPI_IS_BUSY(SPI0) != EEPROM_STATUS_COMPLETE)
        {
            CS_M = 1;
            DPRINTF(DBG_ERR,
                    "SPI-EEPROM ReadBuffer data timeout addr=0x%lX index=%lu\r\n",
                    (unsigned long)ReadAddr, (unsigned long)index);
            return EEPROM_STATUS_ERROR;
        }
        *(pBuffer + index) = (uint8_t)SPI_READ_RX(SPI0);
    }

    // wait tx finish
    if (wait_SPI_IS_BUSY(SPI0) != EEPROM_STATUS_COMPLETE)
    {
        CS_M = 1;
        DPRINTF(DBG_ERR, "SPI-EEPROM ReadBuffer final timeout\r\n");
        return EEPROM_STATUS_ERROR;
    }

    CS_M = 1;

    return EEPROM_STATUS_COMPLETE;
#if 0
    while (EEPROM_SPI->State != HAL_SPI_STATE_READY)
    {
        osDelay(1);
    }

    /*
        We gonna send all commands in one packet of 3 bytes
     */

    uint8_t header[4];

    header[0] = EEPROM_READ;  // Send "Read from Memory" instruction
    header[1] = ReadAddr >> 16;
    header[2] = ReadAddr >> 8;  // Send 16-bit address
    header[3] = ReadAddr;

    // Select the EEPROM: Chip Select low
    EEPROM_CS_LOW();

    /* Send WriteAddr address byte to read from */
    EEPROM_SPI_SendInstruction(header, 4);

    while (HAL_SPI_Receive(EEPROM_SPI, (uint8_t*)pBuffer, NumByteToRead, 200) ==
           HAL_BUSY)
    {
        osDelay(1);
    };

    // Deselect the EEPROM: Chip Select high
    EEPROM_CS_HIGH();

    return EEPROM_STATUS_COMPLETE;
#endif
}

uint32_t dsm_spi_eep_write(uint8_t _erase, uint32_t eeprom_addr,
                           uint8_t* pwdata, uint32_t length)
{
    EepromOperations pageWriteStatus = EEPROM_STATUS_PENDING;

    pageWriteStatus =
        EEPROM_SPI_WriteBuffer(_erase, pwdata, eeprom_addr, length);

    if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
    {
        DPRINTF(DBG_ERR, "%s: error 3!!!\r\n", __func__);
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint32_t dsm_spi_eep_read(uint32_t eeprom_addr, uint8_t* prdata,
                          uint32_t length)
{
    EepromOperations pageWriteStatus = EEPROM_STATUS_PENDING;

    pageWriteStatus = EEPROM_SPI_ReadBuffer(prdata, eeprom_addr, length);

    if (pageWriteStatus != EEPROM_STATUS_COMPLETE)
    {
        DPRINTF(DBG_ERR, "%s: error 3!!!\r\n", __func__);
        return HAL_ERROR;
    }

    return HAL_OK;
}

uint32_t dsm_spi_eep_erase(uint32_t eeprom_addr, uint32_t length)
{
    uint8_t wdata[EEPROM_PAGESIZE];
    memset(wdata, 0xFF, EEPROM_PAGESIZE);

    uint32_t ret = dsm_spi_eep_write(TRUE, eeprom_addr, wdata, length);
    if (ret != HAL_OK)
    {
        DPRINTF(DBG_ERR, "%s: error !!!\r\n", __func__);
        return HAL_ERROR;
    }

    return HAL_OK;
}
#endif
