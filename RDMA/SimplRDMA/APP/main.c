#include "SWM330.h"
#include <string.h>


uint32_t * RAM_Buf = (uint32_t *)(RAM_BASE + 0x8000);
uint32_t * PSRAM_Buf = (uint32_t *)(PSRAMM_BASE + 0x10000);


void SerialInit(void);
void PSRAM_RAM_Copy(void);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	RDMA_InitStructure RDMA_initStruct;
	RDMA_initStruct.BurstSize = RDMA_BURST_INC16;
	RDMA_initStruct.BlockSize = RDMA_BLOCK_64;
	RDMA_initStruct.Interval  = CyclesPerUs;
	RDMA_initStruct.Handshake = RDMA_HANDSHAKE_NO;
	RDMA_initStruct.INTEn = 0;
	RDMA_Init(&RDMA_initStruct);
	
	PSRAM_RAM_Copy();
	
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
	PSRAM_initStruct.RowSize = PSRAM_RowSize_1KB;
	PSRAM_initStruct.tRWR = 50;
	PSRAM_initStruct.tACC = 50;
	PSRAM_initStruct.tCSM = 4;
	PSRAM_Init(&PSRAM_initStruct);
	
	printf("HyperRAM ID0 = 0x%04X, ID1 = 0x%04X\n", (PSRAMC->IR & PSRAMC_IR_ID0_Msk) >> PSRAMC_IR_ID0_Pos,
													(PSRAMC->IR & PSRAMC_IR_ID1_Msk) >> PSRAMC_IR_ID1_Pos);
	
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
