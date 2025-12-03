#ifndef __AMG_CAN_H__
#define __AMG_CAN_H__

typedef enum
{
    CAN_BITRATE_10K,    ///< 10k baud
    CAN_BITRATE_20K,    ///< 20k baud
    CAN_BITRATE_50K,    ///< 50k baud
    CAN_BITRATE_100K,   ///< 100k baud
    CAN_BITRATE_125K,   ///< 125k baud
    CAN_BITRATE_250K,   ///< 250k baud
    CAN_BITRATE_500K,   ///< 500k baud
    CAN_BITRATE_1000K,  ///< 1M baud
} canBitrate;

void dsm_can_init(uint32_t baudrate_idx);
void dsm_can_deinit(void);
void dsm_can_tx(uint32_t ext_id, uint8_t* ptx, uint8_t len);
void dsm_can_rx_int_cb(void);
#ifdef M2354_CAN /* bccho, 2023-11-28 */
void CAN_NormalMode_SetRxMsg(CAN_T* tCAN);
#endif
#endif  // __AMG_CAN_H__
