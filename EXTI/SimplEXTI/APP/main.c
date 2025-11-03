#include "SWM330.h"


int main(void)
{
	SystemInit();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);			// output, connect LED
	
	GPIO_Init(GPIOA, PIN8, 0, 1, 0, 0);			// input, pull-up enable, connect key
	
	EXTI_Init(GPIOA, PIN8, EXTI_FALL_EDGE);		// falling edge on PB6 pin trigger a EXTI interrupt
	
	NVIC_EnableIRQ(EXTI8_11_IRQn);
	
	EXTI_Open(GPIOA, PIN8);
	
	while(1==1)
	{
	}
}


void EXTI8_11_Handler(void)
{
	if(EXTI_State(GPIOA, PIN8))
	{
		EXTI_Clear(GPIOA, PIN8);
		
		GPIO_InvBit(GPIOA, PIN5);
	}
}
