#include "SWM330.h"
#include <string.h>


uint32_t * RAM_Buf = (uint32_t *)(RAM_BASE + 0x8000);
uint32_t * PSRAM_Buf = (uint32_t *)(PSRAMM_BASE + 0x10000);


void SerialInit(void);
void PSRAM_RAM_Copy(void);
void PSRAM_Flash_Copy(void);
void PSRAM_SPIFlash_Copy(void);
extern const uint16_t FlashData[0x10000];

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	RDMA_InitStructure RDMA_initStruct;
	RDMA_initStruct.BurstSize = RDMA_BURST_INC8;	// PSRAM Burst len is 32-byte, so word INC8
	RDMA_initStruct.BlockSize = RDMA_BLOCK_64;
	RDMA_initStruct.Interval  = CyclesPerUs;
	RDMA_initStruct.INTEn = 0;
	RDMA_Init(&RDMA_initStruct);
	
	PSRAM_RAM_Copy();
	
	PSRAM_Flash_Copy();
	
	PSRAM_SPIFlash_Copy();
	
	while(1==1)
	{
	}
}


void PSRAM_RAM_Copy(void)
{
	PORT_Init(PORTE, PIN9,  PORTE_PIN9_PSRAM_RST, 0);
	PORT_Init(PORTE, PIN10, PORTE_PIN10_PSRAM_CS, 0);
	PORT_Init(PORTE, PIN11, PORTE_PIN11_PSRAM_CK, 0);
	PORT_Init(PORTE, PIN8,  PORTE_PIN8_PSRAM_RWDS,1);
	PORT_Init(PORTE, PIN0,  PORTE_PIN0_PSRAM_D0,  1);
	PORT_Init(PORTE, PIN1,  PORTE_PIN1_PSRAM_D1,  1);
	PORT_Init(PORTE, PIN2,  PORTE_PIN2_PSRAM_D2,  1);
	PORT_Init(PORTE, PIN3,  PORTE_PIN3_PSRAM_D3,  1);
	PORT_Init(PORTE, PIN4,  PORTE_PIN4_PSRAM_D4,  1);
	PORT_Init(PORTE, PIN5,  PORTE_PIN5_PSRAM_D5,  1);
	PORT_Init(PORTE, PIN6,  PORTE_PIN6_PSRAM_D6,  1);
	PORT_Init(PORTE, PIN7,  PORTE_PIN7_PSRAM_D7,  1);
	
	PSRAM_InitStructure PSRAM_initStruct;
#ifndef PSRAM_XCCELA
	PSRAM_initStruct.RowSize = PSRAM_RowSize_1KB;
	PSRAM_initStruct.tRWR = 50;
	PSRAM_initStruct.tACC = 50;
	PSRAM_initStruct.tCSM = 4;
	PSRAM_Init(&PSRAM_initStruct);
#else
	PSRAM_initStruct.RowSize = PSRAM_RowSize_1KB;
	PSRAM_initStruct.tRC = 60;
	PSRAM_initStruct.tCEM = 8;
	PSRAM_Init(&PSRAM_initStruct);
#endif
	
#define N_WORD	(1024 * 64 / 4)

	for(int i = 0; i < N_WORD; i++)
		RAM_Buf[i] = i | (i << 16);
	
	RDMA_memcpy(PSRAM_Buf, RAM_Buf, RDMA_UNIT_WORD, N_WORD);
	while(RDMA_Remaining()) __NOP();
	
	for(int i = 0; i < N_WORD; i++)
		RAM_Buf[i] = 0;
	
	RDMA_memcpy(RAM_Buf, PSRAM_Buf, RDMA_UNIT_WORD, N_WORD);
	while(RDMA_Remaining()) __NOP();
	
	for(int i = 0; i < N_WORD; i++)
	{
		if((RAM_Buf[i] != PSRAM_Buf[i]) || (RAM_Buf[i] != (i | (i << 16))))
		{
			printf("PSRAM_RAM_Copy fail: expected=0x%08X, PSRAM=0x%08X, RAM=0x%08X\n", i | (i << 16), PSRAM_Buf[i], RAM_Buf[i]);
			
			return;
		}
	}
	
	printf("PSRAM_RAM_Copy pass!\n");
}


void PSRAM_SPIFlash_Copy(void)
{
	PORT_Init(PORTB, PIN5, PORTB_PIN5_QSPI0_CK, 0);
	PORT_Init(PORTB, PIN4, PORTB_PIN4_QSPI0_CS, 0);
	PORT_Init(PORTB, PIN3, PORTB_PIN3_QSPI0_D0, 1);
	PORT_Init(PORTB, PIN2, PORTB_PIN2_QSPI0_D1, 1);
	PORT_Init(PORTB, PIN1, PORTB_PIN1_QSPI0_D2, 1);
	PORT_Init(PORTB, PIN0, PORTB_PIN0_QSPI0_D3, 1);
	
	QSPI_InitStructure QSPI_initStruct;
	QSPI_initStruct.Size = QSPI_Size_16MB;
	QSPI_initStruct.ClkDiv = 4;
	QSPI_initStruct.ClkMode = QSPI_ClkMode_3;
	QSPI_initStruct.SampleShift = QSPI_SampleShift_NONE;
	QSPI_initStruct.IntEn = 0;
	QSPI_Init(QSPI0, &QSPI_initStruct);
	QSPI_Open(QSPI0);
	
	int id = QSPI_ReadJEDEC(QSPI0);
	printf("SPI Flash JEDEC: %06X\n", id);
	
	QSPI_QuadSwitch(QSPI0, 1);
	
#define QSPI_DATA_BASE	0x000000
#define PSRAM_HPtr	((uint16_t *)PSRAM_Buf)
	
	for(int addr = 0; addr < sizeof(FlashData); addr += 1024*64)
	{
		QSPI_Erase_Block64KB(QSPI0, QSPI_DATA_BASE + addr, 1);
	}
	
	for(int addr = 0; addr < sizeof(FlashData); addr += 256)
	{
		QSPI_Write_4bit(QSPI0, QSPI_DATA_BASE + addr, (uint8_t *)FlashData + addr, 256);
	}
	
	for(int i = 0; i < sizeof(FlashData)/4; i++) PSRAM_Buf[i] = 0;
	
	QSPI_DMAEnable(QSPI0, QSPI_Mode_IndirectRead);
	
	QSPI_Read_(QSPI0, QSPI_DATA_BASE, (uint8_t *)PSRAM_Buf, sizeof(FlashData), 1, 4, 0);
	
	RDMA_memcpy(PSRAM_Buf, (void *)&QSPI0->DRW, RDMA_UNIT_WORD, sizeof(FlashData)/4);
	while(RDMA_Remaining()) __NOP();
	
	QSPI_DMADisable(QSPI0);
	
	for(int i = 0; i < sizeof(FlashData)/2; i++)
	{
		if(PSRAM_HPtr[i] != FlashData[i])
		{
			printf("PSRAM_SPIFlash_Copy fail: expected=0x%04X, PSRAM=0x%04X\n", FlashData[i], PSRAM_HPtr[i]);
			
			return;
		}
	}
	
	printf("PSRAM_SPIFlash_Copy pass!\n");
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
