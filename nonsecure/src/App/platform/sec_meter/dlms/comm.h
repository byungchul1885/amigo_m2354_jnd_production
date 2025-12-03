
#ifndef COMM_H
#define COMM_H 1

extern bool mdm_conf_delay_until_txcomplete;
extern bool mt_init_delay_until_txcomplete;
extern bool sap_is_utility_when_mt_init;
extern bool comm_en_coveropen_changed;
extern uint8_t comm_en_coveropen_val;
extern int amr_tx_len;

void amr_init(void);
void amr_proc(void);
void amr_set_send_frame(uint8_t *buf, int len, int dly);
bool comm_is_connected(void);
bool fcs16 (uint8_t *cp, uint16_t len, bool set);
uint16_t crc16_update(uint16_t fcs, uint8_t *cp, int len);
void amr_disc_ind_end_proc(void);
void amr_send_frame(uint32_t poll_flag);



#endif

