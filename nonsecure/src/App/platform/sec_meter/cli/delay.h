#ifndef DELAY_H
#define DELAY_H 1

#include "max71315.h"

void delay (int32_t cclk);

#define US2CLK(__y__) (((__y__ * SYS_CLK * 2) + 500000) / 1000000)
#define MS2CLK(__y__) (((__y__ * SYS_CLK * 2) + 500) / 1000)

#define DELAY_US(__y__) (((__y__ * (SYS_CLK/100)) + 50000) / 100000)
#define DELAY_MS(__y__) (((__y__ * (SYS_CLK/100)) + 50) / 100)


#endif // delay


