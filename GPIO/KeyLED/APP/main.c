#include "SWM330.h"


/* 	Press the button connected to PA8, and the LED connected to PA5 will light up;
	Release the button and the LED goes off!
*/


int main(void)
{	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN8, 0, 1, 0, 0);				// input, pull-up enable, connect key
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);				// output, connect LED
	
	while(1==1)
	{
		if(GPIO_GetBit(GPIOA, PIN8) == 0)			// key pressed
		{
			GPIO_SetBit(GPIOA, PIN5);
		}
		else
		{
			GPIO_ClrBit(GPIOA, PIN5);
		}
	}
}

