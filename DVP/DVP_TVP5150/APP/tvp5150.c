#include "SWM330.h"

#include "tvp5150.h"


#define	DEV_ADDR  0xB8


void TVP_HW_Init(void);

void TVP_Init(void)
{
	TVP_HW_Init();
	
	uint16_t devid = (TVP_Read(TVP_REG_DeviceMSB) << 8) +
					  TVP_Read(TVP_REG_DeviceLSB);
	if(devid != 0x5150)
		return;
	
	TVP_Write(TVP_REG_InputSource, TVP_InputSource_AIP1A);
	
	TVP_Write(TVP_REG_PinsConfig, TVP_PIN23_FID | TVP_PIN24_VSYNC | TVP_PIN27_VBLK);
	TVP_Write(TVP_REG_MiscellaneousControls, 0xEF);
	
	TVP_Write(TVP_REG_OutputAndRatesSelect, TVP_FMT_YUV422 | TVP_CODE_RAW | TVP_RANGE_EXT);
}


void TVP_HW_Init(void)
{
	I2C_InitStructure I2C_initStruct;
	
	PORT_Init(PORTA, PIN12, FUNMUX0_I2C0_SCL, 1);
	PORTA->OPEND |= (1 << PIN12);	// open-drain
	PORTA->PULLU |= (1 << PIN12);	// pull-up
	PORT_Init(PORTA, PIN13, FUNMUX1_I2C0_SDA, 1);
	PORTA->OPEND |= (1 << PIN13);
	PORTA->PULLU |= (1 << PIN13);
	
	I2C_initStruct.Master = 1;
	I2C_initStruct.MstClk = 100000;
	I2C_initStruct.Addr10b = 0;
	I2C_initStruct.TXEmptyIEn = 0;
	I2C_initStruct.RXNotEmptyIEn = 0;
	I2C_Init(I2C0, &I2C_initStruct);
	
	I2C_Open(I2C0);
}

void TVP_Write(uint8_t reg, uint8_t data)
{
	I2C_Start(I2C0, DEV_ADDR | 0, 1);
	I2C_Write(I2C0, reg, 1);
	I2C_Write(I2C0, data, 1);
}

uint8_t TVP_Read(uint8_t reg)
{
	I2C_Start(I2C0, DEV_ADDR | 0, 1);
	I2C_Write(I2C0, reg, 1);
	
	I2C_Start(I2C0, DEV_ADDR | 1, 1);
	return I2C_Read(I2C0, 1, 1);
}
