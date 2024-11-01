#ifndef __ST77903_h__
#define __ST77903_h__


#define LCD_HDOT	400
#define LCD_VDOT	400


#define ST77903_CMD_SWRST		0x01	// Software Reset
#define ST77903_CMD_RDDID		0x04	// Read Display ID
#define ST77903_CMD_RDDST		0x09	// Read Display Status
#define ST77903_CMD_DISP_OFF	0x28
#define ST77903_CMD_DISP_ON		0x29
#define ST77903_CMD_VSYNC		0x61
#define ST77903_CMD_HSYNC		0x60


int ST77903_Init(void);
void ST77903_Start(uint16_t data[]);
void ST77903_Stop(void);

void ST77903_WriteCmd(uint8_t cmd, uint8_t para[], uint32_t len);
void ST77903_ReadCmd(uint8_t cmd, uint8_t para[], uint32_t len);


#endif // __ST77903_h__
