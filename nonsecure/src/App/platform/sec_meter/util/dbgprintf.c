#include "options.h"
#include "ser.h"
#include "dlms_todo.h"
#include "meter.h"


#define DBG_STR_SIZE		10

static bool b_debug_print = false;

void debug_port_chg(void)
{
	int _baud = 0;

	#ifdef SER_1
	ser_init(1, _baud);
	#endif

	b_debug_print = true;
}


