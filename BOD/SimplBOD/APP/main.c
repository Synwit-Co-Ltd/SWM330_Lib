#include "SWM330.h"


int main(void)
{
	SystemInit();
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);	// output, connect a LED
	
	SYS->BODCR = SYS_BODCR_EN_Msk |
				 SYS_BODCR_IE_Msk | (2 << SYS_BODCR_LVL_Pos);		// enable interrupt, interrupt trigger level is 2.7v
	SYS->BODSR = SYS_BODSR_IF_Msk;			// clear interrupt flag
	NVIC_EnableIRQ(BOD_IRQn);
	
	while(1==1)
	{
		GPIO_ClrBit(GPIOA, PIN5);
	}
}


void BOD_Handler(void)
{
	SYS->BODSR = SYS_BODSR_IF_Msk;
	
	GPIO_SetBit(GPIOA, PIN5);
}
