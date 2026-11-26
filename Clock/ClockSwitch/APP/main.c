#include "SWM330.h"


/* system clock switching demo program */


void SerialInit(void);
void SystemInit_80MHz(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
   	
	printf("SystemCoreClock = %dMHz\r\n", SystemCoreClock/1000000);
	
	while(1==1)
	{
		SystemInit_80MHz();
		
		UART_SetBaudrate(UART0, 57600);		// after the system frequency is changed, the UART baudrate needs to be reconfigured
		
		for(int i = 0; i < 4; i++)
		{
			printf("SystemCoreClock = %dMHz\r\n", SystemCoreClock/1000000);
			
			SW_DelayMS(1000);
		}
		
		SystemInit();
		
		UART_SetBaudrate(UART0, 57600);
		
		for(int i = 0; i < 4; i++)
		{
			printf("SystemCoreClock = %dMHz\r\n", SystemCoreClock/1000000);
			
			SW_DelayMS(1000);
		}
	}
}


void SystemInit_80MHz(void)
{
	Flash_Param_at_xMHz(150);
	
	switchToPLL(0, 2, 40, 2, 0);
	
	SystemCoreClockUpdate();
	
	Flash_Param_at_xMHz(CyclesPerUs);
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
	
	while(UART_IsTXBusy(UART0)) __NOP();
 	
	return ch;
}
