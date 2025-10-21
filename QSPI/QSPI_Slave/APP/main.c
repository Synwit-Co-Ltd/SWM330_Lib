#include "SWM330.h"


#define N_HALF	1024
uint16_t Wrbuf[N_HALF] = { 0 };
uint16_t * Rxbuf = (uint16_t *)(PSRAMM_BASE + 0x10000);


void SerialInit(void);
void MemoryInit(void);
void QSPI0_Slave_Init(void);
void QSPI0_Slave_Receive(uint8_t *buff, uint32_t count);
void QSPI1_Master_Init(void);
void QSPI1_Master_Write(uint8_t *buff, uint32_t count);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	MemoryInit();
	
	QSPI1_Master_Init();
	
	QSPI0_Slave_Init();		// only QSPI0 support slave mode
	
	QSPI0_Slave_Receive((uint8_t *)Rxbuf, N_HALF*2);
	
	for(int i = 0; i < N_HALF; i++)
		Wrbuf[i] = i;
	
	while(1==1)
	{
		QSPI1_Master_Write((uint8_t *)Wrbuf, N_HALF*2);
		
		for(int i = 0; i < SystemCoreClock; i++) __NOP();
		
		for(int i = 0; i < N_HALF; i++)
			Wrbuf[i] += 1;
	}
}


void RDMA_Handler(void)
{
	RDMA_INTClr(RDMA_IT_DONE);
	
	int i = 0;
	for(i = 0; i < N_HALF; i++)
	{
		if(Rxbuf[i] != Wrbuf[i])
			break;
	}
	
	if(i == N_HALF)
		printf("QSPI Slave Rx pass!\n");
	else
		printf("QSPI Slave Rx fail!\n");
	
	QSPI0_Slave_Receive((uint8_t *)Rxbuf, N_HALF*2);
}


void QSPI0_Slave_Init(void)
{
	QSPI_InitStructure QSPI_initStruct;
	
	PORT_Init(PORTB, PIN5, PORTB_PIN5_QSPI0_CK, 1);		// slave, input enable
	PORT_Init(PORTB, PIN4, PORTB_PIN4_QSPI0_CS, 1);		// slave, input enable
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
	
	QSPI0->CR |= QSPI_CR_SLAVE_Msk;
	
	QSPI_DMAEnable(QSPI0, QSPI_Mode_IndirectRead);
	
	QSPI_Open(QSPI0);
	
	
	RDMA_InitStructure RDMA_initStruct;
	RDMA_initStruct.BurstSize = RDMA_BURST_INC8;	// PSRAM Burst len is 32-byte, so word INC8
	RDMA_initStruct.BlockSize = RDMA_BLOCK_64;
	RDMA_initStruct.Interval  = CyclesPerUs;
	RDMA_initStruct.INTEn = RDMA_IT_DONE;
	RDMA_Init(&RDMA_initStruct);
}


void QSPI0_Slave_Receive(uint8_t *buff, uint32_t count)
{
	QSPI_CmdStructure cmdStruct;
	QSPI_CmdStructClear(&cmdStruct);
	cmdStruct.InstructionMode 	 = QSPI_PhaseMode_None;
	cmdStruct.AddressMode 		 = QSPI_PhaseMode_None;
	cmdStruct.AlternateBytesMode = QSPI_PhaseMode_None;
	cmdStruct.DataMode 			 = QSPI_PhaseMode_4bit;
	cmdStruct.DataCount 		 = count;
	
	QSPI_Command(QSPI0, QSPI_Mode_IndirectRead, &cmdStruct);
	
	RDMA_memcpy(buff, (void *)&QSPI0->DRW, RDMA_UNIT_WORD, count/4);
}


void QSPI1_Master_Init(void)
{
	QSPI_InitStructure QSPI_initStruct;
	
	PORT_Init(PORTC, PIN12, PORTC_PIN12_QSPI1_CK, 0);
	PORT_Init(PORTC, PIN11, PORTC_PIN11_QSPI1_CS, 0);
	PORT_Init(PORTC, PIN10, PORTC_PIN10_QSPI1_D0, 1);
	PORT_Init(PORTA, PIN11, PORTA_PIN11_QSPI1_D1, 1);
	PORT_Init(PORTA, PIN10, PORTA_PIN10_QSPI1_D2, 1);
	PORT_Init(PORTA, PIN9,  PORTA_PIN9_QSPI1_D3,  1);
	
	QSPI_initStruct.Size = QSPI_Size_16MB;
	QSPI_initStruct.ClkDiv = 4;
	QSPI_initStruct.ClkMode = QSPI_ClkMode_3;
	QSPI_initStruct.SampleShift = QSPI_SampleShift_NONE;
	QSPI_initStruct.IntEn = 0;
	QSPI_Init(QSPI1, &QSPI_initStruct);
	QSPI_Open(QSPI1);
}


void QSPI1_Master_Write(uint8_t *buff, uint32_t count)
{
	QSPI_CmdStructure cmdStruct;
	QSPI_CmdStructClear(&cmdStruct);
	cmdStruct.InstructionMode 	 = QSPI_PhaseMode_None;
	cmdStruct.AddressMode 		 = QSPI_PhaseMode_None;
	cmdStruct.AlternateBytesMode = QSPI_PhaseMode_None;
	cmdStruct.DataMode 			 = QSPI_PhaseMode_4bit;
	cmdStruct.DataCount 		 = count;
	
	QSPI_Command(QSPI1, QSPI_Mode_IndirectWrite, &cmdStruct);
	
	uint32_t n_word = count / 4;
	
	for(int i = 0; i < n_word; i++)
	{
		uint32_t * p_word = (uint32_t *)buff;
		
		while(QSPI_FIFOSpace(QSPI1) < 4) __NOP();
		
		QSPI1->DRW = p_word[i];
	}
	
	if((count % 4) / 2)
	{
		uint16_t * p_half = (uint16_t *)&buff[n_word * 4];
		
		while(QSPI_FIFOSpace(QSPI1) < 2) __NOP();
		
		QSPI1->DRH = p_half[0];
	}
	
	if(count % 2)
	{
		while(QSPI_FIFOSpace(QSPI1) < 1) __NOP();
		
		QSPI1->DRB = buff[count - 1];
	}
	
	while(QSPI_Busy(QSPI1)) __NOP();
}


void MemoryInit(void)
{
	PSRAM_InitStructure PSRAM_initStruct;
	
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
	
	PSRAM_initStruct.RowSize = PSRAM_RowSize_1KB;
	PSRAM_initStruct.tRWR = 50;
	PSRAM_initStruct.tACC = 50;
	PSRAM_initStruct.tCSM = 4;
	PSRAM_Init(&PSRAM_initStruct);
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
