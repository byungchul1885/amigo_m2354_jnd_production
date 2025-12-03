
#ifndef ACT_REQ_H
#define ACT_REQ_H 1

#define act_is_curr_prog_cmd() (act_cmd_sel_ap == 0)
#define act_is_progdl_cmd() (act_devcmd == DEVICE_CMD_PGM_DL)

extern uint8_t act_cmd_sel_ap;
extern device_cmd_type act_devcmd;  // pre_action command

void approc_act_req(int idx);
void ob_dev_cmd(uint16_t cmd);

void dsm_touimage_default(U8 rate_2);
void dsm_image_update_go_proc(void);

#endif
