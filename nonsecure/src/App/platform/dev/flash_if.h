/**
 ******************************************************************************
 * @file    IAP_Main/Inc/flash_if.h
 * @author  MCD Application Team
 * @brief   This file provides all the headers of the flash_if functions.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 *SLA0044, the "License"; You may not use this file except in compliance with
 *the License. You may obtain a copy of the License at:
 *                        http://www.st.com/SLA0044
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/

#if 0 /* bccho, IFLASH, 2023-07-15 */
#include "stm32l4xx_hal.h"
#else
#define FLASH_PAGE_SIZE 2048
#define FLASH_BANK_1 ((uint32_t)0x01) /*!< Bank 1   */
#define FLASH_BANK_2 ((uint32_t)0x02) /*!< Bank 2   */
#endif                                /* bccho */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* Error code */
enum
{
    FLASHIF_OK = 0,
    FLASHIF_ERASEKO,
    FLASHIF_WRITINGCTRL_ERROR,
    FLASHIF_WRITING_ERROR,
    FLASHIF_PROTECTION_ERRROR
};

/* protection type */
enum
{
    FLASHIF_PROTECTION_NONE = 0,
    FLASHIF_PROTECTION_PCROPENABLED = 0x1,
    FLASHIF_PROTECTION_WRPENABLED = 0x2,
    FLASHIF_PROTECTION_RDPENABLED = 0x4,
};

/* protection update */
enum
{
    FLASHIF_WRP_ENABLE,
    FLASHIF_WRP_DISABLE
};

/* Define the address from where user application will be loaded.
   Note: this area is reserved for the IAP code                  */
#define FLASH_PAGE_STEP FLASH_PAGE_SIZE
#define FLASH_PAGE_NBPERBANK 256
#define FLASH_BANK_NUMBER 2

#define USER_FLASH_START_ADDRESS 0x08000000
#define USER_FLASH_START_BANK2_ADDRESS \
    (FLASH_PAGE_NBPERBANK * FLASH_PAGE_SIZE + USER_FLASH_START_ADDRESS)
#define APPLICATION_VECTOR_OFFSET (uint32_t)(0x4000)
#define APPLICATION_FW_HEADROOM (uint32_t)(0x200)

#define APPLICATION_BASE_ADDRESS \
    (uint32_t)(USER_FLASH_START_ADDRESS + APPLICATION_VECTOR_OFFSET)
#define APPLICATION_ADDRESS \
    (uint32_t)(APPLICATION_BASE_ADDRESS + APPLICATION_FW_HEADROOM)

#define APPLICATION_BASE_2_ADDRESS \
    (uint32_t)(USER_FLASH_START_BANK2_ADDRESS + APPLICATION_VECTOR_OFFSET)
#define APPLICATION_2_ADDRESS \
    (uint32_t)(APPLICATION_BASE_2_ADDRESS + APPLICATION_FW_HEADROOM)

/* Define the user application size */
#define USER_FLASH_SIZE (USER_FLASH_END_ADDRESS - APPLICATION_ADDRESS + 1)
#define BOOTLOADER_INT_FLASH_ADDRESS (uint32_t)(USER_FLASH_START_ADDRESS)

/* Notable Flash addresses */
#define USER_FLASH_END_ADDRESS 0x08100000

/* Exported macro ------------------------------------------------------------*/
/* ABSoulute value */
#define ABS_RETURN(x, y) ((x) < (y)) ? ((y) - (x)) : ((x) - (y))

/* Get the number of sectors from where the user program will be loaded */
#define FLASH_SECTOR_NUMBER \
    ((uint32_t)(ABS_RETURN(APPLICATION_ADDRESS, FLASH_START_BANK1)) >> 12)

/* Compute the mask to test if the Flash memory, where the user program will be
  loaded, is write protected */
#define FLASH_PROTECTED_SECTORS (~(uint32_t)((1 << FLASH_SECTOR_NUMBER) - 1))
/* Exported functions ------------------------------------------------------- */
void FLASH_If_Init(void);
uint32_t FLASH_If_Erase(uint32_t StartSector);
uint32_t FLASH_If_FW_Bank2_erase(void);
uint32_t FLASH_If_Erase_selected(uint32_t start, uint32_t page_num);
uint8_t *FLASH_If_Read(uint8_t *src, uint8_t *dest, uint32_t Len);
uint32_t FLASH_If_GetPage(uint32_t Address);
uint32_t FLASH_If_GetWriteProtectionStatus(void);
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source,
                        uint32_t length);
uint32_t FLASH_If_WriteProtectionConfig(uint32_t protectionstate,
                                        uint32_t bank);

#endif /* __FLASH_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
