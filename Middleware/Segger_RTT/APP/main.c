#include "SWM330.h"

#include "SEGGER_RTT.h"

void SerialInit(void);

int main(void)
{
 	uint32_t i;
 	
 	SystemInit();
	
	SerialInit();
	
	SEGGER_RTT_Init();
 	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);		// output
	
 	while(1==1)
 	{
 		GPIO_InvBit(GPIOA, PIN5);
 		for(i = 0; i < SystemCoreClock/5; i++) __NOP();
		
   		printf("Hi, World!\r\n");
		
		SEGGER_RTT_printf(0, "Hi, World!\r\n");
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
 	while(UART_IsTXFIFOFull(UART0));
	
	UART_WriteByte(UART0, ch);
 	
	return ch;
}
