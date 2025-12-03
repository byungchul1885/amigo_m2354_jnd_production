#if 0 /* bccho, SPI-FLASH, 2023-07-15 */
#include "mx25r4035f-interface.h"
#if defined(USE_FULL_LL_DRIVER)
#include "stm32l4xx_ll_spi.h"
#else
#include "stm32l4xx_hal_spi.h"
#endif

#include "platform.h"
#endif /* bccho */

#include "amg_spi.h"
#include "amg_debug.h" /* bccho, Add, 2023-08-02 */
/*
******************************************************************************
* 	LOCAL CONSTANTS
******************************************************************************
*/

/*
******************************************************************************
*	LOCAL DATA TYPES
******************************************************************************
*/

#if 0 /* bccho, SPI-FLASH, 2023-07-15 */
#define SPI_CS_HIGH LL_GPIO_SetOutputPin(SPIM_1_NSS_GPIO_Port, SPIM_1_NSS_Pin);
#define SPI_CS_LOW LL_GPIO_ResetOutputPin(SPIM_1_NSS_GPIO_Port, SPIM_1_NSS_Pin);
#else /* bccho */
#define SPI_CS_HIGH SPI_SET_SS_HIGH(SPI0)
#define SPI_CS_LOW SPI_SET_SS_LOW(SPI0)
#endif /* bccho */

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/
/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/

void mx25r4035f_interface_init()
{
    MSG05("mx25r4035f_interface_init()");
    dsm_spi_init();
}
