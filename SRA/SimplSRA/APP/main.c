#include <string.h>
#include "SWM330.h"

#include "Background_rgb565.h"
#include "Background_rgb888.h"
#include "JPG_Synwit128_rgb565.h"
#include "JPG_Synwit128_rgb888.h"
#include "PNG_Synwit128_rgb565.h"
#include "PNG_Synwit128_rgb888.h"


#define LCD_HDOT	480		// 水平点数
#define LCD_VDOT	272		// 垂直点数
#define LCD_DIRH	1		// 水平显示？

#define IMG_HDOT	128
#define IMG_VDOT	128

#define SRA_RGB565	1	// 0 SRA 处理 RGB888 图片   1 SRA 处理 RGB565 图片
#define SRA_ARGB	0	// 0 前景图片像素不带 Alpha 值，使用固定值 0xFF，因此重叠部分必定显示前景图片	1 前景图片像素带 Alpha 值
#define SRA_BG		0	// 0 前景图片与背景图片（OutputAddr 指定）混合    1 前景图片与 SRA->BGCOLOR 混合，BGCOLOR 格式由同 DstFormat


#if SRA_RGB565 && (!SRA_ARGB)
uint16_t *LCD_Buffer = (uint16_t *) PSRAMM_BASE;
uint16_t *SRA_Buffer = (uint16_t *)(PSRAMM_BASE + 0x100000);	// SRA 源图片
#else
uint32_t *LCD_Buffer = (uint32_t *) PSRAMM_BASE;
uint32_t *SRA_Buffer = (uint32_t *)(PSRAMM_BASE + 0x100000);	// SRA 源图片
#endif


void SerialInit(void);
void MemoryInit(void);
void RGBLCDInit(void);
void test_SRA(uint16_t img_width, uint16_t img_height, uint16_t scl_width, uint16_t scl_height, uint32_t roate_angle);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	MemoryInit();
	
	RGBLCDInit();
	
	LCD_Start(LCD);
	
#if SRA_RGB565
	#if SRA_ARGB
	memcpy(SRA_Buffer, PNG_Synwit128_rgb565, sizeof(PNG_Synwit128_rgb565));
	#else
	memcpy(SRA_Buffer, JPG_Synwit128_rgb565, sizeof(JPG_Synwit128_rgb565));
	#endif
#else
	#if SRA_ARGB
	memcpy(SRA_Buffer, PNG_Synwit128_rgb888, sizeof(PNG_Synwit128_rgb888));
	#else
	memcpy(SRA_Buffer, JPG_Synwit128_rgb888, sizeof(JPG_Synwit128_rgb888));
	#endif
#endif
	
	SRA_InitStructure SRA_initStruct;
	SRA_initStruct.IntEOTEn = 0;
	SRA_Init(&SRA_initStruct);
	
	while(1==1)
	{
		// 缩放
		for(int i = IMG_HDOT / 2; i <= IMG_HDOT * 4; i += IMG_HDOT / 2)
			test_SRA(IMG_HDOT, IMG_VDOT, i, i, 0);
		
		// 镜像
		for(int i = SRA_MIRROR_X; i <= SRA_MIRROR_XY; i++)
			test_SRA(IMG_HDOT, IMG_VDOT, IMG_HDOT, IMG_VDOT, i);
		
		// 旋转
		for(int i = 0; i < 360; i += 10)
			test_SRA(IMG_HDOT, IMG_VDOT, IMG_HDOT, IMG_VDOT, i);
	}
}


void test_SRA(uint16_t img_width, uint16_t img_height, uint16_t scl_width, uint16_t scl_height, uint32_t roate_angle)
{
	uint16_t dst_width, dst_height;
	SRA_TransformParam SRA_transParam;
	
#if SRA_RGB565
	memcpy(LCD_Buffer, Image_Background_rgb565, sizeof(Image_Background_rgb565));
#else
	memcpy(LCD_Buffer, Image_Background_rgb888, sizeof(Image_Background_rgb888));
#endif

#if SRA_RGB565
	#if SRA_ARGB
	SRA_transParam.SrcFormat = SRA_FMT_XARGB88565;
	#else
	SRA_transParam.SrcFormat = SRA_FMT_RGB565;
	#endif
	SRA_transParam.DstFormat = SRA_FMT_RGB565;
#else
	#if SRA_ARGB
	SRA_transParam.SrcFormat = SRA_FMT_ARGB8888;
	#else
	SRA_transParam.SrcFormat = SRA_FMT_XRGB8888;
	#endif
	SRA_transParam.DstFormat = SRA_FMT_XRGB8888;
#endif
	SRA_transParam.ImageAddr = (uint32_t)SRA_Buffer;
	SRA_transParam.ImageWidth = img_width;
	SRA_transParam.ImageHeight = img_height;
	SRA_transParam.ScaleWidth = scl_width;
	SRA_transParam.ScaleHeight = scl_height;
	SRA_transParam.RotateAngle = roate_angle;
	SRA_transParam.OutputAddr = (uint32_t)LCD_Buffer;
	SRA_transParam.OutputWidth = LCD_HDOT;
	SRA_Transform(&SRA_transParam, &dst_width, &dst_height);
	
#if SRA_BG
	#if SRA_RGB565
	SRA->BGCOLOR = 0x001F;
	#else
	SRA->BGCOLOR = 0x0000FF;
	#endif
	SRA->CFG |= SRA_CFG_BLENDBG_Msk;
#endif
	
	SRA_Start();
	
	while(SRA_Busy() != 0) __NOP();
	
	for(int i = 0; i < SystemCoreClock / 8; i++) __NOP();
}


void RGBLCDInit(void)
{
	LCD_InitStructure LCD_initStruct;
	
	GPIO_Init(GPIOE, PIN15, 1, 0, 0, 0);	// LCD hardware reset
	GPIO_ClrBit(GPIOE, PIN15);
	for(int i = 0; i < CyclesPerUs*1000; i++) __NOP();
	GPIO_SetBit(GPIOE, PIN15);
	
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
	
	LCD_initStruct.ClkDiv = 8;
#if SRA_RGB565
	LCD_initStruct.Format = LCD_FMT_RGB565;
#else
	LCD_initStruct.Format = LCD_FMT_RGB888;
#endif
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
	LCD_initStruct.SampleEdge = LCD_SAMPLE_FALL;	// ATK-4342 RGBLCD 下降沿采样
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
	
#ifndef PSRAM_XCCELA
	PSRAM_initStruct.RowSize = PSRAM_RowSize_1KB;
	PSRAM_initStruct.tRWR = 50;
	PSRAM_initStruct.tACC = 50;
	PSRAM_initStruct.tCSM = 4;
	PSRAM_Init(&PSRAM_initStruct);
#else
	PSRAM_initStruct.RowSize = PSRAM_RowSize_1KB;
	PSRAM_initStruct.tRC = 60;
	PSRAM_initStruct.tCEM = 8;
	PSRAM_Init(&PSRAM_initStruct);
#endif
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
