#ifndef CAL_H
#define CAL_H 1


#define YES												0x5AA5

extern float Vcal;
extern float Ical;
extern float theta_cal;
extern int Scal;

extern uint16_t cal_flag;


void cal_disable (void);

void cal_begin (void);

void cal_end (void);

void calibrate (uint8_t *tptr);  // Perform calibration. Called by the main loop.
bool cal_restore (uint8_t *tptr);
bool cal_save(uint8_t *tptr);
void cal_start_by_key(void);

void cal_point_init(void);

#endif
