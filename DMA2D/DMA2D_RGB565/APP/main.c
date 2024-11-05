#include "SWM330.h"


#define LCD_HDOT	480		// horizontal dot number
#define LCD_VDOT	272		// vertical dot number
#define LCD_DIRH	1		// horizontal display?


uint16_t *LCD_Buffer = (uint16_t *) PSRAMM_BASE;
uint32_t *Img_Buffer = (uint32_t *)(PSRAMM_BASE + 0x100000);

DMA2D_LayerSetting fgLayer, bgLayer, outLayer;

extern const unsigned char gImage_Synwit128[32768];


void SerialInit(void);
void MemoryInit(void);
void RGBLCDInit(void);
void test_PixelFill(void);
void test_PixelMove(void);
void test_PixelBlend(void);

int main(void)
{
	DMA2D_InitStructure DMA2D_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	MemoryInit();
	
	RGBLCDInit();
	
	LCD_Start(LCD);
	
	DMA2D_initStruct.Interval = CyclesPerUs;
	DMA2D_initStruct.IntEOTEn = 1;
	DMA2D_Init(&DMA2D_initStruct);
	
	test_PixelFill();
	
	test_PixelMove();
	
	test_PixelBlend();
	
	while(1==1)
	{
	}
}


void DMA2D_Handler(void)
{	
	DMA2D_INTClr();
}


void test_PixelFill(void)
{
	uint32_t i;
	
	/* full screen fill blue */
	outLayer.Address = (uint32_t)LCD_Buffer;
	outLayer.LineCount = LCD_VDOT;
	outLayer.LinePixel = LCD_HDOT;
	outLayer.LineOffset = 0;
	outLayer.ColorMode = DMA2D_FMT_RGB565;
	DMA2D_PixelFill(&outLayer, 0x001F);
	
	while(DMA2D_IsBusy()) __NOP();
	
	for(i = 0; i < SystemCoreClock/8; i++) __NOP();
	
	/* Draw a 150*150 green square in the upper left corner */
	outLayer.Address = (uint32_t)LCD_Buffer;
	outLayer.LineCount = 150;
	outLayer.LinePixel = 150;
	outLayer.LineOffset = LCD_HDOT-150;
	outLayer.ColorMode = DMA2D_FMT_RGB565;
	DMA2D_PixelFill(&outLayer, 0x07E0);
	
	while(DMA2D_IsBusy()) __NOP();
	
	for(i = 0; i < SystemCoreClock/8; i++) __NOP();
	
	/* Draw a 150*150 red square in the lower right corner */
	outLayer.Address = (uint32_t)LCD_Buffer + (LCD_HDOT * (LCD_VDOT - 150) + (LCD_HDOT - 150)) * 2;
	outLayer.LineCount = 150;
	outLayer.LinePixel = 150;
	outLayer.LineOffset = LCD_HDOT-150;
	outLayer.ColorMode = DMA2D_FMT_RGB565;
	DMA2D_PixelFill(&outLayer, 0xF800);
	
	while(DMA2D_IsBusy()) __NOP();
	
	for(i = 0; i < SystemCoreClock/8; i++) __NOP();
}


void test_PixelMove(void)
{
	uint32_t i;
	
	fgLayer.Address = (uint32_t)gImage_Synwit128;
	fgLayer.LineOffset = 0;
	fgLayer.ColorMode = DMA2D_FMT_RGB565;
	
	/* Draw a 128*128 picture in the upper left corner */
	outLayer.Address = (uint32_t)LCD_Buffer;
	outLayer.LineCount = 128;
	outLayer.LinePixel = 128;
	outLayer.LineOffset = LCD_HDOT-128;
	DMA2D_PixelMove(&fgLayer, &outLayer);
	
	while(DMA2D_IsBusy()) __NOP();
	
	for(i = 0; i < SystemCoreClock/8; i++) __NOP();
	
	/* Draw a 128*128 picture in the lower right corner */
	outLayer.Address = (uint32_t)LCD_Buffer + (LCD_HDOT * (LCD_VDOT - 128) + (LCD_HDOT - 128)) * 2;
	outLayer.LineCount = 128;
	outLayer.LinePixel = 128;
	outLayer.LineOffset = LCD_HDOT-128;
	DMA2D_PixelMove(&fgLayer, &outLayer);
	
	while(DMA2D_IsBusy()) __NOP();
	
	for(i = 0; i < SystemCoreClock/8; i++) __NOP();
}


void test_PixelBlend(void)
{
	/* Convert images from RGB565 to ARGB888 format */
	fgLayer.Address = (uint32_t)gImage_Synwit128;
	fgLayer.LineOffset = 0;
	fgLayer.ColorMode = DMA2D_FMT_RGB565;
	
	outLayer.Address = (uint32_t)Img_Buffer;
	outLayer.LineCount = 128;
	outLayer.LinePixel = 128;
	outLayer.LineOffset = 0;
	outLayer.ColorMode = DMA2D_FMT_ARGB888;
	DMA2D_PixelConvert(&fgLayer, &outLayer);
	
	while(DMA2D_IsBusy()) __NOP();
	
	/*	Set the Alpha value of the white pixels to 0, so that the background color is displayed instead of the white color of the image when blending later.
		Note: This step can be completed on the computer through the image processing software.
	*/
	for(int i = 0; i < 128; i++)
		for(int j = 0; j < 128; j++)
			if(Img_Buffer[i*128+j] == 0xFFFFFFFF)
				Img_Buffer[i*128+j] = 0x00FFFFFF;
	
	/* do image blending */
	fgLayer.Address = (uint32_t)Img_Buffer;
	fgLayer.LineOffset = 0;
	fgLayer.ColorMode = DMA2D_FMT_ARGB888;
	fgLayer.AlphaMode = DMA2D_AMODE_PIXEL;
	
	bgLayer.Address = (uint32_t)LCD_Buffer + (LCD_HDOT * (LCD_VDOT - 128)/2 + (LCD_HDOT - 128)/2) * 2;;
	bgLayer.LineOffset = LCD_HDOT-128;
	bgLayer.ColorMode = DMA2D_FMT_RGB565;
	bgLayer.AlphaMode = DMA2D_AMODE_PIXEL;
	
	outLayer.Address = (uint32_t)LCD_Buffer + (LCD_HDOT * (LCD_VDOT - 128)/2 + (LCD_HDOT - 128)/2) * 2;
	outLayer.LineCount = 128;
	outLayer.LinePixel = 128;
	outLayer.LineOffset = LCD_HDOT-128;
	outLayer.ColorMode = DMA2D_FMT_RGB565;
	DMA2D_PixelBlend(&fgLayer, &bgLayer, &outLayer);
	
	while(DMA2D_IsBusy()) __NOP();
}


void RGBLCDInit(void)
{
	uint32_t i;
	LCD_InitStructure LCD_initStruct;
	
	GPIO_Init(GPIOA, PIN6, 1, 0, 0, 0);		// LCD backlight switch
	GPIO_SetBit(GPIOA, PIN6);
	GPIO_Init(GPIOC, PIN6, 1, 0, 0, 0);		// LCD hardware reset
	GPIO_ClrBit(GPIOC, PIN6);
	for(i = 0; i < CyclesPerUs*1000; i++) __NOP();
	GPIO_SetBit(GPIOC, PIN6);
	
	PORT_Init(PORTB, PIN8,  PORTB_PIN8_LCD_VS,  0);
	PORT_Init(PORTB, PIN9,  PORTB_PIN9_LCD_HS,  0);
	PORT_Init(PORTB, PIN10, PORTB_PIN10_LCD_DE, 0);
	PORT_Init(PORTB, PIN7,  PORTB_PIN7_LCD_CK,  0);
	PORT_Init(PORTB, PIN11, PORTB_PIN11_LCD_B0, 0);
	PORT_Init(PORTB, PIN12, PORTB_PIN12_LCD_B1, 0);
	PORT_Init(PORTB, PIN13, PORTB_PIN13_LCD_B2, 0);
	PORT_Init(PORTB, PIN14, PORTB_PIN14_LCD_B3, 0);
	PORT_Init(PORTB, PIN15, PORTB_PIN15_LCD_B4, 0);
	PORT_Init(PORTA, PIN0,  PORTA_PIN0_LCD_B5,  0);
	PORT_Init(PORTA, PIN1,  PORTA_PIN1_LCD_B6,  0);
	PORT_Init(PORTA, PIN2,  PORTA_PIN2_LCD_B7,  0);
	PORT_Init(PORTC, PIN9,  PORTC_PIN9_LCD_G0,  0);
	PORT_Init(PORTD, PIN10, PORTD_PIN10_LCD_G1, 0);
	PORT_Init(PORTE, PIN13, PORTE_PIN13_LCD_G2, 0);
	PORT_Init(PORTA, PIN9,  PORTA_PIN9_LCD_G3,  0);
	PORT_Init(PORTA, PIN10, PORTA_PIN10_LCD_G4, 0);
	PORT_Init(PORTA, PIN11, PORTA_PIN11_LCD_G5, 0);
	PORT_Init(PORTC, PIN10, PORTC_PIN10_LCD_G6, 0);
	PORT_Init(PORTC, PIN11, PORTC_PIN11_LCD_G7, 0);
	PORT_Init(PORTC, PIN12, PORTC_PIN12_LCD_R0, 0);
	PORT_Init(PORTC, PIN13, PORTC_PIN13_LCD_R1, 0);
	PORT_Init(PORTD, PIN0,  PORTD_PIN0_LCD_R2,  0);
	PORT_Init(PORTD, PIN1,  PORTD_PIN1_LCD_R3,  0);
	PORT_Init(PORTD, PIN2,  PORTD_PIN2_LCD_R4,  0);
	PORT_Init(PORTD, PIN3,  PORTD_PIN3_LCD_R5,  0);
	PORT_Init(PORTD, PIN4,  PORTD_PIN4_LCD_R6,  0);
	PORT_Init(PORTD, PIN5,  PORTD_PIN5_LCD_R7,  0);
	
	LCD_initStruct.ClkDiv = 8;
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
