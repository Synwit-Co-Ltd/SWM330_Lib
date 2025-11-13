#include "SWM330.h"


void SerialInit(void);
void RTC_Config(void);

int main(void)
{
	for(int i = 0; i < SystemCoreClock; i++) __NOP();
	
	SystemInit();
	
	SerialInit();
	
	SYS->RCCR |= SYS_RCCR_LON_Msk;					// Turn on 32KHz LRC
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);			// output, connect a LED
	
	RTC_Config();
	RTC_WakeupSetup(RTC, 5, 0);						// wake up every 5 seconds
	SYS->RTCWKCR |= SYS_RTCWKCR_EN_Msk;				// enable RTC wake-up
	
	while(1==1)
	{
		GPIO_SetBit(GPIOA, PIN5);					// turn on the LED
		for(int i = 0; i < SystemCoreClock/4; i++) __NOP();
		GPIO_ClrBit(GPIOA, PIN5);					// turn off the LED
		
		__disable_irq();
		
		switchTo8MHz();
		SYS->PLLCR |= SYS_PLLCR_OFF_Msk;
		SYS->XTALCR &= ~SYS_XTALCR_ON_Msk;
		
		SYS->RTCWKSR = SYS_RTCWKSR_FLAG_Msk;		// clear wake-up flag
		RTC->PWRCR |= RTC_PWRCR_SLEEP_Msk;			// enter sleep mode
		while((SYS->RTCWKSR & SYS_RTCWKSR_FLAG_Msk) == 0) __NOP();	// wait wake-up
		
		switchToPLL(1, 2, 50, 2, 0);				// After waking up, switch to PLL
		
		__enable_irq();
		
		RTC_DateTime dateTime;
		RTC_GetDateTime(RTC, &dateTime);
		printf("Now Time: %02d : %02d\r\n", dateTime.Minute, dateTime.Second);
	}
}


void RTC_Config(void)
{
	RTC_InitStructure RTC_initStruct;
	
	RTC_initStruct.clksrc = RTC_CLKSRC_LRC32K;
	RTC_initStruct.Year = 2018;
	RTC_initStruct.Month = 3;
	RTC_initStruct.Date = 23;
	RTC_initStruct.Hour = 10;
	RTC_initStruct.Minute = 5;
	RTC_initStruct.Second = 5;
	RTC_Init(RTC, &RTC_initStruct);
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
