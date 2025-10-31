#include "SWM330.h"

#include "ugui.h"
#include "NT35510.h"


uint16_t Buffer[NT35510_HPIX * 10] = {0};


UG_GUI gui;

void SerialInit(void);
void MPULCDInit(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	MPULCDInit();
	
	NT35510_Init();
	
	NT35510_Clear(C_RED);
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);
	
	UG_Init(&gui,(void(*)(UG_S16,UG_S16,UG_COLOR))NT35510_DrawPoint, 480, 800);
	
	UG_DrawLine(0, 68, 480, 68, C_GREEN);
	UG_DrawLine(0, 136, 480, 136, C_GREEN);
	UG_DrawLine(0, 204, 480, 204, C_GREEN);
	
 	UG_FillFrame(120, 69, 360, 135, C_BLACK);
 	
  	UG_FontSelect(&FONT_12X20);
	UG_PutString(120, 80, "Hi from Synwit");
	
	/* MPU DMA write and interrupt using demo */
	for(int i = 0; i < sizeof(Buffer) / 2; i++)
		Buffer[i] = 0x5555;
	
	while(1==1)
	{
		NT35510_DMAWrite((uint32_t *)Buffer, 400, sizeof(Buffer) / 2 / NT35510_HPIX);
		
		for(int i = 0; i < SystemCoreClock / 100; i++) __NOP();
	}
}


void LCD_Handler(void)
{
	LCD_INTDis(LCD, LCD_IT_DONE);
	
	GPIO_InvBit(GPIOA, PIN5);
}


void MPULCDInit(void)
{
	MPULCD_InitStructure MPULCD_initStruct;
	
	GPIO_Init(GPIOE, PIN15, 1, 0, 0, 0);	// LCD hardware reset
	GPIO_ClrBit(GPIOE, PIN15);
	for(int i = 0; i < CyclesPerUs*1000; i++) __NOP();
	GPIO_SetBit(GPIOE, PIN15);
	
	PORT_Init(PORTB, PIN7,  PORTB_PIN7_LCD_CS,  0);
	PORT_Init(PORTB, PIN6,  PORTB_PIN6_LCD_WR,  0);
	PORT_Init(PORTB, PIN8,  PORTB_PIN8_LCD_RS,  0);
	PORT_Init(PORTD, PIN15, PORTD_PIN15_LCD_RD, 0);
	PORT_Init(PORTB, PIN12, PORTB_PIN12_LCD_B3, 1);		// MPU_D0
	PORT_Init(PORTB, PIN15, PORTB_PIN15_LCD_B4, 1);
	PORT_Init(PORTA, PIN0,  PORTA_PIN0_LCD_B5,  1);
	PORT_Init(PORTA, PIN1,  PORTA_PIN1_LCD_B6,  1);
	PORT_Init(PORTA, PIN2,  PORTA_PIN2_LCD_B7,  1);
	PORT_Init(PORTA, PIN9,  PORTA_PIN9_LCD_G2,  1);
	PORT_Init(PORTA, PIN10, PORTA_PIN10_LCD_G3, 1);
	PORT_Init(PORTA, PIN11, PORTA_PIN11_LCD_G4, 1);
	PORT_Init(PORTC, PIN10, PORTC_PIN10_LCD_G5, 1);
	PORT_Init(PORTC, PIN11, PORTC_PIN11_LCD_G6, 1);
	PORT_Init(PORTC, PIN12, PORTC_PIN12_LCD_G7, 1);
	PORT_Init(PORTD, PIN3,  PORTD_PIN3_LCD_R3,  1);
	PORT_Init(PORTD, PIN4,  PORTD_PIN4_LCD_R4,  1);
	PORT_Init(PORTD, PIN5,  PORTD_PIN5_LCD_R5,  1);
	PORT_Init(PORTD, PIN6,  PORTD_PIN6_LCD_R6,  1);
	PORT_Init(PORTD, PIN7,  PORTD_PIN7_LCD_R7,  1);		// MPU_D15
	
	MPULCD_initStruct.BusWidth = MPULCD_BusWidth_16;
	MPULCD_initStruct.ByteOrder = MPULCD_ByteOrder_MSB;
	MPULCD_initStruct.RDHoldTime = 32;
	MPULCD_initStruct.WRHoldTime = 16;
	MPULCD_initStruct.CSFall_WRFall = 4;
	MPULCD_initStruct.WRRise_CSRise = 4;
	MPULCD_initStruct.RDCSRise_Fall = 32;
	MPULCD_initStruct.WRCSRise_Fall = 16;
	MPULCD_Init(LCD, &MPULCD_initStruct);
	
	LCD_INTClr(LCD, LCD_IT_DONE);
	LCD_INTEn(LCD, LCD_IT_DONE);
	NVIC_EnableIRQ(LCD_IRQn);
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
