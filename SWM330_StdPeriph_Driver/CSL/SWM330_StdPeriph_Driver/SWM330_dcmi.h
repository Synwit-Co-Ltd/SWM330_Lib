#ifndef __SWM330_DCMI_H__
#define __SWM330_DCMI_H__


typedef struct {
	uint8_t  Format;		// DCMI_FMT_MONO, DCMI_FMT_BAYER_8b, ...
	
	/* position and size of output window, can ignore when Format == DCMI_FMT_JPEG */
	uint16_t StartLine;		// 1 for first line
	uint16_t LineCount;		// 1 for first pixel
	uint16_t StartPixel;
	uint16_t PixelCount;
	
	uint8_t  DownSample;	// 0, or ' DCMI_Line_xx | DCMI_Byte_yy'; must be 0 when Format == DCMI_FMT_JPEG
	
	uint8_t  SampleEdge;	// DCMI_PCKPolarity_Falling, DCMI_PCKPolarity_Rising
	
	uint8_t  IntEn;			// Interrupt Enable, can be DCMI_IT_FRAME, DCMI_IT_FIFOOV, ..., DCMI_IT_HSYNC and their '|' operation
} DCMI_InitStructure;


#define DCMI_FMT_MONO				(0 | (0 << 4))
#define DCMI_FMT_BAYER_8b			(0 | (1 << 4) | (0 << 8))
#define DCMI_FMT_BAYER_10b			(0 | (1 << 4) | (1 << 8))
#define DCMI_FMT_BAYER_12b			(0 | (1 << 4) | (2 << 8))
#define DCMI_FMT_BAYER_14b			(0 | (1 << 4) | (3 << 8))
#define DCMI_FMT_RGB565				(0 | (2 << 4))
#define DCMI_FMT_YUV422				(0 | (3 << 4))
#define DCMI_FMT_JPEG				(1)


#define DCMI_PCKPolarity_Falling	0		// sample on falling edge
#define DCMI_PCKPolarity_Rising		1

#define DCMI_HSPolarity_Low			0		// active low, data not valid when HSync active
#define DCMI_HSPolarity_High		1

#define DCMI_VSPolarity_Low			0		// active low, data not valid when VSync active
#define DCMI_VSPolarity_High		1


typedef enum {
	DCMI_Line_All         = ((0 | (0 << 1)) << 3),
	DCMI_Line_1of2_First  = ((1 | (0 << 1)) << 3),	// capture first line, second line being dropped
	DCMI_Line_1of2_Second = ((1 | (1 << 1))	<< 3)	// capture second line, first line being dropped
} DCMI_DownSampling_Line;

typedef enum {
	DCMI_Byte_All         = (0 | (0 << 2)),
	DCMI_Byte_1of2_First  = (1 | (0 << 2)),
	DCMI_Byte_1of2_Second = (1 | (1 << 2)),
	DCMI_Byte_1of4_First  = (2 | (0 << 2)),
	DCMI_Byte_1of4_Second = (2 | (1 << 2)),
	DCMI_Byte_2of4_First  = (3 | (0 << 2)),			// capture first two-byte
	DCMI_Byte_2of4_Second = (3 | (1 << 2))			// capture second two-byte
} DCMI_DownSampling_Byte;



/* interrupt type */
#define DCMI_IT_FRAME		DCMI_IER_FRAME_Msk		// Frame Capture complete
#define DCMI_IT_FIFOOV		DCMI_IER_FIFOOV_Msk		// FIFO Overrun
#define DCMI_IT_ESCERR		DCMI_IER_ESCERR_Msk		// Embedded Synchronization Code Error
#define DCMI_IT_VSYNC		DCMI_IER_VSYNC_Msk		// VSYNC change from inactive to active
#define DCMI_IT_HSYNC		DCMI_IER_HSYNC_Msk		// HSYNC change from inactive to active



void DCMI_Init(DCMI_TypeDef * DCMIx, DCMI_InitStructure * initStruct);
void DCMI_Start(DCMI_TypeDef * DCMIx);
void DCMI_Stop(DCMI_TypeDef * DCMIx);
bool DCMI_Busy(DCMI_TypeDef * DCMIx);

void DCMI_EmbeddedSync(DCMI_TypeDef * DCMIx, uint8_t frame_start, uint8_t line_start, uint8_t line_end, uint8_t frame_end);

void DCMI_INTEn(DCMI_TypeDef * DCMIx, uint32_t it);
void DCMI_INTDis(DCMI_TypeDef * DCMIx, uint32_t it);
void DCMI_INTClr(DCMI_TypeDef * DCMIx, uint32_t it);
bool DCMI_INTStat(DCMI_TypeDef * DCMIx, uint32_t it);


#endif
