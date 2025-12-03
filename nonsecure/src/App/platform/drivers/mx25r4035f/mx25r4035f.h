#ifndef MX25R4035F_H
#define MX25R4035F_H

/**
 * @brief Struct for holding mx25 device state
 */

/*** MX25 series command hex code definition ***/
// ID comands
#define FLASH_CMD_RDID 0x9F  // RDID (Read Identification)
#define FLASH_CMD_RES 0xAB   // RES (Read Electronic ID)
#define FLASH_CMD_REMS 0x90  // REMS (Read Electronic & Device ID)

// Register comands
#define FLASH_CMD_WRSR 0x01    // WRSR (Write Status Register)
#define FLASH_CMD_RDSR 0x05    // RDSR (Read Status Register)
#define FLASH_CMD_WRSCUR 0x2F  // WRSCUR (Write Security Register)
#define FLASH_CMD_RDSCUR 0x2B  // RDSCUR (Read Security Register)
#define FLASH_CMD_RDCR 0x15    // RDCR (Read Configuration Register)

// READ comands
#define FLASH_CMD_READ 0x03      // READ (1 x I/O)
#define FLASH_CMD_2READ 0xBB     // 2READ (2 x I/O)
#define FLASH_CMD_4READ 0xEB     // 4READ (4 x I/O)
#define FLASH_CMD_FASTREAD 0x0B  // FAST READ (Fast read data)
#define FLASH_CMD_DREAD 0x3B     // DREAD (1In/2 Out fast read)
#define FLASH_CMD_QREAD 0x6B     // QREAD (1In/4 Out fast read)
#define FLASH_CMD_RDSFDP 0x5A    // RDSFDP (Read SFDP)

// Program comands
#define FLASH_CMD_WREN 0x06  // WREN (Write Enable)
#define FLASH_CMD_WRDI 0x04  // WRDI (Write Disable)
#define FLASH_CMD_PP 0x02    // PP (page program)
#define FLASH_CMD_4PP 0x38   // 4PP (Quad page program)

// Erase comands
#define FLASH_CMD_SE 0x20     // SE (Sector Erase)
#define FLASH_CMD_BE32K 0x52  // BE32K (Block Erase 32kb)
#define FLASH_CMD_BE 0xD8     // BE (Block Erase)
#define FLASH_CMD_CE 0x60     // CE (Chip Erase) hex code: 60 or C7

// Mode setting comands
#define FLASH_CMD_DP 0xB9    // DP (Deep Power Down)
#define FLASH_CMD_ENSO 0xB1  // ENSO (Enter Secured OTP)
#define FLASH_CMD_EXSO 0xC1  // EXSO  (Exit Secured OTP)
#ifdef SBL_CMD_0x77
#define FLASH_CMD_SBL 0x77  // SBL (Set Burst Length) new: 0x77
#else
#define FLASH_CMD_SBL 0xC0  // SBL (Set Burst Length) Old: 0xC0
#endif

// Reset comands
#define FLASH_CMD_RSTEN 0x66  // RSTEN (Reset Enable)
#define FLASH_CMD_RST 0x99    // RST (Reset Memory)

// Security comands
#ifdef LCR_CMD_0xDD_0xD5
#else
#endif

// Suspend/Resume comands
#define FLASH_CMD_PGM_ERS_S 0xB0  // PGM/ERS Suspend (Suspends Program/Erase)
#define FLASH_CMD_PGM_ERS_R 0x30  // PGM/ERS Erase (Resumes Program/Erase)
#define FLASH_CMD_NOP 0x00        // NOP (No Operation)

// Return Message
typedef enum
{
    FlashOperationSuccess,
    FlashWriteRegFailed,
    FlashTimeOut,
    FlashIsBusy,
    FlashQuadNotEnable,
    FlashAddressInvalid
} ReturnMsg;

extern ReturnMsg CMD_RDID(uint32_t *Identification);
extern ReturnMsg CMD_RES(uint8_t *ElectricIdentification);
extern ReturnMsg CMD_REMS(uint16_t *REMS_Identification);
extern ReturnMsg CMD_RDSR(uint8_t *StatusReg);

extern void CMD_WREN(void);
extern void CMD_WRDI(void);

extern ReturnMsg CMD_BE32K(uint32_t flash_address);
extern ReturnMsg CMD_READ(uint32_t flash_address, uint8_t *target_address,
                          uint32_t byte_length);
extern ReturnMsg CMD_FASTREAD(uint32_t flash_address, uint8_t *target_address,
                              uint32_t byte_length);
extern ReturnMsg CMD_PP(uint32_t flash_address, uint8_t *source_address,
                        uint32_t byte_length);

uint8_t CMD_SE(uint32_t flash_address);
ReturnMsg CMD_CE(void);

extern void mx25r4035f_init(void);

void dsm_sflash_fw_erase(uint8_t type);
uint32_t dsm_sflash_fw_get_startaddr(uint8_t type);

#if 1 /* bccho, 2023-08-02 */
void wait_SPI_IS_BUSY(SPI_T *spi);
#endif
/** @} */

#endif /* MX25R4035F_H */
