#include <string.h>
#include "SWM330.h"


char TX_Buffer[] = "Hello From Synwit\r\n";

#define BUF_SIZE  32
char RX_Buffer[BUF_SIZE] = {0};

void SerialInit(void);

int main(void)
{	
	SPI_InitStructure SPI_initStruct;
	DMA_InitStructure DMA_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	GPIO_Init(GPIOA, PIN13, 1, 0, 0, 0);
#define SPI0_CS_Low()	GPIO_ClrBit(GPIOA, PIN13)
#define SPI0_CS_High()	GPIO_SetBit(GPIOA, PIN13)
	SPI0_CS_High();
	
	PORT_Init(PORTA, PIN12, FUNMUX0_SPI0_SCLK, 0);
	PORT_Init(PORTA, PIN14, FUNMUX0_SPI0_MOSI, 0);	// connect MOSI to MISO, receive data sent by self
	PORT_Init(PORTA, PIN15, FUNMUX1_SPI0_MISO, 1);
	
	SPI_initStruct.clkDiv = SPI_CLKDIV_8;
	SPI_initStruct.FrameFormat = SPI_FORMAT_SPI;
	SPI_initStruct.SampleEdge = SPI_SECOND_EDGE;
	SPI_initStruct.IdleLevel = SPI_HIGH_LEVEL;
	SPI_initStruct.WordSize = 8;
	SPI_initStruct.Master = 1;
	SPI_initStruct.RXThreshold = 0;
	SPI_initStruct.RXThresholdIEn = 0;
	SPI_initStruct.TXThreshold = 0;
	SPI_initStruct.TXThresholdIEn = 0;
	SPI_initStruct.TXCompleteIEn = 0;
	SPI_Init(SPI0, &SPI_initStruct);
	SPI_Open(SPI0);
	
	
	// SPI0 RX DMA
	SPI0->CTRL |= (1 << SPI_CTRL_DMARXEN_Pos);
	
	DMA_initStruct.Mode = DMA_MODE_SINGLE;
	DMA_initStruct.Unit = DMA_UNIT_BYTE;
	DMA_initStruct.Count = BUF_SIZE;
	DMA_initStruct.MemoryAddr = (uint32_t)RX_Buffer;
	DMA_initStruct.MemoryAddrInc = 1;
	DMA_initStruct.PeripheralAddr = (uint32_t)&SPI0->DATA;
	DMA_initStruct.PeripheralAddrInc = 0;
	DMA_initStruct.Handshake = DMA_CH1_SPI0RX;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = DMA_IT_DONE;
	DMA_CH_Init(DMA_CH1, &DMA_initStruct);
	DMA_CH_Open(DMA_CH1);
	
	
	// SPI0 TX DMA
	SPI0->CTRL |= (1 << SPI_CTRL_DMATXEN_Pos);
	
	DMA_initStruct.Mode = DMA_MODE_SINGLE;
	DMA_initStruct.Unit = DMA_UNIT_BYTE;
	DMA_initStruct.Count = strlen(TX_Buffer);
	DMA_initStruct.MemoryAddr = (uint32_t)TX_Buffer;
	DMA_initStruct.MemoryAddrInc = 1;
	DMA_initStruct.PeripheralAddr = (uint32_t)&SPI0->DATA;
	DMA_initStruct.PeripheralAddrInc = 0;
	DMA_initStruct.Handshake = DMA_CH0_SPI0TX;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = DMA_IT_DONE;
	DMA_CH_Init(DMA_CH0, &DMA_initStruct);
	DMA_CH_Open(DMA_CH0);
	
	while(1==1)
	{
	}
}


void DMA_Handler(void)
{
	uint32_t i;
	
	if(DMA_CH_INTStat(DMA_CH0, DMA_IT_DONE))
	{
		DMA_CH_INTClr(DMA_CH0, DMA_IT_DONE);
		
		for(i = 0; i < SystemCoreClock/4; i++)  __NOP();
		
		DMA_CH_Open(DMA_CH0);	// restart, transfer agian
	}
	else if(DMA_CH_INTStat(DMA_CH1, DMA_IT_DONE))
	{
		DMA_CH_INTClr(DMA_CH1, DMA_IT_DONE);
		
		for(i = 0; i < BUF_SIZE; i++)  printf("%c", RX_Buffer[i]);
		
		memset(RX_Buffer, 0x00, sizeof(RX_Buffer));
		DMA_CH_Open(DMA_CH1);	// restart, transfer agian
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
