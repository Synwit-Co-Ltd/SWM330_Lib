#include "SWM330.h"


uint32_t WordBuffer[20] = {0x14141414, 0x15151515, 0x16161616, 0x17171717, 0x18181818, 0x19191919, 0x1A1A1A1A, 0x1B1B1B1B, 0x1C1C1C1C, 0x1D1D1D1D,
				           0x1E1E1E1E, 0x1F1F1F1F, 0x20202020, 0x21212121, 0x22222222, 0x23232323, 0x24242424, 0x25252525, 0x26262626, 0x27272727};

uint16_t HalfBuffer[20] = {0x1414,     0x1515,     0x1616,     0x1717,     0x1818,     0x1919,     0x1A1A,     0x1B1B,     0x1C1C,     0x1D1D,    
				           0x1E1E,     0x1F1F,     0x2020,     0x2121,     0x2222,     0x2323,     0x2424,     0x2525,     0x2626,     0x2727};

uint8_t  ByteBuffer[20] = {0x14,       0x15,       0x16,       0x17,       0x18,       0x19,       0x1A,       0x1B,       0x1C,       0x1D,      
				           0x1E,       0x1F,       0x20,       0x21,       0x22,       0x23,       0x24,       0x25,       0x26,       0x27};

void WordTest(uint32_t addr, uint32_t *buff, uint32_t size);
void HalfTest(uint32_t addr, uint16_t *buff, uint32_t size);
void ByteTest(uint32_t addr, uint8_t  *buff, uint32_t size);
void SerialInit(void);

int main(void)
{
	uint32_t i, val;
	PSRAM_InitStructure PSRAM_initStruct;
	
	SystemInit();
	
	SerialInit();
	
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
	
#ifdef PSRAM_XCCELA
	PORTE->PULLD |= (1 << PIN8);
#endif
	
#ifndef PSRAM_XCCELA
	PSRAM_initStruct.RowSize = PSRAM_RowSize_1KB;
	PSRAM_initStruct.tRWR = 50;
	PSRAM_initStruct.tACC = 50;
	PSRAM_initStruct.tCSM = 4;
	PSRAM_Init(&PSRAM_initStruct);
	
	printf("HyperRAM ID0 = 0x%04X, ID1 = 0x%04X\n", (PSRAMC->IR & PSRAMC_IR_ID0_Msk) >> PSRAMC_IR_ID0_Pos,
													(PSRAMC->IR & PSRAMC_IR_ID1_Msk) >> PSRAMC_IR_ID1_Pos);
#else
	PSRAM_initStruct.RowSize = PSRAM_RowSize_1KB;
	PSRAM_initStruct.tRC = 60;
	PSRAM_initStruct.tCEM = 8;
	PSRAM_Init(&PSRAM_initStruct);
#endif
	WordTest(PSRAMM_BASE, WordBuffer, sizeof(WordBuffer)/4);
	WordTest(PSRAMM_BASE+0x100000, WordBuffer, sizeof(WordBuffer)/4);
	WordTest(PSRAMM_BASE+0x110001, WordBuffer, sizeof(WordBuffer)/4);
	
	HalfTest(PSRAMM_BASE, HalfBuffer, sizeof(HalfBuffer)/2);
	HalfTest(PSRAMM_BASE+0x200000, HalfBuffer, sizeof(HalfBuffer)/2);
	HalfTest(PSRAMM_BASE+0x210000+1, HalfBuffer, sizeof(HalfBuffer)/2);
	
	ByteTest(PSRAMM_BASE, ByteBuffer, sizeof(ByteBuffer)/1);
	ByteTest(PSRAMM_BASE+0x3210, ByteBuffer, sizeof(ByteBuffer)/1);
	ByteTest(PSRAMM_BASE+0x1235, ByteBuffer, sizeof(ByteBuffer)/1);
	

#if 1  // PSRAM Chip Test
#define PSRAM_SIZE  (0x100000 * 2)

	/* Word Test */
	for(i = 0; i < PSRAM_SIZE; i += 4)
	{
		*((volatile uint32_t *)(PSRAMM_BASE + i)) = i;
	}
	
	for(i = 0; i < PSRAM_SIZE; i += 4)
	{
		val = *((volatile uint32_t *)(PSRAMM_BASE + i));
		if(val != i)
		{
			printf("Word Test Error: 0x%08X expected, 0x%08X get\r\n", i, val);
			while(1);
		}
	}
	printf("Word Test Pass!\r\n");
	
	/* Half Test */
	for(i = 0; i < PSRAM_SIZE; i += 2)
	{
		*((volatile uint16_t *)(PSRAMM_BASE + i)) = i&0xFFFF;
	}
	
	for(i = 0; i < PSRAM_SIZE; i += 2)
	{
		val = *((volatile uint16_t *)(PSRAMM_BASE + i));
		if(val != (i&0xFFFF))
		{
			printf("Half Test Error: 0x%04X expected, 0x%04X get\r\n", (i&0xFFFF), val);
			while(1);
		}
	}
	printf("Half Test Pass!\r\n");
	
	/* Byte Test */
	for(i = 0; i < PSRAM_SIZE; i += 1)
	{
		*((volatile uint8_t *)(PSRAMM_BASE + i)) = i&0xFF;
	}
	
	for(i = 0; i < PSRAM_SIZE; i += 1)
	{
		val = *((volatile uint8_t *)(PSRAMM_BASE + i));
		if(val != (i&0xFF))
		{
			printf("Byte Test Error: 0x%02X expected, 0x%02X get\r\n", (i&0xFF), val);
			while(1);
		}
	}
	printf("Byte Test Pass!\r\n");
#endif

	while(1==1)
	{
	}
}


void WordTest(uint32_t addr, uint32_t *buff, uint32_t size)
{
	uint32_t i;
#define SDRW  ((volatile uint32_t *)addr)
	
	printf("RW @ 0x%08X\r\n", addr);

	for(i = 0; i < size; i++)  SDRW[i] = 0x00000000;
	
	printf("\r\nAfter Erase: \r\n");
	for(i = 0; i < size; i++)  printf("0x%08X, ",  SDRW[i]);
	
	for(i = 0; i < size; i++)  SDRW[i] = buff[i];
	
	printf("\r\nAfter Write: \r\n");
	for(i = 0; i < size; i++)  printf("0x%08X, ",  SDRW[i]);
	
	printf("\r\n\r\n\r\n");
}

void HalfTest(uint32_t addr, uint16_t *buff, uint32_t size)
{
	uint32_t i;
#define SDRH  ((volatile uint16_t *)addr)
	
	printf("RW @ 0x%08X\r\n", addr);

	for(i = 0; i < size; i++)  SDRH[i] = 0x0000;
	
	printf("\r\nAfter Erase: \r\n");
	for(i = 0; i < size; i++)  printf("0x%04X, ",  SDRH[i]);
	
	for(i = 0; i < size; i++)  SDRH[i] = buff[i];
	
	printf("\r\nAfter Write: \r\n");
	for(i = 0; i < size; i++)  printf("0x%04X, ",  SDRH[i]);
	
	printf("\r\n\r\n\r\n");
}

void ByteTest(uint32_t addr, uint8_t  *buff, uint32_t size)
{
	uint32_t i;
#define SDRB  ((volatile uint8_t *)addr)
	
	printf("RW @ 0x%08X\r\n", addr);

	for(i = 0; i < size; i++)  SDRB[i] = 0x00;
	
	printf("\r\nAfter Erase: \r\n");
	for(i = 0; i < size; i++)  printf("0x%02X, ",  SDRB[i]);
	
	for(i = 0; i < size; i++)  SDRB[i] = buff[i];
	
	printf("\r\nAfter Write: \r\n");
	for(i = 0; i < size; i++)  printf("0x%02X, ",  SDRB[i]);
	
	printf("\r\n\r\n\r\n");
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
