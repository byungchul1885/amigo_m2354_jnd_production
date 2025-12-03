#include "main.h"
#include "eeprom_at24cm02.h"
#include "amg_gpio.h"
#include "amg_wdt.h"
#if EEPROM_TYPE == SPI
#include "STM32_EEPROM_SPI.h"
#endif

void dsm_eeprom_init(void)
{
#if EEPROM_TYPE == SPI
    dsm_spi_eeprom_init();
#else
    MSG00("dsm_eeprom_init()");

    I2C_Close(I2C1);

    /* – 1 MHz
       – 400 kHz
       – 100 kHz */
    I2C_Open_S(I2C1, 400000);

    dsm_gpio_e2p_pwr_en_port_init();
#endif
}

uint32_t dsm_eeprom_page_write(uint8_t _erase, uint32_t eeprom_addr,
                               uint8_t* pwdata, uint32_t length)
{
    MSG00("page_write() %08X, %d", eeprom_addr, length);

    uint8_t device_addr;

    dsm_wdt_ext_toggle_immd();

    device_addr = (I2C_ADDRESS | (0x0E & (eeprom_addr >> 15))) >> 1;

    if (_erase)
    {
        memset(pwdata, 0xff, length);
    }

    uint32_t tx_bytes = I2C_WriteMultiBytesTwoRegs(I2C1, device_addr,
                                                   eeprom_addr, pwdata, length);

    if (tx_bytes == 0x00)
    {
        dsm_eeprom_init();
    }

    MSG00("   %d bytes written", tx_bytes);

    if (tx_bytes > 0)
    {
        return 0x00; /* success */
    }
    else
    {
        return 0x01; /* fail */
    }
}

#define EEPROM_WRITE_WAIT 10 /* bccho, 2023-08-02 */
#define EEPROM_READ_WAIT 10  /* bccho, 2023-08-02 */
uint32_t dsm_eeprom_write(uint8_t _erase, uint32_t eeprom_addr, uint8_t* pwdata,
                          uint32_t length)
{
#if EEPROM_TYPE == SPI
    return dsm_spi_eep_write(_erase, eeprom_addr, pwdata, length);
#else
    uint16_t lenInPage;

    /* 현재 주소 페이지에서 쓸 수 있는 바이트 */
    lenInPage = _PAGE_SIZE - (eeprom_addr & (_PAGE_SIZE - 1));

    DPRINTF(DBG_NONE, "%s: addr[%d], length[%d], lenInPage[%d]\r\n", __func__,
            eeprom_addr, length, lenInPage);

    if (lenInPage != 0)
    {
        if (lenInPage > length)
        {
            lenInPage = length;
        }

        if (dsm_eeprom_page_write(_erase, eeprom_addr, pwdata, lenInPage))
        {
            DPRINTF(DBG_ERR, "%s: error 1!!!\r\n", __func__);

            return HAL_ERROR;
        }

        if (_erase == FALSE)
        {
            pwdata += (int)lenInPage;
        }

        eeprom_addr += (uint32_t)lenInPage;
        length -= lenInPage;
    }

    OSTimeDly(OS_MS2TICK(EEPROM_WRITE_WAIT));

    while (length > _PAGE_SIZE)
    {
        if (dsm_eeprom_page_write(_erase, eeprom_addr, pwdata, _PAGE_SIZE))
        {
            DPRINTF(DBG_ERR, "%s: error 2!!!\r\n", __func__);
            return HAL_ERROR;
        }

        if (_erase == FALSE)
            pwdata += _PAGE_SIZE;
        eeprom_addr += _PAGE_SIZE;
        length -= _PAGE_SIZE;

        OSTimeDly(OS_MS2TICK(EEPROM_WRITE_WAIT));
    }

    if (0 != length)
    {
        if (dsm_eeprom_page_write(_erase, eeprom_addr, pwdata, length))
        {
            DPRINTF(DBG_ERR, "%s: error 3!!!\r\n", __func__);
            return HAL_ERROR;
        }
        OSTimeDly(OS_MS2TICK(EEPROM_WRITE_WAIT));
    }

    return HAL_OK;
#endif
}

uint32_t dsm_eeprom_read_devce(uint32_t eeprom_addr, uint8_t* prdata,
                               uint32_t length)
{
    uint8_t device_addr;

    MSG00("dsm_eeprom_read_devce()--start");

    dsm_wdt_ext_toggle_immd();

    device_addr = (I2C_ADDRESS | (0x0E & (eeprom_addr >> 15))) >> 1;

    DPRINTF(DBG_NONE, "%s: device[0x%02X], addr[%X], len[%X]\r\n", __func__,
            device_addr, eeprom_addr, length);

    uint32_t rx_bytes = I2C_ReadMultiBytesTwoRegs(I2C1, device_addr,
                                                  eeprom_addr, prdata, length);

    MSG00("    %d bytes read", rx_bytes);
    if (rx_bytes > 0)
    {
        return 0x00; /* success */
    }
    else
    {
        return 0x01; /* fail */
    }
}

uint32_t dsm_eeprom_read(uint32_t eeprom_addr, uint8_t* prdata, uint32_t length)
{
#if EEPROM_TYPE == SPI
    return dsm_spi_eep_read(eeprom_addr, prdata, length);
#else
    uint32_t ret = HAL_OK;

    uint16_t _len;
    uint32_t eeprom_addr_2 = eeprom_addr + length;

    DPRINTF(DBG_TRACE, "%s: eeprom_addr[%X], length[%X]\r\n", __func__,
            eeprom_addr, length);

    if (!(eeprom_addr & BIT18) && (eeprom_addr_2 & BIT18))
    {
        _len = (BIT18 - eeprom_addr);
        length -= _len;

        DPRINTF(DBG_TRACE, "%s: eeprom_addr[%X], len[%X], length[%X]\r\n",
                __func__, eeprom_addr, _len, length);

        ret = dsm_eeprom_read_devce(eeprom_addr, prdata, _len);
        if (ret)
        {
            DPRINTF(DBG_ERR, "%s: error 1 !!!\r\n", __func__);
            return HAL_ERROR;
        }

        eeprom_addr += _len;
        prdata += _len;
    }

    ret = dsm_eeprom_read_devce(eeprom_addr, prdata, length);
    if (ret)
    {
        DPRINTF(DBG_ERR, "%s: error 2 !!!\r\n", __func__);
        return HAL_ERROR;
    }

    return ret;
#endif
}

uint32_t dsm_eeprom_erase(uint32_t eeprom_addr, uint32_t length)
{
#if EEPROM_TYPE == SPI
    return dsm_spi_eep_erase(eeprom_addr, length);
#else
    uint32_t ret;
    uint8_t wdata[512];

    ret = dsm_eeprom_write(TRUE, eeprom_addr, wdata, length);

    if (ret != HAL_OK)
    {
        DPRINTF(DBG_ERR, "%s: error !!!\r\n", __func__);
        return HAL_ERROR;
    }

    return HAL_OK;
#endif
}
