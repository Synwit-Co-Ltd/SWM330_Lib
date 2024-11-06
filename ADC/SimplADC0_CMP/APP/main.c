#include "SWM330.h"

#define ADC_SIZE  16	// ADC can convert a maximum of 256 times consecutively
uint16_t ADC_Result[ADC_SIZE] = {0};
volatile int ADC_ValueFound = 0;

#define ADC_LIMIT_MIN	400
#define ADC_LIMIT_MAX	3600


void SerialInit(void);

int main(void)
{
	DMA_InitStructure DMA_initStruct;
	ADC_InitStructure ADC_initStruct;
	ADC_SEQ_InitStructure ADC_SEQ_initStruct;
	ADC_CMP_InitStructure ADC_CMP_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	PORT_Init(PORTD, PIN1,  PORTD_PIN1_ADC0_CH0,  0);
	PORT_Init(PORTD, PIN0,  PORTD_PIN0_ADC0_CH1,  0);
	PORT_Init(PORTC, PIN13, PORTC_PIN13_ADC0_CH2, 0);
	PORT_Init(PORTC, PIN12, PORTC_PIN12_ADC0_CH3, 0);
	PORT_Init(PORTC, PIN11, PORTC_PIN11_ADC0_CH4, 0);
	PORT_Init(PORTC, PIN10, PORTC_PIN10_ADC0_CH5, 0);
	PORT_Init(PORTA, PIN11, PORTA_PIN11_ADC0_CH6, 0);
	PORT_Init(PORTA, PIN10, PORTA_PIN10_ADC0_CH7, 0);
	PORT_Init(PORTC, PIN9,  PORTC_PIN9_ADC0_CH8,  0);
	PORT_Init(PORTC, PIN8,  PORTC_PIN8_ADC0_CH9,  0);
	PORT_Init(PORTC, PIN7,  PORTC_PIN7_ADC0_CH10, 0);
	
	ADC_initStruct.clk_src = ADC_CLKSRC_HRC_DIV8;
	ADC_initStruct.samplAvg = ADC_AVG_SAMPLE1;
	ADC_initStruct.EOC_IEn = 0;
	ADC_initStruct.HalfIEn = 0;
	ADC_Init(ADC0, &ADC_initStruct);
	
	ADC_SEQ_initStruct.channels = ADC_CH7;
	ADC_SEQ_initStruct.trig_src = ADC_TRIGGER_BTIMER2;
	ADC_SEQ_initStruct.conv_cnt = ADC_SIZE;
	ADC_SEQ_initStruct.samp_tim = ADC_SAMPLE_1CLOCK;
	ADC_SEQ_Init(ADC0, ADC_SEQ0, &ADC_SEQ_initStruct);
	
	ADC_CMP_initStruct.LowerLimit = ADC_LIMIT_MIN;
	ADC_CMP_initStruct.LowerLimitIEn = 0;
	ADC_CMP_initStruct.UpperLimit = ADC_LIMIT_MAX;
	ADC_CMP_initStruct.UpperLimitIEn = 1;
	ADC_CMP_Init(ADC0, ADC_SEQ0, &ADC_CMP_initStruct);
	
	ADC_Open(ADC0);
	ADC_Calibrate(ADC0);
	
	ADC0->CR |= (ADC_SEQ0 << ADC_CR_DMAEN_Pos);
	
	
	DMA_initStruct.Mode = DMA_MODE_CIRCLE;
	DMA_initStruct.Unit = DMA_UNIT_HALFWORD;
	DMA_initStruct.Count = ADC_SIZE;
	DMA_initStruct.MemoryAddr = (uint32_t)ADC_Result;
	DMA_initStruct.MemoryAddrInc = 1;
	DMA_initStruct.PeripheralAddr = (uint32_t)&ADC0->SEQ[0].DR;
	DMA_initStruct.PeripheralAddrInc = 0;
	DMA_initStruct.Handshake = DMA_CH0_ADC0SEQ0;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = 0;
	DMA_CH_Init(DMA_CH0, &DMA_initStruct);
	
	DMA_CH_Open(DMA_CH0);
	
	TIMR_Init(BTIMR2, TIMR_MODE_TIMER, CyclesPerUs, 1000, 0);
	TIMR_Start(BTIMR2);
	
	while(1==1)
	{
		if(ADC_ValueFound)
		{
			ADC_ValueFound = 0;
			
			for(int i = 0; i < ADC_SIZE; i++)
			{
				int adcv = ADC_Result[i] & ADC_DR_VALUE_Msk;
				if(adcv > ADC_LIMIT_MAX)
				{
					printf("ADC Value: %d\n", adcv);
					
					break;
				}
			}
		}
	}
}


void ADC0_Handler(void)
{
	if(ADC_INTStat(ADC0, ADC_SEQ0, ADC_IT_CMP_MAX))
	{
		ADC_INTClr(ADC0, ADC_SEQ0, ADC_IT_CMP_MAX);
		
		ADC_ValueFound = 1;
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
