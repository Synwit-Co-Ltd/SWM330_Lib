#include "SWM330.h"


int main(void)
{
	PWM_InitStructure  PWM_initStruct;
	
	SystemInit();
	
	GPIO_Init(GPIOA, PIN5, 1, 0, 0, 0);			// debug indication pin
	
	PORT_Init(PORTA, PIN12, FUNMUX0_PWM0A_OUT,  0);
	PORT_Init(PORTA, PIN13, FUNMUX1_PWM0AN_OUT, 0);
	PORT_Init(PORTA, PIN14, FUNMUX0_PWM0B_OUT,  0);
	PORT_Init(PORTA, PIN15, FUNMUX1_PWM0BN_OUT, 0);
	
	PORT_Init(PORTC, PIN12, FUNMUX0_PWM1A_OUT,  0);
	PORT_Init(PORTC, PIN13, FUNMUX1_PWM1AN_OUT, 0);
	PORT_Init(PORTC, PIN14, FUNMUX0_PWM1B_OUT,  0);
	PORT_Init(PORTC, PIN15, FUNMUX1_PWM1BN_OUT, 0);
	
	PWM_initStruct.Mode = PWM_ASYM_CENTER_ALIGNED;
	PWM_initStruct.Clkdiv = 4;					// F_PWM = 20M/4 = 5M
	PWM_initStruct.Period = 10000;				// 5M/(10000 + 10000)= 250Hz
	PWM_initStruct.HdutyA =  2500;				// (2500 + 2500)/(10000 + 10000) = 25%
	PWM_initStruct.HdutyA2 = 2500;
	PWM_initStruct.DeadzoneA = 50;				// 50/5M = 10us
	PWM_initStruct.IdleLevelA = 0;
	PWM_initStruct.IdleLevelAN= 0;
	PWM_initStruct.OutputInvA = 0;
	PWM_initStruct.OutputInvAN= 0;
	PWM_initStruct.HdutyB =  7500;				// (7500 + 7500)/(10000 + 10000) = 75%
	PWM_initStruct.HdutyB2 = 7500;
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
	
	while(1==1)
	{
		PWMG->RELOADEN = 0x00;		// working register reload disable and ensure that all registers are updated at same time.
		
		/* Use asymmetric center alignment mode, keep duty cycle unchanged and realize phase shift function */
		if(PWM1->CMPA2 == 0)
		{
			PWM1->CMPA = 2500;
			PWM1->CMPA2 = 2500;
			PWM1->CMPB = 7500;
			PWM1->CMPB2 = 7500;
		}
		else
		{
			PWM1->CMPA += 250;		// increase high level duration of the first half cycle
			PWM1->CMPA2 -= 250;		// decrease high level duration of the second half cycle, the high level shifts to the right
			PWM1->CMPB += 750;
			PWM1->CMPB2 -= 750;
		}
		
		if(PWM0->CMPA == 0)
		{
			PWM0->CMPA = 2500;
			PWM0->CMPA2 = 2500;
			PWM0->CMPB = 7500;
			PWM0->CMPB2 = 7500;
		}
		else
		{
			PWM0->CMPA -= 250;		// decrease high level duration of the first half cycle
			PWM0->CMPA2 += 250;		// increase high level duration of the second half cycle, the high level shifts to the left
			PWM0->CMPB -= 750;
			PWM0->CMPB2 += 750;
		}
		
		PWMG->RELOADEN = 0x3F;
		
		for(int i = 0; i < SystemCoreClock / 32; i++) __NOP();
	}
}

