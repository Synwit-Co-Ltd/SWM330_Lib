#ifndef __ST7789_H__
#define __ST7789_H__


#define LCD_VPIX  320
#define LCD_HPIX  240


void ST7789_Init(void);
void ST7789_Clear(uint16_t rgb);
void ST7789_SetCursor(uint16_t x, uint16_t y);
void ST7789_DrawPoint(uint16_t x, uint16_t y, uint16_t rgb);

void ST7789_DMAWrite(uint32_t * buff, uint32_t start_line, uint32_t count_line);


#endif
