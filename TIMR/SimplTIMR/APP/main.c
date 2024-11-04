#include "SWM330.h"


int main(void)
{
	SystemInit();
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);
	
	TIMR_Init(TIMR0, TIMR_MODE_TIMER, CyclesPerUs, 500000, 1);	// An interrupt is triggered every 0.5 seconds
	
	TIMR_Start(TIMR0);
	
	while(1==1)
	{
	}
}

void TIMR0_Handler(void)
{
	TIMR_INTClr(TIMR0, TIMR_IT_TO);
	
	GPIO_InvBit(GPIOA, PIN5);
}
