// **************************************************************************
//  DESCRIPTION: Implement a dual-priority monitor.
//  This implements two priorities: preemptible, and non-preemptible.
//  Since there's only two, priority inversion is impossible.
//  Since the code runs to completion, hangs are impossible
//  Since there's only one locking flag, and thus one order
//  to get it, deadlocks are impossible.
//  Therefore, it's the guts of a very reliable exokernel.
#ifndef IRQ_H
#define IRQ_H 1

#include "os_wrap.h"

#define IRQ_DISABLE() OS_ENTER_CRITICAL()
#define IRQ_ENABLE()  OS_EXIT_CRITICAL()

#endif

