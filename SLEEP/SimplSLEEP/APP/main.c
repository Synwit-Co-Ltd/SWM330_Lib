#include "SWM330.h"


/* By default, the pull-up resistance of ISP and SWD pins is enabled, which increases the sleep power consumption.
 * To obtain the lowest sleep power consumption, disable the pull-up and pull-down resistance of all pins before sleep
 */


int main(void)
{
	for(int i = 0; i < SystemCoreClock; i++) __NOP();	// Prevents unable to download programs
	
	SystemInit();
	
	SYS->RCCR |= (1 << SYS_RCCR_LON_Pos);			// Turn on 32KHz LRC oscillator
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);			// output, connect a LED
	
	GPIO_INIT(GPIOA, PIN8, GPIO_INPUT_PullUp);		// intput, pull-up enable, connect a key
	SYS->PAWKEN |= (1 << PIN8);						// enable PA10 pin low wake up
	
	while(1==1)
	{
		GPIO_SetBit(GPIOA, PIN5);					// turn on the LED
		for(int i = 0; i < SystemCoreClock/4; i++) __NOP();
		GPIO_ClrBit(GPIOA, PIN5);					// turn off the LED
		
		__disable_irq();
		switchTo8MHz();								// Before sleep, switch to 8MHz
		
		SYS->PAWKSR = (1 << PIN8);					// clear wakeup flag
		RTC->PWRCR |= (1 << RTC_PWRCR_SLEEP_Pos);	// enter sleep mode
		while((SYS->PAWKSR & (1 << PIN8)) == 0);	// wait wake-up
		
		switchToPLL(1, 2, 60, 2, 0);				// After waking up, switch to PLL
		__enable_irq();
	}
}
