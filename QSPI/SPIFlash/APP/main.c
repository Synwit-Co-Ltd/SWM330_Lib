#include <string.h>
#include "SWM330.h"


#define EEPROM_ADDR	  0x008000


#define RWLEN  48
uint8_t WrBuff[RWLEN] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
						 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
						 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
uint8_t RdBuff[RWLEN] = {0};


void SerialInit(void);
void QSPI_Write_DMA(uint32_t addr, uint8_t buff[], uint32_t count, uint8_t data_width);
void QSPI_Read_DMA(uint32_t addr, uint8_t buff[], uint32_t count, uint8_t addr_width, uint8_t data_width);

int main(void)
{
	int i;
	QSPI_InitStructure QSPI_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	PORT_Init(PORTB, PIN5, PORTB_PIN5_QSPI0_CK, 0);
	PORT_Init(PORTB, PIN4, PORTB_PIN4_QSPI0_CS, 0);
	PORT_Init(PORTB, PIN3, PORTB_PIN3_QSPI0_D0, 1);
	PORT_Init(PORTB, PIN2, PORTB_PIN2_QSPI0_D1, 1);
	PORT_Init(PORTB, PIN1, PORTB_PIN1_QSPI0_D2, 1);
	PORT_Init(PORTB, PIN0, PORTB_PIN0_QSPI0_D3, 1);
	
	QSPI_initStruct.Size = QSPI_Size_16MB;
	QSPI_initStruct.ClkDiv = 4;
	QSPI_initStruct.ClkMode = QSPI_ClkMode_3;
	QSPI_initStruct.SampleShift = QSPI_SampleShift_NONE;
	QSPI_initStruct.IntEn = 0;
	QSPI_Init(QSPI0, &QSPI_initStruct);
	QSPI_Open(QSPI0);
	
	int id = QSPI_ReadJEDEC(QSPI0);
	printf("SPI Flash JEDEC: %06X\n", id);
	
	
	int quad = QSPI_QuadState(QSPI0);
	if(quad == 0)
	{
		QSPI_QuadSwitch(QSPI0, 1);
		
		quad = QSPI_QuadState(QSPI0);
	}
	printf("SPI Flash Quad %s\n", quad ? "enabled" : "disabled");
	
	
	QSPI_Erase(QSPI0, (QSPI_initStruct.Size > QSPI_Size_16MB) ? QSPI_C4B_ERASE_SECTOR : QSPI_CMD_ERASE_SECTOR, EEPROM_ADDR, 1);
	
	QSPI_Read(QSPI0, EEPROM_ADDR, RdBuff, RWLEN);
	
	printf("\n\nAfter Erase: \n");
	for(i = 0; i < RWLEN; i++) printf("0x%02X, ", RdBuff[i]);
	
	
	QSPI_Write(QSPI0, EEPROM_ADDR, WrBuff, RWLEN);
	
	QSPI_Read(QSPI0, EEPROM_ADDR, RdBuff, RWLEN);
	
	printf("\n\nAfter Write: \n");
	for(i = 0; i < RWLEN; i++) printf("0x%02X, ", RdBuff[i]);
	
	
	QSPI_Read_2bit(QSPI0, EEPROM_ADDR, RdBuff, RWLEN);
	
	printf("\n\nDual Read: \n");
	for(i = 0; i < RWLEN; i++) printf("0x%02X, ", RdBuff[i]);
	
	
	QSPI_Read_IO2bit(QSPI0, EEPROM_ADDR, RdBuff, RWLEN);
	
	printf("\n\nDual IO Read: \n");
	for(i = 0; i < RWLEN; i++) printf("0x%02X, ", RdBuff[i]);
	
	
	QSPI_Erase(QSPI0, (QSPI_initStruct.Size > QSPI_Size_16MB) ? QSPI_C4B_ERASE_SECTOR : QSPI_CMD_ERASE_SECTOR, EEPROM_ADDR, 1);
	QSPI_Write_4bit(QSPI0, EEPROM_ADDR, WrBuff, RWLEN);
	
	QSPI_Read_4bit(QSPI0, EEPROM_ADDR, RdBuff, RWLEN);
	
	printf("\n\nQuad Read: \n");
	for(i = 0; i < RWLEN; i++) printf("0x%02X, ", RdBuff[i]);
	
	
	QSPI_Read_IO4bit(QSPI0, EEPROM_ADDR, RdBuff, RWLEN);
	
	printf("\n\nQuad IO Read: \n");
	for(i = 0; i < RWLEN; i++) printf("0x%02X, ", RdBuff[i]);
	
	
	QSPI_Erase(QSPI0, (QSPI_initStruct.Size > QSPI_Size_16MB) ? QSPI_C4B_ERASE_SECTOR : QSPI_CMD_ERASE_SECTOR, EEPROM_ADDR, 1);
	QSPI_Write_DMA(EEPROM_ADDR, WrBuff, RWLEN, 4);
	
	QSPI_Read_DMA(EEPROM_ADDR, RdBuff, RWLEN, 4, 4);
	
	printf("\n\nDMA Read: \n");
	for(i = 0; i < RWLEN; i++) printf("0x%02X, ", RdBuff[i]);
	
	
	QSPI_MemoryMap(QSPI0, 4, 4);
	memcpy(RdBuff, (uint8_t *)(SFLASH_BASE + EEPROM_ADDR), RWLEN);
	
	printf("\n\nMemory Map Read: \n");
	for(i = 0; i < RWLEN; i++) printf("0x%02X, ", RdBuff[i]);
	
	
	/* 内存映射开启时，不能执行 QSPI_Erase()、QSPI_Write() 等函数 */
	QSPI_MemoryMapClose(QSPI0);
	
	QSPI_Erase(QSPI0, (QSPI_initStruct.Size > QSPI_Size_16MB) ? QSPI_C4B_ERASE_SECTOR : QSPI_CMD_ERASE_SECTOR, EEPROM_ADDR, 1);
	
	QSPI_MemoryMap(QSPI0, 4, 4);
	memcpy(RdBuff, (uint8_t *)(SFLASH_BASE + EEPROM_ADDR), RWLEN);
	
	printf("\n\nMemory Map Read: \n");
	for(i = 0; i < RWLEN; i++) printf("0x%02X, ", RdBuff[i]);


	/* 2MB 读写校验 */
#if 1
	uint32_t addr;
	uint32_t rwbuff[256 / 4];
	
	QSPI_MemoryMapClose(QSPI0);
	
	for(addr = 0; addr < 0x200000; addr += 1024 * 64)
	{
		QSPI_Erase(QSPI0, (QSPI_initStruct.Size > QSPI_Size_16MB) ? QSPI_C4B_ERASE_BLOCK64KB : QSPI_CMD_ERASE_BLOCK64KB, addr, 1);
	}
	
	for(addr = 0; addr < 0x200000; addr += 256)
	{
		for(i = 0; i < 256; i += 4)
			rwbuff[i / 4] = addr + i;
		
		QSPI_Write_4bit(QSPI0, addr, (uint8_t *)rwbuff, 256);
	}
	
	QSPI_CacheClear(QSPI0);
	QSPI_MemoryMap(QSPI0, 4, 4);
	
	for(addr = 0; addr < 0x200000; addr += 4)
	{
		if(*((uint32_t *)(SFLASH_BASE + addr)) != addr)
		{
			printf("\n\nError: expected 0x%08X, get 0x%08X\n", addr, *((uint32_t *)(SFLASH_BASE + addr)));
			break;
		}
	}
	
	if(addr == 0x200000)
		printf("\n\nPass\n");
#endif
	
	
	while(1==1)
	{
	}
}


void QSPI_Write_DMA(uint32_t addr, uint8_t buff[], uint32_t count, uint8_t data_width)
{
	static bool dma_inited = false;
	
	if(!dma_inited)
	{
		DMA_InitStructure DMA_initStruct;
		
		DMA_initStruct.Mode = DMA_MODE_SINGLE;
		DMA_initStruct.Unit = DMA_UNIT_BYTE;
		DMA_initStruct.Count = count;
		DMA_initStruct.MemoryAddr = (uint32_t)buff;
		DMA_initStruct.MemoryAddrInc = 1;
		DMA_initStruct.PeripheralAddr = (uint32_t)&QSPI0->DRB;
		DMA_initStruct.PeripheralAddrInc = 0;
		DMA_initStruct.Handshake = DMA_CH0_QSPI0TX;
		DMA_initStruct.Priority = DMA_PRI_LOW;
		DMA_initStruct.INTEn = 0;
		DMA_CH_Init(DMA_CH0, &DMA_initStruct);
		
		dma_inited = true;
	}
	
	QSPI_DMAEnable(QSPI0, QSPI_Mode_IndirectWrite);
	
	QSPI_Write_(QSPI0, addr, buff, count, data_width, 0);
	
	DMA_CH_Open(DMA_CH0);
	
	while(DMA_CH_INTStat(DMA_CH0, DMA_IT_DONE) == 0) __NOP();
	DMA_CH_INTClr(DMA_CH0, DMA_IT_DONE);
	
	/* 在 QSPI busy 时，写 QSPI->CR 寄存器无效 */
	while(QSPI_Busy(QSPI0)) __NOP();
	
	QSPI_DMADisable(QSPI0);
	
	while(QSPI_FlashBusy(QSPI0)) __NOP();
}


void QSPI_Read_DMA(uint32_t addr, uint8_t buff[], uint32_t count, uint8_t addr_width, uint8_t data_width)
{
	static bool dma_inited = false;
	
	if(!dma_inited)
	{
		DMA_InitStructure DMA_initStruct;
		
		DMA_initStruct.Mode = DMA_MODE_SINGLE;
		DMA_initStruct.Unit = DMA_UNIT_BYTE;
		DMA_initStruct.Count = count;
		DMA_initStruct.MemoryAddr = (uint32_t)buff;
		DMA_initStruct.MemoryAddrInc = 1;
		DMA_initStruct.PeripheralAddr = (uint32_t)&QSPI0->DRB;
		DMA_initStruct.PeripheralAddrInc = 0;
		DMA_initStruct.Handshake = DMA_CH1_QSPI0RX;
		DMA_initStruct.Priority = DMA_PRI_LOW;
		DMA_initStruct.INTEn = 0;
		DMA_CH_Init(DMA_CH1, &DMA_initStruct);
		
		dma_inited = true;
	}
	
	QSPI_DMAEnable(QSPI0, QSPI_Mode_IndirectRead);
	
	QSPI_Read_(QSPI0, addr, buff, count, addr_width, data_width, 0);
	
	DMA_CH_Open(DMA_CH1);
	
	while(DMA_CH_INTStat(DMA_CH1, DMA_IT_DONE) == 0) __NOP();
	DMA_CH_INTClr(DMA_CH1, DMA_IT_DONE);
	
	QSPI_DMADisable(QSPI0);
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN6, FUNMUX0_UART0_TXD, 0);
	PORT_Init(PORTA, PIN7, FUNMUX1_UART0_RXD, 1);
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThreshold = 3;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;
	UART_initStruct.TimeoutIEn = 0;
	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
	
	return ch;
}
