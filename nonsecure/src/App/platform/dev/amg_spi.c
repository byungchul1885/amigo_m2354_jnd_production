/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_spi.h"
#include "options_sel.h"
/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/

#define SPI_TIMEOUT 5000
/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/
#if 0  /* bccho, SPI-FLASH, 2023-07-15 */
SPI_HandleTypeDef hspi3;
#endif /* bccho */

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/

void dsm_spi_init(void)
{
#if 1 /* bccho, SPI-FLASH, 2023-07-15 */
    MSG05("dsm_spi_init()");

    /* as a master, MSB first, 8-bit transaction, SPI Mode-0 timing, clock is
     * 2MHz */
    SPI_Flash_Open_S(SPI0, 10000000); /* bccho, 2023-12-14, 10 Mhz */

    /* Disable auto SS function, control SS signal manually. */
    SPI_DisableAutoSS(SPI0);
#else
#if 1
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    hspi3.Instance = SPI3;

    hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi3.Init.Direction = SPI_DIRECTION_2LINES;
    hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi3.Init.CRCPolynomial = 7;
    hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi3.Init.NSS = SPI_NSS_SOFT;
    hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi3.Init.Mode = SPI_MODE_MASTER;

    if (HAL_SPI_Init(&hspi3) != HAL_OK)
    {
        /* Initialization Error */
        Error_Handler();
    }

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOG);
    GPIO_InitStruct.Pin = SPIM_1_NSS_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(SPIM_1_NSS_GPIO_Port, &GPIO_InitStruct);
    LL_GPIO_SetOutputPin(SPIM_1_NSS_GPIO_Port, SPIM_1_NSS_Pin);

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOD);
    GPIO_InitStruct.Pin = SPIM_HOLD_Pin | SPIM_WP_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(SPIM_WP_GPIO_Port, &GPIO_InitStruct);
    LL_GPIO_SetOutputPin(SPIM_WP_GPIO_Port, SPIM_WP_Pin);
    LL_GPIO_SetOutputPin(SPIM_WP_GPIO_Port, SPIM_HOLD_Pin);
#endif
#endif /* bccho */
}

#if 1
void dsm_spi_deinit(void)
{
#if 1 /* bccho, SPI-FLASH, 2023-07-15 */
    MSG05("dsm_spi_deinit()");
    SPI_Close(SPI0);
#else
    HAL_SPI_DeInit(&hspi3);
#endif /* bccho */
}

uint8_t dsm_spi_send(uint8_t cmd)
{
#if 0 /* bccho, SPI-FLASH, 2023-07-15 */
    uint8_t rx;
    HAL_SPI_TransmitReceive(&hspi3, &cmd, &rx, 1, SPI_TIMEOUT);
    return rx;
#else /* bccho */
    return 0;
#endif /* bccho */
}

/* bccho, 2023-08-02, 사용하지 않음 */
void dsm_spi_sendblock(const uint8_t *buffer, uint16_t count)
{
#if 1 /* bccho, SPI-FLASH, 2023-07-15 */
    MSG05("dsm_spi_sendblock()");
    (void)buffer;
    (void)count;
#else
    HAL_SPI_Transmit(&hspi3, (uint8_t *)buffer, count, SPI_TIMEOUT);
#endif /* bccho */
}

/* bccho, 2023-08-02, 사용하지 않음 */
void dsm_spi_recvblock(uint8_t *buffer, uint16_t nbytes)
{
#if 1 /* bccho, SPI-FLASH, 2023-07-15 */
    (void)buffer;
    (void)nbytes;
    MSG05("dsm_spi_recvblock()");
#else
    HAL_SPI_Receive(&hspi3, buffer, nbytes, SPI_TIMEOUT);
#endif /* bccho */
}
#endif
