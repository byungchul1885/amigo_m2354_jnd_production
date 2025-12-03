/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "options_sel.h"
#include "amg_lcd.h"

#define LCD_POWER_SOURCE_TESTx

ST_LCD_RAM_DATA st_lcd_ram;

void dsm_lcd_init(void)
{
#if 1                                         /* bccho, LCD, 2023-07-15 */
    S_LCD_CFG_T g_LCDCfg = {__LIRC,           /* LCD clock source frequency */
                            LCD_COM_DUTY_1_4, /* COM duty */
                            LCD_BIAS_LV_1_3,  /* Bias level */
                            64,               /* Operation frame rate */
                            LCD_WAVEFORM_TYPE_A_NORMAL, /* Waveform type */
                            LCD_FRAME_COUNTING_END_INT, /* Interrupt source */
                            LCD_LOW_DRIVING_AND_BUF_ON, /* Driving mode */
#ifdef LCD_POWER_SOURCE_TEST
                            LCD_VOLTAGE_SOURCE_AVDD
#else
                            LCD_VOLTAGE_SOURCE_CP,
#endif
    };

    SYS_ResetModule_S(LCD_RST);
    LCD_Open(&g_LCDCfg);
    LCD_SET_CP_VOLTAGE(LCD_CP_VOLTAGE_LV_1);
    LCD_ENABLE_DISPLAY();
#else
    hlcd.Instance = LCD;
    hlcd.Init.Prescaler = LCD_PRESCALER_1;
    hlcd.Init.Divider = LCD_DIVIDER_16;
    hlcd.Init.Duty = LCD_DUTY_1_4;
    hlcd.Init.Bias = LCD_BIAS_1_3;
    hlcd.Init.VoltageSource = LCD_VOLTAGESOURCE_INTERNAL;
    // hlcd.Init.Contrast = LCD_CONTRASTLEVEL_7;
    hlcd.Init.Contrast = LCD_CONTRASTLEVEL_3;
    hlcd.Init.DeadTime = LCD_DEADTIME_0;
    hlcd.Init.PulseOnDuration = LCD_PULSEONDURATION_3;
    hlcd.Init.MuxSegment = LCD_MUXSEGMENT_DISABLE;
    hlcd.Init.BlinkMode = LCD_BLINKMODE_OFF;
    hlcd.Init.BlinkFrequency = LCD_BLINKFREQUENCY_DIV8;
    hlcd.Init.HighDrive = LCD_HIGHDRIVE_ENABLE;

    __HAL_LCD_RESET_HANDLE_STATE(&hlcd);

    if (HAL_LCD_Init(&hlcd) != HAL_OK)
    {
        Error_Handler();
    }
#endif /* bccho */

    dsm_lcd_dsp_clear();
    dsm_lcd_dsp_lcdmem_clear();
    dsm_dsp_lcdmem_move();
}

void dsm_lcd_deinit(void) {}

void dsm_lcd_dsp_lcdmem_clear(void)
{
    memset(&st_lcd_ram, 0x00, sizeof(st_lcd_ram));
}

void dsm_dsp_lcdmem_move(void)
{
#if 1 /* bccho, LCD, 2023-08-04 */
    uint8_t seg_com[24][4];
    for (int i = 0; i < 24; i++) /* segment 24개 */
    {
        for (int j = 0; j < 4; j++) /* com 4개 */
        {
            seg_com[i][j] = (st_lcd_ram.com_xx_s31_00[j] >> i) & 0x01;
        }
    }

    uint32_t lcd_data[6];
    for (int i = 0, j = 0; j < 6; i += 4, j++)
    {
        lcd_data[j] = (((seg_com[i][3] << 3) | (seg_com[i][2] << 2) |
                        (seg_com[i][1] << 1) | (seg_com[i][0] << 0))
                       << 0) | /* segment n */
                      (((seg_com[i + 1][3] << 3) | (seg_com[i + 1][2] << 2) |
                        (seg_com[i + 1][1] << 1) | (seg_com[i + 1][0] << 0))
                       << 8) | /* segment n+1 */
                      (((seg_com[i + 2][3] << 3) | (seg_com[i + 2][2] << 2) |
                        (seg_com[i + 2][1] << 1) | (seg_com[i + 2][0] << 0))
                       << 16) | /* segment n+2 */
                      (((seg_com[i + 3][3] << 3) | (seg_com[i + 3][2] << 2) |
                        (seg_com[i + 3][1] << 1) | (seg_com[i + 3][0] << 0))
                       << 24); /* segment n+3 */
    }

    for (int j = 0; j < 6; j++) /* segment 4개씩 6개 */
    {
        LCD->DATA[j] = lcd_data[j];
    }
#else
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER0, 0xFFFFFFFF, /* COM0, SEQ[00-31] */
                  st_lcd_ram.com_xx_s31_00[0]);
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER1, 0xFFFFFFFF, /* COM0, SEQ[32-44] */
                  st_lcd_ram.com_xx_s43_32[0]);

    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER2, 0xFFFFFFFF, /* COM1, SEQ[00-31]  */
                  st_lcd_ram.com_xx_s31_00[1]);
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER3, 0xFFFFFFFF, /* COM1, SEQ[32-44] */
                  st_lcd_ram.com_xx_s43_32[1]);

    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER4, 0xFFFFFFFF, /* COM2, SEQ[00-31]  */
                  st_lcd_ram.com_xx_s31_00[2]);
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER5, 0xFFFFFFFF, /* COM2, SEQ[32-44] */
                  st_lcd_ram.com_xx_s43_32[2]);

    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER6, 0xFFFFFFFF, /* COM3, SEQ[00-31]  */
                  st_lcd_ram.com_xx_s31_00[3]);
    HAL_LCD_Write(&hlcd, LCD_RAM_REGISTER7, 0xFFFFFFFF, /* COM3, SEQ[32-44] */
                  st_lcd_ram.com_xx_s43_32[3]);

    HAL_LCD_UpdateDisplayRequest(&hlcd);
#endif /* bccho */
}

void dsm_lcd_dsp_clear(void)
{
#if 1 /* bccho, LCD, 2023-07-15 */
    LCD_SetAllPixels(0);
#else
    HAL_LCD_Clear(&hlcd);
#endif /* bccho */
}
