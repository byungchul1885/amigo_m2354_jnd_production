#ifndef __ISOTP_CONFIG__
#define __ISOTP_CONFIG__

/* Max number of messages the receiver can receive at one time, this value
 * is affectied by can driver queue length
 */

#if 0 /* bccho, 2023-12-22 */
#define ISO_TP_DEFAULT_BLOCK_SIZE 8
#else
#define ISO_TP_DEFAULT_BLOCK_SIZE 128
#endif

/* The STmin parameter value specifies the minimum time gap allowed between
 * the transmission of consecutive frame network protocol data units
 */
#if 0 /* bccho, 2023-12-22 */
#define ISO_TP_DEFAULT_ST_MIN 0
#else
#define ISO_TP_DEFAULT_ST_MIN 1
#endif

/* This parameter indicate how many FC N_PDU WTs can be transmitted by the
 * receiver in a row.
 */
#define ISO_TP_MAX_WFT_NUMBER 1

/* Private: The default timeout to use when waiting for a response during a
 * multi-frame send or receive.
 */
// #define ISO_TP_DEFAULT_RESPONSE_TIMEOUT 100
#define ISO_TP_DEFAULT_RESPONSE_TIMEOUT 2000

/* Private: Determines if by default, padding is added to ISO-TP message frames.
 */
#define ISO_TP_FRAME_PADDING

#endif
