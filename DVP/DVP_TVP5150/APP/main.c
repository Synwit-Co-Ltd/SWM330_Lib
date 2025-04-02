#include "SWM330.h"

#include "tvp5150.h"


#define LCD_HDOT	480		// Horizontal points
#define LCD_VDOT	272		// Vertical points
#define LCD_DIRH	1		// horizontal display?

#define CAP_HDOT	480		// TVP5150 output width: 720
#define CAP_VDOT	240		// TVP5150 output height: 263
#define CAP_YUV		0		// 0 output RGB directly, 1 output YUV, and then software convert it to RGB


uint16_t *LCD_Buffer = (uint16_t *)(PSRAMM_BASE);
uint8_t  *CAP_Y_Buffer = (uint8_t *)(PSRAMM_BASE + 0x100000);
uint8_t  *CAP_UV_Buffer = (uint8_t *)(PSRAMM_BASE + 0x200000);


void SerialInit(void);
void MemoryInit(void);
void RGBLCDInit(void);
void DVP_Config(void);
void YUV422ToRGB565(const uint8_t * Ybuf, const uint8_t * UVbuf, uint16_t * RGBbuf, int width, int height);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	MemoryInit();
	
	RGBLCDInit();
	
	TVP_Init();
	
	DVP_Config();
	
	DVP_Start(DVP, 1, 1);
	
	GPIO_INIT(GPIOA, PIN5, GPIO_OUTPUT);
	
	while(1==1)
	{
		DVP_Start(DVP, 1, 1);
		while(DVP_Busy(DVP)) __NOP();
		
#if CAP_YUV
		YUV422ToRGB565(CAP_Y_Buffer, CAP_UV_Buffer, LCD_Buffer, CAP_HDOT, CAP_VDOT);
#endif
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


void RGBLCDInit(void)
{
	LCD_InitStructure LCD_initStruct;
	
	GPIO_Init(GPIOA, PIN6, 1, 0, 0, 0);		// LCD backlight switch
	GPIO_SetBit(GPIOA, PIN6);
	GPIO_Init(GPIOC, PIN6, 1, 0, 0, 0);		// LCD hardware reset
	GPIO_ClrBit(GPIOC, PIN6);
	for(int i = 0; i < 1000000; i++) __NOP();
	GPIO_SetBit(GPIOC, PIN6);
	
	PORT_Init(PORTB, PIN7,  PORTB_PIN7_LCD_VS,  0);
	PORT_Init(PORTB, PIN6,  PORTB_PIN6_LCD_HS,  0);
	PORT_Init(PORTB, PIN8,  PORTB_PIN8_LCD_DE,  0);
	PORT_Init(PORTD, PIN15, PORTD_PIN15_LCD_CK, 0);
//	PORT_Init(PORTB, PIN9,  PORTB_PIN9_LCD_B0,  0);
//	PORT_Init(PORTB, PIN10, PORTB_PIN10_LCD_B1, 0);
//	PORT_Init(PORTB, PIN11, PORTB_PIN11_LCD_B2, 0);
	PORT_Init(PORTB, PIN12, PORTB_PIN12_LCD_B3, 0);
	PORT_Init(PORTB, PIN15, PORTB_PIN15_LCD_B4, 0);
	PORT_Init(PORTA, PIN0,  PORTA_PIN0_LCD_B5,  0);
	PORT_Init(PORTA, PIN1,  PORTA_PIN1_LCD_B6,  0);
	PORT_Init(PORTA, PIN2,  PORTA_PIN2_LCD_B7,  0);
//	PORT_Init(PORTD, PIN10, PORTD_PIN10_LCD_G0, 0);
//	PORT_Init(PORTE, PIN13, PORTE_PIN13_LCD_G1, 0);
	PORT_Init(PORTA, PIN9,  PORTA_PIN9_LCD_G2,  0);
	PORT_Init(PORTA, PIN10, PORTA_PIN10_LCD_G3, 0);
	PORT_Init(PORTA, PIN11, PORTA_PIN11_LCD_G4, 0);
	PORT_Init(PORTC, PIN10, PORTC_PIN10_LCD_G5, 0);
	PORT_Init(PORTC, PIN11, PORTC_PIN11_LCD_G6, 0);
	PORT_Init(PORTC, PIN12, PORTC_PIN12_LCD_G7, 0);
//	PORT_Init(PORTD, PIN0,  PORTD_PIN0_LCD_R0,  0);
//	PORT_Init(PORTD, PIN1,  PORTD_PIN1_LCD_R1,  0);
//	PORT_Init(PORTD, PIN2,  PORTD_PIN2_LCD_R2,  0);
	PORT_Init(PORTD, PIN3,  PORTD_PIN3_LCD_R3,  0);
	PORT_Init(PORTD, PIN4,  PORTD_PIN4_LCD_R4,  0);
	PORT_Init(PORTD, PIN5,  PORTD_PIN5_LCD_R5,  0);
	PORT_Init(PORTD, PIN6,  PORTD_PIN6_LCD_R6,  0);
	PORT_Init(PORTD, PIN7,  PORTD_PIN7_LCD_R7,  0);
	
	LCD_initStruct.ClkDiv = 12;
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
	
	LCD_Start(LCD);
}


void DVP_Config(void)
{
	DVP_InitStructure DVP_initStruct;
	
//	PORT_Init(PORTD, PIN13, PORTD_PIN13_DVP_VS,  1);
//	PORT_Init(PORTD, PIN14, PORTD_PIN14_DVP_HS,  1);
	PORT_Init(PORTD, PIN12, PORTD_PIN12_DVP_CK,  1);
	PORT_Init(PORTC, PIN14, PORTC_PIN14_DVP_D0,  1);
	PORT_Init(PORTE, PIN12, PORTE_PIN12_DVP_D1,  1);
	PORT_Init(PORTA, PIN12, PORTA_PIN12_DVP_D2,  1);
	PORT_Init(PORTC, PIN0,  PORTC_PIN0_DVP_D3,   1);
	PORT_Init(PORTC, PIN1,  PORTC_PIN1_DVP_D4,   1);
	PORT_Init(PORTC, PIN2,  PORTC_PIN2_DVP_D5,   1);
	PORT_Init(PORTC, PIN3,  PORTC_PIN3_DVP_D6,   1);
	PORT_Init(PORTC, PIN4,  PORTC_PIN4_DVP_D7,   1);
//	PORT_Init(PORTC, PIN6,  PORTC_PIN6_DVP_D8,   1);
//	PORT_Init(PORTC, PIN7,  PORTC_PIN7_DVP_D9,   1);
//	PORT_Init(PORTC, PIN8,  PORTC_PIN8_DVP_D10,  1);
//	PORT_Init(PORTD, PIN8,  PORTD_PIN8_DVP_D11,  1);
//	PORT_Init(PORTD, PIN9,  PORTD_PIN9_DVP_D12,  1);
//	PORT_Init(PORTD, PIN11, PORTD_PIN11_DVP_D13, 1);
	
#if CAP_YUV
	DVP_initStruct.InFormat = DVP_INFMT_BT656_UYVY;
	DVP_initStruct.OutFormat = DVP_OUTFMT_YUV422;
	DVP_initStruct.StartLine = 1;
	DVP_initStruct.LineCount = CAP_VDOT;
	DVP_initStruct.StartPixel = 1;
	DVP_initStruct.PixelCount = CAP_HDOT;
	DVP_initStruct.LineStride = CAP_HDOT;
	DVP_initStruct.SampleEdge = DVP_PCKPolarity_Falling;
	DVP_initStruct.YAddr = (uint32_t)CAP_Y_Buffer;
	DVP_initStruct.UVAddr = (uint32_t)CAP_UV_Buffer;
	DVP_initStruct.IntEn = 0;
	DVP_Init(DVP, &DVP_initStruct);
#else
	DVP_initStruct.InFormat = DVP_INFMT_BT656_UYVY;
	DVP_initStruct.OutFormat = DVP_OUTFMT_RGB565;
	DVP_initStruct.StartLine = 1;
	DVP_initStruct.LineCount = CAP_VDOT;
	DVP_initStruct.StartPixel = 1;
	DVP_initStruct.PixelCount = CAP_HDOT;
	DVP_initStruct.LineStride = LCD_HDOT;
	DVP_initStruct.SampleEdge = DVP_PCKPolarity_Falling;
	DVP_initStruct.RGBAddr = (uint32_t)LCD_Buffer;
	DVP_initStruct.IntEn = 0;
	DVP_Init(DVP, &DVP_initStruct);
#endif
}


#define RANGE_LIMIT(x) (x > 255 ? 255 : (x < 0 ? 0 : x))

void YUV422ToRGB565(const uint8_t * Ybuf, const uint8_t * UVbuf, uint16_t * RGBbuf, int width, int height)
{
	int16_t Y[16], U[16], V[16];
	int16_t y, u, v;
	uint8_t r, g, b;
	
	for(int i = 0; i < height; i++)
	{
		for(int j = 0; j < width; j += 16)
		{
			uint32_t index = i * width + j;
			
			/* 对同一个地址的连续读写，可大幅提升 PSRAM 的读写效率 */
			for(int k = 0; k < 16; k++)
				Y[k] = Ybuf[index + k];
			
			for(int k = 0; k < 16; k++)
				U[k] = UVbuf[((index + k) & 0xFFFFFE)] - 128;
			
			for(int k = 0; k < 16; k++)
				V[k] = UVbuf[((index + k) & 0xFFFFFE) + 1] - 128;
			
			for(int k = 0; k < 16; k++)
			{
				y = Y[k];
				u = U[k];
				v = V[k];
				
				// R = Y + 1.402*(V-128)
				// G = Y - 0.34414*(U-128)
				// B = Y + 1.772*(U-128)
				r = RANGE_LIMIT(y + v + ((v * 103) >> 8));
				g = RANGE_LIMIT(y - ((u * 88) >> 8) - ((v * 183) >> 8));
				b = RANGE_LIMIT(y + u + ((u * 198) >> 8));
				
				RGBbuf[index + k] = (((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3));
			}
		}
	}
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
