#include "main.h"
#include "options.h"
#include "key.h"
#include "disp.h"
#include "ser1.h"
#include "dlms_todo.h"
#include "dl.h"
#include "appl.h"
#include "comm.h"
#include "phy.h"
#include "amg_dlms_hdlc.h"
#include "amg_media_mnt.h"

#define _D "[PHY485] "

static uint8_t phy_rx_state;

static bool phy_ser_in(void);

void phy_init(void)
{
    phy_rx_state = 0;

    modem_conf(mdm_baud, FALSE);
}

char *dsm_485_baud_string(baudrate_type baud)
{
    switch (baud)
    {
    case BAUD_19200:
        return "19200";
        break;
    case BAUD_38400:
        return "38400";
    case BAUD_57600:
        return "57600";
    case BAUD_115200:
        return "115200";
    default:
        return "9600";
    }
}

void modem_conf(baudrate_type baud, bool poll)
{
    int _baud;

    switch (baud)
    {
    case BAUD_9600:
        _baud = 9600;
        break;
    case BAUD_19200:
        _baud = 19200;
        break;
    case BAUD_38400:
        _baud = 38400;
        break;
    case BAUD_57600:
        _baud = 57600;
        break;
    case BAUD_115200:
        _baud = 115200;
        break;
    default:  // to avoid compile warning
        _baud = 9600;
        break;
    }

#ifdef SER_1
    ser_init(1, _baud);
#endif

    DPRINTF(DBG_TRACE, "%s: baud %d, baud_rate %d\r\n", __func__, baud, _baud);

    dsm_rs485if_init(_baud, poll);
}

void amr_rcv_frame(void)
{
    bool ok;

    ok = phy_ser_in();
    if (ok)
    {
        if (dsp_is_input_state() && !input_state_by_comm)
        {
            dl_proc(false);  // no packet data
        }
        else
        {
            comm_inact_timeset(T120SEC);  // TODO: (WD) duplicated
#if 0
            if ((dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_RS485) &&
                (cli_buff[0] == 0xbc))
            {
                prod_dl_proc(true);
            }
            else
#endif
            {
                dl_proc(true);  // frame buffer => cli_buff[]
            }
        }
    }
    else
    {
        dl_proc(false);  // no packet data
    }
}

void phy_rxstate_reset(void) { phy_rx_state = 0; }

static bool phy_ser_in(void)
{
#define RX_ONE_TIME 10

    static int idx;
    static uint16_t frmlen;
    bool frmok;
    int len;
    uint8_t rbuf[RX_ONE_TIME];
    uint8_t *cp;
    int rxloop_cnt;

    frmok = false;
    rxloop_cnt = 0;
    len = dlms_rx(rbuf, RX_ONE_TIME);  // read rx buffer

    if (len == 0)
    {
        if (inter_frame_is_timeout())
        {
            phy_rxstate_reset();
        }

        return false;
    }

    rxloop_cnt += 1;

    inter_frame_timeset(T500MS);

    cp = &rbuf[0];
    while (true)
    {
        switch (phy_rx_state)
        {
        case 0:
            if (*cp == CHAR_FLAG)
            {
                phy_rx_state++;
            }

            break;
        case 1:
            if (*cp != CHAR_FLAG)
            {
                idx = 0;
                cli_buff[idx++] = *cp;
                phy_rx_state++;
#if 1
                if ((dsm_media_get_fsm_if_hdlc() == MEDIA_RUN_RS485) &&
                    (*cp == 0xbc))
                {
                    frmlen = 0;
                }
                else
#endif
                {
                    frmlen = (U16)((*cp & 0x7) << 8);
                }
            }
            break;
        case 2:
            frmlen |= (uint16_t)(*cp);
            if (frmlen <= (CLI_BUFF_SIZE - 1))
            {
                cli_buff[idx++] = *cp;
                phy_rx_state++;
            }
            else
            {
                phy_rx_state = 0;  // error: exceed buffer
            }
            break;
        case 3:
            cli_buff[idx++] = *cp;
            if (idx >= frmlen)
            {
                phy_rx_state++;
            }
            break;
        case 4:
            if (*cp == CHAR_FLAG)
            {
                if (fcs16(cli_buff, (uint16_t)frmlen, false))
                {
                    // DLMS 프레임 데이터가 정상인 경우
                    frmok = true;  // exit loop
                }
            }

            inter_frame_timeset(0);
            phy_rx_state = 0;
            break;

        default:
            phy_rx_state = 0;
            break;
        }

        cp += 1;
        len -= 1;

        if (frmok)
            break;

        if (len == 0)
        {
            len = dlms_rx(rbuf, RX_ONE_TIME);
            if (len == 0)
                break;

            rxloop_cnt += 1;

            cp = &rbuf[0];
        }
    }

    return frmok;
}
