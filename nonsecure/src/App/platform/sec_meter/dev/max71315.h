#ifndef MAX71315_H
#define MAX71315_H 1

// This is the default clock rate after reset.
#define SYS_CLK 9830400L  //    (32768 Hz * 1800) / 6 = 9830400 Hz
// #define SYS_CLK_MHz  (double)9.8304  //   (32768 Hz * 1800) / 6 = 9.830400
// MHz #define FPAGE_SIZE   2048            //   Flash Page: 512 X 32 = 2Kbyte

// Pins that need GPIO manipulations.
#define WPULSE_BIT 22
#define WPULSE_PD PD1
#define WPULSE_PO PO1
#define RPULSE_BIT 23
#define RPULSE_PD PD1
#define RPULSE_PO PO1
#define XPULSE_BIT 12
#define XPULSE_PD PD1
#define XPULSE_PO PO1
#define YPULSE_BIT 13
#define YPULSE_PD PD1
#define YPULSE_PO PO1

/*sfrw*/ #define I2CCN /*=*/ 0x0  // I2C control register
#define I2CCN_I2CEN_MASK 0x1
#define I2CCN_I2CEN 0x1
#define I2CCN_I2CEN_BIT 0
#define I2CCN_MST_MASK 0x2
#define I2CCN_MST 0x2
#define I2CCN_MST_BIT 1
#define I2CCN_GCEN_MASK 0x4
#define I2CCN_GCEN 0x4
#define I2CCN_GCEN_BIT 2
#define I2CCN_IRXM_MASK 0x8
#define I2CCN_IRXM 0x8
#define I2CCN_IRXM_BIT 3
#define I2CCN_ACK_MASK 0x10
#define I2CCN_ACK 0x10
#define I2CCN_ACK_BIT 4
#define I2CCN_ACKV_MASK 0x20
#define I2CCN_ACKV 0x20
#define I2CCN_ACKV_BIT 5
#define I2CCN_SCLO_MASK 0x40
#define I2CCN_SCLO 0x40
#define I2CCN_SCLO_BIT 6
#define I2CCN_SDAO_MASK 0x80
#define I2CCN_SDAO 0x80
#define I2CCN_SDAO_BIT 7
#define I2CCN_SCL_MASK 0x100
#define I2CCN_SCL 0x100
#define I2CCN_SCL_BIT 8
#define I2CCN_SDA_MASK 0x200
#define I2CCN_SDA 0x200
#define I2CCN_SDA_BIT 9
#define I2CCN_OEN_MASK 0x400
#define I2CCN_OEN 0x400
#define I2CCN_OEN_BIT 10
#define I2CCN_READ_MASK 0x800
#define I2CCN_READ 0x800
#define I2CCN_READ_BIT 11
#define I2CCN_HSMDEN_MASK 0x8000
#define I2CCN_HSMDEN 0x8000
#define I2CCN_HSMDEN_BIT 15

/*sfrw*/ #define I2CINT /*=*/ 0x10  // I2C interrupt register
/* Acknowledge an I2C interrupt safely, without masking
 * the register and accidentally clearing other interrupt bits. */
#define ACK_I2CINT0()   /*__insert_opcode(0x9007)*/
#define ACK_I2C_DONEI() /*__insert_opcode(0x9007)*/
#define ACK_I2CINT1()   /*__insert_opcode(0x9017)*/
#define ACK_I2C_IRXMI() /*__insert_opcode(0x9017)*/
#define ACK_I2CINT2()   /*__insert_opcode(0x9027)*/
#define ACK_I2C_GCI()   /*__insert_opcode(0x9027)*/
#define ACK_I2CINT3()   /*__insert_opcode(0x9037)*/
#define ACK_I2C_AMI()   /*__insert_opcode(0x9037)*/
#define ACK_I2CINT4()   /*__insert_opcode(0x9047)*/
#define ACK_I2C_RXTHI() /*__insert_opcode(0x9047)*/
#define ACK_I2CINT5()   /*__insert_opcode(0x9057)*/
#define ACK_I2C_TXTHI() /*__insert_opcode(0x9057)*/
#define ACK_I2CINT6()   /*__insert_opcode(0x9067)*/
#define ACK_I2C_STOPI() /*__insert_opcode(0x9067)*/
#define ACK_I2CINT7()   /*__insert_opcode(0x9077)*/
#define ACK_I2C_ERRI()  /*__insert_opcode(0x9077)*/

#define I2CINT_DONEI_MASK 0x1
#define I2CINT_DONEI 0x1
#define I2CINT_DONEI_BIT 0
#define I2CINT_IRXMI_MASK 0x2
#define I2CINT_IRXMI 0x2
#define I2CINT_IRXMI_BIT 1
#define I2CINT_GCI_MASK 0x4
#define I2CINT_GCI 0x4
#define I2CINT_GCI_BIT 2
#define I2CINT_AMI_MASK 0x8
#define I2CINT_AMI 0x8
#define I2CINT_AMI_BIT 3
#define I2CINT_RXTHI_MASK 0x10
#define I2CINT_RXTHI 0x10
#define I2CINT_RXTHI_BIT 4
#define I2CINT_TXTHI_MASK 0x20
#define I2CINT_TXTHI 0x20
#define I2CINT_TXTHI_BIT 5
#define I2CINT_STOPI_MASK 0x40
#define I2CINT_STOPI 0x40
#define I2CINT_STOPI_BIT 6
#define I2CINT_ERRI_MASK 0x80
#define I2CINT_ERRI 0x80
#define I2CINT_ERRI_BIT 7
#define I2CINT_DONEIE_MASK 0x100
#define I2CINT_DONEIE 0x100
#define I2CINT_DONEIE_BIT 8
#define I2CINT_IRXMIE_MASK 0x200
#define I2CINT_IRXMIE 0x200
#define I2CINT_IRXMIE_BIT 9
#define I2CINT_GCIE_MASK 0x400
#define I2CINT_GCIE 0x400
#define I2CINT_GCIE_BIT 10
#define I2CINT_AMIE_MASK 0x800
#define I2CINT_AMIE 0x800
#define I2CINT_AMIE_BIT 11
#define I2CINT_RXTHIE_MASK 0x1000
#define I2CINT_RXTHIE 0x1000
#define I2CINT_RXTHIE_BIT 12
#define I2CINT_TXTHIE_MASK 0x2000
#define I2CINT_TXTHIE 0x2000
#define I2CINT_TXTHIE_BIT 13
#define I2CINT_STOPIE_MASK 0x4000
#define I2CINT_STOPIE 0x4000
#define I2CINT_STOPIE_BIT 14
#define I2CINT_ERRIE_MASK 0x8000
#define I2CINT_ERRIE 0x8000
#define I2CINT_ERRIE_BIT 15

/*sfrw*/ #define I2CST /*=*/ 0x20  // I2C status register
#define I2CST_ERR_MASK 0x7F
#define I2CST_ERR_BIT 0
#define I2CST_BUS_MASK 0x80
#define I2CST_BUS 0x80
#define I2CST_BUS_BIT 7
#define I2CST_RXE_MASK 0x100
#define I2CST_RXE 0x100
#define I2CST_RXE_BIT 8
#define I2CST_RXF_MASK 0x200
#define I2CST_RXF 0x200
#define I2CST_RXF_BIT 9
#define I2CST_TXE_MASK 0x400
#define I2CST_TXE 0x400
#define I2CST_TXE_BIT 10
#define I2CST_TXF_MASK 0x800
#define I2CST_TXF 0x800
#define I2CST_TXF_BIT 11
#define I2CST_CKMD_MASK 0x1000
#define I2CST_CKMD 0x1000
#define I2CST_CKMD_BIT 12
#define I2CST_ST_MASK 0xE000
#define I2CST_ST_BIT 13

/*sfrw*/ #define I2CDATA /*=*/ \
    0x30                 // I2C data register, read to receive, write to trans.

/*sfrw*/ #define I2CMCN /*=*/ 0x40  // I2C master configuration register
#define I2CMCN_START_MASK 0x1
#define I2CMCN_START 0x1
#define I2CMCN_START_BIT 0
#define I2CMCN_RESTART_MASK 0x2
#define I2CMCN_RESTART 0x2
#define I2CMCN_RESTART_BIT 1
#define I2CMCN_STOP_MASK 0x4
#define I2CMCN_STOP 0x4
#define I2CMCN_STOP_BIT 2
#define I2CMCN_SEA_MASK 0x80
#define I2CMCN_SEA 0x80
#define I2CMCN_SEA_BIT 7
#define I2CMCN_IMCODE_MASK 0x700
#define I2CMCN_IMCODE_BIT 8

/*sfrw*/ #define SPICN /*=*/ 0x50  // SPI control register
#define SPICN_SPIEN_MASK 0x1
#define SPICN_SPIEN 0x1
#define SPICN_SPIEN_BIT 0
#define SPICN_MSTM_MASK 0x2
#define SPICN_MSTM 0x2
#define SPICN_MSTM_BIT 1
#define SPICN_MODFE_MASK 0x4
#define SPICN_MODFE 0x4
#define SPICN_MODFE_BIT 2
#define SPICN_MODF_MASK 0x8
#define SPICN_MODF 0x8
#define SPICN_MODF_BIT 3
#define SPICN_RXRST_MASK 0x10
#define SPICN_RXRST 0x10
#define SPICN_RXRST_BIT 4
#define SPICN_TXRST_MASK 0x20
#define SPICN_TXRST 0x20
#define SPICN_TXRST_BIT 5
#define SPICN_BUSY_MASK 0x80
#define SPICN_BUSY 0x80
#define SPICN_BUSY_BIT 7
#define SPICN_RXIE_MASK 0x100
#define SPICN_RXIE 0x100
#define SPICN_RXIE_BIT 8
#define SPICN_RXOVRIE_MASK 0x200
#define SPICN_RXOVRIE 0x200
#define SPICN_RXOVRIE_BIT 9
#define SPICN_TXIE_MASK 0x400
#define SPICN_TXIE 0x400
#define SPICN_TXIE_BIT 10
#define SPICN_TXEIE_MASK 0x800
#define SPICN_TXEIE 0x800
#define SPICN_TXEIE_BIT 11
#define SPICN_TXOVRIE_MASK 0x1000
#define SPICN_TXOVRIE 0x1000
#define SPICN_TXOVRIE_BIT 12
#define SPICN_STOPIE_MASK 0x2000
#define SPICN_STOPIE 0x2000
#define SPICN_STOPIE_BIT 13

/*sfrw*/ #define SPIST /*=*/ 0x60
/* Acknowledge an SPI interrupt safely, without masking
 * the register and accidentally clearing other interrupt bits. */
#define ACK_SPIST1()    /*__insert_opcode(0xE017)*/
#define ACK_SPIRXOVRI() /*__insert_opcode(0xE017)*/
#define ACK_SPIST3()    /*__insert_opcode(0xE037)*/
#define ACK_SPITXEI()   /*__insert_opcode(0xE037)*/
#define ACK_SPIST4()    /*__insert_opcode(0xE047)*/
#define ACK_SPITXOVRI() /*__insert_opcode(0xE047)*/
#define ACK_SPIST5()    /*__insert_opcode(0xE057)*/
#define ACK_SPISTOPI()  /*__insert_opcode(0xE057)*/
#define SPIST_RXI_MASK 0x1
#define SPIST_RXI 0x1
#define SPIST_RXI_BIT 0
#define SPIST_RXOVRI_MASK 0x2
#define SPIST_RXOVRI 0x2
#define SPIST_RXOVRI_BIT 1
#define SPIST_TXI_MASK 0x4
#define SPIST_TXI 0x4
#define SPIST_TXI_BIT 2
#define SPIST_TXEI_MASK 0x8
#define SPIST_TXEI 0x8
#define SPIST_TXEI_BIT 3
#define SPIST_TXOVRI_MASK 0x10
#define SPIST_TXOVRI 0x10
#define SPIST_TXOVRI_BIT 4
#define SPIST_STOPI_MASK 0x20
#define SPIST_STOPI 0x20
#define SPIST_STOPI_BIT 5
#define SPIST_RXHFI_MASK 0x100
#define SPIST_RXHFI 0x100
#define SPIST_RXHFI_BIT 8
#define SPIST_RXFI_MASK 0x200
#define SPIST_RXFI 0x200
#define SPIST_RXFI_BIT 9
#define SPIST_TXFI_MASK 0x400
#define SPIST_TXFI 0x400
#define SPIST_TXFI_BIT 10

/*sfrw*/ #define SPIB /*=*/ 0x70  // SPI read and write data

/*sfrw*/ #define I2CRX /*=*/ 0x80  // I2C receive FIFO counts
#define I2CRX_RXCNT_MASK 0xF
#define I2CRX_RXCNT_BIT 0
#define I2CRX_RXFIFO_MASK 0xF00
#define I2CRX_RXFIFO_BIT 8

/*sfrw*/ #define I2CRXCFG /*=*/ 0x90  // I2C receive configuration register
#define I2CRXCFG_DNR_MASK 0x1         // "Do not respond" to the address.
#define I2CRXCFG_DNR 0x1
#define I2CRXCFG_DNR_BIT 0
#define I2CRXCFG_RXFSH_MASK 0x80  // Flush the receive FIFO
#define I2CRXCFG_RXFSH 0x80
#define I2CRXCFG_RXFSH_BIT 7
#define I2CRXCFG_RXTH_MASK 0xF00  // Receive FIFO interrupt threshold
#define I2CRXCFG_RXTH_BIT 8

/*sfrw*/ #define I2CTX /*=*/ 0xa0  // I2C transmit FIFO count
#define I2CTX_TXFIFO_MASK 0xF00
#define I2CTX_TXFIFO_BIT 8

/*sfrw*/ #define I2CTXCFG /*=*/ 0xb0  // I2C transmit configuration register
#define I2CTXCFG_TXOVRIE_MASK 0x1
#define I2CTXCFG_TXOVRIE 0x1
#define I2CTXCFG_TXOVRIE_BIT 0
#define I2CTXCFG_TXOVRI_MASK 0x2
#define I2CTXCFG_TXOVRI 0x2
#define I2CTXCFG_TXOVRI_BIT 1
#define I2CTXCFG_TXLAST_MASK 0x20
#define I2CTXCFG_TXLAST 0x20
#define I2CTXCFG_TXLAST_BIT 5
#define I2CTXCFG_TXLO_MASK 0x40
#define I2CTXCFG_TXLO 0x40
#define I2CTXCFG_TXLO_BIT 6
#define I2CTXCFG_TXFSH_MASK 0x80
#define I2CTXCFG_TXFSH 0x80
#define I2CTXCFG_TXFSH_BIT 7
#define I2CTXCFG_TXTH_MASK 0xF00
#define I2CTXCFG_TXTH_BIT 8

/*sfrw*/ #define I2CSLA /*=*/ 0xc0  // I2C slave address
#define I2CSLA_SLA_MASK 0x3FF
#define I2CSLA_SLA_BIT 0
#define I2CSLA_EA_MASK 0x8000
#define I2CSLA_EA 0x8000
#define I2CSLA_EA_BIT 15

// Clock divider for high part of clock (16 bits, min value = 2)
/*sfrw*/ #define I2CCKH /*=*/ 0xd0

// Clock divider for low part of clock (16 bits, min value = 2)
/*sfrw*/ #define I2CCKL /*=*/ 0xe0  // Clock divider low

/*sfrw*/ #define I2CHSCK /*=*/ 0xf0  // Clock configuration reset = 0
#define I2CHSCK_HSCKL_MASK 0xFF
#define I2CHSCK_HSCKL_BIT 0
#define I2CHSCK_HSCKH_MASK 0xFF00
#define I2CHSCK_HSCKH_BIT 8

// I2CTO time = (CPUCLK/8)*(TO+1)
/*sfrw*/ #define I2CTO /*=*/ \
    0x100              // timeout for peripheral holding SCL, 16 bits

/*sfrw*/ #define I2CFIFO /*=*/ \
    0x110                // Sets lengths for tx & rx defaults to 8 bits.
#define I2CFIFO_RXLEN_MASK 0xFF
#define I2CFIFO_RXLEN_BIT 0
#define I2CFIFO_TXLEN_MASK 0xFF00
#define I2CFIFO_TXLEN_BIT 8

/*sfrw*/ #define SPICF /*=*/ 0x120  // SPI controls, including interrupts.
#define SPICF_CKPOL_MASK 0x1
#define SPICF_CKPOL 0x1
#define SPICF_CKPOL_BIT 0
#define SPICF_CKPHA_MASK 0x2
#define SPICF_CKPHA 0x2
#define SPICF_CKPHA_BIT 1
#define SPICF_CHR_MASK 0x4
#define SPICF_CHR 0x4
#define SPICF_CHR_BIT 2
#define SPICF_MODFIE_MASK 0x8
#define SPICF_MODFIE 0x8
#define SPICF_MODFIE_BIT 3
#define SPICF_SAS_MASK 0x40
#define SPICF_SAS 0x40
#define SPICF_SAS_BIT 6
#define SPICF_RXHFIE_MASK 0x100
#define SPICF_RXHFIE 0x100
#define SPICF_RXHFIE_BIT 8
#define SPICF_RXFIE_MASK 0x200
#define SPICF_RXFIE 0x200
#define SPICF_RXFIE_BIT 9
#define SPICF_TXFIE_MASK 0x400
#define SPICF_TXFIE 0x400
#define SPICF_TXFIE_BIT 10

/*sfrw*/ #define SPICK /*=*/ 0x130  // In master mode, [7:0] sets the bits/sec

/*sfrw*/ #define PWCN /*=*/ 0x140  // Power or system control register.
/*sfrw*/ #define SYSCN /*=*/ 0x140
#define SYS_PFD 0x01   // Power fail monitor disable
#define SYS_PFIE 0x02  // Power interrupt enable
#define SYS_PFI 0x04   // Power interrupt
// Note that turning off the voltage regulator (STOP with REGEN/*=*/1) disables
// all internal interrupt sources. (e.g. timers stop working.)
#define SYS_REGEN 0x08  // 1=Voltage regulator off in STOP mode
#define SYS_RESET 0x10  // 1 forces a reset

#define PULSE_EN 0x20  // 1 = meter pulses as alternate functions (P1.12/XPULSE,
#define DEMOD_DIS 0x40  // 1=UART0 has a normal serial input0=demodulator
#define IAP_EN 0x80     // 1= enables flash writes, 0= disables (default)

/*sfrw*/ #define M0_15 /*=*/ 0x150
/*sfrw*/ #define M0_16 /*=*/ 0x160

/*sfrw*/ #define TM2 /*=*/ 0x170

// In-circuit regsiters
/*sfrw*/ #define ICDT0 /*=*/ 0x180  // Debug temporary register 0

/*sfrw*/ #define ICDT1 /*=*/ 0x190  // Debug temporary register 1

/*sfrw*/ #define ICDC /*=*/ 0x1a0  // Debug control register.

/*sfrw*/ #define ICDF /*=*/ 0x1b0  // Debug flag register.

/*sfrw*/ #define ICDB /*=*/ 0x1c0  // Debug buffer register (8 bits)

/*sfrw*/ #define ICDA /*=*/ 0x1d0  // Debug address (32 bits)

/*sfrw*/ #define ICDD /*=*/ 0x1e0  // Debug data (32 bits)

/*sfrw*/ #define TM /*=*/ 0x1f0

/*sfrw*/ #define PO0 /*=*/ 0x1  // Port output for reading and writing.

/*sfrw*/ #define PO1 /*=*/ 0x11  // Port output for reading and writing.

/*sfrw*/ #define EIF /*=*/ 0x21  // External interrupt flags register.
/* Acknowledge an external interrupt safely, without masking
 * the register and accidentally clearing other interrupt bits. */
#define ACK_EIF0() /*__insert_opcode(0xA107)*/
#define ACK_EIF1() /*__insert_opcode(0xA117)*/
#define ACK_EIF2() /*__insert_opcode(0xA127)*/
#define ACK_EIF3() /*__insert_opcode(0xA137)*/
#define ACK_EIF4() /*__insert_opcode(0xA147)*/
#define ACK_EIF5() /*__insert_opcode(0xA157)*/
#define ACK_EIF6() /*__insert_opcode(0xA167)*/
#define ACK_EIF7() /*__insert_opcode(0xA177)*/
#define EIF0_EI0_MASK 0x1 * /
#define EIF0_EI0 0x1
#define EIF0_EI0_BIT 0
#define EIF0_EI1_MASK 0x2
#define EIF0_EI1 0x2
#define EIF0_EI1_BIT 1
#define EIF0_EI2_MASK 0x4
#define EIF0_EI2 0x4
#define EIF0_EI2_BIT 2
#define EIF0_EI3_MASK 0x8
#define EIF0_EI3 0x8
#define EIF0_EI3_BIT 3
#define EIF0_EI4_MASK 0x10
#define EIF0_EI4 0x10
#define EIF0_EI4_BIT 4
#define EIF0_EI5_MASK 0x20
#define EIF0_EI5 0x20
#define EIF0_EI5_BIT 5
#define EIF0_EI6_MASK 0x40
#define EIF0_EI6 0x40
#define EIF0_EI6_BIT 6
#define EIF0_EI7_MASK 0x80
#define EIF0_EI7 0x80
#define EIF0_EI7_BIT 7

/*sfrw*/ #define TB0CN /*=*/ 0x31  // Timer B 0 configuration register.
/* Acknowledge a timer B interrupt safely, without masking
 * the register and accidentally clearing other bits. */
#define ACK_TB0CN6()   /*__insert_opcode(0xB167)*/
#define ACK_TB0_EXFB() /*__insert_opcode(0xB167)*/
#define ACK_TB0CN7()   /*__insert_opcode(0xB177)*/
#define ACK_TB0_TFB()  /*__insert_opcode(0xB177)*/
#define TB0CN_CPRLB_MASK 0x1
#define TB0CN_CPRLB 0x1
#define TB0CN_CPRLB_BIT 0
#define TB0CN_ETB_MASK 0x2
#define TB0CN_ETB 0x2
#define TB0CN_ETB_BIT 1
#define TB0CN_TRB_MASK 0x4
#define TB0CN_TRB 0x4
#define TB0CN_TRB_BIT 2
#define TB0CN_EXENB_MASK 0x8
#define TB0CN_EXENB 0x8
#define TB0CN_EXENB_BIT 3
#define TB0CN_DCEN_MASK 0x10
#define TB0CN_DCEN 0x10
#define TB0CN_DCEN_BIT 4
#define TB0CN_TBOE_MASK 0x20
#define TB0CN_TBOE 0x20
#define TB0CN_TBOE_BIT 5
#define TB0CN_EXFB_MASK 0x40
#define TB0CN_EXFB 0x40
#define TB0CN_EXFB_BIT 6
#define TB0CN_TFB_MASK 0x80
#define TB0CN_TFB 0x80
#define TB0CN_TFB_BIT 7
#define TB0CN_TBPS_MASK 0x700
#define TB0CN_TBPS_BIT 8
#define TB0CN_TBCR_MASK 0x800
#define TB0CN_TBCR 0x800
#define TB0CN_TBCR_BIT 11
#define TB0CN_TBCS_MASK 0x1000
#define TB0CN_TBCS 0x1000
#define TB0CN_TBCS_BIT 12
#define TB0CN_CTB_MASK 0x8000
#define TB0CN_CTB 0x8000
#define TB0CN_CTB_BIT 15

/*sfrw*/ #define TB1CN /*=*/ 0x41  // Timer B 1 control register
/* Acknowledge a timer B interrupt safely, without masking
 * the register and accidentally clearing other bits. */
#define ACK_TB1CN6()   /*__insert_opcode(0xC167)*/
#define ACK_TB1_EXFB() /*__insert_opcode(0xC167)*/
#define ACK_TB1CN7()   /*__insert_opcode(0xC177)*/
#define ACK_TB1_TFB()  /*__insert_opcode(0xC177)*/
#define TB1CN_CPRLB_MASK 0x1
#define TB1CN_CPRLB 0x1
#define TB1CN_CPRLB_BIT 0
#define TB1CN_ETB_MASK 0x2
#define TB1CN_ETB 0x2
#define TB1CN_ETB_BIT 1
#define TB1CN_TRB_MASK 0x4
#define TB1CN_TRB 0x4
#define TB1CN_TRB_BIT 2
#define TB1CN_EXENB_MASK 0x8
#define TB1CN_EXENB 0x8
#define TB1CN_EXENB_BIT 3
#define TB1CN_DCEN_MASK 0x10
#define TB1CN_DCEN 0x10
#define TB1CN_DCEN_BIT 4
#define TB1CN_TBOE_MASK 0x20
#define TB1CN_TBOE 0x20
#define TB1CN_TBOE_BIT 5
#define TB1CN_EXFB_MASK 0x40
#define TB1CN_EXFB 0x40
#define TB1CN_EXFB_BIT 6
#define TB1CN_TFB_MASK 0x80
#define TB1CN_TFB 0x80
#define TB1CN_TFB_BIT 7
#define TB1CN_TBPS_MASK 0x700
#define TB1CN_TBPS_BIT 8
#define TB1CN_TBCR_MASK 0x800
#define TB1CN_TBCR 0x800
#define TB1CN_TBCR_BIT 11
#define TB1CN_TBCS_MASK 0x1000
#define TB1CN_TBCS 0x1000
#define TB1CN_TBCS_BIT 12
#define TB1CN_CTB_MASK 0x8000
#define TB1CN_CTB 0x8000
#define TB1CN_CTB_BIT 15

/*sfrw*/ #define TB2CN /*=*/ 0x51  // Timer B 2 control register
/* Acknowledge a timer B interrupt safely, without masking
 * the register and accidentally clearing other bits. */
#define ACK_TB2CN6()   /*__insert_opcode(0xD167)*/
#define ACK_TB2_EXFB() /*__insert_opcode(0xD167)*/
#define ACK_TB2CN7()   /*__insert_opcode(0xD177)*/
#define ACK_TB2_TFB()  /*__insert_opcode(0xD177)*/
#define TB2CN_CPRLB_MASK 0x1
#define TB2CN_CPRLB 0x1
#define TB2CN_CPRLB_BIT 0
#define TB2CN_ETB_MASK 0x2
#define TB2CN_ETB 0x2
#define TB2CN_ETB_BIT 1
#define TB2CN_TRB_MASK 0x4
#define TB2CN_TRB 0x4
#define TB2CN_TRB_BIT 2
#define TB2CN_EXENB_MASK 0x8
#define TB2CN_EXENB 0x8
#define TB2CN_EXENB_BIT 3
#define TB2CN_DCEN_MASK 0x10
#define TB2CN_DCEN 0x10
#define TB2CN_DCEN_BIT 4
#define TB2CN_TBOE_MASK 0x20
#define TB2CN_TBOE 0x20
#define TB2CN_TBOE_BIT 5
#define TB2CN_EXFB_MASK 0x40
#define TB2CN_EXFB 0x40
#define TB2CN_EXFB_BIT 6
#define TB2CN_TFB_MASK 0x80
#define TB2CN_TFB 0x80
#define TB2CN_TFB_BIT 7
#define TB2CN_TBPS_MASK 0x700
#define TB2CN_TBPS_BIT 8
#define TB2CN_TBCR_MASK 0x800
#define TB2CN_TBCR 0x800
#define TB2CN_TBCR_BIT 11
#define TB2CN_TBCS_MASK 0x1000
#define TB2CN_TBCS 0x1000
#define TB2CN_TBCS_BIT 12
#define TB2CN_CTB_MASK 0x8000
#define TB2CN_CTB 0x8000
#define TB2CN_CTB_BIT 15

/*sfrw*/ #define TB3CN /*=*/ 0x61  // Timer B 3 control register
/* Acknowledge a timer B interrupt safely, without masking
 * the register and accidentally clearing other bits. */
#define ACK_TB3CN6()   /*__insert_opcode(0xE167)*/
#define ACK_TB3_EXFB() /*__insert_opcode(0xE167)*/
#define ACK_TB3CN7()   /*__insert_opcode(0xE177)*/
#define ACK_TB3_TFB()  /*__insert_opcode(0xE177)*/
#define TB3CN_CPRLB_MASK 0x1
#define TB3CN_CPRLB 0x1
#define TB3CN_CPRLB_BIT 0
#define TB3CN_ETB_MASK 0x2
#define TB3CN_ETB 0x2
#define TB3CN_ETB_BIT 1
#define TB3CN_TRB_MASK 0x4
#define TB3CN_TRB 0x4
#define TB3CN_TRB_BIT 2
#define TB3CN_EXENB_MASK 0x8
#define TB3CN_EXENB 0x8
#define TB3CN_EXENB_BIT 3
#define TB3CN_DCEN_MASK 0x10
#define TB3CN_DCEN 0x10
#define TB3CN_DCEN_BIT 4
#define TB3CN_TBOE_MASK 0x20
#define TB3CN_TBOE 0x20
#define TB3CN_TBOE_BIT 5
#define TB3CN_EXFB_MASK 0x40
#define TB3CN_EXFB 0x40
#define TB3CN_EXFB_BIT 6
#define TB3CN_TFB_MASK 0x80
#define TB3CN_TFB 0x80
#define TB3CN_TFB_BIT 7
#define TB3CN_TBPS_MASK 0x700
#define TB3CN_TBPS_BIT 8
#define TB3CN_TBCR_MASK 0x800
#define TB3CN_TBCR 0x800
#define TB3CN_TBCR_BIT 11
#define TB3CN_TBCS_MASK 0x1000
#define TB3CN_TBCS 0x1000
#define TB3CN_TBCS_BIT 12
#define TB3CN_CTB_MASK 0x8000
#define TB3CN_CTB 0x8000
#define TB3CN_CTB_BIT 15

/*sfrw*/ #define TB4CN /*=*/ 0x71  // Timer B 4 control register
#define TB4CN_CPRLB_MASK 0x1
#define TB4CN_CPRLB 0x1
#define TB4CN_CPRLB_BIT 0
#define TB4CN_ETB_MASK 0x2
#define TB4CN_ETB 0x2
#define TB4CN_ETB_BIT 1
#define TB4CN_TRB_MASK 0x4
#define TB4CN_TRB 0x4
#define TB4CN_TRB_BIT 2
#define TB4CN_EXENB_MASK 0x8
#define TB4CN_EXENB 0x8
#define TB4CN_EXENB_BIT 3
#define TB4CN_DCEN_MASK 0x10
#define TB4CN_DCEN 0x10
#define TB4CN_DCEN_BIT 4
#define TB4CN_TBOE_MASK 0x20
#define TB4CN_TBOE 0x20
#define TB4CN_TBOE_BIT 5
#define TB4CN_EXFB_MASK 0x40
#define TB4CN_EXFB 0x40
#define TB4CN_EXFB_BIT 6
#define TB4CN_TFB_MASK 0x80
#define TB4CN_TFB 0x80
#define TB4CN_TFB_BIT 7
#define TB4CN_TBPS_MASK 0x700
#define TB4CN_TBPS_BIT 8
#define TB4CN_TBCR_MASK 0x800
#define TB4CN_TBCR 0x800
#define TB4CN_TBCR_BIT 11
#define TB4CN_TBCS_MASK 0x1000
#define TB4CN_TBCS 0x1000
#define TB4CN_TBCS_BIT 12
#define TB4CN_CTB_MASK 0x8000
#define TB4CN_CTB 0x8000
#define TB4CN_CTB_BIT 15

/*sfrw*/ #define PI0 /*=*/ 0x81  // Port 0 input register (32 bits)

/*sfrw*/ #define PI1 /*=*/ 0x91  // Port 1 input register (32 bits)

/*sfrw*/ #define EIE /*=*/ \
    0xa1             // External interrupt enable register 0 (1 = enable)
#define EIE_EX0_MASK 0x1
#define EIE_EX0 0x1
#define EIE_EX0_BIT 0
#define EIE_EX1_MASK 0x2
#define EIE_EX1 0x2
#define EIE_EX1_BIT 1
#define EIE_EX2_MASK 0x4
#define EIE_EX2 0x4
#define EIE_EX2_BIT 2
#define EIE_EX3_MASK 0x8
#define EIE_EX3 0x8
#define EIE_EX3_BIT 3
#define EIE_EX4_MASK 0x10
#define EIE_EX4 0x10
#define EIE_EX4_BIT 4
#define EIE_EX5_MASK 0x20
#define EIE_EX5 0x20
#define EIE_EX5_BIT 5
#define EIE_EX6_MASK 0x40
#define EIE_EX6 0x40
#define EIE_EX6_BIT 6
#define EIE_EX7_MASK 0x80
#define EIE_EX7 0x80
#define EIE_EX7_BIT 7

/*sfrw*/ #define EIES /*=*/ \
    0xb1              // External interrupt edge select register 0 (1=falling)
#define EIES_IT0_MASK 0x1
#define EIES_IT0 0x1
#define EIES_IT0_BIT 0
#define EIES_IT1_MASK 0x2
#define EIES_IT1 0x2
#define EIES_IT1_BIT 1
#define EIES_IT2_MASK 0x4
#define EIES_IT2 0x4
#define EIES_IT2_BIT 2
#define EIES_IT3_MASK 0x8
#define EIES_IT3 0x8
#define EIES_IT3_BIT 3
#define EIES_IT4_MASK 0x10
#define EIES_IT4 0x10
#define EIES_IT4_BIT 4
#define EIES_IT5_MASK 0x20
#define EIES_IT5 0x20
#define EIES_IT5_BIT 5
#define EIES_IT6_MASK 0x40
#define EIES_IT6 0x40
#define EIES_IT6_BIT 6
#define EIES_IT7_MASK 0x80
#define EIES_IT7 0x80
#define EIES_IT7_BIT 7

// The CRC hardware does CCITT 16 and 32-bit CRCs.
// When calculating the CRC, do the data, without including the CRC.
// When testing for correctness, the CRC should be the last item in the data,
// it is included in the CRC operation and calculates to a fixed value,
// tested by the hardware.
/*sfrw*/ #define CRCNT /*=*/ 0xc1  // CRC control register
#define CRCNT_DONE_MASK 0x1        // 1 = CRC is correct
#define CRCNT_DONE 0x1
#define CRCNT_DONE_BIT 0
#define CRCNT_RESEED_MASK 0x2  // 1 = restart the CRC calculation
#define CRCNT_RESEED 0x2
#define CRCNT_RESEED_BIT 1
#define CRCNT_BWS_MASK 0x4  // 0 = byte, 1 = 16-bit
#define CRCNT_BWS 0x4
#define CRCNT_BWS_BIT 2
#define CRCNT_CRCMD_MASK 0x8  // 0 = CRC-16, 1 = CRC-32
#define CRCNT_CRCMD 0x8
#define CRCNT_CRCMD_BIT 3

#if 0        /* bccho, 2023-07-20 */
/*sfrw*/                                   \
#define CRC1 /*=*/                                                      \
    0xd1     // 16 bit CCITT CRC-16, or lower 16 bits of CCITT CRC-32

/*sfrw*/  \
#define CRC2 /*=*/                                                      \
    0xe1     // most significant 16 bits of CCITT CRC-32

/*sfrw*/               \
#define TB0V /*=*/                                                      \
    0xf1     // Timer B 0 value register (16 bits)

/*sfrw*/                     \
#define PD0  /*=*/                                                      \
    0x101    // Port 0 direction register, 0=input, 1=output (32 bits)

/*sfrw*/ \
#define PD1  /*=*/                                                      \
    0x111    // Port 1direction register, 0=input, 1=output (32 bits)

/*sfrw*/  \
#define TB0R /*=*/                                                      \
    0x121    // Timer B 0 reload register

/*sfrw*/                              \
#define TB0C /*=*/                                                      \
    0x131    // Timer B 0 compare register

/*sfrw*/                             \
#define TB1V /*=*/                                                      \
    0x141    // Timer B 1 value register

/*sfrw*/                               \
#define TB1R /*=*/                                                      \
    0x151    // Timer B 1 reload register

/*sfrw*/                              \
#define TB1C /*=*/                                                      \
    0x161    // Timer B 1 compare register

/*sfrw*/                             \
#define TB2V /*=*/                                                      \
    0x171    // Timer B 2 value register

/*sfrw*/                               \
#define TB2R /*=*/                                                      \
    0x181    // Timer B 2 reload register

/*sfrw*/                              \
#define TB2C /*=*/                                                      \
    0x191    // Timer B 2 compare register

/*sfrw*/                             \
#define TB3V /*=*/                                                      \
    0x1a1    // Timer B 3 value register

/*sfrw*/                               \
#define TB3R /*=*/                                                      \
    0x1b1    // Timer B 3 reload register

/*sfrw*/                              \
#define TB3C /*=*/                                                      \
    0x1c1    // Timer B 3 compare register

/*sfrw*/                             \
#define TB4V /*=*/                                                      \
    0x1d1    // Timer B 4 value register

/*sfrw*/                               \
#define TB4R /*=*/                                                      \
    0x1e1    // Timer B 4 reload register

/*sfrw*/                              \
#define TB4C /*=*/                                                      \
    0x1f1    // Timer B 4 compare register

/*sfrw*/                             \
#define MCNT /*=*/                                                      \
    0x2      // Multiplier control register
#endif

#define MCNT_SUS_MASK 0x1
#define MCNT_SUS 0x1
#define MCNT_SUS_BIT 0
#define MCNT_MMAC_MASK 0x2
#define MCNT_MMAC 0x2
#define MCNT_MMAC_BIT 1
#define MCNT_MSUB_MASK 0x4
#define MCNT_MSUB 0x4
#define MCNT_MSUB_BIT 2
#define MCNT_OPCS_MASK 0x8
#define MCNT_OPCS 0x8
#define MCNT_OPCS_BIT 3
#define MCNT_SQU_MASK 0x10
#define MCNT_SQU 0x10
#define MCNT_SQU_BIT 4
#define MCNT_CLD_MASK 0x20
#define MCNT_CLD 0x20
#define MCNT_CLD_BIT 5
#define MCNT_MCW_MASK 0x40
#define MCNT_MCW 0x40
#define MCNT_MCW_BIT 6
#define MCNT_OF_MASK 0x80
#define MCNT_OF 0x80
#define MCNT_OF_BIT 7

/*sfrw*/ #define MA /*=*/ 0x12  // Multiplier input A, 32-bit

/*sfrw*/ #define MB /*=*/ 0x22  // Multiplier input B, 32-bit

/*sfrw*/ #define M2_03 /*=*/ 0x32  // undefined

/*sfrw*/ #define MC0 /*=*/ 0x42  // Multiplier output C, low, 32-bit

/*sfrw*/ #define MC1 /*=*/ 0x52  // Multiplier output C, high, 32-bit

/*sfrw*/ #define U0CNT /*=*/ 0x62  // "ISO" uart control register (8 bits)
#define U0CNT_CONV_MASK 0x1        // 0 = inverse (use with AUTOC=1)
#define U0CNT_CONV 0x1
#define U0CNT_CONV_BIT 0
#define U0CNT_SCS_MASK 0x2  // 1 = auto
#define U0CNT_SCS 0x2
#define U0CNT_SCS_BIT 1
#define U0CNT_LCT_MASK 0x4  // 1 = last character to transmit, set with char
#define U0CNT_LCT 0x4
#define U0CNT_LCT_BIT 2
#define U0CNT_T_R_MASK 0x8  // 1 = transmit, 0 = receive
#define U0CNT_T_R 0x8
#define U0CNT_T_R_BIT 3
#define U0CNT_PROT_MASK 0x10  // 1 = "T=1", 0 = "T=0"
#define U0CNT_PROT 0x10
#define U0CNT_PROT_BIT 4
#define U0CNT_AUTOC_MASK 0x20  // 0 = use automatic decoding of convention
#define U0CNT_AUTOC 0x20
#define U0CNT_AUTOC_BIT 5
#define U0CNT_FIP_MASK 0x40  // 1 = use inverse parity
#define U0CNT_FIP 0x40
#define U0CNT_FIP_BIT 6
#define U0CNT_ISUEN_MASK 0x8  // 1 = enable (clock on)
#define U0CNT_ISUEN 0x80
#define U0CNT_ISUEN_BIT 7
#define U0CNT_FL_MASK 0x700  // Fifo length
#define U0CNT_FL_BIT 8
#define U0CNT_FTE_MASK 0x800  // Fifo threshold enable
#define U0CNT_FTE 0x800
#define U0CNT_FTE_BIT 11
#define U0CNT_PEC_MASK 0x7000  // Parity error count
#define U0CNT_PEC_BIT 12
#define U0CNT_IOSEL_MASK 0x8000  // 0=PIOm, 1=Select ISO UART interface pins
#define U0CNT_IOSEL 0x8000
#define U0CNT_IOSEL_BIT 15

/*sfrw*/ #define U0TR /*=*/ 0x72  // write=transmit/read=receive data (8 bit)

/*sfrw*/ #define U0ST /*=*/ 0x82  // ISO uart status
#define U0ST_TBE_RBF_MASK 0x1     // 1 = receive buffer full
#define U0ST_TBE_RBF 0x1
#define U0ST_TBE_RBF_BIT 0
#define U0ST_FER_MASK 0x2  // 1 = receive framing error
#define U0ST_FER 0x2
#define U0ST_FER_BIT 1
#define U0ST_OVR_MASK 0x4  // 1 = receive overrun
#define U0ST_OVR 0x4
#define U0ST_OVR_BIT 2
#define U0ST_PE_MASK 0x8  // 1 = receive parity error
#define U0ST_PE 0x8
#define U0ST_PE_BIT 3
#define U0ST_FE_MASK 0x10  // 1 = receive fifo empty
#define U0ST_FE 0x10
#define U0ST_FE_BIT 4
#define U0ST_BGT_MASK 0x2  // 0 = block guard timing error
#define U0ST_BGT 0x20
#define U0ST_BGT_BIT 5
#define U0ST_DISTBE_RBF_MASK 0x40  // 1 = disable receive or transmit interrupt
#define U0ST_DISTBE_RBF 0x40
#define U0ST_DISTBE_RBF_BIT 6
#define U0ST_EUI_MASK 0x80  // 1 = enable receive or transmit interrupt
#define U0ST_EUI 0x80
#define U0ST_EUI_BIT 7

/*sfrw*/ #define U0CK /*=*/ 0x92
#define U0CK_PD_MASK 0xFF  // ETU clock divisor (0 = 1)
#define U0CK_PD_BIT 0
#define U0CK_AC_MASK 0x700  // ISOUART_CLK/(2^AC)
#define U0CK_AC_BIT 8
#define U0CK_PSC_MASK 0x800  // 0 /*=*/ prescale=31, 1 = 32
#define U0CK_PSC 0x800
#define U0CK_PSC_BIT 11
#define U0CK_UCKOEN_MASK 0x1000  // 1 /*=*/ clock output enable
#define U0CK_UCKOEN 0x1000
#define U0CK_UCKOEN_BIT 12
#define U0CK_UCKPHA_MASK 0x2000  // 1 /*=*/ clockoutput enable on rising clock
#define U0CK_UCKPHA 0x2000
#define U0CK_UCKPHA_BIT 13
#define U0CK_UCKSEL_MASK 0x8000  // 0/*=*/clock pin is DIO, 1=clock output
#define U0CK_UCKSEL 0x8000
#define U0CK_UCKSEL_BIT 15

/*sfrw*/ #define U0GT /*=*/ 0xa2  // extra guard time (8 bits)

/*sfrw*/ #define MC0R /*=*/ 0xb2  // multipler output result low (32 bits)

/*sfrw*/ #define MC1R /*=*/ 0xc2  // multipler output result high (32 bits)

/*sfrw*/ #define FCNTL /*=*/ 0x1e2
/* Refresh the TRIMs */
#define SET_FCNTL3()   /*__insert_opcode(0x6B00) __insert_opcode(0xe2b7)*/
#define REFRESH_TRIM() /*__insert_opcode(0x6B00) __insert_opcode(0xe2b7)*/

/*sfrw*/ #define FDATA /*=*/ 0x1f2

/*sfrw*/ #define SCON0 /*=*/ 0x3  // UART 0 control register
/* Acknowledge a UART interrupt safely, without masking
 * the register and accidentally clearing other interrupt bits. */
#define ACK_RI0() /*__insert_opcode(0x8307)*/
#define SCON0_RI_MASK 0x1
#define SCON0_RI 0x1
#define SCON0_RI_BIT 0
#define ACK_TI0() /*__insert_opcode(0x8317)*/
#define SCON0_TI_MASK 0x2
#define SCON0_TI 0x2
#define SCON0_TI_BIT 1
#define SCON0_RB8_MASK 0x4
#define SCON0_RB8 0x4
#define SCON0_RB8_BIT 2
#define SCON0_TB8_MASK 0x8
#define SCON0_TB8 0x8
#define SCON0_TB8_BIT 3
#define SCON0_REN_MASK 0x10
#define SCON0_REN 0x10
#define SCON0_REN_BIT 4
#define SCON0_SM2_MASK 0x20
#define SCON0_SM2 0x20
#define SCON0_SM2_BIT 5
#define SCON0_SM1_MASK 0x40
#define SCON0_SM1 0x40
#define SCON0_SM1_BIT 6
#define SCON0_SM0_MASK 0x80
#define SCON0_SM0 0x80
#define SCON0_SM0_BIT 7
#define SCON0_FE_MASK 0x80
#define SCON0_FE 0x80
#define SCON0_FE_BIT 7

/*sfrw*/ #define SBUF0 /*=*/ 0x13  // UART 0 data read and write (8 bits)

/*sfrw*/ #define SCON1 /*=*/ 0x23  // UART 0 control register
#define ACK_RI1()                  /*__insert_opcode(0xA307)*/
#define SCON1_RI_MASK 0x1
#define SCON1_RI 0x1
#define SCON1_RI_BIT 0
#define ACK_TI1() /*__insert_opcode(0xA317)*/
#define SCON1_TI_MASK 0x2
#define SCON1_TI 0x2
#define SCON1_TI_BIT 1
#define SCON1_RB8_MASK 0x4
#define SCON1_RB8 0x4
#define SCON1_RB8_BIT 2
#define SCON1_TB8_MASK 0x8
#define SCON1_TB8 0x8
#define SCON1_TB8_BIT 3
#define SCON1_REN_MASK 0x10
#define SCON1_REN 0x10
#define SCON1_REN_BIT 4
#define SCON1_SM2_MASK 0x20
#define SCON1_SM2 0x20
#define SCON1_SM2_BIT 5
#define SCON1_SM1_MASK 0x40
#define SCON1_SM1 0x40
#define SCON1_SM1_BIT 6
#define SCON1_SM0_MASK 0x80
#define SCON1_SM0 0x80
#define SCON1_SM0_BIT 7
#define SCON1_FE_MASK 0x80
#define SCON1_FE 0x80
#define SCON1_FE_BIT 7

/*sfrw*/ #define SBUF1 /*=*/ 0x33  // UART 1 data read and write (8 bits)

/*sfrw*/ #define SCON2 /*=*/ 0x43  // UART 2 control register
#define ACK_RI2()                  /*__insert_opcode(0xC307)*/
#define SCON2_RI_MASK 0x1
#define SCON2_RI 0x1
#define SCON2_RI_BIT 0
#define ACK_TI2() /*__insert_opcode(0xC317)*/
#define SCON2_TI_MASK 0x2
#define SCON2_TI 0x2
#define SCON2_TI_BIT 1
#define SCON2_RB8_MASK 0x4
#define SCON2_RB8 0x4
#define SCON2_RB8_BIT 2
#define SCON2_TB8_MASK 0x8
#define SCON2_TB8 0x8
#define SCON2_TB8_BIT 3
#define SCON2_REN_MASK 0x10
#define SCON2_REN 0x10
#define SCON2_REN_BIT 4
#define SCON2_SM2_MASK 0x20
#define SCON2_SM2 0x20
#define SCON2_SM2_BIT 5
#define SCON2_SM1_MASK 0x40
#define SCON2_SM1 0x40
#define SCON2_SM1_BIT 6
#define SCON2_SM0_MASK 0x80
#define SCON2_SM0 0x80
#define SCON2_SM0_BIT 7
#define SCON2_FE_MASK 0x80
#define SCON2_FE 0x80
#define SCON2_FE_BIT 7

/*sfrw*/ #define SBUF2 /*=*/ 0x53  // UART 2 data read and write (8 bits)

/*sfrw*/ #define SCON3 /*=*/ 0x63  // UART 3 control register
#define ACK_RI3()                  /*__insert_opcode(0xE307)*/
#define SCON3_RI_MASK 0x1
#define SCON3_RI 0x1
#define SCON3_RI_BIT 0
#define ACK_TI3() /*__insert_opcode(0xE317)*/
#define SCON3_TI_MASK 0x2
#define SCON3_TI 0x2
#define SCON3_TI_BIT 1
#define SCON3_RB8_MASK 0x4
#define SCON3_RB8 0x4
#define SCON3_RB8_BIT 2
#define SCON3_TB8_MASK 0x8
#define SCON3_TB8 0x8
#define SCON3_TB8_BIT 3
#define SCON3_REN_MASK 0x10
#define SCON3_REN 0x10
#define SCON3_REN_BIT 4
#define SCON3_SM2_MASK 0x20
#define SCON3_SM2 0x20
#define SCON3_SM2_BIT 5
#define SCON3_SM1_MASK 0x40
#define SCON3_SM1 0x40
#define SCON3_SM1_BIT 6
#define SCON3_SM0_MASK 0x80
#define SCON3_SM0 0x80
#define SCON3_SM0_BIT 7
#define SCON3_FE_MASK 0x80
#define SCON3_FE 0x80
#define SCON3_FE_BIT 7

/*sfrw*/ #define SBUF3 /*=*/ 0x73  // UART 3 data read and write (8 bits)

/*sfrw*/ #define DEMCN /*=*/ \
    0x83               // Optical demodulator control register (32 bits)
// 1 /*=*/ AUX ADC uses optical demodulator filter, 0 = standard (sinc^2)
#define DEMCN_OPTICAL 0x00000001
// Clear the "done" to start a conversion
#define AUX_ADC_DONE 0x00000100          // 1 = auxiliary ADC data is done
#define DEMCN_AUTO_THRESHOLD 0x00010000  // 1 = demodulator threshold is auto.
#define DEMCN_CARRIER 0x80000000         // 1 = demodulator detects carrier

/*sfrw*/ #define DEMSAMP /*=*/ 0x93  // bit 20..0 contain the result
/*sfrw*/ #define AUXADCOUT /*=*/ 0x93

/*sfrw*/ #define SMD0 /*=*/ 0xa3  // UART 0 mode select
#define SMD0_FEDE0_MASK 0x1       // 1 = framing error detection
#define SMD0_FEDE0 0x1
#define SMD0_FEDE0_BIT 0
#define SMD0_SMOD0_MASK 0x2
#define SMD0_SMOD0 0x2
#define SMD0_SMOD0_BIT 1
#define SMD0_ESI0_MASK 0x4
#define SMD0_ESI0 0x4
#define SMD0_ESI0_BIT 2

/*sfrw*/ #define PR0 /*=*/ 0xb3  // baudclock = (PR*CPUCLK)/(2^17)

/*sfrw*/ #define SMD1 /*=*/ 0xc3  // UART 1 mode select
#define SMD1_FEDE0_MASK 0x1       // 1 = framing error detection
#define SMD1_FEDE0 0x1
#define SMD1_FEDE0_BIT 0
#define SMD1_SMOD0_MASK 0x2
#define SMD1_SMOD0 0x2
#define SMD1_SMOD0_BIT 1
#define SMD1_ESI0_MASK 0x4
#define SMD1_ESI0 0x4
#define SMD1_ESI0_BIT 2

/*sfrw*/ #define PR1_1 /*=*/ 0xd3  // baudclock = (PR*CPUCLK)/(2^17)

/*sfrw*/ #define SMD2 /*=*/ 0xe3  // UART 2 mode select
#define SMD2_FEDE0_MASK 0x1       // 1 = framing error detection
#define SMD2_FEDE0 0x1
#define SMD2_FEDE0_BIT 0
#define SMD2_SMOD0_MASK 0x2
#define SMD2_SMOD0 0x2
#define SMD2_SMOD0_BIT 1
#define SMD2_ESI0_MASK 0x4
#define SMD2_ESI0 0x4
#define SMD2_ESI0_BIT 2

/*sfrw*/ #define PR2_3 /*=*/ 0xf3  // baudclock = (PR*CPUCLK)/(2^17)

/*sfrw*/ #define SMD3 /*=*/ 0x103  // UART 3 mode select
#define SMD3_FEDE0_MASK 0x1        // 1 = framing error detection
#define SMD3_FEDE0 0x1
#define SMD3_FEDE0_BIT 0
#define SMD3_SMOD0_MASK 0x2
#define SMD3_SMOD0 0x2
#define SMD3_SMOD0_BIT 1
#define SMD3_ESI0_MASK 0x4
#define SMD3_ESI0 0x4
#define SMD3_ESI0_BIT 2

/*sfrw*/ #define PR3 /*=*/ 0x113  // baudclock = (PR*CPUCLK)/(2^17)

/*sfrw*/ #define DEMTHR0 /*=*/ 0x123  // Demodulator lower threshold

/*sfrw*/ #define DEMTHR1 /*=*/ 0x133  // Demodulator upper threshold

/*sfrw*/ #define DEMINT /*=*/ 0x143

/*sfrw*/ #define MUXCN /*=*/ 0x4  // ADC multiplexer (normally unused)
#define EN_BODY 0x00010000        // remote enable body bias
#define VREF_DIS 0x00020000       // ADC reference disable
#define VREF_CAL 0x01000000       // ADC reference calibration
#define FLY_DIS 0x08000000  // 1=disable "flyback" on remote ADC interfaces
#define PINV 0x10000000     // 1=invert CE pulses

/*sfrw*/ #define RMTCN /*=*/ 0x14  // Remote ADC configuration
// 0x0,0x3 /*=*/ diff. ADC inputs reverse on sample, 1=normal, 2=reverse
#define RMT_CHOP 0x3
#define RMTENA 0x4      // Enable remote channel A
#define RMTENB 0x8      // Enable remote channel B
#define RMTENC 0x10     // Enable remote channel C (three phase ICs only)
#define R6K_CLK_E 0x20  // Enable clock to remote interface (0=reduce power)
#define RMT_WPERR 0x8
#define RMT_WPERR_BIT 3
#define RMT_RPERR 0x10
#define RMT_RPERR_BIT 4
#define RMT_PARITY 0x20
#define RMT_PARITY_BIT 5

/*sfrw*/ #define RMTCMD /*=*/ 0x24  // Write a remote command, clears when sent
#define RMTCMD_ADR_MASK 0x03        // Address of remote (0=A, 1=B, 2=C)
#define RMTCMD_ADR_BIT 0
#define RMTCMD_CMD_MASK 0x1C  // Command of remote (see remote's datasheet)
#define RMTCMD_CMD_BIT 2
#define RMTCMD_TMUXR_MASK 0xE0  // TMUX value of remote (see remote's datasheet)
#define RMTCMD_TMUXR_BIT 5

/*sfrw*/ #define ADCN /*=*/ 0x34  // ADC control
// ADC Enables Disabling ADCs saves power
#define ADC0_E 0x0001  // IA/line current
#define ADC1_E 0x0002  // IB/Neutral
#define ADC2_E 0x0004  // VA/line voltage
#define ADC3_E 0x0008  // VB, MAG0, MAG1, MAG2/configurable
// Staring mode (i.e. no multiplexing)
#define ADC2_S 0x0400
#define ADC3_S 0x0800
// ADC clock inversion (for staring)
#define ADC0_CKRI 0x1000
#define ADC1_CKRI 0x2000
#define ADC2_CKRI 0x4000
#define ADC3_CKRI 0x8000

/*sfrw*/ #define ADCFG /*=*/ 0x44
// ADC re
#define ADC0_VINV 0x0001  // Chop the voltage inputs.
#define ADC1_VINV 0x0002  // Chop the voltage inputs.
// 0x004 RFU
#define VINV_ST 0x0008   // Current voltage chop state.
#define VZERO_ST 0x0010  // Read-only of current voltage zero state.
// ADC preamp enable gain is set elsewhere
#define ADC0_P 0x0100  // IA/line current
#define ADC1_P 0x0200  // IB/Neutral
// 0x400 RFU
// ADC preamp gains
#define PGAIN0 0x0C00  // 00=4x,01=8x,10=16x,11=16x
#define PGAIN1 \
    0x3000  // 00=4x,01=8x,10=16x,11=16x
            // 0xC000 RFU

/*sfrw*/ #define U1CNT /*=*/ 0x54  // ISO UART 1 control register
#define U1CNT_CONV_MASK 0x1
#define U1CNT_CONV 0x1
#define U1CNT_CONV_BIT 0
#define U1CNT_SCS_MASK 0x2
#define U1CNT_SCS 0x2
#define U1CNT_SCS_BIT 1
#define U1CNT_LCT_MASK 0x4
#define U1CNT_LCT 0x4
#define U1CNT_LCT_BIT 2
#define U1CNT_T_R_MASK 0x8
#define U1CNT_T_R 0x8
#define U1CNT_T_R_BIT 3
#define U1CNT_PROT_MASK 0x10
#define U1CNT_PROT 0x10
#define U1CNT_PROT_BIT 4
#define U1CNT_AUTOC_MASK 0x20
#define U1CNT_AUTOC 0x20
#define U1CNT_AUTOC_BIT 5
#define U1CNT_FIP_MASK 0x40
#define U1CNT_FIP 0x40
#define U1CNT_FIP_BIT 6
#define U1CNT_ISUEN_MASK 0x80
#define U1CNT_ISUEN 0x80
#define U1CNT_ISUEN_BIT 7
#define U1CNT_FL_MASK 0x700
#define U1CNT_FL_BIT 8
#define U1CNT_FTE_MASK 0x800
#define U1CNT_FTE 0x800
#define U1CNT_FTE_BIT 11
#define U1CNT_PEC_MASK 0x7000
#define U1CNT_PEC_BIT 12
#define U1CNT_IOSEL_MASK 0x8000
#define U1CNT_IOSEL 0x8000
#define U1CNT_IOSEL_BIT 15

/*sfrw*/ #define U1TR /*=*/ 0x64  // ISO UART 1 Transmit/receive register

/*sfrw*/ #define CEPCN /*=*/ \
    0x74               // Compute engine pulse control register 10ms pulses work
#define PLS_INV 0x00001ff  // Pulse interval in CLK_ADC cycles
#define PLS_INV_BIT 0
// for PLS_MAX, 0x1ff/*=*/ sq. wave with a 50% duty cycle
#define PLS_MAX 0x1ff0000  // Maximum pulse width in CLK_ADC cycles
#define PLS_MAX_BIT 16

/*sfrw*/ #define U1ST /*=*/ 0x84  // ISO UART 1 status register
#define U1ST_TBE_RBF_MASK 0x1
#define U1ST_TBE_RBF 0x1
#define U1ST_TBE_RBF_BIT 0
#define U1ST_FER_MASK 0x2
#define U1ST_FER 0x2
#define U1ST_FER_BIT 1
#define U1ST_OVR_MASK 0x4
#define U1ST_OVR 0x4
#define U1ST_OVR_BIT 2
#define U1ST_PE_MASK 0x8
#define U1ST_PE 0x8
#define U1ST_PE_BIT 3
#define U1ST_FE_MASK 0x10
#define U1ST_FE 0x10
#define U1ST_FE_BIT 4
#define U1ST_BGT_MASK 0x20
#define U1ST_BGT 0x20
#define U1ST_BGT_BIT 5
#define U1ST_DISTBE_RBF_MASK 0x40
#define U1ST_DISTBE_RBF 0x40
#define U1ST_DISTBE_RBF_BIT 6
#define U1ST_EUI_MASK 0x80
#define U1ST_EUI 0x80
#define U1ST_EUI_BIT 7

/*sfrw*/ #define U1CK /*=*/ 0x94  // ISO UART 1 clock register
#define U1CK_PD_MASK 0xFF
#define U1CK_PD_BIT 0
#define U1CK_AC_MASK 0x700
#define U1CK_AC_BIT 8
#define U1CK_PSC_MASK 0x800
#define U1CK_PSC 0x800
#define U1CK_PSC_BIT 11
#define U1CK_UCKOEN_MASK 0x1000
#define U1CK_UCKOEN 0x1000
#define U1CK_UCKOEN_BIT 12
#define U1CK_UCKPHA_MASK 0x2000
#define U1CK_UCKPHA 0x2000
#define U1CK_UCKPHA_BIT 13
#define U1CK_UCKSEL_MASK 0x8000
#define U1CK_UCKSEL 0x8000
#define U1CK_UCKSEL_BIT 15

/*sfrw*/ #define U1GT /*=*/ 0xa4  // ISO UART 1 guard time

/*sfrw*/ #define CECN /*=*/ 0xb4  // Compute engine control register
#define EQU_MASK 0x07000000       // Advises the CE code of the equation.
#define EQUATION0 0x00000000      // Wh = VA*IA
#define EQUATION1 0x01000000      // Wh = 0.5*VA*(IA-IB)
#define EQUATION2 0x02000000      // Wh = VA*IA + VB*IB
#define EQUATION5 0x05000000      // Wh = VA*IA + VB*IB + VC*IC
// Chop can induce noise, but it also makes the front-end self-zeroing.
// There are two cross enable signals: pre_cross and pre_cross_fast.
// pre_cross_fast is the high-speed chop, which chops in the preamp
// at the ADC clock rate.  It's only active when a channel's preamp is active.
#define FCHOP_EN (BIT20)
// pre_cross is the low-speed chop, which chops at the ADC sample rate.
// pre_cross chops the bandgap and the preamps together.
// They can占퐐 be chopped separately.
#define CHOP_MASK 0x00030000
// Invert pre_cross at beginning of S state except when the sum flag is set
// This alternates the chop timing on each accumulation interval.
#define CHOP_AUTO 0x00000000
#define CHOP_POSITIVE 0x00010000  // Set pre_cross to 0
#define CHOP_NEGATIVE 0x00020000  // Set pre_cross to 1
// Always invert pre_cross at beginning of S state, regardless of the sum flag.
#define CHOP_AUTO1 0x00030000
#define RTMEN (BIT17)      // Enable the real-time monitor.
#define CEEN_MASK (BIT16)  // Enable the CE
#define CEEN_BIT 16
#define SUM_SAMPS 0x01FFF  // Numebr of samples per acc. cnt

/*sfrw*/ #define CEI /*=*/ 0xc4  // Compute engine interrupt register
/* Acknowledge a CE interrupt safely, without masking
 * the register and accidentally clearing other interrupt bits. */
// CE busy, i.e. when the CE's code completes.
#define ACK_CEI0()    /*__insert_opcode(0x2b00) __insert_opcode(0xC407)*/
#define ACK_CE_BUSY() /*__insert_opcode(0x2b00) __insert_opcode(0xC407)*/
// xfer busy, i.e. when the CE's accumulation interval completes.
#define ACK_CEI1()      /*__insert_opcode(0x2b00) __insert_opcode(0xC417)*/
#define ACK_XFER_BUSY() /*__insert_opcode(0x2b00) __insert_opcode(0xC417)*/
// when there's a VAR pulse
#define ACK_CEI2()       /*__insert_opcode(0x2b00) __insert_opcode(0xC427)*/
#define ACK_VPULSE_INT() /*__insert_opcode(0x2b00) __insert_opcode(0xC427)*/
// when there's a Wh pulse
#define ACK_CEI3()       /*__insert_opcode(0x2b00) __insert_opcode(0xC437)*/
#define ACK_WPULSE_INT() /*__insert_opcode(0x2b00) __insert_opcode(0xC437)*/
// when there's an X pulse
#define ACK_CEI4()       /*__insert_opcode(0x2b00) __insert_opcode(0xC447)*/
#define ACK_XPULSE_INT() /*__insert_opcode(0x2b00) __insert_opcode(0xC447)*/
// when there's a Y pulse
#define ACK_CEI5()       /*__insert_opcode(0x2b00) __insert_opcode(0xC457)*/
#define ACK_YPULSE_INT() /*__insert_opcode(0x2b00) __insert_opcode(0xC457)*/
// when there's a MUXSYNC
#define ACK_CEI6()     /*__insert_opcode(0x2b00) __insert_opcode(0xC467)*/
#define ACK_FIR_DONE() /*__insert_opcode(0x2b00) __insert_opcode(0xC467)*/
// when there's a MUXSYNC
#define ACK_CEI7() /*__insert_opcode(0x2b00) __insert_opcode(0xC477)*/
#define ACK_SYNC() /*__insert_opcode(0x2b00) __insert_opcode(0xC477)*/

#define CE_BUSY_IE 0x0100    // Enable a CE busy interrupt
#define XFER_BUSY_IE 0x0200  // Enable a xfer busy interrupt
#define V_PLS_IE 0x0400      // Enable a V pulse interrupt
#define W_PLS_IE 0x0800      // Enable a W pulse interrupt
#define X_PLS_IE 0x1000      // Enable a X pulse interrupt
#define Y_PLS_IE 0x2000      // Enable a Y pulse interrupt
#define FIR_DONE_IE 0x4000   // Enable an interrupt when ADC finishes
#define MUX_SYNC_IE 0x8000   // Enable a muxsync interrupt

/*sfrw*/ #define RMTDATA /*=*/ \
    0xd4                 // Remote sensor data (result from commands)

/*sfrw*/ #define RMTERR /*=*/ 0xe4  // Remote error data
#define RMTERR_WRERR1 0x1           // Write parity error (local, from line)
#define RMTERR_WRERR1_BIT 0
#define RMTERR_WRERR0 0x100  // Write parity error (from remote)
#define RMTERR_WRERR0_BIT 8
#define RMTERR_RDERR1_MASK 0x10000
#define RMTERR_RDERR1 0x10000
#define RMTERR_RDERR1_BIT 16

/*sfrw*/ #define ADCLK /*=*/ 0xf4
#define ADC_SPD_2P5MHZ 0x01  // 2.5 MHz ADC clocks
#define ADC_SPD_5P0MHZ 0x02  // 5.0 MHz ADC clocks
#define CKR_DIV 0x04         // Staring/remote ADC clock division: 0=6, 1=3
#define MUX_DIV 0x00         // bit[5:4] = 0 = mux sync off
#define MUXDIV 0x30

/*sfrw*/ #define ADMUX3 /*=*/ 0x104
#define ADC3_SLOT0_VB 0x00  // Bit 0,1 = 0
#define ADC3_SLOT0_MAG1 0x01
//                      0x02    // VB
#define ADC3_SLOT0_MAG2 0x03
#define ADC3_SLOT1_VB 0x00  // Bit 2,3 = 0
#define ADC3_SLOT1_MAG1 0x04
//                      0x08    // VB
#define ADC3_SLOT1_MAG2 0x0C

/*sfrw*/ #define FIRLEN /*=*/ 0x114  // FIR length of ADCs, 32-bit register
#define FIRLEN_SLOT0 0xF             // ADC_CYCLES /*=*/ (FIR_LEN+1)*24
#define FIRLEN_SLOT1 0xF0            // ADC_CYCLES /*=*/ (FIR_LEN+1)*24
#define FIR_STR 0x3F000000           // Stretch fir length

/*sfrw*/ #define SDLY /*=*/ \
    0x124             // 16 bits: silent time in mux cycle
                      // 8:0 is delay in CKADC cycles

/*sfrw*/ #define VDLY /*=*/ \
    0x134             // 16 bits: silent time in mux cycle
                      // 8:0 is delay in CKADC cycles

/*sfrw*/ #define VZERO /*=*/ \
    0x144              // Compute engine configuration value 1, 8 bits

/*sfrw*/ #define RTM0 /*=*/ 0x154  // All RTMs are 16 bits in bits 12:2
/*sfrw*/ #define RTM1 /*=*/ 0x164
/*sfrw*/ #define RTM2 /*=*/ 0x174
/*sfrw*/ #define RTM3 /*=*/ 0x184

/*sfrw*/ #define RMTTMP               /*=*/ \
    0x194                             // Remote temperature of a MAX787xx
                                      // 15:0 is ADC0, 31:16 is ADC1
/*sfrw*/ #define RMTCTL0 /*=*/ 0x1a4  // Control a MAX787xx
#define RMT7B_CHOP 0x00000001         // Chop channel B
#define RMT7B_FIR141 0x00000000       // Fir length of channel B
#define RMT7B_FIR288 0x00000002       // Fir length of channel B
#define RMT7B_FIR384 0x00000004       // Fir length of channel B
#define RMT7B_P 0x00000008            // 1 = 9x preamp enable of channel B
#define RMT7B_DF_POS 0x00000000       // Differential positive
#define RMT7B_SE_NEG 0x00000010       // Single-ended negative
#define RMT7B_DF_NEG 0x00000020       // Differential negative
#define RMT7B_SE_POS 0x00000030       // Single-ended positive
#define RMT7A_CHOP 0x00000040         // Chop channel A
#define RMT7A_FIR141 0x00000000       // Fir length of channel A
#define RMT7A_FIR288 0x00000080       // Fir length of channel A
#define RMT7A_FIR384 0x00000100       // Fir length of channel A
#define RMT7A_P 0x00000200            // 1 = 9x preamp enable of channel A
#define RMT7A_DF_POS 0x00000000       // Differential positive
#define RMT7A_SE_NEG 0x00000400       // Single-ended negative
#define RMT7A_DF_NEG 0x00000800       // Differential negative
#define RMT7A_SE_POS 0x00000C00       // Single-ended positive
#define RMT7_BIAS 0x00003000          // bias adjustment mask
#define RMT7_BIAS_BIT 12              // bias adjustment bit number
#define RMT7_ADC1P67 0x00000000       // 1.67MHz ADC clock
#define RMT7_ADC2P50 0x00004000       // 2.50MHz ADC clock
#define RMT7_ADC3P33 0x00008000       // 3.33MHz ADC clock
#define RMT7_ADC5P00 0x0000C000       // 5.00MHz ADC clock
// In practice, the test mux selection and read command become a single command.
#define RMT7_CMD 0x00FC0000  // command mask/"read code select"
#define RMT7_CMD_BIT 18      // command bit number
/*sfrw*/ #define RMTCTL1 /*=*/ 0x1b4
// Up to bit 16, identical to RMTCTL0, except for ADC B1 and A1
#define RMT7_BG_CHOP \
    0x00020000  // Bandgap chop enable
                // command code is the same as RMTCTL0
/*sfrw*/ #define RMTCTL2 /*=*/ 0x1c4
/*sfrw*/ #define RMTCTL3 /*=*/ 0x1d4
#define RMT7_CHB_E 0x00040000    // 1 = enable a remopte 7 on IBP/IBN
#define RMT7_CHB_D 0x00080000    // 1 = no pulse transformer on IBP/IBN
#define RMT7_CHA_E 0x00100000    // 1 = enable a remopte 7 on IAP/IAN
#define RMT7_CHA_D 0x00200000    // 1 = no pulse transformer on IAP/IAN
#define RMT7_FLY_DIS 0x00400000  // 1 = flyback discharge
#define RMT7_FRMCN_E 0x00800000  // 1 = frame counter enable, FRMCNT Int. set

/*sfrw*/ #define RMTSTAT /*=*/ 0x1e4
// bit 2:0 is non zero for a data channel error.
// bit 5:3 is non zero for a control channel error.
// To clear an error, write any value to this register.

/*sfrw*/ #define RMTRG /*=*/ 0x1f4  // 8 bits
#define RMT7_CHA 0x01               // 1=IAP/AN is MAX787xx, 0 = 71M6xxx
#define RMT7_CHB 0x02               // 1=IBP/BN is MAX787xx, 0 = 71M6xxx

/*sfrw*/ #define RTCI /*=*/ 0x5  // real time clock interrupt register
/* Acknowledge one of these interrupts safely, without masking
 * the register and accidentally clearing other interrupt bits. */
/* RTCI.0, The alarm clock interrupt, is cleared by setting the
 * alarm register */
#define ACK_RTCI1()    /*__insert_opcode(0x8517)*/
#define ACK_RTC_I_TR() /*__insert_opcode(0x8517)*/
#define ACK_RTCI2()    /*__insert_opcode(0x8527)*/
#define ACK_RTC_I_VS() /*__insert_opcode(0x8527)*/
#define ACK_RTCI3()    /*__insert_opcode(0x8537)*/
#define ACK_RTC_I_FF() /*__insert_opcode(0x8537)*/
// Interrupt bits, for polling, 1 = event
#define RTC_I_AL 0x00000001  // RTC alarm interrupt
// Clear the alarm by setting a higher alarm value.
#define RTC_I_TR 0x00000002  // RTC temperature range interrupt
// Clear the bit.
#define RTC_I_VS 0x00000004  // Voltage status change interrupt
// Clear the bit.
#define RTC_I_FF 0x00000008  // Fuse checksum failure
// Clear the bit.
// Interrupt mask bits 0 = disable
#define RTC_M_AL 0x00000100  // RTC alarm interrupt
#define RTC_M_TR 0x00000200  // RTC temperature range interrupt
#define RTC_M_VS 0x00000400  // Voltage status change interrupt
#define RTC_M_FF 0x00000800  // Fuse checksum failure
// Status bits
#define PLL_OK 0x00010000     // 1 = clock system using the PLL
#define XTAL_FAIL 0x00020000  // 1 = crystal oscillator has failed
#define FUSE_FAIL 0x00040000  // 1 = fuse checksum is not correct
#define VSTAT 0x1F000000
#define VSTAT_GOOD 0x00000000     // Power is good
#define VSTAT_METER 0x01000000    // Metering is inaccurate
#define VSTAT_CPU 0x03000000      // CPU's low power warning
#define VSTAT_FLASH 0x07000000    // VDD sags Flash write can fail.
#define VSTAT_DIGITAL 0x0F000000  // VDD is too low BADVDD set if CPU on

// Selects the data of 4 segments LCDINDADDR /*=*/ SEG/4
/*sfrw*/ #define LCDINDADDR /*=*/ 0x15  // LCD indirect address register 8 bits

// LCDINDDATA has 4 segs: 31:24,23:16,15:8,7:0 7:0 /*=*/ seg with largest number
/*sfrw*/ #define LCDINDDATA /*=*/ 0x25

/*sfrw*/ #define TSW0CMP /*=*/ \
    0x35                 // Touch switch 0 comparator register. 16 bits
/*sfrw*/ #define TSW0CN /*=*/ 0x45  // Touch switch 0 control register. 8 bits
#define TSW_E 0x01      // 1=Enable the touch switch. (0 to reduce power)
#define TSW_IE 0x02     // 1=Enable the touch switch compare interrupt.
#define TSW_I 0x04      // 1=Touch switch compare interrupt occurred.
#define TSW_I_BIT 2     // Touch switch compare interrupt bit number.
#define TSW_61US 0x00   // Set a 61us monitoring period
#define TSW_122US 0x10  // Set a 122us monitoring period
#define TSW_244US 0x20  // Set a  244us monitoring period
#define TSW_488US 0x30  // Set a  488us monitoring period
/*sfrw*/ #define TSW0CNT /*=*/ 0x55  // Touch switch 0 counter.

// Exactly like TSW0 registers
/*sfrw*/ #define TSW1CMP /*=*/ 0x65  // Touch switch 1 comparator register.
/*sfrw*/ #define TSW1CN /*=*/ 0x75   // Touch switch 1 control register.
/*sfrw*/ #define TSW1CNT /*=*/ 0x85  // Touch switch 1 counter.

/*sfrw*/ #define TMUXSEL /*=*/ 0x95  // test multiplexer selection 8 bits
#define TMUX1 0x000000FF
#define TMUX0 0x00ff0000

// All the RTC registers have a protocol to protect them from errors.
// To read, wait will RTC_RD and RTC_BUSY (in WAKEFROM, below) are 0.
// Set RTC_RD, then wait till RTC_RD reads as 1, and RTC_BUSY is 0.
// Then read the RTC registers, then clear RTC_RD.
// If RTC_FAIL is set, RTC_SEC cannot be trusted, usually because of EMI.
// To write, wait will RTC_WR and RTC_BUSY (in WAKEFROM, below) are 0.
// Set RTC_WR, then write to the RTC registers, then clear RTC_WR.
// After RTC_WR and RTC_BUSY are zero, the write is complete.
// Setting RTC_FAIL clears RTC_FAIL.
/*sfrw*/ #define RTCSEC /*=*/ \
    0xa5                // 31:0 Nonvolatile count of seconds since last set.
/*sfrw*/ #define RTCSUB /*=*/ 0xb5  // 31:24 has the subsecond count, 1/256S
/*sfrw*/ #define M5_C /*=*/ 0xC5    // Undefined
/*sfrw*/ #define RTCCAL /*=*/ 0xc5

/*sfrw*/ #define RTCALARM /*=*/ \
    0xd5                  // 31:0 Nonvolatile count of seconds to fire alarm.

/*sfrw*/ #define TEMPCNTL /*=*/ 0xe5

/*sfrw*/ #define RTCWAKE /*=*/ 0xf5
#define WAKE_TMR_MASK 0x7FFF
#define EW_RU0 0x00008000    // Permit Uart 0 to wake the IC
#define EW_RU1 0x00010000    // Permit UART 1 to wake the IC.
#define EW_RU2 0x00020000    // Permit UART 2 to wake the IC.
#define EW_RU3 0x00040000    // Permit UART 3 to wake the IC.
#define EW_PB0 0x00080000    // Permit pushbutton 0 to wake the IC.
#define EW_PB1 0x00100000    // Permit pushbutton 1 to wake the IC.
#define EW_PB2 0x00200000    // Permit pushbutton 2 to wake the IC.
#define EW_PB3 0x00400000    // Permit pushbutton 3 to wake the IC.
#define EW_TMR 0x00800000    // Permit the timer to wake the IC.
#define EW_VSYS 0x02000000   // Wake on VSYS
#define TEMP_BAT 0x10000000  // Measures battery, as well as temperature.
#define TEMP_COMP \
    0x20000000  // reserved (read only) Enables temperature compensation of RTC.
#define TEMP_PWR 0x40000000   // 1=temp. sensor powered by V3P3D, 0=VBAT_RTC
#define TEMP_BSEL 0x80000000  // 1=measure VBAT, 0=VBAT_RTC

/*sfrw*/ #define TEMP /*=*/ 0x105
#define BSENSE_BIT 24
#define BSENSE_WIDTH_MASK 0xFF
#define BSENSE_LSB (1.0 / 42.7)
#define VBAT_MIN (2.3)  // Logic failure is 2V, IC normally shuts down above it.
#define TEMP_RANGE_BIT 0x16
#define TEMP_RANGE_WIDTH_MASK 0xFF
// LSB of the temp range is STEMP_LSB
#define STEMP_BIT 0
#define STEMP_WIDTH_MASK 0xFFFF

/*sfrw*/ #define TEMPALARM /*=*/ 0x115
#define TEMP_HIGH_BIT 16
#define TEMP_LOW_BIT 0
#define TEMPALARM_WIDTH_MASK 0xFFFF
// LSB of of both fields is STEMP_LSB

// RTC crystal calibration coefficients See data sheet.
/*sfrw*/ #define TCAB /*=*/ 0x125
/*sfrw*/ #define TCCD /*=*/ 0x135

/*sfrw*/ #define WAKEFROM /*=*/ 0x145
#define RTC_WR 0x00000001      // 1 = CPU sets this to write the RTC
#define RTC_RD 0x00000002      // 1 = CPU sets this to freeze the RTC for read
#define RTC_BUSY 0x00000004    // 1 = RTC busy with read, write, sleep etc.
#define WF_BADVDD 0x00000100   // 1 = NV power too low with CPU not sleeping
#define WF_OSCFAIL 0x00000200  // 1 = NV power oscillator failure in sleep
#define WF_WDOF 0x00000800     // 1 = wake from watchdog detected
#define WF_RSTBIT 0x00001000   // 1 = wake from soft reset
#define WF_RST 0x00002000      // 1 = wake from RESETB pin
#define WF_CSTART 0x00004000   // 1 = NV power failure/cold start/battery-on
#define WF_RU0 0x00008000      // 1 = wake from UART 0 RX change
#define WF_RU1 0x00010000      // 1 = wake from UART 1 RX change
#define WF_RU2 0x00020000      // 1 = wake from UART 2 RX change
#define WF_RU3 0x00040000      // 1 = wake from UART 3 RX change
#define WF_PB0 0x00080000      // 1 = wake from pushbutton 0
#define WF_PB1 0x00100000      // 1 = wake from pushbutton 1
#define WF_PB2 0x00200000      // 1 = wake from pushbutton 2
#define WF_PB3 0x00400000      // 1 = wake from pushbutton 3
#define WF_TMR 0x00800000      // 1 = wake from timer
#define WF_TMP 0x01000000      // 1 = wake from temperature change detected
#define WF_VSYS 0x02000000     // 1 = wake from VSYS detected
#define WF_TD 0x04000000       // 1 = wake from touch detected
#define RTC_FAIL 0x80000000    // 1 = RTC failure detected

/*sfrw*/ #define RTCCONT /*=*/ 0x155
#define SLEEP 0x00000001       // 1 = request IC to sleep
#define LCD_ONLY 0x00000002    // 1 = request IC to do LCD-only mode
#define TEMP_START 0x00000100  // 1 = start a temperature measurement
#define TEMP_RST 0x00000200    // 1 = reset temperature measurement (if error)
#define BCURR 0x00002000       // 1 = apply 100ua current to test battery
#define TEMP_ERR 0x00008000    // 1 = temp. measurement error.
#define WD_CLR 0x01000000      // 1 = temp. clear the watchdog.

#if 0            /* bccho, 2023-07-20 */
/*sfrw*/ \
#define TMPPEEK1 /*=*/                    \
    0x165

/*sfrw*/                                \
#define TMPPEEK2 /*=*/                    \
    0x175

/*sfrw*/                                \
#define TMPPEEK3 /*=*/                    \
    0x185

/*sfrw*/                                \
#define TMPPEEK4 /*=*/                    \
    0x195

/*sfrw*/                                \
#define LCDMAP0 /*=*/                     \
    0x1b5

/*sfrw*/                                \
#define LCDMAP1 /*=*/                     \
    0x1c5

/*sfrw*/                                \
#define LCDMAP2 /*=*/                     \
    0x1d5

/*sfrw*/                                \
#define LCDMODE /*=*/                     \
    0x1e5
#define LCD_ON 0x00000001UL
#define LCD_BLANK 0x00000002UL
#define LCD_RST 0x00000004UL
#define LCD_LOWF 0x00000008UL
#define LCD_TEST 0x00000010UL
// 20 is reserved
#define LCD_EXT_VLCD 0x000000C0UL
#define LCD_BOOST_DAC 0x00000080UL
#define LCD_DAC_ENABLE 0x00000040UL
#define LCD_VLCD_3P3V 0x00000000UL
#define LCD_Y 0x04000000UL
#define ALLCOM 0x08000000UL
// LCD Modes: states/bias
#define LCD_4_0P3 0x00000000UL
#define LCD_3_0P3 0x10000000UL
#define LCD_2_0P5 0x20000000UL
#define LCD_3_0P5 0x30000000UL
#define LCD_STATIC 0x40000000UL
#define LCD_5_0P3 0x50000000UL
#define LCD_6_0P3 0x60000000UL
#define LCD_8_0P3 0x70000000UL
#define LCD_E        \
    0x80000000UL

/*sfrw*/    \
#define LCDCTL /*=*/ \
    0x1f5
#define LCD_DAC_MASK 0x0000001FUL
#define LCD_PAGE_RW 0x00000100UL
#define LCD_PAGE_SET 0x00000200UL
// Enable timed pages
#define LCD_PAGE_TIME_4 0x00000400UL
#define LCD_PAGE_TIME_8 0x00000C00UL
#define LCD_PAGE_TIME_16 0x00001400UL
#define LCD_PAGE_TIME_32 0x00001C00UL
#endif

/*sfrw*/ #define AP /*=*/ 0x8

/*sfrw*/ #define APC /*=*/ 0x18
#define APC_MOD_MASK 0x7
#define APC_MOD_BIT 0
#define APC_IDS_MASK 0x40
#define APC_IDS 0x40
#define APC_IDS_BIT 6
#define APC_CLR_MASK 0x80
#define APC_CLR 0x80
#define APC_CLR_BIT 7

/*sfrw*/ #define PSF /*=*/ 0x48
#define PSF_E_MASK 0x1
#define PSF_E 0x1
#define PSF_E_BIT 0
#define PSF_C_MASK 0x2
#define PSF_C 0x2
#define PSF_C_BIT 1
#define PSF_OV_MASK 0x4
#define PSF_OV 0x4
#define PSF_OV_BIT 2
#define PSF_GPF0_MASK 0x8
#define PSF_GPF0 0x8
#define PSF_GPF0_BIT 3
#define PSF_GPF1_MASK 0x10
#define PSF_GPF1 0x10
#define PSF_GPF1_BIT 4
#define PSF_L_MASK 0x20
#define PSF_L 0x20
#define PSF_L_BIT 5
#define PSF_S_MASK 0x40
#define PSF_S 0x40
#define PSF_S_BIT 6
#define PSF_Z_MASK 0x80
#define PSF_Z 0x80
#define PSF_Z_BIT 7
#define PSF_P_MASK 0x100
#define PSF_P 0x100
#define PSF_P_BIT 8
#define PSF_AT_MASK 0x200
#define PSF_AT 0x200
#define PSF_AT_BIT 9

/*sfrw*/ #define IC /*=*/ 0x58
#define IC_IGE_MASK 0x1
#define IC_IGE 0x1
#define IC_IGE_BIT 0
#define IC_IPS_MASK 0x1C
#define IC_IPS_BIT 2

/*sfrw*/ #define SC /*=*/ 0x88
#define SC_CDA_MASK 0x10
#define SC_CDA 0x10
#define SC_CDA_BIT 4
#define SC_TAP_MASK 0x80
#define SC_TAP 0x80
#define SC_TAP_BIT 7
#define SC_SBPE_MASK 0x100
#define SC_SBPE 0x100
#define SC_SBPE_BIT 8
#define SC_AACCE_MASK 0x200
#define SC_AACCE 0x200
#define SC_AACCE_BIT 9

/*sfrw*/ #define IPR0 /*=*/ 0x98
#define IPR0_IPL1_MASK 0x1
#define IPR0_IPL1 0x1
#define IPR0_IPL1_BIT 0
#define IPR0_IPH1_MASK 0x2
#define IPR0_IPH1 0x2
#define IPR0_IPH1_BIT 1
#define IPR0_IPL2_MASK 0x4
#define IPR0_IPL2 0x4
#define IPR0_IPL2_BIT 2
#define IPR0_IPH2_MASK 0x8
#define IPR0_IPH2 0x8
#define IPR0_IPH2_BIT 3
#define IPR0_IPL3_MASK 0x10
#define IPR0_IPL3 0x10
#define IPR0_IPL3_BIT 4
#define IPR0_IPH3_MASK 0x20
#define IPR0_IPH3 0x20
#define IPR0_IPH3_BIT 5
#define IPR0_IPL4_MASK 0x40
#define IPR0_IPL4 0x40
#define IPR0_IPL4_BIT 6
#define IPR0_IPH4_MASK 0x80
#define IPR0_IPH4 0x80
#define IPR0_IPH4_BIT 7
#define IPR0_IPL5_MASK 0x100
#define IPR0_IPL5 0x100
#define IPR0_IPL5_BIT 8
#define IPR0_IPH5_MASK 0x200
#define IPR0_IPH5 0x200
#define IPR0_IPH5_BIT 9
#define IPR0_IPL6_MASK 0x400
#define IPR0_IPL6 0x400
#define IPR0_IPL6_BIT 10
#define IPR0_IPH6_MASK 0x800
#define IPR0_IPH6 0x800
#define IPR0_IPH6_BIT 11
#define IPR0_IPL7_MASK 0x1000
#define IPR0_IPL7 0x1000
#define IPR0_IPL7_BIT 12
#define IPR0_IPH7_MASK 0x2000
#define IPR0_IPH7 0x2000
#define IPR0_IPH7_BIT 13
#define IPR0_IPL8_MASK 0x4000
#define IPR0_IPL8 0x4000
#define IPR0_IPL8_BIT 14
#define IPR0_IPH8_MASK 0x8000
#define IPR0_IPH8 0x8000
#define IPR0_IPH8_BIT 15

/*sfrw*/ #define IPR1 /*=*/ 0xa8
#define IPR1_IPL9_MASK 0x1
#define IPR1_IPL9 0x1
#define IPR1_IPL9_BIT 0
#define IPR1_IPH9_MASK 0x2
#define IPR1_IPH9 0x2
#define IPR1_IPH9_BIT 1
#define IPR1_IPL10_MASK 0x4
#define IPR1_IPL10 0x4
#define IPR1_IPL10_BIT 2
#define IPR1_IPH10_MASK 0x8
#define IPR1_IPH10 0x8
#define IPR1_IPH10_BIT 3
#define IPR1_IPL11_MASK 0x10
#define IPR1_IPL11 0x10
#define IPR1_IPL11_BIT 4
#define IPR1_IPH11_MASK 0x20
#define IPR1_IPH11 0x20
#define IPR1_IPH11_BIT 5
#define IPR1_IPL12_MASK 0x40
#define IPR1_IPL12 0x40
#define IPR1_IPL12_BIT 6
#define IPR1_IPH12_MASK 0x80
#define IPR1_IPH12 0x80
#define IPR1_IPH12_BIT 7
#define IPR1_IPL13_MASK 0x100
#define IPR1_IPL13 0x100
#define IPR1_IPL13_BIT 8
#define IPR1_IPH13_MASK 0x200
#define IPR1_IPH13 0x200
#define IPR1_IPH13_BIT 9
#define IPR1_IPL14_MASK 0x400
#define IPR1_IPL14 0x400
#define IPR1_IPL14_BIT 10
#define IPR1_IPH14_MASK 0x800
#define IPR1_IPH14 0x800
#define IPR1_IPH14_BIT 11
#define IPR1_IPL15_MASK 0x1000
#define IPR1_IPL15 0x1000
#define IPR1_IPL15_BIT 12
#define IPR1_IPH15_MASK 0x2000
#define IPR1_IPH15 0x2000
#define IPR1_IPH15_BIT 13

/*sfrw*/ #define CKCN /*=*/ 0xe8
#define CKCN_CD0_MASK 0x1
#define CKCN_CD0 0x1
#define CKCN_CD0_BIT 0
#define CKCN_CD1_MASK 0x2
#define CKCN_CD1 0x2
#define CKCN_CD1_BIT 1
#define CKCN_PMME_MASK 0x4
#define CKCN_PMME 0x4
#define CKCN_PMME_BIT 2
#define CKCN_SWB_MASK 0x8
#define CKCN_SWB 0x8
#define CKCN_SWB_BIT 3
#define CKCN_STOP_MASK 0x10
#define CKCN_STOP 0x10
#define CKCN_STOP_BIT 4
#define CKCN_RGMD_MASK 0x20
#define CKCN_RGMD 0x20
#define CKCN_RGMD_BIT 5
#define CKCN_RGSL_MASK 0x40
#define CKCN_RGSL 0x40
#define CKCN_RGSL_BIT 6
#define CKCN_IDLE_MASK 0x100
#define CKCN_IDLE 0x100
#define CKCN_IDLE_BIT 8

/*sfrw*/ #define WDCN /*=*/ 0xf8
#define WDCN_RWT_MASK 0x1
#define WDCN_RWT 0x1
#define RESET_WDCN() /*__insert_opcode(0x2B00) __insert_opcode(0xF887)*/
#define WDCN_RWT_BIT 0
#define WDCN_EWT_MASK 0x2
#define WDCN_EWT 0x2
#define WDCN_EWT_BIT 1
#define WDCN_WTRF_MASK 0x4
#define WDCN_WTRF 0x4
#define WDCN_WTRF_BIT 2
#define WDCN_WDIF_MASK 0x8
#define WDCN_WDIF 0x8
#define ACK_WDCN3() /*__insert_opcode(0x2B00) __insert_opcode(0xF837)*/
#define ACK_WDIF()  /*__insert_opcode(0x2B00) __insert_opcode(0xF837)*/
#define WDCN_WDIF_BIT 3
#define WDCN_WD_MASK 0x30
#define WDCN_WD_BIT 4
#define WDCN_EWDI_MASK 0x40
#define WDCN_EWDI 0x40
#define WDCN_EWDI_BIT 6
#define WDCN_POR_MASK 0x80
#define WDCN_POR 0x80
#define WDCN_POR_BIT 7

/*sfrw*/ #define A_0 /*=*/ 0x9

/*sfrw*/ #define A_1 /*=*/ 0x19

/*sfrw*/ #define A_2 /*=*/ 0x29

/*sfrw*/ #define A_3 /*=*/ 0x39

/*sfrw*/ #define A_4 /*=*/ 0x49

/*sfrw*/ #define A_5 /*=*/ 0x59

/*sfrw*/ #define A_6 /*=*/ 0x69

/*sfrw*/ #define A_7 /*=*/ 0x79

/*sfrw*/ #define A_8 /*=*/ 0x89

/*sfrw*/ #define A_9 /*=*/ 0x99

/*sfrw*/ #define A_10 /*=*/ 0xa9

/*sfrw*/ #define A_11 /*=*/ 0xb9

/*sfrw*/ #define A_12 /*=*/ 0xc9

/*sfrw*/ #define A_13 /*=*/ 0xd9

/*sfrw*/ #define A_14 /*=*/ 0xe9

/*sfrw*/ #define A_15 /*=*/ 0xf9

/*sfrw*/ #define max_IP /*=*/ 0xc

/*sfrw*/ #define SP /*=*/ 0x1d

/*sfrw*/ #define LC_0 /*=*/ 0x6d

/*sfrw*/ #define LC_1 /*=*/ 0x7d

/*sfrw*/ #define BP /*=*/ 0xe

/*sfrw*/ #define Offs /*=*/ 0x3e

/*sfrw*/ #define DP_0 /*=*/ 0x3f

/*sfrw*/ #define DP_1 /*=*/ 0xdf

#endif
