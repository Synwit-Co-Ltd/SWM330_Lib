#include "SWM330.h"

#define BUF_N	1024
uint16_t TX_Buffer[BUF_N];

volatile int TX_DoneIndex = 0;	// bit1：半段传输完成标志位
								// bit0：完成传输的是哪半段，0 前半段   1 后半段

void SerialInit(void);
void I2S_Master_Init(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	I2S_Master_Init();
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);
	
	while(1==1)
	{
	}
}


void I2S_Master_Init(void)
{
	I2S_InitStructure I2S_initStruct;
	DMA_InitStructure DMA_initStruct;
	
	PORT_Init(PORTC, PIN1, FUNMUX1_I2S0_WS,   0);
	PORT_Init(PORTC, PIN0, FUNMUX0_I2S0_SCLK, 0);
	PORT_Init(PORTC, PIN2, FUNMUX0_I2S0_DATA, 0);
	
	I2S_initStruct.Mode = I2S_MASTER_TX;
	I2S_initStruct.ClkFreq = 44100 * 2 * 16;	// 44.1K
	I2S_initStruct.ClkPolarity = I2S_CLK_IDLE_LOW;
	I2S_initStruct.FrameFormat = I2S_I2S_PHILIPS;
	I2S_initStruct.ChannelLen = I2S_CHNNLEN_16;
	I2S_initStruct.DataLen = I2S_DATALEN_16;
	I2S_initStruct.INTEn = 0;
	I2S_Init(I2S0, &I2S_initStruct);
	I2S_Open(I2S0);
	
	
	for(int i = 0; i < BUF_N; i++)
		TX_Buffer[i] = i;
	
	
	DMA_initStruct.Mode = DMA_MODE_CIRCLE;
	DMA_initStruct.Unit = DMA_UNIT_HALFWORD;
	DMA_initStruct.Count = BUF_N;
	DMA_initStruct.MemoryAddr = (uint32_t)TX_Buffer;
	DMA_initStruct.MemoryAddrInc = 1;
	DMA_initStruct.PeripheralAddr = (uint32_t)&I2S0->DR;
	DMA_initStruct.PeripheralAddrInc = 0;
	DMA_initStruct.Handshake = DMA_EXMRD_BTIMR0;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = DMA_IT_HALF | DMA_IT_DONE;
	DMA_CH_Init(DMA_CH1, &DMA_initStruct);
	DMA_CH_Open(DMA_CH1);
	
	// each time TIMR's counter overflows, DMA transfers a data to I2S0->DR register
	TIMR_Init(BTIMR0, TIMR_MODE_TIMER, 1, I2S0->CR3 * 2 * ((I2S_initStruct.ChannelLen + 1) * 16), 0);
	TIMR_Start(BTIMR0);
}


void DMA_Handler(void)
{
	/* Master TX */
	if(DMA_CH_INTStat(DMA_CH1, DMA_IT_HALF))
	{
		DMA_CH_INTClr(DMA_CH1, DMA_IT_HALF);
		
		TX_DoneIndex = (1 << 1) | 0;
		
		GPIO_InvBit(GPIOA, PIN5);
	}
	else if(DMA_CH_INTStat(DMA_CH1, DMA_IT_DONE))
	{
		DMA_CH_INTClr(DMA_CH1, DMA_IT_DONE);
		
		TX_DoneIndex = (1 << 1) | 1;
		
		GPIO_InvBit(GPIOA, PIN5);
	}
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN6, FUNMUX0_UART0_TXD, 0);
	PORT_Init(PORTA, PIN7, FUNMUX1_UART0_RXD, 1);
 	
 	UART_initStruct.Baudrate = 500000;
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
