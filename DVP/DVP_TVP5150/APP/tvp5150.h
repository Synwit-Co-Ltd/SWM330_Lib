#ifndef __TVP5150_H__
#define __TVP5150_H__

#include <stdint.h>
#include <stdbool.h>


#define TVP_REG_InputSource					0x00
#define TVP_REG_AnalogChannelControls		0x01
#define TVP_REG_OperatingModeControls		0x02
#define TVP_REG_MiscellaneousControls		0x03
#define TVP_REG_AutoswitchMask				0x04
#define TVP_REG_ColorKillerControl			0x06
#define TVP_REG_LuminanceControl1			0x07
#define TVP_REG_LuminanceControl2			0x08
#define TVP_REG_BrightnessControl			0x09
#define TVP_REG_SaturationControl			0x0A
#define TVP_REG_HueControl					0x0B
#define TVP_REG_ContrastControl				0x0C
#define TVP_REG_OutputAndRatesSelect		0x0D
#define TVP_REG_LuminanceControl3			0x0E
#define TVP_REG_PinsConfig					0x0F
#define TVP_REG_ActiveVideoCropingStartMSB	0x11
#define TVP_REG_ActiveVideoCropingStartLSB	0x12
#define TVP_REG_ActiveVideoCropingStopMSB	0x13
#define TVP_REG_ActiveVideoCropingStopLSB	0x14
#define TVP_REG_Genlock						0x15
#define TVP_REG_HorisontalSynkStart			0x16
#define TVP_REG_VerticalBlankingStart		0x18
#define TVP_REG_VerticalBlankingStop		0x19
#define TVP_REG_CrominanceControl1			0x1A
#define TVP_REG_CrominanceControl2			0x1B
#define TVP_REG_InterruptB_Reset			0x1C
#define TVP_REG_InterruptB_Enable			0x1D
#define TVP_REG_InterruptB_Config			0x1E
#define TVP_REG_VideoStandart				0x28
#define TVP_REG_DeviceMSB					0x80
#define TVP_REG_DeviceLSB					0x81
#define TVP_REG_ROMVersion					0x82
#define TVP_REG_RAMVersion					0x83
#define TVP_REG_VerticalLineCountMSB		0x84
#define TVP_REG_VerticalLineCountLSB		0x85
#define TVP_REG_InterruptB_Status			0x86
#define TVP_REG_InterruptB_Active			0x87
#define TVP_REG_StatusRegister1				0x88
#define TVP_REG_StatusRegister2				0x89
#define TVP_REG_StatusRegister3				0x8A
#define TVP_REG_StatusRegister4				0x8B
#define TVP_REG_StatusRegister5				0x8C
#define TVP_REG_ClosedCaptionData1			0x90
#define TVP_REG_ClosedCaptionData2			0x91
#define TVP_REG_ClosedCaptionData3			0x92
#define TVP_REG_ClosedCaptionData4			0x93
#define TVP_REG_WSS_Data1					0x94
#define TVP_REG_WSS_Data2					0x95
#define TVP_REG_WSS_Data3					0x96
#define TVP_REG_WSS_Data4					0x97
#define TVP_REG_WSS_Data5					0x98
#define TVP_REG_WSS_Data6					0x99
#define TVP_REG_VPS_Data01					0x9A
#define TVP_REG_VPS_Data02					0x9B
#define TVP_REG_VPS_Data03					0x9C
#define TVP_REG_VPS_Data04					0x9D
#define TVP_REG_VPS_Data05					0x9E
#define TVP_REG_VPS_Data06					0x9F
#define TVP_REG_VPS_Data07					0xA1
#define TVP_REG_VPS_Data08					0xA2
#define TVP_REG_VPS_Data09					0xA3
#define TVP_REG_VPS_Data10					0xA4
#define TVP_REG_VPS_Data11					0xA5
#define TVP_REG_VPS_Data12					0xA6
#define TVP_REG_VITC_Data01					0xA7
#define TVP_REG_VITC_Data02					0xA8
#define TVP_REG_VITC_Data03					0xA9
#define TVP_REG_VITC_Data04					0xA7
#define TVP_REG_VITC_Data05					0xAA
#define TVP_REG_VITC_Data06					0xAB
#define TVP_REG_VITC_Data07					0xAC
#define TVP_REG_VITC_Data08					0xAD
#define TVP_REG_VITC_Data09					0xAE
#define TVP_REG_VITC_Data10					0xAF
#define TVP_REG_VBI_FIFO					0xB0
#define TVP_REG_TeletextFilter11			0xB1
#define TVP_REG_TeletextFilter12			0xB2
#define TVP_REG_TeletextFilter13			0xB3
#define TVP_REG_TeletextFilter14			0xB4
#define TVP_REG_TeletextFilter15			0xB5
#define TVP_REG_TeletextFilter21			0xB6
#define TVP_REG_TeletextFilter22			0xB7
#define TVP_REG_TeletextFilter23			0xB8
#define TVP_REG_TeletextFilter24			0xB9
#define TVP_REG_TeletextFilter25			0xBA
#define TVP_REG_TeletextFilterControl		0xBB
#define TVP_REG_InterruptA_Status			0xC0
#define TVP_REG_InterruptA_Enable			0xC1
#define TVP_REG_InterruptA_Config			0xC2
#define TVP_REG_VDP_Conf_RAM_Data			0xC3
#define TVP_REG_VDP_Conf_RAM_AddressLSB		0xC4
#define TVP_REG_VDP_Conf_RAM_AddressMSB		0xC5
#define TVP_REG_VDP_Status					0xC6
#define TVP_REG_FIFO_WordCount				0xC7
#define TVP_REG_FIFO_InterruptThreshold		0xC8
#define TVP_REG_FIFO_Reset					0xC9
#define TVP_REG_LineNumberInterrupt			0xCA
#define TVP_REG_PixelAlignmentLSB			0xCB
#define TVP_REG_PixelAlignmentMSB			0xCC
#define TVP_REG_FIFO_OutputControl			0xCD
#define TVP_REG_FullFieldEnable				0xCF
#define TVP_REG_LineModeStart				0xD0
#define TVP_REG_LineModeEnd					0xFB
#define TVP_REG_FullFieldMode				0xFC


#define TVP_InputSource_AIP1A		0x00	// Composite AIP1A
#define TVP_InputSource_AIP_S		0x01	// S-Video: AIP1A (luminance), AIP1B (chrominance)
#define TVP_InputSource_AIP1B		0x02	// Composite AIP1B

/* TVP_REG_PinsConfig bit-field */
#define TVP_PIN23_FID		((0 << 3) | (0 << 6))	// pin23 used as odd/even field indicator
#define TVP_PIN23_GLCO		((1 << 3) | (0 << 6))
#define TVP_PIN23_LOCK		((0 << 3) | (1 << 6))	// pin23 used as vertical lock indicator

#define TVP_PIN24_VSYNC		((0 << 2) | (0 << 4))	// pin24 used as vertical synchronization signal
#define TVP_PIN24_PALI		((1 << 2) | (0 << 4))	// pin24 used as PAL line indicator
#define TVP_PIN24_LOCK		((0 << 2) | (1 << 4))	// pin24 used as horizontal lock indicator

#define TVP_PIN27_INTREQ	(0 << 1)				// pin27 used as interrupt request output
#define TVP_PIN27_VBLK	 	(1 << 1)				// pin27 used as vertical blanking output

/* TVP_REG_OutputAndRatesSelect bit-field */
#define TVP_FMT_YUV422		(0 << 0)
#define TVP_FMT_BT656		(7 << 0)

#define TVP_CODE_PLUS128	(0 << 5)
#define TVP_CODE_RAW		(1 << 5)

#define TVP_RANGE_BT601		(0 << 6)	// ITU-R BT.601 coding range (Y ranges from 16 to 235. U and V range from 16 to 240)
#define TVP_RANGE_EXT		(1 << 6)	// Extended coding range (Y, U, and V range from 1 to 254)


void TVP_Init(void);

void TVP_Write(uint8_t reg, uint8_t data);
uint8_t TVP_Read(uint8_t reg);


#endif
