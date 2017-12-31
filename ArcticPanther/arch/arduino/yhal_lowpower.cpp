#include "LowPower.h"
#include <ytimer.h>
#include <yassert.h>

/* puts CPU into a short (<1s) sleep */
void yhal_enter_ms_sleep()
{
	LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
	ytimer_millis_add_skew(1000);
}