#include "SWM330.h"


int main(void)
{	
	PWM_InitStructure  PWM_initStruct;
	
	SystemInit();
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);		// debug indication pin
	
	PORT_Init(PORTA, PIN12, FUNMUX0_PWM0A_OUT,  0);
	PORT_Init(PORTA, PIN13, FUNMUX1_PWM0AN_OUT, 0);
	PORT_Init(PORTA, PIN14, FUNMUX0_PWM0B_OUT,  0);
	PORT_Init(PORTA, PIN15, FUNMUX1_PWM0BN_OUT, 0);
	
	PORT_Init(PORTC, PIN12, FUNMUX0_PWM1A_OUT,  0);
	PORT_Init(PORTC, PIN13, FUNMUX1_PWM1AN_OUT, 0);
	PORT_Init(PORTC, PIN14, FUNMUX0_PWM1B_OUT,  0);
	PORT_Init(PORTC, PIN15, FUNMUX1_PWM1BN_OUT, 0);
	
	PWM_initStruct.Mode = PWM_CENTER_ALIGNED;
	PWM_initStruct.Clkdiv = 4;					// F_PWM = 20M/4 = 5M
	PWM_initStruct.Period = 10000;				// 5M/10000 = 500Hz, 250Hz for center alignment mode
	PWM_initStruct.HdutyA =  2500;				// 2500/10000 = 25%
	PWM_initStruct.DeadzoneA = 50;				// 50/5M = 10us
	PWM_initStruct.IdleLevelA = 0;
	PWM_initStruct.IdleLevelAN= 0;
	PWM_initStruct.OutputInvA = 0;
	PWM_initStruct.OutputInvAN= 0;
	PWM_initStruct.HdutyB =  7500;				// 7500/10000 = 75%
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
	PWM_Init(PWM1, &PWM_initStruct);
	
	PWM_Start(PWM0_MSK|PWM1_MSK);
	
	
	/* Brake function using demo */
#if 0
	PORT_Init(PORTA, PIN8, FUNMUX0_PWM_BRK0, 1);
	PORTA->PULLU |= (1 << PIN8);
	PORT_Init(PORTA, PIN9, FUNMUX1_PWM_BRK1, 1);
	PORTA->PULLU |= (1 << PIN9);
	
	PWM_BrkInPolarity(PWM_BRK0 | PWM_BRK1 | PWM_BRK2, 0);			// brake when PWM_BRK0, PWM_BRK1 and PWM_BRK2 is low level
	
	PWM_BrkConfig(PWM0, PWM_CH_A, PWM_BRK0 | PWM_BRK1, 0, 1, 1, 0);	// PWM0 channel A is braked by brake inputs PWM_BRK0 and PWM_BRK1
	PWM_BrkConfig(PWM0, PWM_CH_B, PWM_BRK0 | PWM_BRK1, 0, 1, 1, 0);	// PWM0 channel B is braked by brake inputs PWM_BRK0 and PWM_BRK1
	
	/* Brake interrupt using demo */
#if 1
	PWM_BrkIntEn(PWM_BRKIT_BRK0);
	NVIC_EnableIRQ(PWMBRK_IRQn);
#endif
#endif

	/* Software Brake using demo */
#if 0
	PWM_BrkConfig(PWM0, PWM_CH_A, 0, 0, 1, 1, 0);	// PWM0 channel A cannot be braked by brake input signal, only can brake by software
	PWM_BrkConfig(PWM0, PWM_CH_B, 0, 0, 1, 1, 0);	// PWM0 channel B cannot be braked by brake input signal, only can brake by software
	
	while(1)
	{
		GPIO_SetBit(GPIOA, PIN5);
		PWMG->SWBRK |= (PWMG_SWBRK_PWM0A_Msk |		// software brake
						PWMG_SWBRK_PWM0B_Msk);
		for(int i = 0; i < 300000; i++) __NOP();
		
		GPIO_ClrBit(GPIOA, PIN5);
		PWMG->SWBRK &=~(PWMG_SWBRK_PWM0A_Msk |		// software brake exit, restore normal output
						PWMG_SWBRK_PWM0B_Msk);
		for(int i = 0; i < 900000; i++) __NOP();
	}
#endif

	while(1==1)
	{
	}
}


void PWMBRK_Handler(void)
{
	if(PWM_BrkIntStat(PWM_BRKIT_BRK0))
	{
		PWM_BrkIntDis(PWM_BRKIT_BRK0);	// The brake interrupt must be turned off, otherwise it will trigger the interrupt continuously during braking
		
		GPIO_InvBit(GPIOA, PIN5);
	}
}
