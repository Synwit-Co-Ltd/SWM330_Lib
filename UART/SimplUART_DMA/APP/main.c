#include <string.h>
#include "SWM330.h"


const char TX_String[8][32] = {
	"TestString\r\n",
	"TestString1\r\n",
	"TestString12\r\n",
	"TestString123\r\n",
	"TestString1234\r\n",
	"TestString12345\r\n",
	"TestString123456\r\n",
	"TestString1234567\r\n",
};

#define RX_LEN	256					// It is recommended to use an integer power of 2 to convert the remainder operation into an and operation to speed up the operation
int  RX_Buffer[RX_LEN] = { 0 };		// When using DMA to move the data received by UART to the memory, the unit must be words.
									// Otherwise, the data may temporarily exist in the DMA internal and cannot be read out by the program.
char TX_Buffer[RX_LEN] = { 0 };


volatile uint32_t RX_Start = 0;


void SerialInit(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN8, FUNMUX0_UART1_TXD, 0);
	PORT_Init(PORTA, PIN9, FUNMUX1_UART1_RXD, 1);	// Connect to PA6, receive the data sent by UART0, and send it as-is
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThreshold = 3;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;
	UART_initStruct.TimeoutIEn = 1;
 	UART_Init(UART1, &UART_initStruct);
	UART_Open(UART1);
	
	DMA_InitStructure DMA_initStruct;
	
	DMA_initStruct.Mode = DMA_MODE_CIRCLE;
	DMA_initStruct.Unit = DMA_UNIT_WORD;
	DMA_initStruct.Count = RX_LEN;
	DMA_initStruct.MemoryAddr = (uint32_t)RX_Buffer;
	DMA_initStruct.MemoryAddrInc = 1;
	DMA_initStruct.PeripheralAddr = (uint32_t)&UART1->DATA;
	DMA_initStruct.PeripheralAddrInc = 0;
	DMA_initStruct.Handshake = DMA_CH0_UART1RX;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = 0;
	DMA_CH_Init(DMA_CH0, &DMA_initStruct);
	DMA_CH_Open(DMA_CH0);
	
	DMA_initStruct.Mode = DMA_MODE_SINGLE;
	DMA_initStruct.Unit = DMA_UNIT_BYTE;
	DMA_initStruct.Count = strlen(TX_Buffer);
	DMA_initStruct.MemoryAddr = (uint32_t)TX_Buffer;
	DMA_initStruct.MemoryAddrInc = 1;
	DMA_initStruct.PeripheralAddr = (uint32_t)&UART1->DATA;
	DMA_initStruct.PeripheralAddrInc = 0;
	DMA_initStruct.Handshake = DMA_CH1_UART1TX;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = 0;
	DMA_CH_Init(DMA_CH1, &DMA_initStruct);
   	
	while(1==1)
	{
		for(int i = 0; i < 8; i++)
		{
			for(int j = 0; j < strlen(TX_String[i]); j++)
			{
				UART_WriteByte(UART0, TX_String[i][j]);
				
				while(UART_IsTXBusy(UART0));
			}
			
			for(int j = 0; j < SystemCoreClock/8; j++) __NOP();
		}
	}
}


void UART1_Handler(void)
{
	if(UART_INTStat(UART1, UART_IT_RX_TOUT))
	{
		UART_INTClr(UART1, UART_IT_RX_TOUT);
		
		int str_len = (RX_LEN + RX_LEN - DMA_CH_GetRemaining(DMA_CH0) - RX_Start) % RX_LEN;
		for(int i = 0; i < str_len; i++)
			TX_Buffer[i] = RX_Buffer[(RX_Start + i) % RX_LEN];
		
		RX_Start += str_len;
		
		DMA_CH_SetAddrAndCount(DMA_CH1, (uint32_t)TX_Buffer, str_len);
		DMA_CH_Open(DMA_CH1);
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
