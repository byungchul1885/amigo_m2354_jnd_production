#ifndef OPTIONS_H
#define OPTIONS_H 1

#include "options_sel.h"
#include "options_def.h"

#define SER_1

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "amg_typedef.h"
#include "max71315.h"

#include "dbgprintf.h"
#include "softimer.h"
#include "whm.h"

#ifndef EQUATION            // Experimental 1-phase and split phase code?
#define EQUATION EQUATION0  // Meter equation: 3-phase: Wh = V0*I0
#endif

#define WHPPNUM 1           // 1 Wh per pulse on 1-phase
#define WHPPDEN 1           // 1 Wh per pulse.
#define WHPDISPMAX 1000L    // Keeps display from underflow.
#define WHPMAX 1000000000L  // Turn over a pulse counter at this number.

// Begin options for command line interpreter. Not needed in every meter.
#define BATTERY_MODE 0          // 1=enable battery mode code.
#define EEPROM 0                // 1=enable EEPROM CLI code.
#define HELP 1                  // 0=remove all online help text to save space
#define HIGH_PRECISION_METER 0  // 1= high-precision Vref temp. compensation.
#define POWER_SAVE_MODE \
    0  // 1=Enable CLI code to test power saving modes. (Not in A-version part)
// End the options for the command line interpreter.

#include "irq.h"    // Interrupt monitor
#include "mmath.h"  // Meter math: lmin, lmax, labs, lroundf, add(), etc.
#include "rtc.h"    // real time clock interface
#include "ce.h"     // CE RAM and variable mapping
#include "cal.h"    // calibration API and data structures
#include "meter.h"  // meter API and data structures
#include "amg_wdt.h"

#define MAX_DEMAND 50400  // 240V * 120% * 50A * 120% * 3 phases

#define TEST_PORT_INIT(a)
#define TEST_PORT_OUT_H(a)
#define TEST_PORT_OUT_L(a)
#define TEST_PORT_XOR(a)

void error_software(void);

// Define errors, by decreasing importance
#define ERROR_ERROR 30            // Error system failed.
#define ERROR_TAMPER 29           // Tamper detected
#define ERROR_RTC_TAMPER 28       // RTC tamper was detected
#define ERROR_REGISTER_BAD 27     // Billing register read had bad CRC
#define ERROR_BATTERY_BAD 26      // Battery voltage has been too low
#define MAGNETIC_TAMPER 25        // Magnetic tamper detected
#define ERROR_HARDWARE 24         // Unknown hardware error.
#define ERROR_RTC_UNSET 23        // RTC has not been set yet.
#define ERROR_CALIBRATION_BAD 22  // Calibration has a bad CRC
#define ERROR_OVERVOLTAGE 21      // AC Mains voltage got too high
#define ERROR_OVERCURRENT 20      // AC Mains current exceeded nominal
#define ERROR_SAG 19              // AC Mains below limit
#define ERROR_NEUTRAL 18          // Neutral current out of spec.
#define ERROR_SPURIOUS 17         // Spurious interrupt detected
#define ERROR_SOFTWARE 16         // Software ('impossible') error.
// 0..15 are for status bits, not errors exactly.
#define LINE_BAD 9   // power is measured from neutral
#define RTC_ALARM 8  // RTC posted an alarm
#define CREEP 7      // Meter is in creep mode
#define CREEPV 6     // All voltages are in creep
#define MINVB 4      //  Voltage, phase B is in creep.
#define MINVA 3      //  Voltage, phase A is in creep.
#define MINIB 1      //  Current, phase B is in creep
#define MINIA 0      //  Current, phase A is in creep

// The masks are directly derived from the error numbers above.
#define ERROR_ERROR_MASK (1L << ERROR_ERROR)
#define ERROR_TAMPER_MASK (1L << ERROR_TAMPER)
#define ERROR_RTC_TAMPER_MASK (1L << ERROR_RTC_TAMPER)
#define ERROR_REGISTER_BAD_MASK (1L << ERROR_REGISTER_BAD)
#define ERROR_BATTERY_BAD_MASK (1L << ERROR_BATTERY_BAD)
#define MAGNETIC_TAMPER_MASK (1L << MAGNETIC_TAMPER)
#define ERROR_HARDWARE_MASK (1L << ERROR_HARDWARE)
#define ERROR_RTC_UNSET_MASK (1L << ERROR_RTC_UNSET)
#define ERROR_CALIBRATION_BAD_MASK (1L << ERROR_CALIBRATION_BAD)
#define ERROR_OVERCURRENT_MASK (1L << ERROR_OVERCURRENT)
#define ERROR_OVERVOLTAGE_MASK (1L << ERROR_OVERVOLTAGE)
#define ERROR_SAG_MASK (1L << ERROR_SAG)
#define ERROR_NEUTRAL_MASK (1L << ERROR_NEUTRAL)
#define ERROR_SPURIOUS_MASK (1L << ERROR_SPURIOUS)
#define ERROR_SOFTWARE_MASK (1L << ERROR_SOFTWARE)
#define LINE_BAD_MASK (1L << LINE_BAD)
#define RTC_ALARM_MASK (1L << RTC_ALARM)
#define CREEP_MASK (1L << CREEP)
#define CREEPV_MASK (1L << CREEPV)
#define MINVA_MASK (1L << MINVA)
#define MINVB_MASK (1L << MINVB)
#define MINIA_MASK (1L << MINIA)
#define MINIB_MASK (1L << MINIB)

// Where does the CE status go?
#define ERROR_CE_MASK (0xFFFL)  // CE status here.
// Which bits indicate real errors, worthy of logging?
#define ERROR_VALID_MASK (0xFFFF0000L)  // Top half of errors.

/*
 * $log$
 */
#endif /* OPTIONS_H */
