#include "SWM330.h"

uint32_t ADC_Values[8] = {0};

void SerialInit(void);

int main(void)
{
	uint32_t chn;
	ADC_InitStructure ADC_initStruct;
	ADC_SEQ_InitStructure ADC_SEQ_initStruct;
	
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
	ADC_initStruct.EOC_IEn = ADC_SEQ1;	
	ADC_initStruct.HalfIEn = 0;
	ADC_Init(ADC0, &ADC_initStruct);
	
	ADC_SEQ_initStruct.channels = ADC_CH5;
	ADC_SEQ_initStruct.trig_src = ADC_TRIGGER_SW;
	ADC_SEQ_initStruct.conv_cnt = 1;
	ADC_SEQ_initStruct.samp_tim = ADC_SAMPLE_1CLOCK;
	ADC_SEQ_Init(ADC0, ADC_SEQ0, &ADC_SEQ_initStruct);
	
	ADC_SEQ_initStruct.channels = ADC_CH0 | ADC_CH6;
	ADC_SEQ_initStruct.trig_src = ADC_TRIGGER_BTIMER3;
	ADC_SEQ_Init(ADC0, ADC_SEQ1, &ADC_SEQ_initStruct);
	
	ADC_Open(ADC0);
	ADC_Calibrate(ADC0);
	
	TIMR_Init(BTIMR3, TIMR_MODE_TIMER, CyclesPerUs, 100000, 0);
	TIMR_Start(BTIMR3);
	
	while(1==1)
	{
		ADC_Start(ADC0, ADC_SEQ0);
		while((ADC0->SEQ[0].SR & ADC_SR_EOC_Msk) == 0);
		ADC_Values[5] = ADC_Read(ADC0, ADC_SEQ0, &chn);
		
		printf("CH0: %4d\tCH5: %4d\tCH6: %4d\n", ADC_Values[0], ADC_Values[5], ADC_Values[6]);
// 		printf("%4d,", ADC_Values[5]);
	}
}


void ADC0_Handler(void)
{
	uint32_t val, chn;
	
	if(ADC_INTStat(ADC0, ADC_SEQ1, ADC_IT_EOC))
	{
		ADC_INTClr(ADC0, ADC_SEQ1, ADC_IT_EOC);
		
		while((ADC0->SEQ[1].SR & ADC_SR_EMPTY_Msk) == 0)
		{
			val = ADC_Read(ADC0, ADC_SEQ1, &chn);
			switch(chn)
			{
			case ADC_CH0:  ADC_Values[0] = val;  break;
			case ADC_CH6:  ADC_Values[6] = val;  break;
			}
		}
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
