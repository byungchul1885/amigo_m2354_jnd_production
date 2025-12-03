/*
******************************************************************************
*	INCLUDE
******************************************************************************
*/
#include "main.h"
#include "amg_task.h"
#include "amg_sec.h"
#include "whm.h"
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
#include "kse100_stm32l4.h"
#endif /* bccho */
#include "amg_shell.h"
#include "defines.h"
#include "appl.h"
#include "whm_log.h"
#include "amg_imagetransfer.h"
#include "amg_wdt.h"

/*
******************************************************************************
*   Definition
******************************************************************************
*/
#define _D "[SEC] "

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

/*
******************************************************************************
*	GLOBAL VARIABLES
******************************************************************************
*/

#define SEC_MM_BUFFER_SIZE 128

#if 1 /* bccho, HLS, 2023-08-14 */
hls_context_t *hls_ctx = NULL;
#endif

uint8_t SYS_TITLE_server[SYS_TITLE_LEN];
uint8_t SYS_TITLE_client[SYS_TITLE_LEN];
uint8_t CERT_DS_BUFF[CERT_LEN];
uint8_t XTOX[XTOX_LEN];   /* bccho, 2023-08-15, AARE에 있는 StoC */
uint8_t XTOX_c[XTOX_LEN]; /* bccho, 2023-08-15, AARQ에 있는 CtoS */
uint8_t fXTOX[fXTOX_LEN];
uint8_t fXTOX_s2c[fXTOX_LEN]; /* bccho, 2023-08-15, client로부터 받은 f(StoC) */

ST_CERT_CON gst_cert_con;

uint8_t abBuffer[SEC_M_BUFFER_SIZE];

uint8_t RootCa_cert[500] = {
    0x30, 0x82, 0x01, 0x95, 0x30, 0x82, 0x01, 0x3A, 0xA0, 0x03, 0x02, 0x01,
    0x02, 0x02, 0x02, 0x00, 0x83, 0x30, 0x0C, 0x06, 0x08, 0x2A, 0x86, 0x48,
    0xCE, 0x3D, 0x04, 0x03, 0x02, 0x05, 0x00, 0x30, 0x30, 0x31, 0x0B, 0x30,
    0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x6B, 0x72, 0x31, 0x0E,
    0x30, 0x0C, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x05, 0x6B, 0x65, 0x70,
    0x63, 0x6F, 0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C,
    0x08, 0x52, 0x4F, 0x4F, 0x54, 0x43, 0x41, 0x30, 0x31, 0x30, 0x1E, 0x17,
    0x0D, 0x31, 0x38, 0x30, 0x37, 0x31, 0x35, 0x30, 0x38, 0x30, 0x30, 0x30,
    0x30, 0x5A, 0x17, 0x0D, 0x33, 0x33, 0x30, 0x37, 0x31, 0x35, 0x30, 0x37,
    0x35, 0x39, 0x35, 0x39, 0x5A, 0x30, 0x30, 0x31, 0x0B, 0x30, 0x09, 0x06,
    0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x6B, 0x72, 0x31, 0x0E, 0x30, 0x0C,
    0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x05, 0x6B, 0x65, 0x70, 0x63, 0x6F,
    0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x08, 0x52,
    0x4F, 0x4F, 0x54, 0x43, 0x41, 0x30, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06,
    0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86,
    0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xAE, 0xED,
    0x59, 0xAE, 0x72, 0x96, 0x47, 0xED, 0x5F, 0x48, 0xE2, 0xA5, 0x9B, 0x96,
    0x4D, 0x7B, 0x55, 0xDE, 0xC4, 0xD4, 0x0B, 0x7F, 0xBC, 0xCA, 0x0C, 0x1F,
    0x82, 0x1C, 0x30, 0xD3, 0xA7, 0x9C, 0x00, 0xBC, 0x7A, 0x9A, 0x44, 0x6F,
    0x3C, 0xA5, 0x14, 0x6C, 0x4F, 0xD0, 0x1E, 0x06, 0xA4, 0x58, 0xC6, 0xE5,
    0xBC, 0x2A, 0x0A, 0xF1, 0x08, 0x33, 0x76, 0xA4, 0xAE, 0x33, 0x3B, 0xF9,
    0x5F, 0x97, 0xA3, 0x42, 0x30, 0x40, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D,
    0x0E, 0x04, 0x16, 0x04, 0x14, 0x2C, 0xBB, 0xA0, 0x5A, 0x00, 0xA3, 0x27,
    0x9E, 0x4D, 0xD9, 0x92, 0x07, 0x97, 0x47, 0xD3, 0xB8, 0x5C, 0x5F, 0x33,
    0xC4, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x01, 0x01, 0xFF, 0x04,
    0x04, 0x03, 0x02, 0x01, 0xC6, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x1D, 0x13,
    0x01, 0x01, 0xFF, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF, 0x30, 0x0C,
    0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x05, 0x00,
    0x03, 0x47, 0x00, 0x30, 0x44, 0x02, 0x20, 0x72, 0xA0, 0xFD, 0xED, 0xBB,
    0xE6, 0xB5, 0x32, 0xC3, 0x77, 0x06, 0x80, 0x44, 0xA5, 0xEC, 0x8D, 0xAE,
    0xF4, 0x82, 0x0A, 0x8A, 0x67, 0x3F, 0xE0, 0xFB, 0x59, 0xBE, 0xF9, 0xE7,
    0xA9, 0xE3, 0x64, 0x02, 0x20, 0x2B, 0xB4, 0xBF, 0xE6, 0xCE, 0xD7, 0xDC,
    0x58, 0xD8, 0x35, 0x23, 0xC5, 0x48, 0xFC, 0x90, 0xBC, 0xFE, 0x67, 0x8B,
    0xD8, 0x55, 0x3E, 0xB9, 0xE6, 0x71, 0x19, 0xB2, 0x43, 0xD4, 0xD0, 0x92,
    0xB3};

uint8_t SubCa_cert[500] = {
    0x30, 0x82, 0x01, 0x99, 0x30, 0x82, 0x01, 0x3D, 0xA0, 0x03, 0x02, 0x01,
    0x02, 0x02, 0x02, 0x00, 0x85, 0x30, 0x0C, 0x06, 0x08, 0x2A, 0x86, 0x48,
    0xCE, 0x3D, 0x04, 0x03, 0x02, 0x05, 0x00, 0x30, 0x30, 0x31, 0x0B, 0x30,
    0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x6B, 0x72, 0x31, 0x0E,
    0x30, 0x0C, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x05, 0x6B, 0x65, 0x70,
    0x63, 0x6F, 0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C,
    0x08, 0x52, 0x4F, 0x4F, 0x54, 0x43, 0x41, 0x30, 0x31, 0x30, 0x1E, 0x17,
    0x0D, 0x31, 0x38, 0x30, 0x37, 0x31, 0x35, 0x30, 0x38, 0x30, 0x30, 0x30,
    0x30, 0x5A, 0x17, 0x0D, 0x33, 0x32, 0x30, 0x39, 0x31, 0x35, 0x30, 0x37,
    0x35, 0x39, 0x35, 0x39, 0x5A, 0x30, 0x30, 0x31, 0x0B, 0x30, 0x09, 0x06,
    0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x6B, 0x72, 0x31, 0x0E, 0x30, 0x0C,
    0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x05, 0x6B, 0x65, 0x70, 0x63, 0x6F,
    0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x08, 0x41,
    0x4D, 0x49, 0x43, 0x41, 0x30, 0x30, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06,
    0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86,
    0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xDA, 0x2F,
    0x38, 0x6E, 0x09, 0x15, 0x8D, 0xA3, 0xA5, 0x4A, 0xCE, 0x95, 0xE4, 0x9F,
    0x18, 0x65, 0x4C, 0xCE, 0xCB, 0xA5, 0xCB, 0x79, 0x48, 0xA5, 0x84, 0x20,
    0x8F, 0x6E, 0xA0, 0xF8, 0xDD, 0x75, 0xA5, 0x5C, 0xAC, 0xAF, 0x2B, 0xC7,
    0x07, 0x1A, 0xDF, 0x2D, 0x52, 0x9D, 0x26, 0x88, 0x81, 0xC5, 0xEC, 0xA5,
    0xF2, 0xE3, 0xE7, 0x1A, 0xBA, 0x59, 0xE5, 0xD5, 0xBD, 0xE1, 0xE6, 0xDE,
    0x9E, 0x81, 0xA3, 0x45, 0x30, 0x43, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D,
    0x0E, 0x04, 0x16, 0x04, 0x14, 0x2F, 0x3B, 0xD6, 0xED, 0xF8, 0xC2, 0xCE,
    0x39, 0xCD, 0xC1, 0xE4, 0xF3, 0x79, 0xEA, 0xBC, 0xFD, 0x49, 0x3E, 0x7F,
    0x78, 0x30, 0x0E, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x01, 0x01, 0xFF, 0x04,
    0x04, 0x03, 0x02, 0x01, 0xBE, 0x30, 0x12, 0x06, 0x03, 0x55, 0x1D, 0x13,
    0x01, 0x01, 0xFF, 0x04, 0x08, 0x30, 0x06, 0x01, 0x01, 0xFF, 0x02, 0x01,
    0x00, 0x30, 0x0C, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03,
    0x02, 0x05, 0x00, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02, 0x21, 0x00, 0x9E,
    0xA0, 0xF1, 0xC6, 0x61, 0x76, 0xF2, 0x24, 0x3A, 0x4A, 0x64, 0x45, 0x33,
    0xBE, 0x2A, 0x9F, 0x3D, 0x0B, 0x01, 0x47, 0xE4, 0xA3, 0x01, 0x99, 0x6C,
    0xD4, 0xD5, 0x91, 0x61, 0x9F, 0xDF, 0x31, 0x02, 0x20, 0x6F, 0x12, 0x24,
    0x93, 0x4E, 0x78, 0x04, 0xEB, 0x77, 0xE4, 0xBB, 0x39, 0x0D, 0xA9, 0x69,
    0x50, 0x25, 0x27, 0x8D, 0xE0, 0x10, 0x8F, 0x15, 0x19, 0xAC, 0xFF, 0xF6,
    0x9A, 0xFB, 0x87, 0x7F, 0xCA};

// array size is 476
uint8_t CERT_DS_BUFF[500] = {
    0x30, 0x82, 0x01, 0xd8, 0x30, 0x82, 0x01, 0x7c, 0xa0, 0x03, 0x02, 0x01,
    0x02, 0x02, 0x04, 0x01, 0xbc, 0x45, 0x8d, 0x30, 0x0c, 0x06, 0x08, 0x2a,
    0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x05, 0x00, 0x30, 0x30, 0x31,
    0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x6b, 0x72,
    0x31, 0x0e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x05, 0x6b,
    0x65, 0x70, 0x63, 0x6f, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04,
    0x03, 0x0c, 0x08, 0x41, 0x4d, 0x49, 0x43, 0x41, 0x30, 0x30, 0x31, 0x30,
    0x20, 0x17, 0x0d, 0x32, 0x34, 0x30, 0x34, 0x30, 0x31, 0x32, 0x30, 0x30,
    0x31, 0x32, 0x32, 0x5a, 0x18, 0x0f, 0x39, 0x39, 0x39, 0x39, 0x31, 0x32,
    0x33, 0x31, 0x31, 0x34, 0x35, 0x39, 0x35, 0x39, 0x5a, 0x30, 0x43, 0x31,
    0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x6b, 0x72,
    0x31, 0x0e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x05, 0x6b,
    0x65, 0x70, 0x63, 0x6f, 0x31, 0x0c, 0x30, 0x0a, 0x06, 0x03, 0x55, 0x04,
    0x0b, 0x0c, 0x03, 0x41, 0x4d, 0x49, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03,
    0x55, 0x04, 0x03, 0x0c, 0x0d, 0x4a, 0x4e, 0x44, 0x31, 0x33, 0x32, 0x34,
    0x30, 0x30, 0x30, 0x30, 0x31, 0x30, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07,
    0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02, 0x01, 0x06, 0x08, 0x2a, 0x86, 0x48,
    0xce, 0x3d, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0xa1, 0xdd, 0x62,
    0xd6, 0xa2, 0x27, 0x13, 0xfe, 0x53, 0x45, 0x59, 0x4e, 0x3b, 0xd3, 0x85,
    0xe9, 0x21, 0x0d, 0x58, 0x41, 0x23, 0xf2, 0xbc, 0x51, 0x1e, 0x99, 0x16,
    0x85, 0x21, 0x57, 0x43, 0x59, 0x49, 0x24, 0xa2, 0x7a, 0x5c, 0x05, 0xca,
    0x00, 0x67, 0xef, 0x55, 0x5e, 0x0e, 0x16, 0x67, 0x8a, 0xd8, 0x57, 0x2c,
    0xfa, 0xc3, 0xef, 0x41, 0x8d, 0x59, 0x66, 0x39, 0x13, 0x4c, 0x07, 0x53,
    0xb5, 0xa3, 0x6d, 0x30, 0x6b, 0x30, 0x59, 0x06, 0x03, 0x55, 0x1d, 0x23,
    0x04, 0x52, 0x30, 0x50, 0x80, 0x14, 0x2f, 0x3b, 0xd6, 0xed, 0xf8, 0xc2,
    0xce, 0x39, 0xcd, 0xc1, 0xe4, 0xf3, 0x79, 0xea, 0xbc, 0xfd, 0x49, 0x3e,
    0x7f, 0x78, 0xa1, 0x34, 0xa4, 0x32, 0x30, 0x30, 0x31, 0x0b, 0x30, 0x09,
    0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x6b, 0x72, 0x31, 0x0e, 0x30,
    0x0c, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x05, 0x6b, 0x65, 0x70, 0x63,
    0x6f, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x08,
    0x52, 0x4f, 0x4f, 0x54, 0x43, 0x41, 0x30, 0x31, 0x82, 0x02, 0x00, 0x85,
    0x30, 0x0e, 0x06, 0x03, 0x55, 0x1d, 0x0f, 0x01, 0x01, 0xff, 0x04, 0x04,
    0x03, 0x02, 0x06, 0xc0, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce,
    0x3d, 0x04, 0x03, 0x02, 0x05, 0x00, 0x03, 0x48, 0x00, 0x30, 0x45, 0x02,
    0x20, 0x3e, 0x7c, 0x6d, 0x18, 0xb6, 0xe4, 0xbd, 0x6f, 0xf9, 0x4e, 0xba,
    0xb1, 0x56, 0x4b, 0x39, 0xa1, 0x96, 0xe3, 0xb0, 0x56, 0xd4, 0x36, 0xee,
    0x24, 0x9c, 0xdf, 0xcb, 0xe1, 0x9b, 0x4c, 0x8f, 0xfe, 0x02, 0x21, 0x00,
    0xee, 0x0f, 0x21, 0x82, 0xc3, 0xcb, 0x60, 0x71, 0x45, 0x07, 0x83, 0x30,
    0xdb, 0xcf, 0x56, 0x56, 0x24, 0x5c, 0xf4, 0x1d, 0x4b, 0x02, 0xc4, 0x8a,
    0x2e, 0xbc, 0x4b, 0x64, 0x36, 0xc4, 0x55, 0xf7};

// array size is 32
uint8_t DS_privatekey[32] = {0x21, 0x1c, 0x5b, 0xe5, 0x87, 0x10, 0x33, 0x68,
                             0xe5, 0x42, 0xcc, 0xd4, 0x4f, 0x22, 0x0f, 0x5e,
                             0xc0, 0x31, 0x26, 0xe7, 0x0b, 0xff, 0x61, 0x80,
                             0xdf, 0x54, 0x31, 0xc1, 0xdc, 0x5d, 0xd3, 0xf1};

uint8_t KTC_RootCa_cert[500] = {
    0x30, 0x82, 0x01, 0x26, 0x30, 0x81, 0xCE, 0xA0, 0x03, 0x02, 0x01, 0x02,
    0x02, 0x08, 0x29, 0x49, 0x91, 0xE6, 0x3A, 0xF3, 0xE2, 0xDE, 0x30, 0x0A,
    0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x30, 0x19,
    0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x0E, 0x4B,
    0x54, 0x43, 0x20, 0x46, 0x57, 0x20, 0x54, 0x45, 0x53, 0x54, 0x20, 0x43,
    0x41, 0x30, 0x20, 0x17, 0x0D, 0x32, 0x30, 0x31, 0x30, 0x31, 0x33, 0x31,
    0x30, 0x31, 0x37, 0x33, 0x31, 0x5A, 0x18, 0x0F, 0x33, 0x30, 0x32, 0x30,
    0x31, 0x30, 0x31, 0x33, 0x31, 0x30, 0x31, 0x37, 0x33, 0x31, 0x5A, 0x30,
    0x19, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x0E,
    0x4B, 0x54, 0x43, 0x20, 0x46, 0x57, 0x20, 0x54, 0x45, 0x53, 0x54, 0x20,
    0x43, 0x41, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE,
    0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01,
    0x07, 0x03, 0x42, 0x00, 0x04, 0x34, 0x90, 0xB3, 0x55, 0x5B, 0xFC, 0xF1,
    0xD7, 0xAD, 0xBB, 0xD9, 0x10, 0x9A, 0x11, 0xA8, 0xB6, 0x64, 0x22, 0x83,
    0x4B, 0x73, 0x66, 0x40, 0x52, 0xC7, 0x8D, 0xC8, 0x9A, 0x30, 0x06, 0xEE,
    0x20, 0x97, 0x6A, 0x7D, 0x7C, 0x25, 0xF4, 0x11, 0x55, 0xA0, 0x1A, 0xF9,
    0x61, 0x02, 0xBA, 0x17, 0x45, 0xCD, 0x99, 0x3F, 0x3E, 0x64, 0xCB, 0x87,
    0x5A, 0xE9, 0x18, 0xB1, 0xB1, 0xD2, 0x48, 0x89, 0xA9, 0x30, 0x0A, 0x06,
    0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x47, 0x00,
    0x30, 0x44, 0x02, 0x20, 0x19, 0x12, 0x03, 0xA4, 0x4A, 0x16, 0xDB, 0x4E,
    0xB7, 0xAE, 0x59, 0xB7, 0x42, 0x56, 0xDD, 0x59, 0x09, 0x21, 0x4F, 0x77,
    0xF2, 0x8F, 0x5C, 0x17, 0x7A, 0x38, 0x44, 0x64, 0x06, 0x26, 0x16, 0xD3,
    0x02, 0x20, 0x76, 0xD5, 0x7B, 0x70, 0xA9, 0x17, 0xBF, 0x97, 0xF0, 0x28,
    0xBD, 0xD0, 0xD4, 0x13, 0xCC, 0x16, 0x6E, 0xC7, 0xC5, 0x28, 0xEE, 0xEA,
    0x10, 0x95, 0x85, 0x9D, 0x4D, 0x53, 0xE2, 0x41, 0xE2, 0xF0};

/*
******************************************************************************
*	LOCAL VARIABLES
******************************************************************************
*/
ST_SEC_M_POWER_ON_RLT power_on_rlt;
uint8_t server_key_agreedata[129];
T_DLMS_SC g_req_sc_field;
uint32_t g_sec_invocation_count = 0;
uint8_t g_security_suite = SECURITY_SUITE_ARIA_128GCM_ECDSAP256SIGN;
ST_CSR_INFO gst_csr_info;
bool sec_module_access = false;
/*
******************************************************************************
*	LOCAL FUNCTION PROTOTYPES
******************************************************************************
*/

int16_t dsm_sec_cert_get(uint32_t party, uint32_t cert_idx,
                         uint32_t sys_t_backup_flag);
int16_t dsm_sec_cert_put(uint32_t party, uint32_t cert_type, uint8_t *pcert,
                         uint16_t cert_size);

/*
******************************************************************************
*   FUNCTIONS - extern
******************************************************************************
*/

/*
******************************************************************************
*	GLOBAL FUNCTIONS
******************************************************************************
*/
void dsm_sec_set_operation(bool val) { sec_module_access = val; }

bool dsm_sec_get_operation(void) { return sec_module_access; }

char *dsm_cert_string(uint32_t party, uint32_t cert_idx)
{
    if (party == SEC_SERVER)
    {
        switch (cert_idx)
        {
        case CERT_ROOTCA:
            return "CERT_ROOTCA";
        case CERT_SUBCA:
            return "CERT_SUCA";
        case CERT_DS:
            return "CERT_DS";
        case CERT_KA:
            return "CERT_KA";

        default:
            return "Unknown";
        }
    }
    else if (party == SEC_KTC)
    {
        switch (cert_idx)
        {
        case CERT_KTC_ROOTCA:
            return "CERT_KTC_ROOTCA";
        case CERT_KTC_MT_CERT_DS:
            return "CERT_KTC_MT_CERT_DS";
        case CERT_KTC_MT_CERT_TMP_DS:
            return "CERT_KTC_MT_CERT_TMP_DS";

        default:
            return "Unknown";
        }
    }
    else
    {
        switch (cert_idx)
        {
        case CERT_DS_CLIENT:
            return "CERT_DS_CLIENT";
        case CERT_KA_CLIENT:
            return "CERT_KA_CLIENT";

        default:
            return "Unknown";
        }
    }
    return "Unknown";
}

/* 4 byte align 되어야 한다 */
typedef struct
{
    uint8_t key[32];
    uint8_t certi[480];
} __attribute__((__packed__)) SEC_DATA;

/* bccho, 2024-05-17, 사용안함 */
static void load_certificate(void)
{
    MSGALWAYS("load_certificate()");

    SEC_DATA sd;
    if (FMC_ReadBytes_S(FMC_DTFSH_BASE, (uint32_t *)&sd, sizeof(SEC_DATA)) != 0)
    {
        MSGERROR("FMC_ReadBytes_S from Data Flash\n");
    }

    if (sd.key[0] == 0xff && sd.key[1] == 0xff && sd.key[2] == 0xff)
    {
        /* JND1324000010 */
        memcpy(SYS_TITLE_server, "\x4a\x4e\x44\x13\x24\x00\x00\x0a",
               SYS_TITLE_LEN);
        return;
    }

    DPRINT_HEX(DBG_NONE, "Key", &sd.key, sizeof(sd.key), DUMP_ALWAYS);
    DPRINT_HEX(DBG_NONE, "Certi", &sd.certi, sizeof(sd.certi), DUMP_ALWAYS);

    uint8_t sys_t_raw[13];
    memcpy(sys_t_raw, &sd.certi[0xAD], 13);

    DPRINT_HEX(DBG_NONE, "sys_t_raw", sys_t_raw, 13, DUMP_ALWAYS);

    uint8_t sys_t[8];
    sys_t[0] = sys_t_raw[0];
    sys_t[1] = sys_t_raw[1];
    sys_t[2] = sys_t_raw[2];

    uint8_t AsciiToHEX(unsigned char ch);
    for (int i = 3; i < 13; i++)
    {
        sys_t_raw[i] = AsciiToHEX(sys_t_raw[i]);
    }
    sys_t[3] = (sys_t_raw[3] << 4) + sys_t_raw[4];
    sys_t[4] = ((sys_t_raw[5]) << 4) + sys_t_raw[6];
    sys_t[5] = ((sys_t_raw[7]) << 4) + sys_t_raw[8];
    sys_t[6] = ((sys_t_raw[9]) << 4) + sys_t_raw[10];
    sys_t[7] = ((sys_t_raw[11]) << 4) + sys_t_raw[12];

    DPRINT_HEX(DBG_NONE, "sys_t", sys_t, 8, DUMP_ALWAYS);

    MSGALWAYS("SYS_T: %02X %02X %02X %02X %02X %02X %02X %02X", sys_t[0],
              sys_t[1], sys_t[2], sys_t[3], sys_t[4], sys_t[5], sys_t[6],
              sys_t[7]);

    memcpy(SYS_TITLE_server, sys_t, SYS_TITLE_LEN);
    memcpy(DS_privatekey, sd.key, 32);

    memset(CERT_DS_BUFF, 0xff, 500);
    memcpy(CERT_DS_BUFF, sd.certi, 480);
}

#if 1 /* bccho, HLS, 2023-08-14 */
bool hls_sec_init(void)
{
#if 0 /* bccho, 2024-05-17, 양산버젼은 다른 곳에서 인증서 읽어옴 */
    load_certificate();
#endif

    MSGALWAYS("hls_init()_start");
    hls_ctx = hls_init(HLS_OPMODE_SERVER);
    if (hls_ctx == NULL)
    {
        DPRINTF(DBG_ERR, "hls_init() error!!!\r\n");
        return FALSE;
    }
    MSGALWAYS("hls_init()_completed");

    int rtn = hls_set_system_title(hls_ctx, SYS_TITLE_server, SYS_TITLE_LEN);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_set_system_title() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    uint8_t DS_pulblickey[64];
    int pubkey_len;
    rtn = hls_get_pubkey_from_cert(CERT_DS_BUFF, CERT_LEN, DS_pulblickey,
                                   &pubkey_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_get_pubkey_from_cert() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    DPRINT_HEX(DBG_INFO, "Public Key", DS_pulblickey, pubkey_len, DUMP_ALWAYS);
    DPRINT_HEX(DBG_INFO, "Private Key", DS_privatekey, 32, DUMP_ALWAYS);

    rtn = hls_set_ecdsa_key(hls_ctx, DS_privatekey, 32, DS_pulblickey,
                            pubkey_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_set_ecdsa_key() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    MSGALWAYS("hls_sec_init()_completed");
    return TRUE;
}
#endif

int16_t dsm_sec_initialize(void)
{
    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    memset(&gst_cert_con, 0x00, sizeof(ST_CERT_CON));
    memset(&gst_csr_info, 0x00, sizeof(ST_CSR_INFO));

    return TRUE;

#if 0 /* bccho, HLS, 2023-08-14 */
    MSGALWAYS("hls_init()_start");
    hls_ctx = hls_init(HLS_OPMODE_SERVER);
    if (hls_ctx == NULL)
    {
        DPRINTF(DBG_ERR, "hls_init() error!!!\r\n");
        return FALSE;
    }
    MSGALWAYS("hls_init()_completed");

    memcpy(SYS_TITLE_server, "\x4a\x42\x58\x20\x19\x00\x00\x01", SYS_TITLE_LEN);

    int rtn = hls_set_system_title(hls_ctx, SYS_TITLE_server, SYS_TITLE_LEN);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_set_system_title() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    uint8_t DS_pulblickey[64];
    int pubkey_len;
    rtn = hls_get_pubkey_from_cert(CERT_DS_BUFF, CERT_LEN, DS_pulblickey,
                                   &pubkey_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_get_pubkey_from_cert() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    DPRINT_HEX(DBG_INFO, "Public Key", DS_pulblickey, pubkey_len, DUMP_ALWAYS);
    DPRINT_HEX(DBG_INFO, "Private Key", DS_privatekey, 32, DUMP_ALWAYS);

    rtn = hls_set_ecdsa_key(hls_ctx, DS_privatekey, 32, DS_pulblickey,
                            pubkey_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_set_ecdsa_key() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    return TRUE;
#else
    // int16_t sRv = 0;
    // sRv = _ksePowerOn(
    //     power_on_rlt.abVer, &power_on_rlt.bLifeCycle,
    //     power_on_rlt.chipSerial, power_on_rlt.abSystemTitle,
    //     &power_on_rlt.bVcType, &power_on_rlt.bMaxVcRetryCount,
    //     &power_on_rlt.usMaxKcmvpKeyCount, &power_on_rlt.usMaxCertKeyCount,
    //     &power_on_rlt.usMaxIoDataSize, &power_on_rlt.usInfoFileSize);

    // if (sRv != KSE_FAIL_ALREADY_POWERED_ON)
    // {
    //     if (sRv != KSE_SUCCESS)
    //     {
    //         return FALSE;
    //     }
    // }

    // dsm_sec_por_print(&power_on_rlt);

    // if (power_on_rlt.abVer[0] == 0x02 && power_on_rlt.abVer[1] == 0 &&
    //     power_on_rlt.abVer[2] == 0xFF)
    // {
    //     DPRINTF(DBG_WARN, "SAMPLE version!!!\r\n");
    // }

    // if (power_on_rlt.bLifeCycle == LC_TERMINATED)
    // {
    //     return FALSE;
    // }

    // if (power_on_rlt.bLifeCycle == LC_ISSUED)
    // {
    //     memcpy(SYS_TITLE_server, power_on_rlt.abSystemTitle, SYS_TITLE_LEN);
    //     DPRINT_HEX(DBG_WARN, "SYS_TITLE_server", SYS_TITLE_server,
    //                SYS_TITLE_LEN, DUMP_ALWAYS);

    //     sRv |= dsm_sec_cert_get(SEC_SERVER, CERT_ROOTCA, FALSE);
    //     sRv |= dsm_sec_cert_get(SEC_SERVER, CERT_SUBCA, FALSE);
    //     sRv |= dsm_sec_cert_get(SEC_SERVER, CERT_DS, FALSE);
    // }

    // OSTimeDly(OS_MS2TICK(1));

    // if (sRv == 0)
    // {
    //     return TRUE;
    // }

    // log_cert_ng(SEC_LOG_M_INIT_MIS);

    // return FALSE;
#endif /* bccho */
}

int16_t dsm_sec_deinitialize(void)
{
#if 0  /* bccho, KEYPAIR, 2023-07-15 */
    dsm_wdt_ext_toggle_immd();
    _ksePowerOff();
#endif /* bccho */
    return TRUE;
}

void dsm_sec_recovery(void)
{
#if 0 /* bccho, KEYPAIR, 2023-07-15 */    
    DPRINTF(DBG_ERR, "< %s >\r\n", __func__);
    dsm_sec_deinitialize();
    dsm_sec_initialize();
#endif
}

int16_t dsm_sec_issue_clear(void)
{
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
    int16_t sRv = 0;

    dsm_wdt_ext_toggle_immd();

    sRv |= _ksePowerOn(
        power_on_rlt.abVer, &power_on_rlt.bLifeCycle, power_on_rlt.chipSerial,
        power_on_rlt.abSystemTitle, &power_on_rlt.bVcType,
        &power_on_rlt.bMaxVcRetryCount, &power_on_rlt.usMaxKcmvpKeyCount,
        &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
        &power_on_rlt.usInfoFileSize);

    OSTimeDly(OS_MS2TICK(1));

    dsm_sec_por_print(&power_on_rlt);

    sRv |= _kseClear(CLEAR_ALL);
    OSTimeDly(OS_MS2TICK(1));

    sRv |= _ksePowerOff();
    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: Fail!!!\r\n", __func__);
    dsm_sec_recovery();

    return FALSE;
#else
    return TRUE;
#endif /* bccho */
}

int16_t dsm_sec_issue(uint32_t power_on_force)
{
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
    uint8_t issue_systile[8] = {0x4A, 0x42, 0x58, 0x20, 0x19, 0x00, 0x00, 0x01};

    DPRINTF(DBG_TRACE, "%s: power_on_force[%d]\r\n", __func__, power_on_force);

    dsm_wdt_ext_toggle_immd();

    if (power_on_force)
    {
        sRv = _ksePowerOn(
            power_on_rlt.abVer, &power_on_rlt.bLifeCycle,
            power_on_rlt.chipSerial, power_on_rlt.abSystemTitle,
            &power_on_rlt.bVcType, &power_on_rlt.bMaxVcRetryCount,
            &power_on_rlt.usMaxKcmvpKeyCount, &power_on_rlt.usMaxCertKeyCount,
            &power_on_rlt.usMaxIoDataSize, &power_on_rlt.usInfoFileSize);

        if (sRv == KSE_FAIL_ALREADY_POWERED_ON)
        {
            sRv = _ksePowerOff();
            OSTimeDly(OS_MS2TICK(1));
            sRv = _ksePowerOn(
                power_on_rlt.abVer, &power_on_rlt.bLifeCycle,
                power_on_rlt.chipSerial, power_on_rlt.abSystemTitle,
                &power_on_rlt.bVcType, &power_on_rlt.bMaxVcRetryCount,
                &power_on_rlt.usMaxKcmvpKeyCount,
                &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
                &power_on_rlt.usInfoFileSize);
        }
        dsm_sec_por_print(&power_on_rlt);

        if (power_on_rlt.bLifeCycle != LC_MANUFACTURED)
        {
            OSTimeDly(OS_MS2TICK(1));
            sRv |= _kseClear(CLEAR_ALL);
        }
    }
    else
    {
        OSTimeDly(OS_MS2TICK(1));
        sRv |= _kseClear(CLEAR_ALL);
    }
    OSTimeDly(OS_MS2TICK(1));

    dsm_wdt_ext_toggle_immd();

    sRv |= _kseIssueInit((uint8_t *)issue_systile);

    OSTimeDly(OS_MS2TICK(1));
    sRv |= _kseIssueFinal();
    OSTimeDly(OS_MS2TICK(1));
    sRv |= _ksePowerOff();
    OSTimeDly(OS_MS2TICK(1));

    sRv |= _ksePowerOn(
        power_on_rlt.abVer, &power_on_rlt.bLifeCycle, power_on_rlt.chipSerial,
        power_on_rlt.abSystemTitle, &power_on_rlt.bVcType,
        &power_on_rlt.bMaxVcRetryCount, &power_on_rlt.usMaxKcmvpKeyCount,
        &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
        &power_on_rlt.usInfoFileSize);

    OSTimeDly(OS_MS2TICK(1));
    dsm_sec_por_print(&power_on_rlt);
    dsm_wdt_ext_toggle_immd();
    int16_t sRv = 0;

    sRv |= dsm_sec_cert_put(SEC_SERVER, CERT_ROOTCA, RootCa_cert, 0);
    sRv |= dsm_sec_cert_put(SEC_SERVER, CERT_SUBCA, SubCa_cert, 0);
    sRv |= dsm_sec_cert_put(SEC_SERVER, CERT_DS, CERT_DS_BUFF, 0);
    sRv |= dsm_sec_prikey_put_server();

    if (power_on_rlt.bLifeCycle == LC_ISSUED)
        DPRINTF(DBG_WARN, "%s: issued ok\r\n");

    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: Fail!!!\r\n", __func__);
    dsm_sec_recovery();

    return FALSE;
#else
    return TRUE;
#endif /* bccho */
}

uint8_t dsm_sec_get_security_suite(void) { return g_security_suite; }

void dsm_sec_set_security_suite(uint8_t suite)
{
    DPRINTF(DBG_TRACE, _D "s: suite[0x%02X]\r\n", __func__, suite);
    g_security_suite = suite;
}

void dsm_sec_invocation_count_reset(void) { g_sec_invocation_count = 0; }

void dsm_sec_invocation_count_add(void) { g_sec_invocation_count += 1; }

void dsm_sec_invocation_count_subtract(void)
{
    if (g_sec_invocation_count > 0)
        g_sec_invocation_count -= 1;
}

uint32_t dsm_sec_invocation_count_get(void) { return g_sec_invocation_count; }

void dsm_sec_sc_field_set(T_DLMS_SC *p_sc)
{
    memcpy(&g_req_sc_field, p_sc, sizeof(T_DLMS_SC));
}

T_DLMS_SC *dsm_sec_sc_field_get(void) { return &g_req_sc_field; }

ST_SEC_M_POWER_ON_RLT *dsm_sec_m_get_por_result_info(void)
{
    return &power_on_rlt;
}

int16_t dsm_sec_m_por_result(ST_SEC_M_POWER_ON_RLT *p_por_rlt)
{
    int16_t result = 0;

#if 0  /* bccho, KEYPAIR, 2023-07-15 */
    result = _ksePowerOn(
        p_por_rlt->abVer, &p_por_rlt->bLifeCycle, p_por_rlt->chipSerial,
        p_por_rlt->abSystemTitle, &p_por_rlt->bVcType,
        &p_por_rlt->bMaxVcRetryCount, &p_por_rlt->usMaxKcmvpKeyCount,
        &p_por_rlt->usMaxCertKeyCount, &p_por_rlt->usMaxIoDataSize,
        &p_por_rlt->usInfoFileSize);
    dsm_sec_por_print(p_por_rlt);
#endif /* bccho */

    return result;
}

void dsm_sec_por_print(ST_SEC_M_POWER_ON_RLT *p_por_rlt)
{
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
    DPRINTF(DBG_WARN, "abVer0[%d] abVer1[%d] abVer2[%d]\r\n",
            p_por_rlt->abVer[0], p_por_rlt->abVer[1], p_por_rlt->abVer[2]);
    DPRINTF(DBG_NONE, "bLifeCycle[%02X]\r\n", p_por_rlt->bLifeCycle);
    DPRINT_HEX(DBG_WARN, "ChipSerial", p_por_rlt->chipSerial, 8, DUMP_ALWAYS);
    DPRINT_HEX(DBG_WARN, "SysTitle", p_por_rlt->abSystemTitle, 8, DUMP_ALWAYS);
#endif
}

uint8_t *dsm_sec_get_cert_buff(void) { return &CERT_DS_BUFF[0]; }

uint8_t dsm_sec_cert_idx_2_certcon_idx(uint8_t cert_idx)
{
    uint8_t idx = 0xff;

    switch (cert_idx)
    {
    case CERT_ROOTCA:
        idx = CERT_CON_IDX_ROOTCA_S;
        break;
    case CERT_SUBCA:
        idx = CERT_CON_IDX_SUBCA_S;
        break;
    case CERT_DS:
        idx = CERT_CON_IDX_DS_S;
        break;
    case CERT_DS_CLIENT:
        idx = CERT_CON_IDX_DS_C;
        break;
    case CERT_KA:
    case CERT_KA_CLIENT:
        break;
    case CERT_KTC_ROOTCA:
        idx = CERT_CON_IDX_KTC_CA;
        break;
    case CERT_KTC_MT_CERT_DS:
        idx = CERT_CON_IDX_KTC_DS;
        break;
    default:

        break;
    }

    return idx;
}

uint8_t dsm_sec_certcon_idx_2_entity_idx(uint8_t certcon_idx)
{
    uint8_t idx = 0xff;

    switch (certcon_idx)
    {
    case CERT_CON_IDX_ROOTCA_S:
        idx = CERT_ENTITY_AUTHORITY;
        break;
    case CERT_CON_IDX_SUBCA_S:
        idx = CERT_ENTITY_AUTHORITY;
        break;
    case CERT_CON_IDX_DS_S:
        idx = CERT_ENTITY_SERVER;
        break;
    case CERT_CON_IDX_DS_C:
        idx = CERT_ENTITY_CLIENT;
        break;
    default:

        break;
    }
    return idx;
}

uint8_t dsm_sec_certcon_idx_2_cert_idx(uint8_t certcon_idx)
{
    uint8_t idx = 0xff;

    switch (certcon_idx)
    {
    case CERT_CON_IDX_ROOTCA_S:
        idx = CERT_ROOTCA;
        break;
    case CERT_CON_IDX_SUBCA_S:
        idx = CERT_SUBCA;
        break;
    case CERT_CON_IDX_DS_S:
        idx = CERT_DS;
        break;
    case CERT_CON_IDX_DS_C:
        idx = CERT_DS_CLIENT;
        break;
    case CERT_CON_IDX_KTC_CA:
        idx = CERT_KTC_ROOTCA;
        break;
    case CERT_CON_IDX_KTC_DS:
        idx = CERT_KTC_MT_CERT_DS;
        break;

    default:

        break;
    }
    return idx;
}

uint8_t dsm_sec_set_cert_info(uint8_t cert_field_t, uint8_t cert_idx,
                              uint8_t *pdata, uint8_t len)
{
#if 0  /* bccho, KEYPAIR, 2023-07-15 */        
    uint8_t cert_con_idx = 0;

    cert_con_idx = dsm_sec_cert_idx_2_certcon_idx(cert_idx);

    if (cert_con_idx == 0xff)
    {
        DPRINTF(DBG_ERR, _D "%s: cert_con_idx error !!!\r\n", __func__);
        return 0xFF;
    }

    switch (cert_field_t)
    {
    case CERT_SN:
        gst_cert_con.cert_info[cert_con_idx].sn_size = len;
        memcpy(gst_cert_con.cert_info[cert_con_idx].sn, pdata, len);

        break;
    case CERT_ISSUER_DN:
        gst_cert_con.cert_info[cert_con_idx].issuer_size = len;
        memcpy(gst_cert_con.cert_info[cert_con_idx].issuer, pdata, len);

        break;
    case CERT_SUBJ_DN:
        gst_cert_con.cert_info[cert_con_idx].subj_size = len;
        memcpy(gst_cert_con.cert_info[cert_con_idx].subj, pdata, len);

        break;
    default:

        break;
    }
#endif /* bccho */

    return 0;
}

uint8_t dsm_sec_get_cert_info(uint8_t cert_field_t, uint8_t cert_con_idx,
                              uint8_t *pdata, uint8_t *len)
{
#if 0  /* bccho, KEYPAIR, 2023-07-15 */        
    if (cert_con_idx == 0xff)
    {
        DPRINTF(DBG_ERR, _D "%s: cert_con_idx error !!!\r\n", __func__);
        return 0xFF;
    }

    switch (cert_field_t)
    {
    case CERT_SN:
        *len = gst_cert_con.cert_info[cert_con_idx].sn_size;
        memcpy(pdata, gst_cert_con.cert_info[cert_con_idx].sn, *len);
        break;

    case CERT_ISSUER_DN:
        *len = gst_cert_con.cert_info[cert_con_idx].issuer_size;
        memcpy(pdata, gst_cert_con.cert_info[cert_con_idx].issuer, *len);
        break;

    case CERT_SUBJ_DN:
        *len = gst_cert_con.cert_info[cert_con_idx].subj_size;
        memcpy(pdata, gst_cert_con.cert_info[cert_con_idx].subj, *len);
        break;
    default:
        break;
    }
#endif /* bccho */

    return 0;
}

int16_t dsm_sec_cert_get(uint32_t party, uint32_t cert_idx,
                         uint32_t sys_t_backup_flag)
{
    int16_t sRv = 0;
#if 0 /* bccho, KEYPAIR, 2023-07-15 */    
    uint16_t usSize;

    dsm_sec_set_operation(true);

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_TRACE, _D "%s: %s, cert_idx[%d], sys_t_backup_flag[%d]\r\n",
            __func__, dsm_cert_string(party, cert_idx), cert_idx,
            sys_t_backup_flag);

    OSTimeDly(OS_MS2TICK(1));
    sRv = _certGetCert(abBuffer, cert_idx);
    if (cert_idx == CERT_DS)
    {
        DPRINT_HEX(DBG_INFO, dsm_cert_string(party, cert_idx), abBuffer, 500,
                   DUMP_ALWAYS);
    }
    else
        DPRINT_HEX(DBG_INFO, dsm_cert_string(party, cert_idx), abBuffer, 500,
                   DUMP_SEC);

    OSTimeDly(OS_MS2TICK(1));
    usSize = SEC_MM_BUFFER_SIZE;
    sRv = _certGetCertField(abBuffer, &usSize, CERT_SN, cert_idx);
    if (cert_idx == CERT_DS)
        DPRINT_HEX(DBG_INFO, "CERT_SN", abBuffer, usSize, DUMP_SEC);
    else
        DPRINT_HEX(DBG_INFO, "CERT_SN", abBuffer, usSize, DUMP_SEC);
    dsm_sec_set_cert_info(CERT_SN, cert_idx, abBuffer, usSize);

    OSTimeDly(OS_MS2TICK(1));
    usSize = SEC_MM_BUFFER_SIZE;
    sRv = _certGetCertField(abBuffer, &usSize, CERT_ISSUER_DN, cert_idx);
    if (cert_idx == CERT_DS)
        DPRINT_HEX(DBG_INFO, "CERT_ISSUER_DN", abBuffer, usSize, DUMP_SEC);
    else
        DPRINT_HEX(DBG_INFO, "CERT_ISSUER_DN", abBuffer, usSize, DUMP_SEC);
    dsm_sec_set_cert_info(CERT_ISSUER_DN, cert_idx, abBuffer, usSize);
    OSTimeDly(OS_MS2TICK(1));
    usSize = SEC_MM_BUFFER_SIZE;
    sRv = _certGetCertField(abBuffer, &usSize, CERT_ISSUER_CN, cert_idx);
    if (cert_idx == CERT_DS)
        DPRINT_HEX(DBG_INFO, "CERT_ISSUER_CN", abBuffer, usSize, DUMP_SEC);
    else
        DPRINT_HEX(DBG_INFO, "CERT_ISSUER_CN", abBuffer, usSize, DUMP_SEC);
    OSTimeDly(OS_MS2TICK(1));
    usSize = SEC_MM_BUFFER_SIZE;
    sRv = _certGetCertField(abBuffer, &usSize, CERT_VALIDITY, cert_idx);
    if (cert_idx == CERT_DS)
        DPRINT_HEX(DBG_INFO, "CERT_VALIDITY", abBuffer, usSize, DUMP_SEC);
    else
        DPRINT_HEX(DBG_INFO, "CERT_VALIDITY", abBuffer, usSize, DUMP_SEC);
    OSTimeDly(OS_MS2TICK(1));
    usSize = SEC_MM_BUFFER_SIZE;
    sRv = _certGetCertField(abBuffer, &usSize, CERT_SUBJ_DN, cert_idx);
    if (cert_idx == CERT_DS)
        DPRINT_HEX(DBG_INFO, "CERT_SUBJ_DN", abBuffer, usSize, DUMP_SEC);
    else
        DPRINT_HEX(DBG_INFO, "CERT_SUBJ_DN", abBuffer, usSize, DUMP_SEC);
    dsm_sec_set_cert_info(CERT_SUBJ_DN, cert_idx, abBuffer, usSize);

    OSTimeDly(OS_MS2TICK(1));
    usSize = SEC_MM_BUFFER_SIZE;
    sRv = _certGetCertField(abBuffer, &usSize, CERT_SUBJ_CN, cert_idx);
    if (cert_idx == CERT_DS)
        DPRINT_HEX(DBG_INFO, "CERT_SUBJ_CN", abBuffer, usSize, DUMP_SEC);
    else
        DPRINT_HEX(DBG_INFO, "CERT_SUBJ_CN", abBuffer, usSize, DUMP_SEC);

    if (sys_t_backup_flag)
    {
        if (cert_idx == CERT_DS)
        {
            memcpy(SYS_TITLE_server, abBuffer, 3);
            str_to_hex_n((char *)&abBuffer[3], &SYS_TITLE_server[3], 10);
            DPRINT_HEX(DBG_WARN, "SYS_TITLE_server", SYS_TITLE_server,
                       SYS_TITLE_LEN, DUMP_ALWAYS);
        }
        else if (cert_idx == CERT_DS_CLIENT)
        {
            memcpy(SYS_TITLE_client, abBuffer, 3);
            str_to_hex_n((char *)&abBuffer[3], &SYS_TITLE_client[3], 10);
            DPRINT_HEX(DBG_WARN, "SYS_TITLE_client", SYS_TITLE_client,
                       SYS_TITLE_LEN, DUMP_ALWAYS);
        }
    }

    OSTimeDly(OS_MS2TICK(1));
    usSize = SEC_MM_BUFFER_SIZE;
    sRv = _certGetCertField(abBuffer, &usSize, CERT_SUBJ_PUBKEY, cert_idx);
    DPRINT_HEX(DBG_INFO, "CERT_SUBJ_PUBKEY", abBuffer, usSize, DUMP_SEC);

    if (cert_idx == CERT_DS || cert_idx == CERT_DS_CLIENT)
    {
        OSTimeDly(OS_MS2TICK(1));
        usSize = SEC_MM_BUFFER_SIZE;
        sRv = _certGetCertField(abBuffer, &usSize, CERT_AUTH_KEY_ID, cert_idx);
        // DPRINTF(DBG_TRACE, "%s: 9. sRv[0x%02X]\r\n", __func__, sRv);
        if (cert_idx == CERT_DS)
            DPRINT_HEX(DBG_INFO, "CERT_AUTH_KEY_ID", abBuffer, usSize,
                       DUMP_SEC);
        else
            DPRINT_HEX(DBG_INFO, "CERT_AUTH_KEY_ID", abBuffer, usSize,
                       DUMP_SEC);
    }

    if (party != SEC_KTC)
    {
        usSize = SEC_MM_BUFFER_SIZE;
        sRv = _certGetCertField(abBuffer, &usSize, CERT_KEY_USAGE, cert_idx);
        if (cert_idx == CERT_DS)
            DPRINT_HEX(DBG_INFO, "CERT_KEY_USAGE", abBuffer, usSize, DUMP_SEC);
        else
            DPRINT_HEX(DBG_INFO, "CERT_KEY_USAGE", abBuffer, usSize, DUMP_SEC);
    }
    dsm_wdt_ext_toggle_immd();

#endif /* bccho */
    return sRv;
}

int16_t dsm_sec_cert_put(uint32_t party, uint32_t cert_type, uint8_t *pcert,
                         uint16_t cert_size)
{
    int16_t sRv = 0, cert_idx = 0;
#if 0  /* bccho, KEYPAIR, 2023-07-15 */

    dsm_wdt_ext_toggle_immd();

    dsm_sec_set_operation(true);

    DPRINTF(DBG_TRACE, _D "%s: party[%d], cert_type[%d]\r\n", __func__, party,
            cert_type);
    if (party == SEC_SERVER)
    {
        if (cert_type == CERT_ROOTCA)
        {
            sRv |= _certEraseCertKey(SELECT_ALL, CERT_ROOTCA);
            OSTimeDly(OS_MS2TICK(1));
            if (pcert == NULL)
                sRv |=
                    _certPutCertPubKey((uint8_t *)RootCa_cert, CERT_ROOTCA, 0);
            else
                sRv |= _certPutCertPubKey((uint8_t *)pcert, CERT_ROOTCA, 0);
            cert_idx = CERT_ROOTCA;
        }
        else if (cert_type == CERT_SUBCA)
        {
            sRv |= _certEraseCertKey(SELECT_ALL, CERT_SUBCA);
            OSTimeDly(OS_MS2TICK(1));
            if (pcert == NULL)
                sRv |= _certPutCertPubKey((uint8_t *)SubCa_cert, CERT_SUBCA, 0);
            else
                sRv |= _certPutCertPubKey((uint8_t *)pcert, CERT_SUBCA, 0);
            cert_idx = CERT_SUBCA;
        }
        else if (cert_type == CERT_DS)
        {
            sRv |= _certEraseCertKey(SELECT_ALL, CERT_DS);
            OSTimeDly(OS_MS2TICK(1));
            if (pcert == NULL)
                sRv |= _certPutCertPubKey((uint8_t *)CERT_DS_BUFF, CERT_DS, 1);
            else
                sRv |= _certPutCertPubKey((uint8_t *)pcert, CERT_DS, 1);
            cert_idx = CERT_DS;
        }
        else if (cert_type == CERT_KA)
        {
        }
    }
    else if (party == SEC_KTC)
    {
        if (cert_type == CERT_KTC_ROOTCA)
        {
            sRv |= _certEraseCertKey(SELECT_ALL, CERT_KTC_ROOTCA);
            OSTimeDly(OS_MS2TICK(1));
            sRv |= _certPutCertPubKey((uint8_t *)pcert, CERT_KTC_ROOTCA,
                                      CERT_KTC_ROOTCA);
            cert_idx = CERT_KTC_ROOTCA;
        }
        else if (cert_type == CERT_KTC_MT_CERT_DS)
        {
            sRv |= _certEraseCertKey(SELECT_ALL, CERT_KTC_MT_CERT_DS);
            OSTimeDly(OS_MS2TICK(1));
            sRv |= _certPutCertPubKey((uint8_t *)pcert, CERT_KTC_MT_CERT_DS,
                                      CERT_KTC_ROOTCA);
            cert_idx = CERT_KTC_MT_CERT_DS;
        }
        else if (cert_type == CERT_KTC_MT_CERT_TMP_DS)
        {
            sRv |= _certEraseCertKey(SELECT_ALL, CERT_KTC_MT_CERT_TMP_DS);
            OSTimeDly(OS_MS2TICK(1));
            sRv |= _certPutCertPubKey((uint8_t *)pcert, CERT_KTC_MT_CERT_TMP_DS,
                                      CERT_KTC_ROOTCA);
            cert_idx = CERT_KTC_MT_CERT_TMP_DS;
        }
    }
    else
    {
        if (cert_type == CERT_DS_CLIENT)
        {
            sRv |= _certEraseCertKey(SELECT_ALL, CERT_DS_CLIENT);
            OSTimeDly(OS_MS2TICK(1));
            sRv |= _certPutCertPubKey((uint8_t *)pcert, CERT_DS_CLIENT, 1);
            cert_idx = CERT_DS_CLIENT;
        }
        else if (cert_type == CERT_KA_CLIENT)
        {
            sRv |= _certEraseCertKey(SELECT_ALL, CERT_KA_CLIENT);
            OSTimeDly(OS_MS2TICK(1));
            sRv |= _certPutCertPubKey((uint8_t *)pcert, CERT_KA_CLIENT, 1);
            cert_idx = CERT_KA_CLIENT;
        }
    }

    if (sRv == KSE_SUCCESS)
    {
        sRv |= dsm_sec_cert_get(party, cert_idx, TRUE);
    }
#endif /* bccho */

    return sRv;
}

int16_t dsm_sec_cert_put_server(void)
{
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
    int16_t sRv = 0;
    dsm_sec_por_print(&power_on_rlt);
    OSTimeDly(OS_MS2TICK(1));
    dsm_sec_set_operation(true);
    sRv |= dsm_sec_cert_put(SEC_SERVER, CERT_ROOTCA, NULL, 0);
    sRv |= dsm_sec_cert_put(SEC_SERVER, CERT_SUBCA, NULL, 0);
    sRv |= dsm_sec_cert_put(SEC_SERVER, CERT_DS, NULL, 0);

    sRv = dsm_sec_prikey_put_server();

    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: Fail!!!\r\n", __func__);
    dsm_sec_recovery();
    return FALSE;
#else
    return TRUE;
#endif
}

int16_t dsm_sec_prikey_put_server(void)
{
#if 1 /* bccho, KEYPAIR, 2023-07-15 */
    uint8_t DS_pulblickey[64];
    int pubkey_len;
    int rtn = hls_get_pubkey_from_cert(CERT_DS_BUFF, CERT_LEN, DS_pulblickey,
                                       &pubkey_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_get_pubkey_from_cert() error!!!, %d\r\n", rtn);
        return 0x01;
    }

    DPRINT_HEX(DBG_INFO, "Public Key", DS_pulblickey, pubkey_len, DUMP_ALWAYS);
    DPRINT_HEX(DBG_INFO, "Private Key", DS_privatekey, 32, DUMP_ALWAYS);

    rtn = hls_set_ecdsa_key(hls_ctx, DS_privatekey, 32, DS_pulblickey,
                            pubkey_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_set_ecdsa_key() error!!!, %d\r\n", rtn);
        return 0x01;
    }

    return 0x00;
#else
    int16_t sRv = 0;

    OSTimeDly(OS_MS2TICK(1));

    dsm_sec_set_operation(true);
    sRv = _certPutPriKey(CERT_KEY_EXT_DEV_ECDSA, (uint8_t *)DS_privatekey,
                         CERT_DS);
    return sRv;
#endif /* bccho */
}

int16_t dsm_sec_cert_put_client_ds(uint8_t *pcert)
{
    DPRINTF(DBG_TRACE, "%s: [%02X, %02X, %02X]\r\n", __func__, pcert[0],
            pcert[1], pcert[2]);

#if 1 /* bccho, HLS, 2023-08-14 */
    uint8_t pubkey[64];
    int pubkey_len = HLS_PUBKEY_LEN;
    int rtn = hls_get_pubkey_from_cert(pcert, 500, pubkey, &pubkey_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_get_pubkey_from_cert() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    DPRINT_HEX(DBG_CIPHER, "<-- Rx Client Certificate", pcert, 500, DUMP_SEC);

    rtn = hls_set_other_ecdsa_pub_key(hls_ctx, pubkey, pubkey_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_set_other_ecdsa_pub_key() error!!!, %d\r\n", rtn);
        return FALSE;
    }
#else
    dsm_wdt_ext_toggle_immd();

    OSTimeDly(OS_MS2TICK(1));

    dsm_sec_set_operation(true);

    int16_t sRv = 0;

    sRv = _ksePowerOn(
        power_on_rlt.abVer, &power_on_rlt.bLifeCycle, power_on_rlt.chipSerial,
        power_on_rlt.abSystemTitle, &power_on_rlt.bVcType,
        &power_on_rlt.bMaxVcRetryCount, &power_on_rlt.usMaxKcmvpKeyCount,
        &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
        &power_on_rlt.usInfoFileSize);

    dsm_sec_por_print(&power_on_rlt);

    sRv = _certEraseCertKey(SELECT_ALL, CERT_DS_CLIENT);

    OSTimeDly(OS_MS2TICK(1));

    sRv |= _certPutCertPubKey(pcert, CERT_DS_CLIENT, 1);
    if (sRv != 0)
    {
        DPRINTF(DBG_ERR, "%s: CERT_DS write Fail!!!\r\n", __func__);
        return FALSE;
    }
    OSTimeDly(OS_MS2TICK(1));
#endif /* bccho */

    if (dsm_sec_fc2s_generate(SYS_TITLE_client))
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: f_c2s gen Fail!!!\r\n", __func__);
    return FALSE;
}

int16_t dsm_sec_cert_put_ktc_meter_ca(void)
{
#if 0  /* bccho, KEYPAIR, 2023-07-15 */
    int16_t sRv = 0;
    uint8_t *pcert = KTC_RootCa_cert;

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_TRACE, "%s: [%02X, %02X, %02X]\r\n", __func__, pcert[0],
            pcert[1], pcert[2]);

    OSTimeDly(OS_MS2TICK(1));

    dsm_sec_set_operation(true);

    sRv = _ksePowerOn(
        power_on_rlt.abVer, &power_on_rlt.bLifeCycle, power_on_rlt.chipSerial,
        power_on_rlt.abSystemTitle, &power_on_rlt.bVcType,
        &power_on_rlt.bMaxVcRetryCount, &power_on_rlt.usMaxKcmvpKeyCount,
        &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
        &power_on_rlt.usInfoFileSize);

    dsm_sec_por_print(&power_on_rlt);

    sRv = _certEraseCertKey(SELECT_ALL, CERT_KTC_ROOTCA);

    OSTimeDly(OS_MS2TICK(1));

    sRv |= _certPutCertPubKey(pcert, CERT_KTC_ROOTCA, CERT_KTC_ROOTCA);
    if (sRv != 0)
    {
        DPRINTF(DBG_ERR, "%s: MT_CERT_CA write Fail!!!\r\n", __func__);
        return FALSE;
    }

    sRv = dsm_sec_cert_get(SEC_KTC, CERT_KTC_ROOTCA, FALSE);

    if (sRv != 0)
    {
        DPRINTF(DBG_ERR, "%s: MT_CERT_CA read Fail!!!\r\n", __func__);
        return FALSE;
    }
#endif /* bccho */

    return TRUE;
}

int16_t dsm_sec_cert_erase_ktc_meter_ca(void)
{
#if 0  /* bccho, KEYPAIR, 2023-07-15 */
    int16_t sRv = 0;

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

    OSTimeDly(OS_MS2TICK(1));

    dsm_sec_set_operation(true);

    sRv = _ksePowerOn(
        power_on_rlt.abVer, &power_on_rlt.bLifeCycle, power_on_rlt.chipSerial,
        power_on_rlt.abSystemTitle, &power_on_rlt.bVcType,
        &power_on_rlt.bMaxVcRetryCount, &power_on_rlt.usMaxKcmvpKeyCount,
        &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
        &power_on_rlt.usInfoFileSize);

    dsm_sec_por_print(&power_on_rlt);

    sRv = _certEraseCertKey(SELECT_ALL, CERT_KTC_ROOTCA);

    if (sRv != 0)
    {
        DPRINTF(DBG_ERR, "%s: MT_CERT_CA erase Fail!!!\r\n", __func__);
        return FALSE;
    }
#endif /* bccho */

    return TRUE;
}

int16_t dsm_sec_cert_put_ktc_meter_ds(uint8_t *pcert, uint8_t *p_fwname,
                                      uint8_t *phash, date_time_type *pst,
                                      date_time_type *psp)
{
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
    int16_t sRv = 0;
    uint16_t usSize;

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_TRACE, "%s: [%02X, %02X, %02X]\r\n", __func__, pcert[0],
            pcert[1], pcert[2]);

    OSTimeDly(OS_MS2TICK(1));

    dsm_sec_set_operation(true);

    sRv = _ksePowerOn(
        power_on_rlt.abVer, &power_on_rlt.bLifeCycle, power_on_rlt.chipSerial,
        power_on_rlt.abSystemTitle, &power_on_rlt.bVcType,
        &power_on_rlt.bMaxVcRetryCount, &power_on_rlt.usMaxKcmvpKeyCount,
        &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
        &power_on_rlt.usInfoFileSize);

    dsm_sec_por_print(&power_on_rlt);

    sRv = _certEraseCertKey(SELECT_ALL, CERT_KTC_MT_CERT_DS);

    OSTimeDly(OS_MS2TICK(1));

#if 1
    sRv |= _certPutCertPubKey(pcert, CERT_KTC_MT_CERT_DS, CERT_KTC_ROOTCA);
#else
    sRv |= _certPutCert(pcert, CERT_KTC_MT_CERT_DS);
#endif
    if (sRv != 0)
    {
        DPRINTF(DBG_ERR, "%s: MT_CERT_DS write Fail!!!\r\n", __func__);
        return FALSE;
    }

    usSize = SEC_MM_BUFFER_SIZE;
    sRv =
        _certGetCertField(abBuffer, &usSize, CERT_SUBJ_CN, CERT_KTC_MT_CERT_DS);
    memcpy(&p_fwname[0], abBuffer, IMAGE_FW_NAME_MAX_SIZE);
    DPRINT_HEX(DBG_INFO, "FW_NAME", p_fwname, IMAGE_FW_NAME_MAX_SIZE,
               DUMP_ALWAYS);

    usSize = SEC_MM_BUFFER_SIZE;
    sRv |=
        _certGetCertField(abBuffer, &usSize, CERT_FW_HASH, CERT_KTC_MT_CERT_DS);
    memcpy(&phash[0], abBuffer, IMAGE_HASH_SIZE);
    DPRINT_HEX(DBG_INFO, "CERT_FW_HASH", abBuffer, usSize, DUMP_ALWAYS);

    usSize = SEC_MM_BUFFER_SIZE;
    sRv = _certGetCertField(abBuffer, &usSize, CERT_VALIDITY,
                            CERT_KTC_MT_CERT_DS);
    DPRINT_HEX(DBG_INFO, "CERT_VALIDITY", abBuffer, usSize, DUMP_ALWAYS);

    pst->year = (abBuffer[0] - '0') * 10 + (abBuffer[1] - '0');
    pst->month = (abBuffer[2] - '0') * 10 + (abBuffer[3] - '0');
    pst->date = (abBuffer[4] - '0') * 10 + (abBuffer[5] - '0');
    pst->hour = (abBuffer[6] - '0') * 10 + (abBuffer[7] - '0');
    pst->min = (abBuffer[8] - '0') * 10 + (abBuffer[9] - '0');
    pst->sec = (abBuffer[10] - '0') * 10 + (abBuffer[11] - '0');

    psp->year = (abBuffer[13] - '0') * 10 + (abBuffer[14] - '0');
    psp->month = (abBuffer[15] - '0') * 10 + (abBuffer[16] - '0');
    psp->date = (abBuffer[17] - '0') * 10 + (abBuffer[18] - '0');
    psp->hour = (abBuffer[19] - '0') * 10 + (abBuffer[20] - '0');
    psp->min = (abBuffer[21] - '0') * 10 + (abBuffer[22] - '0');
    psp->sec = (abBuffer[23] - '0') * 10 + (abBuffer[24] - '0');

    if (sRv != 0)
    {
        DPRINTF(DBG_ERR, "%s: MT_CERT_DS read Fail!!!\r\n", __func__);
        dsm_sec_recovery();

        return FALSE;
    }
#endif /* bccho */

    return TRUE;
}

int16_t dsm_sec_random_generate(void)
{
#if 1 /* bccho, HLS, 2023-08-14 */
    int rtn = hls_gen_rand(hls_ctx);
    if (rtn != HLS_SUCCESS)
    {
        hls_deinit(hls_ctx);
        hls_ctx = hls_init(HLS_OPMODE_SERVER);
        rtn = hls_gen_rand(hls_ctx);
        if (rtn != HLS_SUCCESS)
        {
            DPRINTF(DBG_ERR, "hls_gen_rand() error!!!, %d\r\n", rtn);
            return FALSE;
        }
    }

    int rand_len = XTOX_LEN;
    rtn = hls_get_rand(hls_ctx, XTOX, &rand_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_get_rand() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    DPRINT_HEX(DBG_TRACE, "XTOX", XTOX, rand_len, DUMP_ALWAYS);

    return TRUE;
#else
    dsm_wdt_ext_toggle_immd();

    dsm_sec_set_operation(true);

    int16_t sRv = 0;

    sRv = _amiDrbg(XTOX, XTOX_LEN);
    DPRINT_HEX(DBG_TRACE, "XTOX", XTOX, XTOX_LEN, DUMP_ALWAYS);
    if (sRv == 0)
    {
        return TRUE;
    }
    DPRINTF(DBG_ERR, "%s: Fail!!!\r\n", __func__);
    dsm_sec_recovery();
    return FALSE;
#endif /* bccho */
}

int16_t dsm_sec_fs2c_verify(uint8_t *p_fs2c, uint8_t *client_sys_t)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);
    DPRINT_HEX(DBG_CIPHER, "<-- Rx Client Auth", p_fs2c, fXTOX_LEN, DUMP_SEC);

#if 1 /* bccho, HLS, 2023-08-15 */
    int rtn = hls_verify_mutual_auth(hls_ctx, p_fs2c, fXTOX_LEN);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_verify_mutual_auth() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    return true;
#else
    dsm_wdt_ext_toggle_immd();

    dsm_sec_set_operation(true);

    int16_t sRv = 0;
    sRv |= _ksePowerOn(
        power_on_rlt.abVer, &power_on_rlt.bLifeCycle, power_on_rlt.chipSerial,
        power_on_rlt.abSystemTitle, &power_on_rlt.bVcType,
        &power_on_rlt.bMaxVcRetryCount, &power_on_rlt.usMaxKcmvpKeyCount,
        &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
        &power_on_rlt.usInfoFileSize);

    dsm_sec_por_print(&power_on_rlt);

    sRv = _amiMutualAuthVeri(client_sys_t, XTOX_c, p_fs2c, CERT_DS_CLIENT);
    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: Fail!!!\r\n", __func__);
    dsm_sec_recovery();

    log_cert_ng(SEC_LOG_CROSS_CERT_FAIL);

    return FALSE;
#endif /* bccho */
}

int16_t dsm_sec_fc2s_generate(uint8_t *client_sys_t)
{
    DPRINTF(DBG_TRACE, "%s\r\n", __func__);

#if 1 /* bccho, HLS, 2023-08-15 */
    int rtn = hls_set_other_rand(hls_ctx, XTOX_c, XTOX_LEN);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_set_other_rand() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    DPRINT_HEX(DBG_CIPHER, "<-- Rx Client RAND", XTOX_c, XTOX_LEN, DUMP_SEC);

    int signature_len = DLMS_DS_LEN;
    rtn = hls_sign_mutual_auth(hls_ctx, fXTOX, &signature_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_sign_mutual_auth() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    return TRUE;
#else
    dsm_wdt_ext_toggle_immd();

    dsm_sec_set_operation(true);

    int16_t sRv = 0;

    sRv |= _ksePowerOn(
        power_on_rlt.abVer, &power_on_rlt.bLifeCycle, power_on_rlt.chipSerial,
        power_on_rlt.abSystemTitle, &power_on_rlt.bVcType,
        &power_on_rlt.bMaxVcRetryCount, &power_on_rlt.usMaxKcmvpKeyCount,
        &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
        &power_on_rlt.usInfoFileSize);

    dsm_sec_por_print(&power_on_rlt);

    sRv = _amiMutualAuthGen(fXTOX, client_sys_t, XTOX_c, CERT_DS);
    DPRINT_HEX(DBG_INFO, "fC2S", fXTOX, fXTOX_LEN, DUMP_ALWAYS);

    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: Fail!!!\r\n", __func__);
    dsm_sec_recovery();
    log_cert_ng(SEC_LOG_CROSS_CERT_FAIL);

    return FALSE;
#endif /* bccho */
}

int16_t dsm_sec_key_agreement_process(uint8_t *pdata, uint16_t len)
{
    key_info_type *pkey_info = (key_info_type *)&pdata[0];

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_TRACE, "%s: key_info_cnt[%d]\r\n", __func__, pkey_info->cnt);

    if (pkey_info->cnt)
    {
        DPRINT_HEX(DBG_TRACE, "client_key_agreedata",
                   &pkey_info->key_agree[0].id, 129, DUMP_SEC);

        dsm_sec_set_operation(true);

#if 1 /* bccho, HLS, 2023-08-15 */
        int rtn = hls_set_ecdh_other_pubkey(
            hls_ctx, &pkey_info->key_agree[0].data[0], 64);
        if (rtn != HLS_SUCCESS)
        {
            DPRINTF(DBG_ERR, "hls_set_ecdh_other_pubkey() error!!!, %d\r\n",
                    rtn);
        }

        rtn = hls_verify_key_agreement(hls_ctx,
                                       &pkey_info->key_agree[0].data[64], 64);
        if (rtn != HLS_SUCCESS)
        {
            DPRINTF(DBG_ERR, "hls_verify_key_agreement() error!!!, %d\r\n",
                    rtn);
        }

        DPRINT_HEX(DBG_CIPHER, "<-- Rx Client Ephemeral Public Key",
                   &pkey_info->key_agree[0].data[0], 64, DUMP_SEC);
        DPRINT_HEX(DBG_CIPHER, "<-- Rx Client Ephemeral Public Key Signature",
                   &pkey_info->key_agree[0].data[64], 64, DUMP_SEC);

#if 1 /* bccho, HLS, 2023-08-16, hls_gen_ecdh_key() 버그 */
        rtn = axiocrypto_ecdh_genkey(hls_ctx->hls_info->ecdh_key,
                                     ASYM_ECDH_P256, CTX_ATTR_NONE);
        if (rtn != CRYPTO_SUCCESS)
        {
            DPRINTF(DBG_ERR, "axiocrypto_ecdh_genkey() error!!!, %d\r\n", rtn);
        }
#else
        const char t_s_ecdhe_prikey[] =
            "34A8C23A34DBB519D09B245754C85A6CFE05D14A063EFA5AA41545AA8241EF"
            "AE";  // 키교환용 ECDH [개인키]
        const char t_s_ecdhe_pubkey[] =
            "95F41066009B185B074F5FFFF736B71C325FCADB2BC0CF1A4F4B17BBE7AB81"
            "D62946506BC8169C7B539B39A5D8463787F449C9BD2583FA67A1075B0DBFC6"
            "38BA";  // 키교환용 [ECDH 공개키]
        uint8_t s_ecdh_prikey[32];
        uint8_t s_ecdh_pubkey[64];
        hex2bin(t_s_ecdhe_prikey, strlen(t_s_ecdhe_prikey), s_ecdh_prikey);
        hex2bin(t_s_ecdhe_pubkey, strlen(t_s_ecdhe_pubkey), s_ecdh_pubkey);

        rtn = axiocrypto_asym_putkey(hls_ctx->hls_info->ecdh_key,
                                     ASYM_ECDH_P256, s_ecdh_prikey, 32, 0,
                                     s_ecdh_pubkey, 64, 0, CTX_ATTR_NONE);
        if (rtn != CRYPTO_SUCCESS)
        {
            DPRINTF(DBG_ERR, "axiocrypto_asym_putkey() error!!!, %d\r\n", rtn);
        }

        rtn = hls_gen_ecdh_key(hls_ctx);
        if (rtn != HLS_SUCCESS)
        {
            DPRINTF(DBG_ERR, "hls_gen_ecdh_key() error!!!, %d\r\n", rtn);
        }
#endif
        server_key_agreedata[0] = 0; /* key id */
        int pubkey_len = HLS_PUBKEY_LEN;
        rtn =
            hls_get_ecdh_pubkey(hls_ctx, &server_key_agreedata[1], &pubkey_len);
        if (rtn != HLS_SUCCESS)
        {
            DPRINTF(DBG_ERR, "hls_get_ecdh_pubkey() error!!!, %d\r\n", rtn);
        }

        int signature_len;
        rtn = hls_sign_key_agreement(hls_ctx, &server_key_agreedata[65],
                                     &signature_len);
        if (rtn != HLS_SUCCESS)
        {
            DPRINTF(DBG_ERR, "hls_sign_key_agreement() error!!!, %d\r\n", rtn);
        }

        rtn = hls_gen_share_key(hls_ctx);
        if (rtn != HLS_SUCCESS)
        {
            DPRINTF(DBG_ERR, "hls_gen_share_key() error!!!, %d\r\n", rtn);
        }

        return TRUE;
#else  /* bccho */
        int16_t sRv = 0;

        sRv |= _amiEraseKey(CERT_DS_CLIENT);
        sRv |= _amiGenerateEkAkServer(
            server_key_agreedata, &pkey_info->key_agree[0].id, SYS_TITLE_client,
            SYS_TITLE_server, AMI_KDF_KEPCO_E, CERT_DS, CERT_DS_CLIENT);

        if (sRv == 0)
        {
            return TRUE;
        }

        DPRINTF(DBG_ERR, "%s: Fail: sRv[0x%04X]!!!\r\n", __func__, sRv);
        dsm_sec_recovery();

        log_cert_ng(SEC_LOG_Z_GEN_MIS);

        return FALSE;
#endif /* bccho */
    }

#if 1 /* bccho, HLS, 2023-08-15, remove warning */
    return FALSE;
#endif
}

int16_t dsm_sec_ciphering_xDLMS_APDU_build(uint8_t *p_xdlms_apdu,
                                           uint16_t *xdlms_len,
                                           uint8_t *p_plain, uint16_t plain_len)
{
    uint32_t sc_type = 0;
    uint16_t frame_len = 0;
    uint32_t xdlms_index = 0;
    uint8_t *ptr = p_xdlms_apdu;
    T_DLMS_SC sc_field;
    uint8_t sc, ic8[4];
    int16_t sRv = 0;
    uint8_t AT[12];
    uint8_t plain_apdu_type = p_plain[0];

    dsm_wdt_ext_toggle_immd();

    DPRINT_HEX(DBG_CIPHER, "--> Tx PLAIN", p_plain, plain_len, DUMP_SEC);

    sc_type = DLMS_SC_ENC | DLMS_SC_AT;

    if (sc_type & DLMS_SC_AT)
    {
        frame_len = DLMS_SEC_HDR_LEN + plain_len + DLMS_AUTH_TAG_LEN;
    }
    else if (sc_type & DLMS_SC_ENC)
    {
        frame_len = DLMS_SEC_HDR_LEN + plain_len;
    }

    *ptr++ = plain_apdu_type + 0x08;
    if (frame_len >= 256)
    {
        *ptr++ = 0x82;
        *ptr++ = (uint8_t)(frame_len >> 8);
        *ptr++ = (uint8_t)(frame_len);
    }
    else if (frame_len >= 128)
    {
        *ptr++ = 0x81;
        *ptr++ = (uint8_t)(frame_len);
    }
    else
    {
        *ptr++ = (uint8_t)(frame_len);
    }

    memset(&sc_field, 0x00, sizeof(T_DLMS_SC));

    if (sc_type & DLMS_SC_ENC)
        sc_field.enc = 1;
    if (sc_type & DLMS_SC_AT)
        sc_field.auth = 1;

    sc_field.suite_id = 0xf;

    memcpy(&sc, &sc_field, sizeof(T_DLMS_SC));

    *ptr++ = sc;
    WRITE_WORD(ptr, g_sec_invocation_count);
    ptr += 4;

    xdlms_index = ptr - p_xdlms_apdu;

    DPRINTF(
        DBG_NONE,
        _D "%s: plain_apdu[0x%02X], xdlms_len_field[%d], xdlms_hdr_len[%d]\r\n",
        __func__, plain_apdu_type, frame_len, xdlms_index);
    DPRINTF(DBG_NONE, _D "%s: SC[0x%02X], IC[0x%08X]\r\n", __func__, sc,
            g_sec_invocation_count);

    WRITE_WORD(ic8, g_sec_invocation_count);

    dsm_sec_set_operation(true);

#if 1 /* bccho, HLS, 2023-08-15 */
    int out_len = hls_get_encrypt_len(sc, plain_len);
    int rtn =
        hls_encrypt_message(hls_ctx, ic8, p_plain, plain_len, ptr, &out_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_encrypt_message() error!!!, %d\r\n", rtn);
        log_cert_ng(SEC_LOG_ENC_MIS);
        return FALSE;
    }

    xdlms_index += out_len;
    *xdlms_len = xdlms_index;

    DPRINT_HEX(DBG_CIPHER, "--> Tx Cipher", p_xdlms_apdu, *xdlms_len, DUMP_SEC);

    return TRUE;
#else
    sRv = _amiEdGenerate(ptr, AT, SYS_TITLE_server, ic8, AMI_SC_ARIA, p_plain,
                         plain_len, CERT_DS_CLIENT);
    memcpy(ptr + plain_len, AT, DLMS_AUTH_TAG_LEN);
    xdlms_index += plain_len;
    xdlms_index += DLMS_AUTH_TAG_LEN;

    *xdlms_len = xdlms_index;

    DPRINT_HEX(DBG_INFO, "TX_C_APDU", p_xdlms_apdu, *xdlms_len, DUMP_SEC);

    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: enc Fail!!!\r\n", __func__);
    dsm_sec_recovery();

    log_cert_ng(SEC_LOG_ENC_MIS);

    return FALSE;
#endif /* bccho */
}

int16_t dsm_sec_ciphering_xDLMS_APDU_parser(uint8_t *p_i_apdu,
                                            uint16_t xdlms_len,
                                            uint8_t *p_decipher,
                                            uint16_t *decipher_len)
{
    uint32_t ic;
    uint8_t ic8[4];
    T_DLMS_SC *p_sc_field;
    uint8_t sc = 0;
    uint16_t idx = 0, len_offset = 0, length = 0;
    uint16_t text_idx = 0, at_idx = 0, text_len = 0;
    int16_t sRv = 0;

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_NONE, _D "%s: xDLMS_APDU: %02X: %02X: %02X: %02X: %02X: \r\n",
            __func__, p_i_apdu[idx], p_i_apdu[idx + 1], p_i_apdu[idx + 2],
            p_i_apdu[idx + 3], p_i_apdu[idx + 4]);

    len_offset += 1;
    if (p_i_apdu[idx + 1] == 0x81)
    {
        length = p_i_apdu[idx + 2];
        len_offset += 2;
    }
    else if (p_i_apdu[idx + 1] == 0x82)
    {
        length = p_i_apdu[idx + 2];
        length = p_i_apdu[idx + 3] | (length << 8);
        len_offset += 3;
    }
    else
    {
        length = p_i_apdu[idx + 1];
        len_offset += 1;
    }

    DPRINT_HEX(DBG_CIPHER, "<-- Rx Cypher", p_i_apdu, length + 2, DUMP_SEC);

    idx += len_offset;
    sc = p_i_apdu[idx++];

    p_sc_field = (T_DLMS_SC *)&sc;
    dsm_sec_sc_field_set(p_sc_field);

    DPRINTF(DBG_NONE, _D "SC[0x%02X]: A[%d]E[%d]Suite[%d]\r\n", sc,
            p_sc_field->auth, p_sc_field->enc, p_sc_field->suite_id);

    ic = GET_BE32(&p_i_apdu[idx]);
    memcpy(ic8, &p_i_apdu[idx], DLMS_IC_FIELD_LEN);
    DPRINTF(DBG_NONE,
            _D "LEN[%02d], IC[0x%08X], idx[%d], p_i_apdu[idx]=0x%02X\r\n",
            length, ic, idx, p_i_apdu[idx]);

#if 1 /* bccho, HLS, 2023-08-15 */
    if (sc != 0x3F)
#else
    if (sc != AMI_SC_ARIA)
#endif /* bccho */
    {
        log_cert_ng(SEC_LOG_DEC_MIS);
        return FALSE;
    }

    idx += DLMS_IC_FIELD_LEN;
    text_idx = idx;

    length -= DLMS_SEC_HDR_LEN;

    if (p_sc_field->auth == 1)
    {
        text_len = length - DLMS_AUTH_TAG_LEN;
    }
    else
    {
        text_len = length;
    }
#if 1 /* bccho, HLS, 2023-08-15 */
    int plain_len = text_len;
    int rtn = hls_decrypt_message(hls_ctx, ic8, sc, &p_i_apdu[text_idx], length,
                                  p_decipher, &plain_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_decrypt_message() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    DPRINT_HEX(DBG_CIPHER, "<-- Rx Plain", p_decipher, plain_len, DUMP_SEC);

    *decipher_len = plain_len;
    return TRUE;
#else
    dsm_sec_set_operation(true);
    at_idx = text_idx + text_len;

    sRv = _amiEdVerify(p_decipher, (uint8_t *)SYS_TITLE_client, ic8,
                       sc /*AMI_SC_ARIA*/, &p_i_apdu[text_idx], text_len,
                       &p_i_apdu[at_idx], CERT_DS_CLIENT);
    DPRINTF(DBG_NONE, _D "%s: sRv[0x%04X]\r\n", __func__, sRv);
    *decipher_len = text_len;

    DPRINT_HEX(DBG_INFO, "DEC_DATA", p_decipher, text_len, DUMP_ALWAYS);

    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: dec Fail!!!\r\n", __func__);
    dsm_sec_recovery();

    log_cert_ng(SEC_LOG_DEC_MIS);

    return FALSE;
#endif /* bccho */
}

uint16_t dsm_sec_g_signing_opt_build(uint8_t *p_o_apdu)
{
    uint16_t idx = 0;

    p_o_apdu[idx++] = 0x00;
    p_o_apdu[idx++] = SYS_TITLE_LEN;
    memcpy(&p_o_apdu[idx], SYS_TITLE_server, SYS_TITLE_LEN);
    idx += SYS_TITLE_LEN;
    p_o_apdu[idx++] = SYS_TITLE_LEN;
    memcpy(&p_o_apdu[idx], SYS_TITLE_client, SYS_TITLE_LEN);
    idx += SYS_TITLE_LEN;
    p_o_apdu[idx++] = 0x00;
    p_o_apdu[idx++] = 0x00;

    DPRINT_HEX(DBG_TRACE, "SIGN_OPT", p_o_apdu, idx, DUMP_SEC);
    return idx;
}

#if 1 /* bccho, HLS, 2023-08-16 */
int hls_sign_xDLMS_APDU(hls_context_t *ctx, unsigned char *msg, int msg_len,
                        unsigned char *signature, int *signature_len)
{
    int ret = HLS_FAILED;
    unsigned char hash_data[SHA256_LEN] = {
        0,
    };

    /* check input parameter */
    if (ctx == NULL || signature == NULL || signature_len == NULL)
    {
        return HLS_ERR_INPUT_PARAMETER;
    }

    if (ctx->hls_info == NULL)
    {
        return HLS_ERR_INPUT_PARAMETER;
    }

    if (*signature_len < SIGNATURE_R_S_LEN * 2)
    {
        return HLS_ERR_INPUT_PARAMETER;
    }

    /* original data hash */
    if (axiocrypto_hash(HASH_SHA_256, msg, msg_len, hash_data, 32) !=
        CRYPTO_SUCCESS)
    {
        ret = HLS_ERR_SHA256;
        goto end;
    }

    if ((ret = axiocrypto_asym_sign(
             ctx->hls_info->ecdsa_key, hash_data, sizeof(hash_data), HASHED_MSG,
             signature, (uint32_t *)signature_len)) != CRYPTO_SUCCESS)
    {
        ret = HLS_ERR_ECDSA_SIGN;
        goto end;
    }

    if (*signature_len < (2 * SIGNATURE_R_S_LEN))
    {
        ret = HLS_ERR_MISMATCH_ECDSA_SIGN_LEN;
        goto end;
    }

    ret = HLS_SUCCESS;
end:

    return ret;
}

#if 1 /* bccho, HASH, 2023-09-01 */
static ctx_handle_t hHandle = {[31] = 0xCC};
#endif
int hls_verify_sign_xDLMS_APDU(hls_context_t *ctx, unsigned char *msg,
                               int msg_len, unsigned char *signature,
                               int signature_len)
{
    int ret = HLS_FAILED;
    unsigned char hash_data[SHA256_LEN] = {
        0,
    };

    /* original data hash */
    if (axiocrypto_hash(HASH_SHA_256, msg, msg_len, hash_data, 32) !=
        CRYPTO_SUCCESS)
    {
        ret = HLS_ERR_SHA256;
        goto end;
    }

    if (axiocrypto_allocate_slot(hHandle, ASYM_ECDSA_P256, 0) != CRYPTO_SUCCESS)
    {
        ret = HLS_ERR_ECDSA_SIGN;
        goto end;
    }

    if ((ret = axiocrypto_asym_putkey(
             hHandle, ASYM_ECDSA_P256, NULL, 0, 0, ctx->hls_info->other_pubkey,
             HLS_PUBKEY_LEN, 0, CTX_ATTR_NONE)) != CRYPTO_SUCCESS)
    {
        ret = HLS_ERR_ECDSA_SIGN;
        goto end;
    }

    if ((ret = axiocrypto_asym_verify(
             hHandle, hash_data, sizeof(hash_data), HASHED_MSG, signature,
             (uint32_t)signature_len)) != CRYPTO_SIG_ACCEPT)
    {
        ret = HLS_ERR_ECDSA_SIGN;
        goto end;
    }

    /* Check signature length */
    if (signature_len < (2 * SIGNATURE_R_S_LEN))
    {
        ret = HLS_ERR_MISMATCH_ECDSA_SIGN_LEN;
        goto end;
    }

    ret = HLS_SUCCESS;
end:

    return ret;
}
#endif /* bccho */

int16_t dsm_sec_signing_for_month_profile(uint8_t *p_o_sign, uint8_t *p_msg,
                                          uint16_t msg_len)
{
#if 1 /* bccho, HLS, 2023-08-16 */
    int signature_len = DLMS_DS_LEN;
    int rtn =
        hls_sign_xDLMS_APDU(hls_ctx, p_msg, msg_len, p_o_sign, &signature_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_sign_xDLMS_APDU() error!!!, %d\r\n", rtn);
        return FALSE;
    }

    return TRUE;
#else
    int16_t sRv = 0;
    dsm_sec_set_operation(true);
    sRv = _amiDsGenerate(p_o_sign, p_msg, msg_len, CERT_DS);
    if (sRv == 0)
    {
        return TRUE;
    }

    dsm_sec_recovery();

    return FALSE;
#endif
}

#if 1 /* bccho, HLS, 2023-08-16 */
void update_rtc(void);
#endif
int16_t dsm_sec_g_signing_xDLMS_APDU_build(uint8_t *p_o_apdu,
                                           uint16_t *p_o_apdu_len,
                                           uint8_t *p_indata,
                                           uint16_t indata_len)
{
    uint16_t frame_len, msg_idx = 0;
    uint16_t idx = 0;
    uint8_t msg[512];

    dsm_wdt_ext_toggle_immd();

    frame_len = indata_len;

    DPRINTF(DBG_NONE, _D "%s: i_msg_len[%d]\r\n", __func__, indata_len);

    MSG00("sign APDU_build_start, %d", indata_len);

    p_o_apdu[idx++] = APPL_GENERAL_SIGNING;

    msg_idx = dsm_sec_g_signing_opt_build(&p_o_apdu[idx]);
    memcpy(msg, &p_o_apdu[idx], msg_idx);

    idx += msg_idx;

    if (frame_len >= 256)
    {
        p_o_apdu[idx++] = 0x82;
        p_o_apdu[idx++] = (uint8_t)(frame_len >> 8);
        p_o_apdu[idx++] = (uint8_t)(frame_len);
    }
    else if (frame_len >= 128)
    {
        p_o_apdu[idx++] = 0x81;
        p_o_apdu[idx++] = (uint8_t)(frame_len);
    }
    else
    {
        p_o_apdu[idx++] = (uint8_t)(frame_len);
    }

    memcpy(&p_o_apdu[idx], p_indata, indata_len);
    idx += indata_len;
    memcpy(&msg[msg_idx], p_indata, indata_len);
    msg_idx += indata_len;

    dsm_sec_set_operation(true);

    p_o_apdu[idx++] = DLMS_DS_LEN;

#if 1 /* bccho, HLS, 2023-08-16 */
    update_rtc();

    int signature_len = DLMS_DS_LEN;
    int rtn = hls_sign_xDLMS_APDU(hls_ctx, msg, msg_idx, &p_o_apdu[idx],
                                  &signature_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_sign_xDLMS_APDU() error!!!, %d\r\n", rtn);

        log_cert_ng(SEC_LOG_DS_GEN_MIS);
        return FALSE;
    }

    idx += DLMS_DS_LEN;
    *p_o_apdu_len = idx;

    MSG00("sign APDU_build_end");
    update_rtc();

    return TRUE;
#else
    int16_t sRv = 0;

    sRv = _amiDsGenerate(&p_o_apdu[idx], msg, msg_idx, CERT_DS);
    idx += DLMS_DS_LEN;
    *p_o_apdu_len = idx;

    if (sRv == 0)
    {
        DPRINT_HEX(DBG_INFO, "GLO_SIGN_APDU", p_o_apdu, idx, DUMP_ALWAYS);

        return TRUE;
    }

    DPRINTF(DBG_ERR, _D "%s: DS generation fail!!!\r\n", __func__);
    dsm_sec_recovery();

    log_cert_ng(SEC_LOG_DS_GEN_MIS);

    return FALSE;
#endif /* bccho */
}

/* 0x00, 0x08, SYST_O(8), 0x08, SYST_R(8), 0x00, 0x00 --> 21 bytes */
int16_t dsm_sec_g_signing_opt_check(uint8_t *p_i_apdu, uint16_t *p_oidx)
{
    int16_t ret = 0;
    uint16_t idx = 0;
    uint8_t *pt8;

    DPRINT_HEX(DBG_TRACE, "SIGN_OPT", p_i_apdu, 21, DUMP_SEC);

    if (p_i_apdu[idx++] != 0)
        return 0xFF;
    if (p_i_apdu[idx++] != SYS_TITLE_LEN)
        return 0xFF;

    pt8 = &p_i_apdu[idx];
    DPRINT_HEX(DBG_TRACE, "SYST_O", SYS_TITLE_client, SYS_TITLE_LEN, DUMP_SEC);
    DPRINT_HEX(DBG_TRACE, "pt8", pt8, SYS_TITLE_LEN, DUMP_SEC);
    if (memcmp(SYS_TITLE_client, pt8, SYS_TITLE_LEN))
    {
        return 0xFF;
    }
    idx += SYS_TITLE_LEN;

    if (p_i_apdu[idx++] != SYS_TITLE_LEN)
        return 0xFF;

    pt8 = &p_i_apdu[idx];
    DPRINT_HEX(DBG_TRACE, "SYST_R", SYS_TITLE_server, SYS_TITLE_LEN, DUMP_SEC);
    DPRINT_HEX(DBG_TRACE, "pt8", pt8, SYS_TITLE_LEN, DUMP_SEC);
    if (memcmp(SYS_TITLE_server, pt8, SYS_TITLE_LEN))
        return 0xFF;
    idx += SYS_TITLE_LEN;

    if (p_i_apdu[idx++] != 0)
        return 0xFF;
    if (p_i_apdu[idx++] != 0)
        return 0xFF;

    *p_oidx = idx;
    return ret;
}

int16_t dsm_sec_g_signing_xDLMS_APDU_parser(uint8_t *p_i_apdu,
                                            uint16_t xdlms_len,
                                            uint8_t *p_ciphered_idx,
                                            uint16_t *p_ciphered_len)
{
    int16_t sRv = 0;
    uint16_t length, sign_len, o_msgidx = 0;
    uint32_t idx = 0, len_offset = 0, sign_idx = 0;
    uint8_t msg[512];

    dsm_wdt_ext_toggle_immd();

    idx += 1;
    if (dsm_sec_g_signing_opt_check(&p_i_apdu[idx], &o_msgidx) != 0)
        goto signing_err;

    memcpy(msg, &p_i_apdu[idx], o_msgidx);

    DPRINT_HEX(DBG_TRACE, "SIGN_MSG_1", msg, o_msgidx, DUMP_SEC);

    /* 22 bytes skip */
    idx += o_msgidx;

    if (p_i_apdu[idx] == 0x81)
    {
        length = p_i_apdu[idx + 1];
        len_offset = 2;
    }
    else if (p_i_apdu[idx] == 0x82)
    {
        length = p_i_apdu[idx + 1];
        length = p_i_apdu[idx + 2] | (length << 8);
        len_offset = 3;
    }
    else
    {
        length = p_i_apdu[idx];
        len_offset = 1;
    }

    idx += len_offset;
    *p_ciphered_idx = idx;
    *p_ciphered_len = length;

    memcpy(&msg[o_msgidx], &p_i_apdu[idx], length);
    DPRINT_HEX(DBG_TRACE, "SIGN_MSG_2", &msg[o_msgidx], length, DUMP_SEC);
    o_msgidx += length;

    sign_len = p_i_apdu[idx + length];
    idx += 1;
    sign_idx = idx + length;

    DPRINTF(
        DBG_NONE,
        _D
        "%s: ciphered_idx[%d], cipher_len[%d], sign_idx[%d], sign_len[%d]\r\n",
        __func__, *p_ciphered_idx, *p_ciphered_len, sign_idx, sign_len);

    MSG00("sign APDU_parser_start, %d", o_msgidx);

    dsm_sec_set_operation(true);

    DPRINT_HEX(DBG_CIPHER, "<-- Rx Cipher + Sign", p_i_apdu,
               sign_idx + DLMS_DS_LEN, DUMP_SEC);

#if 1 /* bccho, HLS, 2023-08-16 */
    update_rtc();

    int signature_len = DLMS_DS_LEN;
    int rtn = hls_verify_sign_xDLMS_APDU(hls_ctx, msg, o_msgidx,
                                         &p_i_apdu[sign_idx], signature_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_verify_sign_xDLMS_APDU() error!!!, %d\r\n", rtn);
        goto signing_err;
    }

    MSG00("sign APDU_parser_end");

    update_rtc();
    return TRUE;

signing_err:
    DPRINTF(DBG_ERR, _D "%s: DS verification fail!!!\r\n", __func__);

    log_cert_ng(SEC_LOG_DS_VERIFY_MIS);

    return FALSE;
#else
    sRv = _amiDsVerify(msg, o_msgidx, &p_i_apdu[sign_idx], CERT_DS_CLIENT);
    if (sRv == 0)
    {
        return TRUE;
    }

signing_err:
    DPRINTF(DBG_ERR, _D "%s: DS verification fail!!!\r\n", __func__);
    dsm_sec_recovery();

    log_cert_ng(SEC_LOG_DS_VERIFY_MIS);

    return FALSE;
#endif /* bccho */
}

int16_t dsm_sec_g_glo_ciphering_xDLMS_APDU_build(uint8_t *p_xdlms_apdu,
                                                 uint16_t *xdlms_len,
                                                 uint8_t *p_plain,
                                                 uint16_t plain_len)
{
    uint32_t sc_type = 0;
    uint16_t frame_len = 0;
    uint32_t xdlms_index = 0;
    uint8_t *ptr = p_xdlms_apdu;
    T_DLMS_SC sc_field;
    uint8_t sc, ic8[4];
    int16_t sRv = 0;
    uint8_t AT[12];

    if (hls_ctx == NULL)
    {
        hls_ctx = hls_init(HLS_OPMODE_SERVER);
    }

    dsm_wdt_ext_toggle_immd();

    DPRINT_HEX(DBG_CIPHER, "G_GLO_PLAIN", p_plain, plain_len, DUMP_SEC);

    sc_type = DLMS_SC_ENC | DLMS_SC_AT;

    if (sc_type & DLMS_SC_AT)
    {
        frame_len = DLMS_SEC_HDR_LEN + plain_len + DLMS_AUTH_TAG_LEN;
    }
    else if (sc_type & DLMS_SC_ENC)
    {
        frame_len = DLMS_SEC_HDR_LEN + plain_len;
    }

    *ptr++ = APPL_GENERAL_GLO_DATANOTI;
    *ptr++ = SYS_TITLE_LEN;
    memcpy(ptr, SYS_TITLE_server, SYS_TITLE_LEN);
    ptr += SYS_TITLE_LEN;

    if (frame_len >= 256)
    {
        *ptr++ = 0x82;
        *ptr++ = (uint8_t)(frame_len >> 8);
        *ptr++ = (uint8_t)(frame_len);
    }
    else if (frame_len >= 128)
    {
        *ptr++ = 0x81;
        *ptr++ = (uint8_t)(frame_len);
    }
    else
    {
        *ptr++ = (uint8_t)(frame_len);
    }

    memset(&sc_field, 0x00, sizeof(T_DLMS_SC));

    if (sc_type & DLMS_SC_ENC)
        sc_field.enc = 1;
    if (sc_type & DLMS_SC_AT)
        sc_field.auth = 1;

    sc_field.suite_id = 0xf;

    memcpy(&sc, &sc_field, sizeof(T_DLMS_SC));

    *ptr++ = sc;

    dsm_sec_invocation_count_add();
    WRITE_WORD(ptr, g_sec_invocation_count);
    ptr += 4;

    xdlms_index = ptr - p_xdlms_apdu;

    DPRINTF(DBG_NONE, _D "%s: xdlms_len_field[%d], xdlms_hdr_len[%d]\r\n",
            __func__, frame_len, xdlms_index);
    DPRINTF(DBG_NONE, _D "%s: SC[0x%02X], IC[0x%08X]\r\n", __func__, sc,
            g_sec_invocation_count);

    WRITE_WORD(ic8, g_sec_invocation_count);

#if 1 /* bccho, 2023-08-17 */
    int out_len = hls_get_encrypt_len(sc, plain_len);
    int rtn =
        hls_encrypt_message(hls_ctx, ic8, p_plain, plain_len, ptr, &out_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_encrypt_message() error!!!, %d\r\n", rtn);
        log_cert_ng(SEC_LOG_ENC_MIS);
        return FALSE;
    }

    xdlms_index += out_len;
    *xdlms_len = xdlms_index;

    DPRINT_HEX(DBG_CIPHER, "G_GLO_TX_C_APDU", p_xdlms_apdu, *xdlms_len,
               DUMP_SEC);

    return TRUE;
#else
    dsm_sec_set_operation(true);

    sRv = _amiEdGenerate(ptr, AT, SYS_TITLE_server, ic8, AMI_SC_ARIA, p_plain,
                         plain_len, CERT_DS_CLIENT);

    memcpy(ptr + plain_len, AT, DLMS_AUTH_TAG_LEN);

    xdlms_index += plain_len;
    xdlms_index += DLMS_AUTH_TAG_LEN;

    *xdlms_len = xdlms_index;

    DPRINT_HEX(DBG_INFO, "G_GLO_TX_C_APDU", p_xdlms_apdu, *xdlms_len,
               DUMP_ALWAYS);

    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: enc Fail!!!\r\n", __func__);
    dsm_sec_recovery();

    log_cert_ng(SEC_LOG_ENC_MIS);

    return FALSE;
#endif /* bccho */
}

int16_t dsm_sec_g_glo_ciphering_xDLMS_APDU_parser(uint8_t *p_i_apdu,
                                                  uint16_t xdlms_len,
                                                  uint8_t *p_decipher,
                                                  uint16_t *decipher_len)
{
    uint32_t ic;
    uint8_t ic8[4];
    T_DLMS_SC *p_sc_field;
    uint8_t sc = 0;
    uint16_t idx = 0, len_offset = 0, length = 0;
    uint16_t text_idx = 0, at_idx = 0, text_len = 0;
    int16_t sRv = 0;

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_NONE, _D "%s: xDLMS_APDU: %02X: %02X: %02X: %02X: %02X: \r\n",
            __func__, p_i_apdu[idx], p_i_apdu[idx + 1], p_i_apdu[idx + 2],
            p_i_apdu[idx + 3], p_i_apdu[idx + 4]);

    if (p_i_apdu[idx] == APPL_GENERAL_GLO_DATANOTI)
    {
        DPRINTF(DBG_TRACE, "%s: TAG OK\r\n", __func__);
    }
    len_offset += 1;

    if (memcmp(SYS_TITLE_server, &p_i_apdu[idx + 1], SYS_TITLE_LEN) == 0)
    {
        DPRINTF(DBG_TRACE, "%s: SYS-T OK\r\n", __func__);
    }
    len_offset += SYS_TITLE_LEN;

    if (p_i_apdu[idx + 1 + SYS_TITLE_LEN] == 0x81)
    {
        length = p_i_apdu[idx + 2 + SYS_TITLE_LEN];
        len_offset += 2;
    }
    else if (p_i_apdu[idx + 1 + SYS_TITLE_LEN] == 0x82)
    {
        length = p_i_apdu[idx + 2 + SYS_TITLE_LEN];
        length = p_i_apdu[idx + 3 + SYS_TITLE_LEN] | (length << 8);
        len_offset += 3;
    }
    else
    {
        length = p_i_apdu[idx + 1 + SYS_TITLE_LEN];
        len_offset += 1;
    }

    idx += len_offset;
    sc = p_i_apdu[idx++];

    p_sc_field = (T_DLMS_SC *)&sc;
    dsm_sec_sc_field_set(p_sc_field);

    DPRINTF(DBG_NONE, _D "G_GLO SC[0x%02X]: A[%d]E[%d]Suite[%d]\r\n", sc,
            p_sc_field->auth, p_sc_field->enc, p_sc_field->suite_id);

    ic = GET_BE32(&p_i_apdu[idx]);
    memcpy(ic8, &p_i_apdu[idx], DLMS_IC_FIELD_LEN);
    DPRINTF(DBG_NONE,
            _D "G_GLO LEN[%02d], IC[0x%08X], idx[%d], p_i_apdu[idx]=0x%02X\r\n",
            length, ic, idx, p_i_apdu[idx]);

    idx += DLMS_IC_FIELD_LEN;
    text_idx = idx;

    length -= DLMS_SEC_HDR_LEN;

    if (p_sc_field->auth == 1)
    {
        text_len = length - DLMS_AUTH_TAG_LEN;
    }
    else
    {
        text_len = length;
    }

#if 1 /* bccho, 2023-08-17 */
    int plain_len = text_len;
    int rtn = hls_decrypt_message(hls_ctx, ic8, sc, &p_i_apdu[text_idx], length,
                                  p_decipher, &plain_len);
    if (rtn != HLS_SUCCESS)
    {
        DPRINTF(DBG_ERR, "hls_decrypt_message() error!!!, %d\r\n", rtn);
        log_cert_ng(SEC_LOG_DEC_MIS);
        return FALSE;
    }

    DPRINT_HEX(DBG_CIPHER, "G_GLO_DEC_DATA", p_decipher, plain_len, DUMP_SEC);

    *decipher_len = plain_len;
    return TRUE;
#else
    at_idx = text_idx + text_len;
    dsm_sec_set_operation(true);

    sRv = _amiEdVerify(p_decipher, (uint8_t *)SYS_TITLE_server, ic8, sc,
                       &p_i_apdu[text_idx], text_len, &p_i_apdu[at_idx],
                       CERT_DS_CLIENT);
    *decipher_len = text_len;

    DPRINT_HEX(DBG_INFO, "G_GLO_DEC_DATA", p_decipher, text_len, DUMP_ALWAYS);

    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: dec Fail!!!\r\n", __func__);
    dsm_sec_recovery();

    log_cert_ng(SEC_LOG_DEC_MIS);

    return FALSE;
#endif /* bccho */
}

int16_t dsm_sec_generate_key_pair(uint8_t dlms_key_pair_type)
{
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
    int16_t sRv = 0;
    uint8_t bKeyType = 0;

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_TRACE, "%s: dlms_key_pair_type[%d]\r\n", __func__,
            dlms_key_pair_type);

    switch (dlms_key_pair_type)
    {
    case DLMS_KEY_PAIR_DS:
        bKeyType = CERT_KEY_INT_DEV_ECDSA;
        break;

    case DLMS_KEY_PAIR_KA:
        bKeyType = CERT_KEY_INT_DEV_ECDH;
        break;

    default:
        DPRINTF(DBG_WARN, "%s: dlms_key_pair_type error\r\n", __func__);
        return FALSE;
    }
    dsm_sec_set_operation(true);
    sRv |= _certEraseCertKey(SELECT_ALL, CERT_GEN_KEYPAIR);

    OSTimeDly(OS_MS2TICK(1));

    sRv |= _certGenerateKeypair(bKeyType, CERT_GEN_KEYPAIR);
    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: Fail: sRv[0x%04X]!!!\r\n", __func__, sRv);
    dsm_sec_recovery();
    log_cert_ng(SEC_LOG_ETC_MIS);

    return FALSE;
#else /* bccho */
    return TRUE;
#endif /* bccho */
}

int16_t dsm_sec_generate_csr(uint8_t dlms_key_pair_type)
{
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
    int16_t sRv = 0;

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_TRACE, "%s: dlms_key_pair_type[%d]\r\n", __func__,
            dlms_key_pair_type);

    memset(&gst_csr_info, 0x00, sizeof(ST_CSR_INFO));

    switch (dlms_key_pair_type)
    {
    case DLMS_KEY_PAIR_DS:
        break;

    case DLMS_KEY_PAIR_KA:
        break;

    default:
        DPRINTF(DBG_WARN, "%s: dlms_key_pair_type error\r\n", __func__);
        return FALSE;
    }
    dsm_sec_set_operation(true);

    sRv |= _certGetPubKeyCsr(gst_csr_info.info, CERT_GEN_KEYPAIR);
    if (sRv == 0)
    {
        if (gst_csr_info.info[0] == 0x30 && gst_csr_info.info[1] == 0x81)
            gst_csr_info.cnt = gst_csr_info.info[2] + 3;
        else if (gst_csr_info.info[0] == 0x30 && gst_csr_info.info[1] == 0x82)
            gst_csr_info.cnt = (((uint16_t)gst_csr_info.info[2] << 8) |
                                ((uint16_t)(gst_csr_info.info[3]))) +
                               4;
        else
            gst_csr_info.cnt = gst_csr_info.info[1] + 2;

        gst_csr_info.result = 1;
        DPRINT_HEX(DBG_TRACE, "CSR", gst_csr_info.info, gst_csr_info.cnt,
                   DUMP_ALWAYS);

        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: Fail: sRv[0x%04X]!!!\r\n", __func__, sRv);
    dsm_sec_recovery();
    log_cert_ng(SEC_LOG_ETC_MIS);

    return FALSE;
#else
    return TRUE;
#endif
}

int16_t dsm_sec_import_certificate(uint8_t *pcert, uint16_t len)
{
#if 0 /* bccho, KEYPAIR, 2023-07-15 */
    int16_t sRv = 0;

    dsm_wdt_ext_toggle_immd();

    DPRINTF(DBG_TRACE, "%s: [%02X, %02X, %02X]\r\n", __func__, pcert[0],
            pcert[1], pcert[2]);

    OSTimeDly(OS_MS2TICK(1));
    dsm_sec_set_operation(true);

    sRv = _ksePowerOn(
        power_on_rlt.abVer, &power_on_rlt.bLifeCycle, power_on_rlt.chipSerial,
        power_on_rlt.abSystemTitle, &power_on_rlt.bVcType,
        &power_on_rlt.bMaxVcRetryCount, &power_on_rlt.usMaxKcmvpKeyCount,
        &power_on_rlt.usMaxCertKeyCount, &power_on_rlt.usMaxIoDataSize,
        &power_on_rlt.usInfoFileSize);

    dsm_sec_por_print(&power_on_rlt);

    sRv = _certEraseCertKey(SELECT_ALL, CERT_GEN_KEYPAIR);

    OSTimeDly(OS_MS2TICK(1));

    sRv |= _certPutCertPubKey(pcert, CERT_GEN_KEYPAIR, 1);

    if (sRv == 0)
    {
        return TRUE;
    }

    DPRINTF(DBG_ERR, "%s: Fail: sRv[0x%04X]!!!\r\n", __func__, sRv);
    dsm_sec_recovery();
    log_cert_ng(SEC_LOG_ETC_MIS);

    return FALSE;
#else /* bccho */
    return TRUE;
#endif /* bccho */
}
