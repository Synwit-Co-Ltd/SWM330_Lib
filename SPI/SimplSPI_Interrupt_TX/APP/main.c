#include "SWM330.h"


/* SPI send routine, using TX register empty interrupt and TX frame done interrupt, can use the logic analyzer to view the send process */


uint16_t *TXBuff = 0; 
uint16_t TXCount = 0;
uint16_t TXIndex = 0;

void SerialInit(void);
void SPI_Master_Send(uint16_t buff[], uint16_t cnt);

int main(void)
{
	SPI_InitStructure SPI_initStruct;
	uint16_t txbuff[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
	
	SystemInit();
	
	SerialInit();	
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);		// debug indication signal
	
	GPIO_Init(GPIOA, PIN13, 1, 0, 0, 0);
#define SPI0_CS_Low()	GPIO_ClrBit(GPIOA, PIN13)
#define SPI0_CS_High()	GPIO_SetBit(GPIOA, PIN13)
	SPI0_CS_High();
	
	PORT_Init(PORTA, PIN12, FUNMUX0_SPI0_SCLK, 0);
	PORT_Init(PORTA, PIN14, FUNMUX0_SPI0_MOSI, 0);
	PORT_Init(PORTA, PIN15, FUNMUX1_SPI0_MISO, 1);
	
	SPI_initStruct.clkDiv = SPI_CLKDIV_128;
	SPI_initStruct.FrameFormat = SPI_FORMAT_SPI;
	SPI_initStruct.SampleEdge = SPI_SECOND_EDGE;
	SPI_initStruct.IdleLevel = SPI_HIGH_LEVEL;
	SPI_initStruct.WordSize = 8;
	SPI_initStruct.Master = 1;
	SPI_initStruct.RXThreshold = 0;
	SPI_initStruct.RXThresholdIEn = 0;
	SPI_initStruct.TXThreshold = 0;
	SPI_initStruct.TXThresholdIEn = 0;
	SPI_initStruct.TXCompleteIEn = 1;
	SPI_Init(SPI0, &SPI_initStruct);
	
	SPI_Open(SPI0);
	
	while(1==1)
	{
		SPI_Master_Send(txbuff, 16);
		
		for(int i = 0; i < SystemCoreClock/100; i++) __NOP();
	}
}


void SPI_Master_Send(uint16_t buff[], uint16_t cnt)
{	
	TXBuff = buff;
	TXCount = cnt;
	TXIndex = 0;
	
	SPI0_CS_Low();
	
	SPI_INTEn(SPI0, SPI_IT_TX_EMPTY);
	SPI_INTEn(SPI0, SPI_IT_TX_DONE);
}

void SPI0_Handler(void)
{		
	if(SPI_INTStat(SPI0, SPI_IT_TX_DONE))
	{
		SPI_INTClr(SPI0, SPI_IT_TX_DONE);
		
		if(TXIndex == TXCount)	// data to be sent is all filled in TX FIFO
		{
			SPI0_CS_High();
			SPI_INTDis(SPI0, SPI_IT_TX_DONE);
		}
	}
	
	if(SPI_INTStat(SPI0, SPI_IT_TX_EMPTY))
	{
		while(SPI_IsTXFull(SPI0) == 0)
		{
			if(TXIndex < TXCount)
			{
				SPI_Write(SPI0, TXBuff[TXIndex++]);
			}
			else
			{
				break;
			}
		}
		
		if(TXIndex == TXCount)
		{
			SPI_INTDis(SPI0, SPI_IT_TX_EMPTY);
		}
		
		SPI0->IF = SPI_IF_TFE_Msk;	// clear interrupt flag, must clear after filling data to TX FIFO
	}
	
	GPIO_InvBit(GPIOA, PIN3);
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
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThresholdIEn = 0;
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
