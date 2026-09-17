#include "SWM330.h"

/* STOP mode wake-up source: RTC_GPIO pin, RTC Wakeup. reset pin cannot work in STOP mode.
 */


void SerialInit(void);

int main(void)
{	
	RTC_InitStructure RTC_initStruct;

	for(int i = 0; i < SystemCoreClock/10 ;i++) __NOP();	// Prevents unable to download programs

	SystemInit();
	SerialInit();

	GPIO_INIT(GPIOA, PIN8, GPIO_OUTPUT);			// output, connect LED
	GPIO_SetBit(GPIOA, PIN8);
	for(int i = 0; i < SystemCoreClock ;i++) __NOP();	
	
	RTC_initStruct.clksrc = // for STOP mode, can only be RTC_CLKSRC_XTAL32K;

	RTC_initStruct.Year = 2018;
	RTC_initStruct.Month = 3;
	RTC_initStruct.Date = 23;
	RTC_initStruct.Hour = 10;
	RTC_initStruct.Minute = 5;
	RTC_initStruct.Second = 5;
	RTC_Init(RTC, &RTC_initStruct);
		
	printf("BACKUP[0]: %08X\r\n", RTC->BACKUP[0]);
	RTC->BACKUP[0] += 1;
	
	RTC->TAMPER = (1<<RTC_TAMPER_ENA_Pos)|(1<<RTC_TAMPER_IE_Pos) | 
							  (7<<RTC_TAMPER_SAMFREQ_Pos) ;
								
								
	RTC->PWRCR |= (1 << RTC_PWRCR_STOP_Pos);	// enter STOP mode

	while(1==1)
	{

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
