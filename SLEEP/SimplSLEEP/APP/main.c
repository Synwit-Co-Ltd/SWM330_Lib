#include "SWM330.h"


/* sleep wake-up source: GPIO pin, RTC_GPIO pin, RTC Wakeup.
 * deep-sleep wake-up source:      RTC_GPIO pin, RTC Wakeup.
 */

#define DEEP_SLEEP		0
#define GPIO_WAKEUP		1
#define RTC_WAKEUP		0


void SerialInit(void);
void RTC_Config(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	SYS->RCCR |= SYS_RCCR_LON_Msk;					// Turn on 32KHz LRC
	SYS->CLKEN1 |= SYS_CLKEN1_RTC_Msk;
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);			// output, connect a LED
	
#if GPIO_WAKEUP
	GPIO_INIT(GPIOA, PIN8, GPIO_INPUT_PullUp);		// input, pull-up enable, connect a key
	SYS->PAWKEN |= (1 << PIN8);						// enable PA8 pin low level wake-up
#endif
	
#if RTC_WAKEUP
	RTC_Config();
#endif
	
	while(1==1)
	{
		GPIO_SetBit(GPIOA, PIN5);					// turn on the LED
		for(int i = 0; i < SystemCoreClock; i++) __NOP();
		GPIO_ClrBit(GPIOA, PIN5);					// turn off the LED
		for(int i = 0; i < SystemCoreClock; i++) __NOP();
		
		printf("Enter sleeping\n");
		__disable_irq();
		
		switchTo8MHz();
		SYS->PLLCR |= SYS_PLLCR_OFF_Msk;
		SYS->XTALCR &= ~SYS_XTALCR_ON_Msk;
		
		RTC->PWRCR |= RTC_PWRCR_SLEEP_Msk | (DEEP_SLEEP << RTC_PWRCR_DEEPSLP_Pos);	// enter sleep mode
		for(int i = 0; i < CyclesPerUs; i++) __NOP();
		
		switchToPLL(1, 2, 50, 2, 0);				// After waking up, switch to PLL
		
		__enable_irq();
		printf("Leave sleeping\n");
		
#if RTC_WAKEUP
		RTC_DateTime dateTime;
		RTC_GetDateTime(RTC, &dateTime);
		printf("Now Time: %02d : %02d\r\n", dateTime.Minute, dateTime.Second);
#endif
	}
}


void RTC_Config(void)
{
	RTC_InitStructure RTC_initStruct;
	
	RTC_initStruct.clksrc = RTC_CLKSRC_LRC32K;
	RTC_initStruct.Year = 2026;
	RTC_initStruct.Month = 1;
	RTC_initStruct.Date = 16;
	RTC_initStruct.Hour = 10;
	RTC_initStruct.Minute = 5;
	RTC_initStruct.Second = 5;
	RTC_Init(RTC, &RTC_initStruct);
	
	SYS->RTCWKCR |= SYS_RTCWKCR_EN_Msk;				// enable RTC wake-up
	
	RTC_WakeupSetup(RTC, 10, 0);					// wake up after 10 seconds of sleep
	
	RTC->TAMPER = RTC_TAMPER_ENA_Msk | RTC_TAMPER_POLAR_Msk |	// fall-edge tamper detect
				  (7 << RTC_TAMPER_SAMFREQ_Pos) |				// 128 samples per second
				  (0 << RTC_TAMPER_PUPDIS_Pos);					// RTC_GPIO pull-up enable
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN6, FUNMUX0_UART0_TXD, 0);
	PORT_Init(PORTA, PIN7, FUNMUX1_UART0_RXD, 1);
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThreshold = 3;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
