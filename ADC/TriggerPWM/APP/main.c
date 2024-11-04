#include "SWM330.h"


void SerialInit(void);
void PWM0AInit(void);

int main(void)
{
	ADC_InitStructure ADC_initStruct;
	ADC_SEQ_InitStructure ADC_SEQ_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);
	
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
	ADC_initStruct.EOC_IEn = ADC_SEQ0;	
	ADC_initStruct.HalfIEn = 0;
	ADC_Init(ADC0, &ADC_initStruct);
	
	ADC_SEQ_initStruct.channels = ADC_CH5;
	ADC_SEQ_initStruct.trig_src = ADC_TRIGGER_PWM0;
	ADC_SEQ_initStruct.conv_cnt = 1;
	ADC_SEQ_initStruct.samp_tim = ADC_SAMPLE_1CLOCK;
	ADC_SEQ_Init(ADC0, ADC_SEQ0, &ADC_SEQ_initStruct);
	
	ADC_Open(ADC0);
	ADC_Calibrate(ADC0);
	
	PWM0AInit();
	
	while(1==1)
	{
	}
}

void ADC0_Handler(void)
{
	uint32_t chn;
	
	GPIO_InvBit(GPIOA, PIN5);
	
	ADC0->IF = (1 << ADC_IF_SEQ0EOC_Pos);
	
	printf("%d,", ADC_Read(ADC0, ADC_SEQ0, &chn));
}


void PWM0AInit(void)
{
	PWM_InitStructure PWM_initStruct;
	
	PORT_Init(PORTA, PIN12, FUNMUX0_PWM0A_OUT,  0);
	PORT_Init(PORTA, PIN13, FUNMUX1_PWM0AN_OUT, 0);
	PORT_Init(PORTA, PIN14, FUNMUX0_PWM0B_OUT,  0);
	PORT_Init(PORTA, PIN15, FUNMUX1_PWM0BN_OUT, 0);
	
	PWM_initStruct.Mode = PWM_CENTER_ALIGNED;
	PWM_initStruct.Clkdiv = 6;					// F_PWM = 30M/6 = 5M
	PWM_initStruct.Period = 10000;				// 5M/10000 = 500Hz, 250Hz for center alignment mode
	PWM_initStruct.HdutyA =  2500;				// 2500/10000 = 25%
	PWM_initStruct.DeadzoneA = 50;				// 50/5M = 10us
	PWM_initStruct.IdleLevelA = 0;
	PWM_initStruct.IdleLevelAN= 0;
	PWM_initStruct.OutputInvA = 0;
	PWM_initStruct.OutputInvAN= 0;
	PWM_initStruct.HdutyB =  7500;				// 5000/10000 = 50%
	PWM_initStruct.DeadzoneB = 50;				// 50/5M = 10us
	PWM_initStruct.IdleLevelB = 0;
	PWM_initStruct.IdleLevelBN= 0;
	PWM_initStruct.OutputInvB = 0;
	PWM_initStruct.OutputInvBN= 0;
	PWM_initStruct.UpOvfIE    = 0;
	PWM_initStruct.DownOvfIE  = 0;
	PWM_initStruct.UpCmpAIE   = 0;
	PWM_initStruct.DownCmpAIE = 0;
	PWM_initStruct.UpCmpBIE   = 0;
	PWM_initStruct.DownCmpBIE = 0;
	PWM_Init(PWM0, &PWM_initStruct);
	
	PWM_Start(PWM0_MSK);
	
	
	/* Mask */
	PWM_CmpTrigger(PWM0, 1500, PWM_DIR_UP, 40, PWM_TRG_1, 7);	// when counting up and counter == 1500, PWM will generate PWM_TRG_1 signal
	
	PWM_OutMask(PWM0, PWM_CH_A, PWM_EVT_1, 0, PWM_EVT_1, 1);	// PWM0A and PWM0AN output 0 and 1 respectively when detect PWM_EVT_1 event
	PWM_OutMask(PWM0, PWM_CH_B, PWM_EVT_1, 0, PWM_EVT_1, 1);	// PWM0B and PWM0BN output 0 and 1 respectively when detect PWM_EVT_1 event
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
