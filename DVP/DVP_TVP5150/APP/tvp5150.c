#include "SWM330.h"

#include "tvp5150.h"


#define	DEV_ADDR  0xBA


void TVP_HW_Init(void);

void TVP_Init(void)
{
	TVP_HW_Init();
	
	uint16_t devid = (TVP_Read(TVP_REG_DeviceMSB) << 8) +
					  TVP_Read(TVP_REG_DeviceLSB);
	if(devid != 0x5150)
		return;
	
	TVP_Write(TVP_REG_InputSource, TVP_InputSource_AIP1A);
	
	TVP_Write(TVP_REG_PinsConfig, TVP_PIN23_LOCK | TVP_PIN24_LOCK | TVP_PIN27_VBLK);
	TVP_Write(TVP_REG_MiscellaneousControls, 0xEF);
	
	TVP_Write(TVP_REG_OutputAndRatesSelect, TVP_FMT_BT656 | TVP_CODE_PLUS128 | TVP_RANGE_EXT);
}


void TVP_HW_Init(void)
{
	I2C_InitStructure I2C_initStruct;
	
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
}

void TVP_Write(uint8_t reg, uint8_t data)
{
	I2C_Start(I2C1, DEV_ADDR | 0, 1);
	
	I2C_Write(I2C1, reg, 1);
	
	I2C_Write(I2C1, data, 1);
	
	I2C_Stop(I2C1, 1);
}

uint8_t TVP_Read(uint8_t reg)
{
	I2C_Start(I2C1, DEV_ADDR | 0, 1);
	
	I2C_Write(I2C1, reg, 1);
	
	SW_DelayUS(1);	// 不加延时无法发出 restart
	
	I2C_Start(I2C1, DEV_ADDR | 1, 1);
	
	uint8_t data = I2C_Read(I2C1, 0, 1);
	
	I2C_Stop(I2C1, 1);
	
	return data;
}
