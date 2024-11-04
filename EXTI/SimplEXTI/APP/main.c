#include "SWM330.h"


int main(void)
{	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);			// output, connect LED
	
	GPIO_Init(GPIOB, PIN6, 0, 1, 0, 0);			// input, pull-up enable, connect key
	
	EXTI_Init(GPIOB, PIN6, EXTI_FALL_EDGE);		// falling edge on PB6 pin trigger a EXTI interrupt
	
	NVIC_EnableIRQ(EXTI6_IRQn);
	
	EXTI_Open(GPIOB, PIN6);
	
	while(1==1)
	{
	}
}


void EXTI6_Handler(void)
{
	if(EXTI_State(GPIOB, PIN6))
	{
		EXTI_Clear(GPIOB, PIN6);
		
		GPIO_InvBit(GPIOA, PIN5);
	}
}
