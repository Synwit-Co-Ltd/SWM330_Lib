#include "SWM330.h"


void SerialInit(void);
void RTC_Config(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	SYS->LRCCR = (1 << SYS_LRCCR_ON_Pos);			// Turn on 32KHz LRC oscillator
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);				// output, connect a LED
	
	RTC_Config();
	SYS->RTCWKCR |= (1 << SYS_RTCWKCR_EN_Pos);		// enable RTC wake-up
	
	while(1==1)
	{
		GPIO_SetBit(GPIOA, PIN5);					// turn on the LED
		for(int i = 0; i < SystemCoreClock/4; i++) __NOP();
		GPIO_ClrBit(GPIOA, PIN5);					// turn off the LED
		
		__disable_irq();
		switchTo20MHz();							// Before sleep, switch to 20MHz
		
		SYS->RTCWKSR = SYS_RTCWKSR_FLAG_Msk;		// clear wake-up flag
		RTC->PWRCR |= (1 << RTC_PWRCR_SLEEP_Pos);	// enter sleep mode
		while((SYS->RTCWKSR & SYS_RTCWKSR_FLAG_Msk) == 0) __NOP();
		
		switchToPLL(1, 3, 60, PLL_OUT_DIV8, 0);		// After waking up, switch to PLL
		__enable_irq();
	}
}


RTC_DateTime dateTime;
RTC_AlarmStructure alarmStruct;

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
	
	alarmStruct.Mode = RTC_ALARM_Daily;
	alarmStruct.Hour = 10;
	alarmStruct.Minute = 5;
	alarmStruct.Second = 8;
	alarmStruct.AlarmIEn = 1;
	
	RTC_AlarmSetup(RTC, RTC_ALARM_A, &alarmStruct);
}

void RTC_Handler(void)
{
	if(RTC_INTStat(RTC, RTC_IT_ALRMA))
	{
		RTC_INTClr(RTC, RTC_IT_ALRMA);
		
		RTC_GetDateTime(RTC, &dateTime);
		printf("Now Time: %02d : %02d\r\n", dateTime.Minute, dateTime.Second);
	}
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
