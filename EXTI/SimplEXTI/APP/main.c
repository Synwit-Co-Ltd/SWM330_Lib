#include "SWM330.h"


int main(void)
{
	SystemInit();
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);		// output, connect LED
	
	GPIO_INIT(GPIOA, PIN8, GPIO_INPUT_PullUp);	// input, pull-up enable, connect key
	
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
