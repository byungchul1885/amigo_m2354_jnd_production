
#ifndef PHY_H
#define PHY_H 1

#define CHAR_FLAG 0x7e

typedef enum
{
    BAUD_300,
    BAUD_600,
    BAUD_1200,
    BAUD_2400,
    BAUD_4800,
    BAUD_9600,
    BAUD_19200,
    BAUD_38400,
    BAUD_57600,
    BAUD_115200
} baudrate_type;

extern bool b_interframe_timer_enable;
extern int phy_interframe_timer;

void phy_init(void);
char* dsm_485_baud_string(baudrate_type baud);
void amr_rcv_frame(void);
void modem_conf(baudrate_type baud, bool poll);
void phy_rxstate_reset(void);

#endif
