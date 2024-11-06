#ifndef __SWM330_DVP_H__
#define __SWM330_DVP_H__


typedef struct {
	uint16_t InFormat;			// 
	uint16_t OutFormat;			// 
	uint16_t StartField;		// 
	uint16_t StartFrame;		// 
	uint16_t FrameCount;		// 
	uint16_t StartLine;			// 
	uint16_t LineCount;			// 
	uint16_t StartPixel;		// 
	uint16_t PixelCount;		// 
	uint8_t  SampleEdge;		// 
	uint8_t  IntEn;				// Interrupt Enable
} DVP_InitStructure;


#define DVP_PCKPolarity_Falling		0			// DVP_PCK falling edge active
#define DVP_PCKPolarity_Rising		1

#define DVP_VSPolarity_Low			0			// DVP_VSYNC active low
#define DVP_VSPolarity_High			1

#define DVP_HSPolarity_Low			0
#define DVP_HSPolarity_High			1




#endif
