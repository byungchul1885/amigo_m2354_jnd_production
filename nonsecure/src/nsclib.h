#ifndef __LSCLIB_H
#define __LSCLIB_H

void UART_Open_S(UART_T *uart, uint32_t u32baudrate);
void SYS_ResetModule_S(uint32_t u32ModuleIndex);
uint32_t I2C_Open_S(I2C_T *i2c, uint32_t u32BusClock);
void SYS_ResetChip_S(void);
void SYS_ResetCPU_S(void);
void NVIC_SystemReset_S(void);
uint32_t SPI_Flash_Open_S(SPI_T *spi, uint32_t u32BusClock);
uint32_t CheckPowerSource_S(void);
uint32_t get_reset_cause_S(void);
int32_t FMC_WriteMultiple_S(uint32_t u32Addr, uint32_t pu32Buf[],
                            uint32_t u32Len);
int32_t FMC_Erase_S(uint32_t u32PageAddr);
int32_t FMC_ReadBytes_S(uint32_t u32Addr, uint32_t pu32Buf[], uint32_t u32Len);
void goto_loader_S(void);
uint32_t FMC_GetVECMAP_S(void);
uint32_t get_current_bank_S(void);
void CLK_SysTickLongDelay_S(uint32_t us);
uint32_t GetSystemCoreClock_S(void);
uint8_t XMD_Write_S(uint32_t u32Addr, uint32_t u32Data);
uint8_t OTA_WriteNewFW_S(uint32_t u32Address, const uint8_t *pu8Buff,
                         uint32_t u32Size);
void kick_watchdog_S(void);
void CAN_Init_S(void);
uint32_t CAN_Open_S(uint32_t u32BaudRate);
void Set_CLOCK_24M_S(void);
void Set_CLOCK_96M_S(void);
#endif /* __LSCLIB_H */
