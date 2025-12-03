#ifndef CE_LSBS_H
#define CE_LSBS_H 1

#define FS_INT        8192
#define FS_FLOAT      (8192.0)

#define CE_ACCUM       8191

#define MAX_GAIN      0x6FFFL
#define UNITY_GAIN    0x4000L
#define MIN_GAIN      0x1000L
#define ZERO_PHASE    0x0000L
#define MAX_PHASE     (65535L)
#define MIN_PHASE     (-65536L)
#define LIMIT_PHASE_ERROR 1
#define LIMIT_PHASE_CORRECTION 0
#define MAX_TAN_THETA (0.12278)
#define MIN_TAN_THETA (-0.12278)

#define TWOPI_DIV_FS  (6.283185307/FS_FLOAT)
#define TWO_MINUS_G  (2.0-0.001953125)

#define ONE_HZ        (0x0080000l)
#define TWENTIETH_HZ  (ONE_HZ / 20)
#define SEVENTY_HZ    (ONE_HZ * 70)
#define CELCIUS_PER_CE    (0.1)
#define DEG           (360.0/FS_FLOAT)

#define CE_V_PER_CE      (1.70075E-09)

#define CE_V2H_PER_CE    (4.31369E-13)
#define CE_V2S_PER_CE    (1.55312E-09)
#define CE_V2F_PER_CE    (1.27216E-05)

#define CE_I2H_PER_CE    (4.31369E-13)
#define CE_I2S_PER_CE    (1.55312E-09)
#define CE_I2F_PER_CE    (1.27216E-05)

#define CE_WH_PER_CE     (4.31369E-13)
#define CE_WS_PER_CE     (1.55312E-09)
#define CE_WF_PER_CE     (1.27216E-05)

#define I_MIN         107L
#define V_MIN         14030000L
#define I_MAXA        1841L
#define V_MAX          30391L
#define WRATE_MPU     29034076L
#define I_LIMIT       546020000L
#define V_LIMIT       45450000L
#define I_MAXB        1040L


#endif /* CE LSBs */
