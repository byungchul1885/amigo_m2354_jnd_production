#ifndef KEY_H
#define KEY_H 1

#define KEY_CHAT_CNT 3

#define CONTK_L_BIT BIT7
#define COMMK_BIT BIT6
#define MENUK_BIT BIT0
#define MOVEK_BIT BIT1
#define INPUTK_BIT BIT2
#define CALK_BIT BIT3
#define DOUBLEK_BIT BIT4
#define CONTK_S_BIT BIT5

typedef enum
{
    KACT_NONE = 0,
    KACT_MENU = MENUK_BIT,
    KACT_MOVE = MOVEK_BIT,
    KACT_CONT_MENU = (MENUK_BIT | CONTK_L_BIT),
    KACT_CONT_MOVE = (MOVEK_BIT | CONTK_L_BIT),
    KACT_DOUBLEK = (DOUBLEK_BIT | CONTK_L_BIT),
    KACT_CONT_S_CAL = (CALK_BIT | CONTK_S_BIT),
    KACT_CONT_CAL = (CALK_BIT | CONTK_L_BIT),
    KACT_CONT_S_MENU = (MENUK_BIT | CONTK_S_BIT),
    KACT_CONT_S_MOVE = (MOVEK_BIT | CONTK_S_BIT)
} kaction_type;

extern int key_code;
extern bool key_pressed;
extern bool b_inst_key_cancel;
extern bool scurr_block_dur_kinp;
extern bool input_state_by_comm;

void key_init(void);
void key_proc(void);
void kinp_init(void);
void kact_inp_dsp(kaction_type kact, uint8_t *tptr);
void kact_inp_dsp_exit(void);
void kact_test_dsp_exit(void);
uint8_t get_tou_inp_index(void);
uint8_t get_ts_inp_index(void);
void key_circ_step_mode_clear(void);
void set_key_code(uint8_t kval);
bool kact_scurr_is_blocked(void);
bool sCurr_limit_is_valid(uint16_t _val);
bool sCurr_dur_is_valid(uint16_t _dur);
void R_CALL(void);

#endif
