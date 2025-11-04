#include "SWM330.h"


int main(void)
{
	PWM_InitStructure  PWM_initStruct;
	
	SystemInit();
	
	GPIO_INIT(GPIOA, PIN8,  GPIO_OUTPUT);
	GPIO_INIT(GPIOA, PIN9,  GPIO_OUTPUT);
	GPIO_INIT(GPIOA, PIN10, GPIO_OUTPUT);
	
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
	PWM_initStruct.UpOvfIE    = 1;
	PWM_initStruct.DownOvfIE  = 1;
	PWM_initStruct.UpCmpAIE   = 1;
	PWM_initStruct.DownCmpAIE = 0;
	PWM_initStruct.UpCmpBIE   = 0;
	PWM_initStruct.DownCmpBIE = 0;
	PWM_Init(PWM0, &PWM_initStruct);
	
 	PWM_Start(PWM0_MSK);
	
	while(1==1)
	{
	}
}

void PWM0_Handler(void)
{
	if(PWM_IntStat(PWM0, PWM_IT_OVF_UP))
	{
		PWM_IntClr(PWM0, PWM_IT_OVF_UP);
		
		GPIO_InvBit(GPIOA, PIN8);
	}
	else if(PWM_IntStat(PWM0, PWM_IT_CMPA_UP))
	{
		PWM_IntClr(PWM0, PWM_IT_CMPA_UP);
		
		GPIO_InvBit(GPIOA, PIN9);
	}
	else if(PWM_IntStat(PWM0, PWM_IT_CMPB_UP))
	{
		PWM_IntClr(PWM0, PWM_IT_CMPB_UP);
		
		GPIO_InvBit(GPIOA, PIN10);
	}
}
