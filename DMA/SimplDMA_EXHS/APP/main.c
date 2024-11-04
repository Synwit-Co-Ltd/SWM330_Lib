#include "SWM330.h"
#include <string.h>


char str_hi[] = "Hi from Synwit\n";


void SerialInit(void);

int main(void)
{	
	DMA_InitStructure DMA_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	DMA_initStruct.Mode = DMA_MODE_SINGLE;
	DMA_initStruct.Unit = DMA_UNIT_BYTE;
	DMA_initStruct.Count = strlen(str_hi);
	DMA_initStruct.MemoryAddr = (uint32_t)str_hi;
	DMA_initStruct.MemoryAddrInc = 1;
	DMA_initStruct.PeripheralAddr = (uint32_t)&UART0->DATA;
	DMA_initStruct.PeripheralAddrInc = 0;
	DMA_initStruct.Handshake = DMA_EXMRD_BTIMR0;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = 0;
	DMA_CH_Init(DMA_CH0, &DMA_initStruct);
	DMA_CH_Open(DMA_CH0);
	
	TIMR_Init(BTIMR0, TIMR_MODE_TIMER, CyclesPerUs, 500000, 0);	// each time TIMR's counter overflows, DMA transfer a data to UART0->DATA register
	TIMR_Start(BTIMR0);
	
	while(1==1)
	{
	}
}


void DMA_Handler(void)
{
	if(DMA_CH_INTStat(DMA_CH0, DMA_IT_DONE))
	{
		DMA_CH_INTClr(DMA_CH0, DMA_IT_DONE);
		
		DMA->CH[0].MUX &= ~DMA_MUX_EXTHSEN_Msk;
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
