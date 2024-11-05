#include "SWM330.h"

#include "ugui.h"
#include "ST77903.h"


UG_GUI gui;

uint16_t *LCD_Buffer = (uint16_t *)PSRAMM_BASE;

void _HW_DrawPoint(UG_S16 x, UG_S16 y, UG_COLOR c)
{
	LCD_Buffer[y * LCD_HDOT + x] = __REVSH(c);
}


void SerialInit(void);
void MemoryInit(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	MemoryInit();
	
	ST77903_Init();
	
	ST77903_Start(LCD_Buffer);
	
	UG_Init(&gui,(void(*)(UG_S16,UG_S16,UG_COLOR))_HW_DrawPoint, LCD_HDOT, LCD_VDOT);
	
	UG_FillScreen(C_WHITE);
	
	UG_DrawLine(100, 50, 300, 50, C_RED);
	
	UG_DrawLine(50, 100, 50, 300, C_BLUE);
	
	UG_DrawCircle(200, 200, 100, C_GREEN);
	UG_DrawCircle(200, 200, 101, C_GREEN);
	UG_DrawCircle(200, 200, 102, C_GREEN);
	
	while(1==1)
	{
		uint16_t colors[5] = { C_BLACK, C_RED, C_GREEN, C_BLUE, C_GRAY };
		
		for(int i = 0; i < 5; i++)
		{
			UG_FillCircle(200, 200, 50, colors[i]);
			
			for(int j = 0; j < SystemCoreClock/4; j++) __NOP();
		}
	}
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
