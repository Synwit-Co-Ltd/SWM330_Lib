#include "SWM330.h"

#define OV_ARRAY
#include "OV2640.h"


bool OV2640_Init(uint8_t outfmt, uint8_t clkdiv, uint16_t width, uint16_t height)
{
	I2C_InitStructure  I2C_initStruct;
	
	PORT_Init(PORTE, PIN14, FUNMUX0_I2C1_SCL, 1);
	PORTE->OPEND |= (1 << PIN14);	// open-drain
	PORTE->PULLU |= (1 << PIN14);	// pull-up
	PORT_Init(PORTE, PIN15, FUNMUX1_I2C1_SDA, 1);
	PORTE->OPEND |= (1 << PIN15);
	PORTE->PULLU |= (1 << PIN15);
	
	I2C_initStruct.Master = 1;
	I2C_initStruct.MstClk = 100000;
	I2C_initStruct.Addr10b = 0;
	I2C_initStruct.TXEmptyIEn = 0;
	I2C_initStruct.RXNotEmptyIEn = 0;
	I2C_Init(I2C1, &I2C_initStruct);
	I2C_Open(I2C1);
	
	GPIO_INIT(GPIOA, PIN14, GPIO_OUTPUT);		// PA14 -> OV_PWDN，高电平有效
	GPIO_ClrBit(GPIOA, PIN14);
	GPIO_INIT(GPIOA, PIN15, GPIO_OUTPUT);		// PA15 -> OV_RST， 低电平有效
	GPIO_ClrBit(GPIOA, PIN15);
	for(int i = 0; i < SystemCoreClock/1000; i++) __NOP();
	GPIO_SetBit(GPIOA, PIN15);
	for(int i = 0; i < SystemCoreClock/1000; i++) __NOP();
	
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
	else
	{
		for(int i = 0; i < sizeof(ov2640_yuv422_reg_tbl)/2; i++)
		{
			OV2640_WriteReg(ov2640_yuv422_reg_tbl[i][0], ov2640_yuv422_reg_tbl[i][1]);
		}
		
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
	uint8_t reg = OV2640_ReadReg(OV2640_REG_COM7);
	reg = (reg & (~(1 << 1))) | (on << 1);
	OV2640_WriteReg(OV2640_REG_COM7, reg);
}


bool OV2640_ClkDiv(uint8_t clkdiv)
{
	if(clkdiv > 64) return false;
	
	OV2640_WriteReg(0XFF, 0X01);
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
	
	OV2640_WriteReg(0XFF, 0X00);
	OV2640_WriteReg(0XE0, 0X04);
	OV2640_WriteReg(0X5A, width&0XFF);
	OV2640_WriteReg(0X5B, height&0XFF);
	OV2640_WriteReg(0X5C, ((width>>8)&0X03) | ((height>>6)&0X04));
	OV2640_WriteReg(0XE0, 0X00);
	
	return true;
}


const static uint8_t OV2640_AUTOEXPOSURE_LEVEL[5][8]=
{
	{
		0xFF,0x01,
		0x24,0x20,
		0x25,0x18,
		0x26,0x60,
	},
	{
		0xFF,0x01,
		0x24,0x34,
		0x25,0x1c,
		0x26,0x00,
	},
	{
		0xFF,0x01,
		0x24,0x3e,
		0x25,0x38,
		0x26,0x81,
	},
	{
		0xFF,0x01,
		0x24,0x48,
		0x25,0x40,
		0x26,0x81,
	},
	{
		0xFF,0x01,
		0x24,0x58,
		0x25,0x50,
		0x26,0x92,
	},
};

//level:0~4
void OV2640_auto_exposure(uint8_t level)
{
	uint8_t *p = (uint8_t *)OV2640_AUTOEXPOSURE_LEVEL[level];
	
	for(int i = 0; i < 4; i++)
	{
		OV2640_WriteReg(p[i*2], p[i*2+1]);
	}
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
	uint8_t data;
	
	I2C_Start(I2C1, OV2640_I2C_ADDR | 0, 1);

	I2C_Write(I2C1, reg_addr, 1);
	
	I2C_Start(I2C1, OV2640_I2C_ADDR | 1, 1);

	data =  I2C_Read(I2C1, 0, 1);
 
	I2C_Stop(I2C1, 1);
	
	return data;
}
