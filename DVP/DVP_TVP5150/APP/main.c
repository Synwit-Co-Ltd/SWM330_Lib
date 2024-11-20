#include <string.h>
#include "SWM330.h"

#include "tvp5150.h"

#include "../../../JPEG/SimplJPEG/APP/jfif_parser.h"
#include "../../../JPEG/SimplJPEG/APP/jfif_parser.c"


#define LCD_HDOT	480		// Horizontal points
#define LCD_VDOT	272		// Vertical points
#define LCD_DIRH	1		// horizontal display?

#define CAP_FMT		OV_FMT_RGB565
#define CAP_HDOT	320
#define CAP_VDOT	240


uint16_t *LCD_Buffer = (uint16_t *)(PSRAMM_BASE);
uint16_t *CAP_Buffer = (uint16_t *)(PSRAMM_BASE + 0x100000);
uint16_t *RGB_Buffer = (uint16_t *)(PSRAMM_BASE + 0x200000);


void SerialInit(void);
void MemoryInit(void);
void RGBLCDInit(void);
void DVP_Config(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	MemoryInit();
	
	RGBLCDInit();
	
	LCD_Start(LCD);
	
	TVP_Init();
	
	DVP_Config();
	
#if CAP_FMT == OV_FMT_JPEG
	jfif_info_t jfif_info;
	jpeg_outset_t jpeg_outset;
	JPEG_InitStructure JPEG_initStruct;
	
	JPEG_initStruct.DoneIEn = 0;
	JPEG_initStruct.ErrorIEn = 0;
	JPEG_Init(JPEG, &JPEG_initStruct);
#endif

	while(1==1)
	{
		DVP_Start(DVP, 1);
		while(DVP_Busy(DVP)) __NOP();
		
#if CAP_FMT == OV_FMT_RGB565
		for(int i = 0; i < CAP_VDOT; i++)
			for(int j = 0; j < CAP_HDOT; j++)
				LCD_Buffer[LCD_HDOT * i + j] = CAP_Buffer[CAP_HDOT * i + j];
#else
		uint32_t xfer_len = CAP_HDOT * CAP_VDOT * 2 / 4 - DMA_CH_GetRemaining(DMA_CH3);
		
		jfif_parse((uint8_t  *)CAP_Buffer, xfer_len * 4, &jfif_info);
		
		jpeg_outset.format = JPEG_OUT_RGB565;
		jpeg_outset.dither = 0;
		jpeg_outset.RGBAddr = (uint32_t)RGB_Buffer;
		JPEG_Decode(JPEG, &jfif_info, &jpeg_outset);
		while(JPEG_DecodeBusy(JPEG)) __NOP();
		
		for(int i = 0; i < CAP_VDOT; i++)
			for(int j = 0; j < CAP_HDOT; j++)
				LCD_Buffer[LCD_HDOT * i + j] = RGB_Buffer[CAP_HDOT * i + j];
#endif
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
	LCD_initStruct.SampleEdge = LCD_SAMPLE_FALL;	// ATK-4342 RGBLCD sampling on falling edge
	LCD_initStruct.IntEOTEn = 0;
	LCD_Init(LCD, &LCD_initStruct);
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


void DVP_Config(void)
{
	DVP_InitStructure DVP_initStruct;
	
	PORT_Init(PORTC, PIN2,  PORTC_PIN2_DVP_VS,   1);
	PORT_Init(PORTC, PIN3,  PORTC_PIN3_DVP_HS,   1);
	PORT_Init(PORTC, PIN1,  PORTC_PIN1_DVP_CK,   1);
	PORT_Init(PORTC, PIN4,  PORTC_PIN4_DVP_D0,   1);
	PORT_Init(PORTC, PIN5,  PORTC_PIN5_DVP_D1,   1);
	PORT_Init(PORTC, PIN6,  PORTC_PIN6_DVP_D2,   1);
	PORT_Init(PORTC, PIN7,  PORTC_PIN7_DVP_D3,   1);
	PORT_Init(PORTC, PIN8,  PORTC_PIN8_DVP_D4,   1);
	PORT_Init(PORTA, PIN8,  PORTA_PIN8_DVP_D5,   1);
	PORT_Init(PORTA, PIN13, PORTA_PIN13_DVP_D6,  1);
	PORT_Init(PORTA, PIN14, PORTA_PIN14_DVP_D7,  1);
	PORT_Init(PORTA, PIN15, PORTA_PIN15_DVP_D8,  1);
	PORT_Init(PORTC, PIN15, PORTC_PIN15_DVP_D9,  1);
	PORT_Init(PORTC, PIN14, PORTC_PIN14_DVP_D10, 1);
	PORT_Init(PORTE, PIN12, PORTE_PIN12_DVP_D11, 1);
	PORT_Init(PORTA, PIN12, PORTA_PIN12_DVP_D12, 1);
	PORT_Init(PORTC, PIN0,  PORTC_PIN0_DVP_D13,  1);
	
	DVP_initStruct.InFormat = DVP_INFMT_RGB565;
	DVP_initStruct.OutFormat = DVP_OUTFMT_RAW;
	DVP_initStruct.StartLine = 1;
	DVP_initStruct.LineCount = CAP_VDOT;
	DVP_initStruct.StartPixel = 1;
	DVP_initStruct.PixelCount = CAP_HDOT;
	DVP_initStruct.SampleEdge = DVP_PCKPolarity_Rising;
	DVP_initStruct.RawAddr = (uint32_t)CAP_Buffer;
	DVP_initStruct.IntEn = 0;
	DVP_Init(DVP, &DVP_initStruct);
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
