#include "SWM330.h"
#include "ST77903.h"


static uint16_t *LCD_VideoMemory = 0;

static uint32_t  LCD_LineCounter = 0;


typedef struct {
    uint8_t cmd;
    uint8_t len;
    uint8_t dat[16];
} init_line_t;

static const init_line_t init_table[] = {
	{ 0xF0,  1, 0xC3 },
	{ 0xF0,  1, 0x96 },
	{ 0xF0,  1, 0xA5 },
	{ 0xE9,  1, 0x20 },
	{ 0xE7,  4, 0x80, 0x77, 0x1F, 0xCC },
	{ 0xC1,  4, 0x77, 0x07, 0xCF, 0x16 },
	{ 0xC2,  4, 0x77, 0x07, 0xCF, 0x16 },
	{ 0xC3,  4, 0x22, 0x02, 0x22, 0x04 },
	{ 0xC4,  4, 0x22, 0x02, 0x22, 0x04 },
	{ 0xC5,  1, 0xED },
	{ 0xE0, 14, 0x87, 0x09, 0x0C, 0x06, 0x05, 0x03, 0x29, 0x32, 0x49, 0x0F, 0x1B, 0x17, 0x2A, 0x2F },
	{ 0xE1, 14, 0x87, 0x09, 0x0C, 0x06, 0x05, 0x03, 0x29, 0x32, 0x49, 0x0F, 0x1B, 0x17, 0x2A, 0x2F },
	{ 0xE5, 14, 0xBE, 0xF5, 0xB1, 0x22, 0x22, 0x25, 0x10, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 },
	{ 0xE6, 14, 0xBE, 0xF5, 0xB1, 0x22, 0x22, 0x25, 0x10, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 },
	{ 0xEC,  2, 0x40, 0x03 },
	{ 0xB2,  1, 0x00 },
	{ 0xB3,  1, 0x01 },
	{ 0xB4,  1, 0x00 },
	{ 0xB5,  4, 0x00, 0x08, 0x00, 0x08 },
	{ 0xA5,  9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x2A, 0x8A, 0x02 },
	{ 0xA6,  9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x2A, 0x8A, 0x02 },
	{ 0xBA,  7, 0x0A, 0x5A, 0x23, 0x10, 0x25, 0x02, 0x00 },
	{ 0xBB,  8, 0x00, 0x30, 0x00, 0x2C, 0x82, 0x87, 0x18, 0x00 },
	{ 0xBC,  8, 0x00, 0x30, 0x00, 0x2C, 0x82, 0x87, 0x18, 0x00 },
	{ 0xBD, 11, 0xA1, 0xB2, 0x2B, 0x1A, 0x56, 0x43, 0x34, 0x65, 0xFF, 0xFF, 0x0F },
	{ 0x35,  1, 0x00 },
	{ 0x36,  1, 0x0C },
	{ 0x3A,  1, 0x05 },		// 0x05: RGB565   0x06: RGB666   0x07: RGB888
	{ 0x21,  0,  },
	{ 0x11,  0,  },
	{ 0xFF,  1, 120},		// if command is 0xFF, means a delay time unit ms
	{ 0x29,  0,  },
	{ 0xFF,  1, 120},

//	color bar
//	{ 0xB0,  1, 0xA5 },
//	{ 0xCC,  9, 0x40, 0x00, 0x3F, 0x00, 0x0A, 0x0A, 0xEA, 0xEA, 0x03 },
};


static void delayMS(uint32_t ms)
{
	int stop = CyclesPerUs * 1000 * ms / 4;
	
	for(int i = 0; i < stop; i++)  __NOP();
}


int ST77903_Init(void)
{
	QSPI_InitStructure QSPI_initStruct;
	
	PORT_Init(PORTA, PIN0,  PORTA_PIN0_QSPI1_CK,  0);
	PORT_Init(PORTB, PIN15, PORTB_PIN15_QSPI1_CS, 0);
	PORT_Init(PORTB, PIN14, PORTB_PIN14_QSPI1_D0, 1);
	PORT_Init(PORTB, PIN13, PORTB_PIN13_QSPI1_D1, 1);
	PORT_Init(PORTB, PIN12, PORTB_PIN12_QSPI1_D2, 1);
	PORT_Init(PORTB, PIN11, PORTB_PIN11_QSPI1_D3, 1);
	
	QSPI_initStruct.Size = QSPI_Size_16MB;
	QSPI_initStruct.ClkDiv = 4;
	QSPI_initStruct.ClkMode = QSPI_ClkMode_3;
	QSPI_initStruct.SampleShift = QSPI_SampleShift_NONE;
	QSPI_initStruct.IntEn = 0;
	QSPI_Init(QSPI1, &QSPI_initStruct);
	QSPI_Open(QSPI1);
	
	GPIO_INIT(GPIOD, PIN11, GPIO_OUTPUT);
#define ST77903_RST_Low()	GPIO_ClrBit(GPIOD, PIN11)
#define ST77903_RST_High()	GPIO_SetBit(GPIOD, PIN11)

	ST77903_RST_Low();
	delayMS(100);
	ST77903_RST_High();
	delayMS(100);
	
	init_line_t *init = (init_line_t *)&init_table[0];

    for(int i = 0; i < sizeof(init_table)/sizeof(init_line_t); i++, init++)
    {
        if(init->cmd == 0xff)
        {
            delayMS(init->dat[0]);
        }
        else
        {
            ST77903_WriteCmd(init->cmd, init->dat, init->len);
        }
    }

	
	QSPI1->CR |= QSPI_CR_DMAEN_Msk;
	
	DMA_InitStructure DMA_initStruct;
	
	DMA_initStruct.Mode = DMA_MODE_SINGLE;
	DMA_initStruct.Unit = DMA_UNIT_HALFWORD;
	DMA_initStruct.Count = 0;
	DMA_initStruct.MemoryAddr = 0;
	DMA_initStruct.MemoryAddrInc = 1;
	DMA_initStruct.PeripheralAddr = (uint32_t)&QSPI1->DRH;
	DMA_initStruct.PeripheralAddrInc = 0;
	DMA_initStruct.Handshake = DMA_CH1_QSPI1TX;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = 0;
	DMA_CH_Init(DMA_CH1, &DMA_initStruct);
	
	float dotsPerUs = (CyclesPerUs / QSPI_initStruct.ClkDiv / (16 / 4.0));
	
	TIMR_Init(BTIMR3, TIMR_MODE_TIMER, CyclesPerUs, LCD_HDOT / dotsPerUs + 20, 1);
	
	return 1;
}


void ST77903_WriteCmd(uint8_t cmd, uint8_t para[], uint32_t len)
{
	QSPI_CmdStructure cmdStruct;
	QSPI_CmdStructClear(&cmdStruct);
	
	cmdStruct.InstructionMode 	 = QSPI_PhaseMode_1bit;
	cmdStruct.Instruction 		 = 0xDE;
	cmdStruct.AddressMode 		 = QSPI_PhaseMode_1bit;
	cmdStruct.AddressSize		 = QSPI_PhaseSize_24bit;
	cmdStruct.Address			 = cmd << 8;
	cmdStruct.AlternateBytesMode = QSPI_PhaseMode_None;
	cmdStruct.DummyCycles 		 = 0;
	cmdStruct.DataMode 			 = len ? QSPI_PhaseMode_1bit : QSPI_PhaseMode_None;
	cmdStruct.DataCount			 = len;
	
	QSPI_Command(QSPI1, QSPI_Mode_IndirectWrite, &cmdStruct);
	
	for(int i = 0; i < len; i++)
	{
		while(QSPI_FIFOSpace(QSPI1) < 1) __NOP();
		
		QSPI1->DRB = para[i];
	}
	
	while(QSPI_Busy(QSPI1)) __NOP();
}


void ST77903_ReadCmd(uint8_t cmd, uint8_t para[], uint32_t len)
{
	QSPI_CmdStructure cmdStruct;
	QSPI_CmdStructClear(&cmdStruct);
	
	cmdStruct.InstructionMode 	 = QSPI_PhaseMode_1bit;
	cmdStruct.Instruction 		 = 0xDD;
	cmdStruct.AddressMode 		 = QSPI_PhaseMode_1bit;
	cmdStruct.AddressSize		 = QSPI_PhaseSize_24bit;
	cmdStruct.Address			 = cmd << 8;
	cmdStruct.AlternateBytesMode = QSPI_PhaseMode_None;
	cmdStruct.DummyCycles 		 = 0;
	cmdStruct.DataMode 			 = QSPI_PhaseMode_1bit;
	cmdStruct.DataCount			 = len;
	
	QSPI1->CR &= ~QSPI_CR_BIDI_Msk;
		
	QSPI_Command(QSPI1, QSPI_Mode_IndirectRead, &cmdStruct);
	
	for(int i = 0; i < len; i++)
	{
		while(QSPI_FIFOCount(QSPI1) < 1) __NOP();
		
		 para[i] = QSPI1->DRB;
	}
		
	QSPI1->CR |=  QSPI_CR_BIDI_Msk;
}


void ST77903_SendLine(uint16_t data[])
{	
	QSPI_CmdStructure cmdStruct;
	QSPI_CmdStructClear(&cmdStruct);
	
	cmdStruct.InstructionMode 	 = QSPI_PhaseMode_1bit;
	cmdStruct.Instruction 		 = 0xDE;
	cmdStruct.AddressMode 		 = QSPI_PhaseMode_1bit;
	cmdStruct.AddressSize		 = QSPI_PhaseSize_24bit;
	cmdStruct.Address			 = 0x60 << 8;
	cmdStruct.AlternateBytesMode = QSPI_PhaseMode_None;
	cmdStruct.DummyCycles 		 = 0;
	cmdStruct.DataMode 			 = QSPI_PhaseMode_4bit;
	cmdStruct.DataCount			 = LCD_HDOT * 2;
	
	QSPI_Command(QSPI1, QSPI_Mode_IndirectWrite, &cmdStruct);
	
	DMA_CH_SetAddrAndCount(DMA_CH1, (uint32_t)data, LCD_HDOT);
	
	DMA_CH_Open(DMA_CH1);
}


void ST77903_Start(uint16_t data[])
{
	LCD_VideoMemory = data;
	
	LCD_LineCounter = 0;
	
	TIMR_Start(BTIMR3);
}


void ST77903_Stop(void)
{
	TIMR_Stop(BTIMR3);
	
	DMA_CH_Close(DMA_CH1);
	
	QSPI1->CR |= QSPI_CR_ABORT_Msk;
}


void BTIMR3_Handler(void)
{
	TIMR_INTClr(BTIMR3, TIMR_IT_TO);
	
	if(LCD_LineCounter == 0)
	{
		ST77903_WriteCmd(0x61, 0, 0);	// VSW
	}
	else if(LCD_LineCounter < 7)
	{
		ST77903_WriteCmd(0x60, 0, 0);	// VBP
	}
	else if(LCD_LineCounter < 7 + LCD_VDOT)
	{
		ST77903_SendLine(&LCD_VideoMemory[(LCD_LineCounter - 7) * LCD_HDOT]);
	}
	else if(LCD_LineCounter < 7 + LCD_VDOT + 6)
	{
		ST77903_WriteCmd(0x60, 0, 0);	// VFP
	}
	
	LCD_LineCounter = (LCD_LineCounter + 1) % (LCD_VDOT + 20);
}
