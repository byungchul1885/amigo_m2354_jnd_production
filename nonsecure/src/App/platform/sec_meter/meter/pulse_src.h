#ifndef PULSE_SRC_H
#define PULSE_SRC_H

#include "meter_app.h"

void SelectPulses (void);
void pcnt_variable_init(void);
void pcnt_accumulate(void);
vi_quarter_type decision_quarter(int32_t ws, int32_t rs);


#endif
