#include "SWM330.h"

#define OV_ARRAY
#include "OV2640.h"


bool OV2640_Init(uint8_t outfmt, uint8_t clkdiv, uint16_t width, uint16_t height)
{
	I2C_InitStructure  I2C_initStruct;
	
	PORT_Init(PORTC, PIN4, FUNMUX0_I2C1_SCL, 1);
	PORTC->OPEND |= (1 << PIN4);	// open-drain
	PORTC->PULLU |= (1 << PIN4);	// pull-up
	PORT_Init(PORTC, PIN5, FUNMUX1_I2C1_SDA, 1);
	PORTC->OPEND |= (1 << PIN5);
	PORTC->PULLU |= (1 << PIN5);
	
	I2C_initStruct.Master = 1;
	I2C_initStruct.MstClk = 100000;
	I2C_initStruct.Addr10b = 0;
	I2C_initStruct.TXEmptyIEn = 0;
	I2C_initStruct.RXNotEmptyIEn = 0;
	I2C_Init(I2C1, &I2C_initStruct);
	I2C_Open(I2C1);
	
	GPIO_INIT(GPIOC, PIN7, GPIO_OUTPUT);		// PC7 -> OV_RST， 低电平有效
	GPIO_ClrBit(GPIOC, PIN7);
	SW_DelayMS(10);
	GPIO_SetBit(GPIOC, PIN7);
	SW_DelayMS(10);
	
	OV2640_Reset();
	
	uint16_t vid, pid;
	OV2640_ReadID(&vid, &pid);
	
	if((vid != 0x7FA2) || ((pid & 0xFFF0) != 0x2640))
		return false;
	
	
	for(int i = 0; i < sizeof(ov2640_svga_init_reg_tbl)/2; i++)
	{
	   	OV2640_WriteReg(ov2640_svga_init_reg_tbl[i][0], ov2640_svga_init_reg_tbl[i][1]);
 	}
	
	if(outfmt == OV_FMT_RGB565)
	{
		for(int i = 0; i < sizeof(ov2640_rgb565_reg_tbl)/2; i++)
		{
			OV2640_WriteReg(ov2640_rgb565_reg_tbl[i][0], ov2640_rgb565_reg_tbl[i][1]);
		}
	}
	else if(outfmt == OV_FMT_YUV422)
	{
		for(int i = 0; i < sizeof(ov2640_yuv422_reg_tbl)/2; i++)
		{
			OV2640_WriteReg(ov2640_yuv422_reg_tbl[i][0], ov2640_yuv422_reg_tbl[i][1]);
		}
	}
	else if(outfmt == OV_FMT_JPEG)
	{
		for(int i = 0; i < sizeof(ov2640_jpeg_reg_tbl)/2; i++)
		{
			OV2640_WriteReg(ov2640_jpeg_reg_tbl[i][0], ov2640_jpeg_reg_tbl[i][1]);
		}
	}
	
	OV2640_OutSize(width, height);
	OV2640_ClkDiv(clkdiv);
	
	return true;
}


void OV2640_Reset(void)
{
	OV2640_WriteReg(OV2640_DSP_RA_DLMT, 0x01);
	
	OV2640_WriteReg(OV2640_REG_COM7, 0x80);
}


void OV2640_ReadID(uint16_t * vid, uint16_t * pid)
{
	OV2640_WriteReg(OV2640_DSP_RA_DLMT, 0x01);
	
	*vid = (OV2640_ReadReg(OV2640_REG_MIDH) << 8) | OV2640_ReadReg(OV2640_REG_MIDL);
	*pid = (OV2640_ReadReg(OV2640_REG_PIDH) << 8) | OV2640_ReadReg(OV2640_REG_PIDL);
}


void OV2640_ColorBar(bool on)
{
	OV2640_WriteReg(OV2640_DSP_RA_DLMT, 0x01);
	
	uint8_t reg = OV2640_ReadReg(OV2640_REG_COM7);
	reg = (reg & (~(1 << 1))) | (on << 1);
	OV2640_WriteReg(OV2640_REG_COM7, reg);
}


bool OV2640_ClkDiv(uint8_t clkdiv)
{
	if(clkdiv > 64) return false;
	
	OV2640_WriteReg(OV2640_DSP_RA_DLMT, 0x01);
	
	uint8_t reg = OV2640_ReadReg(OV2640_REG_CLKRC);
	reg = (reg & 0xC0) | (clkdiv - 1);
	OV2640_WriteReg(OV2640_REG_CLKRC, reg);
	
	return true;
}


bool OV2640_OutSize(uint16_t width, uint16_t height)
{
	if(width%4)  return false;
	if(height%4) return false;
	
	width  = width/4;
	height = height/4;
	
	OV2640_WriteReg(OV2640_DSP_RA_DLMT, 0x00);
	
	OV2640_WriteReg(OV2640_DSP_RESET, 0x04);
	OV2640_WriteReg(OV2640_DSP_ZMOW, width&0xFF);
	OV2640_WriteReg(OV2640_DSP_ZMOH, height&0xFF);
	OV2640_WriteReg(OV2640_DSP_ZMHH, ((width>>8)&0x03) | ((height>>6)&0x04));
	OV2640_WriteReg(OV2640_DSP_RESET, 0x00);
	
	return true;
}


#define OV2640_I2C_ADDR	0x60

void OV2640_WriteReg(uint8_t reg_addr, uint8_t data)
{
	I2C_Start(I2C1, OV2640_I2C_ADDR | 0, 1);
	
	I2C_Write(I2C1, reg_addr, 1);
	
	I2C_Write(I2C1, data, 1);
	
	I2C_Stop(I2C1, 1);
}


uint8_t OV2640_ReadReg(uint8_t reg_addr)
{
	I2C_Start(I2C1, OV2640_I2C_ADDR | 0, 1);
	
	I2C_Write(I2C1, reg_addr, 1);
	
	SW_DelayUS(1);	// 不加延时无法发出 restart
	
	I2C_Start(I2C1, OV2640_I2C_ADDR | 1, 1);
	
	uint8_t data =  I2C_Read(I2C1, 0, 1);
	
	I2C_Stop(I2C1, 1);
	
	return data;
}
