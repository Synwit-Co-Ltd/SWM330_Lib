#include "SWM330.h"

void SerialInit(void);

int main(void)
{
	ADC_InitStructure ADC_initStruct;
	ADC_SEQ_InitStructure ADC_SEQ_initStruct;
	
	SystemInit();
	
	SerialInit();

	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);
	
	PORT_Init(PORTE, PIN13, PORTE_PIN13_ADC0_CH0, 0);
	PORT_Init(PORTD, PIN10, PORTD_PIN10_ADC0_CH1, 0);
	PORT_Init(PORTC, PIN9,  PORTC_PIN9_ADC0_CH2,  0);
	PORT_Init(PORTA, PIN7,  PORTA_PIN7_ADC0_CH3,  0);
	PORT_Init(PORTC, PIN8,  PORTC_PIN8_ADC0_CH4,  0);
	PORT_Init(PORTC, PIN7,  PORTC_PIN7_ADC0_CH5,  0);
	PORT_Init(PORTC, PIN6,  PORTC_PIN6_ADC0_CH6,  0);
	PORT_Init(PORTC, PIN3,  PORTC_PIN3_ADC0_CH7,  0);
	PORT_Init(PORTC, PIN2,  PORTC_PIN2_ADC0_CH8,  0);
	PORT_Init(PORTC, PIN1,  PORTC_PIN1_ADC0_CH9,  0);
	PORT_Init(PORTC, PIN0,  PORTC_PIN0_ADC0_CH10, 0);
	PORT_Init(PORTA, PIN8,  PORTA_PIN8_ADC0_CH11, 0);
	PORT_Init(PORTB, PIN1,  PORTB_PIN1_ADC0_CH12, 0);
	PORT_Init(PORTB, PIN0,  PORTB_PIN0_ADC0_CH13, 0);
	PORT_Init(PORTE, PIN15, PORTE_PIN15_ADC0_CH14,0);
	
	ADC_initStruct.clk_src = ADC_CLKSRC_HRC_DIV8;
	ADC_initStruct.samplAvg = ADC_AVG_SAMPLE1;
	ADC_initStruct.EOC_IEn = ADC_SEQ0;	
	ADC_initStruct.HalfIEn = 0;
	ADC_Init(ADC0, &ADC_initStruct);
	
	ADC_SEQ_initStruct.channels = ADC_CH5;
	ADC_SEQ_initStruct.trig_src = ADC_TRIGGER_EXTRIG1;
	ADC_SEQ_initStruct.conv_cnt = 1;
	ADC_SEQ_initStruct.samp_tim = ADC_SAMPLE_1CLOCK;
	ADC_SEQ_Init(ADC0, ADC_SEQ0, &ADC_SEQ_initStruct);
	
	ADC_Open(ADC0);
	ADC_Calibrate(ADC0);
	
	PORT_Init(PORTA, PIN8, FUNMUX0_ADC0_TRIG0, 1);	// Whenever a rising edge appears on this pin, ADC starts a conversion
	
	while(1==1)
	{
	}
}

void ADC0_Handler(void)
{
	uint32_t chn;
	
	GPIO_InvBit(GPIOA, PIN5);
	
	ADC0->IF = (1 << ADC_IF_SEQ0EOC_Pos);
	
	printf("%4d,", ADC_Read(ADC0, ADC_SEQ0, &chn));
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
