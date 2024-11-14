#include <string.h>

#include "SWM330.h"

#include "ugui.h"

#define LCD_HDOT	480		// horizontal dot number
#define LCD_VDOT	272		// vertical dot number
#define LCD_DIRH	1		// horizontal display?


UG_GUI gui;


uint16_t *LCD_Buffer = (uint16_t *)PSRAMM_BASE;

void _HW_DrawPoint(UG_S16 x, UG_S16 y, UG_COLOR c)
{	
#if LCD_DIRH
	LCD_Buffer[y * LCD_HDOT + x] = c;
#else
	LCD_Buffer[(LCD_VDOT - x) * LCD_HDOT + y] = c;
#endif
}


void SerialInit(void);
void MemoryInit(void);
void RGBLCDInit(void);

int main(void)
{
	uint32_t i, j;
	uint16_t color[3] = {C_GREEN, C_BLACK, C_BLUE};
	
	SystemInit();
	
	SerialInit();
	
	MemoryInit();
	
	RGBLCDInit();
	
	LCD_Start(LCD);
	
#if LCD_DIRH
	UG_Init(&gui,(void(*)(UG_S16,UG_S16,UG_COLOR))_HW_DrawPoint, LCD_HDOT, LCD_VDOT);
#else
	UG_Init(&gui,(void(*)(UG_S16,UG_S16,UG_COLOR))_HW_DrawPoint, LCD_VDOT, LCD_HDOT);
#endif
	
	UG_FillScreen(C_RED);
	
	UG_DrawLine(40, 50, 240, 50, C_GREEN);
	
	UG_DrawLine(40, 36, 40, 236, C_BLUE);
	
	UG_DrawCircle(200, 210, 50, C_GREEN);
	UG_DrawCircle(200, 210, 51, C_GREEN);
	UG_DrawCircle(200, 210, 52, C_GREEN);
	
	while(1==1)
	{		
		for(i = 0; i < 3; i++)
		{
			for(j = 0; j < SystemCoreClock/20; j++) __NOP();
			
			UG_FillFrame(60, 75, 200, 150, color[i]);
		}
	}
}


void RGBLCDInit(void)
{
	uint32_t i;
	LCD_InitStructure LCD_initStruct;
	
	GPIO_Init(GPIOA, PIN6, 1, 0, 0, 0);		// LCD backlight switch
	GPIO_SetBit(GPIOA, PIN6);
	GPIO_Init(GPIOC, PIN6, 1, 0, 0, 0);		// LCD hardware reset
	GPIO_ClrBit(GPIOC, PIN6);
	for(i = 0; i < 1000000; i++) __NOP();
	GPIO_SetBit(GPIOC, PIN6);
	
	PORT_Init(PORTB, PIN6,  PORTB_PIN6_LCD_VS,  0);
	PORT_Init(PORTB, PIN7,  PORTB_PIN7_LCD_HS,  0);
	PORT_Init(PORTB, PIN8,  PORTB_PIN8_LCD_DE,  0);
	PORT_Init(PORTD, PIN15, PORTD_PIN15_LCD_CK,  0);
	PORT_Init(PORTB, PIN9,  PORTB_PIN9_LCD_B0, 0);
	PORT_Init(PORTB, PIN10, PORTB_PIN10_LCD_B1, 0);
	PORT_Init(PORTB, PIN11, PORTB_PIN11_LCD_B2, 0);
	PORT_Init(PORTB, PIN13, PORTB_PIN13_LCD_B3, 0);
	PORT_Init(PORTB, PIN15, PORTB_PIN15_LCD_B4, 0);
	PORT_Init(PORTA, PIN0,  PORTA_PIN0_LCD_B5,  0);
	PORT_Init(PORTA, PIN1,  PORTA_PIN1_LCD_B6,  0);
	PORT_Init(PORTA, PIN2,  PORTA_PIN2_LCD_B7,  0);
	PORT_Init(PORTD, PIN10, PORTD_PIN10_LCD_G0, 0);
	PORT_Init(PORTE, PIN13, PORTE_PIN13_LCD_G1, 0);
	PORT_Init(PORTA, PIN9,  PORTA_PIN9_LCD_G2,  0);
	PORT_Init(PORTA, PIN10, PORTA_PIN10_LCD_G3,  0);
	PORT_Init(PORTA, PIN11, PORTA_PIN11_LCD_G4, 0);
	PORT_Init(PORTC, PIN10, PORTC_PIN10_LCD_G5, 0);
	PORT_Init(PORTC, PIN11, PORTC_PIN11_LCD_G6, 0);
	PORT_Init(PORTC, PIN12, PORTC_PIN12_LCD_G7, 0);
	PORT_Init(PORTD, PIN0,  PORTD_PIN0_LCD_R0, 0);
	PORT_Init(PORTD, PIN1,  PORTD_PIN1_LCD_R1, 0);
	PORT_Init(PORTD, PIN2,  PORTD_PIN2_LCD_R2,  0);
	PORT_Init(PORTD, PIN3,  PORTD_PIN3_LCD_R3,  0);
	PORT_Init(PORTD, PIN4,  PORTD_PIN4_LCD_R4,  0);
	PORT_Init(PORTD, PIN5,  PORTD_PIN5_LCD_R5,  0);
	PORT_Init(PORTD, PIN6,  PORTD_PIN6_LCD_R6,  0);
	PORT_Init(PORTD, PIN7,  PORTD_PIN7_LCD_R7,  0);
	
	LCD_initStruct.ClkDiv = 6;
	LCD_initStruct.Format = LCD_FMT_RGB565;
	LCD_initStruct.HnPixel = LCD_HDOT;
	LCD_initStruct.VnPixel = LCD_VDOT;
	LCD_initStruct.Hfp = 5;
	LCD_initStruct.Hbp = 40;
	LCD_initStruct.Vfp = 8;
	LCD_initStruct.Vbp = 8;
	LCD_initStruct.HsyncWidth = 5;
	LCD_initStruct.VsyncWidth = 5;
	LCD_initStruct.DataSource = (uint32_t)LCD_Buffer;
	LCD_initStruct.Background = 0xFFFF;
	LCD_initStruct.SampleEdge = LCD_SAMPLE_FALL;	// ATK-4342 samples data on falling edge
	LCD_initStruct.IntEOTEn = 1;
	LCD_Init(LCD, &LCD_initStruct);
}

void LCD_Handler(void)
{
	LCD_INTClr(LCD, LCD_IT_DONE);
	
	LCD_Start(LCD);
}


void MemoryInit(void)
{
	PSRAM_InitStructure PSRAM_initStruct;
	
	PORT_Init(PORTE, PIN9,  PORTE_PIN9_PSRAM_RST, 0);
	PORT_Init(PORTE, PIN10, PORTE_PIN10_PSRAM_CS, 0);
	PORT_Init(PORTE, PIN11, PORTE_PIN11_PSRAM_CK, 0);
	PORT_Init(PORTE, PIN8,  PORTE_PIN8_PSRAM_RWDS,1);
	PORT_Init(PORTE, PIN0,  PORTE_PIN0_PSRAM_D0,  1);
	PORT_Init(PORTE, PIN1,  PORTE_PIN1_PSRAM_D1,  1);
	PORT_Init(PORTE, PIN2,  PORTE_PIN2_PSRAM_D2,  1);
	PORT_Init(PORTE, PIN3,  PORTE_PIN3_PSRAM_D3,  1);
	PORT_Init(PORTE, PIN4,  PORTE_PIN4_PSRAM_D4,  1);
	PORT_Init(PORTE, PIN5,  PORTE_PIN5_PSRAM_D5,  1);
	PORT_Init(PORTE, PIN6,  PORTE_PIN6_PSRAM_D6,  1);
	PORT_Init(PORTE, PIN7,  PORTE_PIN7_PSRAM_D7,  1);

	PSRAM_initStruct.RowSize = PSRAM_RowSize_1KB;
	PSRAM_initStruct.tRWR = 50;
	PSRAM_initStruct.tACC = 50;
	PSRAM_initStruct.tCSM = 4;
	PSRAM_Init(&PSRAM_initStruct);
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
