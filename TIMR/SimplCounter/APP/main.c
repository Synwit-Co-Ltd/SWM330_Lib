#include "SWM330.h"


int main(void)
{
	SystemInit();
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);
	GPIO_INIT(GPIOA, PIN7, GPIO_OUTPUT);
	
	PORT_Init(PORTA, PIN8, FUNMUX0_TIMR0_IN, 1);		// connect PA7 pin and count the rising edge on PA7 pin
	
	TIMR_Init(TIMR0, TIMR_MODE_COUNTER, 1, 5, 1);		// every 5 rising edges trigger an interrupt
	TIMR_Start(TIMR0);
	
	while(1==1)
	{
		GPIO_InvBit(GPIOA, PIN7);
		for(int i = 0; i < SystemCoreClock/64; i++) __NOP();
	}
}

void BTIMR2_Handler(void)
{
	TIMR_INTClr(TIMR0, TIMR_IT_TO);
	
	GPIO_InvBit(GPIOA, PIN5);
}
