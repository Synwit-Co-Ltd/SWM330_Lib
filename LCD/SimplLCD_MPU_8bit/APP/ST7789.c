#include "SWM330.h"

#include "ST7789.h"


/*******************************************************************************************************************************
* @brief	ST7789 init
* @param
* @return
*******************************************************************************************************************************/
void ST7789_Init(void)
{
	uint32_t id = MPULCD_ReadReg8(LCD, 0x04);	// dummy read
	id  = LCD->MPUDRB << 16;
	id |= LCD->MPUDRB <<  8;
	id |= LCD->MPUDRB <<  0;
	if(id != 0x858552)
	{
		printf("ID = %06X, not ST7789V\n", id);
	}
	
	
	MPULCD_WR_REG8(LCD, 0x11);		// Sleep out
	
	MPULCD_WR_REG8(LCD, 0x36);		// Memory Access Control
	MPULCD_WR_DATA8(LCD, 0x00);
	
	MPULCD_WR_REG8(LCD, 0x3A);		// Interface pixel format
	MPULCD_WR_DATA8(LCD, 0x05);
	
	//--------------------------------ST7789S Frame rate setting-------------------------
	MPULCD_WR_REG8(LCD, 0xb2);
	MPULCD_WR_DATA8(LCD, 0x0c);
	MPULCD_WR_DATA8(LCD, 0x0c);
	MPULCD_WR_DATA8(LCD, 0x00);
	MPULCD_WR_DATA8(LCD, 0x33);
	MPULCD_WR_DATA8(LCD, 0x33);
	
	MPULCD_WR_REG8(LCD, 0xb7);
	MPULCD_WR_DATA8(LCD, 0x35);
	
	//---------------------------------ST7789S Power setting-----------------------------
	MPULCD_WR_REG8(LCD, 0xbb);
	MPULCD_WR_DATA8(LCD, 0x20);
	
	MPULCD_WR_REG8(LCD, 0xc0);
	MPULCD_WR_DATA8(LCD, 0x2c);
	
	MPULCD_WR_REG8(LCD,0xc2);
	MPULCD_WR_DATA8(LCD, 0x01);
	
	MPULCD_WR_REG8(LCD, 0xc3);
	MPULCD_WR_DATA8(LCD, 0x0b);
	
	MPULCD_WR_REG8(LCD, 0xc4);
	MPULCD_WR_DATA8(LCD, 0x20);
	
	MPULCD_WR_REG8(LCD, 0xc6);
	MPULCD_WR_DATA8(LCD, 0x0f);
	
	MPULCD_WR_REG8(LCD, 0xd0);
	MPULCD_WR_DATA8(LCD, 0xa4);
	MPULCD_WR_DATA8(LCD, 0xa1);

	//--------------------------------ST7789S Gamma setting------------------------------
	MPULCD_WR_REG8(LCD, 0xe0);
	MPULCD_WR_DATA8(LCD, 0xd0);
	MPULCD_WR_DATA8(LCD, 0x03);
	MPULCD_WR_DATA8(LCD, 0x09);
	MPULCD_WR_DATA8(LCD, 0x0e);
	MPULCD_WR_DATA8(LCD, 0x11);
	MPULCD_WR_DATA8(LCD, 0x3d);
	MPULCD_WR_DATA8(LCD, 0x47);
	MPULCD_WR_DATA8(LCD, 0x55);
	MPULCD_WR_DATA8(LCD, 0x53);
	MPULCD_WR_DATA8(LCD, 0X1A);
	MPULCD_WR_DATA8(LCD, 0x16);
	MPULCD_WR_DATA8(LCD, 0x14);
	MPULCD_WR_DATA8(LCD, 0x1F);
	MPULCD_WR_DATA8(LCD, 0x22);
	
	MPULCD_WR_REG8(LCD, 0xe1);
	MPULCD_WR_DATA8(LCD, 0xd0);
	MPULCD_WR_DATA8(LCD, 0x02);
	MPULCD_WR_DATA8(LCD, 0x08);
	MPULCD_WR_DATA8(LCD, 0x0D);
	MPULCD_WR_DATA8(LCD, 0x12);
	MPULCD_WR_DATA8(LCD, 0x2c);
	MPULCD_WR_DATA8(LCD, 0x43);
	MPULCD_WR_DATA8(LCD, 0x55);
	MPULCD_WR_DATA8(LCD, 0x53);
	MPULCD_WR_DATA8(LCD, 0x1E);
	MPULCD_WR_DATA8(LCD, 0x1B);
	MPULCD_WR_DATA8(LCD, 0x19);
	MPULCD_WR_DATA8(LCD, 0x20);
	MPULCD_WR_DATA8(LCD, 0x22);
	
	MPULCD_WR_REG8(LCD, 0x29);
}


/*******************************************************************************************************************************
* @brief	ST7789 Set cursor position
* @param	(x, y) is new cursor position
* @return
*******************************************************************************************************************************/
void ST7789_SetCursor(uint16_t x, uint16_t y)
{
	MPULCD_WR_REG8(LCD, 0x2A);
	MPULCD_WR_DATA16(LCD, x);
	
	MPULCD_WR_REG8(LCD, 0x2B);
	MPULCD_WR_DATA16(LCD, y);
}


/*******************************************************************************************************************************
* @brief	ST7789 draw point
* @param	(x, y) is the point coord
* @param	rgb is the point color
* @return
*******************************************************************************************************************************/
void ST7789_DrawPoint(uint16_t x, uint16_t y, uint16_t rgb)
{
	ST7789_SetCursor(x, y);
	
	MPULCD_WR_REG8(LCD, 0x2C);
	MPULCD_WR_DATA16(LCD, rgb);
}


/*******************************************************************************************************************************
* @brief	clear screen to color rgb
* @param
* @return
*******************************************************************************************************************************/
void ST7789_Clear(uint16_t rgb)
{
	ST7789_SetCursor(0, 0);
	
	MPULCD_WR_REG8(LCD, 0x2C);
	
	for(int i = 0; i < LCD_VPIX; i++)
	{
		for(int j = 0; j < LCD_HPIX; j++)
		{
			MPULCD_WR_DATA16(LCD, rgb);
		}
	}
}


/*******************************************************************************************************************************
* @brief	NT35510 DMA write
* @param
* @return
* @note		After starting the transmission, return immediately without waiting for completion. completion query method:
*			method 1: use MPULCD_DMABusy() call
*			method 2: use LCD interrupt
*******************************************************************************************************************************/
void ST7789_DMAWrite(uint32_t * buff, uint32_t start_line, uint32_t count_line)
{
	ST7789_SetCursor(0, start_line);
	
	MPULCD_WR_REG8(LCD, 0x2C);
	
	MPULCD_DMAStart(LCD, buff, LCD_HPIX, count_line);
}
