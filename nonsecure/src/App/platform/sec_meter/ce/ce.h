#ifndef CE_CONSTANTS_H
#define CE_CONSTANTS_H 1

#include "ce66a05b.h"

extern const uint32_t NumCeCode;
extern const uint8_t CeCode[];

extern const uint32_t NumCeData;
extern const uint8_t CeData[];

#define CE_DATA_RAM 0x01800
#define CE_CODE_RAM 0x2FFF
#define CE_NAME_OFF (0x38 * 4)
#define CE_NAME_LOC (CeData + CE_NAME_OFF)
#define ce_ram ((int32_t *)(CE_DATA_RAM))
#define CE_PARM_BASE (&CAL_I0)
#define CE_DEFAULT_CNT ((0x31 - 0x10) + 1)

#define I0_RAW (ce_ram[0x00])
#define I1_RAW (ce_ram[0x01])
#define V0_RAW (ce_ram[0x02])
#define V0_RAW1 (ce_ram[0x03])
#define MAG_SLOT0 (ce_ram[0x04])
#define MAG_SLOT1 (ce_ram[0x05])

#define CAL_I0 (ce_ram[0x10])
#define CAL_V0 (ce_ram[0x11])
#define PHADJ_0 (ce_ram[0x12])
#define CAL_I1 (ce_ram[0x13])
#define CAL_V1 (ce_ram[0x14])
#define PHADJ_1 (ce_ram[0x15])
#define I1_SCALE (ce_ram[0x16])
#define ICRTA_0 (ce_ram[0x17])
#define ICRTA_1 (ce_ram[0x18])
#define STEMP22 (ce_ram[0x19])
#define DEGSCALE (ce_ram[0x1a])
#define CE_PPMC (ce_ram[0x1b])
#define CE_PPMC2 (ce_ram[0x1c])
#define VCRT_0 (ce_ram[0x1d])
#define VCRT_1 (ce_ram[0x1e])

#define CE_CONFIG (ce_ram[0x20])
#define CE_6K_TEMP BIT23
#define EXT_TEMP BIT22
#define DO_EDG_INT BIT21
#define DO_SAG_INT BIT20
#define SAG_CNT_MASK 0x00FFF00
#define SAG_CNT_SHIFT 8

#define EXT_PULSE BIT5
#define V0_4X BIT3
#define PULSE_FAST BIT1
#define PULSE_SLOW BIT0

#define PULSE_SPEED_MASK 0x00000003

#define WRATE (ce_ram[0x21])
#define KVAR (ce_ram[0x22])
#define CE_SUMPRE (ce_ram[0x23])
#define SAG_THR (ce_ram[0x24])
#define QUANT_V0 (ce_ram[0x25])
#define QUANT_I0 (ce_ram[0x26])
#define QUANT_W0 (ce_ram[0x27])
#define QUANT_VAR0 (ce_ram[0x28])
#define QUANT_V1 (ce_ram[0x29])
#define QUANT_I1 (ce_ram[0x2A])
#define QUANT_W1 (ce_ram[0x2B])
#define QUANT_VAR1 (ce_ram[0x2C])
#define NFREQ (ce_ram[0x2D])
#define HARM_A (ce_ram[0x2E])
#define PLL_BIAS (ce_ram[0x2F])
#define PLL_LIMIT (ce_ram[0x30])

#define GAIN_ADJ0 (ce_ram[0x40])
#define GAIN_ADJ1 (ce_ram[0x41])
#define GAIN_ADJ2 (ce_ram[0x42])

#define APULSEX (ce_ram[0x3C])
#define XPULSE_CTR (ce_ram[0x3D])
#define XPULSE_FRAC (ce_ram[0x3E])
#define APULSEW (ce_ram[0x45])
#define WPULSE_CTR (ce_ram[0x46])
#define WPULSE_FRAC (ce_ram[0x47])
#define WSUM_ACCUM (ce_ram[0x48])
#define APULSER (ce_ram[0x49])
#define RPULSE_CTR (ce_ram[0x4a])
#define RPULSE_FRAC (ce_ram[0x4b])
#define VSUM_ACCUM (ce_ram[0x4c])

#define SAG1_CNT (ce_ram[0x4d])
#define SWE1_CNT (ce_ram[0x4e])
#define SAG1_THR_H (ce_ram[0x4f])
#define SAG1_THR_L (ce_ram[0x50])
#define SAG1_THR_0 (ce_ram[0x51])
#define SWE1_THR_H (ce_ram[0x52])
#define SWE1_THR_L (ce_ram[0x53])
#define SWE1_THR_0 (ce_ram[0x54])
#define PEAK_CNT (ce_ram[0x55])

#define CESTATUS_X (ce_ram[0x80])
#define TEMP_X (ce_ram[0x81])
#define FREQ_X (ce_ram[0x82])
#define MAINEDGE_X (ce_ram[0x83])
#define WSUM_X (ce_ram[0x84])
#define W0SUM_X (ce_ram[0x85])
#define W1SUM_X (ce_ram[0x86])
#define W2SUM_X (ce_ram[0x87])
#define VARSUM_X (ce_ram[0x88])
#define VAR0SUM_X (ce_ram[0x89])
#define VAR1SUM_X (ce_ram[0x8a])
#define VAR2SUM_X (ce_ram[0x8b])
#define I0SQSUM_X (ce_ram[0x8c])
#define I1SQSUM_X (ce_ram[0x8d])
#define I2SQSUM_X (ce_ram[0x8e])
#define IDSQSUM_X (ce_ram[0x8f])
#define V0SQSUM_X (ce_ram[0x90])
#define V1SQSUM_X (ce_ram[0x91])
#define V2SQSUM_X (ce_ram[0x92])
#define I0SQRES_X (ce_ram[0x93])
#define I1SQRES_X (ce_ram[0x94])
#define W0SUM_H_X (ce_ram[0x95])
#define I0SQSUM_H_X (ce_ram[0x96])
#define V0SQSUM_H_X (ce_ram[0x97])
#define W1SUM_H_X (ce_ram[0x98])
#define I1SQSUM_H_X (ce_ram[0x99])
#define CHIPID_X (ce_ram[0x9A])
#define STEMP_X (ce_ram[0x9B])

#define S_LEN (ce_ram[0xFF])

#endif
