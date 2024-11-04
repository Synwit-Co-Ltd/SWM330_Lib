#include "SWM330.h"


int main(void)
{	
	SystemInit();
	
	PORT_Init(PORTA, PIN8, FUNMUX0_BTIMR0_OUT, 0);
	
	TIMR_Init(BTIMR0, TIMR_MODE_OC, CyclesPerUs, 10000, 0);
	
	TIMR_OC_Init(BTIMR0, 7500, 0, 1);
	
	TIMR_Start(BTIMR0);
	
	while(1==1)
	{
	}
}

