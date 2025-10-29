#include "SWM330.h"

uint16_t sin_data[100] = {
    2048,  2176,  2304,  2431,  2557,  2680,  2801,  2919,  3034,  3144,
    3251,  3352,  3449,  3540,  3625,  3704,  3776,  3841,  3900,  3951,
    3994,  4030,  4058,  4078,  4090,  4095,  4090,  4078,  4058,  4030,
    3994,  3951,  3900,  3841,  3776,  3704,  3625,  3540,  3449,  3352,
    3251,  3144,  3034,  2919,  2801,  2680,  2557,  2431,  2304,  2176,
    2047,  1919,  1791,  1664,  1538,  1415,  1294,  1176,  1061,   951,
     844,   743,   646,   555,   470,   391,   319,   254,   195,   144,
     101,    65,    37,    17,     5,     1,     5,    17,    37,    65,
     101,   144,   195,   254,   319,   391,   470,   555,   646,   743,
     844,   951,  1061,  1176,  1294,  1415,  1538,  1664,  1791,  1919,
};

void SerialInit(void);

int main(void)
{
	DMA_InitStructure DMA_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	PORT_Init(PORTC, PIN4, PORTC_PIN4_DAC_OUT, 0);
	
	DAC_Init(DAC0, DAC_FORMAT_LSB12B);
	DAC_Open(DAC0);
	
	DMA_initStruct.Mode = DMA_MODE_CIRCLE;
	DMA_initStruct.Unit = DMA_UNIT_HALFWORD;
	DMA_initStruct.Count = sizeof(sin_data)/sizeof(sin_data[0]);
	DMA_initStruct.MemoryAddr = (uint32_t)sin_data;
	DMA_initStruct.MemoryAddrInc = 1;
	DMA_initStruct.PeripheralAddr = (uint32_t)&DAC0->DHR;
	DMA_initStruct.PeripheralAddrInc = 0;
	DMA_initStruct.Handshake = DMA_EXMRD_BTIMR0;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = 0;
	DMA_CH_Init(DMA_CH0, &DMA_initStruct);
	DMA_CH_Open(DMA_CH0);
	
	TIMR_Init(BTIMR0, TIMR_MODE_TIMER, CyclesPerUs, 1000, 0);	// each time TIMR's counter overflows, DMA transfers a data to DAC->DHR register
	TIMR_Start(BTIMR0);
   	
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
