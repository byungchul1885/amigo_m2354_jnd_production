#include "options.h"
#include "rtc.h"
#include "tmp.h"
#include "port.h"
#include "whm_1.h"
#include "lp.h"
#include "bat.h"
#include "amg_pwr.h"

bool bat_test_ready;
int nobat_cnt;
bat_state_type bat_state;

static float batlevel_mon;
int batchk_prd;

void bat_meas(void) { batlevel_mon = dsm_bat_volts(); }

float get_bat_level(void) { return batlevel_mon; }
